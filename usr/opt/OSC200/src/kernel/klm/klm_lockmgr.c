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
static char *rcsid = "@(#)$RCSfile: klm_lockmgr.c,v $ $Revision: 1.1.9.2 $ (DEC) $Date: 1993/05/28 19:36:42 $";
#endif
#ifndef lint
static char sccsid[] = 	"@(#)klm_lockmgr.c	2.3 90/10/22 4.1NFSSRC Copyr 1988 Sun Micro";
#endif
/* 
 * Copyright (c) 1988 by Sun Microsystems, Inc.
 * From SUN 1.29
 */

/*
 * Kernel<->Network Lock-Manager Interface
 *
 * File- and Record-locking requests are forwarded (via RPC) to a
 * Network Lock-Manager running on the local machine.  The protocol
 * for these transactions is defined in /usr/src/protocols/klm_prot.x
 */

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/user.h>
#include <sys/kernel.h>
#include <sys/socket.h>
#include <sys/socketvar.h>
#include <sys/mount.h>
#include <sys/vnode.h>
#include <sys/proc.h>
#include <sys/file.h>
#include <sys/stat.h>

#include <rpc/rpc.h>
#include <klm/lockmgr.h>
#include <klm/klm_prot.h>

#define	ESIG	100

static struct sockaddr_in pm_sa;	/* talk to portmapper */
static struct sockaddr_in lm_sa;	/* talk to lock manager */

static talk_to_lockmgr();

extern int wakeup();

#define BACKOFF_MOD     5
int lock_debug_on=0;

/* Define static parameters for run-time tuning */
static int first_retry = 0;		/* first attempt if klm port# known */
static int first_timeout = 1;
static int normal_retry = 1;		/* attempts after new port# obtained */
static int normal_timeout = 5;
static int working_retry = 0;		/* attempts after klm_working */
static int working_timeout = 1;


/*
 * klm_lockctl - process a lock/unlock/test-lock request
 *
 * Calls (via RPC) the local lock manager to register the request.
 * Lock requests are cancelled if interrupted by signals.
 * Last argument permits calling a lock manager listening on an
 * IP address other than the loopback address.  If argument is non-zero
 * use the IP address in the eflock structure else use the loopback
 * address.
 */
