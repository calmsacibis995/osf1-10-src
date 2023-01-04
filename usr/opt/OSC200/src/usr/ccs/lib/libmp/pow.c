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
static char	*sccsid = "@(#)$RCSfile: pow.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 04:11:47 $";
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
pow(a,b,c,d) MINT *a,*b,*c,*d;
{	int i,j,n;
	MINT x,y;
	x.len=y.len=0;
	xfree(d);
	d->len=1;
	d->val=xalloc(1,"pow");
	*d->val=1;
	for(j=0;j<b->len;j++)
	{	n=b->val[b->len-j-1];
		for(i=0;i<15;i++)
		{	mult(d,d,&x);
			mdiv(&x,c,&y,d);
			if((n=n<<1)&0100000)
			{	mult(a,d,&x);
				mdiv(&x,c,&y,d);
			}
		}
	}
	xfree(&x);
	xfree(&y);
	return;
}
rpow(a,n,b) MINT *a,*b;
{	MINT x,y;
	int i;
	x.len=1;
	x.val=xalloc(1,"rpow");
	*x.val=n;
	y.len=n*a->len+4;
	y.val=xalloc(y.len,"rpow2");
	for(i=0;i<y.len;i++) y.val[i]=0;
	y.val[y.len-1]=010000;
	xfree(b);
	pow(a,&x,&y,b);
	xfree(&x);
	xfree(&y);
	return;
}
