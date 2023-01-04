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
#ifndef lint
static char *rcsid = "@(#)$RCSfile: disk.c,v $ $Revision: 4.2.4.2 $ (DEC) $Date: 1993/04/01 20:22:40 $";
#endif
/*
 * @OSF_COPYRIGHT@
 */
/*
 * HISTORY
 * $OSF_Log:	disk.c,v $
 * Revision 1.1.1.2  92/03/31  00:45:00  devrcs
 *  *** OSF1_1B25 version ***
 * 
 * Revision 1.6.2.2  1992/02/13  19:42:37  hosking
 * 	doc changes and added commented out disk_get_superblock() skeleton
 * 	as a reminder to support it some day
 * 	[1992/02/13  19:42:11  hosking]
 *
 * Revision 1.6  1991/03/04  17:43:32  devrcs
 * 	Comment out ident directives
 * 	[91/01/31  08:54:50  lehotsky]
 * 
 * Revision 1.5  91/01/07  15:57:26  devrcs
 * 	rcsid/RCSfile header cleanup
 * 	[90/12/01  17:27:15  dwm]
 * 
 * Revision 1.4  90/10/07  20:06:10  devrcs
 * 	Added EndLog Marker.
 * 	[90/09/28  20:13:17  gm]
 * 
 * Revision 1.3  90/07/17  12:18:55  devrcs
 * 	Internationalized
 * 	[90/07/05  07:23:27  staffan]
 * 
 * Revision 1.2  90/06/22  21:46:29  devrcs
 * 	Initial version from SecureWare
 * 	[90/05/31  11:09:35  staffan]
 * 
 * $OSF_EndLog$
 */
/*
 * Copyright (c) 1988-90 SecureWare, Inc.  All rights reserved.
 */

/* #ident "@(#)disk.c	2.1 16:17:31 4/20/90 SecureWare" */
/*
 * Based on:
 *   "@(#)disk.c	2.5 09:46:13 5/24/89 SecureWare, Inc."
 */

/*LINTLIBRARY*/

/*
 * This file contains a set of routines used to make programs
 * more secure.  Specifically, routines that manipulate the raw
 * disk are prepared to handle both secure and insecure inode formats.
 */

#include <sys/secdefines.h>

#if SEC_ARCH /*{*/

#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <sys/param.h>

#include <sys/security.h>
#include <sys/audit.h>
#include <prot.h>

#ifdef AUX
#include <sys/time.h>
#include <sys/vnode.h>
#include <svfs/inode.h>
#include <svfs/filsys.h>
#endif

#ifdef SYSV
#include <sys/fs/s5param.h>
#include <sys/fs/s5inode.h>
#include <sys/inode.h>
#include <sys/ino.h>
#ifdef SYSV_3
#include <sys/fs/s5filsys.h>
#else
#include <sys/filsys.h>
#endif
#endif

#ifdef _OSF_SOURCE
#include <ufs/inode.h>
#include <ufs/dinode.h>
#include <ufs/fs.h>
#endif

static int secure_file_system = 0;
static int dinode_size = 0;

#if defined(SYSV) || defined(AUX) /*{*/
static int inodes_per_block = 0;
static int inode_shift = 0;

static int shift();

#if notyet
/*
 * This should be added officially some time... but not for 1.1,
 * due to doc impact, etc.  (Needs porting from SYSV format)
 */

int disk_get_superblock(fd, fs)
struct filsys *fs;
{
	long current_pos = lseek(fd, 0L, 1);	/* save current position */
	if (lseek(fd, SUPERBOFF, 0) < 0)	/* seek to superblock */
		return(-1);
	if (read(fd, (char *) fs, sizeof(struct filsys))
		!= sizeof(struct filsys))	/* get it */
		return(-1);
	if (lseek(fd, current_pos, 0))		/* put file pointer back */
		return(-1);
	/* ninodes = sb.s_isize; 	for ginode() sanity checks */
	return(0);
}
#endif

/*
 * Based on the fs argument, determine the type of file system, whether
 * secure or insecure.  Compute some constants that are to be used by
 * other routines.
 *
 * This routine MUST be called once each time a file system is opened.
 * It initializes constants unique to the file system.  Other routines
 * in this file assume these constants have been properly initialized
 * before those routines are called.
 * The routines in this file allow only one file system to be open
 * at any given time, and expect that the caller will do the following
 * (in order) for EACH file system:
 *   use open(2) with at least read access
 *   get the file system superblock (use disk_get_superblock() if available)
 *   call this routine to initialize file system constants
 *   call any other routines in this file
 *   use close(2) when finished with a file system. 
 */

