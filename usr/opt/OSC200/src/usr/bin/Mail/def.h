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
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
static char rcsid[] = "@(#)$RCSfile: def.h,v $ $Revision: 4.2.9.3 $ (DEC) $Date: 1993/12/21 21:36:36 $";
 */
/*
 * HISTORY
 */
/*
 * OSF/1 Release 1.0
 */
/* 
 * COMPONENT_NAME: CMDMAILX def.h
 * 
 * FUNCTIONS: equal, reset, setexit, sigchild, trunc, 
 *            vfork 
 *
 * ORIGINS: 10  26  27 
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 *      def.h       5.2 (Berkeley) 11/2/85
 */

#include <sys/param.h>		/* includes <sys/types.h> */
#include <signal.h>
#include <stdio.h>
#include <sys/termio.h>
#ifdef OSF
#include <stdlib.h>   /* for malloc declarations */
#endif
#include "local.h"

#undef isalpha
#undef isdigit

/*
 * Mail -- a mail program
 *
 * Commands are:
 *	t <message list>		print out these messages
 *	r <message list>		reply to messages
 *	m <user list>			mail to users (analogous to send)
 *	e <message list>		edit messages
 *	c [directory]			chdir to dir or home if none
 *	x				exit quickly
 *	w <message list> file		save messages in file
 *	q				quit, save remaining stuff in mbox
 *	d <message list>		delete messages
 *	u <message list>		undelete messages
 *	h				print message headers
 *
 * Author: Kurt Shoens (UCB) March 25, 1978
 */

#define	ESCAPE		'~'		/* Default escape for sending */
#define	NMLSIZE		1024		/* max names in a message list */
#define	PATHSIZE	1024		/* Size of pathnames throughout */
#define	NAMESIZE	32		/* Max size of user name */
#define	HSHSIZE		59		/* Hash size for aliases and vars */
#define	HDRFIELDS	3		/* Number of header fields */
#define	LINESIZE	BUFSIZ		/* max readable line width */
#define	STRINGSIZE	((unsigned) 128)/* Dynamic allocation units */
#define	MAXARGC		1024		/* Maximum list of raw strings */
#define	NULLSTR		((char *) 0)	/* Null string pointer */
#define	MAXEXP		25		/* Maximum expansion of aliases */
#define	equal(a, b)	(strcmp(a,b)==0)/* A nice function to string compare */

struct message {
	short	m_flag;			/* flags, see below */
	int	m_block;		/* block number of this message */
	short	m_offset;		/* offset in block of message */
	long	m_size;			/* Bytes in the message */
	int	m_lines;		/* Lines in the message */
};

/*
 * flag bits.
 */

#define	MUSED		(1<<0)		/* entry is used, but this bit isn't */
#define	MDELETED	(1<<1)		/* entry has been deleted */
#define	MSAVED		(1<<2)		/* entry has been saved */
#define	MTOUCH		(1<<3)		/* entry has been noticed */
#define	MPRESERVE	(1<<4)		/* keep entry in sys mailbox */
#define	MMARK		(1<<5)		/* message is marked! */
#define	MODIFY		(1<<6)		/* message has been modified */
#define	MNEW		(1<<7)		/* message has never been seen */
#define	MREAD		(1<<8)		/* message has been read sometime. */
#define	MSTATUS		(1<<9)		/* message status has changed */
#define	MBOX		(1<<10)		/* Send this to mbox, regardless */

/*
 * Format of the command description table.
 * The actual table is declared and initialized
 * in lex.c
 */

struct cmd {
	char	*c_name;		/* Name of command */
	int	(*c_func)();		/* Implementor of the command */
	short	c_argtype;		/* Type of arglist (see below) */
	short	c_msgflag;		/* Required flags of messages */
	short	c_msgmask;		/* Relevant flags of messages */
};

/* Yechh, can't initialize unions */

#define	c_minargs c_msgflag		/* Minimum argcount for RAWLIST */
#define	c_maxargs c_msgmask		/* Max argcount for RAWLIST */

/*
 * Argument types.
 */

#define	MSGLIST	 0		/* Message list type */
#define	STRLIST	 1		/* A pure string */
#define	RAWLIST	 2		/* Shell string list */
#define	NOLIST	 3		/* Just plain 0 */
#define	NDMLIST	 4		/* Message list, no defaults */

#define	P	040		/* Autoprint dot after command */
#define	I	0100		/* Interactive command bit */
#define	M	0200		/* Legal from send mode bit */
#define	W	0400		/* Illegal when read only bit */
#define	F	01000		/* Is a conditional command */
#define	T	02000		/* Is a transparent command */
#define	R	04000		/* Cannot be called from collect */

/*
 * Oft-used mask values
 */

#define	MMNORM		(MDELETED|MSAVED)/* Look at both save and delete bits */
#define	MMNDEL		MDELETED	/* Look only at deleted bit */

/*
 * Structure used to return a break down of a head
 * line (hats off to Bill Joy!)
 */

struct headline {
	char	*l_from;	/* The name of the sender */
	char	*l_tty;		/* His tty string (if any) */
	char	*l_date;	/* The entire date string */
};

#define	GTO	1		/* Grab To: line */
#define	GSUBJECT 2		/* Likewise, Subject: line */
#define	GCC	4		/* And the Cc: line */
#define	GBCC	8		/* And also the Bcc: line */
#define	GMASK	(GTO|GSUBJECT|GCC|GBCC)
				/* Mask of places from whence */

