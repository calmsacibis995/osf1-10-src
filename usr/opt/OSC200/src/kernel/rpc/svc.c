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
static char *rcsid = "@(#)$RCSfile: svc.c,v $ $Revision: 1.1.3.5 $ (DEC) $Date: 1992/12/09 19:34:21 $";
#endif
#ifndef lint
static char sccsid[] = 	"@(#)svc.c	1.5 90/07/19 4.1NFSSRC Copyr 1990 Sun Micro";
#endif

/* 
 * Copyright (c) 1988 by Sun Microsystems, Inc.
 * 1.44 88/02/08 
 */


/*
 * svc.c, Server-side remote procedure call interface.
 *
 * There are two sets of procedures here.  The xprt routines are
 * for handling transport handles.  The svc routines handle the
 * list of service routines.
 */

#include <rpc/rpc.h>
#include <sys/domain.h>
caddr_t rqcred_head;  /* head of cashed, free authentication parameters */

/* CJXXX */
#define RPCHOLDXPRT -9999

#define NULL_SVC ((struct svc_callout *)0)
#define	RQCRED_SIZE	400		/* this size is excessive */

#define	SVC_VERSQUIET 0x0001		/* keept quiet about vers mismatch */
#define	version_keepquiet(xp)  ((u_int)(xp)->xp_p3 & SVC_VERSQUIET)

/*
 * The services list
 * Each entry represents a set of procedures (an rpc program).
 * The dispatch routine takes request structs and runs the
 * apropriate procedure.
 */
static struct svc_callout {
	struct svc_callout *sc_next;
	u_int		    sc_prog;
	u_int		    sc_vers;
	int		    (*sc_dispatch)();
} *svc_head;

static struct svc_callout *svc_find();

/* ***************  SVCXPRT related stuff **************** */

/*
 * Activate a transport handle.
 */
/*ARGSUSED*/
void
xprt_register(xprt)
	SVCXPRT *xprt;
{
}

/* ********************** CALLOUT list related stuff ************* */

/*
 * Add a service program to the callout list.
 * The dispatch routine will be called when a rpc request for this
 * program number comes in.
 */
/*ARGSUSED*/ 
bool_t
svc_register(xprt, prog, vers, dispatch, protocol)
	SVCXPRT *xprt;
	u_int prog;
	u_int vers;
	int (*dispatch)();
	int protocol;
{
	struct svc_callout *prev;
	register struct svc_callout *s;

	if ((s = svc_find(prog, vers, &prev)) != NULL_SVC) {
		if (s->sc_dispatch == dispatch)
			goto pmap_it;  /* he is registering another xptr */
		return (FALSE);
	}
	s = (struct svc_callout *)mem_alloc(sizeof(struct svc_callout));
	s->sc_prog = prog;
	s->sc_vers = vers;
	s->sc_dispatch = dispatch;
	s->sc_next = svc_head;
	svc_head = s;
#ifdef	KTRACE
	kern_trace(1,prog,vers,dispatch);
#endif  /* KTRACE */
pmap_it:
	return (TRUE);
}

/*
 * Remove a service program from the callout list.
 */
void
svc_unregister(prog, vers)
	u_int prog;
	u_int vers;
{
	struct svc_callout *prev;
	register struct svc_callout *s;

	if ((s = svc_find(prog, vers, &prev)) == NULL_SVC)
		return;
	if (prev == NULL_SVC) {
		svc_head = s->sc_next;
	} else {
		prev->sc_next = s->sc_next;
	}
	s->sc_next = NULL_SVC;
	mem_free((char *) s, (u_int) sizeof(struct svc_callout));
#ifdef	KTRACE
	kern_trace(2,proc,vers,0);
#endif  /* KTRACE */
}

/*
 * Search the callout list for a program number, return the callout
 * struct.
 */
