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
%}
%{
/*
 * Copyright 1988 by Digital Equipment Corporation, Maynard, Massachusetts.
 * 
 *                         All Rights Reserved
 * 
 * Permission to use, copy, modify, and distribute this software and its 
 * documentation for any purpose and without fee is hereby granted, 
 * provided that the above copyright notice appear in all copies and that
 * both that copyright notice and this permission notice appear in 
 * supporting documentation, and that the name of Digital Equipment
 * Corporation not be used in advertising or publicity pertaining to
 * distribution of the software without specific, written prior permission.  
 * 
 * 
 * DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
 * ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
 * DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
 * ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
 * WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
 * ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
 * SOFTWARE.
 */

/*
 *
 *	dxdiff
 *
 *	parsediffl.l - lex description for diff parser
 *
 *	Author:	Laurence P. G. Cable
 *
 *	Created : 29th February 1988
 *
 *
 *	Description
 *	-----------
 *
 *	This lex description is used to parse the output from the diff(1)
 *	utility !!! Should absorb all the possible interesting output from
 *	the -r -s -b and -h options. the lex routine is called from the yacc
 *	parser.
 *
 *
 *	Modification History
 *	------------ -------
 *	
 *	21st April 1988		Laurence P. G. Cable
 *
 *	Added discard_diffs flag.
 *
 *	22nd April 1988		Laurence P. G. Cable
 *
 *	Added the code to set the input stream
 *	to the fd returned from the pipe.
 */

static char sccsid[] = "@(#)parsediffl.l	1.6	17:45:44 2/21/89";

#undef	output
#define	output(c)	/* discard anything */

#include <sys/types.h>
#include <sys/stat.h>
#ifdef  __osf__
#include <Xm/Xm.h>
#else
typedef unsigned int XmTextPosition;
typedef enum {XmHIGHLIGHT_NORMAL, XmHIGHLIGHT_SELECTED,
	      XmHIGHLIGHT_SECONDARY_SELECTED} XmHighlightMode;
#endif

#include "y.tab.h"
#include "dxdiff.h"
#include "filestuff.h"
#include "parsediff.h"
#include "alloc.h"

extern	YYSTYPE	yylval,yyval;

extern int atoi();
int discard_diffs = 1;	/* for R1 throw diff context lines away! */

#if	DEBUG || LEXDEBUG


static int lexdebug = 1;
int debug = 1;	/***** REMOVE THIS LATER *****/

#define	DEBUGMSG(x)			\
	if (debug || lexdebug) {	\
		printf(x);		\
	}

#define	DEBUGMSG1(x,y)			\
	if (debug || lexdebug) {	\
		printf(x,y);		\
	}

#endif	DEBUG || LEXDEBUG

%}

%o	12000
%a	10000
%p	3000

NUMBER		[0-9]+
NUMSEQ		{NUMBER}(","{NUMBER})?

SPACE		[ \t]+
OPTSPACE	[ \t]*

EDCMD		(a|c|d)
EDCMDLINE	{NUMSEQ}{OPTSPACE}{EDCMD}{OPTSPACE}{NUMSEQ}

DIFF1		"<"
DIFF2		">"

STRING		.*


DIFFCMD		diff
FLAGS		"-"[bhrs]

%{
/* FLAGS		"-"([behrs]|(D[^ \t]+)) */
%}

FILENAME	[a-zA-Z0-9_".";$\-,]+
PATHNAME	"/"*({FILENAME}"/"*)*{FILENAME}

ONLY		Only
FILES		Files
COMMON		Common
BINARY		Binary

%START DIFF EDCMDLINE EDCMD DIFFCMD DIFFER ONLY FILES COMMON BINARY

%%

%{
/*** find a diff context line ***/
%}

^{DIFF1}		{
#if	DEBUG || LEXDEBUG
				DEBUGMSG("<diff1>");
#endif	DEBUG || LEXDEBUG
				BEGIN DIFF;

				yylval.token.ch = '<';
				return YY_DIFF_IN_1_AND_NOT_2;
			}

^{DIFF2}		{
#if	DEBUG || LEXDEBUG
				DEBUGMSG("<diff2>");
#endif	DEBUG || LEXDEBUG
				BEGIN DIFF;

				yylval.token.ch = '>';
				return YY_DIFF_IN_2_AND_NOT_1;
			}

<DIFF>{STRING}		{
#if	DEBUG || LEXDEBUG
				DEBUGMSG1("<string>: \"%s\"\n",yytext);
#endif	DEBUG || LEXDEBUG
				BEGIN 0;

					yylval.token.string = (discard_diffs) 
							    ? (char *)NULL 
							    : DeepCopyString(yytext);
				return YY_STRING;
			}


%{
/*** chuck out some space ***/
%}

<EDCMD,DIFFCMD,ONLY,FILES,BINARY,COMMON>{SPACE}	;




%{
/*** find an edit line ***/
%}

<EDCMD>{NUMBER}$	{
#if	DEBUG || LEXDEBUG
				DEBUGMSG1("<number>: %d\n",atoi(yytext));
#endif	DEBUG || LEXDEBUG
				BEGIN 0;

				yylval.token.number = atoi(yytext);
				return YY_LINE_NUMBER;
			}

