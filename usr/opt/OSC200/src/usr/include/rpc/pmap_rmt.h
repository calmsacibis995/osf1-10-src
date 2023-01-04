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
 *	@(#)$RCSfile: pmap_rmt.h,v $ $Revision: 4.2.2.2 $ (DEC) $Date: 1992/01/23 16:05:18 $
 */
/*
 */


/* 
 * Copyright (c) 1990 by Sun Microsystems, Inc.
 *	1.2 88/02/08 SMI	
 */


/*
 * Structures and XDR routines for parameters to and replies from
 * the portmapper remote-call-service.
 */

#ifndef _rpc_pmap_rmt_h
#define	_rpc_pmap_rmt_h

struct rmtcallargs {
	u_int prog, vers, proc, arglen;
	caddr_t args_ptr;
	xdrproc_t xdr_args;
};

bool_t xdr_rmtcall_args();

struct rmtcallres {
	u_int *port_ptr;
	u_int resultslen;
	caddr_t results_ptr;
	xdrproc_t xdr_results;
};

bool_t xdr_rmtcallres();

#endif /*!_rpc_pmap_rmt_h*/
