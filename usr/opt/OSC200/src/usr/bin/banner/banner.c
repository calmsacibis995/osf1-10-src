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
static char	*sccsid = "@(#)$RCSfile: banner.c,v $ $Revision: 4.2.6.2 $ (DEC) $Date: 1993/10/11 15:36:31 $";
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
 * COMPONENT_NAME: (CMDFILES) commands that manipulate files
 *
 * FUNCTIONS: banner
 *
 * ORIGINS: 3, 26, 27
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
 */
/*
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */
/*
 * 	 Writes character strings in large letters to standard output.
 */

#include <locale.h>
#include "banner_msg.h"

nl_catd	catd;
#define MSGSTR(Num, Str) catgets(catd, MS_BANNER, Num, Str)

#include <NLchar.h>

#define NCHARS 128	/*number of chars in char set*/
#define NLINES  7	/*number of lines in a banner character*/
#define PPOSS  85	/*number of print positions on a line (must be multiple of 4)*/
			/*followed by end of string character*/
#define POSPCH 8	/*number of char positions per banner char*/
#define CHPLN  10	/*number of banner characters per line*/

struct bann{
	  char alphas[NLINES][PPOSS];
};
struct bann buffer, *bp = &buffer;
/*
 * banner characters are plotted using the octal numbers that are in the following table.
 * Each octal number is expanded to a binary number, the plot character is placed
 * according to the ones.
 * Example "A"  (010,024,042,0101,0177,0101,0101) each octal number represents one 
 * row of the banner character.
 *  010 expands to 000 001 000 which translates to (read the numbers from right to left)
 * "   #    " three blanks, plot character and blanks to fill out the banner character.
 */
