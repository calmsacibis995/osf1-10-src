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
 *	@(#)$RCSfile: buf.h,v $ $Revision: 4.3.13.3 $ (DEC) $Date: 1993/12/15 22:11:37 $
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
 * Copyright (c) 1982, 1986, 1989 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice and this paragraph are
 * duplicated in all such forms and that any documentation,
 * advertising materials, and other materials related to such
 * distribution and use acknowledge that the software was developed
 * by the University of California, Berkeley.  The name of the
 * University may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 *

 */
/*
 *	sys/buf.h
 *
 *	Revision History:
 *
 * 6-Apr-91	Ron Widyono
 *	Buffer lock handoff scheme needs RT_PREEMPT case to keep lock counts
 *	consistent and valid.
 *
 */

#ifndef	_SYS_BUF_H_
#define _SYS_BUF_H_

#ifdef	_KERNEL
#include <rt_preempt.h>
#endif

#include <sys/types.h>
#ifdef	_KERNEL
#include <sys/unix_defs.h>
#include <kern/event.h>
#endif

/*
 * The header for buffers in the buffer pool and otherwise used
 * to describe a block i/o request is given here.  The routines
 * which manipulate these things are given in vfs/vfs_bio.c.
 *
 * Each buffer in the pool is usually doubly linked into 2 lists:
 * hashed into a chain by <vnode,blkno> so it can be located in the cache,
 * and (usually) on (one of several) queues.  These lists are circular and
 * doubly linked for easy removal.
 *
 * There are currently three queues for buffers:
 *	one for buffers which must be kept permanently (super blocks)
 * 	one for buffers containing ``useful'' information (the cache)
 *	one for buffers containing ``non-useful'' information
 *		(and empty buffers, pushed onto the front)
 * The latter two queues contain the buffers which are available for
 * reallocation, are kept in lru order.  When not on one of these queues,
 * the buffers are ``checked out'' to drivers which use the available list
 * pointers to keep track of them in their i/o active queues.
 */

/* forward declaration required for C++ */
#ifdef __cplusplus
struct buf;
#endif

/*
 * Bufhd structures used at the head of the hashed buffer queues.
 * We only need three words for these, so this abbreviated
 * definition saves some space.
 */
struct bufhd
{
	int	b_flags;		/* see defines below */
	struct	buf *b_forw, *b_back;	/* fwd/bkwd pointer in chain */
	u_long	bhd_stamp;		/* time stamp for hash chain */
#ifdef	_KERNEL
	udecl_simple_lock_data(,bhd_lock) /* hash chain spin lock */
#endif
};

struct buf
{
	int	b_flags;		/* too much goes here to describe */
	struct	buf *b_forw, *b_back;	/* hash chain (2 way street) */
	struct	buf *av_forw, *av_back;	/* position on free list if not BUSY */
	struct	buf *b_blockf, **b_blockb;/* associated vnode */
#define	b_actf	av_forw			/* alternate names for driver queue */
#define	b_actl	av_back			/*    head - isn't history wonderful */
	int	b_bcount;		/* transfer count */
#define	b_active b_bcount		/* driver queue head: drive active */
	int	b_bufsize;		/* size of allocated buffer */
#define	b_pager	b_bufsize
	short	b_error;		/* returned after I/O */
	dev_t	b_dev;			/* major+minor device name */
#define	b_edev  b_dev			/* SVR4 name for 32 bit major&minor */
	union {
	    caddr_t b_addr;		/* low order core address */
	    int	*b_words;		/* words for clearing */
	    struct fs *b_fs;		/* superblocks */
	    struct filsys *b_s5fs;      /* System V fs suberblock */
	    struct csum *b_cs;		/* superblock summary information */
	    struct cg *b_cg;		/* cylinder group block */
	    struct dinode *b_dino;	/* ilist */
	    struct s5dinode *b_s5dino;	/* s5 ilist */
	    daddr_t *b_daddr;		/* indirect block */
	} b_un;
	daddr_t	b_lblkno;		/* logical block number */
	daddr_t	b_blkno;		/* block # on device */
	int	b_resid;		/* words not transferred after error */
#define b_errcnt b_resid		/* while i/o in progress: # retries */
#define	b_cylin	b_resid			/* while on disk q: cylinder #	*/
	struct  proc *b_proc;		/* proc doing physical or swap I/O */
	struct 	buf *b_hash_chain;	/* head of hash chain owning buffer */
	void	(*b_iodone)();		/* function called by iodone */
	struct	vm_page *b_pagelist;	/* pages associated with buffer */
	struct	vnode *b_vp;		/* vnode for dev */
	struct	vnode *b_rvp;		/* vnode buffer is associated with */
	struct	ucred *b_rcred;		/* ref to read credentials */
	struct	ucred *b_wcred;		/* ref to write credendtials */
	int	b_dirtyoff;		/* offset in buffer of dirty region */
	int	b_dirtyend;		/* offset of end of dirty region */
	union	{	/* these fields reserved _solely_ for device driver */
		long	longvalue;
		void	*pointvalue;
		daddr_t	diskaddr;
		time_t	timevalue;
	} b_driver_un_1, b_driver_un_2;
#ifdef	_KERNEL
/* 
 * 	DDI/DKI compatibility
 */
	unsigned long	b_reltime;	/* Previous release time */
	char		b_oerror;	/* Old one byte I/O error code */

	lock_data_t	b_lock;		/* mutual exclusion buffer lock */
	event_t	b_iocomplete;		/* guard vnode while i/o in progress */
#endif
};

/*
 * Following #ifdefs/#defines are for compatibility with other naming schemes
 * long-term, they should be moved into the driver(s) that use them.
 */
#if defined (mips) || defined (__alpha)
	/* SCSI command in progress */
	/* The fields below are just aliases */
#define b_command	b_driver_un_1.longvalue
#define b_gid		b_driver_un_1.longvalue
#endif

#ifdef exl
	/* added the missing fields for System V driver (SCSI) --- csy */
        /* physical sector of disk request */
#define b_sector	b_driver_un_1.diskaddr
	/* request start time */
#define b_start		b_driver_un_2.timevalue
#endif

#define BQUEUES		4		/* number of free buffer queues */

#define BQ_LOCKED	0		/* super-blocks &c */
#define BQ_LRU		1		/* lru, useful buffers */
#define BQ_AGE		2		/* rubbish */
#define BQ_EMPTY	3		/* buffer headers with no memory */

#define	MINBUFHSZ	16		/* minimum size of bufhash */

#ifdef	_KERNEL
extern int	bufhsz;			/* size of buffer cache hash table */
#define RND	(MAXBSIZE/DEV_BSIZE)
#define BUFHASH(dvp, dblkno)	\
	((struct buf *)&bufhash[((int)(dvp)+(((int)(dblkno))/RND))&(bufhsz-1)])

extern struct	buf *buf;	/* the buffer pool itself */
extern char	*buffers;
extern long	nbuf;		/* number of buffer headers */
extern long	bufpages;	/* number of memory pages in the buffer pool */

extern struct	bufhd *bufhash;		/* heads of hash lists */
extern struct	buf bfreelist[BQUEUES];	/* heads of available lists */
#ifdef __cplusplus
extern "C" {
#endif
extern struct	buf *getblk();
extern struct	buf *geteblk();
extern struct	buf *getnewbuf();

extern unsigned minphys();
#ifdef __cplusplus
}
#endif
#endif	/* _KERNEL */

/*
 * These flags are kept in b_flags.
 *
 * NOTE:
 *	The following flags are provided purely for backward 
 *	compatibility with code (drivers, mostly) that
 *	expect them to be around.  We'd like them to go away:
 *		B_BUSY, B_DONE, B_WANTED
 */
#define B_WRITE		0x00000000	/* non-read pseudo-flag */
#define B_READ		0x00000001	/* read when I/O occurs */
#define B_ERROR		0x00000002	/* transaction aborted */
#define B_BUSY		0x00000004	/* not on av_forw/back list */
#define B_PHYS		0x00000008	/* physical IO */
#define B_WANTED	0x00000010	/* issue wakeup when BUSY goes off */
#define B_AGE		0x00000020	/* delayed write for correct aging */
#define B_ASYNC		0x00000040	/* don't wait for I/O completion */
#define B_DELWRI	0x00000080	/* write at exit of avail list */
#define B_TAPE		0x00000100	/* this is a magtape (no bdwrite) */
#define B_REMAPPED	0x00000200	/* for bp_mapin and bp_mapout */
#define	B_FREE		0x00000400	/* used by ubc to free page */
#define	B_SWAP		0x00000800	/* swap I/O */
#define	B_UBC		0x00001000	/* UBC I/O */
#define	B_DIRTY		0x00002000	/* UBC has page marked dirty */
#define	B_DONE		0x00004000	/* Done with I/O */
#define B_CACHE		0x00008000	/* did bread find us in the cache ? */
#define B_INVAL		0x00010000	/* does not contain valid info  */
#define B_LOCKED	0x00020000	/* locked in core (not reusable) */
#define B_HEAD		0x00040000	/* a buffer header, not a buffer */
#define B_USELESS	0x00080000	/* cache, but at low priority */
#define B_BAD		0x00100000	/* bad block revectoring in progress */
#define	B_RAW		0x00400000	/* set by physio for raw transfers */
#define	B_NOCACHE	0x00800000	/* do not cache block after use */
#define	B_PRIVATE	0x01000000	/* private data, not part of buffers */
#define	B_WRITEV	0x02000000	/* perform verification of writes */
#define	B_HWRELOC	0x04000000	/* relocate/rewrite block */
#define B_WANTFREE	0x08000000	/* want buffer from freelist */
#define	B_MSYNC		0x10000000	/* mmap msync request */

/*
 * OSF/1 has defined flags for both the b_flags of the buf structure
 * and d_flags of the bdevsw structure (e.g. B_TAPE) in this include file.
 * Follow tradition and put bdevsw flag indicating driver uses DDI/DKI
 * standard here.
*/

#define B_DDIDKI	0x20000000	

#define	BUF_NULL	(struct buf *)0
#define BHASH_NULL	(struct buf *)0

#ifdef	_KERNEL
#include <kern/macro_help.h>

/*
 * Insq/Remq for the buffer hash lists.
 */
#define	bremhash(bp) \
MACRO_BEGIN \
	(bp)->b_back->b_forw = (bp)->b_forw; \
	(bp)->b_forw->b_back = (bp)->b_back; \
	(bp)->b_hash_chain = BHASH_NULL; \
MACRO_END
#define	binshash(bp, dp) \
MACRO_BEGIN \
	(bp)->b_forw = (dp)->b_forw; \
	(bp)->b_back = (dp); \
	(bp)->b_hash_chain = (dp); \
	(dp)->b_forw->b_back = (bp); \
	(dp)->b_forw = (bp); \
	(BHASH_STAMP(dp))++; \
MACRO_END

/*
 * Insq/Remq for the buffer free lists.
 */

#define bremfree(bp)						\
MACRO_BEGIN							\
	(bp)->av_back->av_forw = (bp)->av_forw;			\
	(bp)->av_forw->av_back = (bp)->av_back;			\
	(bp)->av_forw = NULL;					\
	(bp)->av_back = NULL;					\
MACRO_END

#define binsheadfree(bp, dp)					\
MACRO_BEGIN							\
	(dp)->av_forw->av_back = (bp);				\
	(bp)->av_forw = (dp)->av_forw;				\
	(dp)->av_forw = (bp);					\
	(bp)->av_back = (dp);					\
MACRO_END

#define binstailfree(bp, dp)					\
MACRO_BEGIN							\
	(dp)->av_back->av_forw = (bp);				\
	(bp)->av_back = (dp)->av_back;				\
	(dp)->av_back = (bp);					\
	(bp)->av_forw = (dp);					\
MACRO_END

#define iodone	biodone
#define iowait	biowait

/*
 * Zero out a buffer's data portion.
 */
#define clrbuf(bp)						\
MACRO_BEGIN							\
	blkclr((bp)->b_un.b_addr, (unsigned)(bp)->b_bcount);	\
	(bp)->b_resid = 0;					\
MACRO_END
#define B_CLRBUF	0x1	/* request allocated buffer be cleared */
#define B_SYNC		0x2	/* do all allocations synchronously */

typedef struct bufhd bufhd_t;

/*
 * There are a couple of places outside of the buffer cache code
 * (in ../vfs/vfs_bio.c) that use buffer locks, and many places
 * that make assertions about the states of buffer locks, so these
 * definitions must be public.
 */
#include <sys/lock_types.h>
#define	BUF_LOCKINIT(bp)	lock_init2(&(bp)->b_lock, TRUE, LTYPE_BUF)

#define	BUF_LOCK(bp)						\
MACRO_BEGIN							\
	int s = splbio();					\
	lock_write(&(bp)->b_lock);				\
	(bp)->b_flags |= B_BUSY;				\
	splx(s);						\
MACRO_END

#define	BUF_UNLOCK(bp)						\
MACRO_BEGIN							\
	int s = splbio();					\
	(bp)->b_flags &= ~B_BUSY;				\
	lock_write_done(&(bp)->b_lock);				\
	splx(s);						\
MACRO_END

#define BUF_LOCK_TRY(bp, ret)					\
MACRO_BEGIN							\
	int s = splbio();					\
	if (ret = lock_try_write(&(bp)->b_lock)) 		\
		(bp)->b_flags |= B_BUSY;			\
	splx(s);						\
MACRO_END

#define	BUF_LOCKED(bp)	(LOCK_LOCKED(&(bp)->b_lock))

/*
 * Asynchronous I/O presents problems for the lock checking package.
 * The sequence of events is:
 *	fetch the buffer, locking it
 *	start the I/O
 *	current thread goes about its business
 *	an innocent victim thread inherits the buffer in interrupt
 *		context and brelse's it during I/O completion,
 *		releasing the buffer's lock.
 *
 * Convincing the lock checking package not to complain during
 * this process is a bit tricky.  Here are some of the problems.
 *
 * Failing to give away ownership of the buffer lock when
 * issuing the I/O request causes problems should the current
 * thread at some future time issue another request for the
 * same buffer it originally asked to read-ahead.  The lock
 * checking package will complain about a deadlock. (waiting for self)
 *
 * The solution requires two steps.  The thread initiating the I/O
 * gives the ownership of the lock away to a fake thread, biodone_ldebug.
 * The 'thread' that receives the buffer accepts the ownership of the lock
 * to itself, using BUF_ACCEPT.
 *
 * This scheme eliminates races in the ownership handoff, but strategy
 * routines are no longer allowed to assert that they own asynchronous
 * I/O buffers. They can assert that the buffer is locked, that they
 * own synchronous buffers, or that the "fake thread" owns asynchronous
 * buffers.
 */

#if	MACH_LDEBUG
extern char biodone_ldebug;

#include <kern/thread.h>

#define	BUF_LOCK_THREAD(bp)	(LOCK_THREAD(&(bp)->b_lock))
#define	BUF_LOCK_OWNER(bp)	(LOCK_OWNER(&(bp)->b_lock))
#define	BUF_LOCK_HOLDER(bp)	(LOCK_HOLDER(&(bp)->b_lock))
#define BUF_IS_LOCKED(bp)	(!LOCK_READERS(&(bp)->b_lock) && \
				 LOCK_LOCKED(&(bp)->b_lock))
