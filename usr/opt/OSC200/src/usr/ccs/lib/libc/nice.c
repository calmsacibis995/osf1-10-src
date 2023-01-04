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
static char	*sccsid = "@(#)$RCSfile: nice.c,v $ $Revision: 4.2.7.2 $ (DEC) $Date: 1993/06/07 23:30:48 $";
#endif 
/*
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 */
/*
 * OSF/1 1.2
 */
#if !defined(lint) && !defined(_NOIDENT)
#endif
/*
 * FUNCTIONS: nice
 *
 * (C) COPYRIGHT International Business Machines Corp. 1989 
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 * nice.c	1.6  com/lib/c/gen,3.1,8943 10/5/89 09:21:52
 */

/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#pragma weak nice = __nice
#endif
#include <sys/time.h>
#include <sys/param.h>
#include <sys/resource.h>
#include <errno.h>

#include "ts_supp.h"

#ifdef _THREAD_SAFE
#include "rec_mutex.h"

extern struct rec_mutex	_nice_rmutex;

#define	RETURN(val)	return(_rec_mutex_unlock(&_nice_rmutex), (val))

#else
#define	RETURN(val)	return(val)
#endif	/* _THREAD_SAFE */


/*
 *
 * FUNCTION: Nice adds an increment value to the nice value of the
 *	calling process.  Nice will fail and not change the nice value if
 *	the increment value is negative and the effective user ID of the
 *	calling process does not have SET_PROC&US1.PRIORITY system privilege.
 *	Nice is restricted to setting priorities in the range 0 to 39.
 *
 * PARAMETERS:
 *	incr	- an integer value by which the process's nice value
 *		  is changed.
 *
 * NOTES: The setpriority system call does the hard work of checking
 *	privilege, enforcing system limits, and changing the process's
 *	nice value.
 *
 * RETURN VALUE DESCRIPTION: Nice returns the new nice value minus NZERO.
 */

/*
 *
 * get/setpriority deal in range [-20..+20] as does the kernel.
 */
int
nice(int incr)
{
	int prio;
	int saverr;

	TS_LOCK(&_nice_rmutex);

	saverr = _Geterrno();
	_Seterrno(0);
	prio = getpriority(PRIO_PROCESS, 0);	/* In range of [-20..+20] */
	if (prio == -1 && _Geterrno())
		RETURN(-1);

	_Seterrno(saverr);

	/*
	 * we do this test here as the kernel will effectively range
	 * check 0..40 (ie 41 nice values). This catches trying to nice
	 * to 40 and limits it to 39.
	 */
	if (prio + incr >= PRIO_MAX - 1)
		incr = PRIO_MAX - 1 - prio;
	/*
	 * Let the kernel do the rest of the range checking.
	 */
	if (setpriority(PRIO_PROCESS, 0, prio + incr) < 0)  {
		if (_Geterrno() == EACCES)
			_Seterrno(EPERM);
		RETURN(-1);
	}
	prio = getpriority(PRIO_PROCESS, 0) - PRIZERO;
	RETURN(prio);
}
