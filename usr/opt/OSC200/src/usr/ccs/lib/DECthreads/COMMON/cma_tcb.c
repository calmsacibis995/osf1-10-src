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
static char *rcsid = "@(#)$RCSfile: cma_tcb.c,v $ $Revision: 4.2.7.4 $ (DEC) $Date: 1993/05/25 19:59:34 $";
#endif
/*
 *  FACILITY:
 *
 *	DECthreads services
 *
 *  ABSTRACT:
 *
 *	Manage TCB structures
 *
 *  AUTHORS:
 *
 *	Dave Butenhof
 *
 *  CREATION DATE:
 *
 *	24 July 1989
 *
 *  MODIFICATION HISTORY:
 *
 *	001	Dave Butenhof	15 August 1989
 *		Add type when initializing new TCB structure.
 *	002	Dave Butenhof	21 August 1989
 *		Manage stack list off TCB.
 *	003	Dave Butenhof	24 August 1989
 *		Convert to use queues.
 *	004	Dave Butenhof	31 August 1989
 *		Modify to handle passing handles by value.
 *	005	Dave Butenhof	15 September 1989
 *		Improve caching by using attribute revision counts.
 *	006	Dave Butenhof	29 September 1989
 *		Flush non-default stacks on free rather than on get.
 *	007	Dave Butenhof	05 October 1989
 *		Assign thread's stack chunks.
 *	008	Dave Butenhof	12 October 1989
 *		Convert to use internal mutex operations.
 *	009	Dave Butenhof	18 October 1989
 *		Convert use of a queue operation as the argument of another
 *		queue operation to use a temporary (since the queue ops are
 *		now macros).
 *	010	Dave Butenhof	19 October 1989
 *		Substitute "cma_t_address" for explicit "void *" to make
 *		porting easier.
 *	011	Dave Butenhof	19 October 1989
 *		Modify use of queue operations to use appropriate casts
 *		rather than depending on ANSI C "void *" (sigh).
 *	012	Dave Butenhof	20 October 1989
 *		Avoid redundant clearing of context array when TCB is reused:
 *		context rundown will do that when the thread terminates.
 *	013	Webb Scales	20 October 1989
 *		Create a "definition" for cma__g_def_tcb_ptr.
 *	014	Dave Butenhof	24 October 1989
 *		Enhance free & destroy to handle attr. obj. delete-pending
 *		and refcnt.
 *	015	Dave Butenhof	27 October 1989
 *		Initialize new "wait_cv" field in TCB at get (so alert code
 *		can tell thread isn't waiting).
 *	016	Dave Butenhof	1 November 1989
 *		Add backpointer for mutex.
 *	017	Dave Butenhof	5 December 1989
 *		Remove initial_test attribute.
 *	018	Dave Butenhof	8 December 1989
 *		Initialize new "exc_stack" field to cma_c_null_ptr (head of
 *		per-thread exception stack).
 *	019	Dave Butenhof	12 February 1990
 *		Initialize errno field in TCB
 *	020	Dave Butenhof	5 April 1990
 *		Create embedded objects using the same attributes object that
 *		the parent object was created with, to avoid worrying about
 *		locking order protocols during access and deletion.
 *	021	Dave Butenhof	10 April 1990
 *		Catch memory errors over object allocation, and set names in
 *		internal objects.
 *	022	Dave Butenhof	12 April 1990
 *		Give names to all mutex & cv objects embedded in TCB.
 *	023	Webb Scales	30 May 1990
 *		Initialized new reference count in tcb for joiners.
 *	024	Dave Butenhof	5 June 1990
 *		Handle TCB cache high water mark in cma__free_tcb.
 *	025	Dave Butenhof	7 June 1990
 *		Erase destroyed objects if not NDEBUG, to catch references to
 *		dead objects (can be used in conjunction with cma_maxthread=0,
 *		which disables caching).
 *	026	Webb Scales	15 June 1990
 *		Added priority scheduling and rearranged the TCB.
 *	027	Bob Conti	12 September 1990
 *		Added initialization for debug fields.
 *		Created the tswait CV.
 *	028	Webb Scales	13 September 1990
 *		Changed tswait to internal CV.
 *	029	Bob Conti	14 September 1990
 *		Add missing init for errno and exc_stack in default TCB
 *		Add init for new thread kind field
 *	030	Webb Scales	20 September 1990
 *		Added a mutex for thread-synchronous wait CV.
 *	031	Paul Curtin	19 December 1990
 *		Added an enter kernel around known threads list op.
 *	032	Dave Butenhof	5 February 1991
 *		Rename "errno" field.
 *	033	Dave Butenhof	7 February 1991
 *		Adapt to changes in alert bits.
 *	034	Dave Butenhof	25 March 1991
 *		Use exc_handling.h defs rather than cma_exception.h
 *	035	Paul Curtin	8 April 1991
 *		Added globals for variables for quick kill.
 *	036	Dave Butenhof	09 April 1991
 *		Don't reference kernel_critical flag directly
 *	037	Paul Curtin	30 April 1991
 *		Added initialization for preallocated keys (10).
 *	038	Dave Butenhof	30 April 1991
 *		Add trace statements.
 *	039	Dave Butenhof	08 May 1991
 *		Use new macro to test for kernel already locked, which
 *		evaporates under multiprocessor implementation.
 *	040	Paul Curtin	24 May 1991
 *		Added a cma__reinit_tcb routine.
 *	041	Paul Curtin	 5 May 1991
 *		Rearranged flags in reinit routine.
 *	042	Dave Butenhof	01 July 1991
 *		For one-to-one mapping, VP is cached with TCB; so delete it
 *		when TCB is destroyed.
 *	043	Dave Butenhof	12 September 1991
 *		Integrate Nat Mishkin's performance improvements to make more
 *		intelligent use of the file descriptor numbers; ULTRIX V4.2
 *		allows 4096 files, but most systems are generated to support
 *		lower numbers, and DECthreads is currently carrying around
 *		all that extra baggage. With this change, it'll only check
 *		and use the ones actually supported on the system. Also,
 *		shrink the TCB by dynamically allocating the file descriptor
 *		mask at thread creation time.
 *	044	Dave Butenhof	24 September 1991
 *		Handle allocation errors in cma___ini_tcb() by deallocating
 *		the TCB and reraising (but bugcheck if it's the default TCB,
 *		since we can't live without it).
 *	045	Dave Butenhof	04 October 1991
 *		Clean up use of _CMA_UNIPROCESSOR_
 *	046	Dave Butenhof	29 October 1991
 *		Handle allocation errors in cma__get_tcb.
 *	047	Dave Butenhof	22 November 1991
 *		Make sure exception stack head is cleared when initializing a
 *		new or recycled tcb!
 *	048	Dave Butenhof	13 December 1991
 *		Change name of exc_t_context to exc_context_t.
 *	049	Webb Scales	28 January 1992
 *		Reworked async context switch (added a tcb field init) on VMS.
 *	050	Dave Butenhof	11 February 1992
 *		A law of nature has just been changed: cma__alloc_mem now
 *		returns cma_c_null_ptr on an allocation failure, rather than
 *		raising an exception. This allows greater efficiency, since
 *		TRY is generally expensive. Anyway, apply the process of
 *		evolution: adapt or die.
 *	051	Webb Scales	13 March 1992
 *		Parameterize scheduling policies.
 *	052	Dave Butenhof	19 March 1992
 *		Ensure that per-thread sigaction data is initialized to zero
 *		(pointed out by Dale Tonogai, HP).
 *	053	Webb Scales	1 April 1992
 *		Unconditionalize async context initialization for U*ix.
 *	054	Webb Scales	13 May 1992
 *		"Cleansed" the known-threads queue links in the tcb prior to
 *		enqueuing a newly created thread.
 *	055	Webb Scales	15 May 1992
 *		"Cleansed" the other two queue links in the tcb during creation.
 *	056	Dave Butenhof	05 June 1992
 *		Fix delete_pending attr handling in free code -- it needs to
 *		free the attr. obj. instead of destroying it (which also
 *		fixes a race in attr. handling).
 *	057	Dave Butenhof	01 September 1992
 *		"event_status" was moved to the timer substructure, so
 *		initialize it there, too.
 *	058	Webb Scales	 3 September 1992
 *              Changed the name of the scheduler header file.
 *	059	Dave Butenhof	14 September 1992
 *		Change the beginning sequence number of threads.
 *	060	Dave Butenhof	17 September 1992
 *		Rearrange ini_tcb code so that debug thread handle is
 *		generated after sequence number is set.
 *	061	Dave Butenhof	24 September 1992
 *		Make more meaningful names for internal TCB objects.
 *	062	Brian Keane, Webb Scales    2 March 1993
 *		Add initialization of sched.processor for initializing
 *		TCBs from raw storage.
 *	063	Dave Butenhof	30 March 1993
 *		Replace "%08x" formatting with "0x%lx" for 64-bit safety.
 *	064	Dave Butenhof	12 April 1993
 *		Fix broken reinit comment.
 *	065	Dave Butenhof	13 April 1993
 *		Initialize TCB's header queue to zero (in !NDEBUG) to avoid
 *		misfiring queue consistency checks.
 *	066	Dave Butenhof	14 April 1993
 *		Update queue macro usage to allow VAX builtins.
 *	067	Dave Butenhof	 3 May 1993
 *		Update 'last living thread' pointer on known thread list
 *		insertion.
 *	068	Webb Scales	 7 May 1993
 *		Free the VP structure when the TCB is destroyed.
 *	069	Dave Butenhof	14 May 1993
 *		Simplify destroy logic -- no need to lock parent of synch.
 *		objects, since they don't cache.
 *	070	Dave Butenhof	14 May 1993
 *		Track first dead thread rather than last living thread
 *		(cma__g_last_thread is now cma__g_dead_zone) -- otherwise,
 *		destroy_tcb and free_tcb would have to enter kernel and check
 *		pointer to avoid moving last_thread onto cache or pool list!
 */


