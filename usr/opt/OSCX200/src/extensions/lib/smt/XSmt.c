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
static char rcsid[] = "$Header: /usr/sde/osf1/rcs/x11/src/extensions/lib/smt/XSmt.c,v 1.1.4.3 1993/09/03 20:28:11 Dave_Hill Exp $";
#endif

/****************************************************************************
**                                                                          *
**            Copyright (c) Digital Equipment Corporation, 1990.            *
**            All Rights Reserved.  Unpublished rights reserved             *
**            under the copyright laws of the United States.                *
**                                                                          *
**            The software contained on this media is proprietary           *
**            to and embodies the confidential technology of                *
**            Digital Equipment Corporation.  Possession, use,              *
**            duplication or dissemination of the software and              *
**            media is authorized only pursuant to a valid written          *
**            license from Digital Equipment Corporation.                   *
**                                                                          *
**            RESTRICTED RIGHTS LEGEND   Use, duplication, or               *
**            disclosure by the U.S. Government is subject to               *
**            restrictions as set forth in Subparagraph (c)(1)(ii)          *
**            of DFARS 252.227-7013, or in FAR 52.227-19, as                *
**            applicable.                                                   *
**                                                                          *
****************************************************************************/

/*
 * This is the Xlib interface to the SMT shared-memory extension.
 * This extension is not intended to be called by user applications,
 * but rather by Xlib after recognizing 1) the user has requested
 * shared memory; and 2) the local server is capable of supporting it.
 *
 * RCS:
 *    $Header: /usr/sde/osf1/rcs/x11/src/extensions/lib/smt/XSmt.c,v 1.1.4.3 1993/09/03 20:28:11 Dave_Hill Exp $
 */

#define NEED_EVENTS
#define NEED_REPLIES
#include "Xlibint.h"

#include <stdio.h>
#include <signal.h>
#include <strings.h>
#include "Xlib.h"
#include "Xext.h"			/* in ../include */
#include "extutil.h"			/* in ../include */
#include "ip.h"
#include "smtstr.h"			/* in ../include */
#include "XSmtint.h"

extern char *getenv();

static char *smt_extension_name = SMT_PROTOCOL_NAME ;
static xReq _dummy_request = { 0, 0, 0 };
#ifdef STATISTICS
#define	INCR(dpy,A,B)	{ if (dpy->statistics &&dpy->statistics->on) dpy->statistics->A += B ;}
#else
#define INCR(dpy,A,B)
#endif

/* indexed by display socket number */
XExtCodes	*smtExtCodes[128];
static Bool     TryExtension = FALSE;
#define SMT_STATS
#ifdef SMT_STATS
static int	nAllocFail = 0;
static int	wakeRec = 0;
static int	wakeSent = 0;
#endif SMT_STATS

Bool SmtWireToEvent();
Status SmtEventToWire();
Status _SmtTerminate();


/*****************************************************************************
 *                                                                           *
 *			   private utility routines                          *
 *                                                                           *
 *****************************************************************************/



/******************************
 * _SmtIpError()
 ******************************/
int
_SmtIpError(dpy, pSmt, status)
Display *dpy;
smtDisplayPtr pSmt;
int status;
{
    /* 
     * Use more familar error messages for the one likely to
     * be hit.
     * All others (likely to be Xlib coding errors) drop a core.
     */
    if (status == IP_SHUTDOWN) {
	errno = EPIPE;
	_XIOError(dpy);
    }
    fprintf(stderr, "Fatal SMT ip error, status=%d\n", status);
    abort();
} /* end _SmtIpError() */


/******************************
 * _SmtBufferOverflow()
 ******************************/
int
_SmtBufferOverflow(dpy, pSmt)
Display *dpy;
smtDisplayPtr pSmt;
{
    fprintf(stderr, "request too big for server\n");
    abort();
} /* end _SmtBufferOverflow() */


/******************************
 * _SmtSetTryExtension()
 ******************************/
int
_SmtSetTryExtension(newval)
int newval;
{
    int oldTry = TryExtension;

    TryExtension = newval;
    return oldTry;
} /* end _SmtSetTryExtension() */


