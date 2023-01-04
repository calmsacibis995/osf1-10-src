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
static char *rcsid = "@(#)$RCSfile: alpha_unwind.c,v $ $Revision: 1.1.9.4 $ (DEC) $Date: 1993/07/07 20:49:39 $";
#endif
/*
 * |-----------------------------------------------------------|
 * | Copyright (c) 1991, 1990 MIPS Computer Systems, Inc.      |
 * | All Rights Reserved                                       |
 * |-----------------------------------------------------------|
 * |          Restricted Rights Legend                         |
 * | Use, duplication, or disclosure by the Government is      |
 * | subject to restrictions as set forth in                   |
 * | subparagraph (c)(1)(ii) of the Rights in Technical        |
 * | Data and Computer Software Clause of DFARS 252.227-7013.  |
 * |         MIPS Computer Systems, Inc.                       |
 * |         950 DeGuigne Avenue                               |
 * |         Sunnyvale, California 94088-3650, USA             |
 * |-----------------------------------------------------------|
 */
/* $Header */
/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#endif
#ifdef _DEBUG
static char *rcs_id="$Header: /usr/sde/osf1/rcs/os/src/usr/ccs/lib/libc/alpha/alpha_unwind.c,v 1.1.9.4 1993/07/07 20:49:39 Mark_Himelstein Exp $";
#endif

#include "excpt.h"
#include "cmplrs/synonyms.h"
#include <machine/inst.h>

#define FP		15			/* frame pointer */
#define SP		30			/* stack pointer */
#define INST_SIZE	sizeof(union alpha_instruction)


static pdsc_address
quad_at_address(pdsc_address	address)
{
    /* eventually if we want we could replace this by a settable call so
     *	apps like debuggers could use this unwind too.
     */
    return *(pdsc_address *)address;
} /* quad_at_address */


static union alpha_instruction
inst_at_pc(pdsc_address		address)
{
    /* hey it's only word-- it's ok to return it */
    union alpha_instruction	*pinst = (union alpha_instruction *) address;
    return *pinst;
} /* inst_at_pc */


static unsigned long
inst_sets_reg(union alpha_instruction	inst, pdsc_register	reg_arg)
{
    /* check if inst uses integer register as a destination */
    pdsc_register_value	reg;

    switch (inst.m_format.opcode) {

    case op_call_pal:		/* 0x00	 see pal.h for pal functions */
    case op_opc01:		/* 0x01 */
    case op_opc02:		/* 0x02 */
    case op_opc03:		/* 0x03 */
    case op_opc04:		/* 0x04 */
    case op_opc05:		/* 0x05 */
    case op_opc06:		/* 0x06 */
    case op_opc07:		/* 0x07 */
	return 0;

    case op_lda:		/* 0x08 */
    case op_ldah:		/* 0x09 */
	reg = inst.m_format.ra;
	break;

    case op_opc0a:		/* 0x0a */
	break;

    case op_ldq_u:		/* 0x0b */
	reg = inst.m_format.ra;
	break;

    case op_opc0c:		/* 0x0c */
    case op_opc0d:		/* 0x0d */
    case op_opc0e:		/* 0x0e */
    case op_stq_u:		/* 0x0f */
	break;

    case op_inta:		/* 0x10	 integer arithmetic group */
    case op_intl:		/* 0x11	 integer logical group */
    case op_ints:		/* 0x12	 integer multiply group */
    case op_intm:		/* 0x13	 integer shift group */
	reg = inst.o_format.rc;
	break;

    case op_opc14:		/* 0x14 */
    case op_fltv:		/* 0x15	 vax floating point group */
    case op_flti:		/* 0x16	 ieee floating point group */
    case op_fltl:		/* 0x17	 datatype independent FP group */
    case op_misc:		/* 0x18	 miscellenous group */
    case op_pal19:		/* 0x19 */
	break;

    case op_jsr:		/* 0x1a	 jsr group */
	reg = inst.m_format.ra;
	break;

    case op_ldf:		/* 0x20 */
    case op_ldg:		/* 0x21 */
    case op_lds:		/* 0x22 */
    case op_ldt:		/* 0x23 */
    case op_stf:		/* 0x24 */
    case op_stg:		/* 0x25 */
    case op_sts:		/* 0x26 */
    case op_stt:		/* 0x27 */
	break;

    case op_ldl:		/* 0x28 */
    case op_ldq:		/* 0x29 */
    case op_ldl_l:		/* 0x2a */
    case op_ldq_l:		/* 0x2b */
	reg = inst.m_format.ra;
	break;

    case op_stl:		/* 0x2c */
    case op_stq:		/* 0x2d */
	break;

    case op_stl_c:		/* 0x2e */
    case op_stq_c:		/* 0x2f */
	reg = inst.m_format.ra;
	break;

    case op_br:		/* 0x30 */
	reg = inst.b_format.ra;
	break;

    case op_fbeq:		/* 0x31 */
    case op_fblt:		/* 0x32 */
    case op_fble:		/* 0x33 */
	break;

    case op_bsr:		/* 0x34 */
	reg = inst.b_format.ra;
	break;

    case op_fbne:		/* 0x35 */
    case op_fbge:		/* 0x36 */
    case op_blbc:		/* 0x38 */
    case op_beq:		/* 0x39 */
    case op_blt:		/* 0x3a */
    case op_ble:		/* 0x3b */
    case op_blbs:		/* 0x3c */
    case op_bne:		/* 0x3d */
    case op_bge:		/* 0x3e */
    case op_bgt:		/* 0x3f */
	break;

    } /* switch */

    return (reg == reg_arg);

} /* inst_sets_reg */