/*
 *  INCLUDE FILES
 */

#include <cma.h>
#include <cma_defs.h>
#include <cma_tcb.h>
#include <cma_vm.h>
#include <cma_vp_defs.h>
#include <cma_mutex.h>
#include <cma_handle.h>
#include <cma_stack.h>
#include <cma_queue.h>
#include <cma_condition.h>
#include <cma_init.h>
#include <cma_debug_client.h>
#include <cma_deb_core.h>
#include <cma_kernel.h>
#include <cma_dispatch.h>
#include <cma_sched_defs.h>

/*
 * GLOBAL DATA
 */

cma__t_int_tcb	    cma__g_def_tcb;		/* Default TCB */
cma__t_static_ctx   *cma__g_def_ctx_buf;	/* Default static context */


/*
 * LOCAL DATA
 */

/*
 * LOCAL FUNCTIONS
 */

static cma_t_boolean
cma___ini_tcb _CMA_PROTOTYPE_ ((
	cma__t_int_tcb	    *tcb,
	cma__t_thkind	    kind,
	cma__t_int_attr	    *attrib,
	cma_t_boolean	    is_raw_storage
	));


/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Deallocate a TCB structure---don't try to cache it (this is used to
 *	remove TCBs from a cache list!)
 *
 *  FORMAL PARAMETERS:
 *
 *	old_tcb		Address of the TCB
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
 */
