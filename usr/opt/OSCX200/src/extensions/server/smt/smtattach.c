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
static char rcsid[] = "$Header: /usr/sde/osf1/rcs/x11/src/extensions/server/smt/smtattach.c,v 1.1.4.2 1993/09/03 20:28:42 Dave_Hill Exp $";
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
#include "Xos.h"
#include "Xmd.h"
#include <errno.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/uio.h>
#include "X.h"

#define _SMT_SERVER_
#include "smtstr.h"

#include "os.h"
#include "osdep.h"
#include "opaque.h"
#include "dixstruct.h"
#include "misc.h"

extern abort();

extern int  SmtReadRequestFromClient();		/* imported from smtio.c */
extern int  SmtDeferRRQFromClient();		/* imported from smtio.c */
extern int  AllSmtClients[];			/* imported from smtselect.c */
extern long SmtHowManyClients;			/* imported from smtselect.c */
Bool SmtDeferFree = FALSE;

/******************************
 * SmtSetDeferFree()
 ******************************/
SmtSetDeferFree(mode)
    Bool mode;
{
    SmtDeferFree = (mode != 0);
} /* end SmtSetDeferFree */


/******************************
 * SmtAttach():
 *	Attempt to attach a shared memory segment (created by the
 *	client) to the address space of the server.  If successful,
 *	allocate and initialize the (per-client) smt control block.
 *	also allocate the SMT Queue structure.  This structure is
 *	used to pipeline requests to autonomous graphics accelerators
 *	such as the PXG.  When a request is made, the space it is
 *	occupying is registered here.  When the graphics accelerator
 *	is finished with it, the space is freed.
 ******************************/
Bool
SmtAttach(client, chan, flags, majorVersion, minorVersion)
    ClientPtr client;
    ChannelPtr chan;
    int flags;
    CARD8 majorVersion, minorVersion;
{
    register int i;
    register smtControlBlockPtr pSmt;
    int privsize;
    SmtPrivatePtr pPriv;

    if ( ipAttachChannel(chan) != IP_SUCCESS ) return FALSE;

    pSmt = (smtControlBlockPtr) Xalloc(sizeof(smtControlBlock));
    bzero(pSmt, sizeof(smtControlBlock));

    if (flags & SMT_NEWVER) {
	pSmt->majorVersion = majorVersion;
	pSmt->minorVersion = minorVersion;
    }

    pSmt->chan = *chan;

    /*
     * Allocate additional space (shared between server and client),
     * to keep track of state.
     */
    if((ipPrivateBuffer(&pSmt->chan, &privsize, &pPriv)) != IP_SUCCESS) {
	return FALSE;
    }
    pPriv->server.status = 0;
    pPriv->server.pid = getpid();
    pSmt->pPriv = pPriv;
    pSmt->wakeRec = 0;		/* statistics */
    pSmt->wakeSent = 0;		/* statistics */

    if (SmtDeferFree) {
	/* Initialize SMT queue structure */
	for ( i = SMT_Q_DEPTH-1; i >= 0; i--) {	/* SMT_QUEUE */
	    pSmt->q.e[i].freep = abort;
	    pSmt->q.e[i].ptr = (unsigned long *)0xdeadbabe;
	}
	pSmt->ReadRequestFromClient = SmtDeferRRQFromClient;
    } else {
	pSmt->ReadRequestFromClient = SmtReadRequestFromClient;
    }
    SmtHowManyClients++;
    ((OsCommPtr)client->osPrivate)->pSmt = pSmt;
    BITSET(AllSmtClients, ((OsCommPtr)client->osPrivate)->fd);
    return TRUE;
} /* end SmtAttach() */


#ifdef IP_STATS
extern int ipnFreed;
extern int ipnConflict;
extern int ipnMaxWait1;
extern int ipnMaxWait2;
extern int ipnEnter;
extern int ipnLeave;
#endif IP_STATS


/******************************
 * SmtCloseDownConnection()
 *	Called when a client is exiting, to clean up all SMT-related
 *	state.
 ******************************/
int
SmtCloseDownConnection(client, oc)
    ClientPtr client;
    OsCommPtr oc;
{
    int status;

#ifdef IP_STATS
    ErrorF("ipnFreed=%d wakeSent=%d wakeRec=%d\n", ipnFreed,
		oc->pSmt->wakeSent,
		oc->pSmt->wakeRec);
    ErrorF("ipnEnter=%d ipnLeave=%d ipnConflict=%d mw1=%d mw2=%d\n",
		ipnEnter, ipnLeave, ipnConflict, ipnMaxWait1, ipnMaxWait2);
#endif IP_STATS
    SmtHowManyClients--;
    status = ipDestroyChannel(&oc->pSmt->chan);
    BITCLEAR(AllSmtClients, (oc->fd));
    if (oc->pSmt) {
	Xfree(oc->pSmt);
	oc->pSmt = NULL;
    }

} /* end SmtCloseDownConnection() */
