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
 *	@(#)$RCSfile: kernel_loader.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 23:38:25 $
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

extern int
kernel_load(char *file_pathname, ldr_load_flags_t load_flags,
	    ldr_module_t *module);

extern int
kernel_unload(ldr_module_t module);

extern int
kernel_ldr_entry(ldr_module_t module, ldr_entry_pt_t *entry);

extern int
kernel_ldr_lookup(ldr_module_t module, char* symbol_name,
		  void *symbol_addr);

extern int
kernel_ldr_lookup_package(char *package_name, char *symbol_name,
			  void *symbol_addr);

extern int
kernel_next_module(ldr_module_t *module);

extern int
kernel_inq_module(ldr_module_t module, ldr_module_info_t *info,
		  size_t info_size, size_t *ret_size);

extern int
kernel_inq_region(ldr_module_t module, ldr_region_t region,
		  ldr_region_info_t *info, size_t info_size,
		  size_t *ret_size); 
