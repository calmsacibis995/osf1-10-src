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
static char *rcsid = "@(#)$RCSfile: vm_pagelru.c,v $ $Revision: 1.1.24.6 $ (DEC) $Date: 1994/01/14 18:51:53 $";
#endif
#include <kern/task.h>
#include <kern/task_swap.h>
#include <sys/types.h>
#include <vm/vm_map.h>
#include <vm/vm_page.h>
#include <vm/vm_vppage.h>
#include <vm/vm_pagelru.h>
#include <vm/vm_object.h>
#include <vm/vm_anon.h>
#include <vm/vm_swap.h>
#include <mach/vm_prot.h>
#include <mach/vm_statistics.h>
#include <kern/zalloc.h>
#include <machine/machparam.h>
#include <vm/pmap.h>
#include <kern/xpr.h>
#include <kern/thread.h>
#include <sys/kernel.h>
#include <vm/vm_perf.h>
#include <sys/dk.h>	/* for SAR counters */
#include <kern/processor.h>

/*
 * This should go in mips specific parameter file
 * and/or we should compute it in vm_machdep.c and set the
 * values.
 */

#define	VM_PAGEOUT_BURST_MAX	20
#define	VM_PAGE_INACTIVE_TARGET(C)	(((C) * 2) / 3)
#define	VM_PAGE_SCAN_RATE	200

#define	nz(VAL)		((VAL) ? (VAL) : 1)

int	vm_page_free_target = 0;
int	vm_page_scans, ubc_page_scans;
int	vm_page_scan_rate = 0;
int	vm_page_free_optimal;
int	vm_page_free_min = 0;
int	vm_page_inactive_target = 0;
int	vm_page_free_reserved = 0;
int	vm_pageout_burst_max = 0;

int vm_page_stealer_last_failure_stamp = 0;
int vm_page_inactive_list_stamp = 0;
boolean_t vm_page_stealer_last_try_succeeded = TRUE;

vm_page_t vm_page_queue_active;
vm_page_t vm_page_queue_inactive;

decl_simple_lock_data(,vm_page_queue_lock)
int	vm_page_active_count;
int	vm_page_inactive_count;
int	vm_page_wire_count;
decl_simple_lock_data(,vm_page_wire_count_lock)


extern vm_size_t vm_swap_space;
extern int vm_managed_pages;
int vm_scan_pages;

/*
 * Memory use averages and I/O rates.
 */

extern long ubc_minpages, ubc_pages;
extern vm_size_t vm_swap_space;

extern struct buf *swdone;
extern struct buf *ubc_iodone_buf;

unsigned long vmlfree_ave;

#define	VM_EVENTS_ON 1

#if	VM_EVENTS_ON
#define	VM_EVENTS 9
int vm_number_events = VM_EVENTS;
struct vm_events {
	long ve_count;
	char ve_name[30];
} vm_events[VM_EVENTS] = {
{0, "OOP_PAGEOUT push"},
{0, "OOP_PAGEOUT calls"},
{0, "pages scanned"},
{0, "active pages examined"},
{0, "active pages deactivated"},
{0, "vm_pageout_scan"},
{0, "vm target met"},
{0, "vm I/O met"},
{0, "no swap bufs"},
};

#define	event_pageout		vm_events[0].ve_count++
#define	event_pageout_call	vm_events[1].ve_count++
#define	event_pages_scanned	vm_events[2].ve_count++
#define	event_active_scanned	vm_events[3].ve_count++
#define event_active_moved	vm_events[4].ve_count++
#define	event_vm_pageout_scan	vm_events[5].ve_count++
#define	event_vm_target		vm_events[6].ve_count++
#define	event_vm_burst		vm_events[7].ve_count++
#define	event_empty		vm_events[8].ve_count++

#else

