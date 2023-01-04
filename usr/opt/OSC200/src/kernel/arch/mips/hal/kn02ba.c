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
static char	*sccsid = "@(#)$RCSfile: kn02ba.c,v $ $Revision: 1.2.3.10 $ (DEC) $Date: 1992/10/13 12:11:26 $";
#endif 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * Mach Operating System
 * Copyright (c) 1989 Carnegie-Mellon University
 * All rights reserved.  The CMU software License Agreement specifies
 * the terms and conditions for use and redistribution.
 */
/*
 * OSF/1 Release 1.0
 */
/* 
 * derived from kn02ba.c	4.8      (ULTRIX)  3/7/91";
 */


/*
 * Modification History:
 *
 * 27-Jun-91	Scott Cranston
 *	Changed optional compilation of binary error logging support startegy.
 *         - added #include <uerf.h>
 *	   - changed instances of '#ifdef _UERF' to '#if UERF'
 *
 * 22-Mar-91	Bill Burns
 *	Ported to osf.
 *
 * 7-Mar-91	Randall Brown
 *	Fixed a bug in isolatepar().  Print out more information about
 *	which CPU card is in the system.
 *
 * 7-Mar-91	Jaw
 *	3min spl optimization.
 *
 * 21-Jan-91	Randall Brown 
 *	Modified kn02ba_isolate_memerr() to use tc_memerr_status struct.
 *	Modified kn02ba_isolate_par() to take the blocksize as
 *	a parameter.
 *
 * 06-Dec-90	Randall Brown
 *	Added isolate_memerr() routine for logging of memory errors
 *	that occur during I/O.
 *
 * 15-Oct-90	Randall Brown
 *	Added error handling code.
 *
 * 06-Sep-90 	Randall Brown 
 *	Changed slot_order to config_order.
 *
 * 23-Feb-90	Randall Brown
 *	Created file for support of 3MIN (DS5000_100).
 *
 */
#include <sys/systm.h>
#include <machine/cpu.h>
#include <sys/proc.h>
#include <mach/machine.h>
#include <sys/vmmac.h>
#include <hal/cpuconf.h>
#include <io/dec/tc/tc.h>
#include <io/common/devdriver.h>
#include <machine/reg.h>
#include <hal/kn02ba.h>
#include <hal/mc146818clock.h>
#include <sys/syslog.h>
#include <dec/binlog/errlog.h>
#include <sys/table.h>


extern 	softclock(), softintr2(), kn_hardclock(), fp_intr();

int	kn02ba_stray(), kn02ba_sysintr(), kn02ba_halt();
int	kn02ba_enable_option(), kn02ba_disable_option();
int	kn02ba_clear_errors(), kn02ba_isolate_memerr();

/* TODO: extern u_int printstate;	/* how to print to the console */
caddr_t vatophys();
extern char *rt_clock_addr;	/* addr of the mc146818clock chip */
extern struct timeval time;	/* the system time */
extern int hz;
extern int tick;
extern int tickadj;
extern int fixtick;		/* set to the number of microsec's that the
				   clock looses per second */
extern int confdebug;		/* debug variable for configuration code */

#if NCPUS > 1
extern volatile unsigned long  system_intr_cnts_type[NCPUS][INTR_TYPE_SIZE];
#else	/* NCPUS */
extern volatile unsigned long  system_intr_cnts_type[INTR_TYPE_SIZE];
#endif	/* NCPUS */

/* Define debugging stuff.
 */
#define DEBUG
#ifdef DEBUG
#define Cprintf if(confdebug)printf
#define Dprintf if( confdebug >= 2 )printf
#else
#define Cprintf ;
#define Dprintf ;
#endif

/* The SR reg masks for splxxx usage */
int kn02ba_splm[SPLMSIZE] = {
	KN02BA_SR_IMASK0,		/* 0 SPLNONE			*/
	KN02BA_SR_IMASK1,		/* 1 SPLSOFTC			*/
	KN02BA_SR_IMASK2,		/* 2 SPLNET			*/
	KN02BA_SR_IMASK5,		/* 3 SPLBIO			*/
	KN02BA_SR_IMASK5,		/* 4 SPLIMP			*/
	KN02BA_SR_IMASK5,		/* 5 SPLTTY			*/
	KN02BA_SR_IMASK6,		/* 6 SPLCLOCK			*/
	KN02BA_SR_IMASK7,		/* 7 SPLMEM			*/
	KN02BA_SR_IMASK8,		/* 8 SPLFPU			*/
};

/* The mask values for the system interrup mask */
int kn02ba_sim[SPLMSIZE] = {
	SPL0_MASK,			/* 0 SPLNONE			*/
	SPL1_MASK,			/* 1 SPLSOFTC			*/
	SPL2_MASK,			/* 2 SPLNET			*/
	SPLIO_MASK,			/* 3 SPLBIO			*/
	SPLIO_MASK,			/* 4 SPLIMP			*/
	SPLIO_MASK,			/* 5 SPLTTY			*/
	SPLCLOCK_MASK,			/* 6 SPLCLOCK			*/
	SPLMEM_MASK,			/* 7 SPLMEM			*/
	SPLFPU_MASK,			/* 8 SPLFPU			*/
};

u_int kn02ba_slotaddr[TC_IOSLOTS] = {KN02BA_SL_0_ADDR, KN02BA_SL_1_ADDR,
	KN02BA_SL_2_ADDR, KN02BA_SL_3_ADDR, KN02BA_SL_4_ADDR,
	KN02BA_SL_5_ADDR, KN02BA_SL_6_ADDR, KN02BA_SL_7_ADDR};

/*
 * Since this system does not follow the "normal" way of dispatching interrupts
 * (via intr.c), we need to handle the interrupt statistics ourself.  Therefore,
 * we will setup the following table in the order they exist in table.h. 
 * However, noone is going to be placing any info in the level table so this
 * is basically a no-op when the data is collected.  All interrupt statistics
 * will be placed in the type table.
 */
static int KN02ba_interrupt_type[INTR_MAX_LEVEL] = {
		INTR_TYPE_NOSPEC,
		INTR_TYPE_HARDCLK,
		INTR_TYPE_SOFTCLK,
		INTR_TYPE_DEVICE,
		INTR_TYPE_OTHER,
		INTR_TYPE_STRAY,
		INTR_TYPE_DISABLED,
		INTR_TYPE_NOSPEC
};

/*
 * Program the order in which to probe the IO slots on the system.
 * This determines the order in which we assign unit numbers to like devices.
 * It also determines how many slots (and what slot numbers) there are to probe.
 * Terminate the list with a -1.
 * Note: this agrees with the console's idea of unit numbers
 */
int kn02ba_config_order [] = { 3, 4, 5, 0, 1, 2, -1 };

