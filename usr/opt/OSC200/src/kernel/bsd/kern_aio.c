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
static char *rcsid = "@(#)$RCSfile: kern_aio.c,v $ $Revision: 1.1.10.3 $ (DEC) $Date: 1993/08/27 21:35:44 $";
#endif

#include <sys/sysaio.h>
#include <sys/errno.h>
#include <sys/signal.h>
#include <sys/limits.h>
#include <sys/proc.h>
#include <sys/vnode.h>
#include <sys/conf.h>
#include <sys/uio.h>
#include <sys/ioctl.h>
#include <sys/file.h>
#include <ufs/inode.h>
#include <kern/task.h>
#include <kern/thread.h>
#include <kern/lock.h>
#include <kern/parallel.h>
#include <kern/zalloc.h>
#include <kern/queue.h>
#include <kern/sched.h>
#include <mach/mach_types.h>
#include <mach/boolean.h>
#include <mach/kern_return.h>
#include <mach/vm_param.h>
#include <mach/memory_object.h>
#include <vm/vm_kern.h>
#include <vm/vm_object.h>
#include <vm/vm_map.h>
#include <vm/vm_page.h>
#include <vm/vm_control.h>
#include <vm/vm_tune.h>
#include <vm/vm_umap.h>
#include <vm/vm_vlock.h>
#include <vm/pmap.h>
#include <vm/u_mape_dev.h>
#include <vm/vm_anon.h>
#include <vm/vm_mmap.h>

/*
 * Local macros.
 */
#define AIO_TEST_DONE	0x1	/* a suspend or lio_list is satisfied */
#define AIO_TEST_WAIT	0x2	/* the thread is sleeping */
#define AIO_TEST_SIGNAL	0x4	/* the thread is awake and wants a signal */

#define AIO_RB_LOCK()		simple_lock(&aio_lock)
#define AIO_RB_UNLOCK()		simple_unlock(&aio_lock)

/*
 * list manipulation macros.
 *
 *   the list argument contains a pointer to the first element of a
 *   doubley linked list.  when the first element of the list changes,
 *   this pointer is modified to point to the new first entry.  When
 *   the list is empty, the pointer is NULL.
 *
 *   the list entries are linked using the next and last members of
 *   the structure.  While an entry is in the list, the next and last
 *   entries point to the start of the entries (unlike VAX queues
 *   where flink/blink always point to the flink's of entries).
 *
 *                  +-----------------------------------------------+
 *                  |                                               |
 *	ptr  -----> +-->+----+<--+  +-->+----+<--+  +-->+----+<--+  |
 *                      |    |   |  |   |    |   |  |   |    |   |  |
 *                      [next]---|--+   [next]---|--+   [next]---|--+
 *                  +---[last]   +------[last]   +------[last]   |
 *                  |   |    |          |    |          |    |   |
 *                  |   +----+          +----+          +----+   |
 *                  |                                            |
 *                  +--------------------------------------------+
 */

/*
 * Remove the entry at the head of the list, e.g.:
 *	LIST_GET_ENTRY(rb.rb_test_list, next_test)
 */
#define LIST_GET_FIRST(list, entry)					\
	if (list) {							\
		(entry) = (list);					\
		if ((entry)->at_next == (list))				\
			(list) = NULL;					\
		else {							\
			(entry)->at_last->at_next = (entry)->at_next;	\
			(entry)->at_next->at_last = (entry)->at_last;	\
			(list) = (entry)->at_next;			\
		}							\
	} else								\
		(entry) = NULL;

/*
 * Remove an entry from a list, e.g.:
 *	LIST_GET_ENTRY(rb.rb_test_list, this_test)
 */
#define LIST_GET_ENTRY(list, entry)					\
	if ((entry)->at_next == (list))					\
		(list) = NULL;						\
	else {								\
		(entry)->at_last->at_next = (entry)->at_next;		\
		(entry)->at_next->at_last = (entry)->at_last;		\
		if ((entry) == (list))					\
			(list) = (entry)->at_next;			\
	}

/*
 * Add an entry to a list, e.g.:
 *	LIST_PUT_LAST(rb.rb_test_list, new_test)
 */
#define LIST_PUT_LAST(list, entry)					\
	if (list) {							\
		(entry)->at_last = (list)->at_last;			\
		(entry)->at_next = (list);				\
		(entry)->at_last->at_next = (list)->at_last = (entry);	\
	} else								\
		(entry)->at_last = (entry)->at_next = (list) = (entry);

/*
 * AIO data base. This global static data is shared by all consumers
 * of AIO services. Access to volatile data is protected by an aio
 * simple lock.
 */

/*
 * aio_thdr_zone: an exhaustible zone for allocating aio test headers
 */
zone_t aio_thdr_zone;

/*
 * aio_buf_zone: an exhaustible zone for allocating aio buf structures
 */
zone_t aio_buf_zone;

/*
 * aio_max: maximum number of result blocks
 */
int aio_max;

/*
 * aio_size: size in bytes of array of result blocks
 */
int aio_size;

/*
 * aio_lock: simple lock for access to aio database: this lock
 * provides synchronized access to the result blocks; it is
 * locked only at spl0.
 */
decl_simple_lock_data(,aio_lock)

/*
 * aio_free: index value of next available aio result block
 */
int aio_free;

/*
 * aio_rb: base pointer for dynamically allocated array of result blocks
 */
aio_result_t aio_rb;

/*
 * aio_phys: invariant physical address of the array of result blocks
 */
vm_offset_t aio_phys;

/*
 * aio_completion_queue: queue of aio_buf structs used to perform
 * aio completion in the aio completion thread. Structs are queued
 * by aio_driver_done() at device spl and dequeued in the aio thread
 * for unwiring pages and signaling completion.
 */
mpqueue_head_t	aio_completion_queue;

/*
 * aio_completion_init: boolean to show whether the thread ever
 * got created. Used to bound aio's back to the library if the
 * thread never came to life but everything else is working.
 */
