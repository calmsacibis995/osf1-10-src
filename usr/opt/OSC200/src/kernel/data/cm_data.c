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
 * This is a list of the compatability and habitat modules that
 * may be statically linked into the kernel. 
 */

#include <sys/systm.h>
#include <sys/habitat.h>
#include "ult_bin.h"
#include "sysv_hab.h"
#include "svid_three_hab.h"
#include "svr_four_hab.h"
#include "soe_two_hab.h"


#if NULT_BIN
extern void ult_cfg_static();
#endif
#if NSYSV_HAB
extern void sysv_cfg_static();
#endif
#if NSVID_THREE_HAB
extern void svid_three_cfg_static();
#endif
#if NSVR_FOUR_HAB
extern void svr_four_cfg_static();
#endif
#if NSOE_TWO_HAB
extern void soe_two_cfg_static();
#endif
extern void rt_cfg_static();


void (* cm_static[MAXSTATICMODS])() = {

#if NULT_BIN
	ult_cfg_static,
#endif
#if NSYSV_HAB
	sysv_cfg_static,
#endif
#if NSVID_THREE_HAB
	svid_three_cfg_static,
#endif
#if NSVR_FOUR_HAB
	svr_four_cfg_static,
#endif
#if NSOE_TWO_HAB
	soe_two_cfg_static,
#endif
	/* real-time is always static */
	rt_cfg_static,
	0,
};



/*
 * List of valid modules/habitats and revisions
 *	Add to the list as the need arises
 */
struct cm_valid cm_valid[] = {
	ULTBIN,		ULT42V11,	/* Ultrix 4.2 compat module	*/
	SYSV_HAB, 	SYSVV11,	/* System V habitat		*/
	SVID3_HAB,	SVID3V11,	/* Svid 3 habitat		*/
	SVR4_HAB,	SVR4V11,	/* Svr 4 habitat		*/
	SOE2_HAB,	SOE2V11,	/* Soe 2 habitat		*/
	RT_HAB,		RTV11,		/* Real-time habitat		*/
	   0,	  	   0
};



/*
 * Compatability module support.
 * 	Control diagnostics
 */
int bin_compat_debug 	= 0;
int bin_compat_trace	= 0;

