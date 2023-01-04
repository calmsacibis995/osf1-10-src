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
static char	*sccsid = "@(#)$RCSfile: schgetsch.c,v $ $Revision: 4.2.3.2 $ (DEC) $Date: 1992/07/19 14:38:04 $";
#endif
/*
 * @DIGITALCOPYRIGHT@
 */
/*
 * sched_getscheduler
 *
 * Returns the scheduling policy of the specified process.
 *
 *	Revision History:
 *
 * 03-May-91	Peter H. Smith
 *	New file.
 */

#include <sched.h>
#include <mach/policy.h>
#include <errno.h>

#ifdef _THREAD_SAFE
#define SETERRNO(errval) seterrno(errval)
#else /* _THREAD_SAFE */
#define SETERRNO(errval) errno = (errval)
#endif /* _THREAD_SAFE */

int sched_getscheduler(pid)
     register pid_t pid;
{
  register int policy;

  /* 
   * Trap negative pid values here, in case we add some escapes to the
   * setpriority() function.
   */
  if (pid < 0) {
    SETERRNO(EINVAL);
    return -1;
  }

  /*
   * Get the current thread policy.
   */
  policy = rt_getprio( PRIO_POSIX | PRIO_POLICY, pid, 0, 0, 0);

  /*
   * Translate the policy.
   */
  switch(policy) {
  case POLICY_FIFO:
    policy = SCHED_FIFO;
    break;
  case POLICY_RR:
    policy = SCHED_RR;
    break;
  case POLICY_TIMESHARE:
  case POLICY_FIXEDPRI:
    policy = SCHED_OTHER;
    break;
  default:
    SETERRNO(EINVAL);
    return -1;
  }

  /*
   * Return the policy or error status.
   */
  return policy;
}



