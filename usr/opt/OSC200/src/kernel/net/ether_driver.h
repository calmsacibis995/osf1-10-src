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
 *	@(#)$RCSfile: ether_driver.h,v $ $Revision: 4.3.3.2 $ (DEC) $Date: 1993/06/29 17:18:38 $
 */ 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0
 */
/* 
 * derived from ether_driver.h	3.1  (ULTRIX)	4/20/90";
 */

/************************************************************************
 *									*
 * Ethernet software status per interface				*
 *	Common portion for all drivers					*
 *									*
 *			Modification History				*
 *									*
 * 18-Nov-88	- Jeffrey Mogul, DECWRL					*
 *	Created (loosely based on if_qe_data.c)				*
 *									*
 ************************************************************************/

#ifndef _ETHER_DRIVER_H_
#define _ETHER_DRIVER_H_
/*
 * Ethernet software status per interface;
 *	Common portion for all drivers (to be embedded in
 *	driver-specific status block).
 *
 * Each interface is referenced by a network interface structure,
 * ess_if, which the routing code uses to locate the interface.
 * This structure contains the output queue for the interface, its address, ...
 */
#define	ess_if	 ess_ac.ac_if		/* network-visible interface 	*/
#define	ess_addr ess_ac.ac_enaddr	/* hardware Ethernet address 	*/

struct	ether_driver {
	struct	arpcom ess_ac;		/* Ethernet common part 	*/
	struct	estat ess_ctrblk;	/* Counter block		*/
	int	ess_ztime;		/* Time counters last zeroed	*/
	short	ess_enetunit;		/* unit no. for packet filter	*/
	int	ess_missed;		/* count of missed rx packets	*/
};


#endif
