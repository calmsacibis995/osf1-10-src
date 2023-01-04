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
/*
 * @(#)$RCSfile: cma_context.c,v $ $Revision: 4.2.5.2 $ (DEC) $Date: 1993/05/11 22:00:25 $
 */
/*
 *  FACILITY:
 *
 *	CMA services
 *
 *  ABSTRACT:
 *
 *	Per-thread context management
 *
 *  AUTHORS:
 *
 *	Dave Butenhof
 *
 *  CREATION DATE:
 *
 *	26 July 1989
 *
 *  MODIFICATION HISTORY:
 *
 *	001	Dave Butenhof	16 August 1989
 *		Make cma_create_key conform to new CMA.h (take att object
 *		handle by reference).
 *	002	Dave Butenhof	31 August 1989
 *		Change back to take att object by value.
 *	003	Dave Butenhof	9 October 1989
 *		Use cma__error to raise exceptions where necessary.
 *	004	Dave Butenhof	12 October 1989
 *		Convert to use internal mutex operations.
 *	005	Dave Butenhof	16 October 1989
 *		Assorted bugs detected by first per-thread context test
 *		program.
 *	006	Dave Butenhof	18 October 1989
 *		Include cma_stack.h header file for cma__get_self_tcb.
 *	007	Dave Butenhof	19 October 1989
 *		Use new type-specific handle validation macros.
 *      008     Webb Scales     19 October 1989
 *              Add type-casts where MIPS pcc requires them.
 *	009	Dave Butenhof	20 October 1989
 *		Support null destructors (sounds useful & doesn't
 *		specifically violate architecture...).
 *	010	Dave Butenhof	30 November 1989
 *		Modify external entries to track POSIX changes to names and
 *		argument ordering.
 *	011	Dave Butenhof	9 March 1990
 *		Fix one-off error in context allocation.
 *	012	Dave Butenhof	11 April 1990
 *		Catch possible errors from cma__alloc_mem while mutex is
 *		held, and unlock before reraising exception.
 *	013	Dave Butenhof	30 April 1990
 *		Catch exceptions during destructor callbacks and cleanup.
 *	014	Dave Butenhof	12 June 1990
 *		Don't raise an exception in cma__run_down_context if a
 *		destructor causes an exception. The thread is running down
 *		anyway, and we don't want to say the thread failed merely
 *		because a destructor didn't work right.  Do we?
 *	015	Dave Butenhof	18 June 1990
 *		Release TCB mutex around call to destructor, to allow it more
 *		freedom.
 *	016	Paul Curtin	06 August 1990
 *		Replaced memcpy w/ cma__memcpy
 *		Replaced memset w/ cma__memset
 *      017     Webb Scales     15 August 1990
 *              Initialize first PTC area.
 *	018	Dave Butenhof	27 August 1990
 *		Change interfaces to pass handles & structures by reference.
 *	019	Dave Butenhof	11 September 1990
 *		1003.4a/D4 removes automatic call to context destructor when
 *		a previous value is replaced by a set_context. Since that's
 *		what we wanted CMA to do originally, and since we've decided
 *		to make incompatible interface changes for BL4 anyway, we're
 *		making the same changes.
 *	020	Dave Butenhof	4 October 1990
 *		Fix a few locking errors; two (accessing the global key
 *		count) are probably relatively safe, being just visibility
 *		issues (if you use a valid key without having any way to know
 *		it's valid, who really cares if you get an error?). The third
 *		would inevitably have gotten a segmentation fault as soon as
 *		we ran with true parallel threads; the destructor routine was
 *		tested and then called in context rundown, without a lock.
 *		Inevitably, some thread would create a new key that resulted
 *		in freeing the old destructor array while the compiler
 *		retained a pointer to it.
 *	021	Dave Butenhof	29 October 1990
 *		Fix a bug in clearing the unused cells of a new context
 *		array; it was using "new_size" as a count of cells rather
 *		than as a size in bytes, causing it to overwrite following
 *		memory packets!
 *	022	Dave Butenhof	14 December 1990
 *		Change cma_attributes.h to cma_attr.h (shorten length)
 *	023	Paul Curtin	30 April 1991
 *		Added initialization for preallocated keys (10).
 *	024	Paul Curtin	24 May 1991
 *		Added a cma__reinit_context routine.
 *	025	Paul Curtin	 5 June 1991
 *		Rearranged flags in reinit routine.
 *	026	Paul Curtin	18 November 1991
 *		Alpha work: added include for cma_vm.h
 *	027	Dave Butenhof	18 December 1991
 *		Fix QAR 151: clear context after calling destructor, so that
 *		if destructor sets a new value it won't affect the next
 *		thread to use the TCB.
 *	028	Dave Butenhof	10 February 1992
 *		Fix off-by-one error in clearing added context cells.
 *	029	Dave Butenhof	10 February 1992
 *		Use cma__alloc_zero where appropriate, and handle null
 *		return instead of exception of allocation error.
 *	030	Dave Butenhof	 4 May 1993
 *		Long ago, in an office not so far away, I made get/set
 *		context stop using the TCB lock, since context isn't shared
 *		anyway. Somehow, I neglected run_down_context, which is even
 *		worse since it unlocks and relocks on every iteration.
 */


