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
	.rdata
	.asciiz "@(#)$RCSfile: cma_assem.s,v $ $Revision: 1.1.6.3 $ (DEC) $Date: 1993/12/15 20:31:18 $"
	.text

/*
 *
 *  Copyright (c) Digital Equipment Corporation, 1992
 *  All Rights Reserved.  Unpublished rights reserved
 *  under the copyright laws of the United States.
 *  
 *  The software contained on this media is proprietary
 *  to and embodies the confidential technology of 
 *  Digital Equipment Corporation.  Possession, use,
 *  duplication or dissemination of the software and
 *  media is authorized only pursuant to a valid written
 *  license from Digital Equipment Corporation.
 *
 *  RESTRICTED RIGHTS LEGEND   Use, duplication, or 
 *  disclosure by the U.S. Government is subject to
 *  restrictions as set forth in Subparagraph (c)(1)(ii)
 *  of DFARS 252.227-7013, or in FAR 52.227-19, as
 *  applicable.
 *
 *  FACILITY:
 *
 *	DECthreads services
 *
 *  ABSTRACT:
 *
 *	These are non-portable subroutines for thread service routines.  They
 *	cannot be written in C because they manipulate specific registers, etc.
 *
 *  AUTHORS:
 *
 *	Dave Butenhof
 *
 *  CREATION DATE:
 *
 *	31 March 1992
 *
 *  MODIFICATION HISTORY:
 *
 *	001	Dave Butenhof	01 April 1992
 *		Remove extra header files that don't seem to be there.
 *	002	Brian Keane	05 June 1992
 *		Add cma__fetch_gp.  Patch in other entry points temporarily.
 *	003	Brian Keane	17 July 1992
 *		Remove a bunch of ifdef'd out code.  Cleanup test_and_set,
 *		and the fetch_xx functions; they don't need to establish
 *		the gp since they are only called from within the 
 *		threads library.  Provisionally implemented do_break
 *		with a call_pal.  Added a few "call_pal halt"s in
 *		spots we should never get to.
 *	004	Dave Butenhof	28 July 1992
 *		Implement cma__do_interrupt; launch pad for async. alert.
 *		Also cleanup cma__do_call_on_stack so it might work.
 *	005	Dave Butenhof	30 July 1992
 *		Fix oversight in cma__do_call_on_stack.
 *	006	Dave Butenhof	12 August 1992
 *		Try utilizing pal read/write unique functions for thread ID
 *	007	Dave Butenhof	 4 September 1992
 *		Add assembly interfaces to rt syscalls.
 *	008	Dave Butenhof	16 September 1992
 *		Remove documentation of context switch layout -- since we
 *		don't do any.
 *	009	Dave Butenhof	 2 October 1992
 *		Remove 007. Do it with syscall() instead.
 *	010	Dave Butenhof	 2 November 1992
 *		Clean up cma__do_interrupt.
 *	011	Dave Butenhof	 4 November 1992
 *		Remove hokey code to support return from cma__do_interrupt;
 *		I've convinced myself it's impractical to do that. Instead,
 *		add some _CMA_MP_HARDWARE_ support, including an explicit MB
 *		instruction, to allow reliable MP reading of target thread
 *		state (e.g., alert enable).
 *	012	Dave Butenhof	20 November 1992
 *		Add .align to all function declarations, and pass valid
 *		return PC to NESTED/LEAF macros. Maybe this will help dbx.
 *	013	Dave Butenhof	23 November 1992
 *		More attempts to help dbx.
 *	014	Dave Butenhof	 2 December 1992
 *		Add .prologue.
 *	015	Dave Butenhof	 5 January 1993
 *		Meaning of .prologue argument is to change for OSF/1 AXP
 *		BL12; current flag is ignored, so switch to BL12 meaning
 *		("1" means $gp is used).
 *	016	Brian Keane	19 January 1993
 *		Added cma__mach_fork().  This is a direct cop from
 *		libmach's fork, so we can do a linktime substitution of
 *		our cma_fork for fork.
 *	017	Brian Keane	3 March 1993
 *		Don't use a temporary to save sp in do_call_on_stack().
 *	018	Dave Butenhof	12 April 1993
 *		Remove functions that can be done in asm code in C.
 *	019	Dave Butenhof	27 April 1993
 *		Al Simons recommended reorganizing the 'already set' path in
 *		test_and_set to remove the mb (which shouldn't be relevant).
 *		Sounds like a good idea.
 *	020	Dave Butenhof	10 May 1993
 *		Restore un-asm code, since cc doesn't optimize our asm code
 *		correctly, based on _CMA_USE_ASM_.
 *	021	Brian Keane	03 August 1993
 *		Restore sp prior to jumping to _cerror on fork syscall failure.
 */

