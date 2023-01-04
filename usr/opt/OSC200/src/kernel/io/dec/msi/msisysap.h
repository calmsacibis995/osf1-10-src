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
 * derived from msisysap.h	2.1	(ULTRIX)	5/9/89
 */
/*
 *   Facility:	Systems Communication Architecture
 *		Mayfair Storage Interconnect Port Driver
 *
 *   Abstract:	This module contains Mayfair Storage Interconnect Port
 *		Driver( MSI ) constants and data structure definitions
 *		visible to SYSAPs.
 *
 *   Creator:	Todd M. Katz	Creation Date:	December 06, 1988
 *
 *   Modification History:
 */

#ifndef _MSISYSAP_H_
#define _MSISYSAP_H_

/* MSI Constants.
 */
#define	MSI_MAXNUM_PORT		 8	/*  DSSI( ports )		     */
#define	MSI_MAPSIZE ( MSI_MAXNUM_PORT / 8 ) /* Size of port bit map in bytes */

/* MSI Data Structure Definitions.
 */
typedef	struct _msilpib	{		/* MSI Local Port Information	     */
    u_int	dg_size;		/* Size of application datagram	     */
    u_int	msg_size;		/* Size of application sequenced msg */
    u_int	pd_ovhd;		/* Size of PD + PPD header overhead  */
    u_int	ppd_ovhd;		/* Size of PPD header overhead	     */
    u_char	rpslogmap[ MSI_MAPSIZE ];/* Remote port state port logmap    */
    u_int			: 24;
} MSILPIB;

typedef	struct	_msirpi {		/* MSI Remote Port Information	     */
    struct	{			/* Remote port microcode level	     */
	u_char	st_level;		/*  Self-test ucode rev level	     */
	u_char	fn_level;		/*  Functional ucode revision level  */
	u_short			: 16;	
    } ucode_rev;
    u_short	port_fcn[ 2 ];		/* Port functionality mask	     */
    struct	{			/* System state information	     */
	u_short	reset_port	:  8;	/*  Port which caused last reset     */
	u_short	port_state	:  3;	/*  Port state( defined by CIPIB )   */
			       /* PS_UNINIT	  - Uninitialized	     */
			       /* PS_UNINIT_MAINT - Maintenance/Uninitialized*/
			       /* PS_DISAB	  - Disabled		     */
			       /* PS_DISAB_MAINT  - Maintenance/Disabled     */
			       /* PS_ENAB	  - Enabled		     */
			       /* PS_ENAB_MAINT	  - Maintenance/Enabled	     */
	u_short	sys_state1	:  5;	/*  Implementation specific state    */
	u_short	sys_state2;
    } sys_state;
    struct	{			/* Port functionality extension	     */
	u_short			: 16;
	u_short	maxbodylen	: 13;	/*  Max num of bytes in packet body  */
	u_short			:  3;
    } port_fcn_ext;
} MSIRPI;

typedef struct _msipib	{		/* MSI Path Information		     */
    struct _msirpi	rpinfo;		/* Remote port information	     */
} MSIPIB;

#endif
