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
static char *rcsid = "@(#)$RCSfile: prot_main.c,v $ $Revision: 1.1.5.5 $ (DEC) $Date: 1993/12/21 20:49:29 $";
#endif
#ifndef lint
static char sccsid[] = "@(#)prot_main.c	1.5 90/12/06 NFSSRC4.1 Copyr 1990 Sun Micro";
#endif

	/*
	 * Copyright (c) 1988 by Sun Microsystems, Inc.
	 */

#include <stdio.h>
#include <netdb.h>
#include <signal.h>
#include <sys/file.h>
#include <sys/param.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include "prot_time.h"
#include "prot_lock.h"
#include "priv_prot.h"
#include <rpc/pmap_prot.h>

#define DEBUG_ON	57
#define DEBUG_OFF	58
#define KILL_IT		59

int HASH_SIZE;
int debug;
int klm, nlm;
int report_sharing_conflicts;
XDR x;
FILE *fp;
SVCXPRT *klm_transp;			/* export klm transport handle */
SVCXPRT *nlm_transp;			/* export nlm transport handle */
int callback_ip;			/* ip address for kernel to callback*/
char *bindhostname = NULL;

extern int grace_period, LockID;
extern msg_entry *klm_msg, *msg_q;
extern int lock_len, res_len;
extern msg_entry *queue();
extern remote_result *get_res();
extern reclock *get_reclock();

extern void release_klm_req();
extern void release_nlm_req();
extern void release_nlm_rep();

extern int xtimer();
extern void priv_prog();

extern remote_result res_nolock;
extern remote_result res_working;
extern remote_result res_grace;

struct svcreg_entry {
	u_int prognum;
	u_int versnum;
	ushort portnum;
	void (*dispatch)();
};

struct bindhostname_entry {
	struct bindhostname_entry *next;
	struct sockaddr_in addr;
};

struct bindhostname_entry *bindhostname_q = NULL;
char STATE[MAXHOSTNAMELEN], CURRENT[MAXHOSTNAMELEN], BACKUP[MAXHOSTNAMELEN];
int debug;

static void
get_bindhostname(name)
	char *name;
{
	int i,sd;
	struct bindhostname_entry *entry;
	struct bindhostname_entry *entry_tmp;
	struct hostent *hp;
	struct ifconf ifc;
	struct ifreq ifreq,*ifr;
	int duplicate_flag;

	while (bindhostname_q != NULL) {
		entry = bindhostname_q;
		bindhostname_q = bindhostname_q->next;
		free(entry);
	}
		
	if (name != NULL) {
		if ((hp=gethostbyname(name)) == NULL) {
			fprintf(stderr,"rpc.lockd: %s is an unknown hostname\n",name);
			exit(1);
		}
		bindhostname_q = (struct bindhostname_entry *)
			malloc(sizeof(struct bindhostname_entry));
		if (bindhostname_q == NULL) {
			fprintf(stderr,"rpc.lockd: get_bindhostname() malloc error\n");
			exit(1);
		}
		bzero((char *)&bindhostname_q->addr, sizeof(bindhostname_q->addr));
		bcopy(hp->h_addr, (char *)&bindhostname_q->addr.sin_addr.s_addr, hp->h_length);
		bindhostname_q->addr.sin_family = hp->h_addrtype;
		bindhostname_q->next = NULL;
		return;
	}


	ifc.ifc_len = UDPMSGSIZE;
	ifc.ifc_buf = (char *) malloc(ifc.ifc_len);
	
	if ((sd = socket(PF_INET, SOCK_DGRAM, 0)) < 0) {
		perror("rpc.lockd: get_bindhostname() socket error\n");
		exit(1);
	}

	if (ioctl(sd, SIOCGIFCONF, (char *) &ifc) < 0) {
		perror("rpc.lockd: getbindhost() ioctl SIOCGIFCONF");
		exit(1);
	}

	ifr = ifc.ifc_req;
	for (i=0; i<(ifc.ifc_len / sizeof (struct ifreq)); i++,ifr++) {
		ifreq = *ifr;
		if (ioctl(sd, SIOCGIFFLAGS, (char *) &ifreq) < 0) {
			perror("rpc.lockd: getbindhost() ioctl SIOCGIFFLAGS");
			exit(1);
		}
		if ((ifreq.ifr_flags & IFF_UP) && ifr->ifr_addr.sa_family == AF_INET) {
			if (ioctl(sd, SIOCGIFADDR, (char *) &ifreq) < 0) {
				perror("rpc.lockd: getbindhost() ioctl SIOCGIFADDR");
				exit(1);
			} else {
				if (bindhostname_q != NULL) {
					duplicate_flag = 0;
					for (entry_tmp = bindhostname_q; entry_tmp != NULL; entry_tmp=entry_tmp->next) {
						if (bcmp(&ifreq.ifr_addr,&entry_tmp->addr,sizeof(struct sockaddr_in)) == 0) {
							duplicate_flag = 1;
							break;
						}
					}
					if (duplicate_flag) {
						continue;
					}
					entry->next = (struct bindhostname_entry *)
						malloc(sizeof(struct bindhostname_entry));
					if (entry->next == NULL) {
						fprintf(stderr,"rpc.lockd: get_bindhostname() malloc error\n");
						exit(1);
					}
					entry = entry->next;
				} else {
					bindhostname_q = (struct bindhostname_entry *)
						malloc(sizeof(struct bindhostname_entry));
					if (bindhostname_q == NULL) {
						fprintf(stderr,"rpc.lockd: get_bindhostname() malloc error\n");
						exit(1);
					}
					entry = bindhostname_q;
				}
				bcopy(&ifreq.ifr_addr,&entry->addr,sizeof(struct sockaddr_in));
				entry->next = NULL;
			}
		}
	}
	(void) close(sd);
	free(ifc.ifc_buf);
}

