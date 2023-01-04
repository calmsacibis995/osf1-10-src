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
 *	@(#)$RCSfile: cma_timer.h,v $ $Revision: 4.2.5.2 $ (DEC) $Date: 1993/04/13 21:34:33 $
 */
/*
 *  FACILITY:
 *
 *	CMA services
 *
 *  ABSTRACT:
 *
 *	Header file for timer functions
 *
 *  AUTHORS:
 *
 *	Hans Oser
 *	Dave Butenhof
 *
 *  CREATION DATE:
 *
 *	5 September 1989
 *
 *  MODIFICATION HISTORY:
 *
 *	001	Hans Oser		    10 October 1989
 *		cma__wait, cma__delay, cma__timed_wait added;
 *		All timer support moved from cma_dispatch into
 *		this module.
 *	002	Hans Oser		    13 October 1989
 *		Changements due to program review
 *	003	Hans Oser		    20 October 1989
 *		Changements for ULTRIX non_blocking I/O
 *	004	Hans Oser		    27 October 1989
 *		cma__g_timer_lock made global
 *	005	Bob Conti		5 November 1989
 *		Renamed time_slice constant to quanta, and
 *		time_slice field to quanta_remaining.
 *		Rename "actual" to "current" everywhere.
 *	006	Dave Butenhof		8 November 1989
 *		Major changes to use system time and simplify algorithms.
 *	007	Dave Butenhof	4 December 1989
 *		Include cma_tcb_defs.h instead of cma_tcb.h
 *	008	Dave Butenhof	5 March 1990
 *		Integrate Kevin Ackley's changes for Apollo M68000 port.
 *	009	Dave Butenhof & Webb Scales	29 March 1990
 *		Add cell for next-timer-queue entry time (for null thread
 *		select).  Make timeslice signal value "global".
 *	010	Webb Scales	4 June 1990
 *		Make timeslice routine available to other modules (cma_signal).
 *	011	Webb Scales	15 June 1990
 *		Added priority scheduling:  added a global ticks counter for
 *		absolute time, and a macro for fetching the value.
 *	012	Webb Scales	16 August 1990
 *		Added include of signal.h for timeslicer declaration on Unix
 *	013	Dave Butenhof & Webb Scales	 4 December 1990
 *		Added proto for cma__check_timer_queue
 *	014	Paul Curtin	31 May 1991
 *		Added a proto for fork() reinitialization routine.
 *	015	Dave Butenhof	18 September 1991
 *		Integrate Apollo CMA5 reverse drop: remove Apollo special
 *		cases for quantum and timer interval.
 *	016	Dave Butenhof	22 November 1991
 *		Remove include of string.h, which isn't needed.
 *	017	Webb Scales	28 January 1992
 *		Update the prototype for cma-periodic on VMS.
 *	018	Webb Scales	30 January 1992
 *		Un-do changes to the cma-periodic prototype.
 *	019	Webb Scales	11 February 1992
 *		Re-do the prototype for cma-periodic on VMS.
 *	020	Dave Butenhof	10 March 1992
 *		Change timeb field references to timeval, since
 *		cma_t_date_time has been changed to avoid requiring libbsd.a
 *		on AIX and OSF/1.
 *	021	Dave Butenhof	24 September 1992
 *		Add entry point to disable timers.
 *	022	Paul Curtin	06 January 1992
 *		Add cma__time_leq macro.
 */

#ifndef CMA_TIMER
#define CMA_TIMER

/*
 *  INCLUDE FILES
 */
#include <cma.h>
#if _CMA_OS_ == _CMA__UNIX
# include <signal.h>
#endif
#include <cma_tcb_defs.h>

/*
 * CONSTANTS AND MACROS
 */

#define cma__c_wait_timer	1
#define cma__c_normal_timer	2
#define cma__c_select_timer     3

#if _CMA_OS_ == _CMA__UNIX
#define cma__c_timer		    ITIMER_VIRTUAL
#define cma__c_timer_signal	    SIGVTALRM
#endif

#define cma__c_quanta		2	/* default quanta for time slicing */

#define cma__get_time_ticks()	cma__g_time_ticks

/*
 * Macro to copy one system time value to another.
 */
#if _CMA_OS_ == _CMA__VMS
#define cma__copy_time(output,input)					\
    ((output)->high = (input)->high, (output)->low = (input)->low)

#define cma__time_lss(t1,t2)						\
    (((t1)->high < (t2)->high) || (((t1)->high == (t2)->high)		\
	&& ((t1)->low < (t2)->low)))

#define cma__time_leq(t1,t2)						\
    (((t1)->high < (t2)->high) || (((t1)->high == (t2)->high)           \
        && ((t1)->low < (t2)->low)) || (((t1)->high == (t2)->high)	\
	&& ((t1)->low == (t2)->low)))
#else
#define cma__copy_time(output,input)					\
    ((output)->tv_sec = (input)->tv_sec, (output)->tv_usec = (input)->tv_usec)

#define cma__time_lss(t1,t2)						\
    (((t1)->tv_sec < (t2)->tv_sec) || (((t1)->tv_sec == (t2)->tv_sec)	\
	&& ((t1)->tv_usec < (t2)->tv_usec)))

#define cma__time_leq(t1,t2)						\
    (((t1)->tv_sec < (t2)->tv_sec) || (((t1)->tv_sec == (t2)->tv_sec)	\
	&& ((t1)->tv_usec < (t2)->tv_usec)) || (((t1)->tv_sec == (t2)->tv_sec) \
	&& ((t1)->tv_usec == (t2)->tv_usec)))

#endif

/*
 * TYPEDEFS
 */

/*
 *  GLOBAL DATA
 */

extern cma_t_date_time	cma__g_one_tick;
extern cma_t_date_time	cma__g_next_tqe;
extern cma_t_integer	cma__g_time_ticks;


/*
 * EXTERNAL INTERFACES
 */

/*
 * Add times
 */
extern void
cma__add_time _CMA_PROTOTYPE_ ((
	cma_t_date_time	*result,
	cma_t_date_time	*time1,
	cma_t_date_time	*time2));

#if !_CMA_THREAD_IS_VP_
/*
 * ready threads on timer queue
 */
extern cma_t_boolean
cma__check_timer_queue _CMA_PROTOTYPE_ ((
	cma_t_date_time *next_time));
#endif

/*
 * Get the current time
 */
extern void
cma__get_time _CMA_PROTOTYPE_ ((		/* Get the current system time */
	cma_t_date_time	*time));

/*
 * Start the timer: this is used for thread timeslicing, timed semaphore
 * waits, and non-blocking I/O.
 */
extern void
cma__init_timer _CMA_PROTOTYPE_ ((void));

extern void
cma__disable_timer _CMA_PROTOTYPE_ ((void));

#if !_CMA_THREAD_IS_VP_
/*
 * Insert an entry (in order) on the timer queue.  If the specified time
 * preceeds the current system time, then the entry is not inserted, and the
 * value cma_c_false is returned.
 */
extern cma_t_boolean
cma__insert_timer _CMA_PROTOTYPE_ ((
	cma__t_int_tcb	*tcb,
	cma_t_date_time	*time));
#endif

/*
 * Convert a floating point interval (cma_t_interval) to a system time
 * (cma_t_date_time).
 */
extern void
cma__interval_to_time _CMA_PROTOTYPE_ ((
	cma_t_interval	interval,
	cma_t_date_time	*time));

#if !_CMA_THREAD_IS_VP_
/*
 * Timeslice routine
 */
extern void
cma__periodic
# if _CMA_OS_ == _CMA__VMS
    _CMA_PROTOTYPE_ ((void));
# endif
# if _CMA_OS_ == _CMA__UNIX
    _CMA_PROTOTYPE_ ((
	int			sig,
	int			code,
	struct sigcontext	*scp));
# endif
#endif

/*
 * Perform pre/post fork() reinitialization work
 */
extern void
cma__reinit_timer _CMA_PROTOTYPE_ ((
	cma_t_integer		flag));

/*
 * Subtract times.
 */
extern void
cma__subtract_time _CMA_PROTOTYPE_ ((		/* difference = minuend - subtrahend */
	cma_t_date_time	*difference,
	cma_t_date_time	*minuend,
	cma_t_date_time	*subtrahend));

#endif
/*  VAX/DEC CMS REPLACEMENT HISTORY, Element CMA_TIMER.H */
/*  *16    6-JAN-1993 15:40:43 CURTIN "Add cma__time_leq macro" */
/*  *15   28-SEP-1992 11:49:35 BUTENHOF "Add disable timer entry" */
/*  *14    2-SEP-1992 16:26:44 BUTENHOF "Separate semaphores from kernel lock" */
/*  *13   10-MAR-1992 16:27:49 BUTENHOF "Change timeb to timeval" */
/*  *12   11-FEB-1992 16:30:31 SCALES "Change periodic's proto" */
/*  *11   30-JAN-1992 22:13:29 SCALES "Repair async alerts for VAX/VMS" */
/*  *10   29-JAN-1992 23:49:11 SCALES "Rework end-quantum preemption for VMS" */
/*  *9    22-NOV-1991 13:32:55 BUTENHOF "Remove include of string.h" */
/*  *8    24-SEP-1991 16:29:43 BUTENHOF "Merge CMA5 reverse IBM/HP/Apollo drops" */
/*  *7    10-JUN-1991 19:57:37 SCALES "Convert to stream format for ULTRIX build" */
/*  *6    10-JUN-1991 19:22:14 BUTENHOF "Fix the sccs headers" */
/*  *5    10-JUN-1991 18:24:55 SCALES "Add sccs headers for Ultrix" */
/*  *4     2-JUN-1991 19:37:08 BUTENHOF "Stop using timers for _CMA_THREAD_IS_VP_" */
/*  *3    31-MAY-1991 16:44:43 CURTIN "Added a fork() reinitialization routine proto" */
/*  *2     2-MAY-1991 13:59:34 BUTENHOF "Export timeslice interval" */
/*  *1    12-DEC-1990 21:55:08 BUTENHOF "Timer services" */
/*  VAX/DEC CMS REPLACEMENT HISTORY, Element CMA_TIMER.H */
