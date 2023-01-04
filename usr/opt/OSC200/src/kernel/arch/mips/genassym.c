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
static char	*sccsid = "@(#)$RCSfile: genassym.c,v $ $Revision: 1.2.4.4 $ (DEC) $Date: 1992/06/03 10:19:31 $";
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

#include <confdep.h>
#include <mach_emulation.h>

/*
 * Copyright (c) 1982, 1986 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 *	@(#)genassym.c	7.1 (Berkeley) 6/5/86
 */
/*
 * genassym.c -- generate struct offset constants for assembler files
 *
 * NOTE: genassym should only be used for struct offsets, or other constants
 * that cannot be easily #defined.  Constants which appear as #defines and are
 * needed by assembler files should be obtained by including the appropriate
 * .h file (adding #ifndef LOCORE's as necessary).
 */

#if	defined(mips) && !defined(__mips__)
#define	__mips__
#endif
#if	defined(MIPSEL) && !defined(__MIPSEL__)
#define	__MIPSEL__
#endif
#if	defined(MIPSEB) && !defined(__MIPSEB__)
#define	__MIPSEB__
#endif

#include <sys/proc.h>

#include <mach/mips/vm_param.h>
#include <machine/pmap.h>

#if	MACH_EMULATION
#include <kern/syscall_emulation.h>
#endif

#ifdef PROFILING
#include <sys/gprof.h>
#endif

main()
{
	register struct proc *p = (struct proc *)0;
	register struct utask *utask = (struct utask *)0;
	register struct uthread *uthread = (struct uthread *)0;
	register struct rusage *rup = (struct rusage *)0;
	struct thread *thread = (struct thread *) 0;
	struct task *task = (struct task *) 0;
	struct vm_map *vm_map = (struct vm_map *) 0;
	struct pmap *pmap = (struct pmap *) 0;
	struct pcb *pcb = (struct pcb *)0;
	struct uuprof *uprof = (struct uuprof *)0;
#if	MACH_EMULATION
	struct eml_dispatch *disp = (struct eml_dispatch *)0;
#endif

#if	defined(PROFILING) && (PROFTYPE == 4)
	struct tostruct *tos = (struct tostruct *)0;
#endif

	printf("#ifdef ASSEMBLER\n");
	printf("#define\tPCB_WIRED_ADDRESS 0x%X\n", PCB_WIRED_ADDRESS);
	printf("#define\t KERNEL_STACK_START_OFFSET 0x%X\n",
					 KERNEL_STACK_START_OFFSET);
	printf("#define\tP_CURSIG %d\n", &p->p_cursig);
	printf("#define\tP_SIG %d\n", &p->p_sig);
	printf("#define\tP_FLAG %d\n", &p->p_flag);
	printf("#define\tNBPW %d\n", NBPW);

	printf("#define\tU_PROCP %d\n", &utask->uu_procp);
	printf("#define\tU_RU %d\n", &utask->uu_ru);

	printf("#define\tRU_MINFLT %d\n", &rup->ru_minflt);

	printf("#define\tPCB_KSTACK %d\n", &pcb->pcb_kstack);
	printf("#define\tPCB_MIPS_USER_FAULT %d\n",&pcb->pcb_mips_user_fault); 	
	printf("#define\tPCB_CPU_NUMBER %d\n", &pcb->pcb_cpu_number);
	printf("#define\tPCB_REGS %d\n", pcb->pcb_regs);
	printf("#define\tPCB_OWNEDFP %d\n", &pcb->pcb_ownedfp);
	printf("#define\tPCB_NOFAULT %d\n", &pcb->pcb_nofault);

	printf("#define\tPCB_FPREGS %d\n", pcb->pcb_fpregs);
	printf("#define\tPCB_FPC_CSR %d\n", &pcb->pcb_fpc_csr);
	printf("#define\tPCB_FPC_EIR %d\n", &pcb->pcb_fpc_eir);

	printf("#define\tPCB_C2REGS %d\n", pcb->pcb_c2regs);
	printf("#define\tPCB_C3REGS %d\n", pcb->pcb_c3regs);

	printf("#define\tPCB_BD_EPC %d\n", &pcb->pcb_bd_epc);
	printf("#define\tPCB_BD_CAUSE %d\n", &pcb->pcb_bd_cause);
	printf("#define\tPCB_BD_RA %d\n", &pcb->pcb_bd_ra);
	printf("#define\tPCB_BD_INSTR %d\n", &pcb->pcb_bd_instr);

	printf("#define\tPCB_SOFTFP_PC %d\n", &pcb->pcb_softfp_pc);


	printf("#define\tU_AR0 %d\n", &uthread->uu_ar0);

	printf("#define\tTHREAD_PCB %d\n", &thread->pcb);
	printf("#define\tTHREAD_RECOVER %d\n", &thread->recover);
	printf("#define\tTHREAD_TASK %d\n", &thread->task);
	printf("#define\tTHREAD_KERNEL_STACK %d\n", &thread->kernel_stack);
	printf("#define\tTHREAD_STATE %d\n", &thread->state);
	printf("#define\tTH_SWAPPED %d\n", TH_SWAPPED);
	printf("#define\tUTHREAD %d\n", &thread->u_address.uthread);
	printf("#define\tUTASK %d\n", &thread->u_address.utask);

	printf("#define\tPR_BASE %d\n", &uprof->pr_base);
	printf("#define\tPR_SIZE %d\n", &uprof->pr_size);
	printf("#define\tPR_OFF %d\n", &uprof->pr_off);
	printf("#define\tPR_SCALE %d\n", &uprof->pr_scale);

	printf("#define\tTASK_MAP %d\n", &task->map);
	printf("#define\tMAP_PMAP %d\n", &vm_map->vm_pmap);
	printf("#define\tPMAP_PID %d\n", &pmap->pid);
	printf("#define\tPMAP_PTEBASE %d\n", &pmap->ptebase);
	printf("#define\tPMAP_PCACHE_DATA %d\n", pmap->pcache.data);

#if	MACH_EMULATION
	printf("#define\tEML_DISPATCH 0x%x\n", &task->eml_dispatch);
	printf("#define\tDISP_COUNT 0x%x\n", &disp->disp_count);
	printf("#define\tDISP_VECTOR 0x%x\n", &disp->disp_vector[0]);
#endif

#if	defined(PROFILING) && (PROFTYPE == 4)
	printf("#define\tTOS_LINK %d\n", &tos->link);
	printf("#define\tTOS_COUNT %d\n", &tos->count);
	printf("#define\tTOS_SELFPC %d\n", &tos->selfpc);
	printf("#define\tTOS_SIZE %d\n", sizeof(*tos));
        printf("#define\tHASHFRACTION %d\n", HASHFRACTION);
#endif

	printf("#endif /* ASSEMBLER */\n");
	exit (0);
}
