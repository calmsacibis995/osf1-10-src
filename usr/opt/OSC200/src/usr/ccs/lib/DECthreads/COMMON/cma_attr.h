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
 *	@(#)$RCSfile: cma_attr.h,v $ $Revision: 4.2.5.2 $ (DEC) $Date: 1993/04/13 21:29:36 $
 */
/*
 *  FACILITY:
 *
 *	DECthreads core
 *
 *  ABSTRACT:
 *
 *	Header file for attributes objects
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
 *	001	Dave Butenhof	24 August 1989
 *		Modify attributes object to use queues instead of lists.
 *	002	Dave Butenhof	15 September 1989
 *		Add attribute revision counts for each object class;
 *		currently only TCBs and stacks have attributes, and stacks
 *		aren't cached, but we do everything, for symmetry.
 *	003	Dave Butenhof	11 October 1989
 *		Convert to use internal mutex operations.
 *	004	Dave Butenhof	19 October 1989
 *		Don't include cma_handle.h, as it now sets up circular
 *		dependencies.
 *	005	Dave Butenhof	24 October 1989
 *		Add "delete-pending" and reference count fields to attributes
 *		object to eliminate some holes in caching.
 *	006	Dave Butenhof	1 November 1989
 *		Add extern for known atts queue.
 *	007	Dave Butenhof	30 November 1989
 *		Add field for scheduling policy (not implemented yet).
 *	008	Dave Butenhof	5 December 1989
 *		Remove initial_test field (since we're not sure what it
 *		should do).
 *	009	Dave Butenhof	9 April 1990
 *		Remove definition of known attributes queue header; it's now
 *		in cma_debugger.
 *	010	Dave Butenhof	26 June 1990
 *		Add new attribute for mutex objects; to control whether lock
 *		is "friendly" (supports nested locks by same thread).
 *	011	Paul Curtin	10 May 1991
 *		Added a number of new macros.
 *	012	Paul Curtin	24 May 1991
 *		Added a prototype for cma__reinit_attr.
 *	013	Dave Butenhof and Webb Scales	05 June 1991
 *		Conditionalize vacuous (forward) structure defs, since MIPS C
 *		V2.1 doesn't like (or, apparently, need).
 *	014	Dave Butenhof	18 September 1991
 *		Integrate HP CMA5 reverse drop.
 *	015	Dave Butenhof	07 October 1991
 *		Macro-ize guardsize functions.
 *	016	Dave Butenhof	31 October 1991
 *		Move initialization of attributes object fields from the
 *		macro for cma_attr_create into the internal creation
 *		function, cma__get_attributes; that way, any (future)
 *		internal-use attr. objs. will be initialized properly.
 *	017	Dave Butenhof	30 January 1992
 *		Allow a guard size of 0 bytes, so that memory-tight
 *		applications can create smaller threads (strictly at their
 *		own risk!)
 *	018	Dave Butenhof	10 February 1992
 *		Remove attributes create macro; functions of CMA and pthread
 *		interfaces now differ slightly (CMA raises exception).
 *	019	Dave Butenhof	12 March 1992
 *		Remove attributes delete macro.
 *	020	Dave Butenhof	 2 December 1992
 *		If stacksize attr. is set to '0', round up to minimum (this
 *		is for compatibility with OSF/1 pthreads).
 *	021	Dave Butenhof	 4 February 1993
 *		Don't round up stack size.
 *	022	Dave Butenhof	 3 March 1993
 *		Integrate review comments. Don't round up guard size, fix
 *		facility, remove some excess parentheses.
 */

#ifndef CMA_ATTR
#define CMA_ATTR

/*
 *  INCLUDE FILES
 */

#include <cma_defs.h>
#include <cma_queue.h>
#ifdef __hpux
# include <sys/param.h>
#endif

/*
 * CONSTANTS AND MACROS
 */

#define cma__c_num_pris		4	/* min, mid, max, 'real max' */

/*
 * In DEC OSF/1 RT, policies start at 1 -- otherwise they start at 0
 */
#if _CMA_RT4_KTHREAD_
# define cma__c_max_policies 8
#else
# define cma__c_max_policies 7
#endif


/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	cma__int_attr_get_guardsize - Performs work of cma_attr_get_guardsize
 *
 *  FORMAL PARAMETERS:
 *
 *      cma_t_attr          *_att_      - Attributes object to use
 *	cma_t_natural	    *_setting_	- Current setting
 *
 *  IMPLICIT INPUTS:
 *
 *	none
 *
 *  IMPLICIT OUTPUTS:
 *
 *	Attribute objects guard size setting
 *
 *  FUNCTION VALUE:
 *
 *	none
 *
 *  SIDE EFFECTS:
 *
 *	none
 */
#define cma__int_attr_get_guardsize(_att_,_setting_) { \
    cma__t_int_attr     *_int_att_; \
    _int_att_ = cma__validate_default_attr (_att_); \
    cma__int_lock (_int_att_->mutex); \
    (*(_setting_)) = _int_att_->guard_size; \
    cma__int_unlock (_int_att_->mutex); \
    }

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	cma__int_attr_get_inherit_sched - Performs work of 
 *	cma_attr_get_inherit_sched
 *
 *  FORMAL PARAMETERS:
 *
 *	cma_t_attr	    *_att_	- Attributes object to use
 *	cma_t_sched_inherit *_setting_	- Current setting
 *
 *  IMPLICIT INPUTS:
 *
 *	none
 *
 *  IMPLICIT OUTPUTS:
 *
 *	Inheritable scheduling policy
 *
 *  FUNCTION VALUE:
 *
 *	none
 *
 *  SIDE EFFECTS:
 *
 *	none
 */
