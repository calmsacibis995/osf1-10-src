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
 *	@(#)$RCSfile: parallel.h,v $ $Revision: 4.2.4.2 $ (DEC) $Date: 1992/03/18 17:45:13 $
 */ 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/* 
 * Mach Operating System
 * Copyright (c) 1989 Carnegie-Mellon University
 * Copyright (c) 1988 Carnegie-Mellon University
 * Copyright (c) 1987 Carnegie-Mellon University
 * All rights reserved.  The CMU software License Agreement specifies
 * the terms and conditions for use and redistribution.
 */

/*
 * OSF/1 Release 1.0
 */

/*
 *	kern/parallel.h
 *
 *	Revision History:
 *
 * 5-May-91	Ron Widyono
 *	Incorporate run-time option for kernel preemption (rt_preempt_enabled).
 *
 * 6-Apr-91	Ron Widyono
 *	Track unix_master/unix_release calls for preemption points.  Preemption
 *	point on unix_release() and unix_release_force().  Conditionalized
 *	by RT_PREEMPT.
 *
 */

#ifndef	_KERN_PARALLEL_H_
#define _KERN_PARALLEL_H_

#include <cpus.h>
#include <rt_preempt.h>
#include <rt_preempt_debug.h>

#if	NCPUS > 1

#define unix_master()  _unix_master()
#define unix_release() _unix_release()
#define unix_release_force()	_unix_release_force()
#define unix_reset()   _unix_reset()
extern void _unix_master(), _unix_release(), _unix_release_force(), 
	    _unix_reset();

#elif	!RT_PREEMPT

#define unix_master()
#define unix_release()
#define unix_release_force()
#define unix_reset()

#else	/* !RT_PREEMPT */

#include <kern/thread.h>
#include <sys/preempt.h>
#include <kern/processor.h>

extern int rt_preempt_enabled;
#if	RT_PREEMPT_DEBUG
extern int rt_preempt_funnel;
#endif

#define unix_master()	{						\
	if (rt_preempt_enabled)						\
	    current_thread()->unix_lock++;				\
}

#define unix_release() {						\
	if (rt_preempt_enabled) {					\
	    if ((--current_thread()->unix_lock) < 0) {			\
		preemption_point_safe(rt_preempt_funnel); \
	    }								\
	}								\
}

#define unix_release_force() {						\
	if (rt_preempt_enabled) {					\
	    current_thread()->unix_lock = -1;				\
	    preemption_point_safe(rt_preempt_funnel);			\
	}								\
}

#define unix_reset() {							\
	if (rt_preempt_enabled)						\
	    if (current_thread()->unix_lock != -1)			\
		current_thread()->unix_lock = 0;			\
}

#endif	/* NCPUS > 1 */

#endif	/* _KERN_PARALLEL_H_ */
