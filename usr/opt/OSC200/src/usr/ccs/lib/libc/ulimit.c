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
static char	*sccsid = "@(#)$RCSfile: ulimit.c,v $ $Revision: 4.2.7.2 $ (DEC) $Date: 1993/06/07 23:42:28 $";
#endif 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0
 */
/*
 * ulimit.c
 *
 *	Revision History:
 *
 * 01-Apr-91	Fred Canter
 *	MIPS C 2.20+, changes for -std.
 *
 * 12-Mar-91	Fred Canter
 *	MIPS C 2.20+
 *
 */
#if !defined(lint) && !defined(_NOIDENT)

#endif

/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#pragma weak ulimit = __ulimit
#endif
#ifdef __STDC__
#include <stdarg.h>
#else
#include <varargs.h>
#endif
#include <sys/time.h>
#include <sys/resource.h>
#include <errno.h>
#include <ulimit.h>
#include <signal.h>
#include "ts_supp.h"

#define	BLK	512		/* number of bytes in filesize unit */

long
#ifndef	_NO_PROTO
ulimit(int cmd, ...)
#else
ulimit(cmd, va_alist)
int cmd;
va_dcl
#endif
{
	struct rlimit rl;
	long	newlimit;
	va_list	ap;
	long	retval;
	struct sigaction action;

#ifdef  __STDC__
	va_start(ap, cmd);
#else
        va_start(ap);
#endif
	switch (cmd) {
	case UL_GETFSIZE:		/* get process file size limit */
		/*
		 * Warning: Sigaction hack.
		 * As we implement ulimit in terms of rlimits there
		 * is a problem. According to rlimits if you excede
		 * the file size limit then you get sent the signal
		 * SIGXFSZ (which by default terminates the process)
		 * Ulimit however want EFBIG returned. The kernel
		 * does both so we ignore the signal for people who call
		 * ulimit.
		 */
		action.sa_handler = SIG_IGN;
		sigaction(SIGXFSZ, &action, NULL);
		if (getrlimit(RLIMIT_FSIZE, &rl) == 0)
			retval = (long)(rl.rlim_cur / BLK);
		else
			retval = (long)-1;
		break;
	case UL_SETFSIZE:		/* set process file size limit */
		/*
		 * Warning: Sigaction hack.
		 * see above.
		 */
		action.sa_handler = SIG_IGN;
		sigaction(SIGXFSZ, &action, NULL);
		/*
		 * We have a small problem here that ulimit only knows
		 * about one number and our resource limits have two so
		 * we have to try and keep them in step in a sensible manner,
		 * here's what happens:
		 *
		 * If new > max
		 *	if privileged
		 *		set both cur and max to new
		 *	else
		 *		EPERM
		 * else if new < cur
		 *	set both cur and max to new
		 * else if cur < new < max
		 *	set max to cur
		 *	if privileged
		 *		set cur to new
		 *	else
		 *		EPERM
		 *
		 * The case that is not dealt with very elegantly is when
		 * cur > max but to do this you must be privileged so attempts
		 * at keeping cur and max in step are pretty futile.
		 */
		newlimit = va_arg(ap, long) * BLK;
		if (getrlimit(RLIMIT_FSIZE, &rl) < 0) {
			retval = (long)-1;
			break;
		}
		if ((newlimit <= rl.rlim_cur) || (newlimit > rl.rlim_max))
			rl.rlim_max = (unsigned long)newlimit;
		else if (rl.rlim_cur < rl.rlim_max) {
			rl.rlim_max = rl.rlim_cur;
			if (setrlimit(RLIMIT_FSIZE, &rl) < 0) {
				retval = (long)-1;
				break;
			}
		}
		rl.rlim_cur = (unsigned long)newlimit;
		if (setrlimit(RLIMIT_FSIZE, &rl) == 0)
			retval = (long)(rl.rlim_cur / BLK);
		else
			retval = (long)-1;
		break;
	case UL_GETBREAK:		/* get maximum possible break value */
		if (getrlimit(RLIMIT_DATA, &rl) == 0)
			retval = (long)rl.rlim_max;
		else
			retval = (long)-1;
		break;
	default:			/* oops. bad cmd argument. */
		TS_SETERR(EINVAL);
		retval = (long)-1;
		break;
	}
	va_end(ap);
	return(retval);
}
