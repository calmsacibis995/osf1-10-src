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
static char	*sccsid = "@(#)$RCSfile: genassym.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:40:43 $";
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
 * Copyright (c) 1988 Carnegie-Mellon University
 * Copyright (c) 1987 Carnegie-Mellon University
 * All rights reserved.  The CMU software License Agreement specifies
 * the terms and conditions for use and redistribution.
 */
/*
 * OSF/1 Release 1.0
 */
/*
 * Copyright (C) 1988,1989 Encore Computer Corporation.  All Rights Reserved
 *
 * Property of Encore Computer Corporation.
 * This software is made available solely pursuant to the terms of
 * a software license agreement which governs its use. Unauthorized
 * duplication, distribution or sale are strictly prohibited.
 *
 */

#if	defined(ns32000) && !defined(__ns32000__)
#define	__ns32000__
#endif

#include <mmax_xpc.h>
#include <mmax_apc.h>
#include <mmax_dpc.h>

#include <mmax/pte.h>

#include <sys/param.h>
#include <sys/buf.h>
#include <sys/vmparam.h>
#include <sys/user.h>
#include <sys/map.h>
#include <sys/proc.h>
#include <sys/msgbuf.h>
#include <sys/reboot.h>
#include <sys/signal.h>

#include <kern/thread.h>
#include <kern/task.h>

thread_t	active_threads[1];
/* struct	u_address	active_uareas[1]; */

#include <mach/mmax/vm_param.h>
#include <mmaxio/io.h>
#include <mmaxio/crqdefs.h>
#include <mmax/sccdefs.h>
#include <mmax/panic.h>
#include <mmax/boot.h>
#if	MMAX_XPC || MMAX_APC
#include <mmax/duart.h>
#endif	MMAX_XPC || MMAX_APC

