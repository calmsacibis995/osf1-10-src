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
static char	*sccsid = "@(#)$RCSfile: memory_object.c,v $ $Revision: 4.2.3.2 $ (DEC) $Date: 1993/01/08 18:07:53 $";
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
 *	File:	vm/memory_object.c
 *	Author:	Michael Wayne Young
 *
 *	External memory management interface control functions.
 */
#include <mach_xp_fpd.h>

/*
 *	Interface dependencies:
 */

#include <mach/std_types.h>	/* For pointer_t */
#include <mach/mach_types.h>

#include <mach/kern_return.h>
#include <vm/vm_object.h>
#include <mach/memory_object.h>
#include <mach/memory_object_user.h>
#include <mach/boolean.h>
#include <mach/vm_prot.h>
#include <mach/message.h>

/*
 *	Implementation dependencies:
 */
#include <vm/vm_page.h>
#include <vm/vm_pageout.h>
#include <vm/pmap.h>		/* For copy_to_phys, pmap_clear_modify */
#include <kern/thread.h>		/* For current_thread() */

#if	!MACH_XP_FPD
#include <vm/vm_kern.h>		/* For kernel_map, vm_move */
#include <vm/vm_map.h>		/* For vm_map_pageable */
#include <kern/ipc_globals.h>
#endif	!MACH_XP_FPD
#include <kern/kern_port.h>

#include <kern/xpr.h>

int		memory_object_debug = 0;

/*
 */

memory_object_t	memory_manager_default = PORT_NULL;
decl_simple_lock_data(,memory_manager_default_lock)

/*
 *	Important note:
 *		All of these routines gain a reference to the
 *		object (first argument) as part of the automatic
 *		argument conversion. Explicit deallocation is necessary.
 */
boolean_t	modp_single_page_optimization = TRUE;