static void
nlm_prog(Rqstp, Transp)
	struct svc_req *Rqstp;
	SVCXPRT *Transp;
{
	bool_t 			(*xdr_Argument)(), (*xdr_Result)();
	char 			*(*Local)();
	extern nlm_testres 	*proc_nlm_test();
	extern nlm_res 		*proc_nlm_lock();
	extern nlm_res 		*proc_nlm_cancel();
	extern nlm_res 		*proc_nlm_unlock();
	extern nlm_res          *proc_nlm_granted();
	extern void 		*proc_nlm_test_msg();
	extern void 		*proc_nlm_lock_msg();
	extern void 		*proc_nlm_cancel_msg();
	extern void 		*proc_nlm_unlock_msg();
	extern void             *proc_nlm_granted_msg();
	extern void 		*proc_nlm_test_res();
	extern void 		*proc_nlm_lock_res();
	extern void 		*proc_nlm_cancel_res();
	extern void 		*proc_nlm_unlock_res();
	extern void             *proc_nlm_granted_res();
	extern int 		nlm_lockargstoreclock();
	extern int 		nlm_unlockargstoreclock();
	extern int 		nlm_testargstoreclock();
	extern int 		nlm_cancargstoreclock();
	extern int		nlm_testrestoremote_result();
	extern int		nlm_restoremote_result();
	extern nlm_lockargs	*get_nlm_lockargs();
	extern nlm_unlockargs	*get_nlm_unlockargs();
	extern nlm_testargs	*get_nlm_testargs();
	extern nlm_cancargs	*get_nlm_cancargs();
	extern nlm_testres	*get_nlm_testres();
	extern nlm_res		*get_nlm_res();
	extern void 		*proc_nlm_share();
	extern void 		*proc_nlm_freeall();
	int			monitor_this_lock = 1;
	char 			*nlm_req, *nlm_rep;
	nlm_lockargs 		*nlm_lockargs_a;
	nlm_unlockargs 		*nlm_unlockargs_a;
	nlm_testargs 		*nlm_testargs_a;
	nlm_cancargs 		*nlm_cancargs_a;
	nlm_testres		*nlm_testres_a;
	nlm_res			*nlm_res_a;
	struct reclock  	*req;
	remote_result 		*reply;
	int 			oldmask;

	if (debug) {
		printf("NLM_PROG+++ version %d proc %d\n",
			Rqstp->rq_vers, Rqstp->rq_proc);
		pr_all();
	}


	oldmask = sigblock (1 << (SIGALRM -1));
	nlm_transp = Transp;		/* export the transport handle */
	switch (Rqstp->rq_proc) {
	case NULLPROC:
		svc_sendreply(Transp, xdr_void, NULL);
		(void) sigsetmask(oldmask);
		return;

	case DEBUG_ON:
		debug = 2;
		svc_sendreply(Transp, xdr_void, NULL);
		(void) sigsetmask(oldmask);
		return;

	case DEBUG_OFF:
		debug = 0;
		svc_sendreply(Transp, xdr_void, NULL);
		(void) sigsetmask(oldmask);
		return;

	case NLM_TEST:
		xdr_Argument = xdr_nlm_testargs;
		xdr_Result = xdr_nlm_testres;
		Local = (char *(*)()) proc_nlm_test;
		if ((nlm_testargs_a = get_nlm_testargs()) == NULL) {
			printf("rpc.lockd: unable to allocate space for nlm_req.\n");
			(void) sigsetmask(oldmask);
			return;
		}
		nlm_req = (char *) nlm_testargs_a;
		break;

	case NLM_LOCK:
		xdr_Argument = xdr_nlm_lockargs;
		xdr_Result = xdr_nlm_res;
		Local = (char *(*)()) proc_nlm_lock;
		if ((nlm_lockargs_a = get_nlm_lockargs()) == NULL) {
			printf("rpc.lockd: unable to allocate space for nlm_req.\n");
			(void) sigsetmask(oldmask);
			return;
		}
		nlm_req = (char *) nlm_lockargs_a;
		break;

	case NLM_GRANTED:
                xdr_Argument = xdr_nlm_testargs;
                xdr_Result = xdr_nlm_res;
                Local = (char *(*)()) proc_nlm_granted;
                if ((nlm_testargs_a = get_nlm_testargs()) == NULL) {
                        printf("rpc.lockd: unable to allocate space for nlm_req.\n");
                        (void) sigsetmask(oldmask);
                        return;
                }
                nlm_req = (char *) nlm_testargs_a;
                break;

	case NLM_CANCEL:
		xdr_Argument = xdr_nlm_cancargs;
		xdr_Result = xdr_nlm_res;
		Local = (char *(*)()) proc_nlm_cancel;
		if ((nlm_cancargs_a = get_nlm_cancargs()) == NULL) {
			printf("rpc.lockd: unable to allocate space for nlm_req.\n");
			(void) sigsetmask(oldmask);
			return;
		}
		nlm_req = (char *) nlm_cancargs_a;
		break;

	case NLM_UNLOCK:
		xdr_Argument = xdr_nlm_unlockargs;
		xdr_Result = xdr_nlm_res;
		Local = (char *(*)()) proc_nlm_unlock;
		if ((nlm_unlockargs_a = get_nlm_unlockargs()) == NULL) {
			printf("rpc.lockd: unable to allocate space for nlm_req.\n");
			(void) sigsetmask(oldmask);
			return;
		}
		nlm_req = (char *) nlm_unlockargs_a;
		break;

	case NLM_TEST_MSG:
		xdr_Argument = xdr_nlm_testargs;
		xdr_Result = xdr_void;
		Local = (char *(*)()) proc_nlm_test_msg;
		if ((nlm_testargs_a = get_nlm_testargs()) == NULL) {
			printf("rpc.lockd: unable to allocate space for nlm_req.\n");
			(void) sigsetmask(oldmask);
			return;
		}
		nlm_req = (char *) nlm_testargs_a;
		break;

	case NLM_LOCK_RECLAIM:
	case NLM_LOCK_MSG:
		xdr_Argument = xdr_nlm_lockargs;
		xdr_Result = xdr_void;
		Local = (char *(*)()) proc_nlm_lock_msg;
		if ((nlm_lockargs_a = get_nlm_lockargs()) == NULL) {
			printf("rpc.lockd: unable to allocate space for nlm_req.\n");
			(void) sigsetmask(oldmask);
			return;
		}
		nlm_req = (char *) nlm_lockargs_a;
		break;

	case NLM_GRANTED_MSG:
                xdr_Argument = xdr_nlm_testargs;
                xdr_Result = xdr_void;
                Local = (char *(*)()) proc_nlm_granted_msg;
                if ((nlm_testargs_a = get_nlm_testargs()) == NULL) {
                        printf("rpc.lockd: unable to allocate space for nlm_req.\n");
                        (void) sigsetmask(oldmask);
                        return;
                }
                nlm_req = (char *) nlm_testargs_a;
                break;

	case NLM_CANCEL_MSG:
		xdr_Argument = xdr_nlm_cancargs;
		xdr_Result = xdr_void;
		Local = (char *(*)()) proc_nlm_cancel_msg;
		if ((nlm_cancargs_a = get_nlm_cancargs()) == NULL) {
			printf("rpc.lockd: unable to allocate space for nlm_req.\n");
			(void) sigsetmask(oldmask);
			return;
		}
		nlm_req = (char *) nlm_cancargs_a;
		break;

	case NLM_UNLOCK_MSG:
		xdr_Argument = xdr_nlm_unlockargs;
		xdr_Result = xdr_void;
		Local = (char *(*)()) proc_nlm_unlock_msg;
		if ((nlm_unlockargs_a = get_nlm_unlockargs()) == NULL) {
			printf("rpc.lockd: unable to allocate space for nlm_req.\n");
			(void) sigsetmask(oldmask);
			return;
		}
		nlm_req = (char *) nlm_unlockargs_a;
		break;

	case NLM_TEST_RES:
		xdr_Argument = xdr_nlm_testres;
		xdr_Result = xdr_void;
		Local = (char *(*)()) proc_nlm_test_res;
		if ((nlm_testres_a = get_nlm_testres()) == NULL) {
			printf("rpc.lockd: unable to allocate space for nlm_req.\n");
			(void) sigsetmask(oldmask);
			return;
		}
		nlm_rep = (char *) nlm_testres_a;
		break;

	case NLM_LOCK_RES:
		xdr_Argument = xdr_nlm_res;
		xdr_Result = xdr_void;
		Local = (char *(*)()) proc_nlm_lock_res;
		if ((nlm_res_a = get_nlm_res()) == NULL) {
			printf("rpc.lockd: unable to allocate space for nlm_req.\n");
			(void) sigsetmask(oldmask);
			return;
		}
		nlm_rep = (char *) nlm_res_a;
		break;

	case NLM_GRANTED_RES:
                xdr_Argument = xdr_nlm_res;
                xdr_Result = xdr_void;
                Local = (char *(*)()) proc_nlm_granted_res;
                if ((nlm_res_a = get_nlm_res()) == NULL) {
                        printf("rpc.lockd: unable to allocate space for nlm_req.\n");
                        (void) sigsetmask(oldmask);
                        return;
                }
                nlm_rep = (char *) nlm_res_a;
                break;

	case NLM_CANCEL_RES:
		xdr_Argument = xdr_nlm_res;
		xdr_Result = xdr_void;
		Local = (char *(*)()) proc_nlm_cancel_res;
		if ((nlm_res_a = get_nlm_res()) == NULL) {
			printf("rpc.lockd: unable to allocate space for nlm_req.\n");
			(void) sigsetmask(oldmask);
			return;
		}
		nlm_rep = (char *) nlm_res_a;
		break;

	case NLM_UNLOCK_RES:
		xdr_Argument = xdr_nlm_res;
		xdr_Result = xdr_void;
		Local = (char *(*)()) proc_nlm_unlock_res;
		if ((nlm_res_a = get_nlm_res()) == NULL) {
			printf("rpc.lockd: unable to allocate space for nlm_req.\n");
			(void) sigsetmask(oldmask);
			return;
		}
		nlm_rep = (char *) nlm_res_a;
		break;

	case NLM_SHARE:
	case NLM_UNSHARE:
		if (Rqstp->rq_vers != NLM_VERSX)  {
			svcerr_noproc(Transp);
			(void) sigsetmask(oldmask);
			return;
		}
		proc_nlm_share(Rqstp, Transp);
		(void) sigsetmask(oldmask);
		return;

	case NLM_NM_LOCK:
		if (Rqstp->rq_vers != NLM_VERSX)  {
			svcerr_noproc(Transp);
			(void) sigsetmask(oldmask);
			return;
		}
		monitor_this_lock = 0;
		xdr_Argument = xdr_nlm_lockargs;
		xdr_Result = xdr_void;
		Local = (char *(*)()) proc_nlm_lock;
		if ((nlm_lockargs_a = get_nlm_lockargs()) == NULL) {
			printf("rpc.lockd: unable to allocate space for nlm_req.\n");
			(void) sigsetmask(oldmask);
			return;
		}
		nlm_req = (char *) nlm_lockargs_a;
		break;

	case NLM_FREE_ALL:
		if (Rqstp->rq_vers != NLM_VERSX)  {
			svcerr_noproc(Transp);
			(void) sigsetmask(oldmask);
			return;
		}
		proc_nlm_freeall(Rqstp, Transp);
		(void) sigsetmask(oldmask);
		return;

	default:
		svcerr_noproc(Transp);
		(void) sigsetmask(oldmask);
		return;
	}

	if ( Rqstp->rq_proc != NLM_LOCK_RES &&
		Rqstp->rq_proc != NLM_CANCEL_RES &&
		Rqstp->rq_proc != NLM_UNLOCK_RES &&
		Rqstp->rq_proc != NLM_TEST_RES && 
		Rqstp->rq_proc != NLM_GRANTED_RES) {
		/* lock request */
		if ((req = get_reclock()) != NULL) {
			if (!svc_getargs(Transp, xdr_Argument, nlm_req)) {
				release_nlm_req(Rqstp->rq_proc, nlm_req);
				svcerr_decode(Transp);
				(void) sigsetmask(oldmask);
				return;
			}

			/*
			 * parameter conversion if NLM_VERSX is used
			 */
			if (Rqstp->rq_vers == NLM_VERSX)
				nlm_versx_conversion(Rqstp->rq_proc, nlm_req);

			switch (Rqstp->rq_proc) {
			case NLM_TEST:
			case NLM_GRANTED:
			case NLM_TEST_MSG:
			case NLM_GRANTED_MSG:
				if (nlm_testargstoreclock(nlm_req, req) == -1)
					goto abnormal;
				break;
			case NLM_NM_LOCK:
				Rqstp->rq_proc = NLM_LOCK; /* fake it */
			case NLM_LOCK:
			case NLM_LOCK_MSG:
			case NLM_LOCK_RECLAIM:
				if (nlm_lockargstoreclock(nlm_req, req) == -1)
					goto abnormal;
				break;
			case NLM_CANCEL:
			case NLM_CANCEL_MSG:
				if (nlm_cancargstoreclock(nlm_req, req) == -1)
					goto abnormal;
				break;
			case NLM_UNLOCK:
			case NLM_UNLOCK_MSG:
				if (nlm_unlockargstoreclock(nlm_req, req) == -1)
					goto abnormal;
				break;
			}
			if (debug == 3) {
				if (fwrite(&nlm, sizeof (int), 1, fp) == 0)
					fprintf(stderr, "fwrite nlm error\n");
				if (fwrite(&Rqstp->rq_proc,
					sizeof (int), 1, fp) == 0)
					fprintf(stderr,
						"fwrite nlm_proc error\n");
				printf("range[%d, %d] \n", req->lck.lox.base,
					req->lck.lox.length);
				(void) fflush(fp);
			}

			if (map_klm_nlm(req) == -1)
				goto abnormal;

			if (grace_period >0 && !(req->reclaim)) {
				if (debug)
					printf("during grace period, please retry later\n");
				nlm_reply(Rqstp->rq_proc, &res_grace, req);
				req->rel = 1;
				release_nlm_req(Rqstp->rq_proc, nlm_req);
				release_reclock(req);
				(void) sigsetmask(oldmask);
				return;
			}
			if (grace_period >0 && debug)
				printf("accept reclaim request(%x)\n", req);
			if (monitor_this_lock &&
				(Rqstp->rq_proc == NLM_LOCK ||
				 Rqstp->rq_proc == NLM_LOCK_MSG ||
				 Rqstp->rq_proc == NLM_GRANTED_MSG ||
				 Rqstp->rq_proc == NLM_LOCK_RECLAIM))
				if (add_mon(req, 1) == -1) {
					req->rel = 1;
					release_reclock(req);
					release_nlm_req(Rqstp->rq_proc, nlm_req);
					fprintf(stderr,
						"req discard due status monitor problem\n");
					(void) sigsetmask(oldmask);
					return;
				}
			(*Local)(req);
			release_reclock(req);
			/* check if req cause nlm calling klm back */
#ifdef NOTUSE
			if (req->rel) {
				release_nlm_req(Rqstp->rq_proc, nlm_req);
			}
#endif
		}

		/* free up orignal request */
		release_nlm_req(Rqstp->rq_proc, nlm_req);
	} else {
		/* msg reply */
		if ((reply = get_res()) != NULL) {
			if (!svc_getargs(Transp, xdr_Argument, nlm_rep)) {
				release_nlm_rep(Rqstp->rq_proc, nlm_rep);
				svcerr_decode(Transp);
				(void) sigsetmask(oldmask);
				return;
			}
			switch (Rqstp->rq_proc) {
			case NLM_TEST_RES:
				if (nlm_testrestoremote_result(nlm_rep, reply) == -1) {
					release_nlm_rep(Rqstp->rq_proc, nlm_rep);
					release_res(reply);
					(void) sigsetmask(oldmask);
					return;
				}
				break;
			case NLM_LOCK_RES:
			case NLM_CANCEL_RES:
			case NLM_UNLOCK_RES:
			case NLM_GRANTED_RES:
				if (nlm_restoremote_result(nlm_rep, reply) == -1) {
					release_nlm_rep(Rqstp->rq_proc, nlm_rep);
					release_res(reply);
					(void) sigsetmask(oldmask);
					return;
				}
				break;
			}
			if (debug == 3) {
				if (fwrite(&nlm, sizeof (int), 1, fp) == 0)
					fprintf(stderr,
						"fwrite nlm_reply error\n");
				if (fwrite(&Rqstp->rq_proc, sizeof (int),
					1, fp) == 0)
					fprintf(stderr,
						"fwrite nlm_reply_proc error \n");
				(void) fflush(fp);
			}
			if (debug)
				printf("msg reply(%d) to procedure(%d)\n",
					reply->lstat, Rqstp->rq_proc);
			(*Local)(reply);
		} else {
			/* malloc failure, do nothing */
		}

		/* free up orignal reply */
		release_nlm_rep(Rqstp->rq_proc, nlm_rep);
	}
	(void) sigsetmask(oldmask);
	if (debug)
		printf("EXITING FROM NLM_PROG...\n");
	return;

abnormal:
	/* malloc error, release allocated space and error return */
#ifdef NOTUSE
	nlm_reply((int) Rqstp->rq_proc, &res_nolock, req);
#endif
	req->rel = 1;
	release_reclock(req);
	release_nlm_req(Rqstp->rq_proc, nlm_req);
	(void) sigsetmask(oldmask);
	return;
}

