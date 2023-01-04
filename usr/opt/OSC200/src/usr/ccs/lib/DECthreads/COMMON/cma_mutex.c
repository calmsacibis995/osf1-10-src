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
static char *rcsid = "@(#)$RCSfile: cma_mutex.c,v $ $Revision: 4.2.9.2 $ (DEC) $Date: 1993/08/18 14:48:08 $";
#endif
/*
 *  FACILITY:
 *
 *	DECthreads services
 *
 *  ABSTRACT:
 *
 *	Operations on mutex object
 *
 *  AUTHORS:
 *
 *	Dave Butenhof
 *
 *  CREATION DATE:
 *
 *	21 August 1989
 *
 *  MODIFICATION HISTORY:
 *
 *	001	Dave Butenhof	18 September 1989
 *		Implement simple mutex on top of dispatcher's semaphores.
 *	002	Dave Butenhof	04 October 1989
 *		Implement internal mutex lock/unlock using object pointers
 *		instead of handles for convenient internal use.
 *	003	Dave Butenhof	9 October 1989
 *		Use cma__error to raise exceptions where necessary.
 *	004	Dave Butenhof	11 October 1989
 *		Convert to use internal mutex operations.
 *	005	Dave Butenhof	18 October 1989
 *		cma__queue_insert is now a macro, which expands to a block;
 *		this module includes a call which is the sole statement on
 *		the "then" of an if statement, and the trailing ";" (after a
 *		"}") breaks the "else" clause.  Fix the reference in such a
 *		way that some future conversion back to a routine call won't
 *		break it again.
 *	006	Dave Butenhof	19 October 1989
 *		Use new type-specific handle validation macros.
 *	007	Dave Butenhof	19 October 1989
 *		Substitute "cma_t_address" for explicit "void *" to make
 *		porting easier.
 *	008	Dave Butenhof	19 October 1989
 *		Modify use of queue operations to use appropriate casts
 *		rather than depending on ANSI C "void *" (sigh).
 *	009	Webb Scales 	20 October 1989
 *		Put in typecasts as required by MIPS/Ultrix C
 *	010	Dave Butenhof	24 October 1989
 *		Enhance free & destroy to handle attr. obj. delete-pending
 *		and refcnt.
 *	011	Dave Butenhof	25 October 1989
 *		Fix a deadlock in previous changes to free code.
 *	012	Dave Butenhof	1 November 1989
 *		- Put active mutexes onto a queue, and record waiting mutex
 *		in
 *		  TCB for debugging.
 *		- Make mutex sequence variable external so init_static can
 *		  set it up for get_first_mutex use (non-interlocked).
 *	013	Dave Butenhof	22 November 1989
 *		Add support for new "owner" field of mutex so that debug
 *		"mutex" command can report current owner of lock.
 *	014	Dave Butenhof	30 November 1989
 *		Modify external entries to track POSIX changes to names and
 *		argument ordering.
 *	015	Dave Butenhof	26 January 1990
 *		Change cma__get_self_tcb_kernel to cma__get_self_tcb (it no
 *		longer locks the kernel, so distinction is irrelevant).
 *	016	Dave Butenhof	9 April 1990
 *		Use new "known_object" structure for known mutex queue
 *		(includes mutex).
 *	017	Dave Butenhof	10 April 1990
 *		Catch memory errors over object allocation, and set names in
 *		internal objects.
 *	018	Dave Butenhof	11 April 1990
 *		Fix mutex ownership (assign after decrement_semaphore).
 *	019	Dave Butenhof	5 June 1990
 *		Support mutex cache high water mark.
 *	020	Dave Butenhof	7 June 1990
 *		Erase destroyed objects if not NDEBUG, to catch references to
 *		dead objects (can be used in conjunction with cma_maxmutex=0,
 *		which disables caching).
 *	021	Dave Butenhof	15 June 1990
 *		Streamline mutex performance by using interlocked bit;
 *		semaphores will be used only to block the thread if the mutex
 *		is already locked.
 *	022	Dave Butenhof	18 June 1990
 *		Use macros to clear object name (only defined for debug
 *		build).
 *	023	Dave Butenhof	26 June 1990
 *		Implement "friendly" mutexes which support nested locking by
 *		the same thread.  Use these to implement a new "global lock"
 *		primitive for use in synchronizing non-reentrant libraries.
 *	024	Dave Butenhof	06 July 1990
 *		Fix missing ';' in NDEBUG conditional code.
 *	025	Dave Butenhof	31 July 1990
 *		Move internal mutex operations to .h so they can be inlined
 *		everywhere within CMA.
 *	026	Paul Curtin	22 August 1990
 *		Removed reference to cma__alloc_mem_nolock.
 *	027	Dave Butenhof	27 August 1990
 *		Change interfaces to pass handles & structures by reference.
 *	028	Webb Scales	30 October 1990
 *		Removed uses of external handle fields.
 *	029	Webb Scales	30 October 1990
 *		Added casts to change #28.
 *	030	Bob Conti	5 November 1990
 *		Restore mutex debugging by setting the wait_mutex field even
 *		in production builds.
 *	031	Dave Butenhof	12 February 1991
 *		Change "friendly" to "recursive"
 *	032	Dave Butenhof	09 April 1991
 *		Use new constants to init atomic bits
 *	033	Paul Curtin	10 May 1991
 *		Use new internal macros added in cma_mutex.h
 *	034	Dave Butenhof	20 May 1991
 *		Fix a possible race condition in cma__int_mutex_block, by
 *		clearing mutex "event" flag after entering kernel.
 *	035	Paul Curtin	30 May 1991
 *		Added a reinit routine for pre/post fork() work.
 *	036	Dave Butenhof	28 June 1991
 *		Initialize the owner field of a mutex to be sure.
 *	037	Dave Butenhof	05 November 1991
 *		Fix a bug in cma__get_mutex pointed out by Peter Burgess of
 *		MCC; memory allocation may not always be zeroed, and
 *		recursive mutex setup assumes that a raw mutex's internal
 *		lock pointer will be zero.
 *	038	Dave Butenhof	11 February 1992
 *		A law of nature has just been changed: cma__alloc_mem now
 *		returns cma_c_null_ptr on an allocation failure, rather than
 *		raising an exception. This allows greater efficiency, since
 *		TRY is generally expensive. Anyway, apply the process of
 *		evolution: adapt or die.
 *	039	Dave Butenhof	19 February 1992
 *		Fix typo in alloc_object call (typing).
 *	040	Dave Butenhof	10 March 1992
 *		Changes to cma__int_[un]block_mutex to return status instead
 *		of raising exceptions. Handle new interface in lock/unlock.
 *	041	Dave Butenhof	12 March 1992
 *		Clean up 040 a little.
 *	042	Dave Butenhof	05 June 1992
 *		Fix delete_pending attr handling in free code -- it needs to
 *		free the attr. obj. instead of destroying it (which also
 *		fixes a race in attr. handling).
 *	043	Dave Butenhof	24 August 1992
 *		Modify Mach mutex locks to spin and yield instead of
 *		blocking; improves performance in most cases. Remove use of
 *		the "waiters" bit (since there aren't any), and no-op the
 *		wake operation in unblock. The mutex_unblock function can't
 *		evaporate, since recursive and nonrecursive mutexes still
 *		need it (but it should never be called for fast mutexes,
 *		since the waiters bit should never be set).
 *	044	Webb Scales	25 August 1992
 *		Added a default case to the int-mutex-locked switch
 *		statement.
 *	045	Dave Butenhof	26 August 1992
 *		Get rid of semaphores and mutex caching.
 *	046	Dave Butenhof	15 September 1992
 *		Get rid of "sequence object" (it's in known object
 *		structure).
 *	047	Dave Butenhof	22 September 1992
 *		Fix a bug in recursive unlock.
 *	048	Dave Butenhof	16 October 1992
 *		Fix the blocking code for VPs (sense of finish_block arg was
 *		wrong).
 *	049	Dave Butenhof	 3 December 1992
 *		Use cma__vp_yield() macro instead of swtch_pri().
 *	050	Dave Butenhof	31 March 1993
 *		User objects get "useful" name -- interface & handle
 *		address.
 *	051	Dave Butenhof	14 April 1993
 *		Update queue macro usage to allow VAX builtins.
 */

