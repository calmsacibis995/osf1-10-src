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
static char *rcsid = "@(#)$RCSfile: cma_vp.c,v $ $Revision: 4.2.10.2 $ (DEC) $Date: 1993/08/18 14:55:28 $";
#endif
/*
 *  FACILITY:
 *
 *	DECthreads core
 *
 *  ABSTRACT:
 *
 *	Implement generic "virtual processor" layer to take advantage of
 *	kernel threads for parallel processing.
 *
 *  AUTHORS:
 *
 *	Dave Butenhof
 *
 *  CREATION DATE:
 *
 *	03 April 1991
 *
 *  MODIFICATION HISTORY:
 *
 *	001	Dave Butenhof	24 April 1991
 *		Add cma__trace statements to aid debugging.
 *	002	Dave Butenhof	03 May 1991
 *		Change "cma__vp_set_state" to "cma__vp_set_start" and
 *		encapsulate more machine-specific information here (like
 *		address of start routine and frame setup).
 *	003	Dave Butenhof	24 May 1991
 *		Add some traces, and change state enums into flags.
 *	004	Dave Butenhof	31 May 1991
 *		Add timeout parameter to vp_suspend.
 *	005	Dave Butenhof	02 June 1991
 *		Add new return status for trying to delete a VP that's not
 *		suspended.
 *	006	Dave Butenhof	03 June 1991
 *		New tack: a thread should delete it's own VP when
 *		terminating, rather than let another thread clean it up.
 *	007	Dave Butenhof	11 June 1991
 *		Change numeric constants in reinit function to use symbols.
 *	008	Dave Butenhof	05 September 1991
 *		Add cma__vp_yield() function to call Mach "sched_yield"
 *		syscall (which I hadn't known about when I designed the VP
 *		module).
 *	009	Dave Butenhof	11 September 1991
 *		Add support for caching VPs to improve performance.
 *	010	Dave Butenhof	07 February 1992
 *		Add cma__vp_dump() function to list all active and cached VPs
 *		for debugging.
 *	011	Dave Butenhof	08 February 1992
 *		Rework state flags for better debugging.
 *	012	Dave Butenhof	06 March 1992
 *		Modify cma__vp_get_state(): thread_get_state() doesn't work
 *		on a running thread, and we'd need extra logic to fix it. We
 *		really don't need the info it gives us (current pc and sp,
 *		basically), so just zero the fields and don't bother!
 *	013	Dave Butenhof	13 March 1992
 *		Cut down initial stack argument allocation, since we're only
 *		passing one argument into thread_base instead of 3.
 *	014	Dave Butenhof	23 March 1992
 *		Modify cma__vp_dump() to use redirectable I/O system.
 *	015	Dave Butenhof	31 March 1992
 *		Add Alpha support.
 *	016	Brian Keane	09 June 1992
 *		Resolve some 64 bit considerations for Alpha.
 *	017	Brian Keane	13 July 1992
 *		On Alpha/OSF, invoke cma__thread_base directly, rather than
 *		via assembly code.
 *	018	Brian Keane	23 July 1992
 *		Zero out the thread state before setting interesting
 *		registers.
 *	019	Dave Butenhof	27 July 1992
 *		Modify cma__vp_interrupt to support Alpha -- 64 bit data
 *		(long int) and different registers than MIPS. Also, move the
 *		functions with heavy machine-dependencies (vp_interrupt and
 *		vp_set_start) into machine-dependent modules to avoid a lot
 *		of messy conditional code.
 *	020	Dave Butenhof	04 August 1992
 *		Remove redundant \n in vp_dump format.
 *	021	Dave Butenhof	20 August 1992
 *		Add information to bugchecks.
 *	022	Dave Butenhof	 2 October 1992
 *		Add syscall() wrappers for RT functions.
 *	023	Dave Butenhof	 7 October 1992
 *		Contrary to expections (judging from sched.h), all OSF/1 RT
 *		scheduling policies have the same priority range (from
 *		SCHED_PRIO_USER_MIN to SCHED_PRIO_RT_MAX); adjust get_min and
 *		get_max functions appropriately.
 *	024	Dave Butenhof	 8 October 1992
 *		Fix typo in non-prototype function definition (sigh).
 *	025	Dave Butenhof	 8 October 1992
 *		Ooops -- add include of policy.h for OSF/1.
 *	026	Dave Butenhof	 4 November 1992
 *		Add maintenance of vp->seq field.
 *	027	Dave Butenhof	Friday the 13th, November 1992
 *		Don't unlock the vp spinlock in the reinit "clear" function!
 *	028	Dave Butenhof	 3 December 1992
 *		Remove cma__vp_yield() to .h for performance.
 *	029	Dave Butenhof	 6 January 1993
 *		Improve error return from vp_set_sched.
 *	030	Dave Butenhof	27 January 1993
 *		Fix a bug in cma__vp_create() that could allow reusing a
 *		cached thread that's still running (caught between queuing
 *		itself and the actual Mach thread_suspend() call). Add a
 *		"zombie" queue for threads that kill themselves -- when
 *		creating a new thread, check for a "really dead" zombie if
 *		there are no cached threads in known-safe state.
 *	031	Dave Butenhof	28 January 1993
 *		Clean up (old business) by removing port_disable() call,
 *		which is obsolete.
 *	032	Dave Butenhof	29 January 1993
 *		Find the actual scheduling policy & priority of the default
 *		Mach thread when creating the default VP structure.
 *	033	Dave Butenhof	17 February 1993
 *		In cma__vp_get_state(), fall back to default policy/priority
 *		if RT option isn't present.
 *	034	Dave Butenhof	26 February 1993
 *		Integrate review comments (23 February 1993)
 *	035	Dave Butenhof	 1 March 1993
 *		Ooops -- the new static cma___g_vp_task ought to be
 *		Mach-only, but wasn't.
 *	036	Dave Butenhof	 3 March 1993
 *		thread_abort() a zombie thread when moving it to cache (or
 *		reusing it), to clear thread_suspend() kernel state.
 *	037	Dave Butenhof	 4 March 1993
 *		Add a "%s" to the format string in vp_dump, so it'll show the
 *		hold state.
 *	038	Dave Butenhof	 8 March 1993
 *		Move abort_thread() to vp_set_start where it can't be missed.
 *	039	Dave Butenhof	 9 March 1993
 *		Maintain new "fresh" bit (set on first thread start cycle).
 *	040	Dave Butenhof	10 March 1993
 *		Add tracing.
 *	041	Dave Butenhof	11 March 1993
 *		Fix a serious problem in cma__vp_reinit. Originally, we'd
 *		assumed that a fork() actually cloned the process with all
 *		threads and simply removed all but the calling mach thread.
 *		That, however, isn't true, and it didn't occur to me to
 *		rewrite the reinit code. It scans the queue of threads
 *		looking for "self" -- but there's no guarantee it'll be
 *		there! Instead, just start from scratch without making any
 *		assumptions.
 *	042	Dave Butenhof	25 March 1993
 *		Clean up some trace messages.
 *	043	Dave Butenhof	30 March 1993
 *		Modify vp_suspend to clear msg_id in RPC buffer before
 *		receiving -- to catch a successful return with stale
 *		(correct) data in the
 *		buffer.
 *	044	Dave Butenhof	 5 April 1993
 *		Add cma__vp_reclaim(), remove recycle ("fresh") bit and the
 *		"reset" pointer -- reclaim "zombie" VPs sychronously under
 *		control of higher level code.
 *	045	Dave Butenhof	14 April 1993
 *		Update queue macro usage to allow VAX builtins.
 *	046	Dave Butenhof	27 April 1993
 *		Lock VP database in vp_reclaim.
 *	047	Dave Butenhof	12 May 1993
 *		Cleanup resource handling -- prevent resource leak on failure
 *		in cma__vp_create, and add cma__vp_cache so higher level code
 *		can return unused VP if thread creation fails after VP is
 *		allocated (without the overhead of "reclaiming" it).
 *	048	Dave Butenhof	17 May 1993
 *		Fix cma__vp_cache() -- it stuck VP on cache queue without
 *		removing from current queue! (assertions are kinda nice).
 *	049	Dave Butenhof	28 May 1993
 *		Trace timeout on vp_suspend.
 *	050	Dave Butenhof	 1 June 1993
 *		vp_suspend_others() and vp_resume_others() should use
 *		tryspinlock -- they're only called from bugcheck and debug
 *		code, and shouldn't deadlock because spinlock was taken.
 *		Also, use thread_self() directly instead of vp_self(), since
 *		the latter needs the VP spinlock also (and suspend_others
 *		doesn't really need the vpstruct, only the port number).
 *	051	Dave Butenhof	 3 June 1993
 *		Clean up get_state -- clear all unused fields unless NDEBUG;
 *		then leave them all uninitialized for speed & let the caller
 *		beware.
 *	052	Brian Keane	15 June 1993
 *		Corrected some compiler glitches with DEC C on  DEC OSF/1 AXP.
 *	053	Brian Keane	1 July 1993
 *		Minor touch-ups to eliminate warnings with DEC C on OpenVMS AXP.
 */
 
