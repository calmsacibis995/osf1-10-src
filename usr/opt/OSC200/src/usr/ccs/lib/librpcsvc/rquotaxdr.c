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
static char *sccsid = "@(#)$RCSfile: rquotaxdr.c,v $ $Revision: 4.2.2.2 $ (DEC) $Date: 1993/05/26 17:45:07 $";
#endif
/*
 */
/*
 * OSF/1 Release 1.0
 */

/* 
 * Copyright (c) 1988,1990 by Sun Microsystems, Inc.
 * @(#) from SUN 1.7
 */

#include <stdio.h>
#include <rpc/rpc.h>
#include <ufs/quota.h>
#include <rpcsvc/rquota.h>


bool_t
xdr_getquota_args(xdrs, gq_argsp)
	XDR *xdrs;
	struct getquota_args *gq_argsp;
{
	extern bool_t xdr_path();

	return (xdr_path(xdrs, &gq_argsp->gqa_pathp) &&
	    xdr_int(xdrs, &gq_argsp->gqa_uid));
}

struct xdr_discrim gqr_arms[2] = {
	{ (int)Q_OK, xdr_rquota },
	{ __dontcare__, NULL }
};

bool_t
xdr_getquota_rslt(xdrs, gq_rsltp)
	XDR *xdrs;
	struct getquota_rslt *gq_rsltp;
{

	return (xdr_union(xdrs,
	    (int *) &gq_rsltp->gqr_status, (char *) &gq_rsltp->gqr_rquota,
	    gqr_arms, (xdrproc_t) xdr_void));
}

bool_t
xdr_rquota(xdrs, rqp)
	XDR *xdrs;
	struct rquota *rqp;
{

	return (xdr_int(xdrs, &rqp->rq_bsize) &&
	    xdr_bool(xdrs, &rqp->rq_active) &&
	    xdr_u_int(xdrs, &rqp->rq_bhardlimit) &&
	    xdr_u_int(xdrs, &rqp->rq_bsoftlimit) &&
	    xdr_u_int(xdrs, &rqp->rq_curblocks) &&
	    xdr_u_int(xdrs, &rqp->rq_fhardlimit) &&
	    xdr_u_int(xdrs, &rqp->rq_fsoftlimit) &&
	    xdr_u_int(xdrs, &rqp->rq_curfiles) &&
	    xdr_u_int(xdrs, &rqp->rq_btimeleft) &&
	    xdr_u_int(xdrs, &rqp->rq_ftimeleft) );
}