/*
 *  INCLUDE FILES
 */

#include <cma.h>
#include <cma_defs.h>
#include <cma_mutex.h>
#include <cma_handle.h>
#include <cma_vm.h>
#include <cma_errors.h>
#include <cma_stack.h>
#include <cma_queue.h>
#include <cma_dispatch.h>
#include <cma_deb_core.h>
#include <cma_init.h>
#include <cma_vp.h>

/*
 * GLOBAL DATA
 */

cma__t_int_mutex *cma__g_global_lock = (cma__t_int_mutex *)cma_c_null_ptr;

/*
 * LOCAL MACROS
 */

/*
 * LOCAL DATA
 */

/*
 * LOCAL FUNCTIONS
 */


/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Lock the global library mutex.
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
extern void
cma_lock_global
#ifdef _CMA_PROTO_
	(void)
#else	/* no prototypes */
	()
#endif	/* prototype */
    {
    cma__int_lock (cma__g_global_lock);
    }

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Create a new mutex object.
 *
 *  FORMAL PARAMETERS:
 *
 *	new_mutex	Output handle
 *
 *	att		Attributes object to use in creation
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
cma_mutex_create
#ifdef _CMA_PROTO_
	(
	cma_t_mutex	*new_mutex,	/* New handle to fill in */
	cma_t_attr	*att)		/* Old attr obj to use */
#else	/* no prototypes */
	(new_mutex, att)
	cma_t_mutex	*new_mutex;	/* New handle to fill in */
	cma_t_attr	*att;		/* Old attr obj to use */
#endif	/* prototype */
    {
    cma__t_int_mutex    *mutex;
    cma__t_int_attr     *int_att;


    int_att = cma__validate_default_attr (att);
    mutex = cma__get_mutex (int_att);

    if ((cma_t_address)mutex == cma_c_null_ptr)
	cma__error (exc_s_insfmem);
    else {
	cma__object_to_handle ((cma__t_object *)mutex, new_mutex);
	cma__obj_set_owner (mutex, (cma_t_integer)new_mutex);
	cma__obj_set_name (mutex, "<CMA user@0x%lx>");
	}

    }

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Delete (free) a mutex object
 *
 *  FORMAL PARAMETERS:
 *
 *	mutex		Mutex object to free
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
cma_mutex_delete
#ifdef _CMA_PROTO_
	(
	cma_t_mutex	*mutex)		/* Mutex to free */
#else	/* no prototypes */
	(mutex)
	cma_t_mutex	*mutex;		/* Mutex to free */
