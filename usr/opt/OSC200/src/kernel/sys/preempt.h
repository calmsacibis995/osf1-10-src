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
 *	@(#)$RCSfile: preempt.h,v $ $Revision: 4.4.3.4 $ (DEC) $Date: 1992/12/09 08:56:37 $
 */ 
/*
 */
/*
 *	File:	preempt.h
 *	Author:	Ron Widyono
 *
 *	Preemption point macros.
 *
 *	Revision History:
 *
 * 14-May-91	Ron Widyono
 *	Remove sleep lock restriction on preemption.
 *
 * 5-May-91	Ron Widyono
 *	Optimize.  Incorporate run-time option for kernel preemption
 *	(rt_preempt_enabled).  Count preemption points within macro.
 *
 */

#ifndef _SYS_PREEMPT_H_
#define _SYS_PREEMPT_H_

#include <rt_preempt.h>
#include <rt_preempt_debug.h>

#include <kern/ast.h>

extern int		slock_count[];
extern boolean_t	iplis0();
extern void		do_preemption();
#if	RT_PREEMPT_DEBUG
extern int		rt_preempt_pp;
#endif

/*
 * The following macro represents a preemption point.  If an AST has been
 * requested and the conditions are appropriate for a context switch, then
 * the AST request is cancelled, and thread_block() is called to allow the
 * context switch to occur.  An AST can also be requested for other reasons.
 * If there is no context switch, the AST is allowed to fire, and those
 * other request(s) are serviced.  If there is a context switch, then the
 * AST request must be restored upon return from the thread block.
 */
#if	!RT_PREEMPT_DEBUG
#define preemption_point(preemption_counter)			\
MACRO_BEGIN							\
	if (ast_needed(cpu_number())) {				\
		do_preemption();				\
	}							\
MACRO_END
#else
#define preemption_point(preemption_counter)			\
MACRO_BEGIN							\
	if (ast_needed(cpu_number())) {				\
		rt_preempt_pp++;				\
		do_preemption(&preemption_counter);	\
	}							\
MACRO_END
#endif

/*
 * The following macro partially determines whether this is a "safe" place
 * for a context switch to take place.  "Safe" means that no simple locks
 * have been taken, the IPL is at the minimum value, and the current thread
 * is not being funneled (bound) to the master CPU.  The last test is de-
 * ferred to the do_preemption() routine to avoid a circular include pro-
 * blem.  The preemption request test (ast_needed) is incorporated here to
 * optimize for performance.
 */
#define PREEMPTION_OK						\
	(ast_needed(cpu_number()) &&				\
	 !(slock_count[cpu_number()]) &&			\
	 iplis0())
/*
 * This version of the preemption point macro checks first that it is
 * running in a "safe" place as defined by the PREEMPTION_OK macro.
 */
#if	!RT_PREEMPT_DEBUG
#define preemption_point_safe(preemption_counter) {		\
          if (PREEMPTION_OK) {					\
	    do_preemption();					\
	  }							\
	}
#else	/* !RT_PREEMPT_DEBUG */
#define preemption_point_safe(preemption_counter) {		\
	  rt_preempt_pp++;					\
          if (PREEMPTION_OK) {					\
	    do_preemption(&preemption_counter);			\
	  }							\
	}
#endif

#ifdef __alpha
/*
 * On the alpha, we need to ignore the spl level when making
 * preemption OK check when returning from an interrupt.
 * This special version of PREEMPTION_OK and preemption_point_safe()
 * are used only from trap() in arch/alpha/trap.c
 */
#define PREEMPTION_OK_NOSPL0					\
	(ast_needed(cpu_number()) &&				\
	 !(slock_count[cpu_number()]))

#if	!RT_PREEMPT_DEBUG
#define preemption_point_safe_nospl0(preemption_counter) {	\
          if (PREEMPTION_OK_NOSPL0) {				\
	    do_preemption();					\
	  }							\
	}
#else	/* !RT_PREEMPT_DEBUG */
#define preemption_point_safe_nospl0(preemption_counter) {	\
	  rt_preempt_pp++;					\
          if (PREEMPTION_OK_NOSPL0) {				\
	    do_preemption(&preemption_counter);			\
	  }							\
	}
#endif	/* !RT_PREEMPT_DEBUG */
#endif  /* __alpha */

#endif	/* _SYS_PREEMPT_H_ */
