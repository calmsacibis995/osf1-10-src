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
static char rcsid[] = "@(#)$RCSfile: time.c,v $ $Revision: 4.2.7.4 $ (DEC) $Date: 1993/10/11 19:20:36 $";
#endif
/*
 * HISTORY
 */
/*
 * OSF/1 1.2
 */
/*
 * COMPONENT_NAME: (CMDSTAT) status
 *
 * FUNCTIONS:
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1991
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 * Copyright 1976, Bell Telephone Laboratories, Inc.
 * 
 * 1.13  com/cmd/stat/time.c, cmdstat, bos320, 9125320 6/7/91 15:47:45
 */

#include	<stdio.h>
#include	<stdlib.h>
#include	<signal.h>
#include	<errno.h>
#include	<locale.h>
#include	<sys/wait.h>
#include	<sys/times.h>		/* times() / clock_t / struct tms */
#include	<unistd.h>		/* sysconf() */

#include "time_msg.h"
nl_catd	catd;
#define MSGSTR(n,s)	catgets(catd,MS_TIME,n,s)


static void	printt( char* str, clock_t ticks );
char errbuffer[BUFSIZ];

void static
usage(void)
{
    fprintf(stderr,MSGSTR(USAGE,"usage: time [-p] command\n"));
}
main(argc, argv)
int argc;
char **argv;
{
	struct tms	buffer;
	register int p;
	int	status;
	clock_t	before, after;
	int	optlet;		/* an option letter found in getopt() */

	(void) setlocale (LC_ALL,"");
	catd = catopen(MF_TIME,NL_CAT_LOCALE);
	setbuf(stderr,errbuffer);

	if(argc<=1) {
	    usage();
	    exit(1);
	}

	while ( (optlet = getopt(argc,argv,"p")) != EOF ) {
		switch ( optlet ) {
			case 'p' :
				/* "POSIX" format, which is same as default */
				--argc;
				++argv;
				break;

			default :
				break;
		}
	}

	/* to cater the case of time -p without specifying
	   the utility
	*/
	if(argc<=1) {
	    usage();
	    exit(1);
	}

	before = times(&buffer);
	p = fork();
	if(p == -1) {
		fprintf(stderr,MSGSTR(NOFORK,"time: cannot fork -- try again.\n"));
		exit(2);
	}
	if(p == 0) {
		execvp(argv[1], &argv[1]);
		if (errno == ENOENT) {
			/* Can't find the file. */
		        fprintf(stderr, "%s: %s\n", strerror(errno), argv[1]);
			exit(127);
		}
		/* Found, but can't execute, the file. */
	        fprintf(stderr, "%s: %s\n", strerror(errno), argv[1]);
		exit(126);
	}
	signal(SIGINT, SIG_IGN);
	signal(SIGQUIT, SIG_IGN);
	while(wait(&status) != p)
		;
	after = times(&buffer);

	if (!WIFEXITED(status))
		fprintf(stderr,MSGSTR(ABTERM, 
				"time: command terminated abnormally.\n"));
	fprintf(stderr,"\n");
	printt(MSGSTR(REAL,"real  "), (after-before));
	printt(MSGSTR(USER,"user  "), buffer.tms_cutime);
	printt(MSGSTR(SYS, "sys   "), buffer.tms_cstime);

	exit(WEXITSTATUS(status));
}

/*
 *  NAME:  printt
 *
 *  FUNCTION:	prints out the string followed by a number.
 *
 *  RETURN VALUE:  	 void
 */

#define	PREC	1	/* number of digits following the radix character */

static void 
printt( char* str, clock_t ticks )
{
	float	seconds = (float)ticks / sysconf( _SC_CLK_TCK );

	fprintf( stderr, "%s %.*f\n", str, PREC, seconds );
}