/******************************
 * _SmtGetTryExtension()
 ******************************/
int
_SmtGetTryExtension()
{
    return TryExtension;
} /* end _SmtGetTryExtension() */


/******************************
 * _SmtAllocFail():
 *	Called when an allocation request fails.  Currently, the implementation
 *	depends on using the socket as an out-of-band signalling path.
 *	First a check is made to see if the server needs to be woken up.
 *	The server will set this bit if it is about to sleep for lack
 *	of work to do.  There are no race conditions here: if the server
 *	sets the wakeup flag after the test, it will empty the request
 *	queue, allowing the client to continue.  Eventually, SmtAllocFail
 *	will be called again, and the server will be woken up.
 *
 *      Even though sockets are used here to signal, they are less
 *	than desirable: much cruft is introduced as a result.  It would
 *	be interesting to experiment with alternatives.  One possiblity
 *	would be to use signals to cause the server to exit a select()
 *	wait.  Another possibility would be a modified form of polling:
 *	instead of sleeping invoke a new syscall which would do nothing
 *	other than force a reschedule.
 ******************************/
int
_SmtAllocFail(chan, nwords, dpy)
ChannelPtr chan;
int nwords;
Display *dpy;
{
    SmtPrivatePtr pPriv = ((smtDisplayPtr)dpy->pSmt)->pPriv;

#ifdef SMT_STATS
    nAllocFail++;
#endif SMT_STATS

#ifdef SMT_SEM
    ipEnterSection(&((smtDisplayPtr)dpy->pSmt)->chan, NULL);
    pPriv->client.status |= SMT_NEED_WAKEUP;
    if ( pPriv->server.status & SMT_NEED_WAKEUP ) {
	ipLeaveSection(&((smtDisplayPtr)dpy->pSmt)->chan, NULL);
	XSmtWakeupServer(dpy);
	pPriv->server.status &= ~SMT_NEED_WAKEUP;
    } else {
	ipLeaveSection(&((smtDisplayPtr)dpy->pSmt)->chan, NULL);
    }
#else SMT_SEM
    XSmtWakeupServer(dpy, SMT_NEED_WAKEUP);
#endif SMT_SEM

    _XWaitForWritable(dpy);
    return IP_RETRY;
} /* end _SmtAllocFail() */


/******************************
 * _SmtInit():
 ******************************/
#define ERRMSG \
"SMT-WARNING: extension not supported, using unix-domain socket instead\n"
int
_SmtInit(dpy)
Display	*dpy;
{
    XExtCodes	*pExtCodes;
    if(!smtExtCodes[ConnectionNumber(dpy)])
    {
        pExtCodes = XInitExtension(dpy, SMT_PROTOCOL_NAME);
	if(!pExtCodes) {
	    fprintf(stderr, ERRMSG);
	    return FALSE;
	}
	smtExtCodes[ConnectionNumber(dpy)] = pExtCodes;
	XESetWireToEvent(dpy, pExtCodes->first_event + XSmtWakeupNotify,
	  SmtWireToEvent);
	XESetEventToWire(dpy, pExtCodes->first_event + XSmtWakeupNotify,
	  SmtEventToWire);
	/* 
	 * don't register a close function !
	 * the function _SmtTerminate must be called outside
	 * of the extension closing loop in XCloseDisplay() to avoid 
	 * the possible problem of killing the connection before 
	 * everyone is really done with it.
	XESetCloseDisplay(dpy, pExtCodes->extension, _SmtTerminate);
	 */
    }
    return TRUE;
} /* end _SmtInit() */
#undef ERRMSG


/******************************
 * _SmtTerminate()
 ******************************/
