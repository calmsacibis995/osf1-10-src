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
 * @(#)$RCSfile: statvfs.h,v $ $Revision: 1.1.3.2 $ (DEC) $Date: 1992/04/14 13:46:04 $
 */

#ifndef _SYS_STATVFS_H
#define _SYS_STATVFS_H

/*
 * Structure returned by statvfs.
 */

#define	FSTYPSZ	16

typedef struct statvfs {
	u_long	f_bsize;		/* prefered file system block size */
	u_long	f_frsize;		/* fundamental file system block size */
	u_long	f_blocks;		/* total # of blocks of f_frsize on fs 
					 */
	u_long	f_bfree;		/* total # of free blocks of f_frsize */
	u_long	f_bavail;		/* # of free blocks avail to 
					   non-superuser */
	u_long	f_files;		/* total # of file nodes (inodes) */
	u_long	f_ffree;		/* total # of free file nodes */
	u_long	f_favail;		/* # of free nodes avail to 
					   non-superuser */
	u_long	f_fsid;			/* file system id */
	char	f_basetype[FSTYPSZ]; 	/* target fs type name, null-terminated
					 */
	u_long	f_flag;			/* bit-mask of flags */
	u_long	f_namemax;		/* maximum file name length */
	char	f_fstr[32];		/* filesystem-specific string */
} statvfs_t;

/*
 * Flag definitions.
 */

#define	ST_RDONLY	0x01	/* read-only file system */
#define	ST_NOSUID	0x02	/* does not support setuid/setgid semantics */

#if defined(__STDC__) && !defined(_KERNEL)
int statvfs(const char *, struct statvfs *);
int fstatvfs(int, struct statvfs *);
#endif

#endif	/* _SYS_STATVFS_H */