/* ipllevel describes the current interrupt level the processor is */
/* running at.  The values that ipllevel can be are SPLNONE -> SPLHIGH. */
/* These values are defined in cpu.h */

int ipllevel = SPLHIGH;

int clk_counter;

int kn02ba_transintvl = KN02BA_TRANSINTVL;/* global var so we can change it */
int kn02ba_translog = 1;		/* is trans logging currently enabled */
struct trans_errcnt kn02ba_trans_errcnt = { 0, 0 };
int kn02ba_tcount[MAXSIMM];		/* # of transient parity errs per simm*/
int kn02ba_scount[MAXSIMM];		/* # of soft errs on each simm */
int kn02ba_hcount[MAXSIMM];		/* # of hard errs on each simm */
int kn02ba_pswarn;			/* set true if we had a ps-warning */
int kn02ba_model_number;

/* Initial value is worst case, real value is determined in kn02ba_init() */
int kn02ba_delay_mult = 25; 


int kn02ba_psintvl = (60 * 1);	/* time delta to check pswarn bit */
                                /* global var so we can change it */

unsigned kn02ba_isolatepar();
int kn02ba_transenable();
int kn02ba_pscheck();

kn02ba_init()
{
        register int i;
	register int  splsize, splx_size;
	extern int splm[];
	extern int kn_delay_mult;
	extern struct cpusw *cpup;
	extern u_int sr_usermask;
	extern u_int sr_kernelmask;
	extern int printstate;
	extern spl0(), splsoftclock(), splnet(), splbio();
	extern splimp(), spltty(), splclock(), splvm(), splfpu();
	extern splx();
	extern char espl0[], esplx[];
	extern kn02ba_spl0(), kn02ba_splsoftclock(), kn02ba_splnet();
	extern kn02ba_splbio(), kn02ba_splimp(), kn02ba_spltty();
	extern kn02ba_splclock(), kn02ba_splvm(), kn02ba_splfpu();
	extern kn02ba_splx();
	extern int mips_spl_arch_type;
	extern volatile int  *system_intr_cnts_type_transl;
#if	RT_PREEMPT
	extern spl0x(), splx1();
#endif
	/*
	 * Setup global interrupt type array to use our definitions.
	 */
	system_intr_cnts_type_transl = KN02ba_interrupt_type;
	
	/* Turn off all interrupts in the SIRM */
	*(u_int *)PHYS_TO_K1(KN02BA_SIRM_ADDR) = 0;

	/*
	 * bcopy down our brain dead spl code. UGH. Double UGH.
	 */
#if	!RT_PREEMPT
	splsize = ((int)espl0 - (int)&spl0);
	bcopy(kn02ba_spl0, spl0, splsize);
#else	/* !RT_PREEMPT */
	splsize = ((int)espl0 - (int)&spl0x);
	bcopy(kn02ba_spl0, spl0x, splsize);
#endif	/* !RT_PREEMPT */
	bcopy(kn02ba_splsoftclock, splsoftclock, splsize);
	bcopy(kn02ba_splnet, splnet, splsize);
	bcopy(kn02ba_splbio, splbio, splsize);
	bcopy(kn02ba_splimp, splimp, splsize);
	bcopy(kn02ba_spltty, spltty, splsize);
	bcopy(kn02ba_splclock, splclock, splsize);
	bcopy(kn02ba_splvm, splvm, splsize);
	bcopy(kn02ba_splfpu, splfpu, splsize);
#if	!RT_PREEMPT
	splx_size = ((int)esplx - (int)&splx);
	bcopy(kn02ba_splx, splx, splx_size);
#else	/* !RT_PREEMPT */
	splx_size = ((int)esplx - (int)&splx1);
	bcopy(kn02ba_splx, splx1, splx_size);
#endif	/* !RT_PREEMPT */


	/*
	 * Initialize the spl table for the system we are running on.
	 * This must be done very early on at boot time and must
	 * be done before any interrupt is allowed.
	 */
	bcopy(kn02ba_splm, splm, (SPLMSIZE) * sizeof(int));

	/* Change the spl architecture type */
	mips_spl_arch_type = 1;

	/* disable all DMA's, but don't reset the chips */
	*(u_int *)(PHYS_TO_K1(KN02BA_SSR_ADDR)) = 0xf00;

/* rpbfix: do we want to enable halt interrupt at this point */

	splhigh();
	
	/*
	 * Set up the system specific value for "hz", the number of clock
	 * interrupts per second; and corresponding tick and tickadj values.
	 */
	hz = cpup->HZ;
	tick = 1000000 / hz;
	tickadj = 240000 / (60 * hz);
	fixtick = 1000000 - (tick * hz);

	/*
	 * Assign the rt_clock_addr for this processor
	 */
	rt_clock_addr = (char *)PHYS_TO_K1(KN02BA_CLOCK_ADDR);

	clk_counter = kn02ba_conf_clk_speed();

	if (clk_counter < 8775) {
	    kn02ba_model_number = 120;
	    kn_delay_mult = 11;
	} else if (clk_counter < 11311) {
	    kn02ba_model_number = 125;
	    kn_delay_mult = 13;
	} else if (clk_counter < 14000) {
	    kn02ba_model_number = 133;
	    kn_delay_mult = 19;
	} else {
	    kn02ba_model_number = 0;
	    kn_delay_mult = 25;
	}

	/*
	 *
	 */
	sr_usermask = (KN02BA_SR_IMASK0 | SR_IEP | SR_KUP );
	sr_usermask &= ~(SR_IEC);
	sr_kernelmask = (KN02BA_SR_IMASK0 | SR_IEP | SR_IEC );

	/* Initialize the TURBOchannel */
	tc_init();

	/* Fill in the TURBOchannel slot addresses */
	for (i = 0; i < TC_IOSLOTS; i++)
	    tc_slotaddr[i] = kn02ba_slotaddr[i];

	/* Fill in the TURBOchannel switch table */
	tc_sw.isolate_memerr = kn02ba_isolate_memerr;
	tc_sw.enable_option = kn02ba_enable_option;
	tc_sw.disable_option = kn02ba_disable_option;
	tc_sw.clear_errors = kn02ba_clear_errors;
	tc_sw.config_order = kn02ba_config_order;

	/*
	 * Fixed 3min IO devices
	 */

	strcpy(tc_slot[KN02BA_SCSI_INDEX].devname,"asc");
	strcpy(tc_slot[KN02BA_SCSI_INDEX].modulename, "PMAZ-BA ");
	tc_slot[KN02BA_SCSI_INDEX].slot = 3;
	tc_slot[KN02BA_SCSI_INDEX].module_width = 1;
	tc_slot[KN02BA_SCSI_INDEX].physaddr = KN02BA_SCSI_ADDR;
	tc_slot[KN02BA_SCSI_INDEX].intr_b4_probe = 0;
	tc_slot[KN02BA_SCSI_INDEX].intr_aft_attach = 1;
	tc_slot[KN02BA_SCSI_INDEX].adpt_config = 0;

	strcpy(tc_slot[KN02BA_LN_INDEX].devname,"ln");
	strcpy(tc_slot[KN02BA_LN_INDEX].modulename, "PMAD-BA ");
	tc_slot[KN02BA_LN_INDEX].slot = 3;
	tc_slot[KN02BA_LN_INDEX].module_width = 1;
	tc_slot[KN02BA_LN_INDEX].physaddr = KN02BA_LN_ADDR;
	tc_slot[KN02BA_LN_INDEX].intr_b4_probe = 0;
	tc_slot[KN02BA_LN_INDEX].intr_aft_attach = 1;
	tc_slot[KN02BA_LN_INDEX].adpt_config = 0;

	strcpy(tc_slot[KN02BA_SCC_INDEX].devname,"scc");
	tc_slot[KN02BA_SCC_INDEX].slot = 3;
	tc_slot[KN02BA_SCC_INDEX].module_width = 1;
	tc_slot[KN02BA_SCC_INDEX].physaddr = KN02BA_SCC_ADDR;
	tc_slot[KN02BA_SCC_INDEX].intr_b4_probe = 0;
	tc_slot[KN02BA_SCC_INDEX].intr_aft_attach = 1;
	tc_slot[KN02BA_SCC_INDEX].adpt_config = 0;

	*(u_int *)PHYS_TO_K1(KN02BA_INTR_REG) = 0;

/* rpbfix: these should go into their respective drivers */
	*(u_int *)(0xbc040160) = 0x3;
	*(u_int *)(0xbc040170) = 0xe;
	*(u_int *)(0xbc040180) = 0x14;
	*(u_int *)(0xbc040190) = 0x16;

	return (0);
}