kern_return_t memory_object_data_provided(object, offset, data, data_cnt, lock_value)
	register
	vm_object_t	object;
	register
	vm_offset_t	offset;
	pointer_t	data;
	unsigned int	data_cnt;
	vm_prot_t	lock_value;
{
	kern_return_t	result = KERN_SUCCESS;
	register
	vm_page_t	m;
	register
	vm_page_t	placeholder_page;
	queue_head_t	placeholder_queue;
	vm_object_t	tmp_object;
	int		absent_count = 0;
	boolean_t	undo = TRUE;
#if	!MACH_XP_FPD
	vm_offset_t	kernel_data;
	vm_size_t	kernel_data_cnt;
#endif	!MACH_XP_FPD

	XPR(XPR_MEMORY_OBJECT, ("memory_object_data_provided, object 0x%x, offset 0x%x",
				object, offset, 0, 0));

#if	MACH_XP_FPD
#define	DESTROY_DATA
#else	MACH_XP_FPD
#define	DESTROY_DATA	vm_deallocate(ipc_soft_map, data, data_cnt);
#endif	MACH_XP_FPD

	/*
	 *	Look for bogus arguments
	 */

	if (object == VM_OBJECT_NULL) {
		DESTROY_DATA;
		return(KERN_INVALID_ARGUMENT);
	}

	if (lock_value & ~VM_PROT_ALL) {
		DESTROY_DATA;
		vm_object_deallocate(object);
		return(KERN_INVALID_ARGUMENT);
	}

	/*
	 *	Adjust the offset from the memory object to the offset
	 *	within the vm_object.
	 *
	 *	Once we look at the paging_offset, we must ensure that
	 *	the object cannot be collapsed.  This means taking a
	 *	paging reference for now.
	 */

	vm_object_lock(object);
	offset -= object->paging_offset;
	vm_object_paging_begin(object);
	vm_object_unlock(object);

#if	MACH_XP_FPD
	/*
	 *	Providing a single page can be done in one pass.
	 *
	 *	See whether the normal case can be optimized:
	 */

	if (modp_single_page_optimization && data_cnt == PAGE_SIZE) {
		vm_object_lock(object);
		if (
		    ((m = vm_page_lookup(object, offset)) != VM_PAGE_NULL) &&
		    m->busy && m->absent && !m->fictitious
		   ) {
			/*
			 *	See the documentation for the full
			 *	algorithm for details.
			 */

			/* Prevent another thread from doing the same */
			m->absent = FALSE;
			vm_object_unlock(object);

			/* Copy the data */
			if (vm_map_pmap(current_task()->map) != kernel_pmap)
				copy_user_to_physical_page(data, m, PAGE_SIZE);
			 else
				copy_to_phys((vm_offset_t) data,
					m->phys_addr, PAGE_SIZE);
			pmap_clear_modify(m->phys_addr);

			/* Set data attributes; wakeup (removes busy) */
			vm_object_lock(object);
			m->page_lock = lock_value;
			m->unlock_request = VM_PROT_NONE;
			PAGE_WAKEUP_DONE(m);

			/* Make page eligible for replacement again */
			vm_page_lock_queues();
			vm_page_activate(m);
			vm_page_unlock_queues();

			/* Release absent and paging references */
			vm_object_absent_release(object);
			vm_object_paging_end(object);

			/* Release object reference given to this routine */
			if (--object->ref_count == 0) {
				object->ref_count++;
				vm_object_unlock(object);
				vm_object_deallocate(object);
			} else
				vm_object_unlock(object);

			return(KERN_SUCCESS);
			
		}
		vm_object_unlock(object);
	}
#endif	MACH_XP_FPD

	/*
	 *	Get a temporary object to hold the pages as we
	 *	copy them in.  The pages in this object are known
	 *	only to this thread and the pageout daemon.
	 */

	if ((tmp_object = current_thread()->tmp_object) == VM_OBJECT_NULL) {
		tmp_object = current_thread()->tmp_object =
			vm_object_allocate((vm_size_t) 0);
	}

#if	!MACH_XP_FPD
	/*
	 *	Copy the data into the kernel
	 */

	if (vm_move(ipc_soft_map, (vm_offset_t) data, ipc_kernel_map,
		    data_cnt, TRUE, &kernel_data) != KERN_SUCCESS) {
		printf("memory_object_data_provided: %s",
			"memory_object_data_provided: cannot move data");
		DESTROY_DATA;
		vm_object_deallocate(object);
		return(KERN_RESOURCE_SHORTAGE);
	}
	data = kernel_data;
	kernel_data_cnt = data_cnt;
#endif	MACH_XP_FPD

	/*
	 *	First pass: copy in the data
	 */

	queue_init(&placeholder_queue);
	for (;
	     data_cnt >= PAGE_SIZE;
	     data_cnt -= PAGE_SIZE, data += PAGE_SIZE, offset += PAGE_SIZE) {

		/*
		 *	See whether there is a page waiting for our data.
		 *	Get a placeholder page now before grabbing locks.
		 *	Initialization postponed until page is used.
		 */
		placeholder_page =
			(vm_page_t) zalloc(vm_page_fictitious_zone);
		vm_object_lock(object);
		if ((memory_object_debug & 0x2) && (object->ref_count <= 1))
			printf("memory_object_data_provided: supplying data to a dead object");

		if ((m = vm_page_lookup(object, offset)) == VM_PAGE_NULL) {
			/*
			 *	No page waiting -- see whether we can allocate
			 *	one, but don't bother waiting.
			 */

			if (memory_object_debug & 0x4) {
				printf("memory_object_data_provided: object providing spurious data");
				printf("; object = 0x%x, offset = 0x%x\n", object, offset);
			}
			if ((m = vm_page_alloc(object, offset)) == VM_PAGE_NULL) {
				char		verify_data;

				result = KERN_RESOURCE_SHORTAGE;

			 VerifyPage:

				/*
				 *	Won't actually page in this page,
				 *	so free placeholder that we won't use.
				 */

				vm_object_unlock(object);
				zfree(vm_page_fictitious_zone,
					(vm_offset_t)placeholder_page);

				/*
				 *	Even if we cannot use the data, we
				 *	must verify that the data being
				 *	supplied is valid, in order to
				 *	preserve the appearance of a msg_send
				 *	operation.
				 *
				 *	Note that the user-supplied data
				 *	may span more than one page.
				 */

				if (copyin(((char *) data),
					   &verify_data, 1) ||
				    (!page_aligned((vm_offset_t) data) &&
				     copyin(((char *) data) + (PAGE_SIZE-1),
					   &verify_data, 1))) {
					/*
					 *	Indicate that an underlying
					 *	IPC error should have
					 *	occurred.
					 */

					current_thread()->ipc_state = SEND_INVALID_MEMORY;
					result = KERN_FAILURE;

					/*
					 *	Remember to undo changes in the second
					 *	pass, rather than committing them.
					 */

					undo = TRUE;

					/*
					 *	There's no point in continuing the first
					 *	pass at this point.
					 */

					break;
				}

				continue;
			}
		} else {
			/*
			 *	Only overwrite pages that are "absent".
			 */

			if (!(m->busy && m->absent))
				goto VerifyPage;
		}

		/*
		 *	At this point, the page in question should be
		 *	"busy".  If it was previously requested, then it
		 *	will also be "absent".
		 */

		/*
		 *	Move the page to the alternate object,
		 *	and install a (busy) placeholder page in
		 *	the original object.
		 *
		 *	XXXO This step could be combined with the
		 *	lookup/allocate step above.
		 */

		vm_object_lock(tmp_object);
		vm_page_rename(m, tmp_object, offset);
		vm_object_unlock(tmp_object);

		vm_page_init(placeholder_page, object, offset, (vm_offset_t) 0);
		placeholder_page->fictitious = TRUE;

		/*
		 *	Remember whether the page has been
		 *	explicitly requested, in case we later abort.
		 */

		if (m->absent) {
			placeholder_page->was_absent = TRUE;
			m->absent = FALSE;
			absent_count++;
		}

		/*
		 *	Enqueue the placeholder page so that we can
		 *	quickly make a second pass.
		 *
		 *	WARNING: we are using the pageout queue
		 *	chain for this list.  This is not a serious
		 *	problem, since the placeholder is wired,
		 *	but if the pageout system changes, this
		 *	will require re-engineering.
		 */

		queue_enter(&placeholder_queue, placeholder_page,
					 vm_page_t, pageq);

		/*
		 *	Copy the data into the page (now in the
		 *	temporary object).
		 *
		 *	Note that we cannot write into fictitious
		 *	pages.
		 *
		 *	XXXO We could be optimistic and try to
		 *	allocate a new page again.
		 */

		if (m->fictitious)
			goto VerifyPage;

		/*
		 *	Copy in the data.
		 *
		 *	Note that we must unlock the object, because
		 *	the copyin may fault (and may be interrupted).
		 */

		vm_object_unlock(object);

#if	MACH_XP_FPD
		if (vm_map_pmap(current_task()->map) != kernel_pmap)
			copy_user_to_physical_page(data, m, data_cnt);
		 else
#endif	MACH_XP_FPD
			copy_to_phys((vm_offset_t) data,
					 m->phys_addr, data_cnt);

		pmap_clear_modify(m->phys_addr);

		/*
		 *	Make this page eligible for pageout if it
		 *	wasn't already requested.
		 *
		 *	This prevents our thread from consuming
		 *	several physical pages and then faulting on
		 *	other bogus user-managed data.
		 */

		if (!placeholder_page->was_absent) {
			m->busy = FALSE;
			vm_page_lock_queues();
			if (!m->active)
				vm_page_activate(m);
			vm_page_unlock_queues();
		}
	}
	
	if (data_cnt != 0)
		uprintf("memory_object_data_provided: partial page discarded\n");

	/*
	 *	Second pass: atomically make the data available.
	 *	[If an error occurred in the first pass, this pass *undoes*
	 *	the changes done in the first pass.]
	 */

	vm_object_lock(object);
	vm_object_lock(tmp_object);
	for (
	    placeholder_page = (vm_page_t) queue_first(&placeholder_queue);
	    !queue_end(&placeholder_queue, (queue_entry_t) placeholder_page);
	    placeholder_page = (vm_page_t) queue_first(&placeholder_queue)
	    ) {
		boolean_t	was_absent = placeholder_page->was_absent;

		/*
		 *	We must look up the real page, as it may have
		 *	disappeared.
		 *
		 *	XXXO We could step through the alternate object's
		 *	page queue, since it should never be reordered.
		 *	[Even if it is reordered, we can always throw
		 *	away pages that we don't want anyway.]
		 */

		offset = placeholder_page->offset;
		queue_remove(&placeholder_queue, placeholder_page,
					vm_page_t, pageq);
		VM_PAGE_FREE(placeholder_page);

		if ((m = vm_page_lookup(tmp_object, offset)) != VM_PAGE_NULL) {
			assert(m == (vm_page_t) queue_first(&tmp_object->memq));

			if (undo) {
				/*
				 *	Put back the page if it was originally
				 *	requested; otherwise, throw it away.
				 */

				if (was_absent) {
					vm_page_rename(m, object, offset);
					m->absent = TRUE;
				} else
					VM_PAGE_FREE(m);

			} else if (m->fictitious) {
				VM_PAGE_FREE(m);
			} else {
				/*
				 *	Put the real page back
				 */

				vm_page_rename(m, object, offset);

				/*
				 *	If the page was explicitly requested,
				 *	it wasn't made eligible for pageout in
				 *	the first pass, so it must be now.
				 */

				if (was_absent) {
					m->busy = FALSE;
					vm_page_lock_queues();
					if (!m->active)
						vm_page_activate(m);
					vm_page_unlock_queues();
				} else {
					vm_page_lock_queues();
					assert(m->active || m->inactive);
					vm_page_unlock_queues();
					assert(!m->busy && !m->absent);
				}

				/*
				 *	Set the page parameters.
				 */

				m->page_lock = lock_value;
				m->unlock_request = VM_PROT_NONE;

				PAGE_WAKEUP(m);
			}
		}
	}
	assert(queue_empty(&tmp_object->memq));
	vm_object_unlock(tmp_object);

	/*
	 *	Release all of the absent_count references at once.
	 */

	if (absent_count && !undo) {
		object->absent_count -= absent_count;
		vm_object_wakeup(object, VM_OBJECT_EVENT_ABSENT_COUNT);
	}

	vm_object_paging_end(object);

	vm_object_unlock(object);

	vm_object_deallocate(object);

#if	!MACH_XP_FPD
	if (kernel_data_cnt != 0)
		vm_deallocate(ipc_kernel_map, kernel_data, kernel_data_cnt);
#endif	!MACH_XP_FPD

	return(result);
}