#ifdef __LANGUAGE_C__
# undef __LANGUAGE_C__
#endif

#ifndef __LANGUAGE_ASSEMBLY__
# define __LANGUAGE_ASSEMBLY__
#endif

#include <syscall.h>
#include <machine/regdef.h>
#include <machine/asm.h>
#include <machine/inst.h>
#include <machine/pal.h>
#include <sys/habitat.h>
#include <sys/rt_syscall.h>
#include <cma_config.h>
#include <cma_defs.h>

	.sdata
	.align	3
	.align	0
$$3:
	.quad	$$4
	.data
	.align	2
	.align	0
$$4:
	.ascii	"%W%	(DEC OSF/1)	%G%\X00"

	.extern	cma__bugcheck
	.extern	cma__thread_base

	.globl	cma__c_default_ps
	.data	
	.align	3
	.align	0
cma__c_default_ps:
	.quad	12 : 1

	.data	
	.align	3
	.align	0
execute_bug:
	.ascii	"execute_thread: return from thread_base\X00"
	.align	3
	.align	0
interrupt_bug:
	.ascii	"do_interrupt: return from interrupt routine\X00"

	.text
	.align	2

/*
 * cma__do_break
 *
 * FUNCTIONAL DESCRIPTION:
 * 
 *	This routine causes a breakpoint. It's used to implement "set visible"
 *	on a VP implementation (not used for uniprocessors). cma__vp_interrupt
 *	is used to cause the target thread to execute this function.
 *
 * FORMAL PARAMETERS:
 * 
 *	none
 * 
 * IMPLICIT INPUTS:
 * 
 *	none
 * 
 * IMPLICIT OUTPUTS:
 * 
 *	none
 *
 * FUNCTION VALUE:
 * 
 *	none
 * 
 * SIDE EFFECTS:
 * 
 *	none
 */
    .align	4
LEAF (cma__do_break)
	call_pal	PAL_bpt
	RET
END(cma__do_break)

 /*
  * cma__do_call_on_stack
  *
  * FUNCTIONAL DESCRIPTION:
  * 
  *	This routine calls the specified routine on the specified stack,
  *	passing the specified parameter.
  *
  * FORMAL PARAMETERS:
  * 
  *	r16	Address of base of target stack
  *	r17	Address of routine entry point
  *	r18	Parameter to pass to routine
  * 
  * IMPLICIT INPUTS:
  * 
  *	none
  * 
  * IMPLICIT OUTPUTS:
  * 
  *	none
  *
  * FUNCTION VALUE:
  * 
  *	value of supplied routine
  * 
  * SIDE EFFECTS:
  * 
  *	none
  */
DOCALLFSZ = 4 * 8
    .align	4
NESTED (cma__do_call_on_stack, DOCALLFSZ, ra)
	ldgp	gp, 0(pv)
	lda	sp, -DOCALLFSZ(sp)	/*  Allocate frame */
	.mask	M_S1 | M_RA, -3*8
	stq	ra, 8(sp)
	stq	s1, 16(sp)
	.prologue 1
	mov	sp, s1			/*  Save current SP in saved register */
	mov	a0, sp			/*  Switch to target stack */
	mov	a2, a0			/*  Get argument */
	mov	a1, pv			/*  Get procedure descriptor */
	jsr	ra, (pv)		/*  Call client function */
	ldgp	gp, 0(ra)		/*  Restore gp */
	mov	s1, sp			/*  Restore normal stack */
	ldq	s1, 16(sp)
	ldq	ra, 8(sp)
	lda	sp, DOCALLFSZ(sp)
	RET