kn02ba_conf()
{
	extern u_int cpu_systype;
	extern int icache_size, dcache_size;
	extern int sccintr(), lnintr(), ascintr();
	register struct bus *sysbus;

	/* 
	 * Report what system we are on
	 */
	if (kn02ba_model_number != 0)
	    printf("DECstation 5000 Model %d - system rev %d\n", 
		   kn02ba_model_number,  (GETHRDREV(cpu_systype)));
	else
	    printf("DECstation 5000 Model 100 Series, Unknown Clock Speed\n");

	printf("%dKb Instruction Cache, %dKb Data Cache\n", 
	       icache_size/1024, dcache_size/1024);

	coproc_find();
	
	/* Turn off all interrupts in the SIRM */
	*(u_int *)PHYS_TO_K1(KN02BA_SIRM_ADDR) = 0;
	
	master_cpu = 0;
#if	NCPUS > 1
	printf("Master cpu at slot %d.\n", master_cpu);
#endif	/* NCPUS > 1 */
	machine_slot[master_cpu].is_cpu = TRUE;
        machine_slot[master_cpu].cpu_type = CPU_TYPE_MIPS;
/* TODO: machine_slot[master_cpu].cpu_subtype = subtype; */
        machine_slot[master_cpu].cpu_subtype = CPU_SUBTYPE_MIPS_R3000a;
	machine_slot[master_cpu].running = TRUE;
        machine_slot[master_cpu].clock_freq = hz;

	/*
	 * Probe the TURBOchannel and find all devices
	 */
	Cprintf("kn02ba_conf: Calling get_sys_bus\n");
	/*
	 * Get the system bus structure and call the bus configuration code.
	 * system_bus is global pointer used by loadable code when walking
	 * the hardware topology tree.  It is extern'd via devdriver.h.
	 */
	system_bus = sysbus = get_sys_bus("tc");
	if (sysbus == 0) 
		panic("No system bus configured");
	Cprintf("kn02ba_conf: Calling level 1 configuration routine\n");
	(*sysbus->confl1)(-1, 0, sysbus); /* call level one configuration */
	Cprintf("kn02ba_conf: Calling level 2 configuration routine\n");
	(*sysbus->confl2)(-1, 0, sysbus); /* call level two configuration */
	
	timeout (kn02ba_pscheck, (caddr_t) 0, kn02ba_psintvl * hz);
	timeout (kn02ba_transenable, (caddr_t) 0, kn02ba_transintvl * hz);

	return (0);	/* tell configure() we have configured */
}

/*
 * Check Power Supply over-heat Warning.
 * If its overheating, warn to shut down the system.
 * If its gone from overheat to OK, cancel the warning.
 */
kn02ba_pscheck()
{
	register u_int sir;		/* a copy of the real csr */
	
	sir = *(u_int *)PHYS_TO_K1(KN02BA_SIR_ADDR);
	
	if (sir & PSWARN) {
	    printf("System Overheating - suggest immediate shutdown and power-off\n");
	    kn02ba_pswarn = 1;
	} else {
	    if (kn02ba_pswarn) {
		printf("System OK - cancel overheat shutdown\n");
		kn02ba_pswarn = 0;
	    }
	}
	timeout (kn02ba_pscheck, (caddr_t) 0, kn02ba_psintvl * hz);
}

/*
 * Enable transient parity memory error logging
 */
kn02ba_transenable()
{
	kn02ba_translog = 1;
	timeout (kn02ba_transenable, (caddr_t) 0, kn02ba_transintvl * hz);
}

/*
 * Routine to handle trap errors: user-mode ibe & dbe, & all kernel mode traps.
 * We try to recover from user-mode errors and panic on kernel mode errors.
 */
