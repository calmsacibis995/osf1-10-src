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
 *	@(#)$RCSfile: glob.h,v $ $Revision: 4.2.4.3 $ (DEC) $Date: 1992/03/20 11:15:50 $
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
 * glob.h
 *
 *	Revision History:
 *
 * 12-Mar-91	Fred Canter
 *	Multiple include protection.
 *
 */
/* 
#if !defined(lint) && !defined(_NOIDENT)

#endif
 */
/* 
 * COMPONENT_NAME: CMDMAILX glob.h
 * 
 * FUNCTIONS: 
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
 *      glob.h      5.1 (Berkeley) 6/6/85
 */

#ifndef	_GLOB_H_
#define	_GLOB_H_

/*
 * A bunch of global variable declarations lie herein.
 * def.h must be included first.
 */

int	msgCount;			/* Count of messages read in */
int	mypid;				/* Current process id */
int	rcvmode;			/* True if receiving mail */
int	sawcom;				/* Set after first command */
int	sawdel;				/* Set after delete command */
int	hflag;				/* Sequence number for network -h */
char	*rflag;				/* -r address for network */
char	*Tflag;				/* -T temp file for netnews */
char	nosrc;				/* Don't source /usr/lib/Mail.rc */
char	noheader;			/* Suprress initial header listing */
int	selfsent;			/* User sent self something */
int	senderr;			/* An error while checking */
int	edit;				/* Indicates editing a file */
int	readonly;			/* Will be unable to rewrite file */
int	noreset;			/* String resets suspended */
int	sourcing;			/* Currently reading variant file */
int	loading;			/* Loading user definitions */
int	shudann;			/* Print headers when possible */
int	cond;				/* Current state of conditional exc. */
FILE	*itf;				/* Input temp file buffer */
FILE	*otf;				/* Output temp file buffer */
FILE	*pipef;				/* Pipe file we have opened */
int	image;				/* File descriptor for image of msg */
FILE	*input;				/* Current command input file */
char	*editfile;			/* Name of file being edited */
char	*sflag;				/* Subject given from non tty */
int	outtty;				/* True if standard output a tty */
int	intty;				/* True if standard input a tty */
int	baud;				/* Output baud rate */
char	mbox[PATHSIZE];			/* Name of mailbox file */
char	mailname[PATHSIZE];		/* Name of system mailbox */
int	uid;				/* The invoker's user id */
char	mailrc[PATHSIZE];		/* Name of startup file */
char	deadletter[PATHSIZE];		/* Name of #/dead.letter */
char	homedir[PATHSIZE];		/* Path name of home directory */
char	myname[PATHSIZE];		/* My login id */
off_t	mailsize;			/* Size of system mailbox */
int	lexnumber;			/* Number of TNUMBER from scan() */
char	lexstring[STRINGLEN];		/* String from TSTRING, scan() */
int	regretp;			/* Pointer to TOS of regret tokens */
int	regretstack[REGDEP];		/* Stack of regretted tokens */
char	*stringstack[REGDEP];		/* Stack of regretted strings */
int	numberstack[REGDEP];		/* Stack of regretted numbers */
struct	message	*dot;			/* Pointer to current message */
struct	message	*message;		/* The actual message structure */
struct	var	*variables[HSHSIZE];	/* Pointer to active var list */
struct	grouphead	*groups[HSHSIZE];/* Pointer to active groups */
struct	ignore		*ignore[HSHSIZE];/* Pointer to ignored fields */
struct	ignore		*retain[HSHSIZE];/* Pointer to retained fields */
int	nretained;			/* Number of retained fields */
char	**altnames;			/* List of alternate names for user */
char	**localnames;			/* List of aliases for our local host */
int	debug;				/* Debug flag set */
int	rmail;				/* Being called as rmail */

extern int     Fflag;                   /* -F option (followup) - SVID-2 */
extern int     Hflag;                 	/* print headers and exit - SVID-2 */
extern int	exitflg;		/* -e for mail test - SVID-2 */
extern char	*prompt;		/* prompt string - SVID-2 */
extern char	*Getf(char *);		/* prompt string - SVID-2 */

#include <strings.h>
#include <setjmp.h>

jmp_buf	srbuf;


/*
 * The pointers for the string allocation routines,
 * there are NSPACE independent areas.
 * The first holds STRINGSIZE bytes, the next
 * twice as much, and so on.
 */

#define	NSPACE	25			/* Total number of string spaces */
struct strings {
	char	*s_topFree;		/* Beginning of this area */
	char	*s_nextFree;		/* Next alloctable place here */
	unsigned s_nleft;		/* Number of bytes left here */
} stringdope[NSPACE];

#endif	/* _GLOB_H_ */

