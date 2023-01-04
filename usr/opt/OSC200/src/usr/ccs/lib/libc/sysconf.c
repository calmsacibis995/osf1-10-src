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
static char	*sccsid = "@(#)$RCSfile: sysconf.c,v $ $Revision: 4.3.11.3 $ (DEC) $Date: 1993/07/29 17:40:21 $";
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
 * COMPONENT_NAME: (LIBCGEN) Standard C Library General Functions
 *
 * FUNCTIONS: sysconf
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 * OBJECT CODE ONLY SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * sysconf.c	1.6  com/lib/c/gen,3.1,8943 10/27/89 15:54:19
 */

/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#pragma weak realtime_kernel = __realtime_kernel
#pragma weak sysconf = __sysconf
#endif
#include <unistd.h>
#include <time.h>
#include <sys/param.h>
#include <limits.h>
#include <sys/proc.h>
#include <errno.h>
#include <userpw.h>
#include <sys/resource.h>
#include "ts_supp.h"
#define H_RT
#include <sys/habitat.h>
#include <sys/rt_syscall.h>

/*
 *	All of the realtime macros are known by sysconf(), but they are not
 *	valid for a non-realtime kernel.  Therefore, for the realtime macros,
 *	we check at runtime to see what kernel is running, using 
 *	rt_getprio(), which returns a -1 and errno of ENOSYS if the non-
 *      realtime kernel is being used.
 */
int
realtime_kernel()
{
 int num, res;

 num = ((HABITAT_STD_CALL(rt_getprio) - HABITAT_BASE)) | HABITAT_INDEX;
 res = syscall(num, 8L, 0L, 0L, 0L, 0L);
 if (res == -1) {
   errno = 0;
   return(FALSE);
 }
 else
   return(TRUE);
}

/*
 * NAME: sysconf
 *
 * FUNCTION: The sysconf() function provides a method for the application to 
 *	determine the current value of a configurable system limit or option.
 *      The 'name' argument represents the system variable to be queried.
 *	These variables are found in <limits.h> or <unistd.h>.
 *
 * EXECUTION ENVIRONMENT:
 *
 * DATA STRUCTURES:  none
 *
 * RETURNS:
 * 	If 'name' is an invalid value, sysconf() returns a -1; otherwise,
 *      the sysconf() function will return the current variable value on 
 * 	the system.
 *
 * ERRORS:
 *	EINVAL		The value of the 'name' argument is invalid.
 *
 */