/*
 *  INCLUDE FILES
 */

#include <cma.h>
#include <cma_defs.h>
#include <cma_thread.h>
#include <cma_vp.h>
#include <cma_vm.h>
#include <cma_assem.h>
#include <cma_int_errno.h>

#if _CMA_KTHREADS_ == _CMA__MACH
# if _CMA_RT4_KTHREAD_
    /*
     * If we're building for RT support, then define the H_RT symbol to get
     * the RT-specific Mach definitions.
     */
#  define H_RT 1
#  include <mach.h>
#  include <mach_error.h>
#  include <sys/habitat.h>
#  include <sys/rt_syscall.h>
#  include <mach/policy.h>
#  undef H_RT
# else
#  include <mach.h>
#  include <mach_error.h>
# endif
# include <mach/machine/syscall_sw.h>
#endif

/*
 * GLOBAL DATA
 */

#if !_CMA_UNIPROCESSOR_
cma__t_atomic_bit	cma__g_vp_lock = cma__c_tac_static_clear;

char	*cma__g_vp_statestr[cma__c_vp_st_len] = {
    "<unused>",				/* state 0 unused */
    "run",
    "stop",
    "susp",
    "hold",
    "<unknown-stop>",
    "<unknown-wait>",
    "<unknown>",
    "stopping"
    };

# if _CMA_KTHREADS_ == _CMA__MACH
char	*cma__g_mach_statestr[6] = {
    "<unused>",				/* state 0 unused */
    "running",
    "stopped",
    "waiting",
    "uninterruptible",
    "halted"
    };
# endif

#endif

/*
 * LOCAL DATA
 */

#if !_CMA_UNIPROCESSOR_
static cma__t_queue		cma___g_vp_queue;
static cma__t_queue		cma___g_vp_cache;
static cma_t_integer		cma___g_vp_seq;
# if _CMA_KTHREADS_ == _CMA__MACH
static task_t			cma___g_vp_task;	/* Process' task */
# endif
#endif

/*
 * LOCAL MACROS
 */

/*
 * LOCAL FUNCTIONS
 */

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	initialize the VP layer
 *
 *  FORMAL PARAMETERS:
 *
 *	none
 *
 *  IMPLICIT INPUTS:
 *
 *	the current kernel thread
 *
 *  IMPLICIT OUTPUTS:
 *
 *	none
 *
 *  FUNCTION VALUE:
 *
 *	none
 *
 *  SIDE EFFECTS:
 *
 *	none
 * 
 */
void
cma__init_vp
#ifdef _CMA_PROTO_
	(void)
#else	/* no prototypes */
	()
