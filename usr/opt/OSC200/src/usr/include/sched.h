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
 *	@(#)$RCSfile: sched.h,v $ $Revision: 4.2.5.2 $ (DEC) $Date: 1993/12/15 22:14:11 $
 */
/*
 */
/*
 * sched.h
 *
 * POSIX 1003.4 Scheduling Interface definitions.
 * Draft 10 compatibility available with POSIX_4D10 switch.
 */

#ifndef _SCHED_H_
#define _SCHED_H_ 1

#ifdef _POSIX_4SOURCE

#include <sys/types.h>
#include <sys/timers.h>

/*
 * POSIX 1003.4 scheduling policies:
 *
 *   SCHED_FIFO		First-In-First-Out (defined by 1003.4).
 *   SCHED_RR		Round-Robin (defined by 1003.4).
 *   SCHED_OTHER	Standard Timesharing Policy (allowed by 1003.4).
 */

#define SCHED_FIFO 1
#define SCHED_RR 2
#define SCHED_OTHER 3


/*
 * POSIX 1003.4 scheduling parameters.
 */

struct sched_param {
#ifdef POSIX_4D10 
  int priority;			/* Scheduling priority. */
#else
  int sched_priority;          	/* Scheduling priority. */
#endif
};


/*
 * POSIX 1003.4 scheduling function prototypes.
 *
 * Note:  The Draft 10 standard is ambiguous about sched_get_rr_interval.  It
 *	  says the prototype should be void, but the Errors section says that
 *	  it should return -1 if an error is returned.  Since passing it an
 *	  invalid pointer can cause it to fail and set errno to EINVAL, the
 *	  prototype is defined as int instead of void.
 */

#ifdef POSIX_4D10
#define sched_get_sched_param sched_getparam
#define sched_set_sched_param sched_setparam
#define sched_get_rr_interval sched_get_rr_interval_d10
#else
#define sched_get_sched_param obsolete_function_sched_get_sched_param
#define sched_set_sched_param obsolete_function_sched_set_sched_param
#endif

#ifdef _NO_PROTO
int sched_setparam();
int sched_getparam();
int sched_setscheduler();
int sched_getscheduler();
int sched_yield();
int sched_get_priority_max();
int sched_get_priority_min();
int sched_get_rr_interval();
#ifdef POSIX_4D10
int sched_get_rr_interval_d10();
#else
int sched_get_rr_interval();
#endif
#else /* _NO_PROTO */
_BEGIN_CPLUSPLUS
int sched_set_param(pid_t pid, struct sched_param *param);
int sched_get_param(pid_t pid, struct sched_param *param);
int sched_setscheduler(pid_t pid, int policy, struct sched_param *param);
int sched_getscheduler(pid_t pid);
int sched_yield(void);
int sched_get_priority_max(int policy);
int sched_get_priority_min(int policy);
#ifdef POSIX_4D10
int sched_get_rr_interval_d10(struct timespec *min);
#else
int sched_get_rr_interval(pid_t pid, struct timespec *min);
#endif 
_END_CPLUSPLUS
#endif /* _NO_PROTO */


#endif /* _POSIX_4SOURCE */

/*
 * The following constants are NOT defined by POSIX 1003.4/D11.  However, the
 * prefix SCHED_ is reserved for POSIX implementations, so it is alright to
 * define these when _POSIX_4SOURCE is on.
 *
 * These constants define priority ranges for various classes of application.
 * There are three classes of application: user (non-time critical), system
 * (more important than user, but still not highly time critical), and
 * realtime (highly time-critical).  Each class has a minimum and maximum
 * priority.  The minimum priority is the lowest priority which will not suffer
 * from interference by lower priority classes.  The maximum priority is the
 * highest priority which should be given to an application of this class.
 *
 * Note that these constants are advisory in nature, and there are no
 * mechanisms for testing or enforcing their validity.
 *
 * These constants may be useful for application developers who want to find
 * a particular part of the priority range to run in.  When porting to other
 * POSIX 1003.4 implementations, the developer will have to define these
 * constants.  For example, an application could do the following:
 *
 *	#include <sched.h>
 *	#ifndef SCHED_PRIO_RT_MIN
 *	#define SCHED_PRIO_RT_MIN sched_get_priority_min(SCHED_FIFO)
 *	#endif
 *
 *	main()
 *	{
 *	  int min_pri = SCHED_PRIO_RT_MIN;
 *
 *	  ...
 *	}
 */
#define SCHED_PRIO_RT_MAX 63
#define SCHED_PRIO_RT_MIN 32
#define SCHED_PRIO_SYSTEM_MAX 31
#define SCHED_PRIO_SYSTEM_MIN 20
#define SCHED_PRIO_USER_MAX 19
#define SCHED_PRIO_USER_MIN 0

#endif /* _SCHED_H_ */
