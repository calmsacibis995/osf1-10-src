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
 *	@(#)$RCSfile: namei.h,v $ $Revision: 4.2.13.2 $ (DEC) $Date: 1993/12/15 22:12:02 $
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
 * Copyright (c) 1985, 1989 Regents of the University of California.
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

#ifndef	_SYS_NAMEI_H_
#define _SYS_NAMEI_H_

#ifdef	_KERNEL
#include <sys/unix_defs.h>
#endif

#include <sys/types.h>
#include <sys/uio.h>
#include <ufs/dir.h>
#include <s5fs/s5dir.h>
#ifdef	_KERNEL
#include <kern/zalloc.h>
#endif

/*
 * Encapsulation of namei parameters.
 * One of these is located in the u. area to
 * minimize space allocated on the kernel stack.
 */

#if	UNIX_LOCKS
/*
 * There is often a delay between creating an in-core inode representing
 * an object and actually filling in all of the relevant information.
 * For example, a symbolic link is created by creating an inode and then
 * writing the link name into a disk block or (multimax) into the inode
 * itself.  These operations are separate for non-UNIX_LOCKS implementations
 * but under UNIX_LOCKS the relevant initial data is passed down to the routine
 * creating the inode, so that the new inode becomes visible to the world
 * in an atomic fashion.
 */
#endif

/* forward declarations as required for C++ */
#ifdef __cplusplus
struct vnode;
struct ucred;
#endif

struct nameidata {
		/* arguments to namei and related context: */
	caddr_t	ni_dirp;		/* pathname pointer */
	enum	uio_seg ni_segflg;	/* location of pathname */
	short	ni_nameiop;		/* see below */
#if	MACH
	struct	utask_nd *ni_utnd;	/* utask nameidata structure */
#define ni_cdir	ni_utnd->utnd_cdir
#define ni_rdir	ni_utnd->utnd_rdir
#else
	struct	vnode *ni_cdir;		/* current directory */
	struct	vnode *ni_rdir;		/* root directory, if not normal root */
#endif
	struct	ucred *ni_cred;		/* credentials */

		/* shared between namei, lookup routines and commit routines: */
	caddr_t	ni_pnbuf;		/* pathname buffer */
	char	*ni_ptr;		/* current location in pathname */
	char	*ni_next;		/* next location in pathname */
	u_int	ni_pathlen;		/* remaining chars in path */
	u_long	ni_hash;		/* hash value of current component */
	short	ni_namelen;		/* length of current component */
	short	ni_loopcnt;		/* count of symlinks encountered */
	char	ni_makeentry;		/* 1 => add entry to name cache */
	char	ni_isdotdot;		/* 1 => current component name is .. */

		/* results: */
	struct	vnode *ni_vp;		/* vnode of result */
	struct	vnode *ni_dvp;		/* vnode of intermediate directory */
	union {
		struct	dirent ni_ufsdent;	/* final component name */
		struct	gpdirect ni_gpfsdent;	/* general purpose final name */
	} ni_dir;

		/* side effects: */
	/* BEGIN UFS SPECIFIC */
	off_t	ni_endoff;		/* end of useful directory contents */
	struct ndirinfo {		/* saved info for new dir entry */
		struct	iovec nd_iovec;		/* pointed to by ni_iov */
		struct	uio nd_uio;		/* directory I/O parameters */
	} ni_nd;
	/* END UFS SPECIFIC */

	long	ni_dirstamp;		/* for directories */
	int	ni_nchtimestamp;	/* for name cache enters */
#if	UNIX_LOCKS
	u_long	ni_vpid;		/* capability of result's vnode */
#endif
	int     (*ni_lookup)();         /* special dce dfs lookup function */
};

#define	ni_iovec	ni_nd.nd_iovec
#define	ni_base		ni_nd.nd_iovec.iov_base
#define	ni_count	ni_nd.nd_iovec.iov_len
#define	ni_uioseg	ni_nd.nd_uio.uio_segflg
#define	ni_iov		ni_nd.nd_uio.uio_iov
#define	ni_iovcnt	ni_nd.nd_uio.uio_iovcnt
#define	ni_offset	ni_nd.nd_uio.uio_offset
#define	ni_resid	ni_nd.nd_uio.uio_resid
#define	ni_rw		ni_nd.nd_uio.uio_rw
#define	ni_uio		ni_nd.nd_uio
#define ni_dent		ni_dir.ni_ufsdent
#define ni_gpdent	ni_dir.ni_gpfsdent
#define	ni_bypassvp	ni_vp

#ifdef	_KERNEL
/*
 * namei operations and modifiers
 */
#define LOOKUP		0x0000	/* perform name lookup only */
#define CREATE		0x0001	/* setup for file creation */
#define DELETE		0x0002	/* setup for file deletion */
#define	RENAME		0x0003	/* setup for file renaming */
#define	OPFLAG		0x0003	/* mask for operation */
#define	WANTPARENT	0x0010	/* want parent vnode returned unlocked */
#define NOCACHE		0x0020	/* name must not be left in cache */
#define FOLLOW		0x0040	/* follow symbolic links */
#define NOFOLLOW	0x0000	/* don't follow symbolic links (pseudo) */
#define	NOCROSSMOUNT	0x0080	/* do not cross mount points */
#define	REMOTE		0x0100	/* lookup for remote filesystem servers */
#define	HASBUF		0x0200	/* has preallocated pathname buffer */
#define	STRIPSLASH	0x0400	/* has preallocated pathname buffer */
#define	BYPASSVP	0x0800	/* ignore lock on fs root vnode if need be */
#if	SEC_MAC
#define	MLDCREATE	0x4000	/* creating mld subdirectory */
#endif
#define SPECLOOKUP      0x8000  /* special dce dfs lookup function */
#endif	/* _KERNEL */

/*
 * This structure describes the elements in the cache of recent
 * names looked up by namei.
 */

#define	NCHNAMLEN	31	/* maximum name segment length we bother with */
#define	NCH_NULL	((struct nchash *) 0)

struct	namecache {
	struct	namecache *nc_forw;	/* hash chain, MUST BE FIRST */
	struct	namecache *nc_back;	/* hash chain, MUST BE FIRST */
	struct	namecache *nc_nxt;	/* LRU chain */
	struct	namecache **nc_prev;	/* LRU chain */
	struct	vnode *nc_dvp;		/* vnode of parent of name */
	u_long	nc_dvpid;		/* capability number of nc_dvp */
	struct	vnode *nc_vp;		/* vnode the name refers to */
	u_long	nc_vpid;		/* capability number of nc_vp */
	char	nc_nlen;		/* length of name */
	char	nc_name[NCHNAMLEN];	/* segment name */
#if	UNIX_LOCKS
	struct	nchash *nc_hash_chain;	/* header of my hash chain */
#endif
};

#ifdef	_KERNEL
/*
 * Namei cache hash chain header.
 */
struct nchash {
	union nchash_u {
		struct	nchash *nch_u_head[2];
		struct	namecache *nch_u_chain[2];
	} nch_u;
	int	nch_timestamp;
	udecl_simple_lock_data(, nch_lock)
};

#define nch_head	nch_u.nch_u_head
#define nch_chain	nch_u.nch_u_chain
#define	nch_forw	nch_chain[0]
#define	nch_back	nch_chain[1]

extern struct	namecache *namecache;
extern int	nchsize;
#define MINNCHSZ 64
extern int	nchsz;
extern struct nchash	*nchash;
extern u_long	nextvnodeid;
extern zone_t	pathname_zone;
#endif

/*
 * Stats on usefulness of namei caches.
 */
struct	nchstats {
	long	ncs_goodhits;		/* hits that we can really use */
	long	ncs_neghits;		/* negative hits that we can use */
	long	ncs_badhits;		/* hits we must drop */
	long	ncs_falsehits;		/* hits with id mismatch */
	long	ncs_miss;		/* misses */
	long	ncs_long;		/* long names that ignore cache */
	long	ncs_pass2;		/* names found with passes == 2 */
	long	ncs_2passes;		/* number of times we attempt it */
	long	ncs_dirscan;		/* # of times we rescan directory */
#ifdef	_KERNEL
	udecl_simple_lock_data(,ncs_lock)
#endif
};

#ifdef	_KERNEL
#define NC_STATS(c)     STATS_ACTION(&nchstats.ncs_lock, (c))
#endif

/*
 * Flags to checkdir when proposing to add a name
 * or remove a name from a directory.
 */
#define ADD	0x1
#define DEL	0x2
#define RNM	0x4
#endif	/* _SYS_NAMEI_H_ */
