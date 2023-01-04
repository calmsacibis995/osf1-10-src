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
 *	@(#)$RCSfile: isr_env.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:41:07 $
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
 *	Interrupt service environment header file. Supports loadable drivers,
 *		deferred interrupt handling, non-parallel drivers, etc.
 */
/*
 * OSF/1 Release 1.0
 */

#ifndef _ISR_ENV_H_
#define _ISR_ENV_H_

#include <sys/unix_defs.h>
#include <sys/table.h>			/* ihs_type defines */

/*
 * Interrupt table entry structure
 */
struct intr_list {
	struct ihandler	*it_intr;	/* handler access structure pointer */
	struct lock	it_rwlock;	/* RW-lock for MP, NON-BLOCKING */
	int it_nextid;			/* next handler ID for this slot */
};

/*
 * The interrupt table itself
 */
extern struct intr_list	itbl[];

/*
 * Per-ISR data storage
 */

/*
 * Identify the current instance of an ISR, and its
 * position in the table.
 */
typedef struct ihandler_id {
	int	ihd_index;
	int	ihd_element;
} ihandler_id_t;

/*
 * Record number of times ISR called, and classify
 * it by type for later recall.
 */
typedef struct ihandler_stats {
	int             ihs_type;
	unsigned long   ihs_count;
} ihandler_stats_t;

/* 
 * Machine dependent parameter union 
 * for resolve and handler functions
 */
typedef union {
	int	intparam;	/* Example parameter */
	char	charparam;	/* Example parameter */
	dev_t	devno;		/* Example paramenter */
} ihandler_param_t;

/*
 * TEMP HACK to use #define's rather than boot params
 */
#define ISR_THREADS_TEST

#ifdef ISR_THREADS_TEST
#define B_NUM_ISRTHREADS	4
#define B_SIZE_ISRQUEUE		(B_NUM_ISRTHREADS*2)
#endif

#define NIHHPARAM	1
#define NIHRPARAM	1

/*
 * The main data storage
 */
typedef struct ihandler {
	struct ihandler *	ih_next;		/* next node */
	ihandler_id_t		ih_id;			/* unique id */
	void			(*ih_handler)();	/* handler routine */
	int			(*ih_resolver)();	/* resolver routine */
	int			ih_flags;		/* configure flags */
	int			ih_state;		/* state info */
	int			ih_level;		/* interrupt level */
	int			ih_priority;		/* interrupt priority */
	ihandler_stats_t	ih_stats;		/* interrupt stats */
	ihandler_param_t	ih_hparam[NIHHPARAM];	/* handler params */
	ihandler_param_t 	ih_rparam[NIHRPARAM];	/* resolver params */
	/*
	 * The following deal with deferred, non-parallel,
	 * or processor bound handlers.
	 */
	struct MP_q_hd		*ih_req_queue;		/* request queue */
	struct MP_q_hd		*ih_free_queue;		/* free queue */
	udecl_funnel_data(, ih_funnel);			/* synchronization */
} ihandler_t;

/*
 * Interrupt handler ih_state defines
*/
#define	IH_STATE_ENABLED	0x01			/* Enable/Disable */
#define	IH_STATE_SPL		0x02			/* SPL mode */
#define	IH_STATE_QUERY(x,y)	((x)->ih_state & (y))	/* Query state flags */

/*
 * Interrupt locking defines
*/
#define ITABLE_LOCKINIT(x)	lock_init2(&itbl[x].it_rwlock, FALSE, LTYPE_SWINTR)
#define ITABLE_READ_LOCK(x)	lock_read(&itbl[x].it_rwlock)
#define ITABLE_READ_UNLOCK(x)	lock_done(&itbl[x].it_rwlock)
#define ITABLE_WRITE_LOCK(x)	lock_write(&itbl[x].it_rwlock)
#define ITABLE_WRITE_UNLOCK(x)	lock_done(&itbl[x].it_rwlock)

/*
 * Handler ID macros (as opposed to functions):
 *
 * id_set - builds a handler id from the interrupt level
 *	and a unique code.
 * id_index - derives the index into the interrupt table for
 *	a handler based on the handlers id.
 * id_cmp - compares two handler id's for equality.
 * id_unique - derives a unique element value for indexed slot.
 */
#define	id_set(Id, Ih, Iq) do { \
	(Id)->ihd_index = Ih->ih_level; \
	(Id)->ihd_element = Iq; \
} while (0)

#define	id_index(Id) \
	(((Id)->ihd_index < 0 || (Id)->ihd_index > NUMSYSITBL)?-1:(Id)->ihd_index)

#define id_cmp(Id1, Id2) \
	(((Id1)->ihd_index != (Id2)->ihd_index) || \
		((Id1)->ihd_element != (Id2)->ihd_element))

#define id_unique(Index) \
	(((Index) < 0 || (Index) > NUMSYSITBL) ? -1 : itbl[Index].it_nextid++)

/*
 * Non-parallel/deferred execution queue structure.
 */
struct isr_que {
	queue_chain_t	chain;		/* Queue chain */
	ihandler_t 	*handle;	/* ISR handler structure */
};

/*
 * Non-parallel/deferred queue defines.
 */
#define ISR_Q_LWM	3		/* Free queue low water mark */
#define ISR_Q_ALC	8		/* # of slots to add to free queue */

#endif /*_ISR_ENV_H_*/