#endif	/* prototype */
    {
#if _CMA_KTHREADS_ == _CMA__MACH
    kern_return_t	status;
    thread_t		virtual_processor;
    port_t		synch_port;
    cma__t_vpid		vp_struct;
    cma__t_vp_state	state;


    cma___g_vp_task = task_self ();
    cma__trace ((
	    cma__c_trc_init | cma__c_trc_vp,
	    "(init_vp) running in task %ld",
	    cma___g_vp_task));

    /*
     * Set up default VP
     */
    cma__queue_init (&cma___g_vp_queue);
    cma__queue_init (&cma___g_vp_cache);
    cma___g_vp_seq = 1;
    virtual_processor = thread_self ();
    status = port_allocate (cma___g_vp_task, &synch_port);

    cma__trace ((
	    cma__c_trc_init | cma__c_trc_vp,
	    "(init_vp) running in vp %ld, synch port is %ld",
	    virtual_processor,
	    synch_port));

    if (status != KERN_SUCCESS)
	cma__bugcheck (
		"init_vp: %s (%ld) port_allocate(vp %ld)",
		mach_error_string (status),
		status,
		virtual_processor);
	    
    if ((status = port_set_backlog (cma___g_vp_task, synch_port, 1)) != KERN_SUCCESS)
	cma__bugcheck (
		"init_vp: %s (%ld) port_set_backlog(%ld)",
		mach_error_string (status),
		status,
		synch_port);

    vp_struct = cma__alloc_object (cma__t_vstruct);

    if ((cma_t_address)vp_struct == cma_c_null_ptr)
	cma__bugcheck ("init_vp: unable to allocate VP struct");

    vp_struct->vp = virtual_processor;
    vp_struct->synch = synch_port;
    vp_struct->flags = cma__c_vp_running | cma__c_vp_default;
    vp_struct->seq = cma___g_vp_seq++;
    cma__queue_insert (&vp_struct->queue, &cma___g_vp_queue);

# if _CMA_RT4_KTHREAD_
    /*
     * Get the actual Mach thread policy and priority, so the VP structure is
     * accurate. cma__init_sched() will pop this into the default TCB, as well.
     */
    cma__vp_get_state (vp_struct, &state);
    cma__trace ((
	    cma__c_trc_vp,
	    "(init_vp) default vp has policy %d, priority %d",
	    state.policy, state.priority));
    vp_struct->policy = state.policy;
    vp_struct->priority = state.priority;
# else
    vp_struct->policy = cma_c_sched_throughput;
    vp_struct->priority = cma_c_prio_through_mid;
# endif
#endif
    }

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	cma__reinit_vp:  Do pre/post re-Initialize 
 *
 *  FORMAL PARAMETERS:
 *
 *	flag
 *
 *  IMPLICIT INPUTS:
 *
 *	None
 *
 *  IMPLICIT OUTPUTS:
 *
 *	None
 *
 *  FUNCTION VALUE:
 *
 *	None
 *
 *  SIDE EFFECTS:
 *
 *	None
 */
extern void
cma__reinit_vp
#ifdef _CMA_PROTO_
	(
	cma_t_integer	    flag)
#else	/* no prototypes */
	(flag)
	cma_t_integer	    flag;
#endif	/* prototype */
    {
#if _CMA_KTHREADS_ == _CMA__MACH

    switch (flag) {
	case cma__c_reinit_prefork_lock : {
	    cma__spinlock (&cma__g_vp_lock);
	    break;
	    }
	case cma__c_reinit_postfork_unlock : {
	    cma__spinunlock (&cma__g_vp_lock);
	    break;
	    }
	case cma__c_reinit_postfork_clear : {
	    thread_t		self;
	    cma__t_queue	*qptr;
	    cma__t_vpid		new_vpid;
	    kern_return_t	status;
	    port_t		synch_port;
	    cma__t_vp_state	state;


	    /*
	     * Remove all active VP structures, except for the VP that's
	     * currently running.
	     */
	    cma___g_vp_task = task_self ();
	    cma___g_vp_seq = 1;
	    self = thread_self ();

	    qptr = cma__queue_next (&cma___g_vp_queue);

	    while (qptr != &cma___g_vp_queue) {
		cma__t_vpid	vptr = (cma__t_vpid)qptr;
		cma__t_queue	*qnext = cma__queue_next (qptr);


		cma__queue_remove (qptr, qptr, cma__t_queue);
		cma__free_mem ((cma_t_address)vptr);
		qptr = qnext;
		}

	    /*
	     * Clear out any cached VPs
	     */
	    qptr = cma__queue_next (&cma___g_vp_cache);

	    while (qptr != &cma___g_vp_cache) {
		cma__t_vpid	vptr = (cma__t_vpid)qptr;
		cma__t_queue	*qnext = cma__queue_next (qptr);


		cma__queue_remove (qptr, qptr, cma__t_queue);
		cma__free_mem ((cma_t_address)vptr);
		qptr = qnext;
		}

	    status = port_allocate (cma___g_vp_task, &synch_port);

	    cma__trace ((
		    cma__c_trc_init | cma__c_trc_vp,
		    "(reinit_vp) running in vp %ld, synch port is %ld",
		    self,
		    synch_port));

	    if (status != KERN_SUCCESS)
		cma__bugcheck (
			"reinit_vp: %s (%ld) port_allocate(%ld)",
			mach_error_string (status),
			status,
			self);

	    if ((status = port_set_backlog (cma___g_vp_task, synch_port, 1)) != KERN_SUCCESS)
		cma__bugcheck (
			"reinit_vp: %s (%ld) port_set_backlog(%ld)",
			mach_error_string (status),
			status,
			synch_port);

	    new_vpid = cma__alloc_object (cma__t_vstruct);

	    if ((cma_t_address)new_vpid == cma_c_null_ptr)
		cma__bugcheck ("reinit_vp: unable to allocate VP struct");

	    new_vpid->vp = self;
	    new_vpid->synch = synch_port;
	    new_vpid->flags = cma__c_vp_running | cma__c_vp_default;
	    new_vpid->seq = cma___g_vp_seq++;
	    cma__queue_insert (&new_vpid->queue, &cma___g_vp_queue);

# if _CMA_RT4_KTHREAD_
	    /*
	     * Get the actual Mach thread policy and priority, so the VP
	     * structure is accurate. cma__init_sched() will pop this into
	     * the default TCB, as well.
	     */
	    cma__vp_get_state (new_vpid, &state);
	    cma__trace ((
		    cma__c_trc_vp,
		    "(reinit_vp) default vp (%ld) has policy %d, priority %d",
		    new_vpid->vp,
		    state.policy, state.priority));
	    new_vpid->policy = state.policy;
	    new_vpid->priority = state.priority;
# else
	    new_vpid->policy = cma_c_sched_throughput;
	    new_vpid->priority = cma_c_prio_through_mid;
# endif

	    break;
	    }

	}

#else
    cma__bugcheck ("vp_reinit: no VPs");
#endif
    }

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Return a VP that was created but not used to cache.
 *
 *  FORMAL PARAMETERS:
 *
 *	vpid		the id of the virtual processor
 *
 *  IMPLICIT INPUTS:
 *
 *	none
 *
 *  IMPLICIT OUTPUTS:
 *
 *	none
 *
 *  FUNCTION VALUE:
 *
 *	none
 *
 *  SIDE EFFECTS:
 *
 *	none
 * 
 */