#define	event_pageout		
#define	event_pageout_call	
#define	event_pages_scanned	
#define	event_active_scanned	
#define event_active_moved	
#define	event_vm_pageout_scan
#define	event_vm_target
#define	event_vm_burst
#define	event_empty

#endif	/* VM_EVENTS_ON */

#define	VMSCHED_TICKS	4			/* 250 milliseconds */
int vmsched_ticks = 0;

vm_pageout_init()
{
	simple_lock_init(&vm_page_queue_lock);
	simple_lock_init(&vm_page_wire_count_lock);
	vm_page_queue_active = vm_page_queue_inactive = (vm_page_t) 0;
}


/*
 *	vm_page_deactivate:
 *	The page queues must be locked.
 *
 *	This routine removes an active page from the free list and puts it on the
 *	end of the inactive list.  At the same time it clears the reference bit 
 *	so that the page stealing demon will know if it has been used by the time it 
 *	gets worked to the head of the inactive list.
 */

void 
vm_page_deactivate(register vm_page_t pp)
{

	if (pg_state(pp) == PG_ACTIVE) {
		pmap_clear_reference(page_to_phys(pp));
		pgl_remove(vm_page_queue_active,pp,p);
		pgl_insert_tail(vm_page_queue_inactive,pp,p);
		pg_setstate(pp,PG_INACTIVE);
		vm_page_active_count--;
		vm_page_inactive_count++;
		vm_page_inactive_list_stamp++;
	}
}


/*
 *	vm_page_activate:
 *	The page queues must be locked.
 *
 *	This routine removes an inactive page from the inactive list and puts it in the tail
 *	of the active list unless the page is wired somewhere.
 */

void 
vm_page_activate(register vm_page_t pp)
{

	if (pg_state(pp) == PG_INACTIVE) {
		pgl_remove(vm_page_queue_inactive,pp,p);
		vm_page_inactive_count--;
		pg_clearstate(pp);
	}
	if (pp->pg_wire_count == 0) {
		if (pg_state(pp) == PG_ACTIVE)
			panic("vm_page_activate: already active");

		pgl_insert_tail(vm_page_queue_active,pp,p);
		pp->pg_reserved |= PG_ACTIVE;
		vm_page_active_count++;
	}
}



/*
 *	Main pageout loop.
 *
 *	This is the main loop for the vm_pageout kernel thread that gets started in init_main.c.
 *	It sleeps until the number of pages on the free list falls below vm_page_free_target at
 *	which time vm_pg_alloc() wakes it up.  After the wakeup, this loop calculated the number
 *	of inactive pages that should be scanned.  Next it determines if the UBC has more than its
 *	fair share of pages and if so calls ubc_memory_purge() specifying the number of pages it 
 *	should free.  Next it calls vm_pageout_scan() specify how many inactive pages should be 
 *	scanned.  
 */

vm_pageout_loop()
{
	register int s;
	register int page_shortage, vm_burst, ubc_burst, burst;
	register int free, scans;


	for (;;)  {
		s = splimp();
		simple_lock(&vm_page_free_lock);
		assert_wait((vm_offset_t)&vm_page_free_wanted, FALSE);
		simple_unlock(&vm_page_free_lock);
		(void) splx(s);
		thread_block();

		s = splimp();
		simple_lock(&vm_page_free_lock);
		free = vm_page_free_count;
		simple_unlock(&vm_page_free_lock);
		(void) splx(s);

		pmap_update();

		scans = nz((ubc_page_scans + vm_page_scans));
		burst = vm_pageout_burst_max / vmsched_ticks;
		vm_burst = (burst * vm_page_scans) / scans;
		ubc_burst = burst - vm_burst;

		if (ubc_pages > (ubc_minpages + 
			vpf_cload(ubchit) + vpf_cload(ubcalloc))) { 
			
			page_shortage = ((vm_page_free_target - free) * 
				ubc_page_scans) / scans;
			page_shortage = MAX(page_shortage, 1);
			ubc_memory_purge(ubc_burst, page_shortage, 
				ubc_page_scans); 
		}


		vm_pageout_scan(vm_burst);

	}
}