int aio_completion_init;

/*
 * aio_vnode, aio_vp: dummy vnode and ptr to properly emulate an mmap
 * of a device special file.
 */
struct vnode aio_vnode, *aio_vp;

/*
 * Typedefs to assist in retrieving function addresses from physio().
 */
typedef int 		strategy_func();
typedef unsigned long	minphys_func();

/*
 * AIODEVCACHE allows CDEVSW_READ or _WRITE to called only
 * once per opened AIO on a CDEV file descriptor. In other words,
 * the address of a cdev's strategy routine and minphys are stored
 * in this dynamically allocated table and are looked up for each
 * aio operation. The no strategy routine exists in the table entry,
 * it is retrieved and stored there, only to be clear when the raw
 * file is closed.
 */
#define AIODEVCACHE

#ifdef AIODEVCACHE
struct aio_dev_cache {
		strategy_func	*adc_strat;
		minphys_func	*adc_mincnt;
} *aio_dev_cache;
size_t aio_dev_cache_size;
#endif

thread_t aio_completion_thread_ptr; /* used for debugging crash dumps */

/*
 * aio_sysinit():
 *	Allocate wired kernel memory to back array of result blocks.
 *	(Memory must be physically contiguous!!!!!!!!!!!!!!!!!!!!!)
 *	Create a zone to hold zalloc'ed test headers and buf structures.
 *	Initialize the result block array. And finally, create an aio
 *	completion thread to handle cleanup after a kernel aio completes
 *	in the driver.
 */
void
aio_sysinit()
{
	extern int 		nproc;
	register int 		k;
	kern_return_t		result;
	aio_thdr_t 		*header;
	aio_buf_t 		*buf;
	register aio_result_t		rbp;
	extern task_t		first_task;
	extern void		aio_completion_thread();

	/*
	 * Initialize a dummy vnode to keep the u_mape_dev routines
	 * happy when a process mapped to the result block array exits
	 * or unmaps the memory.
	 */
	aio_vp = &aio_vnode;
	aio_vp->v_usecount = 1;
	VN_LOCK_INIT(aio_vp);
	VN_BUFLISTS_LOCK_INIT(aio_vp);
	VN_OUTPUT_LOCK_INIT(aio_vp);
	VN_AUX_LOCK_INIT(aio_vp);
	aio_vp->v_mount = 0;
	aio_vp->v_type = VBAD;
	aio_vp->v_numoutput = 0;
	aio_vp->v_object= VM_OBJECT_NULL;

#ifdef AIODEVCACHE
	aio_dev_cache_size = 
		round_page(nchrdev * sizeof(struct aio_dev_cache));
	/*
	 * Try to allocate that many bytes from wired kernel memory.
	 */
	if ((aio_dev_cache = (struct aio_dev_cache *)
			kmem_alloc(kernel_map, aio_dev_cache_size)) == NULL)
		goto aio_bad;

#endif

	/*
	 * Calculate total number of result blocks and their page-aligned
	 * size in bytes.
	 */
	aio_max = AIO_MAX;
	aio_size = round_page(aio_max * sizeof(struct aio_result_block));

	/*
	 * Try to allocate that many bytes from wired kernel memory.
	 */
	if ((aio_rb = (aio_result_t) kmem_alloc(kernel_map, aio_size))
			== NULL)
		goto aio_bad;

	/*
	 * For later use in double mapping the rbs, get the physical
	 * address of the result block array.
	 */
	if ((result = pmap_svatophys(aio_rb, &aio_phys)) != KERN_SUCCESS)
		goto aio_bad;

	/*
	 * Create the zone to hold test header entries.
	 */
	aio_thdr_zone = zinit((vm_size_t) sizeof(aio_thdr_t),
			(vm_size_t) aio_max * sizeof(aio_thdr_t),
			(vm_size_t) aio_max * sizeof(aio_thdr_t),
			"aio test header zone");

	/*
	 * If zone cannot be created, return result block memory
	 * and disable aio database.
	 */
	if (aio_thdr_zone == ZONE_NULL)
		goto aio_bad;

	/*
	 * Make the zone nonpageable, nonsleepable, exhaustible,
	 * noncollectable, etc.
	 */
	zchange(aio_thdr_zone, FALSE, FALSE, TRUE, FALSE);

	/*
	 * Allocate all the zone memory and insert it once and for all.
	 * If the alloc fails, clean up (no safe way to give back zone).
	 */
	if ((header = (aio_thdr_t *) kmem_alloc(kernel_map,
			aio_max * sizeof(aio_thdr_t))) == NULL)
		goto aio_bad;

	zcram(aio_thdr_zone, header, aio_max * sizeof(aio_thdr_t));

	/*
	 * Create the zone to hold aio buf entries.
	 */
	aio_buf_zone = zinit((vm_size_t) sizeof(aio_buf_t),
			(vm_size_t) aio_max * sizeof(aio_buf_t),
			(vm_size_t) aio_max * sizeof(aio_buf_t),
			"aio buf zone");

	/*
	 * If zone cannot be created, return result block memory
	 * and disable aio database.
	 */
	if (aio_buf_zone == ZONE_NULL)
		goto aio_bad;

	/*
	 * Make the zone nonpageable, nonsleepable, exhaustible,
	 * noncollectable, etc.
	 */
	zchange(aio_buf_zone, FALSE, FALSE, TRUE, FALSE);


	/*
	 * Allocate all the zone memory and insert it once and for all.
	 * If the alloc fails, clean up (no safe way to give back zone).
	 */
	if ((buf = (aio_buf_t *) kmem_alloc(kernel_map,
			aio_max * sizeof(aio_buf_t))) == NULL)
		goto aio_bad;
#ifdef AIODEVCACHE
	bzero((char *) aio_dev_cache, aio_dev_cache_size);
#endif
	zcram(aio_buf_zone, buf, aio_max * sizeof(aio_buf_t));


	/*
	 * Create the rb lock and set up the allocator idxs.
	 */
	simple_lock_init(&aio_lock);
	aio_free = 0;
	for (k = 0, rbp = aio_rb; k < aio_max; k++, rbp++) {
		rbp->rb_idx = k + 1;
		AIO_SET_IDX(rbp->rb_key, k);
		AIO_SET_SEQ(rbp->rb_key, 1);
		rbp->rb_errno = EINVAL;
		rbp->rb_pid = rbp->rb_fd = -1;
	}

	/*
	 * Start the aio thread.
	 */
	aio_completion_thread_ptr =
		kernel_thread(first_task, aio_completion_thread);

	/*
	 * Done.
	 */
	return;

aio_bad:
	/*
	 * If we cannot get the memory to back the aiorbs or zone,
	 * set some values to force all aio attempts to fail.
	 */
	if (aio_rb)
	      	(void) kmem_free(kernel_map, aio_rb, aio_size);
	if (header)
		(void) kmem_free(kernel_map, header, aio_max *
			sizeof(aio_thdr_t));
#ifdef AIODEVCACHE
	if (aio_dev_cache)
	      	(void) kmem_free(kernel_map, aio_dev_cache, aio_dev_cache_size);
	aio_dev_cache = NULL;
#endif
	aio_free = -1;
	aio_max = 0;
	aio_rb = NULL;
}

