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
# include "stdio.h"
# define U(x) ((x)&0377)
# define NLSTATE yyprevious=YYNEWLINE
# define BEGIN yybgin = yysvec + 1 +
# define INITIAL 0
# define YYLERR yysvec
# define YYSTATE (yyestate-yysvec-1)
# define YYOPTIM 1
# define YYLMAX 200
# define output(c) putc(c,yyout)
# define input() (((yytchar=yysptr>yysbuf?U(*--yysptr):getc(yyin))==10?(yylineno++,yytchar):yytchar)==EOF?0:yytchar)
# define unput(c) {yytchar= (c);if(yytchar=='\n')yylineno--;*yysptr++=yytchar;}
# define yymore() (yymorfg=1)
# define ECHO fprintf(yyout, "%s",yytext)
# define REJECT { nstr = yyreject(); goto yyfussy;}
int yyleng; extern char yytext[];
int yymorfg;
extern char *yysptr, yysbuf[];
int yytchar;
FILE *yyin ={stdin}, *yyout ={stdout};
extern int yylineno;
struct yysvf { 
	struct yywork *yystoff;
	struct yysvf *yyother;
	int *yystops;};
struct yysvf *yyestate;
extern struct yysvf yysvec[], *yybgin;
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

#if	LEXDEBUG


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

#endif	LEXDEBUG