/*
 *	vm_pageout_scan():
 *
 *	This routine is the VM work loop of the vm_pageout kernel thread also know as the 
 *	page stealing demon.  This routine loops, taking pages off the head of the inactive
 *	list and evaluating them.  referenced and held pages are reactivated(put on the tail 
 *	of the active list) for another ride through the active list then inactive list.  
 *	Pages which are in a locked object are put in the tail of the inactive list and
 *	get another ride through the inactive list.  At this point the page has not been 
 *	referenced and is either clean or dirty.   A clean page is an identical copy of the
 *	swap cell that backs that page therefore its translation can be unloaded and it can
 *	simply be freed.  A dirty page must be written out to the swap cell before it can
 *	be freed.  In either case it is the pageout operation handler that unloads the 
 *	translation and either frees or writes the page.
 */
		
vm_pageout_scan(register int vm_burst)
{
	register int s, free, held_pages, nscans, inactive, modified;
	register int page_shortage, pages_moved, page_outs, page_push;
	register vm_object_t object;
	register vm_page_t pp;

        /* global table() system call counter (see table.h) */
        pg_v_pgout++;

	event_vm_pageout_scan;
	vm_page_lock_queues();
	for (nscans = vm_page_scans, page_outs = 0, 
		inactive = vm_page_inactive_count; 
		inactive && nscans; inactive--, nscans--) {

			/* stop if inactive list page list is empty */

		if ((pp = vm_page_queue_inactive) == VM_PAGE_NULL) 
			break;

		s = splimp();
		simple_lock(&vm_page_free_lock);
		free = vm_page_free_count;
		simple_unlock(&vm_page_free_lock);
		splx(s);

			/* stop if we got the free list up to the target */

		if (free >= vm_page_free_target) {
			event_vm_target;
			break;
		}
		else if (vm_burst && (page_outs > vm_burst)) {

				/* last pass for now if the IO is up to vm_burst */

			event_vm_burst;
			vm_burst = 0;
		}

		event_pages_scanned;
        	/*
		 * Global vm_statistics() system call counter
		 * (see vm_statistics.h).
		 */
		pg_v_scan++;

		 	/* reactivate the page if it has been referenced since last deactivation */

		if (pmap_is_referenced(page_to_phys(pp))) {
			vm_page_activate(pp);
			vpf_ladd(reactivate,1);
			continue;
		}
		else 
			/* remember if the page was modified so it get written out to swap */
			modified = pmap_is_modified(page_to_phys(pp));


		object = pp->pg_object;

		 	/* if the I/O burst is too high and page is modified put on back of inactive list */

		if (!vm_burst && modified) {
			pgl_remove(vm_page_queue_inactive,pp,p);
			pgl_insert_tail(vm_page_queue_inactive,pp,p);
			continue;
		}
			/* if IO is below max burst try to take the object lock */
		else if (!OOP_KLOCK_TRY(object, pp->pg_offset)) {

			 	/* if cant get lock, move page to end of inactive list and continue */

			pgl_remove(vm_page_queue_inactive,pp,p);
			pgl_insert_tail(vm_page_queue_inactive,pp,p);
			continue;
		}

		vm_page_lock(pp);

			/* if there is a hold against page, reactivate it because someone wants it */

		if (pp->pg_hold) {
			pgl_remove(vm_page_queue_inactive,pp,p);
			vm_page_inactive_count--;
			pgl_insert_tail(vm_page_queue_active,pp,p);
			vm_page_active_count++;
			pg_setstate(pp,PG_ACTIVE);
			vm_page_unlock(pp);
			OOP_UNKLOCK(pp->pg_object, pp->pg_offset);
			continue;	
		}

			/* at this point, the page will be reclaimed, inactivate it and get ready to push out */

		pgl_remove(vm_page_queue_inactive,pp,p);
		vm_page_inactive_count--;
		pg_clearstate(pp);
		vm_page_unlock(pp);
		vm_page_unlock_queues();

		event_pageout_call;

			/* push out the page(and potentially others).  If its not modified it will simply get freed */

		page_push = OOP_PAGEOUT(object, pp->pg_offset, 
			MIN((vm_page_free_target - free),
			MAX(vm_burst, PAGE_SIZE)));
		if (page_push) event_pageout;
		page_outs += page_push;
		vm_page_lock_queues();
		inactive = vm_page_inactive_count + 1;

	}	/* end of the pageout scan loop.  terminated when free target is reached, IO burst max, no inactive pages */

	/* figure out how many active pages should be inactivated to achieve VM_PAGE_INACTIVE_TARGET */

	vm_page_inactive_target = VM_PAGE_INACTIVE_TARGET(
		vm_page_active_count + vm_page_inactive_count);

	if (vm_page_inactive_count < vm_page_inactive_target) {
		page_shortage = vm_page_inactive_target - 
			vm_page_inactive_count;	
		if (page_shortage <= 0) {
			if(page_outs == 0) page_shortage = 1;
			else page_shortage = 0;
		}
	}
	else page_shortage = 0;

	pages_moved = 0;
	held_pages = 0;
	if ((pp = vm_page_queue_active) != VM_PAGE_NULL)

	/*
	 *	At this point we have freed and paged out as many pages as we can.  Now it is time to
	 *	replenish the inactive list with the least reciently activated pages.  This loop starts
	 *	at the head of the active list(its an lru queue) removing pages which are not held 
	 *	inactivating them.  This results in pages migrating through the active list and onto the
	 *	inactive list.
	 */

	while (page_shortage > 0 && (vm_page_active_count - held_pages) > 0) {

		event_active_scanned;

        	/*
		 * Global vm_statistics() system call counter
		 * (see vm_statistics.h).
		 */
		pg_v_scan++;

		/* skip pages that are being held, someone wants then */

		vm_page_lock(pp);
		if (pp->pg_hold) {
			if (pp->pg_pnext == pp) {
				vm_page_unlock(pp);
				break;
			}
			else {
				pp = pp->pg_pnext;
				held_pages++;
				vm_page_unlock(pp);
				continue;
			}
		}

		event_active_moved;
		vm_page_deactivate(pp);
		vm_page_unlock(pp);
		pages_moved++;
		page_shortage--;
		if ((pp = vm_page_queue_active) == VM_PAGE_NULL) break;

	}	/* end of page deactivation loop, terminated when VM_PAGE_INACTIVE_TARGET is achieved or active list empty */

	vm_page_unlock_queues();

	return;
}


