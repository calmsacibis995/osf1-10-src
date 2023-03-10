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
static char *sccsid = "@(#)$RCSfile: bootparam_xdr.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 04:59:55 $";
#endif
/*
 */
/*
 * OSF/1 Release 1.0
 */

/* 
 * Copyright (c) 1988,1990 by Sun Microsystems, Inc.
 * @(#) from SUN 1.3
 */

#ifdef KERNEL
#include "../rpc/rpc.h"
#include "../rpcsvc/bootparam.h"
#else
#include <rpc/rpc.h>
#include <rpcsvc/bootparam.h>
#endif


bool_t
xdr_bp_machine_name_t(xdrs,objp)
	XDR *xdrs;
	bp_machine_name_t *objp;
{
	if (! xdr_string(xdrs, objp, MAX_MACHINE_NAME)) {
		return(FALSE);
	}
	return(TRUE);
}




bool_t
xdr_bp_path_t(xdrs,objp)
	XDR *xdrs;
	bp_path_t *objp;
{
	if (! xdr_string(xdrs, objp, MAX_PATH_LEN)) {
		return(FALSE);
	}
	return(TRUE);
}




bool_t
xdr_bp_fileid_t(xdrs,objp)
	XDR *xdrs;
	bp_fileid_t *objp;
{
	if (! xdr_string(xdrs, objp, MAX_FILEID)) {
		return(FALSE);
	}
	return(TRUE);
}




bool_t
xdr_ip_addr_t(xdrs,objp)
	XDR *xdrs;
	ip_addr_t *objp;
{
	if (! xdr_char(xdrs, &objp->net)) {
		return(FALSE);
	}
	if (! xdr_char(xdrs, &objp->host)) {
		return(FALSE);
	}
	if (! xdr_char(xdrs, &objp->lh)) {
		return(FALSE);
	}
	if (! xdr_char(xdrs, &objp->impno)) {
		return(FALSE);
	}
	return(TRUE);
}




bool_t
xdr_bp_address(xdrs,objp)
	XDR *xdrs;
	bp_address *objp;
{
	static struct xdr_discrim choices[] = {
		{ (int) IP_ADDR_TYPE, xdr_ip_addr_t },
		{ __dontcare__, NULL }
	};

	if (! xdr_union(xdrs, (enum_t *) &objp->address_type, (char *) &objp->bp_address, choices, (xdrproc_t) NULL)) {
		return(FALSE);
	}
	return(TRUE);
}




bool_t
xdr_bp_whoami_arg(xdrs,objp)
	XDR *xdrs;
	bp_whoami_arg *objp;
{
	if (! xdr_bp_address(xdrs, &objp->client_address)) {
		return(FALSE);
	}
	return(TRUE);
}




bool_t
xdr_bp_whoami_res(xdrs,objp)
	XDR *xdrs;
	bp_whoami_res *objp;
{
	if (! xdr_bp_machine_name_t(xdrs, &objp->client_name)) {
		return(FALSE);
	}
	if (! xdr_bp_machine_name_t(xdrs, &objp->domain_name)) {
		return(FALSE);
	}
	if (! xdr_bp_address(xdrs, &objp->router_address)) {
		return(FALSE);
	}
	return(TRUE);
}




bool_t
xdr_bp_getfile_arg(xdrs,objp)
	XDR *xdrs;
	bp_getfile_arg *objp;
{
	if (! xdr_bp_machine_name_t(xdrs, &objp->client_name)) {
		return(FALSE);
	}
	if (! xdr_bp_fileid_t(xdrs, &objp->file_id)) {
		return(FALSE);
	}
	return(TRUE);
}




bool_t
xdr_bp_getfile_res(xdrs,objp)
	XDR *xdrs;
	bp_getfile_res *objp;
{
	if (! xdr_bp_machine_name_t(xdrs, &objp->server_name)) {
		return(FALSE);
	}
	if (! xdr_bp_address(xdrs, &objp->server_address)) {
		return(FALSE);
	}
	if (! xdr_bp_path_t(xdrs, &objp->server_path)) {
		return(FALSE);
	}
	return(TRUE);
}


