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
static char *rcsid = "@(#)$RCSfile: lstAlloc.c,v $ $Revision: 1.1.2.3 $ (DEC) $Date: 1993/10/29 22:02:26 $";
#endif

/*
 * Copyright (c) 1988, 1989, 1990 The Regents of the University of California.
 * All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Adam de Boor.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that: (1) source distributions retain this entire copyright
 * notice and comment, and (2) distributions including binaries display
 * the following acknowledgement:  ``This product includes software
 * developed by the University of California, Berkeley and its contributors''
 * in the documentation or other materials provided with the distribution
 * and in all advertising materials mentioning features or use of this
 * software. Neither the name of the University nor the names of its
 * contributors may be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/*-
 * LstAlloc.c --
 *	Allocation and Deallocation of Lst and LstNode structures
 */

#include	"lstInt.h"

Lst LstFreeLst;
int nLstFree;
Lst LstNodeFreeLst;
int nLstNodeFree;
int inited = 0;

#define InitLst(list)					\
    (list) = (Lst)malloc(sizeof(*(list)));		\
    (list)->firstPtr = (list)->lastPtr = NilLstNode;	\
    (list)->isOpen = (list)->isCirc = FALSE;		\
    (list)->atEnd = Unknown

#define RemoveFront(list, front)			\
    if ((front)->nextPtr != NilLstNode)		\
	(front)->nextPtr->prevPtr = (front)->prevPtr;	\
    if ((front)->prevPtr != NilLstNode)		\
	(front)->prevPtr->nextPtr = (front)->nextPtr;	\
    if ((list)->firstPtr == (front))			\
	(list)->firstPtr = (front)->nextPtr;		\
    if ((list)->lastPtr == (front))			\
	(list)->lastPtr = (front)->prevPtr

#define InsertFront(list, front, ln)			\
    (ln)->useCount = (ln)->flags = 0;			\
    if ((front) == NilLstNode) {			\
	(ln)->prevPtr = (ln)->nextPtr = NilLstNode;	\
	(list)->lastPtr = (ln);				\
    } else {						\
	(ln)->prevPtr = (front)->prevPtr;		\
	(ln)->nextPtr = (front);			\
	if ((ln)->prevPtr != NilLstNode)		\
	    (ln)->prevPtr->nextPtr = (ln);		\
	(front)->prevPtr = (ln);			\
    }							\
    (list)->firstPtr = (ln)


Lst
PAlloc()
{
    Lst l, list;
    LstNode ln;
    register LstNode front;

    if (!inited) {
	InitLst(LstFreeLst);
	InitLst(LstNodeFreeLst);
	nLstFree = nLstNodeFree = 0;
	inited = 1;
    }
    if (nLstFree == 0)
	return((Lst)malloc(sizeof(*l)));
    list = LstFreeLst;
    front = list->firstPtr;
    l = (Lst) front->datum;
    RemoveFront(list, front);
    PFreeNode ((Address)front);
    nLstFree--;
    return(l);
}

LstNode
PAllocNode()
{
    register LstNode front;
    register Lst list = (Lst)LstNodeFreeLst;

    if (nLstNodeFree == 0)
	return((LstNode)malloc(sizeof(*front)));
    front = list->firstPtr;
    RemoveFront(list, front);
    nLstNodeFree--;
    return(front);
}

void
PFree(Lst l)
{
    /* Lst l; */

    register Lst list = LstFreeLst;
    register LstNode front;
    register LstNode ln;

    front = list->firstPtr;
    ln = PAllocNode();
    ln->datum = (ClientData)l;
    InsertFront(list, front, ln);
    nLstFree++;
}

void PFreeNode(LstNode ln)
{
    /* LstNode ln; */

    register Lst 	list = LstNodeFreeLst;
    register LstNode	front;

    front = list->firstPtr;
    ln->datum = 0;
    InsertFront(list, front, ln);
    nLstNodeFree++;
}
