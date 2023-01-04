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
static char	*sccsid = "@(#)$RCSfile: vm_fault.c,v $ $Revision: 4.2.8.5 $ (DEC) $Date: 1993/09/22 18:29:43 $";
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
 *	File:	vm_fault.c
 *	Author:	Avadis Tevanian, Jr., Michael Wayne Young
 *
 *	Page fault handling module.
 */

#include <vm/vm_fault.h>
#include <mach/kern_return.h>
#include <mach/boolean.h>
#include <mach/machine/vm_types.h>
#include <vm/vm_map.h>
#include <mach/vm_prot.h>
#include <mach/vm_inherit.h>
#include <sys/time.h>
#include <sys/vnode.h>
#include <sys/specdev.h>
#include <sys/mount.h>
#include <sys/buf.h>
#include <sys/uio.h>
#include <sys/user.h>
#include <vm/vm_perf.h>
#include <sys/dk.h>	/* For SAR counters */

extern vm_offset_t stackinc;
 
/*
 *  Called by to handle an address translation fault
 *  Currently only stack growth down is supported.
 */

kern_return_t 
vm_fault(register vm_map_t map, 
	register vm_offset_t vaddr, 
	vm_prot_t fault_type 
	)
{
	kern_return_t ret;

	vpf_ladd(pagefaults,1);
	if (current_thread()) current_thread()->thread_events.faults++;

	ret = (*map->vm_fault_map)(map, vaddr, fault_type, VM_NOWIRE);

	if (ret == KERN_SUCCESS ) {
		/* global table() system call counter (see table.h) */
		pg_v_pgpgin++;
		return ret;
	}
	if( !map->vm_umap) return ret;
	
	if (vaddr < (vm_offset_t) (u.u_stack_start)) {
		
		kern_return_t saveret;
		vm_offset_t addr;
		unsigned long stack_limit, new_stack_size, old_stack_size;
		
		addr = trunc_page(vaddr & ~(stackinc - 1));

		U_HANDY_LOCK();
		stack_limit = (unsigned long) u.u_rlimit[RLIMIT_STACK].rlim_cur;
 		new_stack_size = (unsigned long) ((vm_offset_t) u.u_stack_end - addr);
		old_stack_size = (unsigned long) u.u_ssize;
		U_HANDY_UNLOCK();	

		if  ((new_stack_size > stack_limit)
		     || (vm_map_vsize(map) - old_stack_size + new_stack_size
			 > u.u_rlimit[RLIMIT_AS].rlim_cur))
			return(ret);

		saveret = ret;

		ret = u_map_grow(map, VM_OBJECT_NULL, (vm_offset_t) 0,
			&addr, (vm_offset_t) (u.u_stack_start) - addr, 
			FALSE, AS_GROWDOWN);
		if (ret == KERN_SUCCESS) {
			ret = u_map_fault(map, vaddr, fault_type, VM_NOWIRE);
			if (ret == KERN_SUCCESS) {
				/* 
				 * global table() system call counter (see
				 * table.h)
				 */
				pg_v_pgpgin++;
				U_HANDY_LOCK();
				(vm_offset_t) (u.u_stack_start) = addr;
				u.u_ssize = 
					btoc((vm_offset_t)(u.u_stack_end)-addr);
				U_HANDY_UNLOCK();
			}
		}
		else ret = saveret;

	}

	return ret;

}