void
cma__vp_cache
#ifdef _CMA_PROTO_
	(cma__t_vpid		vpid)
#else	/* no prototypes */
	(vpid)
	cma__t_vpid		vpid;
#endif	/* prototype */
    {
    cma__t_queue	*tvq;


#if _CMA_KTHREADS_ == _CMA__MACH
    cma__trace ((
	    cma__c_trc_vp,
	    "(vp_cache) caching unused vp %d",
	    vpid->vp));

    vpid->flags = cma__c_vp_cached;
    vpid->seq = 0;			/* Reset sequence number */
    cma__spinlock (&cma__g_vp_lock);
    cma__queue_remove (&vpid->queue, tvq, cma__t_queue);
    cma__queue_insert (&vpid->queue, &cma___g_vp_cache);
    cma__spinunlock (&cma__g_vp_lock);
#endif
    }

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Create a suspended vp
 *
 *  FORMAL PARAMETERS:
 *
 *	vpid		return the id of the new virtual processor
 *
 *  IMPLICIT INPUTS:
 *
 *	none
 *
 *  IMPLICIT OUTPUTS:
 *
 *	none
 *
 *  FUNCTION VALUE:
 *
 *	status of operation
 *
 *  SIDE EFFECTS:
 *
 *	none
 * 
 */
cma__t_vp_status
cma__vp_create
#ifdef _CMA_PROTO_
	(cma__t_vpid		*vpid)
#else	/* no prototypes */
	(vpid)
	cma__t_vpid		*vpid;
#endif	/* prototype */
    {
#if _CMA_KTHREADS_ == _CMA__MACH
    kern_return_t	status;
    thread_t		virtual_processor;
    port_t		synch_port;
    cma__t_vpid		vp_struct;


    /*
     * Lock the VP structures and check for a cached VP to save the trouble
     * of creating a new one.
     */
    cma__spinlock (&cma__g_vp_lock);

    if (!cma__queue_empty (&cma___g_vp_cache)) {
	cma__queue_dequeue (&cma___g_vp_cache, vp_struct, cma__t_vstruct);
	cma__trace ((
		cma__c_trc_vp,
		"(vp_create) using cached vp %ld",
		vp_struct->vp));
	}
    else {				/* Create a new VP */
	cma__t_vp_state	state;


	cma__spinunlock (&cma__g_vp_lock);
	vp_struct = cma__alloc_object (cma__t_vstruct);

	if ((cma_t_address)vp_struct == cma_c_null_ptr)
	    return cma__c_vp_insfmem;

	status = thread_create (cma___g_vp_task, &virtual_processor);

	if (status != KERN_SUCCESS) {
	    cma__free_mem ((cma_t_address)vp_struct);
	    return (cma__t_vp_status)status;
	    }

	status = port_allocate (cma___g_vp_task, &synch_port);

	if (status != KERN_SUCCESS) {
	    kern_return_t	ts;

	    if ((ts = thread_terminate (virtual_processor)) != KERN_SUCCESS)
		cma__bugcheck ("vp_create: %s (%ld) thread_terminate(%ld)",
			mach_error_string (ts),
			ts,
			virtual_processor);

	    cma__free_mem ((cma_t_address)vp_struct);
	    return (cma__t_vp_status)status;
	    }

	if ((status = port_set_backlog (cma___g_vp_task, synch_port, 1)) != KERN_SUCCESS)
	    cma__bugcheck (
		    "vp_create: %s (%ld) port_set_backlog(%ld)",
		    mach_error_string (status),
		    status,
		    synch_port);

	vp_struct->vp = virtual_processor;
	vp_struct->synch = synch_port;
# if _CMA_RT4_KTHREAD_
	/*
	 * Get the actual Mach thread policy and priority, so the VP
	 * structure is accurate. cma__init_sched() will pop this into the
	 * default TCB, as well.
	 */
	cma__vp_get_state (vp_struct, &state);
	vp_struct->policy = state.policy;
	vp_struct->priority = state.priority;
# else
	vp_struct->policy = cma_c_sched_throughput;
	vp_struct->priority = cma_c_prio_through_mid;
# endif
	cma__trace ((
		cma__c_trc_vp,
		"(vp_create) new vp %d; policy %d, priority %d",
		vp_struct->vp, vp_struct->policy, vp_struct->priority));
	cma__spinlock (&cma__g_vp_lock);
	}

    vp_struct->flags = cma__c_vp_new;
    vp_struct->seq = cma___g_vp_seq++;
    cma__queue_insert (&vp_struct->queue, &cma___g_vp_queue);
    cma__spinunlock (&cma__g_vp_lock);
    *vpid = vp_struct;
    return cma__c_vp_normal;
#else
    cma__bugcheck ("vp_create: no VPs");	/* no VP creation on uniprocessor */
    return -1;
#endif
    }

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	delete a VP (should be the current vp).
 *
 *  FORMAL PARAMETERS:
 *
 *	vpid		Which VP to delete
 *
 *  IMPLICIT INPUTS:
 *
 *	none
 *
 *  IMPLICIT OUTPUTS:
 *
 *	none
 *
 *  FUNCTION VALUE:
 *
 *	status of operation
 *
 *  SIDE EFFECTS:
 *
 *	none
 * 
 */
void
cma__vp_delete
#ifdef _CMA_PROTO_
	(cma__t_vpid		vpid)
#else	/* no prototypes */
	(vpid)
	cma__t_vpid		vpid;
