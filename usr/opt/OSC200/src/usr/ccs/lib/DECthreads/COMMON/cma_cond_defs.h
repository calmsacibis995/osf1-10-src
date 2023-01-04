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
 * @(#)$RCSfile: cma_cond_defs.h,v $ $Revision: 1.1.5.2 $ (DEC) $Date: 1993/04/13 21:29:44 $
 */
/*
 *  FACILITY:
 *
 *	DECthreads core
 *
 *  ABSTRACT:
 *
 *	Header file for condition variable structure
 *
 *  AUTHORS:
 *
 *	Dave Butenhof
 *
 *  CREATION DATE:
 *
 *	22 November 1991
 *
 *  MODIFICATION HISTORY:
 *
 *	001	Dave Butenhof	26 August 1992
 *		Get rid of semaphores!
 *	002	Dave Butenhof	14 September 1992
 *		Add debug fields to count deferred operations & pending
 *		wakes.
 *	003	Dave Butenhof	29 March 1993
 *		Integrate March 10 review comments.
 */

#ifndef CMA_COND_DEFS
#define CMA_COND_DEFS

/*
 *  INCLUDE FILES
 */

#include <cma_attr.h>

/*
 * CONSTANTS AND MACROS
 */

/*
 * TYPEDEFS
 */

typedef struct CMA__T_INT_CV {
    cma__t_object	header;		/* Common header (sequence, type) */
    cma__t_queue	queue;		/* Queue of waiters */
    cma__t_atomic_bit	event;		/* Clear when may be waiters */
    cma__t_atomic_bit	spindle;	/* Control internal access */
    cma__t_atomic_bit	nopend;		/* Clear for pending wakeup */
    cma__t_atomic_bit	defer_sig;	/* Clear for deferred signal */
    cma__t_atomic_bit	defer_bro;	/* Clear for deferred broadcast */
#ifndef NDEBUG
    cma_t_integer	defsig_cnt;	/* Count deferred signals */
    cma_t_integer	defbro_cnt;	/* Count deferred broadcasts */
    cma_t_integer	pend_cnt;	/* COunt pending wakes */
#endif
    } cma__t_int_cv;

/*
 *  GLOBAL DATA
 */

/*
 * INTERNAL INTERFACES
 */

#endif
/*  VAX/DEC CMS REPLACEMENT HISTORY, Element CMA_COND_DEFS.H */
/*  *6    29-MAR-1993 13:56:23 BUTENHOF "Integrate March 10 review comments" */
/*  *5     2-NOV-1992 13:25:12 BUTENHOF "Speedyize & fix race" */
/*  *4    15-SEP-1992 13:49:11 BUTENHOF "Add debug fields" */
/*  *3     2-SEP-1992 16:24:01 BUTENHOF "Separate semaphores from kernel lock" */
/*  *2    21-AUG-1992 13:41:29 BUTENHOF "Use spinlocks on kernel thread semaphores instead of kernel_critical" */
/*  *1    22-NOV-1991 13:27:47 BUTENHOF "Condition variable struct" */
/*  VAX/DEC CMS REPLACEMENT HISTORY, Element CMA_COND_DEFS.H */
