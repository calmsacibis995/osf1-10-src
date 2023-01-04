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
static char rcsid[] = "@(#)$RCSfile: mv.c,v $ $Revision: 4.2.10.3 $ (DEC) $Date: 1993/10/11 17:37:01 $";
#endif
/*
 * HISTORY
 */
/*
 * OSF/1 1.2
 */
/*
 * Copyright (c) 1989 The Regents of the University of California.
 * All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Ken Smith of The State University of New York at Buffalo.
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
 * 1.30  com/cmd/files/mv.c, cmdfiles, bos320, 9130320 7/2/91 16:13:26"
 */
/*
char copyright[] =
" Copyright (c) 1989 The Regents of the University of California.\n\
 All rights reserved.\n";
 "mv.c	5.9 (Berkeley) 5/31/90";
*/

#include <sys/param.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <sys/errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <locale.h>
#include "mv_msg.h"
#include "pathnames.h"

nl_catd	catd;

#define MSGSTR(Num,Str) catgets(catd,MS_MV,Num,Str)
#define RESP_LEN 25			/* arbitrary length for response */

int fflg, iflg;

void	usage(void);
int	do_move(char *, char *);
int	fastcopy(char *, char *, struct stat *);
int	copy(char *, char *);

/*
 * NAME: mv [-if] [--] f1 f2  or: mv [-if] [--] f1 ... fn d1
 * 
 * FUNCTION: moves file(s) to the specified target.
 *
 * NOTES: 
 * 	-f  if the destination path exists, do not prompt for confirmation.
 * 	-i  if the destination path exists, prompt for confirmation.
 * 	    -f and -i are mutually exclusive.  The last one specified takes
 *	    precedence.   
 * 	--  denotes end of options to allow for file names beginning with
 * 	    a hyphen.   (documented for backward compatibility.  
 * 	    This use to be a single hyphen.  However, with the 
 * 	    implementation of getopts, the single hyphen became obsolete.  
 * 	    The getopts dash is explicitly documented here for
 * 	    clarity with previous releases)
 */
int
main(int argc, char **argv)
{
	register int baselen, exitval, len;
	register char *p, *endp;
	struct stat sbuf;
	int ch;
	char path[MAXPATHLEN + 1];

        (void)setlocale(LC_ALL, "");
        catd = catopen(MF_MV, NL_CAT_LOCALE);


	while (((ch = getopt(argc, argv, "-if")) != EOF))
		switch((char)ch) {
		case 'i':
			++iflg;
			fflg = 0;
			break;
		case 'f':
			++fflg;
			iflg = 0;
			break;
		case '-':		/* undocumented; for compatibility */
			goto endarg;
		case '?':
		default:
			usage();
		}
endarg:	argc -= optind;
	argv += optind;

	if (argc < 2)
		usage();

	/*
	 * if stat fails on target, it doesn't exist (or can't be accessed
	 * by the user, doesn't matter which) try the move.  If target exists,
	 * and isn't a directory, try the move.  More than 2 arguments is an
	 * error.
	 */
	if (stat(argv[argc - 1], &sbuf) || !S_ISDIR(sbuf.st_mode)) {
		if (argc > 2)
			usage();
		exit(do_move(argv[0], argv[1]));
	}

	/* got a directory, move each file into it */
	(void)strcpy(path, argv[argc - 1]);
	baselen = strlen(path);
	endp = &path[baselen];
	*endp++ = '/';
	++baselen;
	for (exitval = 0; --argc; ++argv) {
		if ((p = strrchr(*argv, '/')) == NULL)
			p = *argv;
		else
			++p;
		if ((baselen + (len = strlen(p))) >= MAXPATHLEN)
			(void)fprintf(stderr,
			    MSGSTR(P2LNG, "mv: %s: destination pathname too long\n"), *argv);
		else {
			bcopy(p, endp, len + 1);
			exitval |= do_move(*argv, path);
		}
	}
	exit(exitval);
	/*NOTREACHED*/
	return(exitval);
}

/*
 * NAME: do_move
 *
 * FUNCTION: main move routine.  performs the
 *           move if possible.
 *
 * RETURNS: 0 if successful, 1 on error
 */

int  
do_move(from, to)
	char *from, *to;
{
	struct stat sbuf, sbuf2;
	int ask;

	/*
	 * Complain if source doesn't exist, or files are identical.
	 */
	if (lstat(from, &sbuf) == -1) {
	    fprintf(stderr,
		    MSGSTR(RENAME, "mv: rename %s to %s: %s\n"), from, to, strerror(errno));
	    return(1);
	} else if (!lstat(to, &sbuf2) && 
	    (sbuf.st_dev == sbuf2.st_dev) && (sbuf.st_ino == sbuf2.st_ino)) {
		fprintf(stderr, MSGSTR(IDENTICLE, "mv: %s and %s are identical.\n"), from, to);
		return(1);
	}
	
	/*
	 * Check access.  If interactive and file exists ask user if it
	 * should be replaced.  Otherwise if file exists but isn't writable
	 * make sure the user wants to clobber it.
	 */
	if (!fflg && !access(to, F_OK)) {
		ask = 0;
		if (iflg) {
			(void)fprintf(stderr, MSGSTR(REMOVE, "overwrite %s? "), to);
			ask = 1;
		}
		else if (access(to, W_OK) && !stat(to, &sbuf) && isatty((int)fileno(stdin))) {
			(void)fprintf(stderr, MSGSTR(REMOVE2, "override mode %o on %s? "),
			    sbuf.st_mode & 07777, to);
			ask = 1;
		}
		if (ask) {
			char response[RESP_LEN];

			if (fgets(response,RESP_LEN,stdin) != NULL) {
				if (rpmatch(response) != 1)
					return(0);
			} else
				return(0);
		}
	}
	if (!rename(from, to))
		return(0);
	if (errno != EXDEV) {
		(void)fprintf(stderr,
		    MSGSTR(RENAME, "mv: rename %s to %s: %s\n"), from, to, strerror(errno));
		return(1);
	}
	/*
	 * if rename fails, and it's a regular file, do the copy
	 * internally; otherwise, use cp and rm.
	 */
	if (lstat(from, &sbuf)) {
		(void)fprintf(stderr,
		    "mv: %s: %s\n", from, strerror(errno));
		return(1);
	}
	return(S_ISREG(sbuf.st_mode) ?
	    fastcopy(from, to, &sbuf) : copy(from, to));
}

