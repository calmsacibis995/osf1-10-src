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
 *	@(#)$RCSfile: prestoioctl.h,v $ $Revision: 4.2.14.2 $ (DEC) $Date: 1993/12/15 21:08:21 $
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
/*
 * derived from prestoioctl.h	4.2.1.1	(ULTRIX)	4/11/91
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
 *  11 Apr 91 -- chet
 *	Add ready routine to prtab struct.
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

#ifndef _PRESTOIOCTL_H_
#define _PRESTOIOCTL_H_

#include <sys/types.h>
#include <sys/param.h>
#include <sys/buf.h>
#include <sys/ioctl.h>

/*
 * Presto is initially down until a PRSETSTATE ioctl cmd
 * with arg = PRUP is done.  When presto is down, nothing is
 * being cached and everything is written through to the real disk.
 */

enum battery {
	BAT_GOOD = 0,
	BAT_LOW = 1,
	BAT_DISABLED = 2,
	BAT_IGNORE = 3,
	BAT_SELFTEST = 4,
	BAT_CHARGING = 5,
};
typedef enum battery battery;
#define BAT_OK BAT_GOOD

#define MAX_BATTERIES 8

enum prstates {
	PRDOWN = 0,
	PRUP = 1,
	PRERROR = 2
};
typedef enum prstates prstates;

struct io {
	u_int total;
	u_int hitclean;
	u_int hitdirty;
	u_int pass;
	u_int alloc;
};
typedef struct io io;

struct presto_status {
	prstates pr_state;	/* if not PRUP, pass all rw commands thru */
	u_int	pr_battcnt;
	battery	pr_batt[MAX_BATTERIES];	/* array of battery status flags */
	u_int	pr_maxsize;	/* total memory size in bytes available */
	u_int	pr_cursize;	/* current presto memory size */
	u_int	pr_ndirty;	/* current number of dirty presto buffers */
	u_int	pr_nclean;	/* current number of clean presto buffers */
	u_int	pr_ninval;	/* current number of invalid presto buffers */
	u_int	pr_nactive;	/* current number of active presto buffers */
	/* the io stats are zeroed each time presto is reenabled */
	io 	pr_rdstats;	/* presto read statistics */
	io 	pr_wrstats;	/* presto write statistics */
	u_int	pr_seconds;	/* seconds of stats taken */
};
typedef struct presto_status presto_status;

struct presto_modstat {
	int ps_status;
	union {
		char *ps_errmsg;
		struct presto_status ps_new;
	} presto_modstat_u;
};
typedef struct presto_modstat presto_modstat;

/*
 * Get the current presto status information.
 */
/* old interface */
#define PRGETSTATUS	_IOR('p', 1, struct presto_status)
/* new interface */
#define PRNGETSTATUS	_IOR('p', 12, struct presto_status)

/*
 * Set the presto state.  Legal values are PRDOWN and PRUP.
 * When presto is enabled, all the io stats are zeroed.  If
 * presto is in the PRERROR state, it cannot be changed.
 */
#define PRSETSTATE	_IOW('p', 2, int)

/*
 * Set the current presto memory size in bytes.
 */
#define PRSETMEMSZ	_IOW('p', 3, int)

/*
 * Reset the entire presto state.  If there are any pending writes
 * back to the real disk which cannot be completed due to IO errors,
 * these writes will be lost forever.  Using this ioctl is the only
 * way presto will ever lose any dirty data if disk errors develop
 * behind presto.  Presto will be left in the PRDOWN state unless
 * all batteries are currently low.
 */
#define PRRESET		_IO('p', 4)

/*
 * Enable presto on a particular presto'ized filesystem.
 * This ioctl is performed on the presto control device, passing in
 * the *block* device number of the presto'ized filesystem to enable.
 */
#define PRENABLE	_IOW('p', 5, dev_t)
/* for BSD compatability where dev_t is a short */
#define PROENABLE	_IOW('p', 5, short)

/*
 * Disable presto on a particular presto'ized filesystem.
 * This ioctl is performed on the presto control device, passing in
 * the *block* device number of the presto'ized filesystem to disable.
 */
/* for BSD compatability where dev_t is a short */
#define PRDISABLE	_IOW('p', 6, dev_t)
#define PRODISABLE	_IOW('p', 6, short)

/*
 * Define a bit array type that is long enough to have room for all
 * partitions in a (device major, unit number) pair.
 */
struct prbits {
	/* bit for each (major,unit) partition */
	unsigned char	bits[(GETDEVS(0xffffffff)+(NBBY-1))/NBBY];
};

/*
 * Prutab structure - new "prtab" replacement for user visible fields.
 */
struct uprtab {
	major_t upt_bmajordev;		/* 0: block major device number */
	unit_t  upt_unit;		/* 4: unit number */
	struct  prbits upt_enabled; 	/* 8: per minor enabled bits */
	struct  prbits upt_bounceio;	/* 12: per minor dev bounceio bits */
	                                /* 16 bytes long */
};

/*
 * Fill a given uprtab structure for the given major device
 * in prtabs[] or get the "next" uprtab in the system.  A
 * bmajordev of NODEV on return says no such or no more entries.
 */
#define PRNEXTUPRTAB		_IOWR('p', 9, struct uprtab)
#define PRGETUPRTAB		_IOWR('p', 10, struct uprtab)

/*
 * Flush the any dirty buffers in the presto cache back to disk
 * without changing the current PRUP/PRDOWN state of the board.
 */
#define PRFLUSH			_IO('p', 11)

/*
 * Prtab structure - kernel data structure kept per presto-ized major device.
 *
 * N.B. The hash function used below is trying to spread enabled spindles over
 * a number of hash lists. This is *very* dependent upon minor device
 * encodings.  The number for NPRUHASH was initially chosen to be 8 as
 * it seemed like a reasonable hashing number and also because conventially 
 * there can be 8 partitions per disk.  The shift by 10 (currently) basically
 * extracts the disk unit number.  There is a CAM related macro similar to 
 * this but seemed inordinately large for such a frequently used operation.
 */
#define NPRUHASH 8
#define PRUHASH(dev) ((minor(dev)>>10)%NPRUHASH) 
struct prtab {
	major_t pt_bmajordev;		/* 0:  block major device number */
	int (*pt_strategy)();		/* 4:  major dev `strategy' routine */
	int (*pt_ready)();		/* 12: major dev `ready' routine */
	struct prunit *pt_unit;		/* 20: enabled unit chain */
	struct prunit *pt_hunit[NPRUHASH]; /* 28: enabled unit hash chains */
};

/*
 * Prunit structure - kernel data structure kept per presto-ized unit.
 */
struct prunit {
	struct prunit *pu_next;		/* 0:  next enabled unit */
	struct prunit *pu_hnext;	/* 8:  next enabled unit (hash list) */
	unit_t        pu_unit;		/* 16: unit number */
	struct prbits pu_bounceio;	/* 20: per unit bounceio bits */
	struct prbits pu_enabled; 	/* 28: per unit enabled bits */
	struct prbits pu_error; 	/* 32: per unit error bits */
	struct prbits pu_flushing; 	/* 40: per unit flushing bits */
};

/*
 * Miscellaneous defines
 */
#define PR_BOUNCEIO	0x1		/* pr_flags value for all bounceio */
#define IOCTL_NUM(x)	((x) & 0xff)	/* macro to extract ioctl cmd no */
#define PRDEV		"/dev/pr0"	/* generic presto control device */

#endif /* __PRESTOIOCTL_H_ */