kn02ba_trap_error(ep, code, sr, cause, signo)
	register u_int *ep;		/* exception frame ptr */
	register u_int code;		/* trap code (trap type) */
	u_int sr, cause;		/* status and cause regs */
	int *signo;			/* set if we want to kill process */
{
	caddr_t pa;			/* the physical addr of the error */	
	int epc;			/* the EPC of the error */	
	unsigned memreg;		/* memory parity error info */
	int vaddr;			/* virt addr of error */
	register struct proc *p;	/* ptr to current proc struct */
	long currtime;			/* current time value */
	unsigned ssr;
	unsigned sir;
	unsigned sirm;
	
	ssr = *(u_int *)(PHYS_TO_K1(KN02BA_SSR_ADDR));
	sir = *(u_int *)(PHYS_TO_K1(KN02BA_SIR_ADDR));
	sirm = *(u_int *)(PHYS_TO_K1(KN02BA_SIRM_ADDR));
	
	p = u.u_procp;
	if (USERMODE(sr)) {
	    /*
	     * If address of bus error is in physical memory, then its
	     * a parity memory error.  Gather additional info in "memreg",
	     * for the error log & to determine how to recover.
	     * If its a transient error then continue the user process.
	     * If its a hard or soft parity error:
	     *    a) on a private process page, terminate the process
	     *	 (by setting signo = SIGBUS)
	     *    b) on a shared page, crash the system.
	     * TBD: on a non-modified page, re-read the page (page fault),
	     *	and continue the process.
	     * TBD: on a shared page terminate all proc's sharing the page,
	     *	instead of crash system.
	     * TBD: on hard errors map out the page.
	     */
	    pa = vatophys(ep[EF_BADVADDR]);
	    if ( (int)pa != -1 && (btop((int)pa) < physmem) ) {
		memreg = kn02ba_isolatepar(pa, ep[EF_BADVADDR], 4);
		/*
		 * If we get 3 or more in 1 second then disable logging
		 * them for 15 minutes.  The variable "kn02ba_translog"
		 * is set by the kn02ba_transenable routine.
		 */
		if (((memreg >> TYPEOFF) & TRANSPAR) == TRANSPAR) {
		    if (kn02ba_translog) {
			currtime = time.tv_sec;
			if (currtime == kn02ba_trans_errcnt.trans_prev) {
			    kn02ba_translog = 0;
			    printf("High rate of transient parity memory errors, logging disabled for 15 minutes\n");
			    kn02ba_trans_errcnt.trans_last = 0;
			    currtime = 0;
			}
			kn02ba_logmempkt(EL_PRIHIGH, ep, memreg, pa);
			kn02ba_trans_errcnt.trans_prev = kn02ba_trans_errcnt.trans_last;
			kn02ba_trans_errcnt.trans_last = currtime;
		    }
		    return(0);
		}
/*** TODO: memory error page info out 		
		if (SHAREDPG(pa)) {	**********/
		    kn02ba_logmempkt(EL_PRISEVERE, ep, memreg, pa);
		    kn02ba_consprint(KN02BA_MEMPKT, ep, memreg, pa, 0, 0, 0);
		    panic("memory parity error");
/* 		} else { 
		    kn02ba_logmempkt(EL_PRIHIGH, ep, memreg, pa);
		    printf("pid %d (%s) was killed on memory parity error\n",
			   p->p_pid, u.u_comm);
		    uprintf("pid %d (%s) was killed on memory parity error\n",
			    p->p_pid, u.u_comm);
	        }	**/
	    } else {
		uprintf("pid %d (%s) was killed on bus error\n",
			p->p_pid, u.u_comm);
	    }
	} else {
	    /*
	     * Kernel mode errors.
	     * They all panic, its just a matter of what we log
	     * and what panic message we issue.
	     */
	    switch (code) {
		
	      case EXC_DBE:
	      case EXC_IBE:
		/*
		 * Figure out if its a memory parity error
		 *     or a read bus timeout error
		 */
		pa = vatophys(ep[EF_BADVADDR]);
		if ( (int)pa != -1 && (btop((int)pa) < physmem) ) {
		    /*
		     * Note that we must save anything "interesting"
		     * from the exception frame, since isolatepar()
		     * may cause additional bus errors which will
		     * stomp on the exception frame in locore.
		     */
		    vaddr = ep[EF_BADVADDR];
		    epc = ep[EF_EPC];
		    memreg = kn02ba_isolatepar(pa, vaddr, 4);
		    ep[EF_BADVADDR] = vaddr;
		    ep[EF_EPC] = epc;
		    kn02ba_logmempkt(EL_PRISEVERE, ep, memreg, pa);
		    kn02ba_consprint(KN02BA_MEMPKT, ep, memreg, pa, 0, 0, 0);
		    panic("memory parity error in kernel mode");
		} else {
		    kn02ba_logesrpkt(EL_PRISEVERE, ep, ssr, sir, sirm);
		    kn02ba_consprint(KN02BA_ESRPKT, ep, 0, 0, ssr, sir, sirm);
		    panic("bus timeout");
		}
		break;
	      case EXC_CPU:
		kn02ba_logesrpkt(EL_PRISEVERE, ep, ssr, sir, sirm);
		kn02ba_consprint(KN02BA_ESRPKT, ep, 0, 0, ssr, sir, sirm);
		panic("coprocessor unusable");
		break;
	      case EXC_RADE:
	      case EXC_WADE:
		kn02ba_logesrpkt(EL_PRISEVERE, ep, ssr, sir, sirm);
		kn02ba_consprint(KN02BA_ESRPKT, ep, 0, 0, ssr, sir, sirm);
		panic("unaligned access");
		break;
	      default:
		kn02ba_logesrpkt(EL_PRISEVERE, ep, ssr, sir, sirm);
		kn02ba_consprint(KN02BA_ESRPKT, ep, 0, 0, ssr, sir, sirm);
		panic("trap");
		break;
	    }
	}
	/*
	 * Default user-mode action is to terminate the process
	 */
	*signo = SIGBUS;
	return(0);
}

extern int atintr_level;

