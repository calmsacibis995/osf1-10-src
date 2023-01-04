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
static char	*sccsid = "@(#)$RCSfile: ldr_kload.c,v $ $Revision: 4.3.5.2 $ (DEC) $Date: 1993/04/09 21:34:28 $";
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
 * This file implements the kloadcall(2) system call.  kloadcall(2)
 * provides functionality to manipulate the kernel address space for
 * kernel dynamic loading.  We would have preferred to use the normal
 * user-mode Mach VM primitives in conjunction with task_by_unix_pid(-1), 
 * however, there were and are many complications to that approach.
 * Due to various constraints we chose to place the necessary
 * functionality into kloadcall(2), at least until some future time.
 * The interfaces to kloadcall(2) have been structured to closely
 * resemble the user-mode Mach VM primitives, in order to make it
 * easier for applications, such as the kernel load server, to switch
 * to the user-mode Mach VM primitives when they are available.
 *
 * N.B.:  Beware that the kloadcall() system call is a hybrid between
 *        a UN*X system call and a Mach VM call.  It can return errors
 *        in two ways.  We are fortunate that ESUCCESS and
 *        KERN_SUCCESS are the same value.  A UN*X error is returned,
 *        in the traditional way, by having kloadcall(), return a UN*X
 *        errno value.  A Mach VM error (i.e. a kern_return_t) is
 *        returned as the return value (i.e. *retval) of the
 *        kloadcall() system call.
 *
 *        Another way of looking at this is from the user-mode
 *        application's perspective.  When kloadcall() return 0 (i.e.
 *        ESUCCESS or KERN_SUCCESS), the kloadcall() operation was
 *        successful.  When kloadcall() returns -1, kloadcall() failed
 *        with a UN*X system call error and the application should
 *        check errno for the reason.  When kloadcall() does not
 *        return zero nor -1, kloadcall() failed with a Mach VM call
 *        error and the return value is a kern_return_t that indicates
 *        the reason for the failure. 
 */

#include <sys/secdefines.h>
#include <sys/security.h>

#include <sys/types.h>

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/user.h>

#include <mach/std_types.h>
#include <mach/mach_types.h>
#include <mach/vm_param.h>

#include <sys/kloadcall.h>

#define	KLOADCALL_TRACE	1

#ifdef	KLOADCALL_TRACE
#define	dprintf(x)	if (kloadcall_trace)	uprintf x
int kloadcall_trace = 0;
#else
#define	dprintf(x)
#endif

kloadcall(p, args, retval)
	struct proc *p;
	void *args;
	long *retval;
{
	register struct args {
		long	op;
	} *uap = (struct args *) args;
	int error;

	/*
	 * Must have privilege
	 */
#if SEC_BASE
	if (!privileged(SEC_DEBUG, EPERM))	/* XXX which priv needed? */
		return(EPERM);
#else
	if (error = suser(u.u_cred, &u.u_acflag))
		return(error);
#endif

#if	1
	/*
	 * This interface has not been tested and actually can panic the
	 * system if incorrectly used (see QAR 11991.)  If and when we need
	 * this syscall make sure that the problems are addressed.
	 */
	return(ENOTSUP);
#endif

	switch (uap->op) {

	default:
		dprintf(("kloadcall: kloadcall(%d)\n", uap->op));
		error = EINVAL;
		break;

	case KLC_VM_ALLOCATE:
		error = klc_vm_allocate(p, args, retval);
		break;

	case KLC_VM_DEALLOCATE:
		error = klc_vm_deallocate(p, args, retval);
		break;

	case KLC_VM_READ:
		error = klc_vm_read(p, args, retval);
		break;

	case KLC_VM_WRITE:
		error = klc_vm_write(p, args, retval);
		break;

	case KLC_VM_PROTECT:
		error = klc_vm_protect(p, args, retval);
		break;

	case KLC_VM_ALLOCATE_WIRED:
		error = klc_vm_allocate_wired(p, args, retval);
		break;

	case KLC_CALL_FUNCTION:
		error = klc_call_function(p, args, retval);
		break;
	}

	if (error) {
		dprintf(("kloadcall: value=-1, errno=%d\n", error));
	} else {
		dprintf(("kloadcall: value=%d(0x%x)\n", *retval,
			*retval));
	}
	return(error);
}