extern void
cma__destroy_tcb
#ifdef _CMA_PROTO_
	(
	cma__t_int_tcb	*old_tcb)	/* The TCB to be freed */
#else	/* no prototypes */
	(old_tcb)
	cma__t_int_tcb	*old_tcb;	/* The TCB to be freed */
#endif	/* prototype */
    {
    /*
     * Free all the objects which are contained in the TCB, and then
     * deallocate the TCB's memory.  THIS ROUTINE ASSUMES THAT THE ATTRIBUTES
     * OBJECT IS LOCKED, AND THAT THE CALLER MANAGES THE CACHE LIST POINTERS!
     */
    cma__assert_warn (
	    cma__int_mutex_locked (old_tcb->attributes->mutex),
	    "cma__destroy_tcb called without attributes object locked.");

    cma__trace ((
	    cma__c_trc_obj,
	    "(destroy_tcb) destroying TCB 0x%lx (%d)",
	    old_tcb,
	    old_tcb->header.sequence));

    if (old_tcb->mutex != (cma__t_int_mutex *)cma_c_null_ptr)
	cma__free_mutex (old_tcb->mutex);

    if (old_tcb->term_cv != (cma__t_int_cv *)cma_c_null_ptr)
	cma__free_cv (old_tcb->term_cv);

    if (old_tcb->tswait_mutex != (cma__t_int_mutex *)cma_c_null_ptr)
	cma__free_mutex (old_tcb->tswait_mutex);

    if (old_tcb->tswait_cv != (cma__t_int_cv *)cma_c_null_ptr)
	cma__free_cv (old_tcb->tswait_cv);

    if (!cma__queue_empty (&old_tcb->stack))
	cma__free_stack_list (&old_tcb->stack);

    if (old_tcb->context_count != 0)
	cma__free_mem ((cma_t_address)old_tcb->contexts);

    if (old_tcb->sched.processor != (struct CMA__T_VP *)cma_c_null_ptr)
	cma__free_mem ((cma_t_address)old_tcb->sched.processor);

#if (_CMA_OS_ == _CMA__UNIX) && !_CMA_THREAD_SYNC_IO_
    /*
     * Free the File I/O masks (note that since rfds, wfds & efds were all
     * set to point to a single allocated chunk of memory, we only need to
     * free that chunk).
     */
    if ((cma_t_address)old_tcb->select.rfds != cma_c_null_ptr)
	cma__free_mem ((cma_t_address)old_tcb->select.rfds);

#endif

    cma__free_mem ((cma_t_address)old_tcb);
    }

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Release a TCB structure
 *
 *  FORMAL PARAMETERS:
 *
 *	old_tcb		Address of the TCB
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
 */
