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
static char	*sccsid = "@(#)$RCSfile: schsetp.c,v $ $Revision: 4.2.4.2 $ (DEC) $Date: 1992/03/19 17:10:56 $";
#endif
/*
 * @DIGITALCOPYRIGHT@
 */
/*
 * sched_setparam
 *
 * Sets the scheduler parameters (currently only the priority) for a
 * specified process.
 *
 *	Revision History:
 * 08-Jan-92    Ron Krueger
 *      Ported to D10 to D11. Changed sched_set_sched_param to sched_setparam.
 *      Also changed name of the priority field in the sched_param structure
 *      from priority to sched_priority.
 *
 * 10-Oct-91	Amy Kessler
 *	Changed to new syscall with two additional parameters.
 *
 * 08-Jan-92    Ron Krueger
 *      Ported to D10 to D11. Changed sched_set_sched_param to sched_setparam.
 *      Also changed name of the priority field in the sched_param structure
 *      from priority to sched_priority.
 *
 * 10-Oct-91	Amy Kessler
 *	Changed to new syscall with two additional parameters.
 *
 * 21-May-91	Peter H. Smith
 *	Return EINVAL if the pid is negative, or if the priority is out of
 *	range.  The first change is for upward compatibility, and the second
 *	is to prevent returning ENOSYS.
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

int sched_setparam(pid, param)
     pid_t pid;
     struct sched_param *param;
{
  register int sts;

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
   * Need to make sure the priority is within the legal range, and return
   * EINVAL here if it isn't.  This is because an EINVAL returned by the
   * syscall is converted to ENOSYS.
   */
  if ((pid < 0) || (param == 0)) {
    SETERRNO(EINVAL);
    return -1;
  }

  /*
   * If the thread is in transit accross processor sets or the thread is
   * being assigned to a processor, the attempt to set the priority could
   * fail.  Just keep trying if that is the case.  The standard doesn't
   * let us return an error in this case.
   */
  do {
    sts = rt_setprio( PRIO_POSIX, pid, param->sched_priority, 0, 0, 0);
  } while (sts == EAGAIN);

  /*
   * The call will return EINVAL if the priority is out of range or if the
   * underlying kernel does not have the RT_SCHED option turned on.  Check
   * here to see if the priority is out of range, and if not, change the
   * errno to ENOSYS.
   *
   * This is done this way so that we don't pay for the range check twice
   * on the non-error path.
   */
  if ((sts == -1) && (errno == EINVAL) &&
      (param->sched_priority >= SCHED_PRIO_USER_MIN) &&
      (param->sched_priority <= SCHED_PRIO_RT_MAX)) {
    SETERRNO(ENOSYS);
  }

  return sts;  
}
