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
static char     *sccsid = "@(#)$RCSfile: ctags.c,v $ $Revision: 4.2.3.2 $ (DEC) $Date: 1993/10/11 16:46:38 $";
#endif
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0
 *
 */

#if !defined(lint) && !defined(_NOIDENT)
#endif
/*
 * COMPONENT_NAME: (CMDEDIT) ctags.c
 *
 * FUNCTION: C_funcs, L_funcs, L_getit, main, PF_funcs, Y_entries, add_node,
 * find_funcs, first_char, free_tree, getit, getline, init, isgood, pfnote,
 * put_entries, put_funcs, rindex, savestr, start_func, striccmp, tail,
 * takeprec, toss_comment, toss_yyse
 *
 * ORIGINS: 3, 10, 13, 26, 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 * OBJECT CODE ONLY SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * ctags.c  1.7  com/cmd/edit/vi,3.1,9013 3/7/90 06:07:01
 * 
 * 
 */
#include <stdio.h>
#include <ctype.h>
#include <locale.h>

#include "ex_msg.h"
nl_catd	ex_catd;
#define msg(id,ds)      catgets(ex_catd, 1, id, ds)

#if defined(_ATT)
/*
 * ctags
 */
#endif
#if defined(BSDNEW)
/*
 * ctags: create a tags file
 */
#endif

#define	reg	register
#define	logical	char
#define BUFFERSIZ (2*8192)	/* to be separate from BUFSIZ in stdio.h */

/* #define	TRUE	(1) */
/* #define	FALSE	(0) */

#define	iswhite(arg)	(_wht[arg])	/* T if char is white		*/
#define	begtoken(arg)	(_btk[arg])	/* T if char can start token	*/
#define	intoken(arg)	(_itk[arg])	/* T if char can be in token	*/
#define	endtoken(arg)	(_etk[arg])	/* T if char ends tokens	*/
#define	isgood(arg)	(_gd[arg])	/* T if char can be after ')'	*/

#define	max(I1,I2)	(I1 > I2 ? I1 : I2)

struct	nd_st {			/* sorting structure			*/
#if defined(_ATT)
	char	*func;			/* function name		*/
#endif
#if defined(BSDNEW)
	char	*entry;			/* entry name	        	*/
#endif
	char	*file;			/* file name			*/
#if defined(BSDNEW)
	logical	f;			/* use pattern or line no	*/
#endif
	int	lno;			/* for -x option		*/
	char	*pat;			/* search pattern		*/
	logical	been_warned;		/* set if noticed dup		*/
	struct	nd_st	*left,*right;	/* left and right sons		*/
};

typedef	struct	nd_st	NODE;

logical	number,				/* T if on line starting with #	*/
	term	= FALSE,		/* T if print on terminal	*/
	makefile= TRUE,			/* T if to creat "tags" file	*/
	gotone,				/* found a func already on line	*/
					/* boolean "func" (see init)	*/
	_wht[0177],_etk[0177],_itk[0177],_btk[0177],_gd[0177];

#if defined(BSDNEW)
	/* typedefs are recognized using a simple finite automata,
	 * tydef is its state variable.
	 */
typedef enum {none, begin, middle, end } TYST;

TYST tydef = none;
char	searchar = '/';			/* use /.../ searches 		*/
#endif

#if defined(_ATT)
char	searchar = '?';			/* use ?...? searches 		*/
#endif

int	lineno;				/* line number of current line */
char	line[4*BUFFERSIZ],		/* current input line			*/
	*curfile,		/* current input file name		*/
	*outfile= "tags",	/* output file				*/
	*white	= " \f\t\n",	/* white chars				*/
	*endtk	= " \t\n\"'#()[]{}=-+%*/&|^~!<>;,.:?",
				/* token ending chars			*/
	*begtk	= "ABCDEFGHIJKLMNOPQRSTUVWXYZ_abcdefghijklmnopqrstuvwxyz",
				/* token starting chars			*/
	*intk	= "ABCDEFGHIJKLMNOPQRSTUVWXYZ_abcdefghijklmnopqrstuvwxyz0123456789",				/* valid in-token chars			*/
	*notgd	= ",;";		/* non-valid after-function chars	*/