extern void
cma__free_tcb
#ifdef _CMA_PROTO_
	(
	cma__t_int_tcb	*old_tcb)	/* The TCB to be freed */
#else	/* no prototypes */
	(old_tcb)
	cma__t_int_tcb	*old_tcb;	/* The TCB to be freed */
#endif	/* prototype */
    {
    cma__t_int_attr	*attrib;


    /*
     * Add the TCB to the cached TCB list
     */
    attrib = old_tcb->attributes;

    cma__int_lock (old_tcb->mutex);
    old_tcb->header.sequence = 0;
    old_tcb->prolog.sentinel = cma_c_tcb_sentinel_nogood;

    /*
     * If there's a stack list, and the list has more than a single entry
     * (the thread's default stack), free the rest of the list... it's not
     * valid when the TCB is reused (and ties up stack chunks).
     */
    while (
	    cma__queue_previous (&old_tcb->stack)
	    != cma__queue_next (&old_tcb->stack))
	{
	cma__t_int_stack	*stack;

	stack = (cma__t_int_stack *)cma__queue_previous (&old_tcb->stack);
	cma__queue_remove (&stack->header.queue, stack, cma__t_int_stack);
	cma__free_stack (stack);
	}

    cma__int_unlock (old_tcb->mutex);

    cma__int_lock (attrib->mutex);
    attrib->refcnt--;

    if ((old_tcb->header.revision == attrib->cache[cma__c_obj_tcb].revision)
	&& (! attrib->delete_pending)) {

	if (attrib->cache[cma__c_obj_tcb].count
		< cma__g_env[cma__c_env_maxthread].value) {
	    attrib->cache[cma__c_obj_tcb].count += 1;
	    cma__queue_insert (
		    &old_tcb->header.queue,
		    &attrib->cache[cma__c_obj_tcb].queue);
	    }
	else {
	    cma__destroy_tcb (old_tcb);

	    while ((attrib->cache[cma__c_obj_tcb].count
		    > cma__g_env[cma__c_env_minthread].value)
		    && (! cma__queue_empty (
			    &attrib->cache[cma__c_obj_tcb].queue))) {
		cma__t_int_tcb	*item;

		cma__queue_dequeue (
			&attrib->cache[cma__c_obj_tcb].queue,
			item,
			cma__t_int_tcb);
		attrib->cache[cma__c_obj_tcb].count -= 1;
		cma__destroy_tcb (item);
		}

	    }

	}
    else
	cma__destroy_tcb (old_tcb);

    if ((attrib->refcnt == 0) && (attrib->delete_pending)) {
	cma__int_unlock (attrib->mutex);
	cma__free_attributes (attrib);
	}
    else
	cma__int_unlock (attrib->mutex);

    }

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Allocate a TCB structure
 *
 *  FORMAL PARAMETERS:
 *
 *	attrib		Address of Attributes object
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
 *	address of TCB
 *
 *  SIDE EFFECTS:
 *
 *	none
 */
extern cma__t_int_tcb *
cma__get_tcb
#ifdef _CMA_PROTO_
	(
	cma__t_int_attr	*attrib)	/* Attributes object to use */
#else	/* no prototypes */
	(attrib)
	cma__t_int_attr	*attrib;	/* Attributes object to use */
#endif	/* prototype */
    {
    cma_t_boolean	list_empty;	/* true if we need to create TCB */
    cma__t_int_tcb	*new_tcb;	/* Pointer to new TCB */


    cma__int_lock (attrib->mutex);

    if (attrib->delete_pending) {
	cma__int_unlock (attrib->mutex);
	return (cma__t_int_tcb *)cma_c_null_ptr;
	}

    if (cma__queue_empty (&attrib->cache[cma__c_obj_tcb].queue)) {
	new_tcb = cma__alloc_object (cma__t_int_tcb);

	if ((cma_t_address)new_tcb == cma_c_null_ptr) {
	    cma__int_unlock (attrib->mutex);
	    return (cma__t_int_tcb *)cma_c_null_ptr;
	    }

	cma__trace ((cma__c_trc_obj, "(get_tcb) allocated tcb 0x%lx", new_tcb));
	list_empty = cma_c_true;
	}
    else {
	cma__queue_dequeue (
		&attrib->cache[cma__c_obj_tcb].queue,
		new_tcb,
		cma__t_int_tcb);
	attrib->cache[cma__c_obj_tcb].count -= 1;
	cma__trace ((
		cma__c_trc_obj | cma__c_trc_cache,
		"(get_tcb) retrieved tcb 0x%lx from cache (%d more)",
		new_tcb,
		attrib->cache[cma__c_obj_tcb].count));
	list_empty = cma_c_false;
	}

    attrib->refcnt++;
    cma__int_unlock (attrib->mutex);

    if (!cma___ini_tcb (new_tcb, cma__c_thkind_normal, attrib, list_empty))
	return (cma__t_int_tcb *)cma_c_null_ptr;

    return new_tcb;
    }

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Initialize CMA_TCB.C global data
 *
 *  FORMAL PARAMETERS:
 *
 *	none
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
 */
