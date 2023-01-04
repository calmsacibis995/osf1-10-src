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
 *	@(#)fs.h	9.2	(ULTRIX/OSF)	10/18/91
 */ 
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
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
 * Copyright (c) 1982, 1986 Regents of the University of California.
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

#ifndef	_UFS_FS_H_
#define _UFS_FS_H_

#ifdef	_KERNEL
#include <sys/unix_defs.h>
#endif

#include <sys/secdefines.h>

#include <sys/types.h>
#include <sys/param.h>

#ifdef	_KERNEL
#include <kern/zalloc.h>
#endif

#ifndef	_EXT_TIME_T
#define _EXT_TIME_T
		/* HACK:  Should fix definition of time_t in types.h */
typedef	int	ext_time_t;	/* external representation of time_t */
#endif

/*
 * Each disk drive contains some number of file systems.
 * A file system consists of a number of cylinder groups.
 * Each cylinder group has inodes and data.
 *
 * A file system is described by its super-block, which in turn
 * describes the cylinder groups.  The super-block is critical
 * data and is replicated in each cylinder group to protect against
 * catastrophic loss.  This is done at `newfs' time and the critical
 * super-block data does not change, so the copies need not be
 * referenced further unless disaster strikes.
 *
 * For file system fs, the offsets of the various blocks of interest
 * are given in the super block as:
 *	[fs->fs_sblkno]		Super-block
 *	[fs->fs_cblkno]		Cylinder group block
 *	[fs->fs_iblkno]		Inode blocks
 *	[fs->fs_dblkno]		Data blocks
 * The beginning of cylinder group cg in fs, is given by
 * the ``cgbase(fs, cg)'' macro.
 *
 * The first boot and super blocks are given in absolute disk addresses.
 * The byte-offset forms are preferred, as they don't imply a sector size.
 */
#ifdef   __hp_osf
#define BBSIZE		10240
#define SBSIZE		8192
#define	BBOFF		((off_t)(2048))
#define	SBOFF		((off_t)(BBOFF + BBSIZE))
#define	BBLOCK		((daddr_t)(2))
#define	SBLOCK		((daddr_t)(BBLOCK + BBSIZE / DEV_BSIZE))
#else   /* __hp_osf */
#define BBSIZE		8192
#define SBSIZE		8192
#define	BBOFF		((off_t)(0))
#define	SBOFF		((off_t)(BBOFF + BBSIZE))
#define	BBLOCK		((daddr_t)(0))
#define	SBLOCK		((daddr_t)(BBLOCK + BBSIZE / DEV_BSIZE))
#endif  /* __hp_osf */

/*
 * Addresses stored in inodes are capable of addressing fragments
 * of `blocks'. File system blocks of at most size MAXBSIZE can 
 * be optionally broken into 2, 4, or 8 pieces, each of which is
 * addressible; these pieces may be DEV_BSIZE, or some multiple of
 * a DEV_BSIZE unit.
 *
 * Large files consist of exclusively large data blocks.  To avoid
 * undue wasted disk space, the last data block of a small file may be
 * allocated as only as many fragments of a large block as are
 * necessary.  The file system format retains only a single pointer
 * to such a fragment, which is a piece of a single large block that
 * has been divided.  The size of such a fragment is determinable from
 * information in the inode, using the ``blksize(fs, ip, lbn)'' macro.
 *
 * The file system records space availability at the fragment level;
 * to determine block availability, aligned fragments are examined.
 *
 * The root inode is the root of the file system.
 * Inode 0 can't be used for normal purposes and
 * historically bad blocks were linked to inode 1,
 * thus the root inode is 2. (inode 1 is no longer used for
 * this purpose, however numerous dump tapes make this
 * assumption, so we are stuck with it)
 */
#define	ROOTINO		((ino_t)2)	/* i number of all roots */

/*
 * MINBSIZE is the smallest allowable block size.
 * In order to insure that it is possible to create files of size
 * 2^32 with only two levels of indirection, MINBSIZE is set to 4096.
 * MINBSIZE must be big enough to hold a cylinder group block,
 * thus changes to (struct cg) must keep its size within MINBSIZE.
 * Note that super blocks are always of size SBSIZE,
 * and that both SBSIZE and MAXBSIZE must be >= MINBSIZE.
 */
#define MINBSIZE	4096

/*
 * The path name on which the file system is mounted is maintained
 * in fs_fsmnt. MAXMNTLEN defines the amount of space allocated in 
 * the super block for this name.
 * The limit on the amount of summary information per file system
 * is defined by MAXCSBUFS. It is currently parameterized for a
 * maximum of two million cylinders.
 */
#define MAXMNTLEN 512
#define MAXCSBUFS 32
/*
 * Alpha needs some additional MEMORY area for the superblock due to
 * increased pointer sizes, so the memory copy of the superblock =
 * SBSIZE + ALPHA_EXT.  Note that the disk copy is not changed.
 * ALPHA_PAD is the extension of the superblock out to 8192 bytes.
 * It is used to push the cylinder summary pointers out of the disk
 * image and into an extension which exists only in memory.
 */
#ifdef __alpha
#define ALPHA_EXT 512
#define ALPHA_PAD 6808
#endif /* __alpha */

/*
 * Per cylinder group information; summarized in blocks allocated
 * from first cylinder group data blocks.  These blocks have to be
 * read in from fs_csaddr (size fs_cssize) in addition to the
 * super block.
 *
 * N.B. sizeof(struct csum) must be a power of two in order for
 * the ``fs_cs'' macro to work (see below).
 */
struct csum {
	int	cs_ndir;	/* number of directories */
	int	cs_nbfree;	/* number of free blocks */
	int	cs_nifree;	/* number of free inodes */
	int	cs_nffree;	/* number of free frags */
};

/*
 * Super block for a file system.
 */
#define	FS_MAGIC	0x011954

#if	SEC_FSCHANGE
#define FS_SEC_MAGIC    0x80011954
#define FsSEC(fs)       ((fs)->fs_magic == FS_SEC_MAGIC)
#endif 

/* #define      FS_CLEAN        0x2 */  /* file system is clean */
#define FS_CLEAN	0x3		/* fsck directory corruption fix */


struct	fs
{
#ifdef __alpha
	int	fs_link;		/* list pointers unused on BSD systems*/
	int	fs_rlink;		/* list pointers unused on BSD systems*/
#else
	struct	fs *fs_link;		/* linked list of file systems */
	struct	fs *fs_rlink;		/*     used for incore super blocks */
#endif /* __alpha */
	daddr_t	fs_sblkno;		/* addr of super-block in filesys */
	daddr_t	fs_cblkno;		/* offset of cyl-block in filesys */
	daddr_t	fs_iblkno;		/* offset of inode-blocks in filesys */
	daddr_t	fs_dblkno;		/* offset of first data after cg */
	int	fs_cgoffset;		/* cylinder group offset in cylinder */
	int	fs_cgmask;		/* used to calc mod fs_ntrak */
	ext_time_t 	fs_time;    		/* last time written */
	int	fs_size;		/* number of blocks in fs */
	int	fs_dsize;		/* number of data blocks in fs */
	int	fs_ncg;			/* number of cylinder groups */
	int	fs_bsize;		/* size of basic blocks in fs */
	int	fs_fsize;		/* size of frag blocks in fs */
	int	fs_frag;		/* number of frags in a block in fs */
/* these are configuration parameters */
	int	fs_minfree;		/* minimum percentage of free blocks */
	int	fs_rotdelay;		/* num of ms for optimal next block */
	int	fs_rps;			/* disk revolutions per second */
/* these fields can be computed from the others */
	int	fs_bmask;		/* ``blkoff'' calc of blk offsets */
	int	fs_fmask;		/* ``fragoff'' calc of frag offsets */
	int	fs_bshift;		/* ``lblkno'' calc of logical blkno */
	int	fs_fshift;		/* ``numfrags'' calc number of frags */
/* these are configuration parameters */
	int	fs_maxcontig;		/* max number of contiguous blks */
	int	fs_maxbpg;		/* max number of blks per cyl group */
/* these fields can be computed from the others */
	int	fs_fragshift;		/* block to frag shift */
	int	fs_fsbtodb;		/* fsbtodb and dbtofsb shift constant*/
	int	fs_sbsize;		/* actual size of super block */
	int	fs_csmask;		/* csum block offset */
	int	fs_csshift;		/* csum block number */
	int	fs_nindir;		/* value of NINDIR */
	int	fs_inopb;		/* value of INOPB */
	int	fs_nspf;		/* value of NSPF */
/* yet another configuration parameter */
	int	fs_optim;		/* optimization preference, see below */
/* these fields are derived from the hardware */
	int	fs_npsect;		/* # sectors/track including spares */
	int	fs_interleave;		/* hardware sector interleave */
	int	fs_trackskew;		/* sector 0 skew, per track */
	int	fs_headswitch;		/* head switch time, usec */
	int	fs_trkseek;		/* track-to-track seek, usec */
/* sizes determined by number of cylinder groups and their sizes */
	daddr_t fs_csaddr;		/* blk addr of cyl grp summary area */
	int	fs_cssize;		/* size of cyl grp summary area */
	int	fs_cgsize;		/* cylinder group size */
/* these fields are derived from the hardware */
	int	fs_ntrak;		/* tracks per cylinder */
	int	fs_nsect;		/* sectors per track */
	int  	fs_spc;   		/* sectors per cylinder */
/* this comes from the disk driver partitioning */
	int	fs_ncyl;   		/* cylinders in file system */
/* these fields can be computed from the others */
	int	fs_cpg;			/* cylinders per group */
	int	fs_ipg;			/* inodes per group */
	int	fs_fpg;			/* blocks per group * fs_frag */
/* this data must be re-computed after crashes */
	struct	csum fs_cstotal;	/* cylinder summary information */
/* these fields are cleared at mount time */
	char   	fs_fmod;    		/* super block modified flag */
	char   	fs_clean;    		/* file system is clean flag */
	char   	fs_ronly;   		/* mounted read-only flag */
	char   	fs_flags;   		/* fs_clean save area */
	char	fs_fsmnt[MAXMNTLEN];	/* name mounted on */
/* these fields retain the current block allocation info */
	int	fs_cgrotor;		/* last cg searched */
#ifdef __alpha
	int	fs_blank[MAXCSBUFS];	/* Unused in alpha */
#else
	struct	csum *fs_csp[MAXCSBUFS];/* list of fs_cs info buffers */
#endif
	int	fs_cpc;			/* cyl per cycle in postbl */
	short	fs_opostbl[16][8];	/* old rotation block list head */
	union {
		int	fsu_sparecon[56]; /* reserved for future constants */
#ifdef	_KERNEL
		udecl_simple_lock_data(,fsu_lock) /* see notes below */
#endif
	} fs_un;
	quad	fs_qbmask;		/* ~fs_bmask, for use with quad size */
	quad	fs_qfmask;		/* ~fs_fmask, for use with quad size */
	int	fs_postblformat;	/* format of positional layout tables*/
	int	fs_nrpos;		/* number of rotaional positions */
	int	fs_postbloff;		/* (short) rotation block list head */
	int	fs_rotbloff;		/* (u_char) blocks for each rotation */
	int	fs_magic;		/* magic number */
	u_char	fs_space[1];		/* list of blocks for each rotation */
#ifdef __alpha
	u_char	fs_xxx[ALPHA_PAD];	/* used for rotation, etc. info */
	struct	csum *fs_csp[MAXCSBUFS];/* list of fs_cs info buffers */
#endif /* __alpha */
/* actually longer */
};

#ifdef	_KERNEL
#define	fs_lock		fs_un.fsu_lock
#endif
#define	fs_sparecon	fs_un.fsu_sparecon

#ifdef	_KERNEL
#define FS_LOCK(fs)		usimple_lock(&(fs)->fs_lock)
#define	FS_UNLOCK(fs)		usimple_unlock(&(fs)->fs_lock)
#define	FS_LOCK_INIT(fs)	usimple_lock_init(&(fs)->fs_lock)
#define	FS_LOCK_HOLDER(fs)	SLOCK_HOLDER(&(fs)->fs_lock)
#endif

/*
 * Cylinder Group Locking.
 * =======================
 * The cylinder group has no lock.  Cylinder groups live in buffer
 * cache buffers and are therefore implicitly protected by the
 * buffer lock.
 *
 * Filesystem (Superblock) Locking.
 * ================================
 * Destructive access to superblock fields is serialized by a per-
 * superblock simple lock, fsu_lock.  This field is only used while
 * the superblock resides in-core and is in fact stolen from the
 * spare on-disk superblock fields (fsu_sparecon).
 *
 * Callers of routines that use superblocks must guarantee that the
 * filesystem does not become unmounted while those routines do their
 * work.  In the typical case, the caller holds a referenced vnode on
 * the filesystem so that unmount attempts will fail.  In extraordinary
 * cases, the caller provides some other guarantee; for example, the
 * caller may hold the associated mount structure's pathname translation
 * lock.
 *
 * Because of the nature of the information kept in the struct fs,
 * we define a locking policy that allows us to read most of this
 * information without actually having to take the filesystem's lock.
 * On machines with appropriate processor/cache/memory architectures,
 * a number of fields can be examined without taking the superblock
 * lock although the superblock lock must always be acquired before
 * modifying any field.  (Refer to sys/unix_defs.h for an explanation
 * of Bogus Memory.)
 *
 * In the table below, Read-Only means that the field is never modified
 * after it is read from disk, or after the field is initialized in the
 * mount code, so no lock need be taken to examine the contents of the
 * field.  Mandatory means that the superblock lock must be taken whether
 * examining or modifying the field.  Optional means that the superblock
 * lock does not have to be taken to examine the field (on some machines)
 * but the filesystem's lock must still be taken when modifying the field.
 *
 * Be wary of code that modifies a field under lock based on decisions
 * made from examining Optional fields without holding a lock.
 *
 *	Field		Comment					  State
 *	-----		-------					  -----
 *	fs_link		linked list of superblocks		Not used
 *	fs_rlink						Not used
 *	fs_sblkno	addr of super-block in filesys		Read-Only
 *	fs_cblkno	offset of cyl-block in filesys		Read-Only
 *	fs_iblkno	offset of inode-blocks in filesys	Read-Only
 *	fs_dblkno	offset of first data after cg		Read-Only
 *	fs_cgoffset	cylinder group offset in cylinder	Read-Only
 *	fs_cgmask	used to calc mod fs_ntrak		Read-Only
 *	fs_time    	last time written (64 bits)		Mandatory
 *	fs_size		number of blocks in fs			Read-Only
 *	fs_dsize	number of data blocks in fs		Read-Only
 *	fs_ncg		number of cylinder groups		Read-Only
 *	fs_bsize	size of basic blocks in fs		Read-Only
 *	fs_fsize	size of frag blocks in fs		Read-Only
 *	fs_frag		number of frags in a block in fs	Read-Only
 *	fs_minfree	minimum percentage of free blocks	Read-Only
 *	fs_rotdelay	num of ms for optimal next block	Read-Only
 *	fs_rps		disk revolutions per second		Read-Only
 *	fs_bmask	``blkoff'' calc of blk offsets		Read-Only
 *	fs_fmask	``fragoff'' calc of frag offsets	Read-Only
 *	fs_bshift	``lblkno'' calc of logical blkno	Read-Only
 *	fs_fshift	``numfrags'' calc number of frags	Read-Only
 *	fs_maxcontig	max number of contiguous blks		Read-Only
 *	fs_maxbpg	max number of blks per cyl group	Read-Only
 *	fs_fragshift	block to frag shift			Read-Only
 *	fs_fsbtodb	fsbtodb and dbtofsb shift constant	Read-Only
 *	fs_sbsize	actual size of super block		Read-Only
 *	fs_csmask	csum block offset			Read-Only
 *	fs_csshift	csum block number			Read-Only
 *	fs_nindir	value of NINDIR				Read-Only
 *	fs_inopb	value of INOPB				Read-Only
 *	fs_nspf		value of NSPF				Read-Only

 *	fs_optim	optimization preference, see below	Optional

 *	fs_sparecon	reserved for future constants		Optional
v. fs_npsect, interleave, trackskew, headswitch, trkseek

 *	fs_csaddr	blk addr of cyl grp summary area	Read-Only
 *	fs_cssize	size of cyl grp summary area		Read-Only
 *	fs_cgsize	cylinder group size			Read-Only
 *	fs_ntrak	tracks per cylinder			Read-Only
 *	fs_nsect	sectors per track			Read-Only
 *	fs_spc   	sectors per cylinder			Read-Only
 *	fs_ncyl   	cylinders in file system		Read-Only
 *	fs_cpg		cylinders per group			Read-Only
 *	fs_ipg		inodes per group			Read-Only
 *	fs_fpg		blocks per group * fs_frag		Read-Only
 *	fs_cstotal	cylinder summary information		Mandatory
 *	fs_fmod    	super block modified flag		Optional
 *	fs_clean    	file system is clean flag		Optional
 *	fs_ronly   	mounted read-only flag			Read-Only
 *	fs_flags   	fs clean save area			Optional
 *	fs_fsmnt	name mounted on				Read-Only XXX
 *	fs_cgrotor	last cg searched			Mandatory
 *	fs_csp		list of fs_cs info buffers		Mandatory
 *	fs_cpc		cyl per cycle in postbl			Read-Only
 *	fs_opostbl	head of blocks for each rotation	Read-Only

 *	fs_un

 *	fs_qbmask						Read-Only
 *	fs_qfmask						Read-Only
 *	fs_postblformat						Read-Only
 *	fs_nrpos
 *	fs_postbloff
 *	fs_rotbloff

 *	fs_magic	magic number				Read-Only
 *	fs_rotbl	list of blocks for each rotation	Read-Only XXX
 *
 *	fs_space
 */

/*
 * Preference for optimization.
 */
#define FS_OPTTIME	0	/* minimize allocation time */
#define FS_OPTSPACE	1	/* minimize disk fragmentation */

/*
 * Rotational layout table format types
 */
#define FS_42POSTBLFMT		-1	/* 4.2BSD rotational table format */
#define FS_DYNAMICPOSTBLFMT	1	/* dynamic rotational table format */
/*
 * Macros for access to superblock array structures
 */
#define fs_postbl(fs, cylno) \
    (((fs)->fs_postblformat == FS_42POSTBLFMT) \
    ? ((fs)->fs_opostbl[cylno]) \
    : ((short *)((char *)(fs) + (fs)->fs_postbloff) + (cylno) * (fs)->fs_nrpos))
#define fs_rotbl(fs) \
    (((fs)->fs_postblformat == FS_42POSTBLFMT) \
    ? ((fs)->fs_space) \
    : ((u_char *)((char *)(fs) + (fs)->fs_rotbloff)))

/*
 * Convert cylinder group to base address of its global summary info.
 *
 * N.B. This macro assumes that sizeof(struct csum) is a power of two.
 */
#define fs_cs(fs, indx) \
	fs_csp[(indx) >> (fs)->fs_csshift][(indx) & ~(fs)->fs_csmask]

/*
 * Cylinder group block for a file system.
 *
 * Cylinder group locking explained above.
 */
#define	CG_MAGIC	0x090255
struct	cg {
#ifdef __alpha
	int	cg_link[1];		/* lists not used on BSD */
#else
	struct	cg *cg_link;		/* linked list of cyl groups */
#endif /* __alpha */
	int	cg_magic;		/* magic number */
	ext_time_t	cg_time;		/* time last written */
	int	cg_cgx;			/* we are the cgx'th cylinder group */
	short	cg_ncyl;		/* number of cyl's this cg */
	short	cg_niblk;		/* number of inode blocks this cg */
	int	cg_ndblk;		/* number of data blocks this cg */
	struct	csum cg_cs;		/* cylinder summary information */
	int	cg_rotor;		/* position of last used block */
	int	cg_frotor;		/* position of last used frag */
	int	cg_irotor;		/* position of last used inode */
	int	cg_frsum[MAXFRAG];	/* counts of available frags */
	int	cg_btotoff;		/* (long) block totals per cylinder */
	int	cg_boff;		/* (short) free block positions */
	int	cg_iusedoff;		/* (char) used inode map */
	int	cg_freeoff;		/* (u_char) free block map */
	int	cg_nextfreeoff;		/* (u_char) next available space */
	int	cg_sparecon[16];	/* reserved for future use */
	u_char	cg_space[1];		/* space for cylinder group maps */
/* actually longer */
};
/*
 * Macros for access to cylinder group array structures
 */
#define cg_blktot(cgp) \
    (((cgp)->cg_magic != CG_MAGIC) \
    ? (((struct ocg *)(cgp))->cg_btot) \
    : ((int *)((char *)(cgp) + (cgp)->cg_btotoff)))
#define cg_blks(fs, cgp, cylno) \
    (((cgp)->cg_magic != CG_MAGIC) \
    ? (((struct ocg *)(cgp))->cg_b[cylno]) \
    : ((short *)((char *)(cgp) + (cgp)->cg_boff) + (cylno) * (fs)->fs_nrpos))
#define cg_inosused(cgp) \
    (((cgp)->cg_magic != CG_MAGIC) \
    ? (((struct ocg *)(cgp))->cg_iused) \
    : ((char *)((char *)(cgp) + (cgp)->cg_iusedoff)))
#define cg_blksfree(cgp) \
    (((cgp)->cg_magic != CG_MAGIC) \
    ? (((struct ocg *)(cgp))->cg_free) \
    : ((u_char *)((char *)(cgp) + (cgp)->cg_freeoff)))
#define cg_chkmagic(cgp) \
    ((cgp)->cg_magic == CG_MAGIC || ((struct ocg *)(cgp))->cg_magic == CG_MAGIC)

/*
 * The following structure is defined
 * for compatibility with old file systems.
 */
struct	ocg {
#ifdef __alpha
	int	cg_link;		/* lists not used on BSD */
	int	cg_rlink;		/* lists not used on BSD */
#else
	struct	ocg *cg_link;		/* linked list of cyl groups */
	struct	ocg *cg_rlink;		/*     used for incore cyl groups */
#endif /* __alpha */
	ext_time_t	cg_time;		/* time last written */
	int	cg_cgx;			/* we are the cgx'th cylinder group */
	short	cg_ncyl;		/* number of cyl's this cg */
	short	cg_niblk;		/* number of inode blocks this cg */
	int	cg_ndblk;		/* number of data blocks this cg */
	struct	csum cg_cs;		/* cylinder summary information */
	int	cg_rotor;		/* position of last used block */
	int	cg_frotor;		/* position of last used frag */
	int	cg_irotor;		/* position of last used inode */
	int	cg_frsum[8];		/* counts of available frags */
	int	cg_btot[32];		/* block totals per cylinder */
	short	cg_b[32][8];		/* positions of free blocks */
	char	cg_iused[256];		/* used inode map */
	int	cg_magic;		/* magic number */
	u_char	cg_free[1];		/* free block map */
/* actually longer */
};

/*
 * Turn file system block numbers into disk block addresses.
 * This maps file system blocks to device size blocks.
 */
#define fsbtodb(fs, b)	((b) << (fs)->fs_fsbtodb)
#define	dbtofsb(fs, b)	((b) >> (fs)->fs_fsbtodb)

/*
 * Cylinder group macros to locate things in cylinder groups.
 * They calc file system addresses of cylinder group data structures.
 */
#define	cgbase(fs, c)	((daddr_t)((fs)->fs_fpg * (c)))
#define cgstart(fs, c) \
	(cgbase(fs, c) + (fs)->fs_cgoffset * ((c) & ~((fs)->fs_cgmask)))
#define	cgsblock(fs, c)	(cgstart(fs, c) + (fs)->fs_sblkno)	/* super blk */
#define	cgtod(fs, c)	(cgstart(fs, c) + (fs)->fs_cblkno)	/* cg block */
#define	cgimin(fs, c)	(cgstart(fs, c) + (fs)->fs_iblkno)	/* inode blk */
#define	cgdmin(fs, c)	(cgstart(fs, c) + (fs)->fs_dblkno)	/* 1st data */

/*
 * Macros for handling inode numbers:
 *     inode number to file system block offset.
 *     inode number to cylinder group number.
 *     inode number to file system block address.
 */
#define	itoo(fs, x)	((x) % INOPB(fs))
#define	itog(fs, x)	((x) / (fs)->fs_ipg)
#define	itod(fs, x) \
	((daddr_t)(cgimin(fs, itog(fs, x)) + \
	(blkstofrags((fs), (((x) % (fs)->fs_ipg) / INOPB(fs))))))

/*
 * Give cylinder group number for a file system block.
 * Give cylinder group block number for a file system block.
 */
#define	dtog(fs, d)	((d) / (fs)->fs_fpg)
#define	dtogd(fs, d)	((d) % (fs)->fs_fpg)

/*
 * Extract the bits for a block from a map.
 * Compute the cylinder and rotational position of a cyl block addr.
 */
#define blkmap(fs, map, loc) \
    (((map)[(loc) / NBBY] >> ((loc) % NBBY)) & (0xff >> (NBBY -(fs)->fs_frag)))
#define cbtocylno(fs, bno) \
    ((bno) * NSPF(fs) / (fs)->fs_spc)
#define cbtorpos(fs, bno) \
    (((bno) * NSPF(fs) % (fs)->fs_spc / (fs)->fs_nsect * (fs)->fs_trackskew + \
     (bno) * NSPF(fs) % (fs)->fs_spc % (fs)->fs_nsect * (fs)->fs_interleave) %\
     (fs)->fs_nsect * (fs)->fs_nrpos / (fs)->fs_npsect)

/*
 * The following macros optimize certain frequently calculated
 * quantities by using shifts and masks in place of divisions
 * modulos and multiplications.
 */
#define blkoff(fs, loc)		/* calculates (loc % fs->fs_bsize) */ \
	((loc) & ~(fs)->fs_bmask)
#define fragoff(fs, loc)	/* calculates (loc % fs->fs_fsize) */ \
	((loc) & ~(fs)->fs_fmask)
#define lblktosize(fs, blk)	/* calculates (blk * fs->fs_bsize) */ \
 	((blk) << (fs)->fs_bshift)
#define lblkno(fs, loc)		/* calculates (loc / fs->fs_bsize) */ \
	((loc) >> (fs)->fs_bshift)
#define numfrags(fs, loc)	/* calculates (loc / fs->fs_fsize) */ \
	((loc) >> (fs)->fs_fshift)
#define blkroundup(fs, size)	/* calculates roundup(size, fs->fs_bsize) */ \
	(((size) + (fs)->fs_bsize - 1) & (fs)->fs_bmask)
#define fragroundup(fs, size)	/* calculates roundup(size, fs->fs_fsize) */ \
	(((size) + (fs)->fs_fsize - 1) & (fs)->fs_fmask)
#define fragstoblks(fs, frags)	/* calculates (frags / fs->fs_frag) */ \
	((frags) >> (fs)->fs_fragshift)
#define blkstofrags(fs, blks)	/* calculates (blks * fs->fs_frag) */ \
	((blks) << (fs)->fs_fragshift)
#define fragnum(fs, fsb)	/* calculates (fsb % fs->fs_frag) */ \
	((fsb) & ((fs)->fs_frag - 1))
#define blknum(fs, fsb)		/* calculates rounddown(fsb, fs->fs_frag) */ \
	((fsb) &~ ((fs)->fs_frag - 1))

/*
 * Determine the number of available frags given a
 * percentage to hold in reserve
 */
#define freespace(fs, percentreserved) \
	(blkstofrags((fs), (fs)->fs_cstotal.cs_nbfree) + \
	(fs)->fs_cstotal.cs_nffree - \
	((fs)->fs_dsize * (long)(percentreserved) /100))

/*
 * Determining the size of a file block in the file system.
 */
#define blksize(fs, ip, lbn) \
	(((lbn) >= NDADDR || (ip)->i_size >= ((lbn) + 1) << (fs)->fs_bshift) \
	    ? (fs)->fs_bsize \
	    : (fragroundup(fs, blkoff(fs, (ip)->i_size))))
#define dblksize(fs, dip, lbn) \
	(((lbn) >= NDADDR || (dip)->di_size >= ((lbn) + 1) <<(fs)->fs_bshift) \
	    ? (fs)->fs_bsize \
	    : (fragroundup(fs, blkoff(fs, (dip)->di_size))))

/*
 * Number of disk sectors per block; assumes DEV_BSIZE byte sector size.
 */
#define	NSPB(fs)	((fs)->fs_nspf << (fs)->fs_fragshift)
#define	NSPF(fs)	((fs)->fs_nspf)

/*
 * INOPB is the number of inodes in a secondary storage block.
 */
#define	INOPB(fs)	((fs)->fs_inopb)
#if	!SEC_FSCHANGE
/*
 * This macro does not work on a labeled filesystem, since the inode
 * size is not a power of 2 and thus can't be guaranteed not to straddle
 * a fragment boundary.  There are currently no known uses of this macro,
 * but if we are building for labeled filesystems we comment it out just
 * to be sure.
 */
#define	INOPF(fs)	((fs)->fs_inopb >> (fs)->fs_fragshift)
#endif

/*
 * NINDIR is the number of indirects in a file system block.
 */
#define	NINDIR(fs)	((fs)->fs_nindir)

#ifdef	_KERNEL
extern zone_t	superblock_zone;
struct	fs *getfs();
#endif

#endif	/* _UFS_FS_H_ */