int	file_num;		/* current file number			*/
int	aflag;			/* -a: append to tags */
int	mflag;			/* -m: create no tags for macros */
#if defined(BSDNEW)
int	tflag;			/* -t: create tags for typedefs */
#endif
int	uflag;			/* -u: update tags */
#if defined(BSDNEW)
int	vflag;			/* -v: create vgrind style index output */
#endif
int	wflag;			/* -w: suppress warnings */
int	xflag;			/* -x: create cxref style output */

char	lbuf[BUFFERSIZ];

FILE	*inf,			/* ioptr for current input file		*/
	*outf;			/* ioptr for tags file			*/

long	lineftell;		/* ftell after getc( inf ) == '\n' 	*/

NODE	*head;			/* the head of the sorted binary tree	*/

char	*savestr();
#if defined(_ATT)
char	*rindex();
#endif
#if defined(BSDNEW)
char	*rindex(), *index();
char	*toss_comment();
#endif
main(ac,av)
int	ac;
char	*av[];
{
	char cmd[100];
	int i;

	setlocale(LC_ALL,"");		/* required by NLS environment tests */
	ex_catd = catopen(MF_EX,NL_CAT_LOCALE);

	while (ac > 1 && av[1][0] == '-') {
		for (i=1; av[1][i]; i++) {
#if defined(_ATT)
			switch(av[1][i]) {
				case 'a':
					aflag++;
					break;
				case 'u':
					uflag++;
					break;
				case 'w':
					wflag++;
					break;
				case 'x':
					xflag++;
					break;
				default:
					goto usage;
			}
#endif
#if defined(BSDNEW)
			switch(av[1][i]) {
			  case 'B':
				searchar='?';
				break;
			  case 'F':
				searchar='/';
				break;
			  case 'a':
				aflag++;
				break;
			  case 'm':
				mflag++;
				break;
			  case 't':
				tflag++;
				break;
			  case 'u':
				uflag++;
				break;
			  case 'w':
				wflag++;
				break;
			  case 'v':
				vflag++;
				xflag++;
				break;
			  case 'x':
				xflag++;
				break;
			  case 'f':
				if (ac < 2)
					goto usage;
				ac--, av++;
				outfile = av[1];
				goto next;
			  default:
				goto usage;
			}
#endif
		}
next:
		ac--; av++;
	}

#if defined(_ATT)
	if (ac <= 1) {
		usage: printf("Usage: ctags [-auwx] file ...\n");
		exit(1);
	}
#endif
#if defined(BSDNEW)
	if (ac <= 1) {
usage:
		printf(msg(M_600, "Usage: ctags [-BFatuwvx] [-f tagsfile] file ...\n"), 0);
		exit(1);
	}
#endif

	init();			/* set up boolean "functions"		*/
	/*
	 * loop through files finding functions
	 */
	for (file_num = 1; file_num < ac; file_num++)
		find_funcs(av[file_num]);

	if (xflag) {
#if defined(_ATT)
		put_funcs(head);
#endif
#if defined(BSDNEW)
		put_entries(head);
#endif
		exit(0);
	}
	if (uflag) {
		for (i=1; i<ac; i++) {
			sprintf(cmd,
				"mv %s OTAGS;fgrep -v '\t%s\t' OTAGS >%s;rm OTAGS",
				outfile, av[i], outfile);
			system(cmd);
		}
		aflag++;
	}
	outf = fopen(outfile, aflag ? "a" : "w");
	if (outf == NULL) {
		perror(outfile);
		exit(1);
	}
#if defined(_ATT)
	put_funcs(head);
#endif
#if defined(BSDNEW)
	put_entries(head);
#endif
	fclose(outf);
	if (uflag) {
		sprintf(cmd, "sort %s -o %s", outfile, outfile);
		system(cmd);
	}
	exit(0);
}

