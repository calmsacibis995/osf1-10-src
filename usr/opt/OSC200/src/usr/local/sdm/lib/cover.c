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
static char	*sccsid = "@(#)$RCSfile: cover.c,v $ $Revision: 4.3.3.2 $ (DEC) $Date: 1992/01/30 09:03:02 $";
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
#include <sys/types.h>
#include <sys/stat.h>
#include <libc.h>
#include <stdio.h>
#include <pwd.h>

main(argc, argv)
int argc;
char **argv;
{
    char *prog;
    char *user;
    char *testdir;
    char *cmd;
    struct passwd *pw;
    struct stat st;

    if (argc == 0)
	prog = "authcover";
    else if ((prog = rindex(argv[0], '/')) == NULL)
	prog = argv[0];
    else
	prog++;

    if (argc < 2)
	quit(1, "usage: %s cmd [ args ]\n", prog);

    if ((user = getenv("AUTHCOVER_USER")) == NULL)
	quit(1, "%s: AUTHCOVER_USER not defined\n", prog);

    if ((testdir = getenv("AUTHCOVER_TESTDIR")) == NULL)
	quit(1, "%s: AUTHCOVER_USER not defined\n", prog);

    if ((pw = getpwnam(user)) == NULL)
	quit(1, "%s: user \"%s\" unknown\n", prog, user);

    if (stat(testdir, &st) < 0 || (st.st_mode&S_IFMT) != S_IFDIR)
	quit(1, "%s: directory \"%s\" not a directory\n", prog, testdir);

    if (strcmp(user, "root") == 0) {
	if (strcmp(argv[1], "release") == 0) {
	    quit(1, "%s: releases currently disabled\n");
	} else
	    quit(1, "%s: command \"%s\" not authorized\n");
    } else if (strcmp(user, "devrcs") == 0) {
	if (strcmp(argv[1], "makepath") == 0) {
	    cmd = "/net/skippy/u3/dev/latest/tools/pmax/bin/makepath";
	} else if (strcmp(argv[1], "rcs") == 0) {
	    cmd = "/net/skippy/u3/dev/latest/tools/pmax/bin/rcs";
	} else if (strcmp(argv[1], "rcsci") == 0) {
	    cmd = "/net/skippy/u3/dev/latest/tools/pmax/bin/rcsci";
	} else if (strcmp(argv[1], "rcsco") == 0) {
	    cmd = "/net/skippy/u3/dev/latest/tools/pmax/bin/rcsco";
	} else
	    quit(1, "%s: command \"%s\" not authorized\n");
    } else if (strcmp(user, "devsrc") == 0) {
	if (strcmp(argv[1], "rcsco") == 0) {
	    cmd = "/net/skippy/u3/dev/latest/tools/pmax/bin/rcsco";
	} else
	    quit(1, "%s: command \"%s\" not authorized\n");
    } else if (strcmp(user, "devobj") == 0) {
	if (strcmp(argv[1], "make") == 0) {
	    cmd = "/net/skippy/u3/dev/latest/tools/pmax/bin/make";
	} else
	    quit(1, "%s: command \"%s\" not authorized\n");
    } else
	quit(1, "%s: user \"%s\" not known to authcover\n", prog, user);

    (void) setgid(pw->pw_gid);
    (void) setuid(pw->pw_uid);
    (void) execv(cmd, &argv[1]);
    quit(1, "%s: execv \"%s\" failed\n", prog, cmd);
}
