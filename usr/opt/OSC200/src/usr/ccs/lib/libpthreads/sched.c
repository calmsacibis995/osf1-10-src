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
static char	*sccsid = "@(#)$RCSfile: sched.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 04:12:54 $";
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

#ifndef	lint
#endif	not lint

/*
 * File: sched.c
 *
 * This file contains all the support for the scheduling control of pthreads
 * which is currently none. As we use kernel threads to run on, we have to get
 * them to behave before we can export this functionality.
 */

#include <pthread.h>
#include "internal.h"
#include <errno.h>

/*
 * Function:
 *	pthread_setprio
 *
 * Parameters:
 *	thread - the thread to alter
 *	prio - the new priority for the thread to run at
 *
 * Return value:
 *	-1	This function is not supported (ENOSYS)
 *
 * Description:
 *	This function is not supported.
 */
int
pthread_setprio(pthread_t thread, int prio)
{
	set_errno(ENOSYS);
	return(-1);
}

/*
 * Function:
 *	pthread_getprio
 *
 * Parameters:
 *	thread - the thread to inquire about
 *
 * Return value:
 *	-1	This function is not supported (ENOSYS)
 *
 * Description:
 *	This function is not supported.
 */
int
pthread_getprio(pthread_t thread)
{
	set_errno(ENOSYS);
	return(-1);
}

/*
 * Function:
 *	pthread_setscheduler
 *
 * Parameters:
 *	thread - the thread to alter
 *	alg - the new scheduling policy
 *	prio - the new priority for the thread to run at
 *
 * Return value:
 *	-1	This function is not supported (ENOSYS)
 *
 * Description:
 *	This function is not supported.
 */
int
pthread_setscheduler(pthread_t thread, int alg, int prio)
{
	set_errno(ENOSYS);
	return(-1);
}

/*
 * Function:
 *	pthread_getscheduler
 *
 * Parameters:
 *	thread - the thread to inquire about
 *
 * Return value:
 *	-1	This function is not supported (ENOSYS)
 *
 * Description:
 *	This function is not supported.
 */
int
pthread_getscheduler(thread)
pthread_t	thread;
{
	set_errno(ENOSYS);
	return(-1);
}