Status
_SmtTerminate(dpy, codes)
Display	*dpy;
XExtCodes	*codes;
{
    if(smtExtCodes[ConnectionNumber(dpy)] == NULL)
    {
	printf("Terminating uninitialized display %d\n", 
	  ConnectionNumber(dpy));
	return (-1);
    }
    ipDestroyChannel(&((smtDisplayPtr)dpy->pSmt)->chan);
    Xfree((smtDisplayPtr)dpy->pSmt);
    Xfree (smtExtCodes[ConnectionNumber(dpy)]);
    smtExtCodes[ConnectionNumber(dpy)] = NULL;
    dpy->pSmt = NULL;
    return (Success);
} /* end _SmtTerminate() */


/******************************
 * SmtWireToEvent():
 *	convert a wire event in network format to a C event structure
 ******************************/
Bool
SmtWireToEvent (dpy, libevent, netevent)
    Display *dpy;
    XEvent *libevent;
    xEvent *netevent;
{

    switch ((netevent->u.u.type & 0x7f) -
      smtExtCodes[ConnectionNumber(dpy)]->first_event)
    {
      case XSmtWakeupNotify:
	{
	    XSmtWakeupEvent *ev;
	    xSmtWakeupEvent *event;
    
#ifdef SMT_STATS
	    wakeRec++;
#endif SMT_STATS
    	    ev = (XSmtWakeupEvent *) libevent;
	    event = (xSmtWakeupEvent *) netevent;
    	    ev->type = event->type & 0x7f;
	    /*  XXXXXXXXXXXXXXXXXXXX 
    	    ev->serial = _XSetLastRequestRead(dpy,(xGenericReply *) netevent);
	    */
    	    ev->serial = 0;
    	    ev->send_event = ((event->type & 0x80) != 0);
    	    ev->display = dpy;
    	    return FALSE;
	}
    } /* end switch */
    return False;
} /* end SmtWireToEvent() */


/******************************
 * SmtEventToWire():
 *	Convert a C event structure to a wire event in network format
 ******************************/
Status
SmtEventToWire (dpy, libevent, netevent)
    Display *dpy;
    XEvent  *libevent;
    xEvent  *netevent;
{

    _SmtInit(dpy);
    switch ((libevent->type & 0x7f) -
      smtExtCodes[ConnectionNumber(dpy)]->first_event)
    {
      case XSmtWakeupNotify:
	{
	    XSmtWakeupEvent *ev;
	    xSmtWakeupEvent *event;
    
    	    ev = (XSmtWakeupEvent *) libevent;
	    event = (xSmtWakeupEvent *) netevent;
    	    event->type = ev->type;
    	    event->sequenceNumber = (ev->serial & 0xffff);
    	    return 1;
	}
    } /* end switch */
    return 0;
} /* end SmtEventToWire() */


/*****************************************************************************
 *                                                                           *
 *		    smt public interfaces                                    *
 *                                                                           *
 *****************************************************************************/


/******************************
 * XSmtGetVersion():
 * 	Gets the major and minor version numbers of the extension.  The return
 * 	value is zero if an error occurs or non-zero if no error happens.
 ******************************/
Status
XSmtGetVersion (dpy, major_version_return, minor_version_return)
    Display *dpy;
    int *major_version_return, *minor_version_return;
{
    xSmtQueryVersionReply rep;
    register xSmtQueryVersionReq *req;

    LockDisplay (dpy);
    SmtGetReq(QueryVersion, req);
    if (!_XReply (dpy, (xReply *) &rep, 0, xTrue)) {
	UnlockDisplay (dpy);
	SyncHandle ();
	return 0;
    }
    *major_version_return = rep.majorVersion;
    *minor_version_return = rep.minorVersion;
    UnlockDisplay (dpy);

    SyncHandle ();
    return 1;
} /* end XSmtGetVersion() */


/******************************
 * XSmtAttach():
 * 	Attempt to create a shared memory segment.  If successful, send
 *	attach request to server.  If succesful reply, non-null value
 *	of pSmt field in the display structure indicates shared-memory
 *	transport is active.
 ******************************/
