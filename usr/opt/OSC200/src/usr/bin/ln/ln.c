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
static char rcsid[] = "@(#)$RCSfile: ln.c,v $ $Revision: 4.3.10.6 $ (DEC) $Date: 1994/01/14 23:06:26 $";
#endif
/*
 * HISTORY
 */
/*
 * OSF/1 1.2
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
 *
 * 1.6  com/cmd/files/ln.c, cmdfiles, bos320, 9138320 9/9/91 15:44:52
 *
char copyright[] =
" Copyright (c) 1987 Regents of the University of California.\n\
 All rights reserved.\n";
 "ln.c	4.13 (Berkeley) 6/19/90";
 */

#include <sys/param.h>
#include <sys/stat.h>
#include <stdio.h>
#include <errno.h>
#include <locale.h>
#include <nl_types.h>
#include "ln_msg.h"

nl_catd	catd;

#define MSGSTR(Num,Str) catgets(catd,MS_LN,Num,Str)

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
	catd = catopen(MF_LN, NL_CAT_LOCALE);

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
		exit(linkit(argv[0], "."));
	case 2:				/* ln target source */
		exit(linkit(argv[0], argv[1]));
	default:			/* ln target1 target2 directory */
		sourcedir = argv[argc - 1];
		if (stat(sourcedir, &buf)) {
			perror(sourcedir);
			exit(1);
		}
		if ((buf.st_mode & S_IFMT) != S_IFDIR)
			usage();
		for (exitval = 0; *argv != sourcedir; ++argv)
			exitval |= linkit(*argv, sourcedir);
		exit(exitval);
	}
	/*NOTREACHED*/
}

static
linkit(source, target)
	char	*source, *target;
{
	extern int	errno;
	int target_exists = -1;
	int source_exists = -1;
	struct stat	sbuf, tbuf;
        mode_t cur_umask;  /* 001GZ. Save there umask value before symlink */
	char	path[MAXPATHLEN],
		*cp, *rindex(), *strcpy();

        source_exists = stat(source, &sbuf); /* 001GZ */
	if (!sflag) {
		/* if source doesn't exist, quit now */
		if (source_exists != 0) {
			perror(source);
			return(1);
		}
		if ((sbuf.st_mode & S_IFMT) == S_IFDIR) {
			printf(MSGSTR(ISDIR, "%s is a directory.\n"), source);
			return(1);
		}
	}

        target_exists = stat(target, &tbuf);
	/* if the target is a directory, append the source's name */
	if ( target_exists == 0) { 
	  if ((tbuf.st_mode & S_IFMT) == S_IFDIR) {
	    if (!(cp = rindex(source, '/')))
	      cp = source;
	    else
	      ++cp;
	    (void)sprintf(path, "%s/%s", target, cp);
	    target = path;
	    target_exists = stat(target, &tbuf);
	  }
	} else	/* Check for a symlink */
		target_exists = lstat(target, &tbuf);

	if ( target_exists == 0) {
	  if (tbuf.st_dev == sbuf.st_dev && 
	      tbuf.st_ino == sbuf.st_ino) {
          /* DS001 When -f flag specified, omit error and return success. */
            if ( !fflag ) {
	       fprintf(stderr, 
		    MSGSTR(IDENTICAL,"ln: %s and %s are identical.\n"), 
		    target, source);
	       return(1);
            }
            else {
              return(0);
            }
	  }
          if ( !fflag ) {
	    fprintf(stderr, 
		    MSGSTR(FOPT,
			   "ln: %s exists, specify -f to remove.\n"), 
		    target);
	    return(1);
	  }
	  if (unlink(target) < 0) {
	    perror(target);
	    return(1);
	  }
	}
	  
        if (sflag)
	     cur_umask = umask(~(S_IRWXU | S_IRWXG | S_IRWXO));

	if ((*linkf)(source, target)) {
		switch (errno)	{
			case (EXDEV):
				fprintf (stderr, 
					 MSGSTR
					 (DIFFFS,"ln: %s and %s are located on different file systems.\n"), 
					 source, target);
				break;
			case (EACCES):
				fprintf (stderr, 
					 MSGSTR
					 (NOPERM,"ln: insufficient permission for %s.\n"), 
					 target);
				break;
			default:
				perror(target);
				break;
			      }
		if (sflag) umask(cur_umask); /* 001GZ. Restore umask */
		return(1);
	      }
	if (sflag) umask(cur_umask); /* 001GZ. Restore umask */
	return(0);
}

static
usage()
{
	fputs(MSGSTR(USAGE, "usage:\tln [-fs] sourcename [targetname]\n\tln [-fs] sourcename1 sourcename2 [... sourcenameN] targetdirectory\n"), stderr);
	exit(1);
}
