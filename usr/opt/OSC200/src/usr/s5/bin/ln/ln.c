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
static char	*sccsid = "@(#)$RCSfile: ln.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 06:57:26 $";
#endif 
/*
 */
/*
 * (c) Copyright 1990, 1991, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0.1
 */
/*
 * Copyright (c) 1987 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted provided
 * that: (1) source distributions retain this entire copyright notice and
 * comment, and (2) distributions including binaries display the following
 * acknowledgement:  ``This product includes software developed by the
 * University of California, Berkeley and its contributors'' in the
 * documentation or other materials provided with the distribution and in
 * all advertising materials mentioning features or use of this software.
 * Neither the name of the University nor the names of its contributors may
 * be used to endorse or promote products derived from this software without
 * specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */
/*
#ifndef lint
char copyright[] =
"@(#) Copyright (c) 1987 Regents of the University of California.\n\
 All rights reserved.\n";
#endif 

#ifndef lint
static char sccsid[] = "ln.c	4.13 (Berkeley) 6/19/90";
#endif
*/

#include <sys/param.h>
#include <sys/stat.h>
#include <stdio.h>
#include <errno.h>
#include <locale.h>
#include <nl_types.h>

#include <sys/access.h>		/* added for SVID-2 */
#include <unistd.h>
#include <langinfo.h>

#include "ln_msg.h"

nl_catd	catd;

#define MSGSTR(Num,Str) catgets(catd,MS_LN,Num,Str)


#define MODEBITS 07777					/* added for SVID-2 */

static linkit();
static usage();

static int	fflag=0,			/* force flag */
		sflag=0,			/* symbolic, not hard, link */
		(*linkf)();			/* system link call */

main(argc, argv)
	int	argc;
	char	**argv;
{
	extern int optind;
	struct stat buf;
	int ch, exitval, link(), symlink();
	char *sourcedir;

	(void)setlocale(LC_ALL, "");
	catd = catopen(MF_LN, 0);

	while ((ch = getopt(argc, argv, "fs")) != EOF)
		switch((char)ch) {
		case 'f':
			fflag = 1;
			break;
		case 's':
			sflag = 1;
			break;
		case '?':
		default:
			usage();
		}

	argv += optind;
	argc -= optind;

	linkf = sflag ? symlink : link;

	switch(argc) {
	case 0:
		usage();
	case 1:				/* ln target */
		exit(linkit(argv[0], ".", 1));
	case 2:				/* ln target source */
		exit(linkit(argv[0], argv[1], 0));
	default:			/* ln target1 target2 directory */
		sourcedir = argv[argc - 1];
		if (stat(sourcedir, &buf)) {
			perror(sourcedir);
			exit(1);
		}
		if ((buf.st_mode & S_IFMT) != S_IFDIR)
			usage();
		for (exitval = 0; *argv != sourcedir; ++argv)
			exitval |= linkit(*argv, sourcedir, 1);
		exit(exitval);
	}
	/*NOTREACHED*/
}

static
linkit(target, source, isdir)
	char	*target, *source;
	int	isdir;
{
	extern int	errno;
	struct stat	buf;
	char	path[MAXPATHLEN],
		*cp, *rindex(), *strcpy();
	char	resp[CBLOCK];		/* for SVID-2 */
	char	yes[CBLOCK];		/* for SVID-2 */
	char	no[CBLOCK];		/* for SVID-2 */

	if (!sflag) {
		/* if target doesn't exist, quit now */
		if (stat(target, &buf)) {
			perror(target);
			return(1);
		}
		/* only symbolic links to directories, unless -F option used */
		if ((buf.st_mode & S_IFMT) == S_IFDIR) {
			printf(MSGSTR(ISDIR, "%s is a directory.\n"), target);
			return(1);
		}
	}

	/* if the source is a directory, append the target's name */
	if (isdir || !stat(source, &buf) && (buf.st_mode & S_IFMT) == S_IFDIR) {
		if (!(cp = rindex(target, '/')))
			cp = target;
		else
			++cp;
		(void)sprintf(path, "%s/%s", source, cp);
		source = path;
	}


	/* added for SVID-2, note usage of target & source are inverse of
	 * their usage in the man page;
	 * If the destination (source) file already exists, and it is not a
	 * not a directory, we want to unlink it so the linkf() can succeed.
	 *
	 * If the file is not writeable, and we are being run interactively,
	 * and the user did not spefify the -f option, prompt the user for
	 * permission to unlink the destination (source) file. If the user
	 * does not respond with an affirmative, return 1 i.e. do not attempt
	 * the unlink() and linkf().
	 * Note, that the default behavior if ln is not run interactively is
	 * to silently unlink an existing write-protected file and link it to
	 * the input (target).
	 */

	if( !isdir && access( source, F_OK) == 0)
	{
		if(access(source, W_OK) < 0 && isatty(fileno(stdin)) && !fflag)
		{
			/* get file mode bits in case it is write protected */
			if (stat(source, &buf)) {
				perror(source);
				return(1);
			}

			/* ask user for OK to unlink "source" */
			/* only accepts YESSTR or NOSTR, exits on NOSTR */

			setlocale(LC_ALL, "");	/* from appl_prog.gd ch4 */
			strcpy(yes, nl_langinfo(YESSTR));
			strcpy(no, nl_langinfo(NOSTR));
			do
			{
			   printf(MSGSTR(QUERY,
			   "%s is not writeable, mode %o: override %s or %s ?"),
			   source, (buf.st_mode & MODEBITS), yes, no);

			   gets(resp);
			}
			while(strcmp(resp, yes) != 0 && strcmp(resp, no) != 0 );

			if( strcmp(resp, no) == 0)
				return(1);

		} /* end ask user for OK to unlink */

		/* unlink "source", perror & return 1 if error */

		if( unlink(source) < 0)
		{
			perror(source);
			return(1);
		}

	} /* end code for SVID-2 */

	if ((*linkf)(target, source)) {
		perror(source);
		return(1);
	}
	return(0);
}

static
usage()
{
	fputs(MSGSTR(S5USAGE, "usage:\tln [-fs] targetname [sourcename]\n\tln [-fs] targetname1 targetname2 [... targetnameN] sourcedirectory\n"), stderr);
	exit(1);
}
