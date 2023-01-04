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
static char	*sccsid = "@(#)$RCSfile: ttyname.c,v $ $Revision: 4.2.10.2 $ (DEC) $Date: 1993/06/07 23:41:26 $";
#endif 
/*
 */
/*
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 1.2
 */
#if !defined(lint) && !defined(_NOIDENT)

#endif

/*
 * ttyname.c	5.2 (Berkeley) 3/9/86
 */

/*
 * ttyname(f): return "/dev/ttyXX" which the the name of the
 * tty belonging to file f.
 *  NULL if it is not a tty
 */

/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#if defined(_THREAD_SAFE)
#pragma weak ttyname_r = __ttyname_r
#endif
#if !defined(_THREAD_SAFE)
#pragma weak ttyname = __ttyname
#endif
#endif
#define	NULL	0
#include <unistd.h>
#include <sys/param.h>
#include <sys/dir.h>
#include <sys/stat.h>
#include <errno.h>
#include <strings.h>
#include <pty.h>
#include <paths.h>
#include "ts_supp.h"

static	char	dev[]	= "/dev/";
char	*strcpy();
char	*strcat();

static struct ttymap {
	int tm_rdev;   		/* dev_t   			*/
	char tm_base;		/* 9th character of ttyname 	*/
	char tm_count;		/* no. of 9th char variations 	*/
	char tm_len;		/* no. of 10th char variations  */
	char tm_last_chr;	/* 10th character of ttyname    */
} ttymap[] = {
	-1,	'p',	11,    16,   '0',  /*  ttyp0 - ttyzf minors 1-175    */
	-1,	'a',	3,     16,   '0',  /*  ttya0 - ttycf minors 176-223  */
	-1,	'e',	11,    16,   '0',  /*  ttye0 - ttyof minors 224-499  */
	-1,	'A',	26,    16,   '0',  /*  ttyA0 - ttyZf minors 500-815  */
	-1,	'p',	11,    46,   'g',  /*  ttypg - ttyzZ minors 816-1321 */
	-1,	'a',	3,     46,   'g',  /*  ttyag - ttycZ minors 1322-1459*/
	-1,	'e',	11,    46,   'g',  /*  ttyeg - ttyoZ minors 1460-1965*/
	-1,	'A',	26,    46,   'g',  /*  ttyAg - ttyZZ minors 1966-3161*/
};

static char *
#ifdef _THREAD_SAFE
fastttyname(dev_t rdev, char *ttytemp, int len)
#else
fastttyname(dev_t rdev)
#endif
{
	struct stat stb;
	register int i, n, rmin, rmaj = major(rdev);
	register struct ttymap *tm;
#ifndef _THREAD_SAFE
	static char ttytemp[32] = "/dev/tty??";
#else

	strcpy(ttytemp, "/dev/tty??");
#endif

	for (i = 0; i < (sizeof(ttymap)/sizeof(ttymap[0])); i++) {
		tm = &ttymap[i];
		if (tm->tm_rdev < 0) {
			ttytemp[8] = tm->tm_base;
			ttytemp[9] = tm->tm_last_chr;
			if (stat(ttytemp, &stb) < 0) {
				continue;
			}
			tm->tm_rdev = stb.st_rdev;
		}
		if (major(tm->tm_rdev) == rmaj) {
			rmin = minor(rdev);
			if (rmin < minor(tm->tm_rdev) ||
			    rmin >= minor(tm->tm_rdev) + (tm->tm_len * tm->tm_count))
				continue;
			rmin -= minor(tm->tm_rdev);
			ttytemp[8] = tm->tm_base + (rmin/tm->tm_len);
			rmin = rmin % tm->tm_len;
			if (tm->tm_last_chr == '0') {
				ttytemp[9] = (rmin) < 10 ?
					     (rmin) + '0' :
					     (rmin) - 10 + 'a';
			}
			else if (tm->tm_last_chr == 'g') {
				ttytemp[9] = (rmin) < 20 ?
					     (rmin) + 'g' :
					     (rmin) - 20 + 'A';
			}
			else
				ttytemp[9] += rmin;
			if (stat(ttytemp, &stb) >= 0 && stb.st_rdev == rdev)
				return(ttytemp);
		}
	}
	return(NULL);
}

#ifdef _THREAD_SAFE
int
ttyname_r(int f, char *rbuf, int len)
{
#else
char *
ttyname(int f)
{
	static char rbuf[MAX_PTYNAME_LEN];
#endif	/* _THREAD_SAFE */

	struct stat fsb;
	struct stat tsb;
	register DIR *df;
	char *p;
	register struct dirent *db;

	TS_EINVAL((rbuf == NULL) || (len < MAX_PTYNAME_LEN));

	if (isatty(f)==0)
		return(TS_FAILURE);

	if (fstat(f, &fsb) < 0)
		return(TS_FAILURE);

	if ((fsb.st_mode&S_IFMT) != S_IFCHR) 
		return(TS_FAILURE);

#ifdef _THREAD_SAFE
	if ((p = fastttyname(fsb.st_rdev, rbuf, len)) != NULL)
		return(0);
#else
	if ((p = fastttyname(fsb.st_rdev)) != NULL)
		return(p);
#endif

	if ((df = opendir(_PATH_DEV)) == NULL)
		return(TS_FAILURE);

	while ((db = readdir(df)) != NULL) {
		if (db->d_fileno != fsb.st_ino)
			continue;

		strcpy(rbuf, _PATH_DEV);
		strcat(rbuf, db->d_name);
		if (stat(rbuf, &tsb) < 0)
			continue;

		if (tsb.st_dev == fsb.st_dev && tsb.st_ino == fsb.st_ino) {
			closedir(df);
			return(TS_FOUND(rbuf));
		}
	}
	closedir(df);

	TS_ERROR(ENOENT);
	return(TS_FAILURE);
}
