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
static char rcsid[] = "@(#)$RCSfile: xqt.c,v $ $Revision: 4.3.5.2 $ (DEC) $Date: 1993/09/07 16:12:47 $";
#endif
/* 
 * COMPONENT_NAME: UUCP xqt.c
 * 
 * FUNCTIONS: euucico, xuucico, xuuxqt 
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
xqt.c	1.3  com/cmd/uucp,3.1,9013 10/10/89 14:04:24";
*/

/*	/sccs/src/cmd/uucp/s.xqt.c
	xqt.c	1.1	7/29/85 16:34:28
*/
#include "uucp.h"
/* VERSION( xqt.c	5.2 -  -  ); */

static euucico();

/*
 * start up uucico for rmtname
 * return:
 *	none
 */
void
xuucico(rmtname)
char *rmtname;
{
	/*
	 * start uucico for rmtname system
	 */
	if (fork() == 0) {	/* can't vfork() */
		/*
		 * hide the uid of the initiator of this job so that he
		 * doesn't get notified about things that don't concern him.
		 */
		(void) setuid(geteuid());
		euucico(rmtname);
	}
	return;
}

static
euucico(rmtname)
char	*rmtname;
{
	char opt[100];

	(void) close(0);
	(void) close(1);
	(void) close(2);
	(void) open("/dev/null", 0);
	(void) open("/dev/null", 1);
	(void) open("/dev/null", 1);
	(void) signal(SIGINT, SIG_IGN);
	(void) signal(SIGHUP, SIG_IGN);
	(void) signal(SIGQUIT, SIG_IGN);
	closelog();
	if (rmtname[0] != '\0')
		(void) sprintf(opt, "-s%s", rmtname);
	else
		opt[0] = '\0';
	(void) execle(UUCICO, "UUCICO", "-r1", opt, 0, Env);
	exit(100);
}


/*
 * start up uuxqt
 * return:
 *	none
 */
void
xuuxqt(rmtname)
char	*rmtname;
{
	char	opt[100];

	/*
	 * start uuxqt
	 */
	if (vfork() == 0) {
		(void) close(0);
		(void) close(1);
		(void) close(2);
		(void) open("/dev/null", 2);
		(void) open("/dev/null", 2);
		(void) open("/dev/null", 2);
		(void) signal(SIGINT, SIG_IGN);
		(void) signal(SIGHUP, SIG_IGN);
		(void) signal(SIGQUIT, SIG_IGN);
		closelog();
		/*
		 * hide the uid of the initiator of this job so that he
		 * doesn't get notified about things that don't concern him.
		 */
		(void) setuid(geteuid());
		if (rmtname && rmtname[0] != '\0') {
			(void) sprintf(opt, "-s%s", rmtname);
			(void) execle(UUXQT, "UUXQT", opt, (char *) 0, Env);
		}
		else {
			(void) execle(UUXQT, "UUXQT", (char *) 0, Env);
		}
		(void) _exit(100);
	}
	return;
}