klc_vm_allocate(p, args, retval)
	struct proc *p;
	void *args;
	register long *retval;
{
	register struct args {
		long		op;
		vm_offset_t	*address;
		vm_size_t	size;
		long		anywhere;
	} *uap = (struct args *) args;
	vm_offset_t	address;
	boolean_t       anywhere = (boolean_t) uap->anywhere;
	
	dprintf(("kloadcall: kloadcall(KLC_VM_ALLOCATE, address=0x%x, size=%d(0x%x), anywhere=%d)\n",
		uap->address, uap->size, uap->size, anywhere));

	if (!anywhere) {
		if (copyin((caddr_t)uap->address, (caddr_t)&address,
		    sizeof(address))) {
			*retval = KERN_INVALID_ADDRESS;
			return(0);
		}

		dprintf(("kloadcall: *address=0x%x IN\n", address));

		if (trunc_page(address) != address) {
			*retval = KERN_INVALID_ARGUMENT;
			return(0);
		}
	}

	if (trunc_page(uap->size) != uap->size) {
		*retval = KERN_INVALID_ARGUMENT;
		return(0);
	}

	if ((*retval = kernel_memory_allocate_paged(kernel_map, &address,
	    uap->size, anywhere)) != KERN_SUCCESS)
		return(0);

	/*
	 * Set the default protection to RW so that a later
	 * call to vm_protect() that adds X will cause caches
	 * to be synchronized.  RW will be the default for
	 * vm_allocate() in a future release and at that time
	 * this code should be removed.
	 */
	if ((*retval = vm_protect(kernel_map, address, uap->size,
	    FALSE, (VM_PROT_READ|VM_PROT_WRITE))) != KERN_SUCCESS) {
		kmem_free(kernel_map, address, uap->size);
		*retval = KERN_INVALID_ADDRESS;
		return(0);
	}

	if (copyout((caddr_t)&address, (caddr_t)uap->address,
	    sizeof(address))) {
		kmem_free(kernel_map, address, uap->size);
		*retval = KERN_INVALID_ADDRESS;
		return(0);
	}

	dprintf(("kloadcall: *address=0x%x OUT\n", address));

	*retval = KERN_SUCCESS;
	return(0);
}

klc_vm_deallocate(p, args, retval)
	struct proc *p;
	void *args;
	register long *retval;
{
	register struct args {
		long		op;
		vm_offset_t	address;
		vm_size_t	size;
	} *uap = (struct args *) args;

	dprintf(("kloadcall: kloadcall(KLC_VM_DEALLOCATE, address=0x%x, size=%d(0x%x))\n",
		uap->address, uap->size, uap->size));

	if ((trunc_page(uap->address) != uap->address)
	    || (trunc_page(uap->size) != uap->size)) {
		*retval = KERN_INVALID_ARGUMENT;
		return(0);
	}

	kmem_free(kernel_map, uap->address, uap->size);
	*retval = KERN_SUCCESS;
	return(0);
}

klc_vm_read(p, args, retval)
	struct proc *p;
	void *args;
	register long *retval;
{
	register struct args {
		long		op;
		vm_offset_t	address;
		vm_size_t	size;
		vm_offset_t	*data;
		int		*data_count;
	} *uap = (struct args *) args;
	vm_offset_t	data;
	int		error;

	dprintf(("kloadcall: kloadcall(KLC_VM_READ, address=0x%x, size=%d(0x%x), data=0x%x, data_count=0x%x)\n",
		uap->address, uap->size, uap->size, uap->data,
		uap->data_count));

	if ((trunc_page(uap->address) != uap->address)
	    || (trunc_page(uap->size) != uap->size)) {
		*retval = KERN_INVALID_ARGUMENT;
		return(0);
	}

	if ((*retval = vm_allocate(current_task()->map, &data, uap->size,
	    TRUE)) != KERN_SUCCESS)
		return(0);

	if (error = copyout((caddr_t)&data, (caddr_t)uap->data, sizeof(data)))
		goto out;

	if (error = copyout((caddr_t)&uap->size, (caddr_t)uap->data_count,
	    sizeof(uap->size)))
		goto out;

	/* 
	 * Should check to make sure that source kernel address
	 * is acessible.  For now, we expect copyout() to catch
	 * not only an inacessible user target address but also
	 * an inacessible source kernel address.
	 */

	if (error = copyout((caddr_t)uap->address, (caddr_t)data, uap->size))
		goto out;

	dprintf(("kloadcall: *data=0x%x, *data_count=%d(0x%x))\n",
		data, uap->size, uap->size));

	*retval = KERN_SUCCESS;
	error = 0;
out:
	if (error) {
		*retval = KERN_INVALID_ADDRESS;
		(void)vm_deallocate(current_task()->map, data, uap->size);
	}
	return(0);
}

