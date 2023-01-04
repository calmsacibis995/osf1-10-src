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
static char	*sccsid = "@(#)$RCSfile: parse.c,v $ $Revision: 4.2.7.2 $ (DEC) $Date: 1993/10/01 01:22:48 $";
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

#endif

#include <stdio.h>
#include <ctype.h>
#include <locale.h>
#include <nl_types.h>

#include "awk.def"
#include "awk.h"
#include "awk_msg.h"

#define MSGSTR(Num, Str) catgets(catd,NL_SETD, Num, Str)

extern nl_catd catd;

node *ALLOC(n)
{
	register node *x;
	x = (node *) malloc(sizeof(node) + (n-1)*sizeof(node *));
	if (x == NULL)
		error(FATAL, MSGSTR(NOALLOC, "out of space in ALLOC"));

	return(x);
}

node *exptostat(node *a)
{
	a->ntype = NSTAT;
	return(a);
}

node	*nullstat;

node *node0(obj *a)
{
	register node *x;
	x=ALLOC(0);
	x->nnext = NULL;
	x->nobj=(long) a;
	return(x);
}

node *node1(obj *a, node *b)
{
	register node *x;
	x=ALLOC(1);
	x->nnext = NULL;
	x->nobj=(long)a;
	x->narg[0]=b;
	return(x);
}

node *node2(obj *a, node *b, node *c)
{
	register node *x;
	x = ALLOC(2);
	x->nnext = NULL;
	x->nobj = (long) a;
	x->narg[0] = b;
	x->narg[1] = c;
	return(x);
}

node *node3(obj *a, node *b, node *c, node *d)
{
	register node *x;
	x = ALLOC(3);
	x->nnext = NULL;
	x->nobj = (long) a;
	x->narg[0] = b;
	x->narg[1] = c;
	x->narg[2] = d;
	return(x);
}

node *node4(obj *a, node *b, node *c, node *d, node *e)
{
	register node *x;
	x = ALLOC(4);
	x->nnext = NULL;
	x->nobj = (long) a;
	x->narg[0] = b;
	x->narg[1] = c;
	x->narg[2] = d;
	x->narg[3] = e;
	return(x);
}
node *stat3(obj *a, node *b, node *c, node *d)
{
	register node *x;
	x = node3(a,b,c,d);
	x->ntype = NSTAT;
	return(x);
}

node *op2(obj *a, node *b, node *c)
{
	register node *x;
	x = node2(a,b,c);
	x->ntype = NEXPR;
	return(x);
}

node *op1(obj *a, node *b)
{
	register node *x;
	x = node1(a,b);
	x->ntype = NEXPR;
	return(x);
}

node *stat1(obj *a, node *b)
{
	register node *x;
	x = node1(a,b);
	x->ntype = NSTAT;
	return(x);
}

node *op3(obj *a,node *b, node *c, node *d)
{
	register node *x;
	x = node3(a,b,c,d);
	x->ntype = NEXPR;
	return(x);
}

node *stat2(obj *a, node *b, node *c)
{
	register node *x;
	x = node2(a,b,c);
	x->ntype = NSTAT;
	return(x);
}

node *stat4(obj *a, node *b, node *c, node *d, node *e)
{
	register node *x;
	x = node4(a,b,c,d,e);
	x->ntype = NSTAT;
	return(x);
}

node *valtonode(obj *a, int b)
{
	register node *x;
	x = node0(a);
	x->ntype = NVALUE;
	x->subtype = b;
	return(x);
}

node *pa2stat(node *a, node *b, node *c)
{
	register node *x;
	x = node3((obj *) paircnt++, a, b, c);
	x->ntype = NPA2;
	return(x);
}

node *linkum(node *a, node *b)
{
	register node *c;
	if(a == NULL) return(b);
	else if(b == NULL) return(a);
	for(c=a; c->nnext != NULL; c=c->nnext);
	c->nnext = b;
	return(a);
}

node *genprint()
{
	register node *x;
	x = stat2((obj *) PRINT,valtonode((obj *) lookup("$record", symtab, 0), CFLD), nullstat);
	return(x);
}
