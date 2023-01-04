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
static char     *sccsid = "@(#)$RCSfile: pmap_prot2.c,v $ $Revision: 4.2.5.2 $ (DEC) $Date: 1993/06/07 23:34:40 $";
#endif
/*
 */


/* 
 * Copyright (c) 1988 by Sun Microsystems, Inc.
 * 1.4 88/02/08 
 */


/*
 * pmap_prot2.c
 * Protocol for the local binder service, or pmap.
 */

/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#if !defined(_THREAD_SAFE)
#pragma weak xdr_pmaplist = __xdr_pmaplist
#endif
#endif
#include <rpc/types.h>
#include <rpc/xdr.h>
#include <rpc/pmap_prot.h>


/* 
 * What is going on with linked lists? (!)
 * First recall the link list declaration from pmap_prot.h:
 *
 * struct pmaplist {
 *	struct pmap pml_map;
 *	struct pmaplist *pml_map;
 * };
 *
 * Compare that declaration with a corresponding xdr declaration that 
 * is (a) pointer-less, and (b) recursive:
 *
 * typedef union switch (bool_t) {
 * 
 *	case TRUE: struct {
 *		struct pmap;
 * 		pmaplist_t foo;
 *	};
 *
 *	case FALSE: struct {};
 * } pmaplist_t;
 *
 * Notice that the xdr declaration has no nxt pointer while
 * the C declaration has no bool_t variable.  The bool_t can be
 * interpreted as ``more data follows me''; if FALSE then nothing
 * follows this bool_t; if TRUE then the bool_t is followed by
 * an actual struct pmap, and then (recursively) by the 
 * xdr union, pamplist_t.  
 *
 * This could be implemented via the xdr_union primitive, though this
 * would cause a one recursive call per element in the list.  Rather than do
 * that we can ``unwind'' the recursion
 * into a while loop and do the union arms in-place.
 *
 * The head of the list is what the C programmer wishes to past around
 * the net, yet is the data that the pointer points to which is interesting;
 * this sounds like a job for xdr_reference!
 */
bool_t
xdr_pmaplist(xdrs, rp)
	register XDR *xdrs;
	register struct pmaplist **rp;
{
	/*
	 * more_elements is pre-computed in case the direction is
	 * XDR_ENCODE or XDR_FREE.  more_elements is overwritten by
	 * xdr_bool when the direction is XDR_DECODE.
	 */
	bool_t more_elements;
	register int freeing = (xdrs->x_op == XDR_FREE);
	register struct pmaplist **next;

	while (TRUE) {
		more_elements = (bool_t)(*rp != NULL);
		if (! xdr_bool(xdrs, &more_elements))
			return (FALSE);
		if (! more_elements)
			return (TRUE);  /* we are done */
		/*
		 * the unfortunate side effect of non-recursion is that in
		 * the case of freeing we must remember the next object
		 * before we free the current object ...
		 */
		if (freeing)
			next = &((*rp)->pml_next); 
		if (! xdr_reference(xdrs, (caddr_t *)rp,
		    (u_int)sizeof(struct pmaplist), xdr_pmap))
			return (FALSE);
		rp = (freeing) ? next : &((*rp)->pml_next);
	}
}
