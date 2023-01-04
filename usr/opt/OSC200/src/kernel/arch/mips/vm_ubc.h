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
 * @(#)$RCSfile: vm_ubc.h,v $ $Revision: 1.1.3.4 $ (DEC) $Date: 1992/05/05 12:19:02 $
 */
#ifndef	__VM_UBC__
#define	__VM_UBC__ 1
#include <machine/cpu.h>
#include <ref_bits.h>
#include <vm/vm_debug.h>

#define ubc_kmem_alloc(PP) (PP)->pg_addr = PHYS_TO_K0(page_to_phys(PP))
#define ubc_kmem_free(PP)
#define ubc_kmem_cache(PP) FALSE
#define	ubc_load(PP, OFFSET, SIZE) (PP)->pg_addr
#define	ubc_unload(PP, OFFSET, SIZE)
#define	ubc_page_zero(PP, O, S)						\
	bzero(PHYS_TO_K0(page_to_phys((PP))) + (O) , (S))

#define	ubc_page_referenced(PP)						\
	pmap_is_referenced(page_to_phys(PP))
#define	ubc_page_clear_reference(PP) 					\
	pmap_clear_reference(page_to_phys(PP))


#if	UBC_PAGE_CHECK

#undef	ubc_load
#undef	ubc_page_zero
#define ubc_load(PP, OFFSET, SIZE) 					\
	ubc_load_page_check(PP, OFFSET, SIZE)
#define	ubc_page_zero(PP, O, S) 					\
	ubc_page_zero_page_check(PP, O, S)

#endif	/* UBC_PAGE_CHECK */

#endif	/* !__VM_UBC__ */