#define cma__int_attr_get_inherit_sched(_att_,_setting_) { \
    cma__t_int_attr	*_int_att_; \
    _int_att_ = cma__validate_default_attr (_att_); \
    cma__int_lock (_int_att_->mutex); \
    (*(_setting_)) = (_int_att_->inherit_sched ? cma_c_sched_inherit \
	: cma_c_sched_use_default); \
    cma__int_unlock (_int_att_->mutex); \
    }

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	cma__int_attr_get_priority -  Performs the work of cma_attr_get_priority
 *
 *  FORMAL PARAMETERS:
 *
 *	cma_t_attr	    *_att_	- Attribute object to get from
 *	cma_t_priority	    *_setting_	- Current setting
 *
 *  IMPLICIT INPUTS:
 *
 *	none
 *
 *  IMPLICIT OUTPUTS:
 *
 *	priority
 *
 *  FUNCTION VALUE:
 *
 *	none
 *
 *  SIDE EFFECTS:
 *
 *	none
 */
#define cma__int_attr_get_priority(_att_,_setting_) { \
    cma__t_int_attr     *_int_att_; \
    _int_att_ = cma__validate_default_attr (_att_); \
    cma__int_lock (_int_att_->mutex); \
    (*(_setting_)) = _int_att_->priority; \
    cma__int_unlock (_int_att_->mutex); \
    }


/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	cma__int_attr_get_sched - Performs work of cma_attr_get_sched
 *
 *  FORMAL PARAMETERS:
 *
 *	cma_t_attr	    *_att_	_ Attributes object used
 *	cma_t_sched_policy  *_setting_	- Current setting
 *
 *  IMPLICIT INPUTS:
 *
 *	none
 *
 *  IMPLICIT OUTPUTS:
 *
 *	scheduling policy
 *
 *  FUNCTION VALUE:
 *
 *	none
 *
 *  SIDE EFFECTS:
 *
 *	none
 */
