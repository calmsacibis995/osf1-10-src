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
/* @(#)remque.c	9.1	(ULTRIX) 8/7/90 */
/* 0001, Ken Lesniak, 20-Mar-1990 */

/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#pragma weak remque = __remque
#endif
#ifdef link
#undef link
#endif

/*
 * Remove queue element.
 */

struct qhead {
	struct qhead	*link;		/* forward link */
	struct qhead	*rlink;		/* reverse link */
};

void remque(elem)
	register struct qhead *elem;		/* element */
    {
	register struct qhead *link;
	register struct qhead *rlink;

	link = elem->link;
	rlink = elem->rlink;
	rlink->link = link;
	link->rlink = rlink;
    }
