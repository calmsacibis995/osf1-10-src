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
 *	@(#)quota.h	9.3	(ULTRIX/OSF)	10/23/91
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
 * Copyright (c) 1982, 1986 Regents of the University of California.
 * All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Robert Elz at The University of Melbourne.
 *
 * Redistribution and use in source and binary forms are permitted provided
 * that: (1) source distributions retain this entire copyright notice and
 * comment, and (2) distributions including binaries display the following
 * acknowledgement:  ``This product includes software developed by the
 * University of California, Berkeley and its contributors'' in the
 * documentation or other materials provided with the distribution and in
 * all advertising materials mentioning features or use of this software.
 * Neither the name of the University nor the names of its contributors may
 * be used to endorse or promote products derived from this software without
 * specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 *

 */

#ifndef	_UFS_QUOTA_H_
#define	_UFS_QUOTA_H_

#ifdef	_KERNEL
#include <quota.h>
#endif

/*
 * Definitions for disk quotas imposed on the average user
 * (big brother finally hits UNIX).
 *
 * The following constants define the amount of time given a user
 * before the soft limits are treated as hard limits (usually resulting
 * in an allocation failure). The timer is started when the user crosses
 * their soft limit, it is reset when they go below their soft limit.
 */
#define	MAX_IQ_TIME	(7*24*60*60)	/* 1 week */
#define	MAX_DQ_TIME	(7*24*60*60)	/* 1 week */

/*
 * The following constants define the usage of the quota file array
 * in the ufsmount structure and dquot array in the inode structure.
 * The semantics of the elements of these arrays are defined in the
 * routine getinoquota; the remainder of the quota code treats them
 * generically and need not be inspected when changing the size of
 * the array.
 */
#define	MAXQUOTAS	2
#define	USRQUOTA	0	/* element used for user quotas */
#define	GRPQUOTA	1	/* element used for group quotas */

/*
 * Definitions for the default names of the quotas files.
 */
#define INITQFNAMES { \
	"user",		/* USRQUOTA */ \
	"group",	/* GRPQUOTA */ \
	"undefined", \
};
#ifndef _KERNEL
/*
 * These variables are initialized in libc.
 */
extern char *qfname;
extern char *qfextension[];
extern char *quotagroup;
#endif

/*
 * Command definitions for the 'quotactl' system call.
 * The commands are broken into a main command defined below
 * and a subcommand that is used to convey the type of
 * quota that is being manipulated (see above).
 */
#define SUBCMDMASK	0x00ff
#define SUBCMDSHIFT	8
#define	QCMD(cmd, type)	(((cmd) << SUBCMDSHIFT) | ((type) & SUBCMDMASK))

#define	Q_QUOTAON	0x0100	/* enable quotas */
#define	Q_QUOTAOFF	0x0200	/* disable quotas */
#define	Q_GETQUOTA	0x0300	/* get limits and usage */
#define	Q_SETQUOTA	0x0400	/* set limits and usage */
#define	Q_SETUSE	0x0500	/* set usage */
#define	Q_SYNC		0x0600	/* sync disk copy of a filesystems quotas */

/*
 * The following structure defines the format of the disk quota file
 * (as it appears on disk) - the file is an array of these structures
 * indexed by user or group number.  The setquota system call establishes
 * the vnode for each quota file (a pointer is retained in the ufsmount
 * structure).
 */
struct	dqblk {
	u_int	dqb_bhardlimit;	/* absolute limit on disk blks alloc */
	u_int	dqb_bsoftlimit;	/* preferred limit on disk blks */
	u_int	dqb_curblocks;	/* current block count */
	u_int	dqb_ihardlimit;	/* maximum # allocated inodes + 1 */
	u_int	dqb_isoftlimit;	/* preferred inode limit */
	u_int	dqb_curinodes;	/* current # allocated inodes */
	time_t	dqb_btime;	/* time limit for excessive disk use */
	time_t	dqb_itime;	/* time limit for excessive files */
};

