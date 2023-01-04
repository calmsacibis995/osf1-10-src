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
static char rcsid[] = "@(#)$RCSfile: what.c,v $ $Revision: 4.2.3.2 $ (OSF) $Date: 1993/09/29 02:07:23 $";
#endif
/*
 * COMPONENT_NAME: CMDSCCS      Source Code Control System (sccs)
 *
 * FUNCTIONS: dowhat, main
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * what.c 1.6  com/cmd/sccs/cmd/what.c, 9121320k, bos320 4/25/91 19:26:24";
 */

#include <stdio.h>
#include <unistd.h>
#include <locale.h>
#include "defines.h"
#include "what_msg.h"

#define TRUE  1
#define FALSE 0

#define MSGSTR(Num, Str) catgets(catd, MS_WHAT, Num, Str)

int found;
int silent;

#define pat0    '@'
char pat1[]  =  "(#)";

nl_catd catd;

main(argc,argv)
int argc;
register char **argv;
{
	register int i;

	(void)setlocale(LC_ALL,"");
	catd = catopen(MF_SCCS, NL_CAT_LOCALE);

	while((i = getopt(argc, argv, "s")) != -1) {

		switch (i) {

		case 's':
			silent = TRUE;
			break;

		default:
			fprintf(stderr,MSGSTR(WHATUSAGE, "Usage: what [-s] file ...\n"));
			exit(1);
		}
	}
			
	if (optind == argc) {
		fprintf(stderr,MSGSTR(WHATUSAGE, "Usage: what [-s] file ...\n"));
			exit(1);
	}

	for (i = optind; i < argc; i++) {
		if (!(freopen(argv[i],"r",stdin)))
			perror(argv[i]);
		else {
			printf("%s:\n",argv[i]);
			dowhat();
		}
	}
	exit(!found);				/* shell return code */
}


dowhat()
{
	register int c;
	register char *p;

	while ((c = getchar()) != EOF)
		while (c == pat0)
			for (p = pat1;;) {
				if ((c = getchar()) == EOF) return;
				if (c != *p) break;
				if (!*++p) {
					putchar('\t');
					while ((c = getchar()) != EOF && c &&
					    !any(c,"\"\\>\n"))
						putchar(c);
					putchar('\n');
					found = TRUE;
					if (silent) return;
					break;
				}
			}
}
