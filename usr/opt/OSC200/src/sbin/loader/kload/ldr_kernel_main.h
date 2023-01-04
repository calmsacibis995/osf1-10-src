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
 *	@(#)$RCSfile: ldr_kernel_main.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 23:39:25 $
 */ 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * ldr_kernel_main.h
 *
 * OSF/1 Release 1.0
 */

extern ldr_context_t ldr_kernel_context;  /* the kernel context */

/* Bootstrap the loader system.  Includes building the loader process
 * context, containing the module and region records and exported symbol
 * list for the loader itself.  The loader_name argument is the name of
 * the loader's object module, for building the module record.
 */

extern int ldr_kernel_bootstrap(char *);

/* Absolute region allocator for regions to be loaded into a process.  This is
 * the "default" absolute region allocator used in initializing the process
 * loader context.
 */

extern int alloc_abs_kernel_region __((univ_t vaddr, size_t size,
				       ldr_prot_t prot, univ_t *baseaddr));

/* Relocatable region allocator for regions to be loaded into a process.  This is
 * the "default" relocatable region allocator used in initializing the process
 * loader context.
 */

extern int alloc_rel_kernel_region __((size_t size, ldr_prot_t prot,
				       univ_t *vaddr, univ_t *baseaddr));

/* Region deallocator for regions loaded into a process.  This is
 * the "default" region deallocator used in initializing the process loader
 * context.
 */

extern int dealloc_kernel_region __((univ_t vaddr, univ_t mapaddr, size_t size));
