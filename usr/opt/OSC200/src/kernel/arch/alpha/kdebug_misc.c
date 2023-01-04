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
static char *rcsid = "@(#)$RCSfile: kdebug_misc.c,v $ $Revision: 1.1.2.4 $ (DEC) $Date: 1993/10/12 13:30:55 $";
#endif

#include <sys/kdebug.h>
#include <sys/errno.h>
#include <machine/pcb.h>
#include <machine/reg.h>
#include <machine/rpb.h>
#include <machine/inst.h>
#include <machine/cpu.h>

extern KdebugInfo kdebug_info;
void _XentMM(), _XentKdebugMM();

/*
 * the index is the dbx register number, and the value is the offset into 
 * the pcb register array
 */

long pcbmap[] = {
	/*
	 * 31 integer registers
	 */
	PCB_V0,  PCB_T0,  PCB_T1,  PCB_T2,  PCB_T3,  PCB_T4,  PCB_T5,  PCB_T6,
	PCB_T7,  PCB_S0,  PCB_S1,  PCB_S2,  PCB_S3,  PCB_S4,  PCB_S5,  PCB_S6,
	PCB_A0,  PCB_A1,  PCB_A2,  PCB_A3,  PCB_A4,  PCB_A5,  PCB_T8,  PCB_T9,
	PCB_T10, PCB_T11, PCB_RA,  PCB_T12, PCB_AT,  PCB_GP,  PCB_SP
};

long
reg_from_pcb(
    struct pcb *pcb,
    long reg,
    u_long *val)
{
    u_long *addr;

    switch (reg) {
    case R_EPC:
	/* r64 - pc */
	addr = &pcb->pcb_regs[PCB_PC];
	break;

    case R_PS:
	/* r65 - ps */
	addr = &pcb->pcb_regs[PCB_PS];
	break;

    case R_R31:
	/* r31 - mbz */
	*val = 0L;
	return 0;
	break;

    default:
        if (reg > R_R31 && reg < R_EPC) {
	    /* floating point */
	    addr = &pcb->pcb_fpregs[reg - 32];
	} else {
	    if (reg >= R_R0 && reg < R_R31) {
	        /* standard register */
	        addr = &pcb->pcb_regs[pcbmap[reg]];
	    } else {
	        /* bad register */
		return -1;
	    }
	}
	break;
    }

    return kdebug_bcopy_fault(addr, val, sizeof(long));
}

reg_to_pcb(
    struct pcb *pcb,
    long reg,
    unsigned long val)
{
    u_long *addr;

    switch (reg) {
    case R_EPC:
	/* r64 - pc */
	addr = &pcb->pcb_regs[PCB_PC];
	break;

    case R_PS:
	/* r65 - ps */
	addr = &pcb->pcb_regs[PCB_PS];
	break;

    case R_R31:
	/* mbz */
	return 0;
	break;

    default:
        if (reg > R_R31 && reg < R_EPC) {
	    /* floating point */
	    addr = &pcb->pcb_fpregs[reg - 32];
	} else {
	    if (reg >= R_R0 && reg < R_R31) {
	        /* standard register */
	        addr = &pcb->pcb_regs[reg];
	    } else {
	        /* bad register */
		return -1;
	    }
	}
	break;
    }

    return kdebug_bcopy_fault(&val, addr, sizeof(long));
}

/*
 * the index is the dbx register number, and the value is the offset into 
 * the exception frame
 */
long excmap[] = {
	/*
	 * 31 integer registers
	 */
	EF_V0,   EF_T0,   EF_T1,   EF_T2,   EF_T3,   EF_T4,   EF_T5,   EF_T6,
	EF_T7,   EF_S0,   EF_S1,   EF_S2,   EF_S3,   EF_S4,   EF_S5,   EF_S6,
	EF_A0,   EF_A1,   EF_A2,   EF_A3,   EF_A4,   EF_A5,   EF_T8,   EF_T9,
	EF_T10,  EF_T11,  EF_RA,   EF_T12,  EF_AT,   EF_GP,   EF_SP
};

