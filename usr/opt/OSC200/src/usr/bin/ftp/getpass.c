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
static char	*sccsid = "@(#)$RCSfile: getpass.c,v $ $Revision: 4.2.3.2 $ (DEC) $Date: 1993/07/22 17:00:41 $";
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
/*
 * Copyright (c) 1985 Regents of the University of California.
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
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */
#include <stdio.h>
#include <sys/limits.h>
#include <sys/types.h>
#include <signal.h>
#include <unistd.h>
#include <termios.h>

#define  PASS_MAX	80

static int intrupt;
static int catch(void);

/* This routine is a copy of libc getpass and has been made local to ftp. */
/* ftp needs the password  buffer to be more than 16 characters that libc */
/* getpass allows.  When ever there is a correction/change to libc getpass*/
/* we should make sure that it is done here too.		          */

char *
mygetpass(const char *prompt)
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
		return((char*)NULL);
	else
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
