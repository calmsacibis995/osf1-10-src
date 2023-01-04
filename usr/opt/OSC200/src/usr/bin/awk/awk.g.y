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
/*	awk.g.y	4.2	87/09/16	*/

%token	FIRSTTOKEN	/*must be first*/
%token	FINAL FATAL
%token	LT LE GT GE EQ NE
%token	MATCH NOTMATCH
%token	APPEND
%token	ADD MINUS MULT DIVIDE MOD UMINUS 
%token	ASSIGN ADDEQ SUBEQ MULTEQ DIVEQ MODEQ
%token	JUMP
%token	XBEGIN XEND
%token	NL
%token	PRINT PRINTF SPRINTF SPLIT
%token	IF ELSE WHILE FOR IN NEXT EXIT BREAK CONTINUE
%token	PROGRAM PASTAT PASTAT2

%right	ASGNOP
%left	BOR
%left	AND
%left	NOT
%left	NUMBER VAR ARRAY FNCN SUBSTR LSUBSTR INDEX
%left	GETLINE
%nonassoc RELOP MATCHOP
%left	OR
%left	STRING  DOT CCL NCCL CHAR
%left	'(' '^' '$'
%left	CAT
%left	'+' '-'
%left	'*' '/' '%'
%left	STAR PLUS QUEST
%left	POSTINCR PREINCR POSTDECR PREDECR INCR DECR
%left	FIELD INDIRECT
%token	LASTTOKEN	/* has to be last */

%{
#include "awk.def"
#ifndef	DEBUG	
#	define	PUTS(x)
#endif
%}
%%

program:
	  begin pa_stats end	{ if (errorflag==0) winner = (node *)stat3((obj *) PROGRAM, (node *) $1, (node *) $2, (node *) $3); }
	| error			{ yyclearin; yyerror("bailing out"); }
	;

begin:
	  XBEGIN '{' stat_list '}'	{ PUTS("XBEGIN list"); $$ = (YYSTYPE) $3; }
	| begin NL
	| 	{ PUTS("empty XBEGIN"); $$ = (YYSTYPE) nullstat; }
	;

end:
	  XEND '{' stat_list '}'	{ PUTS("XEND list"); $$ = (YYSTYPE) $3; }
	| end NL
	|	{ PUTS("empty END"); $$ = (YYSTYPE) nullstat; }
	;

compound_conditional:
	  conditional BOR conditional	{ PUTS("cond||cond"); $$ = (YYSTYPE) op2((obj *) BOR, (node *) $1, (node *) $3); }
	| conditional AND conditional	{ PUTS("cond&&cond"); $$ = (YYSTYPE) op2((obj *) AND, (node *) $1, (node *) $3); }
	| NOT conditional		{ PUTS("!cond"); $$ = (YYSTYPE) op1((obj *) NOT, (node *) $2); }
	| '(' compound_conditional ')'	{ $$ = (YYSTYPE) $2; }
	;

compound_pattern:
	  pattern BOR pattern	{ PUTS("pat||pat"); $$ = (YYSTYPE) op2((obj *) BOR, (node *) $1, (node *) $3); }
	| pattern AND pattern	{ PUTS("pat&&pat"); $$ = (YYSTYPE) op2((obj *) AND, (node *) $1, (node *) $3); }
	| NOT pattern		{ PUTS("!pat"); $$ = (YYSTYPE) op1((obj *) NOT, (node *) $2); }
	| '(' compound_pattern ')'	{ $$ = (YYSTYPE) $2; }
	;

conditional:
	  expr	{ PUTS("expr"); $$ = (YYSTYPE) op2((obj *) NE, (node *) $1, valtonode((obj *) lookup("$zero&null", symtab, 0), CCON)); }
	| rel_expr		{ PUTS("relexpr"); }
	| lex_expr		{ PUTS("lexexpr"); }
	| compound_conditional	{ PUTS("compcond"); }
	;

else:
	  ELSE optNL	{ PUTS("else"); }
	;

field:
	  FIELD		{ PUTS("field"); $$ = (YYSTYPE) valtonode((obj *) $1, CFLD); }
	| INDIRECT term { PUTS("ind field"); $$ = (YYSTYPE) op1((obj *) INDIRECT, (node *) $2); }
	;

if:
	  IF '(' conditional ')' optNL	{ PUTS("if(cond)"); $$ = (YYSTYPE) $3; }
	;

