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
static char	*sccsid = "@(#)$RCSfile: mklinks.c,v $ $Revision: 4.3 $ (DEC) $Date: 1991/11/26 11:44:43 $";
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
#include <sys/param.h>
#include <sys/dir.h>
#include <sys/stat.h>
#include <sys/errno.h>
#include <sys/file.h>
#include <sys/time.h>
#include <stdio.h>
#include <sdm/parse_cmd_line.h>

#define CERROR	(-1)
#define FALSE	0
#define TRUE	1

extern int errno;

extern char *concat();

extern char *prog;

#ifndef S_ISDIR
#define	S_ISDIR(m)	(((m)&S_IFMT) == S_IFDIR)
#endif
#ifndef S_ISREG
#define	S_ISREG(m)	(((m)&S_IFMT) == S_IFREG)
#endif
#ifndef S_ISLNK
#define	S_ISLNK(m)	(((m)&S_IFMT) == S_IFLNK)
#endif

int
makelink(stp, isnew, isdir, verbose, namebuf, srcpath, curpath, srcpref,
	 cmpfunc, linkfunc)
struct stat *stp;
int isnew, isdir, verbose;
char *namebuf, *srcpath, *curpath, *srcpref;
int (*cmpfunc)();
int (*linkfunc)();
{
    char buf1[MAXPATHLEN];
    struct stat st, bst;
    int status;

if (verbose) printf("makelink(stp %x isnew %d isdir %d verbose %d\n",
		    stp, isnew, isdir, verbose);
if (verbose) printf("         namebuf %s srcpath %s curpath %s srcpref %s)\n",
		    namebuf, srcpath, curpath, srcpref ? srcpref : NULL);
    if (isdir) {
	if (!isnew) {
	    if (stat(curpath, &st) < 0) {
		if (errno != ENOENT) {
		    fprintf(stderr, "mklinks: stat %s: %s\n",
			    curpath, errmsg(-1));
		    return(CERROR);
		}
	    } else if (S_ISDIR(st.st_mode))
		return(FALSE);
	}
	if (cmpfunc != NULL)
	    return(TRUE);
	if (mkdir(curpath, 0777) < 0) {
	    fprintf(stderr, "mklinks: mkdir %s: %s\n", curpath, errmsg(-1));
	    return(CERROR);
	}
	if (verbose)
	    printf("%s: created directory\n", namebuf);
	return(TRUE);
    }
    if (!isnew && lstat(curpath, &st) == 0) {
	if (cmpfunc == NULL)
	    return(FALSE);
if (verbose) printf("%s: mode %o\n", curpath, st.st_mode);
	if (S_ISLNK(st.st_mode)) {
	    if (stat(curpath, &st) != 0) {
		fprintf(stderr, "mklinks: stat %s: %s\n", curpath, errmsg(-1));
		return(CERROR);
	    }
	}
if (verbose) printf("%s: mode %o\n", curpath, st.st_mode);
	if (!S_ISREG(st.st_mode))
	    return(FALSE);
	if (stp == NULL) {
	    stp = &bst;
	    if (lstat(namebuf, stp) != 0) {
		fprintf(stderr, "mklinks: lstat %s: %s\n",
			namebuf, errmsg(-1));
		return(CERROR);
	    }
	}
if (verbose) printf("%s: mode %o\n", namebuf, stp->st_mode);
	if (S_ISLNK(stp->st_mode)) {
	    if (stat(namebuf, stp) != 0) {
		fprintf(stderr, "mklinks: stat %s: %s\n", namebuf, errmsg(-1));
		return(CERROR);
	    }
	}
if (verbose) printf("%s: mode %o\n", namebuf, stp->st_mode);
	if (!S_ISREG(stp->st_mode))
	    return(FALSE);
if (verbose) printf("%s: size %d vs %d\n", curpath, st.st_size, stp->st_size);
	if (st.st_size != stp->st_size || st.st_size == 0)
	    return(FALSE);
	status = (*cmpfunc)(namebuf, curpath);
if (verbose) printf("%s: cmp %d\n", namebuf, status);
	if (status != 0)
	    return((status == CERROR) ? CERROR : FALSE);
	(void) unlink(curpath);
    } else {
	if (!isnew && errno != ENOENT) {
	    fprintf(stderr, "mklinks: lstat %s: %s\n", curpath, errmsg(-1));
	    return(CERROR);
	}
	if (cmpfunc != NULL)
	    return(FALSE);
    }
    if (srcpref != NULL && *srcpref != '\0') {
	if (concat(buf1, sizeof(buf1), srcpref, srcpath, NULL) == NULL) {
	    fprintf(stderr, "mklinks: %s: path too long\n", buf1);
	    return(FALSE);
	}
if (verbose) printf("linkfunc(%s, %s)\n", buf1, curpath);
	if ((*linkfunc)(buf1, curpath) == CERROR)
	    return(CERROR);
    } else {
if (verbose) printf("linkfunc(%s, %s)\n", srcpath, curpath);
	if ((*linkfunc)(srcpath, curpath) == CERROR)
	    return(CERROR);
    }
    if (verbose)
	printf("%s: created\n", namebuf);
    return(FALSE);
}

