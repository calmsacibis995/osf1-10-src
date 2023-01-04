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
 *	@(#)$RCSfile: dispatcher.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:17:32 $
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

#ifndef _SYS_DISPATCHER_H_
#define _SYS_DISPATCHER_H_

#include <sys/types.h>

#if  INTRSYNC_LOCK
#include <kern/lock.h>
#endif

/* 
 * Interrupt base table 
 */
typedef struct {
	ihandler_t *	it_ih;			/* Interrupt handler */
	int		it_elementcnt;		/* Number of handlers */
#if  INTRSYNC_LOCK
	lock_data_t	it_rwlock;		/* R/W lock */
#endif
} itable_t;
extern itable_t itable[];


/*
 * Interrupt Table locking macros.  
 * Note: These locks must not block/sleep since we are operating in the
 * 	 interupt context of a thread.
 */
#if  INTRSYNC_LOCK
#define ITABLE_LOCKINIT(x)	lock_init (&itable[x].it_rwlock, FALSE)
#define ITABLE_READ_LOCK(x)	lock_read (&itable[x].it_rwlock)
#define ITABLE_READ_UNLOCK(x)	lock_done (&itable[x].it_rwlock)
#define ITABLE_WRITE_LOCK(x)	lock_write(&itable[x].it_rwlock)
#define ITABLE_WRITE_UNLOCK(x)	lock_done (&itable[x].it_rwlock)
#else
#define ITABLE_LOCKINIT(x)	
#define ITABLE_READ_LOCK(x)	
#define ITABLE_READ_UNLOCK(x)	
#define ITABLE_WRITE_LOCK(x)	
#define ITABLE_WRITE_UNLOCK(x)	
#endif
#endif /* _SYS_DISPATCHER_H_ */
