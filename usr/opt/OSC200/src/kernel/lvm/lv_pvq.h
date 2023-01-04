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
 *	@(#)$RCSfile: lv_pvq.h,v $ $Revision: 4.2.3.2 $ (DEC) $Date: 1992/04/29 14:20:18 $
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
#ifndef _LV_PVQ_H_
#define _LV_PVQ_H_

/*
 *  queues, and macros for manipulating them.
 */

struct lv_pvqueue {
	struct pvol *lv_pvhead;
	struct pvol **lv_pvtail;
	int	lv_pvcount;
};

#include "lv_qmacros.h"		/* helpful macro definitions */
	/* Q is always a "pointer to a struct lv_pvqueue", E is a "pointer
	 * to an element of the queue" */
#define LV_PVQUEUE_APPEND(Q,E)	Q_APPEND_PROTO(Q,E,\
					lv_pvhead,lv_pvtail,pv_cache_next,\
					lv_pvcount)
#define LV_PVQUEUE_FETCH(Q,E)	Q_FETCH_PROTO(Q,E,\
					lv_pvhead,lv_pvtail,pv_cache_next,\
					lv_pvcount)
#define LV_PVQUEUE_INIT(Q)	Q_INIT_PROTO(Q,lv_pvhead,lv_pvtail,lv_pvcount)

#endif  /* _LV_PVQ_H_ */
