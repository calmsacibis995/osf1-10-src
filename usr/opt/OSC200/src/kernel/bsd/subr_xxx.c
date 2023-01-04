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
static char	*sccsid = "@(#)$RCSfile: subr_xxx.c,v $ $Revision: 4.3.3.7 $ (DEC) $Date: 1993/01/20 14:11:16 $";
#endif 
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
 * Copyright (c) 1982, 1986 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 */

#include <cputypes.h>
#include <rt_preempt.h>
#include <ser_compat.h>

#include <vm/vm_map.h>
#include <vm/vm_kern.h>
#include <kern/parallel.h>

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/conf.h>
#include <sys/user.h>
#include <sys/proc.h>
#include <sys/uio.h>

/*
 * Routine placed in illegal entries in the bdevsw and cdevsw tables.
 */
nodev()
{

	return (ENODEV);
}

/*
 * Null routine; placed in insignificant entries
 * in the bdevsw and cdevsw tables.
 */
nulldev()
{

	return (0);
}

/*
 * Null system calls, invalid syscall, or just not configured.
 */
errsys(p, args, retval)
	struct proc *p;
	void *args;
	long *retval;

{

	return (EINVAL);
}

nullsys(p, args, retval)
	struct proc *p;
	void *args;
	long *retval;
{

	return (0);
}

#if	defined(ibmrt) || defined(ns32000) || defined(sun) || defined(i386) || defined(mips) || defined(__hp_osf) || defined(__alpha)
imin(a, b)
{

	return (a < b ? a : b);
}

imax(a, b)
{

	return (a > b ? a : b);
}

unsigned
min(a, b)
	u_int a, b;
{

	return (a < b ? a : b);
}

unsigned
max(a, b)
	u_int a, b;
{

	return (a > b ? a : b);
}
#endif	/* ibmrt || ns32000 || sun || i386 || mips || __hp_osf */

#ifdef	balance
/*
 * Sequent Balance kernel uses calloc() differently.  See sqt/startup.c.
 */
#else
caddr_t calloc(size)
	long size;
{
	return((caddr_t)kmem_alloc(kernel_map, (vm_offset_t)size));
}
#endif

#if PROFILING
/*
 * Stub routine in case it is ever possible to free space.
 */
cfreemem(cp, size)
	caddr_t cp;
	long size;
{
	printf("freeing %x, size %d\n", cp, size);
}
#endif


#if	!defined(vax) && !defined(balance) && !defined(mips) && !defined(i386)

ffs(mask)
	register long mask;
{
	register int i;

	for(i = 1; i <= NSIG; i++) {
		if (mask & 1)
			return (i);
		mask >>= 1;
	}
	return (0);
}

#if !defined(__alpha)
bcmp(s1, s2, len)
	register char *s1, *s2;
	register long len;
{

	while (len--)
		if (*s1++ != *s2++)
			return (1);
	return (0);
}

strlen(s1)
	register char *s1;
{
	register int len;

	if (s1 == 0)
		panic("strlen(NULL)");
	for (len = 0; *s1++ != '\0'; len++)
		/* void */;
	return (len);
}
#endif
#endif	/* !defined(vax) && !defined(balance) && !defined(mips) */

/*
 * Find First Set in a Long word.  Unlike NSIG in the ffs()
 * version, this uses the #of bits in a long word.
 */
ffs_l(mask)
register long mask;
{
	register int i;

	for(i = 1; i <= sizeof(long)*NBBY; i++) {
		if (mask & 1L)
			return (i);
		mask >>= 1;
	}
	return (0);
}


char *
index(strp, c)
register char *strp;
register char c;
{
	for (; *strp; strp++)
		if (*strp == c)
			return(strp);
	return(NULL);
}

rndup(size, minsize)
unsigned long size, minsize;
{
	while (minsize < size)
		minsize <<= 1;
	return(minsize);
}

#if     SER_COMPAT || RT_PREEMPT
ser_funnel(f)
long f;
{
	unix_master();
}

ser_unfunnel(f)
long f;
{
	unix_release();
}
#endif
