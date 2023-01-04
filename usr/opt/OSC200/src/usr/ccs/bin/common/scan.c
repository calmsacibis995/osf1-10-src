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
static char	*sccsid = "@(#)$RCSfile: scan.c,v $ $Revision: 4.2.6.7 $ (DEC) $Date: 1993/11/22 21:22:47 $";
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

/*
 * COMPONENT_NAME: (CMDPROG) scan.c
 *
 * FUNCTIONS: asmout, gchar, getmore, lxcom, lxenter, lxget, lxinit, lxmore  
 *            lxres, lxstr, lxtitle, mainp1, ungchar, yylex                   
 *
 * ORIGINS: 27 03 09 32 00 
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 * OBJECT CODE ONLY SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Changes for ANSI C were developed by HCR Corporation for IBM
 * Corporation under terms of a work made for hire contract.
 */

/* AIWS C compiler */

# include "mfile1.h"
# include "messages.h"
# include <ctype.h>

#undef isdigit
#undef isupper
#undef islower

	/* temporarily */
#ifdef LINT
extern char *bycode();
#endif

# ifndef ASMBUF
# define ASMBUF 50
# endif
char asmbuf[ASMBUF];
char *asmp;
int asm_esc = 0; /* asm escaped used in file */
	/* lexical actions */

# define A_ERR 0		/* illegal character */
# define A_LET 1		/* saw a letter */
# define A_DIG 2		/* saw a digit */
# define A_1C 3			/* return a single character */
# define A_STR 4		/* string */
# define A_CC 5			/* character constant */
# define A_BCD 6		/* GCOS BCD constant */
# define A_SL 7			/* saw a / */
# define A_DOT 8		/* saw a . */
# define A_PL 9			/* + */
# define A_MI 10		/* - */
# define A_EQ 11		/* = */
# define A_NOT 12		/* ! */
# define A_LT 13		/* < */
# define A_GT 14		/* > */
# define A_AND 16		/* & */
# define A_OR 17		/* | */
# define A_WS 18		/* whitespace (not \n) */
# define A_NL 19		/* \n */
# define A_MUL 20		/* * */
# define A_MOD 21		/* % */
# define A_ER 22		/* ^ */

	/* character classes */

# define LEXLET 01
# define LEXDIG 02
# define LEXOCT 04
# define LEXHEX 010
# define LEXWS 020
# define LEXDOT 040

	/* reserved word actions */

# define AR_TY 0		/* type specifier word */
# define AR_RW 1		/* simple reserved word */
# define AR_CL 2		/* storage class word */
# define AR_S 3			/* struct */
# define AR_U 4			/* union */
# define AR_E 5			/* enum */
# define AR_A 6			/* asm */
# define AR_QU 7		/* type qualifier word */
# define AR_GO 8		/* goto */

#define INBUFSZ 8192		/* Input buffer size */
	/* text buffer */
# define LXTSZ 8192
char yytext[LXTSZ];
char * lxgcp;

/*****************************************************************************
* I added an option to pass source lines through to the intermediate and     *
* assembler files for debugging.  To implement that, all of the calls to     *
* getchar() and ungetc() are changed to gchar() and ungchar().  Gchar        *
* reads characters from a line buffer and calls getmore() to refill it.      *
*
* Handling of builtins:
*	Builtins are handled in 2 ways:
*	1. As function prototypes. The SBUILTIN flag is or'd into the
*	   symtab. This flag is tested to prevent the entry from being
*	   generated to the output.
*	2. As a special case in cgram.y.
*****************************************************************************/

int doing_builtins = 0;	/* Builtins are generated at initialization time */
int cdebug;             /* pass through source code */
int ntrnodes = TREESZ;  /* number of tree nodes */
static char sbuf[INBUFSZ];   	/* source buffer */
static char *scp = sbuf;   /* current character in sbuf[] */


static char getmore();
static unsigned int dbg_from_lno; /* Line number fromwhich to turn on all 
				     debugging*/
static unsigned int dbg_to_lno; /* Line number to which to turn off dbg */
# define gchar() 	(*scp? *scp++: getmore())
# define ungchar(ch,foo) (ch >= 0? *--scp = ch: ch)

/* -------------------- mainp1 -------------------- */

	/* ARGSUSED */