#endif	/* prototype */
    {
#if _CMA_KTHREADS_ == _CMA__MACH
    kern_return_t	status;
    thread_t		vp;


    cma__spinlock (&cma__g_vp_lock);
    cma__queue_remove (&vpid->queue, vpid, cma__t_vstruct);

    /*
     * Mark thread as "zombie" (not running, suspended, or cached)
     */
    vpid->flags = cma__c_vp_zombie;
    cma__spinunlock (&cma__g_vp_lock);

    cma__trace ((
	    cma__c_trc_vp,
	    "(vp_delete) vp %ld terminating",
	    vpid->vp));

    /*
     * Suspend the thread, so that when it's pulled off the cache queue it'll
     * be OK to yank it to a new PC/SP with thread_set_state().
     */
    if ((status = thread_suspend (vpid->vp)) != KERN_SUCCESS)
	cma__bugcheck (
		"vp_delete: %s (%ld) thread_suspend(%ld)",
		mach_error_string (status),
		status,
		vpid->vp);

    cma__bugcheck ("vp_delete: %ld resumed without yank", vpid->vp);
#else
    cma__bugcheck ("vp_delete: no VPs");
#endif
    }

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Dump the queues of active and cached VPs for debugging. (This is
 *	compiled unconditionally because there is no cost unless it is
 *	called).
 *
 *  FORMAL PARAMETERS:
 *
 *	none
 *
 *  IMPLICIT INPUTS:
 *
 *	the VP queues
 *
 *  IMPLICIT OUTPUTS:
 *
 *	none
 *
 *  FUNCTION VALUE:
 *
 *	none
 *
 *  SIDE EFFECTS:
 *
 *	print information to stdout
 * 
 */
void
cma__vp_dump
#ifdef _CMA_PROTO_
	(void)
#else	/* no prototypes */
	()
#endif	/* prototype */
    {
#if _CMA_KTHREADS_ == _CMA__MACH
    cma__t_queue	*qptr;
    char		buffer[256];
    cma_t_boolean	lock;


    buffer[0] = '\0';
    lock = cma__tryspinlock (&cma__g_vp_lock);

    /*
     * Report active VPs
     */
    qptr = cma__queue_next (&cma___g_vp_queue);

    while (qptr != &cma___g_vp_queue) {
	cma__t_vpid	vptr = (cma__t_vpid)qptr;


	cma__putformat (
		buffer,
		"VPa 0x%lx: port %ld, synch %ld, seq %ld, flags-%s%s%s%s%s%s%s",
		vptr,
		vptr->vp,
		vptr->synch,
		vptr->seq,
		(vptr->flags & cma__c_vp_new ? ":new" : ""),
		(vptr->flags & cma__c_vp_running ? ":running" : ""),
		(vptr->flags & cma__c_vp_susp ? ":suspend" : ""),
		(vptr->flags & cma__c_vp_zombie ? ":zombie" : ""),
		(vptr->flags & cma__c_vp_cached ? ":cached" : ""),
		(vptr->flags & cma__c_vp_default ? ":default" : ""),
		(vptr->flags & cma__c_vp_hold ? ":hold" : ""));
	cma__puteol (buffer);
	qptr = cma__queue_next (qptr);
	}

    /*
     * Report cached VPs
     */
    qptr = cma__queue_next (&cma___g_vp_cache);

    while (qptr != &cma___g_vp_cache) {
	cma__t_vpid	vptr = (cma__t_vpid)qptr;


	cma__putformat (
		buffer,
		"VPc 0x%lx: port %ld, synch %ld, seq %ld, flags-%s%s%s%s%s%s%s",
		vptr,
		vptr->vp,
		vptr->synch,
		vptr->seq,
		(vptr->flags & cma__c_vp_new ? ":new" : ""),
		(vptr->flags & cma__c_vp_running ? ":running" : ""),
		(vptr->flags & cma__c_vp_susp ? ":suspend" : ""),
		(vptr->flags & cma__c_vp_zombie ? ":zombie" : ""),
		(vptr->flags & cma__c_vp_cached ? ":cached" : ""),
		(vptr->flags & cma__c_vp_default ? ":default" : ""),
		(vptr->flags & cma__c_vp_hold ? ":hold" : ""));
	cma__puteol (buffer);
	qptr = cma__queue_next (qptr);
	}

    if (lock)
	cma__spinunlock (&cma__g_vp_lock);

#endif
    }

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	return ID of current VP
 *
 *  FORMAL PARAMETERS:
 *
 *	none
 *
 *  IMPLICIT INPUTS:
 *
 *	the current kernel thread
 *
 *  IMPLICIT OUTPUTS:
 *
 *	none
 *
 *  FUNCTION VALUE:
 *
 *	kernel thread ID
 *
 *  SIDE EFFECTS:
 *
 *	none
 * 
 */
cma__t_vpid
cma__vp_get_id
#ifdef _CMA_PROTO_
	(void)
#else	/* no prototypes */
	()
#endif	/* prototype */
    {
#if _CMA_KTHREADS_ == _CMA__MACH
    thread_t		virtual_processor;
    cma__t_queue	*qptr;


    virtual_processor = thread_self ();
    cma__spinlock (&cma__g_vp_lock);
    qptr = cma__queue_next (&cma___g_vp_queue);

    while (qptr != &cma___g_vp_queue) {
	cma__t_vpid	vptr = (cma__t_vpid)qptr;

	if (vptr->vp == virtual_processor) {
	    cma__spinunlock (&cma__g_vp_lock);
	    return vptr;
	    }

	qptr = cma__queue_next (qptr);
	}

    cma__spinunlock (&cma__g_vp_lock);
    cma__bugcheck ("vp_get_id: unknown VP %ld", virtual_processor);
#else
    return (cma__t_vpid)0;
#endif
    }

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	get state of VP
 *
 *  FORMAL PARAMETERS:
 *
 *	vpid		ID of kernel thread
 *	state		state array
 *
 *  IMPLICIT INPUTS:
 *
 *	none
 *
 *  IMPLICIT OUTPUTS:
 *
 *	none
 *
 *  FUNCTION VALUE:
 *
 *	status
 *
 *  SIDE EFFECTS:
 *
 *	none
 * 
 */
cma__t_vp_status
cma__vp_get_state
#ifdef _CMA_PROTO_
	(cma__t_vpid	vpid,
	cma__t_vp_state	*state)
#else	/* no prototypes */
	(vpid, state)
	cma__t_vpid	vpid;
	cma__t_vp_state	*state;
