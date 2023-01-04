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
static char	*sccsid = "@(#)$RCSfile: ruptime.c,v $ $Revision: 4.2.3.2 $ (DEC) $Date: 1993/10/11 19:06:27 $";
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

/* static char sccsid[] = "ruptime.c	1.12  com/sockcmd/simple,3.1,9021 4/4/90 11:10:33"; */
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
static char sccsid[] = "ruptime.c	5.5 (Berkeley) 8/25/88";
#endif  not lint */

/*
 * ruptime.c
 *
 *	Revision History:
 *
 * 16-Apr-91    Mary Walker
 *      - allow user to provide hostname
 *      - allow any number of hosts to be display (no more "too many hosts")
 *      - changed format to vary depending on longest host name.
 */
#include <sys/param.h>
#include <sys/dir.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <netdb.h>
#include <protocols/rwhod.h>
#include <stdio.h>

#include <nl_types.h>
#include <locale.h>
#include "ruptime_msg.h" 
nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_RUPTIME,n,s) 

#define	NHOSTS	100
int	nhosts;
struct hs {
	struct	whod *hs_wd;
	int	hs_nusers;
} *hs;

struct	whod awhod;

#define	WHDRSIZE	(sizeof (awhod) - sizeof (awhod.wd_we))
#define	down(h)		(now - (h)->hs_wd->wd_recvtime > 11 * 60)

time_t	now;
int	rflg = 1;
int	hscmp(), ucmp(), lcmp(), tcmp();

main(argc, argv)
	int argc;
	char **argv;
{
	extern char *optarg;
	extern int optind;
	register struct hs *hsp;
	register struct whod *wd;
	register struct whoent *we;
	register DIR *dirp;
	struct direct *dp;
	int aflg, cc, ch, f, i, maxloadav;
	char buf[sizeof(struct whod)];
	int (*cmp)() = hscmp;
	time_t time();
	char *interval(), *malloc();
	char *upmsg, *dnmsg;
	char *matchto = NULL;
	char file_name[256];
	struct stat *statb;
	struct hostent *host_info;
	int fnd;
	int j;
	int width;
	int sizehs = NHOSTS;

	char  *msg;   /* added to pass msg from msg catalog */
	setlocale(LC_ALL, "");
	catd = NLcatopen(MF_RUPTIME, NL_CAT_LOCALE);

	aflg = 0;
	maxloadav = -1;
	while ((ch = getopt(argc, argv, "alrut")) != EOF)
		switch((char)ch) {
		case 'a':
			aflg = 1;
			break;
		case 'l':
			cmp = lcmp;
			break;
		case 'r':
			rflg = -1;
			break;
		case 't':
			cmp = tcmp;
			break;
		case 'u':
			cmp = ucmp;
			break;
		default: 
			NLfprintf(stderr, MSGSTR(USEAGE, "Usage: ruptime [-arlut]\n")); /*MSG*/
			exit(1);
		}
	argc -= optind;
        argv += optind;
	if (argc > 0) {
	        matchto = *argv;

		/* allocate memory for statb */
		if ((statb = (struct stat *)malloc(sizeof (struct stat))) == NULL) {
			NLfprintf(stderr, MSGSTR(NOMEM, "Can't allocate memory\n")); /*MSG*/
			exit(1);
		}

		(void) sprintf (file_name, "%s/whod.%s", _PATH_RWHODIR, matchto);
		if ((fnd = stat (file_name,statb)) < 0) {

			/* get the official host name */
			if ((host_info = gethostbyname(matchto)) == NULL){
			   exit (1);
			}
			/* stat the file required using host_info*/
			(void) sprintf (file_name, "%s/whod.%s", 
					_PATH_RWHODIR,
					host_info->h_name);
			if ((fnd = stat (file_name,statb)) < 0) {
			        for (i=0; host_info->h_aliases[i] != NULL; i++){
				        (void) sprintf(file_name, 
						       "%s/whod.%s",
						       _PATH_RWHODIR, 
						       host_info->h_aliases[i]);
					if ((fnd = stat (file_name, statb)) < 0) {
					           continue;
					} else {
					           break; /* found it */
					}
				}
			}                             
		}
		if (fnd < 0) {
			exit(1);
		}
		argc--;
		argv++;
	}

	if (chdir(_PATH_RWHODIR) || (dirp = opendir(".")) == NULL) {
		perror(MSGSTR(RWHODERR, _PATH_RWHODIR)); /*MSG*/
		exit(1);
	}
	hs = hsp = (struct hs *) (malloc((unsigned)sizehs * sizeof(struct hs)));
	if (hs == NULL) {
		perror(MSGSTR(NOMEM, "Can't allocate memory\n"));
		exit(1);
	}
	while (dp = readdir(dirp)) {
		if (dp->d_ino == 0 || strncmp(dp->d_name, "whod.", 5))
			continue;
		if (nhosts == sizehs) {
			sizehs += NHOSTS;
			hs = (struct hs *)(realloc((char *)hs, (unsigned)
			    sizehs * sizeof(struct hs)));
			if (hs == NULL) {
			        perror(MSGSTR(TOOMANYHOSTS,"too many hosts\n")); /*MSG*/
				exit(1);
			}
			hsp = &hs[nhosts];
		}
		if (matchto != NULL) { /* only need to open one file */
			f = open (file_name, O_RDONLY, 0);
		} else { 
			f = open(dp->d_name, O_RDONLY, 0);
		}

		if (f > 0) {
			cc = read(f, buf, sizeof(struct whod));
			if (cc >= WHDRSIZE) {
				/* NOSTRICT */
				hsp->hs_wd = (struct whod *)malloc(WHDRSIZE);
				wd = (struct whod *)buf;
				bcopy(wd, hsp->hs_wd, WHDRSIZE);
				hsp->hs_nusers = 0;
				for (i = 0; i < 2; i++)
					if (wd->wd_loadav[i] > maxloadav)
						maxloadav = wd->wd_loadav[i];
				we = (struct whoent *)(buf+cc);
				while (--we >= wd->wd_we)
					if (aflg || we->we_idle < 3600)
						hsp->hs_nusers++;
				nhosts++; hsp++;
			}
			(void)close(f);
		}
		if (matchto != NULL)
		         break;
	}
	closedir (dirp);
	if (!nhosts) {
		if (matchto != NULL) {
			exit(0);
		} 
		else {
		        printf(MSGSTR(NOHOSTS, "ruptime: no hosts!?!\n")); /*MSG*/
			exit(1);
		}
	}
	(void)time(&now);
	qsort((char *)hs, nhosts, sizeof (hs[0]), cmp);

	/*
	 *  find the longest hostname (for formatting)
	 */
	hsp = &hs[0];
	width = 0;
	for (i = 0; i < nhosts; i++, hsp++) {
		j = strlen(hsp->hs_wd->wd_hostname) + 1;
		if (j > width)
			width = j;
	}

#ifdef MSG
	msg = NLcatgets(catd,MS_RUPTIME,UPLAB,"  up");
	upmsg = malloc(strlen(msg)+1);		/* need to copy from catalog static */
	(void) NLstrcpy(upmsg,msg);			/* string area to our own  */
	msg = NLcatgets(catd,MS_RUPTIME,DNLAB,"down");
	dnmsg = malloc(strlen(msg)+1);
	(void) NLstrcpy(dnmsg,msg);
#else
	upmsg = "  up";
	dnmsg = "down";
#endif 
	for (i = 0; i < nhosts; i++) {
		hsp = &hs[i];
		if (down(hsp)) {
			NLprintf(MSGSTR(DNTIM, "%-*.*s%s\n"), width, width,
			hsp->hs_wd->wd_hostname,
		    	interval(now - hsp->hs_wd->wd_recvtime,dnmsg)); /*MSG*/
			continue;
		}
		NLprintf(MSGSTR(UPTIME, "%-*.*s%s,  "), width, width, 
                    hsp->hs_wd->wd_hostname, 
		    interval(hsp->hs_wd->wd_sendtime -
				hsp->hs_wd->wd_boottime,upmsg)); /*MSG*/
		if(hsp->hs_nusers == 1)
			NLprintf(MSGSTR(USER1, "%4d user, "),hsp->hs_nusers); /*MSG*/
		else
			NLprintf(MSGSTR(USER2, "%4d users,"),hsp->hs_nusers);	     /*MSG*/
		NLprintf(MSGSTR(LOAD, "  load %*.2f, %*.2f, %*.2f\n"),  /*MSG*/
		    maxloadav >= 1000 ? 5 : 4, hsp->hs_wd->wd_loadav[0] / 100.0,
		    	maxloadav >= 1000 ? 5 : 4, hsp->hs_wd->wd_loadav[1] / 100.0,
		    		maxloadav >= 1000 ? 5 : 4,hsp->hs_wd->wd_loadav[2] / 100.0);
		cfree(hsp->hs_wd);
	}
	exit(0);
}

