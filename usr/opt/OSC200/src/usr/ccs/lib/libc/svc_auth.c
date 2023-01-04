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
static char     *sccsid = "@(#)$RCSfile: svc_auth.c,v $ $Revision: 4.2.5.2 $ (DEC) $Date: 1993/06/07 23:10:33 $";
#endif
/*
 */


/* 
 * Copyright (c) 1988 by Sun Microsystems, Inc.
 * 1.20 88/02/08 
 */


/*
 * svc_auth.c, Server-side rpc authenticator interface.
 */

/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#endif
#ifdef KERNEL
#include "param.h"
#include "../netinet/in.h"
#include "../rpc/types.h"
#include "../rpc/xdr.h"
#include "../rpc/auth.h"
#include "../rpc/clnt.h"
#include "../rpc/rpc_msg.h"
#include "../rpc/svc.h"
#include "../rpc/svc_auth.h"
#else
#include <rpc/rpc.h>
#endif

/*
 * svcauthsw is the bdevsw of server side authentication. 
 * 
 * Server side authenticators are called from authenticate by
 * using the client auth struct flavor field to index into svcauthsw.
 * The server auth flavors must implement a routine that looks  
 * like: 
 * 
 *	enum auth_stat 
 *	flavorx_auth(rqst, msg)
 *		register struct svc_req *rqst; 
 *		register struct rpc_msg *msg;
 *  
 */

enum auth_stat _svcauth_null();		/* no authentication */
enum auth_stat _svcauth_unix();		/* unix style (uid, gids) */
enum auth_stat _svcauth_short();	/* short hand unix style */
#ifdef DESOK
enum auth_stat _svcauth_des();		/* des style */
#endif

static struct {
	enum auth_stat (*authenticator)();
} svcauthsw[] = {
	_svcauth_null,			/* AUTH_NULL */
	_svcauth_unix,			/* AUTH_UNIX */
	_svcauth_short,			/* AUTH_SHORT */
#ifdef DESOK 

	_svcauth_des			/* AUTH_DES */
#endif
};
#define	AUTH_MAX	3		/* HIGHEST AUTH NUMBER */


/*
 * The call rpc message, msg has been obtained from the wire.  The msg contains
 * the raw form of credentials and verifiers.  authenticate returns AUTH_OK
 * if the msg is successfully authenticated.  If AUTH_OK then the routine also
 * does the following things:
 * set rqst->rq_xprt->verf to the appropriate response verifier;
 * sets rqst->rq_client_cred to the "cooked" form of the credentials.
 *
 * NB: rqst->rq_cxprt->verf must be pre-alloctaed;
 * its length is set appropriately.
 *
 * The caller still owns and is responsible for msg->u.cmb.cred and
 * msg->u.cmb.verf.  The authentication system retains ownership of
 * rqst->rq_client_cred, the cooked credentials.
 *
 * There is an assumption that any flavour less than AUTH_NULL is
 * invalid.
 */
enum auth_stat
_authenticate(rqst, msg)
	register struct svc_req *rqst;
	struct rpc_msg *msg;
{
	register int cred_flavor;

	rqst->rq_cred = msg->rm_call.cb_cred;
	rqst->rq_xprt->xp_verf.oa_flavor = _null_auth.oa_flavor;
	rqst->rq_xprt->xp_verf.oa_length = 0;
	cred_flavor = rqst->rq_cred.oa_flavor;
	if ((cred_flavor <= AUTH_MAX) && (cred_flavor >= AUTH_NULL)) {
		return ((*(svcauthsw[cred_flavor].authenticator))(rqst, msg));
	}

	return (AUTH_REJECTEDCRED);
}

enum auth_stat
_svcauth_null(/*rqst, msg*/)
	/*struct svc_req *rqst;
	struct rpc_msg *msg;*/
{

	return (AUTH_OK);
}
