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
static char *rcsid = "@(#)$RCSfile: np_data.c,v $ $Revision: 1.1.3.5 $ (DEC) $Date: 1992/06/04 17:13:07 $";
#endif
/*
 * derived from np_data.c	5.1	(ULTRIX)	8/5/91";
 */
/*
 *
 *   Facility:	Systems Communication Architecture
 *		N_PORT Computer Interconnect Port Driver
 *
 *   Abstract:	This module contains Computer Interconnect Port Driver( CI )
 *		configurable variables and variables required by installation
 *		procedures.
 *
 *   Creator:	Pete Keilty 	Creation Date:	May 25, 1991
 *
 *   Modification History:
 *
 */

/* Libraries and Include Files.
 */
#include		<sys/types.h>
#include		<io/dec/scs/scaparam.h>

/* CI Configuration Variables.
 */
u_int		np_nci_supported	/* Number of N_PORT CI's supported   */
		    = 8;		/*  MAX:   16; DEF:	  8; MIN:  1 */
u_short		np_cippdburst		/* CI PPD polling burst size	     */
		    = 8;		/*  MAX:   224; DEF:	  4; MIN:  1 */
u_short		np_cippdcontact		/* CI PPD port contact intrvl( secs )*/
		    = 30;		/*  MAX: 32767; DEF:     60; MIN:  2 */
u_short		np_max_reinits		/* N_PORT max num consecutive reinits*/
		    = 10;		/*  MAX:   255; DEF:     10; MIN:  1 */
u_short		np_maint_timer		/* N_PORT maint timer enable  flag   */
		    = 1;		/*  MAX:     1; DEF:      1; MIN:  0 */
u_short		np_nosanity_chk		/* CI PPD port no sanity enable flag */
		    = 0;		/*  MAX:     1; DEF:      0; MIN:  0 */
u_short		np_maint_intrvl		/* N_PORT maintenance timer interval */
		    = 0;		/*  MAX:   100; DEF:      0; MIN:  0 */
u_int		np_intr_holdoff		/* N_PORT intr holdoff	interval     */
		    = 0;		/*  MAX:   0; DEF:    0; MIN:  0     */
u_short		np_lpc_panic		/* CI local port crash panic flag    */
		    = SCA_PANIC;	/*  MAX:     3; DEF:      0; MIN:  0 */
u_short		np_severity		/* CI console logging severity	     */
		    = SCA_SEVERITY;	/*  MAX:     5; DEF:	  0; MIN:  0 */
u_short		np_errlog		/* CI errlog logging severity	     */
		    = SCA_ERRLOG0;	/*  MAX:     3; DEF:	  0; MIN:  0 */

/* CI Variables for Installation Procedures.
 */
u_short		np_first_port = 0;	/* Port number of first local CI port*/

npinit() { /* temp stub */ }
