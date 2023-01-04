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
 *	@(#)$RCSfile: cma_context.h,v $ $Revision: 4.2.5.2 $ (DEC) $Date: 1993/05/11 22:00:34 $
 */
/*
 *  FACILITY:
 *
 *	CMA services
 *
 *  ABSTRACT:
 *
 *	Header file for per-thread context
 *
 *  AUTHORS:
 *
 *	Dave Butenhof
 *
 *  CREATION DATE:
 *
 *	26 July 1989
 *
 *  MODIFICATION HISTORY:
 *
 *	001	Dave Butenhof	4 December 1989
 *		Include cma_tcb_defs.h instead of cma_tcb.h
 *	002	Paul Curtin	24 May 1991
 *		Added a prototype for cma__reinit_context.
 *	003	Dave Butenhof	12 March 1992
 *		Make most of the variables global so pthread interface can
 *		avoid extra call.
 *	004	Dave Butenhof	 4 May 1993
 *		Make the increment symbol global (__) instead of local (___)
 */

#ifndef CMA_CONTEXT
#define CMA_CONTEXT

/*
 *  INCLUDE FILES
 */

#include <cma.h>
#include <cma_tcb_defs.h>
#include <cma_mutex.h>

/*
 * CONSTANTS AND MACROS
 */

#define cma__c_context_increment 100	/* Realloc arrays this much bigger */

/*
 * TYPEDEFS
 */

typedef cma_t_destructor	*cma__t_destructor_array;

/*
 *  GLOBAL DATA
 */

extern cma_t_natural	cma__g_context_next;	/* Next available key */
extern cma_t_integer	cma__g_context_size;	/* Maximum index of key array */
extern cma__t_int_mutex	*cma__g_context_mutex;	/* Mutex for key creation */

/*
 * INTERNAL INTERFACES
 */

extern void
cma__init_context _CMA_PROTOTYPE_ ((void));

extern void
cma__reinit_context _CMA_PROTOTYPE_ ((
	cma_t_integer	flag));

extern void
cma__run_down_context _CMA_PROTOTYPE_ ((
	cma__t_int_tcb	*tcb));		/* TCB for the thread */

#endif
/*  VAX/DEC CMS REPLACEMENT HISTORY, Element CMA_CONTEXT.H */
/*  *7     4-MAY-1993 11:17:26 BUTENHOF "Rename global symbol appropriately" */
/*  *6    13-MAR-1992 14:08:11 BUTENHOF "Make context mutex global for pthreads" */
/*  *5    10-JUN-1991 19:51:19 SCALES "Convert to stream format for ULTRIX build" */
/*  *4    10-JUN-1991 19:20:16 BUTENHOF "Fix the sccs headers" */
/*  *3    10-JUN-1991 18:18:20 SCALES "Add sccs headers for Ultrix" */
/*  *2    24-MAY-1991 16:45:06 CURTIN "Added a new reinit routine" */
/*  *1    12-DEC-1990 21:43:24 BUTENHOF "Per-thread context" */
/*  VAX/DEC CMS REPLACEMENT HISTORY, Element CMA_CONTEXT.H */
