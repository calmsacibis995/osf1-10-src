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
static char	*sccsid = "@(#)$RCSfile: temp.c,v $ $Revision: 4.2.4.3 $ (DEC) $Date: 1992/09/29 12:19:59 $";
#endif 
/*
 */
/*
 * (c) Copyright 1990, 1991, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0.1
 */
#if !defined(lint) && !defined(_NOIDENT)

#endif
/* 
 * COMPONENT_NAME: CMDMAILX temp.c
 * 
 * FUNCTIONS: MSGSTR, tinit 
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
 *	temp.c       5.2 (Berkeley) 6/21/85
 */

#include "rcv.h"

#include "Mail_msg.h" 
extern nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_MAILX,n,s) 

/*
 * Mail -- a mail program
 *
 * Give names to all the temporary files that we will need.
 */

#define FILE_LEN (PATH_MAX)  /* long enough to hold "gettmpdir()/Rs" + pid */
char	tempMail[FILE_LEN];
char	tempQuit[FILE_LEN];
char	tempEdit[FILE_LEN];
char	tempSet[FILE_LEN];
char	tempResid[FILE_LEN];
char	tempMesg[FILE_LEN];

tinit()
{
	register char *cp, *cp2;
	char uname[PATHSIZE];
	char flatuname[PATHSIZE];
	register int err = 0;
	register pid_t pid;

	pid = getpid();
	sprintf(tempMail, "%s/Rs%05d", gettmpdir(), pid);
	sprintf(tempResid, "%s/Rq%05d", gettmpdir(), pid);
	sprintf(tempQuit, "%s/Rm%05d", gettmpdir(), pid);
	sprintf(tempEdit, "%s/Re%05d", gettmpdir(), pid);
	sprintf(tempSet, "%s/Rx%05d", gettmpdir(), pid);
	sprintf(tempMesg, "%s/Rx%05d", gettmpdir(), pid);

	if (strlen(myname) != 0) {
		uid = getuserid(myname);
		if (uid == -1) {
			printf(MSGSTR(NOUSER, "\"%s\" is not a user of this system\n"), myname); /*MSG*/
			exit(1);
		}
	}
	else {
		uid = getuid() & UIDMASK;
		if (username(uid, uname) < 0) {
			copy("ubluit", myname);
			err++;
			if (rcvmode) {
				printf(MSGSTR(WHOAREYOU, "Who are you!?\n")); /*MSG*/
				exit(1);
			}
		}
		else
			copy(uname, myname);
	}
	cp = value("HOME");
	if (cp == NULLSTR)
		cp = ".";
	copy(cp, homedir);
	cp = copy(homedir, mailrc);
	copy("/.mailrc", cp);
	assign("MBOX", Getf("MBOX"));	/* for SVID-2 */
	copy(Getf("MBOX"), mbox);	/* SVID-2, reduces other code changes */
	assign("DEAD", Getf("DEAD"));	/* for SVID-2 */
	copy(Getf("DEAD"), deadletter);	/* SVID-2, reduces other code changes */
	assign("save", "");		/* for SVID-2 */
	assign("asksub", "");		/* for SVID-2 */
	assign("header", "");		/* for SVID-2 */
}

/*
 * get $TMPDIR if it exists, otherwise "/tmp"
 */

char *
gettmpdir()
{
	char *tmpdir;

	return((tmpdir = value("TMPDIR")) ? tmpdir : "/tmp");
}
