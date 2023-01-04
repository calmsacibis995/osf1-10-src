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
 * @(#)$RCSfile: rpc_msg.h,v $ $Revision: 1.1.3.3 $ (DEC) $Date: 1992/03/30 23:16:01 $
 */
/*	@(#)rpc_msg.h	1.2 90/07/17 4.1NFSSRC SMI	*/

/* 
 * Copyright (c) 1990 by Sun Microsystems, Inc.
 *      @(#)rpc_msg.h 1.8 88/02/08 SMI      
 */


/*
 * rpc_msg.h
 * rpc message definition
 */

#ifndef _rpc_rpc_msg_h
#define	_rpc_rpc_msg_h

#define	RPC_MSG_VERSION		((u_int) 2)
#define	RPC_SERVICE_PORT	((u_short) 2048)

/*
 * Bottom up definition of an rpc message.
 * NOTE: call and reply use the same overall stuct but
 * different parts of unions within it.
 */

enum msg_type {
	CALL=0,
	REPLY=1
};

enum reply_stat {
	MSG_ACCEPTED=0,
	MSG_DENIED=1
};

enum accept_stat {
	SUCCESS=0,
	PROG_UNAVAIL=1,
	PROG_MISMATCH=2,
	PROC_UNAVAIL=3,
	GARBAGE_ARGS=4,
	SYSTEM_ERR=5
};

enum reject_stat {
	RPC_MISMATCH=0,
	AUTH_ERROR=1
};

/*
 * Reply part of an rpc exchange
 */

/*
 * Reply to an rpc request that was accepted by the server.
 * Note: there could be an error even though the request was
 * accepted.
 */
struct accepted_reply {
	struct opaque_auth	ar_verf;
	enum accept_stat	ar_stat;
	union {
		struct {
			u_int	low;
			u_int	high;
		} AR_versions;
		struct {
			caddr_t	where;
			xdrproc_t proc;
		} AR_results;
		/* and many other null cases */
	} ru;
#define	ar_results	ru.AR_results
#define	ar_vers		ru.AR_versions
};

/*
 * Reply to an rpc request that was rejected by the server.
 */
struct rejected_reply {
	enum reject_stat rj_stat;
	union {
		struct {
			u_int low;
			u_int high;
		} RJ_versions;
		enum auth_stat RJ_why;  /* why authentication did not work */
	} ru;
#define	rj_vers	ru.RJ_versions
#define	rj_why	ru.RJ_why
};

/*
 * Body of a reply to an rpc request.
 */
struct reply_body {
	enum reply_stat rp_stat;
	union {
		struct accepted_reply RP_ar;
		struct rejected_reply RP_dr;
	} ru;
#define	rp_acpt	ru.RP_ar
#define	rp_rjct	ru.RP_dr
};

/*
 * Body of an rpc request call.
 */
struct call_body {
	u_int cb_rpcvers;	/* must be equal to two */
	u_int cb_prog;
	u_int cb_vers;
	u_int cb_proc;
	struct opaque_auth cb_cred;
	struct opaque_auth cb_verf; /* protocol specific - provided by client */
};

/*
 * The rpc message
 */
struct rpc_msg {
	u_int			rm_xid;
	enum msg_type		rm_direction;
	union {
		struct call_body RM_cmb;
		struct reply_body RM_rmb;
	} ru;
#define	rm_call		ru.RM_cmb
#define	rm_reply	ru.RM_rmb
};
#define	acpted_rply	ru.RM_rmb.ru.RP_ar
#define	rjcted_rply	ru.RM_rmb.ru.RP_dr


/*
 * XDR routine to handle a rpc message.
 * xdr_callmsg(xdrs, cmsg)
 * 	XDR *xdrs;
 * 	struct rpc_msg *cmsg;
 */
extern bool_t	xdr_callmsg();

/*
 * XDR routine to pre-serialize the static part of a rpc message.
 * xdr_callhdr(xdrs, cmsg)
 * 	XDR *xdrs;
 * 	struct rpc_msg *cmsg;
 */
extern bool_t	xdr_callhdr();

/*
 * XDR routine to handle a rpc reply.
 * xdr_replymsg(xdrs, rmsg)
 * 	XDR *xdrs;
 * 	struct rpc_msg *rmsg;
 */
extern bool_t	xdr_replymsg();

/*
 * Fills in the error part of a reply message.
 * _seterr_reply(msg, error)
 * 	struct rpc_msg *msg;
 * 	struct rpc_err *error;
 */
extern void	_seterr_reply();

#endif /*!_rpc_rpc_msg_h*/
