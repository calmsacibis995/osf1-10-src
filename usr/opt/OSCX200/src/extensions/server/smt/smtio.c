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
static char rcsid[] = "$Header: /alphabits/u3/x11/ode/rcs/x11/src/extensions/server/smt/smtio.c,v 1.1.2.5 93/01/08 15:51:16 Jim_Ludwig Exp $";
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
#include "Xproto.h"

#define _SMT_SERVER_
#include "smtstr.h"

#include "os.h"
#include "osdep.h"
#include "opaque.h"
#include "dixstruct.h"
#include "misc.h"

extern int  ClientsWithInput[];		/* imported from connection.c */
static int  timesThisConnection = 0;
extern Bool SmtDeferFree;		/* imported from smtattach.c */

/*
 * For a multi-threaded server, these need to go into the
 * client structure. XXX
 */
static xReq *lastReq = NULL;
static int lastReqLen;
int smtLastReqDeferred = 0;		/* SMT_DEFER_FREE */
static smtControlBlockPtr lastSmt;	/* SMT_DEFER_FREE */

extern abort();

/*
 * Max requests to allow per client, before yielding.  This probably
 * should (thought it needn't) track the corresponding definition in
 * os/4.2bsd/io.c
 */
#define MAX_TIMES_PER         10

#define YieldControl()				\
        { isItTimeToYield = TRUE;		\
	  timesThisConnection = 0; }
#define YieldControlNoInput()			\
        { YieldControl();			\
	  BITCLEAR(ClientsWithInput, client); }
#define YieldControlAndReturnNull()		\
        { YieldControlNoInput();		\
	  who->requestBuffer = (pointer) NULL;  \
	  who->req_len = 0;			\
	  return(0); }
#define YieldControlWithInput()			\
        { YieldControl();			\
	  who->requestBuffer = (pointer)0xdeadbabe; \
	  return(0); }


/******************************
 * SmtFreeLastRequest()
 *	Free last outstanding request.  Warning: assumes single threaded,
 *	and that it is called *only* by a client which is just about
 *	to yield.  Check to see if the client needs to be woken up
 *	(e.g. it ran out of buffers).  This must be done in a critical
 *	section, in order to prevent deadlock.  For this reason,
 *	this action only occurs when yielding the connection
 *	(also, there is no point waking up the client until a significant
 *	chunk of the request buffer has been freed).
 ******************************/
void
SmtFreeLastRequest(who)
    ClientPtr who;
{
    register smtControlBlockPtr pSmt;
    
    /* return if bogus client */
    if ((!who) || (!who->osPrivate)) return;

    pSmt = ((OsCommPtr)who->osPrivate)->pSmt;

    /* if SMT client and request in progress */
    if (pSmt && lastReq != NULL) {
	if (SmtDeferFree) {
	    /* SmtGCRequests will fail if the client was shutdown */
	    if (SmtGCRequests(pSmt) == IP_SUCCESS) {
		SmtMaybeWakeupClient(who, pSmt);
		smtLastReqDeferred = 0;
	    }
	} else {
	    if (ipFreeData(&pSmt->chan, lastReqLen) == IP_SUCCESS)
		SmtMaybeWakeupClient(who, pSmt);
	    else
		FatalError("SmtFreeLastRequest(): ipFreeData() failed\n");
	}
	lastReq = NULL;
    }
} /* end SmtFreeLastRequest() */


/******************************
 * SmtReadRequestFromClient():
 *    Returns one request from client.  If the client misbehaves,
 *    returns NULL.
 *
 *        client:  index into bit array returned from WaitForSomething() 
 *
 *        status: status is set to
 *            > 0 the number of bytes in the request if the read is sucessful 
 *            = 0 if action would block (entire request not ready)
 *            < 0 indicates an error (probably client died)
 *
 ******************************/
