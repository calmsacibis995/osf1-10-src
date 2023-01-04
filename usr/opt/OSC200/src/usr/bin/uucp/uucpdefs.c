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
#if !defined(lint) && !defined(_NOIDENT)
static char rcsid[] = "@(#)$RCSfile: uucpdefs.c,v $ $Revision: 4.3.6.3 $ (DEC) $Date: 1994/01/11 17:45:19 $";
#endif
/* 
 * COMPONENT_NAME: UUCP uucpdefs.c
 * 
 * FUNCTIONS: 
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */



/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/
/*
uucpdefs.c	1.4  com/cmd/uucp,3.1,9013 11/27/89 10:58:15";
*/

/*	uucp:uucpdefs.c	1.6
*/
#include "uucp.h"
/* VERSION( uucpdefs.c	5.2 -  -  ); */

int	Ifn, Ofn;
int	Uid, Euid;		/* user-id and effective-uid */
char	Progname[NAMESIZE];
char	Pchar;
char	Rmtname[MAXFULLNAME];
char	RemSpool[MAXFULLNAME];	/* spool subdirectory for remote system */
char	User[MAXFULLNAME];
char	Uucp[NAMESIZE];
char	Loginuser[NAMESIZE];
char	Myname[MAXBASENAME+1];
char	Wrkdir[MAXFULLNAME];
char	Logfile[MAXFULLNAME];
char	*Spool = SPOOL;
char	*Pubdir = PUBDIR;
char	**Env;

int	Retrytime = 0;
struct	nstat Nstat;
char	Dc[15];			/* line name				*/
int	Seqn;			/* sequence #				*/
int	Role;
char	*Bnptr;			/* used when BASENAME macro is expanded */
char	Jobid[NAMESIZE] = "";	/* Jobid of current C. file */
int	Uerror;			/* global error code */

int	Verbose = 0;	/* for cu and ct only */
int	Debug = 0;
int IsTcpIp = 0;        /* 1 == TCP/IP connection, else 0.  suppress ioctl */

/* used for READANY and READSOME macros */
struct stat __s_;
#ifdef CT_MESSAGES
char    *Ct_OPEN =      "CAN'T OPEN";
char    *Ct_WRITE =     "CAN'T WRITE";
char    *Ct_READ =      "CAN'T READ";
char    *Ct_CREATE =    "CAN'T CREATE";
char    *Ct_ALLOCATE =  "CAN'T ALLOCATE";
char    *Ct_LOCK =      "CAN'T LOCK";
char    *Ct_STAT =      "CAN'T STAT";
char    *Ct_CHOWN =     "CAN'T CHOWN";
char    *Ct_CHMOD =     "CAN'T CHMOD";
char    *Ct_LINK =      "CAN'T LINK";
char    *Ct_CHDIR =     "CAN'T CHDIR";
char    *Ct_UNLINK =    "CAN'T UNLINK";
char    *Wr_ROLE =      "WRONG ROLE";
char    *Ct_CORRUPT =   "CAN'T MOVE TO CORRUPTDIR";
char    *Ct_CLOSE =     "CAN'T CLOSE";
char    *Ct_FORK =      "CAN'T FORK";
char    *Fl_EXISTS =    "FILE EXISTS";
#endif /* CT_MESSAGES */

char *UerrorText[] = {
  /* SS_OK			0 */ "SUCCESSFUL",
  /* SS_NO_DEVICE		1 */ "NO DEVICES AVAILABLE",
  /* SS_TIME_WRONG		2 */ "WRONG TIME TO CALL",
  /* SS_INPROGRESS		3 */ "TALKING",
  /* SS_CONVERSATION		4 */ "CONVERSATION FAILED",
  /* SS_SEQBAD			5 */ "BAD SEQUENCE CHECK",
  /* SS_LOGIN_FAILED		6 */ "LOGIN FAILED",
  /* SS_DIAL_FAILED		7 */ "DIAL FAILED",
  /* SS_BAD_LOG_MCH		8 */ "BAD LOGIN/MACHINE COMBINATION",
  /* SS_LOCKED_DEVICE		9 */ "DEVICE LOCKED",
  /* SS_ASSERT_ERROR		10 */ "ASSERT ERROR",
  /* SS_BADSYSTEM		11 */ "SYSTEM NOT IN Systems FILE",
  /* SS_CANT_ACCESS_DEVICE	12 */ "CAN'T ACCESS DEVICE",
  /* SS_DEVICE_FAILED		13 */ "DEVICE FAILED",
  /* SS_WRONG_MCH		14 */ "WRONG MACHINE NAME",
  /* SS_CALLBACK		15 */ "CALLBACK REQUIRED",
  /* SS_RLOCKED			16 */ "REMOTE HAS A LCK FILE FOR ME",
  /* SS_RUNKNOWN		17 */ "REMOTE DOES NOT KNOW ME",
  /* SS_RLOGIN			18 */ "REMOTE REJECT AFTER LOGIN",
  /* SS_UNKNOWN_RESPONSE	19 */ "REMOTE REJECT, UNKNOWN MESSAGE",
  /* SS_STARTUP			20 */ "STARTUP FAILED",
  /* SS_CHAT_FAILED		21 */ "CALLER SCRIPT FAILED",
};
