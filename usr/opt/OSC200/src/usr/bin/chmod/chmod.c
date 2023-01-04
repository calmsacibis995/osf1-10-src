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
static char rcsid[] = "@(#)$RCSfile: chmod.c,v $ $Revision: 4.2.11.3 $ (DEC) $Date: 1993/10/11 16:00:57 $";
#endif
/*
 * HISTORY
 */
/*
 * OSF/1 1.2
 */
/*
 * Copyright (c) 1980, 1988 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 * 
 * chmod.c	5.7 (Berkeley) 4/21/88
 */

#ifndef lint
char copyright[] =
"@(#) Copyright (c) 1980, 1988 Regents of the University of California.\n\
 All rights reserved.\n";
#endif /* not lint */


/*
 * chmod options mode files
 * where
 *	mode is [ugoa][+-=][rwxXstugo] or an octal number
 *	options are -Rf
 */
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/dir.h>
#include <locale.h>
#include <sys/errno.h>
#include <nl_types.h>
#include "chmod_msg.h"

#define MSGSTR(num,str) catgets(catd,MS_CHMOD,num,str)  /*MSG*/


static int	fflag, rflag, retval, um;
static char	*modestring, *ms;
nl_catd catd;

main(argc, argv)
	int argc;
	char **argv;
{
	extern char *optarg;
	extern int optind, opterr;
	int ch;
	int optsave;

        (void ) setlocale(LC_ALL,"");
        catd = catopen(MF_CHMOD, NL_CAT_LOCALE);

	/*
	 * since "-[rwx]" etc. are valid file modes, we don't let getopt(3)
	 * print error messages, and we mess around with optind as necessary.
	 */
	opterr = 0;
	optsave = 1;
	while ((ch = getopt(argc, argv, "Rf")) != EOF)
		switch((char)ch) {
		case 'R':
			rflag++;
			optsave = optind;
			break;
		case 'f':
			fflag++;
			optsave = optind;
			break;
		case '?':
		default:
			if (optind > optsave)
				--optind;
			goto done;
		}
done:	argv += optind;
	argc -= optind;

	if (argc < 2) {
		fprintf(stderr, MSGSTR(USAGE,
                        "usage:  chmod [-fR][ugoa][+-=][rwxXstugo] file...\n\
\tchmod [-fR] absolute_mode file...\n"));
		catclose(catd);
		exit(4);
	}

	modestring = *argv;
	um = umask(0);
	(void)newmode((u_short)0);

	while (*++argv)
		change(*argv);
	catclose(catd);
	exit(retval);
}

change(file)
	char *file;
{
	register DIR *dirp;
	register struct dirent *dp;
	struct stat buf;

	if (lstat(file, &buf)) {
		err(file);
		return;
	}
        /*
         * don't chmod the file if it's a symbolic link and -R was specified
	 */
	if (!(rflag && ((buf.st_mode & S_IFMT) == S_IFLNK)))
	    if (chmod(file, newmode(buf.st_mode))) {
		    err(file);
		    return;
	    }
	if (rflag && ((buf.st_mode & S_IFMT) == S_IFDIR)) {
		if (chdir(file) < 0 || !(dirp = opendir("."))) {
			err(file);
			return;
		}
		for (dp = readdir(dirp); dp; dp = readdir(dirp)) {
			if (dp->d_name[0] == '.' && (!dp->d_name[1] ||
			    dp->d_name[1] == '.' && !dp->d_name[2]))
				continue;
			change(dp->d_name);
		}
		closedir(dirp);
		if (chdir("..")) {
			err("..");
			catclose(catd);
			exit(fflag ? 0 : 4);
		}
	}
}

err(s)
	char *s;
{
	if (fflag)
		return;
	fputs("chmod: ", stderr);
	perror(s);
	retval = 4;
}

newmode(nm)
	u_short nm;
{
	register int o, m, b;

	ms = modestring;
	m = abs_();
	if (*ms == '\0')
		return (m);
	do {
		m = who();
		while (o = what()) {
			b = where((int)nm);
			switch (o) {
			case '+':
				nm |= b & m;
				break;
			case '-':
				nm &= ~(b & m);
				break;
			case '=':
				nm &= ~m;
				nm |= b & m;
				break;
			}
		}
	} while (*ms++ == ',');
	if (*--ms) {
		fprintf(stderr, MSGSTR(EMODE, "chmod: invalid mode.\n"));
		fprintf(stderr, MSGSTR(USAGE,
                        "usage:  chmod [-fR][ugoa][+-=][rwxXstugo] file...\n\
\tchmod [-fR] absolute_mode file...\n"));
		catclose(catd);
		exit(4);
	}
	return ((int)nm);
}

abs_()
{
	register int c, i;

	i = 0;
	while ((c = *ms++) >= '0' && c <= '7')
		i = (i << 3) + (c - '0');
	ms--;
	return (i);
}

#define	USER	05700	/* user's bits */
#define	GROUP	02070	/* group's bits */
#define	OTHER	00007	/* other's bits */
/* Changed from 01777 for QAR 4773 , from D Long.  C Richmond IISC */
#define	ALL	07777	/* all (note absence of setuid, etc) */

#define	READ	00444	/* read permit */
#define	WRITE	00222	/* write permit */
#define	EXEC	00111	/* exec permit */
#define	SETID	06000	/* set[ug]id */
#define	STICKY	01000	/* sticky bit */

who()
{
	register int m;

	m = 0;
	for (;;) switch (*ms++) {
	case 'u':
		m |= USER;
		continue;
	case 'g':
		m |= GROUP;
		continue;
	case 'o':
		m |= OTHER;
		continue;
	case 'a':
		m |= ALL;
		continue;
	default:
		ms--;
		if (m == 0)
			m = ALL & ~um;
		return (m);
	}
}

what()
{
	switch (*ms) {
	case '+':
	case '-':
	case '=':
		return (*ms++);
	}
	return (0);
}

where(om)
	register int om;
{
	register int m;

 	m = 0;
	switch (*ms) {
	case 'u':
		m = (om & USER) >> 6;
		goto dup;
	case 'g':
		m = (om & GROUP) >> 3;
		goto dup;
	case 'o':
		m = (om & OTHER);
	dup:
		m &= (READ|WRITE|EXEC);
		m |= (m << 3) | (m << 6);
		++ms;
		return (m);
	}
	for (;;) switch (*ms++) {
	case 'r':
		m |= READ;
		continue;
	case 'w':
		m |= WRITE;
		continue;
	case 'x':
		m |= EXEC;
		continue;
	case 'X':
		if ((om & S_IFDIR) || (om & EXEC))
			m |= EXEC;
		continue;
	case 's':
		m |= SETID;
		continue;
	case 't':
		m |= STICKY;
		continue;
	default:
		ms--;
		return (m);
	}
}