kn02ba_intr(ep, code, sr, cause)
u_int *ep;
u_int code, sr, cause;
{
	register u_int req, sir, tmp_cause;
        register int current_ipl;
	u_int current_sirm = *(u_int *)PHYS_TO_K1(KN02BA_SIRM_ADDR);

	atintr_level++;
	
	/* remove any pending intr's that are masked */
	tmp_cause = cause & sr;
	
	req = ffintr(tmp_cause);
	switch (req) {
	  case CR_HALT:
	    /* loop until the halt button is released */
	    incr_interrupt_counter_type(INTR_TYPE_OTHER);
            while (get_cause() & SR_IBIT7) ;
	    kn02ba_halt(ep);
	    break;
	    
	  case CR_FPU:
	    incr_interrupt_counter_type(INTR_TYPE_OTHER);
	    splx(SPLFPU);
	    fp_intr(ep);
	    break;
	    
	  case CR_SYSTEM:
	    sir = *(u_int *)PHYS_TO_K1(KN02BA_SIR_ADDR);
	    
	    /* mask out bits that we don't want */
	    sir &= current_sirm;
	    
	    if (sir & CPU_IO_TIMEOUT) {
		incr_interrupt_counter_type(INTR_TYPE_OTHER);
		splx(SPLMEM);
		kn02ba_errintr(ep);
	    } else if (sir & TOY_INTR) {
		incr_interrupt_counter_type(INTR_TYPE_HARDCLK);
		splx(SPLCLOCK);
		kn_hardclock(ep);
	    } else if (sir & SLU_INTR) {
		incr_interrupt_counter_type(INTR_TYPE_DEVICE);
		splx(SPLIO);
		(*(tc_slot[KN02BA_SCC_INDEX].intr))();
	    } else if (sir & LANCE_INTR) {
		incr_interrupt_counter_type(INTR_TYPE_DEVICE);
		splx(SPLIO);
		(*(tc_slot[KN02BA_LN_INDEX].intr))();
	    } else if (sir & SCSI_INTR) {
		incr_interrupt_counter_type(INTR_TYPE_DEVICE);
		splx(SPLIO);
		(*(tc_slot[KN02BA_SCSI_INDEX].intr))();
	    } else {
		incr_interrupt_counter_type(INTR_TYPE_STRAY);
		/* rpbfix: downgrade to a printf after debugging */
		printf("Stray interrupt from System Interrupt Register");
	    }
	    break;
	    
	  case CR_OPTION_2:
	    incr_interrupt_counter_type(INTR_TYPE_DEVICE);
	    splx(SPLIO);
	    (*(tc_slot[2].intr))(tc_slot[2].param);
	    break;
	    
	  case CR_OPTION_1:
	    incr_interrupt_counter_type(INTR_TYPE_DEVICE);
	    splx(SPLIO);
	    (*(tc_slot[1].intr))(tc_slot[1].param);
	    break;
	    
	  case CR_OPTION_0:
	    incr_interrupt_counter_type(INTR_TYPE_DEVICE);
	    splx(SPLIO);
	    (*(tc_slot[0].intr))(tc_slot[0].param);
	    break;
	    
	  case CR_SOFTNET:
	    incr_interrupt_counter_type(INTR_TYPE_SOFTCLK);
	    splx(SPLNET);
	    softintr2(ep);
	    break;
	    
	  case CR_SOFTCLOCK:
	    incr_interrupt_counter_type(INTR_TYPE_SOFTCLK);
	    splx(SPLSOFTC);
	    softclock(ep);
	    break;
	    
	  case CR_NONE:
	    /* rpbfix: downgrade to printf later */
	    incr_interrupt_counter_type(INTR_TYPE_STRAY);
	    /*	    panic("no interrupt pending");*/
/* Don't define this, because the SII generates strays for TZ30 & RRD42 */
#ifdef  TRAP_LOG_STRAYS
	    printf("no interrupt pending, cause = %x, sr = %x\n", cause, sr);
#endif
	    break;
	}
	
	atintr_level--;
}

int	kn02ba_wto_event = 0;
int	kn02ba_wto_pfn = 0;

kn02ba_errintr(ep)
    register u_int *ep;		/* exception frame ptr */
{
        register unsigned cpu_addr_err;
	register unsigned ssr;
	register unsigned sir;
	register unsigned sirm;
	struct kn02ba_consinfo_t kn02ba_consinfo, *p;
	
	ssr = *(u_int *)(PHYS_TO_K1(KN02BA_SSR_ADDR));
	sir = *(u_int *)(PHYS_TO_K1(KN02BA_SIR_ADDR));
	sirm = *(u_int *)(PHYS_TO_K1(KN02BA_SIRM_ADDR));
	
	cpu_addr_err = *(u_int *)PHYS_TO_K1(KN02BA_ADDR_ERR);
	ep[EF_BADVADDR] = cpu_addr_err;
	kn02ba_logesrpkt(EL_PRISEVERE, ep, ssr, sir, sirm);
	kn02ba_consprint(KN02BA_ESRPKT, ep, 0, 0, ssr, sir, sirm);
	*(u_int *)PHYS_TO_K1(KN02BA_INTR_REG) = 0;
	wbflush();
	panic("CPU write timeout");
}

kn02ba_stray()
{
        /* rpbfix: downgrade to printf */
        panic("Received stray interrupt");
}

halt_cnt = 0;

kn02ba_halt(ep)
u_int *ep;
{
    /* print out value of PC, SP, and EP with labels */
    rex_printf("\nPC:\t0x%x\nSP:\t0x%x\nEP:\t0x%x\n\n", ep[EF_EPC], ep[EF_SP], ep);
    rex_halt(0,0);
}

kn02ba_enable_option(index)
    register int index;
{
        register int i;
	
	switch (index) {
	  case KN02BA_SCC_INDEX:
	    for (i = 0; i < SPLIO; i++)
		kn02ba_sim[i] |= (SLU_INTR);
	    break;
	    
	  case KN02BA_LN_INDEX:
	    for (i = 0; i < SPLIO; i++)
		kn02ba_sim[i] |= (LANCE_INTR);
	    break;
	    
	  case KN02BA_SCSI_INDEX:
	    /* rpbfix: reenable when scsi is really turned on */
	    for (i = 0; i < SPLIO; i++)
		kn02ba_sim[i] |= (SCSI_INTR);
	    break;
	    
	  case TC_OPTION_SLOT_0:
	    for (i = 0; i < SPLIO; i++)
		splm[i] |= SR_IBIT3;
	    sr_usermask |= SR_IBIT3;
	    sr_kernelmask |= SR_IBIT3;
	    break;
	    
	  case TC_OPTION_SLOT_1:
	    for (i = 0; i < SPLIO; i++)
		splm[i] |= SR_IBIT4;
	    sr_usermask |= SR_IBIT4;
	    sr_kernelmask |= SR_IBIT4;
	    break;
	    
	  case TC_OPTION_SLOT_2:
	    for (i = 0; i < SPLIO; i++)
		splm[i] |= SR_IBIT5;
	    sr_usermask |= SR_IBIT5;
	    sr_kernelmask |= SR_IBIT5;
	    break;
	    
	  default:
	    /* rpbfix: downgrade to printf */
	    panic("Enable_option call to non existent slot");
	    break;
	}
	
	/* load the registers with the new values */
	splx(ipllevel);
}

kn02ba_disable_option(index)
    register int index;
{
        register int i;

	switch (index) {
	  case KN02BA_SCC_INDEX:
	    for (i = 0; i < SPLIO; i++)
		kn02ba_sim[i] &= ~(SLU_INTR);
	    break;
	    
	  case KN02BA_LN_INDEX:
	    for (i = 0; i < SPLIO; i++)
		kn02ba_sim[i] &= ~(LANCE_INTR);
	    break;
	    
	  case KN02BA_SCSI_INDEX:
	    for (i = 0; i < SPLIO; i++)
		kn02ba_sim[i] &= ~(SCSI_INTR);
	    break;
	    
	  case TC_OPTION_SLOT_0:
	    for (i = 0; i < SPLIO; i++)
		splm[i] &= ~(SR_IBIT3);
	    sr_usermask &= ~(SR_IBIT3);
	    sr_kernelmask &= ~(SR_IBIT3);
	    break;
	    
	  case TC_OPTION_SLOT_1:
	    for (i = 0; i < SPLIO; i++)
		splm[i] &= ~(SR_IBIT4);
	    sr_usermask &= ~(SR_IBIT4);
	    sr_kernelmask &= ~(SR_IBIT4);
	    break;
	    
	  case TC_OPTION_SLOT_2:
	    for (i = 0; i < SPLIO; i++)
		splm[i] &= ~(SR_IBIT5);
	    sr_usermask &= ~(SR_IBIT5);
	    sr_kernelmask &= ~(SR_IBIT5);
	    break;
	    
	  default:
	    /* rpbfix: downgrade to printf */
	    panic("disable_option call to non existent slot");
	    break;
	}
	
	/* load the registers with the new values */
	splx(ipllevel);
}