/*
 * BUF_GIVE_AWAY(bp): give away ownership of an asynchronous
 * I/O request buffer.
 */
#define	BUF_GIVE_AWAY(bp)					\
MACRO_BEGIN							\
	int s = splbio();					\
	simple_lock(&(bp)->b_lock.interlock);			\
	assert((thread_t)(bp)->b_lock.lthread == current_thread());	\
	dec_lock(&(bp)->b_lock, (bp)->b_lock.lthread);  	\
	(bp)->b_lock.lthread =  &biodone_ldebug; 		\
	simple_unlock(&(bp)->b_lock.interlock);			\
	splx(s);						\
MACRO_END
/*
 * BUF_ACCEPT(bp): accept an asynchronous I/O buffer that was
 * previously given away.
 */
#define	BUF_ACCEPT(bp)						\
MACRO_BEGIN							\
	int s = splbio();					\
	simple_lock(&(bp)->b_lock.interlock);			\
	assert((bp)->b_lock.lthread == &biodone_ldebug);	\
	inc_lock(&(bp)->b_lock, current_thread());		\
	(bp)->b_lock.lthread = (char *) current_thread();	\
	simple_unlock(&(bp)->b_lock.interlock);			\
	splx(s);						\
MACRO_END
#elif	RT_PREEMPT
#include <kern/lock.h>
#include <kern/thread.h>

/*
 * BUF_GIVE_AWAY(bp): give away ownership of an asynchronous
 * I/O request buffer.
 */
#define	BUF_GIVE_AWAY(bp) DEC_LOCK_COUNT
/*
 * BUF_ACCEPT(bp): accept an asynchronous I/O buffer that was
 * previously given away.
 */
#define	BUF_ACCEPT(bp) INC_LOCK_COUNT
#else	/* MACH_LDEBUG */
#define BUF_GIVE_AWAY(bp)
#define	BUF_ACCEPT(bp)
#endif	/* MACH_LDEBUG */
#endif	/* _KERNEL */
#endif	/* _SYS_BUF_H_ */
