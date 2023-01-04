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
static char *sccsid = "@(#)$RCSfile: ipalloc.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 04:59:52 $";
#endif
/*
 */
/*
 * OSF/1 Release 1.0
 */

/*
 * Portions of this software have been licensed to
 * Digital Equipment Company, Maynard, MA.
 * Copyright (c) 1988 by Sun Microsystems, Inc.
 */

#include <rpc/rpc.h>
#include <rpcsvc/ipalloc.h>

bool_t
xdr_ip_status(xdrs, objp)
	XDR *xdrs;
	ip_status *objp;
{
	if (!xdr_enum(xdrs, (enum_t *)objp)) {
		return (FALSE);
	}
	return (TRUE);
}

bool_t
xdr_ip_alloc_arg(xdrs, objp)
	XDR *xdrs;
	ip_alloc_arg *objp;
{
	if (!xdr_opaque(xdrs, objp->etheraddr, 6)) {
		return (FALSE);
	}
	if (!xdr_u_long(xdrs, &objp->netnum)) {
		return (FALSE);
	}
	if (!xdr_u_long(xdrs, &objp->subnetmask)) {
		return (FALSE);
	}
	return (TRUE);
}

bool_t
xdr_ip_alloc_res(xdrs, objp)
	XDR *xdrs;
	ip_alloc_res *objp;
{
	if (!xdr_ip_status(xdrs, &objp->status)) {
		return (FALSE);
	}
	switch (objp->status) {
	case ip_success:
		if (!xdr_u_long(xdrs, &objp->ip_alloc_res_u.ipaddr)) {
			return (FALSE);
		}
		break;
	default:
		return (FALSE);
	}
	return (TRUE);
}

bool_t
xdr_ip_addr_arg(xdrs, objp)
	XDR *xdrs;
	ip_addr_arg *objp;
{
	if (!xdr_u_long(xdrs, &objp->ipaddr)) {
		return (FALSE);
	}
	return (TRUE);
}

bool_t
xdr_ip_toname_res(xdrs, objp)
	XDR *xdrs;
	ip_toname_res *objp;
{
	if (!xdr_ip_status(xdrs, &objp->status)) {
		return (FALSE);
	}
	switch (objp->status) {
	case ip_success:
		if (!xdr_string(xdrs, &objp->ip_toname_res_u.name, MAX_MACHINELEN)) {
			return (FALSE);
		}
		break;
	default:
		return (FALSE);
	}
	return (TRUE);
}