kn02ba_clear_errors()
{
}

#define KN02BA_LOG_ESRPKT(elrp, cause,epc,sr,badva,sp,ssr,sir,sirm)	\
	elrp->el_body.elesr.elesr.el_esrkn02ba.esr_cause = cause;	\
	elrp->el_body.elesr.elesr.el_esrkn02ba.esr_epc = epc;		\
	elrp->el_body.elesr.elesr.el_esrkn02ba.esr_status = sr;		\
	elrp->el_body.elesr.elesr.el_esrkn02ba.esr_badva = badva;	\
	elrp->el_body.elesr.elesr.el_esrkn02ba.esr_sp = sp;		\
	elrp->el_body.elesr.elesr.el_esrkn02ba.esr_ssr = ssr;		\
	elrp->el_body.elesr.elesr.el_esrkn02ba.esr_sir = sir;		\
	elrp->el_body.elesr.elesr.el_esrkn02ba.esr_sirm = sirm;		\

/*
 * Log Error & Status Registers to the error log buffer
 */
kn02ba_logesrpkt(priority, ep, ssr, sir, sirm)
	int priority;		/* for pkt priority */
	register u_int *ep;	/* exception frame ptr */
	u_int ssr;
	u_int sir;
	u_int sirm;
{
	struct el_rec *elrp;
	
	elrp = ealloc(sizeof(struct el_esr), priority);
	if (elrp != NULL) {
	    LSUBID(elrp,ELCT_MCK,ELESR_KN02BA,EL_UNDEF,EL_UNDEF,EL_UNDEF,EL_UNDEF);
	    KN02BA_LOG_ESRPKT(elrp, ep[EF_CAUSE], ep[EF_EPC], ep[EF_SR], ep[EF_BADVADDR], 
			      ep[EF_SP], ssr, sir, sirm);
	    
	    EVALID(elrp);
	}

	log(LOG_ERR, "kn02ba error: Cause 0x%x PC 0x%x Status 0x%x Bad VA 0x%x\n",
	    ep[EF_CAUSE], ep[EF_EPC], ep[EF_SR], ep[EF_BADVADDR]);
	log(LOG_ERR, "Sp 0x%x ssr 0x%x sir 0x%x sirm 0x%x\n",
	    ep[EF_SP], ssr, sir, sirm);
}

/*
 * Log a memory error packet, so uerf can find it as a main memory error.
 */
kn02ba_logmempkt(priority, ep, memreg, pa)
	int priority;		/* pkt priority: panic: severe; else: high */
	register u_int *ep;	/* exception frame ptr */
	unsigned memreg;	/* assorted parity error info */
	int pa;			/* physical addr where memory err occured */
{
	struct el_rec *elrp;
	register struct el_mem *mrp;
	
	elrp = ealloc(EL_MEMSIZE, priority);
	if (elrp != NULL) {
	    LSUBID(elrp,ELCT_MEM,EL_UNDEF,ELMCNTR_KN02BA,EL_UNDEF,EL_UNDEF,EL_UNDEF);
	    mrp = &elrp->el_body.elmem;
	    mrp->elmem_cnt = 1;
	    mrp->elmemerr.cntl = 1;
	    mrp->elmemerr.type = ELMETYP_PAR;
	    mrp->elmemerr.numerr = 1;
	    mrp->elmemerr.regs[0] = memreg;
	    mrp->elmemerr.regs[1] = pa;
	    mrp->elmemerr.regs[2] = ep[EF_EPC];;
	    mrp->elmemerr.regs[3] = ep[EF_BADVADDR];;
	    EVALID(elrp);
	}

	log(LOG_ERR, "kn02ba memory parity error: Mem reg 0x%x Pa 0x%x PC 0x%x Bad VA 0x%x\n",
	    memreg, pa, ep[EF_EPC], ep[EF_BADVADDR]);
}

/*
 * 	Logs error information to the error log buffer.
 *	Exported through the cpu switch.
 */

kn02ba_log_errinfo(p)
struct kn02ba_consinfo_t *p;
{
	struct kn02ba_consinfo_esr_t *esrp;
	struct el_rec *elrp;
	
	switch (p->pkt_type) {
	    
	  case KN02BA_ESRPKT:
	    esrp = &(p->pkt.esrp);
	    elrp = ealloc(sizeof(struct el_esr), EL_PRISEVERE);
	    if (elrp != NULL) {
		LSUBID(elrp,ELCT_MCK,ELESR_KN02BA,EL_UNDEF,EL_UNDEF,EL_UNDEF,EL_UNDEF);
		KN02BA_LOG_ESRPKT(elrp, esrp->cause, esrp->epc, esrp->status, esrp->badva, 
				  esrp->sp, esrp->ssr, esrp->sir, esrp->sirm);
		EVALID(elrp);
	    }
	    break;
	    
	  default: 
	    printf("bad pkt type\n");
	    return;
	}

	log(LOG_ERR, "kn02ba error: Cause 0x%x PC 0x%x Status 0x%x Bad VA 0x%x\n",
	    esrp->cause, esrp->epc, esrp->status, esrp->badva);
	log(LOG_ERR, "Sp 0x%x ssr 0x%x sir 0x%x sirm 0x%x\n",
	    esrp->sp, esrp->ssr, esrp->sir, esrp->sirm);
}

/*
 * Print error packet to the console.
 * This is only done when we are about to panic on the error.
 * It calls kn02_print_consinfo to actually print the information.
 *
 */
