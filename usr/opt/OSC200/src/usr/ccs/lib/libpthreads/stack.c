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
static char	*sccsid = "@(#)$RCSfile: stack.c,v $ $Revision: 4.2.3.2 $ (DEC) $Date: 1992/05/05 16:16:30 $";
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

#ifndef	lint
#endif	not lint

/*
 * File: stack.c
 *
 * The functions in this file perform all the stack management and stack
 * related functions. 
 *
 * Stacks are (currently) attached to the vps executing on them. This happens
 * to be OK as there is a 1:1 mapping of pthreads to vps. Stacks have two
 * extra pages allocated for them. The first is the red-zone, which is both
 * read and write protected for overflow detection. The second is for thread
 * specific data. The initial stack (ie the stack for the thread that calls
 * main() is treated differently in that it is not deallocated in the same way.
 * It may be reused safely if the main() thread calls pthread_exit().
 */

#include <pthread.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <errno.h>
#include "internal.h"


/*
 * Local Definitions
 */
#define FIRST_STACK_BASE	0
#define	PTHREAD_MIN_STACK_SIZE	(2 * vm_page_size)

/*
 * Local Macros
 */
#define PAGE_ROUND_DOWN(b)	((b) & ~(vm_page_size - 1))
#define PAGE_ROUND_UP(b)	PAGE_ROUND_DOWN((b) + vm_page_size - 1)

/*
 * Local Variables
 */
private volatile int	initial_stack_lock;
private vm_address_t	initial_stack_base;
private vm_size_t	initial_stack_size;
private vm_size_t	initial_stack_realsize;
private	int		initial_stack_allocated;
private vm_address_t	next_stack_base;

/*
 * Exported Variables
 */
int	pthread_default_stack_size;

/*
 * Function:
 *	get_stack_pointer
 *
 * Return value:
 *	This function returns a value just a little way up the stack from
 *	the callers frame.
 *
 * Description:
 *	Return the address of an automatic variable that will be on the
 *	stack. We know it will not be in a register as we take its address.
 *
 */
private vm_address_t
get_stack_pointer()
{
	int	x;

	return((vm_address_t)&x);
}

/*
 * Function:
 *	stack_self
 *
 * Parameters:
 *	vp - the vp which may be us
 *
 * Return value:
 *	TRUE	if the vp passed uses the same stack we do
 *	FALSE	otherwise
 *
 * Description:
 *	The vp structure contains the base and range of the stack it is using.
 *	get our stack pointer and see if it falls in that range.
 */
int
stack_self(register vp_t vp)
{
	register vm_address_t	sp;
	register vm_address_t	base;

	base = vp->stackbase;
	sp = get_stack_pointer();

	/*
	 * check if our stack is below this stack
	 */
	if (sp < base)
		return(FALSE);
	/*
	 * Our stack pointer is beyond the base, check it is within range
	 */
	if (sp < (base + vp->stacksize))
		return(TRUE);
	/*
	 * The sp is not within range but we may be on the initial stack
	 * which is a little bigger than what is contained in the vp data
	 * so check that too
	 */
	if ((vp->flags&VP_INITIAL_STACK) &&
	    (sp < (base + initial_stack_realsize)))
		return(TRUE);
	/*
	 * This is not our vp
	 */
	return(FALSE);
}

/*
 * Function:
 *	setup_stack
 *
 * Parameters:
 *	vp	- the vp structure we are attaching this stack to
 *	base	- base address of the new stack
 *	size	- the size of the new stack
 *
 * Description:
 *	Add the base/range description of the stack to the vp data and
 *	protect the red zone.
 */
private void
setup_stack(vp_t vp, vm_address_t base, vm_size_t size)
{

#ifdef DEBUG
	/*
	 * check the alignment is OK
	 */
	if (((base & (vm_page_size - 1)) != 0) ||
	    ((size & (vm_page_size - 1)) != 0))
		pthread_internal_error("setup_stack");
#endif

	/*
	 * save the stack dimensions away ignoring the red zone
	 */
	vp->stackbase = base + RED_ZONE_SIZE;
	vp->stacksize = size - RED_ZONE_SIZE;

	/*
	 * Make the red zone untouchable
	 */
	if (vm_protect(task_self(), base + vm_page_size, vm_page_size, FALSE, VM_PROT_NONE) != KERN_SUCCESS)
		pthread_internal_error("vm_protect");
}