#endif	/* prototype */
    {
#if _CMA_KTHREADS_ == _CMA__MACH
    kern_return_t		status;
    thread_info_data_t		info_array;
    thread_basic_info_t		info = (thread_basic_info_t)&info_array[0];
    unsigned int		info_count = THREAD_INFO_MAX;
    int				syscall_num;
    int				kpriority, kpolicy;


    status = thread_info (
	    vpid->vp,
	    THREAD_BASIC_INFO,
	    (thread_info_t)info_array,
	    &info_count);

    if (status != KERN_SUCCESS)
	return (cma__t_vp_status)status;

    syscall_num =
	((HABITAT_STD_CALL(rt_getprio) - HABITAT_BASE)) | HABITAT_INDEX;
    kpriority = syscall (
	    syscall_num,		/* Syscall habitat number */
	    PRIO_POSIX,			/* Get POSIX priority */
	    0,				/* Use current process always */
	    0,				/* <data>:reserved */
	    0,				/* <sched_param_version>:reserved */
	    vpid->vp);			/* The thread port */

    /*
     * If RT option isn't installed, don't bother with second kernel call.
     */
    if (kpriority == -1) {
	/*
	 * Because there's (probably) no RT option installed, all threads
	 * will behave as if they were throughput policy with "mid" priority;
	 * which is the DECthreads default. So we might as well just lie.
	 */
	state->policy = cma_c_sched_throughput;
	state->priority = cma_c_prio_through_mid;
	}
    else {
	kpolicy = syscall (
		syscall_num,		/* Syscall habitat number */
		PRIO_POSIX | PRIO_POLICY,	/* Get POSIX policy */
		0,			/* Use current process always */
		0,			/* <data>:reserved */
		0,			/* <sched_param_version>:reserved */
		vpid->vp);		/* The thread port */

	/*
	 * DEC OSF/1 RT doesn't implement all of the DECthreads scheduling
	 * policies directly -- this switch statement attempts to decode the
	 * Mach thread policy and priority into DECthreads policies.
	 */
	switch (kpolicy) {
	    case POLICY_FIFO :

		if (kpriority >= cma_c_prio_fifo_min)
		    state->policy = cma_c_sched_fifo;
		else
		    state->policy = cma_c_sched_ada_rtb;

		break;
	    case POLICY_RR :

		if (kpriority >= cma_c_prio_rr_min)
		    state->policy = cma_c_sched_rr;
		else
		    state->policy = cma_c_sched_ada_low;

		break;
	    case POLICY_TIMESHARE :
	    case POLICY_FIXEDPRI :

		if (kpriority >= cma_c_prio_through_min)
		    state->policy = cma_c_sched_throughput;
		else
		    state->policy = cma_c_sched_background;

		break;
	    default :
		state->policy = 0;
		kpriority = 0;
		break;
	    }

	state->priority = kpriority;
	}

#ifndef NDEBUG
    /*
     * The stack, start_routine, and start_arg fields in the state array
     * really aren't recoverable. We could return current PC and SP, except
     * that requires an extra Mach call (thread_get_state) as well as some
     * precautionary logic since it won't work on a thread that's running.
     * The simplest solution is to simply not worry about it, and just zero
     * the fields. (which we do only for !NDEBUG, to simplify debugging; and
     * skip entirely for production builds to save time).
     */
    state->tcb = 0;
    state->start_routine = 0;
    state->start_arg = 0;
    state->stack = 0;
#endif
    state->mach_state = info->run_state;
    state->suspend_count = info->suspend_count;

    switch (info->run_state) {
	case TH_STATE_STOPPED : {

	    if (vpid->flags & (cma__c_vp_cached | cma__c_vp_zombie))
		state->run_state = cma__c_vp_st_stop;
	    else if (vpid->flags & cma__c_vp_hold) {
		state->mach_state = vpid->hold_mach;
		state->run_state = vpid->hold_run;
		state->suspend_count = vpid->hold_suspend;
		}
	    else
		state->run_state = cma__c_vp_st_unks;

	    break;
	    }
	
	case TH_STATE_WAITING : {

	    if (vpid->flags & cma__c_vp_susp)
		state->run_state = cma__c_vp_st_susp;
	    else
		state->run_state = cma__c_vp_st_unkw;

	    break;
	    }

	case TH_STATE_RUNNING : {

	    if (vpid->flags & cma__c_vp_zombie)
		state->run_state = cma__c_vp_st_stopping;
	    else
		state->run_state = cma__c_vp_st_run;

	    break;
	    }

	default : {
	    state->run_state = cma__c_vp_st_unk;
	    break;
	    }

	}

    cma__trace ((
	    cma__c_trc_vp,
	    "(vp_get_state) vp %ld [vp %s, mach %s(%d)]",
	    vpid->vp,
	    cma__g_vp_statestr[state->run_state],
	    cma__g_mach_statestr[state->mach_state],
	    state->suspend_count));

    return cma__c_vp_normal;
#else
    cma__bugcheck ("vp_get_state: no VPs");
    return -1;
#endif
    }

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	reclaim a VP from a terminated (zombie) thread.
 *
 *  FORMAL PARAMETERS:
 *
 *	vpid		Which VP to attempt to reclaim
 *
 *  IMPLICIT INPUTS:
 *
 *	none
 *
 *  IMPLICIT OUTPUTS:
 *
 *	none
 *
 *  FUNCTION VALUE:
 *
 *	cma_c_true if VP was reclaimed; cma_c_false if still running
 *
 *  SIDE EFFECTS:
 *
 *	none
 * 
 */
cma_t_boolean
cma__vp_reclaim
#ifdef _CMA_PROTO_
	(cma__t_vpid		vpid)
#else	/* no prototypes */
	(vpid)
	cma__t_vpid		vpid;
