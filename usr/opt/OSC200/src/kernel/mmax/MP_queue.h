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
 *	@(#)$RCSfile: MP_queue.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:39:25 $
 */ 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 *
 * Copyright 1990 Encore Computer Corporation
 *
 * ALL RIGHTS RESERVED. Licensed Material - Property of Encore Computer
 * Corporation. This software is made available solely pursuant to the
 * terms of a software license agreement which governs its use. 
 * Unauthorized duplication, distribution or sale are strictly prohibited.
 *
 * Module Function:
 *	General purpose multiprocessing queuing header file.
 */
/*
 * OSF/1 Release 1.0
 */

#ifndef _MP_QUEUE_H_
#define _MP_QUEUE_H_

#define MP_QWAIT	1
#define MP_QNOWAIT	2

struct MP_q_hd {
	struct queue_entry	head;	/* Header for queue */
	struct slock		lock;	/* Lock for queue */
	long			cnt;	/* # of elements in queue */
	long			alloc;	/* is anyone allocating memory? */
};

/* MP_queue environment locking macros */
#define MPQ_SLOCK_INIT(X)	simple_lock_init(X)
#define MPQ_SLOCK(X)	simple_lock(X)
#define MPQ_SULOCK(X)	simple_unlock(X)
#define MPQ_RLOCK(X)	lock_read(X)
#define MPQ_RULOCK(X)	lock_done(X)
#define MPQ_WLOCK(X)	lock_write(X)
#define MPQ_WULOCK(X)	lock_done(X)

#endif /*_MP_QUEUE_H_*/
