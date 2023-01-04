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
static char     *sccsid = "@(#)$RCSfile: scan.c,v $ $Revision: 1.1.5.3 $ (DEC) $Date: 1993/12/20 21:31:00 $";
#endif
/*
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 */
/*
 * OSF/1 1.2
 */
/*
 * COMPONENT_NAME: (CMDLOC) Locale Database Commands
 *
 * FUNCTIONS:
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1991
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 * 
 * 1.8  com/cmd/nls/scan.c, cmdnls, bos320, 9132320m 8/11/91 14:08:48
 */
#undef _AIX
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <limits.h>
#include <ctype.h>
#include "semstack.h"
#include "symtab.h"
#include "gram.h"
#include "err.h"

#define YYTEXT_SIZE  16384	/* line continuation requires big bufs. */
char yytext[YYTEXT_SIZE+1];
char sym_text[MAX_SYM_LEN+1];		/* Bigger than largest possible symbol name */

int value = 0;
int	skip_to_EOL=0;		/* Flag to ignore characters til EOL */

#define MAX_KW_LEN  32
typedef struct {
    char key[MAX_KW_LEN+1];
    int  token_val;
} keyword_t;

/*
 * Keywords for lexical scan.  THIS TABLE MUST BE IN ASCII ALPHABETIC
 * ORDER!!!  bsearch() is used to look things up in it.
 */
static const
keyword_t kw_tbl[]={
"<code_set_name>",   KW_CODESET,
"<comment_char>",    KW_COMMENT_CHAR,
"<escape_char>",     KW_ESC_CHAR,
"<mb_cur_max>",      KW_MB_CUR_MAX,
"<mb_cur_min>",      KW_MB_CUR_MIN,
"CHARMAP",           KW_CHARMAP,
#ifdef _AIX
"CHARSETID",         KW_CHARSETID,
#endif
"DISPWID",           KW_DISPWID,
"END",               KW_END,
"LC_COLLATE",        KW_LC_COLLATE,
"LC_CTYPE",          KW_LC_CTYPE,
"LC_MESSAGES",       KW_LC_MSG,
"LC_MONETARY",       KW_LC_MONETARY,
"LC_NUMERIC",        KW_LC_NUMERIC,
"LC_TIME",           KW_LC_TIME,
"METHODS",           KW_METHODS,
"__mbstopcs",	     KW_MBSTOPCS,
"__mbtopc",	     KW_MBTOPC,
"__pcstombs",	     KW_PCSTOMBS,
"__pctomb",	     KW_PCTOMB,
"abday",             KW_ABDAY,
"abmon",             KW_ABMON,
"alt_digits",        KW_ALT_DIGITS,
"am_pm",             KW_AM_PM,
"backward",          KW_BACKWARD,
"charclass",	     KW_CHARCLASS,
"collating-element", KW_COLLATING_ELEMENT,
"collating-symbol",  KW_COLLATING_SYMBOL,
"copy",              KW_COPY,
"credit_sign",       KW_CREDIT_SIGN,
#ifdef _AIX
"csid",		     KW_CSID,
#endif /* _AIX */
"currency_symbol",   KW_CURRENCY_SYMBOL,
"d_fmt",             KW_D_FMT,
"d_t_fmt",           KW_D_T_FMT,
"day",               KW_DAY,
"debit_sign",        KW_DEBIT_SIGN,
"decimal_point",     KW_DECIMAL_POINT,
"era",               KW_ERA,
"era_d_fmt",         KW_ERA_D_FMT,
"era_d_t_fmt",	     KW_ERA_D_T_FMT,
"era_t_fmt",	     KW_ERA_T_FMT,
"era_year",          KW_ERA_YEAR,
"forward",           KW_FORWARD,
"frac_digits",       KW_FRAC_DIGITS,
"from",              KW_FROM,
"grouping",          KW_GROUPING,
"int_curr_symbol",   KW_INT_CURR_SYMBOL,
"int_frac_digits",   KW_INT_FRAC_DIGITS,
"iswctype",	     KW_ISWCTYPE,
"left_parenthesis",  KW_LEFT_PARENTHESIS,
"m_d_old",           KW_M_D_OLD,
"m_d_recent",        KW_M_D_RECENT,
"mblen",	     KW_MBLEN,
"mbstowcs",	     KW_MBSTOWCS,
"mbtowc",	     KW_MBTOWC,
"mon",               KW_MON,
"mon_decimal_point", KW_MON_DECIMAL_POINT,
"mon_grouping",      KW_MON_GROUPING,
"mon_thousands_sep", KW_MON_THOUSANDS_SEP,
"n_cs_precedes",     KW_N_CS_PRECEDES,
"n_sep_by_space",    KW_N_SEP_BY_SPACE,
"n_sign_posn",       KW_N_SIGN_POSN,
"negative_sign",     KW_NEGATIVE_SIGN,
"no-substitute",     KW_NO_SUBSTITUTE,
"noexpr",	     KW_NOEXPR,
"nostr",             KW_NOSTR,
"order_end",         KW_ORDER_END,
"order_start",       KW_ORDER_START,
"p_cs_precedes",     KW_P_CS_PRECEDES,
"p_sep_by_space",    KW_P_SEP_BY_SPACE,
"p_sign_posn",       KW_P_SIGN_POSN,
"position",          KW_POSITION,
"positive_sign",     KW_POSITIVE_SIGN,
"regcomp",	     KW_REGCOMP,
"regerror",	     KW_REGERROR,
"regexec",	     KW_REGEXEC,
"regfree",	     KW_REGFREE,
"right_parenthesis", KW_RIGHT_PARENTHESIS,
"strcoll",	     KW_STRCOLL,
"strfmon",	     KW_STRFMON,
"strftime",	     KW_STRFTIME,
"strptime",	     KW_STRPTIME,
"strxfrm",	     KW_STRXFRM,
"substitute",        KW_SUBSTITUTE,
"t_fmt",             KW_T_FMT,
"t_fmt_ampm",        KW_T_FMT_AMPM,
"thousands_sep",     KW_THOUSANDS_SEP,
"tolower",           KW_TOLOWER,
"toupper",           KW_TOUPPER,
"towlower",	     KW_TOWLOWER,
"towupper",	     KW_TOWUPPER,
"wcscoll",	     KW_WCSCOLL,
"wcsftime",	     KW_WCSFTIME,
#ifdef _AIX
"wcsid",	     KW_WCSID,
#endif
"wcstombs",	     KW_WCSTOMBS,
"wcswidth",	     KW_WCSWIDTH,
"wcsxfrm",	     KW_WCSXFRM,
"wctomb",	     KW_WCTOMB,
"wctype",	     KW_WCTYPE,
"wcwidth",	     KW_WCWIDTH,
"with",              KW_WITH,
"yesexpr",           KW_YESEXPR,
"yesstr",            KW_YESSTR,
};

