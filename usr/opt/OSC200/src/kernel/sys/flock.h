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
 *	@(#)$RCSfile: flock.h,v $ $Revision: 4.3.7.3 $ (DEC) $Date: 1993/05/28 20:34:03 $
 */ 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0
 */

#ifndef	_SYS_FLOCK_H
#define	_SYS_FLOCK_H

#ifdef	_KERNEL
#include <unix_locks.h>
#if	UNIX_LOCKS
#include <kern/lock.h>
#endif
#endif

/* file locking structure (connected to file table entry) */
#ifdef	_KERNEL
/*
 * Locking constraints on the filock structure.
 * 	Field			Comment
 *	-----			-------
 *	set			FICHAIN_LOCK (from flip field)
 *	stat			FICHAIN_LOCK
 *	prev			FICHAIN_LOCK
 *	next			FICHAIN_LOCK
 *	flip			read-only
 */
#endif
struct	filock	{
	struct	eflock set;	/* contains type, start, and length */
	union	{
		int wakeflg;	/* for locks sleeping on this one */
		int blkpid;	/* pid of blocking lock
				 * (for sleeping locks only)
				 */
	}	stat;
	struct	filock *prev;
	struct	filock *next;
	struct	flino  *flip;	/* back pointer to chain we are on */
	int 	class;		/* Class of lock (FILE, LOCKMGR, or IO) */
	pid_t   blkrpid;	/* rpid of blocking lock */
	u_int	blkrsys;	/* rsys of blocking lock */
	struct	vnode *vp;	/* Pointer to vnode - need on sleeplck
					 * and klm grant chains
					 */
};

/* table to associate files with chain of locks */
#ifdef	_KERNEL
/*
 * Locking constraints for the flino structure.
 *	Field			Comment
 *	-----			-------
 *	vp			read-only
 *	fl_refcnt		flino_lock
 *	fl_flck			fi_chain_lock
 *	prev			FIDS_LOCK (global lock)
 *	next			FIDS_LOCK (global lock)
 *
 * The FIDS_LOCK cannot be taken while holding the flino_lock
 */
#endif
struct	flino {
	struct vnode *vp;	 /* vnode address for the file */
	int	fl_refcnt;	 /* # procs currently referencing this flino */
	struct	filock *fl_flck; /* pointer to chain of locks for this file */
	struct	flino  *prev;
	struct	flino  *next;
#ifdef i386
	unsigned short status;	/* for Xenix compatibility */
#endif
	udecl_simple_lock_data(, flino_lock)
#if	defined(_KERNEL) && UNIX_LOCKS
	lock_data_t	fichain_lock;	/* lock to protect fl_flck chain */
#endif
};

/* file and record locking configuration structure */
/* record and file use totals may overflow */
#ifdef	_KERNEL
/*
 * Locking constraints for the flockinfo structure.
 *	Field		Comment
 *	-----		-------
 *	recs		read-only
 *	fils		read-only
 *	reccnt		flckinfo_lock
 *	filcnt		flckinfo_lock
 *	rectot		flckinfo_lock
 *	filtot		flckinfo_lock
 */
#endif
struct flckinfo {
	int recmax;	/* maximum number of records locked at one time on system */
	int filmax;	/* maximum number of files locked at one time on system */
	int reccnt;	/* number of records currently in use */
	int filcnt;	/* number of file headers currently in use */
	int rectot;	/* number of records used since system boot */
	int filtot;	/* number of file headers used since system boot */
#ifdef	_KERNEL
	udecl_simple_lock_data(, flckinfo_lock)
#endif
};

extern struct flckinfo	flckinfo;

#ifdef	_KERNEL

/*  Lock classes, determines how they are treated when they are unlocked. */
#define FILE_LOCK       0       /* Generic class no special treatment */
#define IO_LOCK         1       /* Indicates I/O waiting to complete */
#define LOCKMGR         2       /* Indicates lock manager lock */

#define	FLCKINFO_LOCK(f)	usimple_lock(&(f)->flckinfo_lock)
#define	FLCKINFO_UNLOCK(f)	usimple_unlock(&(f)->flckinfo_lock)
#define	FLCKINFO_LOCKINIT(f)	usimple_lock_init(&(f)->flckinfo_lock)

#define	FLINO_LOCK(f)		usimple_lock(&(f)->flino_lock)
#define	FLINO_UNLOCK(f)		usimple_unlock(&(f)->flino_lock)
#define	FLINO_LOCKINIT(f)	usimple_lock_init(&(f)->flino_lock)

#define	FICHAIN_LOCKINIT(l)	ulock_init(&(l)->fichain_lock,TRUE,LTYPE_FICHAIN)
#define	FICHAIN_LOCK(l)		ulock_write(&(l)->fichain_lock)
#define	FICHAIN_UNLOCK(l)	ulock_done(&(l)->fichain_lock)
#define	FICHAIN_LOCK_HOLDER(p)	ULOCK_HOLDER(&(p)->fichain_lock)

#endif	/* _KERNEL */
#endif	/* _SYS_FLOCK_H */