/*
 * Function:
 *	alloc_initial_stack
 *
 * Parameters:
 *	vp	- The vp needing a new stack
 *	size	- the minimum size of the new stack
 *
 * Return value:
 *	TRUE	The initial stack was allocated to this vp
 *	FALSE	otherwise
 *
 * Description:
 *	The initial stack allocation is simply controlled by a global
 *	flag (protected by a spin lock). If the flag is true and the
 *	initial flag is at least the requested size then we grab it.
 */
private int
alloc_initial_stack(vp_t vp, vm_size_t size)
{
	/*
	 * There is no point in looking if it is free if it isn't big enough
	 */
	if (size > initial_stack_size)
		return(FALSE);

	/*
	 * take the lock and see if the stack is free.
	 */
	spin_lock(&initial_stack_lock);
	if (initial_stack_allocated) {
		spin_unlock(&initial_stack_lock);
		return(FALSE);
	}

	/*
	 * the stack is free and it is big enough. Mark it as taken and
	 * drop the lock
	 */
	initial_stack_allocated = TRUE;
	spin_unlock(&initial_stack_lock);

	/*
	 * Tell the world that we have the initial stack
	 */
	vp->flags |= VP_INITIAL_STACK;

	/*
	 * attach this stack to our vp and return success
	 */
	setup_stack(vp, initial_stack_base, initial_stack_size);
	return(TRUE);
}

/*
 * Function:
 *	stack_startup
 *
 * Parameters:
 *	vp	- The initial vp
 *
 * Description:
 *	This function is called by pthread_init() at startup time to
 *	initialize the stack data and to allocate the initial stack
 *	to the initial vp.
 */
void
stack_startup(vp_t vp)
{
	struct rlimit	limits;
	vm_address_t	sp;

	/*
	 * Find the default stack size. Use the maximum break size from
	 * getrlimit().
	 */
	if (getrlimit(RLIMIT_STACK, &limits) != 0) {
		perror("getrlimit");
		pthread_internal_error("stack_init");
	}

	/*
	 * Remember the default size, round it up to the nearest page and
	 * then subtract the red zone (as this is added in again when a
	 * new stack is allocated.
	 */
	pthread_default_stack_size = PAGE_ROUND_UP(limits.rlim_cur) - RED_ZONE_SIZE;

	/*
	 * Initialize the stack allocation point
	 */
	next_stack_base = FIRST_STACK_BASE;

	/*
	 * find out where the initial stack is. We mark the base as being
	 * beyond the current point so we don't trample on the process entry
	 * information when we reallocate it to another thread. We do need
	 * to remember the real information too though.
	 */
	sp = get_stack_pointer();
	initial_stack_realsize = pthread_default_stack_size;
	initial_stack_base = sp & ~(pthread_default_stack_size - 1);
	initial_stack_size = PAGE_ROUND_DOWN(sp - initial_stack_base);
	initial_stack_allocated = FALSE;
	initial_stack_lock = SPIN_LOCK_UNLOCKED;

	/*
	 * Now allocate the initial stack to the initial vp so it looks
	 * like any other vp. First, map the red zone so we can protect
	 * its first page as a guard page and allow easy access to its
	 * second page for thread-specific data.
	 */
	if (vm_allocate(task_self(), &initial_stack_base, RED_ZONE_SIZE,
			 FALSE) != KERN_SUCCESS)
		pthread_internal_error("vm_allocate of initial stack");

	alloc_initial_stack(vp, initial_stack_size);
}

/*
 * Function:
 *	new_stack
 *
 * Parameters:
 *	vp	- the vp in need of a new stack
 *	size	- The size of the stack to be created
 *
 * Description:
 *	Allocate a new stack for a vp. This function is called if a new vp
 *	is being created from scratch or a vp is having a larger stack created.
 */