mainp1( argc, argv ) int argc; char *argv[]; {  /* control multiple files */

	register i;
	register char *cp;
	extern int idebug, bdebug, tdebug, edebug, fdebug, 
		ddebug, xdebug, odebug, gdebug;
	extern int yydebug; /* Defined by cgram.y */
	extern int NoRegisters;
#ifdef LINT
	register merr;
	extern int max_errors, LintErrRC; /*In error.h*/
#endif
#ifdef __alpha
	char *release = "Digital OSF/1 AXP";
#else
	char *release = "IBM AIX/RT Version 2.2.1 Enhanced";
#endif
	/* HACK HACK malloc so fits on PC */
	dimtab = (int *)getmem(ndiments * sizeof(int));
	node = (NODE *)getmem(ntrnodes * sizeof(NODE));
#ifdef XCOFF
	saved_lab = (int *)getmem(max_strings * sizeof(int));
	saved_str = (int *)getmem(max_chars * sizeof(int));
#endif

	offsz = caloff();
	for( i=1; i<argc; ++i ){
		if( *(cp=argv[i]) == '-' && *++cp == 'X' ){
		  /* bypass "-Xdollar" otherwise look at the -X options */
		  if (strcmp(cp, LINT_XDOLLAR))
			while( *++cp ){
				switch( *cp ){

				case 'A': /* Upper case A== ALL */
				  ++ddebug;
				  ++idebug; 
				  ++bdebug;
				  ++tdebug;
				  ++edebug;
				  ++fdebug;
				  ++xdebug;
				  ++NoRegisters;
				  ++cdebug;
				  ++odebug;
				  ++gdebug;
				  ++yydebug;
				  /* Fall through to display version */    
				case 'r':
					fprintf( stderr, TOOLSTR(M_MSG_284, "Version: %s\n"),
						release );
					break;
#ifdef LINT
				case 'E':
				  if ((merr = atoi(++cp)) > 0)
				    max_errors = merr;
				  LintErrRC = 0; /* Suppress error return code */
				  break;
#endif
				case 'F':
				  dbg_from_lno = atoi(++cp);
				  cp += strlen(cp) - 1; /* Adjust to terminate */
				  break;

				case 'T':
				  dbg_to_lno = atoi(++cp);
				  cp += strlen(cp) - 1; /* Adjust to terminate */
				  break;
				case 'd':
					++ddebug;
					break;
				case 'i':
					++idebug;
					break;
				case 'b':
					++bdebug;
					break;
				case 't':
					++tdebug;
					break;
				case 'e':
					++edebug;
					break;
				case 'f':
					++fdebug;
					break;
				case 'x':
					++xdebug;
					break;
				case 'R':
					++NoRegisters;
					break;
				case 'c':
					++cdebug;
					break;
				case 'o':
					++odebug;
					break;
				case 'y':
				        ++yydebug;
				        break;
					}
				}
			}
		}

# ifdef ONEPASS
	p2init( argc, argv );
# endif

	InitStab();
	InitTypes();
	InitParse();
	lxinit();
	tinit();
	mkdope();

	lineno = 1;

	/* initialization of paramstk to allow it to grow */
	/* as needed.                                     */
	paramstk = (int *)getmem(paramsz*sizeof(int));

	/* initialization of protostk to allow it to grow */
	/* as needed.                                     */
	protostk = (int *)getmem(protosz*sizeof(int));

	/* dimension table initialization */

	dimtab[TNULL] = 0;
	dimtab[UNDEF] = 0;
	dimtab[TVOID] = 0;
	dimtab[CHAR] = SZCHAR;
	dimtab[INT] = SZINT;
	dimtab[SHORT] = SZSHORT;
	dimtab[LONG] = SZLONG;
	dimtab[LLONG] = SZLLONG;
	dimtab[SCHAR] = SZCHAR;
	dimtab[FLOAT] = SZFLOAT;
	dimtab[DOUBLE] = SZDOUBLE;
	dimtab[LDOUBLE] = SZLDOUBLE;
	dimtab[UCHAR] = SZCHAR;
	dimtab[USHORT] = SZSHORT;
	dimtab[UNSIGNED] = SZINT;
	dimtab[ULONG] = SZLONG;
	dimtab[ULLONG] = SZLLONG;

	/*
	** Array dimtab continues after the above basic types.
	** Variable curdim is set at one more than the value of last
	** basic type defined in m_ind/manifest.h
	*/
	curdim = NBTYPES;
	reached = 1;
	lintnrch = 0;

#ifdef	LINT
	OutFileBeg(LINTBOF);
#endif
#ifdef	CFLOW
	OutFileBeg(CFLOWBOF);
#endif
	yyparse();
	yyaccpt();

	/* Clear out leftover scoped-out externals */
	clearst();

	deftents();

#ifdef XCOFF
	prFTN();
	unbuffer_str();
#if	!defined (CXREF) && !defined (LINT) && !defined (CFLOW)
	printf("\t.extern\t._ptrgl[pr]\n");
#endif
#endif
#ifdef DBG_HASH
	PrintHash();
#endif
	ejobcode( nerrors ? 1 : 0 );
#ifdef	LINT
	OutFileEnd(LINTEOF);
#endif
#ifdef	CFLOW
	OutFileEnd(CFLOWEOF);
#endif
	return(nerrors?1:0);

	}

# define CSMASK 0177
# define CSSZ 128


short lxmask[CSSZ+1];

/* -------------------- lxenter -------------------- */

lxenter( s, m ) register char *s; register short m; {
	/* enter a mask into lxmask */
	register c;
	while( c= *s++ ) lxmask[c+1] |= m;
	}

# define lxget(c,m) (lxgcp=yytext,lxmore(c,m))

/* -------------------- lxmore -------------------- */

lxmore( c, m )  register c, m; {
	register char *cp;

	*(cp = lxgcp) = c;
	while( c=gchar(), lxmask[c+1]&m ){
		if( cp < &yytext[LXTSZ-1] ){
			*++cp = c;
			}
		}
	ungchar(c,stdin);
	*(lxgcp = cp+1) = '\0';
	}

/*
 *	This array controls the scanning of tokens. The first 4 slots MUST be
 *	maintained:
 *	lxdope[0]:	A_ERR - Default error slot for illegal characters.
 *	lxdope[1]:	A_LET - All characters and  '-' use this.
 *	lxdope[2]:	A_DIG - Digits.
 *	lxdope[3]:	A_WS  - White space characters.
 *	There is no implied ordering for the rest of the table.
 */
struct lxdope {
	short lxch;	/* the character */
	short lxact;	/* the action to be performed */
	short lxtok;	/* the token number to be returned */
	short lxval;	/* the value to be returned */
	} lxdope[] = {

	'$',	A_ERR,	0,	0,      /* Default errors */
	'_',	A_LET,	0,	0,	/* letters point here */
	'0',	A_DIG,	0,	0,	/* digits point here */
	' ',	A_WS,	0,	0,	/* whitespace goes here */
	'\n',	A_NL,	0,	0,
	'"',	A_STR,	0,	0,	/* character string */
	'\'',	A_CC,	0,	0,	/* character constant */
	'`',	A_BCD,	0,	0,	/* GCOS BCD constant */
	'(',	A_1C,	LP,	0,
	')',	A_1C,	RP,	0,
	'{',	A_1C,	LC,	0,
	'}',	A_1C,	RC,	0,
	'[',	A_1C,	LB,	0,
	']',	A_1C,	RB,	0,
	'*',	A_MUL,	MUL,	MUL,
	'?',	A_1C,	QUEST,	0,
	':',	A_1C,	COLON,	0,
	'+',	A_PL,	PLUS,	PLUS,
	'-',	A_MI,	MINUS,	MINUS,
	'/',	A_SL,	DIVOP,	DIV,
	'%',	A_MOD,	DIVOP,	MOD,
	'&',	A_AND,	AND,	AND,
	'|',	A_OR,	OR,	OR,
	'^',	A_ER,	ER,	ER,
	'!',	A_NOT,	UNOP,	NOT,
	'~',	A_1C,	UNOP,	COMPL,
	',',	A_1C,	CM,	CM,
	';',	A_1C,	SM,	0,
	'.',	A_DOT,	STROP,	DOT,
	'<',	A_LT,	RELOP,	LT,
	'>',	A_GT,	RELOP,	GT,
	'=',	A_EQ,	ASSIGN,	ASSIGN,
	-1,	A_1C,	0,	0,
	};

struct lxdope *lxcp[CSSZ+1];

/* -------------------- lxinit -------------------- */

