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
static char *rcsid = "@(#)$RCSfile: prot_priv.c,v $ $Revision: 1.1.9.4 $ (DEC) $Date: 1993/08/17 20:05:41 $";
#endif
#ifndef lint
static char sccsid[] = "@(#)prot_priv.c	1.3 90/11/09 NFSSRC4.1 Copyr 1990 Sun Micro";
#endif

	/*
	 * Copyright (c) 1988 by Sun Microsystems, Inc.
	 */

	/*
	 * consists of all private protocols for comm with
	 * status monitor to handle crash and recovery
	 */

#include <stdio.h>
#include <netdb.h>
#include <sys/file.h>
#include <sys/param.h>
#include <sys/socket.h>
#include <sys/fcntl.h>
#include "prot_lock.h"
#include "priv_prot.h"
#include <rpcsvc/sm_inter.h>

extern int debug;
extern int pid;
extern char hostname[MAXHOSTNAMELEN];
extern int local_state;
extern struct msg_entry *retransmitted();
void proc_priv_crash(), proc_priv_recovery();
void reclaim_locks();
extern struct lm_vnode *find_me();
extern msg_entry *msg_q;	/* head of msg queue */
extern int callback_ip;
void reclaim_pending();

int cookie;

void
priv_prog(rqstp, transp)
	struct svc_req *rqstp;
	SVCXPRT *transp;
{
	char *(*Local)();
	struct status stat;
	extern bool_t xdr_status();

	if (debug)
		printf("Enter PRIV_PROG ...............\n");

	switch (rqstp->rq_proc) {
	case PRIV_CRASH:
		Local = (char *(*)()) proc_priv_crash;
		break;
	case PRIV_RECOVERY:
		Local = (char *(*)()) proc_priv_recovery;
		break;
	default:
		svcerr_noproc(transp);
		return;
	}

	bzero(&stat, sizeof (struct status));
	if (!svc_getargs(transp, xdr_status, &stat)) {
		svcerr_decode(transp);
		return;
	}
	(*Local)(&stat);
	if (!svc_sendreply(transp, xdr_void, NULL)) {
		svcerr_systemerr(transp);
	}
	if (!svc_freeargs(transp, xdr_status, &stat)) {
		fprintf(stderr, "unable to free arguments\n");
		exit(1);
	}
}

void
proc_priv_crash(statp)
	struct status *statp;
{
	struct hostent *hp;
	struct eflock ld;
	int cmd, fd, err;
	struct reclock grant_lock;

	if (debug)
		printf("enter proc_priv_CRASH....\n");

	if ((hp = gethostbyname(statp->mon_name)) == NULL) {
                if (debug)
                        printf( "RPC_UNKNOWNHOST\n");
		return;
        }
	ld.l_rsys = *(u_int *)hp->h_addr;

	fd = open("/tmp/lm_crash_foo", O_CREAT|O_RDWR);

	cmd = F_RSETLK;
	ld.l_type = F_UNLKSYS;
	ld.l_whence = 0;
       	ld.l_start = 0;
       	ld.l_len = 0;
       	ld.l_pid = getpid();
       	ld.l_rpid = getpid();
	ld.l_cb = callback_ip;
       	if (debug) {
               	printf("ld.l_start=%d ld.l_len=%d ld.l_rpid=%d ld.l_rsys=%x ld.l_cb=%x\n",
                       	ld.l_start, ld.l_len, ld.l_rpid, ld.l_rsys, ld.l_cb);
       	}
       	if ((err = fcntl(fd, cmd, &ld)) == -1) {
               	perror("fcntl");
               	printf("rpc.lockd: unable to clear a lock. \n");
       	}