task_t	pageout_task;
thread_t pageout_thread;


/*
 *	vm_mem_sched():
 *
 *	This routine is called every 250 milliseconds via a timeout call.
 *	On every fourth invocation(every second) it calculates the average
 *	vm statistics and to determine if the longterm freelist average has 
 *	been pushed low enough to induce task swapping(the free list has
 *	remained below vm_page_free_min for more than 5 seconds).  
 *	On every invocation(1/4 second) it waked up the vm_pageout thread
 *	if the free list is below vm_page_free_target.  Before the wakeup it
 *	caclulates how many vm and ubc pages should be scaned by vm_pageout_scan
 *	and ubc_memory_purge respectively.  Finally it schedules another 250 
 *	millisecond wakeup for itself.
 */

#define	ave(SUM, COUNT, TIME)					\
	SUM = ((TIME - 1) * (SUM) + (COUNT))/ TIME		

vm_mem_sched()
{
	static int ticks = 0;
	register vpf_t *c, *ce, *r, *s;
	register int free;



	/*
	 * Compute average memory consumption and I/O rates once a second.
	 * Also update free swap space vm_perf value.
	 */

	if (++ticks == vmsched_ticks) {
	
			/* compute the free list average over the last 30 seconds */
		ave(vmlfree_ave, vm_page_free_count, 30);

		/* There are three identical structures that the vm/ubc uses to keep track of events,
		 * the vm_perf structure contains the numver of events that happened the last second,
		 * the vm_perfcomp structure contains the average number of events over the last 5 seconds
		 * and the vm_perfsum structure contains the total number of events that have happened.
		 * The next loops sum up the total events that have occurred in the vm_perfsum structure,
		 * calculate the 5 second averages in the vm_perfcomp structure and zero out the accumulators
		 * in the vm_perf structure to start off the next one second interval.
		 *
		 * The structure contains 3 types of information, events which are measured in rates(like
		 * pagefaults), sizes of queues(like the length of the free list) and current system wide
		 * quantities(like currently available swap space).
		 */

		 	/* Compute rates portion of the vm performance data */

		for (c = &vm_perf.vpf_first_rat, ce = &vm_perf.vpf_last_rat,
			r = &vm_perfcomp.vpf_first_rat, 
			s = &vm_perfsum.vpf_first_rat; 
			c <= ce; c++, r++, s++) {
			ave(*r, *c, 5);
			*s += *c;
			*c = 0;
		}

		 	/* Compute averages portion of the vm performance data */

		for (c = &vm_perf.vpf_first_aac, ce = &vm_perf.vpf_last_aac,
			r = &vm_perfcomp.vpf_first_aac, 
			s = &vm_perfsum.vpf_first_aac; 
			c <= ce; c++, r++, s++) {
			ave(*r, *c, 5);
			*s = *c;
		}

			/* store the quantities portion of the vm performance data */

		vpf_sstore(swapspace,vm_swap_space);

			/* swap if average free pages for last 5 seconds is less than vm_page_free_min */
		if (vpf_cload(freepages) < vm_page_free_min) 
			task_swapper_timeout(FALSE);

		ticks = 0;
	}

	free = vm_page_free_count;

	
	if (free < vm_page_free_target) {
		register int scanrate, vmpages, ubcpages, pages, pagescans;
		register int vmrate, ubcrate, c_allrates, p_allrates;
		register int vmp, ubcp;


		/*
		 * Compute memory totals.
		 */
			/* all reclaimable vm pages are active and inactive */
		vmpages = vm_page_inactive_count + vm_page_active_count;
			/* all ubc pages are reclaimable */
		ubcpages = ubc_pages;
			/* total recalimable pages from vm and ubc */
		pages = vmpages + ubcpages;


		/*
		 * Compute memory weight factors
		 */

		vmrate = vpf_cload(pgreads) + vpf_cload(cowfaults) + 
			vpf_cload(zfod);
		ubcrate = vpf_cload(ubcalloc);

		c_allrates = nz(ubcrate + vmrate);

		vmp = vpf_cload(pgwrites) + vpf_cload(reactivate); 
		ubcp = vpf_cload(ubcpagepushes) + vpf_cload(ubchit);

		p_allrates = nz(vmp + ubcp);

			

		/*
		 * Compute scan rate
		 */

		scanrate = ((vm_page_scan_rate << 1) * free + 
			vm_page_scan_rate * (vm_page_free_target - free)) /
			vm_page_free_target;

		/*
		 * Compute total pages to examine
		 */

		pagescans = (pages / nz(scanrate) / vmsched_ticks) / 3;


		/*
		 * Normalize the scans per subsystem
		 */

		ubc_page_scans = nz(((pagescans * ubcpages) / vm_scan_pages)  + 
			((pagescans * ubcrate) / c_allrates) +
			((pagescans * vmp) / p_allrates));

		vm_page_scans = nz(((pagescans * vmpages) / vm_scan_pages)  + 
			((pagescans * vmrate) / c_allrates) +
			((pagescans * ubcp) / p_allrates));


		thread_wakeup((vm_offset_t) &vm_page_free_wanted);
	}

	timeout(vm_mem_sched, (caddr_t) 0, hz/vmsched_ticks);
}