klc_vm_write(p, args, retval)
	struct proc *p;
	void *args;
	register long *retval;
{
	register struct args {
		long		op;
		vm_offset_t	address;
		vm_offset_t	data;
		long		data_count;
	} *uap = (struct args *) args;

	/* 
	 * Should check to make sure that target kernel address
	 * is acessible.  For now, we expect copyin() to catch
	 * not only an inacessible user source address but also
	 * an inacessible target kernel address.
	 */

	dprintf(("kloadcall: kloadcall(KLC_VM_WRITE, address=0x%x, data=0x%x, data_count=%d(0x%x))\n",
		uap->address, uap->data, uap->data_count,
		uap->data_count));

	if ((trunc_page(uap->address) != uap->address)
	    || (trunc_page(uap->data) != uap->data)
	    || (trunc_page(uap->data_count) != uap->data_count)) {
		*retval = KERN_INVALID_ARGUMENT;
		return(0);
	}

	if (copyin((caddr_t)uap->data, (caddr_t)uap->address,
	    uap->data_count)) {
		*retval = KERN_INVALID_ADDRESS;
		return(0);
	}

	*retval = KERN_SUCCESS;
	return(0);
}

klc_vm_protect(p, args, retval)
	struct proc *p;
	void *args;
	register long *retval;
{
	register struct args {
		long		op;
		vm_offset_t	address;
		vm_size_t	size;
		long		set_maximum;
		long		new_protection;
	} *uap = (struct args *) args;
	boolean_t       set_maximum = (boolean_t) uap->set_maximum;
	vm_prot_t       new_protection = (vm_prot_t) uap->new_protection;
	

	dprintf(("kloadcall: kloadcall(KLC_VM_PROTECT, address=0x%x, size=%d(0x%x), set_maximum=%d, new_protection=0x%x)\n",
		uap->address, uap->size, uap->size, set_maximum,
		new_protection));

	if ((trunc_page(uap->address) != uap->address)
	    || (trunc_page(uap->size) != uap->size)) {
		*retval = KERN_INVALID_ARGUMENT;
		return(0);
	}

	*retval = vm_protect(kernel_map, uap->address, uap->size,
		set_maximum, new_protection);
	return(0);
}

klc_vm_allocate_wired(p, args, retval)
	struct proc *p;
	void *args;
	register long *retval;
{
	register struct args {
		long		op;
		vm_offset_t	*address;
		vm_size_t	size;
		long		wire_prot;
		long		anywhere;
	} *uap = (struct args *) args;
	vm_offset_t	address;
	vm_prot_t       wire_prot = (vm_prot_t) uap->wire_prot;
	boolean_t       anywhere = (boolean_t) uap->anywhere;
	
	dprintf(("kloadcall: kloadcall(KLC_VM_ALLOCATE_WIRED, address=0x%x, size=%d(0x%x), wire_prot=0x%x, anywhere=%d)\n",
		uap->address, uap->size, uap->size, wire_prot, anywhere));

	if (!anywhere) {
		if (copyin((caddr_t)uap->address, (caddr_t)&address,
		    sizeof(address))) {
			*retval = KERN_INVALID_ADDRESS;
			return(0);
		}

		dprintf(("kloadcall: *address=0x%x IN\n", address));

		if (trunc_page(address) != address) {
			*retval = KERN_INVALID_ARGUMENT;
			return(0);
		}
	}

	if (trunc_page(uap->size) != uap->size) {
		*retval = KERN_INVALID_ARGUMENT;
		return(0);
	}

	if (trunc_page(uap->size) != uap->size) {
		*retval = KERN_INVALID_ARGUMENT;
		return(0);
	}

	if ((*retval = kernel_memory_allocate_wired(kernel_map, &address,
	    uap->size, wire_prot, anywhere)) != KERN_SUCCESS)
		return(0);

	/*
	 * Set the default protection to RW so that a later
	 * call to vm_protect() that adds X will cause caches
	 * to be synchronized.  RW will be the default for
	 * vm_allocate() in a future release and at that time
	 * this code should be removed.
	 */
	if ((*retval = vm_protect(kernel_map, address, uap->size,
	    FALSE, (VM_PROT_READ|VM_PROT_WRITE))) != KERN_SUCCESS) {
		kmem_free(kernel_map, address, uap->size);
		*retval = KERN_INVALID_ADDRESS;
		return(0);
	}

	if (copyout((caddr_t)&address, (caddr_t)uap->address,
	    sizeof(address))) {
		kmem_free(kernel_map, address, uap->size);
		*retval = KERN_INVALID_ADDRESS;
		return(0);
	}

	dprintf(("kloadcall: *address=0x%x OUT\n", address));

	*retval = KERN_SUCCESS;
	return(0);
}

klc_call_function(p, args, retval)
	struct proc *p;
	void *args;
	long *retval;
{
	register struct args {
		long		op;
		vm_offset_t	address;
		long		arg1;
		long		arg2;
		long		arg3;
	} *uap = (struct args *) args;
	int (*funcp)();

	dprintf(("kloadcall: kloadcall(KLC_CALL_FUNCTION, address=0x%x, arg1=0x%x, arg2=0x%x, arg3=0x%x)\n",
		uap->address, uap->arg1, uap->arg2, uap->arg3));

	funcp = (int(*)())uap->address;
	*retval = (*funcp)(uap->arg1, uap->arg2, uap->arg3);
	return(0);
}