/*ARGSUSED*/
int
SmtReadRequestFromClient(who)
    ClientPtr who;
{
    register int client = ((OsCommPtr)who->osPrivate)->fd;
    register smtControlBlockPtr pSmt = ((OsCommPtr)who->osPrivate)->pSmt;
    xReq *request;
    int nwords;
    int status;
    int avail;
    pointer oldbuf = who->requestBuffer;

    if (oldbuf != NULL && lastReq != NULL) {
	ipFreeData(&pSmt->chan, lastReqLen);
	lastReq = NULL;
    }

    status = ipReceiveDataAvail(&pSmt->chan, &nwords, &request, &avail);
    if (status != IP_SUCCESS ) {
	/* ip error status codes are always negative */
	YieldControlAndReturnNull();
    }

    /*
     * We may assume that the shared memory transport will never
     * release a request until it is complete (there is nothing
     * to be gained by having the client do this).  Therefore,
     * it suffices to determine if there is enough room for
     * a minimun request.
     */
    if ((nwords<<2) >= sizeof(xReq)) {	/* assume full request available */
	/*
	 * Whenever a request is found, its location and size are saved.
	 * The next time ReadRequest() is called, the space occupied
	 * by the request will be freed.  It is necessary to compare
	 * oldbuf to the last saved pointer before doing this because
	 * there is no guarantee that the previous buffer did not come
	 * in through the socket (e.g. wakeup requests).
	 */
	lastReq = request;
	lastReqLen = request->length;
	if (lastReqLen == 0) {
	    SMT_DIAG( ErrorF("Zero-Length request\n"))
	}

	BITSET( ClientsWithInput, client );
	if ((++timesThisConnection == MAX_TIMES_PER) || (isItTimeToYield)) {
	    YieldControl();
	}
	who->requestBuffer = (pointer)request;
	who->req_len = request->length;
    	return(request->length << 2);
    } else {				/* client idle */
	YieldControlAndReturnNull();
    }
	
} /* end SmtReadReqestFromClient() */


/******************************
 * SmtMaybeWakeupClient():
 *	Send a wakeup request to the client, if one is needed.
 *	With semaphore semantics, the server must interlock
 *	with the client; otherwise deadlock is possible (if the
 *	client says it needs a wakeup after the server has tested
 *	for this condition).  With non-semaphore semantics,
 *	no interlock is necessary, because XXX
 ******************************/
SmtMaybeWakeupClient(who, pSmt)
    ClientPtr who;
    smtControlBlockPtr pSmt;
{
#ifdef SMT_SEM
    ipEnterSection(&pSmt->chan, NULL);
    if (pSmt->pPriv->client.status & SMT_NEED_WAKEUP) {
	ipLeaveSection(&pSmt->chan, NULL);
	SmtWakeupClient(who);
	pSmt->pPriv->client.status &= ~SMT_NEED_WAKEUP;
    } else {
	ipLeaveSection(&pSmt->chan, NULL);
    }
#else SMT_SEM
    if (pSmt->pPriv->client.status & SMT_NEED_WAKEUP) {
	SmtWakeupClient(who);
	SMT_DIAG( ErrorF("wakeup client\n"); )
	pSmt->pPriv->client.status &= ~SMT_NEED_WAKEUP;
    }
#endif SMT_SEM
} /* end SmtMaybeWakeupClient() */


/* {}{}{}{}{}{}{}{}{}{}{}{}{}{}{}{}{}{}{}{}{}{}{}{}{}{}{}{}{}{}{}{}{}{} */


/******************************
 * SmtGCRequests():
 *	Garbage Collect outstanding requests.  Before the SMT_QUEUE
 *	optimization was added, this proc would just return the
 *	result of ipFreeData().  The SMT_QUEUE optimization allows
 *	multiple requests to be pending (in the sense that their
 *	space has not yet been freed).
 ******************************/
