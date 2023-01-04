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
 *	@(#)$RCSfile: inode.h,v $ $Revision: 4.3.4.6 $ (DEC) $Date: 1992/07/27 11:36:01 $
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
 * Copyright (c) 1982, 1989 The Regents of the University of California.
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

#ifndef	_UFS_INODE_H_
#define _UFS_INODE_H_

#ifdef	_KERNEL
#include <quota.h>
#endif

#include <sys/secdefines.h>
#include <ufs/dinode.h>
#include <sys/types.h>
#include <ufs/quota.h>
#include <machine/endian.h>
#ifdef	_KERNEL
#include <sys/unix_defs.h>
#include <kern/event.h>
#endif

#if	SEC_FSCHANGE
#include <sys/security.h>
#endif
#if	QUOTA
#include <ufs/quota.h>
#endif

/*
 * The I node is the focus of all file activity in UNIX.
 * There is a unique inode allocated for each active file,
 * each current directory, each mounted-on file, text file, and the root.
 * An inode is 'named' by its dev/inumber pair. (iget/iget.c)
 * Data in `struct dinode' is read in from permanent inode on volume.
 */

struct inode {
	struct	inode *i_chain[2]; /* hash chain, MUST be first */
	struct	vnode *i_vnode;	/* vnode associated with this inode */
	struct	vnode *i_devvp;	/* vnode for block I/O */
	u_int	i_flag;		/* see below */
	dev_t	i_dev;		/* device where inode resides */
	ino_t	i_number;	/* i number, 1-to-1 with device address */
	struct	fs *i_fs;	/* file sys associated with this inode */
	int	i_diroff;	/* offset in dir, where we found last entry */
	off_t	i_endoff;	/* end of useful stuff in directory */
	short	i_dirstamp;	/* optimize directory insertions */
	struct	dinode i_din;	/* the on-disk inode */
	struct	dquot *i_dquot[MAXQUOTAS]; /* pointer to dquot structures */
	off_t	i_delayoff;	/* Offset in file write cluster begins */
	int	i_delaylen;	/* Current length of write cluster */
	int	i_consecreads;	/* Number of consecutive reads */
	int	i_clusterlbn;	/* lbn of last read cluster */
	int	i_clustersz;	/* Size in blocks of above */
#if	SEC_FSCHANGE
	struct	dinode_sec i_disec;	/* security extension */
#endif
#ifdef	_KERNEL
	lock_data_t	i_io_lock;	/* reading/writing file's contents */
	event_t		i_iodone;	/* inode is in transit from disk */
	udecl_simple_lock_data(,i_incore_lock)
#endif
};

typedef	struct inode	inode_t;

#define	i_mode		i_din.di_mode
#define	i_nlink		i_din.di_nlink
#define	i_uid		i_din.di_uid
#define	i_gid		i_din.di_gid
#ifdef __alpha
#define i_size		i_din.di_qsize
#else
#if	BYTE_ORDER == LITTLE_ENDIAN
#define	i_size		i_din.di_qsize.val[0]
#else
#define	i_size		i_din.di_qsize.val[1]
#endif
#endif
#define	i_db		i_din.di_db
#define	i_ib		i_din.di_ib
#define i_symlink	i_din.di_symlink
#define	i_atime		i_din.di_atime
#define	i_uatime	i_din.di_atspare
#define	i_mtime		i_din.di_mtime
#define	i_umtime	i_din.di_mtspare
#define	i_ctime		i_din.di_ctime
#define	i_uctime	i_din.di_ctspare
#define i_blocks	i_din.di_blocks
#define	i_rdev		i_din.di_db[0]
#define i_flags		i_din.di_flags
#define i_gen		i_din.di_gen
#define	i_forw		i_chain[0]
#define	i_back		i_chain[1]
#if SEC_FSCHANGE
#define i_gpriv		i_disec.di_gpriv
#define i_ppriv		i_disec.di_ppriv
#define i_tag		i_disec.di_tag
#define i_parent	i_disec.di_parent
#define i_type_flags	i_disec.di_type_flags
#endif

/* i_flag values */
#define	IRENAME		0x0004		/* inode is being renamed */
#define	IUPD		0x0010		/* file has been modified */
#define	IACC		0x0020		/* inode access time to be updated */
#define	ICHG		0x0040		/* inode has been changed */
#define	IMOD		0x0080		/* inode has been modified */
#define	ISHLOCK		0x0100		/* file has shared lock */
#define	IEXLOCK		0x0200		/* file has exclusive lock */
#define	ILWAIT		0x0400		/* someone waiting on file lock */
#define	INACTIVATING	0x0800		/* inode is being inactivated */
#define	INACTWAIT	0x1000		/* waiting for inode inactivation */
#define	IREADERROR	0x2000		/* disk read for inode failed */
#define	IQUOTA		0x4000		/* inode has quotas attached */
#define	IQUOTING	0x8000		/* quota attach in progress */
#define	IQUOTWAIT	0x10000		/* waiting while attach progresses */
#define ICHGMETA	0x20000		/* Meta data changed and not on disk */
#define IOLDDEVT	0x40000		/* TEMPORARY: this is an old devt */

#ifdef	_KERNEL
#include <kern/macro_help.h>

/*
 * Convert between inode pointers and vnode pointers
 */
#define VTOI(vp)	((struct inode *)(vp)->v_data)
#define ITOV(ip)	((ip)->i_vnode)

/*
 * Convert between vnode types and inode formats
 */
extern enum vtype	iftovt_tab[];
extern int		vttoif_tab[];
#define IFTOVT(mode)	(iftovt_tab[((mode) & IFMT) >> 12])
#define VTTOIF(indx)	(vttoif_tab[(int)(indx)])

#define MAKEIMODE(indx, mode)	(int)(VTTOIF(indx) | (mode))

extern ino_t	dirpref();
extern u_int	get_nextgen();

/*
 * Occasionally a fake inode must be built ``by-hand''.
 */
#define	FAKE_INODE_SIZE	(sizeof(struct vnode)+sizeof(struct inode))

/*
 * Inode locking.			XXX sadly out of date XXX
 *
 * The phrase "read-only [after in-core inode creation]" mean that as long
 * as the code examining the field holds a reference to the inode, the inode
 * won't be deallocated -- and as long as the inode is allocated, the
 * contents of the field never change.
 *
 * File I/O is handled as follows.  For IFREG files, file reads and writes
 * take the inode I/O lock for reading or writing, as appropriate.  For IFDIR
 * files, the I/O lock is taken for writing when atomic operations are desired
 * (e.g., examining then updating a directory.)  The I/O lock is taken for
 * reading when the only operation to be done is examining the contents of the
 * directory.
 *
 * For IFIFO files.
 *
 * For IFBLK files and IFCHR files, no inode lock whatsoever is taken across
 * an I/O operation.  All locking is at the discretion of the lower-level
 * driver.
 *
 * Inode locking requirements per field:
 *
 *	i_chain[2]		protected by inode hash chain lock
 *	i_vnode			internal vnode locks, cf vnode.h
 *	i_devvp			incore lock
 *	i_flag			incore lock
 *	i_dev			read-only
 *	i_number		read-only
 *	i_diroff		incore lock
 *	i_fs			read-only
 *	i_dquot			governed by IQUOTA and ufsmount's quota lock
 *	di_mode			incore lock (no lock needed to check IFMT)
 *	di_nlink		incore lock
 *	di_uid			incore lock
 *	di_gid			incore lock
 *	di_size			incore lock (only modified under i/o writelock)
 *	di_atime		incore lock
 *	di_atspare		n/a
 *	di_mtime		incore lock
 *	di_mtspare		n/a
 *	di_ctime		incore lock
 *	di_ctspare		n/a
 *	union
 *		di_db & di_ib	inode I/O lock (usually held for writing)
 *		di_symlink	read-only
 *	di_flags		IC_FASTLINK is read-only
 *	di_blocks		inode I/O lock
 *	di_gen			read-only
 *	di_spare[]		n/a
#if SEC_FSCHANGE
 *	i_gpriv			TBD
 *	i_ppriv			TBD
 *	i_tag			TBD
 *	i_parent		TBD
 *	i_type_flags		TBD
#endif
#if	QUOTA
 * Special considerations apply to the inode's i_dquot array.  Refer to
 * ufs/ufs_quota.c for a description of the locking protocol.
#endif
 */

#define	IN_READ_LOCK(ip)	lock_read(&(ip)->i_io_lock)
#define	IN_WRITE_LOCK(ip)	lock_write(&(ip)->i_io_lock)
#define	IN_WRITE_LOCK_TRY(ip)	lock_try_write(&(ip)->i_io_lock)
#define	IN_READ_UNLOCK(ip)	lock_read_done(&(ip)->i_io_lock)
#define	IN_WRITE_UNLOCK(ip)	lock_write_done(&(ip)->i_io_lock)
#define	IN_SET_RECURSIVE(ip)	lock_set_recursive(&(ip)->i_io_lock)
#define IN_CLEAR_RECURSIVE(ip)	lock_clear_recursive(&(ip)->i_io_lock)
#define	IN_IO_LOCK_INIT(ip)	lock_init2(&(ip)->i_io_lock, TRUE, \
					   LTYPE_INODE_IO)

