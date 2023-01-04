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
 * Protect the file from multiple includes.
 */
#ifndef CPU_HDR
#define CPU_HDR

#ifndef ASSEMBLER
/*
 * Interrupt context stuff
 */
extern  int	atintr_level;
#define AT_INTR_LVL() (atintr_level)

#ifdef KERNEL

int	master_cpu;
/* No Mips multiprocessors yet */
#define	cpu_number()	(0)
#define	set_cpu_number()

#endif /* KERNEL */

/*
 * Generic Machinecheck Stack Frame
 */
struct mcframe	{
	int	mc_bcnt;		/* byte count */
	int	mc_summary;		/* summary parameter */
};

struct	cpu_archdep {
	struct  proc	*cp_exitproc; /* if the process is exiting, its proc */
	struct	proc    *cp_idleproc; /* address of the idle proc this cpu */
	int		cp_inisr;	/* IN an ISR */
}; 
#define cpu_exitproc	cpu_archdep.cp_exitproc
#define cpu_idleproc	cpu_archdep.cp_idleproc
#define	cpu_inisr	cpu_archdep.cp_inisr

#define splextreme()	swap_ipl(SPLEXTREME)
#define spl7()		swap_ipl(SPLEXTREME)
#define spl6()		swap_ipl(SPLHIGH)
#define splhigh()	swap_ipl(SPLHIGH)
#define splsched()	swap_ipl(SPLHIGH)
#if NCPUS > 1
#define splvm()		swap_ipl(SPLHIGH)
#else
#define splvm()		swap_ipl(SPLIMP)
#endif

#define splclock()	swap_ipl(SPLCLOCK)
#define	splsoftclock()	swap_ipl(SPLSOFTC)


#define spldevhigh()	swap_ipl(SPLDEVHIGH)
#define spl5()		swap_ipl(SPLDEVHIGH)
#define spltty()	swap_ipl(SPLTTY)
#define splimp()	swap_ipl(SPLIMP)
#define splbio()	swap_ipl(SPLBIO)
#define splnet()	swap_ipl(SPLNET)
#define splsoftc()	swap_ipl(SPLSOFTC)
#define spl1()		swap_ipl(1)
#if	!RT_PREEMPT
#define spl0()		swap_ipl(SPLNONE)
#define splnone()	swap_ipl(SPLNONE)
#define splx(S)		swap_ipl(S)
#else	/* RT_PREEMPT */
#define spl0x()		swap_ipl(SPLNONE)
#define splx1(S)	swap_ipl(S)

#endif	/* !RT_PREEMPT */
#endif /* !ASSEMBLER */

/*
 * OSF PAlcode has 8 levels 0..7
 *
 *	7	machine check
 *	6	realtime device interrupt
 *	5	clock and ip interrupts
 *	4	device level 1 interrupts
 *	3	device level 0 interrupts
 *	2	software level 1 interrupts
 *	1	software level 0 interrupts
 *	0	any interrupt
 */
#define SPLEXTREME	7
#define SPLCLOCK	5
#define SPLHIGH		5
#define SPLDEVHIGH	4
#define SPLTTY		4
#define SPLIMP	   4 /* because of SLIP interface, splimp must be >= spltty */
#define SPLBIO		4
#define SPLNET		2		/* software (soft net) */
#define SPLSOFTC	1		/* software (soft clock) */
#define SPLNONE		0		/* no interrupts blocked */

/*
 * Flags for the nofault handler. 0 means no fault is expected.
 */
#define	NF_BADADDR	1	/* badaddr, wbadaddr */
#define	NF_COPYIO	2	/* copyin, copyout */
#define	NF_ADDUPC	3	/* addupc */
#define	NF_FSUMEM	4	/* fubyte, subyte, fuword, suword */
#define	NF_USERACC	5	/* useracc */
#define	NF_SOFTFP	6	/* softfp */
#define	NF_REVID	7	/* revision ids */
#define	NF_COPYSTR	8	/* copyinstr, copyoutstr */
#define	NF_SOFTFPI	9	/* fp instr fetch */
#define NF_SIGRETURN_COPYIN 10	/* copyin errors for sigreturn */ 
#define NF_SENDSIG_COPYOUT 11	/* copy out errors for sendsig */ 
#define NF_LWERR        12      /* faults in pmap_lw_wire */
#define NF_LW_UNERR     13      /* faults in pmap_lw_unwire */
#define NF_LW_UNERR_AUD 14      /* faults in pmap_lw_unwire */
#define NF_LWERR_ASS    15

#define       NF_NENTRIES     16


/*
 * State bits for the kernel global "printstate"
 */
#define PROMPRINT 0x1		/* only print available is thru PROM */
#define CONSPRINT 0x2		/* print thru console device driver available */
#define MEMPRINT  0x4		/* print to errlog available */
#define PANICPRINT 0x8		/* panic in progress, print to screen */

/* MCES Register bit defines per ECO's #45/#51 */
#define MCES_MCK 0x1	/* Clear machine check flag */
#define MCES_SCE 0x2	/* Clear system corrected error flag */
#define MCES_PCE 0x4	/* Clear processor correct error flag */
#define MCES_DPC 0x8	/* Disable processor corrected error reporting */
#define MCES_DSC 0x10	/* Disable system corrected error reporting */
#define MCES_DISABLE	(MCES_DPC|MCES_DSC)	/* Disable both PCE/SCE */

#endif /*CPU_HDR*/			/* Multi include protection ends here */