/*
 * aio_completion_thread:
 *	This kernel thread runs to remove aio_buf structs from the
 *	completion queue, unwire the user bufs they point to, and
 *	call aio_complete() with the result and error value from
 *	the buf struct. Runs in thread context at spl0.
 */
void aio_completion_thread()
{
	thread_t		thread;
	aio_buf_t		*ab;
	struct buf		*bp;
	char 			*a;
	vm_offset_t		start, end;
	register int		done;
	register long		requested;
	register int s;

	thread = current_thread();
	thread_swappable(thread, FALSE);
	thread->priority = thread->sched_pri = BASEPRI_PSIGNAL;
	(void) spl0();
	mpqueue_init(&aio_completion_queue);
	aio_completion_init++;

	for (;;) {
		/*
		 * Get a buf structure from the completion queue.
		 * Wait until one is available.
		 */
		mpdequeue1(&aio_completion_queue, &bp, QWAIT);

		/*
		 * Since buf was queued in the middle, back up to start of
		 * buf structure.
		 */
		bp = (struct buf *)((unsigned long) bp -
				(unsigned long) &(((struct buf *)0)->b_forw));
		ab = (aio_buf_t *) bp;

		/*
		 * Set up and then unwire the user's buffer.
		 */
		a = bp->b_un.b_addr;
		requested = ab->ab_requested; /* number of pages actually wired */
		start = trunc_page(a);
		end = round_page(a + requested);
		(void) vm_map_pageable(bp->b_proc->task->map, start, end, VM_PROT_NONE);

		/*
		 * We need to do this because the process may be waiting in
		 * aioexit() for a kernel aio to complete.
		 */
		event_post(&bp->b_iocomplete);

		/*
		 * Call the aio completion routine and free the aio buffer.
		 */
		done = bp->b_bcount - bp->b_resid;
		aio_complete(ab->ab_key, done, bp->b_error);
		zfree(aio_buf_zone, ab);
	}
}

/*
 * aio_mapfunc: called by u_dev_fault() to return the pfn of a mapped
 * device va during page faulting. This is a soft fault on the user va.
 */
int
aio_mapfunc(dev_t dummy, off_t offset, int prot)
{
	return(pmap_phys_to_frame(aio_phys + offset));
}

/*
 * aio_cdevsw_del: called during a loadable driver unload to clear
 * out the aio device cache entry for the specified major device
 * table entry.
 */
aio_cdevsw_del(int maj)
{
#ifdef AIODEVCACHE
	if (aio_dev_cache)
		aio_dev_cache[maj].adc_strat = NULL;
#endif
	return;
}

/*
 * aio_init: initialize kernel context for an aio library by mapping
 * the result block area into the caller's vm map and returning the
 * starting va and the number array elements.
 */
int
aio_init(p, args, retval)
	register struct proc *p;
	void *args;
	long *retval;
{
	register struct args {
		vm_offset_t	*rb_base;
		int		*rb_size;
	} *uap = (struct args *) args;
	register kern_return_t	result;
	vm_offset_t		addr;
	struct u_dev_object 	*dp;
	register vm_offset_t	offset;
	vm_map_t		user_map = p->task->map;
	extern kern_return_t	vm_map_delete();

	/*
	 * If there is no result block array, fail.
	 */
	if (aio_rb == NULL)
		return EAGAIN;

	/*
	 * Create the dummy device vm object.
	 */
	if (vm_object_allocate(OT_DEVMAP, aio_size, (caddr_t) 0, &dp))
		return EAGAIN;

	dp->ud_mapfunc = aio_mapfunc;
	dp->ud_flags = 0;
	dp->ud_offset = (vm_offset_t)0;
	dp->ud_dev = 0;
	dp->ud_vp = aio_vp;
	VREF(dp->ud_vp);

	addr = NULL;
	offset = (vm_offset_t) 0;

	/*
	 * Enter the vm object into the caller's map and get back
	 * the user va where it was entered. The va is mapped to be
	 * read only, and the max_prot is set the same to keep the
	 * memory from being set to read/write by the user.
	 */
	if ((result = u_map_enter(user_map, &addr, aio_size, 0, 1,
			dp, (vm_offset_t) 0, FALSE, VM_PROT_READ,
			VM_PROT_READ, VM_INHERIT_NONE)) != KERN_SUCCESS) {
		vm_object_free(dp);
		return EAGAIN;
	}

	/*
	 * Copy out base va and # of elements to caller.
	 */
	if (result = copyout((caddr_t) &addr, (caddr_t)uap->rb_base,
			sizeof(vm_offset_t)))
		goto aio_init_fail;

	if (result = copyout((caddr_t) &aio_max, (caddr_t)uap->rb_size,
			sizeof(int)))
		goto aio_init_fail;

	/*
	 * Set the AIO flag for the process. Allows cleanup on exit.
	 * Note: this flag is inhereted on exec(), but not on fork().
	 * Passing it through to exec() makes sure that aioexit() is
	 * called for the process, even though none of its aio state
	 * was inherited expect aio result blocks tagged by the
	 * exec'ed process' pid.
	 */
	p->p_flag |= SAIO;
	return 0;

aio_init_fail:
	vm_map_lock(user_map);
	(void) vm_map_delete(user_map, addr, aio_size);
	vm_map_unlock(user_map);
	vm_object_free(dp);
	return(result);
}

