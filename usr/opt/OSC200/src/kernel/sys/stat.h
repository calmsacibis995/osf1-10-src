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
 *	@(#)$RCSfile: stat.h,v $ $Revision: 4.3.4.3 $ (DEC) $Date: 1992/04/23 15:36:07 $
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
 * COMPONENT_NAME: SYSLFS - Logical File System
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#ifndef _SYS_STAT_H_
#define _SYS_STAT_H_

#include <standards.h>
#include <sys/types.h>
#include <sys/mode.h>

/*
 * POSIX requires that certain values be included in stat.h.  It also
 * requires that when _POSIX_SOURCE is defined only those standard
 * specific values are present.  This header includes all the POSIX
 * required entries.
 */

#ifdef _POSIX_SOURCE 

/*
 *	Stat structure
 *
 */

struct  stat
{
	dev_t	st_dev;			/* ID of device containing a directory*/
					/*   entry for this file.  File serial*/
					/*   no + device ID uniquely identify */
					/*   the file within the system */
	ino_t	st_ino;			/* File serial number */
	mode_t	st_mode;		/* File mode; see #define's below */
	nlink_t	st_nlink;		/* Number of links */
	uid_t	st_uid;			/* User ID of the file's owner */
	gid_t	st_gid;			/* Group ID of the file's group */
	dev_t	st_rdev;		/* ID of device */
					/*   This entry is defined only for */
					/*   character or block special files */
	off_t	st_size;		/* File size in bytes */
	time_t	st_atime;		/* Time of last access */
	int	st_spare1;
	time_t	st_mtime;		/* Time of last data modification */
	int	st_spare2;
	time_t	st_ctime;		/* Time of last file status change */
	int	st_spare3;
					/* Time measured in seconds since */
					/*   00:00:00 GMT, Jan. 1, 1970 */
	uint_t	st_blksize;		/* Size of block in file */
        int    st_blocks;              /* blocks allocated for file */
        uint_t  st_flags;               /* user defined flags for file */
        uint_t  st_gen;                 /* file generation number */

};
			/* End of the stat structure */


#ifndef _KERNEL
#ifdef _NO_PROTO
	extern int	mkdir(); 
	extern mode_t	umask(); 
	extern int	stat();
	extern int	fstat();
	extern int	chmod();
	extern int	mkfifo();
#else				/* use POSIX required prototypes */
#if defined(__STDC__) || defined(__cplusplus)
#if defined(__cplusplus)
extern "C"
{
#endif
	extern int	mkdir(const char *, mode_t ); 
	extern mode_t	umask(mode_t ); 
	extern int	stat(const char *, struct stat *);
	extern int	fstat(int , struct stat *);
	extern int	chmod(const char *, mode_t );
	extern int	mkfifo(const char *, mode_t );
#if defined(__cplusplus)
}
#endif
#endif
#endif /* _NO_PROTO */
#endif /* _KERNEL */

#endif /* _POSIX_SOURCE */

#ifdef _AES_SOURCE

#ifdef _NO_PROTO
	extern int	lstat();
#else 
#if defined(__STDC__) || defined(__cplusplus)
#if defined(__cplusplus)
extern "C"
{
#endif
	extern int 	lstat(const char *, struct stat *);
#if defined(__cplusplus)
}
#endif
#endif
#endif /* _NO_PROTO */

#endif /* _AES_SOURCE */

#ifdef _OSF_SOURCE

#define S_BLKSIZE       512     /* block size used in the stat struct */

#endif /* _OSF_SOURCE */
#endif /* _SYS_STAT_H_ */