lxinit(){
	register struct lxdope *p;
	register i;
	register char *cp;
	/* set up character classes */
#ifdef LINT
	/*
	 **** Fii:
	** If -Xdollar is specified then include "$" as a letter.
         */     
	if (lintXdlr)
	  lxenter( "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_$", LEXLET );
	else
#endif
	  lxenter( "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_", LEXLET );
	lxenter( "0123456789", LEXDIG );
	lxenter( "0123456789abcdefABCDEF", LEXHEX );
		/* \013 should become \v someday; \013 is OK for ASCII and EBCDIC */
	lxenter( " \t\r\b\f\013", LEXWS );
	lxenter( "01234567", LEXOCT );
	lxmask['.'+1] |= LEXDOT;

	/* make lxcp point to appropriate lxdope entry for each character */

	/* initialize error entries */

	for( i= 0; i<=CSSZ; ++i ) lxcp[i] = lxdope;

	/* make unique entries */

	for( p=lxdope; ; ++p ) {
		lxcp[p->lxch+1] = p;
		if( p->lxch < 0 ) break;
		}

	/* handle letters, digits, and whitespace */
	/* by convention, first, second, and third places */

	cp = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
	while( *cp ) lxcp[*cp++ + 1] = &lxdope[1];
/* gaf: Allow the '$' character in identifiers (DEC internal only) */
#if defined LINT
	if (lintXdlr)
	  lxcp['$' + 1] = &lxdope[1];
#endif
	cp = "123456789";
	while( *cp ) lxcp[*cp++ + 1] = &lxdope[2];
	cp = "\t\b\r\f\013";
	while( *cp ) lxcp[*cp++ + 1] = &lxdope[3];
	/* first line might have title */
	lxtitle();

	}

int lxmatch;  /* character to be matched in char or string constant */

/* -------------------- lxstr -------------------- */

lxstr(ct){
	/* match a string or character constant, up to lxmatch */

	register c;
	char lxchar, lxchar1;
	int wideerrs;
	register CONSZ val;
	register i;

	wideerrs = 0;
	i=0;

morestr:
	while( (c=gchar()) != lxmatch ){
		switch( c ) {

		case EOF:
#ifdef LINT
		  if (AL_MIGCHK) AL_PRINTIT = 1;
#endif
			/* "unexpected EOF" */
			UERROR( ALWAYS, MESSAGE(113) );
			break;

		case '\n':
#ifdef LINT
		  if (AL_MIGCHK) AL_PRINTIT = 1;
#endif
			/* "newline in string or char constant" */
			UERROR( ALWAYS, MESSAGE(78) );
			++lineno;
			lxtitle();
			break;

		case '\\':
			switch( c = gchar() ){
			case '\n':
				/*
				 * line splicing is done by the 'cpp'
				 * in ANSI-C. thus, we flag it as an error.
				 */
				WERROR( ALWAYS, MESSAGE(78) );
				++lineno;
				continue;
			default:
				/* "\%c unknown escape sequence" */
				WERROR( devdebug[ANSI_PARSE], MESSAGE(129), c );
				val = c;
				goto mkcc;
			case 'a':
				val = '\007'; /* ascii bell */
				goto mkcc;

			case '\'':
				val = '\''; /* an apostrophe */
				goto mkcc;

			case '"':
				val = '"'; /* a quote */
				goto mkcc;

			case '?':
				val = '?'; /* a question mark */
				goto mkcc;

			case '\\':
				val = '\\'; /* a backslash */
				goto mkcc;

			case 'x':
			case 'X':
				/* start collecting a hexadecimal constant */
				val = 0;
				{
					register char *cp, lxchar;
					lxget( lxchar, LEXHEX);
					for( cp = yytext+1; *cp; ++cp ){
						/* this code won't work for all wild character sets,
						   but seems ok for ascii and ebcdic */
						val <<= 4;
						if( isdigit( *cp ) )
							val += *cp-'0';
						else if( isupper( *cp ) )
							val += *cp - 'A'+ 10;
						else 	val += *cp - 'a'+ 10;
						}
				}

				/* we might want to make locale
				 * specific check if that's needed.
				 */
				/* we check is the value is representable
				 * in an unsigned char.
				 */
				if ((val<0) || (val>UCHAR_MAX))
					/* "constant value (0x%x) exceeds (0x%x)" */
					WARNING( WSTORAGE, MESSAGE(139),
						val, UCHAR_MAX );

				goto mkcc;

			case 'n':
				val = '\n';
				goto mkcc;

			case 'r':
				val = '\r';
				goto mkcc;

			case 'b':
				val = '\b';
				goto mkcc;

			case 't':
				val = '\t';
				goto mkcc;

			case 'f':
				val = '\f';
				goto mkcc;

			case 'v':
				val = '\013';
				goto mkcc;

			case '0':
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
				val = c-'0';
				c=gchar();  /* try for 2 */
				if( lxmask[c+1] & LEXOCT ){
					val = (val<<3) | (c-'0');
					c = gchar();  /* try for 3 */
					if( lxmask[c+1] & LEXOCT ){
						val = (val<<3) | (c-'0');
						}
					else ungchar( c ,stdin);
					}
				else ungchar( c ,stdin);

				/* we might want to make locale
				 * specific check if that's needed.
				 */

				/* we check is the value is representable
				 * in an unsigned char.
				 */
				if ((val<0) || (val>UCHAR_MAX))
					/* "constant value (0x%x) exceeds (0x%x)" */
					WARNING( WSTORAGE, MESSAGE(139),
						val, UCHAR_MAX );
				goto mkcc1;

				}
		default:
			val =c;
		mkcc:
			val = CCTRANS(val);
		mkcc1:
			if( lxmatch == '\'' ){
				val = CHARCAST(val);  /* it is, after all, a "character" constant */
				makecc( val, i );
				}
			else { /* stash the byte into the string */
				if( strflg ) {
					if( ct==0 || i<ct ) putbyte( val );
					else if( i == ct )
						/* "non-null byte ignored in string initializer" */
						WARNING( ALWAYS, MESSAGE(81) );
					}
#ifdef XCOFF
				else savestr( (int) val );
#else
				else bycode( (int)val, i );
#endif
				}
			++i;
			continue;
			}
		break;
		}
	/* end of string or  char constant */

	/* skip white space and white space characters */
	for (;;){
		switch ( (lxcp[(lxchar=gchar())+1])->lxact ) {
		case A_SL:
			if ((lxchar = gchar()) == '*' )
				lxcom();
			/* C++ comment non-ansi only*/
			else if (lxchar == '/' && !devdebug[ANSI_MODE] ) { 
			  while((lxchar = gchar()) != '\n' && lxchar != EOF);
			  ungchar(lxchar, stdin);
			  goto done;
			} else {
 				ungchar((int)lxchar, stdin);
				ungchar('/', stdin);
				goto done;
			}
			continue;
		case A_STR:
			/* if yylval.intval == 1 and wideerrs == 0
			   then me have a "string" mixed with an initial
			   L"string". And if we did not warn about that,
			   do so.
			*/
			if (yylval.intval && !wideerrs) {
				/* "'string literals' mixed with 'wide string literals'" */
				WERROR( ALWAYS, MESSAGE(130) );
				wideerrs++;
			}
			goto morestr;
		case A_NL:
			++lineno;
			lxtitle();
			continue;
		case A_WS:
			continue;
		case A_LET:
			if (lxchar == 'L')
				if ( (lxchar1 = gchar()) == '"' ) {
				/* if yylval.intval == 0 and wideerrs == 0
				   then me have a "string" mixed with an initial
				   L"string". And if we did not warn about that,
				   do so.
				*/
					if (!yylval.intval && !wideerrs) {
						/* "'string literals' mixed with 'wide string literals'" */
						WERROR( ALWAYS, MESSAGE(130) );
						wideerrs++;
					}
					goto morestr;
				}
				else
					ungchar((int)lxchar1, stdin);
			/* fall thru */
		default  :
			ungchar((int)lxchar, stdin);
			goto done;
		}
	}


done:

	 if( lxmatch == '"' ){
		/* "array not large enough to store terminating null" */
		if (ct != 0 && i == ct)
			WARNING( WSTORAGE, MESSAGE(175) );

		if( strflg ){ /* end the string */
			if( ct==0 || i<ct ) putbyte( 0 );  /* the null at the end */
			}
		else {  /* the initializer gets a null byte */
#ifdef XCOFF
			savestr( -1 );
			strsize = ++i;  /* for type in buildtree */
#else
			bycode( 0, i++ );
#ifdef LINT
			saved_str_ptr = bycode( -1, i );
#else
			bycode( -1, i );
#endif
			strsize = i;  /* for type in buildtree */
#endif
			}
		}
	else { /* end the character constant */
		/* "empty character constant" */
		if( i == 0 ) UERROR( ALWAYS, MESSAGE(36) );

		/* in a different locale, we might test:
		 *     if (i>(sizeof(wchar_t)/sizeof(char)))
		 */
		if( i>(SZINT/SZCHAR) || ( WPORTABLE&&i>1 ) )
			/* "too many characters in character constant" */
			WERROR( ALWAYS, MESSAGE(107) );
		}
	}