lex_expr:
	  expr MATCHOP regular_expr	{ PUTS("expr~re"); $$ = (YYSTYPE) op2((obj *) $2, (node *) $1, (node *) makedfa((node *) $3)); }
	| '(' lex_expr ')'	{ PUTS("(lex_expr)"); $$ = (YYSTYPE) $2; }
	;

var:
	  NUMBER	{PUTS("number"); $$ = (YYSTYPE) valtonode((obj *) $1, CCON); }
	| STRING 	{ PUTS("string"); $$ = (YYSTYPE) valtonode((obj *) $1, CCON); }
	| VAR		{ PUTS("var"); $$ = (YYSTYPE) valtonode((obj *) $1, CVAR); }
	| VAR '[' expr ']'	{ PUTS("array[]"); $$ = (YYSTYPE) op2((obj *) ARRAY, (node *) $1, (node *) $3); }
	| field
	;
term:
	  var
	| GETLINE	{ PUTS("getline"); $$ = (YYSTYPE) op1((obj *) GETLINE, (node *) 0); }
	| FNCN		{ PUTS("func");
			$$ = (YYSTYPE) op2((obj *) FNCN, (node *) $1, valtonode((obj *) lookup("$record", symtab, 0), CFLD));
			}
	| FNCN '(' ')'	{ PUTS("func()"); 
			$$ = (YYSTYPE) op2((obj *) FNCN, (node *) $1, valtonode((obj *) lookup("$record", symtab, 0), CFLD));
			}
	| FNCN '(' expr ')'	{ PUTS("func(expr)"); $$ = (YYSTYPE) op2((obj *) FNCN, (node *) $1, (node *) $3); }
	| SPRINTF print_list	{ PUTS("sprintf"); $$ = (YYSTYPE) op1((obj *) $1, (node *) $2); }
	| SUBSTR '(' expr ',' expr ',' expr ')'
			{ PUTS("substr(e,e,e)"); $$ = (YYSTYPE) op3((obj *) SUBSTR, (node *) $3, (node *) $5, (node *) $7); }
	| SUBSTR '(' expr ',' expr ')'
			{ PUTS("substr(e,e,e)"); $$ = (YYSTYPE) op3((obj *) SUBSTR, (node *) $3, (node *) $5, nullstat); }
	| SPLIT '(' expr ',' VAR ',' expr ')'
			{ PUTS("split(e,e,e)"); $$ = (YYSTYPE) op3((obj *) SPLIT, (node *) $3, (node *) $5, (node *) $7); }
	| SPLIT '(' expr ',' VAR ')'
			{ PUTS("split(e,e,e)"); $$ = (YYSTYPE) op3((obj *) SPLIT, (node *) $3, (node *) $5, nullstat); }
	| INDEX '(' expr ',' expr ')'
			{ PUTS("index(e,e)"); $$ = (YYSTYPE) op2((obj *) INDEX, (node *) $3, (node *) $5); }
	| '(' expr ')'			{PUTS("(expr)");  $$ = (YYSTYPE) $2; }
	| term '+' term			{ PUTS("t+t"); $$ = (YYSTYPE) op2((obj *) ADD, (node *) $1, (node *) $3); }
	| term '-' term			{ PUTS("t-t"); $$ = (YYSTYPE) op2((obj *) MINUS, (node *) $1, (node *) $3); }
	| term '*' term			{ PUTS("t*t"); $$ = (YYSTYPE) op2((obj *) MULT, (node *) $1, (node *) $3); }
	| term '/' term			{ PUTS("t/t"); $$ = (YYSTYPE) op2((obj *) DIVIDE, (node *) $1, (node *) $3); }
	| term '%' term			{ PUTS("t%t"); $$ = (YYSTYPE) op2((obj *) MOD, (node *) $1, (node *) $3); }
	| '-' term %prec QUEST		{ PUTS("-term"); $$ = (YYSTYPE) op1((obj *) UMINUS, (node *) $2); }
	| '+' term %prec QUEST		{ PUTS("+term"); $$ = (YYSTYPE) $2; }
	| INCR var	{ PUTS("++var"); $$ = (YYSTYPE) op1((obj *) PREINCR, (node *) $2); }
	| DECR var	{ PUTS("--var"); $$ = (YYSTYPE) op1((obj *) PREDECR, (node *) $2); }
	| var INCR	{ PUTS("var++"); $$ = (YYSTYPE) op1((obj *) POSTINCR, (node *) $1); }
	| var DECR	{ PUTS("var--"); $$ = (YYSTYPE) op1((obj *) POSTDECR, (node *) $1); }
	;

