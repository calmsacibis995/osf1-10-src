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
static char *rcsid = "@(#)$RCSfile: genassym.c,v $ $Revision: 1.2.11.2 $ (DEC) $Date: 1993/04/01 19:56:33 $";
#endif
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
 * Modification History: machine/genassym.c
 *
 * 08-May-91	afd
 *      Created this file for processor support of the Alpha ADU.
 *	Started from ../mips/genasym.c (Tin BL 3) and Alpha/Ultrix-64 version.
 *
 */


#include <confdep.h>
#include <mach_emulation.h>

#include <sys/proc.h>

#include <machine/pmap.h>
#include <machine/rpb.h>

#if	MACH_EMULATION
#include <kern/syscall_emulation.h>
#endif

#if PROFILING
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
	struct rpb_percpu *percpu = (struct rpb_percpu *)0;
#if	MACH_EMULATION
	struct eml_dispatch *disp = (struct eml_dispatch *)0;
#endif

	printf("#ifdef ASSEMBLER\n");
	printf("#define\tP_CURSIG %d\n", &p->p_cursig);
	printf("#define\tP_SIG %d\n", &p->p_sig);
	printf("#define\tP_FLAG %d\n", &p->p_flag);
	printf("#define\tP_UAC %d\n", &p->p_uac);
	printf("#define\tNBPW %d\n", NBPW);

	printf("#define\tU_PROCP %d\n", &utask->uu_procp);
	printf("#define\tU_RU %d\n", &utask->uu_ru);

	printf("#define\tRU_MINFLT %d\n", &rup->ru_minflt);
        printf("#define\tPCB_KSTACK %d\n", &pcb->pcb_kstack);
        printf("#define\tPCB_CPU_NUMBER %d\n", &pcb->pcb_cpu_number);
	printf("#define\tPCB_REGS %d\n", pcb->pcb_regs);
	printf("#define\tPCB_SSWAP %d\n", &pcb->pcb_sswap);
	printf("#define\tPCB_RESCHED %d\n", &pcb->pcb_resched);
	printf("#define\tPCB_OWNEDFP %d\n", &pcb->pcb_ownedfp);
	printf("#define\tPCB_PADDR %d\n", &pcb->pcb_paddr);

	printf("#define\tPCB_FEN %d\n", &pcb->pcb_fen);
	printf("#define\tPCB_NOFAULT %d\n", &pcb->pcb_nofault);

	printf("#define\tPCB_FPREGS %d\n", pcb->pcb_fpregs);
	printf("#define\tPCB_FPCR %d\n", &pcb->pcb_fpcr);
	printf("#define\tPCB_CPUPTR %d\n", &pcb->pcb_cpuptr);

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

	printf("#define\tRPB_STATE %d\n", &percpu->rpb_state);
	printf("#define\tRPB_HALTPB %d\n", &percpu->rpb_haltpb);
	printf("#define\tRPB_HALTPC %d\n", &percpu->rpb_haltpc);
	printf("#define\tRPB_HALTPS %d\n", &percpu->rpb_haltps);
	printf("#define\tRPB_HALTAL %d\n", &percpu->rpb_haltal);
	printf("#define\tRPB_HALTRA %d\n", &percpu->rpb_haltra);
	printf("#define\tRPB_HALTPV %d\n", &percpu->rpb_haltpv);

	printf("#define\tTASK_MAP %d\n", &task->map);
	printf("#define\tMAP_PMAP %d\n", &vm_map->vm_pmap);

#if	MACH_EMULATION
	printf("#define\tEML_DISPATCH 0x%x\n", &task->eml_dispatch);
	printf("#define\tDISP_COUNT 0x%x\n", &disp->disp_count);
	printf("#define\tDISP_VECTOR 0x%x\n", &disp->disp_vector[0]);
#endif

#if	PROFILING && PROFTYPE == 4
	{   struct tostruct *tos = (struct tostruct *)0;
	    printf("#define\tTOS_LINK %d\n", &tos->link);
	    printf("#define\tTOS_COUNT %d\n", &tos->count);
	    printf("#define\tTOS_SELFPC %d\n", &tos->selfpc);
	    printf("#define\tTOS_SIZE %d\n", sizeof(*tos));
	    printf("#define\tHASHFRACTION %d\n", HASHFRACTION);
	}
#endif /* PROFILING && PROFTYPE == 4 */

	printf("#endif /* ASSEMBLER */\n");
	exit (0);
}
