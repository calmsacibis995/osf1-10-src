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
static char rcsid[] = "@(#)$RCSfile: ctags.c,v $ $Revision: 4.2.6.4 $ (OSF) $Date: 1993/10/07 20:23:10 $";
#endif
/*
 * Copyright (c) 1987 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 * 
 * ctags.c	5.5 (Berkeley) 6/1/90
 */

#ifndef lint
char copyright[] =
"@(#) Copyright (c) 1987 Regents of the University of California.\n\
 All rights reserved.\n";
#endif /* not lint */


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <locale.h>
#include "ctags.h"

/*
 * ctags: create a tags file
 */

NODE	*head;			/* head of the sorted binary tree */

				/* boolean "func" (see init()) */
bool	_wht[0177],_etk[0177],_itk[0177],_btk[0177],_gd[0177];

FILE	*inf,			/* ioptr for current input file */
	*outf;			/* ioptr for tags file */

long	lineftell;		/* ftell after getc( inf ) == '\n' */

int	lineno,			/* line number of current line */
	dflag,			/* -d: non-macro defines */
	tflag = 1,		/* -t: create tags for typedefs(POSIX default)*/
	wflag,			/* -w: suppress warnings */
	vflag,			/* -v: vgrind style index output */
	xflag;			/* -x: cxref style output */

char	*curfile,		/* current input file name */
	searchar = '/',		/* use /.../ searches by default */
	lbuf[BUFSIZ];

nl_catd	catd;

void	init(void);
void	find_entries(char *);

main(int argc, char **argv)
{
	extern char	*optarg;		/* getopt arguments */
	extern int	optind;
	static char	*outfile = "tags";	/* output file */
	int	aflag,				/* -a: append to tags */
		uflag,				/* -u: update tags */
		nflag,				/* -n: disable sort */
		exit_val,			/* exit value */
		step,				/* step through args */
		ch;				/* getopts char */
	char	cmd[100];			/* too ugly to explain */

	(void)setlocale(LC_ALL,"");
	catd = catopen(MF_CTAGS, NL_CAT_LOCALE);

	aflag = uflag = nflag = NO;
	while ((ch = getopt(argc,argv,"BFadf:ntuwvx")) != EOF)
		switch((char)ch) {
			case 'B':
				searchar = '?';
				break;
			case 'F':
				searchar = '/';
				break;
			case 'a':
				aflag++;
				break;
			case 'd':
				dflag++;
				break;
			case 'f':
				outfile = optarg;
				break;
			case 'n':
				nflag++;
				break;
			case 't':
				tflag++;	/* POSIX default */
				break;
			case 'u':
				uflag++;
				break;
			case 'w':
				wflag++;
				break;
			case 'v':
				vflag++;
			case 'x':
				xflag++;
				break;
			case '?':
			default:
				goto usage;
		}
	argv += optind;
	argc -= optind;
	if (!argc) {
usage:		puts(MSGSTR(USAGE, 
			"Usage: ctags [-BFadntuwv] [-x | -f tagsfile] file ..."));
		exit(1);
	}

	init();

	for (exit_val = step = 0;step < argc;++step)
		if (!(inf = fopen(argv[step],"r"))) {
			perror(argv[step]);
			exit_val = 1;
		}
		else {
			curfile = argv[step];
			find_entries(argv[step]);
			(void)fclose(inf);
		}

	if (head)
		if (xflag)
			put_entries(head);
		else {
			if (uflag) {
				for (step = 0;step < argc;step++) {
					(void)sprintf(cmd,"mv %s OTAGS;fgrep -v '\t%s\t' OTAGS >%s;rm OTAGS",outfile,argv[step],outfile);
					system(cmd);
				}
				++aflag;
			}
			if (!(outf = fopen(outfile, aflag ? "a" : "w"))) {
				perror(outfile);
				exit(exit_val);
			}
			put_entries(head);
			(void)fclose(outf);
			if (uflag || aflag) {
				if (!nflag) {
					(void)sprintf(cmd,"sort -u -o %s %s",outfile,outfile);
					system(cmd);
				}
			}
		}
	exit(exit_val);
}

/*
 * init --
 *	this routine sets up the boolean psuedo-functions which work by
 *	setting boolean flags dependent upon the corresponding character.
 *	Every char which is NOT in that string is false with respect to
 *	the pseudo-function.  Therefore, all of the array "_wht" is NO
 *	by default and then the elements subscripted by the chars in
 *	CWHITE are set to YES.  Thus, "_wht" of a char is YES if it is in
 *	the string CWHITE, else NO.
 */
void
init(void)
{
	register int	i;
	register char	*sp;

	for (i = 0; i < 0177; i++) {
		_wht[i] = _etk[i] = _itk[i] = _btk[i] = NO;
		_gd[i] = YES;
	}
#define	CWHITE	" \f\t\n"
	for (sp = CWHITE; *sp; sp++)	/* white space chars */
		_wht[*sp] = YES;
#define	CTOKEN	" \t\n\"'#()[]{}=-+%*/&|^~!<>;,.:?"
	for (sp = CTOKEN; *sp; sp++)	/* token ending chars */
		_etk[*sp] = YES;
#define	CINTOK	"ABCDEFGHIJKLMNOPQRSTUVWXYZ_abcdefghijklmnopqrstuvwxyz0123456789"
	for (sp = CINTOK; *sp; sp++)	/* valid in-token chars */
		_itk[*sp] = YES;
#define	CBEGIN	"ABCDEFGHIJKLMNOPQRSTUVWXYZ_abcdefghijklmnopqrstuvwxyz"
	for (sp = CBEGIN; *sp; sp++)	/* token starting chars */
		_btk[*sp] = YES;
#define	CNOTGD	",;"
	for (sp = CNOTGD; *sp; sp++)	/* invalid after-function chars */
		_gd[*sp] = NO;
}

/*
 * find_entries --
 *	this routine opens the specified file and calls the function
 *	which searches the file.
 */
void
find_entries(char *file)
{
	register char	*cp;

	lineno = 0;				/* should be 1 ?? KB */
	if (cp = rindex(file, '.')) {
		if (cp[1] == 'l' && !cp[2]) {
			register int	c;

			for (;;) {
				if (GETC(==,EOF))
					return;
				if (!iswhite(c)) {
					rewind(inf);
					break;
				}
			}
#define	LISPCHR	";(["
/* lisp */		if (index(LISPCHR,(char)c)) {
				l_entries();
				return;
			}
/* lex */		else {
				/*
				 * we search all 3 parts of a lex file
				 * for C references.  This may be wrong.
				 */
				toss_yysec();
				(void)strcpy(lbuf,"%%$");
				pfnote("yylex",lineno);
				rewind(inf);
			}
		}
/* yacc */	else if (cp[1] == 'y' && !cp[2]) {
			/*
			 * we search only the 3rd part of a yacc file
			 * for C references.  This may be wrong.
			 */
			toss_yysec();
			(void)strcpy(lbuf,"%%$");
			pfnote("yyparse",lineno);
			y_entries();
		}
/* fortran */	else if ((cp[1] != 'c' && cp[1] != 'h') && !cp[2]) {
			if (PF_funcs())
				return;
			rewind(inf);
		}
	}
/* C */	c_entries();
}
