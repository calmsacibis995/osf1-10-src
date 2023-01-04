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
/*
 *	@(#)$RCSfile: pmap_prot.h,v $ $Revision: 4.2.2.2 $ (DEC) $Date: 1992/01/23 16:00:51 $
 */
/*
 */


/* 
 * Copyright (c) 1990 by Sun Microsystems, Inc.
 * 1.14 88/02/08 SMI	
 */


/*
 * pmap_prot.h
 * Protocol for the local binder service, or pmap.
 *
 *
 * The following procedures are supported by the protocol:
 *
 * PMAPPROC_NULL() returns ()
 * 	takes nothing, returns nothing
 *
 * PMAPPROC_SET(struct pmap) returns (bool_t)
 * 	TRUE is success, FALSE is failure.  Registers the tuple
 *	[prog, vers, prot, port].
 *
 * PMAPPROC_UNSET(struct pmap) returns (bool_t)
 *	TRUE is success, FALSE is failure.  Un-registers pair
 *	[prog, vers].  prot and port are ignored.
 *
 * PMAPPROC_GETPORT(struct pmap) returns (int unsigned).
 *	0 is failure.  Otherwise returns the port number where the pair
 *	[prog, vers] is registered.  It may lie!
 *
 * PMAPPROC_DUMP() RETURNS (struct pmaplist *)
 *
 * PMAPPROC_CALLIT(unsigned, unsigned, unsigned, string<>)
 * 	RETURNS (port, string<>);
 * usage: encapsulatedresults = PMAPPROC_CALLIT(prog, vers, proc, encapsulatedargs);
 * 	Calls the procedure on the local machine.  If it is not registered,
 *	this procedure is quite; ie it does not return error information!!!
 *	This procedure only is supported on rpc/udp and calls via
 *	rpc/udp.  This routine only passes null authentication parameters.
 *	This file has no interface to xdr routines for PMAPPROC_CALLIT.
 *
 * The service supports remote procedure calls on udp/ip or tcp/ip socket 111.
 */

#ifndef _rpc_pmap_prot_h
#define	_rpc_pmap_prot_h
#define PMAPPORT		((u_short)111)
#define PMAPPROG		((u_int)100000)
#define PMAPVERS		((u_int)2)
#define PMAPVERS_PROTO		((u_int)2)
#define PMAPVERS_ORIG		((u_int)1)
#define PMAPPROC_NULL		((u_int)0)
#define PMAPPROC_SET		((u_int)1)
#define PMAPPROC_UNSET		((u_int)2)
#define PMAPPROC_GETPORT	((u_int)3)
#define PMAPPROC_DUMP		((u_int)4)
#define PMAPPROC_CALLIT		((u_int)5)

struct pmap {
	int unsigned pm_prog;
	int unsigned pm_vers;
	int unsigned pm_prot;
	int unsigned pm_port;
};

extern bool_t xdr_pmap();

struct pmaplist {
	struct pmap	pml_map;
	struct pmaplist *pml_next;
};

#ifndef KERNEL
extern bool_t xdr_pmaplist();
#endif /*!KERNEL*/

#endif /*!_rpc_pmap_prot_h*/
