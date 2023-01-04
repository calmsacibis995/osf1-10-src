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
 *	@(#)$RCSfile: mman.h,v $ $Revision: 4.2.9.4 $ (DEC) $Date: 1993/07/15 18:28:26 $
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

#ifndef	_SYS_MMAN_H_
#define _SYS_MMAN_H_

/* protections are chosen from these bits, or-ed together */
#define PROT_NONE	0		/* no access to these pages */
#define PROT_READ	0x1		/* pages can be read */
#define PROT_WRITE	0x2		/* pages can be written */
#define PROT_EXEC	0x4		/* pages can be executed */

/* flags contain sharing type, mapping type, and options */

/* mapping visibility: choose either SHARED or PRIVATE */
#define MAP_SHARED	0x1		/* share changes */
#define MAP_PRIVATE	0x2		/* changes are private */

/* mapping region: choose either FILE or ANONYMOUS */
#define	MAP_FILE	0x00		/* map from a file */
#define	MAP_ANONYMOUS	0x10		/* man an unnamed region */
#define	MAP_ANON	0x10		/* man an unnamed region */
#define	MAP_TYPE	0xf0		/* the type of the region */

/* mapping placement: choose either FIXED or VARIABLE */
#define	MAP_FIXED	0x100		/* map addr must be exactly as specified */
#define	MAP_VARIABLE	0x00		/* system can place new region */

/* other flags */

#define	MAP_HASSEMAPHORE 0x0200		/* region may contain semaphores */
#define	MAP_INHERIT	0x0400		/* region is retained after exec */
#define MAP_UNALIGNED	0x0800		/* allow non-page-aligned file offset */

/* advice to madvise */
#define MADV_NORMAL	0		/* no further special treatment */
#define MADV_RANDOM	1		/* expect random page references */
#define MADV_SEQUENTIAL	2		/* expect sequential page references */
#define MADV_WILLNEED	3		/* will need these pages */
#define MADV_DONTNEED	4		/* dont need these pages */
#define	MADV_SPACEAVAIL	5		/* ensure that resources are available */

/* functions to memctl */
#define	MC_SYNC		1		/* sync with backing store */
#define	MC_LOCK		2		/* lock pages in memory */
#define	MC_UNLOCK	3		/* unlock pages from memory */
#define	MC_LOCKAS	4		/* lock address space in memory */
#define	MC_UNLOCKAS	5		/* unlock address space from memory */

/* additional attr flags used by memcntl */
#define SHARED		0x10
#define PRIVATE		0x20
#define PROC_TEXT	(PROT_EXEC|PROT_READ)
#define PROC_DATA	(PROT_WRITE|PROT_READ)

/* msem conditions and structure */
typedef struct {
	int msem_state;		/* The semaphore is locked is non-zero. */
	int msem_wanted;	/* Threads are waiting on the semaphore. */
}msemaphore;

#define MSEM_UNLOCKED 	0	/* Initialize the semahore to unlocked */
#define MSEM_LOCKED 	1	/* Initialize the semahore to locked */
#define MSEM_IF_NOWAIT	2	/* Do not wait if semaphore is locked */
#define MSEM_IF_WAITERS	3	/* Unlock only if there are waiters */

/* msync flags */
#define MS_ASYNC 1		/* Asynchronous cache flush */
#define MS_SYNC  2		/* Synchronous cache flush */
#define MS_INVALIDATE 4		/* Invalidate cached pages */

#if !defined(_NO_PROTO) && !defined(KERNEL)

extern int	madvise(caddr_t addr, size_t len, int behav);
extern caddr_t	mmap(caddr_t addr, size_t len, int prot, int flags, int filedes, off_t off);
extern int	mprotect(caddr_t addr, size_t len, int prot);
extern msemaphore 	*msem_init(msemaphore *sem, int initial_value);
extern int	msem_lock(msemaphore *sem, int condition);
extern int	msem_remove(msemaphore *sem);
extern int	msem_unlock(msemaphore *sem, int condition);
extern int	msync(caddr_t addr, size_t len, int flags);
extern int	munmap(caddr_t addr, size_t len);
extern int	mvalid(caddr_t addr, size_t len, int prot);
extern int      shm_open(const char *path, int oflag, mode_t mode);
extern int      shm_unlink(const char *path);

#else /* _NO_PROTO || KERNEL */

extern int	madvise();
extern caddr_t	mmap();
extern int	mprotect();
extern msemaphore 	*msem_init();
extern int	msem_lock();
extern int	msem_remove();
extern int	msem_unlock();
extern int	msync();
extern int	munmap();
extern int	mvalid();
extern int      shm_open();
extern int      shm_unlink();
#endif /* _NO_PROTO || KERNEL */

#define MCL_CURRENT   8192  /* Lock all currently mapped paged */
#define MCL_FUTURE   16384  /* Lock all additions to address space */

#ifdef _POSIX_4SOURCE
#define REGLOCK    	16  /* Lock region */

#ifndef _NO_PROTO
extern int	memlk(int, void *, size_t);
extern int	memunlk(int, void *, size_t);
extern int	mlockall(int);
extern int	munlockall(void);
extern int	mlock(void *, size_t);
extern int	munlock(void *, size_t);
#else
extern int	memlk();
extern int	memunlk();
extern int	mlockall();
extern int	munlockall();
extern int	mlock();
extern int	munlock();
#endif

/*
 * POSIX and SVID differ on how mlockall() handles a flags field if
 * neither MCL_CURRENT nor MCL_FUTURE are set. The following macro follows
 * the SVID behavior by setting the TYPE field in the memlk() to NULL (EINVAL).
 * If POSIX is accepted, TYPE should be set to NOP.
*/
#define mlockall(flags) memlk((flags)&(MCL_CURRENT|MCL_FUTURE), NULL, NULL)
#define munlockall() 	memunlk((MCL_CURRENT|MCL_FUTURE), NULL, NULL)
#define mlock(addr, size) 	memlk(REGLOCK, (addr), (size))
#define munlock(addr, size) 	memunlk(REGLOCK, (addr), (size))

#endif /* _POSIX_4SOURCE */

#endif	/* _SYS_MMAN_H_ */
