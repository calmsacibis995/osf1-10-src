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
static char *rcsid = "@(#)$RCSfile: login.c,v $ $Revision: 1.1.9.2 $ (DEC) $Date: 1993/06/07 23:26:06 $";
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
 * Copyright (c) 1988 The Regents of the University of California.
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


#if !defined(lint) && !defined(_NOIDENT)

#endif

/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#if !defined(_THREAD_SAFE)
#pragma weak login = __login
#endif
#endif
#include <sys/types.h>
#include <sys/file.h>
#include <sys/param.h>
#include <utmp.h>
#include <stdio.h>

extern struct utmp *getutent(void);
extern struct utmp *pututline(struct utmp *);

void
login(ut)
	struct utmp *ut;
{
	register int fd;
	int tty,len;
	struct utmp *xut;

	/*
	 * The bsd programs which call this routine will fill in the
	 * bsd elements of the utmp structure: name, line, time and host
	 */
	setutent();
	ut->ut_type = USER_PROCESS;
	ut->ut_pid = getpid();
	ut->ut_exit.e_exit = 0;
	ut->ut_exit.e_termination = 0;

	/*
	 * look for a slot in the utmp file with our pid 
	 * If we find one, when use that id (which is probably from inittab
	 */
	while ((xut = getutent()) != NULL) 
		if (xut->ut_pid == ut->ut_pid) {
		    strncpy(ut->ut_id, xut->ut_id, sizeof(ut->ut_id));
		    break;
		}
	/*
	 * If we couldn't find our pid, then use the last five characters
	 * of the tty name (probably ttyp0, ttyp1, etc)
	 */
	if (xut == NULL) {
		len = MIN(strlen(ut->ut_line),5);
		strncpy(ut->ut_id,ut->ut_line + strlen(ut->ut_line) - len,len);
	}
	(void)pututline(ut);
	endutent();

	if ((fd = open(WTMP_FILE, O_WRONLY|O_APPEND, 0)) >= 0) {
		(void)write(fd, (char *)ut, sizeof(struct utmp));
		(void)close(fd);
	}
}
