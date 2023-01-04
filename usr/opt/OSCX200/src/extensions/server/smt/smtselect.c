/*
 * *****************************************************************
 * *                                                               *
 * *    Copyright (c) Digital Equipment Corporation, 1991, 1994    *
 * *                                                               *
 * *   All Rights Reserved.  Unpublished rights  reserved  under   *
 * *   the copyright laws of the United States.                    *
 * *                                                               *
 * *   The software contained on this media  is  proprietary  to   *
 * *   and  embodies  the  confidential  technology  of  Digital   *
 * *   Equipment Corporation.  Possession, use,  duplication  or   *
 * *   dissemination of the software and media is authorized only  *
 * *   pursuant to a valid written license from Digital Equipment  *
 * *   Corporation.                                                *
 * *                                                               *
 * *   RESTRICTED RIGHTS LEGEND   Use, duplication, or disclosure  *
 * *   by the U.S. Government is subject to restrictions  as  set  *
 * *   forth in Subparagraph (c)(1)(ii)  of  DFARS  252.227-7013,  *
 * *   or  in  FAR 52.227-19, as applicable.                       *
 * *                                                               *
 * *****************************************************************
 */
/*
 * HISTORY
 */
#ifndef lint
static char rcsid[] = "$Header: /alphabits/u3/x11/ode/rcs/x11/src/extensions/server/smt/smtselect.c,v 1.1.2.4 92/08/25 13:48:23 Jim_Ludwig Exp $";
#endif

/***********************************************************
Copyright 1990 by Digital Equipment Corporation, Maynard, Massachusetts,
and the Massachusetts Institute of Technology, Cambridge, Massachusetts.

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its 
documentation for any purpose and without fee is hereby granted, 
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in 
supporting documentation, and that the names of Digital or MIT not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.  

DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.

******************************************************************/

#include <stdio.h>
#include <sys/param.h>
#include <signal.h>
#include <errno.h>

#include "Xos.h"			/* for strings, fcntl, time */
#include "X.h"
#include "misc.h"

#define _SMT_SERVER_
#include "smtstr.h"

#include "osdep.h"
#include "dixstruct.h"

extern int  AllSockets[];
extern int  AllClients[];
extern int  LastSelectMask[];
extern int  ClientsWithInput[];
extern int  ConnectionTranslation[];

/* Keep track of all clients using shared-memory */
int  AllSmtClients[mskcnt];
#ifdef SMT_SEM
long *SmtServerStatus[MAXSOCKS];
#endif SMT_SEM
long SmtHowManyClients;

extern int errno;

#if (mskcnt>4)
/*
 * This is a macro if mskcnt <= 4
 */
ANYSET(src)
    int 	*src;
{
    int i;

    for (i=0; i<mskcnt; i++)
	if (src[ i ])
	    return (TRUE);
    return (FALSE);
}
#endif


/******************************
 * SmtSelect():
 *	X version of select() to suspend server waiting for something
 *	to do.  However, we must check whether there is any data ready
 *	on the shared memory connections first.  This code is derived
 *	from XSelect(), which is used by SM_UNIXCONN.
 ******************************/
int SmtSelect(maxsocks, rmask, wmask, emask, timeout)
int maxsocks;
int  *rmask, *wmask, *emask;
struct timeval *timeout;
{
    int nready = 0;
    int readyClients[mskcnt];
    int tmpmask[mskcnt];
    int status, i, j;
    int wi = 0;
    int nwords;
    int doneNormalSelectAlready = FALSE;
    int Rescan4SharedMemoryData = TRUE;
#ifdef SMT_SEM
    int ServerStaysAwake = FALSE;
#endif SMT_SEM