void
disk_set_file_system(fs, block_size)
	struct filsys *fs;
	int block_size;
{
	secure_file_system = ((fs->s_type & FsSW) != 0);

	dinode_size = secure_file_system
#if defined(AUX)
			? sizeof(struct sec_dinode)
			: sizeof(struct dinode);
#else
			? sizeof(struct dinode)
			: sizeof(struct insec_dinode);
#endif

	inodes_per_block = block_size / dinode_size;

	inode_shift = shift();
}
#else
/*
 * All BSD constants are built into the macros.
 */
void
disk_set_file_system(fs, block_size)
	struct fs *fs;
	int block_size;
{
	if (secure_file_system = FsSEC(fs))
		dinode_size = sizeof(struct sec_dinode);
	else
		dinode_size = sizeof(struct dinode);
}
#endif /*}*/


/*
 * Return 1 if the file system is secure and 0 if not.
 */
int
disk_secure_file_system()
{
	return secure_file_system;
}


/*
 * Increment the inode pointer by incr number of entries.  The hard
 * value of the result depends on the type of file system.
 */
void
disk_inode_incr(dp, incr)
	struct dinode **dp;
	int incr;
{
#if defined(AUX) || defined(_OSF_SOURCE)
	if (secure_file_system)
		*(struct sec_dinode **)dp += incr;
	else
		*dp += incr;
#else
	if (secure_file_system)
		*dp = *dp + incr;
	else
		*dp = (struct dinode *) ((struct insec_dinode *) *dp + incr);
#endif
}


/*
 * Return the size of an inode on the current file system being used.
 */
int
disk_dinode_size()
{
	return dinode_size;
}


#if !defined(_OSF_SOURCE) /*{*/
/*
 * Return 1 if the file system appears valid, and 0 if not.
 * A file system, to be valid, is necessary to satisfy the following
 * conditions:
 *
 *	1. The total blocks exceeds smaller totals, like free blocks
 *	   and the last inode block.
 *	2. The total number of inode blocks must be more than the
 *	   block count of free inodes.
 *	3. The number of inodes must fit into a short, or some inodes
 * 	   cannot be references.
 */
int
disk_valid(fs)
	register struct filsys *fs;
{
	register int total_inodes;

	total_inodes = (fs->s_isize - (SUPERB+1)) * inodes_per_block;

	return (int) ((fs->s_fsize > fs->s_isize) &&
		      (fs->s_fsize > fs->s_tfree) &&
		      (total_inodes >= fs->s_tinode) &&
		      ((ulong) total_inodes < ((ulong) ((ushort) ~0)) + 1));
}


/*
 * Given an inode number, return the file system disk block containing
 * that inode.
 */
daddr_t
disk_itod(inode_number)
	int inode_number;
{
	return (daddr_t) (((unsigned)inode_number + (2*inodes_per_block-1)) >>
		inode_shift);
}


/*
 * Given an inode number, return the offset within the file system disk
 * block containing that inode.
 */
daddr_t
disk_itoo(inode_number)
	int inode_number;
{
	return (daddr_t) (((unsigned)inode_number + (2*inodes_per_block-1)) &
		(inodes_per_block-1));
}


/*
 * Return the number of inodes per block on the file system.
 */
int
disk_inodes_per_block()
{
	return inodes_per_block;
}

/*
 * Compute the amount of shifting needed for the given inode size.  The
 * results are used in calculations of inode disk block locations.
 */
static int
shift()
{
	register int shifts = 0;
	register int value;

	value = inodes_per_block >> 1;
	while (value > 0)  {
		shifts++;
		value >>= 1;
	}

	return shifts;
}
#endif /*}*/

#ifdef _OSF_SOURCE /*{*/
/*
 * increment a block pointer to point to the dinode entry corresponding
 * to the inode number passed in.  Acts differently for secure and non-secure
 * file systems.
 */
void
disk_inode_in_block(fs, bp, dipp, inumber)
struct fs *fs;
char *bp;
struct dinode **dipp;
ino_t inumber;
{
	int i = itoo(fs, inumber);

	if (FsSEC(fs))
		*dipp = (struct dinode *) (((struct sec_dinode *) bp) + i);
	else
		*dipp = ((struct dinode *) bp) + i;
}
#endif /*}*/

#endif /*} SEC_ARCH */