void
descend(ndirs, isnew, verbose, query, recurse,
	namebuf, endname,
	srcpath, endsrc,
	curpath, endcur,
	srcpref, endpref,
	cmpfunc, linkfunc)
int ndirs;
int isnew;
int verbose;
int query;
int recurse;
char *namebuf, *endname;
char *srcpath, *endsrc;
char *curpath, *endcur;
char *srcpref, *endpref;
int (*cmpfunc)();
int (*linkfunc)();
{
    struct stat st;
    DIR *dirp;
    struct direct *dp;
    int subisnew;
    char *np, *sp, *cp, *pp;
    char buf[MAXPATHLEN];

    if (verbose)
	printf("%s\n", namebuf);
if (verbose)
    printf("descend(ndirs %d isnew %d verbose %d query %d recurse %d\n",
	   ndirs, isnew, verbose, query, recurse);
if (verbose)
    printf("        namebuf %s srcpath %s curpath %s srcpref %s)\n",
	   namebuf, srcpath, curpath, srcpref ? srcpref : NULL);
    if ((dirp = opendir(namebuf)) == NULL) {
	perror(namebuf);
	return;
    }
    for (dp = readdir(dirp); dp != NULL; dp = readdir(dirp)) {
	if (strcmp(dp->d_name, ".") == 0 ||
	    strcmp(dp->d_name, "..") == 0) {
	    continue;
	}
	np = concat(endname, namebuf+MAXPATHLEN-endname,
		   "/", dp->d_name, NULL);
	if (np == NULL) {
	    fprintf(stderr, "mklinks: %s: path too long\n", namebuf);
	    continue;
	}
	sp = concat(endsrc, srcpath+MAXPATHLEN-endsrc,
		   "/", dp->d_name, NULL);
	if (sp == NULL) {
	    fprintf(stderr, "mklinks: %s: path too long\n", srcpath);
	    continue;
	}
	cp = concat(endcur, curpath+MAXPATHLEN-endcur,
		   "/", dp->d_name, NULL);
	if (cp == NULL) {
	    fprintf(stderr, "mklinks: %s: path too long\n", curpath);
	    continue;
	}
	if (ndirs == 0) {
	    (void) makelink((struct stat *)NULL, isnew, FALSE, verbose,
			    namebuf, srcpath, curpath, srcpref,
			    cmpfunc, linkfunc);
	    continue;
	}
	if (lstat(namebuf, &st) != 0) {
	    perror(namebuf);
	    continue;
	}
	if (!S_ISDIR(st.st_mode)) {
	    (void) makelink(&st, isnew, FALSE, verbose,
			    namebuf, srcpath, curpath, srcpref,
			    cmpfunc, linkfunc);
	    continue;
	}
	ndirs--;
	if (query) {
	    (void) concat(buf, sizeof(buf),
			  "Link directory ", namebuf, " ?", NULL);
	    if (!getbool(buf, TRUE))
		continue;
	}
	if (!recurse)
	    continue;
	subisnew = makelink(&st, isnew, TRUE, verbose,
			    namebuf, srcpath, curpath, srcpref,
			    cmpfunc, linkfunc);
	if (subisnew == CERROR)
	    continue;
	if (subisnew && cmpfunc != NULL)
	    continue;
	if (srcpref != NULL) {
	    pp = concat(endpref, srcpref+MAXPATHLEN-endpref, "../", NULL);
	    if (pp == NULL) {
		fprintf(stderr, "mklinks: %s: path too long\n", srcpref);
		return;
	    }
	} else
	    pp = NULL;
	descend(st.st_nlink-2, subisnew, verbose, query, recurse,
		namebuf, np, srcpath, sp,
		curpath, cp, srcpref, pp,
		cmpfunc, linkfunc);
	if (endpref != NULL)
	    *endpref = '\0';
    }
    (void) closedir(dirp);
}

mklinks(isnew, verbose, query, recurse,
	srcpath, curpath, srcpref, cmpfunc, linkfunc)
int isnew, verbose, query, recurse;
char *srcpath, *curpath, *srcpref;
int (*cmpfunc)();
int (*linkfunc)();
{
    char namebuf[MAXPATHLEN];
    struct stat st;

    namebuf[0] = '.';
    namebuf[1] = '\0';
    if (stat(namebuf, &st) < 0)
	quit(1, "mklinks: stat %s: %s\n", srcpath, errmsg(-1));

    descend(st.st_nlink - 2, isnew, verbose, query, recurse,
	    namebuf, namebuf + 1,
	    srcpath, srcpath + strlen(srcpath),
	    curpath, curpath + strlen(curpath),
	    (srcpref != NULL) ? srcpref : NULL,
	    (srcpref != NULL) ? srcpref + strlen(srcpref): NULL,
	    cmpfunc, linkfunc);
}
