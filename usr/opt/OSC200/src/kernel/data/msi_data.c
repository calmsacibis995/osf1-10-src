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
static char *rcsid = "@(#)$RCSfile: msi_data.c,v $ $Revision: 1.1.3.4 $ (DEC) $Date: 1992/06/04 17:12:56 $";
#endif
/*
 * derived from msi_data.c	4.1  (ULTRIX)        7/2/90";
 */
/*
 *
 *   Facility:	Systems Communication Architecture
 *		Mayfair Storage Interconnect Port Driver
 *
 *   Abstract:	This module contains Mayfair Storage Interconnect Port
 *		Driver( MSI ) configurable variables.
 *
 *   Creator:	Todd M. Katz	Creation Date:	December 07, 1988
 *
 *   Modification History:
 */

/* Libraries and Include Files
 */
#include		<dssc.h>
#include		<sys/types.h>
#include		<io/common/devdriver.h>
#include		<io/dec/scs/scaparam.h>


/* MSI Configuration Variables
 */
u_short		msi_cippdburst		/* CI PPD port polling burst size    */
		    = 8;		/*  MAX:    8; DEF:	  8; MIN:  1 */
u_short		msi_cippdcontct		/* CI PPD port contact intrvl( secs )*/
		    = 5;		/*  MAX: 32767; DEF:      5; MIN:  2 */
u_short		msi_lpc_panic		/* MSI local port crash panic flag   */
		    = SCA_PANIC;	/*  MAX:     3; DEF:      0; MIN:  0 */
u_short		msi_severity		/* MSI console logging severity	     */
		    = SCA_SEVERITY;	/*  MAX:     5; DEF:	  4; MIN:  0 */

extern struct device 	 *mscpdinfo[];
#if NDSSC > 0
struct controller *dsscinfo[NDSSC];
struct driver dsscdriver = { 0, 0, 0, 0, 0, 0, "ra", mscpdinfo, "dssc",
			     dsscinfo, 0, 0, 0, 0, 0 };
#else
struct controller *dsscinfo[1];
#endif