#define	GNL	16		/* Print blank line after */
#define	GDEL	32		/* Entity removed from list */
#define	GCOMMA	64		/* detract puts in commas */

/*
 * Structure used to pass about the current
 * state of the user-typed message header.
 */

struct header {
	char	*h_to;			/* Dynamic "To:" string */
	char	*h_subject;		/* Subject string */
	char	*h_cc;			/* Carbon copies string */
	char	*h_bcc;			/* Blind carbon copies */
	int	h_seq;			/* Sequence for optimization */
};

/*
 * Structure of namelist nodes used in processing
 * the recipients of mail and aliases and all that
 * kind of stuff.
 */

struct name {
	struct	name *n_flink;		/* Forward link in list. */
	struct	name *n_blink;		/* Backward list link */
	short	n_type;			/* From which list it came */
	char	*n_name;		/* This fella's name */
};

/*
 * Structure of a variable node.  All variables are
 * kept on a singly-linked list of these, rooted by
 * "variables"
 */

struct var {
	struct	var *v_link;		/* Forward link to next variable */
	char	*v_name;		/* The variable's name */
	char	*v_value;		/* And it's current value */
};

struct group {
	struct	group *ge_link;		/* Next person in this group */
	char	*ge_name;		/* This person's user name */
};

struct grouphead {
	struct	grouphead *g_link;	/* Next grouphead in list */
	char	*g_name;		/* Name of this group */
	struct	group *g_list;		/* Users in group. */
};

#define	NIL	((struct name *) 0)	/* The nil pointer for namelists */
#define	NONE	((struct cmd *) 0)	/* The nil pointer to command tab */
#define	NOVAR	((struct var *) 0)	/* The nil pointer to variables */
#define	NOGRP	((struct grouphead *) 0)/* The nil grouphead pointer */
#define	NOGE	((struct group *) 0)	/* The nil group pointer */

/*
 * Structure of the hash table of ignored header fields
 */
struct ignore {
	struct ignore	*i_link;	/* Next ignored field in bucket */
	char		*i_field;	/* This ignored field */
};

/*
 * Token values returned by the scanner used for argument lists.
 * Also, sizes of scanner-related things.
 */

#define	TEOL	0			/* End of the command line */
#define	TNUMBER	1			/* A message number */
#define	TDASH	2			/* A simple dash */
#define	TSTRING	3			/* A string (possibly containing -) */
#define	TDOT	4			/* A "." */
#define	TUP	5			/* An "^" */
#define	TDOLLAR	6			/* A "$" */
#define	TSTAR	7			/* A "*" */
#define	TOPEN	8			/* An '(' */
#define	TCLOSE	9			/* A ')' */
#define TPLUS	10			/* A '+' */

#define	REGDEP	2			/* Maximum regret depth. */
#define	STRINGLEN	1024		/* Maximum length of string token */

/*
 * Constants for conditional commands.  These describe whether
 * we should be executing stuff or not.
 */

#define	CANY		0		/* Execute in send or receive mode */
#define	CRCV		1		/* Execute in receive mode only */
#define	CSEND		2		/* Execute in send mode only */

/*
 * Kludges to handle the change from setexit / reset to setjmp / longjmp
 */
#define	setexit()	setjmp(srbuf)
#define	reset(x)	longjmp(srbuf, x)


#ifndef	SIGRETRO
#define	sigchild()
#endif

/*
 * Truncate a file to the last character written. This is
 * useful just before closing an old file that was opened
 * for read/write.
 */
#define trunc(stream)	ftruncate(fileno(stream), (long) ftell(stream))

/*
 * Forward declarations of routine types to keep lint and cc happy.
 */
FILE	*Fdopen();
FILE	*collect();
FILE	*infix();
FILE	*mesedit();
FILE	*mespipe();
/*FILE	*popen(); */
FILE	*setinput();
char	**unpack();
char	*addto();
char	*arpafix();
#ifndef OSF
char	*calloc();  /* use declaration in stdlib.h */
#endif
char	*copy();
char	*copyin();
char	*detract();
char	*expand();
/*char	*gets(); */
char	*hfield();

#ifdef _NO_PROTO
char *index();
char *rindex();
#else
char *index(const char*, char);
char *rindex(const char*, char);
#endif  /* _NO_PROTO */

char	*name1();
char	*nameof();
char	*nextword();
char	*getenv();
char	*getfilename();
char	*gettmpdir(); /*GAG*/
char	*hcontents();
char	*netmap();
char	*netname();
char	*readtty();
char	*reedit();
char	*revarpa();
char	*rpair();
char	*salloc();
char	*savestr();
char	*skin();
char	*snarf();
char	*strcat();
char	*strcpy();
char	*value();
char	*vcopy();
char	*yankword();
off_t	fsize();
struct	cmd	*lex();
struct	grouphead	*findgroup();
struct	name	*cat();
struct	name	*delname();
struct	name	*elide();
struct	name	*extract();
struct	name	*gexpand();
struct	name	*map();
struct	name	*outof();
struct	name	*put();
struct	name	*usermap();
struct	name	*verify();
struct	var	*lookup();
long	transmit();
int	icequal();
int	cmpdomain();
