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
 * @(#)$RCSfile: cma_attr.c,v $ $Revision: 4.2.7.2 $ (DEC) $Date: 1993/08/18 14:43:35 $
 */
/*
 *  FACILITY:
 *
 *	DECthreads core
 *
 *  ABSTRACT:
 *
 *	Manage attributes objects
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
 *		Add external interface routines.
 *	002	Dave Butenhof	21 August 1989
 *		Correct some comments.
 *	003	Dave Butenhof	24 August 1989
 *		Convert to use queue package instead of list traversal.
 *	004	Dave Butenhof	30 August 1989
 *		First debugging run: fix problems with cma_c_null and
 *		validation of handles.
 *	005	Dave Butenhof	14 September 1989
 *		Allow cma_c_null on cma_get_*_att routines (return the
 *		default value).
 *	006	Dave Butenhof	15 September 1989
 *		Beef up cache handling with revision counts (to handle
 *		objects which are still "loose" when a cache is flushed
 *		because of modifications to attributes).
 *	007	Dave Butenhof	21 September 1989
 *		cma_set_stacksize_att and cma_set_guardsize_att need to bump
 *		stack revision count as well as TCB count (since they will
 *		also affect stack caching, if ever implemented).
 *	008	Dave Butenhof	29 September 1989
 *		Don't flush TCB cache on priority attribute change: since
 *		cma_fork will always overwrite priority anyway.
 *	009	Bob Conti	6 October 1989
 *		Add call to raise badparam.
 *	010	Dave Butenhof	11 October 1989
 *		Convert to use internal mutex operations.
 *	011	Dave Butenhof	18 October 1989
 *		cma__queue_insert is now a macro, which expands to a block;
 *		this module includes a call which is the sole statement on
 *		the "then" of an if statement, and the trailing ";" (after a
 *		"}") breaks the "else" clause.  Fix the reference in such a
 *		way that some future conversion back to a routine call won't
 *		break it again.
 *	012	Dave Butenhof	19 October 1989
 *		Use new type-specific handle validation macros.
 *	013	Dave Butenhof	19 October 1989
 *		Substitute "cma_t_address" for explicit "void *" to make
 *		porting easier.
 *	014	Dave Butenhof	19 October 1989
 *		Modify use of queue operations to use appropriate casts
 *		rather than depending on ANSI C "void *" (sigh).
 *	015	Webb Scales	19 October 1989
 *		Add type-casts where MIPS pcc requires them.
 *	016	Webb Scales	20 October 1989
 *		Add type-casts on comparison of enum's for MIPS pcc.
 *	017	Webb Scales	20 October 1989
 *		Created a "definition" for cma__g_def_attr.
 *	018	Dave Butenhof	23 October 1989
 *		Fix locking during attributes object free.
 *	019	Dave Butenhof	24 October 1989
 *		Fix some anticipated problems with object caching coherency.
 *	020	Dave Butenhof	25 October 1989
 *		Fix some subtle problems introduced by 019 changes.
 *	021	Dave Butenhof	1 November 1989
 *		Let cma__get_first_mutex handle sequence #.
 *	022	Webb Scales	10 November 1989
 *		In cma__get_attributes, release the attrib->mutex earlier to
 *		avoid a deadlock with cma__get_mutex trying to lock it.
 *	023	Dave Butenhof	30 November 1989
 *		Modify external entries to track POSIX changes to names and
 *		argument ordering.
 *	024	Dave Butenhof	5 December 1989
 *		Fix remaining bug in conversion from "inherit_pri" to
 *		"inherit_sched" (initialization of attribute).
 *	025	Dave Butenhof	5 December 1989
 *		Remove functions dealing with initial_test.
 *	026	Dave Butenhof	8 December 1989
 *		Signal cma_s_unimp when client attempts to change priority or
 *		scheduling policy.
 *	027	Dave Butenhof	27 February 1990
 *		Make scheduling policy/priority conform to new arch. spec;
 *		cma_attr_set_sched takes both policy and priority; support
 *		new symbols for policy and priority.
 *	028	Dave Butenhof	28 February 1990
 *		Ooops--we've got two "cma_c_sched_default" symbols.  Change
 *		the inherit-sched one to "cma_c_sched_use_default".
 *	029	Dave Butenhof	28 February 1990
 *		Ooops--we've got two "cma_c_sched_default" symbols.  Change
 *		the inherit-sched one to "cma_c_sched_use_default".
 *	030	Dave Butenhof	8 March 1990
 *		Raise cma_e_badparam if client tries to set stack size to
 *		0 or less.
 *	031	Dave Butenhof	5 April 1990
 *		Create embedded objects using the same attributes object that
 *		the parent object was created with, to avoid worrying about
 *		locking order protocols during access and deletion.
 *	032	Dave Butenhof	9 April 1990
 *		Use new "known_object" structure for known attributes queue
 *		(includes mutex).
 *	033	Dave Butenhof	10 April 1990
 *		Catch memory errors over object allocation, and set names in
 *		internal objects.
 *	034	Dave Butenhof	12 April 1990
 *		Name embedded mutex in attr obj.
 *	035	Dave Butenhof	1 June 1990
 *		Implement highwater marking on cache queues.
 *	036	Dave Butenhof	7 June 1990
 *		Erase destroyed objects if not NDEBUG, to catch references to
 *		dead objects (can be used in conjunction with cma_maxattr=0,
 *		which disables caching).
 *	037	Dave Butenhof	18 June 1990
 *		Use macros to clear object name (only defined for debug
 *		build).
 *	038	Webb Scales	19 June 1990
 *		"Turn on" scheduling policies and priorities.
 *	039	Dave Butenhof	26 June 1990
 *		Add new attribute for mutex objects; to control whether lock
 *		is "friendly" (supports nested locks by same thread).
 *	040	Paul Curtin	3 July 1990
 *		Added an include for cma_stack_int.h 
 *	041	Paul Curtin	24 July 1990
 *		Putting cma__roundup_chunksize to use.
 *	042	Dave Butenhof	31 July 1990
 *		Increment mutex cache revision count when setting mutex kind.
 *	043	Dave Butenhof	27 August 1990
 *		Change interfaces to pass handles & structures by reference.
 *	044	Dave Butenhof	14 December 1990
 *		Change cma_attributes.h to cma_attr.h (shorten length)
 *	045	Dave Butenhof	12 February 1991
 *		Change "friendly" mutex kind to "recursive", and add
 *		"nonrecursive"
 *	046	Dave Butenhof	01 May 1991
 *		Add arguments to cma__bugcheck() calls.
 *	047	Paul Curtin	10 May 1991
 *		Converted a number of function to use new internal macros.
 *	048	Paul Curtin	24 May 1991
 *		Added a cma__reinit_attr routine.
 *	049	Dave Butenhof	29 May 1991
 *		Raise cma_e_unimp on attempts to modify thread
 *		priority or policy.
 *	050	Paul Curtin	 5 May 1991
 *		Rearranged flags in reinit routine.
 *	051	Dave Butenhof	07 October 1991
 *		Macro-ize guardsize functions.
 *	052	Dave Butenhof	31 October 1991
 *		Fix a bug in attributes object cache recovery, analyzed by
 *		Alan Peckham (thanks!) -- I implemented an attributes
 *		revision count to prevent accidentally caching objects on an
 *		attr obj that had been recycled (potentially with different
 *		attribute values). However, I also CLEARED the revision count
 *		when reusing a cached attr. object, which nullified the fix.
 *		The revision count should never be cleared... in fact,
 *		freeing/reusing an attr object should increment the revision
 *		count to separate it from its past life!
 *	053	Dave Butenhof	20 December 1991
 *		It'd really be nice if cma__init_attr set the mutex_kind of
 *		the default attr. object before creating a mutex using it,
 *		wouldn't it?
 *	054	Dave Butenhof	23 December 1991
 *		Stacks aren't usually cached separately, but there IS an
 *		entry for the cache, and the queue isn't being initialized
 *		in the default attributes object (cma__init_attr). Do so.
 *	055	Dave Butenhof	10 February 1992
 *		A law of nature has just been changed: cma__alloc_mem now
 *		returns cma_c_null_ptr on an allocation failure, rather than
 *		raising an exception. This allows greater efficiency, since
 *		TRY is generally expensive. Anyway, apply the process of
 *		evolution: adapt or die.
 *	056	Dave Butenhof	12 March 1992
 *		Modify fork() wrapper reinit functions to lock/unlock all
 *		known attributes objects rather than only the default:
 *		leaving any unlocked would allow some other thread a chance
 *		to acquire one during prefork processing, leaving the
 *		attributes object locked in the child.
 *	057	Webb Scales	4 June 1992
 *		Fix problem in which attr obj with pending delete set is 
 *		prematurely placed on the cache.
 *	058	Webb Scales	12 June 1992
 *		Removed no longer needed variable from free-attr
 *	059	Dave Butenhof	02 September 1992
 *		Having decided that caching CV & mutex objects (which are
 *		cheap to initialize) on attributes objects is less efficient
 *		than just getting them from memory, remove the code in this
 *		module that deals with CV & mutex caching.
 *	060	Dave Butenhof	10 September 1992
 *		Generalize the check for unsupported POSIX scheduling
 *		functions, using _CMA_POSIX_SCHED_ rather than assuming Mach
 *		thread systems don't support it.
 *	061	Dave Butenhof	15 September 1992
 *		Move sequence number into known_atts object to remove extra
 *		lock cycle.
 *	062	Webb Scales	23 September 1992
 *		Add the "ada-rtb" scheduling policy.
 *	063	Dave Butenhof	 7 December 1992
 *		Allow priorities above max for FIFO & RR on RT4_KTHREAD.
 *	064	Dave Butenhof	 3 March 1993
 *		Integrate review results. Improve comments, rearrange some
 *		code for simplicity and performance.
 *	065	Dave Butenhof	13 April 1993
 *		Update queue macro usage to allow VAX builtins.
 *	066	Dave Butenhof	12 May 1993
 *		Catch up with other objects in user name creation.
 *	067	Dave Butenhof	 1 July 1993
 *		Zero default attr. obj queue element before inserting on
 *		queue.
 */


/*
 *  INCLUDE FILES
 */

#include <cma.h>
#include <cma_defs.h>
#include <cma_attr.h>
#include <cma_errors.h>
#include <cma_handle.h>
#include <cma_init.h>
#include <cma_tcb.h>
#include <cma_mutex.h>
#include <cma_condition.h>
#include <cma_vm.h>
#include <cma_queue.h>
#include <cma_stack.h>
#include <cma_deb_core.h>

/*
 * LOCAL MACROS
 */

/*
 *  GLOBAL DATA
 */

cma__t_int_attr		cma__g_def_attr;

cma_t_priority	cma__g_pri_range[cma__c_max_policies][cma__c_num_pris] =
    {
#if _CMA_RT4_KTHREAD_
    {0, 		     0,			     0}, /* Unused policy */
#endif
    {cma_c_prio_fifo_min,    cma_c_prio_fifo_mid,    cma_c_prio_fifo_max},
    {cma_c_prio_rr_min,      cma_c_prio_rr_mid,      cma_c_prio_rr_max},
    {cma_c_prio_through_min, cma_c_prio_through_mid, cma_c_prio_through_max},
    {cma_c_prio_back_min,    cma_c_prio_back_mid,    cma_c_prio_back_max},
    {cma_c_prio_ada_low_min, cma_c_prio_ada_low_mid, cma_c_prio_ada_low_max},
    {cma_c_prio_ada_rtb_min, cma_c_prio_ada_rtb_mid, cma_c_prio_ada_rtb_max},
    {0, 		     0,			     0}	/* Idle policy */
    };

/*
 *  LOCAL DATA
 */

/*
 * LOCAL FUNCTIONS
 */


/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Create a new public attributes structure.
 *
 *  FORMAL PARAMETERS:
 *
 *	new_att		Output handle
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
cma_attr_create
#ifdef _CMA_PROTO_
	(
	cma_t_attr	*new_att,	/* New handle to fill in */
	cma_t_attr	*att)		/* Old attr obj to use */

#else	/* no prototypes */
	(new_att, att)
	cma_t_attr	*new_att;	/* New handle to fill in */
	cma_t_attr	*att;		/* Old attr obj to use */
#endif	/* prototype */
    {
    cma__t_int_attr     *att_obj;
    cma__t_int_attr     *int_att;


    int_att = cma__validate_default_attr (att);
    att_obj = cma__get_attributes (int_att);

    if ((cma_t_address)att_obj == cma_c_null_ptr)
	cma__error (exc_s_insfmem);
    else {
	cma__object_to_handle ((cma__t_object *)att_obj, new_att);
	cma__obj_set_owner (att_obj, (cma_t_integer)new_att);
	cma__obj_set_name (att_obj, "<CMA user@0x%lx>");
	}

    }

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Delete (free) a public attributes structure.
 *
 *  FORMAL PARAMETERS:
 *
 *	att		Attributes object to free
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
cma_attr_delete
#ifdef _CMA_PROTO_
	(
	cma_t_attr	*att)		/* Attr obj to use */
#else	/* no prototypes */
	(att)
	cma_t_attr	*att;		/* Attr obj to use */
#endif	/* prototype */
    {
    cma__t_int_attr     *int_att;


    int_att = cma__validate_null_attr (att);

    if (int_att != (cma__t_int_attr *)cma_c_null_ptr) {
	cma__free_attributes (int_att);
	cma__clear_handle (att);
	}

    }

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Get and Set attribute routines
 *
 *  FORMAL PARAMETERS:
 *
 *	att		Attributes object to modify (set) or read (get)
 *
 *	setting		New (set) or Current (get) attribute setting
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

/*
 * Return current setting of "guard size" attribute
 */
extern void
cma_attr_get_guardsize
#ifdef _CMA_PROTO_
	(
	cma_t_attr	*att,		/* Attr obj to use */
	cma_t_natural	*setting)	/* Current setting */
#else	/* no prototypes */
	(att, setting)
	cma_t_attr	*att;		/* Attr obj to use */
	cma_t_natural	*setting;	/* Current setting */
#endif	/* prototype */
    {
    cma__int_attr_get_guardsize (att, setting);
    }

/*
 * Return current setting of "inherit scheduling" attribute
 */
extern void
cma_attr_get_inherit_sched
#ifdef _CMA_PROTO_
	(
	cma_t_attr		*att,	/* Attr obj to use */
	cma_t_sched_inherit	*setting) /* Current setting */
#else	/* no prototypes */
	(att, setting)
	cma_t_attr		*att;	/* Attr obj to use */
	cma_t_sched_inherit	*setting; /* Current setting */
#endif	/* prototype */
    {
    cma__int_attr_get_inherit_sched (att, setting);
    }

/*
 * Return current setting of mutex kind attribute
 */
extern void
cma_attr_get_mutex_kind
#ifdef _CMA_PROTO_
	(
	cma_t_attr		*att,	/* Attr obj to use */
	cma_t_mutex_kind	*setting) /* Current setting */
#else	/* no prototypes */
	(att, setting)
	cma_t_attr		*att;	/* Attr obj to use */
	cma_t_mutex_kind	*setting; /* Current setting */
#endif	/* prototype */
    {
    cma__t_int_attr	*int_att;


    int_att = cma__validate_default_attr (att);
    cma__int_lock (int_att->mutex);
    *setting = int_att->mutex_kind;
    cma__int_unlock (int_att->mutex);
    }

/*
 * Return current setting of "priority" attribute
 */
extern void
cma_attr_get_priority
#ifdef _CMA_PROTO_
	(
	cma_t_attr	*att,		/* Attr obj to use */
	cma_t_priority	*setting)	/* Current setting */
#else	/* no prototypes */
	(att, setting)
	cma_t_attr	*att;		/* Attr obj to use */
	cma_t_priority	*setting;	/* Current setting */
#endif	/* prototype */
    {
#if !_CMA_POSIX_SCHED_
    /*
     * If the implementation doesn't support POSIX.4 policy, we can't
     * support this function.
     */
    cma__error (cma_s_unimp);
#else
    cma__int_attr_get_priority (att, setting);
#endif
    }

/*
 * Return current setting of "scheduling policy" attribute
 */
extern void
cma_attr_get_sched
#ifdef _CMA_PROTO_
	(
	cma_t_attr		*att,	/* Attr obj to use */
	cma_t_sched_policy	*setting) /* Current setting */
#else	/* no prototypes */
	(att, setting)
	cma_t_attr		*att;	/* Attr obj to use */
	cma_t_sched_policy	*setting; /* Current setting */
#endif	/* prototype */
    {
#if !_CMA_POSIX_SCHED_
    /*
     * If the implementation doesn't support POSIX.4 policy, we can't
     * support this function.
     */
    cma__error (cma_s_unimp);
#else
    cma__int_attr_get_sched (att, setting);
#endif
    }

/*
 * Return current setting of "stack size" attribute
 */
extern void
cma_attr_get_stacksize
#ifdef _CMA_PROTO_
	(
	cma_t_attr	*att,		/* Attr obj to use */
	cma_t_natural	*setting)	/* Current setting */
#else	/* no prototypes */
	(att, setting)
	cma_t_attr	*att;		/* Attr obj to use */
	cma_t_natural	*setting;	/* Current setting */
#endif	/* prototype */
    {
    cma__int_attr_get_stacksize (att, setting);
    }

/*
 * Modify current setting of "guard size" attribute
 */
extern void
cma_attr_set_guardsize
#ifdef _CMA_PROTO_
	(
	cma_t_attr	*att,		/* Attr obj to use */
	cma_t_natural	setting)	/* New setting */
#else	/* no prototypes */
	(att, setting)
	cma_t_attr	*att;		/* Attr obj to use */
	cma_t_natural	setting;	/* New setting */
#endif	/* prototype */
    {
    cma__int_attr_set_guardsize (att, setting);
    }

/*
 * Modify current setting of "inherit scheduling" attribute
 */
extern void
cma_attr_set_inherit_sched
#ifdef _CMA_PROTO_
	(
	cma_t_attr		*att,	/* Attr obj to use */
	cma_t_sched_inherit	setting) /* New setting */
#else	/* no prototypes */
	(att, setting)
	cma_t_attr		*att;	/* Attr obj to use */
	cma_t_sched_inherit	setting; /* New setting */
#endif	/* prototype */
    {
    cma__t_int_attr	*int_att;


    int_att = cma__validate_attr (att);

    if ((setting != cma_c_sched_inherit)
	    && (setting != cma_c_sched_use_default))
	cma__error (cma_s_badparam);

    cma__int_lock (int_att->mutex);

    if (setting == cma_c_sched_inherit)
	int_att->inherit_sched = cma_c_true;
    else
	int_att->inherit_sched = cma_c_false;

    cma__int_unlock (int_att->mutex);
    }

/*
 * Modify current setting of mutex kind attribute
 */
extern void
cma_attr_set_mutex_kind
#ifdef _CMA_PROTO_
	(
	cma_t_attr		*att,	/* Attr obj to use */
	cma_t_mutex_kind	setting)	/* New setting */
#else	/* no prototypes */
	(att, setting)
	cma_t_attr		*att;	/* Attr obj to use */
	cma_t_mutex_kind	setting;	/* New setting */
#endif	/* prototype */
    {
    cma__t_int_attr	*int_att;


    int_att = cma__validate_attr (att);

    if ((setting != cma_c_mutex_fast)
	    && (setting != cma_c_mutex_recursive)
	    && (setting != cma_c_mutex_nonrecursive))
	cma__error (cma_s_badparam);

    cma__int_lock (int_att->mutex);
    int_att->mutex_kind = setting;
    cma__int_unlock (int_att->mutex);
    }

/*
 * Modify current setting of "priority" attribute
 */
extern void
cma_attr_set_priority
#ifdef _CMA_PROTO_
	(
	cma_t_attr	*att,		/* Attr obj to use */
	cma_t_priority	setting)	/* New setting */
#else	/* no prototypes */
	(att, setting)
	cma_t_attr	*att;		/* Attr obj to use */
	cma_t_priority	setting;	/* New setting */
#endif	/* prototype */
    {
#if !_CMA_POSIX_SCHED_
    /*
     * If the implementation doesn't support POSIX.4 policy, we can't
     * support this function.
     */
    cma__error (cma_s_unimp);
#else
    cma__t_int_attr	*int_att;


    int_att = cma__validate_attr (att);
    cma__int_lock (int_att->mutex);

    /*
     * Check the range of the priority value against the allowed range for
     * the specified policy. Note that on the DEC OSF/1 realtime kernel, we
     * allow realtime policies (RR & FIFO) to set up to the maximum kernel
     * priority (63) even though that's above the DECthreads max constant.
     */
    if (setting >= cma__g_pri_range[int_att->policy][0]
	    && (setting <= cma__g_pri_range[int_att->policy][2]
#if _CMA_RT4_KTHREAD_
	    || (int_att->policy <= cma_c_sched_rr && setting <= 63)
#endif
	    )) {
	int_att->priority = setting;
	cma__int_unlock (int_att->mutex);
	}
    else {
	cma__int_unlock (int_att->mutex);
	cma__error (cma_s_badparam);
	}

#endif
    }

/*
 * Modify current setting of "scheduling policy" attribute
 */
extern void
cma_attr_set_sched
#ifdef _CMA_PROTO_
	(
	cma_t_attr		*att,	/* Attr obj to use */
	cma_t_sched_policy	setting,	/* New policy */
	cma_t_priority		priority)	/* New priority */
#else	/* no prototypes */
	(att, setting, priority)
	cma_t_attr		*att;	/* Attr obj to use */
	cma_t_sched_policy	setting;	/* New policy */
	cma_t_priority		priority;	/* New priority */
#endif	/* prototype */
    {
#if !_CMA_POSIX_SCHED_
    /*
     * If the implementation doesn't support POSIX.4 policy, we can't
     * support this function.
     */
    cma__error (cma_s_unimp);
#else
    cma__t_int_attr	*int_att;


    int_att = cma__validate_attr (att);

    /*
     * Check the range of the priority value against the allowed range for
     * the specified policy. Note that on the DEC OSF/1 realtime kernel, we
     * allow realtime policies (RR & FIFO) to set up to the maximum kernel
     * priority (63) even though that's above the DECthreads max constant.
     */
    if (
	    setting >= cma_c_sched_fifo && setting <= cma_c_sched_idle
	    && priority >= cma__g_pri_range[setting][0]
	    && (priority <= cma__g_pri_range[setting][2]
#if _CMA_RT4_KTHREAD_
	    || (setting <= cma_c_sched_rr && priority <= 63)
#endif
	    )) {
	cma__int_lock (int_att->mutex);
	int_att->policy = setting;
	int_att->priority = priority;
	cma__int_unlock (int_att->mutex);
	}
    else
	cma__error (cma_s_badparam);

#endif
    }

/*
 * Modify current setting of "stack size" attribute
 */
extern void
cma_attr_set_stacksize
#ifdef _CMA_PROTO_
	(
	cma_t_attr	*att,		/* Attr obj to use */
	cma_t_natural	setting)	/* Current setting */
#else	/* no prototypes */
	(att, setting)
	cma_t_attr	*att;		/* Attr obj to use */
	cma_t_natural	setting;	/* Current setting */
#endif	/* prototype */
    {
    cma__int_attr_set_stacksize (att, setting);
    }

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Deallocate an attributes structure---don't try to cache it (this is
 *	used to remove attributes from a cache list!)
 *
 *  FORMAL PARAMETERS:
 *
 *	old_attr	Address of the object
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
cma__destroy_attributes
#ifdef _CMA_PROTO_
	(
	cma__t_int_attr	*old_attr)	/* The attr obj to delete */
