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
 *	@(#)$RCSfile: cma_thread.h,v $ $Revision: 4.2.5.2 $ (DEC) $Date: 1993/05/25 20:00:45 $
 */
/*
 *  FACILITY:
 *
 *	CMA services
 *
 *  ABSTRACT:
 *
 *	Header file for cma thread services
 *
 *  AUTHORS:
 *
 *	Webb Scales
 *
 *  CREATION DATE:
 *
 *	31 August 1989
 *
 *  MODIFICATION HISTORY:
 *
 *	001	Dave Butenhof	4 December 1989
 *		Include cma_tcb_defs.h instead of cma_tcb.h
 *	002	Dave Butenhof	12 December 1989
 *		Add cma__thread_start routine.
 *	003	Dave Butenhof	05 April 1991
 *		Add cma__int_make_thread routine.
 *	004	Dave butenhof	02 July 1991
 *		Simplify prototype for cma__thread_base()... it'll get start
 *		routine and argument from the TCB.
 *	005	Dave Butenhof	11 February 1992
 *		Modify prototype for cma__int_make_thread
 *	006	Dave Butenhof	30 November 1992
 *		Change int_make_thread to return status value instead of
 *		boolean.
 *	007	Dave Butenhof	13 May 1993
 *		Add cma__int_detach() prototype.
 */

#ifndef CMA_THREAD
#define CMA_THREAD

/*
 *  INCLUDE FILES
 */

#include <cma.h>
#include <cma_tcb_defs.h>

/*
 * CONSTANTS AND MACROS
 */

/*
 * TYPEDEFS
 */

/*
 *  GLOBAL DATA
 */

/*
 * INTERNAL INTERFACES
 */

extern void
cma__cleanup_thread _CMA_PROTOTYPE_ ((
	cma__t_int_tcb		*tcb,	/* Thread to clean up */
	cma_t_address		result,	/* Value thread's function */
	cma_t_exit_status	exit_status)); /* Indicates successful term*/

extern void
cma__int_detach _CMA_PROTOTYPE_ ((
	cma__t_int_tcb		*tcb));

extern cma_t_status
cma__int_make_thread _CMA_PROTOTYPE_ ((
	cma__t_int_tcb		*tcb,
	cma_t_thread		*handle,
	cma_t_start_routine	start_routine,
	cma_t_address		start_arg));

extern void
cma__thread_base _CMA_PROTOTYPE_ ((
	cma__t_int_tcb		*tcb));

#endif
/*  VAX/DEC CMS REPLACEMENT HISTORY, Element CMA_THREAD.H */
/*  *10   14-MAY-1993 15:56:57 BUTENHOF "Add cma__int_detach" */
/*  *9     1-DEC-1992 14:05:57 BUTENHOF "OSF/1 scheduling" */
/*  *8    13-MAR-1992 14:10:28 BUTENHOF "Remove excess thread_base arguments" */
/*  *7    18-FEB-1992 15:30:55 BUTENHOF "Add prototype" */
/*  *6     2-JUL-1991 16:53:08 BUTENHOF "Change prototype for thread_base" */
/*  *5    10-JUN-1991 19:57:24 SCALES "Convert to stream format for ULTRIX build" */
/*  *4    10-JUN-1991 19:22:06 BUTENHOF "Fix the sccs headers" */
/*  *3    10-JUN-1991 18:24:36 SCALES "Add sccs headers for Ultrix" */
/*  *2    12-APR-1991 23:37:12 BUTENHOF "Add internal function" */
/*  *1    12-DEC-1990 21:54:29 BUTENHOF "Thread management" */
/*  VAX/DEC CMS REPLACEMENT HISTORY, Element CMA_THREAD.H */