/*
 * Allocation and translation routines.
 * Should be called with the rb simple lock held.
 */

/*
 * aio_key_to_addr:
 *	macro to validate and translate aio_key and return address
 */
#define aio_key_to_addr(key)					\
	(((key) < 0) || (AIO_GET_IDX((key)) >= aio_max) ||	\
		(aio_rb == NULL) || 				\
		aio_rb[AIO_GET_IDX(key)].rb_key != (key)) ?	\
		NULL : &aio_rb[AIO_GET_IDX((key))]

/*
 * aio_alloc:
 *	allocate and initialize a result block, returning address.
 */
aio_result_t
aio_alloc()
{
	int i;
	register aio_result_t rbp;

	if (aio_free >= aio_max || aio_free < 0) {
		return(NULL);
	}
	i = aio_free;
	rbp = &aio_rb[i];
	aio_free = rbp->rb_idx;
	rbp->rb_errno = EINPROGRESS;
	rbp->rb_result = -1;
	rbp->rb_test_list = NULL;
	return(rbp);
}

/*
 * aio_dealloc:
 *	free a result block and its state. MUST BE CALLED WITH RB LOCK HELD.
 */
int
aio_dealloc(aio_key_t key)
{
	register int k = AIO_GET_IDX(key);
	register int seq = AIO_GET_SEQ(key);
	register aio_result_t rbp = aio_key_to_addr(key);

	if (rbp == NULL)
		return -1;

	rbp->rb_errno = EINVAL;
	rbp->rb_result = rbp->rb_pid = rbp->rb_pid = -1;
	rbp->rb_proc = NULL;
	rbp->rb_driver = 0;
	rbp->rb_buf = NULL;
	rbp->rb_idx = aio_free;
	AIO_SET_SEQ(rbp->rb_key, ++seq);
	aio_free = k;
	return 0;
}

/*
 * aio_ioctl:
 *	request aio service from a raw character device driver.
 */
aio_ioctl(dev_t dev, unsigned int cmd, aiocb_t *data, int flags, void *private)
{
	int error = 0;
	
	switch (data->aio_lio_opcode) {
		case LIO_READ:
       			return aio_rw(dev, data, flags, private);
		case LIO_WRITE:
			return aio_rw(dev, data, flags, private);
#ifdef notyet
     		case LIO_CANCEL:
#endif
		default:
			return EIO;
	}
}

/*
 * aio_rw:
 *	Called by aio_ioctl() to perform a driver I/O.
 */