kern_return_t pager_data_provided_inline(object, offset, data, lock_value)
	vm_object_t	object;
	vm_offset_t	offset;
	vm_page_data_t	data;
	vm_prot_t	lock_value;
{
#ifdef	lint
	offset++; data[0]++; lock_value++;
#endif	lint

	uprintf("pager_data_provided_inline: no longer supported -- use");
	uprintf(" memory_object_data_provided instead... trust us\n");
	
	vm_object_deallocate(object);
	return(KERN_FAILURE);
}

kern_return_t memory_object_data_error(object, offset, size, error_value)
	vm_object_t	object;
	vm_offset_t	offset;
	vm_size_t	size;
	kern_return_t	error_value;
{
	XPR(XPR_MEMORY_OBJECT, ("memory_object_data_error, object 0x%x, offset 0x%x",
				object, offset, 0, 0));
	if (object == VM_OBJECT_NULL)
		return(KERN_INVALID_ARGUMENT);

	if (size != round_page(size))
		return(KERN_INVALID_ARGUMENT);

#if	defined(hc) || defined(lint)
	/* Error value is ignored at this time */
	error_value++;
#endif	defined(hc) || defined(lint)

	vm_object_lock(object);
	offset -= object->paging_offset;

	while (size != 0) {
		register vm_page_t m;

		if (((m = vm_page_lookup(object, offset)) != VM_PAGE_NULL)) {
			if (m->absent) {
				if (m->fictitious) {
					VM_PAGE_FREE(m);
				} else {
					m->busy = FALSE;
					m->absent = FALSE;
					m->error = TRUE;
					vm_object_absent_release(object);
					PAGE_WAKEUP(m);

					vm_page_lock_queues();
					if (!m->active)
						vm_page_activate(m);
					vm_page_unlock_queues();
				}
			}
		}
		size -= PAGE_SIZE;
		offset += PAGE_SIZE;
	 }
	vm_object_unlock(object);

	vm_object_deallocate(object);
	return(KERN_SUCCESS);
}