klm_lockctl(lh, ld, cmd, cred, clid, whereto)
	register lockhandle_t *lh;
	register struct eflock *ld;
	int cmd;
	struct ucred *cred;
	int clid;
	int whereto;
{
	register int	error;
	char 		*args;
	klm_lockargs	klm_lockargs_args;
	klm_unlockargs  klm_unlockargs_args;
	klm_testargs    klm_testargs_args;
	klm_testrply	reply;
	u_int		xdrproc;
	xdrproc_t	xdrargs;
	xdrproc_t	xdrreply;
	int backoff_timeout = 0;  /* time to wait on klm_denied_nolocks */
	int stat;

	if (lock_debug_on)
		printf("klm_lockctl cmd %d...whereto=%d\n", cmd, whereto);

	/* initialize sockaddr_in used to talk to local processes */
	if (pm_sa.sin_port == 0) {
		pm_sa.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
		pm_sa.sin_family = AF_INET;
		lm_sa.sin_family = AF_INET;
	}

	/*
	 * If whereto set; point sockaddr_in at appropriate instance
	 * of lock manager else point at lock manager listening on
	 * loopback.
	 */
	if (whereto) {
		lm_sa.sin_addr.s_addr = ld->l_cb;
	} else {
		lm_sa.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
	}

	if (!ld->l_pid) ld->l_pid = clid;
	switch (cmd) {
	case F_SETLK:
	case F_SETLKW:
		if (ld->l_type != F_UNLCK) {
			if (cmd == F_SETLKW)
				klm_lockargs_args.block = TRUE;
			else
				klm_lockargs_args.block = FALSE;
			if (ld->l_type == F_WRLCK) {
				klm_lockargs_args.exclusive = TRUE;
			} else {
				klm_lockargs_args.exclusive = FALSE;
			}
			klm_lockargs_args.alock.fh.n_bytes = (char *)&lh->lh_id;
			klm_lockargs_args.alock.fh.n_len = sizeof (lh->lh_id);
			klm_lockargs_args.alock.server_name = lh->lh_servername;
			klm_lockargs_args.alock.pid = clid;
			klm_lockargs_args.alock.base = (int)ld->l_start;
			klm_lockargs_args.alock.length = (int)ld->l_len;
			klm_lockargs_args.alock.rsys = ld->l_rsys;
			klm_lockargs_args.alock.rpid = ld->l_rpid;
			args = (char *) &klm_lockargs_args;
			xdrproc = KLM_LOCK;
			xdrargs = (xdrproc_t)xdr_klm_lockargs;
			xdrreply = (xdrproc_t)xdr_klm_stat;
		} else {
			klm_unlockargs_args.alock.fh.n_bytes =
				(char *)&lh->lh_id;
			klm_unlockargs_args.alock.fh.n_len =
				sizeof (lh->lh_id);
			klm_unlockargs_args.alock.server_name =
				lh->lh_servername;
			klm_unlockargs_args.alock.pid = clid;
			klm_unlockargs_args.alock.base = (int)ld->l_start;
			klm_unlockargs_args.alock.length = (int)ld->l_len;
			klm_unlockargs_args.alock.rsys = ld->l_rsys;
			klm_unlockargs_args.alock.rpid = ld->l_rpid;
			args = (char *) &klm_unlockargs_args;
			xdrreply = (xdrproc_t)xdr_klm_stat;
			xdrproc = KLM_UNLOCK;
			xdrargs = (xdrproc_t)xdr_klm_unlockargs;
		}
		break;

	case F_GETLK:
		if (ld->l_type == F_WRLCK) {
			klm_testargs_args.exclusive = TRUE;
		} else {
			klm_testargs_args.exclusive = FALSE;
		}
		klm_testargs_args.alock.fh.n_bytes = (char *)&lh->lh_id;
		klm_testargs_args.alock.fh.n_len = sizeof (lh->lh_id);
		klm_testargs_args.alock.server_name = lh->lh_servername;
		klm_testargs_args.alock.pid = clid;
		klm_testargs_args.alock.base = (int)ld->l_start;
		klm_testargs_args.alock.length = (int)ld->l_len;
		klm_testargs_args.alock.rsys = ld->l_rsys;
		klm_testargs_args.alock.rpid = ld->l_rpid;
		args = (char *) &klm_testargs_args;
		xdrproc = KLM_TEST;
		xdrargs = (xdrproc_t)xdr_klm_testargs;
		xdrreply = (xdrproc_t)xdr_klm_testrply;
		break;
	case KLM_GRANTED:
		if (ld->l_type == F_WRLCK) {
			klm_lockargs_args.block = TRUE;
			klm_lockargs_args.exclusive = TRUE;
		} else {
			klm_lockargs_args.block = FALSE;
			klm_lockargs_args.exclusive = FALSE;
		}
		klm_lockargs_args.alock.server_name = lh->lh_servername;
		klm_lockargs_args.alock.fh.n_bytes = (char *)&lh->lh_id;
		klm_lockargs_args.alock.fh.n_len = sizeof (lh->lh_id);
		klm_lockargs_args.alock.base = (int)ld->l_start;
		klm_lockargs_args.alock.length = (int)ld->l_len;
		klm_lockargs_args.alock.type = ld->l_type;
		klm_lockargs_args.alock.granted = TRUE;
		klm_lockargs_args.alock.color = 0;
		klm_lockargs_args.alock.LockID = 0;
		klm_lockargs_args.alock.pid = ld->l_pid;
		klm_lockargs_args.alock.class = 0;
		klm_lockargs_args.alock.rsys = ld->l_rsys;
		klm_lockargs_args.alock.rpid = ld->l_rpid;
		args = (char *) &klm_lockargs_args;
		xdrproc = KLM_GRANTED;
		xdrargs = (xdrproc_t)xdr_klm_lockargs;
		xdrreply = (xdrproc_t)xdr_klm_stat;
		break;
	default:
		error = EINVAL;
		goto ereturn;
	}

requestloop:
	/* send the request out to the local lock-manager and wait for reply */
	error = talk_to_lockmgr(xdrproc, xdrargs, args, xdrreply, &reply, cred);
	stat = reply.stat;

	if (lock_debug_on)
		printf("klm_lockctl: talk_to_lockmgr error %d stat %d\n",
			error, stat);

	if ((error == RPC_FAILED) || (error == ENOLCK)) {
		error = ENOLCK;
		goto ereturn;	/* no way the request could have gotten out */
	}

	/*
	 * The only other possible return values are:
	 *   klm_granted  |  klm_denied  | klm_denied_nolocks |  EINTR
	 */
	switch (xdrproc) {
	case KLM_GRANTED:
		switch (stat) {
		case klm_granted:
			error = 0;
			goto ereturn;
		case klm_working:
			goto nolocks_wait;
		default:
			goto nolocks_wait;
		}
	case KLM_LOCK:
		switch (error) {
		case ESIG:
		case EINTR:
			if (lock_debug_on) {
				if (klm_lockargs_args.block)
					printf("BLOCK: GOTO CANCEL\n");
				else
					printf("NOT BLOCK: GOTO REQUESTLOOP\n");
			}
			if (klm_lockargs_args.block)
				goto cancel;	/* cancel blocking locks */
			else
				goto requestloop;	/* loop forever */
		}
		switch (stat) {
		case klm_working:
			goto nolocks_wait;
		case klm_granted:
			error = 0;              /* got the requested lock */
			goto ereturn;
		case klm_denied:
			if (klm_lockargs_args.block) {
				printf("klm_lockmgr: blocking lock denied?!\n");
				goto requestloop;	/* loop forever */
			}
			error = EACCES;		/* EAGAIN?? */
			goto ereturn;		
		case klm_denied_nolocks:
			error = ENOLCK;		/* no resources available?! */
			goto ereturn;
		case klm_deadlck:
			error = EDEADLK;	/* deadlock condition */
			goto ereturn;
		}

	case KLM_UNLOCK:
		switch (error) {
		case ESIG:
		case EINTR:
			goto requestloop;       /* now try again */
		}
		switch (stat) {
		case klm_granted:
			error = 0;
			goto ereturn;
		case klm_working:
			goto nolocks_wait;
		case klm_denied:
			printf("klm_lockmgr: unlock denied?!\n");
			error = EINVAL;
			goto ereturn;
		case klm_denied_nolocks:
			printf("klm_lockmgr: unlock denied no locks?!\n");
			error = ENOLCK;
			goto ereturn;
		default:
			goto nolocks_wait;	
		}

	case KLM_TEST:
		switch (error) {
		case ESIG:
		case EINTR:
			error = EINTR;
			goto ereturn;   /* loop forever */
		}
		switch (stat) {
		case klm_granted:
			ld->l_type = F_UNLCK;	/* mark lock available */
			error = 0;
			goto ereturn;
		case klm_working:
			goto nolocks_wait;
		case klm_denied:
			ld->l_type = (reply.klm_testrply_u.holder.exclusive) ?
			    F_WRLCK : F_RDLCK;
			ld->l_start = (off_t)reply.klm_testrply_u.holder.base;
			ld->l_len = (off_t)reply.klm_testrply_u.holder.length;
			ld->l_pid = reply.klm_testrply_u.holder.pid;
			ld->l_rsys = reply.klm_testrply_u.holder.rsys;
			ld->l_rpid = reply.klm_testrply_u.holder.rpid;
			error = 0;
			goto ereturn;
		case klm_denied_nolocks:
			error = ENOLCK;	/* no resources available?! */
			goto ereturn;
		default:
			goto nolocks_wait;
		}
	default:
		goto ereturn;
	}

/* NOTREACHED */
nolocks_wait:
	if (lock_debug_on)
		printf("klm_lockctl: %d blocking\n", clid);
	backoff_timeout = (backoff_timeout % BACKOFF_MOD) + 1;
	timeout(wakeup, (caddr_t)&lm_sa, (backoff_timeout * hz));
	error = tsleep((caddr_t)&lm_sa, (PZERO+1)|PCATCH, "lockctl", 0);
	untimeout(wakeup, (caddr_t)&lm_sa);
	if (!error)
		goto requestloop; /* now try again */

cancel:
	/*
	 * If we get here, a signal interrupted a rqst that must be cancelled.
	 * Change the procedure number to KLM_CANCEL and reissue the exact same
	 * request.  Use the results to decide what return value to give.
	 */
	xdrproc = KLM_CANCEL;
	error = talk_to_lockmgr(xdrproc, xdrargs, args, xdrreply, &reply, cred);
	switch (error) {
	case klm_granted:
		error = EINTR;		/* lock granted */
		goto ereturn;
	case klm_denied:
		/* may want to take a longjmp here */
		error = EINTR;
		goto ereturn;
	case klm_deadlck:
		error = EDEADLK;
		goto ereturn;
	case ESIG:
	case EINTR:
		error = EINTR;
		goto ereturn;

	case klm_denied_nolocks:
		error = ENOLCK;		/* no resources available?! */
		goto ereturn;
	case ENOLCK:
		printf("klm_lockctl: ENOLCK on KLM_CANCEL request\n");
		goto ereturn;
	default:
		error = EINTR;
		goto ereturn;
	}
/* NOTREACHED */
ereturn:
	return (error);
}


