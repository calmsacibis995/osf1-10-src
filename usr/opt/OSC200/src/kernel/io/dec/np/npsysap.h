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
 *	@(#)npsysap.h	5.1	(ULTRIX)	8/5/91
*/

/*
 *
 *   Facility:	Systems Communication Architecture
 *		Computer Interconnect N_PORT Port Driver
 *
 *   Abstract:	This module contains Computer Interconnect N_PORT Port Driver
 *		constants and data structure definitions visible to SYSAPs.
 *
 *   Creator:	Peter Keilty	Creation Date:	July 1, 1991
 *	        This file derived from Todd Katz CI port driver.
 *
 *   Modification History:
 *
 *   31-Oct-1991	Peter Keilty
 *	Ported to OFS/1
 *
 */

#ifndef _NPSYSAP_H_
#define _NPSYSAP_H_

/* CI Constants.
 */

/* N_PORT CI Data Structure Definitions.
 */
typedef struct _npbname {		/* Buffer Name structure	     */
    u_int    bdl_idx	: 8;		/* Index into buffer descriptor leaf */
    u_int    bdlt_idx	:12;		/* Index into BDLT		     */
    u_int    key	:12;		/* Buffer key - matched with command */
					/* ( sequence number )		     */
} NPBNAME;

typedef struct _npbhandle {		/* N_PORT Buffer Handle		     */
    u_int	    boff;		/* Transfer offset of buffer         */
    NPBNAME	    bname;		/* N_PORT buffer name		     */
} NPBHANDLE;

typedef	struct _nplpib	{		/* N_PORT CI Local Port Information  */
    u_int      dg_size;                 /* Size of application datagram      */
    u_int      msg_size;                /* Size of application message       */
    u_short     ovhd_pd;                /* Size of PD overhead 		     */
    u_short     ovhd;                   /* Size of PD + PPD overhead 	     */
    u_char	rpslogmap[ CIPPD_MAPSIZE ];/* Remote port state port log map */
} NPLPIB;

typedef struct _nppib	{		/* CI Path Information		     */
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
} NPPIB;

#endif
