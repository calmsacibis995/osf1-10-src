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
 * @(#)$RCSfile: klm_prot.h,v $ $Revision: 1.1.5.2 $ (DEC) $Date: 1993/07/14 18:18:54 $
 */
/*	@(#)klm_prot.h	1.3 90/07/19 4.1NFSSRC SMI	*/

/* 
 * Copyright (c) 1988,1990 by Sun Microsystems, Inc.
 * @(#) from SUN 1.6
 */

#ifndef _KLM_PROT_H_
#define _KLM_PROT_H_

#ifdef	KERNEL
#include "../rpc/types.h"
#else
#include <rpc/types.h>
#endif /* KERNEL */
#define KLM_PROG ((u_int)100020)
#define KLM_VERS ((u_int)2)
#define KLM_TEST ((u_int)1)
#define KLM_LOCK ((u_int)2)
#define KLM_CANCEL ((u_int)3)
#define KLM_UNLOCK ((u_int)4)
#define KLM_GRANTED ((u_int)5)
#define LM_MAXSTRLEN 1024

enum klm_stats {
	klm_granted = 0,
	klm_denied = 1,
	klm_denied_nolocks = 2,
	klm_working = 3,
	klm_deadlck = 5,
};
typedef enum klm_stats klm_stats;
bool_t xdr_klm_stats();


struct klm_lock {
	char *server_name;
	netobj fh;
	int base;
	int length;
	int type;
	int granted;
	int color;
	int LockID;
	int pid;
	int class;
	int rsys;
	int rpid;
};
typedef struct klm_lock klm_lock;
bool_t xdr_klm_lock();


struct klm_holder {
	bool_t exclusive;
	int base;
	int length;
	int type;
	int granted;
	int color;
	int LockID;
	int pid;
	int class;
	int rsys;
	int rpid;
};
typedef struct klm_holder klm_holder;
bool_t xdr_klm_holder();

struct klm_stat {
	klm_stats stat;
};
typedef struct klm_stat klm_stat;
bool_t xdr_klm_stat();


struct klm_testrply {
	klm_stats stat;
	union {
		struct klm_holder holder;
	} klm_testrply_u;
};
typedef struct klm_testrply klm_testrply;
bool_t xdr_klm_testrply();


struct klm_lockargs {
	bool_t block;
	bool_t exclusive;
	struct klm_lock alock;
};
typedef struct klm_lockargs klm_lockargs;
bool_t xdr_klm_lockargs();


struct klm_testargs {
	bool_t exclusive;
	struct klm_lock alock;
};
typedef struct klm_testargs klm_testargs;
bool_t xdr_klm_testargs();


struct klm_unlockargs {
	struct klm_lock alock;
};
typedef struct klm_unlockargs klm_unlockargs;
bool_t xdr_klm_unlockargs();

#endif