kern_return_t memory_object_data_unavailable(object, offset, size)
	vm_object_t	object;
	vm_offset_t	offset;
	vm_size_t	size;
{
	XPR(XPR_MEMORY_OBJECT, ("memory_object_data_unavailable, object 0x%x, offset 0x%x",
				object, offset, 0, 0));

	if (object == VM_OBJECT_NULL)
		return(KERN_INVALID_ARGUMENT);

	if (size != round_page(size))
		return(KERN_INVALID_ARGUMENT);

/*
	if (!object->temporary) {
		uprintf("memory_object_data_unavailable: called on a permanent object -- converted to memory_object_data_error\n");
		return(memory_object_data_error(object, offset, size, KERN_SUCCESS));
	}
*/
	vm_object_lock(object);
	offset -= object->paging_offset;

	while (size != 0) {
		register vm_page_t m;

		if (((m = vm_page_lookup(object, offset)) != VM_PAGE_NULL)) {
			/*
			 *	We're looking for pages that are both busy and
			 *	absent (waiting to be filled), converting them
			 *	to just absent.
			 *
			 *	Pages that are just busy can be ignored entirely.
			 */
			if (m->busy && m->absent) {
				if (m->fictitious) {
					VM_PAGE_FREE(m);
				}
				else {
					PAGE_WAKEUP_DONE(m);
					vm_page_lock_queues();
					if (!m->active)
						vm_page_activate(m);
					vm_page_unlock_queues();
				}
			}
		}
		size -= PAGE_SIZE;
		offset += PAGE_SIZE;
	}

	vm_object_unlock(object);

	vm_object_deallocate(object);
	return(KERN_SUCCESS);
}

