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
static char *sccsid = "@(#)$RCSfile: showfh_xdr.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 05:00:37 $";
#endif
/*
 */
/*
 * OSF/1 Release 1.0
 */

/*
 * Copyright (c) 1988,1990 by Sun Microsystems, Inc.
 */

#include <rpc/rpc.h>
#include <rpcsvc/showfh.h>

bool_t
xdr_res_handle(xdrs, objp)
	XDR *xdrs;
	res_handle *objp;
{
	if (!xdr_string(xdrs, &objp->file_there, MAXNAMELEN)) {
		return (FALSE);
	}
	if (!xdr_int(xdrs, &objp->result)) {
		return (FALSE);
	}
	return (TRUE);
}

bool_t
xdr_nfs_handle(xdrs, objp)
	XDR *xdrs;
	nfs_handle *objp;
{
	if (!xdr_array(xdrs, (char **)&objp->cookie.cookie_val, (u_int *)&objp->cookie.cookie_len, MAXNAMELEN, sizeof(u_int), xdr_u_int)) {
		return (FALSE);
	}
	return (TRUE);
}
