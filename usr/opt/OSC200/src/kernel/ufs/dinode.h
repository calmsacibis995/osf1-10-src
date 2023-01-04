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
 *	@(#)dinode.h	9.3	(ULTRIX/OSF)	10/23/91
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

#ifndef	_UFS_DINODE_H_
#define	_UFS_DINODE_H_

#include <sys/secdefines.h>
#if	SEC_FSCHANGE
#include <sys/security.h>
#endif

#include <sys/types.h>
#include <machine/endian.h>

/*
 * This structure defines the on-disk format of an inode.
 *
 * Refer to ../sys/inode.h for an explanation of locking constraints
 * when the inode is in-core.
 */

#define	NDADDR	12		/* direct addresses in inode */
#define	NIADDR	3		/* indirect addresses in inode */

#define MAX_FASTLINK_SIZE	((NDADDR + NIADDR) * sizeof (daddr_t))

struct dinode {
	u_short	di_mode;	/*  0: mode and type of file */
	short	di_nlink;	/*  2: number of links to file */
	u_short	di_uid;		/*  4: owner's user id */
	u_short	di_gid;		/*  6: owner's group id */
#ifdef __alpha
	long	di_qsize;	/*  8: number of bytes in file */
#else
	quad	di_qsize;	/*  8: number of bytes in file */
#endif
	time_t	di_atime;	/* 16: time last accessed */
	int	di_atspare;
	time_t	di_mtime;	/* 24: time last modified */
	int	di_mtspare;
	time_t	di_ctime;	/* 32: last time inode changed */
	int	di_ctspare;
	union {
	    struct {
		daddr_t	Mb_db[NDADDR]; /* 40: disk block addresses*/
		daddr_t	Mb_ib[NIADDR]; /* 88: indirect blocks */
	    } di_Mb;
#define di_db	di_Mun.di_Mb.Mb_db
#define di_ib	di_Mun.di_Mb.Mb_ib
	    char	di_Msymlink[MAX_FASTLINK_SIZE];
						/* 40: symbolic link name */
	} di_Mun;
#define di_symlink	di_Mun.di_Msymlink
	int	di_flags;	/* 100: status, currently unused */
#define IC_FASTLINK	0x0001		/* Symbolic link in inode */
	int	di_blocks;	/* 104: blocks actually held */
	int	di_gen;		/* 108: generation number */
	int	di_spare[4];	/* 112: reserved, currently unused */
};

#if	SEC_FSCHANGE

/*
 * Security extensions to the on-disk inode format:
 */
struct dinode_sec {
	priv_t  di_gpriv[2];    /* granted privilege vector */
	priv_t  di_ppriv[2];    /* potential privilege vector */
	tag_t   di_tag[SEC_TAG_COUNT];  /* security policy tags */
	ino_t   di_parent;      /* inode number of parent of MLD child */
	u_short di_type_flags;  /* type flags (MLD, 2 person rule, etc.) */
};

/*
 * On-disk inode format for a secure filesystem:
 */
struct sec_dinode {
	struct dinode           di_node;
	struct dinode_sec       di_sec;
};
#endif	/* SEC_FSCHANGE */

#if __alpha
#define di_size		di_qsize
#else
#if	BYTE_ORDER == LITTLE_ENDIAN
#define	di_size		di_qsize.val[0]
#else
#define	di_size		di_qsize.val[1]
#endif
#endif
#define	di_rdev		di_db[0]

/* file modes */
#define	IFMT		0170000		/* type of file */
#define	IFIFO		0010000		/* fifo (named pipe) */
#define	IFCHR		0020000		/* character special */
#define	IFDIR		0040000		/* directory */
#define	IFBLK		0060000		/* block special */
#define	IFREG		0100000		/* regular */
#define	IFLNK		0120000		/* symbolic link */
#define	IFSOCK		0140000		/* socket */

#define	ISUID		04000		/* set user id on execution */
#define	ISGID		02000		/* set group id on execution */
#define	ISVTX		01000		/* save swapped text even after use */
#define	IREAD		0400		/* read, write, execute permissions */
#define	IWRITE		0200
#define	IEXEC		0100
#endif	/* _UFS_DINODE_H_ */