    /*
     * There are two implementations, depending on whether SMT_SEM is
     * defined or not.  Currently, SMT_SEM is not defined.  If SM_SEM
     * is defined, then using the socket for synchronization messages
     * is minimized by having server and client notify each other
     * (via shared memory) when they are awake/sleeping: this
     * requires critical sections.
     * Scan all clients to see if they are shared memory clients, and if so,
     * whether there is data ready to be read.  Wait until the last
     * possible moment to tell an SM client that the server may be
     * sleeping.  If *any* connection has new data, we can unilaterally
     * tell all clients not the send wakeup requests, and furthermore
     * can stop making this test for subsequent clients tested.  The
     * critical section is needed to prevent the server from sleeping
     * until the client(s) have decided what to do.  Without this,
     * deadlock is possible.
     *
     * If SMT_SEM is not defined, then this routine is only concerned
     * with determining which clients (both SMT and non-SMT) are ready.
     */
    while (Rescan4SharedMemoryData) {

	/* do this now to guard against race conditions */
	CLEARBITS(readyClients);
	MASKANDSETBITS(tmpmask, rmask, AllSmtClients);

	if (ANYSET(tmpmask)) {
	    register int curclient, i;
	    int *pdata;

	    for (i=0; i<mskcnt; i++) {
		while (tmpmask[i]) {
		    register smtControlBlock *pSmt;
		    register ClientPtr client;
		    register clientindex;

		    curclient = ffs(tmpmask[i]) - 1 + (i << 5);
		    clientindex = ConnectionTranslation[curclient];
		    client = clients[clientindex];

		    if ( client && (pSmt = 
		     (smtControlBlockPtr)((OsCommPtr)client->osPrivate)->pSmt))
		    {
		        int avail;

#ifdef SMT_SEM
			if (ServerStaysAwake == FALSE) {
			    ipEnterSection(&pSmt->chan, NULL);
			    pSmt->pPriv->server.status |= SMT_NEED_WAKEUP;
			    SmtServerStatus[wi++] =
				&pSmt->pPriv->server.status;
			    ipLeaveSection(&pSmt->chan, NULL);
			}
#endif SMT_SEM
			/*
			 * If the client died, we have to close it down
			 * now.  Otherwise there is a race condition
			 * whereby the server starts selecting before
			 * the client has shutdown the socket, leading
			 * to a hang.
			 */
			status = ipReceiveDataAvail(&pSmt->chan, &nwords,
						    &pdata, &avail);
			if (status != IP_SUCCESS) {
			    CloseDownClient(client);
			} else if (nwords > 0) {
			    BITSET(readyClients, curclient);
			    nready++;
#ifdef SMT_SEM
			    if (ServerStaysAwake == FALSE) {
				ServerStaysAwake = TRUE;
				for (j=wi-1; j>=0; j--)
				    *SmtServerStatus[j] &= ~SMT_NEED_WAKEUP;
			    } /* end if */
#endif SMT_SEM
			}
		    } /* end if SMT client */
		    BITCLEAR(tmpmask, curclient);
		} /* end while */
	    } /* end for */
	} /* end if any SMT clients ready */

	/* presumably the server has work to do, so clear wakeup bits */
	if (doneNormalSelectAlready) {
#ifdef SMT_SEM
	    for (j=wi-1; j>=0; j--)
		*SmtServerStatus[j] &= ~SMT_NEED_WAKEUP;
#endif SMT_SEM
	    return(nready);
	}

	/*
	 * At this point, readyClients[] has bits set for SM connections with
	 * ready data.  If work exists, then poll to pickup any additional 
	 * requests, else select() as usual (to avoid starving non-SM clients).
	 */
	if (nready > 0) {
	    register int n;
	    struct timeval notime;

	    notime.tv_sec = notime.tv_usec = 0;
	    /*
	     * Don't select on clients which already have input pending
	     * (otherwise they'll be counted twice).
	     */
	    UNSETBITS(rmask, readyClients);
	    n = select(maxsocks, 
		(int *)rmask, (int *)wmask, (int *)emask, &notime);
	    if (n > 0) {
		ORBITS(rmask, readyClients, rmask);
		nready += n;
	    } else if (n < 0) {		/* phew!!! Somebody died!!! */
		COPYBITS(readyClients, rmask);
		return n;
	    } else {
		COPYBITS(readyClients, rmask);
	    }
	    return(nready);
	} /* end if */

	nready = select(maxsocks, rmask, wmask, emask, timeout);

	/* clients no longer have to wakeup the server */
#ifdef SMT_SEM
	for (j=wi-1; j>=0; j--)
	    *SmtServerStatus[j] &= ~SMT_NEED_WAKEUP;
#endif SMT_SEM
	if (nready <= 0) {
	    return(nready);
	} 
#ifdef SMT_SEM
	else
	{
	    ServerStaysAwake = TRUE;
	}
#endif SMT_SEM
	/*
	 * Take another peek at the shared-memory clients to see
	 * if there is any work.  This is a cheap test since it
	 * involves no syscalls, so its worthwhile to try again
	 */
	doneNormalSelectAlready = TRUE;
    } /* end while(Rescan4SharedMemoryData) */

} /* end SmtSelect() */