#define KW_TBL_SZ  (sizeof(kw_tbl) / sizeof(kw_tbl[0]))
#define NIL(t)     ((t *)NULL)

void scan_error(int);

/*
SYM [0-9A-Za-z_-]
LTR [a-zA-Z_-]
HEX [0-9a-fA-F]
*/

/* character position tracking for error messages */
int yycharno=1;
int yylineno=1;

char escape_char = '\\';
static char comment_char = '#';
static int seenfirst=0;		/* Once LC_<category> seen, the
				 * escape_char and comment_char cmds are
				 * no longer permitted
				 */

static int  eol_pos=0;
static int peekc = 0;		/* One-character pushback (beyond stdio) */
static char locale_name[PATH_MAX+1];	/* for copy directive */
static int copyflag = 0;		/* for copy directive */

void initlex(void) {
    comment_char = '#';
    escape_char = '\\';

    yycharno = 1;
    yylineno = 1;
}

static int getspecialchar(int errcode) {
    int	c;

    /* Eat white-space */
    while (isspace(c=input()) && c != '\n');

    if (c != '\n') {
	if (c > 127)
	    diag_error(ERR_CHAR_NOT_PCS,c);
    } else
	diag_error(errcode);
 
   while (input() != '\n');

    return(c);
}

static void uninput(int);

