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
 *	@(#)$RCSfile: counters.h,v $ $Revision: 1.2 $ (DEC) $Date: 1992/01/15 01:08:06 $
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

#if	COUNTERS
unsigned counters[30];

#define vtp_count counters[0]		/* virt_to_pmap */
#define mod_count counters[1]		/* PMAP_SET_MODIFY */
#define mpp_count counters[2]		/* get_more_pte_pages */
#define ptf_count counters[3]		/* pmap_pte_fault */
#define pgc_count counters[4]		/* pte_page_gc */
#define bmp_count counters[5]		/* pmap_map */
#define pcr_count counters[6]		/* pmap_create */
#define pds_count counters[7]		/* pmap_destroy */
#define rrg_count counters[8]		/* pmap_remove_range */
#define ral_count counters[9]		/* pmap_remove_all */
#define cow_count counters[10]		/* pmap_copy_on_write */
#define ppt_count counters[11]		/* pmap_protect */
#define ent_count counters[12]		/* pmap_enter */
#define wir_count counters[13]		/* pmap_change_wiring */
#define ext_count counters[14]		/* pmap_extract */
#define acc_count counters[15]		/* pmap_access */
#define zer_count counters[16]		/* pmap_zero_page */
#define cop_count counters[17]		/* pmap_copy_page */
#define prt_count counters[18]		/* pmap_page_protect */
#define smd_count counters[19]		/* pmap_set_modify */
#define clr_count counters[20]		/* pmap_clear_modify */
#define imd_count counters[21]		/* pmap_is_modified */
#define crr_count counters[22]		/* pmap_clear_reference */
#define irr_count counters[23]		/* pmap_is_referenced */
#define tph_count counters[24]		/* pmap_copy_to_phys */
#define tpa_count counters[25]		/* assign_tlbpid */
#define tpf_count counters[26]		/* destroy_tlbpid */
#define ifl_count counters[27]		/* mipscache_Iflush */
#define ifn_count counters[28]		/* Iflush needed */
#define xx3_count counters[29]

#define INC_VTP_COUNT vtp_count++
#define INC_MOD_COUNT mod_count++
#define INC_MPP_COUNT mpp_count++
#define INC_PTF_COUNT ptf_count++
#define INC_PGC_COUNT pgc_count++
#define INC_BMP_COUNT bmp_count++
#define INC_PCR_COUNT pcr_count++
#define INC_PDS_COUNT pds_count++
#define INC_RRG_COUNT rrg_count++
#define INC_RAL_COUNT ral_count++
#define INC_COW_COUNT cow_count++
#define INC_PPT_COUNT ppt_count++
#define INC_ENT_COUNT ent_count++
#define INC_WIR_COUNT wir_count++
#define INC_EXT_COUNT ext_count++
#define INC_ACC_COUNT acc_count++
#define INC_ZER_COUNT zer_count++
#define INC_COP_COUNT cop_count++
#define INC_PRT_COUNT prt_count++
#define INC_SMD_COUNT smd_count++
#define INC_CLR_COUNT clr_count++
#define INC_IMD_COUNT imd_count++
#define INC_CRR_COUNT crr_count++
#define INC_IRR_COUNT irr_count++
#define INC_TPH_COUNT tph_count++
#define INC_TPA_COUNT tpa_count++
#define INC_TPF_COUNT tpf_count++
#define INC_IFL_COUNT ifl_count++
#define INC_IFN_COUNT ifn_count++
#define INC_XX3_COUNT xx3_count++

#else /*   COUNTERS */

#define INC_VTP_COUNT
#define INC_MOD_COUNT
#define INC_MPP_COUNT
#define INC_PTF_COUNT
#define INC_PGC_COUNT
#define INC_BMP_COUNT
#define INC_PCR_COUNT
#define INC_PDS_COUNT
#define INC_RRG_COUNT
#define INC_RAL_COUNT
#define INC_COW_COUNT
#define INC_PPT_COUNT
#define INC_ENT_COUNT
#define INC_WIR_COUNT
#define INC_EXT_COUNT
#define INC_ACC_COUNT
#define INC_ZER_COUNT
#define INC_COP_COUNT
#define INC_PRT_COUNT
#define INC_SMD_COUNT
#define INC_CLR_COUNT
#define INC_IMD_COUNT
#define INC_CRR_COUNT
#define INC_IRR_COUNT
#define INC_TPH_COUNT
#define INC_TPA_COUNT
#define INC_TPF_COUNT
#define INC_IFL_COUNT
#define INC_IFN_COUNT
#define INC_XX3_COUNT

#endif	/* COUNTERS */

