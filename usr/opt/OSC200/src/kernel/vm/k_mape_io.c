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
static char *rcsid = "@(#)$RCSfile: k_mape_io.c,v $ $Revision: 1.1.3.3 $ (DEC) $Date: 1992/12/01 16:04:57 $";
#endif
#include <mach/vm_param.h>
#include <vm/vm_map.h>

/*
 * Support for I/O space mappings
 */

extern int k_mape_op_bad(), k_io_fault(), k_io_checkprot();
extern int k_io_unmap(), k_io_grow();

struct vm_map_entry_ops k_mape_ops_io = {
	&k_io_fault,		/* fault */
	&k_mape_op_bad,		/* dup */
	&k_io_unmap,		/* unmap */
	&k_mape_op_bad,		/* msync */
	&k_mape_op_bad,		/* lockop */
	&k_mape_op_bad,		/* swap */
	&k_mape_op_bad,		/* corefile */
	&k_mape_op_bad,		/* control */
	&k_mape_op_bad,		/* protect */
	&k_io_checkprot,	/* check protection */
	&k_mape_op_bad,		/* kluster */
	&k_mape_op_bad,		/* copyout */
	&k_io_grow,		/* grow */
};

/*
 * We enter with the map locked.
 * The pmap is managed by the I/O pmap cache management tools.
 */

k_io_fault(vm_map_entry_t ep,
	vm_offset_t va,
	vm_size_t size,
	vm_prot_t access_type,
	vm_fault_t fault_type,
	struct vm_page **rpp)
{
	return KERN_INVALID_ADDRESS;
}

k_io_unmap(register vm_map_entry_t ep,
	vm_offset_t vaddr,
	vm_size_t size)
{
	vm_map_clip_start(ep->vme_map, ep, vaddr);
	vm_map_clip_end(ep->vme_map, ep, vaddr + size);
	vm_map_unlock(ep->vme_map);
	return KERN_SUCCESS;
}

boolean_t
k_io_checkprot(vm_map_entry_t ep,
	vm_offset_t addr,
	vm_size_t size,
	vm_prot_t prot)
{
	return FALSE;
}

k_io_grow(vm_map_entry_t ep,
	vm_prot_t prot,
	vm_size_t size,
	as_grow_t direction)
{
	return KERN_NO_SPACE;
}