int
input(void)
{
    int c;

    if (peekc != 0) {
	c = peekc;
	peekc = 0;
	return c;
    }
	
    while ((c=getchar()) != EOF) {

	/* if we see an escape char check if it's 
	 * for line continuation char 
	 */
	if (c==escape_char) {
	    yycharno++;
	    c = getchar();

	    if (c!='\n') {
		uninput(c);
		return escape_char;
	    } else {
		yylineno++;
		eol_pos = yycharno;
		yycharno = 1;
		continue;
	    }
	}

        /* eat comment */
        if (c==comment_char && yycharno == 1)
	    /*
	     * Comment character must be first on line or it's
	     * just another character.
	     */
	    while ((c=getchar()) != '\n') {	/* Don't count inside comment */
		if(c==EOF) return c;
	    }

	if (c=='\n') {
	    yylineno++;
	    eol_pos = yycharno;

	    /* 
	      Don't return '\n' if empty line 
	    */
	    if (yycharno==1)
		continue;
	    yycharno = 1;
	} else
	    yycharno++;

	return c;
    }
    
    return c;
}


void
uninput(int c)
{
    int  t;

    if (peekc != 0) {		/* already one char pushed back */
	ungetc(peekc, stdin);
	t = peekc;
	peekc = c;
	c = t;
    } else
	ungetc(c, stdin);

    if (c!='\n')
	yycharno--;
    else {
	yylineno--;
	yycharno = eol_pos;
    }
}


#ifdef DEBUG
#define RETURN(t) do {	\
    switch(t) {		\
      case INT_CONST:	fprintf(stderr,"INT %s\n",yytext);	break;	\
      case CHAR_CONST:	fprintf(stderr,"CHAR '%c'\n",value);	break;	\
      case SYMBOL:	fprintf(stderr,"SYMB %s\n",sym_text);	break;	\
      case STRING:	fprintf(stderr,"STR %s\n",yytext);	break;	\
      case EOF:		fprintf(stderr,"EOF\n"); break;			\
      default:	{							\
	  		if (isascii(t)) {				\
			    fprintf(stderr,"'%c'\n",t);			\
			} else {					\
			    fprintf(stderr,"KEYW %s\n", yytext);	\
			}						\
		}							\
    }									\
    return (t);								\
    } while (0)
    
#else
#define RETURN(t)	return(t)
#endif /* DEBUG */
			    
