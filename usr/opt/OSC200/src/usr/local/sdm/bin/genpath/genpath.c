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
static char	*sccsid = "@(#)$RCSfile: genpath.c,v $ $Revision: 4.3.3.2 $ (DEC) $Date: 1992/01/30 09:01:31 $";
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
 * genpath.c
 *
 *	Modification History:
 *
 * 01-Apr-91	Fred Canter
 *	MIPS C 2.20+, changes for -std
 *
 */
/*
 * program to generate command flags for sandboxes
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <stdio.h>
#ifdef __STDC__
#include <stdarg.h>
#else
#include <varargs.h>
#endif
#ifndef _BLD
#include <sdm/parse_rc_file.h>
#include <sdm/parse_cmd_line.h>
#endif

extern int errno;

extern char *index();
extern char *rindex();
extern char *malloc();
extern char *salloc();
extern char *getenv();

#ifdef __STDC__
char *concat(char *buf, int buflen, ...);
#else
char *concat();
#endif
char *vconcat();

char **expand_flag();
char *path_relative_to();

char *prog;

usage()
{
    fprintf(stderr, "usage: %s [ options ] [ switches ... ]\n", prog);
    fprintf(stderr, "options include:\n");
    fprintf(stderr, "    -rc <rc_file>\n");
    fprintf(stderr, "    -sb <sandbox_name>\n");
    fprintf(stderr, "    -sb_rc <sandbox_rc_file>\n");
    fprintf(stderr, "    -usage\n");
    fprintf(stderr, "    -v\n");
    exit(1);
}

main(argc, argv)
    int argc;
    char *argv[];
{
    char *rcfile_source_base;
    char *rcfile_object_base;
    char sandbox[BUFSIZ];
    char basedir[BUFSIZ];
    char sb_rcfile[BUFSIZ];
    char usr_rcfile[BUFSIZ];
    char search_path[1024*2];
    char source_base[1024];
    char sourcebase[1024];
    char sourcedir[1024];
    char curdir[1024];
    int curdir_len;
    char *relpath;
    int i, j, k;
    char ch,*b,*p,*p2,*p3,**nargv;
    int nargc;
    char buf[1024+2];
    char ibuf[1024+2];
    int verbose = 0;
    char *space;
#ifndef _BLD
    struct rcfile rcfile;
    struct field *field_p;
    struct arg_list *args_p;
#endif
    int read_rcfile;
    int in_objdir;

    if (argc > 0) {
	if ((prog = rindex(argv[0], '/')) != NULL)
	    prog++;
	else
	    prog = argv[0];
	argc--; argv++;
    } else
	prog = "genpath";

#ifndef _BLD
    bzero(&rcfile, sizeof(struct rcfile));
#endif
    sandbox[0] = '\0';
    sb_rcfile[0] = '\0';
    usr_rcfile[0] = '\0';
    read_rcfile = 0;

    while (argc > 0) {
	if (strcmp(argv[0], "-v") == 0) {
	    verbose++;
	    argv++;
	    argc--;
	    continue;
	}
	if (strcmp(argv[0], "-sb") == 0) {
	    read_rcfile = 1;
	    if (argc == 1) {
		fprintf(stderr, "%s: missing argument to %s switch\n",
			prog, argv[0]);
		usage();
	    }
	    argc--;
	    argv++;
	    (void) strcpy(sandbox, argv[0]);
	    argc--;
	    argv++;
	    continue;
	}
	if (strcmp(argv[0], "-sb_rc") == 0) {
	    read_rcfile = 1;
	    if (argc == 1) {
		fprintf(stderr, "%s: missing argument to %s switch\n",
			prog, argv[0]);
		usage();
	    }
	    argc--;
	    argv++;
	    (void) strcpy(sb_rcfile, argv[0]);
	    argc--;
	    argv++;
	    continue;
	}
	if (strcmp(argv[0], "-rc") == 0) {
	    read_rcfile = 1;
	    if (argc == 1) {
		fprintf(stderr, "%s: missing argument to %s switch\n",
			prog, argv[0]);
		usage();
	    }
	    argc--;
	    argv++;
	    (void) strcpy(usr_rcfile, argv[0]);
	    argc--;
	    argv++;
	    continue;
	}
	if (strcmp(argv[0], "-usage") == 0)
	    usage();
	break;
    }

    if (argc == 0)
	exit(0);

    for (i = 0; i < argc; i++) {
	if (argv[i][0] != '-') {
	    fprintf(stderr, "%s: argument %s is not a switch\n",
		    prog, argv[i]);
	    exit(1);
	}
	if (argv[i][1] != 'I' && argv[i][1] != 'L') {
	    fprintf(stderr, "%s: switch %s not recognized\n",
		    prog, argv[i]);
	    exit(1);
	}
    }

    if (read_rcfile ||
	(b = getenv("SOURCEBASE")) == NULL ||
	(p = getenv("SOURCEDIR")) == NULL) {
#ifdef _BLD
	fprintf(stderr, "%s: SOURCEBASE or SOURCEDIR not defined\n", prog);
	exit(1);
#else	/* _BLD */
	read_rcfile = 1;
	if (get_current_sb_basedir(sandbox, basedir,
				   sb_rcfile, usr_rcfile) != 0) {
	    fprintf(stderr, "%s: unable to get sandbox basedir\n", prog);
	    exit(1);
	}
	if (parse_rc_file(sb_rcfile, &rcfile) != 0) {
	    fprintf(stderr, "%s: unable to parse %s sandbox description\n",
		    prog, sandbox);
	    exit(1);
	}
	if (rc_file_field(&rcfile, "source_base", &field_p) != 0) {
	    fprintf(stderr, "%s: source_base not defined\n", prog);
	    exit(1);
	}
	args_p = field_p->args;
	if (args_p->ntokens != 1) {
	    fprintf(stderr, "%s: improper source_base\n", prog);
	    exit(1);
	}
	rcfile_source_base = args_p->tokens[0];
	if (*rcfile_source_base != '/') {
	    fprintf(stderr, "%s: source_base is not an absolute path\n", prog);
	    exit(1);
	}
	if (rc_file_field(&rcfile, "object_base", &field_p) != 0) {
	    fprintf(stderr, "%s: object_base not defined\n", prog);
	    exit(1);
	}
	args_p = field_p->args;
	if (args_p->ntokens != 1) {
	    fprintf(stderr, "%s: improper object_base\n", prog);
	    exit(1);
	}
	rcfile_object_base = args_p->tokens[0];
	if (*rcfile_object_base != '/') {
	    fprintf(stderr, "%s: object_base is not an absolute path\n", prog);
	    exit(1);
	}
	if ((b = getenv("SOURCEBASE")) == NULL) {
	    fprintf(stderr, "%s: SOURCEBASE is not defined\n", prog);
	    exit(1);
	}
	if ((p = getenv("SOURCEDIR")) == NULL) {
	    fprintf(stderr, "%s: SOURCEDIR is not defined\n", prog);
	    exit(1);
	}
