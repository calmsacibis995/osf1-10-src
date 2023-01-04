%{
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

/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0
 */
/*
 * Copyright (c) 1989 Carnegie-Mellon University
 * All rights reserved.  The CMU software License Agreement specifies
 * the terms and conditions for use and redistribution.
 */

#ifndef lint
static	char *RCSid = "$Id: gram.y,v 4.1.3.2 1992/03/19 13:17:56 Al_Delorey Exp $";
static	char *sccsid = "@(#)gram.y	4.2 (Berkeley) 87/10/22";
#endif

#include "defs.h"

struct depchain {
	struct depblock *head;
	struct depblock *tail;
};

FSTATIC struct depblock *pp, *prevdep;
FSTATIC struct shblock *prevshp;
FSTATIC struct nameblock *prevlock;
FSTATIC int sepc;

FSTATIC int retsh(), nextlin(), GETC();
FSTATIC fstack();
%}

%token NAME LOCKNAME SHELLINE START MACRODEF
%token COLON DOUBLECOLON LPAREN RPAREN AMPERSAND

%union {
	struct shblock *yshblock;
	struct depblock *ydepblock;
	struct nameblock *ynameblock;
	struct depchain ydepchain;
}

%type <yshblock> SHELLINE, shlist, shellist
%type <ydepblock> deplist, dlist
%type <ydepchain> name, namelist, optnamelist
%type <ynameblock> NAME LOCKNAME


%%

file	: /* empty */
	| file comline
	;

comline	: START
	| MACRODEF
	| START dlist shellist
		/* NULL target - rule ignored */
	| START namelist deplist shellist
		{ rule($2.head, prevlock, $3, $4); }
	| error
		{
#ifdef lint
			YYERROR;
#endif
		}
	;

namelist: name
	| namelist name
		{
			$1.tail->nxtdepblock = $2.head;
			$$.head = $1.head;
			$$.tail = $2.tail;
		}
	;

deplist	: /* empty */
		{ nosepchar(); }
	| dlist
	;

dlist	: sepchar lockname
		{ $$ = 0; }
	| dlist AMPERSAND
		/* DYNIX style parallelism - currently just ignored */
	| dlist name
		{
			if ($1 == 0)
				$$ = $2.head;
			else
				prevdep->nxtdepblock = $2.head;
			prevdep = $2.tail;
		}
	;

name	: NAME
		{
			pp = ALLOC(depblock);
			pp->nxtdepblock = 0;
			pp->depname = $1;
			$$.head = $$.tail = pp;
		}
	| NAME LPAREN optnamelist RPAREN
		{
			for (pp = $3.head; pp; pp = pp->nxtdepblock)
				if (pp->depname) {
					pp->depname->archp = $1;
					$1->isarch = 1;
				}
			$$ = $3;
		}
	| NAME LPAREN LPAREN optnamelist RPAREN RPAREN
		{
			for (pp = $4.head; pp; pp = pp->nxtdepblock)
				if (pp->depname) {
					pp->depname->archp = $1;
					pp->depname->objarch = 1;
					$1->isarch = 1;
				}
			$$ = $4;
		}
	;

optnamelist: /* empty */
		{
			pp = ALLOC(depblock);
			pp->nxtdepblock = 0;
			pp->depname = 0;
			$$.head = $$.tail = pp;
		}
	| namelist
	;

sepchar	: COLON
		{ sepc = ALLDEPS; }
	| DOUBLECOLON
		{ sepc = SOMEDEPS; }
	;

lockname: /* empty */
		{ prevlock = 0; }
	| LOCKNAME
		{ prevlock = $1; }
	;

shellist: /* empty */
		{ $$ = 0; }
	| shlist
	;

shlist	: SHELLINE
		{ prevshp = $1; }
	| shlist SHELLINE
		{ prevshp = prevshp->nxtshblock = $2; }
	;

%%
extern char *builtin[];
FSTATIC char **linesptr	= builtin;

FILE *fin	= NULL;
int yylineno	= 0;
char *zznextc	= 0;


FSTATIC
rule(np, kp, dp, sp)
	register struct depblock *np;
	struct nameblock *kp;
	struct depblock *dp;
	struct shblock *sp;
{
	register struct nameblock *leftp;
	register struct lineblock *lp, *lpp;

	for (; np; np = np->nxtdepblock) {
		leftp = np->depname;
		if (leftp->septype == 0)
			leftp->septype = sepc;
		else if (leftp->septype != sepc)
			fprintf(stderr, "Inconsistent rules lines for `%s'\n", leftp->namep);
		else if (sepc == ALLDEPS && *(leftp->namep) != '.' && sp) {
			lp = leftp->linep;
			do {
				if (lp->shp == 0)
					continue;
				fprintf(stderr, "Multiple rules lines for `%s'\n", leftp->namep);
				break;
			} while (lp = lp->nxtlineblock);
		}

		lp = ALLOC(lineblock);
		lp->nxtlineblock = 0;
		lp->lockp = kp;
		lp->depp = dp;
		lp->shp = sp;

		if (!unequal(leftp->namep, ".SUFFIXES") && dp == 0)
			leftp->linep = 0;
		else if (leftp->linep == 0)
			leftp->linep = lp;
		else {
			lpp = leftp->linep;
			while (lpp->nxtlineblock)
				lpp = lpp->nxtlineblock;
			if (sepc == ALLDEPS
			&& leftp->namep[0] == '.'
			&& index(leftp->namep, '/') == 0)
				lpp->shp = 0;
			lpp->nxtlineblock = lp;
		}
	}
}


