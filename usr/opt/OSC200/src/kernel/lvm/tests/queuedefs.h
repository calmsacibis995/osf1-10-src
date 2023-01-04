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
 *	@(#)$RCSfile: queuedefs.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:29:40 $
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
struct item {
	struct item *next;
	int value;
};

struct queue {
	struct item *head;	/* first item on queue */
	struct item **tail;	/* pointer to place to hang next item */
	int count;		/* for statistics */
};

#include <lvm/lv_qmacros.h>
#define Q_APPEND(Q,NEW)	Q_APPEND_PROTO(Q,NEW,head,tail,next,count)
#define Q_FETCH(Q,E)	Q_FETCH_PROTO(Q,E,head,tail,next,count)
#define Q_INIT(Q)	Q_INIT_PROTO(Q,head,tail,count)
