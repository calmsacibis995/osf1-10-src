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
static char	*sccsid = "@(#)$RCSfile: main.c,v $ $Revision: 4.2.8.2 $ (DEC) $Date: 1993/10/01 01:08:18 $";
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
#ifndef lint

#endif

#include <stdio.h>
#include <ctype.h>
#include <locale.h>
#include <nl_types.h>

#include "awk.def"
#include "awk.h"
#include "awk_msg.h"

#define TOLOWER(c)	(isupper(c) ? tolower(c) : c) /* ugh!!! */
#define MSGSTR(Num, Str) catgets(catd, NL_SETD, Num, Str)

int	dbg	= 0;
int	ldbg	= 0;
int	svflg	= 0;
int	rstflg	= 0;
int	svargc;
char	**svargv, **xargv;
extern FILE	*yyin;	/* lex input file */
char	*lexprog;	/* points to program argument if it exists */
extern	errorflag;	/* non-zero if any syntax errors; set by yyerror */

nl_catd catd;

int filefd, symnum, ansfd;
char *filelist;
extern int maxsym, errno;

main(int argc, char *argv[])
{
       (void) setlocale(LC_ALL,"");

       catd = catopen(MF_AWK, NL_CAT_LOCALE);

	if (argc == 1)
usage:		error(FATAL, 
                MSGSTR(USAGE, "Usage: awk [-f source | 'cmds'] [files]"));

	syminit();

	while (argc > 1) {
		argc--;
		argv++;
		/* this nonsense is because gcos argument handling */
		/* folds -F into -f.  accordingly, one checks the next
		/* character after f to see if it's -f file or -Fx.
		*/
		if (argv[0][0] == '-' && TOLOWER(argv[0][1]) == 'f' && argv[0][2] == '\0') {
			if (--argc == 0) goto usage;
			if (argv[1][0] == '-' && argv[1][1] == '\0')
				yyin = stdin;
			else {
				yyin = fopen(argv[1], "r");
				if (yyin == NULL)
					error(FATAL, 
                                       MSGSTR(NOPEN, "can't open %s"), argv[1]);
			}
			argv++;
			break;
		} else if (argv[0][0] == '-' && TOLOWER(argv[0][1]) == 'f') {	/* set field sep */
			if (argv[0][2] == 't')	/* special case for tab */
				**FS = '\t';
			else
				**FS = argv[0][2];
			continue;

		} else if (argv[0][0] != '-') {
			dprintf("cmds=|%s|\n", argv[0], NULL, NULL);
			yyin = NULL;
			lexprog=argv[0];
			argv[0] = argv[-1];	/* need this space */
			break;
		} else if (strcmp("-d", argv[0])==0) {
			dbg = 1;
		} else if (strcmp("-l", argv[0])==0) {
			ldbg = 1;
		}
		else if(strcmp("-S", argv[0]) == 0) {
			svflg = 1;
		}
		else if(strncmp("-R", argv[0], 2) == 0) {
			if(thaw(argv[0] + 2) == 0)
				rstflg = 1;
			else {
				fprintf(stderr,MSGSTR(NOREST,"not restored\n"));
				exit(1);
			}
		} else {
			goto usage;	/* Invalid option... */
		}
	}
	if (argc <= 1) {
		argv[0][0] = '-';
		argv[0][1] = '\0';
		argc++;
		argv--;
	}
	svargc = --argc;
	svargv = ++argv;
	dprintf("svargc=%d svargv[0]=%s\n", svargc, svargv[0], NULL);
	*FILENAME = *svargv;	/* initial file name */
 	if(rstflg == 0)
		yyparse(); 
	dprintf("errorflag=%d\n", errorflag, NULL, NULL);
	if (errorflag)
		exit(errorflag);
	if(svflg) {
		svflg = 0;
		if(freeze("awk.out") != 0)
			fprintf(stderr, MSGSTR(NOSAVE,"not saved\n"));
		exit(0);
	}
	run();
	exit(errorflag);
}

yywrap()
{
	return(1);
}
