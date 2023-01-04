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
static char	*sccsid = "@(#)$RCSfile: logout.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 04:21:50 $";
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

#include <sys/types.h>
#include <sys/file.h>
#include <sys/time.h>
#include <utmp.h>
#include <stdio.h>

extern struct utmp *getutline(struct utmp *);
extern struct utmp *pututline(struct utmp *);

/* 0 on failure, 1 on success */

logout(line)
	register char *line;
{
	register FILE *fp;
	struct utmp ut;
	struct utmp *utptr;
	int rval;
	time_t time();

	rval = 0;
	setutent();
	strncpy(ut.ut_line, line, sizeof(ut.ut_line));
	if ((utptr = getutline(&ut)) != NULL) {
		utptr->ut_type = DEAD_PROCESS;
		bzero(utptr->ut_user, sizeof(utptr->ut_user));
                bzero(utptr->ut_host, sizeof(utptr->ut_host));
		(void) time(&utptr->ut_time);
		if (pututline(utptr) != NULL) rval = 1;
	}
	endutent();
	return(rval);
}