static struct svc_callout *
svc_find(prog, vers, prev)
	u_int prog;
	u_int vers;
	struct svc_callout **prev;
{
	register struct svc_callout *s, *p;

	p = NULL_SVC;
	for (s = svc_head; s != NULL_SVC; s = s->sc_next) {
		if ((s->sc_prog == prog) && (s->sc_vers == vers))
			goto done;
		p = s;
	}
done:
	*prev = p;
	return (s);
}

/* ******************* REPLY GENERATION ROUTINES  ************ */

/*
 * Send a reply to an rpc request
 */
bool_t
svc_sendreply(xprt, xdr_results, xdr_location)
	register SVCXPRT *xprt;
	xdrproc_t xdr_results;
	caddr_t xdr_location;
{
	struct rpc_msg rply; 

	rply.rm_direction = REPLY;  
	rply.rm_reply.rp_stat = MSG_ACCEPTED; 
	rply.acpted_rply.ar_verf = xprt->xp_verf; 
	rply.acpted_rply.ar_stat = SUCCESS;
	rply.acpted_rply.ar_results.where = xdr_location;
	rply.acpted_rply.ar_results.proc = xdr_results;
#ifdef	KTRACE
	kern_trace(3, 0, 0, 0);
#endif  /* KTRACE */
	return (SVC_REPLY(xprt, &rply)); 
}

/*
 * No procedure error reply
 */
void
svcerr_noproc(xprt)
	register SVCXPRT *xprt;
{
	struct rpc_msg rply;

	rply.rm_direction = REPLY;
	rply.rm_reply.rp_stat = MSG_ACCEPTED;
	rply.acpted_rply.ar_verf = xprt->xp_verf;
	rply.acpted_rply.ar_stat = PROC_UNAVAIL;
	SVC_REPLY(xprt, &rply);
}

/*
 * Can't decode args error reply
 */
void
svcerr_decode(xprt)
	register SVCXPRT *xprt;
{
	struct rpc_msg rply; 

	rply.rm_direction = REPLY; 
	rply.rm_reply.rp_stat = MSG_ACCEPTED; 
	rply.acpted_rply.ar_verf = xprt->xp_verf;
	rply.acpted_rply.ar_stat = GARBAGE_ARGS;
	SVC_REPLY(xprt, &rply); 
}

/*
 * Authentication error reply
 */
void
svcerr_auth(xprt, why)
	SVCXPRT *xprt;
	enum auth_stat why;
{
	struct rpc_msg rply;

	rply.rm_direction = REPLY;
	rply.rm_reply.rp_stat = MSG_DENIED;
	rply.rjcted_rply.rj_stat = AUTH_ERROR;
	rply.rjcted_rply.rj_why = why;
	SVC_REPLY(xprt, &rply);
}

/*
 * Auth too weak error reply
 */
void
svcerr_weakauth(xprt)
	SVCXPRT *xprt;
{

	svcerr_auth(xprt, AUTH_TOOWEAK);
}

/*
 * Program unavailable error reply
 */
void 
svcerr_noprog(xprt)
	register SVCXPRT *xprt;
{
	struct rpc_msg rply;  

	rply.rm_direction = REPLY;   
	rply.rm_reply.rp_stat = MSG_ACCEPTED;  
	rply.acpted_rply.ar_verf = xprt->xp_verf;  
	rply.acpted_rply.ar_stat = PROG_UNAVAIL;
	SVC_REPLY(xprt, &rply);
}

/*
 * Program version mismatch error reply
 */
void  
svcerr_progvers(xprt, low_vers, high_vers)
	register SVCXPRT *xprt; 
	u_int low_vers;
	u_int high_vers;
{
	struct rpc_msg rply;

	rply.rm_direction = REPLY;
	rply.rm_reply.rp_stat = MSG_ACCEPTED;
	rply.acpted_rply.ar_verf = xprt->xp_verf;
	rply.acpted_rply.ar_stat = PROG_MISMATCH;
	rply.acpted_rply.ar_vers.low = low_vers;
	rply.acpted_rply.ar_vers.high = high_vers;
	SVC_REPLY(xprt, &rply);
}