int
SmtGCRequests(pSmt)			/* SMT_QUEUE */
    smtControlBlockPtr pSmt;
{
    register int len = 0;
    register int last = pSmt->q.last;
    register int next = pSmt->q.next;

    if (last == next) {
	SMT_DIAG( ErrorF("smtgc noop\n"); )
	return IP_ABORT;		/* nothing to free... */
    }
    /*
     * loop through any previous requests that haven't been freed and
     * free them now.
     */
    do {
	register smtQEntryPtr pQ = &(pSmt->q.e[last]);
	if (pQ->ptr == 0 || (pQ->freep && 
			     (*pQ->freep)(pQ->Q, pQ->ptr, pQ->dat))) {
	    SMT_DIAG(
		ErrorF("smtgc free p%x(q%x d%x n%d)[i%d]\n"\,
		       pQ->freep\, pQ->ptr\, pQ->dat\, pQ->len\, last);
		)
	    len += pQ->len;
	    SMT_DIAG(
		pQ->ptr = (unsigned long *)0xdeadbabe;
		pQ->freep = abort;
		pQ->len = 0;
		);
	    last++;
	    if (last == SMT_Q_DEPTH)
		last = 0;
	} else {
	    SMT_DIAG(
	        ErrorF("smtgc busy p%x(q%x d%x n%d)[i%d]\n"\,
		       pQ->freep\, pQ->ptr\, pQ->dat\, pQ->len\, last);
	        );
	    break;
	}
    } while (last != next);

    pSmt->q.last = last;

    /*
     * If any finished requests were found in the previous pass,
     * call into the ip layer to free up their space.
     */
    if (len) {
	if (ipBulkFreeData(&pSmt->chan, len) != IP_SUCCESS)
	    FatalError("SmtGCRequests(): smtgc bulk free\n");
	return IP_SUCCESS;
    }
    return IP_RETRY;
} /* end SmtGCRequests() */


/******************************
 * SmtDeferRRQFromClient():
 *    Returns one request from client.  If the client misbehaves,
 *    returns NULL.
 *
 *        client:  index into bit array returned from WaitForSomething() 
 *
 *        status: status is set to
 *            > 0 the number of bytes in the request if the read is sucessful 
 *            = 0 if action would block (entire request not ready)
 *            < 0 indicates an error (probably client died)
 *
 ******************************/