/* -------------------- lxcom -------------------- */

lxcom(){
	register c;
	/* saw a "/_*": process a comment */

#ifdef LINT
	switch ( c = gchar() ) {
		case 'V':
			lxget( c, LEXLET|LEXDIG );
			c = yytext[7]?yytext[7]-'0':0;
			yytext[7] = '\0';
			if( strcmp( yytext, "VARARGS" ) ) break;
			lintvarg = c;
			break;

		case 'L':
			lxget( c, LEXLET );
			if( !strcmp( yytext, "LINTLIBRARY" ) ){
				lintlib = 1;
			}
			else if( !strcmp( yytext, "LINTSTDLIB" ) ){
				lintrsvd = 1;
				lintused = 1;
				lintlib = 1;
			}
			break;

		case 'A':
			lxget( c, LEXLET );
			if( strcmp( yytext, "ARGSUSED" ) ) break;
			lintargu = 1;
			break;

		case 'N':
			lxget( c, LEXLET );
			if( !strcmp( yytext, "NOTREACHED" ) )
				lintnrch = 1;
			else if( !strcmp( yytext, "NOTUSED" ) ){
				lintused = 1;
				lintlib = 1;
			}
			else if( !strcmp( yytext, "NOTDEFINED" ) )
				lintdefd = 1;
			break;
		default:
			ungchar(c, stdin);
			break;
	}
#endif
	for(;;){

		switch( c = gchar() ){

		case EOF:
			/* "unexpected EOF"  */
			UERROR( ALWAYS, MESSAGE(113) );
			return;

		case '\n':
			++lineno;
			continue;
		case '/':
			if( (c = gchar()) == '*' )
				/* "nested comments not supported" */
				WARNING( ALWAYS, MESSAGE(127) );
				/* ignore text until we reach the first
				   '*' followed immediately by a '/'.
				 */
			else ungchar(c, stdin);
			continue;
		default:
			continue;

		case '*':
			if( (c = gchar()) == '/' ) return;
			else ungchar( c ,stdin);
			continue;

			}
		}
	}

/* -------------------- yylex -------------------- */

