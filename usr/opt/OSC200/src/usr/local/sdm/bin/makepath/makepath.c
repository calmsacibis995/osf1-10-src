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
static char	*sccsid = "@(#)$RCSfile: makepath.c,v $ $Revision: 4.2.5.2 $ (DEC) $Date: 1992/01/30 09:01:48 $";
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
 *  makepath - create intermediate directories along a path
 *
 *  makepath destpath [ srcpath ]
 *
 *  Create any directories missing in destpath.  If srcpath is given,
 *  then it must have a non-null trailing subpath that is common to
 *  destpath.  The entire path will be created if srcpath is given and
 *  srcpath is the path of a directory, otherwise only the components
 *  preceding the final component will be created for destpath.
 */
#include <sys/param.h>

#ifndef _BLD
#include <sys/limits.h>
#endif /* _BLD */

#include <sys/stat.h>
#include <sys/time.h>
#include <stdio.h>

#ifndef	PATH_MAX
#define PATH_MAX	1024
#endif

#define	TRUE	1
#define	FALSE	0

char *rindex();
char *nxtarg();

main(argc, argv)
    int argc;
    char *argv[];
{
    char *progname;
    int quiet = FALSE;

    if (argc > 0) {
	if ((progname = rindex(argv[0], '/')) == NULL)
	    progname = argv[0];
	else
	    progname++;
    } else
	progname = "makepath";
    argc--;
    argv++;

    if (argc > 0 && strcmp(argv[0], "-quiet") == 0) {
	quiet = TRUE;
	argc--;
	argv++;
    }

    if (argc == 0 || argc > 2) {
	fprintf(stderr,
		"usage: %s [ -quiet ] [ - | destpath ] [ srcpath ]\n",
		progname);
	exit(1);
    }

    if (strcmp(argv[0], "-") == 0) {
	int status = 0;
	char *base, *ref, *endp, *ptr;
	char buffer[PATH_MAX];

	while (fgets(buffer, sizeof(buffer), stdin) != NULL) {
	    ptr = buffer;
	    base = nxtarg(&ptr, " \t\n");
	    ref = nxtarg(&ptr, " \t\n");
	    endp = nxtarg(&ptr, " \t\n");
	    if (*endp != '\0') {
		fprintf(stderr, "improper input format\n");
		continue;
	    }
	    if (*base == '\0')
		continue;
	    status |= makepath(base, *ref != '\0' ? ref : NULL, !quiet, TRUE);
	}
	exit(status);
    }

    exit(makepath(argv[0], argc == 2 ? argv[1] : NULL, !quiet, TRUE));
}

#ifdef _BLD
static unsigned char tab[256] = {
	0
};

char *skipto (string,charset)
unsigned char *string, *charset;
{
	register unsigned char *setp,*strp;

	tab[0] = 1;		/* Stop on a null, too. */
	for (setp=charset;  *setp;  setp++) tab[*setp]=1;
	for (strp=string;  tab[*strp]==0;  strp++)  ;
	for (setp=charset;  *setp;  setp++) tab[*setp]=0;
	return ((char *)strp);
}

char *nxtarg (q,brk)
char **q,*brk;
{
	register char *front,*back;
	front = *q;			/* start of string */
	/* leading blanks and tabs */
	while (*front && (*front == ' ' || *front == '\t')) front++;
	/* find break character at end */
	if (brk == 0)  brk = " ";
	back = skipto (front,brk);
	*q = (*back ? back+1 : back);	/* next arg start loc */
	/* elim trailing blanks and tabs */
	back -= 1;
	while ((back >= front) && (*back == ' ' || *back == '\t')) back--;
	back++;
	if (*back)  *back = '\0';
	return (front);
}

static
char *fixpath(path, buf)
register char *path;
char *buf;
{
    register char *ls = NULL;
    register char *p = buf;

    *p = *path;
    while (*path != '\0') {
	path++;
	while (*p == '/' && *path == '/')
	    path++;
	*++p = *path;
	if (*p == '/')
	    ls = p;
    }
    return(ls);
}

extern int errno;
extern int sys_nerr;
extern char *sys_errlist[];

static char *itoa(p,n)
char *p;
unsigned n;
{
    if (n >= 10)
	p =itoa(p,n/10);
    *p++ = (n%10)+'0';
    return(p);
}

char *errmsg(cod)
int cod;
{
	static char unkmsg[] = "Unknown error ";
	static char unk[sizeof(unkmsg)+11];		/* trust us */

	if (cod < 0) cod = errno;

	if((cod >= 0) && (cod < sys_nerr))
	    return(sys_errlist[cod]);

	strcpy(unk,unkmsg);
	*itoa(&unk[sizeof(unkmsg)-1],cod) = '\0';

	return(unk);
}

