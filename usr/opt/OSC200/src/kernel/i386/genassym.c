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
static char	*sccsid = "@(#)$RCSfile: genassym.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:18:04 $";
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
 * All rights reserved.  The CMU software License Agreement specifies
 * the terms and conditions for use and redistribution.
 */
/* 
 * OSF/1 Release 1.0
 */

#include <confdep.h>

/*
 * Copyright (c) 1982, 1986 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *

 */

#include <sys/param.h>
#include <sys/buf.h>
#include <sys/vmparam.h>
#include <ufs/dir.h>
#include <sys/user.h>
#include <sys/map.h>
#include <sys/proc.h>
#include <sys/msgbuf.h>

#include <kern/thread.h>
#include <kern/task.h>
#include <machine/pcb.h>

#include <mach/i386/vm_param.h>
#include <stack_limit_check.h>

#if	MACH_EMULATION
#include <kern/syscall_emulation.h>
#endif	MACH_EMULATION

thread_t	active_threads[1];

main()
{
	register struct proc *p = (struct proc *)0;
	register struct utask *utask = (struct utask *)0;
	register struct uthread *uthread = (struct uthread *)0;
	register struct pcb *pcb = (struct pcb *)0;
	register struct rusage *rup = (struct rusage *)0;
	struct rpb *rp = (struct rpb *)0;
	struct thread *thread = (struct thread *) 0;
	struct task *task = (struct task *) 0;
	struct uuprof *uprof = (struct uuprof *)0;
#if	MACH_EMULATION
	struct eml_dispatch *disp = (struct eml_dispatch *)0;
#endif	MACH_EMULATION

	printf("#ifdef ASSEMBLER\n");
	printf("#define\tP_LINK %d\n", &p->p_link);
	printf("#define\tP_RLINK %d\n", &p->p_rlink);
	printf("#define\tP_PRI %d\n", &p->p_pri);
	printf("#define\tP_STAT %d\n", &p->p_stat);
	printf("#define\tP_CURSIG %d\n", &p->p_cursig);
	printf("#define\tP_SIG %d\n", &p->p_sig);
	printf("#define\tP_FLAG %d\n", &p->p_flag);
	printf("#define\tSSLEEP %d\n", SSLEEP);
	printf("#define\tSRUN %d\n", SRUN);
	printf("#define\tUPAGES %d\n", UPAGES);
	printf("#define\tU_PROCP %d\n", &utask->uu_procp);
	printf("#define\tU_RU %d\n", &utask->uu_ru);
	printf("#define\tRU_MINFLT %d\n", &rup->ru_minflt);
	printf("#define\tPR_BASE %d\n", &uprof->pr_base);
	printf("#define\tPR_SIZE %d\n", &uprof->pr_size);
	printf("#define\tPR_OFF %d\n", &uprof->pr_off);
	printf("#define\tPR_SCALE %d\n", &uprof->pr_scale);
	printf("#define\tU_AR0 %d\n", &uthread->uu_ar0);
	printf("#define\tpcb_fpvalid %d\n", &pcb->pcb_fpvalid);
	printf("#define\tpcb_fps %d\n", &pcb->pcb_fps);
	printf("#define\tUSER_FP 0x%x\n", USER_FP);
	printf("#define\tKDSSEL 0x%x\n", KDSSEL);
	printf("#define\tKTSSSEL 0x%x\n", KTSSSEL);
	printf("#define\tJTSSSEL 0x%x\n", JTSSSEL);
	printf("#define\tKSSSEL 0x%x\n", KSSSEL);
	printf("#define\tTHREAD_PCB %d\n", &thread->pcb);
	printf("#define\tTHREAD_RECOVER %d\n", &thread->recover);
	printf("#define\tTHREAD_TASK %d\n", &thread->task);
	printf("#define\tTHREAD_STACK %d\n", &thread->kernel_stack);
	printf("#define\tUTHREAD %d\n", &thread->u_address.uthread);
	printf("#define\tUTASK %d\n", &thread->u_address.utask);
#if	MACH_EMULATION
	printf("#define\tEML_DISPATCH 0x%x\n", &task->eml_dispatch);
	printf("#define\tDISP_COUNT 0x%x\n", &disp->disp_count);
	printf("#define\tDISP_VECTOR 0x%x\n", &disp->disp_vector[0]);
#endif	MACH_EMULATION
        printf("#define\tSTACK_LIMIT_CHECK %d\n", STACK_LIMIT_CHECK);
	printf("#endif\n");
	exit(0);
}
