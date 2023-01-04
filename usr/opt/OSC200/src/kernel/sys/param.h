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
 *	@(#)$RCSfile: param.h,v $ $Revision: 4.3.9.6 $ (DEC) $Date: 1993/12/09 21:04:48 $
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
 * Copyright (c) 1982, 1986, 1989 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 */

/*
 * Modification History
 *
 * 27-Oct-91	Fred Canter
 *		Make all SYSV IPC definitions configurable.
 *
 * 6-June-1991	Brian Stevens
 *		Removed static initialization of MAXUPRC so that its
 *		value can be picked up from the config file.
 */

#ifndef	_SYS_PARAM_H_
#define _SYS_PARAM_H_

#define BSD	198911		/* system version (year & month) */
#define BSD4_3  1

#ifdef _KERNEL
#include <mach_assert.h>
#endif
#include <sys/types.h>
#include <sys/limits.h>
#include <machine/machparam.h>

/*
 * Machine-independent constants
 */
#define NMOUNT	128		/* number of mountable file systems */
/* NMOUNT must be <= 255 unless c_mdev (cmap.h) is expanded */
#define MSWAPX	NMOUNT		/* pseudo mount table index for swapdev */
/*
 * The default number of per-process file descriptors is configurable.
 * The getdtablesize(2) system call or the sysconf(3) interface should be 
 * used to obtain the current limit.  OPEN_MAX_SOFT is the per-process
 * limit by default.  The system admin can change it via param.c.  A user
 * process can also change its limit with setrlimit(2) but is bounded by 
 * its hard limit (OPEN_MAX_HARD by default).  OPEN_MAX_SYSTEM is the 
 * absolute high water mark that the system admin can configure.  64 is
 * the low water mark.
 */
#define OPEN_MAX_SYSTEM 4096
#define OPEN_MAX_HARD   4096
#define OPEN_MAX_SOFT   4096
#define	NOFILE	64		/* max open files per process - OBSOLETE */
#define MAXLINK LINK_MAX		/* ??? */
#define	CANBSIZ	MAX_CANON		/* max size of typewriter line */
#define	NCARGS	(ARG_MAX + (2 * PATH_MAX))
				/* # characters in exec arglist */
#define	MAXINTERP	(PATH_MAX + 2)	/* maximum interpreter file name length */
#define NGROUPS	NGROUPS_MAX	/* max number groups */
#define MAXHOSTNAMELEN	64	/* maximum hostname size */

#define NOGROUP	65535		/* marker for empty group set member */

#if	(PIPE_BUF * 2) < 4096		/* sys/syslimits.h */
/* If PIPSIZ is set to < 4096 experience shows that many applications
 * deadlock. Note that PIPE_BUF is the write atomicity limit. */
#define	PIPSIZ	4096
#else
#define	PIPSIZ	(PIPE_BUF * 2)
#endif

/*
 * Priorities
 */
#define PSWP	0
#define PINOD	10
#define PRIBIO	20
#define PVFS	22
#define PRIUBA	24
#define PZERO	25
#define PPIPE	26
#define PWAIT	30
#define PLOCK	35
#define PSLEP	40
#define PUSER	50
#define PMASK	0177
#define PCATCH	0400	/* implies interruptible sleep */

#define PRIZERO	0	/* The kernel version of NZERO */

/*
 * Signals
 */
#if	!(defined(LOCORE) && defined(ibmrt))
#include <sys/signal.h>
#endif	/* !(defined(LOCORE) && defined(ibmrt)) */

#ifdef	_KERNEL
/*
 * Constants passed to mpsleep, the MP-safe sleep function.
 */
#define MS_LOCK_SIMPLE		0x0001		/* simple lock */
#define MS_LOCK_READ		0x0002		/* read lock */
#define MS_LOCK_WRITE		0x0004		/* write lock */
#define MS_LOCK_ON_ERROR	0x1000		/* lock on error return */

/*
 * Non-assertion macros for sleep and tsleep.  MP-safe versions.
 */
#if	!MACH_ASSERT
#define sleep(chan, pri) (void) mpsleep(chan, pri, "Zzzzzz", 0, (void *)NULL, 0)
#define tsleep(chan, pri, wmesg, timo) \
	mpsleep(chan, pri, wmesg, timo, (void *) NULL, 0)
#endif

/*
 *	Check for per-process and per thread signals.
 *	Must be MP-safe.
 */
#define SHOULDissig(p,uthreadp) \
	(((p)->p_sig | (uthreadp)->uu_sig) && ((p)->p_flag&STRC || \
	 (((p)->p_sig | (uthreadp)->uu_sig) &~ (p)->p_sigmask)))

/*
 *	Check for signals, handling possible stop signals.
 *	Ignores signals already 'taken' and per-thread signals.
 *	Use before and after thread_block() in sleep().
 *	(p) is always current process.
 */
#define ISSIG(p) (thread_should_halt(current_thread()) || \
	 (SHOULDissig(p,current_thread()->u_address.uthread) && issig()))