#endif	/* _BLD */
    }
    (void) strcpy(source_base, b);
    if (verbose)
	fprintf(stderr, "[ SOURCEBASE %s ]\n", b);
    (void) strcpy(sourcedir, p);
    if (verbose)
	fprintf(stderr, "[ SOURCEDIR %s ]\n", p);

    if (getwd(curdir) == NULL) {
	fprintf(stderr, "%s: getwd .: %s\n", prog, curdir);
	exit(1);
    }
    curdir_len = strlen(curdir);
    if (curdir_len == 0 || curdir[0] != '/') {
	fprintf(stderr, "%s: getwd returned bad directory \"%s\"\n",
		prog, curdir);
	exit(1);
    }
    if (verbose)
	fprintf(stderr, "[ curdir %s ]\n", curdir);

    if (read_rcfile || (p = getenv("OBJECTDIR")) != NULL) {
	if (read_rcfile)
	    p = rcfile_object_base;
	if (verbose)
	    fprintf(stderr, "[ OBJECTDIR %s ]\n", p);
	if (*p != '/') {
	    canonicalize(source_base, p, buf, sizeof(buf));
	    p = buf;
	}
	if (verbose)
	    fprintf(stderr, "[ object_base %s ]\n", p);
	relpath = path_relative_to(p, curdir, curdir_len);
	in_objdir = (relpath != NULL);
    } else
	in_objdir = 0;
    if (!in_objdir)
	relpath = path_relative_to(source_base, curdir, curdir_len);
    if (relpath == NULL) {
	fprintf(stderr, "%s: unable to find path within sandbox\n", prog);
	return(1);
    }
    if (verbose)
	fprintf(stderr, "[ relative path %s ]\n", relpath);

    b = source_base;
    p = sourcedir;
    if (*p == '\0')
	(void) concat(search_path, sizeof(search_path), ":", b, relpath, NULL);
    else if (*relpath == '\0')
	(void) concat(search_path, sizeof(search_path), ":", b, ":", p, NULL);
    else {
	p3 = concat(search_path, sizeof(search_path), ":", b, relpath, NULL);
	for (;;) {
	    p2 = p;
	    while (*p && *p != ':')
		p++;
	    ch = *p;
	    *p = '\0';
	    if (*p2 != '/') {
		fprintf(stderr, "%s: SOURCEDIR contains a relative path\n",
			prog);
		exit(1);
	    }
	    p3 = concat(p3, search_path + sizeof(search_path) - p3,
			":", p2, relpath, NULL);
	    if (ch == '\0')
		break;
	    *p++ = ch;
	}
    }
    if (verbose)
	fprintf(stderr, "[ search_path %s ]\n", search_path);

    nargc = argc;
    nargv = (char **)malloc(nargc*sizeof(char *));
    j = 0;	/* argument counter for new arg list */

    for (i = 0; i < argc; i++) {
	if (argv[i][2] == '/') {
	    nargv[j++] = argv[i];
	    continue;
	}
	nargv = expand_flag(&j, &nargc, nargv, argv[i], search_path);
    }

    space = "";
    for (i = 0; i < j; i++) {
	printf("%s%s", space, nargv[i]);
	space = " ";
    }
    if (j > 0)
	printf("\n");
    exit(0);
}

