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
#ifndef lint
static char *rcsid = "@(#)$RCSfile: fp_emulator.c,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1992/12/01 16:37:34 $";
#endif

/* this file launches the emulation process and handles exceptions due
 *	to emulation.
 *
 */

#ifndef KERNEL
#include <stdio.h>
#include <setjmp.h>
#else /* KERNEL */
#include <machine/pcb.h>
#endif /* KERNEL */
#include <sys/types.h>
#include <sys/buf.h>
#include <machine/trap.h>
#include <mach/machine/exception.h>
#include <machine/reg.h>
#include <machine/softfp.h>
#include <sys/signal.h>
#include <machine/inst.h>
#include <arch/alpha/fpu.h>
#include <arch/alpha/local_ieee.h>

#define MODULE_NAME	emulator

/* Interface compatibility macros -- so the some of the source can look the same
 *	between kernel and user land.
 */


#define SFP_FIRST_BRANCH_OP	op_br
#define SFP_LAST_BRANCH_OP	op_bgt


#ifndef KERNEL

static void
emulator_handler(signal, code, scp)
struct sigcontext	*scp;
{
    /* We have just come from a small emulation routine which caused
     *	a SIGFPE. The caller of that routine has placed the pointer
     *	to a place for the exception summary and a jmpbuf in argument
     *	registers for the third and forth arguments.
     *
     * We will take the exception summary from the sigcontext, stuff it into
     *	the place the caller of the emulation routine pointed us to and
     *	longjmp back to where we were told.
     */
    excsum_t		*pexcsum;

#define SCALAR_ARG_REGISTER_2		18	/* count from 0 */
#define SCALAR_RETURN_VALUE_REGISTER	0

    DPRINTF(("emulator_handler: signal=%d(%d) scp=0x%lx\n", signal, code, scp));
    DPRINTF(("emulator_handler: pc=0x%lx ra=0x%lx\n", scp->sc_pc, scp->sc_regs[26]));

    if (signal != SIGFPE) {

	EPRINTF(("unexpected signal %d\n", signal));

	return;
    } /* if */

    /* retrieve pointers */
    pexcsum = (excsum_t	 *)scp->sc_regs[SCALAR_ARG_REGISTER_2];

    /* set exception summary */
    pexcsum->qval = scp->sc_traparg_a0;

    /* we don't care about result, just continue */
    scp->sc_pc += 4;

    return;
} /* emulator_handler */

#else /* KERNEL */
	/*
	 * The kernel version of emulator_handler is in trap.c and the
	 * corresponding nofault handler in arch/alpha/nofault.s
	 */
#endif /* KERNEL */