FSTATIC
nosepchar()
{
	fatal("%s: Must be a separator on line %d", curfname, yylineno);
}


FSTATIC int
yylex()
{
	register char *p;
	register char *q;
	register int n;
	char word[INMAX];

	if (zznextc == 0)
		return nextlin();

	while (isspace(*zznextc))
		++zznextc;
	if (*zznextc == 0)
		return nextlin();

	if (*zznextc == ':') {
		if (*++zznextc == ':') {
			++zznextc;
			return DOUBLECOLON;
		}
		return COLON;
	}

	if (*zznextc == '&') {
		++zznextc;
		return AMPERSAND;
	}

	if (*zznextc == '(' || *zznextc == ')')
		return (*zznextc++ == '(') ? LPAREN : RPAREN;

	if (*zznextc == '[') {
		p = zznextc;
		q = word;
		while (*++p != ']')
			if ((*q++ = *p) == 0)
				fatal("bad lock name: %s", zznextc+1);
		*q = 0;
		yylval.ynameblock = makename(word);
		zznextc = p + 1;
		return LOCKNAME;
	}

	if (*zznextc == ';')
		return retsh(zznextc);

	p = zznextc;
	q = word;

	for (;;) {
		if (p != zznextc && *(p-1) == '$') {
			/*
			 * we must treat macros specially
			 * since (, ), and : are terminals.
			 */
			n = gobblename(p, q);
			p += n;
			q += n;
			continue;
		}
		if (*p == '`') {
			/*
			 * wordify `...` for dynamic dependencies
			 */
			do
				*q++ = *p++;
			while (*p && *p != '`');
			if (*p == 0)
				break;
			*q++ = *p++;
			continue;
		}
		if (funny[*p] & TERMINAL)
			break;
		*q++ = *p++;
	}

	if (p != zznextc) {
		*q = 0;
		yylval.ynameblock = makename(word);
		zznextc = p;
		return NAME;
	}

	fprintf(stderr,"Bad character %c (octal %o), file %s line %d",
			*zznextc, *zznextc, curfname, yylineno);
	fatal((char *) 0);
	/*NOTREACHED*/
}


FSTATIC int
retsh(q)
	char *q;
{
	register char *p;
	struct shblock *sp;
	extern char *copys();
	extern char *sindex();

	for (p = q+1; isspace(*p); ++p)
		;
	sp = ALLOC(shblock);
	sp->nxtshblock = 0;
	sp->shbp = copys(p);
	sp->exok = (sindex(p, "$(MAKE)") || sindex(p, "${MAKE}"));
	yylval.yshblock = sp;
	zznextc = 0;
	return SHELLINE;
}


FSTATIC int
nextlin()
{
	register int c, kc;
	register char *p, *t;
	int incom, indep;
	char *incmd, *text, templin[INMAX];
	static char yytext[INMAX];
	static char *yytextl = yytext+INMAX;

again:
	incom = 0;
	indep = 0;
	incmd = 0;
	zznextc = 0;

	if (fin == NULL) {
		if (*linesptr == 0)
			return 0;
		++yylineno;
		p = text = yytext;
		for (t = *linesptr++; p != yytextl; *p++ = *t++)
			if (*t == 0)
				break;
		if (p == yytextl)
			fatal("line too long");
		*p = 0;
	} else {
		for (p = text = yytext; p < yytextl; *p++ = kc)
			switch (kc = GETC()) {
			case 0:
				goto again;
			case '\t':
				if (p != yytext)
					break;
			case ';':
				if (!incom && !indep && incmd == 0)
					incmd = p;
				break;
			case '#':
				if (incmd)
					break;
				if (p == yytext || p[-1] != '\\')
					incom = 1, kc = 0;
				else
					--p;
				break;
			case '`':
				indep = !indep;
				break;
			case '\n':
				++yylineno;
				if (p == yytext || p[-1] != '\\') {
					*p = 0;
					goto endloop;
				}
				p[-1] = ' ';
				while ((kc = GETC()) != EOF && isspace(kc))
					if (kc == '\n')
						++yylineno;
				if (kc != EOF)
					break;
			case EOF:
				*p = 0;
				return 0;
			}

		fatal("line too long");
	}

endloop:
	if ((c = text[0]) == '\t')
		return retsh(text);

	/*
	 * DO include FILES HERE.
	 */
	kc = 0;
	if (c == '-' && strncmp(text, "-include", 8) == 0) {
		kc = 1;
		text++;
	}
	if (text[0] == 'i' && strncmp(text, "include", 7) == 0
	&& (text[7] == ' ' || text[7] == '\t')) {
		char *pfile;

		for (p = text + 8; *p; p++)
			if (*p != ' ' && *p != '\t')
				break;
		pfile = p;
		while (*p && !isspace(*p))
			p++;
		*p = 0;
		/*
		 * Start using new file.
		 */
		fstack(pfile, &fin, &yylineno, kc == 0);
		goto again;
	}
	if (kc)
		text--;

	if (isalpha(c) || isdigit(c) || c == '_'
	|| c == ' ' || c == '.' || c == '$')
		for (p = text+1; *p; p++) {
			if (*(p-1) == '$')
				p += gobblename(p, templin);
			if (*p == ':') {
				if (*(p+1) != '=')
					break;
				(void) teqsign(text, p);
				return MACRODEF;
			}
			if (*p == '=') {
				(void) eqsign(text, p, 0);
				return MACRODEF;
			}
		}

	/*
	 * substitute for macros on dependency
	 * line up to the semicolon if any
	 */
	if (incmd)
		*incmd = 0;
	t = subst(text, templin);
	if (incmd) {
		*t = ';';
		while (*++t = *++incmd)
			;
	}

	p = templin;
	t = text;
	while (*t++ = *p++)
		;
	for (p = zznextc = text; *p; ++p)
		if (*p != ' ' && *p != '\t')
			return START;
	goto again;
}


