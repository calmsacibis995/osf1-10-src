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
static char *rcsid = "@(#)$RCSfile: alpha_exception.c,v $ $Revision: 1.2.7.4 $ (DEC) $Date: 1994/01/17 22:19:51 $";
#endif
#ifndef lint
static char	*sccsid = "@(#)alpha_exception.c	9.2	(ULTRIX/OSF)	10/28/91";
#endif 
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
 * Mach Operating System
 * Copyright (c) 1989 Carnegie-Mellon University
 * All rights reserved.  The CMU software License Agreement specifies
 * the terms and conditions for use and redistribution.
 */
/*
 * OSF/1 Release 1.0
 */

#include <mach/boolean.h>
#include <mach/exception.h>
#include <machine/trap.h>
#include <arch/alpha/gentrap.h>
#include <sys/signal.h>
#include <mach/kern_return.h>
#include <sys/siginfo.h>

/*
 *	machine_exception() translates a mach exception to a unix exception
 *	and code.  This handles all the hardware-specific exceptions for
 *	Alpha.  unix_exception() handles the machine-independent ones.
 */

boolean_t
machine_exception(exception, code, subcode, unix_signal, unix_code, siginfo_p)
	unsigned long exception, code, subcode;
	int *unix_signal, *unix_code;
	register k_siginfo_t *siginfo_p;
{
	/*
	 * The subcode is almost always the address for the following 
	 * signals.  When it is not, the field is not looked at.
	 */
        siginfo_p->si_addr = (caddr_t) subcode;
	siginfo_p->si_errno = 0; /* set it here in case it is not set later */

	switch (exception) {
	case EXC_BAD_INSTRUCTION:
	        *unix_signal = SIGILL;

		switch (code) {
		case T_IFAULT_OPDEC:
		default:
			*unix_code = ILL_INST_FAULT;
			break;
		}
		siginfo_p->si_code = ILL_ILLOPC;
		break;

	case EXC_ARITHMETIC:
	        *unix_signal = SIGFPE;
		*unix_code = code;
		switch (code) {
		case  FPE_FLTDIV_FAULT:
			siginfo_p->si_code = FPE_FLTDIV;
			break;
		case FPE_FLTUND_FAULT:
			siginfo_p->si_code = FPE_FLTUND;
			break;
		case FPE_INEXACT_FAULT:
			siginfo_p->si_code = FPE_FLTRES;
			break;
		case FPE_FLTOVF_FAULT:
		case FPE_INTOVF_FAULT:
			siginfo_p->si_code = FPE_FLTOVF;
			break;
		case FPE_INVALID_FAULT: /* Fall Through */
		default:
			siginfo_p->si_code = FPE_FLTINV;
			break;
		}
		break;

	case EXC_AST:
		*unix_signal = SIGKILL;
		*unix_code = code;
		break;

	case EXC_BAD_ACCESS:
		*unix_signal = SIGSEGV;
		*unix_code = code;

		switch (code) {
		case KERN_INVALID_ADDRESS:
		case KERN_MEMORY_ERROR:
			siginfo_p->si_code = SEGV_MAPERR;
			break;
		case KERN_PROTECTION_FAILURE:
			siginfo_p->si_code = SEGV_ACCERR;
			break;
		default:
			*unix_signal = SIGBUS;
			siginfo_p->si_code = BUS_ADRERR;
			break;
		}
		break;

	case EXC_BREAKPOINT:
		*unix_signal = SIGTRAP;
		*unix_code = code;

		switch (subcode) {
		case EXC_ALPHA_BPT:
			siginfo_p->si_code = TRAP_BRKPT;
			break;

		case EXC_ALPHA_TRACE: /* Fall Through */
		default:
			siginfo_p->si_code = TRAP_TRACE; 
			break;
		}
		break;

	/* GENTRAP handling */
	case EXC_SOFTWARE:
		switch (code) {
		case GEN_INTOVF:
			*unix_signal = SIGFPE;
			siginfo_p->si_code = FPE_INTOVF;
			break;
		case GEN_INTDIV:
			*unix_signal = SIGFPE;
			siginfo_p->si_code = FPE_INTDIV;
			break;
	  	case GEN_FLTOVF:
			*unix_signal = SIGFPE;
			siginfo_p->si_code = FPE_FLTOVF;
			break;
		case GEN_FLTDIV:
			*unix_signal = SIGFPE;
			siginfo_p->si_code = FPE_FLTDIV;
			break;
		case GEN_FLTUND:
			*unix_signal = SIGFPE;
			siginfo_p->si_code = FPE_FLTUND;
			break;
		case GEN_FLTINV:
			*unix_signal = SIGFPE;
			siginfo_p->si_code = FPE_FLTINV;
			break;
		case GEN_FLTINE:
			*unix_signal = SIGFPE;
			siginfo_p->si_code = FPE_FLTRES;
			break;
		case GEN_ROPRAND:
			*unix_signal = SIGFPE;
			siginfo_p->si_code = FPE_FLTINV;
			break;
		case EXC_ALPHA_SOFT_SEGV:
			*unix_signal = SIGSEGV;
			siginfo_p->si_code = SEGV_MAPERR; 
			break;
		case EXC_ALPHA_SOFT_STK:
			*unix_signal = SIGILL;
			siginfo_p->si_code = ILL_BADSTK; 
			break;
		case EXC_ALPHA_INST_TRAP: /* gen inst fault from trap() */
			return(FALSE); /* let ux_exception() handle it */
		default:
			/*
			 * If the unmatched code isn't in the hardware
			 * range, then don't handle it here.
			 */
			if ((long)code >= 0)
				return(FALSE);
			*unix_signal = SIGTRAP;
			break;
		}
		*unix_code = code;
		break;

	default:
		return(FALSE);
        }
	return(TRUE);
}
