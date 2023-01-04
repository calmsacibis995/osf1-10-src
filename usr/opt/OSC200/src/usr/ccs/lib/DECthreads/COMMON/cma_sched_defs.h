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
 * @(#)$RCSfile: cma_sched_defs.h,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1992/12/10 18:19:30 $
 */

/*
 *  FACILITY:
 *
 *	CMA services
 *
 *  ABSTRACT:
 *
 *	Header file for priority scheduling data definitions
 *
 *  AUTHORS:
 *
 *	Webb Scales
 *
 *  CREATION DATE:
 *
 *	3 September 1992
 *
 *  MODIFICATION HISTORY:
 *
 *	001	Dave Butenhof	15 September 1992
 *		Modify cma__sched_parameterize macro for _CMA_RT4_KTHREAD_
 *		systems, since all non-realtime policies map into
 *		SCHED_OTHER.
 *	002	Webb Scales	23 September 1992
 *		Add the "ada-rtb" scheduling policy.
 *	003	Webb Scales	28 September 1992
 *		Parenthesize ready-queue offset constant macros expressions 
 *		to ensure correct evaluation; delete special Ada offset.
 *	004	Dave Butenhof	25 November 1992
 *		Remove the special case for RT4_KTHREAD -- it makes more
 *		sense to define unique values for all DECthreads policies
 *		even if some overlap at the Mach level.
 */


#ifndef CMA_SCHED_DEFS
#define CMA_SCHED_DEFS

/*
 *  INCLUDE FILES
 */


/*
 * CONSTANTS AND MACROS
 */

/*
 * Scaling factor for integer priority calculations
 */
#define cma__c_prio_scale   8

/*
 * Min. num. of ticks between self-adjustments for priority adjusting policies.
 */
#define cma__c_prio_interval	10
#define cma__c_init_cpu_time	cma__scale_up (cma__c_prio_interval)

/*
 * Number of queues in each class of queues
 */
#define cma__c_prio_n_id    1	    /* Very-low-priority class threads */
#define cma__c_prio_n_bg    8	    /* Background class threads */
#define cma__c_prio_n_0	    1	    /* Very low priority throughput quartile */
#define cma__c_prio_n_1	    2	    /* Low priority throughput quartile */
#define cma__c_prio_n_2	    3	    /* Medium priority throughput quartile */
#define cma__c_prio_n_3	    4	    /* High priority throughput quartile */
#define cma__c_prio_n_rt    1	    /* Real Time priority queues */

/*
 * Number of queues to skip (offset) to get to the queues in this section of LA
 */
#define cma__c_prio_o_id 0
#define cma__c_prio_o_bg (cma__c_prio_o_id + cma__c_prio_n_id)
#define cma__c_prio_o_0  (cma__c_prio_o_bg + cma__c_prio_n_bg)
#define cma__c_prio_o_1  (cma__c_prio_o_0  + cma__c_prio_n_0)
#define cma__c_prio_o_2  (cma__c_prio_o_1  + cma__c_prio_n_1)
#define cma__c_prio_o_3  (cma__c_prio_o_2  + cma__c_prio_n_2)
#define cma__c_prio_o_rt (cma__c_prio_o_3  + cma__c_prio_n_3)

/*
 * Total number of ready queues, for declaration purposes
 */
#define cma__c_prio_n_tot  \
	(cma__c_prio_n_id + cma__c_prio_n_bg + cma__c_prio_n_rt \
	+ cma__c_prio_n_0 + cma__c_prio_n_1 + cma__c_prio_n_2 + cma__c_prio_n_3)

/*
 * Scaled integer arithemetic
 */
#if _CMA_VENDOR_ == _CMA__APOLLO
/*
 * FIX-ME: Apollo cc 6.8 blows contant folded "<<" and ">>"
 */
# define cma__scale_up(exp)  ((exp) * 256)
# define cma__scale_dn(exp)  ((exp) / 256)
#else
# define cma__scale_up(exp)  ((exp) << cma__c_prio_scale)
# define cma__scale_dn(exp)  ((exp) >> cma__c_prio_scale)
#endif

/*
 * Formulae for determining a thread's priority.  Variable priorities (such
 * as foreground and background) are scaled values.
 */
#define cma__sched_priority(tcb)	\
    ((tcb)->sched.class == cma__c_class_fore  ? cma__sched_prio_fore (tcb)  \
    :((tcb)->sched.class == cma__c_class_back ? cma__sched_prio_back (tcb)  \
    :((tcb)->sched.class == cma__c_class_rt   ? cma__sched_prio_rt (tcb)    \
    :((tcb)->sched.class == cma__c_class_idle ? cma__sched_prio_idle (tcb)  \
    :(cma__bugcheck ("cma__sched_priority: unrecognized class"), 0) ))))

#define cma__sched_prio_fore(tcb)	cma__sched_prio_fore_var (tcb)
#define cma__sched_prio_back(tcb)	((tcb)->sched.fixed_prio	\
	? cma__sched_prio_back_fix (tcb) : cma__sched_prio_back_var (tcb) )
#define cma__sched_prio_rt(tcb)		((tcb)->sched.priority)
#define cma__sched_prio_idle(tcb)	((tcb)->sched.priority)

#define cma__sched_prio_back_fix(tcb)	\
	(cma__g_prio_bg_min + (cma__g_prio_bg_max - cma__g_prio_bg_min) \
	* (tcb)->sched.priority / cma__c_prio_n_bg)