/*
 *  INCLUDE FILES
 */

#include <cma.h>
#include <cma_defs.h>
#include <cma_handle.h>
#include <string.h>
#include <cma_context.h>
#include <cma_tcb.h>
#include <cma_errors.h>
#include <cma_mutex.h>
#include <cma_attr.h>
#include <cma_stack.h>
#include <cma_util.h>
#include <cma_vm.h>

/*
 *  GLOBAL DATA
 */

cma_t_natural		cma__g_context_next;	/* Next available key */
cma_t_integer		cma__g_context_size;	/* Maximum index of key array */
cma__t_int_mutex	*cma__g_context_mutex;	/* Mutex for key creation */

/*
 *  LOCAL DATA
 */

static cma__t_destructor_array	cma___g_destructors;	/* Destructor table */

/*
 * LOCAL FUNCTIONS
 */

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Create CMA context key.
 *
 *  FORMAL PARAMETERS:
 *
 *	attr		Attributes object to use (or cma_c_null).
 * 
 *	destructor	Address of the key's destructor routine (to be called
 *			when the context under this key for a thread is run
 *			down).
 *
 *  IMPLICIT INPUTS:
 *
 *	cma__g_context_next	Next available key number
 *	cma__g_context_size	Size of destructor (key) array
 *
 *  IMPLICIT OUTPUTS:
 *
 *	none
 *
 *  FUNCTION VALUE:
 *
 *	The new key
 *
 *  SIDE EFFECTS:
 *
 *	May reallocate the the key array.
 */
extern void
cma_key_create
#ifdef _CMA_PROTO_
	(
	cma_t_key		*key,	/* Return value of new key */
	cma_t_attr		*attr,	/* Attributes to use */
	cma_t_destructor	destructor) /* Destructor for this key */
#else	/* no prototypes */
	(key, attr, destructor)
	cma_t_key		*key;	/* Return value of new key */
	cma_t_attr		*attr;	/* Attributes to use */
	cma_t_destructor	destructor; /* Destructor for this key */
#endif	/* prototype */
    {
    cma_t_natural	int_key;	/* Temporary to hold new key */
    cma__t_int_attr	*int_att;


    int_att = cma__validate_default_attr (attr);
    cma__int_lock (cma__g_context_mutex);
    int_key = cma__g_context_next++;

    /*
     * Key 0 is reserved to CMA for use in our TCB prolog
     */
    cma__assert_warn (
	    int_key > 0,
	    "context key 0 [reserved to DECthreads] is being given to a client."
	    );

    /*
     * If the current highest index in the array is too small to fit the new
     * key, then reallocate the array.
     */
    if (int_key >= cma__g_context_size) {
	cma__t_destructor_array	temp_array;	/* Hold new array */
	cma_t_integer		temp_size;	/* Size of new array */


	temp_size = cma__g_context_size + cma__c_context_increment;
	temp_array = (cma__t_destructor_array)cma__alloc_mem (
		temp_size * sizeof (cma_t_destructor));

	if ((cma_t_address)temp_array == cma_c_null_ptr) {
	    cma__int_unlock (cma__g_context_mutex);
	    cma__error (exc_s_insfmem);
	    }

	if (cma___g_destructors 
		!= (cma__t_destructor_array)cma_c_null_ptr) {
	    cma__memcpy (
		    (char *)temp_array,
		    (char *)cma___g_destructors,
		    (cma_t_integer) 
			(sizeof(cma_t_destructor) * cma__g_context_size)
		    );
	    }

	cma__g_context_size = temp_size;
	cma___g_destructors = temp_array;
	}

    cma___g_destructors[int_key] = destructor;
    cma__int_unlock (cma__g_context_mutex);
    *key = (cma_t_key)int_key;
    }

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Get the value for a context key within a thread
 *
 *  FORMAL PARAMETERS:
 *
 *	key		Context key to set a value for
 *
 *	context_value	Return value of context key in current thread
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
 *	Context value (or null pointer if no value)
 *
 *  SIDE EFFECTS:
 *
 *	none
 */