aio_rw(dev_t dev, aiocb_t *ap, int flags, void *private)
{
	strategy_func		*strategy;
	minphys_func		*mincnt;
	register aio_buf_t 	*ab;
	register aio_result_t 	rbp;
	register int 		done, rw, append = 0;
	register long		requested;
	int			error = 0, s;
	vm_offset_t		start, end;
	vm_map_t		map;
	extern void		aio_driver_done();
	struct uio		uio;
	struct iovec		vec[2];
	struct file 		*fp = NULL;
	register off_t 		offset;
	register int		maj = major(dev);
	
	/*
	 * Determine operation.
	 */
	rw = ((ap->aio_lio_opcode == LIO_READ) ? B_READ : B_WRITE);

	/*
	 * Check accessability to the user's I/O buffer.
	 */
	if (!useracc(ap->aio_buf, (u_int)ap->aio_nbytes, rw))
		return(EFAULT);

	/*
	 * Before anything really important happens, set up a uio struct
	 * and pass it to the character device's read or write routine.
	 * The purpose of this is merely to get the address of its strategy
	 * and minphys routines, which we expect back in the 2 iovec structs
	 * strung off uio.uio_iov field. This works because the physio()
	 * function (ufs/ufs_physio.c) makes a special check for our UIO_AIORW
	 * code in the uio structure passed in. When it finds, it puts the
	 * strategy and minphys routine addresses it was passed into the uio
	 * structure and returns EAIO, which eventually ends up here to tell
	 * us that we can get the addresses from the uio. All the work physio()
	 * normally does -- mainly setting up the buf structure -- is done
	 * here instead.
	 */
#ifdef AIODEVCACHE
	if ((strategy = aio_dev_cache[maj].adc_strat) == NULL) {
#endif
		vec[0].iov_base = vec[1].iov_base = NULL;
		uio.uio_iov = vec;
		vec[0].iov_len = 1;
		uio.uio_rw = UIO_AIORW;
		uio.uio_iovcnt = 1;
		uio.uio_offset = (off_t) 0;
		uio.uio_segflg = UIO_SYSSPACE;
		/*
		 * Set resid flag to 0, just in case this uio ends up
		 * where it shouldn't. The 0 should prevent any action
		 * being taken on this obviously bogus uio.
		 */
		uio.uio_resid = 0;
#ifdef AIODEVCACHE
	}
#endif
	
	/*
	 * Call the read or write routine from this device's cdevsw
	 * table entry. On success, error will be EAIO, and the
	 * devices strategy routine's address will be in uio.
	 */
#ifdef AIODEVCACHE
	if (strategy == NULL) {
#endif
	if (rw == B_READ)
		CDEVSW_READ(maj, dev, &uio, flags, error, private);
	else
		CDEVSW_WRITE(maj, dev, &uio, flags, error, private);
	/*
	 * If we didn't get back the expected EAIO, or if the strategy
	 * iovec struct does not contain an address, give up.
	 */
	if ((error != EAIO) ||
		((strategy = (strategy_func *) vec[0].iov_base) == NULL))
		return ENODEV;

#ifdef AIODEVCACHE
	aio_dev_cache[maj].adc_strat = strategy;
#endif
	/*
	 * Now get the minphys function. If NULL, use the system
	 * default minphys().
	 */
	if ((mincnt = (minphys_func *) vec[1].iov_base) == NULL)
		mincnt = (minphys_func *) minphys;

#ifdef AIODEVCACHE
	aio_dev_cache[maj].adc_mincnt = mincnt;
	}
	else
		mincnt = aio_dev_cache[maj].adc_mincnt;
#endif

	/*
	 * Got the routines. Now create a buf struct to pass to it...
	 */
	error = 0;

	/*
	 * Get the address of the result block associated with the
	 * active AIO.
	 */
	AIO_RB_LOCK();	
	if ((rbp = aio_key_to_addr(ap->aio_key)) == NULL) {
		AIO_RB_UNLOCK();	
      		return EINVAL;
	}
	AIO_RB_UNLOCK();	

	/*
	 * Get a buf structure from the aio buffer zone.
	 */
	if ((ab = (aio_buf_t *) zget(aio_buf_zone)) == NULL)
		return EAGAIN;

	/*
	 * Initialize the result block to show that it's going into a driver.
	 */
	rbp->rb_driver = EAIO;
	rbp->rb_buf = ab;

	/*
	 * Initialize the buf structre with the details of the I/O.
	 * NOTE: B_ASYNC not set in b_flags.
	 */
	ab->ab_key = rbp->rb_key;
	ab->ab_buf.b_proc = rbp->rb_proc;
	ab->ab_buf.b_error = 0;
	ab->ab_buf.b_vp = NULL;
	ab->ab_buf.b_un.b_addr = (caddr_t)(ap->aio_buf);
	ab->ab_buf.b_flags = B_PHYS | B_RAW | /*B_ASYNC |*/ B_BUSY  | rw;
	event_init(&ab->ab_buf.b_iocomplete);
	ab->ab_buf.b_iodone = aio_driver_done;
	ab->ab_buf.b_dev = dev;

	/*
	 * Determine how many bytes can be transfered, then
	 * call the strategy mincnt() routine we captured above.
	 * Save what mincnt() did to b_bcount in ab_requested.
	 * This way, in the driver-done routine we'll unwire
	 * *exactly* the number of pages we wired down here.
	 */
	ab->ab_buf.b_bcount = ap->aio_nbytes;
	(void) (mincnt)(&ab->ab_buf);
	requested = ab->ab_requested = ab->ab_buf.b_bcount;

	/*
	 * On a write, check whether the file is open for append.
	 * If so, "seek" the end of the file before allowing the
	 * write. We maintain the file pointer here completely by
	 * ourselves, so we update the pointer by what we think
	 * the next position should be. This isn't ideal, but it's
	 * the only thing to do for asynch I/O.
	 */
	if (!rw) {
		if (error = getf(&fp, ap->aio_fildes, FILE_FLAGS_NULL,
				&u.u_file_state)) {
			zfree(aio_buf_zone, ab);
			return error;
		}
		FP_LOCK(fp);
		if (fp->f_flag & O_APPEND) {
			append = 1;
			ap->aio_offset = fp->f_offset;
			fp->f_offset += requested;
			FP_UNLOCK(fp);
		} else
			FP_UNLOCK(fp);
		FP_UNREF(fp);
	}

	/*
	 * If aio_offset is AIO_SEEK_CUR, then we want to use the current
	 * file position. Take a chance and update the file pointer as well.
	 * Note that if we already know the file is open for append, we
	 * should simply ignore the aio_offset field and skip this. If
	 * this is a write using AIO_SEEK_CUR, we know we already looked
	 * up fp above, so we only need to take an FP_REF instead of an
	 * getf() call.
	 */
	if (!append && (ap->aio_offset == AIO_SEEK_CUR)) {
		if (fp == NULL) {
			if (error = getf(&fp, ap->aio_fildes,
					FILE_FLAGS_NULL, &u.u_file_state)) {
				zfree(aio_buf_zone, ab);
				return error;
			}
		} else
			FP_REF(fp);
		FP_LOCK(fp);
		offset = fp->f_offset;
		fp->f_offset += requested;
		FP_UNLOCK(fp);
		FP_UNREF(fp);
		ab->ab_buf.b_blkno = btodb(offset);
	} else
		ab->ab_buf.b_blkno = btodb(ap->aio_offset);

	/*
	 * Wire the user's buffer for safe access from device spl.
	 */
	start = trunc_page(ab->ab_buf.b_un.b_addr);
	end = round_page(ab->ab_buf.b_un.b_addr + requested);
	map = current_task()->map;
	if (vm_map_pageable(map, start, end,
			(rw & B_READ) ? VM_PROT_WRITE : VM_PROT_READ)
			!= KERN_SUCCESS) {
		zfree(aio_buf_zone, ab);
		error = EFAULT;
	} else {
		/*
		 * Okay, call the strategy routine...
		 */
		(void) (strategy)(&ab->ab_buf);

		/*
		 * Now, find out what happened. If there's no error,
		 * we can clean up and leave. Since we're not waiting
		 * around here, we don't really expect to execute this
		 * code unless we were interrupted.
		 */
		if (event_posted(&ab->ab_buf.b_iocomplete) &&
				(ab->ab_buf.b_flags & B_ERROR)) { 
			(void)vm_map_pageable(map, start, end, VM_PROT_NONE);
			error = biowait(&ab->ab_buf);
			zfree(aio_buf_zone, ab);
		}

	}
	return error;
}

/* aio_driver_done:
 *	called by biodone() when the driver is finished with buf.
 */
