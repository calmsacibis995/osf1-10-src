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
 * Modification History: /sys/machine/alpha/pcb.h
 *
 *  9-Sep-91 -- rjl
	Added pcb physical address for context switching.
 *	
 * 10-Sep-90 -- rjl
 *	Created this file for Alpha support.
 *
 * The first part of this structure is 128 bytes long.  The HWPCB
 * on ALPHA must be aligned on a 128 byte boundary
 */

#ifndef	_PCB_H_
#define	_PCB_H_

#define	PCB_V0		0		/* r0: return address	*/
#define	PCB_T0		1		/* r1: caller saved 0	*/
#define PCB_T1		2		/* r2: caller saved 1	*/
#define PCB_T2		3		/* r3: caller saved 2	*/
#define PCB_T3		4		/* r4: caller saved 3	*/
#define PCB_T4		5		/* r5: caller saved 4	*/
#define PCB_T5		6		/* r6: caller saved 5	*/
#define PCB_T6		7		/* r7: caller saved 6	*/
#define	PCB_T7		8		/* r8: caller saved 7	*/
#define	PCB_S0		9		/* r9: callee saved 0	*/
#define	PCB_S1		10		/* r10: callee saved 1	*/
#define	PCB_S2		11		/* r11: callee saved 2	*/
#define	PCB_S3		12		/* r12: callee saved 3	*/
#define	PCB_S4		13		/* r13: callee saved 4	*/
#define	PCB_S5		14		/* r14: callee saved 5	*/
#define PCB_S6		15		/* r15:	callee saved 6	*/
#define	PCB_A3		16		/* r19: argument 3	*/
#define	PCB_A4		17		/* r20: argument 4	*/
#define	PCB_A5		18		/* r21: argument 5	*/
#define	PCB_T8		19		/* r22: caller saved 8	*/
#define	PCB_T9		20		/* r23: caller saved 9	*/
#define	PCB_T10		21		/* r24: caller saved 10	*/
#define	PCB_T11		22		/* r25: caller saved 11	*/
#define	PCB_RA		23		/* r26: return address	*/
#define	PCB_T12		24		/* r27: caller saved 12	*/
#define	PCB_AT		25		/* r28: assembler temp	*/
#define	PCB_SP		26		/* r30: stack pointer	*/

/*
 * Hardware portion of the exception frame
 */
#define PCB_PS		27		/* processor status 	*/
#define PCB_PC		28		/* program counter	*/
#define	PCB_GP		29		/* r29: global pointer	*/
#define	PCB_A0		30		/* r16: argument 0	*/
#define	PCB_A1		31		/* r17: argument 1	*/
#define	PCB_A2		32		/* r18: argument 2	*/

#define PCB_NREGS	33

#ifndef ASSEMBLER
#include <sys/types.h>
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

struct pcb {
	/*
	 * Hardware portion of the PCB
	 */
	u_long	pcb_ksp;	/* kernel stack pointer		*/
	u_long	pcb_usp;	/* user stack pointer		*/
	u_long	pcb_ptbr;	/* page table base register	*/
	u_int	pcb_cc;		/* process cycle counter	*/
        u_int	pcb_asn;	/* address space number		*/
	u_long	pcb_proc_uniq;	/* process/thread unique value	*/
        u_long	pcb_fen;	/* floating point enable	*/
	u_long	pcb_palscr[2];	/* pal scratch area		*/
	/*
	 * Software portion of the PCB
	 */
        u_long	pcb_asten;	/* reschedule ast enable	*/
	u_long	pcb_regs[PCB_NREGS];	/* kernel context (C callee+pc) */
	u_long	pcb_fpregs[32];	/* floating point registers	*/
	u_long	pcb_fpcr;	/* floating point control register */

	struct pcb *pcb_paddr;		/* physical of this pcb		*/
	struct	cpudata	*pcb_cpuptr;	/* per-cpu data of current processor */
	int	*pcb_sswap;	/* ptr for non-local resume	*/

	long	pcb_resched;	/* non-zero if time to resched	*/
	long	pcb_sstep;	/* non-zero if single stepping	*/
	struct	ssi pcb_ssi;	/* single step state info */
	long	pcb_ownedfp;	/* has used the fp at one time	*/
	long	pcb_nofault;	/* no fault trap handler flag	*/
	u_long	pcb_traparg_a0;	/* 1st trap() arg passed to sendsig() */
	u_long	pcb_traparg_a1;	/* 2nd trap() arg passed to sendsig() */
	u_long	pcb_traparg_a2;	/* 3rd trap() arg passed to sendsig() */
	int	pcb_cpu_number;
	u_long	pcb_kstack;
};

struct pcb *current_pcb;	/* needs to be per processor */

#endif  /* ASSEMBLER */

/*
 * The PME bit in the pcb_fen quadword (per ECO #50).
 */
#define PCB_PME_BIT	(1L<<62)

#endif  /* pcb */
