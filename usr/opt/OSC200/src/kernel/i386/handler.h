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
 *	@(#)$RCSfile: handler.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:18:12 $
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

#ifndef _SYS_HANDLER_H_
#define _SYS_HANDLER_H_

#include <sys/types.h>
#include <i386/cpu.h>

/*
 * Size of interrupt dispatch table and
 * Number of handlers to install at boot time
 */

#define ITABLE_SIZE	16

/*
 * Interrupt handler ih_flag defines
 */
#define	IH_VEC_DYNAMIC_OK	0x01	/* Allow relocation of target vector */
#define	IH_VEC_MULTIPLE_OK	0x02	/* Allow multiple handlers per vector */

/*
 * Interrupt handler ih_state defines
 */
#define IH_STATE_ENABLED	0x01			/* enable/disable */
#define IH_STATE_SPL		0x02			/* SPL mode */
#define IH_STATE_QUERY(x,y)	( (x)->ih_state & (y) )	/* Query state flags */


/* 
 * Machine dependent parameter union 
 * for resolve and handler functions
 */
typedef union {
	int	intparam;			/* Integer (e.g. unit #) */
	struct	isa_dev *dev;			/* Device for resolver */
	struct	isa_ctlr *ctlr;			/* Controller for resolver */
	char	charparam;			/* Example parameter */
	dev_t	devnoparam;			/* Example paramenter */
} ihandler_param_t;


/* 
 * Machine dependent id type for opaque
 * use with machine dependent id functions:
 * id_set, id_index, id_cmp, id_unique
 */
typedef struct {
	short int	id_index;		/* Itable index */
	short int	id_element;		/* Unique node/index */
} ihandler_id_t;

typedef struct {
        int     intr_type;                      /* defines in sys/table.h */
        long    intr_cnt;
} ihandler_stats_t;



/* 
 * Interrupt handler structure 
 */

#define NIHHPARAM       1
#define NIHRPARAM       2

typedef struct ihandler {
        struct ihandler * ih_next;		/* next node */
	ihandler_id_t	ih_id;			/* unique id */
        int		(*ih_handler)();	/* handler routine */
        int		(*ih_resolver)();	/* resolver routine */
        int		ih_flags;		/* configure flags */
        int		ih_state;		/* state info */
        int		ih_level;		/* interrupt level */
        int		ih_priority;		/* interrupt priority */
        ihandler_stats_t ih_stats;              /* interrupt statistics */
	ihandler_param_t ih_hparam[NIHHPARAM];	/* handler params */
	ihandler_param_t ih_rparam[NIHRPARAM];	/* resolver params */
#if NCPUS > 1
						/* deferred, Non-MP queues */
	isrq_t *	ih_req_queue;		/* request queue */
	isrq_t *	ih_free_queue;		/* free queue */
	funnel_t 	ih_funnel;		/* synchronization */
#endif
} ihandler_t;

int i386_resolver();
/* Shorthand for device/controller to resolver */
#define ih_rdev		ih_rparam[1].dev
#define ih_rctlr	ih_rparam[0].ctlr

ihandler_id_t *handler_add();
ihandler_t *handler_override();

#endif /* _SYS_HANDLER_H_ */

