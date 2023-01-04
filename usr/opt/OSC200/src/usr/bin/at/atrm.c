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
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
#if !defined(lint) && !defined(_NOIDENT)
static char rcsid[] = "@(#)$RCSfile: atrm.c,v $ $Revision: 4.2.7.3 $ (DEC) $Date: 1993/10/11 15:36:00 $";
#endif
/*
 * HISTORY
 */
/*
 * COMPONENT_NAME: (CMDCNTL) 
 *
 * FUNCTIONS: atrm
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1983 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 * 
 * 1.12  com/cmd/cntl/cron/atrm.c, cmdcntl, bos320, 9125320 6/7/91 09:26:38
 * atrm.c	4.2 13:36:06 7/11/90 SecureWare 
 */

#include <sys/secdefines.h>

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <locale.h>
#include <pwd.h>
#include <sys/wait.h>
#include "cron.h"

/*
 * NAME: atrm
 *                                                                    
 * FUNCTION: command line processing to call the correct atrm functions
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 * 	This function will compute the command line arguments and
 *	call the correct "at" function based on the user flags.
 *	The correct at command flag arguments are added and then 
 *	at is called.
 *                                                                   
 * RETURNS: nothing
 */  

#include "cron_msg.h"
#define MSGSTR(Num,Str) catgets(catd,MS_CRON,Num,Str)
nl_catd catd;

#define ATRMDEL "atrm: only jobs belonging to user: %s may be deleted\n"

main(argc,argv)
int argc;
char *argv[];
{
	char	argbuf[BUFSIZ];
	int	allflag = 0;
	int	iflag = 0;
	int	fflag = 0;
	uid_t	user;
	struct passwd *pwinfo;	/* struct for pwd name info */
	int	rec;		/* return value from system("at -r") */
	int	c,n;		/* option scratch variables */

	(void ) setlocale(LC_ALL,"");
	catd = catopen(MF_CRON,NL_CAT_LOCALE);

#if SEC_BASE
	set_auth_parameters(argc, argv);
	initprivs();
#endif
	/* 
	 * If job number, user name, or "-" is not specified, just print
	 * usage info and exit.
	 */
	if (argc < 2)
		usage ();

	/*
	 * Since getopt() is going to try to interpret "-" as a flag indicating
	 * that there are no more options, we pre-read the arglist and change
	 * "-" to it's equivalent "-a"
	 */
	for (n=0; n<argc; n++)
	    if ( !strcmp(argv[n], "-") )
		argv[n] = "-a";

	while ((c = getopt(argc, argv, "afi")) != -1) {
		switch (c) {
			case 'a':
				allflag++;
				break;
			case 'f':
				++fflag;
				iflag=0;
				break;
			case 'i':
				++iflag;
				fflag=0;
				break;
			default:
				usage ();
		}
	}

	argc -= optind;
	argv += optind;

	/*
	 * If all jobs are to be removed and extra command line arguments
	 * are given, print usage info and exit.
	 */
	if (allflag && argc)
		usage();

	/*
	 * If only certain jobs are to be removed and no job #'s or user
	 * names are specified, print usage info and exit.
	 */
	if (!allflag && !argc)
		usage();

	/*
	 * Move to spooling area and get user name of person requesting 
 	 * removal.
	 */
#if SEC_BASE
	user = getluid();
#else
	user = getuid();
#endif
	pwinfo = getpwuid(user);

#if SEC_BASE
	if (at_authorized()) {
#else
	if (!user) { /* super user */
#endif
		if (allflag) {
			if (iflag)
				remove_aj(CRON_PROMPT, NULL);
			else if (fflag)
				remove_aj(CRON_QUIET, NULL);
			else remove_aj(NULL, NULL);
		}
		else {
			strcpy (argbuf, "/usr/bin/at -r ");
			if (iflag)
				strcat(argbuf, "-i ");
			else if (fflag)
				strcat(argbuf, "-F ");
			for (; argc--; argv++) {
				if ((strchr(*argv, '.')) != NULL) {
					/* job#'s */
					strcat(argbuf, *argv);
					strcat(argbuf, " ");
				}
				else {	/* users name */
					strcat(argbuf,"-u ");
					strcat(argbuf, *argv);
					strcat(argbuf, " ");
				}
			}
			rec = system(argbuf);
			if (WIFSIGNALED(rec))
				exit(WTERMSIG(rec));	/* return signal */
			else
				exit(WEXITSTATUS(rec));	/* return exit status */
		}
	}
	else {	/* normal user */
		strcpy (argbuf, "/usr/bin/at -r ");
		if (iflag)
			strcat(argbuf, "-i ");
		else if (fflag)
			strcat(argbuf, "-F ");
		if (allflag) {	/* remove only users jobs */
			strcat(argbuf,"-u ");
			strcat(argbuf, pwinfo->pw_name);
		}
		for (; argc--; argv++) {
			if ((strchr(*argv, '.')) != NULL) { /* job#'s */
				strcat(argbuf, *argv);
				strcat(argbuf, " ");
			}
			else {	/* users  name */
				strcat(argbuf,"-u ");
				strcat(argbuf, *argv);
				strcat(argbuf, " ");
			}
		}
		rec = system(argbuf);
		if (WIFSIGNALED(rec))
			exit(WTERMSIG(rec));	/* return signal */
		else
			exit(WEXITSTATUS(rec));	/* return exit status */
	}
}

usage()
{
	fprintf(stderr, MSGSTR(MS_ATRMUSAGE, 
	"Usage: atrm [-a] [-f|-i] [job #|username]\n"));
	exit(1);
}

