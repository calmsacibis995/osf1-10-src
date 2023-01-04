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
#if !defined(lint) && !defined(_NOIDENT)
static char rcsid[] = "@(#)$RCSfile: uudecode.c,v $ $Revision: 4.3.5.5 $ (DEC) $Date: 1993/10/16 15:08:56 $";
#endif
/*
 * Copyright (c) 1983 Regents of the University of California.
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
uudecode.c	5.10 (Berkeley) 6/1/90";
 */


/*
 * uudecode [file ...]
 *
 * create the specified file, decoding as you go.
 * used with uuencode.
 */
#include <sys/param.h>
#include <sys/stat.h>
#include <pwd.h>
#include <stdio.h>
#include <string.h>

#include        "uudecode_msg.h"
#include        <locale.h>
nl_catd catd;
extern *malloc();
#define MSGSTR(n,s)     catgets(catd,MS_UUDECODE,n,s)
char *filename;

/* ARGSUSED */
main(argc, argv)
	int argc;
	char **argv;
{
	extern int errno;
	int rval;
        setlocale(LC_ALL,"");
        catd = catopen(MF_UUDECODE, NL_CAT_LOCALE);

	if (argc > 1) 	/*CMR002*/
	if (strcmp(*(argv+1), "--")==0)
		argv++;
	if (*++argv) {
		rval = 0;
		do {
			if (!freopen(filename = *argv, "r", stdin)) {
				(void)fprintf(stderr, MSGSTR( UUDEC_ARGS, 
					"uudecode: %s: %s\n"),
				    *argv, strerror(errno));
				rval = 1;
				continue;
			}
			rval |= decode();
		} while (*++argv);
	} else {
		filename = "stdin";
		rval = decode();
	}
	exit(rval);
}

decode()
{
	extern int errno;
	struct passwd *pw;
	register int n;
	register char ch, *p;
	int mode, n1;
	char buf[MAXPATHLEN];

	/* search for header line */
	do {
		if (!fgets(buf, sizeof(buf), stdin)) {
			(void)fprintf(stderr, MSGSTR( UUDEC_NO_BEGIN,
			    "uudecode: %s: no \"begin\" line\n"), filename);
			return(1);
		}
	} while (strncmp(buf, "begin ", 6));
	(void)sscanf(buf, "begin %o %s", &mode, buf);

	/* handle ~user/file format */
	if (buf[0] == '~') {
		if (!(p = index(buf, '/'))) {
			(void)fprintf(stderr, MSGSTR( UUDEC_ILL_USER, 
			"uudecode: %s: illegal ~user.\n"),
			    filename);
			return(1);
		}
		*p++ = NULL;
		if (!(pw = getpwnam(buf + 1))) {
			(void)fprintf(stderr, MSGSTR( UUDEC_NO_USER, 
				"uudecode: %s: no user %s.\n"),
			    filename, buf);
			return(1);
		}
		n = strlen(pw->pw_dir);
		n1 = strlen(p);
		if (n + n1 + 2 > MAXPATHLEN) {
			(void)fprintf(stderr, MSGSTR( UUDEC_PATH_LONG,
				"uudecode: %s: path too long.\n"),
			    filename);
			return(1);
		}
		bcopy(p, buf + n + 1, n1 + 1);
		bcopy(pw->pw_dir, buf, n);
		buf[n] = '/';
	}

	/* create output file, set mode */
	if (!freopen(buf, "w", stdout) ||
	    fchmod(fileno(stdout), mode)) {
		(void)fprintf(stderr, MSGSTR( UUDEC_CHMOD, 
			"uudecode: %s: %s: %s\n"), buf,
		    filename, strerror(errno));
		return(1);
	}

	/* for each input line */
	for (;;) {
		if (!fgets(p = buf, sizeof(buf), stdin)) {
			(void)fprintf(stderr, MSGSTR( UUDEC_SHORT,
				"uudecode: %s: short file.\n"),
			    filename);
			return(1);
		}
#define	DEC(c)	(((c) - ' ') & 077)		/* single character decode */
		/*
		 * `n' is used to avoid writing out all the characters
		 * at the end of the file.
		 */
		if ((n = DEC(*p)) <= 0)
			break;
		for (++p; n > 0; p += 4, n -= 3)
			if (n >= 3) {
				ch = DEC(p[0]) << 2 | DEC(p[1]) >> 4;
				putchar(ch);
				ch = DEC(p[1]) << 4 | DEC(p[2]) >> 2;
				putchar(ch);
				ch = DEC(p[2]) << 6 | DEC(p[3]);
				putchar(ch);
			}
			else {
				if (n >= 1) {
					ch = DEC(p[0]) << 2 | DEC(p[1]) >> 4;
					putchar(ch);
				}
				if (n >= 2) {
					ch = DEC(p[1]) << 4 | DEC(p[2]) >> 2;
					putchar(ch);
				}
				if (n >= 3) {
					ch = DEC(p[2]) << 6 | DEC(p[3]);
					putchar(ch);
				}
			}
	}
	if (!fgets(buf, sizeof(buf), stdin) || strcmp(buf, "end\n")) {
		(void)fprintf(stderr, MSGSTR( UUDEC_NO_END,
			"uudecode: %s: no \"end\" line.\n"), filename);
		return(1);
	}
	return(0);
}

usage()
{
	(void)fprintf(stderr, MSGSTR( UUDEC_USAGE, 
		"usage: uudecode [file ...]\n"));
	exit(1);
}