/*ARGSUSED*/
int
SmtDeferRRQFromClient(who)
    ClientPtr who;
{
    register int client = ((OsCommPtr)who->osPrivate)->fd;
    register smtControlBlockPtr pSmt = ((OsCommPtr)who->osPrivate)->pSmt;
    xReq *request;
    int nwords;
    int status;
    int avail;
    pointer oldbuf = who->requestBuffer;

    if (smtLastReqDeferred && SmtGCRequests(pSmt) == IP_RETRY) {
	/*
	 * last time we were here, we wouldn't read a request because there
	 * wasn't room for the client or the graphics board was still busy.
	 * if the condition persists, then we needn't proceed further, since we
	 * haven't freed any space for the client...
	 */
        YieldControlWithInput();
    }
    else if (oldbuf != NULL && lastReq != NULL) {
	/*
	 * stripped-down, in-line version of GCRequests.  if we can do a quick
	 * free, then do it so that the client will have space when it looks,
	 * and we won't have to select waiting for input.  if not, no big deal.
	 * we'll just do it later...
	 */
	register int last = pSmt->q.last;
	register smtQEntryPtr pQ = &(pSmt->q.e[last]);
	if (pQ->ptr == 0) {
	    if (ipFreeData(&pSmt->chan, pQ->len) != IP_SUCCESS)
		FatalError("SmtDeferRRQFromClient(): smt bad free\n");
	    if (last == (SMT_Q_DEPTH-1))
		pSmt->q.last = 0;
	    else
		pSmt->q.last = last+1;
	}
    }
    smtLastReqDeferred = 0;
    lastReq = NULL;

    status = ipReceiveDataAvail(&pSmt->chan, &nwords, &request, &avail);
    if (status != IP_SUCCESS ) {
	/* ip error status codes are always negative */
	YieldControlAndReturnNull();
    }

    /*
     * We may assume that the shared memory transport will never
     * release a request until it is complete (there is nothing
     * to be gained by having the client do this).  Therefore,
     * it suffices to determine if there is enough room for
     * a minimun request.
     */
    if ((nwords<<2) >= sizeof(xReq)) {	/* assume full request available */
	register int pipe;
	int next = pSmt->q.next;
	int this = next;
	/*
	 * If there is a request available, but we don't have anywhere to
	 * register it, or we're not leaving room for the client to be able
	 * write, then yield so that we'll come back here again later.
	 * nb: we depend on SmtSelect() to poll the SMT connections before
	 *     sleeping!  And the dispatch loop to consider status == 0 to
	 *     mean EWOULDBLOCK.
	 */
	next++;
	if (next == SMT_Q_DEPTH)
	    next = 0;
	pipe = ((next < pSmt->q.last) ?
		(SMT_Q_DEPTH - pSmt->q.last + next) :
		(next - pSmt->q.last));
	if (pSmt->q.last == next || (avail < MAX_REQUEST_SIZE && pipe > 2)) {
	    SMT_DIAG( ErrorF("pipe %d a%d\n"\, pipe\, avail));
	    smtLastReqDeferred = 1;
	    YieldControlWithInput();
	}

	/*
	 * Whenever a request is found, its location and size are saved.
	 * The next time ReadRequest() is called, the space occupied
	 * by the request might be freed.
	 */
	lastSmt = pSmt;
	lastReq = request;
	lastReqLen = request->length;
	if (lastReqLen == 0) {
	    ErrorF("Zero-Length request for client %x\n", who);
	    /* XXX should return instead of proceeding? */
	    YieldControlAndReturnNull();
	}

	/*******************************************************************
	 * Enqueue this request to be freed later.  Note that we assume the
	 * dispatch loop is:
	 *	free last req
	 *	read request
	 *	adv read ptr
	 *	exec request
	 *	if yielding, free last request.
	 * Therefore, before reading another request, we check to see if we
	 * can GC previous request(s), and ditto before yielding.  We may
	 * not free up previous request(s) for some time, because we're
	 * processing other requests round-robin before we get to another
	 * request on the same SMT connection.
	 *******************************************************************/

	pSmt->q.e[this].ptr = 0;
	pSmt->q.e[this].len = lastReqLen;
	pSmt->q.this = this;

	/* XXX this needs to be folded into an existing call! XXX */
	if (ipAdvanceRdPtr(&pSmt->chan, lastReqLen) != IP_SUCCESS)
	    FatalError("SmtDeferRRQFromClient(): rd ptr corrupt!\n");

	SMT_DIAG(ErrorF("smtrd (r%x n%d)[i%d]\n"\, request\,lastReqLen\,this));

	pSmt->q.next = next;

	BITSET( ClientsWithInput, client );
	if ((++timesThisConnection == MAX_TIMES_PER) || (isItTimeToYield)) {
	    YieldControl();
	}
	who->requestBuffer = (pointer)request;
	who->req_len = request->length;
    	return(request->length << 2);
    } else {				/* client idle */
	YieldControlAndReturnNull();
    }
	
} /* end SmtDeferRRQFromClient() */


/******************************
 * SmtEnq():
 *	Register DDX request with current SMT request.  Assumes request
 *	read from SMT is processed right away (atomic read/exec).
 ******************************/
Bool
SmtEnq(procp, Q, ptr)			/* SMT_QUEUE */
    int (*procp)();
    unsigned char *Q;			/* not used yet: which Q? */
    unsigned long *ptr;
{
    smtControlBlockPtr pSmt;
    smtQEntryPtr qp;

    if (SmtDeferFree == FALSE)
	FatalError("SmtEnq(): SmtDeferFree not set !!!\n");
    /* done if we're not reading an SMT request... */
    if (lastReq == NULL) return FALSE;
    
    qp = &(lastSmt->q.e[lastSmt->q.this]);
    qp->freep = procp;
    qp->Q = Q;
    qp->ptr = ptr;
    qp->dat = *ptr;

    SMT_DIAG(
	ErrorF("smtenq (q%x d%x)[i%d]\n"\,
	       ptr\, dat\, pSmt->q.this);
	);

    return TRUE;
} /* end SmtEnq() */
