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
static char *rcsid = "@(#)$RCSfile: fp_ieee_handler.c,v $ $Revision: 1.1.6.3 $ (DEC) $Date: 1993/12/09 20:29:58 $";
#endif

#include <sys/types.h>
#include <sys/buf.h>
#include <machine/trap.h>
#include <mach/machine/exception.h>
#include <machine/reg.h>
#include <machine/softfp.h>
#ifndef KERNEL
#include <stdio.h>
#include <setjmp.h>
#endif
#include <sys/signal.h>
#include <sys/user.h>
#include <machine/inst.h>
#include <arch/alpha/emulate.h>
#include <arch/alpha/fpu.h>
#include <arch/alpha/local_ieee.h>

#define BITMASK(x) ((1 << (x)) - 1)

#define MODULE_NAME	ieee_handler

#ifdef FP_CONTROL_IN_SCP

#define SCP_FP_CONTROL	scp->sc_fp_control

#else /* FP_CONTROL_IN_SCP */

#ifndef KERNEL
static ieee_fp_control_t	fp_control;
#define SCP_FP_CONTROL	fp_control
#endif

#endif /* FP_CONTROL_IN_SCP */

#ifndef KERNEL

ieee_handler(signal_number, code, scp)
struct sigcontext	*scp;
{
    struct alpha_fp_trap_context	fp_context;

    DPRINTF(("signal_number,code(%d,%d) 0x%lx\n", signal_number, code, scp->sc_pc));

    if (signal_number != SIGFPE) {
	    EPRINTF(("unexpected signal %d\n", signal));
    } /* if */

#ifndef KERNEL_FP_CONTROL
    /* if no kernel support for fp control get it from userland copy */
    SCP_FP_CONTROL.qval = ieee_get_fp_control();
#endif

    /* set up */
    memset(&fp_context, 0, sizeof(fp_context));
    fp_context.fpcr.qval = scp->sc_fpcr;
    fp_context.isolated_excsum.qval = scp->sc_traparg_a0;
    fp_context.regmask = scp->sc_traparg_a1;
    fp_context.pc = (union alpha_instruction *)scp->sc_pc;

    DPRINTF(("original exception_summary: "));
    DPRINT_EXCSUM(fp_context.isolated_excsum);
    DPRINTF(("fp regmask = %lx\n", fp_context.regmask));

    DPRINTF(("original fpcr= "));
    DPRINT_FPCR(scp->sc_fpcr);
    DPRINTC(("\n"));


    if (fp_set_trigger_information(&fp_context, 
	(fp_register_t *)scp->sc_fpregs) != SFP_SUCCESS) {

	/* if invalid shadow raise a signal_number always regardless of trap
	 *	settings.
	 */
	DPRINTF(("cannot get trigger info... cause sigfpe\n"));
	signal(SIGFPE, SIG_DFL);
	kill(getpid(), SIGFPE);

    } /* if */
    /* move trigger info to sc context */

    DPRINTF(("op1 = %e (0x%016lx)\n", fp_context.original_operand1.dval, fp_context.original_operand1.qval));
    DPRINTF(("op2 = %e (0x%016lx)\n", fp_context.original_operand2.dval, fp_context.original_operand2.qval));
    DPRINTF(("original result = %e (0x%016lx)\n", fp_context.result.dval, fp_context.result.qval));


    if (fp_context.inst.f_format.opcode != op_flti) {
	DPRINTF(("not ieee instruction, return\n"));
	signal(SIGFPE, SIG_DFL);
	kill(getpid(), SIGFPE);
	return;
    } /* if */

doit_again:

    if (ieee_default_value(&fp_context) != SFP_SUCCESS) {
	EPRINTF(("cannot produce ieee default value, treat like invalid shadow\n"));
	signal(SIGFPE, SIG_DFL);
	kill(getpid(), SIGFPE);
    } /* if */

    DPRINTF(("intermediate ieee fp_control: "));
    DPRINT_FP_CONTROL(fp_context.fp_control);
    DPRINTC(("\n"));

    DPRINTF(("intermediate ieee exception_summary= "));
    DPRINT_EXCSUM(fp_context.isolated_excsum);
    DPRINTC(("\n"));

    if (fp_context.isolated_excsum.qval&EXCSUM_MASK) {

	/* resolving one exception led to another or there were two, 
	 *	so we do it again.
	 */


	goto doit_again;
    } /* if */

    fp_context.actual_excsum.qval = 
     (fp_context.fp_control.qval&IEEE_STATUS_MASK)>>IEEE_STATUS_TO_EXCSUM_SHIFT;

    DPRINTF(("final ieee fp_control: "));
    DPRINT_FP_CONTROL(fp_context.fp_control);
    DPRINTC(("\n"));