char ctbl[NCHARS][NLINES] = {
	0,000,000,000,000,000,000,	/*below 040*/
	0,000,000,000,000,000,000,	/*below 040*/
	0,000,000,000,000,000,000,	/*below 040*/
	0,000,000,000,000,000,000,	/*below 040*/
	0,000,000,000,000,000,000,	/*below 040*/
	0,000,000,000,000,000,000,	/*below 040*/
	0,000,000,000,000,000,000,	/*below 040*/
	0,000,000,000,000,000,000,	/*below 040*/
	0,000,000,000,000,000,000,	/*below 040*/
	0,000,000,000,000,000,000,	/*below 040*/
	0,000,000,000,000,000,000,	/*below 040*/
	0,000,000,000,000,000,000,	/*below 040*/
	0,000,000,000,000,000,000,	/*below 040*/
	0,000,000,000,000,000,000,	/*below 040*/
	0,000,000,000,000,000,000,	/*below 040*/
	0,000,000,000,000,000,000,	/*below 040*/
	0,000,000,000,000,000,000,	/*below 040*/
	0,000,000,000,000,000,000,	/*below 040*/
	0,000,000,000,000,000,000,	/*below 040*/
	0,000,000,000,000,000,000,	/*below 040*/
	0,000,000,000,000,000,000,	/*below 040*/
	0,000,000,000,000,000,000,	/*below 040*/
	0,000,000,000,000,000,000,	/*below 040*/
	0,000,000,000,000,000,000,	/*below 040*/
	0,000,000,000,000,000,000,	/*below 040*/
	0,000,000,000,000,000,000,	/*below 040*/
	0,000,000,000,000,000,000,	/*below 040*/
	0,000,000,000,000,000,000,	/*below 040*/
	0,000,000,000,000,000,000,	/*below 040*/
	0,000,000,000,000,000,000,	/*below 040*/
	0,000,000,000,000,000,000,	/*below 040*/
	0,000,000,000,000,000,000,	/*below 040*/
	0,000,000,000,000,000,000,	/* */
	034,034,034,010,0,034,034,	/*!*/
	0167,0167,042,0,0,0,0,	/*"*/
	024,024,0177,024,0177,024,024,	/*#*/
	076,0111,0110,076,011,0111,076,	/*$*/
	0161,0122,0164,010,027,045,0107,	/*%*/
	030,044,030,070,0105,0102,071,	/*&*/
	034,034,010,020,0,0,0,	/*'*/
	014,020,040,040,040,020,014,	/*(*/
	030,4,2,2,2,4,030,	/*)*/
	0,042,024,0177,024,042,0,	/***/
	0,010,010,076,010,010,0,	/*+*/
	0,0,0,034,034,010,020,	/*,*/
	0,0,0,076,0,0,0,	/*-*/
	0,0,0,0,034,034,034,	/*.*/
	1,2,4,010,020,040,0100,	/* / */
	034,042,0101,0101,0101,042,034,	/*0*/
	010,030,050,010,010,010,076,	/*1*/
	076,0101,1,076,0100,0100,0177,	/*2*/
	076,0101,1,076,1,0101,076,	/*3*/
	0100,0102,0102,0102,0177,2,2,	/*4*/
	0177,0100,0100,0176,1,0101,076,	/*5*/
	076,0101,0100,0176,0101,0101,076,	/*6*/
	0177,0102,04,010,020,020,020,	/*7*/
	076,0101,0101,076,0101,0101,076,	/*8*/
	076,0101,0101,077,1,0101,076,	/*9*/
	010,034,010,0,010,034,010,	/*:*/
	034,034,0,034,034,010,020,	/*;*/
	4,010,020,040,020,010,4,	/*<*/
	0,0,076,0,076,0,0,	/*=*/
	020,010,4,2,4,010,020,	/*>*/
	076,0101,1,016,010,0,010,	/*?*/
	076,0101,0135,0135,0136,0100,076,	/*@*/
	010,024,042,0101,0177,0101,0101,	/*A*/
	0176,0101,0101,0176,0101,0101,0176,	/*B*/
	076,0101,0100,0100,0100,0101,076,	/*C*/
	0176,0101,0101,0101,0101,0101,0176,	/*D*/
	0177,0100,0100,0174,0100,0100,0177,	/*E*/
	0177,0100,0100,0174,0100,0100,0100,	/*F*/
	076,0101,0100,0117,0101,0101,076,	/*G*/
	0101,0101,0101,0177,0101,0101,0101,	/*H*/
	034,010,010,010,010,010,034,	/*I*/
	1,1,1,1,0101,0101,076,	/*J*/
	0102,0104,0110,0160,0110,0104,0102,	/*K*/
	0100,0100,0100,0100,0100,0100,0177,	/*L*/
	0101,0143,0125,0111,0101,0101,0101,	/*M*/
	0101,0141,0121,0111,0105,0103,0101,	/*N*/
	0177,0101,0101,0101,0101,0101,0177,	/*O*/
	0176,0101,0101,0176,0100,0100,0100,	/*P*/
	076,0101,0101,0101,0105,0102,075,	/*Q*/
	0176,0101,0101,0176,0104,0102,0101,	/*R*/
	076,0101,0100,076,1,0101,076,	/*S*/
	0177,010,010,010,010,010,010,	/*T*/
	0101,0101,0101,0101,0101,0101,076,	/*U*/
	0101,0101,0101,0101,042,024,010,	/*V*/
	0101,0111,0111,0111,0111,0111,066,	/*W*/
	0101,042,024,010,024,042,0101,	/*X*/
	0101,042,024,010,010,010,010,	/*Y*/
	0177,2,4,010,020,040,0177,	/*Z*/
	076,040,040,040,040,040,076,/*[*/
	0100,040,020,010,004,002,001,	/*\*/
	076,2,2,2,2,2,076,	/*]*/
	010,024,042,0,0,0,0,	/*^*/
	0,000,000,000,000,000,0177,	/*_*/
	034,034,010,04,0,0,0,	/*`*/
	0,014,022,041,077,041,041,	/*A*/
	0,076,041,076,041,041,076,	/*B*/
	0,036,041,040,040,041,036,	/*C*/
	0,076,041,041,041,041,076,	/*D*/
	0,077,040,076,040,040,077,	/*E*/
	0,077,040,076,040,040,040,	/*F*/
	0,036,041,040,047,041,036,	/*G*/
	0,041,041,077,041,041,041,	/*H*/
	0,004,004,004,004,004,004,	/*I*/
	0,001,001,001,001,041,036,	/*J*/
	0,041,042,074,044,042,041,	/*K*/
	0,040,040,040,040,040,077,	/*L*/
	0,041,063,055,041,041,041,	/*M*/
	0,041,061,051,045,043,041,	/*N*/
	0,036,041,041,041,041,036,	/*O*/
	0,076,041,041,076,040,040,	/*P*/
	0,036,041,041,045,042,035,	/*Q*/
	0,076,041,041,076,042,041,	/*R*/
	0,036,040,036,001,041,036,	/*S*/
	0,037,004,004,004,004,004,	/*T*/
	0,041,041,041,041,041,036,	/*U*/
	0,041,041,041,041,022,014,	/*V*/
	0,041,041,041,055,063,041,	/*W*/
	0,041,022,014,014,022,041,	/*X*/
	0,021,012,004,004,004,004,	/*Y*/
	0,077,002,004,010,020,077,	/*Z*/
	034,040,040,0140,040,040,034,	/*{*/
	010,010,010,0,010,010,010,	/*|*/
	034,2,2,3,2,2,034,	/*}*/
	060,0111,06,0,0,0,0,	/*~*/
	0,000,000,000,000,000,000 	/*DEL*/
};
char blank = ' ';
char plot = '#';
int  msk = 0100; /* ? */

