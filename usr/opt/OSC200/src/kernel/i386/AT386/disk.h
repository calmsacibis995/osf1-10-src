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
 *	@(#)$RCSfile: disk.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:08:15 $
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

/*
 * disk.h
 */

#ifdef	OLD_PARTITIONS
#define PART_DISK	0		/* partition number for entire disk */
#else
#define PART_DISK	2		/* partition number for entire disk */
#endif

#define MAX_ALTENTS     253	/* Maximum # of slots for alts */
				/* allowed for in the table. */

#define ALT_SANITY      0xdeadbeef      /* magic # to validate alt table */

struct  alt_table {
	unsigned short  alt_used;	/* # of alternates already assigned */
	unsigned short  alt_reserved;	/* # of alternates reserved on disk */
	long alt_base;	/* 1st sector (abs) of the alt area */
	long alt_bad[MAX_ALTENTS];	/* list of bad sectors/tracks	*/
};

struct alt_info {	/* table length should be multiple of 512 */
	long    alt_sanity;	/* to validate correctness */
	unsigned short  alt_version;	/* to corroborate vintage */
	unsigned short  alt_pad;	/* padding for alignment */
	struct alt_table alt_trk;	/* bad track table */
	struct alt_table alt_sec;	/* bad sector table */
};
typedef struct alt_info altinfo_t;

#define V_NUMPAR        16              /* maximum number of partitions */

#define VTOC_SANE       0x600DDEEE      /* Indicates a sane VTOC */

#define	HDPDLOC		29		/* location of pdinfo/vtoc */

/* Partition identification tags */
#define V_BOOT		0x01		/* Boot partition */
#define V_ROOT		0x02		/* Root filesystem */
#define V_USR		0x04		/* Usr filesystem */
#define V_BACKUP	0x05		/* full disk */
#define V_ALTS          0x06            /* alternate sector space */
#define V_OTHER         0x07            /* non-unix space */
#define V_ALTTRK	0x08		/* alternate track space */

/* Partition permission flags */
#define V_UNMNT		0x01		/* Unmountable partition */
#define V_RONLY         0x10            /* Read only (except by IOCTL) */
#define V_OPEN          0x100           /* Partition open (for driver use) */
#define V_VALID         0x200           /* Partition is valid to use */

/* driver ioctl() commands */
#define V_CONFIG	_IOW('v',1,union io_arg)
#define V_REMOUNT       _IO('v',2)    		/* Remount Drive */
#define V_ADDBAD        _IOW('v',3,union io_arg) 	/* Add Bad Sector */
#define V_GETPARMS      _IOR('v',4,struct disk_parms)   /* Get drive/partition parameters */
#define V_FORMAT        _IOW('v',5,union io_arg) 	/* Format track(s) */
#define V_PDLOC		_IOR('v',6,int)		/* Ask driver where pdinfo is on disk */

#define V_RDABS		_IOW('v',10,struct absio)	/* Read a sector at an absolute addr */
#define V_WRABS		_IOW('v',11,struct absio)	/* Write a sector to absolute addr */
#define V_VERIFY	_IOWR('v',12,union vfy_io) /* Read verify sector(s) */
#define V_XFORMAT	_IO('v',13)		/* Selectively mark sectors as bad */


/* Sanity word for the physical description area */
#define VALID_PD		0xCA5E600D

struct partition	{
	unsigned short p_tag;           /*ID tag of partition*/
	unsigned short p_flag;          /*permision flags*/
	long p_start;        /*physical start sector no of partition*/
	long p_size;            /*# of physical sectors in partition*/
};
typedef struct partition partition_t;

struct evtoc {
	unsigned long fill0[6];
	unsigned long cyls;		/*number of cylinders per drive*/
	unsigned long tracks;		/*number tracks per cylinder*/
	unsigned long sectors;		/*number sectors per track*/
	unsigned long fill1[13];
	unsigned long version;		/*layout version*/
	unsigned long alt_ptr;          /*byte offset of alternates table*/
	unsigned short alt_len;         /*byte length of alternates table*/
	unsigned long sanity;		/*to verify vtoc sanity*/
	unsigned long xcyls;		/*number of cylinders per drive*/
	unsigned long xtracks;		/*number tracks per cylinder*/
	unsigned long xsectors;		/*number sectors per track*/
	unsigned short nparts;		/*number of partitions*/
	unsigned short fill2;		/*pad for 286 compiler*/
	char label[40];
	struct partition part[V_NUMPAR];/*partition headers*/
	char fill[512-352];
};

union   io_arg {
	struct  {
		unsigned short  ncyl;           /* number of cylinders on drive */
		unsigned char nhead;    /* number of heads/cyl */
		unsigned char nsec;     /* number of sectors/track */
		unsigned short  secsiz;         /* number of bytes/sector */
		} ia_cd;                /* used for Configure Drive cmd */
	struct  {
		unsigned short  flags;          /* flags (see below) */
		long bad_sector;     /* absolute sector number */
		long new_sector;     /* RETURNED alternate sect assigned */
		} ia_abs;               /* used for Add Bad Sector cmd */
	struct  {
		unsigned short  start_trk;      /* first track # */
		unsigned short  num_trks;       /* number of tracks to format */
		unsigned short  intlv;          /* interleave factor */
		} ia_fmt;               /* used for Format Tracks cmd */
	struct	{
		unsigned short	start_trk;	/* first track	*/
		char    *intlv_tbl;	/* interleave table */
		} ia_xfmt;		/* used for the V_XFORMAT ioctl */
};

/*
 * Data structure for the V_VERIFY ioctl
 */
union	vfy_io	{
	struct	{
		long abs_sec;	/* absolute sector number        */
		unsigned short  num_sec;	/* number of sectors to verify   */
		unsigned short  time_flg;	/* flag to indicate time the ops */
		}vfy_in;
	struct	{
		long  deltatime;	/* duration of operation */
		unsigned short  err_code;	/* reason for failure    */
		}vfy_out;
};


/* data structure returned by the Get Parameters ioctl: */
struct  disk_parms {
	char    dp_type;			/* Disk type (see below) */
	unsigned char  dp_heads;		/* Number of heads */
	unsigned short  dp_cyls;		/* Number of cylinders */

	unsigned char  dp_sectors;		/* Number of sectors/track */
	unsigned short  dp_secsiz;		/* Number of bytes/sector */
						/* for this partition: */
	unsigned short  dp_ptag;		/* Partition tag */
	unsigned short  dp_pflag;		/* Partition flag */

	long dp_pstartsec;			/* Starting absolute sector number */

	long dp_pnumsec;			/* Number of sectors */

	unsigned char  dp_dosheads;		/* Number of heads */
	unsigned short  dp_doscyls;		/* Number of cylinders */

	unsigned char  dp_dossectors;		/* Number of sectors/track */
};

/* Disk types for disk_parms.dp_type: */
#define DPT_WINI        1               /* Winchester disk */
#define DPT_FLOPPY      2               /* Floppy */
#define DPT_OTHER       3               /* Other type of disk */
#define DPT_NOTDISK     0               /* Not a disk device */

/* Data structure for V_RDABS/V_WRABS ioctl's */
struct absio {
	long	abs_sec;		/* Absolute sector number (from 0) */
	char	*abs_buf;		/* Sector buffer */
};


#define BOOTSZ		446	/* size of boot code in master boot block */
#define FD_NUMPART	4	/* number of 'partitions' in fdisk table */
#define ACTIVE		128	/* indicator of active partition */
#define	BOOT_MAGIC	0xAA55	/* signature of the boot record */
#define	UNIXOS		99	/* UNIX partition */

/*
 * structure to hold the fdisk partition table
 */
struct ipart {
	unsigned char bootid;	/* bootable or not */
	unsigned char beghead;	/* beginning head, sector, cylinder */
	unsigned char begsect;	/* begcyl is a 10-bit number. High 2 bits */
	unsigned char begcyl;	/*     are in begsect. */
	unsigned char systid;	/* OS type */
	unsigned char endhead;	/* ending head, sector, cylinder */
	unsigned char endsect;	/* endcyl is a 10-bit number.  High 2 bits */
	unsigned char endcyl;	/*     are in endsect. */
	long    relsect;	/* first sector relative to start of disk */
	long    numsect;	/* number of sectors in partition */
};

/*
 * structure to hold master boot block in physical sector 0 of the disk.
 * Note that partitions stuff can't be directly included in the structure
 * because of lameo '386 compiler alignment design.
 */

struct  mboot {     /* master boot block */
	char    bootinst[BOOTSZ];
	char    parts[FD_NUMPART * sizeof(struct ipart)];
	unsigned short   signature;
};

