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
 *	@(#)$RCSfile: msioctl.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:44:31 $
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

/*
 * Generic MASS STORE ioctl's:
 *
 * 	Valid for most disks and tapes
 */

#define MSIOC		('s' << 8)
#define MSIOC_CLASS	0 | MSIOC	/* return CLASS_DISK or CLASS_TAPE */

/*
 * Generic DISK ioctl's:
 *
 *	Valid for most disk drives.
 */

typedef struct  fmt_blk {
	char 	*fmt_bad_blk;
	int	fmt_defect_cnt;
	int	fmt_disk_type;
} fmt_blk_t;

/* currently supported fmt_disk_type's */

#define FSD_CDC9715	1	
#define SMD_CDC9766	2
#define EAGLE_FJT2351	3
#define NEC_D2362	4

/*2
 *  Media Defects: 
 *	Used by FORMAT command to EMC, and MDIOC_FORMAT ioctl.
 *
 *  For now, only one buffer full of defects allowed.
 */

typedef	struct media_defect {
	unsigned short md_cylinder;	/* Cylinder number */
	unsigned short md_head;		/* Head number */
	unsigned short md_position;	/* Position in bytes after Index */
	unsigned short md_length;	/* Length in bits */
} md_defect_t;

typedef struct	bad_cyl {		/* Break cylinder into bit-field */
	unsigned	cyl_no : 15;
	unsigned	bad_flag : 1;
} bad_cyl_t;

#define MAX_DEFECTS 	(DEV_BSIZE/sizeof(md_defect_t))

/*2
 *  Sector_id data and ioctl
 */

typedef struct	sect_id {
	unsigned long	sect_pbn;	/* physical sector number */
	char		*sect_buf;	/* Addr for returned data */
	int		sect_bcnt;	/* max size of returned data */
} sect_id_t;



/*
 * MAG TAPE ioctl's:
 * 
 *	Most tape ioctls use the single argument as a count, eg to skip
 *	several records.
 */

#define MTIOC		('m' << 8)

#define MTIOC_WEOF	0 | MTIOC	/* write 'cnt' end-of-file records */
#define MTIOC_FSF	1 | MTIOC	/* 'cnt' forward space file */
#define MTIOC_BSF	2 | MTIOC	/* 'cnt' backward space file */
#define MTIOC_FSR	3 | MTIOC	/* 'cnt' forward space record */
#define MTIOC_BSR	4 | MTIOC	/* 'cnt' backward space record */
#define MTIOC_REW	5 | MTIOC	/* rewind */
#define MTIOC_OFFL	6 | MTIOC	/* rewind and put the drive offline */
#define MTIOC_NOP	7 | MTIOC	/* no operation, sets status only */
#define MTIOC_FSSF	8 | MTIOC	/* 'cnt' forw space sequential file */
#define MTIOC_BSSF	9 | MTIOC	/* 'cnt' back space sequential file */

					/* VALID for STREAMERS only */
#define MTIOC_EOT	10 | MTIOC	/* space to end of date */
#define MTIOC_RETEN	11 | MTIOC	/* retension tape */
#define MTIOC_ERASE	12 | MTIOC	/* erase tape */


/* could implement ioctl to get current drive type, fileno, blkno */

#define MAXBADBLOCKS	2048

/*
 * disk io control commands
 */

#define _MSIO(x,y)	(IOC_IN|(x<<8)|y)

#define DKIOCHDR	_IO('d', 1)	/* next I/O will read/write header */
#define MSIOCFMT	_MSIO('d',2)    /* format volume */
#define MSIOCGEOM	_MSIO('d',3)    /* get geom. return data */
#define MSIOCRECAL	_IO('d', 4)	/* recalibrate drive */
#define MSIOCCHKTRK	_MSIO('d',5)	/* check track */
#define MSIOCSECID	_MSIO('d',6)    /* read sector id */
#define MSIOCRDLAY	_MSIO('d',7)	/* Read layout, using ptr */
#define MSIOCWRLAY	_MSIO('d',8)	/* Write layout, using ptr */
#define MSIOCBADTRACK   _MSIO('d',9)	/* remap a bad track */
