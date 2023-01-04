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
static char *rcsid = "@(#)$RCSfile: k_mape_inv.c,v $ $Revision: 1.1.3.3 $ (DEC) $Date: 1992/12/01 16:04:34 $";
#endif
#include <mach/vm_param.h>
#include <vm/vm_map.h>

int
k_mape_op_bad()
{
	panic("k_mape_op_bad: unsupported map entry operation");
}

extern int k_invalid_grow(), k_invalid_fault(), k_invalid_checkprot();

struct vm_map_entry_ops k_mape_ops_invalid = {
	&k_invalid_fault,	/* fault */
	&k_mape_op_bad,		/* dup */
	&k_mape_op_bad,		/* unmap */
	&k_mape_op_bad,		/* msync */
	&k_mape_op_bad,		/* lockop */
	&k_mape_op_bad,		/* swap */
	&k_mape_op_bad,		/* corefile */
	&k_mape_op_bad,		/* control */
	&k_mape_op_bad,		/* protect */
	&k_invalid_checkprot,	/* check protection */
	&k_mape_op_bad,		/* kluster */
	&k_mape_op_bad,		/* copyout */
	&k_invalid_grow,	/* grow */
};

k_invalid_fault(vm_map_entry_t ep,
	vm_offset_t va,
	vm_size_t size,
	vm_prot_t access_type,
	vm_fault_t fault_type,
	struct vm_page **rpp)
{
	return KERN_INVALID_ADDRESS;
}

boolean_t
k_invalid_checkprot(vm_map_entry_t ep,
	vm_offset_t addr,
	vm_size_t size,
	vm_prot_t prot)
{
	return ((ep->vme_protection & prot) == prot);
}

k_invalid_grow(vm_map_entry_t ep,
	vm_prot_t prot,
	vm_size_t size,
	as_grow_t direction)
{
	return KERN_NO_SPACE;
}
