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
static char rcsid[] = "@(#)$RCSfile: tc.c,v $ $Revision: 4.2.6.2 $ (DEC) $Date: 1993/06/10 18:22:20 $";
#endif
/*
 * HISTORY
 */
/*
static	char	*sccsid = "@(#)tc.c	8.1	(Japanese ULTRIX)  2/19/91";
static char sccsid[] = "@(#)tc.c	4.2 8/11/83";
*/

 /* tc.c: find character not in table to delimit fields */
# include "t..c"
choochar()
{
	/* choose funny characters to delimit fields */
	int had[128], ilin,icol, k;
	unsigned char *s;
	for(icol=0; icol<128; icol++)
		had[icol]=0;
	F1 = F2 = 0;
	for(ilin=0;ilin<nlin;ilin++)
	{
		if (instead[ilin]) continue;
		if (fullbot[ilin]) continue;
		for(icol=0; icol<ncol; icol++)
		{
			k = ctype(ilin, icol);
			if (k==0 || k == '-' || k == '=')
				continue;
			if(table[ilin][icol].col == 0) s = 0;
			else if(point(table[ilin][icol].col))
				s = (unsigned char*)Tombs(table[ilin][icol].col);
			else s = (unsigned char *)table[ilin][icol].col;
			if (point(s))
				while (*s){
					if(!point(*s)) had[*s]=1;
					s++;
				}
			XFREE((char *)s);
			if(table[ilin][icol].rcol == 0) s = 0;
			else if(point(table[ilin][icol].rcol))
				s = (unsigned char*)Tombs(table[ilin][icol].rcol);
			else s = (unsigned char *)table[ilin][icol].rcol;
			if (point(s))
				while (*s){
					if(!point(*s)) had[*s]=1;
					s++;
				}
			XFREE((char *)s);
		}
	}
	/* choose first funny character */
	for(
	s=(unsigned char *)"\002\003\005\006\007!%&#/?,:;<=>@`^~_{}+-*ABCDEFGHIJKMNOPQRSTUVWXYZabcdefgjkoqrstwxyz";
		*s; s++)
	    {
		if (had[*s]==0)
		{
			F1= *s;
			had[F1]=1;
			break;
		}
	}
	/* choose second funny character */
	for(
	s=(unsigned char *)"\002\003\005\006\007:_~^`@;,<=>#%&!/?{}+-*ABCDEFGHIJKMNOPQRSTUVWXZabcdefgjkoqrstuwxyz";
		*s; s++)
	    {
		if (had[*s]==0)
		{
			F2= *s;
			break;
		}
	}
	if (F1==0 || F2==0)
		error(catgets(catd, 1, 26, "couldn't find characters to use for delimiters"));
	return;
}
point(s)
{
	return(s>= 128 || s<0);
}