#else	/* no prototypes */
	(old_attr)
	cma__t_int_attr	*old_attr;	/* The attr obj to delete */
#endif	/* prototype */
    {
    /*
     * Free all the objects which are contained in the attributes obj, and
     * then deallocate the object's memory.  THIS ROUTINE ASSUMES THAT THE
     * PARENT ATTRIBUTES OBJECT IS LOCKED, AND THAT THE CALLER MANAGES THE
     * CACHE LINKS.
     */
    cma__assert_warn (
	    cma__int_mutex_locked (old_attr->attributes->mutex),
	    "cma__destroy_attr called without attributes object locked.");

    cma__int_lock (old_attr->mutex);

    /*
     * If the attributes object has some objects still alive, we can't
     * arbitrarily free the memory (which could have dire consequences when
     * those objects later attempt to lock it and cache themselves).
     * Instead, just mark it as "delete pending", and wait for the objects to
     * go away (the last one will turn the light off).
     */
    if (old_attr->refcnt == 0) {
	cma__free_cache (old_attr, cma__c_obj_attr);
	cma__free_cache (old_attr, cma__c_obj_tcb);
	cma__int_unlock (old_attr->mutex);
	cma__free_mutex (old_attr->mutex);
	cma__free_mem ((cma_t_address)old_attr);
	}
    else {
	old_attr->delete_pending = cma_c_true;
	cma__int_unlock (old_attr->mutex);
	}

    }

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Free an attributes object to the cache list
 *
 *  FORMAL PARAMETERS:
 *
 *	attrib	Attributes object to free
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
cma__free_attributes
#ifdef _CMA_PROTO_
	(
	cma__t_int_attr	*attrib)	/* Attributes object to free */
