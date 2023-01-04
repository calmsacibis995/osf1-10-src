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
#if !defined(lint) && !defined(_NOIDENT)
static char rcsid[] = "@(#)$RCSfile: tip.h,v $ $Revision: 4.2.10.5 $ (DEC) $Date: 1993/09/30 18:39:49 $";
#endif
 */
/* @(#)tip.h	1.4  com/cmd/tip,3.1,9013 10/15/89 10:44:09 */
/* 
 * COMPONENT_NAME: UUCP tip.h
 * 
 * FUNCTIONS: address, boolean, character, equal, logent, number, 
 *            value 
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * Copyright (c) 1983 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 *	tip.h	5.2 (Berkeley) 1/13/86
 *	tip.h	1.8 - 89/08/14 - 19:52:02
 */

/*
 * tip - terminal interface program
 */


#include <sgtty.h>
#include <sys/types.h>
#include <sys/file.h>

#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include <time.h>
#include <pwd.h>
#include <ctype.h>
#include <setjmp.h>
#include <errno.h>
#include "tip_msg.h"

nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_TIP,n,s)

/*
 * Remote host attributes
 */
char	*DV;			/* UNIX device(s) to open */
char	*EL;			/* chars marking an EOL */
char	*CM;			/* initial connection message */
char	*IE;			/* EOT to expect on input */
char	*OE;			/* EOT to send to complete FT */
char	*CU;			/* call unit if making a phone call */
char	*AT;			/* acu type */
char	*PN;			/* phone number(s) */
char	*DI;			/* disconnect string */
char	*PA;			/* parity to be generated */

char	*PH;			/* phone number file */
char	*RM;			/* remote file name */
char	*HO;			/* host name */

int	BR;			/* line speed for conversation */
int	FS;			/* frame size for transfers */

char	DU;			/* this host is dialed up */
char	HW;			/* this device is hardwired, see hunt.c */
char	*ES;			/* escape character */
char	*EX;			/* exceptions */
char	*FO;			/* force (literal next) char*/
char	*RC;			/* raise character */
char	*RE;			/* script record file */
char	*PR;			/* remote prompt */
int	DL;			/* line delay for file transfers to remote */
int	CL;			/* char delay for file transfers to remote */
int	ET;			/* echocheck timeout */
char	HD;			/* this host is half duplex - do local echo */

/*
 * String value table
 */
typedef
	struct {
		char	*v_name;	/* whose name is it */
		char	v_type;		/* for interpreting set's */
		char	v_access;	/* protection of touchy ones */
		char	*v_abrev;	/* possible abreviation */
		char	*v_value;	/* casted to a union later */
	}
	value_t;

#define STRING	01		/* string valued */
#define BOOL	02		/* true-false value */
#define NUMBER	04		/* numeric value */
#define CHAR	010		/* character value */

#define WRITE	01		/* write access to variable */
#define	READ	02		/* read access */

#define CHANGED	01		/* low bit is used to show modification */
#define PUBLIC	1		/* public access rights */
#define PRIVATE	03		/* private to definer */
#define ROOT	05		/* root defined */

#define	TRUE	1
#define FALSE	0

#define ENVIRON	020		/* initialize out of the environment */
#define IREMOTE	040		/* initialize out of remote structure */
#define INIT	0100		/* static data space used for initialization */
#define TMASK	017

/*
 * Definition of ACU line description
 */
typedef
	struct {
		char	*acu_name;
		int	(*acu_dialer)();
		int	(*acu_disconnect)();
		int	(*acu_abort)();
	}
	acu_t;

#define	equal(a, b)	(strcmp(a,b)==0)/* A nice function to string compare */

/*
 * variable manipulation stuff --
 *   if we defined the value entry in value_t, then we couldn't
 *   initialize it in vars.c, so we cast it as needed to keep lint
 *   happy.
 */
typedef
	union {
		long	zz_number;
		short	zz_boolean[sizeof(long) / sizeof(short)];
		char	zz_character[sizeof(long) / sizeof(char)];
		long	*zz_address;
	}
	zzhack;

#define value(v)	vtable[v].v_value

#define number(v)	((((zzhack *)(&(v))))->zz_number)