#endif	/* prototype */
    {
    cma__int_mutex_delete (mutex);
    }

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Lock a mutex
 *
 *  FORMAL PARAMETERS:
 *
 *	mutex		Mutex object to lock
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
cma_mutex_lock
#ifdef _CMA_PROTO_
	(
	cma_t_mutex	*mutex)		/* Mutex to lock */
#else	/* no prototypes */
	(mutex)
	cma_t_mutex	*mutex;		/* Mutex to lock */
#endif	/* prototype */
    {
    cma__t_int_tcb	*tcb;
    cma__t_int_mutex	*int_mutex;

    /*
     * Get a pointer to the mutex structure; if this is a debugging CMA,
     * we'll validate the mutex handle to be sure it's valid.  For
     * performance, if it's an NDEBUG ("production") CMA, just fetch the
     * address of the object from the handle's pointer field.
     */
#ifdef NDEBUG
    int_mutex = (cma__t_int_mutex *)((cma__t_int_handle *)mutex)->pointer;
#else
    int_mutex = cma__validate_mutex (mutex);
    tcb = cma__get_self_tcb ();
#endif

    /*
     * First, try to acquire the lock; if we get it, then we're done
     */
    if (cma__test_and_set (&int_mutex->lock)) {
	cma_t_status	res;


	res = cma__int_mutex_block (int_mutex);

	if (res != cma_s_normal)
	    cma__error (res);

	}

#ifndef NDEBUG
    int_mutex->owner = tcb;
#endif
    }

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Try to lock a mutex, but return immediately with boolean status if
 *	the mutex is already locked
 *
 *  FORMAL PARAMETERS:
 *
 *	mutex		Mutex object to lock
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
extern cma_t_boolean
cma_mutex_try_lock
#ifdef _CMA_PROTO_
	(
	cma_t_mutex	*mutex)		/* Mutex to lock */
#else	/* no prototypes */
	(mutex)
	cma_t_mutex	*mutex;		/* Mutex to lock */
#endif	/* prototype */
    {
    cma__t_int_mutex	*int_mutex;
    cma__t_int_tcb	*tcb;


#ifdef NDEBUG
    int_mutex = (cma__t_int_mutex *)((cma__t_int_handle *)mutex)->pointer;
#else
    int_mutex = cma__validate_mutex (mutex);
    tcb = cma__get_self_tcb ();
#endif

    if (cma__test_and_set (&int_mutex->lock)) {

	switch (int_mutex->mutex_kind) {
	    case cma_c_mutex_nonrecursive : {
#ifdef NDEBUG
		tcb = cma__get_self_tcb ();
#endif
		cma__mulock (int_mutex);

		if (int_mutex->owner == (cma__t_int_tcb *)cma_c_null_ptr) {
		    int_mutex->owner = tcb;
		    cma__muunlock (int_mutex);
		    return cma_c_true;
		    }
		else {
		    cma__muunlock (int_mutex);
		    return cma_c_false;
		    }

		break;
		}

	    case cma_c_mutex_recursive : {
#ifdef NDEBUG
		tcb = cma__get_self_tcb ();
#endif
		cma__mulock (int_mutex);

		if (tcb == int_mutex->owner
			|| int_mutex->owner == (cma__t_int_tcb *)cma_c_null_ptr) {
		    int_mutex->nest_count++;
		    int_mutex->owner = tcb;
		    cma__muunlock (int_mutex);
		    return cma_c_true;
		    }
		else {
		    cma__muunlock (int_mutex);
		    return cma_c_false;
		    }

		break;
		}

	    case cma_c_mutex_fast : {
		return cma_c_false;
		break;
		}

	    }

	}

#ifndef NDEBUG
    int_mutex->owner = tcb;
#endif
    return cma_c_true;
    }

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Unlock a mutex
 *
 *  FORMAL PARAMETERS:
 *
 *	mutex		Mutex object to unlock
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
cma_mutex_unlock
#ifdef _CMA_PROTO_
	(
	cma_t_mutex	*mutex)		/* Mutex to unlock */
#else	/* no prototypes */
	(mutex)
	cma_t_mutex	*mutex;		/* Mutex to unlock */
#endif	/* prototype */
    {
    cma__t_int_mutex	*int_mutex;

#ifdef NDEBUG
    int_mutex = (cma__t_int_mutex *)((cma__t_int_handle *)mutex)->pointer;
#else
    cma__t_int_tcb	*tcb;

    int_mutex = cma__validate_mutex (mutex);
    tcb = cma__get_self_tcb ();

    if (int_mutex->mutex_kind == cma_c_mutex_fast) {
	cma__assert_warn (
		(tcb == int_mutex->owner),
		"attempt to release mutex owned by another thread");
	int_mutex->owner = (cma__t_int_tcb *)cma_c_null_ptr;
	}

#endif
    cma__unset (int_mutex->unlock);

    /*
     * Check whether there might be waiters, and reset the bit (TRUE means
     * "no waiters").  If there might be waiters, release one.
     */
    if (!cma__test_and_set (&int_mutex->event)) {
	cma_t_status	res;


	res = cma__int_mutex_unblock (int_mutex);

	if (res != cma_s_normal)
	    cma__error (res);

	}

    }

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Unlock the global library mutex.
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
extern void
cma_unlock_global
#ifdef _CMA_PROTO_
	(void)
#else	/* no prototypes */
	()
