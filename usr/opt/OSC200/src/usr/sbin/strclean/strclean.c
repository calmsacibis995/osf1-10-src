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
static char *rcsid = "@(#)$RCSfile: strclean.c,v $ $Revision: 4.2.5.3 $ (DEC) $Date: 1993/10/08 16:01:59 $";
#endif
/*
 * (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.2
 */
/** Copyright (c) 1991  Mentat Inc.
 ** strclean.c 1.1, last change 4/30/91
 **/

#include <sys/secdefines.h>
#if SEC_BASE
#include <sys/security.h>
#endif

#include <stdio.h>
#include <sys/errno.h>
#include <time.h>
#include <dirent.h>
#include <sys/stat.h>


#include <locale.h>
#include "strclean_msg.h"
nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_STRCLEAN,n,s)

#ifndef DEFAULT_DIRECTORY
#define	DEFAULT_DIRECTORY	"/var/adm/streams"
#endif

#define	DAY_IN_SECONDS		(60 * 60 * 24)	/* seconds * minutes * hours */

extern	char	* optarg;
extern	int	optind;

main (argc, argv)
	int	argc;
	char	** argv;
{
	int	age;
	char	buf[FILENAME_MAX];
	int	c;
	char	* dir_name;
	DIR	* dirp;
	struct dirent	* direntp;
	struct stat	st;
	struct tm	* tmp;
	time_t	cutoff_time;

	setlocale( LC_ALL, "" );
	catd = catopen(MF_STRCLEAN,NL_CAT_LOCALE);

#if SEC_BASE
	set_auth_parameters(argc, argv);
	initprivs();

	if (!authorized_user("sysadmin")) {
		fprintf(stderr, MSGSTR(STRCL_AUTH,
		    "strclean: need sysadmin authorization\n"));
		exit(1);
	}
	if (forceprivs(privvec(SEC_ALLOWDACACCESS,
#if SEC_MAC
	    SEC_ALLOWMACACCESS,
#endif
	    -1), (priv_t *) 0)) {
		fprintf(stderr, MSGSTR(STRCL_PRIV,
		    "strclean: insufficient privileges\n"));
		exit(1);
	}
#endif
	age = 3;
	dir_name = DEFAULT_DIRECTORY;
	while ((c = getopt(argc, argv, "d:D:a:A:")) != -1) {
		switch (c) {
		case 'a': case 'A':	/* Age */
			age = atoi(optarg);
			break;
		case 'd': case 'D':	/* Directory name */
			dir_name = optarg;
			break;
		case '?':
		default:
			fprintf(stderr, MSGSTR( STRCL_USE, 
				"usage: strclean [-d logdir] [-a age]\n"));
			exit(1);
		}
	}
	/* Convert the age into seconds */
	age *= DAY_IN_SECONDS;

	/* Get current month and day */
	time(&cutoff_time);

	/*
	 * Convert to a cutoff time.
	 * All files older than this will be removed.
	 */
	cutoff_time -= age;

	dirp = opendir(dir_name);
	if (!dirp)
		return 0;

	/* Check all files in the directory. */
	while ( (direntp = readdir(dirp)) != NULL) {
		/*
		 * Ignore . and .. as well as all files that don't start
		 * with "error."
		 */
		if (strcmp(direntp->d_name, "..") == 0
		||  strcmp(direntp->d_name, ".") == 0
		||  strncmp(direntp->d_name, "error.", 6) != 0)
			continue;

		sprintf(buf, "%s/%s", dir_name, direntp->d_name);
		if (stat(buf, &st) != -1) {
			/*
			 * Compare the file modification time to the
			 * cutoff time.
			 */
			if (st.st_mtime < cutoff_time)
				if (unlink(buf) < 0) {	/* Remove the file. */
					fprintf(stderr, MSGSTR(STRCL_NOPERM,
					    "strclean: %s: "), buf);
					perror("");
				}
		}
	}
	closedir(dirp);
	return 0;
}