kn02ba_consprint(pkt, ep, memreg, pa, ssr, sir, sirm)
	int pkt;		/* error pkt: Error & Stat Regs / memory pkt */
	register u_int *ep;	/* exception frame ptr */
	unsigned memreg;	/* For MEMPKT: assorted parity error info */
	unsigned pa;		/* For MEMPKT: physical addr of error */	
	unsigned ssr;
	unsigned sir;
	unsigned sirm;
{
	register int i;
	struct kn02ba_consinfo_t p;
	
	
	p.pkt_type = pkt;
	switch (pkt) {
	  case KN02BA_ESRPKT:
	    p.pkt.esrp.cause	= ep[EF_CAUSE];
	    p.pkt.esrp.epc		= ep[EF_EPC];
	    p.pkt.esrp.status	= ep[EF_SR];
	    p.pkt.esrp.badva	= ep[EF_BADVADDR];
	    p.pkt.esrp.sp		= ep[EF_SP];
	    p.pkt.esrp.ssr		= ssr;
	    p.pkt.esrp.sir		= sir;
	    p.pkt.esrp.sirm		= sirm;
	    break;
	    
	  case KN02BA_MEMPKT:
	    p.pkt.memp.epc		= ep[EF_EPC];
	    p.pkt.memp.badva	= ep[EF_BADVADDR];
	    p.pkt.memp.memreg	= memreg;
	    p.pkt.memp.pa		= pa;
	    break;
	    
	  default:
	    printf("bad consprint\n");
	    return;
	}
	kn02ba_print_consinfo(&p);
	
}

/*
 *	This routine is similar to kn02consprint().
 *	This is exported through the cpusw structure. 
 *	
 */

kn02ba_print_consinfo(p)
struct kn02ba_consinfo_t *p;
{
        int simm, byte;
	u_int memreg;
	
	/*
	 * If console is a graphics device,
	 * force printf messages directly to screen.
	 */
/* TODO:	printstate |= PANICPRINT; */
	
	switch (p->pkt_type) {
	  case KN02BA_ESRPKT:
	    printf("\nException condition\n");
	    printf("\tCause reg\t= 0x%x\n", p->pkt.esrp.cause);
	    printf("\tException PC\t= 0x%x\n", p->pkt.esrp.epc);
	    printf("\tStatus reg\t= 0x%x\n", p->pkt.esrp.status);
	    printf("\tBad virt addr\t= 0x%x\n", p->pkt.esrp.badva);
	    printf("\tStack ptr\t= 0x%x\n", p->pkt.esrp.sp);
	    printf("\tSystem Support reg = 0x%x\n", p->pkt.esrp.ssr);
	    printf("\tSystem Interrupt reg = 0x%x\n", p->pkt.esrp.sir);
	    printf("\tSystem Interrupt Mask reg = 0x%x\n", p->pkt.esrp.sirm);
	    break;
	    
	  case KN02BA_MEMPKT:
	    memreg = p->pkt.memp.memreg;
	    printf("\nMemory Parity Error\n");
	    simm =  (memreg >> SIMMOFF) & 0xf;
	    printf("\tSIMM (module number)\t= BANK %d, %s\n", 
		    simm/2, ((simm & 0x1) ? "D16-31" : "D0-15"));
	    if (((memreg >> TYPEOFF) & HARDPAR) == HARDPAR)
		printf("\tHard error\t\n");
	    else if (((memreg >> TYPEOFF) & SOFTPAR) == SOFTPAR)
		printf("\tSoft error\t\n");
	    else printf("\tTransient error\t\n");
	    if (simm & 0x1) {
		/* D16-31(high) simm: high half word */
		if ((memreg >> BYTEOFF) & 0x1)
		    byte = 3;
		else
		    byte = 2;
	    } else {
		/* D0-15(low) simm: low half word */
		if ((memreg >> BYTEOFF) & 0x1)
		    byte = 1;
		else
		    byte = 0;
	    }
	    printf("\tByte in error (0-3)\t= %d\n", byte);
	    printf("\t%s bit error\n", ((memreg >> DPOFF) & 0x1) ? "Parity" : "Data");
	    printf("\tTransient errors for this SIMM\t= %d\n", kn02ba_tcount[simm]);
	    printf("\tSoft errors for this SIMM\t= %d\n", kn02ba_scount[simm]);
	    printf("\tHard errors for this SIMM\t= %d\n", kn02ba_hcount[simm]);
	    printf("\tPhysical address of error\t= 0x%x\n", p->pkt.memp.pa);
	    printf("\tException PC\t\t\t= 0x%x\n", p->pkt.memp.epc);
	    printf("\tVirtual address of error\t= 0x%x\n", p->pkt.memp.badva);
	    break;
	    
	  default:
	    printf("bad print_consinfo \n");
	    break;
	}
}

kn02ba_isolate_memerr(memerr_status)
	struct tc_memerr_status *memerr_status;
{
        unsigned memreg;
	int ep[EF_SIZE/4];

	if (btop((int)memerr_status->pa) >= physmem)
	    return (-1);

	/* zero out these since they are not pertinent 	*/
	/* for this type of error			*/
	ep[EF_EPC] = 0;
	ep[EF_BADVADDR] = 0;
	
	memreg = kn02ba_isolatepar(memerr_status->pa, memerr_status->va, memerr_status->blocksize);

	memerr_status->errtype = TC_MEMERR_NOERROR;

	if (((memreg >> TYPEOFF) & HARDPAR) == HARDPAR)
	    memerr_status->errtype = TC_MEMERR_HARD;
	else if (((memreg >> TYPEOFF) & SOFTPAR) == SOFTPAR)
	    memerr_status->errtype = TC_MEMERR_SOFT;
	else if (((memreg >> TYPEOFF) & TRANSPAR) == TRANSPAR)
	    memerr_status->errtype = TC_MEMERR_TRANS;

	if (memerr_status->log == TC_LOG_MEMERR) {
	    kn02ba_logmempkt(EL_PRISEVERE, ep, memreg, memerr_status->pa);
	    kn02ba_consprint(KN02BA_MEMPKT, ep, memreg, memerr_status->pa, 0, 0, 0);
	}

}

/*
 * Isolate a memory parity error to which SIMM is in error.
 * This routine is machine specific, in that it "knows" how the memory
 * is laid out, i.e. how to convert a physical address to a module number.
 *
 * Block faults from occuring while we isolate the parity error by using
 * "nofault" facility thru the bbadaddr routine.
 */