#define ERRMSG1 \
"SMT-WARNING: local shmem attach failed, check shmmax/shmseg config params\n"
#define ERRMSG2 \
"SMT-WARNING: server refused attach, check shmseg sys config params\n"
Status
XSmtAttach (dpy, flags, reqbufsize, repbufsize)
Display *dpy;
int flags;
int reqbufsize;
int repbufsize;
{
    xSmtAttachReply rep;
    register xSmtAttachReq *req;
    smtDisplayPtr pSmt;
    int status;
    int type;
    int mode;
    int privsize;
    SmtPrivatePtr pPriv;

    if (flags & SMT_REQ) {
	if (flags & SMT_REP) {
	    type = IP_FULLDUP;
	    mode = IP_BOTH;
	} else {
	    type = IP_HALFDUP;
	    mode = IP_SENDER;
	    repbufsize = 0;
	}
    } else {
	return 0;
    }

    /* assume Xmalloc returns zeroed block ??? */
    pSmt = (smtDisplayPtr) Xmalloc(sizeof(smtDisplay));
    bzero(pSmt, sizeof(smtDisplay));

    status = ipCreateChannel(&pSmt->chan, type, mode, reqbufsize,
			repbufsize, sizeof(SmtPrivate) >> 2);

    if (status != IP_SUCCESS) {
	fprintf(stderr, ERRMSG1);
	Xfree(pSmt);
	return 0;
    }

    /* needed for backwards compatibility */
#define ERRMSG1a \
"SMT-WARNING: Xlib SMT protocol (V%d/%d) newer than server (V%d/%d).\n"
#define ERRMSG1b \
"             Server upgrade recommended.\n"
    {
	int Maj, Min;

	XSmtGetVersion(dpy, &Maj, &Min);	/* returns ints */
	pSmt->majorVersion = Maj;
	pSmt->minorVersion = Min;
	if (Maj != SMT_MAJOR_VERSION || Min != SMT_MINOR_VERSION) {
	    fprintf(stderr, ERRMSG1a,
		    SMT_MAJOR_VERSION, SMT_MINOR_VERSION,
		    Maj, Min);
	    fprintf(stderr, ERRMSG1b);
	}
#undef ERRMSG1a
#undef ERRMSG1b
    }

    ipSetAllocCallback(&pSmt->chan, _SmtAllocFail, dpy);

    LockDisplay (dpy);
    SmtGetReq(Attach, req);
    req->chan = pSmt->chan;
    req->flags = flags | SMT_NEWVER;	/* backwards compatibility */
    req->majorVersion = SMT_MAJOR_VERSION;
    req->minorVersion = SMT_MAJOR_VERSION;

    if (!_XReply (dpy, (xReply *) &rep, 0, xTrue)) {
	UnlockDisplay (dpy);
	SyncHandle ();
	ipDestroyChannel(&pSmt->chan);
	Xfree(pSmt);
	fprintf(stderr, ERRMSG2);
	return 0;
    }

    if (!rep.status) {
	UnlockDisplay (dpy);
	SyncHandle ();
	ipDestroyChannel(&pSmt->chan);
	Xfree(pSmt);
	fprintf(stderr, ERRMSG2);
	return 0;
    }

    dpy->pSmt = pSmt;
    pSmt->buffer = dpy->buffer;
    pSmt->bufptr = dpy->bufptr;
    pSmt->bufmax = dpy->bufmax;
    dpy->buffer = NULL;

    if((status=ipPrivateBuffer(&pSmt->chan, &privsize, &pPriv)) != IP_SUCCESS) {
	_SmtIpError(dpy, pSmt, status);
	return 0;
    }
	
    pPriv->client.status = 0;
    pPriv->client.pid = getpid();
    pSmt->pPriv = pPriv;

    UnlockDisplay (dpy);

    SyncHandle ();
    return 1;
} /* end XSmtAttach() */
#undef ERRMSG2
#undef ERRMSG1

/******************************
 * XSmtWakeupServer():
 *	Send a Wakeup message to the server.  This has to go out
 *	over the wire, so fool macros into thinking that the
 *	shared memory extension is not active.
 ******************************/