#if BYTE_ORDER == LITTLE_ENDIAN
#define boolean(v)      ((((zzhack *)(&(v))))->zz_boolean[0])
#define character(v)    ((((zzhack *)(&(v))))->zz_character[0])
#else	/* BYTE_ORDER == BIG_ENDIAN */
#define boolean(v)      ((((zzhack *)(&(v))))->zz_boolean[1])
#define character(v)    ((((zzhack *)(&(v))))->zz_character[3])
#endif	/* BYTE_ORDER */

#define address(v)	((((zzhack *)(&(v))))->zz_address)

/*
 * Escape command table definitions --
 *   lookup in this table is performed when ``escapec'' is recognized
 *   at the begining of a line (as defined by the eolmarks variable).
*/

typedef
	struct {
		char	e_char;		/* char to match on */
		char	e_flags;	/* experimental, priviledged */
		char	*e_help;	/* help string */
		int 	(*e_func)();	/* command */
	}
	esctable_t;

#define NORM	00		/* normal protection, execute anyone */
#define EXP	01		/* experimental, mark it with a `*' on help */
#define PRIV	02		/* priviledged, root execute only */

extern int	vflag;		/* verbose during reading of .tiprc file */
extern value_t	vtable[];	/* variable table */

#ifndef ACULOG
#define logent(a, b, c, d)
#define loginit()
#endif

/*
 * Definition of indices into variable table so
 *  value(DEFINE) turns into a static address.
 */

#define BEAUTIFY	0
#define BAUDRATE	1
#define DIALTIMEOUT	2
#define EOFREAD		3
#define EOFWRITE	4
#define EOL		5
#define ESCAPE		6
#define EXCEPTIONS	7
#define FORCE		8
#define FRAMESIZE	9
#define HOST		10
#define LOG		11
#define PHONES		12
#define PROMPT		13
#define RAISE		14
#define RAISECHAR	15
#define RECORD		16
#define REMOTE		17
#define SCRIPT		18
#define TABEXPAND	19
#define VERBOSE		20
#define SHELL		21
#define HOME		22
#define ECHOCHECK	23
#define DISCONNECT	24
#define TAND		25
#define LDELAY		26
#define CDELAY		27
#define ETIMEOUT	28
#define RAWFTP		29
#define HALFDUPLEX	30
#define	LECHO		31
#define	PARITY		32

#define NOVAL	((value_t *)NULL)
#define NOACU	((acu_t *)NULL)
#define NOSTR	((char *)NULL)
#define TIP_NOFILE	((FILE *)NULL)
#define NOPWD	((struct passwd *)0)

struct sgttyb	arg;		/* current mode of local terminal */
struct sgttyb	defarg;		/* initial mode of local terminal */
struct tchars	tchars;		/* current state of terminal */
struct tchars	defchars;	/* initial state of terminal */
struct ltchars	ltchars;	/* current local characters of terminal */
struct ltchars	deflchars;	/* initial local characters of terminal */

FILE	*fscript;		/* FILE for scripting */

int	fildes[2];		/* file transfer synchronization channel */
int	repdes[2];		/* read process sychronization channel */
int	FD;			/* open file descriptor to remote host */
int	AC;			/* open file descriptor to dialer (v831 only) */
int	vflag;			/* print .tiprc initialization sequence */
int	sfd;			/* for ~< operation */
int	pid;			/* pid of tipout */
uid_t	uid, euid;		/* real and effective user id's */
gid_t	gid, egid;		/* real and effective group id's */
int	stop;			/* stop transfer session flag */
int	quit;			/* same; but on other end */
int	intflag;		/* recognized interrupt */
int	stoprompt;		/* for interrupting a prompt session */
int	timedout;		/* ~> transfer timedout */
int	cumode;			/* simulating the "cu" program */

char	fname[80];		/* file name buffer for ~< */
char	copyname[80];		/* file name buffer for ~> */
char	ccc;			/* synchronization character */
char	ch;			/* for tipout */
char	*uucplock;		/* name of lock file for uucp's */

int	odisc;				/* initial tty line discipline */
extern	int disc;			/* current tty discpline */


extern	char *ctrl();
extern	char *vinterp();
extern  char *getenv();
extern  void *malloc();
extern	char *connect();
#ifdef GENACU

#define DEFDELAY 1
#define GENBUF 512
#define MAXSTR 128


extern  char *syncstr, *respstr, *dialstr, *respdial, *abostr,
                *disconstr, charup, *compstr, hangup, dtr,
                *respsync, *dialterm;
extern int syncdel, dialdel, cdelay, compdel;

#endif
