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
static char	*sccsid = "@(#)$RCSfile: printpw.c,v $ $Revision: 4.2.2.2 $ (DEC) $Date: 1993/10/07 21:53:49 $";
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
#if !defined( lint) && !defined(_NOIDENT)

#endif

/*
 * Print the password entries. Access to the password data base
 * is through the getpwent() routine.
 * So yellow pages or secure systems must have a modifies getpwent()
 * routine in libc.
 */


#include <stdio.h>
#include <pwd.h>

#include <locale.h>
#include "acct_msg.h"
nl_catd catd;
#define MSGSTR(Num, Str) catgets(catd, MS_ACCT, Num, Str)

main(argc, argv)
int argc;
char *argv[];
{
	char c;
	struct passwd *pw;
	int uflg = 0;	/* flag for uid entry */
	int gflg = 0;	/* flag for gid entry */
	int cflg = 0;	/* flag for comment entry */
	int dflg = 0;	/* flag for directory entry */
	int sflg = 0;	/* flag for shell entry */
	int aflg = 0;	/* flag for all entries */

	(void) setlocale (LC_ALL,"");
	catd = catopen(MF_ACCT,NL_CAT_LOCALE);

	while((c = getopt(argc, argv, "acdgpsu")) != EOF)
        switch(c) {
                case 'a':
                        aflg++;
                        continue;
                case 'c':
                        cflg++;
                        continue;
                case 'd':
                        dflg++;
                        continue;
                case 'g':
                        gflg++;
                        continue;
                case 's':
                        sflg++;
                        continue;
                case 'u':
                        uflg++;
                        continue;
		case '?':
			fprintf(stderr, 
			    MSGSTR(PWUSAGE, "Usage: %s [-acdgsu]\n"), argv[0]);
			exit(1);
        }

	/* rewind password database */
	(void)setpwent();

	while ((pw = getpwent()) != (struct passwd *)0) {
		printf("%s", pw->pw_name);

		if (uflg || aflg)
			printf(":%d", pw->pw_uid);
		if (gflg || aflg)
			printf(":%d", pw->pw_gid);
		if (cflg || aflg)
			printf(":%s", pw->pw_comment);
		if (dflg || aflg)
			printf(":%s", pw->pw_dir);
		if (sflg || aflg)
			printf(":%s", pw->pw_shell);

		printf("\n");
	}

	/* close password database */
	(void)endpwent();
	return(0);
}