/*
 *	Routine:	memory_object_lock_page
 *
 *	Description:
 *		Perform the appropriate lock operations on the
 *		given page.  See the description of
 *		"memory_object_lock_request" for the meanings
 *		of the arguments.
 *
 *		Returns an indication that the operation
 *		completed, blocked, or that the page must
 *		be cleaned.
 */
typedef	int		memory_object_lock_result_t;
#define	MEMORY_OBJECT_LOCK_RESULT_DONE		0
#define	MEMORY_OBJECT_LOCK_RESULT_MUST_BLOCK	1
#define	MEMORY_OBJECT_LOCK_RESULT_MUST_CLEAN	2

memory_object_lock_result_t memory_object_lock_page(m, should_clean, should_flush, prot)
	vm_page_t	m;
	boolean_t	should_clean;
	boolean_t	should_flush;
	vm_prot_t	prot;
{
	vm_object_t	object = m->object;

	/*
	 *	Don't worry about pages for which the kernel
	 *	does not have any data.
	 */

	if (m->absent)
		return(MEMORY_OBJECT_LOCK_RESULT_DONE);

	/*
	 *	If we cannot change access to the page,
	 *	either because a mapping is in progress
	 *	(busy page) or because a mapping has been
	 *	wired, then give up.
	 */

	if (m->busy || (m->wire_count != 0)) {
		/*
		 *	If no change would take place
		 *	anyway, return successfully.
		 */

		if (!should_flush &&
		    (m->page_lock == prot)) {
			if (!should_clean ||
			    ((prot & VM_PROT_WRITE) &&
			     !m->dirty &&
			     !pmap_is_modified(m->phys_addr)
			    )
			   ) {
				/*
				 *	Restart page unlock requests,
				 *	even though no change took place.
				 *	[Memory managers may be expecting
				 *	to see new requests.]
				 */
				m->unlock_request = VM_PROT_NONE;
				PAGE_WAKEUP(m);

				return(MEMORY_OBJECT_LOCK_RESULT_DONE);
			}
		}

		return(MEMORY_OBJECT_LOCK_RESULT_MUST_BLOCK);
	}

	/*
	 *	If the page is to be flushed, allow
	 *	that to be done as part of the protection.
	 */

	if (should_flush)
		prot = VM_PROT_ALL;

	/*
	 *	Set the page lock.
	 *
	 *	If we are decreasing permission, do it now;
	 *	else, let the fault handler take care of it.
	 */

	if ((m->page_lock ^ prot) & prot)
		pmap_page_protect(m->phys_addr, VM_PROT_ALL & ~prot);
	m->page_lock = prot;


	/*
	 *	Restart any past unlock requests, even if no
	 *	change was made.
	 */

	m->unlock_request = VM_PROT_NONE;
	PAGE_WAKEUP(m);

	/*
	 *	Handle cleaning.
	 */

	if (should_clean) {
		/*
		 *	Check whether the page is dirty.  If
		 *	write permission has not been removed,
		 *	this may have unpredictable results.
		 */

		if (!m->dirty)
			m->dirty = pmap_is_modified(m->phys_addr);

		if (m->dirty) {
			/*
			 *	If we weren't planning
			 *	to flush the page anyway,
			 *	we may need to remove the
			 *	page from the pageout
			 *	system and from physical
			 *	maps now.
			 */

			vm_page_lock_queues();
			VM_PAGE_QUEUES_REMOVE(m);
			vm_page_unlock_queues();

			if (!should_flush)
				pmap_page_protect(m->phys_addr,
						VM_PROT_NONE);

			/*
			 *	Cleaning a page will cause
			 *	it to be flushed.
			 */

			return(MEMORY_OBJECT_LOCK_RESULT_MUST_CLEAN);
		}
	}

	/*
	 *	Handle flushing
	 */

	if (should_flush)
		VM_PAGE_FREE(m);

	return(MEMORY_OBJECT_LOCK_RESULT_DONE);
}