expr:
	  term		{ PUTS("term"); }
	| expr term	{ PUTS("expr term"); $$ = (YYSTYPE) op2((obj *) CAT, (node *) $1, (node *) $2); }
	| var ASGNOP expr	{ PUTS("var=expr"); $$ = (YYSTYPE) stat2((obj *)$2, (node *) $1, (node *)$3); }
	;

optNL:
	  NL
	|
	;

pa_stat:
	  pattern	{ PUTS("pattern"); $$ = (YYSTYPE) stat2((obj *) PASTAT, (node *) $1, genprint()); }
	| pattern '{' stat_list '}'	{ PUTS("pattern {...}"); $$ = (YYSTYPE) stat2((obj *) PASTAT, (node *) $1, (node *) $3); }
	| pattern ',' pattern		{ PUTS("srch,srch"); $$ = (YYSTYPE) pa2stat((node *) $1, (node *) $3, genprint()); }
	| pattern ',' pattern '{' stat_list '}'	
					{ PUTS("srch, srch {...}"); $$ = (YYSTYPE) pa2stat((node *) $1, (node *) $3, (node *) $5); }
	| '{' stat_list '}'	{ PUTS("null pattern {...}"); $$ = (YYSTYPE) stat2((obj *) PASTAT, nullstat, (node *) $2); }
	;

pa_stats:
	  pa_stats pa_stat st	{ PUTS("pa_stats pa_stat"); $$ = (YYSTYPE) linkum((node *) $1, (node *) $2); }
	|	{ PUTS("null pa_stat"); $$ = (YYSTYPE) nullstat; }
	| pa_stats pa_stat	{PUTS("pa_stats pa_stat"); $$ = (YYSTYPE) linkum((node *) $1, (node *) $2); }
	;

pattern:
	  regular_expr	{ PUTS("regex");
		$$ = (YYSTYPE) op2((obj *) MATCH, valtonode((obj *) lookup("$record", symtab, 0), CFLD), (node *) makedfa((node *) $1));
		}
	| rel_expr	{ PUTS("relexpr"); }
	| lex_expr	{ PUTS("lexexpr"); }
	| compound_pattern	{ PUTS("comp pat"); }
	;

print_list:
	  expr	{ PUTS("expr"); }
	| pe_list	{ PUTS("pe_list"); }
	|		{ PUTS("null print_list"); $$ = (YYSTYPE) valtonode((obj *) lookup("$record", symtab, 0), CFLD); }
	;

pe_list:
	  expr ',' expr	{$$ = (YYSTYPE) linkum((node *) $1, (node *) $3); }
	| pe_list ',' expr	{$$ = (YYSTYPE) linkum((node *) $1, (node *) $3); }
	| '(' pe_list ')'		{$$ = (YYSTYPE) $2; }
	;

redir:
	  RELOP
	| '|'
	;

regular_expr:
	  '/'	{ startreg(); }
	  r '/'
		{ PUTS("/r/"); $$ = (YYSTYPE) $3; }
	;

r:
	  CHAR		{ PUTS("regex CHAR"); $$ = (YYSTYPE) op2((obj *) CHAR, (node *) 0, (node *) $1); }
	| DOT		{ PUTS("regex DOT"); $$ = (YYSTYPE) op2((obj *) DOT, (node *) 0, (node *) 0); }
	| CCL		{ PUTS("regex CCL"); $$ = (YYSTYPE) op2((obj *) CCL, (node *) 0, (node *) cclenter((char *) $1)); }
	| NCCL		{ PUTS("regex NCCL"); $$ = (YYSTYPE) op2((obj *) NCCL, (node *) 0, (node *) cclenter((char *) $1)); }
	| '^'		{ PUTS("regex ^"); $$ = (YYSTYPE) op2((obj *) CHAR, (node *) 0, (node *) HAT); }
	| '$'		{ PUTS("regex $"); $$ = (YYSTYPE) op2((obj *) CHAR, (node *) 0, (node *) 0); }
	| r OR r	{ PUTS("regex OR"); $$ = (YYSTYPE) op2((obj *) OR, (node *) $1, (node *) $3); }
	| r r   %prec CAT
			{ PUTS("regex CAT"); $$ = (YYSTYPE) op2((obj *) CAT, (node *) $1, (node *) $2); }
	| r STAR	{ PUTS("regex STAR"); $$ = (YYSTYPE) op2((obj *) STAR, (node *) $1, (node *) 0); }
	| r PLUS	{ PUTS("regex PLUS"); $$ = (YYSTYPE) op2((obj *) PLUS, (node *) $1, (node *) 0); }
	| r QUEST	{ PUTS("regex QUEST"); $$ = (YYSTYPE) op2((obj *) QUEST, (node *) $1, (node *) 0); }
	| '(' r ')'	{ PUTS("(regex)"); $$ = (YYSTYPE) $2; }
	;

