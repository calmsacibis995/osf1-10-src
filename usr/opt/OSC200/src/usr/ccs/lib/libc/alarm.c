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
static char	*sccsid = "@(#)$RCSfile: alarm.c,v $ $Revision: 4.2.4.3 $ (DEC) $Date: 1993/06/08 01:22:57 $";
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
 * Copyright (c) 1983 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 * alarm.c	5.2 (Berkeley) 3/9/86
 */


/*
 * Backwards compatible alarm.
 */
/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#pragma weak alarm = __alarm
#endif
#include <unistd.h>
#include <sys/time.h>
#include <sys/table.h>
#include <values.h>
#include <errno.h>
#ifdef _THREAD_SAFE
#include "rec_mutex.h"
extern struct rec_mutex	_alarm_rmutex;
#endif

unsigned int
alarm(unsigned int secs)
{
	struct itimerval it, oitv;
	register struct itimerval *itp = &it;

#ifdef _THREAD_SAFE
	rec_mutex_lock(&_alarm_rmutex);
#endif
	timerclear(&itp->it_interval);
	itp->it_value.tv_sec = secs;
	itp->it_value.tv_usec = 0;

	if (setitimer(ITIMER_REAL, itp, &oitv) == -1) {
#ifdef _THREAD_SAFE
		rec_mutex_unlock(&_alarm_rmutex);
#endif
		return 0;
	}
	if (oitv.it_value.tv_usec)
		oitv.it_value.tv_sec++;
#ifdef _THREAD_SAFE
	rec_mutex_unlock(&_alarm_rmutex);
#endif
	return ((unsigned int) oitv.it_value.tv_sec);
}