<EDCMD,EDCMDLINE>{NUMBER}	{
#if	DEBUG || LEXDEBUG
				DEBUGMSG1("<number>: %d\n",atoi(yytext));
#endif	DEBUG || LEXDEBUG

				yylval.token.number = atoi(yytext);
				return YY_LINE_NUMBER;
			}

<EDCMDLINE,EDCMD>","	{
#if	DEBUG || LEXDEBUG
				DEBUGMSG("<comma>\n");
#endif	DEBUG || LEXDEBUG
				
				return YY_COMMA;
			}

<EDCMDLINE>{EDCMD}	{
#if	DEBUG || LEXDEBUG
				DEBUGMSG1("<edcmd>: '%c'\n",yytext[yyleng - 1]);
#endif	DEBUG || LEXDEBUG
				BEGIN EDCMD;

				yylval.token.ch = yytext[yyleng - 1];
				switch (yylval.token.ch) { 
					case 'a':
						return YY_APPEND_CMD;
					case 'c':
						return YY_CHANGE_CMD;
					case 'd':
						return YY_DELETE_CMD;
				}
			}

^{EDCMDLINE}$		{
#if	DEBUG || LEXDEBUG
				DEBUGMSG1("<edcmdline>%d\n",yyleng);
#endif	DEBUG || LEXDEBUG
				BEGIN EDCMDLINE;
				yyless(0);
			}


%{
/*** swallow some junk ***/
%}

^---\n		{ 
#if	DEBUG || LEXDEBUG
				DEBUGMSG("--- : back to state 0\n");
#endif	DEBUG || LEXDEBUG
				BEGIN 0;

				return YY_SEP;
}


\n			{
#if	DEBUG || LEXDEBUG
				DEBUGMSG("\\n: back to state 0\n");
#endif	DEBUG || LEXDEBUG
				BEGIN 0;

				return	YY_EOLN;

			}


%{
/*** find diff error messages ***/
%}

^{DIFFCMD}:		{
#if	DEBUG || LEXDEBUG
				DEBUGMSG("<differr>\n");
#endif	DEBUG || LEXDEBUG
				BEGIN DIFFER;

				return YY_DIFF_ERROR;
			}

<DIFFER>[: \t]+		;

<DIFFER>{STRING}	{
#if	DEBUG || LEXDEBUG
				DEBUGMSG1("<differr>: '%s'\n",yytext);
#endif	DEBUG || LEXDEBUG

				yylval.token.string = DeepCopyString(yytext);
				return YY_STRING;
			}


%{
/*** discard uncessessary text  ***/
%}

<COMMON>in			|
<FILES>are			|
<BINARY>differ			|
<BINARY>which			|
<FILES,COMMON>identical		|
<BINARY,COMMON>files		|
<FILES,BINARY,COMMON>and	;




%{
/*** find a diff cmd line echoed ***/
%}

^{DIFFCMD}({SPACE}{FLAGS})*		{
#if	DEBUG || LEXDEBUG
				DEBUGMSG("<diffcmd>\n");
#endif	DEBUG || LEXDEBUG
				BEGIN DIFFCMD;

				return YY_DIFF_CMD_LINE;
			}


%{
/*** find an only line ***/
%}

^{ONLY}{SPACE}in{SPACE}	{
#if	DEBUG || LEXDEBUG
				DEBUGMSG("<only>\n");
#endif	DEBUG || LEXDEBUG
				BEGIN ONLY;

				return YY_ONLY_NOTIFICATION;
			}

<ONLY>.*':'		{
#if	DEBUG || LEXDEBUG
				DEBUGMSG1("<colon>: '%s'\n",yytext);
#endif	DEBUG || LEXDEBUG
				if (yytext[yyleng - 1] == ':') {
					yytext[yyleng - 1] = ' ';
					yyless(0);
				}
			
			}

%{
/*** return a pathname ***/
%}

<ONLY,COMMON,FILES,BINARY,DIFFCMD>{PATHNAME}	{
#if	DEBUG || LEXDEBUG
					DEBUGMSG1("<path>: '%s'\n",yytext);
#endif	DEBUG || LEXDEBUG

					yylval.token.string = DeepCopyString(yytext);
					return YY_PATHNAME;
			}



%{
/*** find a files line ***/
%}

^{FILES}		{
#if	DEBUG || LEXDEBUG
				DEBUGMSG("<files>\n");
#endif	DEBUG || LEXDEBUG
				BEGIN FILES;

				return YY_FILES_NOTIFICATION;
			}


%{
/*** find a common line ***/
%}


^{COMMON}		|
^{COMMON}.*:		{
#if	DEBUG || LEXDEBUG
				DEBUGMSG("<common>\n");
#endif	DEBUG || LEXDEBUG
				BEGIN COMMON;

				return YY_COMMON_NOTIFICATION;
			}



%{
/*** find a binary line ***/
%}


^{BINARY}{SPACE}files	{
#if	DEBUG || LEXDEBUG
				DEBUGMSG("<binary>\n");
#endif	DEBUG || LEXDEBUG
				BEGIN BINARY;

				return YY_BINARY_NOTIFICATION;
			}
%%

SetyylexInputStream(fd)
	int	fd;
{
	extern	FILE *fdopen();

	yyin = fdopen(fd, "r"); /* assign the pipe to the input stream */
}
