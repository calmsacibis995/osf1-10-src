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
static char	*sccsid = "@(#)$RCSfile: mkpasswd.c,v $ $Revision: 4.2.5.2 $ (DEC) $Date: 1993/10/08 15:46:34 $";
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
 * COMPONENT_NAME: mkpasswd - create hash look-aside password file
 *
 * FUNCTIONS: main
 *
 * ORIGINS: 26,24,27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 * OBJECT CODE ONLY SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1983 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

char copyright[] =
"@(#) Copyright (c) 1980 Regents of the University of California.\n\
 All rights reserved.\n";

#include <sys/secdefines.h>
#if SEC_BASE
#include <sys/security.h>
#endif

#include <sys/file.h>
#include <sys/stat.h>
#include <stdio.h>
#include <pwd.h>
#include <ndbm.h>

#ifdef NLS
#include <locale.h>
#endif

#ifdef MSG
#include "mkpasswd_msg.h" 
nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_MKPASSWD,n,s) 
#else
#define MSGSTR(n,s) s
#endif

#if defined(KJI) || defined(NLS)
#include <NLchar.h>
#include <NLctype.h>
#endif

#if SEC_BASE
extern priv_t   *privvec();
#endif

char	buf[BUFSIZ];



main(argc, argv)
int	argc;
char	*argv[];
{
	DBM	*dp;
	FILE	*fp;
	datum	key;
	datum	content;
	register char	*cp, *tp;
	struct	passwd	*pwd;
	int	verbose = 0;
	int	entries = 0;
	int	maxlen = 0;
	struct stat statbuf;		/* DAL001 */

	/*
	 * The only valid option is `verbose'.  Test for it and
	 * shift the remaining arguments if it is present.
	 */

#ifdef NLS
	setlocale( LC_ALL, "" );
#endif

#ifdef MSG
	catd = catopen(MF_MKPASSWD,NL_CAT_LOCALE);
#endif

#if SEC_BASE
        set_auth_parameters(argc, argv);
        initprivs();
#endif

	if (argc > 1 && strcmp(argv[1], "-v") == 0) {
		verbose++;
		argv++, argc--;
	}

	/*
	 * There can be no remaining arguments.  The only option
	 * left is the name of the look-aside file.
	 */

	if (argc != 2) {
		fprintf (stderr, MSGSTR(USAGE, "usage: mkpasswd [ -v ] file\n"));
		exit (1);
	}

#if SEC_BASE
        if (!authorized_user("auth")) {
                fprintf(stderr, "mkpasswd: need auth authorization\n");
                exit(1);
        }
#endif

	/*
	 * See if the file exists already.  If the file is non-existent
	 * or can't be read, tell the user what's wrong.
	 */

	if (access (argv[1], R_OK) < 0) {
		fprintf (stderr, MSGSTR(MKPW, "mkpasswd: "));
		perror (argv[1]);
		exit (1);
	}

#if SEC_BASE
        if (forceprivs(privvec(SEC_ALLOWDACACCESS,
#if SEC_MAC
                                SEC_ALLOWMACACCESS,
#endif
#if SEC_ILB
                                SEC_ILNOFLOAT,
#endif
#if SEC_NCAV
                                SEC_ALLOWNCAVACCESS,
#endif
                                -1), (priv_t *) 0)) {
                fprintf(stderr, "mkpasswd: insufficient privileges\n");
                exit(1);
        }
        disablepriv(SEC_SUSPEND_AUDIT);
#endif

	umask(0);
/*
 * If outdated database files exist, delete them.  DAL001
 */
	strcpy(buf, argv[1]);			/* DAL001 */
	strcat(buf, ".pag");			/* DAL001 */
	if(stat(buf, &statbuf) == 0) {		/* DAL001 */
		long mtime;			/* DAL001 */

		mtime = statbuf.st_mtime;		/* DAL001 */
		if(stat(argv[1], &statbuf) < 0) {	/* DAL001 */
			perror(argv[1]);	/* DAL001 */
			exit(2);		/* DAL001 */
		}				/* DAL001 */
		if(mtime > statbuf.st_mtime) {	/* DAL001 */
			if(verbose)		/* DAL001 */
				fputs(MSGSTR(SYNC, "The database is already up to date."), stderr);		/* DAL001 */
			exit(0);		/* DAL001 */
		} else {		/* DAL001 */
			unlink(buf);		/* DAL001 */
			strcpy(buf, argv[1]);	/* DAL001 */
			strcat(buf, ".dir");	/* DAL001 */
			unlink(buf);		/* DAL001 */
		}			/* DAL001 */
	}				/* DAL001 */

	dp = dbm_open(argv[1], O_WRONLY|O_CREAT|O_EXCL, 0644);
	if (dp == NULL) {
		fprintf (stderr, MSGSTR(MKPW, "mkpasswd: "));
		perror (argv[1]);
		exit(1);
	}


        setpwfile(argv[1]);

	/*
	 * Read the entire file and cram all of the fields into a
	 * single DBM block.  Strings a NUL separated from each
	 * other.  Binary objects, such as uids and gids, are
	 * stored directly in an unaligned fashion.
	 */

	while (pwd = getpwent_local ()) {

		int i;

		cp = buf;

		tp = pwd->pw_name; while (*cp++ = *tp++);

		tp = pwd->pw_passwd; while (*cp++ = *tp++);

		i = pwd->pw_uid;
		bcopy ((char *)&i, cp, sizeof i);
		cp += sizeof i;

		i = pwd->pw_gid;
		bcopy ((char *)&i, cp, sizeof i);
		cp += sizeof i;

		i = pwd->pw_quota;
		bcopy ((char *)&i, cp, sizeof i);
		cp += sizeof i;

		tp = pwd->pw_comment; while (*cp++ = *tp++);

		tp = pwd->pw_gecos; while (*cp++ = *tp++);

		tp = pwd->pw_dir; while (*cp++ = *tp++);

		tp = pwd->pw_shell; while (*cp++ = *tp++);
		/*
		 * Tell the user what we are up to ...
		 */

		if (verbose)
			fprintf(stdout,MSGSTR(STORE, "store %s, uid %d\n"), pwd->pw_name, pwd->pw_uid);

		/*
		 * Build the values for the key and content structures
		 * to be handed to DBM.  Each record is stored twice -
		 * once by its name and again by its UID.
		 */

		content.dptr = buf;
		content.dsize = cp - buf;

		/*
		 * Create a name to entry mapping
		 */

		key.dptr = pwd->pw_name;
		key.dsize = strlen (pwd->pw_name);

		if (dbm_store (dp, key, content, DBM_INSERT) < 0) {
			fprintf (stderr, MSGSTR(MKPW, "mkpasswd: "));
			perror (MSGSTR(FAIL, "dbm_store failed"));
			exit (1);
		}

		/*
		 * Create another entry for the UID mapping in the file.
		 */

		key.dptr = (char *) &pwd->pw_uid;
		key.dsize = sizeof (int);

		if (dbm_store (dp, key, content, DBM_INSERT) < 0) {
			fprintf (stderr, MSGSTR(MKPW, "mkpasswd: "));
			perror (MSGSTR(STORE, "dbm_store failed"));
			exit (1);
		}

		/*
		 * Tally up the entries and update the long entry contest.
		 */

		entries++;
		if (cp - buf > maxlen)
			maxlen = cp - buf;
	}

	/*
	 * Close the password file and exit.
	 */

	dbm_close (dp);
	fprintf (stdout,MSGSTR(ENTRIES, "%d password entries, maximum length %d\n"), entries, maxlen);
	exit (0);
}