#else	/* no prototypes */
	(attrib)
	cma__t_int_attr	*attrib;	/* Attributes object to free */
#endif	/* prototype */
    {
    cma__t_int_attr	*parent;


    parent = attrib->attributes;
    cma__int_lock (parent->mutex);
    cma__int_lock (attrib->mutex);

    /*
     * Check this under the mutex
     */
    cma__assert_fail (
	    attrib->refcnt >= 0,
	    "cma__free_attributes: refcnt is less than zero");

    /*
     * This will help prevent this object from being used in subsequent
     * creations -- zero indicates an invalid object.
     */
    attrib->header.sequence = 0;

    /*
     * If there are still referants to this attributes object, then
     * don't do anything more.
     */
    if (attrib->refcnt > 0) {
	attrib->delete_pending = cma_c_true;
	cma__int_unlock (attrib->mutex);
	cma__int_unlock (parent->mutex);
	return;
	}

    /*
     * Remove it from known attributes objects list
     */
    cma__int_lock (cma__g_known_atts.mutex);
    cma__queue_remove (&attrib->header.queue, attrib, cma__t_int_attr);
    cma__int_unlock (cma__g_known_atts.mutex);

    /*
     * If the attributes settings are still compatible with this object to be
     * cached and if the parent attributes object is not being deleted, then
     * add this object to the cache.
     */
    if ((attrib->header.revision == parent->cache[cma__c_obj_attr].revision)
	    && (!parent->delete_pending)) {

	if (parent->cache[cma__c_obj_attr].count
		< cma__g_env[cma__c_env_maxattr].value) {
	    parent->cache[cma__c_obj_attr].count += 1;
	    cma__queue_insert (
		    &attrib->header.queue,
		    &parent->cache[cma__c_obj_attr].queue);
	    cma__int_unlock (attrib->mutex);
	    }
	else {
	    cma__int_unlock (attrib->mutex);
	    cma__destroy_attributes (attrib);

	    while ((parent->cache[cma__c_obj_attr].count
		    > cma__g_env[cma__c_env_minattr].value)
		    && (!cma__queue_empty (
			    &parent->cache[cma__c_obj_attr].queue))) {
		cma__t_int_attr	*item;


		cma__queue_dequeue (
			&parent->cache[cma__c_obj_attr].queue,
			item,
			cma__t_int_attr);
		parent->cache[cma__c_obj_attr].count -= 1;
		cma__destroy_attributes (item);
		}

	    }

	}
    /*
     * Either the parent attributes object is being deleted, or this object
     * has the wrong characteristics to be cached, so destroy it.
     */
    else {
	cma__int_unlock (attrib->mutex);
	cma__destroy_attributes (attrib);
	}

    /*
     * There is now one less reference to the parent, so decrement its 
     * reference count.
     */
    parent->refcnt--;

    /*
     * If parent attributes object is marked for deletion, and the object we
     * just deleted was the last one using it, then delete the parent also.
     */
    if ((parent->refcnt == 0) && (parent->delete_pending)) {
	cma__int_unlock (parent->mutex);
	cma__free_attributes (parent);
	}
    else
	cma__int_unlock (parent->mutex);

    }

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Free all the objects which are hung off an attributes object's cache
 *	list.  This must be done when an attributes object structure is
 *	deleted or reused (since the cached items won't be valid).
 *
 *	THIS ROUTINE ASSUMES THAT THE ATTRIBUTES OBJECT MUTEX IS LOCKED!
 *
 *  FORMAL PARAMETERS:
 *
 *	head	Address of queue head for list
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
cma__free_cache
#ifdef _CMA_PROTO_
	(
	cma__t_int_attr	*att,
	cma_t_natural	type)
#else	/* no prototypes */
	(att, type)
	cma__t_int_attr	*att;
	cma_t_natural	type;
#endif	/* prototype */
    {
    cma__assert_warn (
	    cma__int_mutex_locked (att->mutex),
	    "cma__free_cache called without attributes object locked.");

    if (cma__queue_empty (&att->cache[type].queue))
	return;				/* Just return if queue is empty */

    while (!cma__queue_empty (&att->cache[type].queue))
	{
	cma__t_object	*item;

	cma__queue_dequeue (
		&att->cache[type].queue,
		item,
		cma__t_object);

	switch (item->type) {
	    case cma__c_obj_attr : {
		cma__destroy_attributes ((cma__t_int_attr *)item);
		break;
		}
	    case cma__c_obj_tcb : {
		cma__destroy_tcb ((cma__t_int_tcb *)item);
		break;
		}
	    default :
		cma__assert_fail (
			0,
			"Bad type code in object at cma__free_cache.");
	    }

	}

    att->cache[type].count = 0;		/* No more cached items */
    }

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Allocate an attributes object.
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
 *	Address of new attributes object
 *
 *  SIDE EFFECTS:
 *
 *	none
 */
extern cma__t_int_attr *
cma__get_attributes
#ifdef _CMA_PROTO_
	(
	cma__t_int_attr	*attrib)	/* Attributes object to use */
#else	/* no prototypes */
	(attrib)
	cma__t_int_attr	*attrib;	/* Attributes object to use */
#endif	/* prototype */
    {
    cma_t_boolean	new;		    /* true if we need to create attr */
    cma__t_int_attr	*new_attributes;    /* Pointer to new attributes */


    cma__int_lock (attrib->mutex);

    if (attrib->delete_pending) {
	cma__int_unlock (attrib->mutex);
	return (cma__t_int_attr *)cma_c_null_ptr;
	}

    /*
     * Get a pre-owned attributes object if one is available, otherwise create
     * a fresh one.
     */
    if (new = cma__queue_empty (&attrib->cache[cma__c_obj_attr].queue)) {
	new_attributes = cma__alloc_object (cma__t_int_attr);

	if ((cma_t_address)new_attributes == cma_c_null_ptr) {
	    cma__int_unlock (attrib->mutex);
	    return (cma__t_int_attr *)cma_c_null_ptr;
	    }

	new_attributes->mutex = cma__get_mutex (&cma__g_def_attr);

	if ((cma_t_address)new_attributes->mutex == cma_c_null_ptr) {
	    cma__int_unlock (attrib->mutex);
	    return (cma__t_int_attr *)cma_c_null_ptr;
	    }

	cma__obj_set_name (new_attributes->mutex, "for attr obj %d");
	}
    else {
	cma__queue_dequeue (
		&attrib->cache[cma__c_obj_attr].queue,
		new_attributes,
		cma__t_int_attr);
	attrib->cache[cma__c_obj_attr].count -= 1;
	}

    attrib->refcnt++;			/* Count reference to "parent" */
    cma__int_unlock (attrib->mutex);

    /*
     * Whether the attributes object is old or new, we need to ensure certain
     * field initialization. The most important step is generating a unique
     * sequence number for this new incharnation of the attributes object.
     */
    new_attributes->header.revision = attrib->cache[cma__c_obj_attr].revision;
    cma__obj_clear_name (new_attributes);
    new_attributes->refcnt = 0;
    new_attributes->delete_pending = cma_c_false;
    new_attributes->priority = cma_c_prio_through_mid;
    new_attributes->policy = cma_c_sched_throughput;
    new_attributes->inherit_sched = cma_c_true;

    /*
     * Since the default attributes object can't be changed, it's a good
     * place to find the "official" stack manager's opinion on the default
     * sizes of the stack and guard area. This increases the autonomy of the
     * stack manager, and also improves efficiency (marginally) since the
     * alternative would be a couple of round-up calculations.
     */
    new_attributes->stack_size = cma__g_def_attr.stack_size;
    new_attributes->guard_size = cma__g_def_attr.guard_size;
    new_attributes->mutex_kind = cma_c_mutex_fast;

    if (new) {
	cma_t_integer	i;


	/*
	 * Initialize the appropriate fields (including creating the mutex)
	 */
	new_attributes->header.type = cma__c_obj_attr;
	new_attributes->attributes = attrib;

	for (i = 1; i < cma__c_obj_num; i++)
	    {
	    new_attributes->cache[i].revision = 0;
	    new_attributes->cache[i].count = 0;
	    cma__queue_init (&new_attributes->cache[i].queue);
	    }

	}
    else {
	cma_t_integer	i;

	/*
	 * If we found a quality pre-owned attributes object, clean it up for
	 * resale (note that we do this when we allocate it, not when we free
	 * it: that way free_attributes pays minimal overhead for caching).
	 *
	 * Primarily, this involves deallocating all objects on the cache
	 * list (since they presumably won't be valid in the attributes
	 * object's new incarnation). Also, increment the revision count of
	 * each cache queue, to be sure that no objects created on the old
	 * attributes object can be accidentally cached on the new
	 * incarnation.
	 */
	cma__int_lock (new_attributes->mutex);

	for (i = 1; i < cma__c_obj_num; i++)
	    {
	    new_attributes->cache[i].revision++;
	    cma__free_cache (new_attributes, i);
	    }

	cma__int_unlock (new_attributes->mutex);
	}

    cma__int_lock (cma__g_known_atts.mutex);
    new_attributes->header.sequence = cma__g_known_atts.sequence++;
    cma__obj_set_owner (new_attributes->mutex, new_attributes->header.sequence);
    cma__queue_insert (&new_attributes->header.queue, &cma__g_known_atts.queue);
    cma__int_unlock (cma__g_known_atts.mutex);
    return new_attributes;
    }

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Initialize CMA_ATTRIBUTES.C local data
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
cma__init_attr
#ifdef _CMA_PROTO_
	(void)
#else	/* no prototypes */
	()
#endif	/* prototype */
    {
    cma_t_integer	i;


    cma__g_def_attr.header.type		= cma__c_obj_attr;
    cma__g_def_attr.header.sequence	= 1;
    cma__obj_set_name (&cma__g_def_attr, "default attr");
    cma__g_def_attr.priority		= cma_c_prio_through_mid;
    cma__g_def_attr.policy		= cma_c_sched_throughput;
    cma__g_def_attr.inherit_sched	= cma_c_true;
    cma__g_def_attr.mutex_kind		= cma_c_mutex_fast;
    cma__g_def_attr.refcnt		= 0;
    cma__g_def_attr.delete_pending	= cma_c_false;

    for (i = 1; i < cma__c_obj_num; i++)
	{
	cma__g_def_attr.cache[i].revision = 0;
	cma__g_def_attr.cache[i].count = 0;
	cma__queue_init (&cma__g_def_attr.cache[i].queue);
	}

    cma__g_def_attr.mutex	= cma__get_first_mutex (&cma__g_def_attr);
    cma__obj_set_name (cma__g_def_attr.mutex, "default attr's mutex");
    cma__g_known_atts.sequence	= 2;
    cma__g_known_atts.mutex	= cma__get_first_mutex (&cma__g_def_attr);
    cma__obj_set_name (cma__g_known_atts.mutex, "known attr list");
    cma__queue_zero (&cma__g_def_attr.header.queue);
    cma__queue_insert (&cma__g_def_attr.header.queue, &cma__g_known_atts.queue);
    }

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Perform work both prior to, and after, a fork depending
 *	upon flag passed in.
 *
 *  FORMAL PARAMETERS:
 *
 *	flag - indicating which work to perform.
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
cma__reinit_attr
#ifdef _CMA_PROTO_
	(
	cma_t_integer	    flag)
#else	/* no prototypes */
	(flag)
	cma_t_integer	    flag;
#endif	/* prototype */
    {
    cma__t_int_attr	*next;
    cma__t_queue	*ptr;


    if (flag == cma__c_reinit_prefork_lock) {
	cma__int_lock(cma__g_known_atts.mutex);
	ptr = cma__queue_next (&cma__g_known_atts.queue);

	while (ptr != &cma__g_known_atts.queue) {
	    next = (cma__t_int_attr *)ptr;
	    cma__int_lock (next->mutex);
	    ptr = cma__queue_next (ptr);
	    }

	}
    else if (flag == cma__c_reinit_postfork_unlock) {	
	ptr = cma__queue_next (&cma__g_known_atts.queue);

	while (ptr != &cma__g_known_atts.queue) {
	    next = (cma__t_int_attr *)ptr;
	    cma__int_unlock (next->mutex);
	    ptr = cma__queue_next (ptr);
	    }

	cma__int_unlock(cma__g_known_atts.mutex);
	}

    }

/*  VAX/DEC CMS REPLACEMENT HISTORY, Element CMA_ATTR.C */
/*  *27    2-JUL-1993 14:37:26 BUTENHOF "zero def_attr queue element" */
/*  *26   14-MAY-1993 15:54:23 BUTENHOF "Add user names" */
/*  *25   16-APR-1993 13:02:21 BUTENHOF "New queue_remove" */
/*  *24    3-MAR-1993 17:15:23 BUTENHOF "Fold in review comments" */
/*  *23    8-DEC-1992 15:14:35 BUTENHOF "Allow FIFO/RR prio about max on RT" */
/*  *22    1-DEC-1992 14:04:53 BUTENHOF "OSF/1 scheduling" */
/*  *21   24-SEP-1992 08:56:23 SCALES "Add ""ada-rtb"" scheduling policy" */
/*  *20   15-SEP-1992 13:48:53 BUTENHOF "Support Mach scheduling" */
/*  *19    2-SEP-1992 16:23:32 BUTENHOF "Change use of obsolete CV & mutex routines" */
/*  *18   12-JUN-1992 12:27:04 SCALES "remove unneeded variable from free-attr" */
/*  *17    5-JUN-1992 19:19:54 SCALES "Fix attr obj pending delete problem" */
/*  *16   13-MAR-1992 14:07:32 BUTENHOF "Lock all known attributes objects across fork" */
/*  *15   18-FEB-1992 15:27:20 BUTENHOF "Adapt to new alloc_mem protocol" */
/*  *14   23-DEC-1991 07:52:34 BUTENHOF "Init stack cache queue" */
/*  *13   20-DEC-1991 11:01:44 BUTENHOF "Set mutex kind default before creating mutex" */
/*  *12   31-OCT-1991 12:39:05 BUTENHOF "Fix cache consistency bug!" */
/*  *11   14-OCT-1991 13:37:33 BUTENHOF "Macro-ize guardsize functions" */
/*  *10   10-JUN-1991 18:17:07 SCALES "Add sccs headers for Ultrix" */
/*  *9     5-JUN-1991 16:11:15 CURTIN "fork work" */
/*  *8    29-MAY-1991 16:58:56 BUTENHOF "Disable setting priority/policy for Mach threads" */
/*  *7    24-MAY-1991 16:42:04 CURTIN "Added a new reinit routine" */
/*  *6    10-MAY-1991 11:23:48 CURTIN "Converted functions to use internal macros" */
/*  *5     2-MAY-1991 13:57:26 BUTENHOF "Add argument to cma__bugcheck() calls" */
/*  *4    13-FEB-1991 17:54:29 BUTENHOF "Change mutex attribute name symbols" */
/*  *3    12-FEB-1991 23:09:36 BUTENHOF "Change friendly to recursive" */
/*  *2    14-DEC-1990 00:54:59 BUTENHOF "Change module names" */
/*  *1    12-DEC-1990 21:41:50 BUTENHOF "Attributes" */
/*  VAX/DEC CMS REPLACEMENT HISTORY, Element CMA_ATTR.C */
