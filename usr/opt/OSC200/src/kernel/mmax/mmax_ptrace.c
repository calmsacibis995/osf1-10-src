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
static char	*sccsid = "@(#)$RCSfile: mmax_ptrace.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:42:05 $";
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
 *	 Copyright (C) 1985,1986 Hydra Computer Systems, Inc.
 *
 * ALL RIGHTS RESERVED. Licensed Material - Property of Hydra Computer
 * Systems, Inc. This software is made available solely pursuant to the
 * terms of a software license agreement which governs its use.
 * Unauthorized duplication, distribution or sale are strictly 
 * prohibited.
 *
 * Module Function:
 * 	This module contains the functions for performing process tracing
 *	for debuggers.  Included are:
 *
 *	mmax_get_ptrace_u:	Get a value form the Ptrace U-area.
 *	mmax_set_ptrace_u:	Set a value in the Ptrace U-area.
 *	mmax_fake_ptrace_u:	Create a fake Ptrace U-area for core dump.
 *		
 *
 * Original Author: Tony Anzelmo	Created on: 85/01/12
 */



/*
 *****************************************************************************
 *                                                                           *
 *		Include Files and External Definitions			     *
 *									     *
 *****************************************************************************
 */
#include <sys/param.h>
#include <sys/proc.h>
#include <sys/user.h>
#include <sys/ptrace.h>
#include <kern/thread.h>
#include <mmax/pcb.h>
#include <mmax/psl.h>
#include <mmax/reg.h>
#include <mmax/vmparam.h>

/*****************************************************************************
 *
 * NAME:
 *	mmax_get_ptrace_u -- Get a value from the Ptrace U-area.
 *
 * DESCRIPTION:
 *	Get the value at the specified offset in the ptrace U-area.
 *
 * ARGUMENTS:
 *	offset -- The offset into the ptrace U-area.
 *	value  -- The returned value at offset in ptrace U-area (by
 *		  reference).
 *	thread -- Specifies which thread to get the value from.
 *
 * RETURN VALUE:
 *	0: The call succeeded.
 *	-1:   The offset was invalid.
 *
 * SIDE EFFECTS:
 *	None.
 *
 * EXCEPTIONS:
 *	None.
 *
 * ASSUMPTIONS:
 *	None.
 */