#ifdef _KERNEL
/*
 * The following structure records disk usage for a user or group on a
 * filesystem. There is one allocated for each quota that exists on any
 * filesystem for the current user or group. A cache is kept of recently
 * used entries.
 */
struct	dquot {
	struct	dquot *dq_forw, *dq_back;/* MUST be first entry */
	struct	dquot *dq_freef, **dq_freeb; /* free list */
	short	dq_flags;		/* flags, see below */
	short	dq_cnt;			/* count of active references */
	short	dq_spare;		/* unused spare padding */
	short	dq_type;		/* quota type of this dquot */
	u_int	dq_id;			/* identifier this applies to */
	struct	ufsmount *dq_ump;	/* filesystem dquot belongs to */
	struct	dqblk dq_dqb;		/* actual usage & quotas */
	struct	dqhead *dq_hash;	/* hash chain dquot lives on */
	lock_data_t dq_lock;		/* blocking mutual exclusion */
};

/*
 * Lock constraints on dquots.
 *
 *	dq_forw, dq_back	dqhead table hash chain lock (see ufs_quota.c)
 *	dq_freef, dq_freeb	freelist lock
 *	dq_flags		dquot lock
 *	dq_cnt			dqhead table hash chain lock
 *	dq_spare		n/a
 *	dq_type			read-only for dq_cnt > 1, else hash chain lock
 *	dq_id			read-only for dq_cnt > 1, else hash chain lock
 *	dq_ump			read-only for dq_cnt > 1, else hash chain lock
 *	dq_dqb			dquot lock
 *	dq_hash			read-only for dq_cnt > 1, else hash chain lock
 *
 * In other words, the dq_type, dq_id, and dq_ump fields are all
 * essentially read-only after the dquot has been found through dqget().
 *
 * The dquot hash table contains the active dquots.  The hash bucket lock
 * protects the individual dquots' dq_cnt field (as well as the dquot's
 * hash chain pointers, of course).
 *
 * For a description of the dquot lock protocol, refer to ufs/ufs_quota.c.
 */

#define	DQ_LOCK(dq)		lock_write(&(dq)->dq_lock)
#define	DQ_UNLOCK(dq)		lock_done(&(dq)->dq_lock)
#define	DQ_LOCK_TRY(dq)		lock_try_write(&(dq)->dq_lock)
#define	DQ_LOCK_INIT(dq)	lock_init2(&(dq)->dq_lock, TRUE, LTYPE_DQUOT)
#define	DQ_HOLDER(dq)		LOCK_HOLDER(&(dq)->dq_lock)

/*
 * Flag values.
 */
#define	DQ_MOD		0x04		/* this quota modified since read */
#define	DQ_FAKE		0x08		/* no limits here, just usage */
#define	DQ_BLKS		0x10		/* has been warned about blk limit */
#define	DQ_INODS	0x20		/* has been warned about inode limit */
#define	DQ_READERROR	0x40		/* I/O error reading dquot */

/*
 * Shorthand notation.
 */
#define	dq_bhardlimit	dq_dqb.dqb_bhardlimit
#define	dq_bsoftlimit	dq_dqb.dqb_bsoftlimit
#define	dq_curblocks	dq_dqb.dqb_curblocks
#define	dq_ihardlimit	dq_dqb.dqb_ihardlimit
#define	dq_isoftlimit	dq_dqb.dqb_isoftlimit
#define	dq_curinodes	dq_dqb.dqb_curinodes
#define	dq_btime	dq_dqb.dqb_btime
#define	dq_itime	dq_dqb.dqb_itime

/*
 * If the system has never checked for a quota for this file,
 * then it is set to NODQUOT. Once a write attempt is made
 * the inode pointer is set to reference a dquot structure.
 */
#define	NODQUOT		((struct dquot *) 0)

/*
 * Flags to chkdq() and chkiq()
 */
#define	FORCE	0x01	/* force usage changes independent of limits */
#define	CHOWN	0x02	/* (advisory) change initiated by chown */

#endif /* _KERNEL */

#endif	/* _UFS_QUOTA_H_ */
