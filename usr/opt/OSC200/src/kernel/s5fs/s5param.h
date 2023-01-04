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
 *	@(#)$RCSfile: s5param.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:52:12 $
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

#ifndef _S5_PARAM_H_
#define _S5_PARAM_H_
/*
 * Fundamental variables and macros for the System V file system
 * don't change too often
 * 
 */
			/* Some day we should move it to where it belong */

typedef ushort			s5ino_t;

#define	s5ROOTINO	((s5ino_t)2)	/* i number of all roots */

#define	SUPERB	((daddr_t)1)	/* physical block number of the super block */
#define	SUPERBOFF	512	/* byte offset of the super block */
#define	s5DIRSIZ	14	/* max characters per directory */
#define	NICINOD	100		/* number of superblock inodes */
#define	NICFREE	50		/* number of superblock free blocks */


#define MAX_S5BSIZE	2048	/* max bsize supported */

#define	FsBSIZE(fp) /* bsize of S5FS of fp */		\
	(((fp)->s_type == Fs3b) ? 2048 :		\
	 ((fp)->s_type == Fs2b) ? 1024 : 512 )


#define	FsBSHIFT(bsize) /* LOG2(bsize) */		\
	(((bsize) == 2048) ? 11 : 			\
	 ((bsize) == 1024) ? 10 : 9)

#define	FsNINDIR(bsize) /* bsize/sizeof(daddr_t)) */	\
	(((bsize) == 2048) ? 512 : 			\
	 ((bsize) == 1024) ? 256 : 128)

#define	FsBMASK(bsize)	/* bsize-1 */			\
	(((bsize) == 2048) ? 03777 : 			\
	 ((bsize) == 1024) ? 01777 : 0777)

#define	FsBOFF(bsize, x) /* offset in block */		\
	(((bsize) == 2048) ? ((x)&03777) :		\
	 ((bsize) == 1024) ? ((x)&01777) : ((x)&0777))

#define	FsBNO(bsize, x)	/* logical block number	*/	\
        (((bsize) == 2048) ? ((x)>>11) : 		\
	 ((bsize) == 1024) ? ((x)>>10) : ((x)>>9))

#define	FsINOPB(bsize)	/* inodes per block */		\
        (((bsize) == 2048) ? 32 : 			\
	 ((bsize) == 1024) ? 16 : 8)
	
#define	FsLTOP(bsize, b) /* logical to phys block */    \
        (((bsize) == 2048) ? (b)<<2 : 			\
	 ((bsize) == 1024) ? (b)<<1 : (b))

#define	FsPTOL(bsize, b)  /* phys to logical block */	\
        (((bsize) == 2048) ? (b)>>2 : 			\
	 ((bsize) == 1024) ? (b)>>1 : (b))

#define	FsNMASK(bsize) /*  NMASK <==> NINDIR - 1 */	\
        (((bsize) == 2048) ? 0777 : 			\
	 ((bsize) == 1024) ? 0377 : 0177)

#define	FsNSHIFT(bsize)	/* LOG2(NINDIR) */		\
        (((bsize) == 2048) ? 9 : 			\
	 ((bsize) == 1024) ? 8 : 7)

#define	FsINOS(bsize, x)					\
	(((bsize) == 2048) ? (((x)&~037)+1) : 			\
	 ((bsize) == 1024) ? (((x)&~017)+1) : (((x)&~07)+1) )

#define	FsITOD(bsize, x) /* Inode number to disk block */      \
        (daddr_t) (((bsize) == 2048) ? ((unsigned)(x)+63)>>5 : \
        (((bsize) == 1024) ? \
	((unsigned)(x)+31)>>4 : ((unsigned)(x)+15)>>3))

#define	FsITOO(bsize, x) /* Inode offset within disk block */   \
        (daddr_t) (((bsize) == 2048) ? ((unsigned)(x)+63)&037 : \
        ((bsize) == 1024) ? \
	((unsigned)(x)+31)&017 : ((unsigned)(x)+15)&07 )
#endif /* _S5_PARAM_H_ */