/*
 *	Routine:	memory_object_lock_request [user interface]
 *
 *	Description:
 *		Control use of the data associated with the given
 *		memory object.  For each page in the given range,
 *		perform the following operations, in order:
 *			1)  restrict access to the page (disallow
 *			    forms specified by "prot");
 *			2)  write back modifications (if "should_clean"
 *			    is asserted, and the page is dirty); and,
 *			3)  flush the cached copy (if "should_flush"
 *			    is asserted).
 *		The set of pages is defined by a starting offset
 *		("offset") and size ("size").  Only pages with the
 *		same page alignment as the starting offset are
 *		considered.
 *
 *		A single acknowledgement is sent (to the "reply_to"
 *		port) when these actions are complete.
 */
kern_return_t	memory_object_lock_request(object, offset, size, should_clean, should_flush, prot, reply_to)
	register
	vm_object_t	object;
	register
	vm_offset_t	offset;
	register
	vm_size_t	size;
	boolean_t	should_clean;
	boolean_t	should_flush;
	vm_prot_t	prot;
	port_t		reply_to;
{
	register
	vm_page_t	m;
	vm_offset_t	original_offset = offset;
	vm_size_t	original_size = size;
	queue_head_t	dirty_list;

	XPR(XPR_MEMORY_OBJECT,
("memory_object_lock_request, object 0x%x, offset 0x%x clean|flush %x prot %d",
	object, offset, (should_clean << 16) | should_flush, prot));
	/*
	 *	Check for bogus arguments
	 */

	if (object == VM_OBJECT_NULL)
		return(KERN_FAILURE);

	if ((prot & ~VM_PROT_ALL) != 0)
		return(KERN_INVALID_ARGUMENT);

	size = round_page(size);

	vm_object_lock(object);
	offset -= object->paging_offset;

	if (atop(size) > object->resident_page_count) {
		/* XXX
		 *	Should search differently!
		 *	Must be careful to preserve ordering appearance.
		 */;
	}
		
	/*
	 *	To avoid blocking while scanning for pages, save
	 *	dirty pages to be cleaned all at once.
	 *
	 *	XXXO A similar strategy could be used to limit the
	 *	number of times that a scan must be restarted for
	 *	other reasons.  Those pages that would require blocking
	 *	could be temporarily collected in another list, or
	 *	their offsets could be recorded in a small array.
	 */

	queue_init(&dirty_list);
#define	CLEAN_DIRTY_PAGES					\
	MACRO_BEGIN						\
	for (							\
	    m = (vm_page_t) queue_first(&dirty_list);		\
	    !queue_end(&dirty_list, (queue_entry_t) m);		\
	    m = (vm_page_t) queue_first(&dirty_list)		\
	    ) {							\
		queue_remove(&dirty_list, m, vm_page_t, pageq);	\
		vm_pageout_page(m, FALSE);			\
	}							\
	MACRO_END

	/*
	 */

	for (; size != 0; size -= PAGE_SIZE, offset += PAGE_SIZE) {

		while ((m = vm_page_lookup(object, offset)) != VM_PAGE_NULL) {
			switch (memory_object_lock_page(
					m,
					should_clean,
					should_flush,
					prot)) {

			case MEMORY_OBJECT_LOCK_RESULT_DONE:
				break;
			case MEMORY_OBJECT_LOCK_RESULT_MUST_BLOCK:
				/*
				 *	Since it is necessary to
				 *	block, clean any dirty
				 *	pages now.
				 */

				if (!queue_empty(&dirty_list)) {
					CLEAN_DIRTY_PAGES;
					continue;
				}

				PAGE_ASSERT_WAIT(m, FALSE);
				vm_object_unlock(object);
				thread_block();
				vm_object_lock(object);
				continue;
			case MEMORY_OBJECT_LOCK_RESULT_MUST_CLEAN:
				queue_enter(&dirty_list, m, vm_page_t, pageq);
				break;
			}
			break;
		}
	}

	/*
	 *	We have completed the scan for applicable pages.
	 *	Clean any pages that have been saved.
	 */

	CLEAN_DIRTY_PAGES;

	if (reply_to != PORT_NULL) {
		/*
		 *	Prevent the control port from being destroyed
		 *	while we're making the completed call.
		 */

		vm_object_paging_begin(object);
		vm_object_unlock(object);

		(void) memory_object_lock_completed(reply_to, object->pager_request, original_offset, original_size);

		vm_object_lock(object);
		vm_object_paging_end(object);
	}

	vm_object_unlock(object);
	vm_object_deallocate(object);
	return(KERN_SUCCESS);
}