mmax_get_ptrace_u (offset, value, thread)
register int	offset;
register int	*value;
register thread_t	thread;
{
/*
 * Switch on the offset.  The cases are offsets from mmax_ptrace.h.
 */
	switch (offset) {

	case PU_PSL:		/* Program status longword (psr + mod).	    */
		*value = thread->u_address.uthread->uu_ar0[PS];
		break;

	case PU_PC:		/* Program counter.			    */
		*value = thread->u_address.uthread->uu_ar0[PC];
		break;

	case PU_FP:		/* Frame pointer.			    */
		*value = thread->u_address.uthread->uu_ar0[FP];
		break;

	case PU_SP:		/* Stack pointer.			    */
		*value = thread->u_address.uthread->uu_ar0[SP];
		break;

	case PU_R0:		/* Register 0.				    */
		*value = thread->u_address.uthread->uu_ar0[R0];
		break;

	case PU_R1:		/* Register 1.				    */
		*value = thread->u_address.uthread->uu_ar0[R1];
		break;

	case PU_R2:		/* Register 2.				    */
		*value = thread->u_address.uthread->uu_ar0[R2];
		break;

	case PU_R3:		/* Register 3.				    */
		*value = thread->u_address.uthread->uu_ar0[R3];
		break;

	case PU_R4:		/* Register 4.				    */
		*value = thread->u_address.uthread->uu_ar0[R4];
		break;

	case PU_R5:		/* Register 5.				    */
		*value = thread->u_address.uthread->uu_ar0[R5];
		break;

	case PU_R6:		/* Register 6.				    */
		*value = thread->u_address.uthread->uu_ar0[R6];
		break;

	case PU_R7:		/* Register 7.				    */
		*value = thread->u_address.uthread->uu_ar0[R7];
		break;

	case PU_FSR:		/* Floating point status register.	    */
		*value = thread->pcb->pcb_fsr;
		break;

	case PU_F0:		*value = thread->pcb->pcb_f0.val[0]; break;
	case PU_F1:		*value = thread->pcb->pcb_f0.val[1]; break;
	case PU_F2:		*value = thread->pcb->pcb_f2.val[0]; break;
	case PU_F3:		*value = thread->pcb->pcb_f2.val[1]; break;
	case PU_F4:		*value = thread->pcb->pcb_f4.val[0]; break;
	case PU_F5:		*value = thread->pcb->pcb_f4.val[1]; break;
	case PU_F6:		*value = thread->pcb->pcb_f6.val[0]; break;
	case PU_F7:		*value = thread->pcb->pcb_f6.val[1]; break;

	case PU_D0:		*value = thread->pcb->pcb_f0.val[0]; break;
	case PU_D0+sizeof(int):	*value = thread->pcb->pcb_f0.val[1]; break;
	case PU_D2:		*value = thread->pcb->pcb_f2.val[0]; break;
	case PU_D2+sizeof(int):	*value = thread->pcb->pcb_f2.val[1]; break;
	case PU_D4:		*value = thread->pcb->pcb_f4.val[0]; break;
	case PU_D4+sizeof(int):	*value = thread->pcb->pcb_f4.val[1]; break;
	case PU_D6:		*value = thread->pcb->pcb_f6.val[0]; break;
	case PU_D6+sizeof(int):	*value = thread->pcb->pcb_f6.val[1]; break;

#if	MMAX_XPC
	case PU_D1:		*value = thread->pcb->pcb_f1.val[0]; break;
	case PU_D1+sizeof(int):	*value = thread->pcb->pcb_f1.val[1]; break;
	case PU_D3:		*value = thread->pcb->pcb_f3.val[0]; break;
	case PU_D3+sizeof(int):	*value = thread->pcb->pcb_f3.val[1]; break;
	case PU_D5:		*value = thread->pcb->pcb_f5.val[0]; break;
	case PU_D5+sizeof(int):	*value = thread->pcb->pcb_f5.val[1]; break;
	case PU_D7:		*value = thread->pcb->pcb_f7.val[0]; break;
	case PU_D7+sizeof(int):	*value = thread->pcb->pcb_f7.val[1]; break;
#endif	MMAX_XPC
#if	MMAX_APC || MMAX_DPC
	/* these registers aren't available on this chip, return zero */
	case PU_D1:
	case PU_D1+sizeof(int):
	case PU_D3:
	case PU_D3+sizeof(int):
	case PU_D5:
	case PU_D5+sizeof(int):
	case PU_D7:
	case PU_D7+sizeof(int):
		*value = 0;
		break;
#endif	MMAX_APC || MMAX_DPC

#if	0
/*
 * arguments to the current system call are not available except in
 * the context of the thread itself.
 */
	case PU_ARG0:		/* Arguments to the current system call.    */
		*value = thread->u_address.uthread->uu_ap[0];
		break;

	case PU_ARG1:
		*value = thread->u_address.uthread->uu_ap[1];
		break;

	case PU_ARG2:
		*value = thread->u_address.uthread->uu_ap[2];
		break;

	case PU_ARG3:
		*value = thread->u_address.uthread->uu_ap[3];
		break;

	case PU_ARG4:
		*value = thread->u_address.uthread->uu_ap[4];
		break;

	case PU_ARG5:
		*value = thread->u_address.uthread->uu_ap[5];
		break;

	case PU_ARG6:
		*value = thread->u_address.uthread->uu_ap[6];
		break;

	case PU_ARG7:
		*value = thread->u_address.uthread->uu_ap[7];
		break;

	case PU_ARG8:
		*value = thread->u_address.uthread->uu_ap[8];
		break;

	case PU_ARG9:
		*value = thread->u_address.uthread->uu_ap[9];
		break;
#endif	/* 0 */

	case PU_DSIZE:		/* Data size.				    */
		*value = ctob(thread->u_address.utask->uu_dsize);
		break;

	case PU_SSIZE:		/* Stack size.				    */
		*value = ctob(thread->u_address.utask->uu_ssize);
		break;

	case PU_DATAPTR:	/* Offset of data after ptrace_user	*/
		*value = sizeof(struct ptrace_user);
		break;

	case PU_AH_MAGIC:
		/*
		 *	Magic number; always return 0413 as this is usually
		 *	correct.
		 */
		*value = 0413;
		break;

	case PU_AH_TSIZE:	/* Text size */
		*value = ctob(thread->u_address.utask->uu_tsize);
		break;

	case PU_AH_ENTRY:
		/*
		 *	Entry point -- always return 2 because this
		 *	is where the standard crt0 starts.
		 */
		*value = 2;
		break;

	case PU_AH_TSTART:	/* Text start */
		*value = (int)thread->u_address.utask->uu_text_start;
		break;

	case PU_AH_DSTART:	/* Data start.   */
		*value = (int)thread->u_address.utask->uu_data_start;
		break;

	case PU_SIGNAL:		/* signal that stopped me */
		*value = thread->u_address.utask->uu_procp->p_cursig;
		break;

	case PU_USRSTACK:	/* base of the user stack */
		*value = USRSTACK;
		break;

	case PU_NMEM:		/* number of shared segs: UNUSED */
		*value = 0;
		break;

	default:		/* Either the command or illegal.	    */
		if ((offset >= PU_COMM) && (offset < PU_COMM + MAXCOMLEN)) {
			offset -= PU_COMM;
			*value =
			    (int) thread->u_address.utask->uu_comm[offset];
			break;
		}
		else {
			return (-1);	/* ERROR */
		}
	}

	return (0);
}

