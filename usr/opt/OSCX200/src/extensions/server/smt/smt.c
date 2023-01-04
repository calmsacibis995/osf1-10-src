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
static char rcsid[] = "$Header: /alphabits/u3/x11/ode/rcs/x11/src/extensions/server/smt/smt.c,v 1.1.2.4 92/08/25 13:47:52 Jim_Ludwig Exp $";
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

/* $Header: /alphabits/u3/x11/ode/rcs/x11/src/extensions/server/smt/smt.c,v 1.1.2.4 92/08/25 13:47:52 Jim_Ludwig Exp $ */
#include "Xos.h"			/* for strings, fcntl, time */

#include <stdio.h>
#include <sys/param.h>

#include "X.h"
#include "misc.h"

#define _SMT_SERVER_
#include "smtstr.h"

#include "extnsionst.h"
#include "dixstruct.h"
#include "resource.h"
/* XXXX #include "resourcest.h" */
#include "osdep.h"


static unsigned char	SmtReqCode;
static int		SmtEventBase;
static int		SmtErrorBase;


extern char *getenv();
extern void (* EventSwapVector[128]) ();
extern int AllSmtClients[];		/* imported from smtselect.c */
extern long SmtHowManyClients;		/* imported from smtselect.c */
extern int ClientsWithInput[];		/* imported from connection.c */
extern unsigned short StandardMinorOpcode(); /* imported from extension.c */
extern Bool SmtDeferFree;		/* imported from smtattach.c */
extern Bool SmtLocalConnection();	/* from smtutil.c */


/****************
 * SmtExtensionInit():
 *	Called from InitExtensions in main().  Register the
 *	extension, and initialize some global variables.
 *
 ****************/
void
SmtExtensionInit(argc, argv)
    int   argc;
    char *argv[];
{
    static void  SmtResetProc();
    ExtensionEntry *extEntry, *AddExtension();
    static int ProcSmtDispatch(), SProcSmtDispatch();
    char *pstr;

    if (extEntry = AddExtension(SMT_PROTOCOL_NAME,
				XSmtNumberOfEvents, 
				XSmtNumberOfErrors,
				ProcSmtDispatch, SProcSmtDispatch,
				SmtResetProc, StandardMinorOpcode)) {
	SmtReqCode = (unsigned char)extEntry->base;
	SmtEventBase = extEntry->eventBase;
	SmtErrorBase = extEntry->errorBase;
    }
    else
        ErrorF("SmtExtensionInit(): failed to AddExtension\n");

    CLEARBITS(AllSmtClients);
    SmtHowManyClients = 0;

    if (pstr = getenv("SMT_DEFER_FREE")) {
	SmtDeferFree = atoi(pstr);
	ErrorF("SmtExtensionInit(): set DeferFree to %d\n", SmtDeferFree);
    }

} /* end SmtExtensionInit() */


/******************************
 * SmtResetProc():
 *	This routine is called by CloseDownExtensions(),
 *	which in turn is called after return from Dispatch().
 *	Since SmtExtensionInit() (which is called before Dispatch())
 *	does not allocate any private storage, there really isn't
 *	anything which needs to be done here.
 ******************************/
/*ARGSUSED*/
static void
SmtResetProc (extEntry)
    ExtensionEntry	*extEntry;
{

} /* end SmtResetProc() */


/******************************
 * SmtWakeupClient():
 *	Send a wakeup event to the specified client.
 ******************************/
/*ARGSUSED*/
int
SmtWakeupClient (client)
    ClientPtr client;
{
    xSmtWakeupEvent ev;

    ((OsCommPtr)client->osPrivate)->pSmt->wakeSent++;
    ev.type = SmtEventBase + XSmtWakeupNotify;
    ev.sequenceNumber = client->sequence;
    ev.flags = 0;
    WriteEventsToClient(client, 1, &ev);
} /* end SmtWakeupClient() */


/******************************
 * ProcSmtQueryVersion():
 *	Execute the SMT Query Version request.
 ******************************/
static int
ProcSmtQueryVersion (client)
    register ClientPtr client;
{
    xSmtQueryVersionReply	rep;
    register int		n;
    REQUEST(xSmtQueryVersionReq);

    REQUEST_SIZE_MATCH (xSmtQueryVersionReq);
    rep.type = X_Reply;
    rep.length = 0;
    rep.sequenceNumber = client->sequence;
    rep.majorVersion = SMT_MAJOR_VERSION;
    rep.minorVersion = SMT_MINOR_VERSION;
    rep.uid = getuid();
    rep.gid = getgid();

    if (client->swapped) {
    	swaps(&rep.sequenceNumber, n);
    	swapl(&rep.length, n);
    }
    WriteToClient(client, sizeof (xSmtQueryVersionReply), (char *)&rep);
    return (client->noClientException);

} /* end ProcSmtQueryVersion() */


