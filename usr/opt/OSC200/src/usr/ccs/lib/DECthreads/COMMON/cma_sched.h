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
 * @(#)$RCSfile: cma_sched.h,v $ $Revision: 4.2.7.2 $ (DEC) $Date: 1993/08/18 14:50:59 $
 */
/*
 *  FACILITY:
 *
 *	CMA services
 *
 *  ABSTRACT:
 *
 *	Header file for priority scheduling
 *
 *  AUTHORS:
 *
 *	Webb Scales
 *
 *  CREATION DATE:
 *
 *	15 June 1990
 *
 *  MODIFICATION HISTORY:
 *
 *	001	Webb Scales	18 March 1991
 *		Reworked the "update-time" macro to remove floating point ops.
 *		Added general scale-up & scale-down macros.
 *	002	Dave Butenhof	03 April 1991
 *		Move "vp" definitions to cma_vp.h.
 *	003	Dave Butenhof	18 September 1991
 *		Merge Apollo CMA5 reverse drop: Apollo cc doesn't do constant
 *		folding correctly for shifts, so switch to arithmetic.
 *	004	Webb Scales	13 march 1992
 *		Parameterize scheduling policies.
 *	005	Dave Butenhof	11 August 1992
 *		Remove unused VP globals (susp_vps and vp_count).
 *	006	Webb Scales	28 August 1992
 *		Reorganize to accomodate new cma_sched.c module.
 *	007	Dave Butenhof	08 September 1992
 *		Fix externs.
 *	008	Dave Butenhof	24 May 1993
 *		Modify manager thread control -- transfer to it
 *		responsibility for undeferring CV operations, and wake it up
 *		by explicitly scheduling it rather than signalling a CV (so
 *		it can be done within the kernel).
 *	009	Dave Butenhof	28 May 1993
 *		Fix mgr_wake macro to work on VAX ULTRIX.
 */


#ifndef CMA_SCHED
#define CMA_SCHED

/*
 *  INCLUDE FILES
 */

#include <cma_queue.h>
#include <cma_tcb_defs.h>

/*
 * CONSTANTS AND MACROS
 */

#if !_CMA_THREAD_IS_VP_
# define cma__mgr_wake() (cma__kernel_set (&cma__g_mgr_running) \
	? (cma__kernel_unset (&cma__g_mgr_wake),0) \
	: (cma__ready (cma__g_mgr_tcb, cma_c_false),1))
#endif

/*
 *  GLOBAL DATA
 */

/*
 * Minimuma and maximum prioirities, for foreground and background threads,
 * as of the last time the scheduler ran.  (Scaled once.)
 */
extern cma_t_integer	cma__g_prio_fg_min;
extern cma_t_integer	cma__g_prio_fg_max;
extern cma_t_integer	cma__g_prio_bg_min;
extern cma_t_integer	cma__g_prio_bg_max;

/*
 * The "m" values are the slopes of the four sections of linear approximation.
 *
 * cma__g_prio_m_I = 4*N(I)/cma__g_prio_range	    (Scaled once.)
 */
extern cma_t_integer	cma__g_prio_m_0,
		    	cma__g_prio_m_1,
		    	cma__g_prio_m_2,
		    	cma__g_prio_m_3;

/* 
 * The "b" values are the intercepts of the four sections of linear approx.
 *  (Not scaled.)
 *
 * cma__g_prio_b_I = -N(I)*(I*prio_max + (4-I)*prio_min)/prio_range + prio_o_I
 */
extern cma_t_integer	cma__g_prio_b_0,
		    	cma__g_prio_b_1,
		    	cma__g_prio_b_2,
		    	cma__g_prio_b_3;

/* 
 * The "p" values are the end points of the four sections of linear approx.
 *
 * cma__g_prio_p_I = cma__g_prio_fg_min + (I/4)*cma__g_prio_range
 *
 * [cma__g_prio_p_0 is not defined since it is not used (also, it is the same
 *  as cma__g_prio_fg_min).]	    (Scaled once.)
 */
extern cma_t_integer	cma__g_prio_p_1,
		    	cma__g_prio_p_2,
		    	cma__g_prio_p_3;

/*
 * Points to the next queue for the dispatcher to check for ready threads.
 */
extern cma_t_integer	cma__g_next_ready_queue;

/*
 * Points to the queues of virtual processors (for preempt victim search)
 */
extern cma__t_queue	cma__g_run_vps;

/* 
 * Threads put on hold by debugger
 */
extern cma__t_queue	cma__g_hold_list;

/*
 * Manager thread objects
 */
extern cma__t_int_tcb		*cma__g_mgr_tcb;	/* Scheduling manager thread */
extern cma__t_atomic_bit	cma__g_mgr_sort_rq;
extern cma__t_atomic_bit	cma__g_mgr_running;
extern cma__t_atomic_bit	cma__g_mgr_wake;	/* Defer mgr wakeup */

/*
 * INTERNAL INTERFACES
 */

/*
 * Initialize the scheduler
 */
extern void
cma__init_sched _CMA_PROTOTYPE_ ((void));

/*
 * Perform pre and/or post fork() reinitialization work.
 */
extern void
cma__reinit_sched _CMA_PROTOTYPE_ ((
	cma_t_integer	flag));

# if !_CMA_THREAD_IS_VP_
void
cma__try_run _CMA_PROTOTYPE_ ((cma__t_int_tcb	*tcb));
# else
#  define cma__try_run(tcb)
# endif

#endif
/*  VAX/DEC CMS REPLACEMENT HISTORY, Element CMA_SCHED.H */
/*  *13   28-MAY-1993 12:17:56 BUTENHOF "Fix mgr_wake" */
/*  *12   27-MAY-1993 14:32:34 BUTENHOF "Teach mgr thd to undefer CVs" */
/*  *11    8-SEP-1992 11:00:22 BUTENHOF "Fix externs" */
/*  *10    4-SEP-1992 15:53:17 SCALES "Reorganize to accomodate new cma_sched.c module" */
/*  *9    11-AUG-1992 13:15:59 BUTENHOF "Remove vp queues" */
/*  *8    18-MAR-1992 19:01:29 SCALES "Parameterize scheduling policies" */
/*  *7    24-SEP-1991 16:27:48 BUTENHOF "Merge CMA5 reverse IBM/HP/Apollo drops" */
/*  *6    10-JUN-1991 19:55:24 SCALES "Convert to stream format for ULTRIX build" */
/*  *5    10-JUN-1991 19:21:23 BUTENHOF "Fix the sccs headers" */
/*  *4    10-JUN-1991 18:23:07 SCALES "Add sccs headers for Ultrix" */
/*  *3    12-APR-1991 23:36:32 BUTENHOF "Implement VP layer to support Mach threads" */
/*  *2    28-MAR-1991 17:22:30 SCALES "Improve variable priority dispatch" */
/*  *1    12-DEC-1990 21:51:58 BUTENHOF "Scheduling" */
/*  VAX/DEC CMS REPLACEMENT HISTORY, Element CMA_SCHED.H */
