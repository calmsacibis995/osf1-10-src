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
 * derived from cisysap.h	4.1	(ULTRIX)	7/2/90
 */
/*
 *
 *   Facility:	Systems Communication Architecture
 *		Computer Interconnect Port Driver
 *
 *   Abstract:	This module contains Computer Interconnect Port Driver( CI )
 *		constants and data structure definitions visible to SYSAPs.
 *
 *   Creator:	Todd M. Katz	Creation Date:	April 22, 1985
 *
 *   Modification History:
 *
 *   19-Sep-1989	Pete Keilty
 *	Added CI port info. port_fcn_ext2.
 *
 *   18-Jan-1989	Todd M. Katz		TMK0003
 *	Add padding when it is necessary to keep longword alignment.  While
 *	some space is wasted such alignment is essential for ports of SCA to
 *	hardware platforms which require field alignment and access type to
 *	match( ie- only longword aligned entities may be longword accessed ).
 *
 *   03-May-1988	Todd M. Katz		TMK0002
 *	Rename ram_level -> fn_level within structure ucode_rev of the CIPIB.
 *
 *   08-Jan-1988	Todd M. Katz		TMK0001
 *	Formated module, revised comments, increased generality and
 *	robustness, made CI PPD and GVP completely independent from underlying
 *	port drivers, and added SMP support.
 */
#ifndef _CISYSAP_H_
#define _CISYSAP_H_

/* CI Constants.
 */
#define	CI_NLOG			16	/* Number of CI port logout entries  */

/* CI Data Structure Definitions.
 */
typedef	struct _cilpib	{		/* CI Local Port Information	     */
    u_char	rpslogmap[ CIPPD_MAPSIZE ];/* Remote port state port log map */
} CILPIB;

typedef struct _cipib	{		/* CI Path Information		     */
    struct	{			/* Remote port microcode level	     */
	u_int	rom_level	:  8;	/*  PROM/Self-test ucode rev level   */
	u_int	fn_level	:  8;	/*  Functional ucode revision level  */
	u_int			: 16;	
    } ucode_rev;
    u_int	  port_fcn;		/* Remote port functionality mask    */
    u_int	  port_fcn_ext;		/* Rem port functionality extension  */
    u_int	  port_fcn_ext2;	/* Rem port functionality extension 2*/
    u_char	  rport_state;		/* Remote port state		     */
#define	PS_UNINIT		0	/*  Uninitialized		     */
#define	PS_UNINIT_MAINT		1	/*  Maintenance/Uninitialized	     */
#define	PS_DISAB		2	/*  Disabled			     */
#define	PS_DISAB_MAINT		3	/*  Maintenance/Disabled	     */
#define	PS_ENAB			4	/*  Enabled			     */
#define	PS_ENAB_MAINT		5	/*  Maintenance/Enabled		     */
    u_char	  reset_port;		/* Remote port's resetting port      */
    u_short			: 16;
} CIPIB;

#endif
