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
 *	@(#)$RCSfile: cma_host.h,v $ $Revision: 4.2.5.2 $ (DEC) $Date: 1992/09/29 14:28:58 $
 */
/*
 *  Copyright (c) 1989, 1992 by
 *  Digital Equipment Corporation, Maynard Massachusetts.
 *  All rights reserved.
 *
 *  This software is furnished under a license and may be used and  copied
 *  only  in  accordance  with  the  terms  of  such  license and with the
 *  inclusion of the above copyright notice.  This software or  any  other
 *  copies  thereof may not be provided or otherwise made available to any
 *  other person.  No title to and ownership of  the  software  is  hereby
 *  transferred.
 *
 *  The information in this software is subject to change  without  notice
 *  and  should  not  be  construed  as  a commitment by DIGITAL Equipment
 *  Corporation.
 *
 *  DIGITAL assumes no responsibility for the use or  reliability  of  its
 *  software on equipment which is not supplied by DIGITAL.
 */

/*
 *  FACILITY:
 *
 *	CMA services
 *
 *  ABSTRACT:
 *
 *	Header file for MIPS/ULTRIX host-specific functions
 *
 *	Note:  This header file is used by *both* C and Assembler source code.
 *
 *  AUTHORS:
 *
 *	Dave Butenhof and Webb Scales
 *
 *  CREATION DATE:
 *
 *	19 October 1989
 *
 *  MODIFICATION HISTORY:
 *
 *	001	Dave Butenhof	20 October 1989
 *		Add "cma__fetch_sp" function (currently not implemented) to
 *		shadow the cma_host_vms.h implementation.
 *	002	Dave Butenhof	31 October 1989
 *		Time to implement cma__fetch_sp: as prototype for assembly
 *		code routine.
 *	003	Webb Scales	18 November 1989
 *		Add host dependent thread context structures, and #define's for
 *		mips assembly code.
 *	004	Dave Butenhof and Webb Scales	9 January 1990
 *		Delete cma__add_atomic, since it's not used anywhere.
 *	005	Webb Scales	16 April 1990
 *		Add default stack limits 
 *	006	Webb Scales	3 May 1990
 *		Replace cma__lock_any and cma__unlock_any by 
 *		cma__interrupt_disable and cma__interrupt_enable.
 *	007	Dave Butenhof	14 May 1990
 *		Replace the old cma__test_and_set with the new
 *		cma__kernel_set, and cma__unset with cma__kernel_unset.
 *		These assume that the kernel is locked, or that they are
 *		being used to manipulate the kernel lock.  The
 *		cma__test_and_set and cma__unset operations now lock the
 *		kernel (using cma__kernel_set) before attempting the
 *		operation, to ensure atomicity on a uniprocessor.
 *	008	Webb Scales	16 August 1990
 *		Rearranged things, moving a lot into the host-generic file.
 *	009	Dave Butenhof	18 December 1990
 *		Reintegrate things from old "host-generic file," since we've
 *		removed two-tiered cma_host mechanism.
 *	010	Dave Butenhof	09 April 1991
 *		Improve portability by adding a type for internal locks,
 *		rather than assuming it can be just an int.
 *	011	Dave Butenhof	25 April 1991
 *		Remove experimental locking and rely on O/S calls (blech).
 *	012	Dave Butenhof	07 May 1991
 *		Add cma__fetch_gp() function to set up child thread state.
 *	013	Dave Butenhof	14 May 1991
 *		Fix MP cma__t_atomic_bit macros.  When using the emulated TAS
 *		opcode on OSF/1, the "set" state is the address of the
 *		variable... static initialization to "1" doesn't work.
 *	014	Dave Butenhof	22 November 1991
 *		Straightening out dependencies: this file is so low-level, it
 *		shouldn't use cma.h or cma_defs.h types (such as
 *		cma_t_boolean). Change typedef of atomic bit to "int".
 *	015	Dave Butenhof	21 January 1992
 *		Add some registers used by "picie" (position independent code
 *		converter) in setting up OSF/1 shared libraries, so that
 *		DECthreads can become a shared library.
 *	016	Webb Scales	31 March 1992
 *		Reworked asynch context switch after the VMS CLRAST model.
 */


#ifndef CMA_HOST_DEFS
# define CMA_HOST_DEFS

# if !defined(__LANGUAGE_ASSEMBLY__) && !defined(LANGUAGE_ASSEMBLY)

/*
 *  INCLUDE FILES
 */

/*
 * CONSTANTS AND MACROS
 */

/*
 * Initialization for "atomic bit" objects
 */
#  if _CMA_UNIPROCESSOR_
#   define cma__c_tac_static_clear	0
#   define cma__tac_clear(addr)		((*addr) = 0)
#   define cma__tac_set(addr)		((*addr) = 1)
#   define cma__tac_isset(addr)		((*addr) != 0)
#  else
#   define cma__c_tac_static_clear	0
#   define cma__tac_clear(addr)		((*addr) = 0)
#   define cma__tac_set(addr)		((*addr) = (cma__t_atomic_bit)(addr))
#   define cma__tac_isset(addr)		((*addr) != 0)
#  endif

#  define cma__c_def_stack_min	0x40000000
#  define cma__c_def_stack_max	0x7fffffff

/*
 * This platform provides a secondary signal code.  Raise specific exceptions
 * where possible.
 */
#  define _CMA_DECODE_SIGNALS_	1

/*
 * Disable interrupts (signals)
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
 * Clear interrupt context (to allow new interrupts to come in)
 */
#  define cma__clear_interrupt(sig_mask) \
    (sigprocmask (SIG_SETMASK, sig_mask, cma_c_null_ptr) == -1 ? \
	cma__bugcheck("cma__clear_interrupt, mask = %x", sig_mask), 0 : \
	0)

/*
 * Perform an interlocked test-and-set operation... the argument is the
 * address of a variable of type cma__t_atomic_bit (normally an int) and the
 * result is a boolean indicating the previous state of the variable (i.e.,
 * when cma__test_and_set() is used as a lock, it returns the value
 * cma_c_false (0) when the lock is acquired).
 */
#  if _CMA_UNIPROCESSOR_
/*
 * The uniprocessor operations use the kernel lock to create uniprocessor
 * atomic operations on a bit.  (Note that cma__unset doesn't actually need
 * to lock the kernel, since writing a single value is atomic anyway.)
 */
#   define cma__kernel_set(address) \
    ((*(address) != 0) ? cma_c_true : (*(address) = 1, cma_c_false))
#   define cma__test_and_set(address) \
    ((cma__kernel_set (&cma__g_kernel_critical), (*(address) != 0)) ? \
	(cma__kernel_unset (&cma__g_kernel_critical), cma_c_true) : \
	(*(address) = 1, cma__kernel_unset (&cma__g_kernel_critical), cma_c_false))
#  else					/* Use VP lock for MP */
#   define cma__test_and_set(address)	(cma__tas (address))
#   define cma__kernel_set(address)	cma__test_and_set (address)
#  endif

/*
 * Clear the low bit of longword at the specified address.
 */
#  define cma__unset(address) (cma__tac_clear (address))
#  define cma__kernel_unset(address) (cma__tac_clear (address))

/*
 * TYPEDEFS
 */

typedef int	cma__t_atomic_bit;

typedef struct CMA__T_STATIC_CTX {
    long int	sp;
    } cma__t_static_ctx;

typedef struct CMA__T_ASYNC_CTX {
    cma_t_boolean   valid;
    cma_t_address   interrupt_ctx;
    } cma__t_async_ctx;
# endif /* LANGUAGE_C */

# if defined (LANGUAGE_ASSEMBLY) || defined (__LANGUAGE_ASSEMBLY__)
#  define	AC_SIG_MASK	0
#  define	AC_RESTART_PC	1
#  define	AC_USED_FPC	2
#  define	AC_FPC_CSR	3
#  define	AC_FPC_EIR	4
#  define	AC_CP0_CAUSE	5
#  define	AC_CP0_BAD_VA	6
#  define	AC_CPU_BAD_PA	7
#  define	AC_T9           8
#  define	AC_GP           9
# endif /* LANGUAGE_ASSEMBLY */

# if !defined(__LANGUAGE_ASSEMBLY__) && !defined(LANGUAGE_ASSEMBLY)
/*
 *  GLOBAL DATA
 */

/*
 * (this is actually declared by cma_kernel.h; redeclare it here so that
 * cma_host.h can be independent of cma_kernel.h, to avoid circular
 * dependency!
 */
extern cma__t_atomic_bit	cma__g_kernel_critical;	/* CMA in krnl */

/*
 * INTERNAL INTERFACES
 */

#  if !_CMA_UNIPROCESSOR_
extern int
cma__tas _CMA_PROTOTYPE_ ((cma__t_atomic_bit *addr));
#  endif

/*
 * Return the present value of the stack pointer for the current thread
 */
extern long int
cma__fetch_sp _CMA_PROTOTYPE_ ((void));

extern long int
cma__fetch_gp _CMA_PROTOTYPE_ ((void));

# endif					/* LANGUAGE_C */

#endif					/* CMA_HOST_DEFS */
/*  DEC/CMS REPLACEMENT HISTORY, Element CMA_HOST.H */
/*  *15    3-APR-1992 18:34:50 SCALES "Implement clrast analog for U*ix" */
/*  *14   23-JAN-1992 14:22:35 BUTENHOF "Integrate shared library support" */
/*  *13   22-NOV-1991 12:02:09 BUTENHOF "Use int rather than cma_t_boolean" */
/*  *12   10-JUN-1991 18:57:23 SCALES "Fix sccs header" */
/*  *11   10-JUN-1991 18:09:13 SCALES "Add sccs headers for Ultrix" */
/*  *10    6-JUN-1991 11:18:24 BUTENHOF "Change language conditionals for Tin" */
/*  *9    15-MAY-1991 13:22:14 BUTENHOF "Fix bug introduced into kernel_set" */
/*  *8    14-MAY-1991 16:18:50 BUTENHOF "[-.mips]" */
/*  *7    14-MAY-1991 13:57:38 BUTENHOF "Define a cma__fetch_gp function" */
/*  *6     6-MAY-1991 17:46:00 BUTENHOF "Change test-and-set" */
/*  *5     3-MAY-1991 11:27:03 BUTENHOF "Change test-and-set" */
/*  *4    12-APR-1991 23:33:55 BUTENHOF "Change type of internal locks" */
/*  *3    18-DEC-1990 22:39:25 BUTENHOF "Make cma_host.h standalone" */
/*  *2    12-DEC-1990 20:33:37 BUTENHOF "Fix assem include, and clean up CMS history" */
/*  *1    12-DEC-1990 18:57:23 BUTENHOF "MIPS specific host defs" */
/*  DEC/CMS REPLACEMENT HISTORY, Element CMA_HOST.H */