/*
 * NAME: main
 * FUNCTION: display a character string banner characters
 */
main(argc,argp)
char **argp;int argc;
{
	int i;

	(void) setlocale (LC_ALL,"");

	/*if invoked with no arguments, prints error comment;
	  if invoked with an argument, prints it in banner form.
	*/
	catd = catopen(MF_BANNER, NL_CAT_LOCALE);
	if(argc<2){
		printf(MSGSTR(USAGE,
                    "Usage: banner \"up to 10 characters per string\" . . .\n"));
		exit(1);
	}
	for(i = 1; i<argc ; ++i) {
		banner(argp[i],bp);
		banprt(bp);
		printf("\n");
	}
	exit(0);
}

/*
 * NAME: banner 
 * FUNCTION:  gets the translation for each characer and then fill the buffer.
 */
banner(s,bufp)
char *s;struct bann *bufp;
{
	char c,*p,*q;
	struct bann *r;
	p=s;
	r=bufp;
	banset(blank,bufp);

	while( (c= *s++) ) {
	  if((s-p)>CHPLN)return(s-p);
	  if (NLisNLcp(s-1))
#ifdef KJI
	  {
		printf(MSGSTR(KJISTR,
			" Banner does not support Kanji characters\n"));
		exit(1);
	  }
#else
	  	q=ctbl[NCflatchr(NCtoupper(c))];
#endif
	  else {
	    	q=ctbl[c];
	  	banfil(q,r);
	  	r = (struct bann *)((char *)r + POSPCH);
	  }
	}
}

/*
 * NAME: banfil
 * FUNCTION: put the banner character into the buffer
 */
banfil(c,p)
char *c;
struct bann *p;
{
	int i,j;
	for(i=0;i<NLINES;i++){
	  for(j=0;j<POSPCH;j++){
	    if(((c[i]<<j)&msk)!=0)p->alphas[i][j] = plot;
	  }
	}
	return(0);
}

/*
 * NAME: banset
 * FUNCTION: blank out the buffer
 */
banset(c,p)
char c;
struct bann *p;
{
	int i,j;
	for(i=0;i<NLINES;i++)
	  for(j=0;j<PPOSS-1;j++)
	    p->alphas[i][j] = c;
}

/*
 * NAME: banprt
 * FUNCTION: print out buffer
 */
banprt(ptr)
struct bann *ptr;
{
	int i,j;
	for(i=0;i<NLINES;i++){
	  ptr->alphas[i][PPOSS-1]='\0';
	  for(j=PPOSS-2;j>=0;j--){
	    if(ptr->alphas[i][j]!=blank)break;
	    ptr->alphas[i][j]='\0';
	  }
	printf("%s\n",ptr->alphas[i]);
	}
}
