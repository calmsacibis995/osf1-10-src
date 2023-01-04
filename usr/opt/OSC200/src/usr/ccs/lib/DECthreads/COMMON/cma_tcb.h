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
 *	@(#)$RCSfile: cma_tcb.h,v $ $Revision: 4.2.2.3 $ (DEC) $Date: 1992/12/10 18:20:47 $
 */

/*
 *  FACILITY:
 *
 *	Concert Multithread Architecture (CMA) services
 *
 *  ABSTRACT:
 *
 *	TCB-related definitions.
 *
 *  AUTHORS:
 *
 *	Dave Butenhof
 *
 *  CREATION DATE:
 *
 *	24 July 1989
 *
 *  MODIFICATION HISTORY:
 *
 *	001	Dave Butenhof	14 August 1989
 *		Extract context list into separate typedef for casts.
 *	002	Dave Butenhof	15 August 1989
 *		Add type field to object structure.
 *	003	Dave Butenhof	21 August 1989
 *		Add pointer to first stack descriptor for thread.
 *	004	Dave Butenhof	24 August 1989
 *		Modify TCB to use queues instead of lists
 *	005	Webb Scales	26 August 1989
 *		Change from jmp_buf context to private context type
 *	006	Dave Butenhof	29 August 1989
 *		Remove #include of cma_stack.h, since we don't need it any
 *		more.  Define context buffer locally to minimize dependencies
 *		and risk of circularities.
 *	007	Dave Butenhof	31 August 1989
 *		Rename cv field to term_cv.
 *	008	Dave Butenhof	14 September 1989
 *		Add alert state field to TCB.
 *	009	Hans Oser	22 Septemper 1989
 *		Add pc and psl field to TCB.
 *	010	Webb Scales	26 September 1989
 *		Change position of FP field in context buffer.
 *	011	Dave Butenhof	29 September 1989
 *		Add initial_test field to TCB (TCB creation will copy from
 *		attributes object so TCB is independent of changes to attr.
 *		obj between creation and execution).
 *	012	Dave Butenhof	11 October 1989
 *		Convert to use internal mutex operations.
 *	013	Dave Butenhof	16 October 1989
 *		Change TCB fields for Hans...
 *	014	Hans Oser	20 October 1989
 *		Add TCB fields for ultrix "select"
 *	015	Webb Scales	23 October 1989
 *		Changed "..._PSL" to "..._ps"
 *	016	Dave Butenhof	27 October 1989
 *		Add TCB field so alert can find condition variable to
 *		broadcast on (simplest way to break through wait).
 *	017	Dave Butenhof	1 November 1989
 *		Add backpointer for mutex.
 *	018	Dave Butenhof	16 November 1989
 *		Define tcb_time structure here to avoid circular
 *		dependencies.
 *	019	Webb Scales	18 November 1989
 *		Make thread context structure(s) host dependent
 *	020	Dave Butenhof	22 November 1989
 *		Make forward decl for semaphore structure to resolve circular
 *		references.
 *	021	Dave Butenhof	30 November 1989
 *		Add field for scheduling policy (not implemented yet).
 *	022	Dave Butenhof	4 December 1989
 *		Remove all the type definitions to cma_tcb_def.h to try to
 *		break circular dependencies.
 *	023	Paul Curtin	8 April 1991
 *		Added externs for use with quick kill
 *	024	Paul Curtin	24 May 1991
 *		Added a prototype for cma__reinit_tcb
 *	025	Dave Butenhof	 9 September 1992
 *		Conditionalize cma__g_abort_signal on the symbols under which
 *		it is used.
 */
#ifndef CMA_TCB
#define CMA_TCB

/*
 *  INCLUDE FILES
 */

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

extern cma__t_int_tcb	    cma__g_def_tcb;	 /* Default thread's TCB */
extern cma__t_static_ctx   *cma__g_def_ctx_buf;	 /* Default static context */
#if _CMA_OS_ != _CMA__VMS
 extern cma_t_integer	    cma__g_abort_signal; /* Process abort signal */
#endif

/*
 * INTERNAL INTERFACES
 */

extern void
cma__destroy_tcb _CMA_PROTOTYPE_ ((
	cma__t_int_tcb	*old_tcb));	/* The TCB to be deleted */

extern void
cma__free_tcb _CMA_PROTOTYPE_ ((
	cma__t_int_tcb	*old_tcb));	/* The TCB to be freed */

extern cma__t_int_tcb *
cma__get_tcb _CMA_PROTOTYPE_ ((
	cma__t_int_attr	*attrib));	/* Attributes object to use */

extern void
cma__init_tcb _CMA_PROTOTYPE_ ((void));

extern void
cma__reinit_tcb _CMA_PROTOTYPE_ ((
	cma_t_integer	flag));

#endif
/*  VAX/DEC CMS REPLACEMENT HISTORY, Element CMA_TCB.H */
/*  *8    11-SEP-1992 07:08:44 BUTENHOF "Conditionalize cma__g_abort_signal def" */
/*  *7    18-FEB-1992 15:30:38 BUTENHOF "Add return value to int_make_thread" */
/*  *6    10-JUN-1991 19:57:00 SCALES "Convert to stream format for ULTRIX build" */
/*  *5    10-JUN-1991 19:21:54 BUTENHOF "Fix the sccs headers" */
/*  *4    10-JUN-1991 18:24:12 SCALES "Add sccs headers for Ultrix" */
/*  *3    24-MAY-1991 16:48:36 CURTIN "Added a new reinit routine" */
/*  *2     8-APR-1991 20:31:17 CURTIN "Added some global defs for quick kill" */
/*  *1    12-DEC-1990 21:53:52 BUTENHOF "Thread bookkeeping" */
/*  VAX/DEC CMS REPLACEMENT HISTORY, Element CMA_TCB.H */