/* ******************* SERVER INPUT STUFF ******************* */

/*
 * Get server side input from some transport.
 *
 * Statement of authentication parameters management:
 * This function owns and manages all authentication parameters, specifically
 * the "raw" parameters (msg.rm_call.cb_cred and msg.rm_call.cb_verf) and
 * the "cooked" credentials (rqst->rq_clntcred).
 * However, this function does not know the structure of the cooked 
 * credentials, so it make the following assumptions: 
 *   a) the structure is contiguous (no pointers), and
 *   b) the cred structure size does not exceed RQCRED_SIZE bytes. 
 * In all events, all three parameters are freed upon exit from this routine.
 * The storage is trivially management on the call stack in user land, but
 * is mallocated in kernel land.
 */

int
svc_getreq(xprt)
	SVCXPRT *xprt;
{
	enum xprt_stat stat;
	struct rpc_msg msg;
	int prog_found;
	u_int low_vers;
	u_int high_vers;
	struct svc_req r;
	int (*disp)();	  /* temp storage of function address */
	char *cred_area;  /* too big to allocate on call stack */
	int ret = 0;

	/*
	 * Firstly, allocate the authentication parameters' storage
	 */
	/* SMP lock head of list of auth storage while accessing it */
	/* smp_lock(&lk_rpcrqcred, LK_RETRY); */
	if (rqcred_head) {
		cred_area = rqcred_head;
		rqcred_head = *(caddr_t *)rqcred_head;
		/* smp_unlock(&lk_rpcrqcred); */
	} else {
		/* smp_unlock(&lk_rpcrqcred); */
		cred_area = mem_alloc(2*MAX_AUTH_BYTES + RQCRED_SIZE);
	}

	msg.rm_call.cb_cred.oa_base = cred_area;
	msg.rm_call.cb_verf.oa_base = &(cred_area[MAX_AUTH_BYTES]);
	r.rq_clntcred = &(cred_area[2*MAX_AUTH_BYTES]);

#ifdef	KTRACE
	kern_trace(4, 0,0,0);
#endif  /* KTRACE */

	if (SVC_RECV(xprt, &msg)) {

		/* now find the exported program and call it */
		register struct svc_callout *s;
		enum auth_stat why;
		
		sbunlock(&xprt->xp_sock->so_rcv);
		SOCKET_UNLOCK(xprt->xp_sock);

		r.rq_xprt = xprt;
		r.rq_prog = msg.rm_call.cb_prog;
		r.rq_vers = msg.rm_call.cb_vers;
		r.rq_proc = msg.rm_call.cb_proc;
		r.rq_cred = msg.rm_call.cb_cred;
		r.rq_xid = msg.rm_xid;

#ifdef	KTRACE
		kern_trace(5, msg.rm_xid, r.rq_proc ,0);
#endif  /* KTRACE */
		/* first authenticate the message */
		if ((why= _authenticate(&r, &msg)) != AUTH_OK) {
			svcerr_auth(xprt, why);
			/*
			 * Free the arguments.
			 */
			(void) SVC_FREEARGS(xprt, (xdrproc_t)0,
					    (caddr_t)0);
			goto call_done;
		}
		/* now match message with a registered service*/
		prog_found = FALSE;
		low_vers = 0 - 1;
		high_vers = 0;
		/* SMP lock service list during search and reference */
		/* smp_lock(&lk_rpccallout, LK_RETRY); */
		for (s = svc_head; s != NULL_SVC; s = s->sc_next) {
			if (s->sc_prog == r.rq_prog) {
				if (s->sc_vers == r.rq_vers) {
					disp = s->sc_dispatch;
					/* smp_unlock(&lk_rpccallout); */

					/* Where it all happens */
					ret = (*disp)(&r, xprt);
#ifdef	KTRACE
					kern_trace(6, msg.rm_xid, r.rq_proc ,0);
#endif  /* KTRACE */

					goto call_done;
				}  /* found correct version */
				prog_found = TRUE;
				if (s->sc_vers < low_vers)
					low_vers = s->sc_vers;
				if (s->sc_vers > high_vers)
					high_vers = s->sc_vers;
			}   /* found correct program */
		}
		/* smp_unlock(&lk_rpccallout); */
		/*
		 * if we got here, the program or version
		 * is not served ...
		 */
		if (prog_found && !version_keepquiet(xprt))
			svcerr_progvers(xprt,
					low_vers, high_vers);
		else
			svcerr_noprog(xprt);
		/*
		 * Free the argument storage.
		 * This is done in the dispatch routine
		 * for successful calls.
		 */
		(void) SVC_FREEARGS(xprt,(xdrproc_t)0,
				    (caddr_t)0);
	} else 
		sbunlock(&xprt->xp_sock->so_rcv);

      call_done:
	/*
	 * free authentication parameters' storage
	 */
	/* SMP lock head of list of auth storage while accessing it */
	/* smp_lock(&lk_rpcrqcred, LK_RETRY); */
	*(caddr_t *)cred_area = rqcred_head;
	rqcred_head = cred_area;
	/* smp_unlock(&lk_rpcrqcred); */
#ifdef	KTRACE
	kern_trace(7, msg.rm_xid, r.rq_proc ,0);
#endif  /* KTRACE */

	if (ret && ret != RPCHOLDXPRT)
		printf("NFS action routine returns unexpected %d\n",
		       ret);
	return (ret);
}


