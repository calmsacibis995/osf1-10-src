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
static char	*sccsid = "@(#)$RCSfile: getlesson.c,v $ $Revision: 4.2.3.2 $ (DEC) $Date: 1992/04/10 15:56:42 $";
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

/*
 * COMPONENT_NAME: (CMDMAN) commands that allow users to read online
 * documentation
 *
 * FUNCTIONS: getlesson
 *
 * ORIGINS: 26, 27 
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 * OBJECT CODE ONLY SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989 
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 * getlesson.c	1.3  com/cmd/man/learn,3.1,9021 9/14/89 06:31:25
 */

#include "stdio.h"
#include <sys/limits.h>
#include "lrnref.h"

#include "learn_msg.h" 
extern nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_LEARN,n,s) 

char *
getlesson()
{
	char *p;
	char ans[MAX_INPUT], line[MAX_LEN];
	int isnum, found, fd[2];
	FILE *fp;

	sprintf(ans, "%s/%s/L%s", dname, sname, level);
	if (access(ans, 04) == 0)		/* there is a file */
		return(level);
	isnum = 1;
	for (p=level; *p; p++)		/* accept:  (digit|dot)*anychar  */
		if (*p != '.' && (*p < '0' || *p > '9') && *(p+1) != '\0')
			isnum = 0;
	if (isnum) {
		strcpy(line, level);
		p = level;
		while (*p != '.' && *p >= '0' && *p <= '9')
			p++;
		*p = '\0';
		strcat(level, ".1a");
		sprintf(ans, "%s/%s/L%s", dname, sname, level);
		if (access(ans, 04) == 0) {	/* there is a file */
			printf(MSGSTR(LNOLESSON, "There is no lesson %s; trying lesson %s instead.\n\n"), line, level); /*MSG*/
			return(level);
		}
		printf(MSGSTR(LTHEREISNOLESSON, "There is no lesson %s.\n"), line); /*MSG*/
		return(0);
	}
	/* fgrep through lessons for one containing the string in 'level' */
	pipe(fd);
	if (fork() == 0) {
		close(fd[0]);
		dup2(fd[1], 1);
		sprintf(ans,"cd %s/%s ; fgrep '%s' L?.* L??.* L???.*", dname, sname, level);
		execl("/usr/bin/sh", "sh", "-c", ans, (char *) 0);
		perror("/usr/bin/sh");
		fprintf(stderr, MSGSTR(LGETLESSONCANT, "Getlesson:  can't do %s\n"), ans); /*MSG*/
	}
	close(fd[1]);
	fp = fdopen(fd[0], "r");
	found = 0;
	while (fgets(line, MAX_LEN, fp) != NULL) {
		for (p=line; *p != ':'; p++) ;
		p++;
		if (*p == '#')
			continue;
		else {
			found = 1;
			break;
		}
	}
	/*fclose(fp);*/
	if (found) {
		backstep(line,&p);
		*p = '\0';
		strcpy(level, &line[1]);
		sprintf(ans, "%s/%s/L%s", dname, sname, level);
		if (access(ans, 04) == 0) {	/* there is a file */
			printf(MSGSTR(LTRYINGLESSON, "Trying lesson %s.\n\n"), level); /*MSG*/
			return(level);
		}
	}
	printf(MSGSTR(LNOLESSONCONTAIN, "There is no lesson containing \"%s\".\n"), level); /*MSG*/
	return(0);
}