void
cma__init_tcb
#ifdef _CMA_PROTO_
	(void)
#else	/* no prototypes */
	()
#endif	/* prototype */
    {
    static cma__t_string	tcb_name = "default thread";

    /*
     * Create a thread sequence object (default thread is number 1)
     */
    cma__g_known_threads.sequence = 1;
    cma__g_threadcnt = 0;

    /*
     * Initialize the initial thread's TCB
     */

    if (!cma___ini_tcb (
	    (cma__t_int_tcb *)&cma__g_def_tcb,
	    cma__c_thkind_initial,
	    (cma__t_int_attr *)&cma__g_def_attr,
	    cma_c_true))
	cma__bugcheck ("init: ini def. tcb failed");

    cma__obj_set_name (&cma__g_def_tcb, tcb_name);
    cma__g_def_ctx_buf = &cma__g_def_tcb.static_ctx;
    }

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Initialize the fields of the TCB
 *
 *  FORMAL PARAMETERS:
 *
 *	tcb		- The tcb to initialize
 *	kind		- Kind of thread being created 
 *	attrib		- The attributes object to use
 *	is_raw_storage	- True if this is the first use of the bytes as a 
 *			  tcb, false if the tcb is coming from cache.
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
 */
static cma_t_boolean
cma___ini_tcb
#ifdef _CMA_PROTO_
	(cma__t_int_tcb	    *tcb,
	cma__t_thkind	    kind,
	cma__t_int_attr	    *attrib,
	cma_t_boolean	    is_raw_storage)
#else	/* no prototypes */
	(tcb,
	kind,
	attrib,
	is_raw_storage)

	cma__t_int_tcb	    *tcb;
	cma__t_thkind	    kind;
	cma__t_int_attr	    *attrib;
	cma_t_boolean	    is_raw_storage;
