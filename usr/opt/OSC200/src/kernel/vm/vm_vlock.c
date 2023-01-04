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
static char *rcsid = "@(#)$RCSfile: vm_vlock.c,v $ $Revision: 1.1.3.2 $ (DEC) $Date: 1992/03/19 08:56:49 $";
#endif
#include <kern/assert.h>
#include <vm/vm_map.h>
#include <vm/vm_umap.h>
#include <vm/vm_page.h>
#include <kern/zalloc.h>
#include <mach/mach_types.h>
#include <mach/kern_return.h>
#include <vm/vm_vlock.h>
#include <vm/vm_tune.h>

/*
 * This routine supports fast kernel locking of user virtual address space.
 * It is not intended for use by user mode locking primitives.
 * The primary advantage of maintaining locking state here is to
 * keep it consistent without having additional state in the map entry
 * handler routines to administer transient locking conditions such
 * as these.
 *
 */

zone_t vl_zone;
extern int physmem;

void
vl_init()
{
	vl_zone = zinit((vm_size_t) sizeof(struct vm_vlock),
				sizeof(struct vm_vlock) * physmem,
				PAGE_SIZE, "vlock map");
}

/*
 * Wirings are grouped by the wiring count.
 * So if a user does strange physical I/Os
 * and or the kernel does stranges wirings
 * then this becomes an expensive operation.
 * Note it was previously expensive but with
 * a persistent cost.
 *
 */

#define	vl_clip_start(LP, START) {				\
	if ((LP)->vl_start < (START)) {				\
		ZALLOC(vl_zone, nlp, struct vm_vlock *);	\
		*nlp = *lp;					\
		nlp->vl_end = (START);				\
		(LP)->vl_start = (START);			\
		if (up->um_vlock == (LP)) up->um_vlock = nlp;	\
		nlp->vl_next = (LP);				\
		nlp->vl_prev = (LP)->vl_prev;			\
		(LP)->vl_prev->vl_next = nlp;			\
		(LP)->vl_prev = nlp;				\
	}							\
}

#define vl_clip_end(LP, END) {					\
	if ((END) < (LP)->vl_end) {				\
		ZALLOC(vl_zone, nlp, struct vm_vlock *);	\
		*nlp = *(LP);					\
		nlp->vl_start = (END);				\
		(LP)->vl_end = (END);				\
		nlp->vl_next = (LP)->vl_next;			\
		nlp->vl_prev = (LP);				\
		(LP)->vl_next->vl_prev = nlp;			\
		(LP)->vl_next = nlp;				\
	}							\
}
		

		
kern_return_t
vl_wire(register vm_map_entry_t ep,
	vm_offset_t start,
	register vm_offset_t end,
	vm_prot_t prot)
{
	register struct u_map_private *up = 
			(struct u_map_private *) (ep->vme_map->vm_private);
	register struct vm_vlock *lp, *nlp;
	register vm_size_t size;
	register vm_offset_t first;
	kern_return_t ret;

	first = start;	
	if ((lp = up->um_vlock) == (struct vm_vlock *) 0) {
		if ((ret = (*ep->vme_fault)(ep, first, end - first, prot, 
				VM_WIRE, (vm_page_t *) 0))
			!= KERN_SUCCESS) return ret;
		ZALLOC(vl_zone, lp, struct vm_vlock *);
		up->um_vlock = lp;
		lp->vl_next = lp->vl_prev = lp;
		lp->vl_start = start;
		lp->vl_end = end;
		lp->vl_count = 1;
		return KERN_SUCCESS;
	}
	else {
		register struct vm_vlock *nlp;

		do {

			/*
			 * Contained within this entry.
			 * Is entry a cached one or not.
			 */

			if (first >= lp->vl_start && first < lp->vl_end) {
				vl_clip_start(lp, first);
				vl_clip_end(lp, end);
				lp->vl_count++;
				if (lp->vl_end == end) goto done;
				else first = lp->vl_end;
			}

			/*
			 * Starts before this entry
			 */

			else if (first <  lp->vl_start) {

				/*
				 * New entry required.
				 */
				
				size = MIN(lp->vl_start, end) - first;
				if ((ret = (*ep->vme_fault)(ep, first, size,
					prot, VM_WIRE, (vm_page_t *) 0)) 
					!= KERN_SUCCESS) 
						goto failed;

				ZALLOC(vl_zone, nlp, struct vm_vlock *);
				nlp->vl_start = first;
				nlp->vl_end = first + size;
				nlp->vl_count = 1;

				if (up->um_vlock == lp) up->um_vlock = nlp;
				nlp->vl_next = lp;
				nlp->vl_prev = lp->vl_prev;
				lp->vl_prev->vl_next = nlp;
				lp->vl_prev = nlp;
	
				if (size == (end - first)) goto done;
				else {
					first = nlp->vl_end;
					continue;
				}
			}
			lp = lp->vl_next;
		} while (lp != up->um_vlock);


		/*
		 * New entry at tail of list.
		 */

		if ((ret = (*ep->vme_fault)
			(ep, first, end - first, prot, 
			VM_WIRE, (vm_page_t *) 0))
				!= KERN_SUCCESS) goto failed;
		ZALLOC(vl_zone, nlp, struct vm_vlock *);
		nlp->vl_start = first;
		nlp->vl_end = end;
		nlp->vl_count = 1;
		nlp->vl_next = lp;
		nlp->vl_prev = lp->vl_prev;
		lp->vl_prev->vl_next = nlp;
		lp->vl_prev = nlp;
	}
done:
	return KERN_SUCCESS;
failed:
	vl_unwire(ep, start, first);	
	return ret;
}

