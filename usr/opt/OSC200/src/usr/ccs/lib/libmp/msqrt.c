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
static char	*sccsid = "@(#)$RCSfile: msqrt.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 04:11:41 $";
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
#ifndef lint

#endif not lint

#include <mp.h>
msqrt(a,b,r) MINT *a,*b,*r;
{	MINT x,junk,y;
	int j;
	x.len=junk.len=y.len=0;
	if(a->len<0) fatal("msqrt: neg arg");
	if(a->len==0)
	{	b->len=0;
		r->len=0;
		return(0);
	}
	if(a->len%2==1) x.len=(1+a->len)/2;
	else x.len=1+a->len/2;
	x.val=xalloc(x.len,"msqrt");
	for(j=0;j<x.len;x.val[j++]=0);
	if(a->len%2==1) x.val[x.len-1]=0400;
	else x.val[x.len-1]=1;
	xfree(b);
	xfree(r);
loop:
	mdiv(a,&x,&y,&junk);
	xfree(&junk);
	madd(&x,&y,&y);
	sdiv(&y,2,&y,(short *)&j);
	if(mcmp(&x,&y)>0)
	{	xfree(&x);
		move(&y,&x);
		xfree(&y);
		goto loop;
	}
	xfree(&y);
	move(&x,b);
	mult(&x,&x,&x);
	msub(a,&x,r);
	xfree(&x);
	return(r->len);
}
