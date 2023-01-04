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
/*	
 *	@(#)$RCSfile: insque.s,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 03:02:54 $
 */ 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0
 */
/* insque:	Insert element into circular, doubly-linked queue
 * Calling sequence:	insque (elem, pred);
 *   where pred and elem are pointers to structs having the following form:
 *   struct qelem {
 *	struct qelem *next;	 Ptr to next element in queue 
 *	struct qelem *prev;	 Ptr to previous element in queue 
 *	char   whatever[1];	 Rest of struct - whatever is needed 
 *   };
 *  Note: This version, unlike the Vax version, will not blow up if passed 
 *	 a pred whose next ptr is null.  Pred and Elem must not be null, 
 *	 however.
 */
#include "SYS.h"

	.file	"insque.s"
	.text
	.align	1
	.set	prev,4
	.set	next,0
ENTRY(insque)
	movd	SP(4),r1	# elem
	movd	SP(8),r0	# pred
	movd	next(r0),r2	# preds former next ptr
	movd	r0,prev(r1)	# Point elems prev at pred
	movd	r2,next(r1)	# Point elems next at preds old next
	movd	r1,next(r0)	# Change preds next to point to elem
	cmpqd	$0,r2		# Was preds old next ptr null?
	beq	.Lendit		# If so, do not update nexts prev ptr
	movd	r1,prev(r2)	# Else update it
.Lendit:
	EXIT
	ret	$0