/*
 *	vm_pageout is the high level pageout daemon.
 */

void 
vm_pageout()
{
	pageout_task = current_task();
	pageout_task->kernel_vm_space = TRUE;
	pageout_task->kernel_ipc_space = TRUE;
	pageout_task->ipc_privilege = TRUE;
	pageout_thread = current_thread();
	pageout_thread->vm_privilege = TRUE;
	thread_swappable(pageout_thread, FALSE);

	(void) spl0();

	/*
	 *	Initialize some paging parameters.
	 */

#ifndef	VM_PAGEOUT_BURST_MAX
#define VM_PAGEOUT_BURST_MAX	10
#endif
	if (vm_pageout_burst_max == 0)
		vm_pageout_burst_max = VM_PAGEOUT_BURST_MAX;

#ifndef	VM_PAGE_FREE_RESERVED
#define	VM_PAGE_FREE_RESERVED			10
#endif
	if (vm_page_free_reserved == 0)
		vm_page_free_reserved = VM_PAGE_FREE_RESERVED;

	if (vm_page_free_min == 0)
		vm_page_free_min = vm_page_free_reserved * 2;

#ifndef	VM_PAGE_FREE_TARGET
#define	VM_PAGE_FREE_TARGET			128
#endif
	if (vm_page_free_target == 0) 
		vm_page_free_target = VM_PAGE_FREE_TARGET;

	vm_scan_pages = vm_managed_pages - vm_page_wire_count;
	if (vm_page_scan_rate == 0) 
		vm_page_scan_rate = vm_scan_pages / VM_PAGE_SCAN_RATE;

	vm_page_free_optimal = vm_page_free_min + 
		((vm_page_free_target - vm_page_free_min) / 2);

	/*
	 * Minimum free memory where pagein klustering is enabled.
	 */

	if (!vm_page_kluster) vm_page_kluster = vm_page_free_target * 3;


	/*
	 * Memory average/scheduler.
	 */

	if (!vmsched_ticks) vmsched_ticks = VMSCHED_TICKS;


 	if (vm_page_free_target <= vm_page_free_min)
		vm_page_free_target = vm_page_free_min + vmsched_ticks;

	vm_mem_sched();

	vm_pageout_loop();
}

