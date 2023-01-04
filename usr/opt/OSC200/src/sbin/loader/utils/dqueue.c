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
static char	*sccsid = "@(#)$RCSfile: dqueue.c,v $ $Revision: 4.2.3.2 $ (DEC) $Date: 1992/12/07 16:19:39 $";
#endif 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/* dqueue.c
 * This file contains the routines to enqueue and dequeue objects.
 * These routines are identical to the standard VAX C library
 * insque and remque functions, with two differences:
 *	1) this remque returns its argument as its value
 *	2) this remque guarantees that the forward and backward links of
 *	   the dequeued element point to itself.
 *
 * OSF/1 Release 1.0
 */

#include <sys/types.h>
#include <loader.h>

#include "ldr_types.h"
#include "dqueue.h"


void
insque(struct dqueue_elem *elem, struct dqueue_elem *pred)
{
	elem->dq_forw = pred->dq_forw;
	elem->dq_back = pred;
	pred->dq_forw->dq_back = elem;
	pred->dq_forw = elem;
}

struct dqueue_elem *
remque(struct dqueue_elem *elem)
{
	elem->dq_forw->dq_back = elem->dq_back;
	elem->dq_back->dq_forw = elem->dq_forw;
	elem->dq_forw = elem->dq_back = elem;
	return(elem);
}