    DPRINTF(("actual ieee exception_summary= "));
    DPRINT_EXCSUM(fp_context.actual_excsum);
    DPRINTC(("\n"));

    /* set result */
    DPRINTF(("setting register $f%d to %e(0x%016lx)\n", fp_context.inst.f_format.fc, fp_context.result.dval, fp_context.result.qval));
    scp->sc_fpregs[fp_context.inst.f_format.fc] = fp_context.result.qval;

    /* call fixup to detect invalid and provide correct value */
    if (IEEE_TRAP_HIT(fp_context.actual_excsum, SCP_FP_CONTROL)) {

	DPRINTF(("got a trap the user wanted to handle, raise signal\n"));

	signal(SIGFPE, SIG_DFL);
	kill(getpid(), SIGFPE);

    } /* if */

    /* no handler, set sticky bits from our temporary fp_control to permanent */
    SCP_FP_CONTROL.qval |= (fp_context.fp_control.qval&IEEE_STATUS_MASK);
#ifndef KERNEL_FP_CONTROL
    ieee_set_fp_control(SCP_FP_CONTROL.qval);
#endif

    DPRINTF(("actual ieee fp_control: "));
    DPRINT_FP_CONTROL(SCP_FP_CONTROL);
    DPRINTC(("\n"));

    scp->sc_pc = (unsigned long)fp_context.pc + sizeof(union alpha_instruction);
    sigreturn(scp);

} /* ieee_handler */

#else /* KERNEL */

/* used when flaging cvtqlsv that has overflowed and should be signaled as 
   FPE_INTOVF_FAULT */
unsigned int cvtqlsv_flag; 

ieee_handler(traparg_a0, traparg_a1, exc_frame)
ulong_t traparg_a0, traparg_a1;
ulong_t	*exc_frame;
{
    struct alpha_fp_trap_context        fp_context;
    ulong_t freg_save_area[32];
    long retval = 0;
    long set_imprecise_error_return();
    long set_precise_error_return();
    cvtqlsv_flag = 0;

    /* set up */
    bzero(&fp_context, sizeof(fp_context));
    fp_context.fpcr.qval = _get_fpcr();
    fp_context.isolated_excsum.qval = traparg_a0;
    fp_context.regmask = traparg_a1;
    fp_context.pc = (union alpha_instruction *)exc_frame[EF_PC];

    freg_save(&freg_save_area[0]);

    DPRINTF(("original exception_summary: "));
    DPRINT_EXCSUM(fp_context.isolated_excsum);
    DPRINTF(("fp regmask = %lx\n", fp_context.regmask));

    DPRINTF(("original fpcr= "));
    DPRINT_FPCR(fp_context.fpcr.qval);
    DPRINTC(("\n"));

    if (retval = fp_set_trigger_information(&fp_context, 
	(fp_register_t *)freg_save_area) != SFP_SUCCESS) {

	/* invalid shadow, raise a signal regardless of trap settings. */
	DPRINTF(("cannot get trigger info... cause sigfpe\n"));
	freg_restore(&freg_save_area[0]);
	return(set_imprecise_error_return(retval, traparg_a0));
    } /* if */

    /* move trigger info to sc context */
    DPRINTF(("op1 = %e (0x%016lx)\n",
	fp_context.original_operand1.dval, fp_context.original_operand1.qval));
    DPRINTF(("op2 = %e (0x%016lx)\n",
	fp_context.original_operand2.dval, fp_context.original_operand2.qval));
    DPRINTF(("original result = %e (0x%016lx)\n",
	fp_context.result.dval, fp_context.result.qval));

   /*
    *  cvtqlsv is a special case where we are signalling floating overflow
    *  even though the instruction cvtqlsv isn't considered an ieee floating 
    *  point instruction. Because cvtqlsv will normally be used in converting 
    *  floating point values into 32-bit integers, it makes sense to treat it 
    *  as a floating overflow.
    */
   if (fp_context.inst.f_format.opcode == op_fltl 
         && fp_context.pfloat_entry->function_code == fltl_cvtqlsv
         && fp_context.isolated_excsum.fields.iov) {
       DPRINTF(("Overflowing fltl_cvtqlsv instruction\n"));
       fp_context.isolated_excsum.fields.inv = 1;
       fp_context.actual_excsum = fp_context.isolated_excsum;
       cvtqlsv_flag = 1;
       goto cvtqlsv_case;
     }

    if (fp_context.inst.f_format.opcode != op_flti) {
	DPRINTF(("not ieee instruction, return\n"));
	freg_restore(&freg_save_area[0]);
	return(set_imprecise_error_return(FPE_ILLEGAL_SHADOW_TRAP, traparg_a0));
    } /* if */

doit_again:

    if (ieee_default_value(&fp_context) != SFP_SUCCESS) {
	EPRINTF(("can't produce ieee default value, treat like invalid shadow\n"));
	freg_restore(&freg_save_area[0]);
	return(set_imprecise_error_return(FPE_ILLEGAL_SHADOW_TRAP, traparg_a0));
    } /* if */

    DPRINTF(("intermediate ieee fp_control: "));
    DPRINT_FP_CONTROL(fp_context.fp_control);
    DPRINTC(("\n"));

    DPRINTF(("intermediate ieee exception_summary= "));
    DPRINT_EXCSUM(fp_context.isolated_excsum);
    DPRINTC(("\n"));

    if (fp_context.isolated_excsum.qval&EXCSUM_MASK) {

	/* resolving one exception led to another or there were two, 
	 *	so we do it again.
	 */

	goto doit_again;
    } /* if */

    fp_context.actual_excsum.qval = 
     (fp_context.fp_control.qval&IEEE_STATUS_MASK)>>IEEE_STATUS_TO_EXCSUM_SHIFT;

    DPRINTF(("final ieee fp_control: "));
    DPRINT_FP_CONTROL(fp_context.fp_control);
    DPRINTC(("\n"));

    DPRINTF(("actual ieee exception_summary= "));
    DPRINT_EXCSUM(fp_context.actual_excsum);
    DPRINTC(("\n"));

  cvtqlsv_case:

    /* set result */
    DPRINTF(("setting register $f%d to %e(0x%016lx)\n",
	fp_context.inst.f_format.fc, fp_context.result.dval,
	fp_context.result.qval));

    freg_save_area[fp_context.inst.f_format.fc] = fp_context.result.qval;

    /* set result to zero on underflows if user requests */
    if ((fp_context.fp_control.qval&IEEE_STATUS_UNF) && (u.u_ieee_fp_control & IEEE_MAP_UMZ)){
        freg_save_area[fp_context.inst.f_format.fc] = IEEE_PLUS_ZERO;
	fp_context.fp_control.qval |= IEEE_STATUS_INE;
    }

    /* call fixup to detect invalid and provide correct value */
    if (IEEE_TRAP_HIT(fp_context.actual_excsum, *(ieee_fp_control_t *)&u.u_ieee_fp_control)) {

	DPRINTF(("got a trap the user wanted to handle, raise signal\n"));

	freg_restore(&freg_save_area[0]);
	return(set_precise_error_return(&fp_context));
    } /* if */

    /* set sticky bits from our temporary fp_control to permanent */
    u.u_ieee_fp_control |= (fp_context.fp_control.qval&IEEE_STATUS_MASK);
    ieee_set_fp_control(u.u_ieee_fp_control);

    DPRINTF(("actual ieee fp_control: "));
    DPRINT_FP_CONTROL(u.u_ieee_fp_control);
    DPRINTC(("\n\n"));

    exc_frame[EF_PC] = (unsigned long)fp_context.pc +
						sizeof(union alpha_instruction);
    freg_restore(&freg_save_area[0]);
    return(0);
}