yylex(){

	for(;;){

		int inttype, intmode;
		int float_col = 0;
		static int have_type = 0;

		register lxchar;
		register struct lxdope *p;
		register struct symtab *sp;
		int id;
		int chrcnt=0;  /* char. counter for ambiguous unary ops */
		char tempchar[80]; /* temp chars for ambig. unary ops */

		lxchar=gchar();
		if (lxchar >= CSSZ || lxchar < -1) {
		  p = lxdope;
		} else {
		  p=lxcp[lxchar + 1];
		}
		switch( p->lxact ) {

		case A_MUL:
			/* * *= are valid, '*' '/' is a warning */
			switch (lxchar = gchar()) {
			case '=':
				return(yylval.intval = ASG_MUL);
			case '/':
				/* "*_/ found outside of a comment context" */
				WERROR( ALWAYS, MESSAGE(128) );
			default :
				goto onechar;
			}

		case A_MOD:
			/* % %= are valid */
			if ( (lxchar = gchar()) == '=')
				return(yylval.intval = ASG_MOD);
			goto onechar;

		case A_ER :
			/* ^ ^= are valid */
			if ( (lxchar = gchar()) == '=')
				return(yylval.intval = ASG_ER);
			/* goto onechar; */

		onechar:
			ungchar( lxchar ,stdin);

		case A_1C:
			/* eat up a single character, and return an opcode */

			if (yydebug)
			  if (p->lxch == EOF)
				printf("LEX:EOF\n");
			  else
				printf("LEX:%c\n", p->lxch);
			yylval.intval = p->lxval;
			if (p->lxtok >= LP && p->lxtok <= SM) {
			  have_type = 0;
			}
			return( p->lxtok );

		case A_ERR:
#ifdef LINT
			AL_PRINTIT=1;	/* Turn on migration printing */
#endif
			/* "illegal character: %03o (octal)" */
			UERROR( ALWAYS, MESSAGE(51), lxchar );
			break;
			
		case A_LET:
			/* collect an identifier, check for reserved word, and return */
			lxget( lxchar, LEXLET|LEXDIG );
			/* check if the collected string is "L" and that the
			   next char is a "'" or a '"'.
			*/

			if (yydebug) 
			  printf("LEX:%s\n", yytext);
			if (yytext[0] == 'L' && yytext[1] == '\0')
					switch (lxchar = gchar()) {
					case '\'': /* char constant */
						lxmatch = '\'';
						lastcon = 0;
						lxstr(0);
						yylval.intval =
							devdebug[TYPING] ?
							CHAR : UCHAR;
						return( ICON );
					case '\"': /* string constant */
						yylval.intval = 1; /* fake wide strings */
						lxmatch = '"';
						return( STRING );
					default: /* neither */
						ungchar(lxchar, stdin);
					}

			/* check "keywords" : lxres is a misnomer */
			if( (lxchar=lxres()) > 0 )
				return( lxchar ); /* reserved word */

			if( lxchar== 0 ) continue;
			id = lookup( yytext,
				/* tag name for struct/union/enum */
				(stwart&TAGNAME) ? STAG :
				/* member name for struct/union */
				(stwart&(INSTRUCT|INUNION|FUNNYNAME)) ? SMOS :
				/* label name */
				(stwart&LABNAME) ? SLABEL :
				/* regular identifier */
				0 );
			sp = &stab[id];
			if (doing_builtins) {
			  sp->sflags |= SBUILTIN;
			}
			/* If a typedef, and we have not yet seen it in context */
			if( sp->sclass == TYPEDEF && !SeenType() && !have_type && !stwart){
#ifdef	CXREF
				CXRefName(id, lineno);
#endif
				stwart = instruct;
				yylval.nodep = mkty( sp->stype );
				sp->suse = -lineno;
				have_type = 1;
				return( TYDEF );
				}
			have_type = 0;
			stwart = (stwart&SEENAME) ? instruct : 0;
			yylval.intval = id;
			return( NAME );

		case A_DIG:
			/* collect a digit string, then look at last one... */
			{
			CONSZ maxint;

			/* float_col is needed to disambiguate the string
			   123l to mean long int rather than long double.
			   lxget (directly below) reads the '123' and then
			   the switch reads the 'l or L' and goes directly
			   into the middle of the floating point collection
			   code. So we to test whether we're looking
			   at the 'l or L' legally => if float_col == 1.
			   float_col == 1 only after reading an 'e or E or .'
			   BRAIN DEATH...sigh
			*/
			float_col = 0;

			lastcon = 0;
			lxget( lxchar, LEXDIG );
			switch( lxchar=gchar() ){

			case 'x':
			case 'X':
				lxmore( 'x', LEXHEX );
				if( yytext[0] != '0' || yytext[1] != 'x' || strlen(yytext) == 2 )
					/* "illegal hex constant"  */
					UERROR( ALWAYS, MESSAGE(59) );
				/* convert the value */
				{
					register char *cp;
					for( cp = yytext+2; *cp; ++cp ){
						/* this code won't work for all wild character sets,
						   but seems ok for ascii and ebcdic */
						lastcon <<= 4;
						if( isdigit( *cp ) ) lastcon += *cp-'0';
						else if( isupper( *cp ) ) lastcon += *cp - 'A'+ 10;
						else lastcon += *cp - 'a'+ 10;
						}
					}

				/* 0 =	hex or octal so that I can get the
					type promotion right for ansi-c
				*/
				intmode = 0;
				goto islong;

			case '.':
				lxmore( lxchar, LEXDIG );
			getfp:
				float_col = 1; /* collecting a float */
				if( (lxchar=gchar()) == 'e' || lxchar == 'E' ){ /* exponent */

			case 'e':
			case 'E':
					float_col = 1; /* collecting a float */
					if( (lxchar=gchar()) == '+' || lxchar == '-' ){
						*lxgcp++ = 'e';
						*lxgcp++ = lxchar;
						}
					else {
						ungchar(lxchar,stdin);
						*lxgcp++ = 'e';
						}
					if (!isdigit(lxchar = gchar())) {
						/* bad floating number */
						UERROR( ALWAYS, MESSAGE(189) );
						
					}
					lxmore( lxchar, LEXDIG );
					/* now have the whole thing... */
					}
				else {  /* no exponent */
					ungchar( lxchar ,stdin);
					}
				if( (lxchar=gchar()) == 'f' || lxchar == 'F' ){
			case 'f':
			case 'F':
					/* is a real float? if not
					   it can only be octal or decimal.
					 */
					if (!float_col)
						goto octdecimal;

					yylval.intval = FLOAT;
					return( isitfloat( yytext ) );
				}
				else if (lxchar == 'l' || lxchar == 'L') {
			case 'l':
			case 'L':
					/* is a real float? if not
					   it can only be octal or decimal.
					 */
					if (!float_col)
						goto octdecimal;
					yylval.intval = LDOUBLE;
					return( isitfloat( yytext ) );
				}
				else {
					ungchar( lxchar ,stdin);
					}

				yylval.intval = DOUBLE;
				return( isitfloat( yytext ) );
			default:
			octdecimal:
				ungchar( lxchar, stdin);
				if( yytext[0] == '0' ){
					/* convert in octal */
					register char *cp;
					for( cp = yytext+1; *cp; ++cp ){
						if( *cp > '7' )
							/* bad octal digit */
							UERROR( ALWAYS,
							 MESSAGE(124), *cp );
						lastcon <<= 3;
						lastcon += *cp - '0';
						}
					/* 0 = octal or hex mode */
					intmode = 1;
					goto islong;
					}
				else {
					/* convert in decimal */
					register char *cp;
					for( cp = yytext; *cp; ++cp ){ /*don't let lastcon overflow needlessly */
						lastcon *= (CONSZ)10;
						lastcon += (CONSZ)(*cp - '0');
						}
					}

				/* 1 = decimal integer constant */
				intmode = 1;
	islong:
	{
		unsigned long nlastcon;

		nlastcon = (unsigned long) lastcon;

#if 0
		if (lastcon < 0 )
			printf("--> negative lastcon \n");
#endif

		lxchar = gchar();
		switch (lxchar) {
		case 'l':
		case 'L':
			/* constant is LONG */
			lxchar = gchar();
			switch (lxchar) {
			case 'u':
			case 'U':
				/* constant is ULONG */
				yylval.intval = ULONG;
				return(ICON);
			default	:
				/* we think constant is only a LONG */
				ungchar(lxchar, stdin);

				if( nlastcon > LONG_MAX 
#ifndef __alpha || mips64
   						&& devdebug[PROMOTION] 
#endif
				   )
					yylval.intval = ULONG;
				else
					yylval.intval = LONG;
				return(ICON);
			}
			/*NOTREACHED*/
			break;
		case 'u':
		case 'U':
			/* constant is UNSIGNED */
			lxchar = gchar();
			switch (lxchar) {
			case 'l':
			case 'L':
				/* constant is ULONG */
				yylval.intval = ULONG;
				return(ICON);
			default :
				/* we think constant is only UNSIGNED */
				ungchar(lxchar, stdin);

				if (nlastcon > INT_MAX)
					yylval.intval = ULONG;
				else {
#if (SZINT != SZLONG)
#ifndef __alpha || mips64
					lastcon = (unsigned) lastcon;
					yylval.intval = UNSIGNED;
#else
					lastcon = (unsigned long) lastcon;
					yylval.intval = ULONG;
#endif
#endif

				}
				return(ICON);
			}
			/*NOTREACHED*/
			break;
		default :
			/* constant is unsuffixed normal promotion
			   rules apply. We assume INT to be out basic type.
			 */
			ungchar(lxchar, stdin);

			switch (intmode) {
			case 0:	/* hex or octal integers */
#ifndef __alpha || mips64
				if( !devdebug[PROMOTION] ){
					yylval.intval = INT;
					return( ICON );
				}
#endif
				if (nlastcon > INT_MAX)
					if (nlastcon > UINT_MAX)
						if (nlastcon > LONG_MAX)
							yylval.intval = ULONG;
						else	yylval.intval = LONG;
					else {
#if (SZINT != SZLONG )
						lastcon = (unsigned) lastcon;
#endif
						yylval.intval = UNSIGNED;
					}
				else {
					/* our base assumption */
#if (SZINT != SZLONG )
					lastcon = (int) lastcon;
#endif
					yylval.intval = INT;
				}
				return(ICON);
			case 1: /* decimal integers */
#ifndef __alpha || mips64
				if( !devdebug[PROMOTION] ){
					yylval.intval = INT;
					return( ICON );
				}
#endif
				if (nlastcon > INT_MAX)
					if (nlastcon > LONG_MAX)
						yylval.intval = ULONG;
					else	yylval.intval = LONG;
				else {
					/* our base assumption */
#if (SZINT != SZLONG )
					lastcon = (int) lastcon;
#endif
					yylval.intval = INT;
				}
				return(ICON);
			}
		}
	}
				return( ICON );
				}
			} /* end of case A_DIG */

		case A_DOT:
			/* look for a dot:
			 *    if followed by a digit, floating point
			 *    if followed by two more dots, ellipsis
			 */
			lxchar = gchar();
			if( lxmask[lxchar+1] & LEXDIG ){
				ungchar(lxchar,stdin);
				lxget( '.', LEXDIG );
				goto getfp;
				}

			if ( lxmask[lxchar+1] & LEXDOT ) {
				if ( lxmask[(lxchar=gchar())+1] & LEXDOT ) {
					return( ELLIPSIS );
				}
				else {
					/* saw two dots but not a third,
					 * return the last character seen
					 * and reset lxchar to the dot we
					 * did see
					 */
					ungchar(lxchar,stdin);
					lxchar = '.';
				}
			}

			stwart = FUNNYNAME;
			goto onechar;

		case A_STR:
			/* string constant */
			yylval.intval = 0; /* fake the regular type string type */
			lxmatch = '"';
			return( STRING );

		case A_CC:
			/* character constant */
			lxmatch = '\'';
			lastcon = 0;
			lxstr(0);
			yylval.intval = UCHAR;
			return( ICON );

		case A_BCD:
			{
				register i;
				int j;
				for( i=0; i<LXTSZ; ++i ){
					if( ( j = gchar() ) == '`' ) break;
					if( j == '\n' ){
						/* "newline in BCD constant" */
						UERROR( ALWAYS, MESSAGE(77) );
						break;
						}
					yytext[i] = j;
					}
				yytext[i] = '\0';
				/* "BCD constant exceeds 6 characters" */
				if( i>6 ) UERROR( ALWAYS, MESSAGE(10) );
				/* "gcos BCD constant illegal" */
				UERROR( ALWAYS, MESSAGE(48) );
				yylval.intval = 0;  /* not long */
				return( ICON );
				}

		case A_SL:
			/* / "/_*" /= are valid*/
			switch (lxchar = gchar()) {
			case '*':
				lxcom();
				/* continue yylex main for loop */
				continue;
			case '/':  /* C++ style comments */
				if (!devdebug[ANSI_MODE]) {
				  while((lxchar = gchar()) != '\n' 
					&& lxchar != EOF);
				  ungchar(lxchar, stdin);
				  continue;
				} else
				  goto onechar;

			case '=':
				return(yylval.intval = ASG_DIV);
			default :
				goto onechar;
			}

		case A_WS:
			continue;

		case A_NL:
			++lineno;
			lxtitle();
			continue;

		case A_NOT:
			/* ! */
			if( (lxchar=gchar()) != '=' ) goto onechar;
			yylval.intval = NE;
			return( EQUOP );

		case A_MI:
			/* - -> -= -- are valid */
			switch (lxchar = gchar()) {
			case '-':
				yylval.intval = DECR;
				return( INCOP );
			case '>':
				stwart = FUNNYNAME;
				yylval.intval=STREF;
				return( STROP );
			case '=':
				return(yylval.intval = ASG_MINUS);
			default :
				goto onechar;
			}

		case A_PL:
			/* + ++ += */
			switch(lxchar = gchar()) {
			case '+':
				yylval.intval = INCR;
				return( INCOP );
			case '=':
				return(yylval.intval = ASG_PLUS);
			default :
				goto onechar;
			}

		case A_AND:
			/* & && &= are valid */
			switch(lxchar = gchar()) {
			case '&':
				return( yylval.intval = ANDAND );
			case '=':
				return( yylval.intval = ASG_AND );
			default :
				goto onechar;
			}

		case A_OR:
			/* | || |= are valid */
			switch(lxchar = gchar()) {
			case '|':
				return( yylval.intval = OROR );
			case '=':
				return( yylval.intval = ASG_OR );
			default :
				goto onechar;
			}

		case A_LT:
			/* < << <= <<= are valid */
			if( (lxchar=gchar()) == '<' ){
				if ((lxchar = gchar()) == '=')
					return(yylval.intval = ASG_LS);
				else {
					ungchar(lxchar, stdin);
					yylval.intval = LS;
					return( SHIFTOP );
				}
			}
			if( lxchar != '=' ) goto onechar;
			yylval.intval = LE;
			return( RELOP );

		case A_GT:
			/* > */
			if( (lxchar=gchar()) == '>' ){
				if ((lxchar = gchar()) == '=')
					return(yylval.intval = ASG_RS);
				else {
					ungchar(lxchar, stdin);
					yylval.intval = RS;
					return(SHIFTOP );
				}
			}
			if( lxchar != '=' ) goto onechar;
			yylval.intval = GE;
			return( RELOP );

		case A_EQ:
			/* = */
			switch( lxchar = gchar() ){

			case '=':
				yylval.intval = EQ;
				return( EQUOP );

			case '+':
				yylval.intval = ASG_PLUS;
				goto warn;

			case '-':
				yylval.intval = ASG_MINUS;
				/* goto warn; */

			warn:
				while( lxmask[ (lxchar=gchar())+1] & LEXWS) {
					/* save any white spaces in temp array
					   to restore later */
					tempchar[chrcnt++] = lxchar;
					}
				if( (lxmask[lxchar] & (LEXLET|LEXDIG|LEXDOT)) ||
				    (lxchar == '-') || (lxchar == '+') ||
				    (lxchar == '&') || (lxchar == '(') ||
				    (lxchar == '*') ){
					/* "ambiguous assignment for non-ansi compilers" */
					WARNING( WPORTABLE, MESSAGE(12) );
					}
				/* restore the last char read by the while*/
				ungchar( lxchar ,stdin);  /* restore meaningful char */
				while (chrcnt--)  /* restore any white spaces */
					ungchar( (int)tempchar[chrcnt], stdin);
				/* restore the op character */
				lxchar = *(scp-1);
				chrcnt = 0;
				break;

			case '*':
				yylval.intval = ASG_MUL;
				goto warn;

			case '/':
				yylval.intval = ASG_DIV;
				goto warn;

			case '%':
				yylval.intval = ASG_MOD;
				goto warn;

			case '&':
				yylval.intval = ASG_AND;
				goto warn;

			case '|':
				yylval.intval = ASG_OR;
				goto warn;

			case '^':
				yylval.intval = ASG_ER;
				goto warn;

			case '<':
				if( (lxchar=gchar()) != '<' ){
					/* "=<%c illegal" */
					UERROR( ALWAYS, MESSAGE(8), lxchar );
					}
				ungchar(lxchar, stdin);
				yylval.intval = ASG_LS;
				goto warn;

			case '>':
				if( (lxchar=gchar()) != '>' ){
					/* "=>%c illegal" */
					UERROR( ALWAYS, MESSAGE(9), lxchar );
					}
				ungchar(lxchar, stdin);
				yylval.intval = ASG_RS;
				goto warn;

			default:
				goto onechar;

				}

			goto onechar;

		default:
			cerror(TOOLSTR(M_MSG_213, "yylex error, character %03o (octal)"), lxchar );

			}

		/* ordinarily, repeat here... */
		cerror(TOOLSTR(M_MSG_214, "out of switch in yylex" ));

		}

	}