/*****************************************************************************
 *
 * NAME:
 *	mmax_set_ptrace_u -- Set the offset to the value.
 *
 * DESCRIPTION:
 *	Case into the ptrace U structure and set the appropriate location in
 *	the U area.
 *
 * ARGUMENTS:
 *	offset -- The offset into the ptrace U-area.
 *	value  -- The value to set the offset into the U-area.
 *	thread -- Thread for which to change the value.
 *
 * RETURN VALUE:
 *	0:  If the offset was set.
 *	-1:   If the offset was illegal.
 *
 * SIDE EFFECTS:
 *	None.
 *
 * EXCEPTIONS:
 *	None.
 *
 * ASSUMPTIONS:
 *	None.
 */


mmax_set_ptrace_u (offset, value, thread)
register int	offset;
register int	value;
register thread_t	thread;
{
/*
 * Switch on the offset.  The cases are offsets from mmax_ptrace.h.
 */
	switch (offset) {

	case PU_PSL:		/* Program status longword (psr + mod).	    */
		/* Firewalls from sys_process.c */	
		value |= PSL_USERSET; 
		value &= PSL_USERCLR;

		thread->u_address.uthread->uu_ar0[PS] = value;
		break;

	case PU_PC:		/* Program counter.			    */
		thread->u_address.uthread->uu_ar0[PC] = value;
		break;

	case PU_FP:		/* Frame pointer.			    */
		thread->u_address.uthread->uu_ar0[FP] = value;
		break;

	case PU_SP:		/* Stack pointer.			    */
		thread->u_address.uthread->uu_ar0[SP] = value;
		break;

	case PU_R0:		/* Register 0.				    */
		thread->u_address.uthread->uu_ar0[R0] = value;
		break;

	case PU_R1:		/* Register 1.				    */
		thread->u_address.uthread->uu_ar0[R1] = value;
		break;

	case PU_R2:		/* Register 2.				    */
		thread->u_address.uthread->uu_ar0[R2] = value;
		break;

	case PU_R3:		/* Register 3.				    */
		thread->u_address.uthread->uu_ar0[R3] = value;
		break;

	case PU_R4:		/* Register 4.				    */
		thread->u_address.uthread->uu_ar0[R4] = value;
		break;

	case PU_R5:		/* Register 5.				    */
		thread->u_address.uthread->uu_ar0[R5] = value;
		break;

	case PU_R6:		/* Register 6.				    */
		thread->u_address.uthread->uu_ar0[R6] = value;
		break;

	case PU_R7:		/* Register 7.				    */
		thread->u_address.uthread->uu_ar0[R7] = value;
		break;

	case PU_FSR:		/* Floating point status register.	    */
		thread->pcb->pcb_fsr = value;
		break;

	case PU_F0:		thread->pcb->pcb_f0.val[0] = value; break;
	case PU_F1:		thread->pcb->pcb_f0.val[1] = value; break;
	case PU_F2:		thread->pcb->pcb_f2.val[0] = value; break;
	case PU_F3:		thread->pcb->pcb_f2.val[1] = value; break;
	case PU_F4:		thread->pcb->pcb_f4.val[0] = value; break;
	case PU_F5:		thread->pcb->pcb_f4.val[1] = value; break;
	case PU_F6:		thread->pcb->pcb_f6.val[0] = value; break;
	case PU_F7:		thread->pcb->pcb_f6.val[1] = value; break;

	case PU_D0:		thread->pcb->pcb_f0.val[0] = value; break;
	case PU_D0+sizeof(int):	thread->pcb->pcb_f0.val[1] = value; break;
	case PU_D2:		thread->pcb->pcb_f2.val[0] = value; break;
	case PU_D2+sizeof(int):	thread->pcb->pcb_f2.val[1] = value; break;
	case PU_D4:		thread->pcb->pcb_f4.val[0] = value; break;
	case PU_D4+sizeof(int):	thread->pcb->pcb_f4.val[1] = value; break;
	case PU_D6:		thread->pcb->pcb_f6.val[0] = value; break;
	case PU_D6+sizeof(int):	thread->pcb->pcb_f6.val[1] = value; break;

#if	MMAX_XPC
	case PU_D1:		thread->pcb->pcb_f1.val[0] = value; break;
	case PU_D1+sizeof(int):	thread->pcb->pcb_f1.val[1] = value; break;
	case PU_D3:		thread->pcb->pcb_f3.val[0] = value; break;
	case PU_D3+sizeof(int):	thread->pcb->pcb_f3.val[1] = value; break;
	case PU_D5:		thread->pcb->pcb_f5.val[0] = value; break;
	case PU_D5+sizeof(int):	thread->pcb->pcb_f5.val[1] = value; break;
	case PU_D7:		thread->pcb->pcb_f7.val[0] = value; break;
	case PU_D7+sizeof(int):	thread->pcb->pcb_f7.val[1] = value; break;
#endif	MMAX_XPC
#if	MMAX_APC || MMAX_DPC
	/* these registers don't exist, don't bother */
	case PU_D1:
	case PU_D1+sizeof(int):
	case PU_D3:
	case PU_D3+sizeof(int):
	case PU_D5:
	case PU_D5+sizeof(int):
	case PU_D7:
	case PU_D7+sizeof(int):
		break;
#endif	MMAX_APC || MMAX_DPC

	default:		/* Illegal offset.			    */
		return (-1);	/* ERROR */
	}

	return (0);
}

