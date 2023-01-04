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
 *	@(#)$RCSfile: cma_vp.h,v $ $Revision: 4.2.8.2 $ (DEC) $Date: 1993/08/18 14:55:48 $
 */
/*
 *  FACILITY:
 *
 *	DECthreads core
 *
 *  ABSTRACT:
 *
 *	Header file for virtual processor services
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
 *	001	Dave Butenhof	25 April 1991
 *		Add "vp_lock" and "vp_unlock" functions to emulate atomic
 *		test-and-set. (later removed!!!)
 *	002	Dave Butenhof	31 May 1991
 *		Add cma__vp_delay() function to do VP-synchronous delay
 *		without relying on timer interrupts.
 *	003	Dave Butenhof	05 September 1991
 *		Add prototype for new function cma__vp_yield() to use the
 *		Mach "sched_yield" syscall.
 *	004	Dave Butenhof	07 February 1992
 *		Add cma__vp_dump() function to list all active and cached VPs
 *		for debugging.
 *	005	Dave Butenhof	 2 October 1992
 *		Add syscall() functions for the RT stuff we need to implement
 *		_CMA_RT4_KTHREAD_.
 *	006	Dave Butenhof	 4 November 1992
 *		Add macro to get VP sequence number, and add vp_interrupt
 *		argument to ensure "atomicity" without locking at the
 *		caller's level (vp_interrupt can detect if VP has been
 *		reincarnated).
 *	007	Dave Butenhof	25 November 1992
 *		Turn getpri_min/max into macros, since they're constants.
 *		And add get_sched() macro to fetch scheduling info from VP
 *		structure.
 *	008	Dave Butenhof	 3 December 1992
 *		Implement cma__vp_yield() as a macro.
 *	009	Dave Butenhof	26 February 1993
 *		Integrate review comments (23 February 1993)
 *	010	Dave Butenhof	10 March 1993
 *		Add externs for VP & Mach state strings.
 *	011	Dave Butenhof	 5 April 1993
 *		Add cma__vp_reclaim().
 *	012	Dave Butenhof	12 May 1993
 *		Add cma__vp_cache()
 */

#ifndef CMA_VP
#define CMA_VP

/*
 *  INCLUDE FILES
 */

#include <cma_vp_defs.h>

/*
 *  MACROS
 */

#if _CMA_KTHREADS_ == _CMA__MACH
# define cma__vp_get_sequence(_vp_)	(((cma__t_vpid)(_vp_))->seq)
# define cma__vp_getpri_max(_policy_)	63
# define cma__vp_getpri_min(_policy_)	0
# define cma__vp_get_sched(_vp_,_pol_,_pri_) \
    (*(_pol_)=((cma__t_vpid)(_vp_))->policy, \
    *(_pri_)=((cma__t_vpid)(_vp_))->priority)
# define cma__vp_yield()		swtch_pri(0)
#else
# define cma__vp_get_sequence(_vp_)	0
# define cma__vp_getpri_max(_policy_)	0
# define cma__vp_getpri_min(_policy_)	0
# define cma__vp_get_sched(_vp_,_pol_,_pri_) 0
# define cma__vp_yield()		cma__bugcheck( \
	"vp_yield on non-VP: %s:%d", __FILE__,__LINE__)
#endif

/*
 *  GLOBAL DATA
 */

#if !_CMA_UNIPROCESSOR_
extern cma__t_atomic_bit	cma__g_vp_lock;
extern char			*cma__g_vp_statestr[cma__c_vp_st_len];
# if _CMA_KTHREADS_ == _CMA__MACH
  extern char			*cma__g_mach_statestr[6];
# endif
#endif

/*
 * INTERNAL INTERFACES
 */

extern void
cma__init_vp _CMA_PROTOTYPE_ ((void));

extern void
cma__vp_cache _CMA_PROTOTYPE_ ((
	cma__t_vpid		vpid));

extern cma__t_vp_status
cma__vp_create _CMA_PROTOTYPE_ ((
	cma__t_vpid		*vpid));

extern void
cma__vp_delete _CMA_PROTOTYPE_ ((
	cma__t_vpid		vpid));

extern cma__t_vpid
cma__vp_get_id _CMA_PROTOTYPE_ ((void));

extern cma__t_vp_status
cma__vp_get_state _CMA_PROTOTYPE_ ((
	cma__t_vpid		vpid,
	cma__t_vp_state		*state));

extern void
cma__vp_dump _CMA_PROTOTYPE_ ((void));

extern cma__t_vp_status
cma__vp_interrupt _CMA_PROTOTYPE_ ((
	cma__t_vpid		vpid,
	cma__t_vp_handler	handler,
	cma_t_address		arg,
	cma_t_integer		vpseq));

extern cma_t_boolean
cma__vp_reclaim _CMA_PROTOTYPE_ ((
	cma__t_vpid		vpid));

extern cma__t_vp_status
cma__vp_resume _CMA_PROTOTYPE_ ((
	cma__t_vpid		vpid));

extern cma__t_vp_status
cma__vp_resume_others _CMA_PROTOTYPE_ ((void));

extern cma__t_vp_status
cma__vp_set_sched _CMA_PROTOTYPE_ ((
	cma__t_vpid		vpid,
	cma_t_sched_policy	policy,
	cma_t_priority		priority));

extern cma__t_vp_status
cma__vp_set_start _CMA_PROTOTYPE_ ((
	cma__t_vpid		vpid,
	cma__t_vp_state		*state));

extern cma__t_vp_status
cma__vp_suspend _CMA_PROTOTYPE_ ((
	cma__t_vpid		vpid,
	cma_t_integer		milliseconds));

extern cma__t_vp_status
cma__vp_suspend_others _CMA_PROTOTYPE_ ((void));

#endif