kern_return_t
vl_unwire(register vm_map_entry_t ep,
	register vm_offset_t start,
	register vm_offset_t end)
{
	register struct u_map_private *up = 
			(struct u_map_private *) (ep->vme_map->vm_private);
	register struct vm_vlock *lp, *nlp;
	register vm_size_t size;

	lp = up->um_vlock;
	while (start < end) {
		if (start >= lp->vl_start && start < lp->vl_end) {
			vl_clip_start(lp, start);
			vl_clip_end(lp, end);
			lp->vl_count--;
			size = lp->vl_end - start;
			nlp = lp->vl_next;

			/*
			 * Unload translations
			 */

			if (!lp->vl_count) {
				if ((*ep->vme_fault)(ep, start, size,
					VM_PROT_NONE, VM_UNWIRE, 
					(vm_page_t *) 0) != KERN_SUCCESS)
						panic("vl_unwire: failed");
				if (up->um_vlock == lp) {
					if (lp->vl_next != lp) 
						up->um_vlock = lp->vl_next;
					else {
						if ((start + size) != end)
							panic("vl_unwire: bad");
						up->um_vlock = 
							(struct vm_vlock *) 0;
						ZFREE(vl_zone, lp);
						break;
					}
				} 
				lp->vl_next->vl_prev = lp->vl_prev;
				lp->vl_prev->vl_next = lp->vl_next;
				ZFREE(vl_zone, lp);
			}
			start += size;
			lp = nlp;
		}
		else lp = lp->vl_next;
		assert((lp != up->um_vlock));
	}
	return KERN_SUCCESS;
}

/*
 * Is this virtual address range 
 * wired at any single page.
 */

boolean_t
vl_kwire(register vm_map_entry_t ep,
	vm_offset_t start,
	vm_offset_t end)
{
	register struct u_map_private *up = 
			(struct u_map_private *) (ep->vme_map->vm_private);
	register struct vm_vlock *lp;
	boolean_t found;

	found = FALSE;
	lp = up->um_vlock;
	if (lp) do {

		/*
		 * Further out on list
		 */

		if (start >= lp->vl_end) lp = lp->vl_next;

		/*
		 * Before anything on the list.
		 */

		else if (end <= lp->vl_start) break;

		/*
		 * An intersection was found.
		 */

		else {
			if (lp->vl_count) {	
				found = TRUE;
				break;
			}
			else lp = lp->vl_next;
		}

	} while (lp != up->um_vlock);

	return found;
}

/*
 * Its fatal to discover an active vl
 */

void
vl_remove(register vm_map_t map)
{
	register struct u_map_private *up = 
			(struct u_map_private *) (map->vm_private);
	
	if (up->um_vlock) panic("vl_remove: vl active");
}