/* 
 * vm_page_stealer()
 *
 *	Called from vm_pg_alloc() if the free list is less that vm_page_free_reserved
 *	so the calling thread was going to sleep for more pages and the vm has the 
 *	majority of the available pages.
 */

boolean_t
vm_page_stealer()
{

	vm_page_t pp;

	vm_page_lock_queues();

	/* if we failed last time, dont even try if if the inactive list wasnt updated since */

	if ((!vm_page_stealer_last_try_succeeded) &&
	    (vm_page_stealer_last_failure_stamp == vm_page_inactive_list_stamp)) 
		return FALSE;
	
	if (pp = vm_page_queue_inactive) {

		do {

                        if ((cpu_to_processor(cpu_number())->runq.count > 0) ||
                            (cpu_to_processor(cpu_number())->processor_set->runq.count > 0) ||
                            (ubc_iodone_buf) || (swdone)) {
                                vm_page_unlock_queues();
                                return FALSE;
                        }

			if ((!pp->pg_busy) &&
			    (!pp->pg_hold) &&
			    (!pp->pg_dirty) &&
			    (!pmap_is_referenced(page_to_phys(pp))) &&
			    (!pmap_is_modified(page_to_phys(pp))) &&
			    (pmap_page_protect(page_to_phys(pp), VM_PROT_NONE|VM_PROT_TRY) !=
                                                                              KERN_FAILURE)) {
 

				if (OOP_PAGESTEAL(pp->pg_object, pp) == KERN_SUCCESS) {

					vm_page_stealer_last_try_succeeded = TRUE;
					vm_page_unlock_queues();
					return TRUE;
				}
			}

		} while ((pp = pp->pg_pnext) != vm_page_queue_inactive);

	}

	/* tell the caller that there were no ubc pages to give up */

	vm_page_unlock_queues();
	vm_page_stealer_last_try_succeeded = FALSE;
	vm_page_stealer_last_failure_stamp = vm_page_inactive_list_stamp;
	return FALSE;

}
				