struct lxrdope {
	/* dope for reserved, in alphabetical order */

	char *lxrch;	/* name of reserved word */
	short lxract;	/* reserved word action */
	short lxrval;	/* value to be returned */
	} lxrdope[] = {
/*****	"asm",		AR_A,	0,	*********removed********/
	"auto",		AR_CL,	AUTO,
	"break",	AR_RW,	BREAK,
	"char",		AR_TY,	CHAR,
	"case",		AR_RW,	CASE,
	"continue",	AR_RW,	CONTINUE,
	"const",	AR_QU,	CONST,
	"double",	AR_TY,	DOUBLE,
	"default",	AR_RW,	DEFAULT,
	"do",		AR_RW,	DO,
	"extern",	AR_CL,	EXTERN,
	"else",		AR_RW,	ELSE,
	"enum",		AR_E,	ENUM,
	"for",		AR_RW,	FOR,
	"float",	AR_TY,	FLOAT,
/*****	"fortran",	AR_CL,	FORTRAN,	****removed*******/
	"goto",		AR_GO,	GOTO,
	"if",		AR_RW,	IF,
	"int",		AR_TY,	INT,
	"long",		AR_TY,	LONG,
	"return",	AR_RW,	RETURN,
	"register",	AR_CL,	REGISTER,
	"switch",	AR_RW,	SWITCH,
	"struct",	AR_S,	0,
	"sizeof",	AR_RW,	SIZEOF,
	"signed",       AR_TY,  SIGNED,
	"short",	AR_TY,	SHORT,
	"static",	AR_CL,	STATIC,
	"typedef",	AR_CL,	TYPEDEF,
	"unsigned",	AR_TY,	UNSIGNED,
	"union",	AR_U,	0,
	"void",		AR_TY,	TVOID,
	"volatile",	AR_QU,	VOLATILE,
	"while",	AR_RW,	WHILE,
	"__builtin_isfloat", AR_RW, ISFLT,
	"__unaligned", AR_QU, UNALIGNED,
	"",		0,	0,	/* to stop the search */
	};