unsigned
kn02ba_isolatepar(pa, va, blocksize)
	register caddr_t pa;	/* the phys addr to convert to a SIMM */	
	caddr_t va;		/* the virtual addr of the error */	
	int blocksize;		/* the size of the block error occured in */
{
	register int i;		/* loop index */	
	register int *blockaddr;/* address of the beginning of block in error */
	register int *addr;	/* increment thru the block w/ parity error */
	register char *baddr;	/* increment thru the word w/ parity error */
	unsigned memreg;	/* collection of memory error info */
	int low;		/* true if its the D0-15(low) SIMM */
	int simm;		/* which simm had the error */
	register int allzeros;	/* true if parity err occurs on all 0's write */
	register int allones;	/* true if parity err occurs on all 1's write */
	register int oneone;	/* true if parity err occurs on 1 1 write */
	int dp;			/* 0 for data bit, 1 for parity bit */
	int type;		/* error type: transient, soft, hard */
	int byte;		/* 0 for low byte; 1 for high byte in word */
	vm_offset_t phys;
	int bank;
	int banksize;
	int parityerr;
	int blockcnt;


	/* 
	 * Round physical address to beginning of block
	 */
	blockaddr = (int *)(PHYS_TO_K1((int)pa & ~((blocksize << 2) - 1)));
	addr = blockaddr;
	for (blockcnt = 0; blockcnt < blocksize; blockcnt++, addr++) {
	
	    type = 0;
	    dp = 0;
	    /*
	     * Do badaddr probe on addr (a few times),
	     * to see if it was only a transient.
	     */
	    parityerr = 0;
	    for (i = 0; i < 4; i++) {
		if (bbadaddr(addr, 4, 0)) {
		    parityerr = 1;
		    break;
		}
	    }
	    if (!parityerr) {
		/* if no error, try the next word */
		continue;
	    }
	    /*
	     * Isolate the parity error to which SIMM is in error (which byte in
	     * the word) and isolate the type of error: soft or hard, data bit
	     * or parity bit.
	     *
	     * This is done by writing (& reading) each byte in the word first
	     * with all 0's then with all 1's (0xff) then with one 1 (0x1).
	     *
	     * Use k1 address in order not to get TLBMOD exception when writing
	     * shared memory space.
	     */
	    for (i = 0, baddr = (char *)addr; i < 4; i++, baddr += 1) {
		allzeros = 0;
		*baddr = 0x00;
		if (bbadaddr(baddr, 1, 0))
		    allzeros = 1;
		allones = 0;
		*baddr = 0xff;
		if (bbadaddr(baddr, 1, 0))
		    allones = 1;
		oneone = 0;
		*baddr = 0x1;
		if (bbadaddr(baddr, 1, 0))
		    oneone = 1;
		/*
		 * If all 3 reads caused the error then this is the wrong
		 * byte, go on to the next byte
		 */
		if (allzeros && allones && oneone)
		    continue;
		/*
		 * If only one of the allones/allzeros patterns caused a
		 * parity error, then we have a hard data bit stuck to
		 * zero or one.
		 */
		if ((allzeros && !allones && !oneone) ||
		    (allones && !allzeros && !oneone)) {
		    type = HARDPAR;
		    break;
		}
		/*
		 * If only the "oneone" (0x1) pattern caused a parity error,
		 *   then we have a parity bit stuck to zero.
		 * If only the "oneone" (0x1) pattern did NOT cause a parity
		 *   error then we have a parity bit stuck to one.
		 */
		if ((oneone && !allzeros && !allones) ||
		    (allzeros && allones && !oneone)) {
		    type = HARDPAR;
		    dp = 1;
		    break;
		}
		/*
		 * If no parity error on all 3 patterns then we had a soft
		 * parity error in one of the data bits or in the parity bit
		 * of this byte.
		 */
		if (!allzeros && !allones && !oneone) {
		    type = SOFTPAR;
		    break;
		}
	    }
	    /*
	     * If i is 0 or 1, parity error is on the D0-15(low) SIMM.
	     * If i is 2 or 3, parity error is on the D16-31(high) SIMM.
	     * Also record high or low byte position in half-word.
	     */
	    switch (i) {
	      case 0:
		byte = 0;
		low = 1;
		break;
	      case 1:
		byte = 1;
		low = 1;
		break;
	      case 2:
		byte = 0;
		low = 0;
		break;
	      case 3:
	      default:
		byte = 1;
		low = 0;
		break;
	    }
	    /* we found the bad word, now determine which simm */
	    break;
	}
	
	/* if none of the words checked found an error, the error must */
	/* have been a transient parity error. */
	if (!parityerr) {
	    unsigned int mer;

	    type = TRANSPAR;
	    mer = *(u_int *)(PHYS_TO_K1(KN02BA_MEM_ERR));
	    mer &= LAST_BYTE_ERR_MASK;
	    if (mer & 0x800) {
		byte = 1;
		low = 0;
	    } else if (mer & 0x400) {
		byte = 0;
		low = 0;
	    } else if (mer & 0x200) {
		byte = 1;
		low = 1;
	    } else {
		byte = 0;
		low = 1;
	    }
	    /* clear error bits */
	    *(u_int *)(PHYS_TO_K1(KN02BA_MEM_ERR)) = 0;
	    baddr = (char *)blockaddr;
	}

	if (*((u_int *)PHYS_TO_K1(KN02BA_MEM_SIZE)) & KN02BA_16MB_MEM) 
	    banksize = 16 * 1024 * 1024;
	else
	    banksize = 4 * 1024 * 1024;
	
	/* There are 8 banks, numbered 0 - 7 */
	svatophys(baddr, &phys);
	bank = phys / banksize;
	
	/* There are 16 simms, numbered 0 - 15 */
	if (low)
	    simm = (bank * 2);
	else
	    simm = (bank * 2) + 1;
	/*
	 * Increment error counts
	 */
	switch (type) {
	  case TRANSPAR:
	  default:
	    kn02ba_tcount[simm]++;
	    if (kn02ba_tcount[simm] > 255) {
		printf("Transient parity error count on simm in BANK # %d, %s reached 255, reset to zero.\n", bank, (low ? "D0-15" : "D16-31"));
		kn02ba_tcount[simm] = 0;
	    }
	    break;
	  case SOFTPAR:
	    kn02ba_scount[simm]++;
	    break;
	  case HARDPAR:
	    kn02ba_hcount[simm]++;
	    break;
	}
	memreg = MEMREGFMT(simm, type, byte, dp, kn02ba_tcount[simm], 
			   kn02ba_scount[simm], kn02ba_hcount[simm]);
	return(memreg);
}

kn02ba_conf_clk_speed()
{
    register volatile struct rt_clock *rt =(struct rt_clock *)rt_clock_addr;
    register volatile int dummy;
    register int s, counter = 0;
    int save_rega, save_regb;

    s = splhigh();

    /* allow TOY interrupt to get to the CPU */
    *(u_int *)(PHYS_TO_K1(KN02BA_SIRM_ADDR)) = TOY_INTR;

    /* enable periodic interrupt */
    save_rega = rt->rt_rega;
    save_regb = rt->rt_regb;
    rt->rt_rega = RTA_DV32K|RTA_4ms;
    rt->rt_regb = RTB_DMBINARY|RTB_24HR|RTB_PIE;

    /* clear any old interrupts */
    dummy = rt->rt_regc;

    /* wait for start */
    while ((get_cause() & SR_IBIT6) == 0);

    dummy = rt->rt_regc;

    /* wait for finish and count */
    while ((get_cause() & SR_IBIT6) == 0)
	counter++;

    dummy = rt->rt_regc;
    rt->rt_rega = save_rega;
    rt->rt_regb = save_regb;

    splx(s);
    return (counter);
}
