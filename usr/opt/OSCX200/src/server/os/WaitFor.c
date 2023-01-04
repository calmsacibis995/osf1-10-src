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
/***********************************************************
Copyright 1987 by Digital Equipment Corporation, Maynard, Massachusetts,
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

/* $XConsortium: WaitFor.c,v 1.57 92/03/13 15:47:39 rws Exp $ */

/*****************************************************************
 * OS Depedent input routines:
 *
 *  WaitForSomething,  GetEvent
 *
 *****************************************************************/

#include "Xos.h"			/* for strings, fcntl, time */

#include <errno.h>
#include <stdio.h>
#include "X.h"
#include "misc.h"

#include <sys/param.h>
#include <signal.h>
#include "osdep.h"
#include "dixstruct.h"
#include "opaque.h"
#ifdef SMT
extern int SmtHowManyClients;          /* imported from smtselect.c */
#endif SMT

extern int AllSockets[];
extern int AllClients[];
extern int LastSelectMask[];
extern int WellKnownConnections;
extern int EnabledDevices[];
extern int ClientsWithInput[];
extern int ClientsWriteBlocked[];
extern int OutputPending[];

#if LONG_BIT == 64
extern int ScreenSaverTime;               /* milliseconds */
extern int ScreenSaverInterval;               /* milliseconds */
#else /* LONG_BIT == 32 */
extern long ScreenSaverTime;               /* milliseconds */
extern long ScreenSaverInterval;               /* milliseconds */
#endif /* LONG_BIT */
extern int ConnectionTranslation[];

extern Bool NewOutputPending;
extern Bool AnyClientsWriteBlocked;

extern WorkQueuePtr workQueue;

extern void CheckConnections();
extern Bool EstablishNewConnections();
extern void SaveScreens();
extern void ResetOsBuffers();
extern void ProcessInputEvents();
extern void BlockHandler();
extern void WakeupHandler();

extern int errno;

#ifdef apollo
extern long apInputMask[];

static long LastWriteMask[mskcnt];
#endif

#ifdef XTESTEXT1
/*
 * defined in xtestext1dd.c
 */
extern int playback_on;
#endif /* XTESTEXT1 */

#ifdef  XDPS
int xDPSReady = 0;
  /* This flag is used by the X/DPS extension to indicate that  */
  /* DPS has contexts that are runnable.                        */
extern ClientPtr serverClient;
int ClientsWithOverflow[mskcnt];
int AnyClientsWithOverflow;
void XDPSPrivInitOverflow();
#endif  XDPS


/*****************
 * WaitForSomething:
 *     Make the server suspend until there is
 *	1. data from clients or
 *	2. input events available or
 *	3. ddx notices something of interest (graphics
 *	   queue ready, etc.) or
 *	4. clients that have buffered replies/events are ready
 *
 *     If the time between INPUT events is
 *     greater than ScreenSaverTime, the display is turned off (or
 *     saved, depending on the hardware).  So, WaitForSomething()
 *     has to handle this also (that's why the select() has a timeout.
 *     For more info on ClientsWithInput, see ReadRequestFromClient().
 *     pClientsReady is an array to store ready client->index values into.
 *****************/

#if LONG_BIT == 64
static int timeTilFrob = 0;		/* while screen saving */
#else /* LONG_BIT == 32 */
static long timeTilFrob = 0;		/* while screen saving */
#endif /* LONG_BIT */