/* -------------------- lxres -------------------- */

lxres() {
	/* check to see of yytext is reserved; if so,
	 * do the appropriate action and return 
	 * otherwise, return -1 */

	register c, ch;
	register struct lxrdope *p;

	ch = yytext[0];

	if( !islower(ch) && ch != '_') return -1;

	switch( ch ){

	case 'a':
		c=0; break;
	case 'b':
		c=1; break;
	case 'c':
		c=2; break;
	case 'd':
		c=6; break;
	case 'e':
		c=9; break;
	case 'f':
		c=12; break;
	case 'g':
		c=14; break;
	case 'i':
		c=15; break;
	case 'l':
		c=17; break;
	case 'r':
		c=18; break;
	case 's':
		c=20; break;
	case 't':
		c=26; break;
	case 'u':
		c=27; break;
	case 'v':
		c=29; break;
	case 'w':
		c=31; break;
	case '_':
		c=32; break;
	default:
		return( -1 );
		}

	for( p= lxrdope+c; p->lxrch[0] == ch; ++p ){
		if( !strcmp( yytext, p->lxrch ) ){ /* match */
			switch( p->lxract ){

			case AR_TY:
				/* type specifier word */
				stwart = instruct;
				yylval.nodep = mkty(tyalloc((TWORD)p->lxrval));
				return( TYPE );

			case AR_RW:
				/* ordinary reserved word */
				return( yylval.intval = p->lxrval );

			case AR_CL:
				/* class word */
				yylval.intval = p->lxrval;
				return( CLASS );

			case AR_S:
				/* struct */
				stwart = INSTRUCT|SEENAME|TAGNAME;
				yylval.intval = INSTRUCT;
				return( STRUCT );

			case AR_U:
				/* union */
				stwart = INUNION|SEENAME|TAGNAME;
				yylval.intval = INUNION;
				return( STRUCT );

			case AR_E:
				/* enums */
				stwart = SEENAME|TAGNAME;
				return( yylval.intval = ENUM );

			case AR_GO:
				/* goto */
				stwart = SEENAME|LABNAME;
				return( yylval.intval = GOTO );

			case AR_A:
				/* asm */
				asm_esc = 1; /* warn the world! */
				asmp = asmbuf;
				lxget( ' ', LEXWS );
				if( gchar() != '(' ) goto badasm;
				lxget( ' ', LEXWS );
				if( gchar() != '"' ) goto badasm;
				while( (c=gchar()) != '"' ){
					if( c=='\n' || c==EOF ) goto badasm;
					*asmp++ = c;
					if( asmp >= &asmbuf[ASMBUF-1] ) {
						UERROR( ALWAYS,
							"asm > %d chars",
							ASMBUF );
						}
					}
				lxget( ' ', LEXWS );
				if( gchar() != ')' ) goto badasm;
				*asmp++ = '\0';
				return( ASM );

			case AR_QU:
				/* type qualifier word */
				yylval.intval = p->lxrval;
				return( QUAL );

			badasm:
				/* "bad asm construction" */
				UERROR( ALWAYS, MESSAGE(16) );
				return( 0 );

			default:
				cerror(TOOLSTR(M_MSG_215, "bad AR_?? action" ));
				}
			}
		}
	return( -1 );
	}