/*
 * set up returns for imprecise exceptions.
 */
long
set_imprecise_error_return(code, traparg_a0)
long code, traparg_a0;
{
    if (code != FPE_HPARITH_TRAP)
	code = FPE_ILLEGAL_SHADOW_TRAP;

    u.u_ieee_fp_trigger_inst = 0;
    u.u_ieee_fp_trigger_sum = traparg_a0 & EXCSUM_SWC;
    return (code);
}

/*
 * set up returns for precise exceptions.
 */
long
set_precise_error_return(fp_context)
struct alpha_fp_trap_context        *fp_context;
{
    long code = 0;
    
    /* check for cvtqlsv special case first */
    if ((fp_context->actual_excsum.qval & EXCSUM_IOV) && (cvtqlsv_flag))
        code = FPE_INTOVF_FAULT;
    else {
    if (fp_context->actual_excsum.qval & EXCSUM_OVF)
	code = FPE_FLTOVF_FAULT;
    else if (fp_context->actual_excsum.qval & EXCSUM_INV)
	code = FPE_INVALID_FAULT;
    else if (fp_context->actual_excsum.qval & EXCSUM_DZE)
	code = FPE_FLTDIV_FAULT;
    else if (fp_context->actual_excsum.qval & EXCSUM_UNF)
	code = FPE_FLTUND_FAULT;
    else if (fp_context->actual_excsum.qval & EXCSUM_INE)
	code = FPE_INEXACT_FAULT;
    }

    if (code) {
	    u.u_ieee_fp_trigger_inst = (u_int)fp_context->inst.word;
	    u.u_ieee_fp_trigger_sum = fp_context->actual_excsum.qval;
	    u.u_ieee_fp_trap_pc = (ulong_t)fp_context->pc;
	    return (code);
    }
    return (SFP_FAILURE);
}
#endif /* KERNEL */