static unsigned long
inst_is_ret(union alpha_instruction	inst, pdsc_register	ra_reg)
{
    unsigned long	j_func;		/* jump sub-opcode */

    /* extra jump sub opcode */
    j_func = (((unsigned)inst.m_format.memory_displacement) >> 14) & 0x3;

    return (j_func == jsr_ret && inst.m_format.rb == ra_reg);

} /* inst_is_ret */

static void
restore_registers_using_mask(
    pdsc_register_value	**from,
    pdsc_register_value	*to,
    pdsc_mask		mask_arg)
{

    /* given a pointer to pointer to a stack frame, work your way
     *	down and restore registers as marked in the the mask_arg
     *	into an array of registers pointed to by to.
     */

    pdsc_mask bitmask;
    pdsc_uint_32 word;
    unsigned int quad;
    unsigned int iregbase;
    /*---8-24-88---*/
    unsigned int quad_limit = 0;

    union {
	pdsc_uint_32 word;
	unsigned short half[2];
	unsigned char byte[4];
    } mask;
    unsigned int ireg;

#ifdef DEBUG
    printf("restore_reg: mask_arg=0x%lx, from=0x%lx\n", mask_arg, from);
#endif
    mask.word = mask_arg;

#if defined(_MIPSEB)

    if (mask.half[0] == 0)
	quad = 2;
    else
	quad = 0;

    if (mask.half[1] == 0)
	quad_limit = 2;

    for (; quad < quad_limit; quad++) {

	if (mask.byte[quad] == 0)
	    continue;

	iregbase = (3 - quad) << 3;

/*---8-24-88---*/
#else /* MIPSEL or alpha Missing piece from JLR */

    if (mask.half[1] == 0)
	quad_limit = 2;
    else
	quad_limit = 4;

    if (mask.half[0] == 0)
	quad = 2;
    else
	quad = 0;

    for (; quad < quad_limit; quad++) {

	if (mask.byte[quad] == 0)
	    continue;

	iregbase = quad << 3;

#endif
	word = mask.word;
	ireg = iregbase;
	bitmask = 1 << ireg;

	for (; ireg < iregbase+8; ireg++) {

	    if (word & bitmask) {
#ifdef DEBUG
		printf("restore_reg: %d old=0x%lx, ", ireg, to[ireg]);
#endif
		to[ireg] = quad_at_address((exc_address)*from);
#ifdef DEBUG
		printf(" new=0x%lx@0x%lx\n", to[ireg], *from);
#endif
		(*from)++;
	    } /* if */

	    bitmask <<= 1;

	} /* for */

    } /* for */

} /* restore_regs */





extern unsigned long
__exc_virtual_unwind( PRUNTIME_FUNCTION pcrd, PCONTEXT pcontext )
{
    register pdsc_address	pc;		/* current pc */
    register pdsc_address	pc_minus_ftable;/* current pc */
    union alpha_instruction	inst;		/* temp inst */
    union alpha_instruction	inst1;		/* temp inst */
    union alpha_instruction	inst2;		/* temp inst */
    pdsc_address		rsa_location;	/* register save area */
    pdsc_rpd			*prpd;		/* points to proc desc */
    unsigned long		in_prolog_or_epilog;/* flag returned */

    in_prolog_or_epilog = 0;

    /* make sure we have the runtime pdr for this pc */

    pc = pcontext->sc_pc;

#ifdef DEBUG
    printf("virtual_unwind: pc = 0x%lx\n", pc);
#endif

    if (pcrd == 0)
	pcrd = (PRUNTIME_FUNCTION) __exc_lookup_function_entry(pc);
    if (pcrd == 0)
	__exc_raise_status_exception (EXC_STATUS_INVALID_DISPOSITION);
    prpd = EXCPT_PD(pcrd);
    pc_minus_ftable = (pdsc_address)((long)pc - 
	(long)__exc_lookup_function_table(pc));

    /*
     * list of what we need to do for DCS:
     *
     *	if at epilogue
     *		check if sp has been restored
     *	if after prologue
     *		restore registers
     *	if in prologue
     *		check if sp has been modified
     *		check if fp has been modified
     */

    if (!PDSC_CRD_CONTAINS_PROLOG(pcrd)) {
	printf("we don't support real code ranges without prolog's yet\n");
	exit(1);
    } /* if */

#ifdef DEBUG
    printf("virtual_unwind: pcrd 0x%lx\n", pcrd);
    printf("virtual_unwind: rpd_offset %ld\n", pcrd->words.rpd_offset);
    printf("virtual_unwind: prpd 0x%lx\n", prpd);
    printf("virtual_unwind: short %ld\n", (long)PDSC_RPD_SHORT(prpd));
    printf("virtual_unwind: frame_size %ld\n", (long)PDSC_RPD_SIZE(prpd));
    printf("virtual_unwind: entry_length %ld\n", (long)PDSC_RPD_ENTRY_LENGTH(prpd));
    printf("virtual_unwind: entry_ra %d\n", (long)PDSC_RPD_ENTRY_RA(prpd));
    printf("virtual_unwind: pc-fptable 0x%lx\n", pc_minus_ftable);
    printf("virtual_unwind: begin addr rel to fptable0x%lx\n", (long)EXCPT_BEGIN_ADDRESS(pcrd));
    printf("virtual_unwind: end of prolog rel to ftable 0x%lx\n", (long)EXCPT_PROLOG_END_ADDRESS(pcrd));
#endif DEBUG


    if (PDSC_RPD_FLAGS(prpd)&PDSC_FLAGS_EXCEPTION_FRAME) {
	/*
	 *	os specific, means it is a special flag indicating
	 *	that this is an exception frame
	 */

#if defined(__osf__)
#define SIGTRAMP_SIGCONTEXT_OFFSET	0	/* should be in signal.h */
	/* we use this to tell us how to step off of signal handlers.
	 *	the signal establisher's registers are at 
	 *	sp+SIGTRAMP_SIGCONTEXT_OFFSET
	 */
	/* should use indirect set here so we can use this routine
	 *	even if we are not in the same process just like
	 *	the register restore does.
	 */

#ifdef DEBUG
	printf("virtual_unwind: exception frame restore from 0x%lx, (0x%lx)\n",
	    quad_at_address(pcontext->sc_regs[SP]+SIGTRAMP_SIGCONTEXT_OFFSET),
	    pcontext->sc_regs[SP]+SIGTRAMP_SIGCONTEXT_OFFSET);
#endif

	memcpy(pcontext, 
	    quad_at_address(pcontext->sc_regs[SP]+SIGTRAMP_SIGCONTEXT_OFFSET),
	    sizeof(pcontext[0]));

#ifdef DEBUG
	printf("virtual_unwind: new pc = 0x%lx, sp = 0x%lx\n",
		pcontext->sc_pc,
		pcontext->sc_regs[SP]+SIGTRAMP_SIGCONTEXT_OFFSET);
#endif
	return in_prolog_or_epilog;
#endif /* osf */

    } else if (pc_minus_ftable < EXCPT_PROLOG_END_ADDRESS(pcrd)) {

	/* 
	 * don't need to restore any registers except for possibly 
	 *	SP & FP
	 */

	in_prolog_or_epilog = 1;

#ifdef DEBUG
	printf("virtual_unwind: in prolog\n");
#endif
	if ((PDSC_RPD_FLAGS(prpd)&PDSC_FLAGS_BASE_REG_IS_FP) &&
	    (inst_sets_reg(inst_at_pc(pc), FP))) {

	    /*
	     * bill noyce claims that all you need to do is check
	     *	the current instruction to see
	     *	if the FP was set. I am following his instructions.
	     */

	    rsa_location = pcontext->sc_regs[SP]+PDSC_RPD_RSA_OFFSET(prpd);
	    restore_registers_using_mask((pdsc_register_value **)&rsa_location, 
		    (pdsc_register_value *)pcontext->sc_fpregs, 
		    1<<FP);

	} /* if */

	if (pc_minus_ftable>(EXCPT_BEGIN_ADDRESS(pcrd)+PDSC_RPD_SP_SET(prpd))) {

#ifdef DEBUG
	printf("virtual_unwind: after sp set\n");
#endif
	    /* restore sp */
	    pcontext->sc_regs[SP] += PDSC_RPD_SIZE(prpd);

	} /* if */

    } else {

	/* after prolog */

	if ((PDSC_RPD_FLAGS(prpd)&PDSC_FLAGS_REGISTER_FRAME)) {

#ifdef DEBUG
		printf("virtual_unwind: register frame\n");
#endif
		if (PDSC_RPD_ENTRY_RA(prpd) != PDSC_RPD_SAVE_RA(prpd)) {
		    /* restore ra */
		    pcontext->sc_regs[PDSC_RPD_ENTRY_RA(prpd)] =
			pcontext->sc_regs[PDSC_RPD_SAVE_RA(prpd)];

		} /* if */

		/* check if in epilog */
		inst = inst_at_pc(pc);
	        in_prolog_or_epilog = inst_is_ret(inst,PDSC_RPD_ENTRY_RA(prpd));
		if (!in_prolog_or_epilog) {
		    /* need to restore the SP */
		    pcontext->sc_regs[SP] += PDSC_RPD_SIZE(prpd);
		} /* if */



		pcontext->sc_pc = pcontext->sc_regs[PDSC_RPD_ENTRY_RA(prpd)];
#ifdef DEBUG
		printf("virtual_unwind: new pc = 0x%lx\n", pcontext->sc_pc);
#endif

	} else {

	    /* 
	     * check for epilogue: the calling standard says what to expect
	     * of exit sequences (if optional's are they must be in this order):
	     *
	     *		RESTORE FP		optional
	     *		RESTORE SP		optional
	     *		RETURN
	     */
	    inst = inst_at_pc(pc);

	    inst1 = inst_at_pc(pc + INST_SIZE);
	    inst2 = inst_at_pc(pc + (2 * INST_SIZE));
	    if (inst_is_ret(inst1, PDSC_RPD_ENTRY_RA(prpd)) && 
		inst_sets_reg(inst, SP)) {

#ifdef DEBUG
	printf("virtual_unwind: at retore sp before ret\n");
#endif
		/* restore sp */
		pcontext->sc_regs[SP] += PDSC_RPD_SIZE(prpd);
		in_prolog_or_epilog = 1;

	    } else if (inst_is_ret(inst2, PDSC_RPD_ENTRY_RA(prpd)) && 
		inst_sets_reg(inst1, SP) && inst_sets_reg(inst, FP)) {


#ifdef DEBUG
	printf("virtual_unwind: at retore fp before ret\n");
#endif
		rsa_location = pcontext->sc_regs[SP]+PDSC_RPD_RSA_OFFSET(prpd);
		restore_registers_using_mask((pdsc_register_value **)&rsa_location, 
			(pdsc_register_value *)pcontext->sc_fpregs, 
			1<<FP);

		/* restore sp */
		pcontext->sc_regs[SP] += PDSC_RPD_SIZE(prpd);
		in_prolog_or_epilog = 1;

	    } else if (!inst_is_ret(inst, PDSC_RPD_ENTRY_RA(prpd))) {

		/* restore all registers */

		if ((PDSC_RPD_FLAGS(prpd)&PDSC_FLAGS_BASE_REG_IS_FP)) {

#ifdef DEBUG
	printf("virtual_unwind: fp manifested, fix sp\n");
#endif
		    /* restore sp before fp was manifested */
		    pcontext->sc_regs[SP] = pcontext->sc_regs[FP];

		} /* if */

		rsa_location = pcontext->sc_regs[SP]+PDSC_RPD_RSA_OFFSET(prpd);
		restore_registers_using_mask((pdsc_register_value **)&rsa_location, 
			(pdsc_register_value *)pcontext->sc_regs, 
			(1<<PDSC_RPD_ENTRY_RA(prpd)));
		restore_registers_using_mask((pdsc_register_value **)&rsa_location, 
			(pdsc_register_value *)pcontext->sc_regs, 
			PDSC_RPD_IMASK(prpd));
		restore_registers_using_mask((pdsc_register_value **)&rsa_location, 
			(pdsc_register_value *)pcontext->sc_fpregs, 
			PDSC_RPD_FMASK(prpd));

	    } else {

		in_prolog_or_epilog = 1;

	    } /* if */

	    /* restore sp */
#ifdef DEBUG
	printf("virtual_unwind: old sp = 0x%lx, new = 0x%lx\n",
	    pcontext->sc_regs[SP],pcontext->sc_regs[SP] + PDSC_RPD_SIZE(prpd));
#endif
	    pcontext->sc_regs[SP] += PDSC_RPD_SIZE(prpd);


	} /* if */
    } /* if */

    /* retsore pc */
    pcontext->sc_pc = pcontext->sc_regs[PDSC_RPD_ENTRY_RA(prpd)];

    return in_prolog_or_epilog;

}



extern unsigned long
__exc_find_frame_ptr( PRUNTIME_FUNCTION pcrd, PCONTEXT pcontext, PCONTEXT pnext_context )
{
    CONTEXT  dup_context;

    if (pnext_context)
	return pnext_context->sc_regs[SP];

    dup_context = *pcontext;
    __exc_virtual_unwind(pcrd, &dup_context);
    return dup_context.sc_regs[SP];
}