/*
 * This routine sets up the boolean psuedo-functions which work
 * by seting boolean flags dependent upon the corresponding character
 * Every char which is NOT in that string is not a white char.  Therefore,
 * all of the array "_wht" is set to FALSE, and then the elements
 * subscripted by the chars in "white" are set to TRUE.  Thus "_wht"
 * of a char is TRUE if it is the string "white", else FALSE.
 */
init()
{

	reg	char	*sp;
	reg	int	i;

	for (i = 0; i < 0177; i++) {
		_wht[i] = _etk[i] = _itk[i] = _btk[i] = FALSE;
		_gd[i] = TRUE;
	}
	for (sp = white; *sp; sp++)
		_wht[*sp] = TRUE;
	for (sp = endtk; *sp; sp++)
		_etk[*sp] = TRUE;
	for (sp = intk; *sp; sp++)
		_itk[*sp] = TRUE;
	for (sp = begtk; *sp; sp++)
		_btk[*sp] = TRUE;
	for (sp = notgd; *sp; sp++)
		_gd[*sp] = FALSE;
}

#if defined(_ATT)
/*
 * This routine opens the specified file and calls the function
 * which finds the function definitions.
 */
#endif
#if defined(BSDNEW)
/*
 * This routine opens the specified file and calls the function
 * which finds the function and type definitions.
 */
#endif
find_funcs(file)
char	*file;
{
	char *cp;

	lineftell = 0;	/* zero out the offset when starting on a new file */
	if ((inf=fopen(file,"r")) == NULL) {
		perror(file);
		return;
	}
	curfile = savestr(file);
#if defined(_ATT)
	cp = rindex(file, '.');
	if (cp && (cp[1] != 'c' && cp[1] != 'h') && cp[2] == 0) {
		if (PF_funcs(inf) == 0) {
			rewind(inf);
			C_funcs();
		}
	} else
		C_funcs();
#endif
#if defined(BSDNEW)
	lineno = 0;
	cp = rindex(file, '.');
	/* .l implies lisp or lex source code */
	if (cp && cp[1] == 'l' && cp[2] == '\0') {
		if (index(";([", first_char()) != NULL) {	/* lisp */
			L_funcs(inf);
			fclose(inf);
			return;
		}
		else {						/* lex */
			/*
			 * throw away all the code before the second "%%"
			 */
			toss_yysec();
			getline();
			pfnote("yylex", lineno, TRUE);
			toss_yysec();
			C_funcs();
			fclose(inf);
			return;
		}
	}
	/* .y implies a yacc file */
	if (cp && cp[1] == 'y' && cp[2] == '\0') {
		toss_yysec();
		Y_entries();
		C_funcs();
		fclose(inf);
		return;
	}
	/* if not a .c or .h file, try fortran */
	if (cp && (cp[1] != 'c' && cp[1] != 'h') && cp[2] == '\0') {
		if (PF_funcs(inf) != 0) {
			fclose(inf);
			return;
		}
		rewind(inf);	/* no fortran tags found, try C */
	}
	C_funcs();
#endif
	fclose(inf);
}

#if defined(_ATT)
pfnote(name, ln)
#endif
#if defined(BSDNEW)
pfnote(name, ln, f)
int	ln;
logical	f;		/* f == TRUE when function */
#endif
	char *name;
{
	register char *fp;
	register NODE *np;
	char nbuf[BUFFERSIZ];

	if ((np = (NODE *) malloc(sizeof (NODE))) == NULL) {
		fprintf(stderr, msg(M_601,"ctags: too many functions to sort\n"));
#if defined(_ATT)
		put_funcs(head);
#endif
#if defined(BSDNEW)
		put_entries(head);
#endif
		free_tree(head);
		head = np = (NODE *) malloc(sizeof (NODE));
	}
	if (xflag == 0 && !strcmp(name, "main")) {
		fp = rindex(curfile, '/');
		if (fp == 0)
			fp = curfile;
		else
			fp++;
		sprintf(nbuf, "M%s", fp);
		fp = rindex(nbuf, '.');
		if (fp && fp[2] == 0)
			*fp = 0;
		name = nbuf;
	}
#if defined(_ATT)
	np->func = savestr(name);
#endif
#if defined(BSDNEW)
	np->entry = savestr(name);
#endif
	np->file = curfile;
#if defined(BSDNEW)
	np->f = f;
#endif
	np->lno = ln;
	np->left = np->right = 0;
	if (xflag == 0) {
		lbuf[50] = 0;
		strcat(lbuf, "$");
		lbuf[50] = 0;
	}
	np->pat = savestr(lbuf);
	if (head == NULL)
		head = np;
	else
		add_node(np, head);
}