#define cma__int_attr_get_sched(_att_,_setting_) { \
    cma__t_int_attr     *_int_att_; \
    _int_att_ = cma__validate_default_attr (_att_); \
    cma__int_lock (_int_att_->mutex); \
    (*(_setting_)) = _int_att_->policy; \
    cma__int_unlock (_int_att_->mutex); \
    }

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	cma__int_attr_get_stacksize - Performs work of cma_attr_get_stacksize
 *
 *  FORMAL PARAMETERS:
 *
 *      cma_t_attr          *_att_      - Attributes object to use
 *	cma_t_natural	    *_setting_	- Current setting
 *
 *  IMPLICIT INPUTS:
 *
 *	none
 *
 *  IMPLICIT OUTPUTS:
 *
 *	Attribute objects stack size setting
 *
 *  FUNCTION VALUE:
 *
 *	none
 *
 *  SIDE EFFECTS:
 *
 *	none
 */
#define cma__int_attr_get_stacksize(_att_,_setting_) { \
    cma__t_int_attr     *_int_att_; \
    _int_att_ = cma__validate_default_attr (_att_); \
    cma__int_lock (_int_att_->mutex); \
    (*(_setting_)) = _int_att_->stack_size; \
    cma__int_unlock (_int_att_->mutex); \
    }

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	cma__int_attr_set_guardsize - Performs work for cma_attr_set_guardsize
 *
 *  FORMAL PARAMETERS:
 *
 *      cma_t_attr          *_att_      - Attributes object to use
 *	cma_t_natural	    _setting_	- Setting
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
 *	Change attribute objects guard size setting
 */
#define cma__int_attr_set_guardsize(_att_,_setting_) { \
    cma__t_int_attr     *_int_att_; \
    if ((_setting_) < 0) cma__error (cma_s_badparam); \
    _int_att_ = cma__validate_attr (_att_); \
    cma__int_lock (_int_att_->mutex); \
    _int_att_->guard_size = _setting_; \
    cma__free_cache (_int_att_, cma__c_obj_tcb); \
    _int_att_->cache[cma__c_obj_tcb].revision++; \
    _int_att_->cache[cma__c_obj_stack].revision++; \
    cma__int_unlock (_int_att_->mutex); \
    }

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	cma__int_attr_set_stacksize - Performs work for cma_attr_set_stacksize
 *
 *  FORMAL PARAMETERS:
 *
 *      cma_t_attr          *_att_      - Attributes object to use
 *	cma_t_natural	    _setting_	- Setting
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
 *	Change attribute objects stack size setting
 */
#define cma__int_attr_set_stacksize(_att_,_setting_) { \
    cma__t_int_attr     *_int_att_; \
    if ((_setting_) < 0) cma__error (cma_s_badparam); \
    _int_att_ = cma__validate_attr (_att_); \
    cma__int_lock (_int_att_->mutex); \
    _int_att_->stack_size = (_setting_); \
    cma__free_cache (_int_att_, cma__c_obj_tcb); \
    _int_att_->cache[cma__c_obj_tcb].revision++; \
    _int_att_->cache[cma__c_obj_stack].revision++; \
    cma__int_unlock (_int_att_->mutex); \
    }

/*
 * TYPEDEFS
 */
#ifndef __STDC__
struct CMA__T_INT_MUTEX;		/* Avoid circular dependency */
#endif

typedef struct CMA__T_CACHE {
    cma_t_natural		revision;	/* Revisions */
    cma_t_natural		count;
    cma__t_queue		queue;		/* Cache headers */
    } cma__t_cache;

typedef struct CMA__T_INT_ATTR {
    cma__t_object		header;		/* Common header */
    struct CMA__T_INT_ATTR	*attributes;	/* Point to controlling attr */
    struct CMA__T_INT_MUTEX	*mutex;		/* Serialize access to object */
    cma_t_priority		priority;	/* Priority of new thread */
    cma_t_sched_policy		policy;		/* Sched policy of thread */
    cma_t_boolean		inherit_sched;	/* Is scheduling inherited? */
    cma_t_natural		stack_size;	/* Size of stack (bytes) */
    cma_t_natural		guard_size;	/* Size of guard (bytes) */
    cma_t_mutex_kind		mutex_kind;	/* Mutex kind */
    cma__t_cache		cache[cma__c_obj_num];	/* Cache information */
    cma_t_boolean		delete_pending;	/* attr. obj. is deleted */
    cma_t_natural		refcnt;	/* Number of objects using attr. obj */
    } cma__t_int_attr;

