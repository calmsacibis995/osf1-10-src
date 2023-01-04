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
static char	*sccsid = "@(#)$RCSfile: selsub.c,v $ $Revision: 4.2.3.3 $ (DEC) $Date: 1992/11/24 10:16:30 $";
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
 * FUNCTIONS: selsub, chknam, cntlessons
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
 * selsub.c	1.3  com/cmd/man/learn,3.1,9021 11/28/89 13:59:18
 */

#include "stdio.h"
#include <stdlib.h>
#include "sys/types.h"
#include "sys/stat.h"
#include <dirent.h>
#include "lrnref.h"

#include "learn_msg.h" 
extern nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_LEARN,n,s) 

char learnrc[LEN_MAX];

selsub(argc,argv)
char *argv[];
{
	char ans1[LEN_MAX];
	static char ans2[NAME_MAX];
	static char dirname[NAME_MAX];
	static char subname[NAME_MAX];
	FILE *fp;
	char *home;

	if (argc > 1 && argv[1][0] == '-') {
		dname = argv[1]+1;
		argc--;
		argv++;
	}
	if (chdir(dname) != 0) {
		perror(dname);
		fprintf(stderr, MSGSTR(LCANTCD, "Selsub:  couldn't cd to non-standard directory\n")); /*MSG*/
		exit(1);
	}
	sname = argc > 1 ? argv[1] : 0;
	if (argc > 2) {
		strcpy (level=ans2, argv[2]);
		if (strcmp(level, "-") == 0)	/* no lesson name is - */
			ask = 1;
		else if (strcmp(level, "0") == 0)
			level = 0;
		else
			again = 1;	/* treat as if "again" lesson */
	}
	else
		level = 0;
	if (argc > 3 )
		speed = atoi(argv[3]);
	if ((home = getenv("HOME")) != NULL) {
		sprintf(learnrc, "%s/.learnrc", home);
		if ((fp=fopen(learnrc, "r")) != NULL) {
			char xsub[NAME_MAX], xlev[NAME_MAX]; int xsp;
			fscanf(fp, "%s %s %d", xsub, xlev, &xsp);
			fclose(fp);
			if (*xsub && *xlev && xsp >= 0	/* all read OK */
			    && (argc == 2 && strcmp(sname, xsub) == 0
			      || argc <= 1)) {
				strcpy(sname = subname, xsub);
				strcpy(level = ans2, xlev);
				speed = xsp;
				again = 1;
	PRINTF(MSGSTR(LTAKINGUP, "[ Taking up where you left off last time:  learn %s %s.\n"), /*MSG*/
		sname, level);
	PRINTF(MSGSTR(LRMNRENETER, "%s\n  \"rm $HOME/.learnrc\", and re-enter with \"learn %s\". ]\n"), /*MSG*/
		MSGSTR(LTOSTART, "  To start this sequence over leave learn by typing \"bye\", then"), /*MSG*/
		sname);
			}
		}
	}
	if (!sname) {
		printf(MSGSTR(LTHESEAVAIL, "These are the available courses -\n")); /*MSG*/
		list("Linfo");
		printf(MSGSTR(LWANTMOREINFO, "If you want more information about the courses,\n")); /*MSG*/
		printf(MSGSTR(LWANTMOREINFO2, "or if you have never used 'learn' before,\n")); /*MSG*/
		printf(MSGSTR(LWANTMOREINFO3, "press RETURN; otherwise type the name of\n")); /*MSG*/
		printf(MSGSTR(LWANTMOREINFO4, "the course you want, followed by RETURN.\n")); /*MSG*/
		fflush(stdout);
		gets(sname=subname);
		if (sname[0] == '\0') {
			list("Xinfo");
			do {
				printf(MSGSTR(LWHICHSUB, "\nWhich subject?  ")); /*MSG*/
				fflush(stdout);
				gets(sname=subname);
			} while (sname[0] == '\0');
		}
	}
	chknam(sname);
	total = cntlessons(sname);
	if (!level) {
		printf(MSGSTR(LSUBJ1, "If you were in the middle of this subject\n")); /*MSG*/
		printf(MSGSTR(LSUBJ2, "and want to start where you left off, type\n")); /*MSG*/
		printf(MSGSTR(LSUBJ3, "the last lesson number the computer printed.\n")); /*MSG*/
		printf(MSGSTR(LSUBJ4, "If you don't know the number, type in a word\n")); /*MSG*/
		printf(MSGSTR(LSUBJ5, "you think might appear in the lesson you want,\n")); /*MSG*/
		printf(MSGSTR(LSUBJ6, "and I will look for the first lesson containing it.\n")); /*MSG*/
		printf(MSGSTR(LSUBJ7, "To start at the beginning, just hit RETURN.\n")); /*MSG*/
		fflush(stdout);
		gets(ans2);
		if (ans2[0]==0)
			strcpy(ans2,"0");
		else
			again = 1;
		level=ans2;
		getlesson();
	}

	/* make new directory for user to play in */
	if (chdir("/tmp") != 0) {
		perror("/tmp");
		fprintf(stderr, MSGSTR(LCANTCDPUB, "Selsub:  couldn't cd to public directory\n")); /*MSG*/
		exit(1);
	}
	sprintf(dir=dirname, "pl%da", getpid());
	mkdir(dir, (mode_t)0755);
	if (chdir(dir) < 0) {
		perror(dir);
		fprintf(stderr, MSGSTR(LCOUNDNTPLAY, "Selsub:  couldn't make play directory with %s.\nBye.\n"), ans1); /*MSG*/
		exit(1);
	}
	/* after this point, we have a working directory. */
	/* have to call wrapup to clean up */
	sprintf(ans1, "%s/%s/Init", dname, sname);
	if (access(ans1, 04) == 0) {
	        sprintf(ans1, "%s/%s/Init %s", dname, sname, level);
		if (system(ans1) != 0) {
			printf(MSGSTR(LLEAVINGLRN, "Leaving learn.\n")); /*MSG*/
			wrapup(1);
		}
	}
}

chknam(name)
char *name;
{
	if (access(name, 05) < 0) {
		printf(MSGSTR(LNOSUBORLESS, "Sorry, there is no subject or lesson named %s.\nBye.\n"), name); /*MSG*/
		exit(1);
	}
}


cntlessons(sname)	/* return number of entries in lesson directory; */
char *sname;		/* approximate at best since I don't count L0, Init */
{			/* and lessons skipped by good students */

	register struct dirent *ep;	/* directory entry pointer */
	int n = 0;
	DIR *dp;

	if ((dp = opendir(sname)) == NULL) {
		perror(sname);
		wrapup(1);
	}
	for (ep = readdir(dp); ep != NULL; ep = readdir(dp)) {
		if (ep->d_fileno != 0)
			n++;
	}
	closedir(dp);
	return (n - 2);				/* minus . and .. */
}
