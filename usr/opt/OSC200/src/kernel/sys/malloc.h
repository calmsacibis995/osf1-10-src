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
 * @(#)$RCSfile: malloc.h,v $ $Revision: 1.1.3.5 $ (DEC) $Date: 1993/12/15 22:11:51 $
 */
/*
 * Copyright (c) 1987 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted provided
 * that: (1) source distributions retain this entire copyright notice and
 * comment, and (2) distributions including binaries display the following
 * acknowledgement:  ``This product includes software developed by the
 * University of California, Berkeley and its contributors'' in the
 * documentation or other materials provided with the distribution and in
 * all advertising materials mentioning features or use of this software.
 * Neither the name of the University nor the names of its contributors may
 * be used to endorse or promote products derived from this software without
 * specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 *
 *	Base: malloc.h	7.16 (Berkeley) 6/28/90
 */

#ifndef	_SYS_MALLOC_H
#define _SYS_MALLOC_H

/*
 * flags to malloc
 */
#define M_WAITOK	0x0000
#define M_NOWAIT	0x0001

/*
 * Types of memory to be allocated.
 */
#define	M_FREE		0	/* should be on free list */
#define M_MBUF		1	/* mbuf */
#define	M_CLUSTER	2	/* mbuf cluster page */
#define	M_SOCKET	3	/* socket structure */
#define	M_PCB		4	/* protocol control block */
#define	M_RTABLE	5	/* routing tables */
#define	M_FTABLE	6	/* fragment reassembly header */
#define	M_IFADDR	7	/* interface address */
#define	M_SOOPTS	8	/* socket options */
#define	M_SONAME	9	/* socket name */
#define	M_MBLK		10	/* mblk */
#define	M_MBDATA	11	/* mblk data */
#define	M_STRHEAD	12	/* Stream head */
#define	M_STRQUEUE	13	/* Streams queue pair */
#define	M_STRQBAND	14	/* Streams queue band */
#define	M_STRMODSW	15	/* Streams modsw */
#define	M_STRSIGS	16	/* Streams setsig */
#define	M_STRPOLLS	17	/* Streams poll */
#define	M_STROSR	18	/* Streams osr */
#define	M_STRSQ		19	/* Streams synch queue */
#define	M_STREAMS	20	/* misc Streams memory */
#define M_IOV		21	/* large iov's */
#define M_FHANDLE	22	/* network file handle */
#define	M_NFSREQ	23	/* NFS request header */
#define	M_NFSMNT	24	/* NFS mount structure */
#define M_FILE		25	/* file struct  */
#define M_FILEDESC	26	/* filedesc struct */
#define M_IOCTLOPS	27	/* ioctl data */
#define M_SELPOLL	28	/* select/poll arrays */
#define M_DEVBUF	29	/* device driver memory */
#define M_PATHNAME	30	/* pathname */
#define M_KTABLE	31	/* kernel table */
#define M_IPMOPTS	32	/* ip multicast options */
#define M_IPMADDR	33	/* ip multicast address */
#define M_IFMADDR	34	/* interface multicast addess */
#define M_MRTABLE	35	/* multicast routing table */
/* Available */
#define M_KALLOC	48	/* kalloc - obsolescent */
#define M_TEMP		49	/* misc temporary data buffers */
#define M_LAST		50

#define KMEMNAMSZ       12
#define INITKMEMNAMES { \
        "FREE",         /* 0 M_FREE */ \
        "MBUF",         /* 1 M_MBUF */ \
        "MCLUSTER",     /* 2 M_CLUSTER */ \
        "SOCKET",       /* 3 M_SOCKET */ \
        "PCB",          /* 4 M_PCB */ \
        "ROUTETBL",     /* 5 M_RTABLE */ \
        "FRAGTBL",      /* 6 M_FTABLE */ \
        "IFADDR",       /* 7 M_IFADDR */ \
        "SOOPTS",       /* 8 M_SOOPTS */ \
        "SONAME",       /* 9 M_SONAME */ \
        "MBLK",         /* 10 M_MBLK */ \
        "MBLKDATA",     /* 11 M_MBDATA */ \
        "STRHEAD",      /* 12 M_STRHEAD */ \
        "STRQUEUE",     /* 13 M_STRQUEUE */ \
        "STRQBAND",     /* 14 M_STRQBAND */ \
        "STRMODSW",     /* 15 M_STRMODSW */ \
        "STRSIGS",      /* 16 M_STRSIGS */ \
        "STRPOLL",      /* 17 M_STRPOLLS */ \
        "STROSR",       /* 18 M_STROSR */ \
        "STRSYNCQ",     /* 19 M_STRSQ */ \
        "STREAMS",      /* 20 M_STREAMS */ \
        "IOV",          /* 21 M_IOV */ \
        "FHANDLE",      /* 22 M_FHANDLE */ \
        "NFS REQ",      /* 23 M_NFSREQ */ \
        "NFS MOUNT",    /* 24 M_NFSMNT */ \
        "FILE",         /* 25 M_FILE */ \
        "FILE DESC",    /* 26 M_FILEDESC */ \
        "IOCTLOPS",     /* 27 M_IOCTLOPS */ \
        "SELECT/POLL",  /* 28 M_SELPOLL */ \
        "DEVBUF",       /* 29 M_DEVBUF */ \
        "PATHNAME",     /* 30 M_PATHNAME */ \
        "KERNEL TABLE", /* 31 M_KTABLE */ \
	"IPM OPT",	/* 32 M_IPMOPTS */ \
	"IPM ADDR",	/* 33 M_IPMADDR */ \
	"IFM ADDR",	/* 34 M_IFMADDR */ \
	"MCAST RTTBL",	/* 35 M_MRTABLE */ \
        "", "", "", "", "", "", "", "", "", "", "", "", \
        "KALLOC",       /* 48 M_KALLOC */ \
        "TEMP",         /* 49 M_TEMP */ \
}