/*
 * NAME: fastcopy
 *
 * FUNCTION: performs an internal copy 
 *	     for regular files in the event
 *           the call to rename fails
 *
 * RETURNS: 0 if successful, 1 on error
 */

int
fastcopy(from, to, sbp)
	char *from, *to;
	struct stat *sbp;
{
	struct timeval tval[2];
	static u_int blen;
	static char *bp;
	register int nread, from_fd, to_fd;
	int spcerr, rc;

	if ((from_fd = open(from, O_RDONLY, 0)) < 0) {
		(void)fprintf(stderr,
		    "mv: %s: %s\n", from, strerror(errno));
		return(1);
	}

	/*
	 * If target file exists, remove it first before creating.
	 * If we can't remove it, write a message and move on.
	 */
	if (access(to, F_OK) == 0 && unlink(to) < 0) {
		(void)fprintf(stderr,
		    "mv: %s: %s\n", to, strerror(errno));
		(void)close(from_fd);
		return(1);
	}
		
	/*
	 * Call open to create the target file.  The permissions will
	 * be set to that of the source file modified by the umask.  Hence,
	 * the need for the fchmod call below.
	 */
	if ((to_fd = open(to, O_WRONLY|O_CREAT|O_TRUNC, sbp->st_mode)) < 0) {
		(void)fprintf(stderr,
		    "mv: %s: %s\n", to, strerror(errno));
		(void)close(from_fd);
		return(1);
	}
	if (!blen && !(bp = (char *)malloc(blen = sbp->st_blksize))) {
		(void)fprintf(stderr, MSGSTR(NOMEM, "mv: %s: out of memory.\n"), from);
		return(1);
	}
	while ((nread = read(from_fd, bp, blen)) > 0)
		if (write(to_fd, bp, nread) != nread) {
			(void)fprintf(stderr, "mv: %s: %s\n",
			    to, strerror(errno));
			goto err;
		}
	if (nread < 0) {
		(void)fprintf(stderr, "mv: %s: %s\n", from, strerror(errno));
err:		(void)unlink(to);
		(void)close(from_fd);
		(void)close(to_fd);
		return(1);
	}

	/*
	 * call fchown and fchmod to duplicate the source's
	 * owner i.d., group i.d., and file mode.  Clear the
	 * setuid and setgid bits if fchown fails.
	 */
	rc = 0;
	if (fchown(to_fd, sbp->st_uid, sbp->st_gid)) {  /* fail on fchown */
		rc = 1;
		sbp->st_mode &= ~(S_ISUID | S_ISGID);
	}
	if (fchmod(to_fd, sbp->st_mode))	/* fail on fchmod */
		rc = 1;
	if (rc) 
		(void) fprintf(stderr, "mv: %s: %s\n", to, strerror(errno));

	(void)close(from_fd);
	/*
	 * If close fails, don't want an empty target file, so
	 * print diagnostic msg and unlink the target file.
	 */
	spcerr=close(to_fd);
	if(spcerr)
	{
	  fprintf (stderr, "mv: ");
	  perror("close");
	  unlink(to);
	}

	tval[0].tv_sec = sbp->st_atime;
	tval[1].tv_sec = sbp->st_mtime;
	tval[0].tv_usec = tval[1].tv_usec = 0;
	(void)utimes(to, tval);
	(void)unlink(from);
	return(0);
}

/*
 * NAME: copy
 *
 * FUNCTION: calls cp and rm to move special files.
 *
 * RETURNS: 0 if successful, 1 on error
 */

int
copy(from, to)
	char *from, *to;
{
	int pid, status;

	if (!(pid = fork())) {
		execl(_PATH_CP, "mv", "-pR", from, to, 0);
		(void)fprintf(stderr, MSGSTR(NOEXEC, "mv: can't exec %s.\n"), _PATH_CP);
		_exit(1);
	}
	(void)waitpid(pid, &status, 0);
	if (!WIFEXITED(status) || WEXITSTATUS(status))
		return(1);
	if (!(pid = fork())) {
		execl(_PATH_RM, "mv", "-rf", from, 0);
		(void)fprintf(stderr, MSGSTR(NOEXEC, "mv: can't exec %s.\n"), _PATH_RM);
		_exit(1);
	}
	(void)waitpid(pid, &status, 0);
	return(!WIFEXITED(status) || WEXITSTATUS(status));
}

/*
 * NAME: usage
 *
 * FUNCTION: prints usage message
 */
void
usage(void)
{
	fprintf(stderr,
MSGSTR(USAGE, 
"usage: mv [-i | f] [--] src target\n   or: mv [-i | f] [--] src1 ... srcN directory\n   or: mv [-i | f] [--] directory1 ... destination_directory\n"));
	exit(1);
}
