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
 * 
 * @(#)$RCSfile: cma_host.h,v $ $Revision: 1.1.4.2 $ (DEC) $Date: 1993/05/25 20:02:08 $
 */
/*
 *  FACILITY:
 *
 *	CMA services
 *
 *  ABSTRACT:
 *
 *	Header file for ALPHA OSF/1 host-specific functions
 *
 *  AUTHORS:
 *
 *	Dave Butenhof
 *
 *  CREATION DATE:
 *
 *	01 April 1992
 *
 *  MODIFICATION HISTORY:
 *
 *	001	Dave Butenhof	01 April 1992
 *		Add conditionals for use from cma_assem.s
 *	002	Webb Scales	03 April 1992
 *		Rework asynch context switch after the VMS CLRAST model.
 *	003	Brian Keane	08 June 1992
 *		Make a guess at stack limit on Alpha OSF
 *	004	Dave Butenhof	13 August 1992
 *		Add prototype for cma__set_unique
 *	005	Dave Butenhof	20 August 1992
 *		Minor optimization: it's ridiculous to call a cma__unset
 *		assembler routine that just stores a 0. Do it as a macro
 *		instead!
 *	006	Dave Butenhof	 8 September 1992
 *		Add assembly interfaces to rt syscalls.
 *	007	Dave Butenhof	 2 October 1992
 *		Remove 006. Do it with syscall() instead.
 *	008	Dave Butenhof	13 October 1992
 *		Tell signal module to decode arithmetic signals.
 *	009	Dave Butenhof	14 October 1992
 *		Correct stack range.
 *	010	Dave Butenhof	 4 November 1992
 *		It's a placeholder, but define prototype for
 *		cma__memory_barrier, which we'll need for true MP hardware
 *		with kernel threads. Also, fix definition of cma__unset to
 *		use the real assembly code (with an MB) rather than the
 *		inline C code if running on MP hardware.
 *	011	Dave Butenhof	12 April 1993
 *		Experiment with OSF/1 cc's asm directive to avoid assembly
 *		calls.
 *	012	Dave Butenhof	10 May 1993
 *		Conditionalize asm inlining, since cc doesn't optimize right.
 *		Also, improve guess on stack range.
 */


#ifndef CMA_HOST_DEFS
# define CMA_HOST_DEFS

/*
 *  INCLUDE FILES
 */

# include <sys/types.h>
# ifdef _CMA_USE_ASM_
#  include <c_asm.h>
# endif

/*
 * CONSTANTS AND MACROS
 */

# if __LANGUAGE_C__

/*
 * Initialization for "atomic bit" objects
 */
#  define cma__c_tac_static_clear	0
#  define cma__tac_clear(addr)	(*(addr) = (cma__t_atomic_bit)0)
#  define cma__tac_set(addr)	(*(addr) = (cma__t_atomic_bit)1)
#  define cma__tac_isset(addr)	(*(addr) != (cma__t_atomic_bit)0)

/*
 * This platform provides a secondary signal code.  Raise specific exceptions
 * where possible.
 */
#  define _CMA_DECODE_SIGNALS_	1

/*
 * Disable interrupts (ASTs)
 */
#  if 0 /* This macro is not currently needed */
#   define cma__interrupt_disable(sig_mask) \
    (sigprocmask (SIG_SETMASK, &cma___g_sig_block_mask, &sig_mask) == -1 ? \
	cma__bugcheck(), 0 : \
	0)
#  else
#   define cma__interrupt_disable(dummy)
#  endif

/*
 * Enable interrupts (signals)
 */
#  if 0 /* This macro is not currently needed */
#   define cma__interrupt_enable(sig_mask) \
    (sigprocmask (SIG_SETMASK, &sig_mask, cma_c_null_ptr) == -1 ? \
	cma__bugcheck(), 0 : \
	0)
#  else
#   define cma__interrupt_enable(dummy)
#  endif

/*
 * Clear interrupt (AST).  Allow other interrupts to interrupt, even though
 * the current interrupt is still active.
 */
#  define cma__clear_interrupt(sig_mask) \
    (sigprocmask (SIG_SETMASK, sig_mask, cma_c_null_ptr) == -1 ? \
	cma__bugcheck("cma__clear_interrupt, mask = %x", sig_mask), 0 : \
	0)

/*
 * Atomically clear the low bit of longword at the specified address.
 */
#  define cma__kernel_set(address)	cma__test_and_set(address)
#  define cma__kernel_unset(address)	cma__unset(address)

#  ifdef _CMA_USE_ASM_
#   define cma__memory_barrier()	asm ("mb")
#   define cma__fetch_sp()	(void *)asm ("bis %sp,%r31,%v0")
#   define cma__fetch_gp()	(void *)asm ("bis %r29,%r31,%v0")
#   define cma__fetch_fp()	(void *)asm ("bis %r29,%r31,%v0")
#   define cma__set_unique(_v_)	asm ("call_pal 0x9f",_v_)
#  endif

#  if !_CMA_MP_HARDWARE_
#   define cma__unset(address)	(*(address) = (cma__t_atomic_bit)0)
#  else
#   define cma__unset(address)	(cma__memory_barrier(),\
	(*(address) = (cma__t_atomic_bit)0))
