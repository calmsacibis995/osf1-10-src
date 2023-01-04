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
static char rcsid[] = "@(#)$RCSfile: pathchk.c,v $ $Revision: 1.1.5.4 $ (DEC) $Date: 1993/10/11 20:00:47 $";
#endif
/*
 * HISTORY
 */
/*
 * OSF/1 1.2
 */
/*
 * COMPONENT_NAME: (CMDPOSIX) commands required by Posix 1003.2
 *
 * FUNCTIONS: pathchk
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1990
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * 1.3  com/cmd/posix/pathchk.c, cmdposix, bos320, 9130320 7/8/91 12:03:15
 */

#include <stdlib.h>
#include <locale.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/access.h>
#include <nl_types.h>
#include "pathchk_msg.h"

#define POSIX_PORTABLE_CHARS	\
	"/abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789.-_"

nl_catd catd;
#define MSGSTR(Num, Str) catgets(catd, MS_PATHCHK, Num, Str) 

extern char *optarg;
extern int optind;
int exit_status = 0;

main(argc, argv)
int argc;
char **argv;
{
	int j = 1;
	char *pathname;
	register c;
	int pflag = 0;

	setlocale(LC_ALL, "");
	catd = catopen(MF_PATHCHK, NL_CAT_LOCALE);

	while ( (c = getopt(argc, argv, "p") ) != EOF) {
		switch(c) {
		case 'p':
			pflag++;
			break;
		case '?':
		default:
			fprintf(stderr, MSGSTR(USAGE, 
				"Usage: pathchk [-p] pathname ...\n"));
			j++;
			exit_status = 1;
			}
		}

	if (pflag) {
		while (optind < argc) {
			check_posix(argv[optind]);
			optind++;
			}
		}
	else {
		while ( j < argc )
			check_path(argv[j++]);
		}

	exit(exit_status);
}

/*
 * Function: check_posix(pathname)
 *
 * Description: checks pathname length and component length
 *      against limits as defined by Posix.  It also checks for
 *      illegal characters in the portable filename character set,
 *      defined by Posix.
 */

check_posix(pathname)
char *pathname;
{
	char *ptr, *ptr2, *s3;
	static char buf[PATH_MAX+1];
	int mbcnt;
	wchar_t wc_ptr;
	char mbchar[10];

	if (strlen(pathname) > _POSIX_PATH_MAX) {
		fprintf(stderr, MSGSTR(POSIX_PATH, "pathname %s exceeds _POSIX_PATH_MAX %d\n"), pathname, _POSIX_PATH_MAX);
		exit_status++;
		}
	

	strcpy(buf, pathname);
	ptr = buf;

	while (*ptr != '\0') {
		if (*ptr == '/') ptr++;
		if ( (ptr2 = strchr(ptr, '/')) != NULL) {
			*ptr2 = '\0';
			if (strlen(ptr) > _POSIX_NAME_MAX) {
				fprintf(stderr, MSGSTR(POSIX_NAME, "name %s exceeds _POSIX_NAME_MAX %d\n"), ptr, _POSIX_NAME_MAX);
				exit_status++;
				}
			}
		else {
			if (strlen(ptr) > _POSIX_NAME_MAX) {
				fprintf(stderr, MSGSTR(POSIX_NAME, "name %s exceeds _POSIX_NAME_MAX %d\n"), ptr, _POSIX_NAME_MAX);
				exit_status++;
				}
			break;
			}	
		ptr = ptr2;
		ptr++;
		}

	s3 = pathname;
	for (; *s3 != '\0'; s3++) { 
		if (strchr(POSIX_PORTABLE_CHARS, *s3) == 0) {
           		if ( (mbcnt = mblen(s3, MB_CUR_MAX)) > 0)  {
				mbtowc(&wc_ptr, s3, MB_CUR_MAX);
                     		if (iswprint(wc_ptr)) {
					strncpy(mbchar, s3, mbcnt);
					mbchar[mbcnt]='\0';
                          		fprintf(stderr, "%s ", mbchar);
					s3 += mbcnt - 1;
                        	}
                 		else
                     			fprintf(stderr, "0x%2.2x ", *s3);
			}
                 	else
                     		fprintf(stderr, "0x%2.2x ", *s3);

			fprintf(stderr, MSGSTR(CHAR_SET, "is not in portable filename character set for %s\n"), pathname);
                        exit_status++;
			}
		}
	return;
}

/*
 * check_path is the default path of the pathchk function, this
 * checks for pathnames not to exceed the filesystems maximum
 * and for components not to exceed filesystems maximum.  It also
 * checks for search permission on that directory/file.
 */

check_path(pathname)
char *pathname;
{
	char *ptr, *ptr2;
	char file_access[PATH_MAX+1];
	int num;
	char *buf;

	if ((num=strlen(pathname)) > PATH_MAX) {
		fprintf( stderr, MSGSTR(PATHN_MAX, 
				"pathname %s exceeds PATH_MAX %d\n"), pathname, PATH_MAX+1);
		exit_status++;
	}

	*file_access = '\0';
	buf = strdup(pathname);
	ptr = buf;
	while (*ptr != '\0') {
		if (*ptr == '/') ptr++;
		if ( ( ptr2 = strchr(ptr, '/')) != NULL) {
			*ptr2 = '\0';
			if (strlen(ptr) > NAME_MAX) {
				fprintf(stderr, MSGSTR(NAMEN_MAX, 
						"name %s exceeds NAME_MAX %d\n"), ptr, NAME_MAX);
				exit_status++;
				}
			/* check for search permission,
			 * note the file does not have to
			 * exist, we can ignore ENOENT
			 */
			sprintf(file_access, "%s/%s", file_access, ptr);
			if ( (access(file_access, X_ACC)) == -1) {
				if (errno == ENOENT)
					/* ENOENT errno is ok */
					;
				else {
					fprintf(stderr, "pathchk: %s ", file_access);
					perror("access");
					exit_status++;
				}
			}
		}
		else {
			if (strlen(ptr) > NAME_MAX) {
				fprintf(stderr, MSGSTR(NAMEN_MAX, 
						"name %s exceeds NAME_MAX %d\n"), ptr, NAME_MAX);
				exit_status++;
			}
			if ( (access(ptr, X_ACC)) == -1) {
				if (errno == ENOENT)
					/* ENOENT errno is ok */
					;
				else {
					fprintf(stderr, "pathchk: %s ", ptr);
					perror("access");
					exit_status++;
				}
			}
			break;
		}
		ptr = ptr2;
		ptr++;
	}
	return;
}