END(cma__do_call_on_stack)

/*
 * FUNCTIONAL DESCRIPTION:
 * 
 *	This routine is responsible for delivering an async call from the
 *	virtual processor interface cma__vp_interrupt. It's not actually
 *	called.
 *
 * FORMAL PARAMETERS:
 * 
 *	none
 * 
 * IMPLICIT INPUTS:
 * 
 *	a0	interrupt handler
 *	a1	interrupt handler argument
 * 
 * IMPLICIT OUTPUTS:
 * 
 *	None
 *
 * FUNCTION VALUE:
 * 
 *	None
 * 
 * SIDE EFFECTS:
 * 
 *	None
 */
    .align	4
DOINTRFSZ = 16
NESTED (cma__do_interrupt, DOINTRFSZ, ra)
	ldgp	gp, 0(pv)
	lda	sp, -DOINTRFSZ(sp)
	.mask	M_RA, -8
	stq	ra, 8(sp)
	.prologue 1
	bis	a0, zero, pv		/* Get routine to call */
	bis	a1, zero, a0		/* Get the argument to pass */
	jsr	ra, (pv)		/* Call it */
	/*
         * We shouldn't get here!
	 */
	ldgp	gp, 0(ra)		/* Restore gp */
	lda	a0, interrupt_bug	/*  Get bugcheck text */
	jsr	ra, cma__bugcheck	
	call_pal	PAL_halt	/* Should NEVER get here */
	ldq	ra, 8(sp)
	lda	sp, DOINTRFSZ(sp)
	RET
END (cma__do_interrupt)

 /*
  * cma__test_and_set
  *
  * FUNCTIONAL DESCRIPTION:
  * 
  *	This routine performs an atomic test-and-set operation on the data
  *	at a specified address. If the address is locked (currently 1), then
  *	a value of 1 is returned. If the address is currently free (0) then
  *	a value of 1 is written, and the value 0 is returned. If the STL_C
  *	instructions fail (because the lock has been cleared), then the lock
  *	is retried up to some limit.
  *
  * FORMAL PARAMETERS:
  * 
  *	r16	Address of an int on which to perform test-and-set function
  * 
  * IMPLICIT INPUTS:
  * 
  *	none
  * 
  * IMPLICIT OUTPUTS:
  * 
  *	none
  *
  * FUNCTION VALUE:
  * 
  *	r0 == 0 if word was previously clear, 1 if previously set.
  * 
  * SIDE EFFECTS:
  * 
  *	Sets 0(r16) to 1.
  */
    .align	4
LEAF (cma__test_and_set)
$$10:	ldl_l	$0, ($16)	/*  Load flag value, and set CPU lock bit */
	blbs	$0, $$20	/*  If lock was set, return 1 */
	or	$0, 1, $1	/*  Set new lock value */
	stl_c	$1, ($16)	/*  Write it out */
	beq	$1, $$30	/*  If lock was already clear, retry */
#if _CMA_MP_HARDWARE_
	mb
#endif
$$20:
	RET			/*  Done! */
$$30:	br	$31, $$10	/*  For now, loop forever if necessary */
END(cma__test_and_set)

    .align	4

#ifndef _CMA_USE_ASM_
 /*
  * cma__fetch_fp
  *
  * FUNCTIONAL DESCRIPTION:
  * 
  *	This function returns the caller's fp
  *
  * FORMAL PARAMETERS:
  * 
  *	none
  * 
  * IMPLICIT INPUTS:
  * 
  *	none
  * 
  * IMPLICIT OUTPUTS:
  * 
  *	none
  *
  * FUNCTION VALUE:
  * 
  *	r0 == the fp of the caller's frame
  * 
  * SIDE EFFECTS:
  * 
  *	none
  */
    .align	4
