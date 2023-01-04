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
#ifndef lint
static char	*sccsid = "@(#)$RCSfile: schsetsch.c,v $ $Revision: 4.2.4.2 $ (DEC) $Date: 1992/03/19 17:13:16 $";
#endif
/*
 * sched_setscheduler
 *
 * Sets the scheduling policy and parameters (currently only priority) for a
 * specified process.
 *
 *	Revision History:
 * 10-Oct-91	Amy Kessler
 *	Changed syscall to a new one with additional parameters.
 *
 * 08-Jan-92    Ron Krueger
 *      Ported to D10 to D11. Changed name of the priority field in the
 *      sched_param structure from priority to sched_priority.
 *
 * 10-Oct-91	Amy Kessler
 *	Changed syscall to a new one with additional parameters.
 *
 * 21-May-91	Peter H. Smith
 *    - Shout EINVAL if the priority is out of range.  This guarantees that
 *	a return status of EINVAL from the syscall will always mean that the
 *	realtime kernel is not installed.
 *    - Return the old scheduling policy.
 *
 * 03-May-91	Peter H. Smith
 *	New file.
 */

#include <syscall.h>
#include <sched.h>
#include <mach/policy.h>
#include <errno.h>

#ifdef _THREAD_SAFE
#define SETERRNO(errval) seterrno(errval)
#else /* _THREAD_SAFE */
#define SETERRNO(errval) errno = (errval)
#endif /* _THREAD_SAFE */

int sched_setscheduler(pid, policy, param)
     register pid_t pid;
     register int policy;
     register struct sched_param *param;
{
  register int old_policy;

  /*
   * Trap various user errors here.
   *
   * If we want to use negative pid values to extend the interface in the
   * future, we need to complain to people who pass them to us now.
   *
   * Check for null param pointer.  Doesn't catch all invalid pointers, but
   * catches a common failure.  Note that we will blow up with a SEGV if the
   * pointer is invalid.
   *
   */
  if ((pid < 0) || (param == 0)) {
    SETERRNO(EINVAL);
    return -1;
  }

  /*
   * Translate the policy to the internal value which is passed accross the
   * kernel interface.  Return an error if the policy is invalid.
   */
  switch(policy) {
  case SCHED_FIFO:
    policy = PRIO_POLICY_FIFO;
    break;
  case SCHED_RR:
    policy = PRIO_POLICY_RR;
    break;
  case SCHED_OTHER:
    policy = PRIO_POLICY_OTHER;
    break;
  default:
    SETERRNO(EINVAL);
    return -1;
  }

  /*
   * If the thread is in transit accross processor sets or the thread is
   * being assigned to a processor, the attempt to set the priority could
   * fail.  Just keep trying if that is the case.
   */
  do {
    old_policy = rt_setprio( PRIO_POSIX | policy, pid, 
			 param->sched_priority, 0, 0, 0);
  } while (old_policy == EAGAIN);

  /*
   * The call will return EINVAL if the priority is out of range or if the
   * underlying kernel does not have the RT_SCHED option turned on.  Check
   * here to see if the priority is out of range, and if not, change the
   * errno to ENOSYS.
   *
   * This is done this way so that we don't pay for the range check twice
   * on the non-error path.
   */
  if ((old_policy == -1) && (errno == EINVAL) &&
      (param->sched_priority >= SCHED_PRIO_USER_MIN) &&
      (param->sched_priority <= SCHED_PRIO_RT_MAX)) {
    SETERRNO(ENOSYS);
  }

  /*
   * If the return value is a policy, translate it.  Otherwise, leave it
   * alone (assume that the kernel routine keeps policies and errnos straight).
   */
  switch(old_policy) {
  case PRIO_POLICY_FIFO:
    old_policy = SCHED_FIFO;
    break;
  case PRIO_POLICY_RR:
    old_policy = SCHED_RR;
    break;
  case PRIO_POLICY_OTHER:
  case PRIO_POLICY_FIXED:
    old_policy = SCHED_OTHER;
    break;
  default:
    break;
  }

  return old_policy;
}
