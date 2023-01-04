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
static char *rcsid = "@(#)$RCSfile: getpass.c,v $ $Revision: 4.2.3.2 $ (DEC) $Date: 1992/01/29 14:29:48 $";
#endif
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0
 */
/*
 * COMPONENT_NAME: (LIBCIO) Standard C Library I/O Functions 
 *
 * FUNCTIONS: getpass 
 *
 * ORIGINS: 3, 27 
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 * OBJECT CODE ONLY SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989 
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * getpass.c	1.9  com/lib/c/io,3.1,8943 9/12/89 18:23:20
 */

/*LINTLIBRARY*/
#include <stdio.h>
#include <sys/limits.h>
#include <sys/types.h>
#include <signal.h>
#include <unistd.h>
#include <termios.h>

static int intrupt;
static int catch(void);

char *
getpass(prompt)
char	*prompt;
{
	struct termios ttyb;
	char     savel;
	tcflag_t flags;
	register char *p;
	register int c;
	FILE	*fi;
	static char pbuf[PASS_MAX+1];
	/* static int catch(); */
	sig_t	sig;

	if((fi = fopen("/dev/tty", "r+")) == NULL)
		fi = stdin;
	setbuf(fi, (char*)NULL);
	sig = signal(SIGINT, (void (*)(int))catch);
	intrupt = 0;
	(void) tcgetattr(fileno(fi), &ttyb);
	flags = ttyb.c_lflag;
	ttyb.c_lflag &= ~(ECHO | ECHOE | ECHOK | ECHONL);
	(void) tcsetattr(fileno(fi), TCSAFLUSH, &ttyb);
	(void) fputs(prompt, stderr);

	for(p=pbuf; !intrupt && (c = getc(fi)) != '\n' && c != EOF; ) {
		if(p < &pbuf[PASS_MAX])
			*p++ = c;
	}
	*p = '\0';
	(void) putc('\n', stderr);
	ttyb.c_lflag = flags;
	(void) tcsetattr(fileno(fi), TCSADRAIN, &ttyb);
	(void) signal(SIGINT, (void (*)(int))sig);
	if(fi != stdin)
		(void) fclose(fi);
	if(intrupt)
		(void) kill(getpid(), SIGINT);
	return(pbuf);
}

static int
catch()
{
	++intrupt;
}
