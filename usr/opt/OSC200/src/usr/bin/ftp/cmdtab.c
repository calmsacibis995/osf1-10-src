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
#ifndef lint
static char	*sccsid = "@(#)$RCSfile: cmdtab.c,v $ $Revision: 4.2.4.3 $ (DEC) $Date: 1993/01/18 14:21:53 $";
#endif 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0
 */ 
#if !defined(lint) && !defined(_NOIDENT)

#endif
/* static char sccsid[] = "cmdtab.c	1.5  com/sockcmd/ftp,3.1,9011 10/8/89 16:11:21"; */
/* 
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1985, 1989 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice and this paragraph are
 * duplicated in all such forms and that any documentation,
 * advertising materials, and other materials related to such
 * distribution and use acknowledge that the software was developed
 * by the University of California, Berkeley.  The name of the
 * University may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */
/*
#ifndef lint
static char sccsid[] = "cmdtab.c	5.10 (Berkeley) 6/1/90";
#endif  not lint */

#include "ftp_var.h"

/*
 * User FTP -- Command Tables.
 */
int	setascii(), setbell(), setbinary(), setdebug(), setform();
int	setglob(), sethash(), setmode(), setpeer(), setport();
int	setprompt(), setstruct();
int	settenex(), settrace(), settype(), setverbose();
int	disconnect(), restart(), reget(), syst();
int	cd(), lcd(), delete(), mdelete(), user();
int	ls(), mls(), get(), mget(), help(), append(), put(), mput();
int	quit(), renamefile(), status();
int	quote(), rmthelp(), shell(), site();
int	pwd(), makedir(), removedir(), setcr();
int	account(), doproxy(), reset(), setcase(), setntrans(), setnmap();
int	setsunique(), setrunique(), cdup(), macdef(), domacro();
int	sizecmd(), modtime(), newer(), rmtstatus();
int     do_chmod(), do_umask(), idle();

char	accounthelp[] =	"send account command to remote server";
char	appendhelp[] =	"append to a file";
char	asciihelp[] =	"set ascii transfer type";
char	beephelp[] =	"beep when command completed";
char	binaryhelp[] =	"set binary transfer type";
char	casehelp[] =	"toggle mget upper/lower case id mapping";
char	cdhelp[] =	"change remote working directory";
char	cduphelp[] = 	"change remote working directory to parent directory";
char    chmodhelp[] =   "change file permissions of remote file";
char	connecthelp[] =	"connect to remote tftp";
char	crhelp[] =	"toggle carriage return stripping on ascii gets";
char	deletehelp[] =	"delete remote file";
char	debughelp[] =	"toggle/set debugging mode";
char	dirhelp[] =	"list contents of remote directory";
char	disconhelp[] =	"terminate ftp session";
char	domachelp[] = 	"execute macro";
char	formhelp[] =	"set file transfer format";
char	globhelp[] =	"toggle metacharacter expansion of local file names";
char    imagehelp[] =   "set image transfer type";
char	hashhelp[] =	"toggle printing `#' for each buffer transferred";
char	helphelp[] =	"print local help information";
char    idlehelp[] =    "get (set) idle timer on remote side";
char	lcdhelp[] =	"change local working directory";
char	lshelp[] =	"nlist contents of remote directory";
char	macdefhelp[] =  "define a macro";
char	mdeletehelp[] =	"delete multiple files";
char	mdirhelp[] =	"list contents of multiple remote directories";
char	mgethelp[] =	"get multiple files";
char	mkdirhelp[] =	"make directory on the remote machine";
char	mlshelp[] =	"list contents of multiple remote directories";
char	modtimehelp[] = "show last modification time of remote file";
char	modehelp[] =	"set file transfer mode";
char	mputhelp[] =	"send multiple files";
char    newerhelp[] =   "get file if remote file is newer than local file ";
char	nlisthelp[] =	"nlist contents of remote directory";
char	nmaphelp[] =	"set templates for default file name mapping";
char	ntranshelp[] =	"set translation table for default file name mapping";
char	porthelp[] =	"toggle use of PORT cmd for each data connection";
char	prompthelp[] =	"force interactive prompting on multiple commands";
char	proxyhelp[] =	"issue command on alternate connection";
char	pwdhelp[] =	"print working directory on remote machine";
char	quithelp[] =	"terminate ftp session and exit";
char	quotehelp[] =	"send arbitrary ftp command";
char	receivehelp[] =	"receive file";
char    regethelp[] =   "get file restarting at end of local file";
char	remotehelp[] =	"get help from remote server";
char	renamehelp[] =	"rename file";
char    restarthelp[]=  "restart file transfer at bytecount";
char	rmdirhelp[] =	"remove directory on the remote machine";
char	rmtstatushelp[]="show status of remote machine";
char	runiquehelp[] = "toggle store unique for local files";
char	resethelp[] =	"clear queued command replies";
char	sendhelp[] =	"send one file";
char    sitehelp[] =    "send site specific command to remote server\n\t\tTry \"rhelp site\" or \"site help\" for more information";
char	shellhelp[] =	"escape to the shell";
char	sizecmdhelp[] = "show size of remote file";
char	statushelp[] =	"show current status";
char	structhelp[] =	"set file transfer structure";
char	suniquehelp[] = "toggle store unique on remote machine";
char	systemhelp[] =  "show remote system type";
char	tenexhelp[] =	"set tenex file transfer type";
char	tracehelp[] =	"toggle packet tracing";
char	typehelp[] =	"set file transfer type";
char    umaskhelp[] =   "get (set) umask on remote side";
char	userhelp[] =	"send new user information";
char	verbosehelp[] =	"toggle verbose mode";


