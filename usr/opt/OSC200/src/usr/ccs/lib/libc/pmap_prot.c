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
static char     *sccsid = "@(#)$RCSfile: pmap_prot.c,v $ $Revision: 4.2.7.2 $ (DEC) $Date: 1993/06/07 23:34:22 $";
#endif
/*
 */


/* 
 * Copyright (c) 1988 by Sun Microsystems, Inc.
 * 1.18 88/02/08 
 */


/*
 * pmap_prot.c
 * Protocol for the local binder service, or pmap.
 */

/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#if !defined(_THREAD_SAFE)
#pragma weak xdr_pmap = __xdr_pmap
#endif
#endif
#ifdef KERNEL 
#include "../rpc/types.h"
#include "../rpc/xdr.h" 
#include "../rpc/pmap_prot.h"
#else 
#include <rpc/types.h>
#include <rpc/xdr.h>
#include <rpc/pmap_prot.h>
#endif 


bool_t
xdr_pmap(xdrs, regs)
	XDR *xdrs;
	struct pmap *regs;
{

	if (xdr_u_int(xdrs, &regs->pm_prog) && 
		xdr_u_int(xdrs, &regs->pm_vers) && 
		xdr_u_int(xdrs, &regs->pm_prot))
		return (xdr_u_int(xdrs, &regs->pm_port));
	return (FALSE);
}