#endif	/* prototype */
    {
    cma__int_unlock (cma__g_global_lock);
    }

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Free a mutex object to the memory pool
 *
 *  FORMAL PARAMETERS:
 *
 *	old_mutex	Mutex object to free
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
cma__free_mutex
#ifdef _CMA_PROTO_
	(
	cma__t_int_mutex	*old_mutex) /* Mutex object to free */
#else	/* no prototypes */
	(old_mutex)
	cma__t_int_mutex	*old_mutex; /* Mutex object to free */
#endif	/* prototype */
    {
    /*
     * Assert that the mutex being freed is unlocked, just to be safe...
     */
    cma__assert_warn (
	    !cma__int_mutex_locked (old_mutex),
	    "cma__free_mutex called for locked mutex.");

    cma__trace ((
	    cma__c_trc_obj,
	    "(free_mutex) freeing mutex %d",
	    old_mutex->header.sequence));

    old_mutex->header.sequence = 0;
    cma__int_lock (cma__g_known_mutexes.mutex);
    cma__queue_remove (&old_mutex->header.queue, old_mutex, cma__t_int_mutex);
    cma__int_unlock (cma__g_known_mutexes.mutex);
    cma__free_mem ((cma_t_address)old_mutex);
    }

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Allocate the first mutex objects.  This mode doesn't lock the parent
 *	attribute object's mutex, nor look for cached mutex objects.  It
 *	should ONLY be called during startup, to allow creating mutexes for
 *	the first attributes object & the attributes and mutex sequence
 *      objects (it breaks an otherwise nasty circular dependency).
 *
 *  FORMAL PARAMETERS:
 *
 *	attrib		Attributes object to use
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
 *	Address of new mutex object
 *
 *  SIDE EFFECTS:
 *
 *	none
 */
extern cma__t_int_mutex *
cma__get_first_mutex
#ifdef _CMA_PROTO_
	(
	cma__t_int_attr	*attrib)	/* Attributes object to use */
#else	/* no prototypes */
	(attrib)
	cma__t_int_attr	*attrib;	/* Attributes object to use */
#endif	/* prototype */
    {
    cma__t_int_mutex	*new_mutex;	/* Pointer to new mutex */


    cma__trace ((
	    cma__c_trc_obj,
	    "(get_first_mutex) creating mutex, attr %d",
	    attrib->header.sequence));

    new_mutex = cma__alloc_object (cma__t_int_mutex);

    if ((cma_t_address)new_mutex == cma_c_null_ptr)
	cma__bugcheck ("init: can't allocate mutex");

    new_mutex->header.type = cma__c_obj_mutex;
    new_mutex->attributes = attrib;
    new_mutex->mutex_kind = attrib->mutex_kind;
    cma__obj_clear_name (new_mutex);
    new_mutex->owner = (cma__t_int_tcb *)cma_c_null_ptr;
    cma__queue_init (&new_mutex->queue);
    cma__tac_clear (&new_mutex->spindle);

    if (new_mutex->mutex_kind == cma_c_mutex_fast) {
	cma__tac_clear (&new_mutex->lock);
	new_mutex->unlock = &new_mutex->lock;
	cma__tac_set (&new_mutex->event);
	}
    else {
	cma__tac_set (&new_mutex->lock);
	new_mutex->unlock = &new_mutex->bitbucket;
	new_mutex->nest_count = 0;
	cma__tac_clear (&new_mutex->event);
	cma__tac_set (&new_mutex->waiters);
	}

    /*
     * Normally, the known_mutex structure should be LOCKED before inserting
     * a new entry.  However, this particular routine is only called during
     * CMA "bootstrap" when there is only a single thread, and mutexes may
     * not be fully initialized (in fact, this call may be to create the lock
     * which controls access to the known_mutex queue!); so we'll do the
     * insertion without locking.
     */
    new_mutex->header.sequence = cma__g_known_mutexes.sequence++;
    cma__queue_insert (&new_mutex->header.queue, &cma__g_known_mutexes.queue);

    return new_mutex;
    }

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Allocate a mutex object.
 *
 *  FORMAL PARAMETERS:
 *
 *	attrib		Attributes object to use
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
 *	Address of new mutex object
 *
 *  SIDE EFFECTS:
 *
 *	none
 */
extern cma__t_int_mutex *
cma__get_mutex
#ifdef _CMA_PROTO_
	(
	cma__t_int_attr	*attrib)	/* Attributes object to use */
#else	/* no prototypes */
	(attrib)
	cma__t_int_attr	*attrib;	/* Attributes object to use */