#endif	/* prototype */
    {
    cma_t_integer   i;

    if (is_raw_storage) {
	cma__t_int_stack	*stack;
	cma__t_context_list	init_list;	/* Pointer to initial list */
	cma_t_natural		init_size;	/* Size of new list */
	cma_t_boolean		ok = cma_c_true;


	/*
	 * This is the initialization required if the TCB is raw
	 * new storage (did not come from a cache).  We must create
	 * the internal objects hanging off the TCB and do any other
	 * one-time inits.
	 */

	tcb->header.type = cma__c_obj_tcb;
	tcb->attributes = attrib;
	cma__queue_init (&tcb->stack);

	/*
	 * If we can't allocate any of the subobjects in the TCB, destroy the
	 * TCB and reraise the allocation exception. Except if we're trying
	 * to set up the default thread TCB (far too early for any reasonable
	 * memory allocation error, and anyway, we kinda need it); then we
	 * bugcheck.
	 */
	tcb->mutex = cma__get_mutex (&cma__g_def_attr);

	if ((cma_t_address)tcb->mutex == cma_c_null_ptr)
	    ok = cma_c_false;
	else {
	    cma__trace ((
		    cma__c_trc_obj,
		    "(ini_tcb) tcb 0x%lx's mutex is 0x%lx",
		    tcb,
		    tcb->mutex));
	    cma__obj_set_name (tcb->mutex, "thread %d lock");
	    }

	if (ok) {
	    tcb->term_cv = cma__get_cv (&cma__g_def_attr);

	    if ((cma_t_address)tcb->term_cv == cma_c_null_ptr)
		ok = cma_c_false;
	    else {
		cma__trace ((
			cma__c_trc_obj,
			"(ini_tcb) tcb 0x%lx's cv is 0x%lx",
			tcb,
			tcb->term_cv));
		cma__obj_set_name (tcb->term_cv, "thread %d join");
		}

	    }

	if (ok) {
	    tcb->tswait_mutex = cma__get_mutex (&cma__g_def_attr);

	    if ((cma_t_address)tcb->tswait_mutex == cma_c_null_ptr)
		ok = cma_c_false;
	    else {
		cma__trace ((
		    cma__c_trc_obj,
		    "(ini_tcb) tcb 0x%lx's wait mutex is 0x%lx",
		    tcb,
		    tcb->tswait_mutex));
		cma__obj_set_name (tcb->tswait_mutex, "thread %d wait");
		}

	    }

	if (ok) {
	    tcb->tswait_cv = cma__get_cv (&cma__g_def_attr);

	    if ((cma_t_address)tcb->tswait_cv == cma_c_null_ptr)
		ok = cma_c_false;
	    else {
		cma__trace ((
			cma__c_trc_obj,
			"(ini_tcb) tcb 0x%lx's wait cv is 0x%lx",
			tcb,
			tcb->tswait_cv));
		cma__obj_set_name (tcb->tswait_cv, "thread %d wait");
		}

	    }

	if (ok) {
	    init_size = sizeof (cma_t_address) * 11;	/* 10 plus cma's */
	    init_list = (cma__t_context_list)cma__alloc_zero (init_size);

	    if ((cma_t_address)init_list == cma_c_null_ptr)
		ok = cma_c_false;
	    else {
		tcb->context_count = 10;	/* preallocated */
		tcb->contexts = init_list;
		}

	    }

#if (_CMA_OS_ == _CMA__UNIX) && !_CMA_THREAD_SYNC_IO_
	    /*
	     * Allocate the dynamic read, write, and error file masks if this
	     * is a UNIX-derived system. Note that we save some time by
	     * allocating a single block large enough for all 3 masks and
	     * then subdividing.
	     */

	    if (ok) {
		tcb->select.rfds = (cma__t_file_mask *)cma__alloc_mem (
			sizeof (cma__t_mask) * cma__g_nspm * 3);

		if ((cma_t_address)tcb->select.rfds == cma_c_null_ptr)
		    ok = cma_c_false;
		else {
		    tcb->select.wfds = (cma__t_file_mask *)
			((int)tcb->select.rfds
			+ (sizeof (cma__t_mask) * cma__g_nspm));
		    tcb->select.efds = (cma__t_file_mask *)
			((int)tcb->select.wfds
			+ (sizeof (cma__t_mask) * cma__g_nspm));
		    cma__queue_zero (&tcb->select.queue);
		    }

		}
#endif

	    if (ok && kind != cma__c_thkind_initial) {
		stack = cma__get_stack (attrib);

		if ((cma_t_address)stack == cma_c_null_ptr)
		    ok = cma_c_false;
		else
		    cma__assign_stack (stack, tcb);

		}

	/*
	 * Initialize the TCB's VP.
	 */
	tcb->sched.processor = (cma__t_vp *)cma_c_null_ptr;

	if (!ok) {			/* If an allocation failed, back off */

	    if (kind != cma__c_thkind_initial) {
		cma__t_int_mutex	*am;


		am = tcb->attributes->mutex;
		cma__int_lock (am);
		tcb->attributes->refcnt--;
		cma__destroy_tcb (tcb);
		cma__int_unlock (am);
		}

	    return cma_c_false;
	    }

	}

    /*
     * Whether the TCB is old or new, we need to ensure certain field
     * initialization. The most important step is generating a unique
     * sequence number for the new TCB object.
     */

    cma__obj_clear_name (tcb);
    tcb->header.revision = attrib->cache[cma__c_obj_tcb].revision;
    tcb->kind = kind;
    tcb->start_code = (cma_t_start_routine)cma_c_null_ptr;
    tcb->start_arg = cma_c_null_ptr;
    tcb->wait_cv = (cma__t_int_cv *)cma_c_null_ptr;
    tcb->wait_mutex = (cma__t_int_mutex *)cma_c_null_ptr;
    tcb->return_value = cma_c_null_ptr;
    tcb->exit_status = cma_c_term_normal;
    tcb->async_ctx.valid = cma_c_false;
    tcb->async_ctx.interrupt_ctx = cma_c_null_ptr;
    cma__queue_zero (&tcb->timer.queue);
    tcb->timer.event_status = 0;
    tcb->detached = cma_c_false;
    tcb->terminated = cma_c_false;
    tcb->joiners = 0;
    cma__tac_set (&tcb->alert_pending);
    tcb->alert.g_enable = cma_c_true;
    tcb->alert.a_enable = cma_c_false;
    tcb->alert.count = 0;
    tcb->debug.did_preempt = cma_c_false;
    tcb->debug.notify_debugger = cma_c_false;
    tcb->debug.on_hold = cma_c_false;
    tcb->exc_stack = (exc_context_t *)cma_c_null_ptr;

#if _CMA_OS_ == _CMA__UNIX
    for (i = 0; i < NSIG; i++)
	tcb->sigaction_data[i].sa_handler = 0;
#endif

    for (i = cma__c_debevt__first; i <= cma__c_debevt__last; i++)
	tcb->debug.flags[i] = cma_c_false;

    tcb->thd_errno = 0;
#if _CMA_OS_ == _CMA__VMS
    tcb->thd_vmserrno = 0;
#endif

    /*
     * Do inits that depend on the kind of thread being created
     */
    if (kind == cma__c_thkind_initial) {

	/* Inits for the main thread ONLY */

	tcb->sched.priority	= attrib->priority;
	tcb->sched.policy	= attrib->policy;
	tcb->state		= cma__c_state_running;
	tcb->debug.activated	= cma_c_true;
	tcb->debug.substate	= cma__c_substa_normal;

        /* 
	 * Don't modify exc_stack, 
	 *	so TRY blocks can be used before INIT (with PTHREADS)
	 */

	/*
	 * Simulate thread creation functions for the initial thread
	 */
	tcb->debug.start_pc	= (cma_t_address)0;
	tcb->debug.object_addr  = (cma_t_address)0;
	}
    else {
	cma__t_int_tcb	*cur_tcb;   

	/* Inits for null and normal threads ONLY */

	/*
	 * Allow priority inheritance if other than initial thread
	 */
	cur_tcb = cma__get_self_tcb ();

	if (attrib->inherit_sched) {
	    tcb->sched.priority	= cur_tcb->sched.priority;
	    tcb->sched.policy	= cur_tcb->sched.policy;
	    }
	else {
	    tcb->sched.priority	= attrib->priority;
	    tcb->sched.policy	= attrib->policy;
	    };

	tcb->state		= cma__c_state_ready;
	tcb->debug.activated	= cma_c_false;
	tcb->debug.substate	= cma__c_substa_not_yet_run;
	}

    /*
     * Set scheduling parameters from the policy.
     */
    cma__sched_parameterize (tcb, tcb->sched.policy);

    cma__trace ((
	    cma__c_trc_obj,
	    "(ini_tcb) placing 0x%lx on known threads queue (k = %d)",
	    tcb, cma__tac_isset (&cma__g_kernel_critical)));

    cma__queue_zero (&tcb->threads);
    cma__queue_zero (&tcb->header.queue);
    cma__assert_not_kernel ();
    cma__enter_kernel ();
    cma__g_threadcnt++;			/* Another thread */
    tcb->header.sequence = cma__g_known_threads.sequence++;
    /*
     * Insert the new thread before the first zombie.
     */
    cma__queue_insert (&tcb->threads, cma__g_dead_zone);
    cma__exit_kernel ();

    /*
     * Set TCB's sequence number into sub-object headers for debug
     * formatting.
     */
    cma__obj_set_owner (tcb->mutex, tcb->header.sequence);
    cma__obj_set_owner (tcb->term_cv, tcb->header.sequence);
    cma__obj_set_owner (tcb->tswait_mutex, tcb->header.sequence);
    cma__obj_set_owner (tcb->tswait_cv, tcb->header.sequence);

    /*
     * Make the thread known for debugging purposes.
     */
    tcb->prolog.reserved1   = cma_c_null_ptr;
    tcb->prolog.client_key  = (cma_t_key)0;
    cma__object_to_handle (
	(cma__t_object *)tcb, 
	(cma_t_handle *)&(tcb->prolog.client_thread));
    tcb->prolog.sentinel    = cma_c_tcb_sentinel;

    return cma_c_true;
    }

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Perform work prior to or after a fork(), depending upon flag.
 *
 *  FORMAL PARAMETERS:
 *
 *	flag
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
 */