extern void
cma_key_get_context
#ifdef _CMA_PROTO_
	(
	cma_t_key	key,		/* Key to get */
	cma_t_address	*context_value)
#else	/* no prototypes */
	(key, context_value)
	cma_t_key	key;		/* Key to get */
	cma_t_address	*context_value;
#endif	/* prototype */
    {
    cma__t_int_tcb	*self;		/* Pointer to TCB */
    cma_t_integer	next_context;	/* Local copy of next key value */


    /*
     * Note that we don't lock any mutexes. This keeps the getcontext
     * function lean and mean. There are two important assumptions in this:
     *
     * 1) If any other thread is modifying the maximum context value, we may
     * get the wrong value, with a possible "spurious" failure. However, that
     * would mean someone is creating a context key in one thread and USING
     * it in another without synchronization. Tough cookies: you get what you
     * pay for.
     *
     * 2) We assume that no other thread will access or modify the context
     * data in this thread's TCB. Since we don't lock the TCB, such access
     * could result in erroneous results in one or the other. DECthreads
     * doesn't support any interfaces to touch another thread's context
     * values, so anyone doing so is just asking for trouble, and I don't
     * mind giving it to them to save a lock operation!
     */
    next_context = cma__g_context_next;

    if ((key <= 0) || (key >= next_context))
	cma__error (cma_s_badparam);

    self = cma__get_self_tcb ();	/* Get current thread's TCB */

    /*
     * If the requested key is not within the allocated context array (or if
     * there is no context array), then return the value null ("no context");
     * otherwise return the current value of the context (which may also be
     * null).
     */
    if (self->context_count <= key)
	*context_value = cma_c_null_ptr;
    else
	*context_value = self->contexts[key];

    }

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Set the value for a context key within a thread.  If a value is
 *	already set, then call the destructor routine for the key (if any).
 *
 *  FORMAL PARAMETERS:
 *
 *	key		Context key to set a value for
 *
 *	context_value	Value of the context
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
 *	May reallocate the the thread's context value array.
 */
extern void
cma_key_set_context
#ifdef _CMA_PROTO_
	(
	cma_t_key	key,		/* Key to set */
	cma_t_address	context_value)	/* Context value */
#else	/* no prototypes */
	(key, context_value)
	cma_t_key	key;		/* Key to set */
	cma_t_address	context_value;	/* Context value */
#endif	/* prototype */
    {
    cma__t_int_tcb	*self;		/* Pointer to TCB */
    cma_t_integer	next_context;	/* Snapshot of global key count */


    /*
     * Note that we don't lock any mutexes. This keeps the getcontext
     * function lean and mean. There are two important assumptions in this:
     *
     * 1) If any other thread is modifying the maximum context value, we may
     * get the wrong value, with a possible "spurious" failure. However, that
     * would mean someone is creating a context key in one thread and USING
     * it in another without synchronization. Tough cookies: you get what you
     * pay for.
     *
     * 2) We assume that no other thread will access or modify the context
     * data in this thread's TCB. Since we don't lock the TCB, such access
     * could result in erroneous results in one or the other. DECthreads
     * doesn't support any interfaces to touch another thread's context
     * values, so anyone doing so is just asking for trouble, and I don't
     * mind giving it to them to save a lock operation!
     */
    next_context = cma__g_context_next;

    if ((key <= 0) || (key >= next_context))
	cma__error (cma_s_badparam);

    self = cma__get_self_tcb ();	/* Get current thread's TCB */

    /*
     * If the key value is higher than the current size of the thread's
     * context array, then allocate a new array.  Make it large enough to
     * hold the highest defined key value.  Copy the current context array
     * (if any) into the new one, and free the old one.
     */
    if (self->context_count <= key) {
	cma__t_context_list	new_list;	/* Pointer to new list */
	cma_t_natural		new_size;	/* Size of new list */


	new_size = sizeof (cma_t_address) * next_context;
	new_list = (cma__t_context_list)cma__alloc_zero (new_size);

	if ((cma_t_address)new_list == cma_c_null_ptr) {
	    cma__error (exc_s_insfmem);
	    }

	if (self->contexts != (cma__t_context_list)cma_c_null_ptr) {
	    cma__memcpy (
		    (char *)new_list,
		    (char *)self->contexts,
		    (self->context_count) * sizeof (cma_t_address));
	    cma__free_mem ((cma_t_address)self->contexts);
	    }

	self->contexts = new_list;
	self->context_count = next_context;
	}

    self->contexts[key] = context_value;
    }

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Internal initialization for context sub-facility
 *
 *  FORMAL PARAMETERS:
 *
 *	none
 *
 *  IMPLICIT INPUTS:
 *
 *	cma__g_context_next	Next available key number
 *	cma__g_context_size	Size of destructor (key) array
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
cma__init_context
#ifdef _CMA_PROTO_
	(void)
