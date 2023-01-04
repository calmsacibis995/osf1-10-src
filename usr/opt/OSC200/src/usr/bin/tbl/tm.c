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
static char rcsid[] = "@(#)$RCSfile: tm.c,v $ $Revision: 4.2.6.2 $ (DEC) $Date: 1993/06/10 18:37:31 $";
#endif
/*
 * HISTORY
 */
/*
static	char	*sccsid = "@(#)tm.c	8.1	(Japanese ULTRIX)  2/19/91";
static char sccsid[] = "@(#)tm.c	4.2 8/11/83";
*/

 /* tm.c: split numerical fields */
# include "t..c"
wchar_t *maknew(str)
wchar_t *str;
{
/* make two numerical fields */
	int  c;
	wchar_t *p, *q, *ba, *dpoint;
	p = str;
	for (ba= 0; c = *str; str++)
		if (c == '\\' && *(str+1)== '&')
			ba=str;
	str=p;
	if (ba==0)
	{
		for (dpoint=0; *str; str++)
		{
			if (*str=='.' && !wineqn(str,p) &&
			    (str>p && digit((int )*(str-1)) ||
			    digit((int )*(str+1))))
				dpoint=str;
		}
		if (dpoint==0)
			for(; str>p; str--)
			{
				if (digit((int )*(str-1) ) && !wineqn(str, p))
					break;
			}
		if (!dpoint && p==str) /* not numerical, don't split */
			return(0);
		if (dpoint) str=(wchar_t *)dpoint;
	}
	else
		str = ba;
	p =str;
	if (exstore ==0 || exstore >exlim)
	{
		exstore = chspace();
		exlim= exstore+MAXCHS;
	}
	q = exstore;
	while (*exstore++ = *str++);
	*p = 0;
	return(q);
}
ineqn (s, p)
char *s, *p;
{
/* true if s is in a eqn within p */
	int ineq = 0, c;
	while (c = *p)
	{
		if (s == p)
			return(ineq);
		p++;
		if ((ineq == 0) && (c == delim1))
			ineq = 1;
		else
			if ((ineq == 1) && (c == delim2))
				ineq = 0;
	}
	return(0);
}
wineqn (s, p)
wchar_t *s, *p;
{
/* true if s is in a eqn within p */
	int ineq = 0;
	wchar_t c;
	while (c = *p)
	{
		if (s == p)
			return(ineq);
		p++;
		if ((ineq == 0) && (c == (wchar_t)delim1))
			ineq = 1;
		else
			if ((ineq == 1) && (c == (wchar_t)delim2))
				ineq = 0;
	}
	return(0);
}
