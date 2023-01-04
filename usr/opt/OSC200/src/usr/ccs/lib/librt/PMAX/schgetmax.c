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
static char	*sccsid = "@(#)$RCSfile: schgetmax.c,v $ $Revision: 4.2.3.2 $ (DEC) $Date: 1992/01/28 17:30:47 $";
#endif
/*
 * sched_get_priority_max
 *
 * Returns the maximum allowable scheduling priority for a given policy.
 *
 * For now, just return a constant.  Because of the way the kernel include
 * files are laid out, it is not possible to get at NRQS here.
 *
 * This mechanism will break if we make the number of run queues configurable.
 *
 *	Revision History:
 *
 * 11-Oct-91	Amy Kessler
 *	Change to new syscall with additional parameters.
 *
 * 05-Jun-1991	Peter H. Smith
 *	Replace hardcoded max priority with a constant.  This still doesn't
 *	query the kernel for the info, but it is at least a step closer.
 *
 * 03-May-1991	Peter H. Smith
 *	New file.
 */

#include <sys/syscall.h>
#include <errno.h>
#include <sched.h>
#include <mach/policy.h>

#ifdef _THREAD_SAFE
#define SETERRNO(errval) seterrno(errval)
#else /* _THREAD_SAFE */
#define SETERRNO(errval) errno = (errval)
#endif /* _THREAD_SAFE */

int sched_get_priority_max(policy)
     register int policy;
{
  register int sts;

  /* 
   * Check the policy, and blow up if it is invalid.  Will help to preserve
   * compatibility if we get fancy later.
   */
  if ((policy != SCHED_FIFO)
      && (policy != SCHED_RR)
      && (policy != SCHED_OTHER)) {
    SETERRNO(EINVAL);
    return(-1);
  }

  /*
   * If the RT Update was not installed on the running kernel, let
   * the caller know.  This syscall just tests for the presence of the
   * RT support code.
   */
  sts = rt_getprio( PRIO_POSIX, 0, 0, 0, 0);
  if (sts == -1) {
    SETERRNO(ENOSYS);
    return(-1);
  }

  return SCHED_PRIO_RT_MAX;
}