#endif	/* prototype */
    {
#if _CMA_KTHREADS_ == _CMA__MACH
    kern_return_t	status;
    cma__t_vp_state	state;

    status = cma__vp_get_state (vpid, &state);

    if (status != cma__c_vp_normal || state.mach_state != TH_STATE_STOPPED)
	return cma_c_false;		/* Unable to reclaim */

    cma__trace ((
	    cma__c_trc_vp,
	    "(vp_reclaim) reclaimed zombie vp %d",
	    vpid->vp));

    if ((status = thread_abort (vpid->vp)) != KERN_SUCCESS)
	cma__bugcheck (
		"vp_reclaim: %s (%ld) thread_abort(%ld)",
		mach_error_string (status),
		status,
		vpid->vp);

    vpid->flags = cma__c_vp_cached;
    vpid->seq = 0;			/* Reset sequence number */
    cma__spinlock (&cma__g_vp_lock);
    cma__queue_insert (&vpid->queue, &cma___g_vp_cache);
    cma__spinunlock (&cma__g_vp_lock);
    return cma_c_true;			/* Yes, we reclaimed it */
#else
    cma__bugcheck ("vp_reclaim: no VPs");
    return -1;
#endif
    }

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	resume a suspended VP
 *
 *  FORMAL PARAMETERS:
 *
 *	vpid		Which VP to resume
 *
 *  IMPLICIT INPUTS:
 *
 *	none
 *
 *  IMPLICIT OUTPUTS:
 *
 *	none
 *
 *  FUNCTION VALUE:
 *
 *	status of operation
 *
 *  SIDE EFFECTS:
 *
 *	none
 * 
 */
cma__t_vp_status
cma__vp_resume
#ifdef _CMA_PROTO_
	(cma__t_vpid		vpid)
#else	/* no prototypes */
	(vpid)
	cma__t_vpid		vpid;
#endif	/* prototype */
    {
#if _CMA_KTHREADS_ == _CMA__MACH
    kern_return_t	status;


    if (!(vpid->flags & cma__c_vp_new)) {
	msg_header_t	mhdr;


	cma__trace ((
		cma__c_trc_vp,
		"(vp_resume) wake vp %ld from vp %ld",
		vpid->vp,
		thread_self()));

	mhdr.msg_simple = cma_c_true;
	mhdr.msg_size = sizeof (mhdr);
	mhdr.msg_type = MSG_TYPE_NORMAL;
	mhdr.msg_local_port = PORT_NULL;
	mhdr.msg_remote_port = vpid->synch;
	mhdr.msg_id = cma__c_vp_msg_resume;

	if ((status = msg_send (&mhdr, MSG_OPTION_NONE, 0)) != SEND_SUCCESS)
	    cma__bugcheck (
		    "vp_resume: %s (%ld) msg_send(vp %ld, port %ld)",
		    mach_error_string (status),
		    status,
		    vpid->vp,
		    vpid->synch);

	}
    else {
	cma__trace ((
		cma__c_trc_vp,
		"(vp_resume) start vp %ld from vp %ld",
		vpid->vp,
		thread_self()));

	vpid->flags &= ~cma__c_vp_new;
	vpid->flags |= cma__c_vp_running;

	/*
	 * Unsuspend the thread.
	 */
	if ((status = thread_resume (vpid->vp)) != KERN_SUCCESS)
	    cma__bugcheck (
		    "vp_resume: %s (%ld) thread_resume(%ld)",
		    mach_error_string (status),
		    status,
		    vpid->vp);

	}
#else
    cma__bugcheck ("vp_resume: no VPs");
    return -1;
#endif
    }

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	resume all VPs held for debugging
 *
 *  FORMAL PARAMETERS:
 *
 *	none
 *
 *  IMPLICIT INPUTS:
 *
 *	queue of VPs
 *
 *  IMPLICIT OUTPUTS:
 *
 *	none
 *
 *  FUNCTION VALUE:
 *
 *	status of operation
 *
 *  SIDE EFFECTS:
 *
 *	none
 * 
 */
cma__t_vp_status
cma__vp_resume_others
#ifdef _CMA_PROTO_
	(void)
#else	/* no prototypes */
	()
#endif	/* prototype */
    {
#if _CMA_KTHREADS_ == _CMA__MACH
    cma__t_queue	*qptr;
    kern_return_t	status;
    cma_t_boolean	lock;


    cma__trace ((
	    cma__c_trc_vp,
	    "(vp_resume_others) from vp %ld",
	    thread_self ()));

    lock = cma__tryspinlock (&cma__g_vp_lock);
    qptr = cma__queue_next (&cma___g_vp_queue);

    while (qptr != &cma___g_vp_queue) {
	cma__t_vpid	vptr = (cma__t_vpid)qptr;

	if (vptr->flags & cma__c_vp_hold) {

	    if ((status = thread_resume (vptr->vp)) != KERN_SUCCESS) {

		if (lock)
		    cma__spinunlock (&cma__g_vp_lock);

		cma__bugcheck (
			"vp_resume_others: %s (%ld) thread_resume",
			mach_error_string (status),
			status,
			vptr->vp);
		}

	    vptr->flags &= ~cma__c_vp_hold;
	    }

	qptr = cma__queue_next (qptr);
	}

    if (lock)
	cma__spinunlock (&cma__g_vp_lock);

    return cma__c_vp_normal;
#else
    cma__bugcheck ("vp_resume_others: no VPs");
    return -1;
#endif
    }

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	set scheduling info on a VP
 *
 *  FORMAL PARAMETERS:
 *
 *	vpid		Which VP to change
 *	policy		scheduling policy
 *	priority	scheduling priority
 *
 *  IMPLICIT INPUTS:
 *
 *	none
 *
 *  IMPLICIT OUTPUTS:
 *
 *	none
 *
 *  FUNCTION VALUE:
 *
 *	status of operation
 *
 *  SIDE EFFECTS:
 *
 *	none
 * 
 */
cma__t_vp_status
cma__vp_set_sched
#ifdef _CMA_PROTO_
	(cma__t_vpid		vpid,
	cma_t_sched_policy	policy,
	cma_t_priority		priority)
#else	/* no prototypes */
	(vpid, policy, priority)
	cma__t_vpid		vpid;
	cma_t_sched_policy	policy;
	cma_t_priority		priority;
