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
static char rcsid[] = "@(#)$RCSfile: chgrp.c,v $ $Revision: 4.2.10.3 $ (DEC) $Date: 1993/10/11 16:00:54 $";
#endif
/*
 * HISTORY
 */
/*
 * OSF/1 1.2
 */
/*
 * COMPONENT_NAME: (CMDOPER) commands needed for basic system needs
 *
 * FUNCTIONS: chgrp
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 * 
 * chgrp.c	1.14  com/cmd/oper,3.1,9013 2/12/90 16:55:32
 */

/*
 * chgrp gid file ...
 */
#include <sys/secdefines.h>
#if SEC_BASE
#include <sys/security.h>
#endif
 
#include <stdio.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <grp.h>
#include <dirent.h>
#include <locale.h>
#include "chgrp_msg.h" 
nl_catd  catd;   /* Cat descriptor for scmc conversion */
#define MSGSTR(num,str) catgets(catd,MS_chgrp,num,str)  /*MSG*/

extern char *optarg;
extern int optind;

struct	group	*gr;
struct	stat	stbuf;
int	gid;
int	status = 0;
char	*arg;
int	fflag = 0;
int 	Rflag = 0;

void usage(void) {
	fprintf(stderr,MSGSTR(M_MSG_0,
		"usage: chgrp [-fR] group_name | group_ID file ...\n"));
	catclose(catd);
	exit(4);
}

/*
 * NAME: chgrp
 *                                                                    
 * FUNCTION: Chages the group ownership of a file or directory.
 *     FLAGS: 
 *      -f   Suppresses all error reporting
 *      -R   Causes chgrp to descend recursively through its directory 
 *           arguments, setting the specified group ID.  When symbolic links 
 *           are encountered, their group is changed, but they are not 
 *           traversed.
 */  
main(argc, argv)
int argc;
char *argv[];
{
	register int c;
	char *strp;

	(void) setlocale(LC_ALL,"");
	catd = catopen(MF_CHGRP, NL_CAT_LOCALE);

#if SEC_BASE
        set_auth_parameters(argc, argv);
        initprivs();

        if (authorized_user("anygroup") && !forcepriv(SEC_CHOWN)) {
                fprintf(stderr, MSGSTR(M_MSG_1,
			"chgrp: insufficient privileges\n"));
                catclose(catd);
                exit(4);
        }
#endif

	if(argc < 3)
		usage();

	arg = argv[1];

	while ((c = getopt(argc,argv,"fR")) != EOF) {
		switch(c) {
			case 'f':
				fflag++;	
				break;
			case 'R':
				Rflag++;	
				break;
			case '?':
				usage();
				/* break; */
		}
	}

	if (optind+1 >= argc)
		usage();

	if ((gr = getgrnam(argv[optind])) != NULL) {
		gid = gr->gr_gid;
	} else {
		gid = strtoul(argv[optind], &strp, 10);
		if (strp == NULL  || *strp != '\0') {
			if (!fflag) {
				fprintf(stderr,MSGSTR(M_MSG_2,
					"chgrp: unknown group: %s\n"),argv[optind]);
			}
			exit(4);
		}
	}

	optind++;
	for(; optind<argc; optind++) {
		/* do stat for directory arguments */
		if (lstat(argv[optind], &stbuf)) {
			status = Perror(argv[optind]);
			continue;
		}
		if (Rflag && ((stbuf.st_mode & S_IFMT) == S_IFDIR)) {
			status = chownr(argv[optind], gid);
			continue;
		}
#if SEC_BASE
                disablepriv(SEC_SUSPEND_AUDIT);
#endif
		if(chown(argv[optind],-1,gid) < 0) {
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
 * NAME: chownr
 * FUNCTION: recursively descend directory and change the group id of all
 *   files.
 * 
 * NOTE: this code is essentially duplicated in chown.c.  
 * 	 Any changes made here will probably need to be made there too!
 */
chownr(dir, gid)
char *dir;
gid_t gid;
{
	DIR *dirp;
	struct dirent *dp;
	struct stat st;
	char savedir[PATH_MAX+1];
	static int ecode = 0;
	int tmp;

	if (getwd(savedir) == 0) {
		fprintf(stderr, MSGSTR(M_MSG_3,
			"chgrp: chown: cannot get directory path %s"), savedir);
		catclose(catd);
		exit(4);
	}
	/*
	 * Change what we are given before doing its contents.
	 */
#if SEC_BASE
        disablepriv(SEC_SUSPEND_AUDIT);
#endif
	if (chown(dir, -1, gid) < 0 ) {
#if SEC_BASE
                forcepriv(SEC_SUSPEND_AUDIT);
#endif
		ecode = Perror(dir);	
	}
#if SEC_BASE
        forcepriv(SEC_SUSPEND_AUDIT);
#endif
	if (chdir(dir) < 0) {
		return (Perror(dir));
	}
	if ((dirp = opendir(".")) == NULL) {
		return (Perror(dir));
	}
	for (dp = readdir(dirp); dp != NULL; dp = readdir(dirp)) {
		if (!strcmp(dp->d_name, ".") ||
		    !strcmp(dp->d_name, ".."))
			continue;
		if (lstat(dp->d_name, &st) < 0) {
			ecode = Perror(dp->d_name);
			continue;
		}
		if ((st.st_mode & S_IFMT) == S_IFDIR) {
			/*
			 * do not overwrite ecode w/ zero
			 * and continue on regardless of errors
			 */
			if (chownr(dp->d_name, gid))
				ecode = 4;
			continue;
		}
#if SEC_BASE
                disablepriv(SEC_SUSPEND_AUDIT);
#endif
		if (Rflag && S_ISLNK(st.st_mode)) {

			if (lchown(dp->d_name, -1, gid) < 0 ) {
#if SEC_BASE
				forcepriv(SEC_SUSPEND_AUDIT);
#endif
				ecode = Perror(dp->d_name);
				continue;
			}
		} else if (chown(dp->d_name, -1, gid) < 0 ) {
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
		fprintf(stderr,MSGSTR(M_MSG_4,
			"chgrp: cannnot change back to %s"), savedir);
		catclose(catd);
		exit(4);
	}
	return (ecode);
}


/*
 * NAME: Perror
 * FUNCTION: displays perror messages
 */
int
Perror(s)
	char *s;
{

	if (!fflag) {
		fprintf(stderr, "chgrp: ");
		perror(s);
	}
	return (4);
}