long
sysconf(int name)
{
	struct rlimit rl;

	switch (name)
	{
		case _SC_AES_OS_VERSION:
#ifndef _AES_OS_VERSION
	  		return(-1);
#else
			return(_AES_OS_VERSION);
#endif

	  	case _SC_ATEXIT_MAX:
#ifndef ATEXIT_MAX
	  		return(-1);
#else
	  		return(ATEXIT_MAX);
#endif

		case _SC_PAGE_SIZE:
		case _SC_PAGESIZE:
			return (getpagesize());

		case _SC_ARG_MAX: 
		/*
 	 	 * ARG_MAX should really be set to the value of NCARGS so that
	 	 * if ARG_MAX is undefined, a -1 can be returned (like the POSIX
	 	 * document says) and if it is defined, ARG_MAX can be returned
	 	 * (which is really the value that currently exists as NCARGS.
 	 	 */
#ifndef ARG_MAX
			return(-1);
#else
			return(ARG_MAX);
#endif

		case _SC_CHILD_MAX:		 
#ifndef CHILD_MAX
			return(-1);
#else
			/*
 		 	 * root has no limitation of the number of child
 		 	 * processes, so we are returning the number of
			 * process table slots.
 			 */
			if (geteuid() == 0)
				return(CHILD_MAX);
			else
				return(CHILD_MAX);	
#endif

		case _SC_CLK_TCK:
#ifndef CLK_TCK
			return(-1);
#else
			return(CLK_TCK);
#endif

		case _SC_NGROUPS_MAX:
#ifndef NGROUPS_MAX
			return(-1);
#else
			return(NGROUPS_MAX);
#endif

		case _SC_STREAM_MAX:
		case _SC_OPEN_MAX:
			getrlimit(RLIMIT_NOFILE, &rl);
			return(rl.rlim_cur); 

		case _SC_PASS_MAX:
#ifdef PASS_MAX
			return(PASS_MAX);
#else
			return(MAX_PASS);
#endif
                case _SC_TZNAME_MAX:
			return (TZNAME_MAX);

		case _SC_JOB_CONTROL:
#ifdef _POSIX_JOB_CONTROL
			return(TRUE);
#else	
			return(-1);
#endif

		case _SC_SAVED_IDS:
#ifdef _POSIX_SAVED_IDS
			return(TRUE);
#else
			return(-1);
#endif

		case _SC_VERSION:
#ifndef _POSIX_VERSION
			return(-1);
#else
			return(_POSIX_VERSION);
#endif

		case _SC_XOPEN_VERSION:
#ifndef	_XOPEN_VERSION
			return(-1);
#else
			return(_XOPEN_VERSION);
#endif

		case _SC_LISTIO_AIO_MAX:
#ifndef AIO_LISTIO_MAX
			return(-1);
#else
			return(AIO_LISTIO_MAX);
#endif

		case _SC_AIO_MAX:
#ifndef AIO_MAX
			return(-1);
#else
			return(AIO_MAX);
#endif

		case _SC_CLOCKDRIFT_MAX:
			if (!realtime_kernel()) return(-1);
#ifndef CLOCKDRIFT_MAX
			return(-1);
#else
			return(CLOCKDRIFT_MAX);
#endif

		case _SC_DELAYTIMER_MAX:
			if (!realtime_kernel()) return(-1);
#ifndef DELAYTIMER_MAX
			return(-1);
#else
			return(DELAYTIMER_MAX);
#endif

		case _SC_RTSIG_MAX:
			if (!realtime_kernel()) return(-1);
#ifndef RTSIG_MAX
			return(-1);
#else
			return(RTSIG_MAX);
#endif

		case _SC_SEM_NAME_MAX:
			if (!realtime_kernel()) return(-1);
#ifndef SEM_NAME_MAX
			return(-1);
#else
			return(SEM_NAME_MAX);
#endif

		case _SC_SEM_NSEMS_MAX:
			if (!realtime_kernel()) return(-1);
#ifndef SEM_NSEMS_MAX
			return(-1);
#else
			return(SEM_NSEMS_MAX);
#endif

		case _SC_SEM_NSETS_MAX:
			if (!realtime_kernel()) return(-1);
#ifndef SEM_NSETS_MAX
			return(-1);
#else
			return(SEM_NSETS_MAX);
#endif

		case _SC_TIMER_MAX:
			if (!realtime_kernel()) return(-1);
#ifndef TIMER_MAX
			return(-1);
#else
			return(TIMER_MAX);
#endif

		case _SC_ASYNCHRONOUS_IO:
#ifndef _POSIX_ASYNCHRONOUS_IO
			return(-1);
#else
			return(TRUE);
#endif

		case _SC_BINARY_SEMAPHORES:
			if (!realtime_kernel()) return(-1);
#ifndef _POSIX_BINARY_SEMAPHORES
			return(-1);
#else
			return(TRUE);
#endif

		case _SC_FSYNC:
			if (!realtime_kernel()) return(-1);
#ifndef _POSIX_FSYNC
			return(-1);
#else
			return(TRUE);
#endif

		case _SC_MAPPED_FILES:
			if (!realtime_kernel()) return(-1);
#ifndef _POSIX_MAPPED_FILES
			return(-1);
#else
			return(TRUE);
#endif

		case _SC_MEMLOCK:
			if (!realtime_kernel()) return(-1);
#ifndef _POSIX_MEMLOCK
			return(-1);
#else
			return(TRUE);
#endif

		case _SC_MEMLOCK_RANGE:
			if (!realtime_kernel()) return(-1);
#ifndef _POSIX_MEMLOCK_RANGE
			return(-1);
#else
			return(TRUE);
#endif

		case _SC_MEMORY_PROTECTION:
			if (!realtime_kernel()) return(-1);
#ifndef _POSIX_MEMORY_PROTECTION
			return(-1);
#else
			return(TRUE);
#endif

		case _SC_MESSAGE_PASSING:
			if (!realtime_kernel()) return(-1);
#ifndef _POSIX_MESSAGE_PASSING
			return(-1);
#else
			return(TRUE);
#endif

		case _SC_PRIORITIZED_IO:
			if (!realtime_kernel()) return(-1);
#ifndef _POSIX_PRIORITIZED_IO
			return(-1);
#else
			return(TRUE);
#endif

		case _SC_PRIORITY_SCHEDULING:
			if (!realtime_kernel()) return(-1);
#ifndef _POSIX_PRIORITY_SCHEDULING
			return(-1);
#else
			return(TRUE);
#endif

		case _SC_REALTIME_FILES:
			if (!realtime_kernel()) return(-1);
#ifndef _POSIX_REALTIME_FILES
			return(-1);
#else
			return(TRUE);
#endif

		case _SC_REALTIME_SIGNALS:
			if (!realtime_kernel()) return(-1);
#ifndef _POSIX_REALTIME_SIGNALS
			return(-1);
#else
			return(TRUE);
#endif

		case _SC_SHARED_MEMORY_OBJECTS:
			if (!realtime_kernel()) return(-1);
#ifndef _POSIX_SHARED_MEMORY_OBJECTS
			return(-1);
#else
			return(TRUE);
#endif

		case _SC_SYNCHRONIZED_IO:
			if (!realtime_kernel()) return(-1);
#ifndef _POSIX_SYNCHRONIZED_IO
			return(-1);
#else
			return(TRUE);
#endif

		case _SC_TIMERS:
			if (!realtime_kernel()) return(-1);
#ifndef _POSIX_TIMERS
			return(-1);
#else
			return(TRUE);
#endif

		case _SC_4VERSION:
#ifndef _POSIX4_VERSION
			return(-1);
#else
			return(_POSIX4_VERSION);
#endif

		case _SC_BC_BASE_MAX:

#ifndef BC_BASE_MAX
			return(-1);
#else
			return(BC_BASE_MAX);
#endif
		case _SC_BC_DIM_MAX:

#ifndef BC_DIM_MAX
			return(-1);
#else
			return(BC_DIM_MAX);
#endif
		case _SC_BC_SCALE_MAX:

#ifndef BC_SCALE_MAX
			return(-1);
#else
			return(BC_SCALE_MAX);
#endif
		case _SC_BC_STRING_MAX:

#ifndef BC_STRING_MAX
			return(-1);
#else
			return(BC_STRING_MAX);
#endif
		case _SC_COLL_WEIGHTS_MAX:

#ifndef COLL_WEIGHTS_MAX
			return(-1);
#else
			return(COLL_WEIGHTS_MAX);
#endif
		case _SC_EXPR_NEST_MAX:

#ifndef EXPR_NEST_MAX
			return(-1);
#else
			return(EXPR_NEST_MAX);
#endif
		case _SC_LINE_MAX:

#ifndef LINE_MAX
			return(-1);
#else
			return(LINE_MAX);
#endif
		case _SC_2_C_BIND:

#ifndef _POSIX2_C_BIND
			return(-1);
#else
			return(_POSIX2_C_BIND);
#endif
		case _SC_2_C_DEV:

#ifndef _POSIX2_C_DEV
			return(-1);
#else
			return(_POSIX2_C_DEV);
#endif
		case _SC_2_C_VERSION:

#ifndef _POSIX2_C_VERSION
			return(-1);
#else
			return(_POSIX2_C_VERSION);
#endif
		case _SC_2_CHAR_TERM:

#ifndef _POSIX2_CHAR_TERM
			return(-1);
#else
			return(_POSIX2_CHAR_TERM);
#endif
		case _SC_2_FORT_DEV:

#ifndef _POSIX2_FORT_DEV
			return(-1);
#else
			return(_POSIX2_FORT_DEV);
#endif
		case _SC_2_FORT_RUN:

#ifndef _POSIX2_FORT_RUN
			return(-1);
#else
			return(_POSIX2_FORT_RUN);
#endif
		case _SC_2_LOCALEDEF:

#ifndef _POSIX2_LOCALEDEF
			return(-1);
#else
			return(_POSIX2_LOCALEDEF);
#endif
		case _SC_2_SW_DEV:

#ifndef _POSIX2_SW_DEV
			return(-1);
#else
			return(_POSIX2_SW_DEV);
#endif
		case _SC_2_UPE:

#ifndef _POSIX2_UPE
			return(-1);
#else
			return(_POSIX2_UPE);
#endif
		case _SC_2_VERSION:

#ifndef _POSIX2_VERSION
			return(-1);
#else
			return(_POSIX2_VERSION);
#endif
		case _SC_RE_DUP_MAX:

#ifndef RE_DUP_MAX
			return(-1);
#else
			return(RE_DUP_MAX);
#endif

		case _SC_XOPEN_CRYPT:

#ifndef _XOPEN_CRYPT
			return(-1);
#else
			return(_XOPEN_CRYPT);
#endif

		case _SC_XOPEN_ENH_I18N:

#ifndef _XOPEN_ENH_I18N
			return(-1);
#else
			return(_XOPEN_ENH_I18N);
#endif

		case _SC_XOPEN_SHM:

#ifndef _XOPEN_SHM
			return(-1);
#else
			return(_XOPEN_SHM);
#endif

		case _SC_XOPEN_XCU_VERSION:

#ifndef _XOPEN_XCU_VERSION
			return(-1);
#else
			return(_XOPEN_XCU_VERSION);
#endif

		default:
			TS_SETERR(EINVAL);
			return(-1);
	}

}  /* end sysconf */