static void
klm_prog(Rqstp, Transp)
	struct svc_req *Rqstp;
	SVCXPRT *Transp;
{
	int			oldmask;
	char			*klm_req;
	struct reclock		*req;
	msg_entry		*msgp;
	bool_t 			(*xdr_Argument)(), (*xdr_Result)();
	char 			*(*Local)();
	klm_lockargs		*klm_lockargs_a;
	klm_unlockargs		*klm_unlockargs_a;
	klm_testargs		*klm_testargs_a;

	extern klm_testrply 	*proc_klm_test();
	extern klm_stat 	*proc_klm_lock();
	extern klm_stat 	*proc_klm_cancel();
	extern klm_stat 	*proc_klm_unlock();
	extern klm_stat         *proc_klm_granted();
	extern klm_lockargs 	*get_klm_lockargs();
	extern klm_unlockargs 	*get_klm_unlockargs();
	extern klm_testargs 	*get_klm_testargs();
	extern int 		klm_lockargstoreclock();
	extern int 		klm_unlockargstoreclock();
	extern int 		klm_testargstoreclock();

	if (debug)
		printf("KLM_PROG+++ version %d proc %d\n",
			Rqstp->rq_vers, Rqstp->rq_proc);

	oldmask = sigblock (1 << (SIGALRM -1));
	klm_transp = Transp;
	klm_msg = NULL;

	switch (Rqstp->rq_proc) {
	case NULLPROC:
		svc_sendreply(Transp, xdr_void, NULL);
		(void) sigsetmask(oldmask);
		return;

	case DEBUG_ON:
		debug = 2;
		svc_sendreply(Transp, xdr_void, NULL);
		(void) sigsetmask(oldmask);
		return;

	case DEBUG_OFF:
		debug = 0;
		svc_sendreply(Transp, xdr_void, NULL);
		(void) sigsetmask(oldmask);
		return;

	case KILL_IT:
		svc_sendreply(Transp, xdr_void, NULL);
		fprintf(stderr, "rpc.lockd killed upon request\n");
		exit(2);

	case KLM_TEST:
		xdr_Argument = xdr_klm_testargs;
		xdr_Result = xdr_klm_testrply;
		Local = (char *(*)()) proc_klm_test;
		if ((klm_testargs_a = get_klm_testargs()) == NULL) {
			printf("rpc.lockd: unable to allocate space for klm_req.\n");
			(void) sigsetmask(oldmask);
			return;
		}
		klm_req = (char *) klm_testargs_a;
		break;

	case KLM_LOCK:
		xdr_Argument = xdr_klm_lockargs;
		xdr_Result = xdr_klm_stat;
		Local = (char *(*)()) proc_klm_lock;
		if ((klm_lockargs_a = get_klm_lockargs()) == NULL) {
			printf("rpc.lockd: unable to allocate space for klm_req.\n");
			(void) sigsetmask(oldmask);
			return;
		}
		klm_req = (char *) klm_lockargs_a;
		break;

	case KLM_CANCEL:
		xdr_Argument = xdr_klm_lockargs;
		xdr_Result = xdr_klm_stat;
		Local = (char *(*)()) proc_klm_cancel;
		if ((klm_lockargs_a = get_klm_lockargs()) == NULL) {
			printf("rpc.lockd: unable to allocate space for klm_req.\n");
			(void) sigsetmask(oldmask);
			return;
		}
		klm_req = (char *) klm_lockargs_a;
		break;

	case KLM_UNLOCK:
		xdr_Argument = xdr_klm_unlockargs;
		xdr_Result = xdr_klm_stat;
		Local = (char *(*)()) proc_klm_unlock;
		if ((klm_unlockargs_a = get_klm_unlockargs()) == NULL) {
			printf("rpc.lockd: unable to allocate space for klm_req.\n");
			(void) sigsetmask(oldmask);
			return;
		}
		klm_req = (char *) klm_unlockargs_a;
		break;

	case KLM_GRANTED:
		xdr_Argument = xdr_klm_lockargs;
		xdr_Result = xdr_klm_stat;
		Local = (char *(*)()) proc_klm_granted;
		if ((klm_lockargs_a = get_klm_lockargs()) == NULL) {
			printf("rpc.lockd: unable to allocate space for klm_req.\n");                   
			(void) sigsetmask(oldmask);
			return;
		}
		klm_req = (char *) klm_lockargs_a;
		break;

	default:
		svcerr_noproc(Transp);
		(void) sigsetmask(oldmask);
		return;
	}

	if ((req = get_reclock()) != NULL) {
		if (!svc_getargs(Transp, xdr_Argument, klm_req)) {
			release_klm_req(Rqstp->rq_proc, klm_req);
			svcerr_decode(Transp);
			req->rel= 1;
			release_reclock(req);
			(void) sigsetmask(oldmask);
			return;
		}
		switch (Rqstp->rq_proc) {
		case KLM_TEST:
			if (klm_testargstoreclock(klm_req, req) == -1)
				goto abnormal;
			break;
		case KLM_LOCK:
		case KLM_CANCEL:
		case KLM_GRANTED:
			if (klm_lockargstoreclock(klm_req, req) == -1)
				goto abnormal;
			break;
		case KLM_UNLOCK:
			if (klm_unlockargstoreclock(klm_req, req) == -1)
				goto abnormal;
			break;
		}
		req->lck.lox.LockID = LockID++;

		if (debug == 3){
			if (fwrite(&klm, sizeof (int), 1, fp) == 0)
				fprintf(stderr, "fwrite klm error\n");
			if (fwrite(&Rqstp->rq_proc, sizeof (int), 1, fp) == 0)
				fprintf(stderr, "fwrite klm_proc error\n");
			(void) fflush(fp);
		}

		if (map_kernel_klm(req) == -1)
			goto abnormal;
		if (grace_period > 0 && !(req->reclaim)) {
			/*
			 * put msg in queue and delay reply, unless there is
			 * no queue space
			 */
			if (debug)
				printf("during grace period, please retry later\n");
			if ((msgp = queue(req, (int) Rqstp->rq_proc)) == NULL) {
				klm_reply(Rqstp->rq_proc, &res_working);
				req->rel = 1;
				release_reclock(req);
				release_klm_req(Rqstp->rq_proc, klm_req);
				(void) sigsetmask(oldmask);
				return;
			}
			req->rel = 1;
			release_reclock(req);
			release_klm_req(Rqstp->rq_proc, klm_req);
			klm_msg = msgp;
			(void) sigsetmask(oldmask);
			return;
		}
		if (grace_period >0 && debug)
			printf("accept reclaim request\n");
		if (Rqstp->rq_proc == KLM_LOCK)
			if (add_mon(req, 1) == -1) {
				req->rel = 1;
				release_reclock(req);
				release_klm_req(Rqstp->rq_proc, klm_req);
				fprintf(stderr,
					"req discard due status monitor problem\n");
				(void) sigsetmask(oldmask);
				return;
			}

/*
		insert_proentry(req);
*/
		(*Local)(req);
		release_reclock(req);
	}
	else { /* malloc failure */
		klm_reply((int) Rqstp->rq_proc, &res_nolock);
	}
	(void) sigsetmask(oldmask);
	if (debug)
		printf("EXITING FROM KLM_PROG....\n");

	/* release storage used initially for klm */
	release_klm_req(Rqstp->rq_proc, klm_req);

	return;

abnormal:

	klm_reply((int) Rqstp->rq_proc, &res_nolock);
	req->rel = 1;
	release_reclock(req);
	release_klm_req(Rqstp->rq_proc, klm_req);
	(void) sigsetmask(oldmask);
	return;
}