/*
 * The following are exported interfaces.
 */

vm_pageout_activate(register vm_page_t pp,
	boolean_t release)
{
	vm_page_lock_queues();
	vm_page_lock(pp);
	if (pp->pg_wire_count) panic("vm_pageout_activate: page is wired");
	pg_setstate(pp,PG_ACTIVE);
	pp->pg_busy = 0;
	if (pp->pg_wait) {
		pp->pg_wait = 0;
		thread_wakeup((vm_offset_t) pp);
	}
	if (release == TRUE) pp->pg_hold--;
	vm_page_active_count++;
	pgl_insert_tail(vm_page_queue_active,pp,p);
	vm_page_unlock(pp);
	vm_page_unlock_queues();
}

boolean_t 
vm_pageout_remove(register vm_page_t pp, 
	boolean_t hard,
	boolean_t unload)
{
	boolean_t removed;

	vm_page_lock_queues();
	vm_page_lock(pp);
	if (pp->pg_wire_count || pp->pg_busy || pp->pg_hold) removed = FALSE;
	else if (!hard && (pg_state(pp) != PG_INACTIVE)) removed = FALSE;
	else {
		if (unload == TRUE) {
			if (pmap_page_protect(page_to_phys(pp),
					      VM_PROT_NONE|VM_PROT_TRY)) {
				removed = FALSE;
				goto done;
			}
			pp->pg_dirty = pmap_is_modified(page_to_phys(pp));
		}
		pp->pg_busy = 1;
		VM_PAGE_QUEUES_REMOVE(pp);
		removed = TRUE;
	}
done:
	vm_page_unlock(pp);
	vm_page_unlock_queues();
	return removed;
}

/*
 * The pageout code called the object's pageout handler
 * after acquiring the object lock for the page.
 * However, the object pageout handler decided it couldn't
 * proceed pushing this pageout.
 */

vm_pageout_abort(register vm_page_t pp)
{
	vm_page_lock_queues();
	vm_page_lock(pp);
	pp->pg_busy = 0;
	if (pp->pg_wait) thread_wakeup((vm_offset_t) pp);
	pp->pg_wait = FALSE;
	vm_page_unlock(pp);	
	pgl_insert_tail(vm_page_queue_active,pp,p);
	pp->pg_reserved |= PG_ACTIVE;
	vm_page_active_count++;
	vpf_ladd(reactivate,1);
	vm_page_unlock_queues();
}

vm_page_t 
vm_page_clean(register vm_page_t pp)
{
  vm_page_lock_queues();
  vm_page_lock(pp);
  if (pp->pg_wire_count || pp->pg_busy || 
      pp->pg_hold || !pmap_is_modified(page_to_phys(pp))) {
    vm_page_unlock(pp);
    vm_page_unlock_queues();
    return VM_PAGE_NULL;
  }
  else if(pmap_page_protect(page_to_phys(pp), VM_PROT_NONE|VM_PROT_TRY) ==
	  KERN_SUCCESS) {
    if (pp->pg_free) panic("vm_page_clean: page on free list");
    VM_PAGE_QUEUES_REMOVE(pp);
    pp->pg_busy = 1;
    pp->pg_reserved = PG_PREWRITE;
  }
  else {
    vm_page_unlock(pp);
    vm_page_unlock_queues();
    return VM_PAGE_NULL;
  }
  
  vm_page_unlock(pp);
  vm_page_unlock_queues();
  return pp;
  
}