void
aio_driver_done(struct buf *bp)
{
	/*
	 * Simply queue up the buf structure and return. Let
	 * the aio completion thread take care of everything.
	 */
	mpenqueue1(&aio_completion_queue, &bp->b_forw);
}

/*
 * aio_complete:
 * 	work routine called for driver completion and by
 *	aio_transfer_done(). Sets the errno and result of an AIO
 *	operation, and checks to see whether anyone needs to be awakened
 *	as a result of the completion.
 */
int
aio_complete(aio_key_t key, int result, int errno)
{
	int flag;
	register aio_result_t rb;

	/*
	 * Get the address of the completed result block.
	 */
	AIO_RB_LOCK();
	if ((rb = aio_key_to_addr(key)) == NULL) {
		AIO_RB_UNLOCK();
		return EINVAL;
	}

	/*
	 * Value of aio_result controls acces to the result block.
	 * Only if the result == EINPROGRESS on entry can we assume
	 * that it's an active AIO.
	 */
	if (rb->rb_errno == EINPROGRESS) {
		rb->rb_result = result;
		rb->rb_errno = errno;

		/*
		 * If a completion signal was requested, post it now.
		 */
		if (rb->rb_sigevent.sigev_signo) {
			unix_master();
			psignal(rb->rb_proc, rb->rb_sigevent.sigev_signo);
			unix_release();
		}

		/*
		 * Mark this AIO done, and kick off tests for total completion.
		 */
		aio_next_done(rb);
	}

	AIO_RB_UNLOCK();
	return 0;
}

/*
 * aio_next_done:
 *	Mark this test block as complete and check for total completion,
 *	that is, whether a wakeup or completion signal is required.
 */
void
aio_next_done(aio_result_t rb)
{
	aio_test_t *at;

	/*
	 * Test for total aio satisfaction.
	 */
	LIST_GET_FIRST(rb->rb_test_list, at)
	while (at) {
		aio_test_done(at);
		LIST_GET_FIRST(rb->rb_test_list, at);
	}
}

/*
 * aio_test_done:
 *	tests whether the wait associated with this I/O completion
 *	is satisfied. If so, the wait nexus is torn down.
 */
void
aio_test_done(aio_test_t *at)
{
	register aio_thdr_t *th;

	th = at->at_thdr;
	if ((!(th->thdr_flags & AIO_TEST_DONE)) && (--th->thdr_notdone == 0)) {
		th->thdr_flags |= AIO_TEST_DONE;
		aio_remove_tests(th);
		aio_unwait_signal(th);
	}
}

/*
 * aio_remove_tests:
 *	 dequeues test blocks after a satisfied wait.
 */
void
aio_remove_tests(aio_thdr_t *th)
{
	register int i;

	for (i = 0; i < th->thdr_count; i++) {
		register aio_test_t *at = &th->thdr_list[i];

		if (at->at_rb && (at->at_rb->rb_errno == EINPROGRESS)) {
			LIST_GET_ENTRY(at->at_rb->rb_test_list, at)
		}
	}
}

/*
 * aio_unwait_signal:
 * 	wakeup any sleeping threads and deliver any LIO completion signals.
 */
void
aio_unwait_signal(aio_thdr_t *th)
{
	int flags, sig;

	flags = th->thdr_flags;

	if ((flags & AIO_TEST_SIGNAL) &&
			(sig = th->thdr_sigevent.sigev_signo)) {
		unix_master();
		psignal(th->thdr_proc, sig);
		unix_release();
	}
	if (flags & AIO_TEST_WAIT)
		clear_wait(th->thdr_thread, THREAD_AWAKENED, 0);
	else
		/*
		 * If this wasn't a wait, free the test header. If it was
		 * a wait, then the code executed by the awakened thread
		 * will free the header.
		 */
		zfree(aio_thdr_zone, th);
}

/*
 * aioexit:
 * 	Clean up the AIO context for a process that is exiting. The
 *	strategy is simple: walk the entire array of result blocks
 *	looking for any with the address of our proc. If we find one,
 *	and if it's not a kernel aio, call aio_complete() for the block
 *	and let that teardown any wait structures. If it is a kernel aio
 *	and call biowait(). The driver completion will aio_complete()
 *	for us with the same result. Then free up the result block.
 */
void
aioexit(struct proc *cp)
{
	register int i;
	register aio_result_t rbp;
	register struct proc *p = cp;

	if (aio_rb == NULL)
		return;

	rbp = aio_rb;
	for (i = 0; i < aio_max; i++, rbp++) {
		if (rbp->rb_proc != p)
			continue;

		if (!rbp->rb_driver)
			aio_complete(rbp->rb_key, -1, ECANCELED);
		else {
			biowait(&rbp->rb_buf->ab_buf);
		}
		AIO_RB_LOCK();
		aio_dealloc(rbp->rb_key);
		AIO_RB_UNLOCK();
	}
}

/*
 * aio_transfer:
 *	Set up to do an aio_read() or aio_write(). Both library calls
 *	come here to allocate a result block (and get back key to
 *	identify the transfer in subsequent calls). The first time
 *	here for an AIO on any file descriptor, we'll pass the
 *	request off to the system by setting up a standard ioctl() call.
 *	If that succeeds, we return a value to the library the tells it
 *	that the AIO was taken by the driver. Otherwise, the library
 *	passes it to a thread and calls us with in_driver == FALSE
 *	so we can skip further attempts to post the AIO to a driver.
 */
