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
 *	@(#)$RCSfile: remque.s,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 03:05:57 $
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
/* remque:	Remove element from circular, doubly-linked queue
 * Calling sequence:	remque (elem);
 *   where elem is a pointer to a struct having the following form:
 *   struct qelem {
 *	struct qelem *next;	 Ptr to next element in queue
 *	struct qelem *prev;	 Ptr to previous element in queue
 *	char   whatever[1];	 Rest of struct - whatever is needed
 *   };
 *  Note: This version, unlike the Vax version, will not blow up if passed 
 *	 an elem whose next ptr is null.  Elem itself must not be null, 
 *	 however.
 */
#include "SYS.h"

	.file	"remque.s"
	.text
	.align	1
	.set	prev,4
	.set	next,0
ENTRY(remque)
	movd	SP(4),r0	# elem
	movd	prev(r0),r1	# elem->pred
	movd	next(r0),r2	# elem->next
	movd	r2,next(r1)	# (elem->prev)->next = elem->next
	cmpqd	$0,r2		# is elem->next null?
	beq	.Lendit		# If so, do not update it
	movd	r1,prev(r2)	# Else (elem->next)->prev = elem->prev
.Lendit:
	EXIT
	ret	$0