#ifdef	_KERNEL
#include <kern/lock.h>
#include <mach/vm_param.h>
#include <kern/thread.h>
#include <kern/sched_prim.h>
#endif

struct kmemtypes {
	long	kt_memuse;	/* # of bytes of this type in use */
	long	kt_limit;	/* # of bytes of this type allowed to exist */
	long	kt_wait;	/* blocking flag for this type of memory */
	long	kt_limblocks;	/* stat: # of times blocked for hitting limit */
	long	kt_failed;	/* stat: # of times failed for hitting
							limit with NOWAIT */
        long    kt_maxused;     /* stat: max # of bytes of this type ever used*/
#ifdef  _KERNEL
	simple_lock_data_t	kt_lock;
#endif
};

/*
 * Set of buckets for each size of memory block that is retained
 */
struct kmembuckets {
	void *	kb_next;	/* list of free blocks */
	long	kb_indx;	/* this buckets index */
	long	kb_size;	/* bucket element size */
	long	kb_totalfree;	/* # of free elements in this bucket */
	long	kb_total;	/* # of elements allocated at to this bucket
							 any given time */
	long	kb_elmpercl;	/* # of elements in this sized allocation */
	long	kb_highwat;	/* # of elements allowed to keep before gc */
	long	kb_calls;	/* stat: total calls to allocate this size */
	long	kb_couldfree;	/* stat: # of pages over high water mark
							 and could free */
	long	kb_failed;	/* stat: # of times failed due to NOWAIT */
	long	kb_noelem;	/* stat: # of times that (kb_next == NULL) */
	long	kb_borrowed;	/* stat: # of times that this bucket borrowed */
	long	kb_lent;	/* stat: # of times that this bucket lent */
#ifdef	_KERNEL
	simple_lock_data_t	kb_lock;
#endif
};

/*
 * Array of descriptors that describe the contents of each page
 */
struct kmemusage {
	struct kmembuckets *ku_kbp; /* pointer to bucket */
	union {
		long	freecnt;/* for small allocations, free pieces in page */
		long	size;	/* for large allocations, size in bytes */
	} ku_un;
};
#define ku_freecnt ku_un.freecnt
#define ku_size ku_un.size


#ifdef _KERNEL
#define MINALLOCSIZE	(1 << MINBUCKET)

#define BUCKETINDX(size) \
	( (size) <= (MINALLOCSIZE * 128) \
		? (size) <= (MINALLOCSIZE * 8) \
			? (size) <= (MINALLOCSIZE * 2) \
				? (size) <= (MINALLOCSIZE * 1) \
					? (MINBUCKET + 0) \
					: (MINBUCKET + 1) \
				: (size) <= (MINALLOCSIZE * 4) \
					? (MINBUCKET + 2) \
					: (MINBUCKET + 3) \
			: (size) <= (MINALLOCSIZE* 32) \
				? (size) <= (MINALLOCSIZE * 16) \
					? (MINBUCKET + 4) \
					: (MINBUCKET + 5) \
				: (size) <= (MINALLOCSIZE * 64) \
					? (MINBUCKET + 6) \
					: (MINBUCKET + 7) \
		: (size) <= (MINALLOCSIZE * 2048) \
			? (size) <= (MINALLOCSIZE * 512) \
				? (size) <= (MINALLOCSIZE * 256) \
					? (MINBUCKET + 8) \
					: (MINBUCKET + 9) \
				: (size) <= (MINALLOCSIZE * 1024) \
					? (MINBUCKET + 10) \
					: (MINBUCKET + 11) \
			: (size) <= (MINALLOCSIZE * 8192) \
				? (size) <= (MINALLOCSIZE * 4096) \
					? (MINBUCKET + 12) \
					: (MINBUCKET + 13) \
				: (size) <= (MINALLOCSIZE * 16384) \
					? (MINBUCKET + 14) \
					: (MINBUCKET + 15) )

#define BUCKETP(size) \
	(&bucket[BUCKETINDX((size))])
/*
 * Turn virtual addresses into kmem map indicies
 */
#define btokup(addr)	(&kmemusage[((char *)(addr) - (char *)kmembase) >> PAGE_SHIFT])

#define MALLOC(space, cast, size, type, flags) \
	(space) = (cast)malloc((u_long)(size), BUCKETP((size)), type, flags)

#define FREE(addr, type) free((void *)(addr), type)

extern struct kmemusage *kmemusage;
extern struct kmembuckets bucket[];
extern struct kmemtypes kmemtypes[];
extern const  char kmemnames[M_LAST][KMEMNAMSZ];
extern void *kmembase;

#ifdef __cplusplus
extern "C" {
#endif
extern void *malloc(u_long, struct kmembuckets *, int, int);
extern void free(void *, int);
extern void kmeminit(void);
extern int kmemsetlimit(int, long);
extern void kmeminit_thread(int);
#ifdef __cplusplus
}
#endif

#endif	/* _KERNEL */
#endif	/* _SYS_MALLOC_H */