	ld.l_type = F_UNLCK;
	while (ld.l_xxx == GRANT_LOCK_FLAG &&
	       fcntl(fd, F_RSETLKW, &ld) != -1) {
		grant_lock.block = TRUE;
		grant_lock.exclusive = TRUE;
		grant_lock.alock.lox.base = ld.l_start;
		grant_lock.alock.lox.length = ld.l_len;
		grant_lock.alock.lox.pid = ld.l_pid;
		grant_lock.alock.lox.rpid = ld.l_rpid;
		grant_lock.alock.lox.rsys = ld.l_rsys;
		grant_lock.alock.lox.type = ld.l_type;
		grant_lock.rel = 0;

		/* send this granted lock to the respective remote */
		/* lock mgr                                        */
		remote_grant(&grant_lock, MSG);
	}

	close(fd);
	delete_hash(statp->mon_name);
	/*
	 * In case /tmp/lm_crash_foo never get removed.
	 */
	unlink("/tmp/lm_crash_foo");
	return;
}

void
proc_priv_recovery(statp)
	struct status *statp;
{
	struct lm_vnode *mp;
	struct priv_struct *privp;
	char *xmalloc();

	if (debug)
		printf("enter proc_priv_RECOVERY.....\n");
	privp = (struct priv_struct *) statp->priv;
	if (privp->pid != pid) {
		if (debug)
			printf("this is not for me(%d): %d\n", privp->pid, pid);
		return;
	}

	if (debug)
		printf("enter proc_lm_recovery due to %s state(%d)\n",
			statp->mon_name, statp->state);

	destroy_client_shares(statp->mon_name);

	delete_hash(statp->mon_name);
	if (!up(statp->state)) {
		if (debug)
			printf("%s is not up.\n", statp->mon_name);
		return;
	}
	if (strcmp(statp->mon_name, hostname) == 0) {
		if (debug)
			printf("I have been declared as failed!!!\n");
		/*
		 * update local status monitor number
		 */
		local_state = statp->state;
	}

	mp = find_me(statp->mon_name);
	reclaim_locks(mp->exclusive);
	reclaim_locks(mp->shared);
	reclaim_pending(&mp->pending);
}

/*
 * reclaim_locks() -- will send out reclaim lock requests to the server.
 *		      listp is the list of established/granted lock requests.
 */
void
reclaim_locks(listp)
	struct reclock *listp;
{
	struct reclock *ff;

	for (ff = listp; ff; ff = ff->next) {
		/* set reclaim flag & send out the request */
		ff->reclaim = 1;
		if (nlm_call(NLM_LOCK_RECLAIM, ff, NULL) == -1) {
			if (queue(ff, NLM_LOCK_RECLAIM) == NULL)
				fprintf(stderr,
"reclaim request (%x) cannot be sent and cannot be queued for resend later!\n", ff);
		}
		if (ff->next == listp)	return;
	}
}

/*
 * reclaim_pending() -- will setup the existing queued msgs of the pending
 *		      	lock requests to allow retransmission.
 *			note that reclaim requests for these pending locks
 *			are not sent out.  The pending list passed in is purged
 *			because once these are retransmitted, the response
 *			message will put the reclock on a new queue.  [EJWXXX:
 *			can a reclock be on pending and not in msg_q?]
 */
void
reclaim_pending(listp)
	struct reclock **listp;
{
	msg_entry *msgp;
	struct reclock *ff, *next;

	/* for each pending lock request for the recovered server */
	for (ff = *listp; ff; ff = next) {
		next = ff->next;

		/* find the msg in the queue that holds this lock request */
		for (msgp = msg_q; msgp != NULL; msgp = msgp->nxt) {
			
			/* if msg is found, free & nullify the exisiting   */
			/* response of this lock request.  this will allow */
			/* retransmission of the requests.		   */
			if (msgp->req == ff) {
				if (msgp->reply != NULL) {
					release_res(msgp->reply);
					msgp->reply = NULL;
					del_req_from_list(&ff->lck.lox, listp);
				}
				break;
			}
		}
		if (!*listp || next == *listp)	return;
	}
	return;
}