/*
 * Send the given request to the local lock-manager.
 * If timeout or error, go back to the portmapper to check the port number.
 * This routine loops forever until one of the following occurs:
 *	1) A legitimate (not 'klm_working') reply is returned (returns 'stat').
 *
 *	2) A signal occurs (returns EINTR).  In this case, at least one try
 *	   has been made to do the RPC; this protects against jamming the
 *	   CPU if a KLM_CANCEL request has yet to go out.
 *
 *	3) A drastic error occurs (e.g., the local lock-manager has never
 *	   been activated OR cannot create a client-handle) (returns ENOLCK).
 */
static
talk_to_lockmgr(xdrproc, xdrargs, args, xdrreply, reply, cred)
	u_int xdrproc;
	xdrproc_t xdrargs;
	char *args;
	xdrproc_t xdrreply;
	klm_testrply *reply;
	struct ucred *cred;
{
	register CLIENT *client;
	struct timeval tmo;
	register int error;
	int reply_stat;

	if (lock_debug_on)
		printf("talk_to_lockmgr...\n");

	/* set up a client handle to talk to the local lock manager */
	client = clntkudp_create(&lm_sa, (u_int)KLM_PROG, (u_int)KLM_VERS,
	    first_retry, cred);
	if (client == (CLIENT *) NULL) {
		return (ENOLCK);
	}
	tmo.tv_sec = first_timeout;
	tmo.tv_usec = 0;

