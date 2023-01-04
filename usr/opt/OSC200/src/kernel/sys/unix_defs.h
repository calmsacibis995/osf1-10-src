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
 *	@(#)$RCSfile: unix_defs.h,v $ $Revision: 4.2.3.2 $ (DEC) $Date: 1993/01/20 14:06:39 $
 */ 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */

/*
 * unix_defs.h:  definitions and macros for extractable UNIX portions
 * of the OSF/1 kernel.
 */
/* 
 * OSF/1 Release 1.0
 */

#ifndef	_UNIX_DEFS_H_
#define	_UNIX_DEFS_H_

#include <rt_preempt.h>
#include <bogus_memory.h>
#include <lock_stats.h>
#include <mach_assert.h>
#include <mach_ldebug.h>
#include <ser_compat.h>
#include <slock_stats.h>
#include <unix_locks.h>
#include <unix_uni.h>
#include <vague_stats.h>
#ifndef	NCPUS
#include <cpus.h>
#endif

#include <kern/lock.h>
#include <kern/macro_help.h>
#include <kern/assert.h>

/*
 * The OSF/1 kernel can be divided into two portions:  the core kernel,
 * including all the Mach functionality, and the compatibility code,
 * including the filesystem, network, and the Unix interfaces.
 *
 * All of the core code uses lock-based synchronization; in the uni-
 * processor case, simple locks do not get compiled because they are
 * not needed.  In the multiprocessor case, the code is fully
 * parallelized.  Distinguishing between the two cases is done by
 * using the compile-time definition, NCPUS.
 *
 * The compatibility code has also been parallelized but can be built in
 * one of four ways:
 *
 *	1.  Uniprocessor only, with software interrupt synchronization.
 *		Defines:  UNIX_UNI + NCPUS=1
 *	2.  Uniprocessor only, with lock synchronization.
 *		Defines:  UNIX_LOCKS + NCPUS=1
 *	3.  Multiprocessor, with lock synchronization.
 *		Defines:  UNIX_LOCKS + NCPUS>1
 *	4.  Multiprocessor, with lock synchronization and hooks
 *		for compatibility with unparallelized subsystems.
 *		Defines:  UNIX_LOCKS + NCPUS>1 + SER_COMPAT
 *
 * In the first two "uniprocessor" cases, the resulting kernel runs
 * master/slave on a multiprocessor; the Mach core code executes on any
 * processor but the compatibility code runs only on the master processor.
 * Note that using software interrupt synchronization does not imply that
 * no locks are being used at all; just that the original BSD notions of
 * locks are being used rather than OSF locks.
 * 
 * The multiprocessor cases only use lock synchronization.  Interrupt
 * handling is all done at a low-level and what might have been interrupt-
 * level activities in the uniprocessor case (e.g., packet processing) are
 * now handled by additional threads.  When backwards compatibility is
 * turned on so that unparallelized code can be used alongside parallelized
 * code, the kernel pays a time and space penalty to track the various
 * unparallelized portions.
 *
 * Caveat programmer:  there is no internal consistency checking among
 * the various options.
 *
 * There are three debug options that can be used to assist resolving
 * kernel problems.
 *
 *	MACH_LDEBUG turns on lock debugging.  Simple locks will always
 *	be compiled when MACH_LDEBUG is turned on.  Read/write locks
 *	will be checked for common cases like taking the same lock
 *	twice or releasing an unowned lock.  MACH_LDEBUG should be
 *	used in conjunction with MACH_ASSERT.
 *
 *	MACH_ASSERT turns on the assert/ASSERT macros that check
 *	run-time conditions and, if the checks fail, cause a system
 *	panic and crash dump or a pit-stop into the debugger.  There
 *	is a special form of ASSERT, LASSERT, that depends on both
 *	MACH_LDEBUG and MACH_ASSERT being turned on.
 *
 *	XPR_DEBUG records trace messages in a buffer that can be
 *	examined at run-time or in a crash dump.
 *	
 * Lock statistics may be gathered by enabling one or both of the
 * following options:
 *
 *	SLOCK_STATS	simple lock statistics
 *	LOCK_STATS	read/write lock statistics
 *
 * Neither of these options is related to VAGUE_STATS, which indicates
 * that the user is willing to accept counts that may be slightly off
 * for things like the NFS and RPC operations counts.  When VAGUE_STATS
 * is turned off, simple locks are used around these statistics updates,
 * guaranteeing their accuracy.
 */

#if	__STDC__
#if	UNIX_UNI && UNIX_LOCKS
/* #error this is an error */
#endif
#endif

/*
 * The mp_master and mp_release definitions are used for portions of the
 * Mach code that must interface to Unix without knowing whether the
 * Unix portions have been parallelized.  For unparallelized portions,
 * the Mach code uses mp_master and mp_release, which go away under
 * UNIX_LOCKS but otherwise invoke the unix_* routines.
 */
#if	UNIX_UNI
#define	mp_master()	unix_master()
#define	mp_release()	unix_release()
#endif

#if	UNIX_LOCKS
#define	mp_master()
#define	mp_release()
#endif

#if	!UNIX_LOCKS
/*
 * When locking is disabled for the compatibilty code, synchronization
 * is done via spl's.  The following macros are used for spl synchronization
 * that applies only to the uniprocessor case.  For spl synchronization
 * that must be done in all cases (e.g., when communicating with a device
 * driver) the original, lower-case spl macros must be used.
 *
 * Note that the USPLVAR macro is defined in such a way that it may be used
 * in the middle of a block of declarations as long as the declaration is
 * not followed by a semi-colon.  For example,
 *	int foo;
 *	USPLVAR(s)
 *	int bar;
 * will correctly declare "int s;" when locking is disabled and disappear
 * when locking is enabled.
 */
#define USPLVAR(s)		int s;
#ifdef	__STDC__
#define USPL(s,level)		s = spl##level()
#else
#define USPL(s,level)		s = spl/**/level()
#endif
#define	USPLNET(s)		USPL(s,net)
#define	USPLTTY(s)		USPL(s,tty)
#define	USPLIMP(s)		USPL(s,imp)
#define	USPLBIO(s)		USPL(s,bio)
#define	USPLVM(s)		USPL(s,vm)
#define	USPLHI(s)		USPL(s,hi)
#define	USPLCLOCK(s)		USPL(s,clock)
#define	USPLSOFTCLOCK(s)	USPL(s,softclock)
#define	USPLHIGH(s)		USPL(s,high)
#define	USPLSCHED(s)		USPL(s,sched)
#define USPLX(s)		splx(s)

#else	/* !UNIX_LOCKS */

/*
 * Use locks for synchronization.  Note again that lower-case spl
 * macros must be used to obtain spl synchronization independent of
 * the UNIX_LOCKS conditional.
 */
#define USPLVAR(s)
#define USPL(s,level)
#define	USPLNET(s)
#define	USPLTTY(s)
#define	USPLIMP(s)
#define	USPLBIO(s)
#define	USPLVM(s)
#define	USPLHI(s)
#define	USPLCLOCK(s)
#define	USPLSOFTCLOCK(s)
#define	USPLHIGH(s)
#define	USPLSCHED(s)
#define USPLX(s)

#endif	/* !UNIX_LOCKS */

#if	UNIX_LOCKS
#define	ulock_read(l)			lock_read(l)
#define	ulock_write(l)			lock_write(l)
#define	ulock_done(l)			lock_done(l)
#define	ulock_init(l,w,t)		lock_init2((l),(w),(t))
#define	udecl_lock_data(class,name)	class lock_data_t name;
#define	ULOCK_HOLDER(l)			LOCK_HOLDER(l)
#define	usimple_lock(l)			simple_lock(l)
#define	usimple_unlock(l)		simple_unlock(l)
#define	usimple_lock_try(l)		simple_lock_try(l)
#define	usimple_lock_init(l)		simple_lock_init(l)
#define	udecl_simple_lock_data(class,name) decl_simple_lock_data(class,name)
#else
#define	ulock_read(l)
#define	ulock_write(l)
#define	ulock_done(l)
#define	ulock_init(l,w,t)
#define	udecl_lock_data(class,name)
#define	ULOCK_HOLDER(l)
#define	usimple_lock(l)
#define	usimple_unlock(l)
#define	usimple_lock_try(l)	1
#define	usimple_lock_init(l)
#define	udecl_simple_lock_data(class,name)
#endif

#if	SER_COMPAT && !RT_PREEMPT
#include <kern/parallel.h>
/*
 * OSF/1 provides some backwards compatibility with unparallelized
 * subsystems through the funnel mechanism.  A funnel indicates whether
 * a subsystem is unparallelized and, if so, how the kernel should
 * serialize before invoking the subsystem.
 */
#ifdef	notyet
struct funnel {
	funnel_type	f_type;
	union {
		lock_data_t	f_lock;	/* lock guarding subsystem */
		processor_t	f_proc;	/* always use this processor */
		queue_head_t	f_threads; /* use one of these threads */
	} fu_un;
} funnel;
#endif

/*
 * Cheat for now, using a funnel as a boolean; TRUE means
 * take unix_master.
 */
extern lock_data_t	default_uni_lock;
#define	udecl_funnel_data(class,name)	class int name;
#define FUNNEL_NULL	((int) NULL)

/*
 *
 * N.B.  The commas in the FUNNEL macros keep lint and some compilers happy.
 */
#define	FUNNEL(f)						\
MACRO_BEGIN							\
	if ((f) != FUNNEL_NULL)					\
		ser_funnel(f);					\
MACRO_END

#define	UNFUNNEL(f)						\
MACRO_BEGIN							\
	if ((f) != FUNNEL_NULL)					\
		ser_unfunnel(f);				\
MACRO_END

#elif RT_PREEMPT

#define	udecl_funnel_data(class,name)
#define	FUNNEL(p)
#define	UNFUNNEL(p)
#define FUNNEL_NULL	((int) NULL)

#define	RT_FUNNEL(f)						\
MACRO_BEGIN							\
	if ((f) != FUNNEL_NULL)					\
		ser_funnel((f));				\
MACRO_END

#define	RT_UNFUNNEL(f)						\
MACRO_BEGIN							\
	if ((f) != FUNNEL_NULL)					\
		ser_unfunnel((f));				\
MACRO_END

#else	/* SER_COMPAT */

#define	udecl_funnel_data(class,name)
#define	FUNNEL(p)
#define	UNFUNNEL(p)
#define FUNNEL_NULL

#endif	/* SER_COMPAT */


#if	UNIX_LOCKS
#define	MP_ONLY(clause)		(clause)
#define	MP_DECL(decl)		decl;
#define	UNI_ONLY(clause)
#else
#define	MP_ONLY(clause)
#define	MP_DECL(decl)
#define	UNI_ONLY(clause)	(clause)
#endif

/*
 * Statistics.
 */
#if	VAGUE_STATS
#define	vdecl_simple_lock_data(class,name)
#define	VSTATS_LOCK(l)
#define	VSTATS_UNLOCK(l)
#define	VSTATS_LOCK_INIT(l)
#else
#define	vdecl_simple_lock_data(class,name) udecl_simple_lock_data(class,name)
#define	VSTATS_LOCK(l)			usimple_lock(l)
#define	VSTATS_UNLOCK(l)		usimple_unlock(l)
#define	VSTATS_LOCK_INIT(l)		usimple_lock_init(l)
#endif

#define	STATS_ACTION(l,stmt)						\
MACRO_BEGIN								\
	VSTATS_LOCK(l);							\
	stmt;								\
	VSTATS_UNLOCK(l);						\
MACRO_END

/*
 * Bogus memory locking.  On some architectures, data aligned on
 * "natural" boundaries (e.g., longs always allocated on a longword
 * boundary) can be fetched from memory atomically; i.e., the current
 * value or a slightly stale value can be fetched, but the value will
 * never be trashed as the result of interleaved write operations.
 * Depending on the case, we can take advantage of this property by
 * not holding a lock during the access because the use of the lock
 * makes no difference.  (The Multimax is such a machine.)
 *
 * BM is placed around statements that should only be executed for
 * bogus memory machines.  NM is placed around statements that should
 * only be executed on "nice" memory machines.
 */
#if	BOGUS_MEMORY
#define	BM(clause)	clause
#define	NM(clause)
#else
#define	BM(clause)
#define	NM(clause)	clause
#endif

#endif	/* _UNIX_DEFS_H_ */