/*
 * FIX-ME: Enable after modeling (if we like it)
 */
#if 1
# define cma__sched_prio_fore_var(tcb)  \
	((cma__g_prio_fg_max + cma__g_prio_fg_min)/2)
# define cma__sched_prio_back_var(tcb)  \
	((cma__g_prio_bg_max + cma__g_prio_bg_min)/2)
#else
# define cma__sched_prio_back_var(tcb)  cma__sched_prio_fore_var (tcb)

# if 1
/*
 * Re-scale, since the division removes the scale factor.
 * Scale and multiply before dividing to avoid loss of precision.
 */
#  define cma__sched_prio_fore_var(tcb)  \
	((cma__g_vp_count * cma__scale_up((tcb)->sched.tot_time)) \
	/ (tcb)->sched.cpu_time)
# else
/*
 * Re-scale, since the division removes the scale factor.
 * Scale and multiply before dividing to avoid loss of precision.
 * Left shift the numerator to multiply by two.
 */
#  define cma__sched_prio_fore_var(tcb)  \
    (((cma__g_vp_count * cma__scale_up((tcb)->sched.tot_time)  \
    * (tcb)->sched.priority * cma__g_init_frac_sum) << 1)  \
    / ((tcb)->sched.cpu_time * (tcb)->sched.priority * cma__g_init_frac_sum  \
	+ (tcb)->sched.tot_time))
# endif
#endif

/*
 * Update weighted-averaged, scaled tick counters
 */
#define cma__sched_update_time(ave, new) \
    (ave) = (ave) - ((cma__scale_dn((ave)) - (new)) << (cma__c_prio_scale - 4))

#define cma__sched_parameterize(tcb, policy) { \
    switch (policy) { \
	case cma_c_sched_fifo : { \
	    (tcb)->sched.rtb =		cma_c_true; \
	    (tcb)->sched.spp =		cma_c_true; \
	    (tcb)->sched.fixed_prio =	cma_c_true; \
	    (tcb)->sched.class =	cma__c_class_rt; \
	    break; \
	    } \
	case cma_c_sched_rr : { \
	    (tcb)->sched.rtb =		cma_c_false; \
	    (tcb)->sched.spp =		cma_c_true; \
	    (tcb)->sched.fixed_prio =	cma_c_true; \
	    (tcb)->sched.class =	cma__c_class_rt; \
	    break; \
	    } \
	case cma_c_sched_throughput : { \
	    (tcb)->sched.rtb =		cma_c_false; \
	    (tcb)->sched.spp =		cma_c_false; \
	    (tcb)->sched.fixed_prio =	cma_c_false; \
	    (tcb)->sched.class =	cma__c_class_fore; \
	    break; \
	    } \
	case cma_c_sched_background : { \
	    (tcb)->sched.rtb =		cma_c_false; \
	    (tcb)->sched.spp =		cma_c_false; \
	    (tcb)->sched.fixed_prio =	cma_c_false; \
	    (tcb)->sched.class =	cma__c_class_back; \
	    break; \
	    } \
	case cma_c_sched_ada_low : { \
	    (tcb)->sched.rtb =		cma_c_false; \
	    (tcb)->sched.spp =		cma_c_true; \
	    (tcb)->sched.fixed_prio =	cma_c_true; \
	    (tcb)->sched.class =	cma__c_class_back; \
	    break; \
	    } \
	case cma_c_sched_ada_rtb : { \
	    (tcb)->sched.rtb =		cma_c_true; \
	    (tcb)->sched.spp =		cma_c_true; \
	    (tcb)->sched.fixed_prio =	cma_c_true; \
	    (tcb)->sched.class =	cma__c_class_back; \
	    break; \
	    } \
	case cma_c_sched_idle : { \
	    (tcb)->sched.rtb =		cma_c_false; \
	    (tcb)->sched.spp =		cma_c_false; \
	    (tcb)->sched.fixed_prio =	cma_c_false; \
	    (tcb)->sched.class =	cma__c_class_idle; \
	    break; \
	    } \
	default : { \
	    cma__bugcheck ("cma__sched_parameterize: bad scheduling policy"); \
	    break; \
	    } \
	} \
    }

/*
 * TYPEDEFS
 */

/*
 * Scheduling classes
 */
typedef enum CMA__T_SCHED_CLASS {
    cma__c_class_rt,
    cma__c_class_fore,
    cma__c_class_back,
    cma__c_class_idle
    } cma__t_sched_class;

#endif
/*  VAX/DEC CMS REPLACEMENT HISTORY, Element CMA_SCHED_DEFS.H */
/*  *6     1-DEC-1992 14:05:49 BUTENHOF "OSF/1 RT" */
/*  *5    28-SEP-1992 16:11:18 SCALES "Parenthesize macro constant expressions" */
/*  *4    24-SEP-1992 08:56:56 SCALES "Add ""ada-rtb"" scheduling policy" */
/*  *3    15-SEP-1992 13:50:21 BUTENHOF "Fix sched_parameterize for RT4_KTHREAD" */
/*  *2    11-SEP-1992 04:55:52 BUTENHOF "Convert to stream format for ULTRIX build" */
/*  *1     3-SEP-1992 21:29:45 SCALES "Scheduler data definitions" */
/*  VAX/DEC CMS REPLACEMENT HISTORY, Element CMA_SCHED_DEFS.H */