kern_return_t	memory_object_set_attributes(object, object_ready,
						may_cache, copy_strategy)
	vm_object_t	object;
	boolean_t	object_ready;
	boolean_t	may_cache;
	memory_object_copy_strategy_t copy_strategy;
{
	if (object == VM_OBJECT_NULL)
		return(KERN_INVALID_ARGUMENT);

	/*
	 *	Verify the attributes of importance
	 */

	switch(copy_strategy) {
		case MEMORY_OBJECT_COPY_NONE:
		case MEMORY_OBJECT_COPY_CALL:
		case MEMORY_OBJECT_COPY_DELAY:
			break;
		default:
			vm_object_deallocate(object);
			return(KERN_INVALID_ARGUMENT);
	}

	if (object_ready)
		object_ready = TRUE;
	if (may_cache)
		may_cache = TRUE;

	vm_object_lock(object);

	/*
	 *	Wake up anyone waiting for the ready attribute
	 *	to become asserted.
	 */

	if (object_ready && !object->pager_ready)
		vm_object_wakeup(object, VM_OBJECT_EVENT_PAGER_READY);

	/*
	 *	Copy the attributes
	 */

	object->can_persist = may_cache;
	object->pager_ready = object_ready;
	object->copy_strategy = copy_strategy;

	vm_object_unlock(object);

	vm_object_deallocate(object);

	return(KERN_SUCCESS);
}

