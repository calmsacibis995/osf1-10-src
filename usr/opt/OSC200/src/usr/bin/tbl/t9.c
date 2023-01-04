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
#if !defined(lint) && !defined(_NOIDENT)
static char rcsid[] = "@(#)$RCSfile: t9.c,v $ $Revision: 4.2.6.2 $ (DEC) $Date: 1993/06/10 18:15:08 $";
#endif
/*
 * HISTORY
 */
/*
static	char	*sccsid = "@(#)t9.c	8.1	(Japanese ULTRIX)  2/19/91";
static char sccsid[] = "@(#)t9.c	4.2 8/11/83";
*/

 /* t9.c: write lines for tables over 200 lines */
# include "t..c"
static useln;
yetmore()
{
	for(useln=0; useln<MAXLIN && table[useln]==0; useln++);
	if (useln>=MAXLIN)
		error(catgets(catd, 1, 21, "Wierd.  No data in table."));
	table[0]=table[useln];
	for(useln=nlin-1; useln>=0 && (fullbot[useln] || instead[useln]); useln--);
	if (useln<0)
		error(catgets(catd, 1, 22, "Wierd.  No real lines in table."));
	domore(leftover);
	while (wgets1(cstore=cspace) && domore(cstore))
		;
	XFREE(last);
	last =Tombs(cstore);
	return;
}
domore(dataln)
wchar_t *dataln;
{
	int icol, ch;
	char *cp;

	if (wprefix(WCS_dTE, dataln))
		return(0);
	if (dataln[0] == '.' && !isdigit((int)dataln[1]))
	{
		puts(cp = Tombs(dataln));
		XFREE(cp);
		return(1);
	}
	instead[0]=0;
	fullbot[0]=0;
	if (dataln[1]==0)
		switch(dataln[0])
		{
		case '_': 
			fullbot[0]= '-'; 
			putline(useln,0);  
			return(1);
		case '=': 
			fullbot[0]= '='; 
			putline(useln, 0); 
			return(1);
		}
	for (icol = 0; icol <ncol; icol++)
	{
		table[0][icol].col = dataln;
		table[0][icol].rcol=0;
		for(; (ch= *dataln) != '\0' && ch != tab; dataln++)
			;
		*dataln++ = '\0';
		switch(ctype(useln,icol))
		{
		case 'n':
			table[0][icol].rcol = maknew(table[0][icol].col);
			break;
		case 'a':
			table[0][icol].rcol = table[0][icol].col;
			table[0][icol].col= WNULLp;
			break;
		}
		while (ctype(useln,icol+1)== 's') /* spanning */
			table[0][++icol].col = WNULLp;
		if (ch == '\0') break;
	}
	while (++icol <ncol)
		table[0][icol].col = WNULLp;
	putline(useln,0);
	return(1);
}