/******************************
 * ProcSmtAttach():
 *	Execute the SMT Attach request.  Attach the specified shared memory
 *	segemnt (already created by the client) to the server process'
 *	address space, and use this for further communication with the
 *	client.  Return status to the client.
 ******************************/
static int
ProcSmtAttach(client)
    register ClientPtr client;
{
    xSmtAttachReply rep;

    REQUEST(xSmtAttachReq);

    /* If client is not on the same CPU, return an error */
    if (SmtLocalConnection(client) == FALSE) {
	return BadImplementation;
    }

    REQUEST_AT_LEAST_SIZE(xSmtAttachReq);

    rep.status = SmtAttach(client, &stuff->chan, stuff->flags,
			   stuff->majorVersion, stuff->minorVersion);
    rep.type = X_Reply;
    rep.length = 0;
    rep.sequenceNumber = client->sequence;
    /* reply never swapped... don't need WriteReplyToClient */
    WriteToClient(client, sizeof(xSmtAttachReply), &rep);

    return Success;
} /* end ProcSmtAttach() */


SMT_DIAG(static smtControlBlockPtr zSmt = NULL;)
SMT_DIAG(static int nZ0=0\, nZ1=0\, nZ2=0\, nZ3=0;)

/******************************
 * ProcSmtWakeupServer():
 *	Execute the SMT Wakeup request.  If SMT semaphore semantics
 *	are in effect, indicate that the server has received a wakeup.
 ******************************/
static int
ProcSmtWakeupServer(client)
    register ClientPtr client;
{
    register smtControlBlockPtr pSmt = ((OsCommPtr)client->osPrivate)->pSmt;
    register SmtPrivatePtr pPriv = pSmt->pPriv;

    REQUEST(xSmtWakeupServerReq);
    SMT_DIAG(zSmt = pSmt;)

    /*
     * We assume that the client issuing this request is local (i.e. able
     * to get to the same memory as the server).  We don't test this
     * here (by calling SmtLocalConnection()), because this request needs
     * to be as cheap as possible.
     */

    REQUEST_AT_LEAST_SIZE(xSmtWakeupServerReq);

    pSmt->wakeRec++;
    /*
     * This request, because it always arrives over the wire, may be executed
     * out-of-order by the server.  When this happens, the server and client
     * no longer agree on the serial number of requests (since each is
     * incrementing a counter independently).  Since this request
     * 1) is the only one to go over the wire; 2) will never return
     * an error; 3) has no reply, the easiest way out is to pretend
     * it never existed by decrementing the request number.  On the
     * client side, the sequence number decremented when the request
     * is created, keeping everything in sync.  For backwards compatibility,
     * it is necessary to test the minor version.
     */
    if (pSmt->minorVersion) client->sequence--;
#ifdef SMT_SEM
    pPriv->server.status &= ~SMT_NEED_WAKEUP;	/* clear by client ??? */
#else SMT_SEM
    if (stuff->flags & SMT_NEED_WAKEUP) {
	register int fd = ((OsCommPtr)client->osPrivate)->fd;
	int nwords;
	int status;
	xReq *treq;
	SMT_DIAG(nZ1++;)
	/* if the client has no input, wake them up regardless */
	if (GETBIT(ClientsWithInput, fd)) {
	    SMT_DIAG(nZ2++;)
	    pPriv->client.status |= SMT_NEED_WAKEUP;
	} else {
	    SMT_DIAG(nZ3++;)
	    SmtWakeupClient(client);
	    pPriv->client.status &= ~SMT_NEED_WAKEUP;
	}
    }
#endif SMT_SEM

    return Success;
} /* end ProcSmtWakeupServer() */


/******************************
 * ProcSmtDispatch():
 *	Main dispatch procedure called when an extension request is
 *	received.
 ******************************/
static int
ProcSmtDispatch (client)
    register ClientPtr client;
{
    REQUEST(xReq);
    switch (stuff->data) {
    case X_SmtQueryVersion:
	return ProcSmtQueryVersion (client);
    case X_SmtAttach:
	return ProcSmtAttach (client);
    case X_SmtWakeupServer:
	return ProcSmtWakeupServer (client);
    default:
	return BadRequest;
    }
} /* end ProcSmtDispatch() */


/******************************
 * SProcSmtDispatch():
 *	Since this extension will always be used by a
 *	local client, this should never happen.
 ******************************/
static int
SProcSmtDispatch (client)
    register ClientPtr	client;
{
    return BadImplementation;
} /* end SProcSmtDispatch()  */