kern_return_t	memory_object_get_attributes(object, object_ready,
						may_cache, copy_strategy)
	vm_object_t	object;
	boolean_t	*object_ready;
	boolean_t	*may_cache;
	memory_object_copy_strategy_t *copy_strategy;
{
	if (object == VM_OBJECT_NULL)
		return(KERN_INVALID_ARGUMENT);

	vm_object_lock(object);
	*may_cache = object->can_persist;
	*object_ready = object->pager_ready;
	*copy_strategy = object->copy_strategy;
	vm_object_unlock(object);

	vm_object_deallocate(object);

	return(KERN_SUCCESS);
}

kern_return_t	vm_set_default_memory_manager(host, default_manager)
	task_t		host;
	memory_object_t	*default_manager;
{
	memory_object_t	old_DMM;

#ifdef	lint
	host++;
#endif	lint

	/*
	 *	XXX Until there is a privileged host port, we'll receive
	 *	this call on any task port, but only actually do
	 *	anything in privileged *threads*.
	 */

	if (!current_thread()->vm_privilege) {
		*default_manager = PORT_NULL;
		return(KERN_INVALID_ARGUMENT);
	}

	simple_lock(&memory_manager_default_lock);

	/*
	 *	Stash the old value, and take a reference
	 *	for our return value.
	 */

	if ((old_DMM = memory_manager_default) != PORT_NULL)
		port_reference((kern_port_t) old_DMM);
	
	/*
	 *	Record the new value permanently, trading
	 *	references as necessary.
	 *
	 *	Note that it is considered valid to make this
	 *	call with (default_manager == PORT_NULL) merely
	 *	to retrieve the current value.
	 */

	if (*default_manager != PORT_NULL) {
		memory_manager_default = *default_manager;
		port_reference((kern_port_t) memory_manager_default);

		if (old_DMM != PORT_NULL)
			port_release((kern_port_t) old_DMM);

		/*
		 *	In case anyone's been waiting for a memory
		 *	manager to be established, wake them up.
		 */

		thread_wakeup((vm_offset_t)&memory_manager_default);
	}

	simple_unlock(&memory_manager_default_lock);

	*default_manager = old_DMM;

	return(KERN_SUCCESS);
}

port_t		memory_manager_default_reference()
{
	memory_object_t	DMM;

	simple_lock(&memory_manager_default_lock);
	while ((DMM = memory_manager_default) == PORT_NULL)  {
		thread_sleep((vm_offset_t)&memory_manager_default,
			simple_lock_addr(memory_manager_default_lock),
			FALSE);
		simple_lock(&memory_manager_default_lock);
	}

	port_reference(DMM);
	simple_unlock(&memory_manager_default_lock);

	return(DMM);
}

void		memory_manager_default_init()
{
	memory_manager_default = PORT_NULL;
	simple_lock_init(&memory_manager_default_lock);
}