Status
XSmtWakeupServer (dpy, flags)
Display *dpy;
int flags;
{
    register xSmtWakeupServerReq *req;
    smtDisplayPtr pSmt = (smtDisplayPtr)dpy->pSmt;

#ifdef SMT_STATS
    wakeSent++;
#endif SMT_STATS
    LockDisplay (dpy);
    dpy->pSmt = NULL;
    if (dpy->buffer) {		/* ??? should never happen */
	fprintf(stderr, "XSmtWakeupServer(): yowsa! yowsa! yowsa!\n");
    }
    dpy->buffer = pSmt->buffer;
    dpy->bufptr = pSmt->bufptr;
    dpy->bufmax = pSmt->bufmax;

    SmtGetReq(WakeupServer, req);
    req->flags = flags;
    {
	register long size, todo;
	register int write_stat;
	register char *bufindex;
	int (*sigval)();

/* XXXX	if (dpy->async_enabled) sigval = (int(*)())signal(SIGIO, SIG_IGN); */

	size = todo = dpy->bufptr - dpy->buffer;
	bufindex = dpy->bufptr = dpy->buffer;
	/*
	 * While write has not written the entire buffer, keep looping
	 * until the entire buffer is written.  bufindex will be incremented
	 * and size decremented as buffer is written out.
	 */
	while (size) {
	    errno = 0;
	    write_stat = write(dpy->fd, bufindex, (int) todo);
	    INCR(dpy,numwrite, 1);
	    if (write_stat > 0) {
		size -= write_stat;
		todo = size;
		bufindex += write_stat;
		INCR(dpy,bytewrite, write_stat);
	    } else if (errno == EWOULDBLOCK) {
		continue;
	    } else {
		/* Write failed! */
		/* errno set by write system call. */
		(*_XIOErrorFunction)(dpy);
	    }
	} /* end while */
	dpy->last_req = (char *)&_dummy_request;
/* XXXX	if (dpy->async_enabled) signal(SIGIO, sigval); */

    }
    dpy->pSmt = pSmt;
    dpy->buffer = NULL;
    /*
     * This request, because it is going over the wire, may be executed
     * out-of-order by the server.  When this happens, the server and client
     * no longer agree on the serial number of requests (since each is
     * incrementing a counter independently).  Since this request
     * 1) is the only one to go over the wire; 2) will never return
     * an error; 3) has no reply, the easiest way out is to pretend
     * it never existed by decrementing the request number.  On the
     * server side, the sequence number is also decremented, keeping
     * everything in sync. For backwards compatibility, it is necessary
     * to test the minor version.
     */
    if (pSmt->minorVersion) dpy->request--;
    UnlockDisplay (dpy);

    return 1;
} /* end XSmtWakeupServer() */


/******************************
 * _SmtCheckExtension(name);
 ******************************/
#define KOSHER "Adobe-DPS-Extension,DPSExtension,SHAPE,MIT-SHM,\
Multi-Buffering,XInputExtension,X3D-PEX,Wormhole Drawing Extension,\
MIT-SUNDRY-NONSTANDARD,XTEST,Keyboard-Management-Extension,DEC-XTRAP,\
XVideo,Xie"

Bool
_SmtCheckExtension(name)
char *name;
{
    char *penv = getenv("SMT_OVERRIDE");
    char *plist = (penv && (penv != (char *)-1)) ? penv : KOSHER;
    int match = FALSE;

    /*
     * The string "OVERRIDE" disables this check: otherwise, extract
     * each comma-delimited element in the string and compare it
     * to the extension being queried.  If no matches are found,
     * rudely terminate the client.
     */
    if (strcmp(plist,"OVERRIDE")) {			/* skip if == */
	char *cur = plist;
	char *next = NULL;
	char *end = plist + strlen(plist);

	while (cur < end) {
	    next=index(cur, ',');
	    if (!next) next = end;
	    if (!strncmp(name, cur, next-cur)) {	/* break if == */
		match = TRUE;
		break;
	    }
	    cur = next + 1;
	} /* end while list elements */
    } else 
	match = TRUE;
    return match;
} /* end _SmtCheckExtension() */
