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
static char *rcsid = "@(#)$RCSfile: pmap_prot.c,v $ $Revision: 1.1.5.2 $ (DEC) $Date: 1993/05/18 21:06:44 $";
#endif
#if defined(LIBC_SCCS) && !defined(lint)
static char sccsid[] = 	"@(#)pmap_prot.c	1.4 90/07/17 4.1NFSSRC Copyr 1990 Sun Micro";
#endif

/* 
 * Copyright (c) 1988 by Sun Microsystems, Inc.
 * 1.18 88/02/08 
 */


/*
 * pmap_prot.c
 * Protocol for the local binder service, or pmap.
 */

#include <rpc/rpc.h>
#include <rpc/pmap_prot.h>

bool_t
xdr_pmap(xdrs, regs)
	XDR *xdrs;
	struct rpcpmap *regs;
{

	if (xdr_u_int(xdrs, &regs->pm_prog) && 
		xdr_u_int(xdrs, &regs->pm_vers) && 
		xdr_u_int(xdrs, &regs->pm_prot))
		return (xdr_u_int(xdrs, &regs->pm_port));
	return (FALSE);
}