#define	IN_LOCK(ip)		usimple_lock(&(ip)->i_incore_lock)
#define	IN_UNLOCK(ip)		usimple_unlock(&(ip)->i_incore_lock)
#define	IN_LOCK_INIT(ip)	usimple_lock_init(&(ip)->i_incore_lock)
#define	IN_LOCK_TRY(ip)		usimple_lock_try(&(ip)->i_incore_lock)

#define	IN_WRITE_HOLDER(ip)	LOCK_HOLDER(&ip->i_io_lock)
#define	IN_READ_HOLDER(ip)	((ip)->i_io_lock.read_count >= 1)

/*
 * Avoid locking inode here in favor of re-checking under lock
 * in iupdat.			XXX
 */
#define IUPDAT(ip, t1, t2, waitfor)		\
MACRO_BEGIN					\
	if (ip->i_flag&(IUPD|IACC|ICHG|IMOD|ICHGMETA))	\
		(void) iupdat(ip, t1, t2, waitfor); \
MACRO_END

#define	ITIMES(ip, t1, t2)	itimes((ip), (t1), (t2))

/*
 * This overlays the fid structure (see mount.h)
 */
struct ufid {
	u_short	ufid_len;	/* length of structure */
	u_short	ufid_pad;	/* force long alignment */
	ino_t	ufid_ino;	/* file number (ino) */
	int	ufid_gen;	/* generation number */
};


extern int	inohsz;		/* elements in inode hash table */

#define	MININOHSZ	64	/* minimum size of inode hash table */

#define INOHASH(dev,ino)	(((dev)+(ino))&(inohsz-1))

struct ihead {				/* inode LRU cache, Chris Maltby */
	union {
		struct ihead	*iu_head[2];
		struct inode	*iu_chain[2];
	} ih_un;
	int	ih_timestamp;		/* track hash chain insertions */
	udecl_simple_lock_data(,ih_lock) /* MP exclusion */
};

extern struct ihead *ihead;

#define	ih_head		ih_un.iu_head
#define	ih_chain	ih_un.iu_chain

#define	IHASH_LOCK(ih)		usimple_lock(&(ih)->ih_lock)
#define	IHASH_UNLOCK(ih)	usimple_unlock(&(ih)->ih_lock)
#define	IHASH_LOCK_TRY(ih)	usimple_lock_try(&(ih)->ih_lock)
#define	IHASH_LOCK_INIT(ih)	usimple_lock_init(&(ih)->ih_lock)

/*
 * Fast link runtime controls
 */
extern	int		create_fastlinks;

#endif  /* _KERNEL */
#endif	/* _UFS_INODE_H_ */
