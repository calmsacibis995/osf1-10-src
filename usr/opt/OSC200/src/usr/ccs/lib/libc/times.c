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
static char	*sccsid = "@(#)$RCSfile: times.c,v $ $Revision: 4.2.5.2 $ (DEC) $Date: 1993/06/07 23:25:11 $";
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
#if !defined(lint) && !defined(_NOIDENT)

#endif
/*
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 * times.c   5.2 (Berkeley) 3/9/86
 */


/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#if !defined(_THREAD_SAFE)
#pragma weak times = __times
#endif
#endif
#include <sys/time.h>
#include <sys/times.h>		/* for struct tms */
#include <sys/resource.h>

/*
 * times() for X/Open/POSIX compatability.
 *
 * Returns the number of (CLK_TCK)ths of a second since the epoch.
 * The value may wrap.
 */

static
scale(tvp)
	register struct timeval *tvp;
{
	/*
	 * convert seconds and microseconds to CLK_TCKs
	 */
	return ((tvp->tv_sec * CLK_TCK) + 
		((tvp->tv_usec * CLK_TCK) / 1000000));
}

clock_t
times(tmsp)
	register struct tms *tmsp;
{
	struct rusage ru;
	struct timeval tp;

	if (getrusage(RUSAGE_SELF, &ru) < 0)
		return (-1);
	tmsp->tms_utime = scale(&ru.ru_utime);
	tmsp->tms_stime = scale(&ru.ru_stime);
	if (getrusage(RUSAGE_CHILDREN, &ru) < 0)
		return ((clock_t) -1);
	tmsp->tms_cutime = scale(&ru.ru_utime);
	tmsp->tms_cstime = scale(&ru.ru_stime);
	if (gettimeofday(&tp, (struct timezone *)0) < 0)
		return((clock_t) -1);
	return((clock_t) scale(&tp));
}