/*
 * GETC automatically unravels stacked include files. That is,
 * during include file processing, when a new file is encountered
 * fstack will stack the FILE pointer argument. Subsequent
 * calls to GETC with the new FILE pointer will get characters
 * from the new file. When an EOF is encountered, GETC will
 * check to see if the file pointer has been stacked. If so,
 * a character from the previous file will be returned.
 * The external references are "GETC()" and "fstack(fname,stream,lno)".
 * "Fstack(stfname,ream,lno)" is used to stack an old file pointer before
 * the new file is assigned to the same variable. Also stacked are the
 * file name and the old current lineno, generally, yylineno.
 */

FSTATIC int maxifiles = 0;
FSTATIC int morefiles = 0;
FSTATIC struct sfiles_tag {
	char *sfname;
	FILE *sfilep;
	char *sfilen;
	int syylno;
} *sfiles;


FSTATIC int
GETC()
{
	register int c;

	if (fin == NULL)
		fatal("NULL fin in GETC");
	c = getc(fin);
	while (c == EOF && morefiles) {
		(void) fclose(fin);
		free(sfiles[morefiles].sfname);
		sfiles[morefiles].sfname = NULL;
		yylineno = sfiles[--morefiles].syylno;
		curfname = sfiles[morefiles].sfilen;
		if ((fin = sfiles[morefiles].sfilep) == NULL) {
			free(sfiles);
			sfiles = NULL;
			return 0;
		}
		c = getc(fin);
	}
	return c;
}


FSTATIC
fstack(fname, oldfp, oldlno, ferr)
	register char *fname;
	register FILE **oldfp;
	register int *oldlno;
	int ferr;
{
	register char *p, *q, *r;
	register struct nameblock *np;
	FILE *savfp;
	char incname[MAXPATHLEN+1];
	char newname[MAXPATHLEN+1];

	(void) subst(fname, incname);
	if (*incname == 0)
		fatal("No include file name");
	p = newname;
	if (*incname != '/') {
		r = 0;
		for (q = curfname; *q; *p++ = *q++)
			if (*q == '/')
				r = p + 1;
		p = r ? r : newname;
	}
	for (q = incname; *q; *p++ = *q++)
		;
	*p = 0;
	if (dbgflag)
		printf("Include file: \"%s\"\n", newname);
	savfp = *oldfp;
	setvpath();
	np = rcsco(newname);
	if ((*oldfp = fopen(np->alias ? np->alias : newname, "r")) == NULL) {
		if (ferr)
			fatal("Cannot open %s", newname);
		if (dbgflag)
			printf("\"%s\": ignored missing include\n", newname);
		*oldfp = savfp;
		return;
	}
	/*
	 * Stack the new file name, the old file pointer and the
	 * old yylineno.  First dynamically allocate the sfiles
	 * structure array.
	 */
	if (sfiles == (struct sfiles_tag *) NULL) {
		maxifiles = getdtablesize();
		sfiles = (struct sfiles_tag *)
			 calloc(maxifiles,sizeof(struct sfiles_tag));
		if (sfiles == (struct sfiles_tag *) NULL)
			fatal("out of memory");
	}
	if (sfiles[morefiles].sfname == (char *) NULL) {
		sfiles[morefiles].sfname = (char *)
					   calloc(MAXPATHLEN+1,sizeof(char));
		if (sfiles[morefiles].sfname == (char *) NULL)
			fatal("out of memory");
	}
	(void) strcpy(sfiles[morefiles].sfname, newname);
	sfiles[morefiles].sfilep = savfp;
	sfiles[morefiles].sfilen = curfname;
	sfiles[morefiles].syylno = *oldlno;
	curfname = sfiles[morefiles++].sfname;
	if (morefiles >= maxifiles)
		fatal("open file limit exceeded");
	yylineno = 0;
}
