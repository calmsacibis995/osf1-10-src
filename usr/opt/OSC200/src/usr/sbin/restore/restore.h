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
 *	@(#)$RCSfile: restore.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 06:25:01 $
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
 * restore.h
 *
 *	Revision History:
 *
 * 12-Mar-91	Fred Canter
 *	MIPS C 2.20+
 *
 */
/* 

 */
/*
 * This module contains IBM CONFIDENTIAL code. -- (IBM Confidential Restricted
 * when combined with the aggregated modules for this product) OBJECT CODE ONLY
 * SOURCE MATERIALS (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or disclosure
 * restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * Copyright (c) 1983 Regents of the University of California. All
 * rights reserved.  The Berkeley software License Agreement specifies the
 * terms and conditions for redistribution.
 */

#ifndef	_RESTORE_H_
#define	_RESTORE_H_

#include	<stdio.h>
#include	<fcntl.h>
#include	<sys/param.h>
#include	<sys/time.h>
#include	<sys/vnode.h>
#include	<ufs/inode.h>
#include	<ufs/fs.h>
#include	<sys/dir.h>
#include	<protocols/dumprestore.h>
#include	<varargs.h>
#include	<ctype.h>

/* types */

#define	LEAF		1	/* non-directory entry */
#define	NODE		2	/* directory entry */
#define	LINK		4	/* synthesized type, stripped by addentry */

/* flags */

#define	EXTRACT		0x0001	/* entry is to be replaced from the tape */
#define	NEW		0x0002	/* a new entry to be extracted */
#define	KEEP		0x0004	/* entry is not to change */
#define	REMOVED		0x0010	/* entry has been removed */
#define	TMPNAME		0x0020	/* entry has been given a temporary name */
#define	EXISTED		0x0040	/* directory already existed during extract */

/*
 * Constants associated with entry structs
 */

#define	HARDLINK	1
#define	SYMLINK		2
#define	TMPHDR		"RSTTMP"

/* actions */

#define	EXTRACTING	1	/* extracting from the tape */
#define	SKIP		2	/* skipping */
#define	UNKNOWN		3	/* disposition or starting point is unknown */

/*
 * Useful macros
 */

/*	Inodes are numbered 1..n while map bits are numbered 0..n-1.	*/
/*	Low bit in map byte is bit-0 while high bit is bit-NBBY.	*/

#define	MAPWORD(map, num)	(map[(ulong_t) (num - 1) / NBBY])
#define	MAPBIT(num)		(1 << ((ulong_t) (num - 1) % NBBY))
#define	MAPBITSET(map, num)	(MAPWORD(map, num) |= MAPBIT(num))
#define	MAPBITCLEAR(map, num)	(MAPWORD(map, num) &= ~MAPBIT(num))
#define	MAPBITTEST(map, num)	(MAPWORD(map, num) & MAPBIT(num))

#define	dmsg		if (debug_flag == TRUE) msg
#define	vmsg		if (verbose_flag == TRUE) msg

#define	FAIL		0
#define	GOOD		1

#define	NO		0
#define	YES		1

#define	NOMATCH		0
#define	MATCH		1

/* values for overwrite_flag */

#define	OVERWRITE_DEFAULT	0
#define OVERWRITE_ALWAYS	1
#define OVERWRITE_NEVER		2

/*
 * Each file in the file system is described by one of these entries
 */

struct entry
{
	char	       *e_name;		/* the current name of this entry */
	u_char		e_namlen;	/* length of this name */
	char		e_type;		/* type of this entry, see below */
	short		e_flags;	/* status flags, see below */
	ino_t		e_ino;		/* inode number in previous file sys */
	long		e_index;	/* unique index (for dumped table) */
	struct entry   *e_parent;	/* pointer to parent directory (..) */
	struct entry   *e_sibling;	/* next element in this directory (.) */
	struct entry   *e_links;	/* hard links to this inode */
	struct entry   *e_entries;	/* for directories, their entries */
	struct entry   *e_next;		/* hash chain list */
};

/*
 * we can no longer use the library directory code since it has changed
 * in functionality. we now must implement what the old code used to do.
 * We also have to declared our own "DIR" type to sneak past changes.
 */

struct _rst_dirdesc
{
	int	dd_fd;
	long	dd_loc;
	long	dd_size;
	char	dd_buf[DIRBLKSIZ];	/* DIRBLKSIZ will come from dir.h */
};

typedef struct _rst_dirdesc	RST_DIR;

/*
 * functions defined on entry structs
 */

extern struct entry    *lookupino();
extern struct entry    *lookupname();
extern struct entry    *lookupparent();
extern struct entry    *addentry();

/*
 * Other exported routines
 */

extern RST_DIR	       *rst_setupdir();
extern RST_DIR	       *rst_opendir();
extern daddr_t		rst_telldir();
extern struct dirent   *rst_readdir();
extern void		rst_seekdir();

extern char	       *calloc();
extern char	       *ctime();
extern char	       *fgets();
extern char	       *flagvalues();
extern char	       *fmtentry();
extern char	       *gentempname();
extern char	       *index();
extern char	       *keyval();
extern char	       *malloc();
extern char	       *mktemp();
extern char	       *myname();
extern char	       *realloc();
extern char	       *rindex();
extern char	       *strcat();
extern char	       *strcpy();
extern char	       *strncat();
extern char	       *strncpy();
extern char	       *strdup();
extern ino_t		dirlookup();
extern ino_t		lowerbnd();
extern ino_t		psearch();
extern ino_t		upperbnd();
extern int		argcmp();
extern int		linkit();
extern int		query();
extern int		extractfile();
extern long		addfile();
extern long		deletefile();
extern long		listfile();
extern off_t		lseek();
extern long		nodeupdates();
extern long		verifyfile();
extern void		extractdirs();
extern void		skipdirs();
extern void		treescan();
extern void		canon();
extern void		sigintr();
extern void		deleteino();
extern void		freeentry();
extern void		moveentry();
extern void		dumpsymtable();
extern void		initsymtable();
extern void		renameit();
extern void		newnode();
extern void		removenode();
extern void		removeleaf();
extern void		badentry();
extern void		panic();
extern void		xutimes();
extern void		setinput();
extern void		newtapebuf();
extern void		setup();
extern void		getvol();
extern void		skipmaps();
extern void		skipfile();
extern void		getfile();
extern void		xtrfile(), xtrskip();
extern void		xtrlnkfile(), xtrlnkskip();
extern void		xtrmap(), xtrmapskip();
extern void		closemt();
extern void		msg();
extern void		swabst();
extern void		Exit();
extern void		printdumpinfo();
extern struct inotab   *allocinotab();
extern struct inotab   *inotablookup();

/* global "environment" variables */

extern int	errno;
extern char    *optarg;
extern int	optind;
#define	DEFAULT_TAPE	"/dev/rmt0h"		/* default tape device */
#define SYMBOL_TABLE	"./restoresymtable"	/* symboltable */ 

/*
 * Flags
 */

extern int	old_format_flag;	/* convert from old to new tape format */
extern int	block_size_flag;	/* set input block size */
extern int	debug_flag;		/* print out debugging info */
extern int	children_flag;		/* restore heirarchies */
extern int	by_name_flag;		/* restore by name instead of inode number */
extern int	verbose_flag;		/* print out actions taken */
extern int	auto_retry_flag;	/* always try to recover from tape errors */
extern int	byte_swap_flag;	/* Swap Bytes (for CCI or sun) */
extern int	quad_swap_flag;	/* Swap quads (for sun) */
extern int	Nflag;		/* do not write the disk */
extern int	overwrite_flag;	/* overwrite existing files */

/*
 * Global variables
 */

extern char    *dumpmap;	/* map of inodes on this dump tape */
extern char    *clrimap;	/* map of inodes to be deleted */
extern ino_t	maxino;		/* highest numbered inode in this file system */
extern long	dumpnum;	/* location of the dump on this tape */
extern long	volno;		/* current volume being read */
extern long	ntrec;		/* number of TP_BSIZE records per tape block */
extern time_t	dumptime;	/* time that this dump begins */
extern time_t	dumpdate;	/* time that this dump was made */
extern char	command;	/* opration being performed */
extern FILE    *terminal_fp;	/* file descriptor for the terminal input */
extern FILE    *command_fp;	/* file descriptor for the command input */

/*
 * These variables describe the next file available on the tape
 */

extern char	       *curr_file_name;	/* name of file */
extern ino_t		curr_inumber;	/* inumber of file */
extern struct dinode   *curr_inode;	/* pointer to inode */
extern char		curr_action;	/* action being taken on this file */

#define NULLVOIDFP	(void (*)())0

/*	MSG and NLS stuff	*/

#include	"restore_msg.h"
#include	<locale.h>

extern nl_catd		catopen();
extern char	       *catgets();
extern nl_catd		catd;
#define	MSGSTR(Num, Str)	catgets(catd, MS_RESTORE, Num, Str)


/* string conversion */

struct chrflg
{
	unsigned int	chr;
	unsigned int	flg;
};

typedef struct chrflg	chfl;

#define	CHR(cfp)	((cfp)->chr)
#define	FLG(cfp)	((cfp)->flg)

#define	NOFLG		0x0
#define	QUOTED		0x1
#define	SLASH		0x2

extern char	*cfstostr();
extern chfl	*scanstr();
extern chfl	*strtocfs();
extern chfl	*cfscpy();
extern chfl	*cfscat();
extern chfl	*cfsdup();
extern unsigned int	cfslen();
extern unsigned int	cfscmp();
extern unsigned int	cfsncmp();

#endif /* _RESTORE_H_ */
