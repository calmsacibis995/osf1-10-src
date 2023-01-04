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
 *	@(#)gvpsysap.h	4.1	(ULTRIX)	7/2/90
 */
/*
 *
 *   Facility:	Systems Communication Architecture
 *		Generic Vaxport Port Driver
 *
 *   Abstract:	This module contains Generic Vaxport Port Driver( GVP )
 *		data structure definitions visible to SYSAPs.
 *
 *   Creator:	Todd M. Katz	Creation Date:	November 20, 1985
 *
 *   Modification History:
 *
 *   19-Sep-1989 	Pete Keilty
 *	Added ovhd_pd to local port info. block (gvplpib.ovhd_pd).
 *
 *   02-Jun-1988	Ricky S. Palmer
 *	Removed struct entries for msi altogether.
 *
 *   29-Jan-1988        Ricky S. Palmer
 *      Added struct entries for msi in both the Local Port Information Block
 *      and the Path Information Block definitions.
 *
 *   08-Jan-1988	Todd M. Katz
 *	Formated module, revised comments, increased robustness, made GVP
 *	completely independent from underlying port drivers, restructured code
 *	paths, and added SMP support.
 */

#ifndef _GVPSYSAP_H_
#define _GVPSYSAP_H_

/* Generic Vaxport Data Structure Definitions.
 */
typedef struct _gvpbname	{	/* Generic Vaxport Buffer Name	     */
    u_short	index;			/* Index into buffer descriptor array*/
#define ni_chain	0x00008000	/*  NI buffer chaining mask	     */
    u_short	key;			/* Key within buffer descriptor      */
} GVPBNAME;

typedef struct	_gvpbhandle {		/* Generic Vaxport Buffer Handle     */
    u_int	     boff;		/* Transfer offset of buffer	     */
    struct _gvpbname bname;		/* Generic Vaxport buffer name	     */
} GVPBHANDLE;

typedef struct _gvplpib {		/* Generic Vaxport Local Port	     */
					/*  Information			     */
    u_int	dg_size;		/* Size of application datagram      */
    u_int	msg_size;		/* Size of application message	     */
    u_short	ovhd_pd;		/* Size of PD overhead 		     */
    u_short	ovhd;			/* Size of PD + PPD overhead	     */
    union		    {		/* Implementation dependent fields   */
	struct _bvp_ssplpib bvp;	/*  BVP SSP specific information     */
	struct _cilpib	    ci;		/*  CI specific information	     */
    } type;
} GVPLPIB;

typedef struct _gvppib	{		/* Generic Vaxport Path Information  */
    union		{		/* Implementation dependent fields   */
	struct _cipib	ci;		/*  CI path information		     */
    } type;
} GVPPIB;

#endif
