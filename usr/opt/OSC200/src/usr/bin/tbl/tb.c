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
static char rcsid[] = "@(#)$RCSfile: tb.c,v $ $Revision: 4.2.6.2 $ (DEC) $Date: 1993/06/10 18:17:52 $";
#endif
/*
 * HISTORY
 */
/*
static	char	*sccsid = "@(#)tb.c	8.1	(Japanese ULTRIX)  2/19/91";
static char sccsid[] = "@(#)tb.c	4.2 8/11/83";
*/

 /* tb.c: check which entries exist, also storage allocation */
# include "t..c"
checkuse()
{
	int i,c, k;
	for(c=0; c<ncol; c++)
	{
		used[c]=lused[c]=rused[c]=0;
		for(i=0; i<nlin; i++)
		{
			if (instead[i] || fullbot[i]) continue;
			k = ctype(i,c);
			if (k== '-' || k == '=') continue;
			if ((k=='n'||k=='a'))
			{
				rused[c]|= real(table[i][c].rcol);
				if( !real(table[i][c].rcol))
					used[c] |= real(table[i][c].col);
				if (table[i][c].rcol)
					lused[c] |= real(table[i][c].col);
			}
			else
				used[c] |= real(table[i][c].col);
		}
	}
}
real(s)
wchar_t *s;
{
	if (s==0) return(0);
	if (!point(s)) return(1);
	if (*s==0) return(0);
	return(1);
}
int spcount = 0;
/*
extern char * calloc();
*/
# define MAXVEC 20
wchar_t *spvecs[MAXVEC];

wchar_t *chspace()
{
	wchar_t *pp;

	if (spvecs[spcount])
		return(spvecs[spcount++]);
	if (spcount>=MAXVEC)
		error(catgets(catd, 1, 23, "Too many characters in table"));
	spvecs[spcount++]= pp = (wchar_t *)calloc(MAXCHS+200,sizeof(wchar_t));
	if ((pp== (wchar_t *)-1) || (pp == (wchar_t *)0))
		error(catgets(catd, 1, 24, "no space for characters"));
	return(pp);
}
# define MAXPC 50
char *thisvec;
int tpcount = -1;
char *tpvecs[MAXPC];
char	*alocv(n)
{
	char *tp, *q;
	if (tpcount<0 || thisvec+n > tpvecs[tpcount]+MAXCHS)
	{
		tpcount++;
		if (tpvecs[tpcount]==0)
		{
			tpvecs[tpcount] = calloc(MAXCHS,1);
		}
		thisvec = tpvecs[tpcount];
		if (thisvec == (char *)-1)
			error(catgets(catd, 1, 25, "no space for vectors"));
	}
	tp=thisvec;
	thisvec+=n;
	for(q=tp; q<(char *)thisvec; q++)
		*q=0;
	return(tp);
}
release()
{
	extern wchar_t *exstore;
/* give back unwanted space in some vectors */
	spcount=0;
	tpcount= -1;
	exstore=0;
}
