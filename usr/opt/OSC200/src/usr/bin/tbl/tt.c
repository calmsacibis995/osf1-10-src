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
static char rcsid[] = "@(#)$RCSfile: tt.c,v $ $Revision: 4.2.6.2 $ (DEC) $Date: 1993/06/10 18:44:31 $";
#endif
/*
 * HISTORY
 */
/*
static	char	*sccsid = "@(#)tt.c	8.1	(Japanese ULTRIX)  2/19/91";
static char sccsid[] = "@(#)tt.c	4.2 8/11/83";
*/

 /* tt.c: subroutines for drawing horizontal lines */
# include "t..c"
ctype(il, ic)
{
	if (instead[il])
		return(0);
	if (fullbot[il])
		return(0);
	il = stynum[il];
	return(style[il][ic]);
}
min(a,b)
{
	return(a<b ? a : b);
}
fspan(i,c)
{
	c++;
	return(c<ncol && ctype(i,c)=='s');
}
lspan(i,c)
{
	int k;
	if (ctype(i,c) != 's') return(0);
	c++;
	if (c < ncol && ctype(i,c)== 's') 
		return(0);
	for(k=0; ctype(i,--c) == 's'; k++);
	return(k);
}
ctspan(i,c)
{
	int k;
	c++;
	for(k=1; c<ncol && ctype(i,c)=='s'; k++)
		c++;
	return(k);
}
tohcol(ic)
{
	if (ic==0)
		fprintf(tabout, "\\h'|0'");
	else
		fprintf(tabout, "\\h'(|\\n(%du+|\\n(%du)/2u'", ic+CLEFT, ic+CRIGHT-1);
}
allh(i)
{
	/* return true if every element in line i is horizontal */
	/* also at least one must be horizontl */
	int c, one, k;
	if (fullbot[i]) return(1);
	for(one=c=0; c<ncol; c++)
	{
		k = thish(i,c);
		if (k==0) return(0);
		if (k==1) continue;
		one=1;
	}
	return(one);
}
thish(i,c)
{
	int t;
	wchar_t *s;
	struct colstr *pc;
	if (c<0)return(0);
	if (i<0) return(0);
	t = ctype(i,c);
	if (t=='_' || t == '-')
		return('-');
	if (t=='=')return('=');
	if (t=='^') return(1);
	if (fullbot[i] )
		return(fullbot[i]);
	if (t=='s') return(thish(i,c-1));
	if (t==0) return(1);
	pc = &table[i][c];
	s = (t=='a' ? pc->rcol : pc->col);
	if (s==0 || (point(s) && *s==0))
		return(1);
	if (vspen(s)) return(1);
	if (t=barent( s))
		return(t);
	return(0);
}
