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
static char rcsid[] = "@(#)$RCSfile: uuencode.c,v $ $Revision: 4.3.8.3 $ (DEC) $Date: 1993/10/11 19:31:23 $";
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
uuencode.c	5.9 (Berkeley) 6/1/90";
 */


/*
 * uuencode [input] output
 *
 * Encode a file so it can be mailed to a remote system.
 */
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>

#include        "uuencode_msg.h"
#include        <locale.h>
nl_catd catd;
extern *malloc();
#define MSGSTR(n,s)     catgets(catd,MS_UUENCODE,n,s)

main(argc, argv)
	int argc;
	char **argv;
{
	extern int optind;
	extern int errno;
	struct stat sb;
	int mode;
	char *strerror();

        setlocale(LC_ALL,"");
        catd = catopen(MF_UUENCODE, NL_CAT_LOCALE);

	while (getopt(argc, argv, "") != EOF)
		usage();
	argv += optind;
	argc -= optind;

	switch(argc) {
	case 2:			/* optional first argument is input file */
		if (!freopen(*argv, "r", stdin) || fstat(fileno(stdin), &sb)) {
			(void)fprintf(stderr, MSGSTR( UUENC_ARGS, 
				"uuencode: %s: %s.\n"),
			    *argv, strerror(errno));
			exit(1);
		}
#define	RWX	(S_IRWXU|S_IRWXG|S_IRWXO)
		mode = sb.st_mode & RWX;
		++argv;
		break;
	case 1:
#define	RW	(S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH)
		mode = RW & ~umask(RW);
		break;
	case 0:
	default:
		usage();
	}

	(void)printf(MSGSTR( UUENC_BEGIN, "begin %o %s\n"), mode, *argv);
	encode();
	(void)printf(MSGSTR( UUENC_END, "end\n"));
	if (ferror(stdout)) {
		(void)fprintf(stderr, MSGSTR( UUENC_WRT_ERR, "uuencode: write error.\n"));
		exit(1);
	}
	exit(0);
}

/* ENC is the basic 1 character encoding function to make a char printing */
#define	ENC(c) ((c) ? ((c) & 077) + ' ': '`')

/*
 * copy from in to out, encoding as you go along.
 */
encode()
{
	register int ch, n;
	register char *p;
	char buf[80];

	while (n = fread(buf, 1, 45, stdin)) {
		ch = ENC(n);
		if (putchar(ch) == EOF)
			break;
		for (p = buf; n > 0; n -= 3, p += 3) {
			ch = *p >> 2;
			ch = ENC(ch);
			if (putchar(ch) == EOF)
				break;
			ch = (*p << 4) & 060 | (p[1] >> 4) & 017;
			ch = ENC(ch);
			if (putchar(ch) == EOF)
				break;
			ch = (p[1] << 2) & 074 | (p[2] >> 6) & 03;
			ch = ENC(ch);
			if (putchar(ch) == EOF)
				break;
			ch = p[2] & 077;
			ch = ENC(ch);
			if (putchar(ch) == EOF)
				break;
		}
		if (putchar('\n') == EOF)
			break;
	}
	if (ferror(stdin)) {
		(void)fprintf(stderr, MSGSTR( UUENC_READ_ERR,
			"uuencode: read error.\n"));
		exit(1);
	}
	ch = ENC('\0');
	(void)putchar(ch);
	(void)putchar('\n');
}

usage()
{
	(void)fprintf(stderr,MSGSTR( UUENC_USAGE, 
		"usage: uuencode [infile] remotefile\n"));
	exit(1);
}
