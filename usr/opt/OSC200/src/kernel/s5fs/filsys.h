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
 *	@(#)$RCSfile: filsys.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:51:29 $
 */ 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * 20-aug-1991: vipul patel
 *	OSF/1 Release 1.0.1. mp locking macros.
 */

#ifndef _S5_FILSYS_H_
#define _S5_FILSYS_H_

/*
 * Structure of the super-block
 */
struct	filsys
{
	ushort	s_isize;	/* size in blocks of i-list */
	daddr_t	s_fsize;	/* size in blocks of entire volume */
	short	s_nfree;	/* number of addresses in s_free */
	daddr_t	s_free[NICFREE];	/* free block list */
	short	s_ninode;	/* number of i-nodes in s_inode */
	s5ino_t	s_inode[NICINOD];	/* free i-node list */
	char	s_flock;	/* lock during free list manipulation */
	char	s_ilock;	/* lock during i-list manipulation */
	char  	s_fmod; 	/* super block modified flag */
	char	s_ronly;	/* mounted read-only flag */
	time_t	s_time; 	/* last super block update */
	short	s_dinfo[4];	/* device information */
	daddr_t	s_tfree;	/* total free blocks*/
	s5ino_t	s_tinode;	/* total free inodes */
	char	s_fname[6];	/* file system name */
	char	s_fpack[6];	/* file system pack name */
	long	s_fill[13];	/* ADJUST to make sizeof filsys be 512 */
	long	s_magic;	/* magic number to indicate new file system */
	long	s_type;		/* type of new file system */
};

#define	FsMAGIC	0xfd187e20	/* s_magic number */
#define	Fs1b	1	/* 512  byte block */
#define	Fs2b	2	/* 1024 byte block */
#define	Fs3b	3	/* 2048 byte block */

#ifdef _KERNEL
#define s5FS_FLOCK(ump)				\
	lock_write(&(ump)->um_fsflock)
#define s5FS_FUNLOCK(ump)			\
	lock_done(&(ump)->um_fsflock)
#define s5FS_ILOCK(ump)				\
	lock_write(&(ump)->um_fsilock)
#define s5FS_IUNLOCK(ump)			\
	lock_done(&(ump)->um_fsilock)
#endif /* _KERNEL */

#endif /*  _S5_FILSYS_H_ */