main()
{
	struct proc *p = (struct proc *)0;
	struct utask *utaskp = (struct utask *)0;
	struct uthread *uthreadp = (struct uthread *)0; 
	struct thread *thread = (struct thread *) 0;
	struct pcb *pcb = (struct pcb *) 0;
	struct task *task = (struct task *) 0;
	struct itbl *itbl = (struct itbl *) 0;
	struct nmilock *nmilock = (struct nmilock *) 0;
	struct uuprof *prof = (struct uuprof *) 0;
	struct u_address *uaddr = (struct u_address *)0;
	struct psa *psa = (struct psa *) 0;

	printf("#ifdef ASSEMBLER\n");
	printf("#define\tUSRSTACK 0x%x\n", USRSTACK);
	printf("#define\tKERNEL_STACK_SIZE %d\n", KERNEL_STACK_SIZE);
	printf("#define\tP_CURSIG %d\n", &p->p_cursig);
	printf("#define\tP_SIG %d\n", &p->p_sig);
	printf("#define\tU_PROCP %d\n", &utaskp->uu_procp);
	printf("#define\tPCB_R0 %d\n", &pcb->pcb_r0);
	printf("#define\tPCB_R1 %d\n", &pcb->pcb_r1);
	printf("#define\tPCB_R2 %d\n", &pcb->pcb_r2);
	printf("#define\tPCB_R3 %d\n", &pcb->pcb_r3);
	printf("#define\tPCB_R4 %d\n", &pcb->pcb_r4);
	printf("#define\tPCB_R5 %d\n", &pcb->pcb_r5);
	printf("#define\tPCB_R6 %d\n", &pcb->pcb_r6);
	printf("#define\tPCB_R7 %d\n", &pcb->pcb_r7);
	printf("#define\tPCB_FP %d\n", &pcb->pcb_fp);
	printf("#define\tPCB_USP %d\n", &pcb->pcb_usp);
	printf("#define\tPCB_SSP %d\n", &pcb->pcb_ssp);
	printf("#define\tPCB_PTBR %d\n", &pcb->pcb_ptbr);
	printf("#define\tPCB_PC %d\n", &pcb->pcb_pc);
	printf("#define\tPCB_MODPSR %d\n", &pcb->pcb_modpsr);
	printf("#define\tPCB_F0 %d\n", &pcb->pcb_f0);
	printf("#define\tPCB_F1 %d\n", &pcb->pcb_f1);
	printf("#define\tPCB_F2 %d\n", &pcb->pcb_f2);
	printf("#define\tPCB_F3 %d\n", &pcb->pcb_f3);
	printf("#define\tPCB_F4 %d\n", &pcb->pcb_f4);
	printf("#define\tPCB_F5 %d\n", &pcb->pcb_f5);
	printf("#define\tPCB_F6 %d\n", &pcb->pcb_f6);
	printf("#define\tPCB_F7 %d\n", &pcb->pcb_f7);
	printf("#define\tPCB_FSR %d\n", &pcb->pcb_fsr);
#if	MMAX_XPC || MMAX_APC
	printf("#define\tPCB_ISRV %d\n", &pcb->pcb_isrv);
#endif	MMAX_XPC || MMAX_APC
	printf("#define\tTHREAD_PCB %d\n", &thread->pcb);
	printf("#define\tTHREAD_RECOVER %d\n", &thread->recover);
	printf("#define\tTHREAD_KERNEL_STACK %d\n", &thread->kernel_stack);
	printf("#define\tTHREAD_UTASK %d\n", &thread->u_address.utask);
	printf("#define\tTHREAD_UTHREAD %d\n", &thread->u_address.uthread);
	printf("#define\tUADDR_UTASK %d\n", &uaddr->utask);
	printf("#define\tUADDR_UTHREAD %d\n", &uaddr->uthread);
	printf("#define\tITBL_SHIFT\t3\n");
	printf("#define\tPSA_PANIC_VERSION %d\n", PSA_PANIC_VERSION);
	printf("#define\tPSA_DPC_TYPE %d\n", PSA_DPC_TYPE);
	printf("#define\tPSA_APC_TYPE %d\n", PSA_APC_TYPE);
	printf("#define\tPSA_XPC_TYPE %d\n", PSA_XPC_TYPE);
	printf("#define\tPSA_VERSION %d\n", &psa->psa_version);
	printf("#define\tPSA_SIZE %d\n", &psa->psa_size);
	printf("#define\tPSA_STRUCT_SIZE %d\n", sizeof(*psa));
	printf("#define\tPSA_CPUTYPE %d\n", &psa->psa_cputype);
	printf("#define\tPSA_USP %d\n", &psa->psa_usp);
	printf("#define\tPSA_R7 %d\n", &psa->psa_r7);
	printf("#define\tPSA_R6 %d\n", &psa->psa_r6);
	printf("#define\tPSA_R5 %d\n", &psa->psa_r5);
	printf("#define\tPSA_R4 %d\n", &psa->psa_r4);
	printf("#define\tPSA_R3 %d\n", &psa->psa_r3);
	printf("#define\tPSA_R2 %d\n", &psa->psa_r2);
	printf("#define\tPSA_R1 %d\n", &psa->psa_r1);
	printf("#define\tPSA_R0 %d\n", &psa->psa_r0);
	printf("#define\tPSA_FP %d\n", &psa->psa_fp);
	printf("#define\tPSA_PC %d\n", &psa->psa_pc);
	printf("#define\tPSA_PSR %d\n", &psa->psa_psr);
	printf("#define\tPSA_SSP %d\n", &psa->psa_ssp);
	printf("#define\tPSA_STKFRM_SIZE %d\n",
	       (int)&psa->psa_frame - (int) &psa->psa_frame_base);
	printf("#define\tPSA_SB %d\n", &psa->psa_sb);
	printf("#define\tPSA_INTBASE %d\n", &psa->psa_intbase);
	printf("#define\tPSA_CFG %d\n", &psa->psa_cfg);
	printf("#define\tPSA_PTB0 %d\n", &psa->psa_ptb0);
	printf("#define\tPSA_PTB1 %d\n", &psa->psa_ptb1);
#if	MMAX_XPC
	printf("#define\tPSA_MCR %d\n", &psa->psa_mcr);
	printf("#define\tPSA_MSR %d\n", &psa->psa_msr);
	printf("#define\tPSA_TEAR %d\n", &psa->psa_tear);
	printf("#define\tPSA_BPC %d\n", &psa->psa_bpc);
	printf("#define\tPSA_CAR %d\n", &psa->psa_car);
	printf("#define\tPSA_DCR %d\n", &psa->psa_dcr);
	printf("#define\tPSA_DSR %d\n", &psa->psa_dsr);
#endif	MMAX_XPC
#if	MMAX_APC
	printf("#define\tPSA_FEW %d\n", &psa->psa_few);
	printf("#define\tPSA_ASR %d\n", &psa->psa_asr);
	printf("#define\tPSA_TEAR %d\n", &psa->psa_tear);
	printf("#define\tPSA_BEAR %d\n", &psa->psa_bear);
	printf("#define\tPSA_BAR %d\n", &psa->psa_bar);
	printf("#define\tPSA_BMR %d\n", &psa->psa_bmr);
	printf("#define\tPSA_BDR %d\n", &psa->psa_bdr);
#endif	MMAX_APC
#if	MMAX_DPC
	printf("#define\tPSA_MSR %d\n", &psa->psa_msr);
	printf("#define\tPSA_EIA %d\n", &psa->psa_eia);
	printf("#define\tPSA_BPR0 %d\n", &psa->psa_bpr0);
	printf("#define\tPSA_BPR1 %d\n", &psa->psa_bpr1);
	printf("#define\tPSA_BCNT %d\n", &psa->psa_bcnt);
#endif	MMAX_DPC
	printf("#define\tPSA_FSR %d\n", &psa->psa_fsr);
	printf("#define\tPSA_F0 %d\n", &psa->psa_f0);
	printf("#define\tPSA_F1 %d\n", &psa->psa_f1);
	printf("#define\tPSA_F2 %d\n", &psa->psa_f2);
	printf("#define\tPSA_F3 %d\n", &psa->psa_f3);
	printf("#define\tPSA_F4 %d\n", &psa->psa_f4);
	printf("#define\tPSA_F5 %d\n", &psa->psa_f5);
	printf("#define\tPSA_F6 %d\n", &psa->psa_f6);
	printf("#define\tPSA_F7 %d\n", &psa->psa_f7);
#if	MMAX_XPC || MMAX_APC
	printf("#define\tPSA_ICU_IPND %d\n", &psa->psa_icu_ipnd);
	printf("#define\tPSA_ICU_IMSK %d\n", &psa->psa_icu_imsk);
	printf("#define\tPSA_ICU_ISRV %d\n", &psa->psa_icu_isrv);
#endif	MMAX_XPC || MMAX_APC
#if	MMAX_APC
	printf("#define\tPSA_SICU_IPND %d\n", &psa->psa_sicu_ipnd);
	printf("#define\tPSA_SICU_IMSK %d\n", &psa->psa_sicu_imsk);
	printf("#define\tPSA_SICU_ISRV %d\n", &psa->psa_sicu_isrv);
#endif	MMAX_APC
#if	MMAX_XPC || MMAX_APC
	printf("#define\tPSA_VBMAX %d\n", PSA_VBMAX);
	printf("#define\tPSA_VBFIFO %d\n", &psa->psa_vbfifo[0]);
#endif	MMAX_XPC || MMAX_APC
	printf("#define\tPSA_CPU_REG_CSR %d\n", &psa->psa_cpu_reg_csr);
	printf("#define\tPSA_CPU_REG_ERR %d\n", &psa->psa_cpu_reg_err);
	printf("#define\tPSA_TIMESTAMP %d\n", &psa->psa_timestamp);
	printf("#define\tTOT_PSASIZ %d\n", NCPUS*sizeof(*psa));
	printf("#define\tFRCOUNTER %d\n", SCCREG_FRCNT);
	printf("#define\tI_FUNC %d\n", &itbl->i_func);
	printf("#define\tI_PARAM %d\n", &itbl->i_param);
	printf("#define\tRB_B_SINGLE %d\n", RB_B_SINGLE);
	printf("#define\tRB_B_INITNAME %d\n", RB_B_INITNAME);
	printf("#define\tRB_B_MULTICPU %d\n", RB_B_MULTICPU);
	printf("#define\tRB_B_NOSYNC %d\n", RB_B_NOSYNC);
	printf("#define\tRB_B_DEBUG %d\n", RB_B_DEBUG);
	printf("#define\tINTSTACK_SIZE %d\n", INTSTACK_SIZE);
	printf("#define\tFPE_FLTUND_FAULT %d\n", FPE_FLTUND_FAULT);
	printf("#define\tFPE_FLTOVF_FAULT %d\n", FPE_FLTOVF_FAULT);
	printf("#define\tFPE_FLTDIV_FAULT %d\n", FPE_FLTDIV_FAULT);
	printf("#define\tFPE_INVLOP_FAULT %d\n", FPE_INVLOP_FAULT);
	printf("#define\tFPE_INTDIV_TRAP %d\n", FPE_INTDIV_TRAP);
	printf("#define\tSCCREG_FRCNT 0x%x\n", SCCREG_FRCNT);
	printf("#define\tNMI_PANIC %d\n", NMI_PANIC);
	printf("#define\tNMI_RESUME %d\n", NMI_RESUME);
	printf("#define\tNMI_WAIT %d\n", NMI_WAIT);
	printf("#define\tNMI_DEBUG %d\n", NMI_DEBUG);
	printf("#define\tNMI_LOCK 0x%x\n", NMI_LOCK);
	printf("#define\tNMI_STATE %d\n", &nmilock->nmi_state);
	printf("#define\tNMI_OWNER %d\n", &nmilock->nmi_owner);
	printf("#define\tNMI_FLAGS %d\n", &nmilock->nmi_flags[0]);
	printf("#define\tPR_BASE %d\n",&prof->pr_base);
	printf("#define\tPR_SIZE %d\n",&prof->pr_size);
	printf("#define\tPR_OFFSET %d\n",&prof->pr_off);
	printf("#define\tPR_SCALE %d\n",&prof->pr_scale);
	printf("#define\tNUMSYSVEC %d\n", NUMSYSVEC);

#if	MMAX_XPC
	printf("#define\tXPCREG_CSR 0x%x\n", XPCREG_CSR);
	printf("#define\tXPCREG_CTL 0x%x\n", XPCREG_CTL);
	printf("#define\tXPCDUART_ADDR 0x%x\n", XPCDUART_ADDR);
	printf("#define\tVB_FIFO 0x%x\n", XPCREG_VBFIFO);
	printf("#define\tXPCDUART_BRCVDATA 0x%x\n", XPCDUART_BRCVDATA);
	printf("#define\tXPCDUART_BSTATUS 0x%x\n", XPCDUART_BSTATUS);
	printf("#define\tXPCDUART_BMODE 0x%x\n", XPCDUART_BMODE);
	printf("#define\tXPCDUART_BCLKSEL 0x%x\n", XPCDUART_BCLKSEL);
	printf("#define\tXPCDUART_BCMD 0x%x\n", XPCDUART_BCMD);
	printf("#define\tXPCVB_STATUS 0x%x\n", XPCVB_STATUS);
	printf("#define\tXPCREG_ERR 0x%x\n", XPCREG_ERR);
	printf("#define\tXPCREG_LED 0x%x\n", XPCREG_LED);
#endif	MMAX_XPC
#if	MMAX_APC
	printf("#define\tAPCREG_CSR 0x%x\n", APCREG_CSR);
	printf("#define\tAPCREG_ERR 0x%x\n", APCREG_ERR);
	printf("#define\tAPCREG_CTL 0x%x\n", APCREG_CTL);
	printf("#define\tAPCDUART_ADDR 0x%x\n", APCDUART_ADDR);
	printf("#define\tVB_FIFO 0x%x\n", APCREG_VBFIFO);
#endif	MMAX_APC
	printf("#endif\n");
	exit(0);
}
