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
static char	*sccsid = "@(#)$RCSfile: kls_vm.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 23:39:08 $";
#endif 
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
 * This file contains the functions used for manipulation of the
 * kernel of the kernel's address space.  These functions rely one of
 * two possible lower level abstractions to do the actual work: Mach
 * user-mode VM calls the using port returned by task_by_unix_pid(-1)
 * to gain  access to the kernel's address space or the special
 * purpose system  call, kloadcall(2).
 *
 *
 *                    Wired Versus Paged Memory
 *
 * With respect to allocating wired or paged memory in the kernel, if
 * the kloadcall(2) abstraction is used, then the selection of wired
 * or paged memory must be made at the time of memory allocation.
 * Thus the caller must call either kls_vm_allocate() or
 * kls_vm_allocate_wired().
 *
 * If the user-mode Mach VM primitives are used, it is not possible to
 * wire memory at the time of allocation.   kls_vm_allocate(), as in
 * the kloadcall(2) case above, only allocates paged memory.  The
 * caller must explicitly call kls_vm_wire() wire any memory
 * allocated.
 *
 *
 *                       Status Return Values
 *
 * All functions return a status value to indicate the success or
 * failure of the operation.  Unfortunate, we assume that the value
 * zero and the Mach return value of KERN_SUCCESS are one in the same.
 * There for a zero return value implies success.  A non-zero return
 * value implies failure.  Non-zero return values that are positive
 * are Mach  error numbers.  A non-zero return value that is equal to
 * -1 implies a UNIX system call failure, with which errno may be
 * inspected to find the cause of the failure. 
 */

#define	USE_KLOADCALL	1

#include <mach.h>

#ifdef	USE_KLOADCALL
#include <sys/kloadcall.h>
#include "kls_vm.h"
#else
static task_t self_task;
static task_t kernel_task;
static host_priv_t host_priv;
#endif

/*
 * kls_vm_init() - perform any initialization necessary for
 *                 manipulation of the kernel's address space
 */
int
kls_vm_init(void)
{
#ifdef	USE_KLOADCALL
	return(0);
#else
	int rc;

	self_task = task_self();
	if ((rc = task_by_unix_pid(task_self(), -1, &kernel_task))
	    != KERN_SUCCESS)
		return(rc);
	if ((host_priv = host_priv_self()) == PORT_NULL)
		return(KERN_FAILURE);
	return(0);
#endif
}

/*
 * kls_vm_allocate() - allocate paged virtual memory in the kernel's
 *                     address space
 */
int
kls_vm_allocate(vm_address_t *address, vm_size_t size,
		boolean_t anywhere)
{
#ifdef	USE_KLOADCALL
	return(kloadcall(KLC_VM_ALLOCATE, address, size, anywhere));
#else
	return(vm_allocate(kernel_task, address, size, anywhere));
#endif
}

/*
 * kls_vm_deallocate() - deallocate virtual memory in the kernel's
 *                       address space
 */
int
kls_vm_deallocate(vm_address_t address, vm_size_t size)
{
#ifdef	USE_KLOADCALL
	return(kloadcall(KLC_VM_DEALLOCATE, address, size));
#else
	return(vm_deallocate(kernel_task, address, size));
#endif
}

/*
 * kls_vm_read() - read virtual memory from the kernel's address
 *                 space
 */
int
kls_vm_read(vm_address_t address, vm_size_t size, pointer_t *data,
	    int *data_count) 
{
#ifdef	USE_KLOADCALL
	return(kloadcall(KLC_VM_READ, address, size, data, data_count));
#else
	return(vm_read(kernel_task, address, size, data,
		       (unsigned int *)data_count));
#endif
}

/*
 * kls_vm_write() - write virtual memory to the kernel's address
 *                  space
 */
int
kls_vm_write(vm_address_t address, pointer_t data, int data_count)
{
#ifdef	USE_KLOADCALL
	return(kloadcall(KLC_VM_WRITE, address, data, data_count));
#else
	return(vm_write(kernel_task, address, data, data_count));
#endif
}

/*
 * kls_vm_protect() - change protection of virtual memory in the
 *                    kernel's address space
 */
int
kls_vm_protect(vm_address_t address, vm_size_t size,
	       boolean_t set_maximum, vm_prot_t new_protection)
{
#ifdef	USE_KLOADCALL
	return(kloadcall(KLC_VM_PROTECT, address, size, set_maximum,
			 new_protection));
#else
	return(vm_protect(kernel_task, address, size, set_maximum,
			  new_protection));
#endif
}

#ifdef	USE_KLOADCALL
/*
 * kls_vm_allocate_wired() - allocate wired virtual memory in the
 *                           kernel's address space
 *
 * Wiring at time of allocation is only available via kloadcall(2).
 */
int
kls_vm_allocate_wired(vm_address_t *address, vm_size_t size,
		      vm_prot_t prot,  boolean_t anywhere)
{
	return(kloadcall(KLC_VM_ALLOCATE_WIRED, address, size, prot,
			 anywhere));
}
#else	/* USE_KLOADCALL */
/*
 * kls_vm_wire() - wired virtual memory in the kernel's address space
 *
 * This call, used to wire memory, sometime after its allocation, is
 * only provided via the Mach VM primitives.
 */
int
kls_vm_wire(vm_address_t address, vm_size_t size, vm_prot_t prot)
{
	vm_address_t end;

	end = address + ((unsigned int)size) - 1;
	return(vm_wire(host_priv, kernel_task, address, end, prot));
}
#endif	/* USE_KLOADCALL */