int
WaitForSomething(pClientsReady)
    int *pClientsReady;
{
    int i;
    struct timeval waittime, *wt;
#if LONG_BIT == 64
    int timeout;
#else /* LONG_BIT == 32 */
    long timeout;
#endif /* LONG_BIT */
    int clientsReadable[mskcnt];
    int clientsWritable[mskcnt];
    int curclient;
    int selecterr;
    int nready = 0;
    int devicesReadable[mskcnt];

#ifdef XDPS
    static int firstTime = 1;
    
    if (firstTime)
    {
        XDPSPrivInitOverflow();
        firstTime = 0;
    }
#endif XDPS

    CLEARBITS(clientsReadable);

    /* We need a while loop here to handle 
       crashed connections and the screen saver timeout */
    while (1)
    {
	/* deal with any blocked jobs */
	if (workQueue)
	    ProcessWorkQueue();

	if (ANYSET(ClientsWithInput))
	{
	    COPYBITS(ClientsWithInput, clientsReadable);
	    break;
	}
	if (ScreenSaverTime)
	{
	    timeout = (ScreenSaverTime -
		       (GetTimeInMillis() - lastDeviceEventTime.milliseconds));
	    if (timeout <= 0) /* may be forced by AutoResetServer() */
	    {
#if LONG_BIT == 64
		int timeSinceSave;
#else /* LONG_BIT == 32 */
		long timeSinceSave;
#endif /* LONG_BIT */

		timeSinceSave = -timeout;
		if ((timeSinceSave >= timeTilFrob) && (timeTilFrob >= 0))
		{
		    ResetOsBuffers(); /* not ideal, but better than nothing */
		    SaveScreens(SCREEN_SAVER_ON, ScreenSaverActive);
		    if (ScreenSaverInterval)
			/* round up to the next ScreenSaverInterval */
			timeTilFrob = ScreenSaverInterval *
				((timeSinceSave + ScreenSaverInterval) /
					ScreenSaverInterval);
		    else
			timeTilFrob = -1;
		}
		timeout = timeTilFrob - timeSinceSave;
	    }
	    else
	    {
		if (timeout > ScreenSaverTime)
		    timeout = ScreenSaverTime;
		timeTilFrob = 0;
	    }
	    if (timeTilFrob >= 0)
	    {
		waittime.tv_sec = timeout / MILLI_PER_SECOND;
		waittime.tv_usec = (timeout % MILLI_PER_SECOND) *
					(1000000 / MILLI_PER_SECOND);
		wt = &waittime;
	    }
	    else
	    {
		wt = NULL;
	    }
	}
	else
	    wt = NULL;
	COPYBITS(AllSockets, LastSelectMask);
#ifdef apollo
        COPYBITS(apInputMask, LastWriteMask);
#endif
	BlockHandler((pointer)&wt, (pointer)LastSelectMask);
	if (NewOutputPending)
	    FlushAllOutput();
#ifdef XTESTEXT1
	/* XXX how does this interact with new write block handling? */
	if (playback_on) {
	    wt = &waittime;
	    XTestComputeWaitTime (&waittime);
	}
#endif /* XTESTEXT1 */
	/* keep this check close to select() call to minimize race */
	if (dispatchException)
	    i = -1;
#ifdef  XDPS
        else
            /* If DPS is ready, check to see if there is any X activity */
            /* If there is, proceed as if we had done the normal select */
            /* however don't actually repeat the select. If there is no */
            /* X activity, simply return the DPS activity. If DPS isn't */
            /* ready, we take the 'else' case and do the normal select  */
            /* blocking if necessary.                                   */
            if (xDPSReady)
              {
              static struct timeval pollTime = {0, 0};
#if LONG_BIT == 64
	      int *writefds;
	      int readfds[mskcnt];
#else
	      long *writefds;
	      long readfds[mskcnt];
#endif
              if (AnyClientsWriteBlocked)
                {
                COPYBITS(ClientsWriteBlocked, clientsWritable);
                writefds = &clientsWritable[0];
                }
              else writefds = NULL;
              i = select(MAXSOCKS, (int *)LastSelectMask, (int *)writefds, 
			 (int *)NULL, &pollTime);
              if (AnyClientsWithOverflow && i > 0)
                {
                /* If the only fds with activity are clients with overflow,
                   ignore them and let DPS run */
                COPYBITS(LastSelectMask, readfds);
                UNSETBITS(readfds, ClientsWithOverflow);
                if (!ANYSET(readfds) &&
                    !(writefds && ANYSET(writefds)))
                      i = 0;
                }
              if (i == 0)
		/* No X clients ready, but there are dps contexts to run */
                {
                WakeupHandler((unsigned int)i, (pointer)LastSelectMask);
                pClientsReady[nready++] = serverClient->index;
                return nready;
                }
              }
	    /* Fall thru if either there are no dps contexts ready, or
	       there are, but there are also clients ready */
#endif  /* XDPS */
	else if (AnyClientsWriteBlocked)
	{
	    COPYBITS(ClientsWriteBlocked, clientsWritable);
#ifdef SMT
	    i = (SmtHowManyClients > 0) ?
	        SmtSelect(MAXSOCKS, (int *)LastSelectMask, 
			  (int *)clientsWritable,(int *)NULL, wt) :
		select(MAXSOCKS, (int *)LastSelectMask, (int *)clientsWritable,
			(int *)NULL, wt);
#else SMT
	    i = select (MAXSOCKS, (int *)LastSelectMask,
			(int *)clientsWritable, (int *) NULL, wt);
#endif SMT
	}
	else
#ifdef apollo
	    i = select (MAXSOCKS, (int *)LastSelectMask,
			(int *)LastWriteMask, (int *) NULL, wt);
#else
#ifdef SMT
	    i = (SmtHowManyClients > 0) ?
		SmtSelect(MAXSOCKS, (int *)LastSelectMask,(int *)NULL,
			  (int *)NULL, wt) :
			      select(MAXSOCKS, (int *)LastSelectMask,
				     (int *)NULL, (int *)NULL, wt);
#else SMT
	    i = select (MAXSOCKS, (int *)LastSelectMask,
			(int *) NULL, (int *) NULL, wt);
#endif SMT
#endif
	selecterr = errno;
	WakeupHandler((unsigned int)i, (pointer)LastSelectMask);
#ifdef XTESTEXT1
	if (playback_on) {
	    i = XTestProcessInputAction (i, &waittime);
	}
#endif /* XTESTEXT1 */
	if (i <= 0) /* An error or timeout occurred */
	{

	    if (dispatchException)
		return 0;
	    CLEARBITS(clientsWritable);
	    if (i < 0) 
		if (selecterr == EBADF)    /* Some client disconnected */
		{
		    CheckConnections ();
		    if (! ANYSET (AllClients))
			return 0;
		}
		else if (selecterr != EINTR)
		    ErrorF("WaitForSomething(): select: errno=%d\n",
			selecterr);
	    if (*checkForInput[0] != *checkForInput[1])
		return 0;
	}
	else
	{
	    if (AnyClientsWriteBlocked && ANYSET (clientsWritable))
	    {
		NewOutputPending = TRUE;
		ORBITS(OutputPending, clientsWritable, OutputPending);
		UNSETBITS(ClientsWriteBlocked, clientsWritable);
		if (! ANYSET(ClientsWriteBlocked))
		    AnyClientsWriteBlocked = FALSE;
	    }

	    MASKANDSETBITS(devicesReadable, LastSelectMask, EnabledDevices);
#ifdef	hpux
		    /* call the HIL driver to gather inputs. 	*/
	    if (ANYSET(devicesReadable)) store_inputs (devicesReadable);
#endif /* hpux */

	    MASKANDSETBITS(clientsReadable, LastSelectMask, AllClients); 
	    if (LastSelectMask[0] & WellKnownConnections) 
		QueueWorkProc(EstablishNewConnections, NULL,
			      (pointer)LastSelectMask[0]);
	    if (ANYSET (devicesReadable) || ANYSET (clientsReadable))
		break;
	}
    }

#ifdef  XDPS
    if (xDPSReady)
      pClientsReady[nready++] = serverClient->index;
#endif  /* XDPS */
    if (ANYSET(clientsReadable))
    {
#ifdef XDPS
        /* Whether DPS runs or not, don't read from overflow clients */
        if (AnyClientsWithOverflow)
            UNSETBITS(clientsReadable, ClientsWithOverflow);
#endif /* XDPS */
	for (i=0; i<mskcnt; i++)
	{
	    while (clientsReadable[i])
	    {
		curclient = ffs (clientsReadable[i]) - 1;
		pClientsReady[nready++] = 
			ConnectionTranslation[curclient + (i << 5)];
		clientsReadable[i] &= ~(1 << curclient);
	    }
	}	
    }
    return nready;
}

#ifndef ANYSET
/*
 * This is not always a macro.
 */
ANYSET(src)
    long	*src;
{
    int i;

    for (i=0; i<mskcnt; i++)
	if (src[ i ])
	    return (TRUE);
    return (FALSE);
}
#endif
#ifdef XDPS
void
XDPSPrivSetOverflow(client, val) 
    register ClientPtr client;
    register int val;
{
    register OsCommPtr oc = (OsCommPtr) client->osPrivate;
    
    if (val)
      BITSET(ClientsWithOverflow, oc->fd);
    else
      BITCLEAR(ClientsWithOverflow, oc->fd);
    AnyClientsWithOverflow = (ANYSET(ClientsWithOverflow)) ? TRUE : FALSE;
}

void
XDPSPrivInitOverflow()
{
    CLEARBITS(ClientsWithOverflow);
    AnyClientsWithOverflow = FALSE;
}
#endif /* XDPS */