/*
 * canonicalize path - similar to abspath
 */
canonicalize(base, relpath, outbuf, outbuf_size)
char *base;
char *relpath;
char *outbuf;
int outbuf_size;
{
    char *from;
    char *to;
    char *slash;
    char *peek;

    /*
     * concatenate parts of path into buffer
     */
    if (concat(outbuf, outbuf_size, base, "/", relpath, "/", NULL) == NULL) {
	fprintf(stderr, "%s: path length exceeds buffer size\n");
	return(-1);
    }

    /*
     * remember position of first slash
     */
    slash = index(outbuf, '/');
    from = to = slash + 1;

    /*
     * canonicalize the path
     */
    while (*from != '\0') {
	if ((*to++ = *from++) != '/')
	    continue;
	peek = to-2;
	if (*peek == '/') {
	    /*
	     * found "//", back up one
	     */
	    to--;
	    continue;
	}
	if (*peek != '.')
	    continue;
	peek--;
	if (*peek == '/') {
	    /*
	     * found "/./", back up two
	     */
	    to -= 2;
	    continue;
	}
	if (*peek != '.')
	    continue;
	peek--;
	if (*peek != '/')
	    continue;
	/*
	 * found "/../", try to remove preceding token
	 */
	if (peek == slash) {
	    /*
	     * hit the "first" slash, update to not remove any more tokens
	     */
	    slash = to-1;
	    continue;
	}
	/*
	 * backup one token 
	 */
	while (*--peek != '/')
	    ;
	to = peek+1;
    }
    *to-- = '\0';
    if (to > outbuf && *to == '/')
	*to = '\0';
}

