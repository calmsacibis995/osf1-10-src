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
 *	@(#)$RCSfile: cma_assem.h,v $ $Revision: 4.2.5.2 $ (DEC) $Date: 1993/04/13 21:29:09 $
 */

/*
 *  FACILITY:
 *
 *	DECthreads core
 *
 *  ABSTRACT:
 *
 *	Header file for non-portable (ie, non-C) thread services subroutines.
 *	Although the routines themselves are not portable, the interfaces are
 *	the same on each platform and, therefore, so is this file.
 *
 *  AUTHORS:
 *
 *	Webb Scales
 *
 *  CREATION DATE:
 *
 *	25 August 1989
 *
 *  MODIFICATION HISTORY:
 *
 *	001	Webb Scales and Dave Butenhof	27 September 1989
 *		Add globalvalue for cma__c_default_ps value.
 *	002	Bob Conti	6 October 1989
 *		Move check that cma_s_bugcheck remains constant to 
 *		assertion in cma_thread.c
 *	003	Webb Scales	20 October 1989
 *		Created this file from cma_assem_vms.h
 *	004	Webb Scales	23 October 1989
 *		Changed definition of cma__force_dispatch to look like a routine
 *		Changed "..._PSL" to "..._ps"
 *	005	Webb Scales	 8 November 1989
 *		Add cma__init_assem for VMS
 *	006	Webb Scales	 18 November 1989
 *		Changed cma__t_int_ctx_buffer to cma__t_static_ctx
 *		Removed parent_ctx parameter from call to cma__create_thread
 *		Removed proto for defunct routine, cma__save_thread_ctx
 *	007	Dave Butenhof	4 December 1989
 *		Include cma_tcb_defs.h instead of cma_tcb.h
 *	008	Dave Butenhof	04 April 1991
 *		Minor touchups
 *	009	Paul Curtin	04 April 1991
 *		Added proto for cma__transfer_main_abort
 *	010	DECthreads team	    22 July 1991
 *		Added cma__save_exc_context, & cma__restore_exc_context
 *		in final effort to free DECthreads from crtl on VAX/VMS
 *	011	Dave Butenhof	21 November 1991
 *		Clean up the file.
 *	012	Webb Scales	30 January 1992
 *		Implement our own private $CLRAST
 *	013	Dave Butenhof	13 March 1992
 *		Some time ago, I dropped the start routine and arg arguments
 *		from cma__thread_base(), putting them in the TCB instead. I
 *		never got around to changing all the other calls that lead up
 *		to that, however. Some instructions will be saved by not
 *		passing those two arguments from cma__int_make_thread to
 *		cma__create_thread to cma__execute_thread and back up to
 *		cma__thread_base (which doesn't use them anyway). Gee, wish
 *		I'd thought of that! :-)
 *	014	Paul Curtin	18 May 1992
 *		Add a proto on VAX/VMS for cma__find_persistent_frame.
 *	015	Webb Scales	22 May 1992
 *		Added proto for do-REI.
 *	016	Webb Scales	26 February 1993
 *		Fixed save-exc-context proto to match proto in exc_handling.h
 *	017	Dave Butenhof	 1 March 1993
 *		016 was only correct for OpenVMS AXP, and broke OpenVMS VAX
 *		build. Instead of conditionalizing parameters, I'm going to
 *		just remove the prototype for the save_exc function; it's in
 *		exc_handling.h anyway, and shouldn't be used anywhere that
 *		doesn't include it. (I'm not moving the restore_exc prototype
 *		to exc_handling.h because it's not used by client code, even
 *		though it's a bit odd to have them separated.)
 */

#ifndef CMA_ASSEM
#define CMA_ASSEM

/*
 *  INCLUDE FILES
 */

#include <cma.h>
#include <cma_tcb_defs.h>

/*
 * CONSTANTS AND MACROS
 */

/* 
 * Default processor status (long)word value:  this is the value which is used 
 * when a thread is created or interrupted, to insure that the processor is in
 * a known state.
 */
#if _CMA_COMPILER_ == _CMA__VAXC
globalvalue		cma__c_default_ps;	
#else
extern cma_t_address	cma__c_default_ps;
#endif

/*
 * TYPEDEFS
 */

/*
 *  GLOBAL DATA
 */

/*
 * INTERNAL INTERFACES
 */

#if _CMA_PLATFORM_ == _CMA__VAX_VMS
/*
 * Implement a private version of $CLRAST so that DECthreads doesn't have to
 * depend on SYS.STB (i.e., any particular version of the operating system).
 */
extern void
cma__clrast _CMA_PROTOTYPE_ ((void));
#endif

/*
 * Initialize a thread context and stack and prepare it to be scheduled
 */
extern void
cma__create_thread _CMA_PROTOTYPE_ ((
	cma__t_static_ctx	*child_ctx,
	cma_t_address		stack,
	cma__t_int_tcb		*tcb));

/*
 * cma__do_async_alert is the address to jump to deliver an asynchronous
 * alert. The address of the instruction pointed to by this symbol is placed
 * in the  context area for a signal or AST and is used as the restart PC
 * when the  signal or AST ends.  This causes the execution to resume inside
 * the scheduler,  instead of at the point of interruption, and a context
 * switch is made.
 *
 * Note: this symbol does not necessarily point to the beginning of a
 * routine, and explicit calls should not be made to this address.
 */
extern void
cma__do_async_alert _CMA_PROTOTYPE_ ((void));

/*
 * Breakpoint for use by cma_debug ()
 */
extern void
cma__do_break _CMA_PROTOTYPE_ ((void));

/*
 * Actually perform a call to a routine on a different stack
 */
extern cma_t_address
cma__do_call_on_stack _CMA_PROTOTYPE_ ((
	cma_t_address		stack,
	cma_t_call_routine	routine,
	cma_t_address		arg));

/*
 * Asynchronous interrupt launch code.  This isn't actually called; it's
 * simply put into the VP's state.  The arguments are
 *
 *	s0	address of C interrupt handler
 *	a0	argument to handler
 */
extern void
cma__do_interrupt _CMA_PROTOTYPE_ ((void));

#if _CMA_PLATFORM_ == _CMA__VAX_VMS
/*
 * A routine which executes an REI instruction, which is required before
 * executing dynamic code.  (Required for the CMA$TIS_SHR support.)
 */
extern void
cma__do_rei _CMA_PROTOTYPE_ ((void));

/*
 *  This is in cma_assem on OpenVMS VAX, but in cma_init for OpenVMS AXP.
 *  Make external because cma_init module needs to call it.
 */
extern void
cma__find_persistent_frame _CMA_PROTOTYPE_ ((void));
#endif


/*
 * This is the base routine for a thread.  It's frame is at the bottom of the
 * stack and holds the cma catch-all condition handler.  It is responsibile for
 * calling the thread start routine and for calling the thread cleanup routines.
 * This function is never directly called, but the address & argument list is
 * placed into the context area when a new thread is created.
 */
extern void
cma__execute_thread _CMA_PROTOTYPE_ ((
	cma__t_int_tcb		*tcb));

/* 
 * cma__force_dispatch is the address to jump to cause an involuntary dispatch.
 * The address of the instruction pointed to by this symbol is placed in the 
 * context area for a signal or AST and is used as the restart PC when the 
 * signal or AST ends.  This causes execution to resume inside the scheduler, 
 * instead of at the point of interruption, and a context switch is made.
 *
 * Note: this symbol does not necessarily point to the beginning of a routine,
 * and explicit calls should not be made to this address.
 */
extern void
cma__force_dispatch _CMA_PROTOTYPE_ ((void));

#if _CMA_OS_ == _CMA__VMS
/*
 * Initialize the assembler module.
 * Determine "base frame" for thread stacks.
 */
extern void
cma__init_assem _CMA_PROTOTYPE_ ((void));

/*
 *  Restore routine context after the $UNWIND during exception delivery.
 */
extern void
cma__restore_exc_context _CMA_PROTOTYPE_ ((void));

#endif

/*
 * Restore current thread context (From either sync or asynch context switch)
 */
extern void
cma__restore_thread_ctx _CMA_PROTOTYPE_ ((
	cma__t_static_ctx	*ctx_buf)); /* Address of buffer to restore */

/*
 * cma__transfer_main_abort is used to abort the process by forcing a context
 * switch to the main thread and raising a signal. The signal number is
 * stored in cma__g_abort_signal.
 */
extern void
cma__transfer_main_abort _CMA_PROTOTYPE_ ((void));

/*
 * Transfer execution to a new thread 
 */
extern void
cma__transfer_thread_ctx _CMA_PROTOTYPE_ ((
	cma__t_static_ctx	*cur_ctx, /* Addr of cur thread ctx buffer */
	cma__t_static_ctx	*new_ctx)); /* Addr of new thread ctx buffer */

#endif
/*  VAX/DEC CMS REPLACEMENT HISTORY, Element CMA_ASSEM.H */
/*  *14    1-MAR-1993 11:10:33 BUTENHOF "Remove EXC-specific prototypes" */
/*  *13   26-FEB-1993 19:03:44 SCALES "Fix save-exc-context proto" */
/*  *12   22-MAY-1992 17:43:26 SCALES "Add proto for do-REI" */
/*  *11   22-MAY-1992 17:15:53 CURTIN "Added a proto" */
/*  *10   13-MAR-1992 14:07:21 BUTENHOF "Remove excess thread_base arguments" */
/*  *9    30-JAN-1992 22:12:10 SCALES "Implement our own $CLRAST" */
/*  *8    21-NOV-1991 13:54:08 BUTENHOF "Update this file -- it's OLD" */
/*  *7    26-JUL-1991 15:52:16 CURTIN "added *_exc_context routines" */
/*  *6    10-JUN-1991 19:50:20 SCALES "Convert to stream format for ULTRIX build" */
/*  *5    10-JUN-1991 19:19:57 BUTENHOF "Fix the sccs headers" */
/*  *4    10-JUN-1991 18:16:44 SCALES "Add sccs headers for Ultrix" */
/*  *3    12-APR-1991 23:34:46 BUTENHOF "OSF/1 Mach thread support" */
/*  *2     8-APR-1991 20:30:23 CURTIN "added a proto for transfer_main_abort" */
/*  *1    12-DEC-1990 21:40:53 BUTENHOF "Assembly code header" */
/*  VAX/DEC CMS REPLACEMENT HISTORY, Element CMA_ASSEM.H */