#  endif

/*
 * Maximum and minimum addresses in default thread stack
 */
#  define cma__c_def_stack_min	 0x4000000L
#  define cma__c_def_stack_max	0x11fffffffL

/*
 * TYPEDEFS
 */

typedef int	cma__t_atomic_bit;

typedef struct CMA__T_STATIC_CTX {
    long int	fp;
    long int	sp;
    } cma__t_static_ctx;

typedef struct CMA__T_ASYNC_CTX {
    cma_t_boolean   valid;
    cma_t_address   interrupt_ctx;
    } cma__t_async_ctx;

/*
 *  GLOBAL DATA
 */

/*
 * INTERNAL INTERFACES
 */

#  ifndef _CMA_USE_ASM_
long int
cma__fetch_fp (void);

long int
cma__fetch_sp (void);

long int
cma__fetch_gp (void);

void
cma__memory_barrier (void);

void
cma__set_unique (long int);
#  endif

int
cma__test_and_set (cma__t_atomic_bit	*bit);

# endif

#endif					/* CMA_HOST_DEFS */
/*  VAX/DEC CMS REPLACEMENT HISTORY, Element CMA_HOST.H */
/*  *13   10-MAY-1993 14:10:41 BUTENHOF "Improve OSF/1 AXP stack range" */
/*  *12   16-APR-1993 13:07:25 BUTENHOF "try asm statement" */
/*  *11    5-NOV-1992 14:25:37 BUTENHOF "Add MB" */
/*  *10   14-OCT-1992 12:52:05 BUTENHOF "Correct stack range" */
/*  *9    13-OCT-1992 11:45:05 BUTENHOF "decode signals" */
/*  *8     2-OCT-1992 16:12:51 BUTENHOF "Remove assembly code sched stuff" */
/*  *7    16-SEP-1992 10:08:25 BUTENHOF "Add rt interfaces" */
/*  *6    20-AUG-1992 09:46:33 BUTENHOF "optimize cma__unset" */
/*  *5    13-AUG-1992 14:45:17 BUTENHOF "Add prototypes" */
/*  *4    15-JUN-1992 16:53:45 KEANE "Adjust stack limits for Alpha OSF" */
/*  *3     6-APR-1992 12:41:10 SCALES "Rework asynch context switch" */
/*  *2     1-APR-1992 13:31:31 BUTENHOF "Add assem conditionals" */
/*  *1    31-MAR-1992 13:29:01 BUTENHOF "Alpha OSF/1 defs" */
/*  VAX/DEC CMS REPLACEMENT HISTORY, Element CMA_HOST.H */








