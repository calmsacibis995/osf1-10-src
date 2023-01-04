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
 * @(#)$RCSfile: cma_stack.h,v $ $Revision: 4.2.7.3 $ (DEC) $Date: 1993/05/25 19:59:18 $
 */
/*
 *  FACILITY:
 *
 *	CMA services
 *
 *  ABSTRACT:
 *
 *	Header file for stack management
 *
 *  AUTHORS:
 *
 *	Dave Butenhof
 *
 *  CREATION DATE:
 *
 *	21 August 1989
 *
 *  MODIFICATION HISTORY:
 *
 *	001	Dave Butenhof	29 September 1989
 *		Make cma__assign_chunks external, and add tcb backpointer to
 *		stack descriptor.
 *	002	Dave Butenhof	All Saints Day 1989
 *		Add prototype for get_tcb_kernel.
 *	003	Dave Butenhof	4 December 1989
 *		Include cma_tcb_defs.h instead of cma_tcb.h
 *	004	Dave Butenhof	10 April 1990
 *		If _CMA_UNIPROCESSOR_ is defined, declare cma__get_self_tcb
 *		as a macro returning the dispatcher's current thread pointer,
 *		instead of as a prototype for the stack cluster search
 *		function.
 *	005	Paul Curtin	27 June 1990
 *		Added cma__g_chunk_size
 *	006	Dave Butenhof	29 November 1990
 *		Add cma__get_sp_tcb to return a TCB for some specified SP
 *		value.
 *	007	Dave Butenhof	14 December 1990
 *		Change cma_attributes.h to cma_attr.h (shorten length)
 *	008	Dave Butenhof	04 April 1991
 *		Change _CMA_UNIPROCESSOR_ to 0/1 instead of ifdef/ifndef
 *	009	Paul Curtin	24 May 1991
 *		Added cma__reinit_stack prototype.
 *	010	Dave Butenhof	02 October 1991
 *		Integrate changes provided by Alan Peckham to unprotect guard
 *		pages on all stacks before aborting the process; this allows
 *		the UNIX core dump code (which makes the absurd assumption
 *		that the known universe stops just because a page is
 *		protected) to work on threaded processes.
 *	011	Dave Butenhof	13 March 1992
 *		Move cma__roundup_chunksize from cma_stack_int.h, since an
 *		"__" interface belongs here.
 *	012	Dave Butenhof	13 August 1992
 *		Don't compile normal get_self_tcb on Alpha OSF/1 -- use rd/wr
 *		unique PAL instead.
 *	013	Dave Butenhof	25 January 1993
 *		Resolve an ancient FIX-ME in cma_deb_core.c by moving
 *		cma___c_reserve_size to cma_stack.h (as cma__c_reserve_size).
 *	014	Dave Butenhof	30 March 1993
 *		Add cma__debug_list_stacks.
 *	015	Dave Butenhof	12 April 1993
 *		Remove DEC OSF/1 cma_assem.s cma__get_self_tcb() prototype;
 *		it's now an inline asm() macro in cma_host.h.
 *	016	Dave Butenhof	10 May 1993
 *		Restore use of assembly call for get_self_tcb, unless inline
 *		asm is enabled by _CMA_USE_ASM_ (cc optimizes bad code).
 */
#ifndef CMA_STACK
#define CMA_STACK

/*
 *  INCLUDE FILES
 */

#include <cma_attr.h>
#include <cma_queue.h>
#include <cma_tcb_defs.h>
#include <cma_stack_int.h>

/*
 * CONSTANTS AND MACROS
 */

#if _CMA_UNIPROCESSOR_
# define cma__get_self_tcb()	(cma__g_current_thread)
#elif _CMA_HARDWARE_ == _CMA__ALPHA
# ifdef _CMA_USE_ASM_
#  define cma__get_self_tcb()	((cma__t_int_tcb *)asm ("call_pal 0x9e"))
# endif
#endif

/*
 * Round the given value (a) upto cma__g_chunk_size
 */
#define cma__roundup_chunksize(a)   (cma__roundup(a,cma__g_chunk_size))

#if _CMA_VENDOR_ == _CMA__HP
# if _CMA_HARDWARE_ == _CMA__HPPA
/* 
 * These constants are guesses calculated by multiplying the non-hp values
 * by the ratio of hpux hppa page size (2048) to the non-hp page size (512).
 */
#  define cma__c_reserve_size	cma__c_page_size * 6	/* Stack reserve area */
# else
/* 
 * These constants are guesses calculated by multiplying the non-hp values
 * by the ratio of hpux 68k page size (4096) to the non-hp page size (512).
 */
#  define cma__c_reserve_size	cma__c_page_size * 3	/* Stack reserve area */
# endif
#else
# if _CMA_OS_ == _CMA__UNIX
#  define cma__c_reserve_size	cma__c_page_size * 4	/* Stack reserve area */
# else
#  define cma__c_reserve_size	cma__c_page_size * 21	/* Stack reserve area */
# endif
#endif

/*
 * TYPEDEFS
 */

/*
 *  GLOBAL DATA
 */

extern cma__t_list	cma__g_stack_clusters;
extern cma__t_int_tcb	*cma__g_current_thread;
extern cma_t_integer	cma__g_chunk_size;

/*
 * INTERNAL INTERFACES
 */

extern void
cma__assign_stack _CMA_PROTOTYPE_ ((		/* Assign stack chunks to thread */
	cma__t_int_stack	*stack,	/*  stack to assign them to */
	cma__t_int_tcb		*tcb));	/*  tcb to assign them to */

extern void
cma__debug_list_stacks _CMA_PROTOTYPE_ ((	/* Debug -- list stacks */
	cma_t_integer		argc,
	char			*argv[]));

extern void
cma__free_stack _CMA_PROTOTYPE_ ((		/* Free a stack */
	cma__t_int_stack	*desc)); /*  Stack to free */

extern void
cma__free_stack_list _CMA_PROTOTYPE_ ((
	cma__t_queue	*list));	/* Head of stack list to free */

#if !_CMA_UNIPROCESSOR_ && !defined(_CMA_USE_ASM_)
extern cma__t_int_tcb *
cma__get_self_tcb _CMA_PROTOTYPE_ ((void));	/* Get address of thread's TCB */
#endif

extern cma__t_int_tcb *
cma__get_sp_tcb _CMA_PROTOTYPE_ ((cma_t_address	sp));

extern cma__t_int_stack *
cma__get_stack _CMA_PROTOTYPE_ ((		/* Create a new stack */
	cma__t_int_attr	*attr));	/*  Controlling attr. obj */

extern void
cma__init_stack _CMA_PROTOTYPE_ ((void));

extern void
cma__reinit_stack _CMA_PROTOTYPE_ ((
	cma_t_integer	flag));

# if _CMA_PROTECT_MEMORY_
extern void
cma__remap_stack_holes _CMA_PROTOTYPE_ ((void));
# endif

#endif
/*  VAX/DEC CMS REPLACEMENT HISTORY, Element CMA_STACK.H */
/*  *15   10-MAY-1993 14:11:15 BUTENHOF "Conditionalize asm() useage" */
/*  *14   16-APR-1993 13:05:50 BUTENHOF "Move get_self_tcb to OSF/1 host" */
/*  *13    1-APR-1993 14:33:23 BUTENHOF "Add show -s" */
/*  *12   25-JAN-1993 11:34:19 BUTENHOF "Move reserve size to cma_stack.h" */
/*  *11   13-AUG-1992 14:44:12 BUTENHOF "Set and use Alpha pal code unique value" */
/*  *10   13-MAR-1992 14:09:59 BUTENHOF "Rearrange stack/guard rounding" */
/*  *9    29-JAN-1992 14:37:24 BUTENHOF "Improve special cluster management" */
/*  *8    14-OCT-1991 13:40:22 BUTENHOF "Unprotect guard pages on abort" */
/*  *7    10-JUN-1991 19:56:38 SCALES "Convert to stream format for ULTRIX build" */
/*  *6    10-JUN-1991 19:21:44 BUTENHOF "Fix the sccs headers" */
/*  *5    10-JUN-1991 18:23:51 SCALES "Add sccs headers for Ultrix" */
/*  *4    24-MAY-1991 16:47:45 CURTIN "Added a new reinit routine" */
/*  *3    12-APR-1991 23:36:59 BUTENHOF "Change _CMA_UNIPROCESSOR_ to 0/1" */
/*  *2    14-DEC-1990 00:55:58 BUTENHOF "Change module names" */
/*  *1    12-DEC-1990 21:53:27 BUTENHOF "Thread stacks" */
/*  VAX/DEC CMS REPLACEMENT HISTORY, Element CMA_STACK.H */