LEAF (cma__fetch_fp)
	mov	$29, $0		/*  Get caller's fp */
	RET
END (cma__fetch_fp)

 /*
  * cma__fetch_gp
  *
  * FUNCTIONAL DESCRIPTION:
  * 
  *	This function returns the caller's gp
  *
  * FORMAL PARAMETERS:
  * 
  *	none
  * 
  * IMPLICIT INPUTS:
  * 
  *	none
  * 
  * IMPLICIT OUTPUTS:
  * 
  *	none
  *
  * FUNCTION VALUE:
  * 
  *	r0 == the gp of the caller's frame
  * 
  * SIDE EFFECTS:
  * 
  *	none
  */
    .align	4
LEAF (cma__fetch_gp)
	mov	$29, $0		/*  Get caller's gp */
	RET			/*  Done! */
END (cma__fetch_gp)

 /*
  * cma__fetch_sp
  *
  * FUNCTIONAL DESCRIPTION:
  * 
  *	This function returns the caller's sp
  *
  * FORMAL PARAMETERS:
  * 
  *	none
  * 
  * IMPLICIT INPUTS:
  * 
  *	none
  * 
  * IMPLICIT OUTPUTS:
  * 
  *	none
  *
  * FUNCTION VALUE:
  * 
  *	r0 == the sp of the caller's frame
  * 
  * SIDE EFFECTS:
  * 
  *	none
  */
    .align	4
LEAF (cma__fetch_sp)
	mov	sp, $0		/*  Get caller's sp */
	RET			/*  Done! */
END (cma__fetch_sp)

 /*
  * cma__thread_get_self_tcb
  *
  * FUNCTIONAL DESCRIPTION:
  * 
  *	This function returns the caller's PAL code unique value
  *
  * FORMAL PARAMETERS:
  * 
  *	none
  * 
  * IMPLICIT INPUTS:
  * 
  *	none
  * 
  * IMPLICIT OUTPUTS:
  * 
  *	none
  *
  * FUNCTION VALUE:
  * 
  *	r0 == the unique value for the current thread.
  * 
  * SIDE EFFECTS:
  * 
  *	none
  */
    .align	4
LEAF (cma__get_self_tcb)
	call_pal	PAL_rduniq
	RET
END (cma__get_self_tcb)

 /*
  * cma__set_unique
  *
  * FUNCTIONAL DESCRIPTION:
  * 
  *	This function sets the caller's PAL code unique value
  *
  * FORMAL PARAMETERS:
  * 
  *	r16	the unique value for the current thread.
  * 
  * IMPLICIT INPUTS:
  * 
  *	none
  * 
  * IMPLICIT OUTPUTS:
  * 
  *	none
  *
  * FUNCTION VALUE:
  * 
  *	none
  * 
  * SIDE EFFECTS:
  * 
  *	none
  */
    .align	4
LEAF (cma__set_unique)
	call_pal	PAL_wruniq
	RET
END (cma__set_unique)

 /*
  * cma__memory_barrier
  *
  * FUNCTIONAL DESCRIPTION:
  * 
  *	This function causes a hardware memory barrier (only called on true
  *	MP hardware).
  *
  * FORMAL PARAMETERS:
  * 
  *	none
  * 
  * IMPLICIT INPUTS:
  * 
  *	none
  * 
  * IMPLICIT OUTPUTS:
  * 
  *	none
  *
  * FUNCTION VALUE:
  * 
  *	none
  * 
  * SIDE EFFECTS:
  * 
  *	none
  */
    .align	4
LEAF (cma__memory_barrier)
	mb
	RET			/*  Done! */
