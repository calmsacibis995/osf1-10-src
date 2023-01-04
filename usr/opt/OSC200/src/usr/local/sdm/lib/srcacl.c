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
static char	*sccsid = "@(#)$RCSfile: srcacl.c,v $ $Revision: 4.3.3.2 $ (DEC) $Date: 1992/01/30 09:03:35 $";
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
 * srcacl - access control list support for src tree commands
 */
#include <sys/param.h>
#include <sys/stat.h>
#include <stdio.h>

extern char *salloc();
extern char *index();
extern char *rindex();

main(argc, argv)
int argc;
char **argv;
{
    char cmd_prefix[MAXPATHLEN];
    char buf[MAXPATHLEN];
    char command[MAXPATHLEN];
    char *prog;
    char **av = argv;

    if (argc < 1)
	quit(1, "usage: <cmd> <args>\n");
    prog = argv[0];
    if (rindex(argv[0], '/') != NULL)
	quit(1, "%s command contains a '/' character\n", prog);
    argc--;
    argv++;
    if (strcmp(prog, "exists") == 0)
	exists_main(argc, argv);
    if (strcmp(prog, "rm") == 0) {
	rm_main(argc, argv);
	exit(runv("/bin/rm", av));
    } else if (strcmp(prog, "cp") == 0) {
	cp_main(argc, argv);
	exit(runv("/bin/cp", av));
    }
    else
	quit(1, "%s: command not found\n", prog);
}

exists_main(argc, argv)
int argc;
char **argv;
{
    struct stat st;

    if (argc <= 0)
	quit(1, "exists: missing rcs file argument\n");
    if (argc > 1)
	quit(1, "exists: additional arguments to command\n");
    exit(stat(*argv, &st) < 0 || (st.st_mode&S_IFMT) != S_IFREG);
}

rm_main(argc, argv)
int argc;
char **argv;
{
    char topwd[MAXPATHLEN];
    char curwd[MAXPATHLEN];
    char filbuf[MAXPATHLEN];
    char dirbuf[MAXPATHLEN];
    int quiet = 0;
    int i, j, k;
    char *p, *q;

    for (; argc > 0 && **argv == '-'; argc--, argv++) {
	p = *argv;
	if (*(p+1) == 'f' && *(p+2) == '\0') {
	    quiet++;
	    continue;
	}
	quit(1, "rm: unsupported option: %s\n", p);
    }
    if (argc <= 0)
	quit(1, "rm: missing source file argument\n");
    if (argc > 1)
	quit(1, "rm: additional arguments to command\n");
    p = salloc(*argv);
    if (p == NULL)
	quit(1, "salloc: %s\n", errmsg(-1));
    path(p, dirbuf, filbuf);
    if (getwd(topwd) == NULL)
	quit(1, "%s\n", topwd);
    if (chdir(dirbuf) < 0) {
	if (quiet)
	    exit(0);
	quit(1, "chdir: %s\n", errmsg(-1));
    }
    if (getwd(curwd) == NULL)
	quit(1, "%s\n", curwd);
    j = strlen(topwd);
    k = strlen(curwd);
    if (k < j || bcmp(curwd, topwd, j) != 0 ||
	(k > j && curwd[j] != '/'))
	quit(1, "argument not in correct directory tree\n");
    if (chdir(topwd) < 0)
	quit(1, "chdir: %s\n", errmsg(-1));
}

cp_main(argc, argv)
int argc;
char **argv;
{
    char topwd[MAXPATHLEN];
    char curwd[MAXPATHLEN];
    char filbuf[MAXPATHLEN];
    char dirbuf[MAXPATHLEN];
    char temp_prefix[MAXPATHLEN];
    int needmakepath = 0;
    int preserve = 0;
    struct stat st;
    int i, j, k;
    char *p;

    for (; argc > 0 && **argv == '-'; argc--, argv++) {
	p = *argv;
	if (*(p+1) == 'p' && *(p+2) == '\0') {
	    preserve++;
	    continue;
	}
	quit(1, "cp: unsupported option: %s\n", p);
    }
    if (argc <= 0)
	quit(1, "cp: missing temporary file argument\n");
    get_one_line(".temp_prefix", temp_prefix);
    if (strncmp(*argv, temp_prefix, strlen(temp_prefix)) != 0)
	quit(1, "cp: invalid temporary file\n");
    if (stat(*argv, &st) < 0)
	quit(1, "cp: temporary file missing\n");
    if ((st.st_mode&S_IFMT) != S_IFREG)
	quit(1, "cp: temporary file not regular file\n");
    argc--;
    argv++;
    if (argc <= 0)
	quit(1, "cp: missing source file argument\n");
    if (argc > 1)
	quit(1, "cp: additional arguments to command\n");
    p = salloc(*argv);
    if (p == NULL)
	quit(1, "salloc: %s\n", errmsg(-1));
    path(p, dirbuf, filbuf);
    if (getwd(topwd) == NULL)
	quit(1, "%s\n", topwd);
    if (chdir(dirbuf) < 0) {
	i = 0;
	do {
	    path(dirbuf, dirbuf, filbuf);
	    if (filbuf[0] == '.' &&
		(filbuf[1] == '\0' ||
		 (filbuf[1] == '.' && filbuf[2] == '\0')))
		quit(1, "illegal to have %s in pathname\n", filbuf);
	    if (i++ > 256)
		quit(1, "path/chdir infinite loop\n");
	} while (chdir(dirbuf) < 0);
	needmakepath++;
    }
    if (getwd(curwd) == NULL)
	quit(1, "%s\n", curwd);
    j = strlen(topwd);
    k = strlen(curwd);
    if (k < j || bcmp(curwd, topwd, j) != 0 ||
	(k > j && curwd[j] != '/'))
	quit(1, "argument not in correct directory tree\n");
    if (chdir(topwd) < 0)
	quit(1, "chdir: %s\n", errmsg(-1));
    if (needmakepath && makepath(p, NULL, 1, 1) != 0)
	quit(1, "makepath failed\n");
}

get_one_line(file, line)
char *file, *line;
{
    char buf[MAXPATHLEN];
    FILE *inf;
    char *ptr;

    inf = fopen(file, "r");
    if (inf == NULL)
	quit(1, "fopen: %s\n", errmsg(-1));
    if (fgets(buf, sizeof(buf), inf) == NULL)
	quit(1, "fgets: %s\n", errmsg(-1));
    if (ferror(inf) || fclose(inf) == EOF)
	quit(1, "error reading %s", file);
    if ((ptr = index(buf, '\n')) != NULL)
	*ptr = '\0';
    (void) strcpy(line, buf);
}
