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
static char rcsid[] = "@(#)$RCSfile: t5.c,v $ $Revision: 4.2.6.2 $ (DEC) $Date: 1993/06/10 18:01:05 $";
#endif
/*
 * HISTORY
 */
/*
static	char	*sccsid = "@(#)t5.c	8.1	(Japanese ULTRIX)  2/19/91";
static char sccsid[] = "@(#)t5.c	4.2 8/11/83";
*/

 /* t5.c: read data for table */
# include "t..c"
gettbl()
{
	int icol, ch;
	cstore=cspace= chspace();
	textflg=0;
	for (nlin=nslin=0; wgets1(cstore); nlin++)
	{
		stynum[nlin]=nslin;
		if (wprefix(WCS_dTE, cstore))
		{
			leftover=0;
			break;
		}
		if (wprefix(WCS_dTC, cstore) || wprefix(WCS_dTand, cstore))
		{
			readspec();
			nslin++;
		}
		if (nlin>=MAXLIN)
		{
			leftover=(wchar_t *)cstore;
			break;
		}
		fullbot[nlin]=0;
		if (cstore[0] == '.' && !isdigit((int)cstore[1]))
		{
			instead[nlin] = cstore;
			while (*cstore++);
			continue;
		}
		else instead[nlin] = 0;
		if (nodata(nlin))
		{
			if (ch = oneh(nlin))
				fullbot[nlin]= ch;
			nlin++;
			nslin++;
			instead[nlin]=0;
			fullbot[nlin]=0;
		}
		table[nlin] = (struct colstr *)alocv((ncol+2)*sizeof(table[0][0]));
		if (cstore[1]==0)
			switch(cstore[0])
			{
			case '_': 
				fullbot[nlin]= '-'; 
				continue;
			case '=': 
				fullbot[nlin]= '='; 
				continue;
			}
		stynum[nlin] = nslin;
		nslin = min(nslin+1, nclin-1);
		for (icol = 0; icol <ncol; icol++)
		{
			table[nlin][icol].col = cstore;
			table[nlin][icol].rcol=0;
			ch=1;
			if (wmatch(cstore, WCS_Tlbrace)) /* text follows */
				table[nlin][icol].col =
					(wchar_t *)gettext(cstore, nlin, icol,
						font[stynum[nlin]][icol],
						csize[stynum[nlin]][icol]);
			else
			{
				for(; (ch= *cstore) != '\0' && ch != tab; cstore++)
					;
				*cstore++ = '\0';
				switch(ctype(nlin,icol)) /* numerical or alpha, subcol */
				{
				case 'n':
					table[nlin][icol].rcol =
						maknew(table[nlin][icol].col);
					break;
				case 'a':
					table[nlin][icol].rcol = table[nlin][icol].col;
					table[nlin][icol].col = WNULLp;
					break;
				}
			}
			while (ctype(nlin,icol+1)== 's') /* spanning */
				table[nlin][++icol].col = WNULLp;
			if (ch == '\0') break;
		}
		while (++icol <ncol+2)
		{
			table[nlin][icol].col = WNULLp;
			table [nlin][icol].rcol=0;
		}
		while (*cstore != '\0')
			cstore++;
		if (cstore-cspace > MAXCHS)
			cstore = cspace = chspace();
	}
	XFREE(last);
	last = Tombs(cstore);
	permute();
	if (textflg) untext();
	return;
}
nodata(il)
{
	int c;
	for (c=0; c<ncol;c++)
	{
		switch(ctype(il,c))
		{
		case 'c': 
		case 'n': 
		case 'r': 
		case 'l': 
		case 's': 
		case 'a':
			return(0);
		}
	}
	return(1);
}
oneh(lin)
{
	int k, icol;
	k = ctype(lin,0);
	for(icol=1; icol<ncol; icol++)
	{
		if (k != ctype(lin,icol))
			return(0);
	}
	return(k);
}

permute()
{
	int irow, jcol, is;
	wchar_t *start, *strig;
	for(jcol=0; jcol<ncol; jcol++)
	{
		for(irow=1; irow<nlin; irow++)
		{
			if (vspand(irow,jcol,0))
			{
				is = prev(irow);
				if (is<0)
					error(catgets(catd, 1, 20, "Vertical spanning in first row not allowed"));
				start = table[is][jcol].col;
				strig = table[is][jcol].rcol;
				while (irow<nlin &&vspand(irow,jcol,0))
					irow++;
				table[--irow][jcol].col = start;
				table[irow][jcol].rcol = strig;
				while (is<irow)
				{
					table[is][jcol].rcol =0;
					table[is][jcol].col = WCS_SPAN;
					is = next(is);
				}
			}
		}
	}
}
vspand(ir,ij,ifform)
{
	if (ir<0) return(0);
	if (ir>=nlin)return(0);
	if (instead[ir]) return(0);
	if (ifform==0 && ctype(ir,ij)=='^') return(1);
	if (table[ir][ij].rcol!=0) return(0);
	if (fullbot[ir]) return(0);
	return(vspen(table[ir][ij].col));
}
vspen(s)
wchar_t *s;
{
	if (s==0) return(0);
	if (!point(s)) return(0);
	return(wmatch(s, WCS_SPAN));
}
