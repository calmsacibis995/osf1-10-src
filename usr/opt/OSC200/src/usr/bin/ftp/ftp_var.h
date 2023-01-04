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
 *	@(#)$RCSfile: ftp_var.h,v $ $Revision: 4.2.3.2 $ (DEC) $Date: 93/03/02 15:18:56 $
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
 * COMPONENT_NAME: TCPIP ftp_var.h
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
 * Copyright (c) 1985 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 *	ftp_var.h	5.9 (Berkeley) 6/1/90
 */
/*
 * ftp_var.h
 *
 *	Revision History:
 *
 * 12-Mar-91	Fred Canter
 *	Multiple include protection.
 *
 */

#ifndef	_FTP_VAR_H_
#define	_FTP_VAR_H_

#include <nl_types.h>
#include <locale.h>
#include "ftp_msg.h"
#define MSGSTR(Num,Str) catgets(catd,MS_FTP,Num,Str)
nl_catd catd;           /* message catalog file descriptor */

#define MAX_LINE_LEN	256
#define MAX_MARGV_SIZE	64

/*
 * FTP global variables.
 */

/*
 * Options and other state info.
 */
int	trace;			/* trace packets exchanged */
int	hash;			/* print # for each buffer transferred */
int	sendport;		/* use PORT cmd for each data connection */
int	verbose;		/* print messages coming back from server */
int	connected;		/* connected to server */
int	fromatty;		/* input is from a terminal */
int	interactive;		/* interactively prompt on m* cmds */
int	debug;			/* debugging level */
int	bell;			/* ring bell on cmd completion */
int	doglob;			/* glob local file names */
int	autologin;		/* establish user account on connection */
int	proxy;			/* proxy server connection active */
int	proxflag;		/* proxy connection exists */
int	sunique;		/* store files on server with unique name */
int	runique;		/* store local files with unique name */
int	mcase;			/* map upper to lower case for mget names */
int	ntflag;			/* use ntin ntout tables for name translation */
int	mapflag;		/* use mapin mapout templates on file names */
int	code;			/* return/reply code for ftp command */
int	crflag;			/* if 1, strip car. rets. on ascii gets */
char	pasv[64];		/* passive port for proxy data connection */
char	*altarg;		/* argv[1] with no shell-like preprocessing  */
char	ntin[17];		/* input translation table */
char	ntout[17];		/* output translation table */
#include <sys/param.h>
char	mapin[MAXPATHLEN];	/* input map template */
char	mapout[MAXPATHLEN];	/* output map template */
char	typename[32];		/* name of file transfer type */
int     type;                   /* requested file transfer type */
int     curtype;                /* current file transfer type */
char	structname[32];		/* name of file transfer structure */
int	stru;			/* file transfer structure */
char	formname[32];		/* name of file transfer format */
int	form;			/* file transfer format */
char	modename[32];		/* name of file transfer mode */
int	mode;			/* file transfer mode */
char	bytename[32];		/* local byte size in ascii */
int	bytesize;		/* local byte size in binary */

char	*hostname;		/* name of host connected to */
int     unix_server;            /* server is unix, can use binary for ascii */
int     unix_proxy;             /* proxy is unix, can use binary for ascii */

struct	servent *sp;		/* service spec for tcp/ftp */

#include <setjmp.h>
jmp_buf	toplevel;		/* non-local goto stuff for cmd scanner */

char	line[MAX_LINE_LEN];	/* input line buffer */
char	*stringbase;		/* current scan point in line buffer */
char	argbuf[MAX_LINE_LEN];	/* argument storage buffer */
char	*argbase;		/* current storage point in arg buffer */
int	margc;			/* count of arguments on input line */
char	*margv[MAX_MARGV_SIZE];	/* args parsed from input line */
int     cpend;                  /* flag: if != 0, then pending server reply */
int	mflag;			/* flag: if != 0, then active multi command */

int	options;		/* used during socket creation */

/*
 * Format of command table.
 */
struct cmd {
	char	*c_name;	/* name of command */
	int	help_id;	/* index into message table */
	char	*c_help;	/* help string */
	char	c_bell;		/* give bell when command completes */
	char	c_conn;		/* must be connected to use command */
	char	c_proxy;	/* proxy server may execute */
	int	(*c_handler)();	/* function to call */
};

struct macel {
	char mac_name[9];	/* macro name */
	char *mac_start;	/* start of macro in macbuf */
	char *mac_end;		/* end of macro in macbuf */
};

int macnum;			/* number of defined macros */
struct macel macros[16];
char macbuf[4096];

extern	char *tail();
extern	char *index();
extern	char *rindex();
extern	char *remglob();
extern	int errno;
extern	char *mktemp();
extern	char *strncpy();
extern	char *strncat();
extern	char *strcat();
extern	char *strcpy();

#define HASHBYTES 1024  /* number of bytes represented by each '#' mark */

#endif	/* _FTP_VAR_H_ */
