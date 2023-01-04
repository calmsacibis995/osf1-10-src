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
 * @(#)$RCSfile: vm_vlock.h,v $ $Revision: 1.1.3.2 $ (DEC) $Date: 1992/03/19 08:57:05 $
 */
#ifndef	__VM_VLOCK__
#define	__VM_VLOCK__ 1

/*
 * Support of kernel lock of user address space.
 * We don't want to pollute the user space mappings
 * with locks held by the kernel to do physical I/O.
 * Instead we maintain a list of virtual space locked.
 * Also we don't release empty lock space until we
 * scan and its above some limit.  This will work if
 * the u-map uses a common set of VAS locations to do
 * I/O.
 */ 

struct vm_vlock {
	struct vm_vlock *vl_next;	/* Next link */
	struct vm_vlock *vl_prev;	/* Back pointer */
	vm_offset_t vl_start;		/* Beginning */
	vm_offset_t vl_end;		/* Last exclusive */
	unsigned short vl_count;	/* Locks held */
};

#ifdef	KERNEL

extern kern_return_t vl_wire(vm_map_entry_t ep, 
			vm_offset_t start, vm_offset_t end, vm_prot_t prot);
extern kern_return_t vl_unwire(vm_map_entry_t ep, 
			vm_offset_t start, vm_offset_t end);
extern boolean_t vl_kwire(vm_map_entry_t ep, 
			vm_offset_t start, vm_offset_t end);
extern void vl_remove(vm_map_t map);

#endif	/* KERNEL */

#endif	/* !__VM_VLOCK */