END (cma__memory_barrier)
#endif					/* ndef _CMA_USE_ASM_ */

 /*
  * cma__mach_fork
  *
  * FUNCTIONAL DESCRIPTION:
  *
  *     This function performs the system fork call.  It is included here so
  *     that we can let our cma_fork() routine masquerade as the real fork.
  *     This routine was adapted from libmach, and is safe for static and
  *     shared library work.  Notice it isn't a leaf - that because mach_init
  *     must be called in the child.
  *
  * FORMAL PARAMETERS:
  *
  *     none
  *
  * IMPLICIT INPUTS:
  *
  *     none
  *
  * IMPLICIT OUTPUTS:
  *
  *     none
  *
  * FUNCTION VALUE:
  *
  *     none
  *
  * SIDE EFFECTS:
  *
  *     none
  */

FRMSIZE = 16
IMPORT(task_self_, 8) 
NESTED(cma__mach_fork, FRMSIZE, ra)
        ldgp    gp, 0(pv)
        lda     sp, -FRMSIZE(sp)        # need some temp stack space
        stq     ra, (sp)

        ldiq    v0, SYS_fork
        CHMK()
        bne     a3, err
        beq     a4, parent
				stq zero, task_self_
        CALL(mach_init)
        ldiq    v0, 0

parent:
        ldq     ra, (sp)
        lda     sp, FRMSIZE(sp)         # restore sp..
        RET                             # pid = fork()

err:
        lda     sp, FRMSIZE(sp)         # restore sp..
        jmp     zero, _cerror

END(cma__mach_fork)

/*  VAX/DEC CMS REPLACEMENT HISTORY, Element CMA_ASSEM.S */
/*  *24    4-AUG-1993 16:27:51 KEANE "Fix mach fork code error path" */
/*  *23   10-MAY-1993 14:10:37 BUTENHOF "Conditionalize asm()-able code" */
/*  *22   27-APR-1993 12:12:45 BUTENHOF "Speed up test-and-set" */
/*  *21   16-APR-1993 13:07:19 BUTENHOF "try asm statement" */
/*  *20    3-MAR-1993 15:24:11 KEANE "Don't use a temp register to save sp in cma__do_call_on_stack()" */
/*  *19   19-JAN-1993 16:12:33 KEANE "Forgot to include syscall.h" */
/*  *18   19-JAN-1993 15:54:28 KEANE "Added cma__mach_fork" */
/*  *17    5-JAN-1993 08:52:57 BUTENHOF "Change to BL12 .prologue" */
/*  *16   11-DEC-1992 10:11:33 BUTENHOF "More dbx info" */
/*  *15    2-DEC-1992 13:28:34 BUTENHOF "Add .prologue" */
/*  *14   23-NOV-1992 14:00:32 BUTENHOF "Fix prologs again" */
/*  *13   20-NOV-1992 11:20:09 BUTENHOF "Fix routine decls" */
/*  *12    5-NOV-1992 14:25:31 BUTENHOF "cleanup" */
/*  *11    2-NOV-1992 13:25:58 BUTENHOF "Fix do_interrupt" */
/*  *10    2-OCT-1992 16:12:47 BUTENHOF "Remove assembly code sched stuff" */
/*  *9    16-SEP-1992 11:36:56 BUTENHOF "Fix comments" */
/*  *8    16-SEP-1992 10:08:21 BUTENHOF "Add syscall interface for rt sched" */
/*  *7    13-AUG-1992 14:45:12 BUTENHOF "Add read/write unique" */
/*  *6     4-AUG-1992 11:18:31 BUTENHOF "Fix do_call_on_stack" */
/*  *5    28-JUL-1992 13:19:45 BUTENHOF "Implement do_interrupt" */
/*  *4    23-JUL-1992 17:15:06 KEANE "General cleanup, removed some unused code, add some pal calls" */
/*  *3    15-JUN-1992 16:53:14 KEANE "Add (at least minimally) some missing entry points" */
/*  *2     1-APR-1992 14:28:15 BUTENHOF "Remove unnecessary header" */
/*  *1     1-APR-1992 13:32:53 BUTENHOF "Alpha OSF/1 assembly code" */
/*  VAX/DEC CMS REPLACEMENT HISTORY, Element CMA_ASSEM.S */
