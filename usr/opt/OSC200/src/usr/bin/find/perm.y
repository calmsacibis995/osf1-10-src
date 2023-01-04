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
/*
 * HISTORY
 */
/*
 * OSF/1 1.2
 */
%{

#if !defined(lint) && !defined(_NOIDENT)
static char rcsid[] = "@(#)$RCSfile: perm.y,v $ $Revision: 1.1.5.2 $ (DEC) $Date: 1993/06/10 21:52:25 $";
#endif

#include <sys/mode.h>
#include <sys/types.h>
#include <nl_types.h>
#include <stdio.h>
#include "find_msg.h"


extern nl_catd	catd;

static mode_t	permission;	/* computed permission bits */

#define	MSGSTR(Num, Str) catgets(catd,MS_FIND, Num, Str)
%}

%start		symbolic_mode
%%

symbolic_mode	: clause
  	       	| symbolic_mode ',' clause
  		;

clause		: actionlist		{ $$ = $1; }
  		| wholist actionlist	{ $$ = permission |= ($1 & $2); }
  		;

wholist		: who		{ $$ = $1; }
		| wholist who	{ $$ = $1 | $2; }
		;

who		: 'u'	{ $$ = S_IRWXU|S_ISUID; }
		| 'g'	{ $$ = S_IRWXG|S_ISGID; }
		| 'o'	{ $$ = S_IRWXO; }
		| 'a'	{ $$ = S_IRWXU | S_IRWXG | S_IRWXO | S_ISUID | S_ISGID; }
		;

actionlist	: action		{ $$ = $1; }
		| actionlist action	{ $$ = $1 | $2; }
		;

action		: op		{ $$ = list($1,0); }
		| op permlist	{ $$ = list($1,$2); }
		| op permcopy	{ $$ = list($1,$2); }
		;

permcopy	: 'u'	{ mode_t t = (permission&S_IRWXU); $$ = (t>>3) | (t>>6); }
		| 'g'	{ mode_t t = (permission&S_IRWXG); $$ = (t<<3) | (t>>3); }
		| 'o'	{ mode_t t = (permission&S_IRWXO); $$ = (t<<3) | (t<<6); }
		;

op		: '+'	{ $$ = '+'; }
		| '-'	{ $$ = '-'; }
		| '='	{ $$ = '='; }
		;

permlist	: perm			{ $$ = $1; }
		| perm permlist		{ $$ = $1 | $2; }
		;

perm		: 'r'	{ $$ = S_IRUSR | S_IRGRP | S_IROTH; }
		| 'w'	{ $$ = S_IWUSR | S_IWGRP | S_IWOTH; }
		| 'x'	{ $$ = S_IXUSR | S_IXGRP | S_IXOTH; }
		| 'X'	{ $$ = S_IXUSR | S_IXGRP | S_IXOTH; }
		| 's'	{ $$ = S_ISUID | S_ISGID; }
		;

%%

static void yyerror(char *s)
{
  fprintf( stderr, MSGSTR(BADPERM, "find: invalid perm argument") );
  exit(2);
}

static const char *permstring;

static int yylex()
{
  if(!permstring || !*permstring) return EOF;

  return *permstring++;
}

static int list(op,mode)
char op;
mode_t mode;
{
  switch(op) {
  case '+':	return mode;
  case '-':	return 0;
  case '=':	return 0;
  }
}

mode_t permissions( const char *perm )
{
  permstring = perm;

  permission = 0;

  yyparse();

  return permission;
}