/*
 *	Check for signals, including signals already taken and
 *	per-thread signals.  Use in trap() and syscall() before
 *	exiting kernel.
 *	Must be MP-safe.
 */
#define CHECK_SIGNALS(p, thread, uthreadp)	\
	(!thread_should_halt(thread)		\
	 && ((p)->p_cursig		\
	     || SHOULDissig(p,uthreadp)))

#endif	/* _KERNEL */

/*
 * Machine type dependent parameters.
 */

#define NBPW	sizeof(long)	/* sizeof (char *) */

#ifndef NULL
#define	NULL	0L
#endif	/* NULL */
#define CMASK	022		/* default mask for file creation */
#define NODEV	(dev_t)(-1)

/*
 * Clustering of hardware pages on machines with ridiculously small
 * page sizes is done here.  The paging subsystem deals with units of
 * CLSIZE pte's describing NBPG (from machine/machparam.h) pages each.
 *
 * NOTE: SSIZE, SINCR and UPAGES must be multiples of CLSIZE
 */
#define CLBYTES		(CLSIZE*NBPG)
#define CLOFSET		(CLSIZE*NBPG-1)	/* for clusters, like PGOFSET */
#define claligned(x)	((((int)(x))&CLOFSET)==0)
#define CLOFF		CLOFSET
#define CLSHIFT		(PGSHIFT+CLSIZELOG2)

#if	CLSIZE==1
#define clbase(i)	(i)
#define clrnd(i)	(i)
#else
/* give the base virtual address (first of CLSIZE) */
#define clbase(i)	((i) &~ (CLSIZE-1))
/* round a number of clicks up to a whole cluster */
#define clrnd(i)	(((i) + (CLSIZE-1)) &~ (CLSIZE-1))
#endif

/* CBLOCK is the size of a clist block, must be power of 2 */
#define CBLOCK	64
#define CBQSIZE	(CBLOCK/NBBY)	/* quote bytes/cblock - can do better */
#define	CBSIZE	(CBLOCK - sizeof(struct cblock *) - CBQSIZE) /* data chars/clist */
#define CROUND	(CBLOCK - 1)				/* clist rounding */

/* System V IPC definitions */
/* Note: these defaults can be overridden from the kernel config file */

/* messages */
#ifndef MSGMAX
#define MSGMAX   8192
#endif
#ifndef	MSGMNB
#define MSGMNB   16384
#endif
#ifndef	MSGMNI
#define MSGMNI   50
#endif
#ifndef	MSGTQL
#define MSGTQL   40
#endif

/* semaphores */
#ifndef	SEMMNI
#define SEMMNI   10
#endif
#ifndef	SEMMNS
#define SEMMNS   60
#endif
#ifndef	SEMMSL
#define SEMMSL   25
#endif
#ifndef	SEMOPM
#define SEMOPM   10
#endif
#ifndef	SEMUME
#define SEMUME   10
#endif
#ifndef	SEMVMX
#define SEMVMX   32767
#endif
#ifndef	SEMAEM
#define SEMAEM   16384
#endif

/* shared memory (defaults increased for ULTRIX/SQL) */
#ifndef	SHMMIN
#define SHMMIN	 1
#endif
#ifndef	SHMMAX
#define SHMMAX	 (4*1024*1024)	/* OSF default is (128*1024) */
#endif
#ifndef	SHMMNI
#define SHMMNI	 100
#endif
#ifndef	SHMSEG
#define SHMSEG	 32		/* OSF default is 6 */
#endif

/*
 * File system parameters and macros.
 *
 * The file system is made out of blocks of at most MAXBSIZE units,
 * with smaller units (fragments) only in the last direct block.
 * MAXBSIZE primarily determines the size of buffers in the buffer
 * pool. It may be made larger without any effect on existing
 * file systems; however making it smaller make make some file
 * systems unmountable.
 *
 * Note that the blocked devices are assumed to have DEV_BSIZE
 * "sectors" and that fragments must be some multiple of this size.
 * Block devices are read in BLKDEV_IOSIZE units. This number must
 * be a power of two and in the range of
 *	DEV_BSIZE <= BLKDEV_IOSIZE <= MAXBSIZE
 * This size has no effect upon the file system, but is usually set
 * to the block size of the root file system, so as to maximize the
 * speed of ``fsck''.
 */