	/*
	 * If cached port number, go right to CLNT_CALL().
	 * This works because timeouts go back to the portmapper to
	 * refresh the port number.
	 */
	if (pm_sa.sin_port != 0) {
		goto retryloop;		/* skip first portmapper query */
	}

	for (;;) {
remaploop:
		/*
		 * go get the port number from the portmapper...
		 * if return 1, signal was received before portmapper answered;
		 * if return -1, the lock-manager is not registered
		 * else, got a port number
		 */
		if (lock_debug_on)
			printf("Call to getport_loop ip=%x\n", pm_sa.sin_addr.s_addr);
		error = getport_loop(&pm_sa,
		    (u_int)KLM_PROG, (u_int)KLM_VERS, (u_int)KLM_PROTO);

		switch (error) {
		case 1:
			error = ESIG;		/* signal interrupted things */
			goto out;

		case -1:
			printf("fcntl: Local lock-manager not registered\n");
			error = ENOLCK;
			goto out;
		}

		/*
		 * If a signal occurred, pop back out to the higher
		 * level to decide what action to take.  If we just
		 * got a port number from the portmapper, the next
		 * call into this subroutine will jump to retryloop.
		 */
		if (ISSIG(u.u_procp)) {
			error = ESIG;
			goto out;
		}

		/* reset the lock-manager client handle */
		lm_sa.sin_port = pm_sa.sin_port;
		(void) clntkudp_init(client, &lm_sa, normal_retry, cred);
		tmo.tv_sec = normal_timeout;

retryloop:
		/* retry the request until completion, timeout, or error */
		for (;;) {
			if (lock_debug_on)
				printf("Call to lockmgr ip=%x\n", lm_sa.sin_addr.s_addr);
			error = (int) CLNT_CALL(client, xdrproc, xdrargs,
			    (caddr_t)args, xdrreply, (caddr_t)reply, tmo);
			reply_stat = ((klm_testrply *)reply)->stat;			
			switch (error) {
			case RPC_SUCCESS:
			case klm_denied:
				if (error ||
					(int)reply->stat == (int) klm_working) {
					if (ISSIG(u.u_procp)) {
						error = ESIG;
						goto out;
					}
					if (error) {
					    /* lockmgr is up..can wait longer */
					    (void)clntkudp_init(client, &lm_sa,
					          working_retry, cred);
					    tmo.tv_sec = working_timeout;
					    continue;	/* retry */
					}
				} 
				goto out;	/* got a legitimate answer */

			case RPC_CANTRECV:
			case RPC_TIMEDOUT:
				goto remaploop;	/* ask for port# again */

			case klm_denied_nolocks:
				goto out;

			case RPC_INTR:
				error = EINTR;
				goto out;
			default:
				printf("lock-manager: RPC error: (%d) %s\n",
					error,
					clnt_sperrno((enum clnt_stat) error));
				error = RPC_FAILED;
				goto out;

			} /* switch */

		} /* for */	/* loop until timeout, error, or completion */

	} /* for */		/* loop until signal or completion */

out:
	AUTH_DESTROY(client->cl_auth);	/* drop the authenticator */
	CLNT_DESTROY(client);		/* drop the client handle */
	return (error);
}
