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
static char	*sccsid = "@(#)$RCSfile: schgetp.c,v $ $Revision: 4.2.4.2 $ (DEC) $Date: 1992/03/19 17:08:06 $";
#endif
/*
 * sched_getparam
 *
 * Gets the scheduler parameters (currently only the priority) for a
 * specified process.
 *
 *	Revision History:
 * 10-Oct-91	Amy Kessler
 *	Removed syscall and made new get and set priority functions
 *	with extra parameters.
 *
 * 08-Jan-92    Ron Krueger
 *      Ported to D10 to D11. Changed sched_get_sched_param to sched_getparam.
 *      Also changed name of the priority field in the sched_param structure
 *      from priority to sched_priority.
 *
 * 10-Oct-91	Amy Kessler
 *	Removed syscall and made new get and set priority functions
 *	with extra parameters.
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

int sched_getparam(pid, param)
     register pid_t pid;
     register struct sched_param *param;
{
  register int prio;

  /* 
   * Trap negative pid values here.  If we want to use negative pid values to
   * extend the interface in the future, we need to complain to people who
   * pass them to us now.
   */
  if (pid < 0) {
    SETERRNO(EINVAL);
    return -1;
  }

  /*
   * Trap null param structure.  It could be non-null and invalid, but at
   * least capture the obvious case.
   */
  if (param == 0) {
    SETERRNO(EINVAL);
    return -1;
  }

  /* Get the priority. */
  prio = rt_getprio( PRIO_POSIX, pid, 0, 0, 0);

  /* 
   * If the get failed, return -1.  Otherwise, put the priority into the
   * parameter block.
   *
   * If the get failed with EINVAL, it can only mean that getpriority was
   * upset about the PRIO_POSIX flag.  If getpriority didn't understand that
   * flag, it means that the realtime kernel option is not turned on.  In
   * this case, we want to return ENOSYS instead of EINVAL.
   */
  if (prio == -1) {
    if (errno == EINVAL) SETERRNO(ENOSYS);
    return -1;
  }
  else {
    param->sched_priority = prio;
    return 0;
  }
}
