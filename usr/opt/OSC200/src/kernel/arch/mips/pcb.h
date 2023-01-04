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
 *	@(#)$RCSfile: pcb.h,v $ $Revision: 1.2.4.2 $ (DEC) $Date: 1992/03/18 15:28:27 $
 */ 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0
 */
/* 
 * derived from pcb.h	2.1	(ULTRIX/OSF)	12/3/90
 */

/* ------------------------------------------------------------------ */
/* | Copyright Unpublished, MIPS Computer Systems, Inc.  All Rights | */
/* | Reserved.  This software contains proprietary and confidential | */
/* | information of MIPS and its suppliers.  Use, disclosure or     | */
/* | reproduction is prohibited without the prior express written   | */
/* | consent of MIPS.                                               | */
/* ------------------------------------------------------------------ */
#ifndef ASSEMBLER
#include <sys/user.h>
#endif

#ifndef	_PCB_H_
#define	_PCB_H_

/*
 * MIPS process control block
 */

/*
 * pcb_regs indices
 */
#define PCB_ZRO		
#define PCB_AT		0
#define PCB_V0		1
#define PCB_V1		2
#define PCB_A0		3
#define PCB_A1		4
#define PCB_A2		5
#define PCB_A3		6
#define PCB_T0		7
#define PCB_T1		8
#define PCB_T2		9
#define PCB_T3		10
#define PCB_T4		11
#define PCB_T5		12
#define PCB_T6		13
#define PCB_T7		14
#define PCB_S0		15
#define PCB_S1		16
#define PCB_S2		17
#define PCB_S3		18
#define PCB_S4		19
#define PCB_S5		20
#define PCB_S6		21
#define PCB_S7		22
#define PCB_T8		23
#define PCB_T9		24
#define PCB_K0		25
#define PCB_K1		26
#define PCB_GP		27
#define PCB_SP		28
#define PCB_FP		29
#define PCB_RA		30
#define PCB_LO		31
#define PCB_HI		32
#define PCB_PC		33
#define PCB_SR		34
#define PCB_BAD		35
#define PCB_CS		36
#define PCB_TLO		37
#define PCB_THI		38
#define PCB_INX		39
#define PCB_RAN		40
#define PCB_CTX		41

#define PCB_NREGS	42


/*
 * jmp_buf offsets
 * WARNING:
 * if this changes, label_t in types.h must change
 */
#define	JB_S0		0	/* callee saved regs.... */
#define	JB_S1		1
#define	JB_S2		2
#define	JB_S3		3
#define	JB_S4		4
#define	JB_S5		5
#define	JB_S6		6
#define	JB_S7		7
#define	JB_SP		8	/* stack pointer */
#define	JB_FP		9	/* frame pointer */
#define	JB_PC		10	/* program counter */
#define	JB_SR		11	/* C0 status register */
#define	NJBREGS		12

#ifndef ASSEMBLER
/*
 * single step information
 * used to hold instructions that have been replaced by break's when
 * single stepping
 */
struct ssi {
	int ssi_cnt;			/* number of bp's installed */
	struct ssi_bp {
		unsigned *bp_addr;	/* address of replaced instruction */
		unsigned bp_inst;	/* replaced instruction */
	} ssi_bp[2];
};

struct pcb
{
	/*
	 * Space for general purpose registers.
	 * Partially saved at context switch time, and by debugger.
	 *
	 */
	int	pcb_regs[PCB_NREGS];
	/*
	 * Misc.
	 */
	int	pcb_sstep;	/* non-zero if single stepping */
	int	pcb_nofault;	/* saves kernel "nofault" flag */
	struct	ssi pcb_ssi;	/* single step state info */
	/* This is used for single stepping/tracing */
	int	trapcause;	/* why a sigtrap was delivered */
	/* These are used in branch delay instruction emulation */
	int	pcb_bd_epc;	/* epc register */
	int	pcb_bd_cause;	/* cause register */
	int	pcb_bd_ra;	/* address to return to if doing bd emulation */
	int	pcb_bd_instr;	/* the branch instr for the bd emulation */
	/* This is used in fp instruction emulation */
	int	pcb_softfp_pc;	/* resulting pc after fp emulation */
	/*
	 * Space for the state of all the potential coprocessors. WASTEFUL!
	 */
	int	pcb_fpregs[32];	/* floating point */
	int	pcb_fpc_csr;	/* floating point control and status reg */
	int	pcb_fpc_eir;	/* floating point exception instruction reg */
	int	pcb_ownedfp;	/* has owned fp at one time */
	int	pcb_c2regs[32];	/* TBD */
	int	pcb_c3regs[32];	/* TBD */
	int	pcb_cpu_number;
	int	pcb_mips_user_fault;
	int	pcb_kstack;
	struct {
		struct uthread *uthread;
		struct utask *utask;
	} u_address;
	int spare[2];
};

#endif	/* ASSEMBLER */
#endif	/* _PCB_H_ */