rel_expr:
	  expr RELOP expr
		{ PUTS("expr relop expr"); $$ = (YYSTYPE) op2((obj *) $2, (node *) $1, (node *) $3); }
	| '(' rel_expr ')'
		{ PUTS("(relexpr)"); $$ = (YYSTYPE) $2; }
	;

st:
	  NL
	| ';'
	;

simple_stat:
	  PRINT print_list redir expr
		{ PUTS("print>stat"); $$ = (YYSTYPE) stat3((obj *) $1, (node *) $2, (node *) $3, (node *) $4); }
	| PRINT print_list	
		{ PUTS("print list"); $$ = (YYSTYPE) stat3((obj *) $1, (node *) $2, nullstat, nullstat); }
	| PRINTF print_list redir expr
		{ PUTS("printf>stat"); $$ = (YYSTYPE) stat3((obj *) $1, (node *) $2, (node *) $3, (node *) $4); }
	| PRINTF print_list	
		{ PUTS("printf list"); $$ = (YYSTYPE) stat3((obj *) $1, (node *) $2, nullstat, nullstat); }
	| expr	{ PUTS("expr"); $$ = (YYSTYPE) exptostat((node *) $1); }
	|		{ PUTS("null simple statement"); $$ = (YYSTYPE) nullstat; }
	| error		{ yyclearin; yyerror("illegal statement"); $$ = (YYSTYPE) nullstat; }
	;

statement:
	  simple_stat st	{ PUTS("simple stat"); }
	| if statement		{ PUTS("if stat"); $$ = (YYSTYPE) stat3((obj *) IF, (node *) $1, (node *) $2, nullstat); }
	| if statement else statement
		{ PUTS("if-else stat"); $$ = (YYSTYPE) stat3((obj *) IF, (node *) $1, (node *) $2, (node *) $4); }
	| while statement	{ PUTS("while stat"); $$ = (YYSTYPE) stat2((obj *) WHILE, (node *) $1, (node *) $2); }
	| for			{ PUTS("for stat"); }
	| NEXT st		{ PUTS("next"); $$ = (YYSTYPE) stat1((obj *) NEXT, (node *) 0); }
	| EXIT st		{ PUTS("exit"); $$ = (YYSTYPE) stat1((obj *) EXIT, (node *) 0); }
	| EXIT expr st		{ PUTS("exit"); $$ = (YYSTYPE) stat1((obj *) EXIT, (node *) $2); }
	| BREAK st		{ PUTS("break"); $$ = (YYSTYPE) stat1((obj *) BREAK, (node *) 0); }
	| CONTINUE st		{ PUTS("continue"); $$ = (YYSTYPE) stat1((obj *) CONTINUE, (node *) 0); }
	| '{' stat_list '}'	{ PUTS("{statlist}"); $$ = (YYSTYPE) $2; }
	;

stat_list:
	  stat_list statement	{ PUTS("stat_list stat"); $$ = (YYSTYPE) linkum((node *) $1, (node *) $2); }
	|			{ PUTS("null stat list"); $$ = (YYSTYPE) nullstat; }
	;

while:
	  WHILE '(' conditional ')' optNL	{ PUTS("while(cond)"); $$ = (YYSTYPE) $3; }
	;

for:
	  FOR '(' simple_stat ';' conditional ';' simple_stat ')' optNL statement
		{ PUTS("for(e;e;e)"); $$ = (YYSTYPE) stat4((obj *) FOR, (node *) $3, (node *) $5, (node *) $7, (node *) $10); }
	| FOR '(' simple_stat ';'  ';' simple_stat ')' optNL statement
		{ PUTS("for(e;e;e)"); $$ = (YYSTYPE) stat4((obj *) FOR, (node *) $3, nullstat, (node *) $6, (node *) $9); }
	| FOR '(' VAR IN VAR ')' optNL statement
		{ PUTS("for(v in v)"); $$ = (YYSTYPE) stat3((obj *) IN, (node *) $3, (node *) $5, (node *) $8); }
	;

%%