/* -------------------- lxtitle -------------------- */

lxtitle(){
	/* called after a newline; set linenumber and file name */

	register c, val;
	register char *cp;
	char *p, *strip();
	char testprag[100];
	static int first_title = 0;

	for(;;){  /* might be several such lines in a row */
		if( (c=gchar()) != '#' ){
			if( c != EOF ) ungchar(c,stdin);
			return;
			}

		lxget( ' ', LEXWS );
		/* Only set line number and title if a digit is present */
		c = gchar();
		if (isdigit(c)) {
		  val = 0;
		  while( isdigit(c)){
		    val = val*10+ c - '0';
		    c=gchar();
		  }
		  ungchar( c, stdin );
		  lineno = val;
		} else {
		  for( cp=testprag; c!='\n' && c!= EOF; c=gchar(),++cp )
		    *cp = c;
		  *cp = '\0';
		  if (strncmp("pragma", testprag, 6) != 0) 
		    /* "constant expected" */
		    UERROR(ALWAYS, MESSAGE(23));
		  ungchar(c, stdin); /* Return '/n' */
		  return;
		}
		lxget( ' ', LEXWS );
		if( (c=gchar()) != '\n' ){
			for( cp=ftitle; c!='\n'; c=gchar(),++cp )
				*cp = c;
			*cp = '\0';
#if	defined (LINT) || defined (CFLOW)
			/* Get actual filename only once. Note that pfname
			 * is initialized when file is openend
			 */
			p = strip(ftitle);
			if (!first_title++) {
				pfname = getmem(strlen(p) + 1);
				strcpy(pfname, p);
			}
			/* Get include filename. */
			ifname = getmem(strlen(p) + 1);
			strcpy(ifname, p);
#endif
#ifdef	CXREF
			printf("%s\n", ftitle);
#endif
			}
		}
	}

# ifndef MYASMOUT
/* -------------------- asmout -------------------- */

asmout()
	/* write out asm string
	 * this is a null function for lint
	 */
{
#if	!defined (CXREF) && !defined (LINT) && !defined (CFLOW)
# ifndef ONEPASS
	putchar( IASM );
# endif
	printf( "%s\n", asmbuf );
#endif
}
# endif

/*****************************************************************************
* getmore: read a line from the input, echo it if appropriate, and pass      *
* a character back                                                           *
*****************************************************************************/

/* -------------------- getmore -------------------- */
static char *bltins[] = 
{ 
  "extern char 		*__builtin_alloca(int);\n",
  "extern int 		__builtin_abs(int);\n",
  "extern double 	__builtin_fabs(double);\n",
  "extern long		__builtin_labs(long);\n",
  "extern long		__builtin_asm(const char *,... );\n",
  "extern float		__builtin_fasm(const char *,... );\n",
  "extern double	__builtin_dasm(const char *,... );\n",
  "extern char		*__builtin_strcpy(char *, const char *);\n",
  "extern void		__builtin_va_start();\n",
  ""
};
static int debug_sav[12];
static void save_debug(void)
{
  register int i = 0;
  debug_sav[i++] =  ddebug;
  ddebug = 0;
  debug_sav[i++] =  idebug; 
  idebug = 0;
  debug_sav[i++] =  bdebug;
  bdebug = 0;
  debug_sav[i++] =  tdebug;
  tdebug = 0;
  debug_sav[i++] =  edebug;
  edebug = 0;
  debug_sav[i++] =  fdebug;
  fdebug = 0;
  debug_sav[i++] =  xdebug;
  xdebug = 0;
  debug_sav[i++] =  cdebug;
  cdebug = 0;
  debug_sav[i++] =  odebug;
  odebug = 0;
  debug_sav[i++] =  gdebug;
  gdebug = 0;
  debug_sav[i++] =  yydebug;
  yydebug = 0;
};
static void restore_debug(void)
{
  register int i = 0;
  ddebug =  debug_sav[i++]; 
  idebug  =  debug_sav[i++]; 
  bdebug =  debug_sav[i++]; 
  tdebug =  debug_sav[i++]; 
  edebug =  debug_sav[i++]; 
  fdebug =  debug_sav[i++]; 
  xdebug =  debug_sav[i++]; 
  cdebug =  debug_sav[i++]; 
  odebug =  debug_sav[i++]; 
  gdebug =  debug_sav[i++]; 
  yydebug =  debug_sav[i++]; 
};
static char 
  getmore()
{
  static unsigned int actual_lno; /* Used for debugging */
  static char **bip = bltins;
  if (doing_builtins) {
    if (**bip) {
      scp = *bip++;
      if (cdebug)
	printf("#(BUILTINS)#%s", scp);
      return *scp++;
    } else {
      doing_builtins = 0;
      restore_debug();
      lineno = 1;
    }
  }
  if(feof(stdin) || fgets(sbuf, INBUFSZ-1, stdin) == NULL) {
    return EOF;
  }
  
  if (lineno == 0) {
    doing_builtins = 1;
    save_debug();
  }
  if (++actual_lno == dbg_from_lno) {
    if (dbg_to_lno > 0 && dbg_to_lno <= dbg_from_lno)
      dbg_to_lno = dbg_from_lno + 1;
    /* Turn on all debugging flags */
    ++ddebug;
    ++idebug; 
    ++bdebug;
    ++tdebug;
    ++edebug;
    ++fdebug;
    ++xdebug;
    ++cdebug;
    ++odebug;
    ++gdebug;
    ++yydebug;
  }
  if(cdebug) {
    printf("#(%d,%d)#%s", actual_lno, lineno, sbuf);
    /* check for partial line */
    if(sbuf[strlen(sbuf) - 1] != '\n')
      printf("...\n");
  }
  if (actual_lno == dbg_to_lno) {
    /* Turn off all debugging flags */
    if (--ddebug < 0) ddebug = 0;
    if (--idebug < 0) idebug = 0; 
    if (--bdebug < 0) bdebug = 0;
    if (--tdebug < 0) tdebug = 0;
    if (--edebug < 0) edebug = 0;
    if (--fdebug < 0) fdebug = 0;
    if (--xdebug < 0) xdebug = 0;
    if (--cdebug < 0) cdebug = 0;
    if (--odebug < 0) odebug = 0;
    if (--gdebug < 0) gdebug = 0;
    if (--yydebug < 0) yydebug = 0;
  }
  scp = sbuf;
  return *scp++;
}