/*
 *	mmax_fake_ptrace_u creates a ptrace_user structure for the
 *	specified thread.
 */

mmax_fake_ptrace_u (ptrace_up, thread)
register struct ptrace_user	*ptrace_up;
register thread_t		thread;
{
	register struct uthread		*uthreadp;
	register struct utask		*utaskp;
	register struct pcb		*pcb;

	uthreadp = thread->u_address.uthread;
	utaskp = thread->u_address.utask;
	/*
	 *	Usual registers on stack.
	 */
	ptrace_up->pt_sp = uthreadp->uu_ar0[SP];
	ptrace_up->pt_r7 = uthreadp->uu_ar0[R7];
	ptrace_up->pt_r6 = uthreadp->uu_ar0[R6];
	ptrace_up->pt_r5 = uthreadp->uu_ar0[R5];
	ptrace_up->pt_r4 = uthreadp->uu_ar0[R4];
	ptrace_up->pt_r3 = uthreadp->uu_ar0[R3];
	ptrace_up->pt_r2 = uthreadp->uu_ar0[R2];
	ptrace_up->pt_r1 = uthreadp->uu_ar0[R1];
	ptrace_up->pt_r0 = uthreadp->uu_ar0[R0];
	ptrace_up->pt_fp = uthreadp->uu_ar0[FP];
	ptrace_up->pt_pc = uthreadp->uu_ar0[PC];
	ptrace_up->pt_psl = uthreadp->uu_ar0[PSR];
	/*
	 *	Floating point registers in pcb; *((int *) &(...)) is to
	 *	prevent conversion.  Caller is responsible for making sure
	 *	the desired values were saved in the pcb.
	 */
	pcb = thread->pcb;
	ptrace_up->pt_fsr = pcb->pcb_fsr;
	*((int *) &(ptrace_up->pt_f0)) = pcb->pcb_f0.val[0];
	*((int *) &(ptrace_up->pt_f1)) = pcb->pcb_f0.val[1];
	*((int *) &(ptrace_up->pt_f2)) = pcb->pcb_f2.val[0];
	*((int *) &(ptrace_up->pt_f3)) = pcb->pcb_f2.val[1];
	*((int *) &(ptrace_up->pt_f4)) = pcb->pcb_f4.val[0];
	*((int *) &(ptrace_up->pt_f5)) = pcb->pcb_f4.val[1];
	*((int *) &(ptrace_up->pt_f6)) = pcb->pcb_f6.val[0];
	*((int *) &(ptrace_up->pt_f7)) = pcb->pcb_f6.val[1];
	ptrace_up->pt_d0 = *((double *)(&pcb->pcb_f0));
	ptrace_up->pt_d1 = *((double *)(&pcb->pcb_f1));
	ptrace_up->pt_d2 = *((double *)(&pcb->pcb_f2));
	ptrace_up->pt_d3 = *((double *)(&pcb->pcb_f3));
	ptrace_up->pt_d4 = *((double *)(&pcb->pcb_f4));
	ptrace_up->pt_d5 = *((double *)(&pcb->pcb_f5));
	ptrace_up->pt_d6 = *((double *)(&pcb->pcb_f6));
	ptrace_up->pt_d7 = *((double *)(&pcb->pcb_f7));
	/*
	 *	Command and aruments to current syscall.
	 */
	bzero (ptrace_up->pt_comm, MAXCOMLEN + 1);
	bcopy (utaskp->uu_comm, ptrace_up->pt_comm, strlen (utaskp->uu_comm));
#if	0
	ptrace_up->pt_arg[0] = uthreadp->uu_ap[0];
	ptrace_up->pt_arg[1] = uthreadp->uu_ap[1];
	ptrace_up->pt_arg[2] = uthreadp->uu_ap[2];
	ptrace_up->pt_arg[3] = uthreadp->uu_ap[3];
	ptrace_up->pt_arg[4] = uthreadp->uu_ap[4];
	ptrace_up->pt_arg[5] = uthreadp->uu_ap[5];
	ptrace_up->pt_arg[6] = uthreadp->uu_ap[6];
	ptrace_up->pt_arg[7] = uthreadp->uu_ap[7];
	ptrace_up->pt_arg[8] = uthreadp->uu_ap[8];
	ptrace_up->pt_arg[9] = uthreadp->uu_ap[9];
#endif
	/*
	 *	Data and stack sizes.
	 */
	ptrace_up->pt_dsize = ctob (utaskp->uu_dsize);
	ptrace_up->pt_ssize = ctob (utaskp->uu_ssize);
	/*
	 *	aouthdr stuff, some of it faked.
	 */
	ptrace_up->pt_aouthdr.magic = 0413;
	ptrace_up->pt_aouthdr.tsize = ctob (utaskp->uu_tsize);
	ptrace_up->pt_aouthdr.entry = 2;
	ptrace_up->pt_aouthdr.text_start = (int)(utaskp->uu_text_start);
	ptrace_up->pt_aouthdr.data_start = (int)(utaskp->uu_data_start);
	/*
	 *	Signal that caused this core dump, base of the user
	 *	stack, and two more fakes.
	 */
	ptrace_up->pt_signal = utaskp->uu_procp->p_cursig;
	ptrace_up->pt_usrstack = USRSTACK;
	ptrace_up->pt_nmem = 0;
	ptrace_up->pt_memptr = 0;
}
