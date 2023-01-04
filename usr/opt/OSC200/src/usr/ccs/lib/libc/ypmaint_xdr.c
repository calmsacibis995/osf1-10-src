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
static char     *sccsid = "@(#)$RCSfile: ypmaint_xdr.c,v $ $Revision: 4.2.7.2 $ (DEC) $Date: 1993/06/08 00:29:02 $";
#endif
/*
 */


/* 
 * Copyright (c) 1988,1990 by Sun Microsystems, Inc.
 * 1.3 87/08/12 
 */


/*
 * This contains xdr routines used by the NIS interface
 * for systems and maintenance programs only.  This is a separate module
 * because most NIS clients should not need to link to it.
 */

/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#if !defined(_THREAD_SAFE)
#pragma weak xdr_ypall = __xdr_ypall
#pragma weak xdr_ypbind_setdom = __xdr_ypbind_setdom
#pragma weak xdr_ypmaplist = __xdr_ypmaplist
#pragma weak xdr_ypmaplist_wrap_string = __xdr_ypmaplist_wrap_string
#pragma weak xdr_yppushresp_xfr = __xdr_yppushresp_xfr
#pragma weak xdr_ypreq_xfr = __xdr_ypreq_xfr
#pragma weak xdr_ypresp_maplist = __xdr_ypresp_maplist
#pragma weak xdr_ypresp_master = __xdr_ypresp_master
#pragma weak xdr_ypresp_order = __xdr_ypresp_order
#endif
#endif
#define NULL 0L
#include <rpc/rpc.h>
#include <rpcsvc/yp_prot.h>
#include <rpcsvc/ypclnt.h>

bool xdr_ypmaplist();
bool xdr_ypmaplist_wrap_string();

/*
 * Serializes/deserializes a ypresp_master structure.
 */
bool
xdr_ypresp_master(xdrs, ps)
	XDR * xdrs;
	struct ypresp_master *ps;
{
	return (xdr_u_int(xdrs, &ps->status) &&
	     xdr_ypowner_wrap_string(xdrs, &ps->master) );
}

/*
 * Serializes/deserializes a ypresp_order structure.
 */
bool
xdr_ypresp_order(xdrs, ps)
	XDR * xdrs;
	struct ypresp_order *ps;
{
	return (xdr_u_int(xdrs, &ps->status) &&
	     xdr_u_int(xdrs, &ps->ordernum) );
}

/*
 * This is like xdr_ypmap_wrap_string except that it serializes/deserializes
 * an array, instead of a pointer, so xdr_reference can work on the structure
 * containing the char array itself.
 */
bool
xdr_ypmaplist_wrap_string(xdrs, pstring)
	XDR * xdrs;
	char *pstring;
{
	char *s;

	s = pstring;
	return (xdr_string(xdrs, &s, YPMAXMAP) );
}

/*
 * Serializes/deserializes a ypmaplist.
 */
bool
xdr_ypmaplist(xdrs, lst)
	XDR *xdrs;
	struct ypmaplist **lst;
{
	bool more_elements;
	int freeing = (xdrs->x_op == XDR_FREE);
	struct ypmaplist **next;

	while (TRUE) {
		more_elements = (*lst != (struct ypmaplist *) NULL);
		
		if (! xdr_bool(xdrs, &more_elements))
			return (FALSE);
			
		if (! more_elements)
			return (TRUE);  /* All done */
			
		if (freeing)
			next = &((*lst)->ypml_next);

		if (! xdr_reference(xdrs, lst, (u_int) sizeof(struct ypmaplist),
		    xdr_ypmaplist_wrap_string))
			return (FALSE);
			
		lst = (freeing) ? next : &((*lst)->ypml_next);
	}
}


/*
 * Serializes/deserializes a ypresp_maplist.
 */
bool
xdr_ypresp_maplist(xdrs, ps)
	XDR * xdrs;
	struct ypresp_maplist *ps;

{
	return (xdr_u_int(xdrs, &ps->status) &&
	   xdr_ypmaplist(xdrs, &ps->list) );
}

/*
 * Serializes/deserializes a yppushresp_xfr structure.
 */
bool
xdr_yppushresp_xfr(xdrs, ps)
	XDR *xdrs;
	struct yppushresp_xfr *ps;
{
	return (xdr_u_int(xdrs, &ps->transid) &&
	    xdr_u_int(xdrs, &ps->status));
}


/*
 * Serializes/deserializes a ypreq_xfr structure.
 */
bool
xdr_ypreq_xfr(xdrs, ps)
	XDR * xdrs;
	struct ypreq_xfr *ps;
{
	return (xdr_ypmap_parms(xdrs, &ps->map_parms) &&
	    xdr_u_int(xdrs, &ps->transid) &&
	    xdr_u_int(xdrs, &ps->proto) &&
	    xdr_u_short(xdrs, &ps->port) );
}


/*
 * Serializes/deserializes a stream of struct ypresp_key_val's.  This is used
 * only by the client side of the batch enumerate operation.
 */
bool
xdr_ypall(xdrs, callback)
	XDR * xdrs;
	struct ypall_callback *callback;
{
	bool more;
	struct ypresp_key_val kv;
	bool s;
	char keybuf[YPMAXRECORD];
	char valbuf[YPMAXRECORD];

	if (xdrs->x_op == XDR_ENCODE)
		return(FALSE);

	if (xdrs->x_op == XDR_FREE)
		return(TRUE);

	kv.keydat.dptr = keybuf;
	kv.valdat.dptr = valbuf;
	kv.keydat.dsize = YPMAXRECORD;
	kv.valdat.dsize = YPMAXRECORD;
	
	for (;;) {
		if (! xdr_bool(xdrs, &more) )
			return (FALSE);
			
		if (! more)
			return (TRUE);

		s = xdr_ypresp_key_val(xdrs, &kv);
		
		if (s) {
			s = (*callback->foreach)(kv.status, kv.keydat.dptr,
			    kv.keydat.dsize, kv.valdat.dptr, kv.valdat.dsize,
			    callback->data);
			
			if (s)
				return (TRUE);
		} else {
			return (FALSE);
		}
	}
}


/*
 * Serializes/deserializes a ypbind_setdom structure.
 */
bool
xdr_ypbind_setdom(xdrs, ps)
	XDR *xdrs;
	struct ypbind_setdom *ps;
{
	char *domain = ps->ypsetdom_domain;
	
	return (xdr_ypdomain_wrap_string(xdrs, &domain) &&
	    xdr_yp_binding(xdrs, &ps->ypsetdom_binding) &&
	    xdr_u_short(xdrs, &ps->ypsetdom_vers));
}
