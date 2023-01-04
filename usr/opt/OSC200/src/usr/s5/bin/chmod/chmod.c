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
static char	*sccsid = "@(#)$RCSfile: chmod.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 06:57:14 $";
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
#if !defined(lint) && !defined(_NOIDENT)
#endif
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

			/* moved for SVID-2 */
#define	USER	05700	/* user's bits */
#define	GROUP	02070	/* group's bits */
#define	OTHER	00007	/* other's bits */
#define	ALL	07777	/* all added suid bits for SVID-2 */

			/* moved for SVID-2 */
#define	READ	00444	/* read permit */
#define	WRITE	00222	/* write permit */
#define	EXEC	00111	/* exec permit */
#define	SETID	06000	/* set[ug]id */
#define	STICKY	01000	/* sticky bit */

			/* added for SVID-2 */
#define UX	0100	/* user execute permission bit */
#define US	04000	/* set user ID bit */
#define GX	010	/* group execute permission bit */
#define	GS	02000	/* set group ID bit */

static int	fflag, rflag, retval, um;
static char	*modestring, *ms;
nl_catd catd;

int scheck, xcheck; /*added for SVID-2, cleared by newmode(), set by where()*/
			 
main(argc, argv)
	int argc;
	char **argv;
{
	extern char *optarg;
	extern int optind, opterr;
	int ch;

        (void ) setlocale(LC_ALL,"");
        catd = catopen(MF_CHMOD, 0);

	/*
	 * since "-[rwx]" etc. are valid file modes, we don't let getopt(3)
	 * print error messages, and we mess around with optind as necessary.
	 */
	opterr = 0;
	while ((ch = getopt(argc, argv, "Rf")) != EOF)
		switch((char)ch) {
		case 'R':
			rflag++;
			break;
		case 'f':
			fflag++;
			break;
		case '?':
		default:
			--optind;
			goto done;
		}
done:	argv += optind;
	argc -= optind;

	if (argc < 2) {
                fprintf(stderr, MSGSTR(USAGE, "Usage: chmod [-Rf][ugoa][+-=][rwxstugo] file ...\n"));
		exit(-1);
	}

	modestring = *argv;
	um = umask(0);
	/* next line modified for SVID-2, check mode before checking file */
	(void)newmode((mode_t)0777);

	while (*++argv)
		change(*argv);
	exit(retval);
}

change(file)
	char *file;
{
	register DIR *dirp;
	register struct dirent *dp;
	struct stat buf;

	if (lstat(file, &buf) || chmod(file, newmode(buf.st_mode))) {
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
			exit(fflag ? 0 : -1);
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
	retval = -1;
}

newmode(nm)
	mode_t nm;
{
	register int o, m, b;
	int cflag;		/* added for SVID-2 */

	scheck = xcheck = 0;	/* SVID-2, conditionally set by where() */
	cflag = 0;
	ms = modestring;
	m = abs();
	if (*ms == '\0')
		return (m);
	do {
		m = who();
		while (o = what()) {
			b = where((int)nm);
			switch (o) {
			case '+':
				/* this added for set_ID for SVID-2
				 * if one of the set_ID bits is being set, the
				 * corresponding execute bit must also be on,
				 * either in the new or current file permissions
				 */
				if(scheck && ( ((nm | b) &m &EXEC &(USER|GROUP))
				    != (m & EXEC & (USER | GROUP)) ) )
				{
                		    fprintf(stderr, MSGSTR(SETSID,
				    "Need execute permission for set_ID\n"));
				    exit(-1);
				}
				nm |= b & m;
				break;
			case '-':
				/* this added for set_ID for SVID-2
				 * if user or group execute is being cleared,
				 * the corresponding set_ID bit must also be
				 * cleared.
				 */
				if(xcheck)
				{
					if( (b & UX) && !(b & US) && (nm & US))
					{
						b |= US;
						cflag = 1;
					}

					if( (b & GX) && !(b & GS) && (nm & GS))
					{
						b |= GS;
						cflag = 1;
					}
					if (cflag )
					  fprintf(stderr, MSGSTR(CLRSID,
					  "clearing corresponding s bit(s)\n"));
				}
				nm &= ~(b & m);
				break;
			case '=':
				/* this added for set_ID for SVID-2
				 * if one of the set_ID bits is being set, the
				 * corresponding execute bit must also be on
				 */
                                if(scheck  && (b & EXEC & (USER | GROUP))
				!= (m & EXEC & (USER | GROUP)) )
				{
                		    fprintf(stderr, MSGSTR(SETSID,
				    "Need execute permission for set_ID\n"));
				    exit(-1);
				}

				nm &= ~m;
				nm |= b & m;
				break;
			}
		}
	} while (*ms++ == ',');
	if (*--ms) {
                fprintf(stderr, MSGSTR(EMODE, "chmod: invalid mode\n"));
		exit(-1);
	}
	return ((int)nm);
}

abs()
{
	register int c, i;

	i = 0;
	while ((c = *ms++) >= '0' && c <= '7')
		i = (i << 3) + (c - '0');
	ms--;
	return (i);
}

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
			m = ALL;
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
		xcheck = 1;		/* added for SVID-2 */
		m |= EXEC;
		continue;
	case 'X':
		if ((om & S_IFDIR) || (om & EXEC))
			m |= EXEC;
		continue;
	case 's':
		m |= SETID;
		scheck = 1;		/* added for SVID-2 */
		continue;
	case 't':
		m |= STICKY;
		continue;
	default:
		ms--;
		return (m);
	}
}