char *
interval(tval, updown)
	time_t tval;
	char *updown;
{
	static char resbuf[256];
	int days, hours, minutes;

	if (tval < 0 || tval > 365*24*60*60) {
		(void) NLsprintf(resbuf,MSGSTR(RAWTIM, "   %s ??:??"), updown); /*MSG*/
		return(resbuf);
	}
	minutes = (tval + 59) / 60;		/* round to minutes */
	hours = minutes / 60; minutes %= 60;
	days = hours / 24; hours %= 24;
	if (days)
		(void) NLsprintf(resbuf, MSGSTR(DYHRMN, "%s %2d+%02d:%02d"),
		    updown, days, hours, minutes);
	else
		(void) NLsprintf(resbuf, MSGSTR(HRMIN, "%s    %2d:%02d"), 
		    updown, hours, minutes);
	return(resbuf);
}

hscmp(h1, h2)
	struct hs *h1, *h2;
{
	return(rflg * strcmp(h1->hs_wd->wd_hostname, h2->hs_wd->wd_hostname));
}

/*
 * Compare according to load average.
 */
lcmp(h1, h2)
	struct hs *h1, *h2;
{
	if (down(h1))
		if (down(h2))
			return(tcmp(h1, h2));
		else
			return(rflg);
	else if (down(h2))
		return(-rflg);
	else
		return(rflg *
			(h2->hs_wd->wd_loadav[0] - h1->hs_wd->wd_loadav[0]));
}

/*
 * Compare according to number of users.
 */
ucmp(h1, h2)
	struct hs *h1, *h2;
{
	if (down(h1))
		if (down(h2))
			return(tcmp(h1, h2));
		else
			return(rflg);
	else if (down(h2))
		return(-rflg);
	else
		return(rflg * (h2->hs_nusers - h1->hs_nusers));
}

/*
 * Compare according to uptime.
 */
tcmp(h1, h2)
	struct hs *h1, *h2;
{
	return(rflg * (
		(down(h2) ? h2->hs_wd->wd_recvtime - now
			  : h2->hs_wd->wd_sendtime - h2->hs_wd->wd_boottime)
		-
		(down(h1) ? h1->hs_wd->wd_recvtime - now
			  : h1->hs_wd->wd_sendtime - h1->hs_wd->wd_boottime)
	));
}