#if defined(_ATT)
/*
 * This routine finds functions in C syntax and adds them
 * to the list.
 */
#endif
#if defined(BSDNEW)
/*
 * This routine finds functions and typedefs in C syntax 
 * and adds them to the list.
 */
#endif
C_funcs()
{
	register int c;
	register char *token, *tp;
#if defined(_ATT)
	int incomm, inquote, inchar, midtoken, level;
#endif
#if defined(BSDNEW)
	logical incomm, inquote, inchar, midtoken;
	int level;
#endif
	char *sp;
	char tok[BUFFERSIZ];

#if defined(_ATT)
	lineno = 1;
#endif
	number = gotone = midtoken = inquote = inchar = incomm = FALSE;
#if defined(BSDNEW)
	lineno++;
	lineftell = ftell(inf);
#endif
	level = 0;
	sp = tp = token = line;
	for (;;) {
		*sp=c=getc(inf);
		if (feof(inf))
			break;
		if (c == '\n')
			lineno++;
		if (c == '\\') {
			c = *++sp = getc(inf);
#if defined(_ATT)
			if (c = '\n')
#endif
#if defined(BSDNEW)
			if (c == '\n')
#endif
				c = ' ';
		} else if (incomm) {
			if (c == '*') {
				while ((*++sp=c=getc(inf)) == '*')
					continue;
				if (c == '\n')
					lineno++;
				if (c == '/')
					incomm = FALSE;
			}
		} else if (inquote) {
			/*
			 * Too dumb to know about \" not being magic, but
			 * they usually occur in pairs anyway.
			 */
			if (c == '"')
				inquote = FALSE;
			continue;
		} else if (inchar) {
			if (c == '\'')
				inchar = FALSE;
			continue;
		} else switch (c) {
		case '"':
			inquote = TRUE;
			continue;
		case '\'':
			inchar = TRUE;
			continue;
		case '/':
			if ((*++sp=c=getc(inf)) == '*')
				incomm = TRUE;
			else
				ungetc(*sp, inf);
			continue;
 		case '#':
			if (sp == line)
				number = TRUE;
			continue;
		case '{':
#if defined(BSDNEW)
			if (tydef == begin) {
				tydef=middle;
			}
#endif
			level++;
			continue;
		case '}':
			if (sp == line)
				level = 0;	/* reset */
			else
				level--;
#if defined(BSDNEW)
			if (!level && tydef==middle) {
				tydef=end;
			}
#endif
			continue;
		}
#if defined(_ATT)
		if (!level && !inquote && !incomm && gotone == 0) {
#endif
#if defined(BSDNEW)
		if (!level && !inquote && !incomm && gotone == FALSE) {
#endif
			if (midtoken) {
				if (endtoken(c)) {
#if defined(BSDNEW)
					int f;
#endif
					int pfline = lineno;
#if defined(_ATT)
					if (start_func(&sp,token,tp)) {
#endif
#if defined(BSDNEW)
					if (start_func(&sp,token,tp,&f)) {
#endif
						strncpy(tok,token,tp-token+1);
						tok[tp-token+1] = 0;
						getline();
#if defined(_ATT)
						pfnote(tok, pfline);
						gotone = TRUE;
#endif
#if defined(BSDNEW)
						pfnote(tok, pfline, f);
						gotone = f;	/* function */
#endif
					}
					midtoken = FALSE;
					token = sp;
				} else if (intoken(c))
					tp++;
			} else if (begtoken(c)) {
				token = tp = sp;
				midtoken = TRUE;
			}
		}
#if defined(BSDNEW)
		if (c == ';'  &&  tydef==end)	/* clean with typedefs */
			tydef=none;
#endif
		sp++;
		if (c == '\n' || sp > &line[sizeof (line) - BUFFERSIZ]) {
			tp = token = sp = line;
			lineftell = ftell(inf);
			number = gotone = midtoken = inquote = inchar = FALSE;
		}
	}
}

#if defined(_ATT)
/*
 *	This routine  checks to see if the current token is
 * at the start of a function.  It updates the input line
 * so that the '(' will be in it when it returns.
 */
start_func(lp,token,tp)
#endif
#if defined(BSDNEW)
/*
 * This routine  checks to see if the current token is
 * at the start of a function, or corresponds to a typedef
 * It updates the input line so that the '(' will be
 * in it when it returns.
 */
start_func(lp,token,tp,f)
int	*f;
#endif
char	**lp,*token,*tp;
{

	reg	char	c,*sp,*tsp;
	static	logical	found;
	logical	firsttok;		/* T if have seen first token in ()'s */
	int	bad;

#if defined(BSDNEW)
	*f = 1;
#endif
	sp = *lp;
	c = *sp;
	bad = FALSE;
	/* check for the macro cases		*/
	if  ( number && mflag) {
		goto badone;
	}
	if (!number) {		/* space is not allowed in macro defs	*/
		while (iswhite(c)) {
			*++sp = c = getc(inf);
			if (c == '\n') {
				lineno++;
				if (sp > &line[sizeof (line) - BUFFERSIZ])
					goto ret;
			}
		}
	/* the following tries to make it so that a #define a b(c)	*/
	/* doesn't count as a define of b.				*/
	} else {
		logical	define;

		define = TRUE;
		for (tsp = "define"; *tsp && token < tp; tsp++)
			if (*tsp != *token++) {
				define = FALSE;
				break;
			}
		if (define)
			found = 0;
		else
			found++;
		if (found >= 2) {
			gotone = TRUE;
badone:			bad = TRUE;
			goto ret;
		}
	}
#if defined(BSDNEW)
	/* check for the typedef cases		*/
	if (tflag && !strncmp(token, "typedef", 7)) {
		tydef=begin;
		goto badone;
	}
	if (tydef==begin && (!strncmp(token, "struct", 6) ||
	    !strncmp(token, "union", 5) || !strncmp(token, "enum", 4))) {
		goto badone;
	}
	if (tydef==begin) {
		tydef=end;
		goto badone;
	}
	if (tydef==end) {
		*f = 0;
		goto ret;
	}
#endif
	if (c != '(')
		goto badone;
	firsttok = FALSE;
	while ((*++sp=c=getc(inf)) != ')') {
		if (feof(inf))			/* A1473 */
			goto badone;		/* A1473 */
		if (c == '\n') {
			lineno++;
			if (sp > &line[sizeof (line) - BUFFERSIZ])
				goto ret;
		}
		/*
		 * This line used to confuse ctags when it was used in a
		 * file that ctags tried to parse.
		 *	int	(*oldhup)();
		 * This fixes it. A nonwhite char before the first
		 * token, other than a / (in case of a comment in there)
		 * makes this not a declaration.
		 */
		if (begtoken(c) || c=='/') firsttok++;
		else if (!iswhite(c) && !firsttok) goto badone;
	}
	while (iswhite(*++sp=c=getc(inf)))
		if (c == '\n') {
			lineno++;
			if (sp > &line[sizeof (line) - BUFFERSIZ])
				break;
		}
ret:
	*lp = --sp;
	if (c == '\n')
		lineno--;
	ungetc(c,inf);
#if defined(_ATT)
	return !bad && isgood(c);
}
#endif
#if defined(BSDNEW)
	return !bad && (!*f || isgood(c));
					/* hack for typedefs */
}

/*
 * Y_entries:
 *	Find the yacc tags and put them in.
 */
Y_entries()
{
	register char		*sp, *orig_sp;
	register int		brace;
	register logical	in_rule, toklen;
	char		tok[BUFFERSIZ];

	brace = 0;
	getline();
	pfnote("yyparse", lineno, TRUE);
	while (fgets(line, sizeof line, inf) != NULL)
		for (sp = line; *sp; sp++)
			switch (*sp) {
			  case '\n':
				lineno++;
				/* FALLTHROUGH */
			  case ' ':
			  case '\t':
			  case '\f':
			  case '\r':
				break;
			  case '"':
				do {
					while (*++sp != '"')
						continue;
				} while (sp[-1] == '\\');
				break;
			  case '\'':
				do {
					while (*++sp != '\'')
						continue;
				} while (sp[-1] == '\\');
				break;
			  case '/':
				if (*++sp == '*')
					sp = toss_comment(sp);
				else
					--sp;
				break;
			  case '{':
				brace++;
				break;
			  case '}':
				brace--;
				break;
			  case '%':
				if (sp[1] == '%' && sp == line)
					return;
				break;
			  case '|':
			  case ';':
				in_rule = FALSE;
				break;
			  default:
				if (brace == 0  && !in_rule && (isalpha(*sp) ||
								*sp == '.' ||
								*sp == '_')) {
					orig_sp = sp;
					++sp;
					while (isalnum(*sp) || *sp == '_' ||
					       *sp == '.')
						sp++;
					toklen = sp - orig_sp;
					while (isspace(*sp))
						sp++;
					if (*sp == ':' || (*sp == '\0' &&
							   first_char() == ':'))
					{
						strncpy(tok, orig_sp, toklen);
						tok[toklen] = '\0';
						strcpy(lbuf, line);
						lbuf[strlen(lbuf) - 1] = '\0';
						pfnote(tok, lineno, TRUE);
						in_rule = TRUE;
					}
					else
						sp--;
				}
				break;
			}
}

char *
toss_comment(start)
char	*start;
{
	register char	*sp;

	/*
	 * first, see if the end-of-comment is on the same line
	 */
	do {
		while ((sp = index(start, '*')) != NULL)
			if (sp[1] == '/')
				return ++sp;
			else
				start = ++sp;
		start = line;
		lineno++;
	} while (fgets(line, sizeof line, inf) != NULL);
}

#endif

getline()
{
	long saveftell = ftell( inf );
	register char *cp;

	fseek( inf , lineftell , 0 );
	fgets(lbuf, sizeof lbuf, inf);
	cp = rindex(lbuf, '\n');
	if (cp)
		*cp = 0;
	fseek(inf, saveftell, 0);
}

free_tree(node)
NODE	*node;
{

	while (node) {
		free_tree(node->right);
		cfree(node);
		node = node->left;
	}
}

add_node(node, cur_node)
	NODE *node,*cur_node;
{
	register int dif;

#if defined(_ATT)
	dif = strcmp(node->func,cur_node->func);
#endif
#if defined(BSDNEW)
	dif = strcmp(node->entry,cur_node->entry);
#endif
	if (dif == 0) {
		if (node->file == cur_node->file) {
			if (!wflag) {
#if defined(_ATT)
fprintf(stderr,"Duplicate function in file %s, line %d: %s\n",
    node->file,lineno,node->func);
fprintf(stderr,"Second entry ignored\n");
#endif
#if defined(BSDNEW)
fprintf(stderr, msg(M_602,"Duplicate entry in file %s, line %d: %s.  Second entry ignored.\n"),
node->file,lineno,node->entry);
#endif
			}
			return;
		}
		if (!cur_node->been_warned)
			if (!wflag)
#if defined(_ATT)
fprintf(stderr,"Duplicate function in files %s and %s: %s (Warning only)\n",
    node->file, cur_node->file, node->func);
#endif
#if defined(BSDNEW)
fprintf(stderr, msg(M_603,"Duplicate function in files %s and %s: %s (Warning only)\n"),
    node->file, cur_node->file, node->entry);
#endif
		cur_node->been_warned = TRUE;
		return;
	} 
	if (dif < 0) {
		if (cur_node->left != NULL)
			add_node(node,cur_node->left);
		else
			cur_node->left = node;
		return;
	}
	if (cur_node->right != NULL)
		add_node(node,cur_node->right);
	else
		cur_node->right = node;
}

#if defined(_ATT)
put_funcs(node)
reg NODE	*node;
{
	reg char	*sp;

	if (node == NULL)
		return;
	put_funcs(node->left);
	if (xflag == 0) {
		fprintf(outf, "%s\t%s\t%c^", node->func, node->file ,searchar);
		for (sp = node->pat; *sp; sp++)
			if (*sp == '\\')
				fprintf(outf, "\\\\");
			else
				putc(*sp, outf);
		fprintf(outf, "%c\n", searchar);
	}
	else
		fprintf(stdout, "%-16s%4d %-16s %s\n",
		    node->func, node->lno, node->file, node->pat);
	put_funcs(node->right);
}
#endif
#if defined(BSDNEW)
put_entries(node)
reg NODE	*node;
{
	reg char	*sp;

	if (node == NULL)
		return;
	put_entries(node->left);
	if (xflag == 0)
		if (node->f) {		/* a function */
			fprintf(outf, "%s\t%s\t%c^",
				node->entry, node->file, searchar);
			for (sp = node->pat; *sp; sp++)
				if (*sp == '\\')
					fprintf(outf, "\\\\");
				else if (*sp == searchar)
					fprintf(outf, "\\%c", searchar);
				else
					putc(*sp, outf);
			fprintf(outf, "%c\n", searchar);
		}
		else {		/* a typedef; text pattern inadequate */
			fprintf(outf, "%s\t%s\t%d\n",
				node->entry, node->file, node->lno);
		}
	else if (vflag)
		fprintf(stdout, "%s %s %d\n",
				node->entry, node->file, (node->lno+63)/64);
	else
		fprintf(stdout, "%-16s%4d %-16s %s\n",
			node->entry, node->lno, node->file, node->pat);
	put_entries(node->right);
}
#endif

char	*dbp = lbuf;
int	pfcnt;
PF_funcs(fi)
	FILE *fi;
{

	lineno = 0;
	pfcnt = 0;
	while (fgets(lbuf, sizeof(lbuf), fi)) {
		lineno++;
		dbp = lbuf;
		if ( *dbp == '%' ) dbp++ ;	/* Ratfor escape to fortran */
		while (isspace(*dbp))
			dbp++;
		if (*dbp == 0)
			continue;
		switch (*dbp |' ') {

		case 'i':
			if (tail("integer"))
				takeprec();
			break;
		case 'r':
			if (tail("real"))
				takeprec();
			break;
		case 'l':
			if (tail("logical"))
				takeprec();
			break;
		case 'c':
			if (tail("complex") || tail("character"))
				takeprec();
			break;
		case 'd':
			if (tail("double")) {
				while (isspace(*dbp))
					dbp++;
				if (*dbp == 0)
					continue;
				if (tail("precision"))
					break;
				continue;
			}
			break;
		}
		while (isspace(*dbp))
			dbp++;
		if (*dbp == 0)
			continue;
		switch (*dbp|' ') {

		case 'f':
			if (tail("function"))
				getit();
			continue;
		case 's':
			if (tail("subroutine"))
				getit();
			continue;
		case 'p':
			if (tail("program")) {
				getit();
				continue;
			}
			if (tail("procedure"))
				getit();
			continue;
		}
	}
	return (pfcnt);
}

tail(cp)
	char *cp;
{
	register int len = 0;

	while (*cp && (*cp&~' ') == ((*(dbp+len))&~' '))
		cp++, len++;
	if (*cp == 0) {
		dbp += len;
		return (1);
	}
	return (0);
}

takeprec()
{

	while (isspace(*dbp))
		dbp++;
	if (*dbp != '*')
		return;
	dbp++;
	while (isspace(*dbp))
		dbp++;
	if (!isdigit(*dbp)) {
		--dbp;		/* force failure */
		return;
	}
	do
		dbp++;
	while (isdigit(*dbp));
}

getit()
{
	register char *cp;
	char c;
	char nambuf[BUFFERSIZ];

	for (cp = lbuf; *cp; cp++)
		;
	*--cp = 0;	/* zap newline */
	while (isspace(*dbp))
		dbp++;
	if (*dbp == 0 || !isalpha(*dbp))
		return;
	for (cp = dbp+1; *cp && (isalpha(*cp) || isdigit(*cp)); cp++)
		continue;
	c = cp[0];
	cp[0] = 0;
	strcpy(nambuf, dbp);
	cp[0] = c;
	pfnote(nambuf, lineno);
	pfcnt++;
}

char *
savestr(cp)
	char *cp;
{
	register int len;
	register char *dp;

	len = strlen(cp);
	dp = (char *)malloc(len+1);
	strcpy(dp, cp);
	return (dp);
}

/*
 * Return the ptr in sp at which the character c last
 * appears; NULL if not found
 *
 * Identical to v7 rindex, included for portability.
 */

char *
rindex(sp, c)
register char *sp, c;
{
	register char *r;

	r = NULL;
	do {
		if (*sp == c)
			r = sp;
	} while (*sp++);
	return(r);
}
#if defined(BSDNEW)

/*
 * lisp tag functions
 * just look for (def or (DEF
 */

L_funcs (fi)
FILE *fi;
{
	register int	special;

	pfcnt = 0;
	while (fgets(lbuf, sizeof(lbuf), fi)) {
		lineno++;
		dbp = lbuf;
		if (dbp[0] == '(' &&
		    (dbp[1] == 'D' || dbp[1] == 'd') &&
		    (dbp[2] == 'E' || dbp[2] == 'e') &&
		    (dbp[3] == 'F' || dbp[3] == 'f')) {
			dbp += 4;
			if (striccmp(dbp, "method") == 0 ||
			    striccmp(dbp, "wrapper") == 0 ||
			    striccmp(dbp, "whopper") == 0)
				special = TRUE;
			else
				special = FALSE;
			while (!isspace(*dbp))
				dbp++;
			while (isspace(*dbp))
				dbp++;
			L_getit(special);
		}
	}
}

L_getit(special)
int	special;
{
	register char	*cp;
	register char	c;
	char		nambuf[BUFFERSIZ];

	for (cp = lbuf; *cp; cp++)
		continue;
	*--cp = 0;		/* zap newline */
	if (*dbp == 0)
		return;
	if (special) {
		if ((cp = index(dbp, ')')) == NULL)
			return;
		while (cp >= dbp && *cp != ':')
			cp--;
		if (cp < dbp)
			return;
		dbp = cp;
		while (*cp && *cp != ')' && *cp != ' ')
			cp++;
	}
	else
		for (cp = dbp + 1; *cp && *cp != '(' && *cp != ' '; cp++)
			continue;
	c = cp[0];
	cp[0] = 0;
	strcpy(nambuf, dbp);
	cp[0] = c;
	pfnote(nambuf, lineno,TRUE);
	pfcnt++;
}

/*
 * striccmp:
 *	Compare two strings over the length of the second, ignoring
 *	case distinctions.  If they are the same, return 0.  If they
 *	are different, return the difference of the first two different
 *	characters.  It is assumed that the pattern (second string) is
 *	completely lower case.
 */
striccmp(str, pat)
register char	*str, *pat;
{
	register int	c1;

	while (*pat) {
		if (isupper(*str))
			c1 = tolower(*str);
		else
			c1 = *str;
		if (c1 != *pat)
			return c1 - *pat;
		pat++;
		str++;
	}
	return 0;
}

/*
 * first_char:
 *	Return the first non-blank character in the file.  After
 *	finding it, rewind the input file so we start at the beginning
 *	again.
 */
first_char()
{
	register int	c;
	register long	off;

	off = ftell(inf);
	while ((c = getc(inf)) != EOF)
		if (!isspace(c) && c != '\r') {
			fseek(inf, off, 0);
			return c;
		}
	fseek(inf, off, 0);
	return EOF;
}

/*
 * toss_yysec:
 *	Toss away code until the next "%%" line.
 */
toss_yysec()
{
	char		buf[BUFFERSIZ];

	for (;;) {
		lineftell = ftell(inf);
		if (fgets(buf, BUFFERSIZ, inf) == NULL)
			return;
		lineno++;
		if (strncmp(buf, "%%", 2) == 0)
			return;
	}
}

#endif
