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
static char rcsid[] = "@(#)$RCSfile: nohup.c,v $ $Revision: 4.2.5.3 $ (DEC) $Date: 1993/10/11 17:37:18 $";
#endif
/*
 * HISTORY
 */
/*
 * OSF/1 1.2
 */
/*
 * COMPONENT_NAME: (CMDCNTL) system control commands
 *
 * FUNCTIONS:
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright 1976, Bell Telephone Laboratories, Inc.
 *
 * nohup.c	1.7  com/cmd/cntl,3.1,9013 9/10/89 06:22:19
 */

/*
 * Nohup runs commands, ignoring all hangups and quit signals.  If
 * no output is specified, output is redirected to nohup.out.
 */                                                                   

#include <stdio.h>
#include <nl_types.h>
#include "nohup_msg.h"
nl_catd catd;
#define MSGSTR(n,s)	catgets(catd,MS_NOHUP,n,s)

#include <locale.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/mode.h>
#include <errno.h>


char	nout[PATH_MAX+1] = "nohup.out";
extern	int	errno;

main(argc, argv)
char **argv;
{
	char	*home;
	int	err;

	(void) setlocale (LC_ALL,"");
	catd = catopen(MF_NOHUP,NL_CAT_LOCALE);

	/* Skip any argument separator */
	if(argc > 1 && !strcmp(argv[1], "--")) {
		argc--;
		argv[1] = argv[0];
		argv++;
	}
	if(argc < 2) {
		fprintf(stderr,MSGSTR(USAGE,"usage: nohup command arg ...\n")); 
		exit(127);
	}
	argv[argc] = 0;
	signal(SIGHUP, SIG_IGN);
	if(isatty(1)) {
		if(freopen(nout, "a", stdout) == NULL) {
			if((home=getenv("HOME")) == NULL) {
				fprintf(stderr,MSGSTR(NOCREAT,"nohup: cannot open/create nohup.out\n"));
				exit(127);
			}
			strcpy(nout,home);
			strcat(nout,"/nohup.out");
			if(freopen(nout, "a", stdout) == NULL) {
				fprintf(stderr,MSGSTR(NOCREAT,"nohup: cannot open/create nohup.out\n"));
				exit(127);
			}
		}
		chmod(nout, S_IRUSR | S_IWUSR);		/* POSIX */
		fprintf(stderr,MSGSTR(SENDOUT,"Sending output to %s\n"), nout);
	}
	if(isatty(2)) {
		close(2);
		dup(1);
	}
	execvp(argv[1], &argv[1]);
	err = errno;

	/* It failed, so print an error */
	if(freopen("/dev/tty", "w", stderr))
		fprintf(stderr,"%s: %s: %s\n", argv[0], argv[1], strerror(err));

	if ( err == ENOENT )
		exit(127);
	else
		exit(126);
}
