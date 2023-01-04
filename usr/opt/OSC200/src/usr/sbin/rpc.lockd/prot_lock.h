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
/*
 *	@(#)$RCSfile: prot_lock.h,v $ $Revision: 1.1.5.2 $ (DEC) $Date: 1993/08/17 20:04:39 $
 */
/*
 * @(#)prot_lock.h	1.2 90/07/23 NFSSRC4.0 1.8 87/03/09
 *
 * This file consists of all structure information used by lock manager 
 */

#include <rpc/rpc.h>
#include "nlm_prot.h"
#include "klm_prot.h"
#include "lockf.h"

typedef struct nlm_testres remote_result;
#define lstat stat.stat
#define lholder stat.nlm_testrply_u.holder

#define NLM_LOCK_RECLAIM	16
#define MSG 	0		/* choices of comm to remote svr */
#define RPC 	1		/* choices of comm to remote svr */

#define MAXLEN		((1 << 31) -1)
#define lck 		alock
#define svr		server_name
#define caller		caller_name
#define clnt		clnt_name
#define fh_len		fh.n_len
#define fh_bytes	fh.n_bytes
#define oh_len		oh.n_len
#define oh_bytes	oh.n_bytes
#define cookie_len	cookie.n_len
#define cookie_bytes	cookie.n_bytes

/*
#define granted		nlm_granted
*/
#define denied		nlm_denied
#define nolocks 	nlm_denied_nolocks
#define blocking	nlm_blocked
#define grace		nlm_denied_grace_period
#define deadlck		nlm_deadlck

/*
 * warning:  struct alock consists of klm_lock and nlm_lock,
 * it has to be modified if either structure has been modified!!!
 */
struct alock {
	netobj cookie;

	/* from klm_prot.h */
        char *server_name;
        netobj fh;

	/* from lockf.h */
	struct data_lock lox;

	/* addition from nlm_prot.h */
	char *caller_name;
	netobj oh;
	int svid;
	u_int l_offset;
        u_int l_len;

	/* addition from lock manager */
	char *clnt_name;
	int op;
};


struct reclock {
	netobj cookie;
	bool_t block;
	bool_t exclusive;
	struct alock alock;
	bool_t reclaim;
	int state;

	/* auxiliary structure */
	SVCXPRT *transp; /* transport handle for delayed response due to */ 
			 /* blocking or grace period */
	int rel;
	int w_flag;

	struct reclock *prev; 
	struct reclock *next;
};
typedef struct reclock reclock;

/*
 * Lock manager vp->v_filocks
 */
struct lm_vnode {
        char *server_name;
        netobj fh;
        struct reclock *exclusive;	/* exclusive locks */
	struct reclock *shared;		/* shared locks */
	struct reclock *pending;	/* pending locks */
	struct lm_vnode *prev;
	struct lm_vnode *next;
	struct reclock *reclox;
	int    rel;
};
typedef struct lm_vnode lm_vnode;

struct timer {
	/* timer goes off when exp == curr */
	int exp;
	int curr;
};
typedef struct timer timer;

/*
 * msg passing structure
 */
struct msg_entry {
	reclock *req;
	remote_result *reply;
	timer t;
	int proc; /* procedure name that req is sent to; needed for reply purpose */
	struct msg_entry *prev;
	struct msg_entry *nxt;
	int pmap_ctr;		/* Count times we couldn't reach server */
};
typedef struct msg_entry msg_entry;

struct priv_struct {
	int pid;
	int *priv_ptr;
};

struct fd_table {
	netobj  fh;
	int fd;
};
