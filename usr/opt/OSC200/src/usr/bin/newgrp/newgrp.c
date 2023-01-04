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
static char rcsid[] = "@(#)$RCSfile: newgrp.c,v $ $Revision: 4.2.12.5 $ (DEC) $Date: 1993/12/21 20:09:40 $";
#endif
/*
 * HISTORY
 */
/*
 * OSF/1 1.2
 */
/*
 * newgrp.c	5.1 17:50:46 8/15/90 SecureWare
 *
 * newgrp [group]
 *
 * rules
 *	if no arg, group id in password file is used
 *	else if group id == id in password file
 *	else if login name is in member list
 *	else if password is present and user knows it
 *	else too bad
 */
#include <sys/secdefines.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pwd.h>
#include <grp.h>
#include <locale.h>
#include <paths.h>
#include <sys/access.h>

#if SEC_BASE
#include <sys/security.h>

extern priv_t *privvec();
#endif

#include "newgrp_msg.h"
#define MSGSTR(Num, Str) catgets(catd,MS_NEWGRP,Num,Str)

nl_catd	catd;

#define PATH	"PATH=/usr/bin:/usr/bin:"
#define SUPATH	"PATH=/usr/bin:/usr/bin:/usr/sbin:/sbin"
#define ELIM	128

char	PW[] = "Password: ";
char	NG[] = "Sorry";
char	PD[] = "Permission denied";
char	UG[] = "Unknown group";
char	NS[] = "You have no shell";

struct	group *getgrnam();
struct	passwd *getpwnam();
char	*cuserid();
char	*getpass();
char	*crypt();

char homedir[64]="HOME=";
char logname[20]="LOGNAME=";


char *envinit[ELIM];
extern char **environ;
char *path=PATH;
char *supath=SUPATH;


main(argc,argv)
char *argv[];
{
	register struct passwd *p;
	char *rname();
	int eflag = 0;
	int uid;
	char *shell, *dir, *name;
	int c,i;
#if SEC_BASE
	privvec_t	saveprivs;
#endif


        (void) setlocale(LC_ALL,"");
	(void) catopen( MF_NEWGRP, NL_CAT_LOCALE);
#if SEC_BASE
	set_auth_parameters(argc, argv);
	initprivs();
#endif
#ifdef	DEBUG
	chroot(".");
#endif
#if SEC_BASE
	if ((p = getpwuid(getluid())) == NULL)
#else
	if ((p = getpwuid(getuid())) == NULL)
#endif
		error(MSGSTR(NGMSG,NG));
	endpwent();
	/*
	 * Replace obsolete option '-' with '-l'
	 */
	for (i=1; i < argc; i++)
		if (strcmp(argv[i], "-") == 0)
			argv[i] = "-l";

	while ((c = getopt(argc, argv, "l")) != -1)
		switch (c) {
			case 'l':
				eflag++;
				break;
			default:	/* CMR001 */
				warn(MSGSTR(USAGE, 
					"Usage: newgrp [-l] [group]"));
				break;
		}
	argc -= optind;
	argv += optind;

	if (argc > 0)
		p->pw_gid = chkgrp(argv[0], p);

	uid = p->pw_uid;
	dir = strcpy(malloc(strlen(p->pw_dir)+1),p->pw_dir);
	name = strcpy(malloc(strlen(p->pw_name)+1),p->pw_name);
#if SEC_BASE
	if (forceprivs(privvec(SEC_SETPROCIDENT, -1), saveprivs))
		error(MSGSTR(INSUFF, "newgrp: insufficient privileges"));
#endif
	if (setgid(p->pw_gid) < 0 || setuid(getuid()) < 0)
		error(MSGSTR(NGMSG,NG));
#if SEC_BASE
	seteffprivs(saveprivs, (priv_t *) 0);
#endif
	if (!*p->pw_shell)
		p->pw_shell = _PATH_BSHELL;
	if(eflag){
		char *simple;

		strcat(homedir, dir);
		strcat(logname, name);
		envinit[2] = logname;
		chdir(dir);
		envinit[0] = homedir;
		if (uid == 0)
			envinit[1] = supath;
		else
			envinit[1] = path;
		envinit[3] = NULL;
		environ = envinit;
		shell = strcpy(malloc(sizeof(p->pw_shell + 2)), "-");
		shell = strcat(shell,p->pw_shell);
		simple = strrchr(shell,'/');
		if(simple){
			*(shell+1) = '\0';
			shell = strcat(shell,++simple);
		}
	}
	else {
		if(shell=getenv("SHELL"))
			if(access(shell, X_OK) == 0)
				p->pw_shell = shell;
		shell = strrchr(p->pw_shell, '/');
		if(!shell)
			shell = p->pw_shell;
		else
			shell++;
	}

	execl(p->pw_shell,shell, NULL);
	error(MSGSTR(NSMSG,NS));
}

warn(s)
char *s;
{
	fprintf(stderr, "%s\n", s);
}

error(s)
char *s;
{
	warn(s);
	exit(1);
}

chkgrp(gname, p)
char	*gname;
struct	passwd *p;
{
	register char **t;
	register struct group *g;

	if (isdigit(*gname)) {
		int gnumber = atoi(gname);

	/* DS001 Fencepost error. Set "if (gnumber > 0)" to
	   cover the zero. "if (gnumber >+ 0)". */

		if (gnumber >= 0)
			g = getgrgid(gnumber);
	} else
		g = getgrnam(gname);

	endgrent();
	if (g == NULL) {
		warn(MSGSTR(UGMSG,UG));
		return getgid();
	}
#if SEC_BASE
	if (p->pw_gid == g->gr_gid)
#else
	if (p->pw_gid == g->gr_gid || getuid() == 0)
#endif
		return g->gr_gid;
	for (t = g->gr_mem; *t; ++t)
		if (strcmp(p->pw_name, *t) == 0)
			return g->gr_gid;
#if SEC_BASE
	if (authorized_user("anygroup"))
		return g->gr_gid;
#endif
	if (*g->gr_passwd) {
		if (!isatty(fileno(stdin)))
			error(MSGSTR(PDMSG,PD));
#if SEC_BASE
		if (newgrp_good_password(g->gr_passwd, PW))
#else
		if (strcmp(g->gr_passwd, crypt(getpass(PW), g->gr_passwd)) == 0)
#endif
			return g->gr_gid;
	}
	warn(MSGSTR(NGMSG,NG));
	return getgid();
}
/*
 * return pointer to rightmost component of pathname
 */
char *rname(pn)
char *pn;
{
	register char *q;

	q = pn;
	while (*pn)
		if (*pn++ == '/')
			q = pn;
	return q;
}