#endif	/* prototype */
    {
#if _CMA_KTHREADS_ == _CMA__MACH
    kern_return_t	status;
    int			syscall_num;
    int			kpolicy;


    switch (policy) {
	case cma_c_sched_fifo :
	case cma_c_sched_ada_rtb : 
	    kpolicy = PRIO_POSIX | PRIO_POLICY_FIFO;
	    break;
	case cma_c_sched_rr :
	case cma_c_sched_ada_low : 
	    kpolicy = PRIO_POSIX | PRIO_POLICY_RR;
	    break;
	default :
	    kpolicy = PRIO_POSIX | PRIO_POLICY_OTHER;
	}
	
    syscall_num =
	((HABITAT_STD_CALL(rt_setprio) - HABITAT_BASE)) | HABITAT_INDEX;
    status = syscall (
	    syscall_num,		/* Syscall habitat number */
	    kpolicy,			/* POSIX policy */
	    0,				/* Use current process always */
	    priority,			/* Pass on the priority */
	    0,				/* <data>:reserved */
	    0,				/* <sched_param_version>:reserved */
	    vpid->vp);			/* The thread port */

    if (status == -1) {

	if (cma__get_errno() == EPERM)
	    status = cma__c_vp_err_nopriv;
	else
	    status = cma__c_vp_failure;

	cma__trace ((
		cma__c_trc_vp,
		"(vp_set_sched) on thread %ld (%d/%d): %d (%d=\"%s\")",
		vpid->vp,
		policy, priority,
		status,
		cma__get_errno(), strerror (cma__get_errno())));
	}
    else {
	cma__trace ((
		cma__c_trc_vp,
		"(vp_set_sched) on thread %ld (%d/%d) OK (prev %d)",
		vpid->vp,
		policy, priority,
		status));
	status = cma__c_vp_normal;
	}

    vpid->policy = policy;
    vpid->priority = priority;

    return (cma__t_vp_status)status;
#else
    cma__bugcheck ("vp_set_sched: no VPs");
    return -1;
#endif
    }

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	suspend the current VP
 *
 *  FORMAL PARAMETERS:
 *
 *	vpid		Which VP to suspend (must be caller!)
 *	milliseconds	Timeout value (0 means no timeout)
 *
 *  IMPLICIT INPUTS:
 *
 *	none
 *
 *  IMPLICIT OUTPUTS:
 *
 *	none
 *
 *  FUNCTION VALUE:
 *
 *	status of operation
 *
 *  SIDE EFFECTS:
 *
 *	none
 * 
 */
cma__t_vp_status
cma__vp_suspend
#ifdef _CMA_PROTO_
	(cma__t_vpid		vpid,
	cma_t_integer		milliseconds)
#else	/* no prototypes */
	(vpid, milliseconds)
	cma__t_vpid		vpid;
	cma_t_integer		milliseconds;
#endif	/* prototype */
    {
#if _CMA_KTHREADS_ == _CMA__MACH
    kern_return_t	status;
    msg_header_t	mhdr;
    msg_option_t	mopt;


    cma__trace ((
	    cma__c_trc_vp,
	    "(vp_suspend) vp %ld sleeping for %ldmS",
	    vpid->vp,
	    milliseconds));

    vpid->flags &= ~cma__c_vp_running;
    vpid->flags |= cma__c_vp_susp;
    mhdr.msg_simple = cma_c_true;
    mhdr.msg_size = sizeof (mhdr);
    mhdr.msg_local_port = vpid->synch;
    mhdr.msg_id = 0;

    if (milliseconds == 0)
	mopt = MSG_OPTION_NONE;
    else
	mopt = RCV_TIMEOUT;

    status = msg_receive (&mhdr, mopt, milliseconds);

    if (status == RCV_TIMED_OUT) {
	cma__trace ((
		cma__c_trc_vp,
		"(vp_suspend) vp %ld woke (timeout)",
		vpid->vp));
	return (cma__t_vp_status)status;
	}

    if (status != RCV_SUCCESS)
	cma__bugcheck (
		"vp_suspend: %s (%ld) msg_receive(vp %ld, port %ld)",
		mach_error_string (status),
		status,
		vpid->vp,
		vpid->synch);

    if (mhdr.msg_id != cma__c_vp_msg_resume)
	cma__bugcheck (
		"vp_suspend: msg %ld for vp %ld (port %ld) should be %ld",
		mhdr.msg_id,
		vpid->vp,
		vpid->synch,
		cma__c_vp_msg_resume);

    vpid->flags &= ~cma__c_vp_susp;
    vpid->flags |= cma__c_vp_running;    

    cma__trace ((
	    cma__c_trc_vp,
	    "(vp_suspend) vp %ld waking up",
	    vpid->vp));

    return cma__c_vp_normal;
#else
    cma__bugcheck ("vp_suspend: no VPs");
    return -1;
#endif
    }

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	suspend all other VPs for debugging
 *
 *  FORMAL PARAMETERS:
 *
 *	none
 *
 *  IMPLICIT INPUTS:
 *
 *	current VP, queue of VPs
 *
 *  IMPLICIT OUTPUTS:
 *
 *	none
 *
 *  FUNCTION VALUE:
 *
 *	status of operation
 *
 *  SIDE EFFECTS:
 *
 *	none
 * 
 */
cma__t_vp_status
cma__vp_suspend_others
#ifdef _CMA_PROTO_
	(void)
#else	/* no prototypes */
	()
#endif	/* prototype */
    {
#if _CMA_KTHREADS_ == _CMA__MACH
    thread_t		self;
    cma__t_queue	*qptr;
    cma_t_boolean	lock;


    self = thread_self ();

    cma__trace ((
	    cma__c_trc_vp,
	    "(vp_suspend_others) from vp %ld",
	    self));

    lock = cma__tryspinlock (&cma__g_vp_lock);
    qptr = cma__queue_next (&cma___g_vp_queue);

    while (qptr != &cma___g_vp_queue) {
	cma__t_vpid	vptr = (cma__t_vpid)qptr;

	if (self != vptr->vp && !(vptr->flags & cma__c_vp_hold)) {
	    cma__t_vp_status	status;
	    cma__t_vp_state	state;


	    vptr->flags |= cma__c_vp_hold;
	    cma__vp_get_state (vptr, &state);
	    vptr->hold_run = state.run_state;
	    vptr->hold_mach = state.mach_state;
	    vptr->hold_suspend = state.suspend_count;

	    if ((status = thread_suspend (vptr->vp)) != KERN_SUCCESS) {
		cma__trace ((
			cma__c_trc_vp | cma__c_trc_bug,
			"(vp_suspend_others) error \"%s\" (%ld) suspending vp %ld",
			mach_error_string (status),
			status,
			vptr->vp));
		}

	    }

	qptr = cma__queue_next (qptr);
	}

    if (lock)
	cma__spinunlock (&cma__g_vp_lock);

    return cma__c_vp_normal;
#else
    cma__bugcheck ("vp_suspend_others: no VPs");
    return -1;
#endif
    }