struct cmd cmdtab[] = {
	{ "!",
	SHELLHELP,
		shellhelp,	0,	0,	0,	shell },
	{ "$",
	DOMACHELP,
	domachelp,	1,	0,	0,	domacro },
	{ "account",
	ACCOUNTHELP,
	accounthelp,	0,	1,	1,	account},
	{ "append ",
	APPENDHELP,
	appendhelp,	1,	1,	1,	put },
	{ "ascii",	
	ASCIIHELP,
	asciihelp,	0,	1,	1,	setascii },
	{ "bell",	
	BEEPHELP,
	beephelp,	0,	0,	0,	setbell },
	{ "binary",	
	BINARYHELP,
	binaryhelp,	0,	1,	1,	setbinary },
	{ "bye",	
	QUITHELP, 
	quithelp,	0,	0,	0,	quit },
	{ "case",	
	CASEHELP,
	casehelp,	0,	0,	1,	setcase },
	{ "cd",		
	CDHELP,
	cdhelp,		0,	1,	1,	cd },
	{ "cdup",	
	CDUPHELP,
	cduphelp,	0,	1,	1,	cdup },
	{ "close",	
	DISCONHELP,
	disconhelp,	0,	1,	1,	disconnect },
	{ "cr",		
	CRHELP,
	crhelp,		0,	0,	0,	setcr },
	{ "delete",	
	DELETEHELP,
	deletehelp,	0,	1,	1,	delete },
	{ "debug",	
	DEBUGHELP,
	debughelp,	0,	0,	0,	setdebug },
	{ "dir",	
	DIRHELP,
	dirhelp,	1,	1,	1,	ls },
	{ "disconnect",	
	DISCONHELP,
	disconhelp,	0,	1,	1,	disconnect },
	{ "form",	
	FORMHELP,
	formhelp,	0,	1,	1,	setform },
	{ "get",	
	RECEIVEHELP,
	receivehelp,	1,	1,	1,	get },
	{ "glob",	
	GLOBHELP,
	globhelp,	0,	0,	0,	setglob },
	{ "hash",	
	HASHHELP,
	hashhelp,	0,	0,	0,	sethash },
	{ "help",	
	HELPHELP,
	helphelp,	0,	0,	1,	help },
	{ "image",	
	BINARYHELP,
		binaryhelp,	0,	1,	1,	setbinary },
	{ "lcd",	
	LCDHELP,
	lcdhelp,	0,	0,	0,	lcd },
	{ "ls",		
	LSHELP,
	lshelp,		1,	1,	1,	ls },
	{ "macdef",	
	MACDEFHELP,
	macdefhelp,	0,	0,	0,	macdef },
	{ "mdelete",	
	MDELETEHELP,
	mdeletehelp,	1,	1,	1,	mdelete },
	{ "mdir",	
	MDIRHELP,
	mdirhelp,	1,	1,	1,	mls },
	{ "mget",	
	MGETHELP,
	mgethelp,	1,	1,	1,	mget },
	{ "mkdir",	
	MKDIRHELP,
	mkdirhelp,	0,	1,	1,	makedir },
	{ "mls",	
	MLSHELP,
	mlshelp,	1,	1,	1,	mls },
	{ "mode",	
	MODEHELP,
	modehelp,	0,	1,	1,	setmode },
	{ "modtime",	
	MODTIMEHELP,
		modtimehelp,	0,	1,	1,	modtime },
	{ "mput",	
	MPUTHELP,
	mputhelp,	1,	1,	1,	mput },
	{ "nmap",	
	NMAPHELP,
	nmaphelp,	0,	0,	1,	setnmap },
	{ "nlist",	
	NLISTHELP,
		nlisthelp,	1,	1,	1,	ls },
	{ "ntrans",	
	NTRANSHELP,
	ntranshelp,	0,	0,	1,	setntrans },
	{ "open",	
	CONNECTHELP,
	connecthelp,	0,	0,	1,	setpeer },
	{ "prompt",	
	PROMPTHELP,
	prompthelp,	0,	0,	0,	setprompt },
	{ "proxy",	
	PROXYHELP,
	proxyhelp,	0,	0,	1,	doproxy },
	{ "sendport",	
	PORTHELP,
	porthelp,	0,	0,	0,	setport },
	{ "put",	
	SENDHELP,
	sendhelp,	1,	1,	1,	put },
	{ "pwd",	
	PWDHELP,
pwdhelp,	0,	1,	1,	pwd },
	{ "quit",	
	QUITHELP,
	quithelp,	0,	0,	0,	quit },
	{ "quote",	
	QUOTEHELP,
	quotehelp,	1,	1,	1,	quote },
	{ "recv",	
	RECEIVEHELP,
	receivehelp,	1,	1,	1,	get },
	{ "remotehelp",	
	REMOTEHELP,
	remotehelp,	0,	1,	1,	rmthelp },
	{ "rstatus",	
	RMTSTATUSHELP,
		rmtstatushelp,	0,	1,	1,	rmtstatus },
	{ "rhelp",	
	REMOTEHELP,
		remotehelp,	0,	1,	1,	rmthelp },
	{ "rename",	
	RENAMEHELP,
	renamehelp,	0,	1,	1,	renamefile },
	{ "reset",	
	RESETHELP,
	resethelp,	0,	1,	1,	reset },
	{ "rmdir",	
	RMDIRHELP,
	rmdirhelp,	0,	1,	1,	removedir },
	{ "runique",	
	RUNIQUEHELP,
	runiquehelp,	0,	0,	1,	setrunique },
	{ "send",	
	SENDHELP,
	sendhelp,	1,	1,	1,	put },
	{ "site",
	SITEHELP,
	sitehelp,	1,	1,	1,	site },
	{ "size",	
	SIZECMDHELP,
		sizecmdhelp,	1,	1,	1,	sizecmd },
	{ "status",	
	STATUSHELP,
	statushelp,	0,	0,	1,	status },
	{ "struct",	
	STRUCTHELP,
	structhelp,	0,	1,	1,	setstruct },
	{ "sunique",	
	SUNIQUEHELP,
	suniquehelp,	0,	0,	1,	setsunique },
	{ "system",	
	SYSTEMHELP,
		systemhelp,	0,	1,	1,	syst },
	{ "tenex",	
	TENEXHELP,
	tenexhelp,	0,	1,	1,	settenex },
	{ "trace",	
	TRACEHELP,
	tracehelp,	0,	0,	0,	settrace },
	{ "type",	
	TYPEHELP,
	typehelp,	0,	1,	1,	settype },
	{ "user",	
	USERHELP,
	userhelp,	0,	1,	1,	user },
	{ "verbose",	
	VERBOSEHELP,
	verbosehelp,	0,	0,	0,	setverbose },
	{ "?",		
	HELPHELP,
	helphelp,	0,	0,	1,	help },
	{ 0 },
};

int	NCMDS = (sizeof (cmdtab) / sizeof (cmdtab[0])) - 1;