/*
 *  GLOBAL DATA
 */

extern cma__t_int_attr	cma__g_def_attr;
extern cma_t_priority	cma__g_pri_range[cma__c_max_policies][cma__c_num_pris];

/*
 * INTERNAL INTERFACES
 */

extern void
cma__destroy_attributes _CMA_PROTOTYPE_ ((	/* Deallocate (don't cache) */
	cma__t_int_attr	*old_attr));

extern void
cma__free_attributes _CMA_PROTOTYPE_ ((	/* Free an attribute obj to cache */
	cma__t_int_attr	*old_attr));

extern void
cma__free_cache _CMA_PROTOTYPE_ ((	/* Free a cache list */
        cma__t_int_attr *att,
        cma_t_natural   type));

extern cma__t_int_attr *
cma__get_attributes _CMA_PROTOTYPE_ ((	/* Get a new attributes obj */
	cma__t_int_attr	*attrib));

extern void
cma__init_attr _CMA_PROTOTYPE_ ((void));	/* Initialize attr. code */

extern void
cma__reinit_attr _CMA_PROTOTYPE_ ((	/* Reinit after fork() */
	cma_t_integer	flag));

#endif
/*  VAX/DEC CMS REPLACEMENT HISTORY, Element CMA_ATTR.H */
/*  *19    3-MAR-1993 17:15:34 BUTENHOF "Fold in review comments" */
/*  *18    4-FEB-1993 15:43:55 BUTENHOF "Ooops!" */
/*  *17    8-DEC-1992 15:14:39 BUTENHOF "Allow FIFO/RR prio about max on RT" */
/*  *16    2-DEC-1992 13:28:49 BUTENHOF "Allow stack size of 0 (round up)" */
/*  *15    1-DEC-1992 14:04:57 BUTENHOF "OSF/1 scheduling" */
/*  *14   13-MAR-1992 14:07:44 BUTENHOF "Remove int_attr_delete macro" */
/*  *13   18-FEB-1992 15:27:30 BUTENHOF "Adapt to alloc_mem changes" */
/*  *12   30-JAN-1992 11:55:35 BUTENHOF "Allow guard size of 0" */
/*  *11   31-OCT-1991 12:39:16 BUTENHOF "Move attr. obj. field init to get_attributes code" */
/*  *10   14-OCT-1991 13:37:40 BUTENHOF "Macro-ize guardsize functions" */
/*  *9    24-SEP-1991 16:26:16 BUTENHOF "Merge CMA5 reverse IBM/HP/Apollo drops" */
/*  *8    10-JUN-1991 19:50:43 SCALES "Convert to stream format for ULTRIX build" */
/*  *7    10-JUN-1991 19:20:04 BUTENHOF "Fix the sccs headers" */
/*  *6    10-JUN-1991 18:17:23 SCALES "Add sccs headers for Ultrix" */
/*  *5     5-JUN-1991 17:31:03 BUTENHOF "Conditionalize vacuous defs" */
/*  *4    24-MAY-1991 16:42:16 CURTIN "Added a new reinit routine" */
/*  *3    10-MAY-1991 11:24:16 CURTIN "Added a number of new internal macros" */
/*  *2    14-DEC-1990 00:55:06 BUTENHOF "Change module names" */
/*  *1    12-DEC-1990 21:41:53 BUTENHOF "Attributes" */
/*  VAX/DEC CMS REPLACEMENT HISTORY, Element CMA_ATTR.H */
