
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
 * @(#)$RCSfile: dump.h,v $ $Revision: 4.2.17.2 $ (DEC) $Date: 1993/12/16 15:18:16 $
 */


/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 *
 * OSF/1 Release 1.0
 */

/*
 * This module contains IBM CONFIDENTIAL code. -- (IBM Confidential Restricted
 * when combined with the aggregated modules for this product) OBJECT CODE ONLY
 * SOURCE MATERIALS (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or disclosure
 * restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1980 Regents of the University of California. All
 * rights reserved.  The Berkeley software License Agreement specifies the
 * terms and conditions for redistribution.
 */

/*	system include files	*/

#include        <sys/types.h>
#include        <unistd.h>
#include	<stdio.h>
#include	<ctype.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<ufs/fs.h>
#include	<ufs/inode.h>
#include	<protocols/dumprestore.h>
#include	<sys/dir.h>
#include	<utmp.h>
#include	<sys/time.h>
#include	<sys/file.h>
#include	<signal.h>
#include	<fstab.h>
#include	<limits.h>
#include	<varargs.h>
#include	<io/common/devio.h>

/* global definition of important spcl record */

#define	spcl	u_spcl.s_spcl

extern union u_spcl	u_spcl;

/*	local defines		*/

#define	min(a, b)	(((a) < (b))? (a): (b))
#define	max(a, b)	(((a) > (b))? (a): (b))

/* define our own howmany which will work with values like the largest int */

#undef	howmany
#define	howmany(x, y)	(((x) / (y)) + (((x) % (y) != 0)? 1 : 0))

#define	TRUE	1
#define	FALSE	0

#define	YES	1
#define	NO	0

#define	MINUTE	60
#define	HOUR	(60 * MINUTE)
#define	DAY	(24 * HOUR)

/*
 * Exit	status codes
 */

#define	X_FINOK		0		/* normal exit */
#define	X_FINBAD	1		/* bad exit */
#define	X_REWRITE	2		/* rewrite from the checkpoint */
#define	X_ABORT		3		/* abort all of dump; don't attempt
					 * checkpointing */

/*
 * Devices, Files, Operators, Etc.
 */

#define	DEFAULT_TAPE	"/dev/rmt0h"		/* default tape device */
#define	DEFAULT_DISK	"/dev/rrz0g"		/* default disk */
#define	DUMP_HISTORY	"/etc/dumpdates"	/* dump_history info file */
#define	DIALUP_PREFIX	"ttyd"			/* prefix for dialups */
#define	OPERATOR_GROUP	"operator"		/* group entry to notify */

/*
 * Path of remote daemon (rmt)
 *
 * There is an incompatability with the executable path for the rmt
 * command between OSF/1 and other vendors including SUN and ULTRIX.  The
 * rmt command is found in /usr/sbin/rmt for OSF/1 and /etc/rmt or
 * /usr/etc/rmt for other platforms.
 */

#ifdef  DECOSF
#define SERVER_PATH	"sh -c 'PATH=/usr/sbin:/etc:/usr/etc;rmt'"
#else
#define SERVER_PATH	"/usr/sbin/rmt"		/* path of the remote daemon */
#endif  /* DECOSF */

/*	system external variables	*/

extern int		errno;
extern int		optind;
extern char	       *optarg;

/*	local function declarations	*/

extern int		check_tape();
extern int		query();
extern int		set_operators();
extern struct fstab    *fstabsearch();	/* search in fs_file and fs_spec */
extern time_t		unctime();
extern char	       *prdate();
extern char	       *rawname();
extern void		Exit();
extern void		abort_dump();
extern void		bitmap();
extern void		broadcast();
extern void		bread();
extern void		close_at_end();
extern void		dmpblk();
extern void		dump_perror();
extern void		open_at_start();
extern void		getfstab();
extern void		job_trailer();
extern void		lastdump();
extern void		msg();
extern void		msgtail();
extern void		rewrite_tape();
extern void		taprec();
extern void		volume_label();
extern void             open_tape();

/*	remote function declarations	*/

extern void		rmtclose();
extern void		rmthost();
extern int		rmtioctl();
extern int		rmtopen();
extern int		rmtread();
extern int		rmtseek();
extern int		rmtwrite();

/*	routines which deal with dump history file */

extern void		getitime();
extern void		putitime();
extern void		inititimes();

/*	pass controller and four passes over the file system	*/

extern void		pass();
extern void		mark();
extern void		add();
extern void		dirdump();
extern void		dump();

/*	local structures definitions	*/

/*
 * The contents of the file DUMP_HISTORY is maintained both on a linked list,
 * and then (eventually) arrayified.
 */

struct idates
{
	char		id_name[NAME_MAX + 3];
	char		id_incno;
	time_t		id_ddate;
};

struct itime
{
	struct idates	it_value;
	struct itime   *it_next;
};

/*	local external variables	*/

extern int		imap_size;	/* inode map size in bytes	*/
extern char	       *not_clear_map;	/* bit map of inodes which are	*/
					/* (not) clear at time of dump	*/
					/* (0 == clear, 1 == not clear)	*/
extern char	       *directory_map;	/* bit map of inodes which are	*/
					/* directories and eligible for	*/
					/* dumping			*/
					/* (0 == not dir, 1 == is dir)	*/
extern char	       *to_dump_map;	/* bit map of inodes which are	*/
					/* still to be dumped		*/
					/* (0 == do not dump, 1 == do)	*/

extern char	       *disk_file_name;		/* name of the disk file */
extern char	       *tape_file_name;		/* name of the tape file */
extern char	       *dump_hist_file_name;	/* name of the file containing
						 * dump history information */

extern char		incr_num;		/* increment number */
extern char		last_incr_num;		/* increment number of previous
						 * dump */

extern ino_t		curr_inum;	/* current inumber; used globally */

extern int		new_tape_flag;	/* new tape flag */
extern int		curr_tape_num;	/* current tape number */
extern int		curr_volume_num; /* current volume number */

extern int		dir_added_flag;	/* true if added directory to to-be-
					 * dumped list on last pass-two */
extern int		dir_skipped_flag; /* set TRUE in mark() if any
					 * directories are skipped; this
					 * lets us avoid map pass 2 in
					 * some cases */

extern int		by_blocks_flag; /* TRUE when measuring output by blocks,
					 * FALSE when by inches */

extern long		full_size;           /* full dump_file size, blocks or feet */
extern long		full_size_blocks;    /* full size of medium in blocks */
extern double		full_size_inches;    /* full size of medium in inches */
extern long		blocks_written;      /* number of blocks written on
					      * current volume */
extern double		inches_written;      /* number of inches written on
					      * current volume */
extern long		est_tot_blocks;      /* estimated total number of blocks */
extern double		est_tot_inches;      /* estimated total number of inches */
extern int		est_tot_tapes;	     /* estimated number of tapes */
extern int		tape_density;        /* density in bytes/inch */
extern double		tape_record_gap;     /* inter-record gap (inches) */
extern int		blocks_per_write;    /* blocking factor on tape */
extern double		inches_per_write;    /* inches for each tape write */

extern long		dev_bsize;	     /* machine device block size in bytes */

extern int		in_disk_fd;	     /* disk file descriptor */
extern int		out_tape_fd;         /* tape file descriptor */

extern int		pipe_out_flag;	     /* true => output to standard output */
extern int		medium_flag;	     /* type of medium (see flags below) */
extern int		no_rewind_flag;	     /* do not rewind tape */
extern int              estimate_only_flag;  /* print estimate information only */
extern int              invalid_file_system;  /* output file abort dump msg*/
extern int		notify_flag;	     /* notify operators */

extern int		num_idate_records; /* number of idate records (might
					 * be zero) */
extern struct idates  **idate_array;	/* pointer to array of pointers to */
					/* dump_history idates structures */

extern struct fs       *super_block;	/* the file system super block */

/* values for medium_flag */
#define	NO_MEDIUM	0		/* medium is not known */
#define	REGULAR_FILE	1		/* medium is a regular file */
#define TAPE		2		/* medium is a tape */
#define CARTRIDGE	3		/* medium is a cartridge */
#define DISKETTE	4		/* medium is a diskette */

/*	NLS stuff	*/
#include	<locale.h>
#include	"dump_msg.h"

extern nl_catd		catopen();
extern char	       *catgets();

extern nl_catd		catd;
extern char	       *yes_str;
extern char	       *no_str;
extern int		yes_flag;
extern int		no_flag;

#define	MSGSTR(Num, Str)	catgets(catd, MS_DUMP, Num, Str)
