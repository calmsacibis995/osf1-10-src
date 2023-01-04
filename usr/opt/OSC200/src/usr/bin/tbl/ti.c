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
static char rcsid[] = "@(#)$RCSfile: ti.c,v $ $Revision: 4.2.6.2 $ (DEC) $Date: 1993/06/10 18:33:41 $";
#endif
/*
 * HISTORY
 */
/*
static	char	*sccsid = "@(#)ti.c	8.1	(Japanese ULTRIX)  2/19/91";
static char sccsid[] = "@(#)ti.c	4.2 8/11/83";
*/

 /* ti.c: classify line intersections */
# include "t..c"
/* determine local environment for intersections */
interv(i,c)
{
	int ku, kl;
	if (c>=ncol || c == 0)
	{
		if (dboxflg)
		{
			if (i==0) return(BOT);
			if (i>=nlin) return(TOP);
			return(THRU);
		}
		if (c>=ncol)
			return(0);
	}
	ku = i>0 ? lefdata(i-1,c) : 0;
	if (i+1 >= nlin)
		kl=0;
	else
		kl = lefdata(allh(i) ? i+1 : i, c);
	if (ku==2 && kl==2) return(THRU);
	if (ku ==2) return(TOP);
	if (kl==BOT) return(2);
	return(0);
}
interh(i,c)
{
	int kl, kr;
	if (fullbot[i]== '=' || (dboxflg && (i==0 || i>= nlin-1)))
	{
		if (c==ncol)
			return(LEFT);
		if (c==0)
			return(RIGHT);
		return(THRU);
	}
	if (i>=nlin) return(0);
	kl = c>0 ? thish (i,c-1) : 0;
	if (kl<=1 && i>0 && allh(up1(i)))
		kl = c>0 ? thish(up1(i),c-1) : 0;
	kr = thish(i,c);
	if (kr<=1 && i>0 && allh(up1(i)))
		kr = c>0 ? thish(up1(i), c) : 0;
	if (kl== '=' && kr ==  '=') return(THRU);
	if (kl== '=') return(LEFT);
	if (kr== '=') return(RIGHT);
	return(0);
}
up1(i)
{
	i--;
	while (instead[i] && i>0) i--;
	return(i);
}
