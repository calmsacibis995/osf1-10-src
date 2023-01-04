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
static char	*sccsid = "@(#)$RCSfile: util.c,v $ $Revision: 4.2.2.2 $ (DEC) $Date: 1992/04/07 12:26:43 $";
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

char *malloc();
#ifdef lint
int xv_oid;
#endif
#include <stdio.h>
#include <mp.h>
move(a,b) MINT *a,*b;
{	int i,j;
	xfree(b);
	b->len=a->len;
	if((i=a->len)<0) i = -i;
	if(i==0) return;
	b->val=xalloc(i,"move");
	for(j=0;j<i;j++)
		b->val[j]=a->val[j];
	return;
}
dummy(){}
short *xalloc(nint,s) char *s;
{	short *i;
	i=(short *)malloc(2*(unsigned)nint+sizeof(char *));
#ifdef DBG
	if(dbg) fprintf(stderr, "%s: %o\n",s,i);
#endif
	if(i!=NULL) return(i);
	fatal("mp: no free space");
	return(0);
}
fatal(s) char *s;
{
	fprintf(stderr,"%s\n",s);
	VOID fflush(stdout);
	sleep(2);
	abort();
}
xfree(c) MINT *c;
{
#ifdef DBG
	if(dbg) fprintf(stderr, "xfree ");
#endif
	if(c->len==0) return;
	shfree(c->val);
	c->len=0;
	return;
}
mcan(a) MINT *a;
{	int i,j;
	if((i=a->len)==0) return;
	else if(i<0) i= -i;
	for(j=i;j>0 && a->val[j-1]==0;j--);
	if(j==i) return;
	if(j==0)
	{	xfree(a);
		return;
	}
	if(a->len > 0) a->len=j;
	else a->len = -j;
}
MINT *itom(n)
{	MINT *a;
	a=(MINT *)xalloc(2,"itom");
	if(n>0)
	{	a->len=1;
		a->val=xalloc(1,"itom1");
		*a->val=n;
		return(a);
	}
	else if(n<0)
	{	a->len = -1;
		a->val=xalloc(1,"itom2");
		*a->val= -n;
		return(a);
	}
	else
	{	a->len=0;
		return(a);
	}
}
mcmp(a,b) MINT *a,*b;
{	MINT c;
	int res;
	if(a->len!=b->len) return(a->len-b->len);
	c.len=0;
	msub(a,b,&c);
	res=c.len;
	xfree(&c);
	return(res);
}