unsigned long
alpha_emulate_fp_instruction(
	float_entry		*float_entry,
	fp_register_t		fa,
	fp_register_t		fb,
	fp_register_t		*pfc, /* result if it returns */
	excsum_t		*pexcsum)/* points to where to put excsum */
{
    u_long			result;		/* for this routine */
    fp_register_t		dummy;
    fpcr_t			fpcr;
#ifndef KERNEL
    sigset_t			sigset_i;	/* to set sigs we block */
    sigset_t			sigset_o;	/* hold original value */
    struct sigstack		sigstack_i;	/* to set alt signal stack */
    struct sigstack		sigstack_o;	/* hold original value */
    struct sigaction		sigaction_i;	/* to set SIGFPE handler */
    struct sigaction		sigaction_o;	/* hold original value */
    char			emulator_stack_buffer[4096];
    char			*emulator_stack;/* alternate sigstack */
    jmp_buf			emulator_jmpbuf;/* to come back after 	*/
						/* emulation causes sigfpe */


    /* disable signals except for SIGFPE, since we are about to
     *	generate one.
     */
#define sigfillset(set)         { *(set) = ~(sigset_t)0; }
#define sigdelset(set, signo)   ( *(set) &= ~(1 << ((signo) - 1)), 0)
    sigfillset(&sigset_i);
    sigdelset(&sigset_i, SIGFPE);
    if (sigprocmask(SIG_SETMASK, &sigset_i, &sigset_o)) {
	DPRINTF(("sigprocmask failed during emulation setup\n"));
	return SFP_FAILURE;
    } /* if */

    /* set up alternate stack. */

    /* octword align emulator stack pointer */
    emulator_stack = emulator_stack_buffer;
    emulator_stack += sizeof(emulator_stack_buffer);	/* point to top */
    emulator_stack = (char *)(((u_long)emulator_stack) & ~15);
    sigstack_i.ss_sp = (caddr_t)emulator_stack;
    sigstack_i.ss_onstack = 0;
    if (sigstack (&sigstack_i, &sigstack_o)) {
	DPRINTF(("sigstack failed during emulation setup\n"));
	return SFP_FAILURE;
    } /* if */

    /* setup sigaction for SIGFPE */
    sigaction_i.sa_handler = emulator_handler;
    sigfillset(&sigaction_i.sa_mask);	/* stop all signals on entry */
    sigaction_i.sa_flags = SA_ONSTACK;	/* run on alternate stack */
    if (sigaction (SIGFPE, &sigaction_i, &sigaction_o)) {
	DPRINTF(("sigaction failed during emulation setup\n"));
	result = SFP_FAILURE;
	goto cleanup;
    } /* if */
#else /* KERNEL */

    /*
     * The following code will generate a floating point fault so we set
     * up a nofault handler.
     */

     current_pcb->pcb_nofault =  NF_SOFTFP;

#endif /* KERNEL */

    /* emulate a floating point instruction and return either a result
     *	value or an exception summary.
     */
    if (pfc == 0) {
	/* caller doesn't care about result */
	pfc = &dummy;
    } /* if */

    pexcsum->qval = 0;	/* initialize */

    result = SFP_SUCCESS;

    /* we need to get the operands, we don't care the instruction
     *	uses them all or not, we're safe to send Fa and Fb. Result
     *	will always be in $r0 but we shouldn't return.
     */
    
    DPRINTF(("about to emulate fp instruction:\t%s @ 0x%lx\n", float_entry->string, float_entry->routine));
    DPRINTF(("\toperand1 = %e(0x%lx),\n", fa.dval, fa.qval));
    DPRINTF(("\toperand2 = %e(0x%lx)\n", fb.dval, fb.qval));

    fpcr.qval = _get_fpcr();
    DPRINTF(("before finite operand emulation fpcr= "));
    DPRINT_FPCR(fpcr);
    DPRINTF(("\n"));

    /* callee only expects first two arguments, third argument (reg a2) is for
     * the nofault handler (in kernel mode) or the emulator_handler() to set the
     * exception summary.
     */
    pfc->dval = (*float_entry->routine)(fa.dval, fb.dval, pexcsum);

    DPRINTF(("return from emulated instruction, result = %e(0x%lx)\n", pfc->dval, pfc->qval));

    DPRINTF(("returned from emulation, exsum = "));
    DPRINT_EXCSUM(*pexcsum);
    DPRINTC(("\n"));
    if (pexcsum->qval != 0) {
	result = SFP_FAILURE;
    } /* if */

    /*
     * cleanup signal stuff and return status
     */
cleanup:

#ifndef KERNEL
    if (sigprocmask(SIG_SETMASK, &sigset_o, 0)) {
	DPRINTF(("sigprocmask failed during emulation cleanup\n"));
	result = SFP_FAILURE;
    } /* if */

    if (sigstack (&sigstack_o, 0)) {
	DPRINTF(("sigstack failed during emulation cleanup\n"));
	result = SFP_FAILURE;
    } /* if */

    if (sigaction (SIGFPE, &sigaction_o, 0)) {
	DPRINTF(("sigaction failed during emulation cleanup\n"));
	result = SFP_FAILURE;
    } /* if */
#else /* KERNEL */
    current_pcb->pcb_nofault = 0;
#endif /* KERNEL */ 
    return result;

} /* alpha_emulate_fp_instruction */
