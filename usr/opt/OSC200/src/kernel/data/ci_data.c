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
 *	@(#)$RCSfile: ci_data.c,v $ $Revision: 1.2.3.7 $ (DEC) $Date: 1992/06/04 17:12:16 $
 */ 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * Mach Operating System
 * Copyright (c) 1989 Carnegie-Mellon University
 * All rights reserved.  The CMU software License Agreement specifies
 * the terms and conditions for use and redistribution.
 */
/*
 * OSF/1 Release 1.0
 */
/* 
 * derived from ci_data.c	3.1	(ULTRIX)	4/20/90";
 */

/*
 *
 *   Facility:	Systems Communication Architecture
 *		Computer Interconnect Port Driver
 *
 *   Abstract:	This module contains Computer Interconnect Port Driver( CI )
 *		configurable variables and variables required by installation
 *		procedures.
 *
 *   Creator:	Todd M. Katz	Creation Date:	February 25, 1985
 *
 *   Modification History:
 *
 *   18-Sep-1989 	Pete Keilty
 *	Add errlogging variable ci_errlog.
 * 
 *   30-Aug-1988	Todd M. Katz
 *	Add two former CI PPD configuration variables ci_cippdburst and
 *	ci_cippdcontact.  They are now specified by the CI port driver instead
 *	of by the CI PPD because the optimum values for port polling burst size
 *	and contact frequency now differ on an individual port driver basis.
 *	The values for these parameters are passed to the CI PPD through CI PPD
 *	specific PCCB fields.
 *
 *   13-Apr-1988	Todd M. Katz
 *	Add the configuration variable ci_lpc_panic.  This configuration
 *	variable controls whether CI local port failures are recovered from or
 *	immediately panic the system.  Error recovery is the default but may be
 *	overruled by setting the configuration file option SCA_PANIC to an
 *	appropriate value.
 *
 *   08-Jan-1988	Todd M. Katz
 *	Formated module, revised comments, and made CI PPD completely
 *	independent from underlying port drivers.
 */

/* Libraries and Include Files.
 */
#include		<hsc.h>
#include		<sys/types.h>
#include		<io/common/devdriver.h>
#include		<io/dec/scs/scaparam.h>
 
/* CI Configuration Variables.
 */ 
u_short		ci_cippdburst		/* CI PPD polling burst size	     */
		    = 4;		/*  MAX:   224; DEF:	  4; MIN:  1 */
u_short		ci_cippdcontact		/* CI PPD port contact intrvl( secs )*/
		    = 60;		/*  MAX: 32767; DEF:     60; MIN:  2 */
u_short		ci_max_reinits		/* CI max num consecutive reinits    */
		    = 10;		/*  MAX:   255; DEF:     10; MIN:  1 */
u_short		ci_maint_timer		/* CI port mnt timer enable bool flag*/
		    = 1;		/*  MAX:     1; DEF:      1; MIN:  0 */
u_short		ci_maint_intrvl		/* CI port maintenance timer interval*/
		    = 0;		/*  MAX:   100; DEF:      0; MIN:  0 */
u_short		ci_lpc_panic		/* CI local port crash panic flag    */
		    = SCA_PANIC;	/*  MAX:     3; DEF:      0; MIN:  0 */
u_short		ci_severity		/* CI console logging severity	     */
		    = SCA_SEVERITY;	/*  MAX:     5; DEF:	  0; MIN:  0 */
u_short		ci_errlog		/* CI errlog logging severity	     */
		    = SCA_ERRLOG0;	/*  MAX:     3; DEF:	  0; MIN:  0 */

/* CI Variables for Installation Procedures.
 */
u_short		ci_first_port = 0;	/* Port number of first local CI port*/


#if NHSC > 0
extern struct device 	 *mscpdinfo[];
struct controller *hscinfo[NHSC];
struct driver hscdriver = { 0, 0, 0, 0, 0, 0, "ra", mscpdinfo, "hsc",
			     hscinfo, 0, 0, 0, 0, 0 };
#else
struct controller *hscinfo[1];
#endif