#endif	/* prototype */
    {
    cma__t_int_mutex	*new_mutex;	/* Pointer to new mutex */


    cma__trace ((
	    cma__c_trc_obj,
	    "(get_mutex) creating mutex, attr %d",
	    attrib->header.sequence));

    new_mutex = cma__alloc_object (cma__t_int_mutex);

    if ((cma_t_address)new_mutex == cma_c_null_ptr)
	return (cma__t_int_mutex *)cma_c_null_ptr;

    new_mutex->header.type = cma__c_obj_mutex;

    /*
     * NOTE: we do NOT lock the attributes object. Either:
     *
     * a) Application is correctly coded, and does not assume synchronization
     *    between attributes object creation/change and object creation using
     *    that attributes object -- in which case it uses mutexes correctly,
     *    and synchronization is taken care of, or ...
     * b) Application assumes implicit synchronization, in which case there
     *    is a race condition we can't do anything about anyway. Either
     *    it'll get good data or bad data depending on timing... so they get
     *    what they deserve.
     */
    new_mutex->attributes = attrib;
    new_mutex->mutex_kind = attrib->mutex_kind;
    cma__obj_clear_name (new_mutex);
    new_mutex->owner = (cma__t_int_tcb *)cma_c_null_ptr;
    cma__queue_init (&new_mutex->queue);
    cma__tac_clear (&new_mutex->spindle);

    if (new_mutex->mutex_kind == cma_c_mutex_fast) {
	cma__tac_clear (&new_mutex->lock);
	new_mutex->unlock = &new_mutex->lock;
	cma__tac_set (&new_mutex->event);
	}
    else {
	cma__tac_set (&new_mutex->lock);
	new_mutex->unlock = &new_mutex->bitbucket;
	new_mutex->nest_count = 0;
	cma__tac_clear (&new_mutex->event);
	cma__tac_set (&new_mutex->waiters);
	}

    cma__int_lock (cma__g_known_mutexes.mutex);
    new_mutex->header.sequence = cma__g_known_mutexes.sequence++;
    cma__queue_insert (&new_mutex->header.queue, &cma__g_known_mutexes.queue);
    cma__int_unlock (cma__g_known_mutexes.mutex);

    return new_mutex;
    }

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Initialize CMA_MUTEX.C local data
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
 *	initialize static data
 */
extern void
cma__init_mutex
#ifdef _CMA_PROTO_
	(void)
#else	/* no prototypes */
	()
#endif	/* prototype */
    {
    cma__g_known_mutexes.mutex	= cma__get_first_mutex (&cma__g_def_attr);
    cma__obj_set_name (cma__g_known_mutexes.mutex, "known mutex list");
    cma__g_global_lock		= cma__get_first_mutex (&cma__g_def_attr);

    /*
     * Manually "fix up" the global lock mutex to behave as a recursive mutex,
     * rather than creating a second default attributes object just for it.
     */
    cma__g_global_lock->mutex_kind	= cma_c_mutex_recursive;
    cma__tac_set (&cma__g_global_lock->lock);
    cma__g_global_lock->unlock		= &cma__g_global_lock->bitbucket;
    cma__g_global_lock->nest_count	= 0;
    cma__tac_clear (&cma__g_global_lock->event);
    cma__tac_set (&cma__g_global_lock->waiters);
    cma__obj_set_name (cma__g_global_lock, "global lock");
    }

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Internal routine to wait/block on a mutex
 *
 *  FORMAL PARAMETERS:
 *
 *	mutex		Mutex object to wait on
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
 *	cma_t_normal, or error status
 *
 *  SIDE EFFECTS:
 *
 *	none
 */
extern cma_t_status
cma__int_mutex_block
#ifdef _CMA_PROTO_
	(
	cma__t_int_mutex	*mutex)
#else	/* no prototypes */
	(mutex)
	cma__t_int_mutex	*mutex;