#else	/* no prototypes */
	()
#endif	/* prototype */
    {
    cma__g_context_next = 1;
    cma__g_context_size = 0;
    cma___g_destructors = (cma__t_destructor_array)cma_c_null_ptr;
    cma__g_context_mutex = cma__get_mutex (&cma__g_def_attr);
    cma__obj_set_name (cma__g_context_mutex, "per-thread context");
    }


/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Perform work prior to and after fork(), depending on flag value.
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
cma__reinit_context
#ifdef _CMA_PROTO_
	(
	cma_t_integer	    flag)
#else	/* no prototypes */
	(flag)
	cma_t_integer	    flag;
#endif	/* prototype */
    {

    if (flag == cma__c_reinit_prefork_lock) {
	cma__int_lock(cma__g_context_mutex);
	}
    else if (flag == cma__c_reinit_postfork_unlock) {
	cma__int_unlock(cma__g_context_mutex);
	}
    }


/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Cancel all per-thread context for a thread.
 *
 *  FORMAL PARAMETERS:
 *
 *	none
 *
 *  IMPLICIT INPUTS:
 *
 *	cma___g_destructor_array	Array of destructors
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
cma__run_down_context
#ifdef _CMA_PROTO_
	(
	cma__t_int_tcb	*tcb)		/* TCB for the thread */
#else	/* no prototypes */
	(tcb)
	cma__t_int_tcb	*tcb;		/* TCB for the thread */
#endif	/* prototype */
    {
    cma_t_natural	ctx;		/* Context array index */


    /*
     * Traverse the context array; destroy each context which exists in the
     * thread.
     */
    for (ctx = 1; ctx < tcb->context_count; ctx++)
	if (tcb->contexts[ctx] != cma_c_null_ptr)
	    {
	    cma_t_address	old_value = tcb->contexts[ctx];
	    cma_t_destructor	dest_function;


	    dest_function = cma___g_destructors[ctx];

	    /*
	     * Call the destructor for the key, and set the value to null.
	     */
	    if (dest_function != (cma_t_destructor)cma_c_null_ptr)
		(*dest_function) (old_value);

	    tcb->contexts[ctx] = cma_c_null_ptr;
	    }

    }
/*  VAX/DEC CMS REPLACEMENT HISTORY, Element CMA_CONTEXT.C */
/*  *13    4-MAY-1993 11:38:02 BUTENHOF "Reduce locking in rundown" */
/*  *12   13-MAR-1992 14:07:52 BUTENHOF "Make context mutex global for pthreads" */
/*  *11   18-FEB-1992 15:28:09 BUTENHOF "Use cma__alloc_zero" */
/*  *10   10-FEB-1992 08:50:31 BUTENHOF "Fix off-by-one error" */
/*  *9    19-DEC-1991 13:08:20 BUTENHOF "Fix QAR 151" */
/*  *8    18-NOV-1991 11:11:03 CURTIN "Alpha work: added include for cma_vm.h" */
/*  *7    10-JUN-1991 18:18:09 SCALES "Add sccs headers for Ultrix" */
/*  *6     5-JUN-1991 16:11:55 CURTIN "fork work" */
/*  *5    24-MAY-1991 16:44:56 CURTIN "Added a new reinit routine" */
/*  *4    30-APR-1991 17:22:26 CURTIN "quick fix" */
/*  *3    30-APR-1991 16:50:30 CURTIN "Added some pre-allocated context keys" */
/*  *2    14-DEC-1990 00:55:20 BUTENHOF "Change module names" */
/*  *1    12-DEC-1990 21:43:22 BUTENHOF "Per-thread context" */
/*  VAX/DEC CMS REPLACEMENT HISTORY, Element CMA_CONTEXT.C */
