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
static char	*sccsid = "@(#)$RCSfile: passwd.c,v $ $Revision: 4.3.10.3 $ (DEC) $Date: 1993/10/11 17:42:04 $";
#endif 
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0
 */
/*
 * Copyright (c) 1983 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

#ifndef lint
char copyright[] =
"@(#) Copyright (c) 1983 Regents of the University of California.\n\
 All rights reserved.\n";
#endif not lint
/*
 * Modification History
 *
 * 91/10/8 dlong - Numerous bug fixes.
 * 91/10/8 dlong - Print more meaningful warning when no hashed database.
 * 91/11/8 003 dlong - Special characters required within first 8 characters.
 *		   Also, warn if new password is longer than 8 characters.
 */

/*
 * Modify a field in the password file (either password, login shell, or
 * gecos field).  This program should be suid with an owner with write
 * permission on /etc/passwd.
 */

#include <sys/secdefines.h>
/************ SEC_BASE stuff moved to libsecurity siad routines 
#if SEC_BASE
#include <sys/security.h>
#include <prot.h>
#endif
**************/

#include <sys/types.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/resource.h>

#include <stdio.h>
#include <signal.h>
#include <pwd.h>
#include <ndbm.h>
#include <errno.h>
#include <strings.h>
#include <ctype.h>
#include <paths.h>
#include <limits.h>
#include <sia.h>

#ifdef NLS
#include <locale.h>
#endif

#ifdef MSG
#include "passwd_msg.h" 
nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_PASSWD,n,s) 
#else
#define MSGSTR(n,s) s
#endif

#if defined(KJI) || defined(NLS)
#include <NLchar.h>
#include <NLctype.h>
#endif


/*
 * This should be the first thing returned from a getloginshells()
 * but too many programs know that it is /sbin/sh.
 *
 * Note:  The default shell as determined by <paths.h>, and as used in
 *        login(1) is /bin/sh, not /sbin/sh.  /sbin/sh might not be in
 *        /etc/shells.  -dlong 91/10/8
 */

#define	DEFSHELL	_PATH_BSHELL

#define	PASSWD		"/etc/passwd"
#define	PTEMP		"/etc/ptmp"

/************ SEC_BASE stuff moved to libsecurity siad routines 
#if SEC_BASE
extern void passwd_auth();
extern priv_t *privvec();
#endif
**************/

#ifdef __alpha
extern struct passwd *getpwuid_local(uid_t);
extern struct passwd *getpwnam_local(char *);
#endif

#define	EOS		'\0';


main(argc, argv)
	int	argc;
	char	**argv;
{
	extern char	*optarg;
	extern int	errno, optind;
	int argcsav=argc;
	char **argvsav=argv;
	int (*sia_collect)()=sia_collect_trm;
	struct passwd	*pwd;
	DBM	*dp;
	uid_t	uid, getuid();
	int	i, acctlen, ch, fd, docmnd;
	static  char username[80];
	char	*p, *str, *cp, *uname, *progname, *umsg, *getpwline(),
		*getfingerinfo(), *getloginshell(), *getnewpasswd();
	int status=0;		/* DAL001 */

#ifdef NLS
	(void) setlocale( LC_ALL, "" );
#endif

#ifdef MSG
	catd = catopen(MF_PASSWD,NL_CAT_LOCALE);
#endif

	progname = (cp = rindex(*argv, '/')) ? cp + 1 : *argv;
	docmnd = 0; /* 0 => login, 1 => chfn, 2 => chsh */
	if (!strcmp(progname, "chfn")) {
		docmnd = 1;
		umsg = MSGSTR(CHFNU, "usage: chfn [username]\n");
	}
	else if (!strcmp(progname, "chsh")) {
		docmnd = 2;
		umsg = MSGSTR(CHSHU, "usage: chsh [username]\n");
	}
	else
		umsg = MSGSTR(USAGE, "usage: passwd [-fs] [username]\n");

	while ((ch = getopt(argc, argv, "fs")) != EOF)
		switch((char)ch) {
		case 'f':
			if (docmnd == 2)
				goto usage;
			docmnd = 1;
			break;
		case 's':
			if (docmnd == 1)
				goto usage;
			docmnd = 2;
			break;
		case '?':
		default:
usage:			fputs(umsg, stderr);
			exit(2);			/* DAL001 */
		}
	/* setup username argument */
	if (argc - optind < 1) 
		{
		pwd=getpwuid(getuid());
                if(pwd == NULL)
			{
			fprintf(stderr, MSGSTR(UNKUID, "%s: %u: unknown user uid.\n"),
				progname, getuid());	/* DAL001 */
			exit(1);
			}
		else	{
			uname=username;
			strncpy(uname,pwd->pw_name,sizeof username);	/* DAL001 */
			}
		}
	else	uname = *(argv + optind);
	switch(docmnd) {
		case 0:		status = sia_chg_password(sia_collect,uname,argcsav,argvsav);
				break;
		case 1:		status = sia_chg_finger(sia_collect,uname,argcsav,argvsav);
				break;
		case 2:		status = sia_chg_shell(sia_collect,uname,argcsav,argvsav);
				break;
		}
	if(status != SIASUCCESS)	/* DAL001 */
		exit(1);		/* DAL001 */
	else				/* DAL001 */
		exit(0);		/* DAL001 */
}






