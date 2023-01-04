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
static char *rcsid = "@(#)$RCSfile: fp_trigger.c,v $ $Revision: 1.1.5.2 $ (DEC) $Date: 1993/10/13 19:51:42 $";
#endif

/* this file has nothing to do with roy roger's horse.  
 *	it contains routines which will determine the exception pc and
 *	summary.
 *
 */

#ifndef KERNEL
#include <stdio.h>
#include <setjmp.h>
#endif
#include <sys/types.h>
#include <sys/buf.h>
#include <sys/user.h>
#include <machine/trap.h>
#include <mach/machine/exception.h>
#include <machine/reg.h>
#include <machine/softfp.h>
#include <sys/signal.h>
#include <machine/inst.h>
#include <arch/alpha/fpu.h>
#include <arch/alpha/local_ieee.h>

#define MODULE_NAME	set_trigger_info

#define SFP_FIRST_BRANCH_OP	op_br
#define SFP_LAST_BRANCH_OP	op_bgt

#ifdef DEBUG

#ifndef KERNEL

static int	dis_word;
static int
get_word()
{
    /* for diassembler */
    return dis_word;
} /* get_word */
#endif /* KERNEL */

#endif /* DEBUG */

unsigned long
fp_set_trigger_information (
    alpha_fp_trap_context	*pfp_context,
    fp_register_t		*fpregs)
{
    /*
     * Due to pipe-lining, when the kernel gets an fp exception, the trap pc 
     * does not point to the instruction that caused the error.  If the 
     * generated user follows the conventions specified in the SRM, it's 
     * possible to walk back through the "trap shadow" and find the 
     * offending instruction. 
     *
     * We must also isolate the the exception if more than one is reported 
     * in the summary. We do this by emulating the instruction immediately
     * followed by a trapb.
     *
     * We do not modify the incoming state, we only modify the variables
     *	in the fp_context and we expect the caller to decide what to
     *	do with the information.
     */
    union alpha_instruction	fp;
    ulong_t			cur_pc;	/* pc for backward traverse */
    int				back;	/* used as back searcching threshold */

    /* pal generated exception information put into a0 and a1 */
    excsum_t			exsum;		/* exception summary */
    unsigned			reg_mask_copy;	/* only the fp regs */
    uint_t			dest_reg_mask=0;/* check for invalid shadow */

    reg_mask_copy = pfp_context->regmask >> 32;	/* only fp regs */
    exsum = pfp_context->isolated_excsum;

    if ((exsum.qval&EXCSUM_MASK) == 0) {
	    DPRINTF(("non arithmetic fault\n"));
	    return(FPE_HPARITH_TRAP);
    }

    cur_pc = (unsigned long)pfp_context->pc;

    if ((exsum.fields.software_completion) == 0) {
	    /* no fixup possible. pc detection impossible */
	    pfp_context->invalid_shadow = 1;
	    DPRINTF(("SWC bit not set in summary register\n"));
	    return (FPE_HPARITH_TRAP);
    }

    /*
     * Walk back through the trap shadow.  When we get to a trapb (draint)
     * we know that we are at the end.  If a trapb is not found before
     * a branch is detected, then stop there because the code is not
     * following the convention that allows a walkback of the trap shadow.
     * If, during the walkback, a destination register was used was
     * previously used as an input register that walkback convention
     * was not followed and it will be impossible to fix up the intruction.
     * The best we can do here is find an accurate pc.
     */

    /* TODO instead of putting on a threshold, we should useracc the addr */
    for (back=25; back>0; back--, cur_pc -= 4) {

#ifndef KERNEL
	fp.word = *(unsigned int *)(cur_pc);
#else /*KERNEL */
	fp.word = fuiword(cur_pc);
#endif /* KERNEL */

#ifdef DEBUG
	DPRINTF(("0x%016lx	", cur_pc));
#ifdef KERNEL
	DPRINTC(("0x%016lx\n", fp.word));
#else /* KERNEL */
	dis_word = fp.word;
	if (!_ieee_skip_debug)
	    disassembler(cur_pc, 0, 0, 0, get_word, 0);
#endif /* KERNEL */
#endif	/* DEBUG */

	/* trapb marks the end of the trap shadow */
	if ((fp.common.opcode == op_misc) &&
	    (fp.m_format.memory_displacement == misc_trapb) &&
	    (cur_pc != (unsigned long)pfp_context->pc)) {
	    pfp_context->invalid_shadow = 1;
	    DPRINTF(("trapb detected in trap shadow\n"));
	    return SFP_FAILURE;
	}

	/*
	 * If this is a branch instruction, we have an invalid
	 * trap shadow.
	 */
	if ((fp.common.opcode == op_jsr) ||
	    ((fp.common.opcode >= SFP_FIRST_BRANCH_OP) &&
	     (fp.common.opcode <= SFP_LAST_BRANCH_OP))) {

	    /* TODO check if palcode should end shadow */
	    pfp_context->invalid_shadow = 1;
	    DPRINTF(("Branch detected in trap shadow\n"));
	    return SFP_FAILURE;
	}

	/* test only floating point instructions */
	if ((fp.common.opcode != op_flti)  &&
	    (fp.common.opcode != op_fltv) &&
	    (fp.common.opcode != op_fltl)) {
/* is this right can integer instructs set excsum? */
		continue;
	}

	/*
	 * was the same destination register used  more than once
	 * the trap shadow.
	 */

	if ((dest_reg_mask & (1 << fp.f_format.fc))) {
	    DPRINTF(("same register used as a destination"));
	    DPRINTF((" more than once in the trap shadow\n"));
	    DPRINTF(("reg = %x dest_reg_mask = %x cur_pc = %lx\n", fp.f_format.fc, dest_reg_mask, cur_pc));
	    pfp_context->invalid_shadow = 1;
	    return(SFP_FAILURE);
	}
	dest_reg_mask |= (1 << fp.f_format.fc);

	/*
	 * When all the destination registers in the Register
	 * Write Mask have been acounted for, we have our original
	 * faulting instruction.
	 */
	reg_mask_copy &= ~(1 << fp.f_format.fc);

	if (reg_mask_copy == 0)
	    /* found it! */
	    break;

    }

    if (back <= 0) {
	DPRINTF( ("instruction not found in trap shadow threshold\n"));
	pfp_context->invalid_shadow = 1;
	return(SFP_FAILURE);
    }

    DPRINTF(("hard faulting pc detected in trap shadow\n"));
    DPRINTF(("\t hard pc = %lx\n", cur_pc));
    pfp_context->pc = (union alpha_instruction *)cur_pc;
    pfp_context->inst = fp;


    /* lookup this opcode in the function_code specific table to locate
     *	the emulator routine and fill in for caller.
     */
    pfp_context->pfloat_entry = alpha_float_entry_lookup(fp.f_format.opcode, 
	fp.f_format.function, N, NONE);

    if (pfp_context->pfloat_entry == 0) {
	EPRINTF(("cannot find table entry for fp op\n"));
	return SFP_FAILURE;
    } /* if */

    /* we need to get the operands, we don't care the instruction
     *	uses them all or not, we're safe to send Fa and Fb and Fc result. 
     */
    pfp_context->result = fpregs[fp.f_format.fc];
    pfp_context->original_operand1 = fpregs[fp.f_format.fa];
    pfp_context->original_operand2 = fpregs[fp.f_format.fb];
    /* the following is a working copy used by the emulator */
    pfp_context->operand1 = fpregs[fp.f_format.fa];
    pfp_context->operand2 = fpregs[fp.f_format.fb];

    /* fetch dynamic rounding mode */
    pfp_context->rounding = pfp_context->pfloat_entry->rounding;
    if (pfp_context->pfloat_entry->rounding == D/*YNAMIC*/) {
	pfp_context->rounding = pfp_context->fpcr.fields.dynamic_rounding;
    } /* if */

    /* find exception summary */
    if (exsum.fields.invalid_operation+exsum.fields.divide_by_zero+
	exsum.fields.floating_overflow+exsum.fields.floating_underflow+
	exsum.fields.inexact_result+exsum.fields.integer_overflow <= 1) {

        return(SFP_SUCCESS);

    } /* if */

    /* ambiguous exception summary, we must emulate
     *		this must be made thread safe by disabling all other threads.
     */

    if (alpha_emulate_fp_instruction(pfp_context->pfloat_entry,
	pfp_context->operand1, pfp_context->operand2, &pfp_context->result,
	&pfp_context->isolated_excsum) != SFP_SUCCESS && 
	pfp_context->isolated_excsum.qval != 0) {

	DPRINTF(("isolated exception_summary: "));
	DPRINT_EXCSUM(pfp_context->isolated_excsum);
	DPRINTC(("\n"));
	return SFP_SUCCESS;

    } /* if */
    DPRINTF(("emulation does not produce exceptions\n"));

    return SFP_FAILURE;

} /* fp_set_trigger_information */

