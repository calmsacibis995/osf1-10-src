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
 *	@(#)$RCSfile: kls_vm.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 23:39:12 $
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

/*
 * kls_vm_init() - perform any initialization necessary for
 *                 manipulation of the kernel's address space
 */
extern int
kls_vm_init(void);

/*
 * kls_vm_allocate() - allocate paged virtual memory in the kernel's
 *                     address space
 */
extern int
kls_vm_allocate(vm_address_t *address, vm_size_t size,
		boolean_t anywhere);

/*
 * kls_vm_deallocate() - deallocate virtual memory in the kernel's
 *                       address space
 */
extern int
kls_vm_deallocate(vm_address_t address, vm_size_t size);

/*
 * kls_vm_read() - read virtual memory from the kernel's address
 *                 space
 */
extern int
kls_vm_read(vm_address_t address, vm_size_t size, pointer_t *data,
	    int *data_count);

/*
 * kls_vm_write() - write virtual memory to the kernel's address
 *                  space
 */
extern int
kls_vm_write(vm_address_t address, pointer_t data, int data_count);

/*
 * kls_vm_protect() - change protection of virtual memory in the
 *                    kernel's address space
 */
extern int
kls_vm_protect(vm_address_t address, vm_size_t size,
	       boolean_t set_maximum, vm_prot_t new_protection);

/*
 * kls_vm_allocate_wired() - allocate wired virtual memory in the
 *                           kernel's address space
 *
 * Wiring at time of allocation is only available via kloadcall(2).
 */
extern int
kls_vm_allocate_wired(vm_address_t *address, vm_size_t size,
		      vm_prot_t prot, boolean_t anywhere);

/*
 * kls_vm_wire() - wired virtual memory in the kernel's address space
 *
 * This call, used to wire memory, sometime after its allocation, is
 * only provided via the Mach VM primitives.
 */
extern int
kls_vm_wire(vm_address_t address, vm_size_t size, vm_prot_t prot);
