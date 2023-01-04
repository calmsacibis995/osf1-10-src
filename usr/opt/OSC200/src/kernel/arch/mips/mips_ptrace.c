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
static char	*sccsid = "@(#)$RCSfile: mips_ptrace.c,v $ $Revision: 1.2.4.2 $ (DEC) $Date: 1992/04/16 14:22:28 $";
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
 * derived from mips_ptrace.c	2.1	(ULTRIX/OSF)	12/3/90";
 */

#include <sys/user.h>
#include <sys/ptrace.h>

#include <machine/reg.h>
#include <mach/mips/thread_status.h>

#define ALIGNED(addr,size)	(((unsigned)(addr)&((size)-1))==0)


/*
 *	Read user register and u area values
 */
get_ptrace_u(displ, dest, thread)
int displ;
int *dest;
thread_t thread;
{
    register unsigned	regno;
    /* 
     *	See mips_ptrace.h for details.  A user might ask for
     *	some GP or float register, or signal handler.
     */
    if ((regno = displ - GPR_BASE) < NGP_REGS)
	*dest = (regno == 0) ? 0 :
	    ((unsigned int*)USER_REGS(thread))[EF_AT + regno - 1];

    else if ((regno = displ - FPR_BASE) < NFP_REGS) {
	checkfp (thread, 0);
	*dest = thread->pcb->pcb_fpregs[regno];
    }

    else if ((regno = displ - SIG_BASE) < NSIG_HNDLRS) {
	if (thread_signal_disposition(regno))
		*dest = (int) thread->u_address.uthread->uu_tsignal[regno];
	else
		*dest = (int) thread->u_address.utask->uu_signal[regno];

    } else if ((regno = displ - SPEC_BASE) < NSPEC_REGS) {
	switch (displ) {
	case PC:
		*dest = ((unsigned int*)USER_REGS(thread))[EF_EPC];
		break;
	case CAUSE:
		*dest = ((unsigned int*)USER_REGS(thread))[EF_CAUSE];
		break;
	case MMHI:
		*dest = ((unsigned int*)USER_REGS(thread))[EF_MDHI];
		break;
	case MMLO:
		*dest = ((unsigned int*)USER_REGS(thread))[EF_MDLO];
		break;
	case FPC_CSR: checkfp (thread, 0);
		*dest = thread->pcb->pcb_fpc_csr;
		break;
	case FPC_EIR: checkfp (thread, 0);
		*dest = thread->pcb->pcb_fpc_eir;
		break;
	case TRAPCAUSE:
		*dest = thread->pcb->trapcause;
		break;
	case TRAPINFO:
		*dest = 0;
		break;
	default: return EINVAL;
	}
    }
    else
	    return EINVAL;

    return 0;
}

set_ptrace_u(displ, val, thread, retval)
	int displ;
	unsigned val;
	thread_t thread;
	int *retval;
{
    register unsigned   *addr;
    register unsigned    regno;

    if ((regno = displ - GPR_BASE) < NGP_REGS) {
	if (regno == 0)
	    return EINVAL;
	addr = & ((unsigned int*)USER_REGS(thread))[EF_AT + regno - 1];
    }

    else if ((regno = displ - FPR_BASE) < NFP_REGS) {
	checkfp (thread, 0);
	addr = (unsigned*) &(thread->pcb->pcb_fpregs[regno]);
    }

    else if ((regno = displ - SIG_BASE) < NSIG_HNDLRS) {
	if (thread_signal_disposition(regno))
		addr = (unsigned*) &(thread->u_address.uthread->uu_tsignal[regno]);
	else
		addr = (unsigned*) &(thread->u_address.utask->uu_signal[regno]);

    } else if ((regno = displ - SPEC_BASE) < NSPEC_REGS) {

	if (displ == PC && !ALIGNED(val, sizeof(int)))
	    return EINVAL;

	switch (displ) {
	case PC:
		addr = & ((unsigned int*)USER_REGS(thread))[EF_EPC];
		break;
	case MMHI:
		addr = & ((unsigned int*)USER_REGS(thread))[EF_MDHI];
		break;
	case MMLO:
		addr = & ((unsigned int*)USER_REGS(thread))[EF_MDLO];
		break;
	case FPC_CSR:
		checkfp (thread, 0);
		addr = (unsigned*) &(thread->pcb->pcb_fpc_csr);
		break;
	case CAUSE:		/* these are read-only */
	case FPC_EIR:
	case TRAPCAUSE:
	case TRAPINFO:
	default:
		 return EACCES;
	}
    }
    else
	return EINVAL;

    *retval = *addr;
    *addr = val;
    return 0;
}