int
aio_transfer(p, args, retval)
	register struct proc *p;
	void *args;
	long *retval;
{
	register struct args {
		aiocb_t		*aiocb;
		long		in_driver;	/* Boolean: try the driver? */
		long		fd;
		long		signo;
		sigval_t	sigval;
	} *uap = (struct args *) args;
	register aio_result_t rbp;
	register int sig = uap->signo;
	int ret;

	/*
	 * Allocate a result block and a key for the transfer.
	 */
	AIO_RB_LOCK();
	if ((rbp = aio_alloc()) == NULL) {
		AIO_RB_UNLOCK();
		return EAGAIN;
	}

	/*
	 * Set the global flag that tells _exit() that it must call
	 * aioexit().
	 */
	AIO_RB_UNLOCK();

	/*
	 * Initialize the result block with data specific to this transfer.
	 */
	rbp->rb_fd = uap->fd;
	rbp->rb_proc = p;
	rbp->rb_pid = p->p_pid;

	if (sig > 0 && sig <= NSIG) {
		rbp->rb_sigevent.sigev_signo = sig;
		rbp->rb_sigevent.sigev_value = uap->sigval;
	}
	else
		rbp->rb_sigevent.sigev_signo = 0;
		
	/*
	 * Copy out the aio_key value that corresponds to the result block.
	 */
	if (ret = copyout((caddr_t) &rbp->rb_key,
			(caddr_t)&uap->aiocb->aio_key, sizeof(aio_key_t))) {
		AIO_RB_LOCK();
		aio_dealloc(rbp->rb_key);
		AIO_RB_UNLOCK();
		return(ret);
	}

	/*
	 * If in_driver is TRUE, set up a call to ioctl() to attempt
	 * the transfer directly in the driver for raw character devices.
	 * Only do this if the aio completion thread is around.
	 */
	if (uap->in_driver && aio_completion_init) {
		struct {
			long		fdes;
			unsigned long	cmd;
			caddr_t		cmarg;
		} fake_uap;

		/*
		 * Create a fake syscall to ioctl() by passing it the proper
		 * args structure.
		 */
		fake_uap.fdes = (long) uap->fd;
		fake_uap.cmd = (unsigned long) AIOCTLRW;
		fake_uap.cmarg = (caddr_t) uap->aiocb;

		ret = ioctl(p, &fake_uap, retval);

		/*
		 * If the call failed for any reason, try passing it
		 * off to a thread in the library. This could possibly
		 * cause some problems if an earlier call succeed on
		 * this file descriptor and then this one failed. We'll
		 * never try the driver again.
		 */
		if (ret)
			/*
			 * The call to the driver apparently failed.
			 * Give the library a chance to try with a thread.
			 * It may well fail itself, which should notify
			 * the application and be followed by normal
			 * error handling/cleanup...
			 */
			ret = 0;
		else {
			/*
			 * The driver call apparently succeeded.
			 * We're returning EAIO to tell the library not
			 * to queue the AIO to a thread.
			 */
			*retval = EAIO;
			ret = 0;
		}
	} else
		ret = 0;

	return ret;
}

/*
 * aio_transfer_done:
 *	Entry point from the library to call aio_complete() when a
 *	thread-base AIO is complete.
 */
int
aio_transfer_done(p, args, retval)
	register struct proc *p;
	void *args;
	long *retval;
{
	register struct args {
		long		key;		/* really "aio_key_t" */
		long		result;		/* really "int" */
		long		errno;		/* really "int" */
	} *uap = (struct args *) args;

	return(aio_complete((aio_key_t)uap->key, (int)uap->result, (int)uap->errno));
}

/*
 * aio_wait: Entry point to perform the synch work for the library
 * routines aio_suspend() and lio_listio(). Sets up the wait/test
 * structures for the calling thread and puts it to sleep if the
 * wait conditions weren't previously satisfied and the wait type
 * is LIO_SUSPEND (wait-all) or LIO_WAIt (wait-any).
 */
