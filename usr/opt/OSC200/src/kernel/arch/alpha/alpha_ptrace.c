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
static char *rcsid = "@(#)$RCSfile: alpha_ptrace.c,v $ $Revision: 1.1.2.9 $ (DEC) $Date: 1992/10/20 13:55:29 $";
#endif

#include <sys/user.h>
#include <sys/proc.h>
#include <sys/ptrace.h>
#include <kern/thread.h>
#include <machine/reg.h>
#include <mach/alpha/thread_status.h>

#define	R31	31
/*
 * R31 is a special case on alpha.  Reads are always 0, writes are undefined.
 *
 * The register number is the index into the ipcreg array which then maps
 * the register to the proper location in the pcb.
 */

int	ipcreg[] = {
/* 00 */	EF_V0, EF_T0, EF_T1, EF_T2, EF_T3, EF_T4, EF_T5, EF_T6,
/* 08 */	EF_T7, EF_S0, EF_S1, EF_S2, EF_S3, EF_S4, EF_S5, EF_S6,
/* 16 */	EF_A0, EF_A1, EF_A2, EF_A3, EF_A4, EF_A5, EF_T8, EF_T9,
/* 24 */	EF_T10, EF_T11, EF_RA, EF_T12, EF_AT, EF_GP, EF_SP, R31
};

get_ptrace_u(displ, dest, thread)
int displ;
ulong_t *dest;
thread_t thread;
{
	int offset;
	ulong_t *locr0;

	if (displ < GPR_BASE)
		return EINVAL;

	if ((displ < (GPR_BASE+NGP_REGS)) || (displ == PC)) {	/* GPRs  of PC*/
		if (displ == R31) { /* R31 always reads 0 */
			*dest = 0;
		}
		else if (ipcreg[displ] == EF_SP) {
			*dest = thread->pcb->pcb_usp;	
		} else {
			locr0 = (ulong_t *)USER_REGS(thread);
			if (displ == PC)
				*dest = locr0[EF_PC];
			else
				*dest = locr0[ipcreg[displ]];
		}
	}
	else if ((displ >= FPR_BASE) && (displ < FPR_BASE+NFP_REGS)) {
		struct pcb *pcb = thread->pcb;
		locr0 = (ulong_t *)pcb->pcb_fpregs;
		*dest = locr0[displ-FPR_BASE];
	}
	else
		return EINVAL;

	return 0;
}	

set_ptrace_u(displ, val, thread, retval)
int displ;
ulong_t val;
thread_t thread;
ulong_t *retval;
{
	ulong_t *locr0;

	if (displ < GPR_BASE)
		return EINVAL;

	if ((displ < (GPR_BASE+NGP_REGS)) || (displ == PC)) {	/* GPRs  of PC*/
		if (displ == R31) { /* R31 writes are undefined */
			/* do nothing */
		}
		else if (ipcreg[displ] == EF_SP) {
			thread->pcb->pcb_usp = val;	
		}
		else {
			locr0 = (ulong_t *)USER_REGS(thread);
			if (displ == PC)
				locr0[EF_PC] = val;
			else
				locr0[ipcreg[displ]] = val;
		}
	}
	else if ((displ >= FPR_BASE) && (displ < FPR_BASE+NFP_REGS)) {
		struct pcb *pcb = thread->pcb;
		locr0 = (ulong_t *)pcb->pcb_fpregs;
		locr0[displ-FPR_BASE] = val;
	}
	else
		return EINVAL;
	
	*retval = 0;

	return 0;
}
