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
/*	
 *	@(#)$RCSfile: resource.h,v $ $Revision: 4.2.8.2 $ (DEC) $Date: 1993/09/03 18:23:55 $
 */ 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/* 
 * Mach Operating System
 * Copyright (c) 1989 Carnegie-Mellon University
 * Copyright (c) 1988 Carnegie-Mellon University
 * Copyright (c) 1987 Carnegie-Mellon University
 * All rights reserved.  The CMU software License Agreement specifies
 * the terms and conditions for use and redistribution.
 */
/*
 * OSF/1 Release 1.0
 */
/*
 * Copyright (C) 1988,1989 Encore Computer Corporation.  All Rights Reserved
 *
 * Property of Encore Computer Corporation.
 * This software is made available solely pursuant to the terms of
 * a software license agreement which governs its use. Unauthorized
 * duplication, distribution or sale are strictly prohibited.
 *
 */
/*
 * Copyright (c) 1982, 1986 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *

 */

#ifndef	_SYS_RESOURCE_H_
#define _SYS_RESOURCE_H_

#include <sys/time.h>
#include <sys/types.h>

/*
 * Process priority specifications to get/setpriority.
 */
#define PRIO_MIN	-20
#define PRIO_MAX	20

#define PRIO_PROCESS	0
#define PRIO_PGRP	1
#define PRIO_USER	2

/*
 * Resource utilization information.
 */

#define RUSAGE_SELF	0
#define RUSAGE_CHILDREN	-1

struct	rusage {
	struct timeval ru_utime;	/* user time used */
	struct timeval ru_stime;	/* system time used */
	long	ru_maxrss;
#define ru_first	ru_ixrss
	long	ru_ixrss;		/* integral shared memory size */
	long	ru_idrss;		/* integral unshared data " */
	long	ru_isrss;		/* integral unshared stack " */
	long	ru_minflt;		/* page reclaims */
	long	ru_majflt;		/* page faults */
	long	ru_nswap;		/* swaps */
	long	ru_inblock;		/* block input operations */
	long	ru_oublock;		/* block output operations */
	long	ru_msgsnd;		/* messages sent */
	long	ru_msgrcv;		/* messages received */
	long	ru_nsignals;		/* signals received */
	long	ru_nvcsw;		/* voluntary context switches */
	long	ru_nivcsw;		/* involuntary " */
#define ru_last		ru_nivcsw
};

/*
 * Resource limits
 */
#define RLIMIT_CPU	0		/* cpu time in milliseconds */
#define RLIMIT_FSIZE	1		/* maximum file size */
#define RLIMIT_DATA	2		/* data size */
#define RLIMIT_STACK	3		/* stack size */
#define RLIMIT_CORE	4		/* core file size */
#define RLIMIT_RSS	5		/* resident set size */
#define RLIMIT_NOFILE   6               /* open files */
#define RLIMIT_AS	7		/* address space */
#define RLIMIT_VMEM	RLIMIT_AS	/* V.4 alias for AS */

#define RLIM_NLIMITS	8		/* number of resource limits */

#if defined(__alpha)
#define	RLIM_INFINITY	0x7fffffffffffffffL
#else /* !defined(__alpha) */
#define	RLIM_INFINITY	0x7fffffff
#endif /* defined(__alpha) */

struct rlimit {
	rlim_t	rlim_cur;		/* current (soft) limit */
	rlim_t	rlim_max;		/* maximum value for rlim_cur */
};


/*
 *  Special rusage structure returned with WLOGINDEV option to wait3().
 */

struct rusage_dev {
	struct rusage ru_rusage;
	dev_t	      ru_dev;
};

#define RUSAGE_NODEV	((dev_t)-1)	/* same as NODEV */


/*
 *  Resource pause system call definitions
 */

#define RPAUSE_SAME	0		/* leave state unchanged */
#define RPAUSE_DISABLE	1		/* disable pause on error type(s) */
#define RPAUSE_ENABLE	2		/* enable pause on error type(s) */

#define RPAUSE_ALL	0x7fffffff	/* all error number types */

#ifndef _KERNEL
#ifdef 	_NO_PROTO
extern int getpriority();
extern int setpriority();
extern int getrlimit();
extern int setrlimit();
extern int getrusage();
#else	/* _NO_PROTO */
extern int getpriority(int, int);
extern int setpriority(int, int, int);
extern int getrlimit(int, struct rlimit *);
extern int setrlimit(int, struct rlimit *);
extern int getrusage(int, struct rusage *);
#endif	/* _NO_PROTO */
#endif	/* _KERNEL */

#endif	/* _SYS_RESOURCE_H_ */