private void
new_stack(vp_t vp, vm_size_t size)
{
	vm_address_t	base;

	/*
	 * start where we left off and try to allocate more memory from there
	 * If we fail then increment by the size we are looking for and try
	 * again until we succeed. There is a chance this will loop forever.
	 */
	base = next_stack_base;
	for (;;) {
		if (vm_allocate(task_self(), &base, size, FALSE) == KERN_SUCCESS)
			break;
		base += size;
	}

	/*
	 * We have managed to allocate the stack. Note where to look next and
	 * then give the stack to the vp
	 */
	next_stack_base = base + size;
	setup_stack(vp, base, size);
}

/*
 * Function:
 *	alloc_stack
 *
 * Parameters:
 *	vp	- the vp needing the new stack
 *	size	- the minimum size of this new stack
 *
 * Return value:
 *	TRUE	if the new stack is allocated
 *
 * Description:
 *	Try to allocate the initial stack for the vp. If we can't then just
 *	allocate a new one from scratch.
 */
int
alloc_stack(vp_t vp, vm_address_t size)
{
	/*
	 * Make sure the requested stack is at least a sensible size capable
	 * of supporting a couple of stack frames, then add the size of the
	 * red zone.
	 */
	if (size < PTHREAD_MIN_STACK_SIZE)
		size = PTHREAD_MIN_STACK_SIZE;

	size += RED_ZONE_SIZE;

	/*
	 * Try for the intial stack. Don't bother with the lock at this point.
	 * It doesn't matter if we are wrong either way.
	 */
	if (!initial_stack_allocated) {
		if (alloc_initial_stack(vp, size))
			return(TRUE);
	}

	/*
	 * Initial stack was not available so just create a new one
	 */
	new_stack(vp, size);
	return(TRUE);
}

/*
 * Function:
 *	dealloc_stack
 *
 * Parameters:
 *	vp	- the vp to have the stack removed
 *
 * Description:
 *	Throw away the stack attached to this vp. If it is the intial stack
 *	then note the stack is free otherwise vm_deallocate it. Set the vp
 *	data to zero to stop is from deallocting twice by mistake.
 */
void
dealloc_stack(vp_t vp)
{
	if (vp->flags & VP_INITIAL_STACK) {
		/*
		 * The vp has the initial stack, free it and mark the vp
		 * to show it has gone.
		 */
		spin_lock(&initial_stack_lock);
		initial_stack_allocated = FALSE;
		spin_unlock(&initial_stack_lock);
		vp->flags &= ~VP_INITIAL_STACK;
	} else {
		/*
		 * Free ordinary stacks using vm_deallocate.
		 */
		if (vm_deallocate(task_self(), vp->stackbase - RED_ZONE_SIZE,
				vp->stacksize + RED_ZONE_SIZE) != KERN_SUCCESS)
			pthread_internal_error("dealloc_stack");
	}

	/*
	 * Mark the vp as having no stack
	 */
	vp->stackbase = 0;
	vp->stacksize = 0;
}

/*
 * Function:
 *	realloc_stack
 *
 * Parameters:
 *	vp	- the vp in need of the new stack
 *	newsize	- The size of the new stack for the vp
 *
 * Return value:
 *	TRUE	if the new stack was allocated
 *	FALSE	otherwise
 *
 * Description:
 *	Simple deallocate the old stack and allocate a new one.
 */
int
realloc_stack(vp_t vp, vm_size_t newsize)
{
	dealloc_stack(vp);
	return(alloc_stack(vp, newsize));
}

/*
 * Function:
 *	stack_fork_prepare
 *
 * Description:
 *	quiesce the stack allocation prior to a fork()
 */
void
stack_fork_prepare()
{
	/*
	 * Take the only lock we can. This protects all global data
	 * that may change during the fork()
	 */
	spin_lock(&initial_stack_lock);
}

/*
 * Function:
 *	stack_fork_parent
 *
 * Description:
 *	Allow more stack allocations to start after a fork in the parent
 */
void
stack_fork_parent()
{
	/*
	 * Drop the lock protecting initial stack allocation
	 */
	spin_unlock(&initial_stack_lock);
}

/*
 * Function:
 *	stack_fork_child
 *
 * Description:
 *	Allow more stack allocations to start after a fork in the child
 */
void
stack_fork_child()
{
	/*
	 * Drop the lock protecting initial stack allocation
	 */
	spin_unlock(&initial_stack_lock);
}
