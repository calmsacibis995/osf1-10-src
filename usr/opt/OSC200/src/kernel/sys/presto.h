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
 *	@(#)$RCSfile: presto.h,v $ $Revision: 4.2.9.3 $ (DEC) $Date: 1993/12/15 21:08:18 $
 */ 
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0
 */
/*
 * derived from  presto.h	4.4	(ULTRIX)	1/10/91
 */


/*
 *	Portions of this software have been licensed to 
 *	Digital Equipment Company, Maynard, MA.
 *	Copyright (c) 1990 Legato Systems, Inc.  ALL RIGHTS RESERVED.
 */

/*
 *
 *   Modification history:
 *
 *  19 Aug 90 -- chet
 *      V4.1 version.
 *
 *  25 May 90 -- chet
 *	Added this file; it was derived from Legato sources.
 *
 */

/*
 * Definitions for the ``presto'' device driver.
 */

/*
 * Presto is initially down until an PRSETSTATE ioctl cmd
 * with arg = PRUP is done.  When presto is down, nothing is
 * being cached and everything is sync'd back to the real disk.
 */

#ifndef _PRESTO_H_
#define _PRESTO_H_
#include <sys/prestoioctl.h>

/*
 * Presto buffer management code.
 */
#define PRBQUEUES	3		/* number of free buffers queues */

#define PR_DIRTY	0		/* dirty buffers */
#define PR_CLEAN	1		/* clean buffers */
#define PR_INVAL	2		/* buffers without valid data */

/*
 * PRBSIZE and PRFSIZE are the basic sizes for presto buffering.
 * PRBSIZE is the largest block size we buffer.  PRFSIZE is the
 * "fragment" size of the smallest request we will buffer.
 *
 * We choose 8k/1K for these values; this corresponds
 * to the default ufs blocks sizes.
*/
#define PRBSIZE		MAXBSIZE		/* largest presto bsize */
#define PRFSIZE		1024			/* smallest presto bsize */
#define	NCHKSUMS	(PRBSIZE / PRFSIZE)	/* # of chksums per buffer */

#define PRMINSIZE	roundup((sizeof (struct nv) + PRBSIZE), PRBSIZE)

#define PRBALIGN(x)	(((u_long)(x) + ((u_long)PRBSIZE - 1)) & ~((u_long)(PRBSIZE - 1)))
#define PRFALIGN(x)	(((u_long)(x) + ((u_long)PRFSIZE - 1)) & ~((u_long)(PRFSIZE - 1)))

#define PRBUFHSZ	256
#define PRRND		(PRBSIZE / DEV_BSIZE)

extern struct bufhd prbufhash[PRBUFHSZ];
extern struct buf *prbufs;
extern int prnbufs;
extern struct presto_status prstatus;

#define PRBUFHASH(dev, dblkno) \
	((struct buf *)&prbufhash \
		[((u_int)(dev)+(((int)(dblkno))/PRRND)) & (PRBUFHSZ-1)])

/*
 * The nvbuf structure defines the parts of the buffer structure
 * that must be kept in non-volatile memory so that buffers can
 * survive reboots.  These fields are merely copies of the
 * corresponding fields kept in the presto buffers in main memory.
 */
struct nvbuf {
	int	nb_flags;
	daddr_t	nb_blkno;
	dev_t	nb_dev;
	u_short	nb_bcount;
};

struct nvh {
	u_int nvh_dirty;		/* value of PRESTO_DIRTY means
					 * cache contains dirty data */
	u_int nvh_magic;		/* magic number */
	u_int nvh_size;			/* total bytes being used *now* */
	u_int nvh_version;		/* version number for validation */
	u_int nvh_bsize;		/* holds PRBSIZE for validation */
	u_int nvh_fsize;		/* holds PRFSIZE for validation */
	u_int nvh_machineid;		/* holds machine id for validation */
	u_int nvh_nbufs;		/* number of presto buffers */
};

/*
 * The PRVERSION number should be increased everytime
 * the nvh structure and/or NVRAM contents/layout changes.
 */
#define PRVERSION	0xe		/* version number for above */
#define PRMAGIC		0x031758	/* presto non-volatile magic number */

/* Clean/Dirty word (first word of presto NVRAM cache) value */
#define PRESTO_DIRTY	0xbd100248	/* cache contains dirty data */
					/* anything else means it does not */ 

/* Status of NVRAM diagnostics */
#define NVRAM_BAD	0	/* either read/write or read-only diagnostics
				 * run unsuccessfully */
#define NVRAM_RDWR	1	/* read/write diagnostics run successfully */
#define NVRAM_RDONLY	2	/* read-only diagnostics run successfully */

/* Array of pointers to NVRAM cache interface routines */
struct presto_interface {
	int (*nvram_status)();		/* MANDATORY */
	      /* returns diagnostic status of NVRAM */
	int (*nvram_battery_enable)();	/* OPTIONAL */
	      /* enables the use of the battery subsystem upon powerfail */
	int (*nvram_battery_disable)();	/* OPTIONAL */
	      /* disables the use of the battery subsystem upon powerfail */
	int (*nvram_battery_status)();	/* MANDATORY */
	      /* returns battery subsystem status */
	      /*
	       * A dichotomy between small transfers (e.g. 32 bytes or less)
	       * and large transfers (1KB or more) exists within the Presto
	       * driver. The following routines and restrictions are used
	       * to "program" Presto NVRAM transfers.
	       *
	       * The following routines take an argument list identical
	       * to bcopy(), i.e. (fromaddr, toaddr, len).
	       */
	void (*nvram_ioreg_read)();	/* MANDATORY */
	      /* routine to read "small" pieces of NVRAM */
	void (*nvram_ioreg_write)();	/* MANDATORY */
	      /* routine to write "small" pieces of NVRAM */
	void (*nvram_block_read)();	/* MANDATORY */
	      /* routine to read "large" pieces of NVRAM */
	void (*nvram_block_write)();	/* MANDATORY */
	      /* routine to write "large" pieces of NVRAM */
	      /*
	       * The following routines takes an argument list identical
	       * to bzero(), i.e. (addr, len).
	       */
	void (*nvram_ioreg_zero)();	/* MANDATORY */
	      /* routine to clear "small" pieces of NVRAM */
	void (*nvram_block_zero)();	/* MANDATORY */
	      /* routine to clear "large" pieces of NVRAM */
	      /*
	       * Alignment and granularity information.
	       */
	int nvram_min_ioreg;		/* MANDATORY */
	      /* minimum size of a "small" piece (in bytes) */
	u_int nvram_ioreg_align;		/* MANDATORY */
	      /* byte alignment restriction for a "small" piece */
	      /* (e.g. 4 byte boundary = 4) */
	int nvram_min_block;		/* MANDATORY */
	      /* minimum size of a "large" piece (in bytes) */
	u_int nvram_block_align;		/* MANDATORY */
	      /* byte alignment restriction for a "large" piece */
	      /* (e.g. 4 byte boundary = 4) */
        };

#ifdef KERNEL
/* declare interface routine array for /dev/pr0 */
extern struct presto_interface presto_interface0;
#endif /* KERNEL */

/*
/* Battery information for NVRAM cache
 * In this structure:
 *
 * nv_nbatteries is the number of batteries supported
 * by the platform. This number of status fields will be inspected by
 * presto software in priority sequence (nv_batt_status[0] = primary,
 * nv_batt_status[1] = first secondary, ...). 
 *
 * Presto software will either not start normal operation, or will
 * shut itself down, if a "bad battery condition" is detected. This is
 * defined to be the case that there are less than nv_minimum_ok
 * batteries with sufficient power (BATT_OK).
 */
#define BATTCNT		4		/* a maximum of four batteries */
struct nvram_battery_info {
  	int	nv_nbatteries;		/* number of batteries supported */
	int	nv_test_retries;	/* number of successive calls
					 * to nvram_battery_status() for
					 * each presto battery check */
	int	nv_minimum_ok;	    	/* minimal # of BATT_OK batteries
					 * for normal operation */
	int	nv_primary_mandatory;	/* primary must be good */
  	int	nv_status[BATTCNT]; 	/* see status bits below */
        };

#define BATT_NONE	0	/* either no battery or completely bad */
#define BATT_ENABLED 	0x1	/* battery enabled,
				 * i.e. will back up NVRAM on power fail */
#define BATT_HIGH	0x2	/* battery has minimal energy stored,
				 * i.e. has enough power for Prestoserve use */
#define BATT_OK		0x3	/* battery is enabled AND has enough power
				 * i.e. is usable for Prestoserve */
#define BATT_SELFTEST   0x4     /* there is a battery, but charge state
				 * is unknown */
#define BATT_CHARGING   0x8     /* battery does not have enough power for
				 * Prestoserve use, but is on the way up
				 * (as opposed to low, which is on the way
				 *  down for non-rechargeables) */

#ifdef KERNEL
/* declare battery info structure for /dev/pr0 */
extern struct nvram_battery_info nvram_batteries0;
#endif /* KERNEL */

#endif /* _PRESTO_H_ */