#define MAXBSIZE	8192
#if	defined(exl) || defined(__hp_osf)
#define DEV_BSIZE	1024
#define DEV_BSHIFT	10
#else	/* exl || hp_osf */
#define DEV_BSIZE	512
#define DEV_BSHIFT	9		/* log2(DEV_BSIZE) */
#endif	/* exl || hp_osf */
#define BLKDEV_IOSIZE	2048
#define MAXFRAG 	8

#define btodb(bytes)	 		/* calculates (bytes / DEV_BSIZE) */ \
	((unsigned long)(bytes) >> DEV_BSHIFT)
#define dbtob(db)			/* calculates (db * DEV_BSIZE) */ \
	((unsigned long)(db) << DEV_BSHIFT)

/*******
*
*  UBSIZE is the value that commands and libs will use to present 
*  file/blocks to the user. It is now set to 1k blocksize.
*
*******/

#define UBSIZE	1024
#define UBSHIFT 10	/* LOG2(UBSIZE) */

/*
 * MAXPATHLEN defines the longest permissable path length
 * after expanding symbolic links. It is used to allocate
 * a temporary buffer from the buffer pool in which to do the
 * name expansion, hence should be a power of two, and must
 * be less than or equal to MAXBSIZE.
 * MAXSYMLINKS defines the maximum number of symbolic links
 * that may be expanded in a path name. It should be set high
 * enough to allow all legitimate uses, but halt infinite loops
 * reasonably quickly.
 */
#define	MAXPATHLEN	(PATH_MAX+1)
#define MAXSYMLINKS	32

/*
 * Constants for setting the parameters of the kernel memory allocator.
 *
 * 2 ** MINBUCKET is the smallest unit of memory that will be
 * allocated. It must be at least large enough to hold a pointer.
 *
 * Units of memory less or equal to MAXALLOCSAVE will permanently
 * allocate physical memory; requests for these size pieces of
 * memory are quite fast. Allocations greater than MAXALLOCSAVE must
 * always allocate and free physical memory; requests for these
 * size allocations should be done infrequently as they will be slow.
 * Constraints: CLBYTES <= MAXALLOCSAVE <= 2 ** (MINBUCKET + 14)
 * and MAXALLOCSIZE must be a power of two.
 */
#define MINBUCKET	4		/* 4 => min allocation of 16 bytes */
#define MAXALLOCSAVE	(2 * CLBYTES)

/*
 * bit map related macros
 */

#define setbit(a,i)	(*(((char *)(a)) + ((i)/NBBY)) |= 1<<((i)%NBBY))
#define clrbit(a,i)	(*(((char *)(a)) + ((i)/NBBY)) &= ~(1<<((i)%NBBY)))
#define isset(a,i)	(*(((char *)(a)) + ((i)/NBBY)) & (1<<((i)%NBBY)))
#define isclr(a,i)      ((*(((char *)(a)) + ((i)/NBBY)) & (1<<((i)%NBBY))) == 0)

#if	!defined(vax) && !defined(i386)
#define _bit_set(i,a)   setbit(a,i)
#define _bit_clear(i,a)	clrbit(a,i)
#define _bit_tst(i,a)	isset(a,i)
#endif	/* !defined(vax) && !defined(i386) */


/*
 * Macros for fast min/max.
 */
#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))

/*
 * Macros for counting and rounding.
 */
#ifndef	howmany
#define howmany(x, y)	(((x)+((y)-1))/(y))
#endif
#define roundup(x, y)	((((x)+((y)-1))/(y))*(y))
#define powerof2(x)	((((x)-1)&(x))==0)

/*
 * Scale factor for scaled integers used to count %cpu time and load avgs.
 *
 * The number of CPU `tick's that map to a unique `%age' can be expressed
 * by the formula (1 / (2 ^ (FSHIFT - 11))).  The maximum load average that
 * can be calculated (assuming 32 bits) can be closely approximated using
 * the formula (2 ^ (2 * (16 - FSHIFT))) for (FSHIFT < 15).
 *
 * For the scheduler to maintain a 1:1 mapping of CPU `tick' to `%age',
 * FSHIFT must be at least 11; this gives us a maximum load avg of ~1024.
 */
#define	FSHIFT	11		/* bits to right of fixed binary point */
#define FSCALE	(1<<FSHIFT)

#define MAXDOMNAMELEN	256		/* maximum domain name length */

#endif	/* _SYS_PARAM_H_ */