main(argc, argv)
	int argc;
	char ** argv;
{
	SVCXPRT *Transp;
	int c;
	int t;
	int ppid;
	FILE *fopen();
	extern int optind;
	extern char *optarg;
	struct bindhostname_entry *bhn_entry;
	struct hostent *hp;
	int sd;
	struct sockaddr_in addr;
	char hostname[MAXHOSTNAMELEN];
	int i,j;
	char filePath[MAXPATHLEN];
	FILE *runFile;
	pid_t pid;
	struct servent *servbuf;
	ushort portnum;

	struct svcreg_entry svcreg_table[] =
	{ 
	  { NLM_PROG,  NLM_VERSX,  0, nlm_prog  },
          { NLM_PROG,  NLM_VERSX,  0, nlm_prog  },
          { PRIV_PROG, PRIV_VERS,  0, priv_prog },
          { PRIV_PROG, PRIV_VERS,  0, priv_prog },
          { NLM_PROG,  NLM_VERS,   0, nlm_prog  },
          { NLM_PROG,  NLM_VERS,   0, nlm_prog  },
          { KLM_PROG,  KLM_VERS,   0, klm_prog  },
          { KLM_PROG,  KLM_VERS,   0, klm_prog  },
          { NULL },
        };

	LM_GRACE = LM_GRACE_DEFAULT;
	LM_TIMEOUT = LM_TIMEOUT_DEFAULT;
	HASH_SIZE = 29;
	report_sharing_conflicts = 0;
	
	while ((c = getopt(argc, argv, "s:t:d:g:h:b:")) != EOF)
		switch (c) {
		case 's':
			report_sharing_conflicts++;
			break;
		case 't':
			(void) sscanf(optarg, "%d", &LM_TIMEOUT);
			break;
		case 'd':
			(void) sscanf(optarg, "%d", &debug);
			break;

		case 'g':
			(void) sscanf(optarg, "%d", &t);
			LM_GRACE = 1 + t/LM_TIMEOUT;
			break;
		case 'h':
			(void) sscanf(optarg, "%d", &HASH_SIZE);
			break;
		case 'b':
			bindhostname = optarg;
			break;
		default:
			fprintf(stderr, "rpc.lockd -t[timeout] -g[grace_period] -h[hashsize] -s -b[hostname] -d[debug]\n");
			return (0);
		}

	(void) get_bindhostname(bindhostname);
	callback_ip = (int) (bindhostname ? bindhostname_q->addr.sin_addr.s_addr : htonl(INADDR_LOOPBACK));

	if (debug)
		printf("lm_timeout = %d secs, grace_period = %d secs, hashsize = %d\n",
		 LM_TIMEOUT,  LM_GRACE * LM_TIMEOUT, HASH_SIZE);

	if (!debug) {
		ppid = fork();
		if (ppid == -1) {
			(void) fprintf(stderr, "rpc.lockd: fork failure\n");
			(void) fflush(stderr);
			abort();
		}
		if (ppid != 0) {
			exit(0);
		}
		for (t = 0; t< 20; t++) {
			(void) close(t);
		}

		(void) open("/dev/console", 2);
		(void) open("/dev/console", 2);
		(void) open("/dev/console", 2);

		(void) setpgrp(0, 0);
	}
	else {
		setlinebuf(stderr);
		setlinebuf(stdout);
	}

	init();

	(void) signal(SIGALRM, xtimer);

	gethostname(hostname, MAXHOSTNAMELEN);
	hp = gethostbyname(hostname);
	bzero((char *)&addr, sizeof(addr));
	bcopy(hp->h_addr, (char *)&addr.sin_addr.s_addr,hp->h_length);
	addr.sin_family = hp->h_addrtype;
	
	for (bhn_entry = bindhostname_q; bhn_entry != NULL; bhn_entry=bhn_entry->next) {

		if (debug) {
			hp = gethostbyaddr((char *)&bhn_entry->addr.sin_addr,sizeof(struct in_addr),AF_INET);
			printf("\n");
			printf("binding %s (%s)\n",hp->h_name,inet_ntoa(bhn_entry->addr.sin_addr));
		}


		for (i=0; svcreg_table[i].prognum != NULL; i=i++) {                                                  						
			/*
			 * get and bind a IP/port for TCP service
			 *
			 * ...but what is really happening?  
			 *
			 * The code begins by creating a socket and getting a
			 * portnumber.  This is a bit complicated because it
			 * is important for products like ASE that the
			 * portnumber be the same for each instance of the
			 * [prognum,versnum].  The "host binding" will insure
			 * that each instance services the respective RPC.
			 * If a portnumber has been reserved, the code will
			 * ask for a portnumber from portmapper.  If
			 * portmapper does not have the program registered
			 * then it assumes that the code will the system to
			 * get a portnumber via bind.  If portmapper does
			 * return with a portnumber it is checked to see if
			 * we have already registered this program (but it
			 * was for a different version).  If so, it is
			 * treated as if a portnumber had not been reserved.
			 * If the portnumber is new, then the code will then
			 * register the portnumber with the portmapper.
			 * Within the single loop, the sequence is repeated
			 * for both TCP and UDP.
			 */

			if ((sd = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP)) < 0) {
				perror("rpc.lockd socket (tcp)");
				exit(1);
			}
			if (svcreg_table[i].portnum == 0) {
				portnum = pmap_getport(&addr,
						       svcreg_table[i].prognum,
						       svcreg_table[i].versnum,
						       IPPROTO_TCP);
				if (portnum != 0) {
					for (j=0; svcreg_table[j].prognum != NULL; j++) {
						if (((j & 1) == 0) && (portnum == svcreg_table[j].portnum)) {
							portnum = 0;
							break;
						}
					}
				}
				svcreg_table[i].portnum = portnum;
			}
			bhn_entry->addr.sin_port = htons(svcreg_table[i].portnum);
			if (bind(sd, &bhn_entry->addr, sizeof(bhn_entry->addr))) {
				perror("rpc.lockd bind (tcp)");
				exit(1);
			}
			Transp = svctcp_create(sd, 0, 0);
			if (Transp == NULL) {
				fprintf(stderr, "svctcp_create error\n");
				exit(1);
			}
			if (!svc_register(Transp, 
					  svcreg_table[i].prognum,
					  svcreg_table[i].versnum,
					  svcreg_table[i].dispatch,
					  IPPROTO_TCP)) {
				fprintf(stderr,
					"rpc.lockd svc_register error for %d, %d (tcp)\n",
					svcreg_table[i].prognum,
					svcreg_table[i].versnum);
				exit(1);
			}
			if (svcreg_table[i].portnum == 0) {
				svcreg_table[i].portnum = 
					pmap_getport(&addr,
						     svcreg_table[i].prognum,
						     svcreg_table[i].versnum,
						     IPPROTO_TCP);
			}

			/*
			 * bind and register for UDP
			 */

			i++;
			if ((sd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
				perror("rpc.lockd socket (udp)");
				exit(1);
			}			


			if (svcreg_table[i].portnum == 0) {
				portnum = pmap_getport(&addr,
						       svcreg_table[i].prognum,
						       svcreg_table[i].versnum,
						       IPPROTO_UDP);
				if (portnum != 0) {
					for (j=0; svcreg_table[j].prognum != NULL; j++) {
						if ((j & 1) && (portnum == svcreg_table[j].portnum)) {
							portnum = 0;
							break;
						}
					}
				}
				svcreg_table[i].portnum = portnum;
			}
			bhn_entry->addr.sin_port = htons(svcreg_table[i].portnum);
			if (bind(sd, &bhn_entry->addr, sizeof(bhn_entry->addr))) {
				perror("rpc.lockd bind (udp)");
				exit(1);
			}
			Transp = svcudp_bufcreate(sd, 1000, 1000);
			if (Transp == NULL) {
				fprintf(stderr, "svcudp_bufcreate error\n");
				exit(1);
			}
			if (!svc_register(Transp, 
					  svcreg_table[i].prognum,
					  svcreg_table[i].versnum,
					  svcreg_table[i].dispatch,
					  IPPROTO_UDP)) {
				fprintf(stderr, 
					"rpc.lockd svc_register error for %d, %d (udp)\n",
					svcreg_table[i].prognum,
					svcreg_table[i].versnum);
				exit(1);
			}
			if (svcreg_table[i].portnum == 0) {
				svcreg_table[i].portnum = 
					pmap_getport(&addr,
						     svcreg_table[i].prognum,
						     svcreg_table[i].versnum,
						     IPPROTO_UDP);
			}
			if (!svcudp_enablecache(Transp, 15)) {
				fprintf(stderr, "svcudp_enablecache failed\n");
				exit(1);
			}
		}
	}

	cancel_mon();
	init_nlm_share();

	if (debug == 3) {
		printf("lockd create logfile\n");
		klm = KLM_PROG;
		nlm = NLM_PROG;
		if ((fp = fopen("logfile", "w+")) == NULL) {
			perror("logfile fopen:");
			exit(1);
		}
		xdrstdio_create(&x, fp, XDR_ENCODE);
	}

	/*
	 * Place entry in /var/run
	 */
	pid = getpid();
	if (bindhostname) {
		sprintf(filePath, "/var/run/rpc.lockd.%s.pid", bindhostname);
	} else {
		sprintf(filePath, "/var/run/rpc.lockd.pid");
	}
	if ((runFile = fopen(filePath, "w")) == 0) {
		fprintf(stderr, "Can't open PID file: %s: ", filePath);
	} else {
		fprintf(runFile, "%d", pid);
		fclose(runFile);
	}

	(void) alarm(LM_TIMEOUT);
	svc_run();
	fprintf(stderr, "svc_run returned\n");
	exit(1);
	/* NOTREACHED */
}
