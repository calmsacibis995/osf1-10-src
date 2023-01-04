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
 *	@(#)$RCSfile: ms_dev.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:44:22 $
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

/*1
 *  MSD/MST DISK and TAPE driver specific stuff:
 *
 *	This file details structures, macros, and defines which
 *	are relevant to the msd and mst drivers.  This file supports
 *	the drivers represented in ms.c and ms_subr.c, and is used
 *	by the conf.c/space.h configuration program.
 *
 */

#include <kern/lock.h>


/*2
 *	Temporary storage area to preserve a bp (buf_t) state while
 *	doing funny ioctl's or such.
 */

typedef struct bp_env {
	int	b_bcount;
	char	*b_addr;
	bd_t	*b_bdl_p;
	bd_t	*b_bdlphys_p;
	bd_t	b_bdl;
	crq_ms_xfer_msg_t  xfermsg;
	long	b_flags;
} bp_env_t;


/*2
 *  Layout Structure:  (in memory copy)
 *
 *  The disk driver maintains a simplified in memory copy of the layout/
 *  partition structure at the beginning of the disk.  see also msdefs.h
 */
#define NUMOUTQ	10

typedef struct	ms_layout     {
	char		vol_label[VOL_LABEL_SIZE]; /* Label for this volume */
	dk_geom_t   	lay_geom;		/* copy of geometry struct */
	short		current_part;		/* Current # of partitions */
	short		maximum_part;		/* Maximum # of partitions */
	struct	{
		long	part_size;		/* Size of partition */
		long	part_off;		/* Offset to partition */
		long	part_type;		/* sysV ignored */
	} partitions[MAX_V_PARTITIONS];
	unsigned long  md_total_blocks;		/* Num blocks, md_index, and  */
	unsigned int   md_index;	    	/* ... md_outq are  for	      */
	struct { 			    	/* ... sadp profiling program */
		unsigned long md_blknum;
		unsigned long md_blksiz;
		} md_outq[NUMOUTQ];
} ms_layout_t;

/*2
 *	Per disk/tape structure including crq's.
 *
 *	Disks and tapes share the same base structure.  The major 
 *	difference in their utilization is that disks utilize the
 *	pointer to an associated layout_t structure which encodes
 *	the partition layout.
 */
 
typedef struct	ms_struct {
	crq_t		ms_crq;		/* crq itself */
	crq_maxsize_msg_t	ms_attn_msg[2];	/* two attn msgs per crq */
	unsigned char	ms_state;	/* initialized?, formatting?, ok */
	unsigned char	ms_class;	/* DISK_CLASS, or TAPE_CLASS */
	unsigned char	ms_subclass;	/* Unused */
	unsigned char	ms_flags;	/* SAWEOT or SAWFMK */
	time_t		ms_starttime;	/* start of last operation */
	dev_t		ms_dev;		/* creator (logical disk, partition) */
	unsigned long	ms_multidev;	/* multimax lamp, slot, dev, subunit */
	lock_data_t	ms_openlock;	/* serialize parallel open requests  */

					/* STATISTICS RELATED */
	unsigned long	ms_iostart;	/*   disk busy timer */
	struct iotime	ms_devinfo;	/*   used in errlog, and sadc utility */
					/* MORE STATISTICS */
	unsigned long	ms_tot_bytes[2];/*   Thruput in bytes */
	unsigned short	ms_ecc_hard;	/*   # hard ecc errors */
	unsigned short	ms_ecc_soft;	/*   # soft ecc errors */
	unsigned short	ms_errors;	/*   # other errors */
	unsigned short	ms_retries;	/*   # retry attempts */

                                        /* DK stats */
        int             ms_dkn;         /*   Saved DK number for iostat */
        
                                        /* DISK or TAPE specific */
	union {
	  struct {
	    unsigned long  md_block_parts;  /* map of active partitions < 32 */
	    unsigned long  md_char_parts;   /* ... one for block or char devs */
	    ms_layout_t	   *md_layout;	    /* Do we need it all for TAPES ? */
	  } msdisk;
	  struct {
	    unsigned long	mt_currec;	/* current tape record */
	    unsigned long	mt_eofrec;	/* maximum record (eof) */
	    unsigned short	mt_eotcnt;	/* operations since eot */
	    short		mt_owner_pid;	/* tape owner field */
	  } mstape;
	}ms_un;
        
} ms_struct_t;

#define ms_block_parts	ms_un.msdisk.md_block_parts
#define ms_char_parts	ms_un.msdisk.md_char_parts
#define ms_layout	ms_un.msdisk.md_layout
#define ms_total_blocks	ms_un.msdisk.md_layout->md_total_blocks
#define ms_index	ms_un.msdisk.md_layout->md_index
#define ms_outq(i)	ms_un.msdisk.md_layout->md_outq[(i)]
#define ms_blknum(i)	ms_outq(i).md_blknum
#define ms_blksiz(i)	ms_outq(i).md_blksiz

#define ms_currec	ms_un.mstape.mt_currec
#define ms_eofrec	ms_un.mstape.mt_eofrec
#define ms_eotcnt	ms_un.mstape.mt_eotcnt
#define ms_owner_pid	ms_un.mstape.mt_owner_pid


/*3
 *  ms_state values
 */

#define MST_INVALID	0
#define MST_VALID	1
#define MST_QACTIVE	2
#define MST_ACTIVE	3
#define MST_CLOSED	4

/*3
 *  ms_class values
 */

#define CLASS_UNKNOWN	0
#define CLASS_DISK	1
#define CLASS_TAPE	2

/*3
 *  ms_flags values
 */

#define MSF_SAWFMK	1	/* have seen tape file mark */
#define MSF_SAWEOT	2	/* have seen end of tape mark */
#define MSF_STREAMER	4	/* attempting to stream */

/*2 
 * OTHER MACROS for ms*.c
 */

/*3
 *  form values for open/close.  Caused by need of keeping separate
 *  partition bitmaps for block or char opens since we are called for
 *  the final close of either.
 */

#define BLOCK_DEV	1
#define CHAR_DEV	2

/*3
 *  ESUCCESS
 */

#ifndef	ESUCCESS
#define ESUCCESS	0
#endif	ESUCCESS


/*3
 *  Macro's for disecting minor device and partitions.  
 *
 *  For now we have an 8 bit minor which is broken into the low 
 *  MS_PART_BITS (currently 4) of partition info and the high 
 *  (8-MS_PART_BITS) (currently 4) for subdevice.  MS_PART and
 *  MS_SUBUNIT tear an 8bit minor apart, and MS_MAKEUNIT puts them
 *  back together.  
 *
 *  TAPES:  Tapes use the PARTITION bits for magic behaviour such
 *	as rewind_on_close (MT_NOREWIND false) or high_density (MT_HIGHDENS
 *	true).  The 0x1 and 0x2 bit are available for further behaviour
 *	modification experiments.   Thus minor numbers 0,4,8,and 12 all apply
 *	to the first physical tape drive.
 */

#define MS_PART_BITS	4
#define MS_PART_MASK	((1<<MS_PART_BITS)-1)

#define MS_PART(dev)	((dev) & MS_PART_MASK)
#define MS_SUBUNIT(dev)	(((dev)&0xff)>>MS_PART_BITS)

#define MS_MAKEUNIT(logdev,part)		\
		((((logdev) << MS_PART_BITS) &  \
		  (0xff &(~MS_PART_MASK)))  |   \
		 ((part) & MS_PART_MASK))


#define MT_NOREWIND(dev) ((dev) & 0x4)	/* tape rewind on close */
#define MT_HIGHDENS(dev) ((dev) & 0x8)	/* 6250 BPI, else 1600 bpi */

/*3
 *  LOGERR;
 *
 *	Hook for future error logging.  See errlog.c, elog.h.
 */

#define LOGERR	printf

/*3
 *  From routines in ms.c and ms_subr.c
 */

struct	buf	*ms_getblk();
void	msintr();
