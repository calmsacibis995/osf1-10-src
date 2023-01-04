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
 *	"@(#)counters.h	9.1	(ULTRIX/OSF)	10/21/91"
 */ 
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
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0
 */

#ifndef _COUNTERS_H_
#define _COUNTERS_H_

#if	COUNTERS
struct {
unsigned long	mod_count,
		pcr_count,
		ext_count,
		acc_count,
		prt_count,
		smd_count,
		clr_count,
		imd_count,
		crr_count,
		irr_count;
		vtn_count;
		vtm_count;
		vth_count;
		zer_count;
		cop_count;
		mpp_count;
} vm_counter;

#define INC_MOD_COUNT (vm_counter.mod_count++)
#define INC_PCR_COUNT (vm_counter.pcr_count++)
#define INC_EXT_COUNT (vm_counter.ext_count++)
#define INC_ACC_COUNT (vm_counter.acc_count++)
#define INC_PRT_COUNT (vm_counter.prt_count++)
#define INC_SMD_COUNT (vm_counter.smd_count++)
#define INC_CLR_COUNT (vm_counter.clr_count++)
#define INC_IMD_COUNT (vm_counter.imd_count++)
#define INC_CRR_COUNT (vm_counter.crr_count++)
#define INC_IRR_COUNT (vm_counter.irr_count++)
#define INC_VTN_COUNT (vm_counter.vtn_count++)
#define INC_VTM_COUNT (vm_counter.vtm_count++)
#define INC_VTH_COUNT (vm_counter.vth_count++)
#define INC_ZER_COUNT (vm_counter.zer_count++)
#define INC_COP_COUNT (vm_counter.cop_count++)
#define INC_MPP_COUNT (vm_counter.cop_count++)

#else   /*COUNTERS*/

#define INC_MOD_COUNT 0
#define INC_PCR_COUNT 0
#define INC_EXT_COUNT 0
#define INC_ACC_COUNT 0
#define INC_PRT_COUNT 0
#define INC_SMD_COUNT 0
#define INC_CLR_COUNT 0
#define INC_IMD_COUNT 0
#define INC_CRR_COUNT 0
#define INC_IRR_COUNT 0
#define INC_VTN_COUNT 0
#define INC_VTM_COUNT 0
#define INC_VTH_COUNT 0
#define INC_ZER_COUNT 0
#define INC_COP_COUNT 0
#define INC_MPP_COUNT 0

#endif	/*COUNTERS*/

#endif
