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
 *	@(#)$RCSfile: hdreg.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:08:28 $
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
 * All rights reserved.  The CMU software License Agreement specifies
 * the terms and conditions for use and redistribution.
 */
/* 
 * OSF/1 Release 1.0
 */
 
/*
 *
 *  Copyright 1988, 1989 by Intel Corporation
 *
 *         INTEL CORPORATION PROPRIETARY INFORMATION
 *
 *     This software is supplied under the terms of a license 
 *    agreement or nondisclosure agreement with Intel Corpo-
 *    ration and may not be copied or disclosed except in
 *    accordance with the terms of that agreement.
 *    Copyright 1988  Intel Corporation.
 */


#define BOOTRECORDSIGNATURE			(0x55aa & 0x00ff)
#define FIXED_DISK_REG				0x3f6
#define MORETHAN8HEADS				0x008
#define EIGHTHEADSORLESS			0x000
#define FIXEDBITS				0x0a0

#define PORT_DATA				0x1f0
#define PORT_ERROR				0x1f1
#define PORT_PRECOMP				0x1f1
#define PORT_NSECTOR				0x1f2
#define PORT_SECTOR				0x1f3
#define PORT_CYLINDERLOWBYTE			0x1f4
#define PORT_CYLINDERHIBYTE			0x1f5
#define PORT_DRIVE_HEADREGISTER			0x1f6
#define PORT_STATUS				0x1f7	
#define PORT_COMMAND				0x1f7

#define STAT_BUSY				0x080
#define STAT_READY 				0x040
#define STAT_WRITEFAULT				0x020
#define STAT_SEEKDONE				0x010
#define STAT_DATAREQUEST			0x008
#define STAT_ECC				0x004
#define STAT_INDEX 				0x002
#define STAT_ERROR				0x001 

#define CMD_RESTORE 				0x010
#define CMD_SEEK 				0x070
#define CMD_READ				0x020
#define CMD_WRITE				0x030
#define CMD_FORMAT				0x050
#define CMD_READVERIFY				0x040
#define CMD_DIAGNOSE				0x090
#define CMD_SETPARAMETERS			0x091

#define	ERROR_BBD				0x80
#define ERROR_ECC				0x40
#define ERROR_ID				0x10
#define ERROR_ABRT				0x04
#define ERROR_TRK0				0x02
#define ERROR_MARK				0x01

#define	MAX_RETRIES				5
#define	MAX_ALTBUFS				4

#define PATIENCE	3000000		/* how long to wait for controller */
#define PARTITION(z)	(minor(z) & 0x0f)
#define UNIT(z)		(  (minor(z) >> 4)   & 0x01)
#define GOINGUP	1
#define GOINGDOWN 0

#define PDLOCATION	29	
#define GETALTTBL	( ('H' <<8) | 1)
#define FMTBAD		( ('H' <<8) | 2)
#define BAD_BLK		0x80			/* needed for V_VERIFY */

#define NDRIVES 2
#define SECSIZE 512
#define uchar	unsigned char
#define uint	unsigned int

/*  hh holds the state of the one and only (stupid board) current 
    block I/O request
*/
    
struct hh {
	uchar	curdrive;		/* drive the controller is using */
	uchar	controller_busy;	/* controller can't take cmd now */
	uchar	retry_count;		/* # of times cmd has been tried */
	uchar	status;			/* stat read from controller port */

	uchar	restore_request;	/* restore command */
	uchar	reset_request;		/* do a controller reset */
	uchar	setparam_request;	/* have executed SETPARM */
	uchar	format_request;		/* let's do some formatting */

	uint	format_track;		/* used by V_FORMAT in hdioctl() */
#ifdef EUGE
	uchar	open_close_ioctl_mutex;	/* for open/closing/fmt partitions */
#endif EUGE

	uint 	single_mode;		/* 1 = transfer one block each time */
	uint 	block_is_bad;		/* 1 = current block is remapped */
	daddr_t	physblock;		/* block # relative to partition 0 */
	uint	substituteblock;	/* replacement for bad hh.physblock */
	uint	substitutetrack;	/* used during format, see hdioctl() */
	caddr_t	rw_addr;		/* ram addr to read/write sector */
	caddr_t	k_window;		/* kernel virtual space window */
	uint	cylinder;		/* cylinder # rel. to part. 0 */
	uint	head;			/* as it looks */
	uint	sector;			/* as it looks */

	uint	blockcount;		/* blocks done so far */
	uint	blocktotal;		/* total blocks this request */

	uchar	interleave_tab[SECSIZE];/* used to format sector interleave */
	uint	start_of_unix[NDRIVES];		/* unix vs dos partitions */
};

/* the boot record partition table is documented in IBM AT Tech. Ref p. 9-6 */
struct 	boot_record	{
	uchar	boot_ind;	/* if it == 0x80, this partition is active */
	uchar	head;		/* driver does not look at this field */
	uchar	sector;		/* driver does not look at this field */
	uchar	cylinder;	/* driver does not look at this field */

	uchar	sys_ind;	/* driver does not look at this field */
	uchar	end_head;	/* driver does not look at this field */
	uchar	end_sector;	/* driver does not look at this field */
	uchar	end_cylinder;	/* driver does not look at this field */

	uint	rel_sect;	/* where unix starts if boot_ind == 0x80 */
	uint	num_sects;	/* driver does not look at this field */
			};
