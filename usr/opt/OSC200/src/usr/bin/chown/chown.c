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
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
#if !defined(lint) && !defined(_NOIDENT)
static char rcsid[] = "@(#)$RCSfile: chown.c,v $ $Revision: 4.2.8.5 $ (DEC) $Date: 1993/11/03 19:46:53 $";
#endif
/*
 * HISTORY
 */
/*
 * OSF/1 1.2
 */
/*
 * COMPONENT_NAME:  (CMDSDAC) security: access control
 *
 * FUNCTIONS: chown
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. date 1, date 2
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.

 * Copyright (c) 1984 AT&T	
 * All Rights Reserved  
 *
 * THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	
 * The copyright notice above does not evidence any   
 * actual or intended publication of such source code.
 *
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 * 
 * chown.c	1.15  com/cmd/oper,3.1,9013 2/21/90 14:06:43
 * chown.c	4.1 23:49:01 7/12/90 SecureWare 
 */

#include <sys/secdefines.h>
#if SEC_BASE
#include <sys/security.h>
#endif

#include <errno.h>
#include <stdio.h>
#include <locale.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pwd.h>
#include <grp.h>
#include <dirent.h>
#include "chown_msg.h"
nl_catd catd;
#define	MSGSTR(Num, Str) catgets(catd,MS_CHOWN,Num,Str)

extern char *optarg;
extern int optind;

struct	passwd	*pwd;
struct	group	*grp;
struct	stat	stbuf;
uid_t	newuid;
gid_t   newgid=-1;		/* if not specified, leave gid alone */
int     fflag = 0,Rflag = 0;
int	status = 0;
char	*arg;
#define	CHOWN_ERROR	(4)

/*
 * NAME: chown [-fR] uid[:group] file ...
 * FUNCTION: changes the owner of the files to uid
 *    FLAGS
 *      -f   Suppresses all error reporting
 *      -R   Causes chgrp to descend recursively through its directory 
 *           arguments, setting the specified group ID.  When symbolic links 
 *           are encountered, their group is changed, but they are not 
 *           traversed.
 */
main(argc, argv)
char *argv[];
{
	register c;

	(void ) setlocale(LC_ALL,"");
	catd = catopen(MF_CHOWN,NL_CAT_LOCALE);

	if(argc < 3) {
		fprintf(stderr, MSGSTR(USAGE,
			"usage: chown [-fR] username[:groupname] file...\n"));
		catclose(catd);
		exit(CHOWN_ERROR);
	}
	while ((c = getopt(argc,argv,"fR")) != EOF) {
		switch(c) {
			case 'f':
				fflag++;	
				break;
			case 'R':
				Rflag++;	
				break;
			case '?':
				fprintf(stderr, MSGSTR(USAGE,
					"usage: chown [-fR] username[:groupname] file...\n"));
				catclose(catd);
				exit(CHOWN_ERROR);
				break;
		}
	}

	parseug(argv[optind], &newuid, &newgid);

	optind++;

	if( optind == argc) {
		fprintf(stderr, MSGSTR(USAGE,
			"usage: chown [-fR] username[:groupname] file...\n"));
		catclose(catd);
		exit(CHOWN_ERROR);
	}

#if SEC_BASE
	set_auth_parameters(argc, argv);
	initprivs();

	if (authorized_user("chown") && !forcepriv(SEC_CHOWN)) {
		fprintf(stderr, MSGSTR(PRIV, "chown: insufficient privileges\n"));
		catclose(catd);
		exit(CHOWN_ERROR);
	}
#endif

	for(; optind<argc; optind++) {
		/* do stat for directory arguments */
		if (lstat(argv[optind], &stbuf) < 0) {
			status = Perror(argv[optind]);
			continue;
		}
		if (Rflag && ((stbuf.st_mode&S_IFMT) == S_IFDIR)) {
			if (chownr(argv[optind], newuid, newgid))
				status = CHOWN_ERROR;
			continue;
		}
#if SEC_BASE
		disablepriv(SEC_SUSPEND_AUDIT);
#endif
		if (Rflag && S_ISLNK(stbuf.st_mode)) {
			if (lchown(argv[optind], newuid, newgid)) {
#if SEC_BASE
				forcepriv(SEC_SUSPEND_AUDIT);
#endif
				status = Perror(argv[optind]);
				continue;
			}
		} else if (chown(argv[optind], newuid, newgid)) {
#if SEC_BASE
			forcepriv(SEC_SUSPEND_AUDIT);
#endif
			status = Perror(argv[optind]);
			continue;
		}
#if SEC_BASE
		forcepriv(SEC_SUSPEND_AUDIT);
#endif
	}
	catclose(catd);
	exit(status);
}

/*
 * NAME: isnumber
 * FUNCTION: determines if argument is a number 
 */
isnumber(s)
char *s;
{
	register c;

	while(c = *s++)
		if(!isdigit(c))
			return(0);
	return(1);
}

/*
 * NAME: chownr
 * FUCNTION: reccursively descend the directory and change the owner 
 *					and group, if specified, for each file.
 * 
 * NOTE: This code is essentially duplicated in chgrp.c.  
 *	 Any changes made here will probably need to be made there too!
 */
