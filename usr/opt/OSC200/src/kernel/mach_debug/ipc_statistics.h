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
 *	@(#)$RCSfile: ipc_statistics.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:36:22 $
 */ 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/* 
 * Mach Operating System
 * Copyright (c) 1989 Carnegie-Mellon University
 * Copyright (c) 1988 Carnegie-Mellon University
 * Copyright (c) 1987 Carnegie-Mellon University
 * All rights reserved.  The CMU software License Agreement specifies
 * the terms and conditions for use and redistribution.
 */
/*
 * OSF/1 Release 1.0
 */

#ifndef	_MACH_DEBUG_IPC_STATISTICS_H_
#define _MACH_DEBUG_IPC_STATISTICS_H_

/*
 *	Remember to update the mig type definition
 *	in mach_debug_types.defs when adding/removing fields.
 */

typedef struct ipc_statistics {
	int		version;
	int		messages;
	int		complex;
	int		kernel;
	int		large;
	int		current;
	int		emergency;
	int		notifications;
	int		port_copyins;
	int		port_copyouts;
	int		port_copyin_hits;
	int		port_copyin_miss;
	int		port_allocations;
	int		bucket_misses;
	int		ip_data_grams;
} ipc_statistics_t;

typedef struct ipc_bucket_info {
	int		count;		/* number of records in bucket */
} ipc_bucket_info_t;

typedef ipc_bucket_info_t *ipc_bucket_info_array_t;

#ifdef	KERNEL
#include <mach_ipc_stats.h>

#include <kern/lock.h>
#include <kern/macro_help.h>

decl_simple_lock_data(extern,ipc_statistics_lock_data)
extern ipc_statistics_t ipc_statistics;

extern void ipc_stats_init();

#define ipc_statistics_lock()	simple_lock(&ipc_statistics_lock_data)
#define ipc_statistics_unlock()	simple_unlock(&ipc_statistics_lock_data)

#if	MACH_IPC_STATS
#define ipc_event_count(field, count)		\
MACRO_BEGIN					\
	ipc_statistics_lock();			\
	ipc_statistics.field += count;		\
	ipc_statistics_unlock();		\
MACRO_END
#else	/* MACH_IPC_STATS */
#define ipc_event_count(field, count)
#endif	/* MACH_IPC_STATS */

#define ipc_event(field)		ipc_event_count(field, 1)

#endif	/* KERNEL */
#endif	/* _MACH_DEBUG_IPC_STATISTICS_H_ */
