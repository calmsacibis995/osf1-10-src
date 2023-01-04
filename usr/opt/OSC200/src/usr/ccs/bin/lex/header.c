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
static char	*sccsid = "@(#)$RCSfile: header.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 02:07:30 $";
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
#if !defined(lint) && !defined(_NOIDENT)

#endif
/*
 * COMPONENT_NAME: (CMDLANG) Language Utilities
 *
 * FUNCTIONS: chd1, chd2, ctail, phead1, phead2, ptail, rhd1, rtail, statistics
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 * OBJECT CODE ONLY SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * header.c	1.4  com/cmd/lang/lex,3.1,9021 9/7/89 18:35:57"; 
 */

# include "ldefs.h"

#ifndef _BLD
#include "lex_msg.h"
#define MSGSTR(Num,Str) catgets(catd,MS_LEX,Num,Str)
nl_catd catd;
#else
#define MSGSTR(Num,Str) Str
#endif

phead1(){
	ratfor ? rhd1() : chd1();
	}

chd1(){
	fprintf(fout,"# include \"stdio.h\"\n");
	if (ZCH>128)						/* ASSUMES 8-bit bytes */
	fprintf(fout, "# define U(x) ((x)&0377)\n");
	else
	fprintf(fout, "# define U(x) x\n");
	fprintf(fout, "# define NLSTATE yyprevious=YYNEWLINE\n");
	fprintf(fout,"# define BEGIN yybgin = yysvec + 1 +\n");
	fprintf(fout,"# define INITIAL 0\n");
	fprintf(fout,"# define YYLERR yysvec\n");
	fprintf(fout,"# define YYSTATE (yyestate-yysvec-1)\n");
	if(optim)
		fprintf(fout,"# define YYOPTIM 1\n");
# ifdef DEBUG
	fprintf(fout,"# define LEXDEBUG 1\n");
# endif
	fprintf(fout,"# define YYLMAX 200\n");
	fprintf(fout,"# define output(c) putc(c,yyout)\n");
	fprintf(fout, "%s%d%s\n",
  "# define input() (((yytchar=yysptr>yysbuf?U(*--yysptr):getc(yyin))==",
		ctable['\n'],
		"?(yylineno++,yytchar):yytchar)==EOF?0:yytchar)");
	fprintf(fout,
		"# define unput(c) {yytchar= (c);if(yytchar=='\\n')yylineno--;*yysptr++=yytchar;}\n");
	fprintf(fout,"# define yymore() (yymorfg=1)\n");
	fprintf(fout,"# define ECHO fprintf(yyout, \"%%s\",yytext)\n");
	fprintf(fout,"# define REJECT { nstr = yyreject(); goto yyfussy;}\n");
	fprintf(fout,"int yyleng; extern unsigned char yytext[];\n");
	fprintf(fout,"int yymorfg;\n");
	fprintf(fout,"extern unsigned char *yysptr, yysbuf[];\n");
	fprintf(fout,"int yytchar;\n");
	fprintf(fout,"FILE *yyin = {stdin}, *yyout = {stdout};\n");
	fprintf(fout,"extern int yylineno;\n");
	fprintf(fout,"struct yysvf { \n");
	fprintf(fout,"\tstruct yywork *yystoff;\n");
	fprintf(fout,"\tstruct yysvf *yyother;\n");
	fprintf(fout,"\tint *yystops;};\n");
	fprintf(fout,"struct yysvf *yyestate;\n");
	fprintf(fout,"extern struct yysvf yysvec[], *yybgin;\n");
	}

rhd1(){
	fprintf(fout,"integer function yylex(dummy)\n");
	fprintf(fout,"define YYLMAX 200\n");
	fprintf(fout,"define ECHO call yyecho(yytext,yyleng)\n");
	fprintf(fout,"define REJECT nstr = yyrjct(yytext,yyleng);goto 30998\n");
	fprintf(fout,"integer nstr,yylook,yywrap\n");
	fprintf(fout,"integer yyleng, yytext(YYLMAX)\n");
	fprintf(fout,"common /yyxel/ yyleng, yytext\n");
	fprintf(fout,"common /yyldat/ yyfnd, yymorf, yyprev, yybgin, yylsp, yylsta\n");
	fprintf(fout,"integer yyfnd, yymorf, yyprev, yybgin, yylsp, yylsta(YYLMAX)\n");
	fprintf(fout,"for(;;){\n");
	fprintf(fout,"\t30999 nstr = yylook(dummy)\n");
	fprintf(fout,"\tgoto 30998\n");
	fprintf(fout,"\t30000 k = yywrap(dummy)\n");
	fprintf(fout,"\tif(k .ne. 0){\n");
	fprintf(fout,"\tyylex=0; return; }\n");
	fprintf(fout,"\t\telse goto 30998\n");
	}

phead2(){
	if(!ratfor)chd2();
	}

chd2(){
	fprintf(fout,"while((nstr = yylook()) >= 0)\n");
	fprintf(fout,"yyfussy: switch(nstr){\n");
	fprintf(fout,"case 0:\n");
	fprintf(fout,"if(yywrap()) return(0); break;\n");
	}

ptail(){
	if(!pflag)
		ratfor ? rtail() : ctail();
	pflag = 1;
	}

ctail(){
	fprintf(fout,"case -1:\nbreak;\n");		/* for reject */
	fprintf(fout,"default:\n");
	fprintf(fout,"fprintf(yyout,\"bad switch yylook %%d\",nstr);\n");
	fprintf(fout,"} return(0); }\n");
	fprintf(fout,"/* end of yylex */\n");
	}

rtail(){
	register int i;
	fprintf(fout,"\n30998 if(nstr .lt. 0 .or. nstr .gt. %d)goto 30999\n",casecount);
	fprintf(fout,"nstr = nstr + 1\n");
	fprintf(fout,"goto(\n");
	for(i=0; i<casecount; i++)
		fprintf(fout,"%d,\n",30000+i);
	fprintf(fout,"30999),nstr\n");
	fprintf(fout,"30997 continue\n");
	fprintf(fout,"}\nend\n");
	}
statistics(){
	fprintf(errorf, MSGSTR(STATISTIC,"%d/%d nodes(%%e), %d/%d positions(%%p), %d/%d (%%n), %ld transitions\n"),
		tptr, treesize, nxtpos-positions, maxpos, stnum+1, nstates,
		rcount);
	fprintf(errorf, MSGSTR(STATISTIC2,
		", %d/%d packed char classes(%%k)"), pcptr-pchar, pchlen);
	if(optim)fprintf(errorf, MSGSTR(STATISTIC3,
		", %d/%d packed transitions(%%a)"),nptr, ntrans);
	fprintf(errorf, MSGSTR(STATISTIC4,
		", %d/%d output slots(%%o)"), yytop, outsize);
	putc('\n',errorf);
	}