void
vm_page_clean_done(register vm_page_t pp)
{
	vm_page_lock_queues();
	vm_page_lock(pp);
	pp->pg_reserved = 0;
	pp->pg_busy = 0;
	if (pp->pg_wait) {
		pp->pg_wait = 0;
		thread_wakeup((vm_offset_t) pp);
	}
	if (pp->pg_hold) {
		pgl_insert_tail(vm_page_queue_active,pp,p);
	}
	else {
		pgl_insert(vm_page_queue_active,pp,p);
	}
	pp->pg_reserved = PG_ACTIVE | PG_PREWRITTEN;
	vm_page_active_count++;
	vm_page_unlock(pp);
	vm_page_unlock_queues();
}

/*
 *	vm_page_free:
 *
 *	Returns the given page to the free list,
 *	disassociating it with any VM object.
 *
 *	Object must be locked prior to entry.
 */

void 
vm_page_free(register vm_page_t pp)
{
	vm_page_lock_queues();
	vm_page_lock(pp);
	if (pp->pg_free) {
		printf("vm_page_free: duplicate free page %x\n", pp);
		panic("vm_page_free");
	}
	if (pp->pg_hold) {
		vm_page_unlock(pp);
		vm_page_unlock_queues();
		return;
	}
	vm_page_remove(pp);
	VM_PAGE_QUEUES_REMOVE(pp);
	vm_pg_free(pp);
	vm_page_unlock_queues();
}

vm_page_wire_free(register vm_page_t pp)
{
	assert(pp->pg_wire_count == 1);
	simple_lock(&vm_page_wire_count_lock);
	vm_page_wire_count--;
	vpf_store(wiredpages,vm_page_wire_count);
	simple_unlock(&vm_page_wire_count_lock);
	vm_page_remove(pp);
	vm_pg_free(pp);
}

/*
 *	vm_page_wire:
 *
 *	Mark this page as wired down by yet
 *	another map, removing it from paging queues
 *	as necessary.
 *
 *	The page's object must be locked.
 */

void 
vm_page_wire(register vm_page_t pp,
	boolean_t release)
{
	vm_page_lock_queues();

	vm_page_lock(pp);
	if (release == TRUE) pp->pg_hold--;

	if (pp->pg_wire_count == 0) {
		pp->pg_busy = 0;
		VM_PAGE_QUEUES_REMOVE(pp);
		simple_lock(&vm_page_wire_count_lock);
		vm_page_wire_count++;
		vpf_store(wiredpages,vm_page_wire_count);
		simple_unlock(&vm_page_wire_count_lock);
	}
	pp->pg_wire_count++;
	vm_page_unlock(pp);
	vm_page_unlock_queues();
}

/*
 *	vm_page_unwire:
 *
 *	Release one wiring of this page, potentially
 *	enabling it to be paged again.
 *
 *	The page's object must be locked.
 */

void 
vm_page_unwire(register vm_page_t pp,
		boolean_t activate)
{
	vm_page_lock_queues();

	if (--pp->pg_wire_count == 0) {
		simple_lock(&vm_page_wire_count_lock);
		vm_page_wire_count--;
		vpf_store(wiredpages,vm_page_wire_count);
		simple_unlock(&vm_page_wire_count_lock);
		if (activate == TRUE) {
			pgl_insert_tail(vm_page_queue_active,pp,p);
			vm_page_active_count++;
			pp->pg_reserved |= PG_ACTIVE;
		}
	}
	vm_page_unlock_queues();
}

/*
 * These should be inline expanded when were ready.
 */

vm_page_sched()
{
	if ((vm_page_free_count < vm_page_free_min) ||
		((vm_page_free_count < vm_page_free_target) &&
		(vm_page_inactive_count < vm_page_inactive_target)))
		thread_wakeup((vm_offset_t)&vm_page_free_wanted);
}