void
cma__reinit_tcb
#ifdef _CMA_PROTO_
	(
	cma_t_integer	flag)
#else	/* no prototypes */
	(flag)
	cma_t_integer	flag;
#endif	/* prototype */
    {
    /*
     * Lock the current TCB around a fork, to prevent modifications; since
     * only the current thread will exist in the child process, the mutex
     * could otherwise be left locked by the other thread.
     */
    if (flag == cma__c_reinit_prefork_lock) {	/* pre fork work */
	cma__int_lock (cma__get_self_tcb()->mutex);
	}
    else if (flag == cma__c_reinit_postfork_unlock) {	/* post fork work */
	cma__int_unlock (cma__get_self_tcb()->mutex);
	}	

    }
/*  VAX/DEC CMS REPLACEMENT HISTORY, Element CMA_TCB.C */
/*  *41   14-MAY-1993 15:56:29 BUTENHOF "Simplify destroy, since synch objects don't cache!" */
/*  *40    7-MAY-1993 19:17:26 SCALES """Free VP structure during TCB destruction""" */
/*  *39    3-MAY-1993 13:44:48 BUTENHOF "Merge zombies with known threads" */
/*  *38   16-APR-1993 13:05:56 BUTENHOF "Fix broken comment" */
/*  *37    1-APR-1993 14:33:26 BUTENHOF "Fix trace formatting" */
/*  *36    2-MAR-1993 17:52:23 KEANE "Initialize sched.processor when allocating TCB from raw storage" */
/*  *35   28-SEP-1992 11:49:21 BUTENHOF "Set useful names on TCB CVs" */
/*  *34   17-SEP-1992 06:36:51 BUTENHOF "Correct thread init" */
/*  *33   15-SEP-1992 13:50:35 BUTENHOF "Support Mach scheduling" */
/*  *32    3-SEP-1992 21:28:34 SCALES "Add/change scheduler include file" */
/*  *31    2-SEP-1992 16:26:28 BUTENHOF "Change event_status reference" */
/*  *30   13-AUG-1992 14:44:15 BUTENHOF "Set up unique PAL value for main thread" */
/*  *29    5-JUN-1992 13:34:59 BUTENHOF "Fix delete_pending attr handling" */
/*  *28   18-MAY-1992 16:50:07 SCALES "Add additional queue element initializations" */
/*  *27   15-MAY-1992 15:04:49 SCALES "Cleanse tcb links for known-threads queue" */
/*  *26    3-APR-1992 18:34:06 SCALES "Rework async context switch for U*ix" */
/*  *25   19-MAR-1992 13:18:02 BUTENHOF "Fix sigaction() bugs" */
/*  *24   18-MAR-1992 19:01:38 SCALES "Parameterize scheduling policies" */
/*  *23   13-MAR-1992 14:10:05 BUTENHOF "Add tracing" */
/*  *22   18-FEB-1992 15:30:29 BUTENHOF "Adapt to new alloc_mem protocol" */
/*  *21   29-JAN-1992 23:48:17 SCALES "Rework async context switching for VMS" */
/*  *20   13-DEC-1991 10:12:37 BUTENHOF "Fix exc symbols" */
/*  *19   22-NOV-1991 11:57:15 BUTENHOF "Initialize exception stack on thread create" */
/*  *18   31-OCT-1991 12:39:38 BUTENHOF "Handle allocation error in get_tcb" */
/*  *17   14-OCT-1991 13:40:52 BUTENHOF "Refine/fix use of config symbols" */
/*  *16   24-SEP-1991 16:28:42 BUTENHOF "Dynamically allocate file masks" */
/*  *15   17-SEP-1991 13:24:07 BUTENHOF "Dynamically allocate file masks" */
/*  *14    2-JUL-1991 16:52:47 BUTENHOF "Add & use start func/arg fields in TCB" */
/*  *13   10-JUN-1991 18:24:08 SCALES "Add sccs headers for Ultrix" */
/*  *12    5-JUN-1991 16:15:09 CURTIN "fork work" */
/*  *11   24-MAY-1991 16:48:21 CURTIN "Added a new reinit routine" */
/*  *10   10-MAY-1991 17:52:11 BUTENHOF "Use new macro to test for kernel locked" */
/*  *9     2-MAY-1991 13:59:18 BUTENHOF "Add traces" */
/*  *8    30-APR-1991 16:49:46 CURTIN "Added some pre-allocated context keys" */
/*  *7    12-APR-1991 23:37:02 BUTENHOF "Change references to kernel_critical" */
/*  *6     8-APR-1991 20:31:08 CURTIN "Added some global defs for quick kill" */
/*  *5     1-APR-1991 18:09:27 BUTENHOF "Wean from cma_exception.h to exc_handling.h" */
/*  *4    12-FEB-1991 01:29:30 BUTENHOF "Redefine alert state" */
/*  *3     6-FEB-1991 01:33:18 BUTENHOF "Change errno field name" */
/*  *2    19-DEC-1990 17:02:26 CURTIN "added an enter kernel around known threads list op" */
/*  *1    12-DEC-1990 21:53:49 BUTENHOF "Thread bookkeeping" */
/*  VAX/DEC CMS REPLACEMENT HISTORY, Element CMA_TCB.C */
