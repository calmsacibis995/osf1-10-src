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
static char	*sccsid = "@(#)$RCSfile: rwho.c,v $ $Revision: 4.2.3.2 $ (DEC) $Date: 1993/10/11 19:06:32 $";
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
static char sccsid[] = "rwho.c	1.9  com/sockcmd/simple,3.1,9021 4/4/90 11:11:01";
*/
/* 
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1983 The Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice and this paragraph are
 * duplicated in all such forms and that any documentation,
 * advertising materials, and other materials related to such
 * distribution and use acknowledge that the software was developed
 * by the University of California, Berkeley.  The name of the
 * University may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */
/*
#ifndef lint
char copyright[] =
" Copyright (c) 1983 The Regents of the University of California.\n\
 All rights reserved.\n";
#endif

#ifndef lint
static char sccsid[] = "rwho.c	5.3 (Berkeley) 8/25/88";
#endif  not lint */

/*
 * rwho.c
 *
 *	Revision History:
 *
 * 16-Apr-91    Mary Walker
 *      allow user to provide username
 */

#include <sys/param.h>
#include <sys/dir.h>
#include <sys/file.h>
#include <protocols/rwhod.h>
#include <stdio.h>

DIR	*dirp;

struct	whod wd;
int	utmpcmp();
#define	NUSERS	1000
struct	myutmp {
	char	myhost[MAXHOSTNAMELEN];
	int	myidle;
	struct	outmp myutmp;
} myutmp[NUSERS];
int	nusers;

#include <nl_types.h>
#include <locale.h>
#include "rwho_msg.h" 
#define MSGSTR(n,s) NLcatgets(catd,MS_RWHO,n,s) 
nl_catd catd;

#define	WHDRSIZE	(sizeof (wd) - sizeof (wd.wd_we))
/* 
 * this macro should be shared with ruptime.
 */
#define	down(w,now)	((now) - (w)->wd_recvtime > 11 * 60)

#define STRCMP(a,b) (((a)[0] == (b)[0]) ? strcmp((a),(b)) : 1)
#define STRNCMP(a,b,c) (((a)[0] == (b)[0]) ? strncmp((a),(b),(c)) : 1)
#define HOSTCMP(a) (STRCMP(matchto[a], w->wd_hostname      ) != 0)
#define NAMECMP(a) (STRCMP(matchto[a], we->we_utmp.out_name) != 0)
#define LINECMP(a) (STRCMP(matchto[a], we->we_utmp.out_line) != 0)
#define HOSTNCMP(a) (STRNCMP(matchto[a], w->wd_hostname,       mstrlen[a]) != 0)
#define NAMENCMP(a) (STRNCMP(matchto[a], we->we_utmp.out_name, mstrlen[a]) != 0)
#define LINENCMP(a) (STRNCMP(matchto[a], we->we_utmp.out_line, mstrlen[a]) != 0)

char	*ctime(), *strcpy();
time_t	now;
int	aflg;

char *matchto[100];	/* allow 100 names to match */
int   mstrlen[100];	/* lengths of strings */
int   nmatch = 0;		/* how many to check for */

