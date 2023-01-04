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
static char	*sccsid = "@(#)$RCSfile: vm_init.c,v $ $Revision: 4.2.4.6 $ (DEC) $Date: 1992/11/06 15:41:19 $";
#endif 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/* 
 * Mach Operating System
 * Copyright (c) 1989 Carnegie-Mellon University
 * Copyright (c) 1988 Carnegie-Mellon University
 * Copyright (c) 1987 Carnegie-Mellon University
 * All rights reserved.  The CMU software License Agreement specifies
 * the terms and conditions for use and redistribution.
 */
/*
 * OSF/1 Release 1.0
 */
/*
 *	File:	vm/vm_init.c
 *	Author:	Avadis Tevanian, Jr., Michael Wayne Young
 *
 *	Copyright (C) 1985, Avadis Tevanian, Jr., Michael Wayne Young
 *
 *	Initialize the Virtual Memory subsystem.
 */

#include <sys/unix_defs.h>
#include <mach/machine/vm_types.h>
#include <mach/vm_param.h>
#include <kern/lock.h>
#include <vm/vm_map.h>
#include <vm/vm_object.h>
#include <vm/vm_page.h>
#include <vm/vm_kern.h>
#include <vm/vm_perf.h>

/*
 * Performance data gathering information.
 */

struct vm_perf vm_perf, vm_perfcomp, vm_perfsum;
udecl_simple_lock_data(, vm_perf_lock)

/*
 *	vm_init initializes the virtual memory system.
 *	This is done only by the first cpu up.
 *
 *	The start and end address of physical memory is passed in.
 */

void 
vm_mem_init()
{
	extern vm_offset_t avail_start, avail_end;
	extern vm_offset_t virtual_avail, virtual_end;

	usimple_lock_init(&vm_perf_lock);

	/*
	 *	Initializes resident memory structures.
	 *	From here on, all physical memory is accounted for,
	 *	and we use only virtual addresses.
	 *
	 *	Note that vm_page_startup now references avail_start and
	 *	avail_end directly, and will update avail_start after
	 *	reserving space for vm_page structures and hash buckets.
	 */

	virtual_avail = vm_page_startup(virtual_avail);

	/*
	 *	Initialize other VM packages
	 */

	zone_bootstrap();
	vm_object_init();
	vm_map_init();
	kmem_init(virtual_avail, virtual_end);
	pmap_init(avail_start, avail_end);
	zone_init();
	vm_page_module_init();
	h_kmem_init();
	vm_machdep_init();
	msmem_init();
	u_seg_vm_init();
}
