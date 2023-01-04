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
 *	@(#)$RCSfile: layout.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 06:23:18 $
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

 *
 */

/*2
 *  DISK LAYOUT
 *
 *    Must match structure used by UMAX4.2 and SCC ROM CODE:
 */

#define OS_TYPE_UNDEFINED	0
#define OS_TYPE_UMAX_4_2	1
#define OS_TYPE_UMAX_V		2
#define OS_TYPE_MACH		3
#define OS_TYPE_COMMON		4

#define OS_TYPES		\
    char			\
	*os_types[] = {		\
	    "Undefined",	\
	    "Umax 4.2",		\
	    "Umax V",		\
	    "Mach",		\
	    "Common"		\
	}

#define OS_HIGHEST  (sizeof(os_types) / sizeof(char *) - 1)

/* Header Partition Image revisions
 */
typedef struct	header_rev {
    unsigned int
	hdr_rev_valid;
    unsigned char
	hdr_os_type,
	hdr_major,
	hdr_minor,
	hdr_minor_update;
} header_rev_t;

#define MAXPARTITIONS	64
#define MAX_PARTSIZE	((1 << 31) / DEV_BSIZE)

#define ROOT_PART	0
#define PAGE_PART	1
#define	DUMP_PART	1
#define ALL_PART	2
#define HEADER_PART	3

#define MIN_HP_PI	2048	/* Min hdr part size with program images */
#define MIN_HP_NO_PI	36	/* Min hdr part size without program images */

#define VOL_LABEL_SIZE	32
#define DISK_PARTNAMELEN 255

#define FORMAT_VERSION_MAJ	1
#define SCC_MAGIC_NUMBER 0x05ac369f

/*
 * Disk Format Revision Structure
 */
typedef	struct	format_rev {
	short	format_major;		/* Format revision, major */
	short	format_minor;		/* Format revision, minor */
} format_rev_t;

/*
 * Volume Number Structure
 */
typedef	struct	vol_number {
	short	vol_num;		/* Volume number */
	short	vol_total;		/* Total # of volumes in set*/
} vol_number_t;

/* 
 * Program Image Area Descriptor Structure 
 */
typedef	struct	image_desc {
	u_long	image_size;		/* Image data size in bytes */
	u_long	image_addr;		/* Byte address of start of images */
} image_desc_t;

/*
 * Disk Geometry Structure
 */
typedef	struct	dk_geom {
	long	total_blocks;		/* Total blocks on disk */
	long	byte_sector;		/* Bytes per sector */
	long	sector_track;		/* Sectors per track */
	long	track_cylinder;		/* Tracks per cylinder */
	long	cylinders;		/* Total # of cylinders */
	long	total_avail_blocks;	/* Total OS addressable blocks */
	long	avail_sect_cylinder;	/* Addressable sectors/cylinder */
	long	end_cylinders;		/* End cylinders reserved */
	long	reserved_1;		/* Reserved */
	long	reserved_2;		/* Reserved */
} dk_geom_t;

/*
 * Partition Namespace Definition
 */
typedef	struct	part_namesp {
	struct	{
		char	p_name[DISK_PARTNAMELEN+1];	/* Partition name */
	} part[MAXPARTITIONS];
} part_namesp_t;

/*
 * Definition for the on-disk layout structure.  The layout is followed on the
 * the next sector boundry by a namespace which is a MAXPARTITIONS by 
 * DISK_PARTNAMELEN structure, the pointers in the partitions array point
 * to the corresponding index in the name space.
 */
typedef struct	layout	{
	u_long		SCC_magic_number;	/* SCC layout validation # */
	format_rev_t	format_rev;		/* Format revision */
	char		vol_label[VOL_LABEL_SIZE];/* Volume set unique id */
	vol_number_t 	vol_number;		/* Volume number in set */
	u_long		layout_flag;		/* Flag word */
#define IA_DEFINED	0x0001			/* Image area defined */
	image_desc_t 	image_desc;		/* Program image area desc */
	dk_geom_t	lay_geom;		/* Disk geometry */
	header_rev_t	header_rev;		/* Header revision */
	short		current_part;		/* Current # of partitions */
	short		maximum_part;		/* Maximum # of partitions */
	struct	{
		u_long	part_size;		/* Partition size in blocks */
		u_long	part_off;		/* Partition offset in blks */
		u_long	part_type;		/* Type of partition */
		u_long	part_name_off;		/* Partition name byte offset
						 * in part_namesp */
		long	part_reserved;		/* Reserved */
	} partitions[MAXPARTITIONS];
	part_namesp_t	part_namesp;		/* Partition name space */
	long		reserved[10];		/* Reserved */
	u_long		check_sum;		/* Check sum for layout */
} layout_t;

/*
 * The check sum for the layout is calculated by looking at the layout as
 * an array of unsigned characters and adding each one (up to but not 
 * including check_sum) to an (u_long) integer 'sum' which starts as zero.
 */

/*
 * Definitions for part_type
 */
#define ROOT_TYPE	1
#define PAGE_TYPE	2
#define DUMP_TYPE	2
#define ALL_TYPE	3
#define HEADER_TYPE	4
#define STANDARD_TYPE	5
#define NOFORMAT_TYPE	6
#define DIAGNOSTIC_TYPE 7
#define UMAXV_TYPE	8


/*   ****   END of COMPATIBLE LAYOUT ****   */



/* Constants related to disk layout/format */

#define MAX_V_PARTITIONS	16
#define NBYTESPERSECTOR	512		/* will it ever be 1024 ? */

#define BASE_TAPELUN	0x8		/* up to 4 tapes */
#define MAX_TAPELUN	0xb
#define BASE_DISKLUN	0x10		/* up to 23 disks, oh my */
#define MAX_DISKLUN	0x27