main(argc, argv)
	int argc;
	char **argv;
{
	extern char *optarg;
	extern int optind;
	int ch;
	struct direct *dp;
	int cc, width;
	register struct whod *w = &wd;
	register struct whoent *we;
	register struct myutmp *mp;
	int f, n, i;
	time_t time();

	setlocale(LC_ALL, "");
	catd = NLcatopen(MF_RWHO, NL_CAT_LOCALE);

	while ((ch = getopt(argc, argv, "a")) != EOF)
		switch((char)ch) {
		case 'a':
			aflg = 1;
			break;
		case '?':
		default:
			fprintf(stderr, MSGSTR(USAGE, "usage: rwho [-a]\n"));
			exit(1);
		}
	argc -= optind;
        argv += optind;
	while (argc > 0) {
	        matchto[nmatch] = *argv;
		mstrlen[nmatch] = strlen(*argv);
		nmatch++;
		argc--;
		argv++;
	}
	if (chdir(_PATH_RWHODIR) || (dirp = opendir(".")) == NULL) {
		perror(MSGSTR(RWHOCD, _PATH_RWHODIR)); /*MSG*/
		exit(1);
	}
	mp = myutmp;
	(void)time(&now);
	while (dp = readdir(dirp)) {
		if (dp->d_ino == 0 || strncmp(dp->d_name, "whod.", 5))
			continue;
		f = open(dp->d_name, O_RDONLY);
		if (f < 0)
			continue;
		cc = read(f, (char *)&wd, sizeof (struct whod));
		if (cc < WHDRSIZE) {
			(void) close(f);
			continue;
		}
		if (down(w,now)) {
			(void) close(f);
			continue;
		}
		cc -= WHDRSIZE;
		we = w->wd_we;
		for (n = cc / sizeof (struct whoent); n > 0; n--) {
			if (aflg == 0 && we->we_idle >= 60*60) {
				we++;
				continue;
			}
			if (nmatch > 0) {
				for (i=0; i<nmatch; i++) {
					/*
					 * Check if length of matchto is 8.
				 	 * If it is, the null has been 
					 * overwritten and a strcmp will 
					 * not match matchto. A strncmp is 
				         * needed only when mstrlen[i] >= 8
				 	 */
					if  (mstrlen[i] < 8 && matchto[i] &&
						 HOSTCMP(i)
						&& NAMECMP(i) && LINECMP(i)) {
							continue;
					}
					else if (HOSTNCMP(i) && NAMENCMP(i) &&
					 	LINENCMP(i)) {
							continue;
					}
					break;
				}
				if (i >= nmatch) {	/* no match */
					we++;
					continue;
				}
			}
			if (nusers >= NUSERS) {
				printf(MSGSTR(TOOMANYUSERS, "too many users\n")); /*MSG*/
				exit(1);
			}
			mp->myutmp = we->we_utmp; mp->myidle = we->we_idle;
			(void) strcpy(mp->myhost, w->wd_hostname);
			nusers++; we++; mp++;
		}
		(void) close(f);
	}
	qsort((char *)myutmp, nusers, sizeof (struct myutmp), utmpcmp);
	mp = myutmp;
	width = 0;
	for (i = 0; i < nusers; i++) {
		int j = strlen(mp->myhost) + 1 + strlen(mp->myutmp.out_line);
		if (j > width)
			width = j;
		mp++;
	}
	mp = myutmp;
	for (i = 0; i < nusers; i++) {
		char buf[BUFSIZ];
		(void)sprintf(buf, MSGSTR(HOSTNM, "%s:%s"), mp->myhost, mp->myutmp.out_line); /*MSG*/
		NLprintf(MSGSTR(TERM, "%-8.8s %-*s %.12s"), /*MSG*/
		   mp->myutmp.out_name,
		   width,
		   buf,
		   ctime((time_t *)&mp->myutmp.out_time)+4);
		mp->myidle /= 60;
		if (mp->myidle) {
			if (aflg) {
				if (mp->myidle >= 100*60)
					mp->myidle = 100*60 - 1;
				if (mp->myidle >= 60)
					printf(MSGSTR(IDLE, " %2d"), mp->myidle / 60); /*MSG*/
				else
					printf(MSGSTR(SPC3, "   ")); /*MSG*/
			} else
				printf(MSGSTR(SPC1, " ")); /*MSG*/
			printf(MSGSTR(IDLE2, ":%02d"), mp->myidle % 60); /*MSG*/
		}
		printf(MSGSTR(CRLF, "\n")); /*MSG*/
		mp++;
	}
	exit(0);
}

utmpcmp(u1, u2)
	struct myutmp *u1, *u2;
{
	int rc;

	rc = strncmp(u1->myutmp.out_name, u2->myutmp.out_name, 8);
	if (rc)
		return (rc);
	rc = strncmp(u1->myhost, u2->myhost, 8);
	if (rc)
		return (rc);
	return (strncmp(u1->myutmp.out_line, u2->myutmp.out_line, 8));
}
