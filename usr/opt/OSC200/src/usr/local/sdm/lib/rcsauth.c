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
static char	*sccsid = "@(#)$RCSfile: rcsauth.c,v $ $Revision: 4.3.3.2 $ (DEC) $Date: 1992/01/30 09:03:27 $";
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
 * rcsauth - authentication cover for rcs commands
 */
#include <sys/param.h>
#include <sys/errno.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <sys/signal.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>

#define SEND_OVER	1
#define WANT_BACK	2

main(argc, argv)
int argc;
char **argv;
{
    char *authcover_testdir;
    char *authcover_host;
    char *authcover_user;
    char *prog;
    int tempmode = 0;
    int tempslot = 0;
    int debug;
    char buf[MAXPATHLEN];
    char *u;
    char *rindex(), *getenv();

    if (argc < 1)
	prog = "rcsauth";
    else if ((prog = rindex(argv[0], '/')) == NULL)
	prog = argv[0];
    else
	prog++;
    argc--;
    argv++;
    debug = (getenv("AUTHCOVER_DEBUG") != NULL);
    if (argc >= 2 && argv[0][0] == '-' && argv[0][1] == 't') {
	if (argv[0][2] == '0')
	    tempmode = WANT_BACK;
	else if (argv[0][2] == '1')
	    tempmode = SEND_OVER;
	else if (argv[0][2] == '2')
	    tempmode = SEND_OVER|WANT_BACK;
	else
	    quit(1, "%s: invalid temp mode\n", prog);
	tempslot = atoi(argv[1]);
	if (tempslot < 0)
	    quit(1, "%s: bad tempslot\n", prog);
	argc -= 2;
	argv += 2;
    }
    if (argc < 1)
	quit(1, "usage: %s <cmd> <args>\n", prog);
    if ((authcover_host = getenv("AUTHCOVER_HOST")) == NULL)
	quit(1, "%s: no AUTHCOVER_HOST in environment\n", prog);
    if ((authcover_testdir = getenv("AUTHCOVER_TESTDIR")) == NULL)
	quit(1, "%s: no AUTHCOVER_TESTDIR in environment\n", prog);
    if ((authcover_user = getenv("AUTHCOVER_USER")) == NULL)
	quit(1, "%s: no AUTHCOVER_USER in environment\n", prog);

    exit(kxct(authcover_host, authcover_testdir, authcover_user,
	      tempmode, tempslot, debug, argc, argv));
}