#endif	/* prototype */
    {
    cma__t_int_tcb	*tcb = cma__get_self_tcb ();


    switch (mutex->mutex_kind) {
	case cma_c_mutex_recursive : {
	    /*
	     * Lock the internal mutex to gain control of the outer mutex
	     * structure (the mutex lock bit is no use, since it's always
	     * locked).
	     */
	    cma__mulock (mutex);

	    if ((mutex->owner == tcb)
		    || (mutex->owner == (cma__t_int_tcb *)cma_c_null_ptr)) {
		mutex->nest_count++;
		mutex->owner = tcb;	/* Say we have the lock now */
		cma__muunlock (mutex);
		return cma_s_normal;
		}
	    else {
		tcb->wait_mutex = mutex;

		while (1) {

		    if (mutex->owner == (cma__t_int_tcb *)cma_c_null_ptr)
			break;

#if _CMA_THREAD_IS_VP_
		    cma__muunlock (mutex);
		    cma__vp_yield ();	/* "yield gently" */
		    cma__mulock (mutex);
#else
		    cma__mu_set_waiter (mutex);	/* About to wait */
		    cma__prepare_block (tcb);	/* prepare to block */
		    cma__insert_by_sched (tcb, &mutex->queue);
		    cma__dispatch (tcb, cma_c_true, 0);
		    cma__finish_block (tcb, 1);
#endif
		    }

		tcb->wait_mutex = (cma__t_int_mutex *)cma_c_null_ptr;
		mutex->owner = tcb;
		mutex->nest_count++;
		cma__muunlock (mutex);
		}

	    break;
	    }

	case cma_c_mutex_nonrecursive : {
	    /*
	     * Lock the internal mutex to gain control of the outer mutex
	     * structure (the mutex lock bit is no use, since it's always
	     * locked).
	     */
	    cma__mulock (mutex);

	    if (mutex->owner == (cma__t_int_tcb *)cma_c_null_ptr) {
		mutex->owner = tcb;	/* Say we have the lock now */
		cma__muunlock (mutex);
		return cma_s_normal;
		}
	    else {

		/*
		 * Nonrecursive mutexes detect attempts to recursively lock a
		 * mutex (i.e., locking a mutex already owned by the thread).
		 */
		if (mutex->owner == tcb) {
		    cma__muunlock (mutex);
		    return (cma_s_in_use);
		    }

		tcb->wait_mutex = mutex;

		while (1) {

		    if (mutex->owner == (cma__t_int_tcb *)cma_c_null_ptr)
			break;

#if _CMA_THREAD_IS_VP_
		    cma__muunlock (mutex);
		    cma__vp_yield ();	/* "yield gently" */
		    cma__mulock (mutex);
#else
		    cma__mu_set_waiter (mutex);	/* About to wait */
		    cma__prepare_block (tcb);	/* prepare to block */
		    cma__insert_by_sched (tcb, &mutex->queue);
		    cma__dispatch (tcb, cma_c_true, 0);
		    cma__finish_block (tcb, 1);
#endif
		    }

		tcb->wait_mutex = (cma__t_int_mutex *)cma_c_null_ptr;
		mutex->owner = tcb;
		cma__muunlock (mutex);
		}

	    break;
	    }

	case cma_c_mutex_fast : {

	    /*
	     * Give a warning if the same thread already owns the lock (this
	     * evaporates in production builds).
	     */
	    cma__assert_warn (
		    (tcb != mutex->owner),
		    "attempt to relock a mutex");

	    tcb->wait_mutex = mutex;

	    /*
	     * For multiprocessor implementations, loop and retry the lock
	     * bit some number of times before attempting to block.  The
	     * count is configurable via the _CMA_SPINLOOP_ macro (see
	     * cma_config.h).  On multiprocessors, this loop can improve
	     * performance by avoiding the expensive block: on a
	     * uniprocessor, it would just waste time, since the lock can't
	     * be released until a context switch occurs.
	     *
	     * Block the thread.  First, clear the mutex bit saying that
	     * there might be a waiter (this is used to optimize the unlock
	     * path; note the "negative logic" used so the bit can be tested
	     * and reset with a test-and-set operation!). Then, to avoid a
	     * possible race condition, try the mutex again; if it's been
	     * unlocked, just return by exiting the loop.
	     *
	     * Otherwise, enter the kernel and block the thread.
	     */
	    while (1) {

#if _CMA_SPINLOOP_ > 0
		{
		cma_t_integer	spincount;

		for (spincount = 0; spincount < _CMA_SPINLOOP_; spincount++) {

		    if (!cma__tac_isset (&mutex->lock)) {

			if (!cma__test_and_set (&mutex->lock)) {
			    tcb->wait_mutex = (cma__t_int_mutex *)cma_c_null_ptr;
			    return cma_s_normal;
			    }

			}

		    }

		}
#endif

		if (!cma__test_and_set (&mutex->lock)) {
		    tcb->wait_mutex = (cma__t_int_mutex *)cma_c_null_ptr;
		    return cma_s_normal;
		    }

#if _CMA_THREAD_IS_VP_
		cma__vp_yield ();	/* "yield gently" */
#else
		cma__enter_kernel ();
		cma__kernel_unset (&mutex->event);	/* About to wait */
		cma__prepare_block (tcb);	/* prepare to block */
		cma__insert_by_sched (tcb, &mutex->queue);
		cma__dispatch (tcb, cma_c_true, 0);
		cma__finish_block (tcb, 1);
		cma__exit_kernel ();
#endif
		}

	    break;
	    }

	}

    return cma_s_normal;
    }

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Detect whether a mutex is locked.
 *
 *  FORMAL PARAMETERS:
 *
 *	mutex		Pointer to mutex object to test
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
extern cma_t_boolean
cma__int_mutex_locked
#ifdef _CMA_PROTO_
	(
	cma__t_int_mutex	*mutex)	/* Mutex to test */
#else	/* no prototypes */
	(mutex)
	cma__t_int_mutex	*mutex;	/* Mutex to test */
#endif	/* prototype */
    {

    if (cma__tac_isset (&mutex->lock)) {

	switch (mutex->mutex_kind) {
	    case cma_c_mutex_recursive :
	    case cma_c_mutex_nonrecursive : {
		cma__mulock (mutex);

		if (mutex->owner != (cma__t_int_tcb *)cma_c_null_ptr) {
		    cma__muunlock (mutex);
		    return cma_c_true;
		    }
		else {
		    cma__muunlock (mutex);
		    return cma_c_false;
		    }

		break;
		}
	    case cma_c_mutex_fast : {
		return cma_c_true;
		break;
		}
	    default : {
		cma__bugcheck (
			"int_mutex_locked: %lx bad mutex kind %d",
			mutex,
			mutex->mutex_kind);
		return cma_c_false;	/* Not reached */
		}

	    }

	}
    else
	return cma_c_false;

    }

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Internal routine to unblock threads waiting on a mutex
 *
 *  FORMAL PARAMETERS:
 *
 *	mutex		Mutex object
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
 *	cma_s_normal, or error status
 *
 *  SIDE EFFECTS:
 *
 *	none
 */
extern cma_t_status
cma__int_mutex_unblock
#ifdef _CMA_PROTO_
	(
	cma__t_int_mutex	*mutex)
#else	/* no prototypes */
	(mutex)
	cma__t_int_mutex	*mutex;