chownr(dir, uid, gid)
char *dir;
uid_t uid;
gid_t gid;
{
	DIR *dirp;
	struct dirent *dp;
	struct stat st;
	char savedir[PATH_MAX+1];
	int ecode = 0;
	extern char *getwd();

	if (getwd(savedir) == (char *)0) {
		fprintf(stderr,MSGSTR(NODIR,
			"chown: cannot get directory path %s\n"), savedir);
		catclose(catd);
		exit(CHOWN_ERROR);
	}
	/*
	 * Change what we are given before doing it's contents.
	 */
#if SEC_BASE
	disablepriv(SEC_SUSPEND_AUDIT);
#endif
	if (chown(dir, uid, gid) < 0 ) {
#if SEC_BASE
		forcepriv(SEC_SUSPEND_AUDIT);
#endif
		ecode = Perror(dir);
	}
#if SEC_BASE
	forcepriv(SEC_SUSPEND_AUDIT);
#endif
	if (chdir(dir) < 0) {
		Perror(dir);
		return (CHOWN_ERROR);
	}
	if ((dirp = opendir(".")) == NULL) {
		Perror(dir);
		return (CHOWN_ERROR);
	}
	for (dp = readdir(dirp); dp != NULL; dp = readdir(dirp)) {
		if (!strcmp(dp->d_name, ".") ||
		    !strcmp(dp->d_name, ".."))
			continue;
		if (lstat(dp->d_name, &st) < 0) {
			ecode = Perror(dp->d_name);
			continue;
		}
		if ((st.st_mode&S_IFMT) == S_IFDIR) {
			/*
			 * do not overwrite ecode w/ zero 
			 * and continue on regardless of errors
			 */
			if (chownr(dp->d_name, uid, gid))
				ecode = CHOWN_ERROR;
			continue;
		}
#if SEC_BASE
		disablepriv(SEC_SUSPEND_AUDIT);
#endif
		if (Rflag && S_ISLNK(st.st_mode)) {
			if (lchown(dp->d_name, uid, gid) < 0 ) {
#if SEC_BASE
				forcepriv(SEC_SUSPEND_AUDIT);
#endif
				ecode = Perror(dp->d_name);
				continue;
			}
		} else if (chown(dp->d_name, uid, gid) < 0 ) {
#if SEC_BASE
			forcepriv(SEC_SUSPEND_AUDIT);
#endif
			ecode = Perror(dp->d_name);
			continue;
		}
#if SEC_BASE
		forcepriv(SEC_SUSPEND_AUDIT);
#endif
	}
	closedir(dirp);
	if (chdir(savedir) < 0) {
		fprintf(stderr, MSGSTR(NOBACK, 
			"chown: can not change directory back to %s"), savedir);
		catclose(catd);
		exit(CHOWN_ERROR);
	}
	return (ecode);
}

/*
 * NAME: Perror
 * FUNCTION: display perror messages
 */
Perror(s)
	char *s;
{

	if (!fflag) {
		fprintf(stderr, MSGSTR(NAME,"chown: "));
		perror(s);
	}
	return (CHOWN_ERROR);
}


/*
 * Specify both user and group 
 */
parseug(arg, uidp, gidp)
char	*arg;
uid_t	*uidp;
gid_t	*gidp;
{
	char	*gidstr;

	for (gidstr=arg; *gidstr; gidstr++)
                if (*gidstr == ':' | *gidstr == '.')            /*GA001*/
			break;

	if (*gidstr)
	{
		*gidstr = '\0';
		if ((gidstr == arg)	||	/* if ':' at beginning */
		    (*++gidstr == '\0') ||	/* or ':' at the end */
                    (strchr(gidstr, ':')) ||    /* or multiple ':'s: error */
                    (strchr(gidstr, '.')))      /* or multiple '.'s: error */
                {                                               /*GA001*/
			fprintf(stderr, MSGSTR(USAGE,
				"usage: chown [-fR] username[:groupname] file...\n"));
			catclose(catd);
			exit(CHOWN_ERROR);
		}
	}
	else
		gidstr = NULL;

	errno = 0;
	if ((pwd=getpwnam(arg)) != NULL) {
		*uidp = pwd->pw_uid;
	} else if (isnumber(arg)) {
		char *eptr;
		*uidp = strtoul(arg, &eptr, 10);
		if (*uidp == 0 && eptr == arg) {
		    /* conversion failed. */
		    exit(Perror("strtoul"));
		}
	} else 
	{
		fprintf(stderr, MSGSTR(BADUID, 
			"chown: unknown username %s\n"), arg);
		catclose(catd);
		exit(CHOWN_ERROR);
	}

	if (gidstr)
	{
		errno = 0;
		if ((grp=getgrnam(gidstr)) != NULL)
			*gidp = grp->gr_gid;
		else if (isnumber(gidstr)) 
			*gidp = strtoul(gidstr, NULL, 10);
		else 
		{
			if (errno)
				exit(Perror("getgrnam"));
			fprintf(stderr, MSGSTR(BADGID, 
				"chown: unknown groupname %s\n"),gidstr);
			catclose(catd);
			exit(CHOWN_ERROR);
		}
	}
}
