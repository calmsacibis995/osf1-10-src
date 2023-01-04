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
 *	@(#)$RCSfile: fullstat.h,v $ $Revision: 4.3 $ (DEC) $Date: 1992/01/15 02:10:48 $
 */ 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * RESTRICTED RIGHTS LEGEND
 * Use, Duplication or Disclosure by the Government is subject to
 * restrictions as set forth in paragraph (b)(3)(B) of the rights in
 * Technical Data and Computer Software clause in DAR 7-104.9(a).
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
/*
 * OSF/1 Release 1.0
 *
 */

#ifndef _SYS_FULLSTAT_H_
#define _SYS_FULLSTAT_H_

#include <sys/stat.h>

/**********************************************************************
*
* PLEASE NOTE: This file is only staying around until a solution for
* fullstat() is reached....
*
**********************************************************************/
/*
 *	Fullstat structure
 *
 *	Since fullstat() is just a compatibility routine to statx(), the
 *	fullstat structure is the same as the stat structure.  Unfortunately,
 *	there is no way to equate structure definitions easily, so the
 *	fullstat structure has to be maintained separately from the stat
 *	structure.
 */

struct  fullstat
{	 /* copied from the stat structure (see sys/stat.h) */
	dev_t	st_dev;			/* ID of device containing a directory*/
					/*   entry for this file.  File serial*/
					/*   no + device ID uniquely identify */
					/*   the file within the system */
	ino_t	st_ino;			/* File serial number */
	mode_t	st_mode;		/* File mode; see #define's below */
	short	st_nlink;		/* Number of links */
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
	uint_t	st_blocks;		/* Actual number of blocks allocated */

	/********************************************************************/
	/****  End of initialized data for stat(), fstat(), and lstat()  ****/
	/********************************************************************/

	int	st_vfstype;		/* Type of fs (see vnode.h) */
	uint_t	st_vfs;			/* Vfs number */
	uint_t	st_type;		/* Vnode type */
	uint_t	st_gen;			/* Inode generation number */
	uint_t	st_flag;		/* Flag word */
	uid_t	st_uid_raw;		/* Untranslated uid */
	gid_t	st_gid_raw;		/* Untranslated gid */

	/*****************************************************************/
	/****  End of initialized data for fullstat() and ffullstat() ****/
	/*****************************************************************/

	ushort_t st_access;		/* Process' access to file */
	uint_t	st_spare4[5];		/* Reserved */
};

/*
 *	Compatibility macros
 */
#define	fst_type	st_type
#define	fst_vfstype	st_vfstype
#define	fst_vfs		st_vfs
#define	fst_i_gen	st_gen
#define	fst_flag	st_flag
#define	fst_uid_raw	st_uid_raw
#define	fst_gid_raw	st_gid_raw

/* The following fields have been removed.  These defines are TEMPORARY. */
#define	fst_uid_rev_tag	st_spare4[0]
#define	fst_gid_rev_tag	st_spare4[0]
#define	fst_nid		st_spare4[0]

/*
 * Defines for fullstat/ffullstat cmd argument
 */
#define	FL_STAT		STX_NORMAL	/* Normal fullstat		*/
#define	FL_STAT_REV	STX_NORMAL	/* Normal fullstat		*/
#define	FL_NOFOLLOW	STX_LINK	/* Do NOT follow symbolic links	*/
#define FL_STATMASK	0x00FF		/* mask for stat types		*/

#endif /* _SYS_FULLSTAT_H_ */