#endif	/* prototype */
    {
    /*
     * If the event flag was set because this is a friendly mutex, then we
     * need to decrement the count; and only wake other threads if the count
     * goes to 0.
     */
    switch (mutex->mutex_kind) {

	case cma_c_mutex_recursive : {
	    cma__t_int_tcb	*tcb = cma__get_self_tcb ();


	    /*
	     * Before calling this routine, the main mutex unlock code did a
	     * test-and-set on the event bit; since we want a recursive mutex
	     * to always have an "event pending", we need to clear the bit
	     * again.
	     */
	    cma__unset (&mutex->event);	/* Event is always ON */

	    /*
	     * Lock the internal spinlock to gain control of the outer mutex
	     * structure (the mutex lock bit is no use, since it's always
	     * locked).
	     */
	    cma__mulock (mutex);

	    /*
	     * Recursive mutexes detect attempts to release an unowned mutex
	     * or one owned by another thread.
	     */
	    if (mutex->owner == (cma__t_int_tcb *)cma_c_null_ptr) {
		cma__muunlock (mutex);
		return (cma_s_use_error);
		}

	    if (tcb != mutex->owner) {
		cma__muunlock (mutex);
		return (cma_s_in_use);
		}

	    if (--mutex->nest_count == 0) {
		mutex->owner = (cma__t_int_tcb *)cma_c_null_ptr;

#if !_CMA_THREAD_IS_VP_
		if (cma__mu_ifwaiter (mutex)) {
		    cma__t_int_tcb	*itcb;


		    if (!cma__queue_empty (&mutex->queue)) {
			cma__queue_dequeue (
				&mutex->queue,
				itcb,
				cma__t_int_tcb);

			if (!cma__queue_empty (&mutex->queue))
			    cma__mu_set_waiter (mutex);

			cma__ready (itcb, cma_c_false);
			cma__try_run (itcb);
			}

		    }
#endif

		}

	    cma__muunlock (mutex);
	    break;
	    }

	case cma_c_mutex_nonrecursive : {
	    cma__t_int_tcb	*tcb = cma__get_self_tcb ();


	    /*
	     * Before calling this routine, the main mutex unlock code did a
	     * test-and-set on the event bit; since we want a nonrecursive
	     * mutex to always have an "event pending", we need to clear the
	     * bit again.
	     */
	    cma__unset (&mutex->event);	/* Event is always ON */

	    /*
	     * Lock the internal mutex to gain control of the outer mutex
	     * structure (the mutex lock bit is no use, since it's always
	     * locked).  Using a full mutex instead of a simple lock bit
	     * allows us to block if the lock is busy, rather than spinning.
	     * Note that the inner lock is cma_c_mutex_fast, so we don't just
	     * recurse through all this stuff.
	     */
	    cma__mulock (mutex);

	    /*
	     * Nonrecursive mutexes detect attempts to release an unowned
	     * mutex or one owned by another thread.
	     */
	    if (mutex->owner == (cma__t_int_tcb *)cma_c_null_ptr) {
		cma__muunlock (mutex);
		return (cma_s_use_error);
		}

	    if (tcb != mutex->owner) {
		cma__muunlock (mutex);
		return (cma_s_in_use);
		}

	    mutex->owner = (cma__t_int_tcb *)cma_c_null_ptr;

#if !_CMA_THREAD_IS_VP_
		if (cma__mu_ifwaiter (mutex)) {
		    cma__t_int_tcb	*itcb;


		    if (!cma__queue_empty (&mutex->queue)) {
			cma__queue_dequeue (
				&mutex->queue,
				itcb,
				cma__t_int_tcb);

			if (!cma__queue_empty (&mutex->queue))
			    cma__mu_set_waiter (mutex);

			cma__ready (itcb, cma_c_false);
			cma__try_run (itcb);
			}

		    }
#endif

	    cma__muunlock (mutex);
	    break;
	    }

#if !_CMA_THREAD_IS_VP_			/* No action for fast mutex on VP */
	case cma_c_mutex_fast : {
	    cma__t_int_tcb	*itcb;


	    cma__enter_kernel ();

	    if (!cma__queue_empty (&mutex->queue)) {
		cma__queue_dequeue (
			&mutex->queue,
			itcb,
			cma__t_int_tcb);

		if (!cma__queue_empty (&mutex->queue))
		    cma__kernel_unset (&mutex->event);

		cma__ready (itcb, cma_c_false);
		cma__try_run (itcb);
		}

	    cma__exit_kernel ();
	    break;
	    }
#endif

	}

    return cma_s_normal;
    }

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Find out whether a mutex is locked.
 *
 *  FORMAL PARAMETERS:
 *
 *	mutex		Mutex object to text
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
 *	cma_c_true if mutex is locked, cma_c_false otherwise.
 *
 *  SIDE EFFECTS:
 *
 *	none
 */
extern cma_t_boolean
cma__mutex_locked
#ifdef _CMA_PROTO_
	(
	cma_t_mutex	*mutex)		/* Mutex to test */
#else	/* no prototypes */
	(mutex)
	cma_t_mutex	*mutex;		/* Mutex to test */
#endif	/* prototype */
    {
#ifdef NDEBUG
    return cma__int_mutex_locked (
	    (cma__t_int_mutex *)((cma__t_int_handle *)mutex)->pointer);
#else
    cma__t_int_mutex	*int_mutex;

    int_mutex = cma__validate_mutex (mutex);
    return cma__int_mutex_locked (int_mutex);
#endif
    }

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Try to lock a mutex, but return immediately with boolean status if
 *	the mutex is already locked (internal)
 *
 *  FORMAL PARAMETERS:
 *
 *	mutex		Pointer to mutex object to lock
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
extern cma_t_boolean
cma__int_try_lock
#ifdef _CMA_PROTO_
	(
	cma__t_int_mutex	*mutex)	/* Mutex to lock */
#else	/* no prototypes */
	(mutex)
	cma__t_int_mutex	*mutex;	/* Mutex to lock */