/*
 * This is the rpc server side idle loop
 * Wait for input, call server program.
 */
int svc_run_debug=0;
int Rpccnt;
int
svc_run(xprt)
	SVCXPRT *xprt;
{
	int	s, error;
	int 	ret;

	if (svc_run_debug)
		printf("svc_run: starting\n");
#ifdef	KTRACE
	kern_trace(8, 0, 0, 0);
#endif  /* KTRACE */

	SOCKET_LOCK(xprt->xp_sock); 
	while (TRUE) {
		if (svc_run_debug)
			printf("svc_run: top of the while loop\n");

		if (error = sosblock(&xprt->xp_sock->so_rcv, xprt->xp_sock)) {
			printf("svc_run: sosblock returns error %d\n", error);
			goto out;
		}
		if (svc_run_debug)
			printf("svc_run: sosblock returns: %d\n",error);

		if (xprt->xp_sock->so_rcv.sb_cc == 0) {
			if (error == EPIPE || error == EINTR ||
			    error == ERESTART)  
				break;

			xprt->xp_sock->so_error = 0;
			if (svc_run_debug)
				printf("svc_run: empty socket\n");
		
			if (error = sosbwait(&xprt->xp_sock->so_rcv, 
					     xprt->xp_sock)) {
				error = EINTR;  /* don't depend on anyone */
				goto out;
			}

			continue;
		}
		if (svc_run_debug)
			printf("svc_run: calling getreq\n");
		ret = svc_getreq(xprt);
#ifdef	KTRACE
		kern_trace(8, 0, 0, 0);
#endif  /* KTRACE */

		Rpccnt++;
		if (ret) {
			if (ret != RPCHOLDXPRT)
				printf("svc_run: svc_getreq returns unexpected %d\n",
				       ret);
			return (ret);
		}
		if (svc_run_debug)
			printf("svc_run: returning from getreq\n");
		SOCKET_LOCK(xprt->xp_sock); 
	}
	sbunlock(&xprt->xp_sock->so_rcv);
out:
	if (svc_run_debug)
		printf("svc_run returns: %d\n", error);

	SOCKET_UNLOCK(xprt->xp_sock);
	return (error);
}