char **
expand_flag(j, nargc, nargv, rel, search_path)
    int *j;
    int *nargc;
    char **nargv;
    char *rel;
    char *search_path;
{
    char ibuf[1024+2];
    char ch;
    char *p;
    char *p2;
    char *p3;

    ibuf[0] = *rel++;
    ibuf[1] = *rel++;
    p = search_path;
    for (;;) {
	p2 = p;
	while (*p && *p != ':')
	    p++;
	ch = *p;
	*p = '\0';
	p3 = NULL;
	if (*p2 == '\0' || (*p2 == '.' && *(p2+1) == '\0'))
	    p3 = rel;
	else if (*rel == '\0' || (*rel == '.' && *(rel+1) == '\0'))
	    p3 = p2;
	if (p3 == NULL)
	    canonicalize(p2, rel, ibuf+2, sizeof(ibuf)-2);
	else if (*p3 == '/')
	    canonicalize("", p3, ibuf+2, sizeof(ibuf)-2);
	else {
	    canonicalize("", p3, ibuf+1, sizeof(ibuf)-1);
	    ibuf[1] = *(rel-1);
	    if (ibuf[2] == '\0') {
		ibuf[2] = '.';
		ibuf[3] = '\0';
	    }
	}
	(*nargc)++;
	nargv = (char **)realloc((char *)nargv, (*nargc)*sizeof(char *));
	nargv[(*j)++] = salloc(ibuf);
	if (ch == '\0')
	    break;
	*p++ = ch;
    }
    return(nargv);
}

/*
 * If we are within the directory subtree of base_dir, return the path
 * from there to the current directory.  Otherwise, return NULL.
 */
char *
path_relative_to(base_dir, curdir, curdir_len)
char *base_dir, *curdir;
int curdir_len;
{
    char errbuf[BUFSIZ];
    int save_errno;
    char basedir[1024];
    char *path;
    int len;

    if (chdir(base_dir) < 0) {
	save_errno = errno;
	(void) sprintf(errbuf, "%s: chdir %s", prog, base_dir);
	errno = save_errno;
	perror(errbuf);
	exit(1);
    }
    if (getwd(basedir) == NULL) {
	fprintf(stderr, "%s: getwd %s: %s\n", prog, base_dir, basedir);
	exit(1);
    }
    if (chdir(curdir) < 0) {
	save_errno = errno;
	(void) sprintf(errbuf, "%s: chdir %s", prog, base_dir);
	errno = save_errno;
	perror(errbuf);
	exit(1);
    }
    len = strlen(basedir);
    if (len == 0 || basedir[0] != '/') {
	fprintf(stderr, "%s: getwd returned bad base directory \"%s\"\n",
		prog, basedir);
	exit(1);
    }
    if (curdir_len < len)
	return(NULL);
    if (bcmp(basedir, curdir, len) != 0)
	return(NULL);
    if (curdir[len] != '\0' && curdir[len] != '/')
	return(NULL);
    if ((path = salloc(curdir+len)) == NULL) {
	save_errno = errno;
	(void) sprintf(errbuf, "%s: salloc relative path", prog);
	errno = save_errno;
	perror(errbuf);
	exit(1);
    }
    return(path);
}

#ifdef _BLD
char *salloc(p)
char *p;
{
	register char *q;
	register int l;

	q = malloc(l = strlen(p) + 1);
	if (q != 0)
		bcopy(p, q, l);
	return(q);
}

/*VARARGS2*/
char *
#ifdef __STDC__
concat(char *buf, int buflen, ...)
#else
concat(va_alist)
va_dcl
#endif
{
#ifndef __STDC__
    char *buf;
    int buflen;
#endif
    va_list ap;
    char *ptr;

#ifdef __STDC__
    va_start(ap, buflen);
#else
    va_start(ap);
    buf = va_arg(ap, char *);
    buflen = va_arg(ap, int);
#endif
    ptr = vconcat(buf, buflen, ap);
    va_end(ap);
    return(ptr);
}

char *
vconcat(buf, buflen, ap)
char *buf;
int buflen;
va_list ap;
{
    register char *arg, *ptr, *ep;

    if (buf == NULL)
	return(NULL);
    if (buflen <= 0)
	return(NULL);
    ptr = buf;
    *ptr = '\0';
    ep = buf + buflen;
    while (ptr != NULL && (arg = va_arg(ap, char *)) != NULL)
	while (*ptr = *arg++)
	    if (++ptr == ep) {
		ptr = NULL;
		break;
	    }
    return(ptr);
}
#endif	/* _BLD */