#endif	/* prototype */
    {
    cma__t_int_tcb	*tcb;


#ifndef NDEBUG
    tcb = cma__get_self_tcb ();
#endif

    if (cma__test_and_set (&mutex->lock)) {

	switch (mutex->mutex_kind) {
	    case cma_c_mutex_nonrecursive : {
#ifdef NDEBUG
		tcb = cma__get_self_tcb ();
#endif
		cma__mulock (mutex);

		if (mutex->owner == (cma__t_int_tcb *)cma_c_null_ptr) {
		    mutex->owner = tcb;
		    cma__muunlock (mutex);
		    return cma_c_true;
		    }
		else {
		    cma__muunlock (mutex);
		    return cma_c_false;
		    }

		break;
		}

	    case cma_c_mutex_recursive : {
#ifdef NDEBUG
		tcb = cma__get_self_tcb ();
#endif
		cma__mulock (mutex);

		if (tcb == mutex->owner
			|| mutex->owner == (cma__t_int_tcb *)cma_c_null_ptr) {
		    mutex->nest_count++;
		    mutex->owner = tcb;
		    cma__muunlock (mutex);
		    return cma_c_true;
		    }
		else {
		    cma__muunlock (mutex);
		    return cma_c_false;
		    }

		break;
		}

	    case cma_c_mutex_fast : {
		return cma_c_false;
		break;
		}

	    }

	}

#ifndef NDEBUG
    mutex->owner = tcb;
#endif
    return cma_c_true;
    }

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Performs pre (1) and post (0) fork reinitialization work.
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
extern void
cma__reinit_mutex
#ifdef _CMA_PROTO_
	(
	cma_t_integer	flag)
#else	/* no prototypes */
	(flag)
	cma_t_integer	flag;
#endif	/* prototype */
    {
    cma__t_queue        *ptr;
    cma__t_int_mutex    *mutex;

    if (flag == cma__c_reinit_prefork_lock) {
	cma__int_lock (cma__g_known_mutexes.mutex);
	}
    else if (flag == cma__c_reinit_postfork_unlock) {
	cma__int_unlock (cma__g_known_mutexes.mutex);
	}
    else if (flag == cma__c_reinit_postfork_clear) {
	ptr = &cma__g_known_mutexes.queue;

	while ((ptr = cma__queue_next (ptr)) != &cma__g_known_mutexes.queue) {
	    mutex = (cma__t_int_mutex *)ptr;
	    cma__queue_init (&mutex->queue);
	    }

	}

    }
/*  VAX/DEC CMS REPLACEMENT HISTORY, Element CMA_MUTEX.C */
/*  *28    2-JUL-1993 14:38:00 BUTENHOF "Fiddle w/ queue element on unblock" */
/*  *27   16-APR-1993 13:04:10 BUTENHOF "Update queue operations to allow INSQUE/REMQUE" */
/*  *26    1-APR-1993 14:32:52 BUTENHOF "Use formatted name for user handle?" */
/*  *25    4-DEC-1992 12:41:49 BUTENHOF "Change OSF/1 yield" */
/*  *24    2-NOV-1992 13:25:15 BUTENHOF "Speedyize & fix race" */
/*  *23   16-OCT-1992 12:19:46 BUTENHOF "Fix VP block" */
/*  *22    5-OCT-1992 15:50:47 BUTENHOF "Remove kernel-sets inside kernel" */
/*  *21   22-SEP-1992 12:39:21 BUTENHOF "Fix mutex locking" */
/*  *20   15-SEP-1992 13:49:50 BUTENHOF "Change object sequencing" */
/*  *19    2-SEP-1992 16:25:24 BUTENHOF "Separate semaphores from kernel lock" */
/*  *18   25-AUG-1992 12:42:59 SCALES "Add default to int-mutex-locked switch" */
/*  *17   25-AUG-1992 11:48:28 BUTENHOF "Adjust Mach yield operations" */
/*  *16   21-AUG-1992 13:42:09 BUTENHOF "Use spinlocks on kernel thread semaphores instead of kernel_critical" */
/*  *15    5-JUN-1992 13:34:53 BUTENHOF "Fix delete_pending attr handling" */
/*  *14   13-MAR-1992 14:08:57 BUTENHOF "Fix mutex lock/unlock code" */
/*  *13   10-MAR-1992 16:26:01 BUTENHOF "Eliminate need for TRY/CATCH on pthread mutex lock" */
/*  *12   19-FEB-1992 04:34:47 BUTENHOF "Fix typos" */
/*  *11   18-FEB-1992 15:29:21 BUTENHOF "Adapt to new alloc_mem protocol" */
/*  *10    5-NOV-1991 14:58:58 BUTENHOF "Fix a bug in cma__get_mutex()" */
/*  *9    28-JUN-1991 13:20:36 BUTENHOF "Initialize owner at creation" */
/*  *8    10-JUN-1991 18:22:24 SCALES "Add sccs headers for Ultrix" */
/*  *7     5-JUN-1991 16:13:04 CURTIN "fork work" */
/*  *6    29-MAY-1991 17:45:28 BUTENHOF "Fix possible race condition in block" */
/*  *5    10-MAY-1991 11:46:46 CURTIN "Use new macros in cma_mutex.h" */
/*  *4    12-APR-1991 23:36:05 BUTENHOF "Init atomic bits using new constants" */
/*  *3    13-FEB-1991 17:54:43 BUTENHOF "Change mutex attribute name symbols" */
/*  *2    12-FEB-1991 23:09:48 BUTENHOF "Recursive/nonrecursive mutexes" */
/*  *1    12-DEC-1990 21:47:28 BUTENHOF "Mutexes" */
/*  VAX/DEC CMS REPLACEMENT HISTORY, Element CMA_MUTEX.C */
