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
static char	*sccsid = "@(#)$RCSfile: rcsacl.c,v $ $Revision: 4.3.3.2 $ (DEC) $Date: 1992/01/30 09:03:19 $";
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
 * rcsacl - access control list support for rcs tree commands
 */
#include <sys/param.h>
#include <sys/stat.h>
#include <stdio.h>

extern char *salloc();
extern char *getenv();
extern char *index();
extern char *rindex();

static char *outdate_file = NULL;

main(argc, argv)
int argc;
char **argv;
{
    char cmd_prefix[MAXPATHLEN];
    char buf[MAXPATHLEN];
    char command[MAXPATHLEN];
    char *prog;
    char **av = argv;
    int status;

    if (argc < 1)
	quit(1, "usage: <cmd> <args>\n");
    prog = argv[0];
    if (rindex(argv[0], '/') != NULL)
	quit(1, "%s command contains a '/' character\n", prog);
    argc--;
    argv++;
    if (strcmp(prog, "exists") == 0)
	exists_main(argc, argv);
    if (strcmp(prog, "rcs") == 0)
	rcs_main(argc, argv);
    else if (strcmp(prog, "ci") == 0)
	ci_main(argc, argv);
    else if (strcmp(prog, "co") == 0)
	co_main(argc, argv);
    else if (strcmp(prog, "rcsdiff") == 0)
	rcsdiff_main(argc, argv);
    else
	quit(1, "%s: command not found\n", prog);
    get_one_line(".cmd_prefix", cmd_prefix);
    (void) concat(buf, sizeof(buf),
		  cmd_prefix, ":/bin:/usr/bin:/usr/ucb", NULL);
    (void) setenv("PATH", buf, 1);
    (void) concat(command, sizeof(command), cmd_prefix, "/", prog, NULL);
    status = runv(command, av);
    if (status == 0 && outdate_file != NULL)
	remove_empty_file();
    exit(status);
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

rcs_main(argc, argv)
int argc;
char **argv;
{
    char topwd[MAXPATHLEN];
    char curwd[MAXPATHLEN];
    char filbuf[MAXPATHLEN];
    char dirbuf[MAXPATHLEN];
    char *user;
    int userlen;
    int quiet = 0;
    int init = 0;
    int namesymbol = 0;
    int comment = 0;
    int state = 0;
    int lock = 0;
    int unlock = 0;
    int outdate = 0;
    int finish_outdate = 0;
    int needmakepath = 0;
    int isrange = 0;
    int i, j, k;
    char *p, *q;

    if ((user = getenv("USER")) == NULL)
	quit(1, "rcs: undefined user\n");
    userlen = strlen(user);
    for (; argc > 0 && **argv == '-'; argc--, argv++) {
	p = *argv;
	if (*(p+1) == 'q' && *(p+2) == '\0') {
	    quiet++;
	    continue;
	}
	if (*(p+1) == 'N' || *(p+1) == 'n') {
	    namesymbol++;
	    p += 2;
	    if ((*p < 'A' || *p > 'Z') &&
		(bcmp(p, user, userlen) != 0 || *(p+userlen) != '_'))
		quit(1, "rcs: symbol must begin with your user id\n");
	    while (*p != '\0' && *p != ':')
		p++;
	    if (*p == '\0' && quiet)
		finish_outdate++;
	    continue;
	}
	if (*(p+1) == 'c') {
	    comment++;
	    continue;
	}
	if (*(p+1) == 's') {
	    state++;
	    continue;
	}
	if (strcmp(p, "-i") == 0) {
	    init++;
	    argc--;
	    argv++;
	    if (argc <= 0 || strcmp(*argv, "-t/dev/null") != 0)
		quit(1, "rcs: -i must be followed by -t/dev/null\n");
	    continue;
	}
	if (*(p+1) == 'u') {
	    p += 2;
	    if (*p < '0' || *p > '9')
		quit(1, "must use numeric revision for rcs\n");
	    if (nrevfields(p, 0) <= 2)
		quit(1, "rcs: should not be unlocking the trunk\n");
	    unlock++;
	    continue;
	}
	if (*(p+1) == 'l') {
	    p += 2;
	    if (*p < '0' || *p > '9')
		quit(1, "must use numeric revision for rcs\n");
	    if (nrevfields(p, 0) <= 2)
		quit(1, "rcs: should not be locking the trunk\n");
	    lock++;
	    continue;
	}
	if (*(p+1) == 'o') {
	    p += 2;
	    q = p;
	    while (*q != '\0') {
		if (*q == '-') {
		    *q = '\0';
		    isrange++;
		    break;
		}
		q++;
	    }
	    if (*p == '\0' && !isrange)
		quit(1, "invalid outdate revision specification\n");
	    if (*p != '\0' && nrevfields(p, 0) <= 2)
		quit(1, "rcs: should not outdate the trunk\n");
	    if (isrange) {
		*q++ = '-';
		if (*q == '\0')
		    quit(1, "invalid outdate revision specification\n");
		if (nrevfields(q, 0) <= 2)
		    quit(1, "rcs: should not outdate the trunk\n");
	    }
	    outdate++;
	    continue;
	}
	quit(1, "rcs: unsupported option: %s\n", p);
    }
    if (argc <= 0)
	quit(1, "rcs: missing rcs file argument\n");
    if (argc > 1)
	quit(1, "rcs: additional arguments to command\n");
    if (finish_outdate)
	outdate_file = *argv;
    p = salloc(*argv);
    if (p == NULL)
	quit(1, "salloc: %s\n", errmsg(-1));
    path(p, dirbuf, filbuf);
    j = strlen(filbuf);
    if (filbuf[j-2] != ',' || filbuf[j-1] != 'v')
	quit(1, "argument to rcs must be an rcs file\n");
    if (getwd(topwd) == NULL)
	quit(1, "%s\n", topwd);
    if (chdir(dirbuf) < 0) {
	if (!init)
	    quit(1, "chdir: %s\n", errmsg(-1));
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

ci_main(argc, argv)
int argc;
char **argv;
{
    char topwd[MAXPATHLEN];
    char curwd[MAXPATHLEN];
    char filbuf[MAXPATHLEN];
    char dirbuf[MAXPATHLEN];
    char temp_prefix[MAXPATHLEN];
    char *user;
    int userlen;
    int force = 0;
    int update = 0;
    int msg = 0;
    int lock = 0;
    int unlock = 0;
    int createbranch = 0;
    int revision = 0;
    int state = 0;
    int date = 0;
    struct stat st;
    int j, k;
    char *p;

    if ((user = getenv("USER")) == NULL)
	quit(1, "ci: undefined user\n");
    userlen = strlen(user);
    for (; argc > 0 && **argv == '-'; argc--, argv++) {
	p = *argv;
	if (*(p+1) == 'b' && *(p+2) == '\0') {
	    createbranch++;
	    argc--;
	    argv++;
	    if (argc <= 0)
		quit(1, "ci: -b must be followed by -N<name>\n");
	    p = *argv;
	    if (*p != '-' || *(p+1) != 'N')
		quit(1, "ci: -b must be followed by -N<name>\n");
	    p += 2;
	    if ((*p < 'A' || *p > 'Z') &&
		((bcmp(p, user, userlen) != 0 || *(p+userlen) != '_')))
		quit(1, "ci: symbol must begin with your user id\n");
	    continue;
	}
	if (*(p+1) == 'l' && *(p+2) == '\0') {
	    lock++;
	    continue;
	}
	if (*(p+1) == 'u' && *(p+2) == '\0') {
	    unlock++;
	    continue;
	}
	if (*(p+1) == 'f' && *(p+2) == '\0') {
	    force++;
	    continue;
	}
	if (*(p+1) == 'd') {
	    date++;
	    continue;
	}
	if (*(p+1) == 's') {
	    state++;
	    continue;
	}
	if (*(p+1) == 'U') {
	    p += 2;
	    if (*p < '0' || *p > '9')
		quit(1, "must use numeric revision for check-in\n");
	    if (nrevfields(p, !createbranch) <= (!createbranch ? 1 : 2) &&
		!createbranch)
		quit(1, "ci: should not check-in onto the trunk\n");
	    update++;
	    continue;
	}
	if (*(p+1) == 'r') {
	    p += 2;
	    if (*p < '0' || *p > '9')
		quit(1, "must use numeric revision for check-in\n");
	    if (nrevfields(p, 0) <= 2 &&
		!createbranch && strcmp(p, "1.1") != 0)
		quit(1, "ci: only revision 1.1 is permitted\n");
	    revision++;
	    continue;
	}
	if (*(p+1) == 'm') {
	    msg++;
	    continue;
	}
	quit(1, "ci: unsupported option: %s\n", p);
    }
    if (!revision && !update)
	quit(1, "ci: must specify revision to checkin\n");
    if (argc <= 0)
	quit(1, "ci: missing temporary file argument\n");
    get_one_line(".temp_prefix", temp_prefix);
    if (strncmp(*argv, temp_prefix, strlen(temp_prefix)) != 0)
	quit(1, "ci: invalid temporary file\n");
    if (stat(*argv, &st) < 0)
	quit(1, "ci: temporary file missing\n");
    if ((st.st_mode&S_IFMT) != S_IFREG)
	quit(1, "ci: temporary file not regular file\n");
    if (revision && !createbranch && st.st_size != 0)
	quit(1, "ci: initial revision of file must be empty\n");
    argc--;
    argv++;
    if (argc <= 0)
	quit(1, "ci: missing rcs file argument\n");
    if (argc > 1)
	quit(1, "ci: additional arguments to command\n");
    p = salloc(*argv);
    if (p == NULL)
	quit(1, "salloc: %s\n", errmsg(-1));
    path(p, dirbuf, filbuf);
    j = strlen(filbuf);
    if (filbuf[j-2] != ',' || filbuf[j-1] != 'v')
	quit(1, "argument to check-in must be an rcs file\n");
    if (getwd(topwd) == NULL)
	quit(1, "%s\n", topwd);
    if (chdir(dirbuf) < 0)
	quit(1, "chdir: %s\n", errmsg(-1));
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

co_main(argc, argv)
int argc;
char **argv;
{
    char topwd[MAXPATHLEN];
    char curwd[MAXPATHLEN];
    char filbuf[MAXPATHLEN];
    char dirbuf[MAXPATHLEN];
    char temp_prefix[MAXPATHLEN];
    int istrunk = 0;
    int quiet = 0;
    int print = 0;
    int lock = 0;
    int revision = 0;
    struct stat st;
    int j, k;
    char *p;

    for (; argc > 0 && **argv == '-'; argc--, argv++) {
	p = *argv;
	if (*(p+1) == 'q' && *(p+2) == '\0') {
	    quiet++;
	    continue;
	}
	if (*(p+1) == 'l' && *(p+2) == '\0') {
	    lock++;
	    continue;
	}
	if (*(p+1) == 'p') {
	    p += 2;
	    if (*p < '0' || *p > '9')
		quit(1, "must use numeric revision for check-out\n");
	    if (nrevfields(p, 0) <= 2)
		istrunk++;
	    print++;
	    continue;
	}
	if (*(p+1) == 'r') {
	    p += 2;
	    if (*p < '0' || *p > '9')
		quit(1, "must use numeric revision for check-out\n");
	    if (nrevfields(p, 0) <= 2)
		istrunk++;
	    revision++;
	    continue;
	}
	quit(1, "co: unsupported option: %s\n", p);
    }
    if (!print && !revision)
	quit(1, "co: must specify revision to check-out\n");
    if (istrunk && lock)
	quit(1, "check-out should not lock the trunk\n");
    if (revision) {
	if (argc <= 0)
	    quit(1, "co: missing temporary file argument\n");
	get_one_line(".temp_prefix", temp_prefix);
	if (strncmp(*argv, temp_prefix, strlen(temp_prefix)) != 0)
	    quit(1, "co: invalid temporary file\n");
	if (stat(*argv, &st) == 0)
	    quit(1, "co: temporary file already exists\n");
	argc--;
	argv++;
    }
    if (argc <= 0)
	quit(1, "co: missing rcs file argument\n");
    if (argc > 1)
	quit(1, "co: additional arguments to command\n");
    p = salloc(*argv);
    if (p == NULL)
	quit(1, "salloc: %s\n", errmsg(-1));
    path(p, dirbuf, filbuf);
    j = strlen(filbuf);
    if (filbuf[j-2] != ',' || filbuf[j-1] != 'v')
	quit(1, "argument to check-out must be an rcs file\n");
    if (getwd(topwd) == NULL)
	quit(1, "%s\n", topwd);
    if (chdir(dirbuf) < 0)
	quit(1, "chdir: %s\n", errmsg(-1));
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

rcsdiff_main(argc, argv)
int argc;
char **argv;
{
    /* nothing worth checking... */
}

nrevfields(rev, isodd)
char *rev;
int isodd;
{
    int i = 0;

    for (;;) {
	if (!isodd) {
	    if (*rev < '0' || *rev > '9')
		quit(1, "invalid revision specification\n");
	    while (*rev >= '0' && *rev <= '9')
		rev++;
	    if (*rev++ != '.')
		quit(1, "invalid revision specification\n");
	    i++;
	} else
	    isodd = 0;
	if (*rev < '0' || *rev > '9')
	    quit(1, "invalid revision specification\n");
	while (*rev >= '0' && *rev <= '9')
	    rev++;
	i++;
	if (*rev == '\0')
	    return(i);
	if (*rev++ != '.')
	    quit(1, "invalid revision specification\n");
    }
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

struct desc {
    char *data;
    int match_all;
} empty_desc[] = {
	"head     1.1;\n",		1,
	"branch   ;\n",			1,
	"access   ;\n",			1,
	"symbols  ;\n",			1,
	"locks    ; strict;\n",		1,
	"comment  @",			0,
	"\n",				1,
	"\n",				1,
	"1.1\n",			1,
	"date     90.01.01.00.00.00;  author ",	0,
	"branches ;\n",			1,
	"next     ;\n",			1,
	"\n",				1,
	"\n",				1,
	"desc\n",			1,
	"@@\n",				1,
	"\n",				1,
	"\n",				1,
	"1.1\n",			1,
	"log\n",			1,
	"@*** Initial Trunk Revision ***\n",	1,
	"@\n",				1,
	"text\n",			1,
	"@@\n",				1,
	0,				0
};

remove_empty_file()
{
    char buf[MAXPATHLEN];
    FILE *fp;
    char *ptr;
    struct desc *dp;

    fp = fopen(outdate_file, "r");
    if (fp == NULL)
	return;
    for (dp = &empty_desc[0]; dp->data != 0; dp++) {
	if (fgets(buf, sizeof(buf), fp) == NULL)
	    return;
	if (ferror(fp))
	    return;
	if (dp->match_all) {
	    if (strcmp(buf, dp->data) == 0)
		continue;
	} else if (strncmp(buf, dp->data, strlen(dp->data)) == 0)
	    continue;
	(void) fclose(fp);
	return;
    }
    if (fgets(buf, sizeof(buf), fp) != NULL)
	return;
    if (ferror(fp) || fclose(fp) == EOF)
	return;
    (void) unlink(outdate_file);
}