long
reg_from_exc(
    long reg,
    u_long *val)
{
    u_long *addr;

    switch (reg) {
    case R_EPC:
	/* r64 - pc */
	addr = &kdebug_info.exc_frame[EF_PC];
	break;

    case R_PS:
	/* r65 - ps */
	addr = &kdebug_info.exc_frame[EF_PS];
	break;

    case R_R30:
	/* sp */
	*val = (u_long) kdebug_info.exc_frame + EF_SIZE;
	return 0;
	break;

    case R_R31:
	/* mbz */
	*val = 0L;
	return 0;
	break;

    default:
        if (reg > R_R31 && reg < R_EPC) {
	    /* floating point */
	    return -1;
	} else {
	    if (reg >= R_R0 && reg < R_R31) {
	        /* standard register */
	        addr = &kdebug_info.exc_frame[excmap[reg]];
	    } else {
	        /* bad register */
		return -1;
	    }
	}
	break;
    }

    return kdebug_bcopy_fault(addr, val, sizeof(long));
}

reg_to_exc(
    long reg,
    unsigned long val)
{
    u_long *addr;

    switch (reg) {
    case R_EPC:
	/* R_64 - pc */
	addr = &kdebug_info.exc_frame[EF_PC];
	break;

    case R_PS:
	/* R_65 - ps */
	addr = &kdebug_info.exc_frame[EF_PS];
	break;

    case R_R31:
	/* R_31 - mbz */
	return 0;
	break;

    default:
        if (reg > R_R31 && reg < R_EPC) {
	    /* floating point */
	    addr = &kdebug_info.exc_frame[reg - 32];
	} else {
	    if (reg >= R_R0 && reg <= R_R30) {
	        /* standard register */
	        addr = &kdebug_info.exc_frame[reg];
	    } else {
	        /* bad register */
		return -1;
	    }
	}
	break;
    }

    return kdebug_bcopy_fault(&val, addr, sizeof(long));
}

/*
 * Determine what processor and system we are running on and return
 * the cpu type.  To be used as the index into the cpu switch.
 */
long
getcputype()
{
    struct rpb *rpb = (struct rpb *)HWRPB_ADDR;

    return (rpb->rpb_systype);
}

#define JMP_INST	(unsigned)0x68000000
#define BR_INST		(unsigned)0xc0000000
#define FBEQ_INST	(unsigned)0xc4000000
#define FBLT_INST	(unsigned)0xc8000000
#define FBLE_INST	(unsigned)0xcc000000
#define BSR_INST	(unsigned)0xd0000000
#define FBNE_INST	(unsigned)0xd4000000
#define FBGE_INST	(unsigned)0xd8000000
#define FBGT_INST	(unsigned)0xdc000000
#define BLBC_INST	(unsigned)0xe0000000
#define BEQ_INST	(unsigned)0xe4000000
#define BLT_INST	(unsigned)0xe8000000
#define BLE_INST	(unsigned)0xec000000
#define BLBS_INST	(unsigned)0xf0000000
#define BNE_INST	(unsigned)0xf4000000
#define BGE_INST	(unsigned)0xf8000000
#define BGT_INST	(unsigned)0xfc000000

#define OP_MASK		(unsigned)0xfc000000

unsigned long
kdebug_branch_target(
    unsigned int inst,
    unsigned long pc)
{
    union alpha_instruction i;
    unsigned long vaddr;

    i.word = inst;

    if ((inst & OP_MASK) == JMP_INST) {
	reg_from_exc(i.j_format.rb, &vaddr);
	return (vaddr & ~(u_long)3);
    }
    else {
	vaddr = pc + 4 + ((i.b_format.branch_displacement) << 2);
	return (vaddr);
    }
}

kdebug_isa_branch(inst)
    unsigned int inst;
{
    switch (inst & OP_MASK) {
    case JMP_INST:
    case BR_INST:
    case FBEQ_INST:
    case FBLT_INST:
    case FBLE_INST:
    case BSR_INST:
    case FBNE_INST:
    case FBGT_INST:
    case FBGE_INST:
    case BLBC_INST:
    case BEQ_INST:
    case BLT_INST:
    case BLE_INST:
    case BLBS_INST:
    case BNE_INST:
    case BGE_INST:
    case BGT_INST:
	return 1;
	break;
    }

    return(0);
}

kdebug_isa_uncond_branch(inst)
    unsigned int inst;
{
    switch (inst & OP_MASK) {
    case JMP_INST:
    case BR_INST:
    case BSR_INST:
	return 1;
	break;
    }

    return(0);
}

install_mm_handler()
{
    kdebug_install_handler(_XentKdebugMM, 2);
}

restore_mm_handler()
{
    kdebug_install_handler(_XentMM, 2);
}