int
aio_wait(p, args, retval)
	register struct proc *p;
	void *args;
	long *retval;
{
	register struct args {
		long		count;
		aio_key_t	*keys;
		long		type;
		long		signo;
		sigval_t	sigval;
	} *uap = (struct args *) args;
	aio_result_t rbs[AIO_LISTIO_MAX];
	aio_key_t keys[AIO_LISTIO_MAX];
	int ret, type = uap->type;
	register int i, k, tcnt, flags, cnt = uap->count, sig = uap->signo;
	register aio_thdr_t *th;
	register thread_t this_thread = current_thread();
	sigevent_t sigevent;
	register aio_test_t *at;

	/*
	 * Protect ourselves from a bogus key count.
	 */
	if (cnt > AIO_LISTIO_MAX)
		return EINVAL;
	else if (cnt <= 0)
		return 0;
	else
		tcnt = cnt;

	/*
	 * Validate wait type and set up wait flags.
	 */
	switch(type) {
		case LIO_SUSPEND:
		case LIO_WAIT:
			flags = AIO_TEST_WAIT;
			break;
		case LIO_NOWAIT:
			flags = AIO_TEST_SIGNAL;
			break;
		default:
			return EINVAL;
	}

	/*
	 * Copy in the array aio_keys passed by the library to represent
	 * the aiocbs to be waited on.
	 */
	if (uap->keys) {
		if (ret = copyin((caddr_t)uap->keys, (caddr_t)keys,
				cnt * sizeof(aio_key_t))) {		
			return(ret);
		}
	} else
		return 0;

	/*
	 * Make a sigevent structure and validate any signal number.
	 */
	if (sig > 0 && sig <= NSIG) {
		sigevent.sigev_signo = sig;
		sigevent.sigev_value = uap->sigval;
	} else
		sigevent.sigev_signo = 0;

	/*
	 * Try allocating a test header.
	 * Any subsequent failures are obliged to zfree(th);
	 */
	if ((th = (aio_thdr_t *) zget(aio_thdr_zone)) == NULL) {
		return EAGAIN;
	}

	/*
	 * Initialize the test header.
	 */
	th->thdr_sigevent = sigevent;
	th->thdr_flags = flags;
	th->thdr_thread = this_thread;
	th->thdr_proc = p;

	/*
	 * Block ALL access to result blocks with the simple lock.
	 * We'll hold this lock until just before we go to sleep.
	 * To let a aio completion slip by us now means we might
	 * never wakeup this thread!
	 */
	AIO_RB_LOCK();

	/*
	 * Translate keys and see wheter the requested aio is already
	 * done. If so: for aio_suspend(), return success. For, lio_listio(),
	 * remove the completed result block from inclusion in the queue
	 * of test blocks by backing up the rbs[index] value.
	 */
	for (i = 0, k = 0; i < tcnt; i++, k++) {
		rbs[k] = aio_key_to_addr(keys[i]);
		if (rbs[k] == NULL) {
			AIO_RB_UNLOCK();
			zfree(aio_thdr_zone, th);
			return EINVAL;
		}
		if (rbs[k]->rb_errno != EINPROGRESS) {
			if (type == LIO_SUSPEND) {
				AIO_RB_UNLOCK();
				zfree(aio_thdr_zone, th);
				return 0;
			} else {
				/*
				 * This aio is already complete. Remove
				 * it from consideration by backing up
				 * the rb index and the not-done count.
				 */
				--k;
				--cnt;
			}
		}
	}

	/*
	 * If count has reached 0, then an LIO operation is already
	 * satisified. If it was a wait, just return. If a signal
	 * was requested, post that and return.
	 */
	if (cnt <= 0) {
		if (type == LIO_WAIT) {
			AIO_RB_UNLOCK();
			zfree(aio_thdr_zone, th);
			return 0;
		} else {
			if (sigevent.sigev_signo) {
				unix_master();
				psignal(p, sigevent.sigev_signo);
				unix_release();
			}
			AIO_RB_UNLOCK();
			zfree(aio_thdr_zone, th);
			return 0;
		}
	}

	/*
	 * Otherwise:
	 * Set the count values. If this is a suspend, then count need
	 * only be 1 -- the first completion will satisfy the wait.
	 */
	th->thdr_count = cnt;
	th->thdr_notdone = type == LIO_SUSPEND ? 1 : cnt;

	/*
	 * Queue the test blocks and set up back pointers.
	 */
	for (i = 0, at = &th->thdr_list[0]; i < cnt; i++, at++) {
		LIST_PUT_LAST(rbs[i]->rb_test_list, at);
		at->at_rb = rbs[i];
		at->at_thdr = th;
	}

	/*
	 * If type == LIO_NOWAIT, unlock and return!
	 */
	if (type == LIO_NOWAIT) {
		AIO_RB_UNLOCK();
		return 0;
	}

	/*
	 * Assert our intention to wait. Unlock. Try to sleep.
	 * Test the wait result.
	 */
resleep:
	assert_wait((vm_offset_t)th, 1 /* interruptible */);

	/*
	 * Releasing the lock here could cause us to be preempted
	 * or allow a device completion to occur at interrupt level.
	 * In either case, the wait conditions could be satisfied by
	 * time we reach the thread_block(). thread_block() should
	 * see that we are still runnable and continue immediately.
	 */
	AIO_RB_UNLOCK();
	thread_block();
	AIO_RB_LOCK();		/* retake the lock */

	switch(this_thread->wait_result) {
		case THREAD_INTERRUPTED:
		default:
			aio_remove_tests(th);
			AIO_RB_UNLOCK();
			zfree(aio_thdr_zone, th);
			return EINTR;
			break;
		case THREAD_AWAKENED:
			/*
			 * To avoid bogus wakeups on the event, test
			 * one last time for sure and return success.
			 * Otherwise go back to sleep.
			 */
			if (th->thdr_flags & AIO_TEST_DONE) {
				AIO_RB_UNLOCK();
				zfree(aio_thdr_zone, th);
				return 0;
			} else {
				goto resleep;
			}
			break;
	}
}	/* NOTREACHED */

/*
 * aio_done:
 *	Return the result of the AIO transfer operation and free up
 *	the associated result block.
 */
size_t
aio_done(p, args, retval)
	register struct proc *p;
	void *args;
	long *retval;
{
	register struct args {
		long		key;		/* really "aio_key_t" */
	} *uap = (struct args *) args;
	register aio_key_t k = (aio_key_t) uap->key;
	register aio_result_t rbp = aio_key_to_addr(k);

	if (rbp == NULL)
		return EINVAL;

	if (rbp->rb_errno == EINPROGRESS) {
		return EINPROGRESS;
	} else
		*retval = rbp->rb_result;

	AIO_RB_LOCK();
	aio_dealloc(k);
	AIO_RB_UNLOCK();
	return 0;
}

/*
 * aio_transfer_cancel:
 * 	Reserved for use to actually attempt cancelation of AIOs
 *	sent to the driver. Today, we don't try because there are
 *	no cancelation entry points for existing drivers.
 */
int
aio_transfer_cancel(p, args, retval)
	register struct proc *p;
	void *args;
	long *retval;
{
	register struct args {
		long		key;		/* really "aio_key_t" */
	} *uap = (struct args *) args;

	return 0;
}

/*
 * aio_info:
 * 	Returns the number of free test headers and result blocks.
 */
int
aio_info(p, args, retval)
	register struct proc *p;
	void *args;
	long *retval;
{
	register struct args {
		int	*freethdrs;
		int	*freerbs;
	} *uap = (struct args *) args;
	register int i, k;
	aio_thdr_t *th[AIO_MAX];
	register aio_result_t rbp;
	int result, outval;

	if (aio_rb == NULL)
		return EINVAL;

	for (i = 0; th[i] = (aio_thdr_t *) zget(aio_thdr_zone); i++)
		;
	outval = i;
	result = copyout((caddr_t) &outval, (caddr_t)uap->freethdrs, sizeof(int));

	for (k = 0; k < i; k++)
		zfree(aio_thdr_zone, th[k]);

	AIO_RB_LOCK();
	for (i = 0, k = 0, rbp = aio_rb; i < aio_max; i++, rbp++)
		if (rbp->rb_errno == EINVAL)
			k++;		
	AIO_RB_UNLOCK();
	outval = k;
	result = copyout((caddr_t) &outval, (caddr_t)uap->freerbs, sizeof(int));

	return result;
}
