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
 *	@(#)$RCSfile: lv_q.h,v $ $Revision: 4.2.3.2 $ (DEC) $Date: 1992/04/29 14:20:26 $
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
#ifndef _LV_Q_H_
#define _LV_Q_H_

#include <sys/buf.h>

/*
 *  queues, and macros for manipulating them.
 */

struct lv_queue {
	struct buf *lv_head;
	struct buf **lv_tail;
	int	lv_count;
};

#include <lvm/lv_qmacros.h>		/* helpful macro definitions */
	/* Q is always a "pointer to a struct lv_queue", E is a "pointer
	 * to an element of the queue" */
#define LV_QUEUE_APPEND(Q,E)	Q_APPEND_PROTO(Q,E,\
					lv_head,lv_tail,av_forw,lv_count)
#define LV_QUEUE_PREPEND(Q,E)	Q_PREPEND_PROTO(Q,E,\
					lv_head,lv_tail,av_forw,lv_count)
#define LV_QUEUE_FETCH(Q,E)	Q_FETCH_PROTO(Q,E,\
					lv_head,lv_tail,av_forw,lv_count)
#define LV_QUEUE_INIT(Q)	Q_INIT_PROTO(Q,lv_head,lv_tail,lv_count)

#endif  /* _LV_Q_H_ */