makepath(path, refpath, trace, showerrs)
char *path, *refpath;
int trace, showerrs;
{
    char pathbuf[MAXPATHLEN], refpathbuf[MAXPATHLEN];
    char linkbuf[MAXPATHLEN], linkbuf2[MAXPATHLEN];
    char *base, *refbase;
    char *slash, *refslash;
    struct stat statb, refstatb;
    int ch, cc, cc2;
    extern uid_t getuid();
    uid_t uid = getuid();

    if (path == NULL) {
	if (showerrs)
	    fprintf(stderr, "makepath: NULL path argument\n");
	return(1);
    }

    base = fixpath(path, pathbuf);

    if (refpath == NULL) {
	if (base == NULL || base == pathbuf) {
	    if (showerrs)
		fprintf(stderr,
			"makepath: %s must have an imbedded '/' character\n",
			pathbuf);
	    return(1);
	}
	*base = '\0';
	base = pathbuf;
	if (*base == '/')
	    base++;
	if (*base == '\0') {
	    if (showerrs)
		fprintf(stderr, "makepath: illegal pathname %s\n", pathbuf);
	    return(1);
	}
	for (;;) {
	    /* find end of this component */
	    while (*base != '\0' && *base != '/')
		base++;

	    /* create path so far, if necessary */
	    ch = *base; *base = '\0';
	    if (stat(pathbuf, &statb) < 0) {
		if (mkdir(pathbuf, 0777) < 0) {
		    if (showerrs)
			fprintf(stderr,
				"makepath: unable to create directory %s: %s\n",
				pathbuf, errmsg(-1));
		    return(1);
		}
		if (stat(pathbuf, &statb) < 0) {
		    if (showerrs)
			fprintf(stderr,
				"%s: unable to stat directory %s: %s\n",
				pathbuf, errmsg(-1));
		    return(1);
		}
		if (trace)
		    fprintf(stderr, "%s: created directory\n", pathbuf);
	    } else if ((statb.st_mode&S_IFMT) != S_IFDIR) {
		if (showerrs)
		    fprintf(stderr,
			    "makepath: %s is not a directory (mode %#o)\n",
			    pathbuf, (statb.st_mode&S_IFMT));
		return(1);
	    }
	    if (ch == '\0')
		break;
	    *base++ = ch;
	}
	return(0);
    }

    refbase = fixpath(refpath, refpathbuf);

    /* if not a directory path, strip final component */
    if (stat(refpathbuf, &refstatb) < 0 ||
	(refstatb.st_mode&S_IFMT) != S_IFDIR) {
	if (base == NULL || base == pathbuf) {
	    if (showerrs)
		fprintf(stderr,
			"makepath: %s must have an imbedded '/' character\n",
			pathbuf);
	    return(1);
	}
	if (refbase == NULL || refbase == refpathbuf) {
	    if (showerrs)
		fprintf(stderr,
			"makepath: %s must have an imbedded '/' character\n",
			refpathbuf);
	    return(1);
	}
	if (strcmp(base, refbase) != 0) {
	    if (showerrs)
		fprintf(stderr,
			"makepath: %s and %s do not have common trailing components\n",
			base, refbase);
	    return(1);
	}
	*base = *refbase = '\0';
	if (stat(refpathbuf, &refstatb) < 0) {
	    if (showerrs)
		fprintf(stderr,
			"%s: unable to stat reference directory %s: %s\n",
			refpathbuf, errmsg(-1));
	    return(1);
	}
	if ((refstatb.st_mode&S_IFMT) != S_IFDIR) {
	    if (showerrs)
		fprintf(stderr, "makepath: %s is not a directory (mode %#o)\n",
			refpathbuf, (refstatb.st_mode&S_IFMT));
	    return(1);
	}
    }

    /* find beginning of common part of path names */
    base = pathbuf + strlen(pathbuf);
    refbase = refpathbuf + strlen(refpathbuf);
    while (*base == *refbase) {
	if (base == pathbuf || refbase == refpathbuf)
	    break;
	base--; refbase--;
    }
    if (*base == *refbase && *base != '/') {
	if (base == pathbuf && *(refbase-1) == '/') {
	    base = pathbuf + strlen(pathbuf) + 2;
	    do {
		*base = *(base - 2);
	    } while (base-- > pathbuf + 2);
	    *(base-1) = '.';
	    *base = '/';
	    refbase--;
	}
	if (refbase == refpathbuf && *(base-1) == '/') {
	    refbase = refpathbuf + strlen(refpathbuf) + 2;
	    do {
		*refbase = *(refbase - 2);
	    } while (refbase-- > refpathbuf + 2);
	    *(refbase-1) = '.';
	    *refbase = '/';
	    base--;
	}
    }
    while (*base != '\0' && *base != '/') {
	base++; refbase++;
    }
    slash = base++;
    refslash = refbase++;
    ch = *slash; *slash = '\0';
    if (stat(pathbuf, &statb) < 0) {
	if (showerrs)
	    fprintf(stderr,
		    "makepath: unable to stat target directory %s: %s\n",
		    pathbuf, errmsg(-1));
	return(1);
    }
    if ((statb.st_mode&S_IFMT) != S_IFDIR) {
	if (showerrs)
	    fprintf(stderr, "makepath: %s: invalid mode %#o\n",
		    pathbuf, (statb.st_mode&S_IFMT));
	return(1);
    }
    *slash = ch;

    /* check each component along common path and make them the same */
    while (ch != '\0') {
	/* find end of this component */
	while (*base != '\0' && *base != '/') {
	    base++; refbase++;
	}

	/* get stat information for source path */
	ch = *refbase; *refbase = '\0'; *base = '\0';
	if (stat(refpathbuf, &refstatb) < 0) {
	    if (showerrs)
		fprintf(stderr, "makepath: stat %s: %s\n",
			refpathbuf, errmsg(-1));
	    return(1);
	}
	if ((refstatb.st_mode&S_IFMT) != S_IFDIR) {
	    if (showerrs)
		fprintf(stderr, "makepath: %s: invalid mode %#o\n",
			refpathbuf, (refstatb.st_mode&S_IFMT));
	    return(1);
	}
	if (lstat(refpathbuf, &refstatb) < 0) {
	    if (showerrs)
		fprintf(stderr, "makepath: lstat %s: %s\n",
			refpathbuf, errmsg(-1));
	    return(1);
	}
	if ((refstatb.st_mode&S_IFMT) == S_IFLNK) {
	    if ((cc = readlink(refpathbuf, linkbuf, sizeof(linkbuf)-1)) < 0) {
		if (showerrs)
		    fprintf(stderr, "makepath: readlink %s: %s\n",
			    refpathbuf, errmsg(-1));
		return(1);
	    }
	    if (cc > 0 && *linkbuf != '/') {
		*refbase = ch;
		linkbuf[cc] = '\0';
		if (lstat(pathbuf, &statb) < 0) {
		    if (symlink(linkbuf, pathbuf) < 0) {
			if (showerrs)
			    fprintf(stderr, "makepath: symlink %s: %s\n",
				    pathbuf, errmsg(-1));
			return(1);
		    }
		    if (trace)
			fprintf(stderr, "%s: created symlink to %s\n",
				pathbuf, linkbuf);
		} else {
		    if ((statb.st_mode&S_IFMT) != S_IFLNK) {
			if (showerrs)
			    fprintf(stderr, "makepath: %s: invalid mode %#o\n",
				    pathbuf, (statb.st_mode&S_IFMT));
			return(1);
		    }
		    cc2 = readlink(pathbuf, linkbuf2, sizeof(linkbuf2)-1);
		    if (cc2 < 0) {
			if (showerrs)
			    fprintf(stderr, "makepath: readlink %s: %s\n",
				    pathbuf, errmsg(-1));
			return(1);
		    }
		    if (cc != cc2 || bcmp(linkbuf, linkbuf2, cc) != 0) {
			if (showerrs)
			    fprintf(stderr,
				    "makepath: symlinks %s and %s differ\n",
				    refpathbuf, pathbuf, errmsg(-1));
			return(1);
		    }
		}
		(void) strcpy(linkbuf+cc, refbase);
		(void) strcpy(refslash+1, linkbuf);
		(void) strcpy(slash+1, linkbuf);
		(void) fixpath(refslash, refslash);
		(void) fixpath(slash, slash);
		refbase = refslash+1;
		base = slash+1;
		ch = *refslash;
		continue;
	    }
	}

	/* create path so far, if necessary */
	if (lstat(pathbuf, &statb) < 0) {
	    if (mkdir(pathbuf, (int)(refstatb.st_mode&07777)) < 0) {
		if (showerrs)
		    fprintf(stderr, "makepath: mkdir %s: %s\n",
			    pathbuf, errmsg(-1));
		return(1);
	    }
	    if (stat(pathbuf, &statb) < 0) {
		if (showerrs)
		    fprintf(stderr, "makepath: stat %s: %s\n",
			    pathbuf, errmsg(-1));
		return(1);
	    }
	    if (trace)
		fprintf(stderr, "%s: created directory\n", pathbuf);
	} else if ((statb.st_mode&S_IFMT) != S_IFDIR) {
	    if (showerrs)
		fprintf(stderr, "makepath: %s: invalid mode %#o\n",
			pathbuf, (statb.st_mode&S_IFMT));
	    return(1);
	}

	if (uid == 0 && (statb.st_uid != refstatb.st_uid ||
			 statb.st_gid != refstatb.st_gid)) {
	    (void) chown(pathbuf, (int)refstatb.st_uid, (int)refstatb.st_gid);
	    if (trace)
		fprintf(stderr, "%s: owner %d, group %d\n", pathbuf,
			refstatb.st_uid, refstatb.st_gid);
	}
	if ((statb.st_mode&07777) != (refstatb.st_mode&07777)) {
	    (void) chmod(pathbuf, (int)(refstatb.st_mode&07777));
	    if (trace)
		fprintf(stderr, "%s: mode %#o\n", pathbuf,
			refstatb.st_mode&07777);
	}

	refslash = refbase;
	*refbase++ = ch;
	slash = base;
	*base++ = ch;
    }
    return(0);
}
#endif /* _BLD */
