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
static char rcsid[] = "@(#)$RCSfile: atq.c,v $ $Revision: 4.2.5.3 $ (DEC) $Date: 1993/10/11 15:35:57 $";
#endif
/*
 * HISTORY
 */
/*
 * OSF/1 1.2
 */
/*
 * COMPONENT_NAME: (CMDOPER) commands needed for basic system needs
 *
 * FUNCTIONS: atq
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
 * 1.15  com/cmd/cntl/cron/atq.c, cmdcntl, bos320, 9125320 6/7/91 09:23:13
 * atq.c	4.2 09:45:35 7/12/90 SecureWare 
 */
#include <sys/secdefines.h>
#if SEC_BASE
#include <sys/security.h>
#endif

#include <stdio.h>
#include <unistd.h>
#include <locale.h>
#include <pwd.h>
#include "cron.h"

/*
 * NAME: atq
 *                                                                    
 * FUNCTION: command line processing to call the correct atq functions
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 * 	This function will compute the command line arguments and
 *	call the correct "at" function based on the user flags.
 *                                                                   
 */  

#include "cron_msg.h"
#define MSGSTR(Num,Str) catgets(catd,MS_CRON,Num,Str)
nl_catd catd;

#define INVALIDUSER	"you are not a valid user (no entry in /etc/passwd)"
#define CANTPEEK	"you may only look at your own jobs.\n"
#define BADQUE          "bad queue specification"

main(argc,argv)
int argc;
char *argv[];
{
	register int 	i;		/* loop variable */
	register int 	flag=0;		/* boolean indicator */
	char 		argbuf[BUFSIZ];	/* argument buffer */
	uid_t 		uid;		/* real userid of caller */
	struct passwd 	*nptr;		/* passwd struct of caller */
	char 		*user;		/* username of caller */
	short		cflag=0;	/* creation order */
	short		nflag=0;	/* number of jobs */
	char		queue='\0';	/* queue for -q option */
	int		c;		/* getopt option */
	extern char 	*optarg;
	extern int	optind;

	(void ) setlocale(LC_ALL,"");

	catd = catopen(MF_CRON,NL_CAT_LOCALE);

#if SEC_BASE
	set_auth_parameters(argc, argv);
	initprivs();

#if SEC_MAC
	disablepriv(SEC_MULTILEVELDIR);
#endif

	if ((nptr = getpwuid(uid = getluid())) == NULL)
#else /* !SEC_BASE */
	if ((nptr=getpwuid(uid=getuid())) == NULL)
#endif /* !SEC_BASE */
	{
		fprintf(stderr,"atq: %s\n",MSGSTR(MS_INVALIDUSER, INVALIDUSER));
		exit(1);
	}
	else 
		user = nptr->pw_name;


	/* 
	 * check options
	 */

	while ((c = getopt(argc, argv, "cq:n?")) != -1) {
		switch (c) {
		   case 'c':		/* list in order of submission */
			cflag=1;
			break;

		   case 'n':		/* count total number of at jobs */
			nflag=1;
			break;

		   case 'q':
			queue = *optarg;
			if (queue < 'a' || queue > 'f')
				fprintf(stderr,"atq: %s\n", MSGSTR(MS_BADQUE, BADQUE));
			break;

		   case '?':
		   default:
			usage();
		}
	}
	if (cflag && nflag)
		usage();

	/*
	 * no usernames are listed on the command line
	 */
	if (optind == argc) {
		if (cflag)
			list_aj(CRON_SORT_E, user,queue);	
		else if (nflag)
			list_aj(CRON_COUNT_JOBS, user,queue);
		else/* list in execution order */
#if SEC_BASE
		list_aj(CRON_SORT_M, at_authorized() ? NULL : user,queue);
#else
		list_aj(CRON_SORT_M,(uid == ROOT)? NULL : user,queue);
#endif
	} else {

	/*
	 * usernames are listed on command line
	 * nobody but root is allowed to see anyone's but their own
	 */
		int listed = 0;

		for (; optind < argc; optind++) {
#if SEC_BASE
			if (strcmp(argv[optind], user) && !at_authorized())
#else
			if (strcmp(argv[optind], user) && (uid != ROOT))
#endif
				continue;
			++listed;
			if (cflag)
				list_aj(CRON_SORT_E, argv[optind],queue);
			else if (nflag)
				list_aj(CRON_COUNT_JOBS,argv[optind],queue);	
			else
				list_aj(CRON_SORT_M, argv[optind],queue);
		}
		if (listed == 0) {
			fprintf(stderr, "atq: %s\n",
				MSGSTR(MS_CANTPEEK, CANTPEEK));
			exit(1);
		}
	}
	exit(0);
}

usage()
{
	fprintf(stderr, MSGSTR(MS_ATQUSAGE, 
	"Usage: atq [-c|-n] [-q{a|b|e|f}] [username...]\n"));
	exit(1);
}
