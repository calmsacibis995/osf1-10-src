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
static char *sccsid = "@(#)$RCSfile: rpc_util.c,v $ $Revision: 4.2.5.2 $ (DEC) $Date: 1993/04/15 19:41:54 $";
#endif
/*
 */
/*
 * OSF/1 Release 1.0
 */

/* 
 * Portions of this software have been licensed to
 * Digital Equipment Company, Maynard, MA.
 * Copyright (c) 1988 by Sun Microsystems, Inc.
 * from SUN 1.7
 */

/*
 * rpc_util.c, Utility routines for the RPC protocol compiler 
 */
#include <stdio.h>
#include "rpc_scan.h"
#include "rpc_parse.h"
#include "rpc_util.h"

char curline[MAXLINESIZE];	/* current read line */
char *where = curline;	/* current point in line */
int linenum = 0;	/* current line number */

char *infilename;	/* input filename */

#define NFILES 5
char *outfiles[NFILES];		/* output file names */
int nfiles;

FILE *fout;	/* file pointer of current output */
FILE *fin;	/* file pointer of current input */

list *defined;	/* list of defined things */

/*
 * Reinitialize the world 
 */
reinitialize()
{
	bzero(curline, MAXLINESIZE);
	where = curline;
	linenum = 0;
	defined = NULL;
}

/*
 * string equality 
 */
streq(a, b)
	char *a;
	char *b;
{
	return (strcmp(a, b) == 0);
}

/*
 * find a value in a list 
 */
char *
findval(lst, val, cmp)
	list *lst;
	char *val;
	int (*cmp) ();

{
	for (; lst != NULL; lst = lst->next) {
		if ((*cmp) (lst->val, val)) {
			return (lst->val);
		}
	}
	return (NULL);
}

/*
 * store a value in a list 
 */
void
storeval(lstp, val)
	list **lstp;
	char *val;
{
	list **l;
	list *lst;

	for (l = lstp; *l != NULL; l = (list **) & (*l)->next);
	lst = ALLOC(list);
	lst->val = val;
	lst->next = NULL;
	*l = lst;
}


static
findit(def, type)
	definition *def;
	char *type;
{
	return (streq(def->def_name, type));
}


static char *
fixit(type, orig)
	char *type;
	char *orig;
{
	definition *def;

	def = (definition *) FINDVAL(defined, type, findit);
	if (def == NULL || def->def_kind != DEF_TYPEDEF) {
		return (orig);
	}
	switch (def->def.ty.rel) {
	case REL_VECTOR:
		return (def->def.ty.old_type);
	case REL_ALIAS:
		return (fixit(def->def.ty.old_type, orig));
	default:
		return (orig);
	}
}

char *
fixtype(type)
	char *type;
{
	return (fixit(type, type));
}

char *
stringfix(type)
	char *type;
{
	if (streq(type, "string")) {
		return ("wrapstring");
	} else {
		return (type);
	}
}

void
ptype(prefix, type, follow)
	char *prefix;
	char *type;
	int follow;
{
	if (prefix != NULL) {
		if (streq(prefix, "enum")) {
			f_print(fout, "enum ");
		} else {
			f_print(fout, "struct ");
		}
	}
	if (streq(type, "bool")) {
		f_print(fout, "bool_t ");
	} else if (streq(type, "string")) {
		f_print(fout, "char *");
	} else {
		f_print(fout, "%s ", follow ? fixtype(type) : type);
	}
}


static
typedefed(def, type)
	definition *def;
	char *type;
{
	if (def->def_kind != DEF_TYPEDEF || def->def.ty.old_prefix != NULL) {
		return (0);
	} else {
		return (streq(def->def_name, type));
	}
}

isvectordef(type, rel)
	char *type;
	relation rel;
{
	definition *def;

	for (;;) {
		switch (rel) {
		case REL_VECTOR:
			return (!streq(type, "string"));
		case REL_ARRAY:
			return (0);
		case REL_POINTER:
			return (0);
		case REL_ALIAS:
			def = (definition *) FINDVAL(defined, type, typedefed);
			if (def == NULL) {
				return (0);
			}
			type = def->def.ty.old_type;
			rel = def->def.ty.rel;
		}
	}
}

char *
locase(str)
	char *str;
{
	char c;
	static char buf[100];
	char *p = buf;

	while (c = *str++) {
		*p++ = (c >= 'A' && c <= 'Z') ? (c - 'A' + 'a') : c;
	}
	*p = 0;
	return (buf);
}