int
yylex(void)
{
    int yypos;
    int c;

    /*
     * If we have just processed a 'copy' keyword:
     *   locale_name[0] == '"'  - let the quoted string routine handle it
     *   locale_name[0] == '\0' - We hit and EOF while getting the locale
     *   else
     *    we have an unquoted locale name which we already read
     */
    if (copyflag && locale_name[0] != '"') {
	copyflag = 0;
	if (locale_name[0] == '\0')
	    RETURN(EOF);

	for (yypos = 0; locale_name[yypos] != '\0'; yypos++)
	    yytext[yypos] = locale_name[yypos];
	yytext[yypos] = '\0';

	RETURN(LOC_NAME);
    }

    if (skip_to_EOL) {
	/*
	 * Ignore all text til we reach a new line.
	 * (For example, in char maps, after the <encoding>
	 */
	skip_to_EOL = 0;

	while ((c=input()) != EOF) {
	    if (c == '\n')
		RETURN(c);
	}
	RETURN(EOF);
    }
		
    while ((c = input()) != EOF) {
	
	/* 
	        [0-9]+
	        | [-+][0-9]+ 
	*/
	if (isdigit(c) || c=='-' || c=='+') {
	    yytext[yypos=0] = c;
	    while (isdigit(c=input()))
		yytext[++yypos] = c;

	    yytext[++yypos] = '\0';

	    if ((c != EOF && !isspace(c)) || c == '\n')
		uninput(c);

	    if ( yypos >1) {
		/* multiple chars in string */
		RETURN(STRING);
	    } else {
		value = (unsigned char)yytext[0];
		RETURN(CHAR_CONST);
	    }
	}

	
	/* '\\'
	        \\d[0-9]{1,3}
		\\0[0-8]{1,3}
		\\[xX][0-9a-fA-F]{2}
	        \\[^xX0d]
	*/
	if (c==escape_char) {
	    char *junk;
	    char number[80];
	    char *s;

	    value = 0;
	    yypos = -1;

	    do {
		yytext[++yypos] = c;

		c = input();
		yytext[++yypos] = c;

		value <<= 8;

		switch (c) {
		  case 'd':  /* decimal constant */
		    for (c=input(),s=number; isdigit(c); c=input())
			*s++ = yytext[++yypos] = c;

		    *s++ = '\0';

		    if ((c != EOF && !isspace(c)) || c == '\n')
			uninput(c);

		    /* check number of digits in decimal constant */
		    if (((s - number) < 3) || ((s - number) > 4))
			diag_error(ERR_ILL_DEC_CONST,yytext);

		    value += strtol(number, &junk, 10);
		    continue;
		  
		  case 'x':  /* hex constant */
		  case 'X':
		    for (c=input(),s=number; isxdigit(c); c=input())
			*s++ = yytext[++yypos] = c;
		    *s++ = '\0';

		    if ((c != EOF && !isspace(c)) || c == '\n')
			uninput(c);

		    if ((s - number) > 3)
			/*  wrong number of digits in hex const */
			diag_error(ERR_ILL_HEX_CONST,yytext);

		    value += strtol(number, &junk, 16);
		    continue;

		  default:   
		    if (isdigit(c)) {		/* octal constant */	
		        for (s=number; c >= '0' && c <= '7' && c != EOF; 
			     c=input())
			    *s++ = yytext[++yypos] = c;
		        *s++ = '\0';
			if (c == '8' || c == '9'){
			    while (isdigit(c)){	/*invalid oct */
			       yytext[++yypos] = c;
			       c = input();
			    }
			    diag_error(ERR_ILL_OCT_CONST,yytext);
			}
		
		        if ((c != EOF && !isspace(c)) || c == '\n')
			    uninput(c);

			/* check number of digits in octal constant */
		        if (((s - number) < 3) || ((s - number) > 4))
			    /* too many digits in octal constant */
			    diag_error(ERR_ILL_OCT_CONST,yytext);

		        value += strtol(number, &junk, 8);
		        continue;
		    }
		    else {             /* escaped character, e.g. \\ */

			if (yypos == 1) {	/* Single quoted character */
			    value = (unsigned char) yytext[1];
			    RETURN(CHAR_CONST);
			}

			/* Looking at something like: \d12\d12\Q */
			uninput(c);
			uninput('\\');

			goto EndByteString;
		    }
		}
	    } while ( (c=input()) == '\\');

EndByteString:

	    yytext[++yypos] = '\0';

	    if ((c != EOF && !isspace(c)) || c == '\n')
		uninput(c);

	    RETURN(CHAR_CONST);
	}


	/* 
	      symbol for character names - or keyword:

	      < [:isgraph:]+ >
	*/
	if (c=='<') {
	    keyword_t *kw;
	    
	    yytext[yypos=0] = c;
	    do {
		c = input();
		if (c==escape_char)
		    yytext[++yypos] = input();
		else
		    yytext[++yypos] = c;
	    } while (c != '>' && isgraph(c));
	    yytext[++yypos] = '\0';
	    if (c != '>') {
		uninput(c);
	 	yytext[yypos-1] = '\0';
		diag_error(ERR_ILL_CHAR_SYM, yytext);
		yytext[yypos-1] = '>';
		yytext[yypos] = '\0';
	    }

	    /* look for one of the special 'meta-symbols' */
	    kw = bsearch(yytext, kw_tbl, 
			 KW_TBL_SZ, sizeof(keyword_t),
			 (int(*)(const void*,const void*))strcmp);
	    
	    if (kw != NULL) {
		/* check for escape character replacement. */
		
		if (kw->token_val == KW_ESC_CHAR) {
		    escape_char = getspecialchar(ERR_ESC_CHAR_MISSING);
		    continue;
		    
		} 
	        else
		    if (kw->token_val == KW_COMMENT_CHAR) {
			comment_char = getspecialchar(ERR_COM_CHAR_MISSING);
			continue;
		    }
		    else
		        RETURN(kw->token_val);
	    }
	    else {
		strncpy(sym_text, yytext, sizeof(sym_text));
		RETURN(SYMBOL);
	    }
	}
	
	
	/* 
	    symbol for character class names - or keyword.
	  
	    [:alpha:_]+[:digit::alpha:_-]
	*/
	if (isalpha(c) || c == '_') {
	    keyword_t *kw;
	    
	    yytext[yypos=0] = c;
	    while (isalnum((c=input())) || c=='_' || c == '-') 
		yytext[++yypos] = c;
	    yytext[++yypos] = '\0';

	    if ((c != EOF && !isspace(c)) || c == '\n')
		uninput(c);

	    kw = bsearch(yytext, kw_tbl, 
			 KW_TBL_SZ, sizeof(keyword_t),
			 (int(*)(const void*,const void*))strcmp);
	    
	    if (kw != NULL) {
		switch (kw->token_val) {
		  case KW_LC_COLLATE:
		  case KW_LC_CTYPE:
		  case KW_LC_MSG:
		  case KW_LC_MONETARY:
		  case KW_LC_NUMERIC:
		  case KW_LC_TIME:
		    seenfirst++;
		    RETURN(kw->token_val);

		  case KW_COPY:
		    {
			char *ptr, *endp;

			copyflag = 1;
			while (isspace(c=input()) && c != '\n');

			if (c == EOF) {
			    locale_name[0] = '\0';	/* indicate EOF for later */
			    RETURN(KW_COPY);
			}

			if (c == '"' || c == '\n') {
			    locale_name[0] = c;	/* indicate quotes for later */
			    uninput(c);
			    RETURN(KW_COPY);
			}
		    				/* save locale name for later */
			ptr = locale_name;
			endp = &locale_name[PATH_MAX];
			do {
			    *ptr++ = c;
			    if (ptr > endp)
				error(NAME_TOO_LONG, PATH_MAX);
			} while (((c=input()) != EOF) && !isspace(c));
			*ptr++ = '\0';
			uninput(c);
			RETURN(KW_COPY);
		    }

		  default:
		    RETURN(kw->token_val);
		} /* switch */

	    } else if ( !seenfirst && !strcmp(yytext,"comment_char") && yycharno == 14) {
		comment_char = getspecialchar(ERR_COM_CHAR_MISSING);
		continue;
		       
	    } else if ( !seenfirst && !strcmp(yytext,"escape_char") && yycharno == 13) {
		escape_char = getspecialchar(ERR_ESC_CHAR_MISSING);
		continue;
	    } else {
		strncpy(sym_text, yytext, sizeof(sym_text));
		RETURN(SYMBOL);
	    }
	}
	
	
	/* 
	    strings! replacement of escaped constants and <char> references
	    are handled in copy_string() in sem_chr.c
	    
	    Replace all escape_char with \ so that strings will have a
	    common escape_char for locale and processing in other locations.
	  
	    "[^\n"]*"
        */
	if (c == '"') {
	    
	    yypos = 0;
	    while ((c=input()) != '"' && c != '\n' && c != EOF) {
		if (c == escape_char){
		    yytext[yypos++] = escape_char;
		    if ((c = input()) != EOF)
		        yytext[yypos++] = c;
		    else {
			yytext[yypos] = '\0';
			break;
		    }
		}
		else
		    yytext[yypos++] = c;
	    }
	    yytext[yypos] = '\0';

	    if (c=='\n' || c==EOF)
		diag_error(ERR_MISSING_QUOTE,yytext);
	    
	    if (copyflag) {
		copyflag = 0;
		RETURN(LOC_NAME);
	    }

	    RETURN(STRING);
	}

	if (c=='.') {
	    yytext[yypos=0] = c;
	    yytext[++yypos] = c = input();

	    if (c != '.') {
		uninput(c);
		value = '.';
		RETURN(CHAR_CONST);
	    }

	    yytext[++yypos] = c = input();
	    if (c == '.')
		RETURN(KW_ELLIPSES);
	    else {
		yytext[++yypos] = '\0';
		diag_error(ERR_UNKNOWN_KWD, yytext);
		continue;
	    }
	}

	/* 
	  The newline is this grammer statement terminator - yuk!
	*/
	if (c=='\n')
	    RETURN(c);

	if (isspace(c))
	    continue;

	if (c==';' || c=='(' || c==')' || c==',') {
	    RETURN(c);
	}

	if (isascii(c) && isprint(c)) {
	    value = c;
	    RETURN(CHAR_CONST);
	}
	
	diag_error(ERR_ILL_CHAR, c);

    } /* while c != EOF */

    RETURN(EOF);
}

