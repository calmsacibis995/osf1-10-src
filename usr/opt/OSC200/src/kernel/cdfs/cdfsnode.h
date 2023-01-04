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
 *	@(#)$RCSfile: cdfsnode.h,v $ $Revision: 4.3.16.3 $ (DEC) $Date: 1993/07/16 13:05:56 $
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
#ifndef _CDFSNODE_H_
#define _CDFSNODE_H_

struct iso_idir
{
	unsigned int dir_extent;
	unsigned int dir_dat_len;
	unsigned char dir_len;
	unsigned char dir_xar;
	unsigned char dir_dt[7];
	unsigned char dir_file_flags;
	unsigned char dir_file_unit_size;
	unsigned char dir_inger_gap_size;
	unsigned short dir_vol_seq_no;
};

/*
 * The cdnode is the cdfs equivalent to ufs's inode. Any similarity
 * is purely coincidental.
 * There is a unique cdnode allocated for each active file,
 * each current directory, each mounted-on file, text file, and the root.
 * A cdnode is 'named' by its dev and unique number. (cdget/cdfs_node.c)
 */

struct cdnode {
	struct	cdnode *cd_chain[2]; /* hash chain, MUST be first */
	struct	vnode *cd_vnode;	/* vnode associated with this cdnode */
	struct	vnode *cd_devvp;	/* vnode for block I/O */
	u_int	cd_flag;		/* see below */
	u_short	cd_mode;		/* file mode */
	u_short cd_nlink;
	uid_t cd_uid;
	gid_t cd_gid;
	time_t  cd_atime;       /* 16: time last accessed */
        time_t  cd_mtime;       /* 24: time last modified */
        time_t  cd_ctime;       /* 32: last time cdnode changed */
	int	cd_gen;
	dev_t	cd_dev;		/* device where cdnode resides */
	dev_t	cd_rdev;	/* device node identity (mapped) */
	dev_t	cd_ondiskrdev;	/* device node identity on disk */
	ino_t	cd_number;	/* disk address of entries dir record */
	struct	fs *cd_fs;	/* file sys associated with this cdnode */
	struct	iso_idir cd_cdin;	/* the translated cdnode */
	/* Some  RRIP stuff */
	char *cd_linktarg;		/* symlink target */
	int	cd_rrip_fields;		/* logical OR of RRIP_RR_xx_PRESENT */
	unsigned char	cd_rrip_fldoffset[8];


#ifdef	_KERNEL
	lock_data_t	cd_io_lock;	/* reading file's contents */
	event_t		cd_iodone;	/* cdnode is in transit from disk */
	udecl_simple_lock_data(,cd_incore_lock)
#endif
};

#define CDNODE_RRIP_RR_SEEN	0x01000000 /* have we pulled in a RR record? */
#define CDNODE_RRIP_RR_NOT	0x02000000 /* looked & didn't find a RR record */
	/* the _DONE bits are set when the field is read and interpreted,
	   so that later node initialization code knows not to undo
	   the work. */
#define CDNODE_PX_DONE	(RRIP_RR_PX_PRESENT << 8)
#define CDNODE_PN_DONE	(RRIP_RR_PN_PRESENT << 8)
#define CDNODE_SL_DONE	(RRIP_RR_SL_PRESENT << 8)
#define CDNODE_NM_DONE	(RRIP_RR_NM_PRESENT << 8)
#define CDNODE_CL_DONE	(RRIP_RR_CL_PRESENT << 8)
#define CDNODE_PL_DONE	(RRIP_RR_PL_PRESENT << 8)
#define CDNODE_RE_DONE	(RRIP_RR_RE_PRESENT << 8)
#define CDNODE_TF_DONE	(RRIP_RR_TF_PRESENT << 8)
	/* the 8 bits of cd_rrip_fields above the _DONE bits indicate whether
	   cd_rrip_fldoffset[i] is valid.
	   If cd_rrip_fields indicates something has been seen,
	   then the appropriate element of cd_rrip_fldoffset, if nonzero,
	   is its offset in the dirent SUA.  If it's zero, then either
	   the field is not present, or it must be in a continuation area. */
#define CDNODE_SEEN_SHIFT	16
#define CDNODE_PX_OFF	0
#define CDNODE_PN_OFF	1
#define CDNODE_SL_OFF	2
#define	CDNODE_NM_OFF	3
#define	CDNODE_CL_OFF	4
#define	CDNODE_PL_OFF	5
#define	CDNODE_RE_OFF	6
#define	CDNODE_TF_OFF	7

/* "lookfor(node,Field) == TRUE" means we either know the field is there,
   or we know that we don't know yet. */
/* setdone marks that field as processed. */
/* isdone checks whether it has been processed. */
/* cached checks whether an offset to the desired field is cached */

#define lookfor(cdp, xx) ((cdp->cd_rrip_fields & CDNODE_RRIP_RR_NOT) || \
		     (cdp->cd_rrip_fields & RRIP_RR_ ## xx ## _PRESENT))
#define setdone(cdp, xx) cdp->cd_rrip_fields |= CDNODE_ ## xx ## _DONE
#define isdone(cdp, xx) (cdp->cd_rrip_fields & CDNODE_ ## xx ## _DONE)
#define cached(cdp, xx) (cdp->cd_rrip_fields & (RRIP_RR_ ## xx ## _PRESENT << CDNODE_SEEN_SHIFT))

#define	cd_size			cd_cdin.dir_dat_len
#define iso_dir_len		cd_cdin.dir_len
#define iso_dir_xar		cd_cdin.dir_xar
#define iso_dir_extent		cd_cdin.dir_extent
#define iso_dir_dat_len		cd_cdin.dir_dat_len
#define iso_dir_dt		cd_cdin.dir_dt
#define iso_dir_file_flags	cd_cdin.dir_file_flags
#define iso_dir_file_unit_size	cd_cdin.dir_file_unit_size
#define iso_dir_inger_gap_size	cd_cdin.dir_inger_gap_size
#define iso_dir_vol_seq_no	cd_cdin.dir_vol_seq_no
#define	cd_forw			cd_chain[0]
#define	cd_back			cd_chain[1]

/* flags */
#define	INACTIVATING	0x0800		/* cdnode is being inactivated */
#define	INACTWAIT	0x1000		/* waiting for cdnode inactivation */
#define	IREADERROR	0x2000		/* disk read for cdnode failed */
#define	CDNODE_BADVOL	0x4000		/* node is not on this volume */

/* file modes: bits match those in POSIX & BSD extension definitions */
#define	CDFMT		0170000		/* type of file */
#define	CDFIFO		0010000		/* fifo (named pipe) */
#define	CDFCHR		0020000		/* character special */
#define	CDFDIR		0040000		/* directory */
#define	CDFBLK		0060000		/* block special */
#define	CDFREG		0100000		/* regular */
#define	CDFLNK		0120000		/* symbolic link */

#define CD_ISBLK(m) (((m)&CDFMT) == CDFBLK)
#define CD_ISCHR(m) (((m)&CDFMT) == CDFCHR)
#define CD_ISLNK(m) (((m)&CDFMT) == CDFLNK)

#define	CDREAD		0400		/* read, execute permissions */
#define	CDEXEC		0100

/*
 * This overlays the fid structure (see mount.h)
 */
struct cdfid {
	u_short	cdfid_len;	/* length of structure */
	u_short	cdfid_pad;	/* force long alignment */
	ino_t	cdfid_ino;	/* file number (ino) */
	int	cdfid_gen;	/* generation number */
};

#ifdef	_KERNEL
/*
 * CDNODE_LBS returns the logical blocks size of a file.
 * An interleaved file must have a block size of 2K.
 * All others have a block size of the machine page size.
 */
#define CDNODE_LBS(cdnodep) \
	((cdnodep)->iso_dir_file_unit_size ? MIN(PAGE_SIZE, ISO_SECSIZE) : \
	 PAGE_SIZE)
/*
 * Convert between inode pointers and vnode pointers
 */
#define VTOCD(vp)	((struct cdnode *)(vp)->v_data)
#define CDTOV(cdp)	((cdp)->cd_vnode)

#define	IN_READ_LOCK(cdp)	lock_read(&(cdp)->cd_io_lock)
#define	IN_WRITE_LOCK(cdp)	lock_write(&(cdp)->cd_io_lock)
#define	IN_WRITE_LOCK_TRY(cdp)	lock_try_write(&(cdp)->cd_io_lock)
#define	IN_READ_UNLOCK(cdp)	lock_read_done(&(cdp)->cd_io_lock)
#define	IN_WRITE_UNLOCK(cdp)	lock_write_done(&(cdp)->cd_io_lock)
#define	IN_IO_LOCK_INIT(cdp)	lock_init2(&(cdp)->cd_io_lock, TRUE, \
					   LTYPE_INODE_IO)

#define	IN_LOCK(cdp)		usimple_lock(&(cdp)->cd_incore_lock)
#define	IN_UNLOCK(cdp)		usimple_unlock(&(cdp)->cd_incore_lock)
#define	IN_LOCK_INIT(cdp)	usimple_lock_init(&(cdp)->cd_incore_lock)

#endif

#endif
