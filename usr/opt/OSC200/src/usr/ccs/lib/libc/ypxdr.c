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
static char     *sccsid = "@(#)$RCSfile: ypxdr.c,v $ $Revision: 4.2.7.2 $ (DEC) $Date: 1993/06/08 00:30:04 $";
#endif
/*
 */


/* 
 * Copyright (c) 1988,1990 by Sun Microsystems, Inc.
 * 1.14 87/08/12 
 */


/*
 * This contains xdr routines used by the NIS rpc interface.
 */

/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#if !defined(_THREAD_SAFE)
#pragma weak xdr_datum = __xdr_datum
#pragma weak xdr_yp_binding = __xdr_yp_binding
#pragma weak xdr_yp_inaddr = __xdr_yp_inaddr
#pragma weak xdr_ypbind_resp = __xdr_ypbind_resp
#pragma weak xdr_ypdomain_wrap_string = __xdr_ypdomain_wrap_string
#pragma weak xdr_ypmap_parms = __xdr_ypmap_parms
#pragma weak xdr_ypmap_wrap_string = __xdr_ypmap_wrap_string
#pragma weak xdr_ypowner_wrap_string = __xdr_ypowner_wrap_string
#pragma weak xdr_ypreq_key = __xdr_ypreq_key
#pragma weak xdr_ypreq_nokey = __xdr_ypreq_nokey
#pragma weak xdr_ypresp_key_val = __xdr_ypresp_key_val
#pragma weak xdr_ypresp_val = __xdr_ypresp_val
#pragma weak ypbind_resp_arms = __ypbind_resp_arms
#endif
#endif
#define NULL 0L
#include <rpc/rpc.h>
#include <rpcsvc/yp_prot.h>
#include <rpcsvc/ypclnt.h>

typedef struct xdr_discrim XDR_DISCRIM;
bool xdr_datum();
bool xdr_ypdomain_wrap_string();
bool xdr_ypmap_wrap_string();
bool xdr_ypreq_key();
bool xdr_ypreq_nokey();
bool xdr_ypresp_val();
bool xdr_ypresp_key_val();
bool xdr_ypbind_resp ();
bool xdr_yp_inaddr();
bool xdr_yp_binding();
bool xdr_ypmap_parms();
bool xdr_ypowner_wrap_string();
bool xdr_ypref();


/*
 * Serializes/deserializes a dbm datum data structure.
 */
bool
xdr_datum(xdrs, pdatum)
	XDR * xdrs;
	datum * pdatum;

{
	return (xdr_bytes(xdrs, &(pdatum->dptr), &(pdatum->dsize),
	    YPMAXRECORD));
}


/*
 * Serializes/deserializes a domain name string.  This is a "wrapper" for
 * xdr_string which knows about the maximum domain name size.  
 */
bool
xdr_ypdomain_wrap_string(xdrs, ppstring)
	XDR * xdrs;
	char **ppstring;
{
	return (xdr_string(xdrs, ppstring, YPMAXDOMAIN) );
}

/*
 * Serializes/deserializes a map name string.  This is a "wrapper" for
 * xdr_string which knows about the maximum map name size.  
 */
bool
xdr_ypmap_wrap_string(xdrs, ppstring)
	XDR * xdrs;
	char **ppstring;
{
	return (xdr_string(xdrs, ppstring, YPMAXMAP) );
}

/*
 * Serializes/deserializes a ypreq_key structure.
 */
bool
xdr_ypreq_key(xdrs, ps)
	XDR *xdrs;
	struct ypreq_key *ps;

{
	return (xdr_ypdomain_wrap_string(xdrs, &ps->domain) &&
	    xdr_ypmap_wrap_string(xdrs, &ps->map) &&
	    xdr_datum(xdrs, &ps->keydat) );
}

/*
 * Serializes/deserializes a ypreq_nokey structure.
 */
bool
xdr_ypreq_nokey(xdrs, ps)
	XDR * xdrs;
	struct ypreq_nokey *ps;
{
	return (xdr_ypdomain_wrap_string(xdrs, &ps->domain) &&
	    xdr_ypmap_wrap_string(xdrs, &ps->map) );
}

/*
 * Serializes/deserializes a ypresp_val structure.
 */

bool
xdr_ypresp_val(xdrs, ps)
	XDR * xdrs;
	struct ypresp_val *ps;
{
	return (xdr_u_int(xdrs, &ps->status) &&
	    xdr_datum(xdrs, &ps->valdat) );
}

/*
 * Serializes/deserializes a ypresp_key_val structure.
 */
bool
xdr_ypresp_key_val(xdrs, ps)
	XDR * xdrs;
	struct ypresp_key_val *ps;
{
	return (xdr_u_int(xdrs, &ps->status) &&
	    xdr_datum(xdrs, &ps->valdat) &&
	    xdr_datum(xdrs, &ps->keydat) );
}


/*
 * Serializes/deserializes an in_addr struct.
 * 
 * Note:  There is a data coupling between the "definition" of a struct
 * in_addr implicit in this xdr routine, and the true data definition in
 * <netinet/in.h>.  
 */
bool
xdr_yp_inaddr(xdrs, ps)
	XDR * xdrs;
	struct in_addr *ps;

{
	return (xdr_opaque(xdrs, &ps->s_addr, 4));
}

/*
 * Serializes/deserializes a ypbind_binding struct.
 */
bool
xdr_yp_binding(xdrs, ps)
	XDR * xdrs;
	struct ypbind_binding *ps;

{
	return (xdr_yp_inaddr(xdrs, &ps->ypbind_binding_addr) &&
            xdr_opaque(xdrs, &ps->ypbind_binding_port, 2));
}

/*
 * xdr discriminant/xdr_routine vector for yp binder responses
 */
XDR_DISCRIM ypbind_resp_arms[] = {
	{(int) YPBIND_SUCC_VAL, (xdrproc_t) xdr_yp_binding},
	{(int) YPBIND_FAIL_VAL, (xdrproc_t) xdr_u_int},
	{__dontcare__, (xdrproc_t) NULL}
};

/*
 * Serializes/deserializes a ypbind_resp structure.
 */
bool
xdr_ypbind_resp(xdrs, ps)
	XDR * xdrs;
	struct ypbind_resp *ps;

{
	return (xdr_union(xdrs, &ps->ypbind_status, &ps->ypbind_respbody,
	    ypbind_resp_arms, NULL) );
}

/*
 * Serializes/deserializes a peer server's node name
 */
bool
xdr_ypowner_wrap_string(xdrs, ppstring)
	XDR * xdrs;
	char **ppstring;

{
	return (xdr_string(xdrs, ppstring, YPMAXPEER) );
}

/*
 * Serializes/deserializes a ypmap_parms structure.
 */
bool
xdr_ypmap_parms(xdrs, ps)
	XDR *xdrs;
	struct ypmap_parms *ps;

{
	return (xdr_ypdomain_wrap_string(xdrs, &ps->domain) &&
	    xdr_ypmap_wrap_string(xdrs, &ps->map) &&
	    xdr_u_int(xdrs, &ps->ordernum) &&
	    xdr_ypowner_wrap_string(xdrs, &ps->owner) );
}
