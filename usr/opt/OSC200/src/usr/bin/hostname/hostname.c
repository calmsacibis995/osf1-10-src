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
static char	*sccsid = "@(#)$RCSfile: hostname.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 00:36:07 $";
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
 * COMPONENT_NAME: TCPIP hostname.c
 * 
 * FUNCTIONS: Mhostname, printit, usage 
 *
 * ORIGINS: 10  26  27 
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1983 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 *	hostname.c	5.1 (Berkeley) 4/30/85
 */

/*
 * hostname -- get (or set hostname)
 */
#include <sys/secdefines.h>
#if SEC_BASE
#include <sys/security.h>
#include <prot.h>

extern priv_t *privvec();
#endif
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/syslog.h>

#include "hostname_msg.h" 
#define MSGSTR(n,s) NLgetamsg(MF_HOSTNAME,MS_HOSTNAME,n,s) 

#include <locale.h>

#define MAXDNAME	256  	/* maximum domain name	*/
static int usage();

char *cmd_name;

main(argc,argv)
int  argc;
char *argv[];

{

extern int optind;
int ch, sflag = 0;
char *p, hostname[MAXDNAME];

	setlocale(LC_ALL,"");

#if SEC_BASE
	set_auth_parameters(argc, argv);
	initprivs();
#endif
	cmd_name = argv[0];
	if (argc > 3)
		usage();

	while ((ch = getopt(argc, argv, "s")) != EOF)
		switch((char)ch) {
			case 's':
				sflag = 1;
				break;
			case '?':
			default:
				usage();
		}
	argv += optind;

	if (*argv) {  /* set it */
#if SEC_BASE
		privvec_t saveprivs;
		int status;

		if (!authorized_user("sysadmin")) {
			fprintf(stderr, "%s: need sysadmin authorization\n",
				command_name);
			exit(1);
		}
		if (forceprivs(privvec(SEC_SYSATTR, -1), saveprivs)) {
			fprintf(stderr, "%s: insufficient privileges\n",
				command_name);
			exit(1);
		}
		disablepriv(SEC_SUSPEND_AUDIT);
		status = sethostname(*argv, strlen(*argv));
		seteffprivs(saveprivs, (priv_t *) 0);
		if (status < 0) {
			perror(MSGSTR(SETHOST,"sethostname"));
			exit(1);
		}
#else /* !SEC_BASE */
		if (sethostname(*argv,strlen(*argv))) {  /* error */
			perror(MSGSTR(SETHOST,"sethostname"));
			exit(1);
		}
#endif /* !SEC_BASE */
	}

	/* print it in either case */

	if(gethostname(hostname,sizeof(hostname))){
		perror(MSGSTR(GETHOSTNAME, "gethostname"));
		exit(1);
	}
	if(sflag && (p = index(hostname, '.')))  /* trim domain name */
		*p = '\0';
	puts(hostname);

	exit(0);
}

static usage()
{
        printf(MSGSTR(USAGE, "usage: %s [-s] [hostname]\n"), cmd_name ); /*MSG*/
	exit(1);
}
