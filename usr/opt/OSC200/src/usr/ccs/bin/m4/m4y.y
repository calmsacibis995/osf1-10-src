/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0
 */
/*
 * @(#)$RCSfile: m4y.y,v $ $Revision: 4.1 $ (OSF) $Date: 1991/09/18 22:53:12 $  
 */
/*
 * COMPONENT_NAME: (CMDAS) Assembler and Macroprocessor 
 *
 * FUNCTIONS: peek, yyerror, yylex
 *
 * ORIGINS: 3
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 * OBJECT CODE ONLY SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * m4y.y	1.4  com/cmd/as/m4,3.1,9013 9/13/89 17:28:29 
 */

%{
extern long	evalval;
#define	YYSTYPE	long
%}

%term DIGITS
%left OROR
%left ANDAND
%left '|' '^'
%left '&'
%right '!' '~'
%nonassoc GT GE LT LE NE EQ
%left '+' '-'
%left '*' '/' '%'
%right POWER
%right UMINUS
%%

s	: e	={ evalval = $1; }
	|	={ evalval = 0; }
	;

e	: e OROR e	={ $$ = ($1!=0 || $3!=0) ? 1 : 0; }
	| e ANDAND e	={ $$ = ($1!=0 && $3!=0) ? 1 : 0; }
	| '!' e		={ $$ = $2 == 0; }
	| '~' e		={ $$ = ~$2; }
	| e EQ e	={ $$ = $1 == $3; }
	| e NE e	={ $$ = $1 != $3; }
	| e GT e	={ $$ = $1 > $3; }
	| e GE e	={ $$ = $1 >= $3; }
	| e LT e	={ $$ = $1 < $3; }
	| e LE e	={ $$ = $1 <= $3; }
	| e '|' e	={ $$ = ($1|$3); }
	| e '&' e	={ $$ = ($1&$3); }
	| e '^' e	={ $$ = ($1^$3); }
	| e '+' e	={ $$ = ($1+$3); }
	| e '-' e	={ $$ = ($1-$3); }
	| e '*' e	={ $$ = ($1*$3); }
	| e '/' e	={ $$ = ($1/$3); }
	| e '%' e	={ $$ = ($1%$3); }
	| '(' e ')'	={ $$ = ($2); }
	| e POWER e	={ for ($$=1; $3-->0; $$ *= $1); }	
	| '-' e %prec UMINUS	={ $$ = $2-1; $$ = -$2; }
	| '+' e %prec UMINUS	={ $$ = $2-1; $$ = $2; }
	| DIGITS	={ $$ = evalval; }
	;

%%

extern char *pe;

yylex() {

	while (*pe==' ' || *pe=='\t' || *pe=='\n')
		pe++;
	switch(*pe) {
	case '\0':
	case '+':
	case '-':
	case '/':
	case '%':
	case '^':
	case '~':
	case '(':
	case ')':
		return(*pe++);
	case '*':
		return(peek('*', POWER, '*'));
	case '>':
		return(peek('=', GE, GT));
	case '<':
		return(peek('=', LE, LT));
	case '=':
		return(peek('=', EQ, EQ));
	case '|':
		return(peek('|', OROR, '|'));
	case '&':
		return(peek('&', ANDAND, '&'));
	case '!':
		return(peek('=', NE, '!'));
	default: {
		register	base;

		evalval = 0;

		if (*pe == '0') {
			if (*++pe=='x' || *pe=='X') {
				base = 16;
				++pe;
			} else
				base = 8;
		} else
			base = 10;

		for (;;) {
			register	c, dig;

			c = *pe;

			if (c>='0' && c<='9')
				dig = c - '0';
			else if (c>='a' && c<='f')
				dig = c - 'a' + 10;
			else if (c>='A' && c<='F')
				dig = c - 'A' + 10;
			else
				break;

			evalval = evalval*base + dig;
			++pe;
		}

		return(DIGITS);
	}
	}
}

peek(c, r1, r2)
{
	if (*++pe != c)
		return(r2);
	++pe;
	return(r1);
}

/* VARARGS */
yyerror() {;}