void
pvname(pname, vnum)
	char *pname;
	char *vnum;
{
	f_print(fout, "%s_%s", locase(pname), vnum);
}


/*
 * print a useful (?) error message, and then die 
 */
void
error(msg)
	char *msg;
{
	printwhere();
	f_print(stderr, "%s, line %d: ", infilename, linenum);
	f_print(stderr, "%s\n", msg);
	crash();
}

/*
 * Something went wrong, unlink any files that we may have created and then
 * die. 
 */
crash()
{
	int i;

	for (i = 0; i < nfiles; i++) {
		(void) unlink(outfiles[i]);
	}
	exit(1);
}


void
record_open(file)
	char *file;
{
	if (nfiles < NFILES) {
		outfiles[nfiles++] = file;
	} else {
		f_print(stderr, "too many files!\n");
		crash();
	}
}

static char expectbuf[100];
static char *toktostr();

/*
 * error, token encountered was not the expected one 
 */
void
expected1(exp1)
	tok_kind exp1;
{
	s_print(expectbuf, "expected '%s'",
		toktostr(exp1));
	error(expectbuf);
}

/*
 * error, token encountered was not one of two expected ones 
 */
void
expected2(exp1, exp2)
	tok_kind exp1, exp2;
{
	s_print(expectbuf, "expected '%s' or '%s'",
		toktostr(exp1),
		toktostr(exp2));
	error(expectbuf);
}

/*
 * error, token encountered was not one of 3 expected ones 
 */
void
expected3(exp1, exp2, exp3)
	tok_kind exp1, exp2, exp3;
{
	s_print(expectbuf, "expected '%s', '%s' or '%s'",
		toktostr(exp1),
		toktostr(exp2),
		toktostr(exp3));
	error(expectbuf);
}

void
tabify(f, tab)
	FILE *f;
	int tab;
{
	while (tab--) {
		(void) fputc('\t', f);
	}
}



static token tokstrings[] = {
			     {TOK_IDENT, "identifier"},
			     {TOK_CONST, "const"},
			     {TOK_RPAREN, ")"},
			     {TOK_LPAREN, "("},
			     {TOK_RBRACE, "}"},
			     {TOK_LBRACE, "{"},
			     {TOK_LBRACKET, "["},
			     {TOK_RBRACKET, "]"},
			     {TOK_STAR, "*"},
			     {TOK_COMMA, ","},
			     {TOK_EQUAL, "="},
			     {TOK_COLON, ":"},
			     {TOK_SEMICOLON, ";"},
			     {TOK_UNION, "union"},
			     {TOK_STRUCT, "struct"},
			     {TOK_SWITCH, "switch"},
			     {TOK_CASE, "case"},
			     {TOK_DEFAULT, "default"},
			     {TOK_ENUM, "enum"},
			     {TOK_TYPEDEF, "typedef"},
			     {TOK_INT, "int"},
			     {TOK_SHORT, "short"},
			     {TOK_LONG, "long"},
			     {TOK_UNSIGNED, "unsigned"},
			     {TOK_DOUBLE, "double"},
			     {TOK_FLOAT, "float"},
			     {TOK_CHAR, "char"},
			     {TOK_STRING, "string"},
			     {TOK_OPAQUE, "opaque"},
			     {TOK_BOOL, "bool"},
			     {TOK_VOID, "void"},
			     {TOK_PROGRAM, "program"},
			     {TOK_VERSION, "version"},
			     {TOK_HYPER, "hyper"},
			     {TOK_EOF, "??????"}
};

static char *
toktostr(kind)
	tok_kind kind;
{
	token *sp;

	for (sp = tokstrings; sp->kind != TOK_EOF && sp->kind != kind; sp++);
	return (sp->str);
}



static
printbuf()
{
	char c;
	int i;
	int cnt;

#	define TABSIZE 4

	for (i = 0; c = curline[i]; i++) {
		if (c == '\t') {
			cnt = 8 - (i % TABSIZE);
			c = ' ';
		} else {
			cnt = 1;
		}
		while (cnt--) {
			(void) fputc(c, stderr);
		}
	}
}


static
printwhere()
{
	int i;
	char c;
	int cnt;

	printbuf();
	for (i = 0; i < where - curline; i++) {
		c = curline[i];
		if (c == '\t') {
			cnt = 8 - (i % TABSIZE);
		} else {
			cnt = 1;
		}
		while (cnt--) {
			(void) fputc('^', stderr);
		}
	}
	(void) fputc('\n', stderr);
}