/* FLAGS		"-"([behrs]|(D[^ \t]+)) */
# define DIFF 2
# define EDCMDLINE 4
# define EDCMD 6
# define DIFFCMD 8
# define DIFFER 10
# define ONLY 12
# define FILES 14
# define COMMON 16
# define BINARY 18
# define YYNEWLINE 10
yylex(){
int nstr; extern int yyprevious;
/*** find a diff context line ***/
while((nstr = yylook()) >= 0)
yyfussy: switch(nstr){
case 0:
if(yywrap()) return(0); break;
case 1:
	{
#if	LEXDEBUG
				DEBUGMSG("<diff1>");
#endif	LEXDEBUG
				BEGIN DIFF;

				yylval.token.ch = '<';
				return YY_DIFF_IN_1_AND_NOT_2;
			}
break;
case 2:
	{
#if	LEXDEBUG
				DEBUGMSG("<diff2>");
#endif	LEXDEBUG
				BEGIN DIFF;

				yylval.token.ch = '>';
				return YY_DIFF_IN_2_AND_NOT_1;
			}
break;
case 3:
	{
#if	LEXDEBUG
				DEBUGMSG1("<string>: \"%s\"\n",yytext);
#endif	LEXDEBUG
				BEGIN 0;

					yylval.token.string = (discard_diffs) 
							    ? (char *)NULL 
							    : DeepCopyString(yytext);
				return YY_STRING;
			}
break;
/*** chuck out some space ***/
case 4:
;
break;
/*** find an edit line ***/
case 5:
{
#if	LEXDEBUG
				DEBUGMSG1("<number>: %d\n",atoi(yytext));
#endif	LEXDEBUG
				BEGIN 0;

				yylval.token.number = atoi(yytext);
				return YY_LINE_NUMBER;
			}
break;
case 6:
{
#if	LEXDEBUG
				DEBUGMSG1("<number>: %d\n",atoi(yytext));
#endif	LEXDEBUG

				yylval.token.number = atoi(yytext);
				return YY_LINE_NUMBER;
			}
break;
case 7:
{
#if	LEXDEBUG
				DEBUGMSG("<comma>\n");
#endif	LEXDEBUG
				
				return YY_COMMA;
			}
break;
case 8:
{
#if	LEXDEBUG
				DEBUGMSG1("<edcmd>: '%c'\n",yytext[yyleng - 1]);
#endif	LEXDEBUG
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
break;
case 9:
	{
#if	LEXDEBUG
				DEBUGMSG1("<edcmdline>%d\n",yyleng);
#endif	LEXDEBUG
				BEGIN EDCMDLINE;
				yyless(0);
			}
break;
/*** swallow some junk ***/
case 10:
	{ 
#if	LEXDEBUG
				DEBUGMSG("--- : back to state 0\n");
#endif	LEXDEBUG
				BEGIN 0;

				return YY_SEP;
}
break;
case 11:
		{
#if	LEXDEBUG
				DEBUGMSG("\\n: back to state 0\n");
#endif	LEXDEBUG
				BEGIN 0;

				return	YY_EOLN;

			}
break;
/*** find diff error messages ***/
case 12:
	{
#if	LEXDEBUG
				DEBUGMSG("<differr>\n");
#endif	LEXDEBUG
				BEGIN DIFFER;

				return YY_DIFF_ERROR;
			}
break;
case 13:
	;
break;
case 14:
{
#if	LEXDEBUG
				DEBUGMSG1("<differr>: '%s'\n",yytext);
#endif	LEXDEBUG

				yylval.token.string = DeepCopyString(yytext);
				return YY_STRING;
			}
break;
/*** discard uncessessary text  ***/
case 15:
		case 16:
		case 17:
		case 18:
		case 19:
	case 20:
	case 21:
;
break;
/*** find a diff cmd line echoed ***/
case 22:
	{
#if	LEXDEBUG
				DEBUGMSG("<diffcmd>\n");
#endif	LEXDEBUG
				BEGIN DIFFCMD;

				return YY_DIFF_CMD_LINE;
			}
break;
/*** find an only line ***/
case 23:
{
#if	LEXDEBUG
				DEBUGMSG("<only>\n");
#endif	LEXDEBUG
				BEGIN ONLY;

				return YY_ONLY_NOTIFICATION;
			}
break;
case 24:
	{
#if	LEXDEBUG
				DEBUGMSG1("<colon>: '%s'\n",yytext);
#endif	LEXDEBUG
				if (yytext[yyleng - 1] == ':') {
					yytext[yyleng - 1] = ' ';
					yyless(0);
				}
			
			}
break;
/*** return a pathname ***/
case 25:
{
#if	LEXDEBUG
					DEBUGMSG1("<path>: '%s'\n",yytext);
#endif	LEXDEBUG

					yylval.token.string = DeepCopyString(yytext);
					return YY_PATHNAME;
			}
break;
/*** find a files line ***/
case 26:
	{
#if	LEXDEBUG
				DEBUGMSG("<files>\n");
#endif	LEXDEBUG
				BEGIN FILES;

				return YY_FILES_NOTIFICATION;
			}
break;
/*** find a common line ***/
case 27:
	case 28:
	{
#if	LEXDEBUG
				DEBUGMSG("<common>\n");
#endif	LEXDEBUG
				BEGIN COMMON;

				return YY_COMMON_NOTIFICATION;
			}
break;
/*** find a binary line ***/
case 29:
{
#if	LEXDEBUG
				DEBUGMSG("<binary>\n");
#endif	LEXDEBUG
				BEGIN BINARY;

				return YY_BINARY_NOTIFICATION;
			}
break;
case -1:
break;
default:
fprintf(yyout,"bad switch yylook %d",nstr);
} return(0); }
/* end of yylex */

SetyylexInputStream(fd)
	int	fd;
{
	extern	FILE *fdopen();

	yyin = fdopen(fd, "r"); /* assign the pipe to the input stream */
}
int yyvstop[] ={
0,

3,
14,
0,

3,
14,
0,

3,
14,
0,

3,
14,
0,

3,
14,
0,

3,
14,
0,

3,
14,
0,

3,
14,
0,

3,
14,
0,

3,
14,
0,

3,
14,
0,

3,
14,
0,

3,
14,
0,

3,
14,
0,

3,
14,
0,

3,
14,
0,

3,
14,
0,

3,
14,
0,

3,
14,
0,

3,
14,
0,

11,
0,

1,
0,

2,
0,

3,
0,

3,
0,

3,
0,

1,
3,
0,

2,
3,
0,

3,
0,

3,
0,

3,
0,

3,
0,

3,
0,

7,
0,

6,
0,

8,
0,

6,
0,

8,
0,

4,
0,

6,
-5,
0,

6,
-5,
0,

25,
0,

25,
0,

25,
0,

25,
0,

25,
0,

25,
0,

25,
0,

25,
0,

14,
0,

13,
14,
0,

14,
0,

14,
0,

1,
14,
0,

2,
14,
0,

14,
0,

14,
0,

14,
0,

14,
0,

14,
0,

4,
0,

25,
0,

25,
0,

25,
0,

1,
0,

2,
0,

25,
0,

25,
0,

25,
0,

25,
0,

25,
0,

25,
0,

25,
0,

25,
0,

25,
0,

25,
0,

25,
0,

25,
0,

25,
0,

3,
0,

3,
0,

3,
0,

3,
0,

3,
0,

3,
0,

3,
0,

3,
0,

3,
0,

5,
0,

25,
0,

25,
0,

25,
0,

25,
0,

25,
0,

25,
0,

25,
0,

25,
0,

14,
0,

14,
0,

14,
0,

14,
0,

14,
0,

14,
0,

14,
0,

14,
0,

14,
0,

25,
0,

25,
0,

25,
0,

25,
0,

25,
0,

25,
0,

25,
0,

25,
0,

25,
0,

25,
0,

25,
0,

25,
0,

15,
25,
0,

25,
0,

25,
0,

25,
0,

-9,
0,

3,
0,

3,
0,

3,
-9,
0,

3,
0,

3,
0,

3,
0,

3,
0,

3,
0,

25,
0,

25,
0,

25,
-9,
0,

25,
0,

25,
0,

25,
0,

25,
0,

25,
0,

14,
0,

14,
0,

14,
-9,
0,

14,
0,

14,
0,

14,
0,

14,
0,

14,
0,

24,
0,

25,
0,

25,
0,

25,
-9,
0,

25,
0,

25,
0,

25,
0,

25,
0,

25,
0,

21,
25,
0,

16,
25,
0,

25,
0,

25,
0,

25,
0,

25,
0,

25,
0,

10,
0,

9,
0,

22,
0,

3,
0,

3,
0,

3,
0,

3,
0,

3,
0,

3,
22,
0,

25,
0,

25,
0,

25,
0,

25,
0,

25,
0,

22,
25,
0,

14,
0,

14,
0,

14,
0,

14,
0,

14,
0,

14,
22,
0,

-9,
0,

25,
0,

25,
0,

25,
0,

25,
0,

25,
0,

22,
25,
0,

25,
0,

25,
0,

25,
0,

25,
0,

22,
25,
0,

-9,
0,

26,
0,

12,
0,

3,
-9,
0,

3,
0,

3,
0,

3,
26,
0,

3,
0,

3,
0,

3,
12,
0,

25,
-9,
0,

25,
0,

25,
0,

25,
26,
0,

14,
-9,
0,

14,
0,

14,
0,

14,
26,
0,

14,
0,

14,
0,

12,
14,
0,

25,
-9,
0,

25,
0,

25,
0,

25,
26,
0,

12,
0,

25,
0,

20,
25,
0,

25,
0,

18,
25,
0,

27,
0,

3,
0,

3,
27,
0,

3,
0,

3,
0,

25,
0,

25,
27,
0,

14,
0,

14,
27,
0,

14,
0,

14,
0,

-9,
0,

25,
0,

25,
27,
0,

25,
0,

17,
25,
0,

28,
0,

22,
0,

3,
0,

3,
0,

3,
28,
0,

3,
0,

3,
22,
0,

25,
0,

14,
0,

14,
0,

14,
28,
0,

14,
0,

14,
22,
0,

25,
0,

28,
0,

22,
0,

25,
0,

23,
0,

3,
0,

3,
23,
0,

14,
0,

14,
23,
0,

28,
0,

23,
0,

25,
0,

3,
0,

14,
0,

24,
0,

19,
25,
0,

3,
0,

14,
0,

3,
0,

14,
0,

29,
0,

3,
29,
0,

14,
29,
0,

29,
0,
0};
# define YYTYPE int
struct yywork { YYTYPE verify, advance; } yycrank[] ={
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	1,21,	
2,21,	3,31,	0,0,	0,0,	
0,0,	0,0,	0,0,	46,46,	
31,0,	3,31,	3,21,	34,0,	
35,0,	62,0,	63,0,	0,0,	
11,58,	0,0,	4,31,	32,0,	
0,0,	59,59,	59,0,	0,0,	
11,59,	0,0,	4,31,	4,21,	
0,0,	37,0,	46,46,	0,0,	
0,0,	60,0,	3,31,	2,22,	
36,0,	0,0,	2,23,	2,23,	
2,23,	2,23,	2,23,	2,23,	
2,23,	2,23,	2,23,	2,23,	
3,31,	11,58,	2,24,	4,31,	
2,25,	22,91,	32,100,	39,0,	
2,26,	2,27,	3,31,	50,50,	
2,28,	5,21,	4,32,	11,58,	
38,0,	4,33,	40,0,	51,111,	
60,119,	2,29,	59,59,	64,0,	
65,0,	11,59,	67,0,	4,31,	
91,147,	4,34,	111,163,	4,35,	
0,0,	66,0,	0,0,	4,36,	
4,37,	68,0,	0,0,	4,38,	
0,0,	0,0,	2,30,	0,0,	
0,0,	7,46,	7,21,	5,41,	
4,39,	0,0,	3,31,	5,42,	
5,42,	5,42,	5,42,	5,42,	
5,42,	5,42,	5,42,	5,42,	
5,42,	26,95,	27,96,	28,97,	
29,98,	11,58,	6,41,	4,31,	
7,46,	4,40,	6,44,	6,44,	
6,44,	6,44,	6,44,	6,44,	
6,44,	6,44,	6,44,	6,44,	
7,41,	30,99,	37,105,	36,104,	
7,47,	7,47,	7,47,	7,47,	
7,47,	7,47,	7,47,	7,47,	
7,47,	7,47,	8,46,	53,114,	
15,83,	54,115,	55,116,	56,117,	
5,43,	17,85,	5,43,	5,43,	
15,84,	19,85,	17,86,	39,107,	
19,88,	17,87,	19,86,	38,106,	
57,118,	40,108,	70,70,	70,0,	
76,0,	8,46,	64,123,	6,43,	
77,0,	6,43,	6,45,	9,46,	
9,21,	65,124,	67,126,	19,89,	
66,125,	8,41,	84,141,	85,139,	
68,127,	8,48,	8,48,	8,48,	
8,48,	8,48,	8,48,	8,48,	
8,48,	8,48,	8,48,	83,139,	
70,72,	76,72,	9,46,	83,140,	
9,49,	77,72,	9,49,	86,142,	
88,144,	89,145,	90,146,	95,150,	
96,151,	97,152,	9,49,	9,49,	
9,49,	9,50,	9,49,	9,49,	
9,49,	9,49,	9,49,	9,49,	
9,49,	9,49,	9,49,	9,49,	
98,153,	9,49,	99,154,	100,0,	
114,166,	115,167,	108,0,	9,49,	
9,49,	9,49,	9,49,	9,49,	
9,49,	9,49,	9,49,	9,49,	
9,49,	9,49,	9,49,	9,49,	
9,49,	9,49,	9,49,	9,49,	
9,49,	9,49,	9,49,	9,49,	
9,49,	9,49,	9,49,	9,49,	
9,49,	104,0,	116,168,	105,0,	
117,169,	9,49,	100,155,	9,49,	
9,49,	9,49,	9,49,	9,49,	
9,49,	9,49,	9,49,	9,49,	
9,49,	9,49,	9,49,	9,49,	
9,49,	9,49,	9,49,	9,49,	
9,49,	9,49,	9,49,	9,49,	
9,49,	9,49,	9,49,	9,49,	
9,49,	10,51,	106,0,	118,170,	
10,52,	10,52,	10,52,	10,52,	
10,52,	10,52,	10,52,	10,52,	
10,52,	10,52,	102,0,	107,0,	
10,24,	12,58,	10,25,	129,0,	
13,69,	139,189,	10,53,	10,54,	
58,58,	12,59,	10,55,	121,0,	
13,70,	13,21,	71,69,	71,0,	
58,58,	58,0,	108,162,	10,56,	
42,42,	42,42,	42,42,	42,42,	
42,42,	42,42,	42,42,	42,42,	
42,42,	42,42,	14,69,	127,0,	
129,179,	87,141,	12,58,	72,0,	
102,156,	13,71,	14,70,	14,21,	
10,57,	58,58,	13,72,	87,143,	
71,72,	12,60,	103,103,	103,0,	
12,61,	121,172,	13,73,	13,71,	
71,128,	104,158,	105,159,	58,58,	
73,69,	73,0,	12,59,	123,0,	
12,62,	13,69,	12,63,	14,71,	
72,72,	58,58,	12,64,	12,65,	
14,72,	119,0,	12,66,	122,122,	
122,0,	140,190,	14,74,	124,0,	
14,73,	14,75,	141,191,	12,67,	
142,192,	144,193,	145,194,	72,129,	
106,160,	103,157,	73,72,	14,69,	
125,0,	14,76,	126,0,	14,77,	
146,195,	147,196,	73,73,	14,78,	
14,79,	107,161,	12,58,	14,80,	
12,68,	13,71,	74,69,	74,0,	
119,171,	58,58,	150,199,	151,200,	
14,81,	16,51,	122,173,	152,201,	
16,52,	16,52,	16,52,	16,52,	
16,52,	16,52,	16,52,	16,52,	
16,52,	16,52,	153,202,	127,178,	
16,24,	154,203,	16,25,	14,71,	
155,196,	14,82,	16,53,	16,54,	
74,72,	159,0,	16,55,	128,69,	
128,0,	23,92,	74,130,	158,0,	
74,128,	18,51,	161,0,	16,56,	
18,52,	18,52,	18,52,	18,52,	
18,52,	18,52,	18,52,	18,52,	
18,52,	18,52,	160,0,	162,0,	
18,24,	163,196,	18,25,	123,174,	
23,92,	16,83,	18,53,	18,54,	
16,57,	128,72,	18,55,	166,211,	
167,212,	16,84,	168,213,	92,92,	
23,93,	128,128,	124,175,	18,56,	
23,23,	23,23,	23,23,	23,23,	
23,23,	23,23,	23,23,	23,23,	
23,23,	23,23,	125,176,	169,214,	
126,177,	170,215,	101,101,	101,0,	
171,196,	18,85,	92,92,	175,0,	
18,57,	191,229,	18,86,	20,51,	
177,0,	18,87,	20,52,	20,52,	
20,52,	20,52,	20,52,	20,52,	
20,52,	20,52,	20,52,	20,52,	
157,197,	192,230,	20,24,	178,0,	
20,25,	193,231,	120,120,	120,0,	
20,53,	20,54,	33,31,	174,0,	
20,55,	23,94,	158,205,	23,94,	
23,94,	173,197,	33,101,	33,0,	
159,206,	20,56,	44,44,	44,44,	
44,44,	44,44,	44,44,	44,44,	
44,44,	44,44,	44,44,	44,44,	
47,109,	160,207,	157,204,	162,209,	
148,92,	176,0,	157,157,	20,85,	
194,232,	161,208,	20,90,	33,31,	
20,86,	48,109,	195,233,	92,94,	
199,235,	92,94,	92,94,	173,216,	
200,236,	33,102,	201,237,	173,173,	
205,0,	33,33,	202,238,	148,92,	
211,249,	20,89,	212,250,	78,69,	
78,0,	213,251,	101,103,	33,31,	
101,103,	101,103,	47,47,	47,47,	
47,47,	47,47,	47,47,	47,47,	
47,47,	47,47,	47,47,	47,47,	
203,239,	202,238,	175,218,	48,48,	
48,48,	48,48,	48,48,	48,48,	
48,48,	48,48,	48,48,	48,48,	
48,48,	78,72,	120,122,	178,221,	
120,122,	120,122,	174,217,	177,220,	
49,49,	78,128,	49,49,	203,239,	
229,267,	206,0,	33,103,	33,31,	
33,103,	33,103,	49,49,	49,49,	
49,49,	49,110,	49,49,	49,49,	
49,49,	49,49,	49,49,	49,49,	
49,49,	49,49,	49,49,	49,49,	
148,94,	49,49,	148,94,	148,94,	
176,219,	203,240,	230,268,	49,49,	
49,49,	49,49,	49,49,	49,49,	
49,49,	49,49,	49,49,	49,49,	
49,49,	49,49,	49,49,	49,49,	
49,49,	49,49,	49,49,	49,49,	
49,49,	49,49,	49,49,	49,49,	
49,49,	49,49,	49,49,	49,49,	
49,49,	231,269,	207,0,	78,134,	
205,242,	49,49,	204,0,	49,49,	
49,49,	49,49,	49,49,	49,49,	
49,49,	49,49,	49,49,	49,49,	
49,49,	49,49,	49,49,	49,49,	
49,49,	49,49,	49,49,	49,49,	
49,49,	49,49,	49,49,	49,49,	
49,49,	49,49,	49,49,	49,49,	
49,49,	52,92,	93,148,	93,148,	
93,148,	93,148,	93,148,	93,148,	
93,148,	93,148,	93,148,	93,148,	
204,241,	208,245,	208,0,	156,101,	
156,0,	61,58,	206,243,	216,0,	
69,69,	79,69,	79,0,	75,69,	
52,92,	61,120,	61,0,	218,0,	
69,69,	69,0,	217,0,	75,131,	
75,0,	80,69,	80,0,	232,270,	
52,112,	234,197,	81,69,	81,0,	
52,52,	52,52,	52,52,	52,52,	
52,52,	52,52,	52,52,	52,52,	
52,52,	52,52,	61,58,	79,72,	
235,271,	69,69,	156,156,	236,272,	
75,71,	216,252,	69,72,	79,128,	
61,121,	75,72,	214,238,	80,72,	
61,61,	219,0,	75,132,	69,69,	
81,72,	75,128,	75,75,	80,128,	
82,69,	82,0,	61,58,	207,244,	
81,128,	69,69,	179,0,	238,273,	
75,69,	131,131,	131,0,	209,246,	
209,0,	214,238,	220,256,	220,0,	
244,0,	52,113,	243,0,	52,113,	
52,113,	247,0,	242,0,	130,69,	
130,0,	132,69,	132,0,	249,279,	
241,197,	94,94,	82,72,	156,103,	
250,280,	156,103,	156,103,	179,72,	
255,0,	258,0,	82,128,	131,72,	
253,0,	61,122,	61,58,	61,122,	
61,122,	69,69,	267,290,	75,133,	
75,71,	75,133,	75,133,	79,135,	
94,94,	130,72,	179,129,	132,72,	
218,254,	80,136,	269,291,	130,180,	
209,247,	130,128,	217,253,	132,128,	
132,182,	113,94,	241,241,	81,137,	
94,149,	94,149,	94,149,	94,149,	
94,149,	94,149,	94,149,	94,149,	
94,149,	94,149,	112,164,	112,164,	
112,164,	112,164,	112,164,	112,164,	
112,164,	112,164,	112,164,	112,164,	
113,94,	133,181,	133,0,	134,69,	
134,0,	135,69,	135,0,	273,295,	
82,138,	131,181,	219,255,	131,181,	
131,181,	252,197,	136,69,	136,0,	
113,165,	113,165,	113,165,	113,165,	
113,165,	113,165,	113,165,	113,165,	
113,165,	113,165,	137,69,	137,0,	
138,69,	138,0,	243,276,	133,72,	
290,317,	134,72,	149,197,	135,72,	
245,245,	245,0,	254,0,	133,128,	
133,183,	134,128,	266,0,	135,128,	
136,72,	242,275,	164,92,	215,239,	
221,257,	221,0,	292,318,	252,252,	
136,128,	293,0,	256,256,	256,0,	
137,72,	294,0,	138,72,	246,246,	
246,0,	172,120,	172,0,	253,281,	
137,128,	298,0,	138,128,	299,0,	
149,198,	164,92,	215,239,	266,72,	
149,149,	149,149,	149,149,	149,149,	
149,149,	149,149,	149,149,	149,149,	
149,149,	149,149,	180,69,	180,196,	
233,239,	164,164,	164,164,	164,164,	
164,164,	164,164,	164,164,	164,164,	
164,164,	164,164,	164,164,	246,278,	
215,240,	221,258,	277,0,	271,292,	
172,172,	165,197,	274,296,	305,0,	
134,184,	135,185,	278,0,	233,239,	
274,296,	275,297,	275,0,	306,0,	
180,72,	136,186,	281,304,	281,0,	
284,0,	317,327,	274,296,	274,296,	
180,128,	138,188,	271,292,	181,181,	
181,0,	137,187,	182,131,	182,0,	
183,69,	183,197,	184,69,	184,0,	
245,277,	233,240,	164,113,	165,210,	
164,113,	164,113,	254,282,	165,165,	
165,165,	165,165,	165,165,	165,165,	
165,165,	165,165,	165,165,	165,165,	
165,165,	172,122,	256,283,	172,122,	
172,122,	181,72,	185,69,	185,0,	
182,72,	222,197,	183,72,	259,0,	
184,72,	318,328,	181,222,	183,223,	
182,128,	182,182,	183,128,	183,183,	
184,128,	186,69,	186,0,	187,69,	
187,0,	188,69,	188,0,	283,0,	
233,269,	279,292,	327,333,	239,239,	
328,334,	297,297,	297,0,	334,338,	
185,72,	295,319,	222,72,	296,239,	
259,72,	300,321,	300,0,	222,259,	
185,128,	320,0,	278,301,	222,222,	
322,0,	259,285,	277,300,	186,72,	
279,292,	187,72,	239,239,	188,72,	
284,308,	257,257,	257,0,	186,128,	
295,319,	187,128,	296,239,	188,128,	
223,69,	223,0,	182,133,	239,274,	
182,133,	182,133,	184,224,	198,234,	
198,234,	198,234,	198,234,	198,234,	
198,234,	198,234,	198,234,	198,234,	
198,234,	210,248,	210,248,	210,248,	
210,248,	210,248,	210,248,	210,248,	
210,248,	210,248,	210,248,	224,69,	
224,0,	257,284,	223,72,	225,69,	
225,0,	338,342,	226,69,	226,0,	
227,264,	227,0,	223,128,	223,260,	
228,265,	228,0,	185,225,	260,69,	
260,197,	261,69,	261,0,	262,69,	
262,0,	186,226,	263,69,	263,0,	
248,197,	343,0,	188,228,	301,246,	
301,0,	224,72,	307,323,	307,0,	
344,0,	225,72,	297,320,	283,307,	
226,72,	224,128,	227,72,	304,304,	
304,0,	225,128,	228,72,	187,227,	
226,128,	260,72,	227,128,	261,72,	
320,329,	262,72,	228,128,	322,330,	
263,72,	260,128,	260,260,	261,128,	
280,0,	262,128,	264,264,	264,0,	
263,128,	228,266,	248,248,	248,248,	
248,248,	248,248,	248,248,	248,248,	
248,248,	248,248,	248,248,	248,248,	
265,265,	265,0,	272,293,	0,0,	
276,298,	285,197,	282,305,	302,0,	
280,302,	288,0,	272,293,	272,0,	
276,298,	276,0,	282,305,	282,0,	
264,72,	286,309,	286,0,	308,257,	
308,0,	280,303,	280,302,	289,0,	
309,309,	309,0,	329,0,	311,0,	
321,321,	321,0,	265,72,	302,302,	
224,261,	225,262,	285,72,	272,293,	
265,289,	276,298,	288,72,	282,305,	
226,263,	303,0,	312,0,	285,285,	
302,303,	302,302,	330,0,	286,72,	
262,287,	272,293,	335,0,	276,298,	
289,72,	282,305,	309,72,	286,128,	
311,312,	261,286,	287,310,	272,294,	
304,322,	276,299,	336,0,	282,306,	
311,313,	303,302,	287,310,	287,0,	
314,0,	313,0,	339,0,	312,312,	
280,302,	323,323,	323,0,	310,310,	
340,0,	0,0,	303,303,	303,302,	
315,326,	315,0,	264,288,	310,310,	
310,0,	324,0,	316,265,	316,0,	
0,0,	325,0,	312,325,	287,311,	
331,0,	326,326,	326,0,	302,302,	
287,312,	314,312,	313,312,	272,293,	
0,0,	276,298,	332,0,	282,305,	
287,313,	287,311,	313,313,	337,0,	
310,310,	288,315,	315,72,	289,316,	
0,0,	310,312,	324,72,	287,314,	
316,72,	309,324,	325,332,	341,0,	
345,0,	331,72,	310,310,	326,72,	
329,335,	0,0,	0,0,	0,0,	
0,0,	303,302,	0,0,	332,312,	
310,314,	0,0,	0,0,	0,0,	
337,72,	0,0,	0,0,	0,0,	
0,0,	335,339,	0,0,	0,0,	
330,336,	0,0,	0,0,	0,0,	
341,72,	345,72,	332,325,	0,0,	
0,0,	336,340,	0,0,	287,311,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
310,310,	0,0,	0,0,	339,343,	
0,0,	0,0,	0,0,	0,0,	
324,331,	340,344,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	331,337,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	337,341,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
341,345,	0,0,	0,0,	0,0,	
0,0};
struct yysvf yysvec[] ={
0,	0,	0,
yycrank+1,	0,		yyvstop+1,
yycrank+2,	0,		yyvstop+4,
yycrank+-12,	0,		yyvstop+7,
yycrank+-29,	0,		yyvstop+10,
yycrank+63,	0,		yyvstop+13,
yycrank+82,	yysvec+2,	yyvstop+16,
yycrank+96,	0,		yyvstop+19,
yycrank+145,	yysvec+2,	yyvstop+22,
yycrank+174,	0,		yyvstop+25,
yycrank+252,	yysvec+9,	yyvstop+28,
yycrank+-27,	yysvec+3,	yyvstop+31,
yycrank+-312,	yysvec+4,	yyvstop+34,
yycrank+-315,	0,		yyvstop+37,
yycrank+-341,	0,		yyvstop+40,
yycrank+59,	yysvec+9,	yyvstop+43,
yycrank+376,	yysvec+9,	yyvstop+46,
yycrank+64,	yysvec+9,	yyvstop+49,
yycrank+408,	yysvec+9,	yyvstop+52,
yycrank+68,	yysvec+9,	yyvstop+55,
yycrank+466,	yysvec+9,	yyvstop+58,
yycrank+0,	0,		yyvstop+61,
yycrank+20,	0,		0,	
yycrank+440,	0,		0,	
yycrank+0,	0,		yyvstop+63,
yycrank+0,	0,		yyvstop+65,
yycrank+16,	0,		0,	
yycrank+11,	0,		0,	
yycrank+18,	0,		0,	
yycrank+14,	0,		0,	
yycrank+36,	0,		0,	
yycrank+-10,	yysvec+3,	yyvstop+67,
yycrank+-21,	yysvec+3,	yyvstop+69,
yycrank+-533,	0,		yyvstop+71,
yycrank+-13,	yysvec+3,	yyvstop+73,
yycrank+-14,	yysvec+3,	yyvstop+76,
yycrank+-38,	yysvec+3,	yyvstop+79,
yycrank+-31,	yysvec+3,	yyvstop+81,
yycrank+-66,	yysvec+3,	yyvstop+83,
yycrank+-57,	yysvec+3,	yyvstop+85,
yycrank+-68,	yysvec+3,	yyvstop+87,
yycrank+0,	0,		yyvstop+89,
yycrank+284,	0,		yyvstop+91,
yycrank+0,	0,		yyvstop+93,
yycrank+498,	yysvec+23,	yyvstop+95,
yycrank+0,	yysvec+30,	yyvstop+97,
yycrank+10,	0,		yyvstop+99,
yycrank+546,	0,		yyvstop+101,
yycrank+559,	yysvec+23,	yyvstop+104,
yycrank+590,	0,		yyvstop+107,
yycrank+24,	yysvec+49,	0,	
yycrank+34,	yysvec+49,	yyvstop+109,
yycrank+704,	yysvec+49,	yyvstop+111,
yycrank+50,	yysvec+49,	yyvstop+113,
yycrank+46,	yysvec+49,	yyvstop+115,
yycrank+53,	yysvec+49,	yyvstop+117,
yycrank+49,	yysvec+49,	yyvstop+119,
yycrank+67,	yysvec+49,	yyvstop+121,
yycrank+-319,	0,		yyvstop+123,
yycrank+-24,	yysvec+58,	yyvstop+125,
yycrank+-35,	yysvec+58,	yyvstop+128,
yycrank+-728,	0,		yyvstop+130,
yycrank+-15,	yysvec+58,	yyvstop+132,
yycrank+-16,	yysvec+58,	yyvstop+135,
yycrank+-73,	yysvec+58,	yyvstop+138,
yycrank+-74,	yysvec+58,	yyvstop+140,
yycrank+-83,	yysvec+58,	yyvstop+142,
yycrank+-76,	yysvec+58,	yyvstop+144,
yycrank+-87,	yysvec+58,	yyvstop+146,
yycrank+-731,	0,		0,	
yycrank+-165,	yysvec+69,	yyvstop+148,
yycrank+-317,	yysvec+13,	yyvstop+150,
yycrank+-337,	yysvec+69,	0,	
yycrank+-359,	yysvec+13,	0,	
yycrank+-405,	yysvec+13,	yyvstop+152,
yycrank+-734,	0,		yyvstop+154,
yycrank+-166,	yysvec+69,	yyvstop+156,
yycrank+-170,	yysvec+69,	yyvstop+158,
yycrank+-578,	yysvec+13,	yyvstop+160,
yycrank+-724,	yysvec+13,	yyvstop+162,
yycrank+-736,	yysvec+13,	yyvstop+164,
yycrank+-741,	yysvec+13,	yyvstop+166,
yycrank+-775,	yysvec+13,	yyvstop+168,
yycrank+93,	yysvec+49,	yyvstop+170,
yycrank+90,	yysvec+49,	yyvstop+172,
yycrank+81,	yysvec+49,	yyvstop+174,
yycrank+106,	yysvec+49,	yyvstop+176,
yycrank+245,	yysvec+49,	yyvstop+178,
yycrank+107,	yysvec+49,	yyvstop+180,
yycrank+109,	yysvec+49,	yyvstop+182,
yycrank+109,	yysvec+49,	yyvstop+184,
yycrank+43,	0,		0,	
yycrank+474,	0,		0,	
yycrank+666,	0,		0,	
yycrank+804,	0,		0,	
yycrank+105,	0,		0,	
yycrank+107,	0,		0,	
yycrank+109,	0,		0,	
yycrank+124,	0,		0,	
yycrank+132,	0,		0,	
yycrank+-225,	yysvec+3,	yyvstop+186,
yycrank+-493,	yysvec+3,	yyvstop+188,
yycrank+-300,	yysvec+3,	yyvstop+190,
yycrank+-349,	yysvec+3,	yyvstop+192,
yycrank+-255,	yysvec+3,	yyvstop+194,
yycrank+-257,	yysvec+3,	yyvstop+196,
yycrank+-288,	yysvec+3,	yyvstop+198,
yycrank+-301,	yysvec+3,	yyvstop+200,
yycrank+-228,	yysvec+3,	yyvstop+202,
yycrank+0,	0,		yyvstop+204,
yycrank+0,	yysvec+49,	0,	
yycrank+45,	yysvec+49,	yyvstop+206,
yycrank+814,	yysvec+49,	yyvstop+208,
yycrank+840,	yysvec+49,	yyvstop+210,
yycrank+126,	yysvec+49,	yyvstop+212,
yycrank+128,	yysvec+49,	yyvstop+214,
yycrank+158,	yysvec+49,	yyvstop+216,
yycrank+160,	yysvec+49,	yyvstop+218,
yycrank+197,	yysvec+49,	yyvstop+220,
yycrank+-371,	yysvec+58,	yyvstop+222,
yycrank+-521,	yysvec+58,	yyvstop+224,
yycrank+-313,	yysvec+58,	yyvstop+226,
yycrank+-374,	yysvec+58,	yyvstop+228,
yycrank+-361,	yysvec+58,	yyvstop+230,
yycrank+-377,	yysvec+58,	yyvstop+232,
yycrank+-390,	yysvec+58,	yyvstop+234,
yycrank+-392,	yysvec+58,	yyvstop+236,
yycrank+-333,	yysvec+58,	yyvstop+238,
yycrank+-438,	yysvec+13,	0,	
yycrank+-305,	yysvec+69,	0,	
yycrank+-798,	yysvec+13,	yyvstop+240,
yycrank+-784,	yysvec+69,	0,	
yycrank+-800,	yysvec+13,	yyvstop+242,
yycrank+-864,	yysvec+13,	yyvstop+244,
yycrank+-866,	yysvec+13,	yyvstop+246,
yycrank+-868,	yysvec+13,	yyvstop+248,
yycrank+-877,	yysvec+13,	yyvstop+250,
yycrank+-889,	yysvec+13,	yyvstop+252,
yycrank+-891,	yysvec+13,	yyvstop+254,
yycrank+217,	yysvec+49,	yyvstop+256,
yycrank+284,	yysvec+49,	yyvstop+258,
yycrank+289,	yysvec+49,	yyvstop+260,
yycrank+284,	yysvec+49,	yyvstop+262,
yycrank+0,	yysvec+49,	yyvstop+264,
yycrank+291,	yysvec+49,	yyvstop+267,
yycrank+289,	yysvec+49,	yyvstop+269,
yycrank+302,	yysvec+49,	yyvstop+271,
yycrank+395,	0,		0,	
yycrank+551,	yysvec+93,	0,	
yycrank+896,	0,		yyvstop+273,
yycrank+321,	0,		0,	
yycrank+310,	0,		0,	
yycrank+322,	0,		0,	
yycrank+313,	0,		0,	
yycrank+335,	0,		0,	
yycrank+-430,	yysvec+3,	yyvstop+275,
yycrank+-718,	yysvec+3,	yyvstop+277,
yycrank+-514,	yysvec+3,	yyvstop+279,
yycrank+-441,	yysvec+3,	yyvstop+282,
yycrank+-435,	yysvec+3,	yyvstop+284,
yycrank+-456,	yysvec+3,	yyvstop+286,
yycrank+-444,	yysvec+3,	yyvstop+288,
yycrank+-457,	yysvec+3,	yyvstop+290,
yycrank+459,	yysvec+49,	yyvstop+292,
yycrank+909,	yysvec+49,	yyvstop+294,
yycrank+963,	yysvec+49,	yyvstop+296,
yycrank+382,	yysvec+49,	yyvstop+299,
yycrank+371,	yysvec+49,	yyvstop+301,
yycrank+381,	yysvec+49,	yyvstop+303,
yycrank+378,	yysvec+49,	yyvstop+305,
yycrank+399,	yysvec+49,	yyvstop+307,
yycrank+-494,	yysvec+58,	yyvstop+309,
yycrank+-924,	yysvec+58,	yyvstop+311,
yycrank+-531,	yysvec+58,	yyvstop+313,
yycrank+-525,	yysvec+58,	yyvstop+316,
yycrank+-497,	yysvec+58,	yyvstop+318,
yycrank+-551,	yysvec+58,	yyvstop+320,
yycrank+-502,	yysvec+58,	yyvstop+322,
yycrank+-517,	yysvec+58,	yyvstop+324,
yycrank+-780,	yysvec+69,	yyvstop+326,
yycrank+-945,	yysvec+13,	yyvstop+328,
yycrank+-986,	yysvec+69,	0,	
yycrank+-989,	yysvec+13,	yyvstop+330,
yycrank+-991,	yysvec+13,	yyvstop+332,
yycrank+-993,	yysvec+13,	yyvstop+335,
yycrank+-1017,	yysvec+13,	yyvstop+337,
yycrank+-1032,	yysvec+13,	yyvstop+339,
yycrank+-1034,	yysvec+13,	yyvstop+341,
yycrank+-1036,	yysvec+13,	yyvstop+343,
yycrank+0,	yysvec+49,	yyvstop+345,
yycrank+0,	yysvec+49,	yyvstop+348,
yycrank+399,	yysvec+49,	yyvstop+351,
yycrank+424,	yysvec+49,	yyvstop+353,
yycrank+427,	yysvec+49,	yyvstop+355,
yycrank+465,	yysvec+49,	yyvstop+357,
yycrank+468,	yysvec+49,	yyvstop+359,
yycrank+0,	0,		yyvstop+361,
yycrank+0,	0,		yyvstop+363,
yycrank+1043,	0,		0,	
yycrank+458,	0,		0,	
yycrank+465,	0,		0,	
yycrank+463,	0,		0,	
yycrank+573,	0,		0,	
yycrank+595,	0,		yyvstop+365,
yycrank+-676,	yysvec+3,	yyvstop+367,
yycrank+-570,	yysvec+3,	yyvstop+369,
yycrank+-619,	yysvec+3,	yyvstop+371,
yycrank+-672,	yysvec+3,	yyvstop+373,
yycrank+-716,	yysvec+3,	yyvstop+375,
yycrank+-786,	yysvec+3,	yyvstop+377,
yycrank+1053,	yysvec+49,	yyvstop+380,
yycrank+470,	yysvec+49,	yyvstop+382,
yycrank+475,	yysvec+49,	yyvstop+384,
yycrank+474,	yysvec+49,	yyvstop+386,
yycrank+765,	yysvec+49,	yyvstop+388,
yycrank+910,	yysvec+49,	yyvstop+390,
yycrank+-721,	yysvec+58,	yyvstop+393,
yycrank+-732,	yysvec+58,	yyvstop+395,
yycrank+-729,	yysvec+58,	yyvstop+397,
yycrank+-767,	yysvec+58,	yyvstop+399,
yycrank+-789,	yysvec+58,	yyvstop+401,
yycrank+-911,	yysvec+58,	yyvstop+403,
yycrank+-1019,	yysvec+69,	yyvstop+406,
yycrank+-1075,	yysvec+13,	yyvstop+408,
yycrank+-1102,	yysvec+13,	yyvstop+410,
yycrank+-1106,	yysvec+13,	yyvstop+412,
yycrank+-1109,	yysvec+13,	yyvstop+414,
yycrank+-1111,	yysvec+13,	yyvstop+416,
yycrank+-1115,	yysvec+13,	yyvstop+418,
yycrank+512,	yysvec+49,	yyvstop+421,
yycrank+539,	yysvec+49,	yyvstop+423,
yycrank+580,	yysvec+49,	yyvstop+425,
yycrank+643,	yysvec+49,	yyvstop+427,
yycrank+947,	yysvec+49,	yyvstop+429,
yycrank+739,	yysvec+198,	yyvstop+432,
yycrank+643,	0,		0,	
yycrank+657,	0,		0,	
yycrank+0,	0,		yyvstop+434,
yycrank+686,	yysvec+202,	0,	
yycrank+1042,	0,		0,	
yycrank+0,	0,		yyvstop+436,
yycrank+-802,	yysvec+3,	yyvstop+438,
yycrank+-796,	yysvec+3,	yyvstop+441,
yycrank+-792,	yysvec+3,	yyvstop+443,
yycrank+-790,	yysvec+3,	yyvstop+445,
yycrank+-899,	yysvec+3,	yyvstop+448,
yycrank+-922,	yysvec+3,	yyvstop+450,
yycrank+-795,	yysvec+3,	yyvstop+452,
yycrank+1126,	yysvec+49,	yyvstop+455,
yycrank+690,	yysvec+49,	yyvstop+458,
yycrank+706,	yysvec+49,	yyvstop+460,
yycrank+0,	yysvec+49,	yyvstop+462,
yycrank+-875,	yysvec+58,	yyvstop+465,
yycrank+-814,	yysvec+58,	yyvstop+468,
yycrank+-900,	yysvec+58,	yyvstop+470,
yycrank+-810,	yysvec+58,	yyvstop+472,
yycrank+-917,	yysvec+58,	yyvstop+475,
yycrank+-1068,	yysvec+58,	yyvstop+477,
yycrank+-811,	yysvec+58,	yyvstop+479,
yycrank+-1021,	yysvec+69,	0,	
yycrank+-1118,	yysvec+13,	yyvstop+482,
yycrank+-1120,	yysvec+13,	yyvstop+485,
yycrank+-1122,	yysvec+13,	yyvstop+487,
yycrank+-1125,	yysvec+13,	yyvstop+489,
yycrank+-1161,	yysvec+69,	0,	
yycrank+-1175,	yysvec+69,	0,	
yycrank+-904,	yysvec+69,	yyvstop+492,
yycrank+725,	yysvec+49,	yyvstop+494,
yycrank+0,	yysvec+49,	yyvstop+496,
yycrank+728,	yysvec+49,	yyvstop+499,
yycrank+0,	yysvec+49,	yyvstop+501,
yycrank+962,	0,		0,	
yycrank+-1185,	0,		yyvstop+504,
yycrank+769,	0,		0,	
yycrank+876,	0,		0,	
yycrank+-972,	yysvec+3,	yyvstop+506,
yycrank+-1187,	0,		yyvstop+508,
yycrank+-960,	yysvec+3,	yyvstop+511,
yycrank+-968,	yysvec+3,	yyvstop+513,
yycrank+1040,	yysvec+49,	yyvstop+515,
yycrank+-1158,	yysvec+272,	yyvstop+517,
yycrank+-977,	yysvec+58,	yyvstop+520,
yycrank+-1189,	0,		yyvstop+522,
yycrank+-1037,	yysvec+58,	yyvstop+525,
yycrank+-978,	yysvec+58,	yyvstop+527,
yycrank+-1179,	yysvec+69,	yyvstop+529,
yycrank+-1192,	yysvec+13,	yyvstop+531,
yycrank+-1241,	0,		yyvstop+533,
yycrank+-1183,	yysvec+69,	0,	
yycrank+-1197,	yysvec+69,	0,	
yycrank+805,	yysvec+49,	yyvstop+536,
yycrank+0,	yysvec+49,	yyvstop+538,
yycrank+820,	yysvec+271,	0,	
yycrank+-915,	yysvec+272,	0,	
yycrank+-919,	yysvec+272,	yyvstop+541,
yycrank+1048,	0,		0,	
yycrank+1050,	0,		yyvstop+543,
yycrank+-1044,	yysvec+3,	yyvstop+545,
yycrank+-927,	yysvec+276,	yyvstop+547,
yycrank+-929,	yysvec+276,	yyvstop+549,
yycrank+-1052,	yysvec+3,	yyvstop+552,
yycrank+-1130,	yysvec+3,	yyvstop+554,
yycrank+-1181,	yysvec+272,	yyvstop+557,
yycrank+-1215,	yysvec+272,	0,	
yycrank+-1142,	yysvec+58,	yyvstop+559,
yycrank+-965,	yysvec+282,	yyvstop+561,
yycrank+-973,	yysvec+282,	yyvstop+563,
yycrank+-1133,	yysvec+58,	yyvstop+566,
yycrank+-1194,	yysvec+58,	yyvstop+568,
yycrank+-1199,	yysvec+69,	0,	
yycrank+-1258,	0,		0,	
yycrank+-1201,	yysvec+287,	yyvstop+571,
yycrank+-1216,	yysvec+310,	0,	
yycrank+-1243,	yysvec+287,	0,	
yycrank+-1242,	yysvec+310,	yyvstop+573,
yycrank+-1255,	yysvec+69,	0,	
yycrank+-1261,	yysvec+69,	yyvstop+575,
yycrank+892,	yysvec+49,	yyvstop+577,
yycrank+928,	0,		0,	
yycrank+0,	yysvec+295,	yyvstop+579,
yycrank+-1055,	yysvec+3,	yyvstop+581,
yycrank+-1203,	yysvec+3,	yyvstop+583,
yycrank+-1058,	yysvec+58,	yyvstop+586,
yycrank+-1248,	yysvec+58,	yyvstop+588,
yycrank+-1259,	yysvec+69,	0,	
yycrank+-1263,	yysvec+310,	yyvstop+591,
yycrank+-1268,	yysvec+69,	yyvstop+593,
yycrank+942,	yysvec+49,	yyvstop+595,
yycrank+944,	0,		0,	
yycrank+-1200,	yysvec+3,	yyvstop+597,
yycrank+-1220,	yysvec+58,	yyvstop+599,
yycrank+-1266,	yysvec+69,	0,	
yycrank+-1276,	yysvec+310,	yyvstop+601,
yycrank+0,	yysvec+49,	yyvstop+603,
yycrank+954,	0,		0,	
yycrank+-1224,	yysvec+3,	yyvstop+606,
yycrank+-1236,	yysvec+58,	yyvstop+608,
yycrank+-1281,	yysvec+69,	0,	
yycrank+1002,	0,		0,	
yycrank+-1244,	yysvec+3,	yyvstop+610,
yycrank+-1250,	yysvec+58,	yyvstop+612,
yycrank+-1293,	yysvec+69,	0,	
yycrank+0,	0,		yyvstop+614,
yycrank+-1127,	yysvec+3,	yyvstop+616,
yycrank+-1134,	yysvec+58,	yyvstop+619,
yycrank+-1294,	yysvec+69,	yyvstop+622,
0,	0,	0};
struct yywork *yytop = yycrank+1408;
struct yysvf *yybgin = yysvec+1;
char yymatch[] ={
00  ,01  ,01  ,01  ,01  ,01  ,01  ,01  ,
01  ,011 ,012 ,01  ,01  ,01  ,01  ,01  ,
01  ,01  ,01  ,01  ,01  ,01  ,01  ,01  ,
01  ,01  ,01  ,01  ,01  ,01  ,01  ,01  ,
011 ,01  ,'"' ,01  ,'"' ,01  ,01  ,01  ,
01  ,01  ,01  ,01  ,'"' ,'"' ,'"' ,01  ,
'0' ,'0' ,'0' ,'0' ,'0' ,'0' ,'0' ,'0' ,
'0' ,'0' ,':' ,'"' ,01  ,01  ,01  ,01  ,
01  ,'"' ,'"' ,'"' ,'"' ,'"' ,'"' ,'"' ,
'"' ,'"' ,'"' ,'"' ,'"' ,'"' ,'"' ,'"' ,
'"' ,'"' ,'"' ,'"' ,'"' ,'"' ,'"' ,'"' ,
'"' ,'"' ,'"' ,01  ,01  ,01  ,01  ,'"' ,
01  ,'"' ,'b' ,'"' ,'"' ,'"' ,'"' ,'"' ,
'b' ,'"' ,'"' ,'"' ,'"' ,'"' ,'"' ,'"' ,
'"' ,'"' ,'b' ,'b' ,'"' ,'"' ,'"' ,'"' ,
'"' ,'"' ,'"' ,01  ,01  ,01  ,01  ,01  ,
01  ,01  ,01  ,01  ,01  ,01  ,01  ,01  ,
01  ,01  ,01  ,01  ,01  ,01  ,01  ,01  ,
01  ,01  ,01  ,01  ,01  ,01  ,01  ,01  ,
01  ,01  ,01  ,01  ,01  ,01  ,01  ,01  ,
01  ,01  ,01  ,01  ,01  ,01  ,01  ,01  ,
01  ,01  ,01  ,01  ,01  ,01  ,01  ,01  ,
01  ,01  ,01  ,01  ,01  ,01  ,01  ,01  ,
01  ,01  ,01  ,01  ,01  ,01  ,01  ,01  ,
01  ,01  ,01  ,01  ,01  ,01  ,01  ,01  ,
01  ,01  ,01  ,01  ,01  ,01  ,01  ,01  ,
01  ,01  ,01  ,01  ,01  ,01  ,01  ,01  ,
01  ,01  ,01  ,01  ,01  ,01  ,01  ,01  ,
01  ,01  ,01  ,01  ,01  ,01  ,01  ,01  ,
01  ,01  ,01  ,01  ,01  ,01  ,01  ,01  ,
01  ,01  ,01  ,01  ,01  ,01  ,01  ,01  ,
01  ,01  ,01  ,01  ,01  ,01  ,01  ,01  ,
0};
char yyextra[] ={
0,0,0,0,0,1,0,0,
0,1,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,
0};
/*	ncform	4.1	83/08/11	*/

int yylineno =1;
# define YYU(x) x
# define NLSTATE yyprevious=YYNEWLINE
char yytext[YYLMAX];
struct yysvf *yylstate [YYLMAX], **yylsp, **yyolsp;
char yysbuf[YYLMAX];
char *yysptr = yysbuf;
int *yyfnd;
extern struct yysvf *yyestate;
int yyprevious = YYNEWLINE;
yylook(){
	register struct yysvf *yystate, **lsp;
	register struct yywork *yyt;
	struct yysvf *yyz;
	int yych;
	struct yywork *yyr;
# ifdef LEXDEBUG
	int debug;
# endif
	char *yylastch;
	/* start off machines */
# ifdef LEXDEBUG
	debug = 0;
# endif
	if (!yymorfg)
		yylastch = yytext;
	else {
		yymorfg=0;
		yylastch = yytext+yyleng;
		}
	for(;;){
		lsp = yylstate;
		yyestate = yystate = yybgin;
		if (yyprevious==YYNEWLINE) yystate++;
		for (;;){
# ifdef LEXDEBUG
			if(debug)fprintf(yyout,"state %d\n",yystate-yysvec-1);
# endif
			yyt = yystate->yystoff;
			if(yyt == yycrank){		/* may not be any transitions */
				yyz = yystate->yyother;
				if(yyz == 0)break;
				if(yyz->yystoff == yycrank)break;
				}
			*yylastch++ = yych = input();
		tryagain:
# ifdef LEXDEBUG
			if(debug){
				fprintf(yyout,"unsigned char ");
				allprint(yych);
				putchar('\n');
				}
# endif
			yyr = yyt;
			if ( (int)yyt > (int)yycrank){
				yyt = yyr + yych;
				if (yyt <= yytop && yyt->verify+yysvec == yystate){
					if(yyt->advance+yysvec == YYLERR)	/* error transitions */
						{unput(*--yylastch);break;}
					*lsp++ = yystate = yyt->advance+yysvec;
					goto contin;
					}
				}
# ifdef YYOPTIM
			else if((int)yyt < (int)yycrank) {		/* r < yycrank */
				yyt = yyr = yycrank+(yycrank-yyt);
# ifdef LEXDEBUG
				if(debug)fprintf(yyout,"compressed state\n");
# endif
				yyt = yyt + yych;
				if(yyt <= yytop && yyt->verify+yysvec == yystate){
					if(yyt->advance+yysvec == YYLERR)	/* error transitions */
						{unput(*--yylastch);break;}
					*lsp++ = yystate = yyt->advance+yysvec;
					goto contin;
					}
				yyt = yyr + YYU(yymatch[yych]);
# ifdef LEXDEBUG
				if(debug){
					fprintf(yyout,"try fall back character ");
					allprint(YYU(yymatch[yych]));
					putchar('\n');
					}
# endif
				if(yyt <= yytop && yyt->verify+yysvec == yystate){
					if(yyt->advance+yysvec == YYLERR)	/* error transition */
						{unput(*--yylastch);break;}
					*lsp++ = yystate = yyt->advance+yysvec;
					goto contin;
					}
				}
			if ((yystate = yystate->yyother) && (yyt= yystate->yystoff) != yycrank){
# ifdef LEXDEBUG
				if(debug)fprintf(yyout,"fall back to state %d\n",yystate-yysvec-1);
# endif
				goto tryagain;
				}
# endif
			else
				{unput(*--yylastch);break;}
		contin:
# ifdef LEXDEBUG
			if(debug){
				fprintf(yyout,"state %d char ",yystate-yysvec-1);
				allprint(yych);
				putchar('\n');
				}
# endif
			;
			}
# ifdef LEXDEBUG
		if(debug){
			fprintf(yyout,"stopped at %d with ",*(lsp-1)-yysvec-1);
			allprint(yych);
			putchar('\n');
			}
# endif
		while (lsp-- > yylstate){
			*yylastch-- = 0;
			if (*lsp != 0 && (yyfnd= (*lsp)->yystops) && *yyfnd > 0){
				yyolsp = lsp;
				if(yyextra[*yyfnd]){		/* must backup */
					while(yyback((*lsp)->yystops,-*yyfnd) != 1 && lsp > yylstate){
						lsp--;
						unput(*yylastch--);
						}
					}
				yyprevious = YYU(*yylastch);
				yylsp = lsp;
				yyleng = yylastch-yytext+1;
				yytext[yyleng] = 0;
# ifdef LEXDEBUG
				if(debug){
					fprintf(yyout,"\nmatch ");
					sprint(yytext);
					fprintf(yyout," action %d\n",*yyfnd);
					}
# endif
				return(*yyfnd++);
				}
			unput(*yylastch);
			}
		if (yytext[0] == 0  /* && feof(yyin) */)
			{
			yysptr=yysbuf;
			return(0);
			}
		yyprevious = yytext[0] = input();
		if (yyprevious>0)
			output(yyprevious);
		yylastch=yytext;
# ifdef LEXDEBUG
		if(debug)putchar('\n');
# endif
		}
	}
yyback(p, m)
	int *p;
{
if (p==0) return(0);
while (*p)
	{
	if (*p++ == m)
		return(1);
	}
return(0);
}
	/* the following are only used in the lex library */
yyinput(){
	return(input());
	}
yyoutput(c)
  int c; {
	output(c);
	}
yyunput(c)
   int c; {
	unput(c);
	}
