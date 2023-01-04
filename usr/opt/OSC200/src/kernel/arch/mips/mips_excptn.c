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
static char	*sccsid = "@(#)$RCSfile: mips_excptn.c,v $ $Revision: 1.1.3.2 $ (DEC) $Date: 1992/04/15 08:07:45 $";
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
 * derived from mips_exception.c	2.1	(ULTRIX/OSF)	12/3/90";
 */

#include <mach/boolean.h>
#include <mach/exception.h>
#include <sys/signal.h>

/*
 *	machine_exception translates a mach exception to a unix exception
 *	and code.  This handles all the hardware-specific exceptions for
 *	Mips.  unix_exception() handles the machine-independent ones.
 */

boolean_t machine_exception(exception, code, subcode, unix_signal, unix_code)
int	exception, code, subcode;
int	*unix_signal, *unix_code;
{
	switch(exception) {

	    case EXC_BAD_INSTRUCTION:
	        *unix_signal = SIGILL;
		switch (code) {
		    case EXC_MIPS_PRIVINST:
			*unix_code = ILL_PRIVIN_FAULT;
			break;
		    case EXC_MIPS_RESOPND:
			*unix_code = ILL_RESOP_FAULT;
			break;
		    case EXC_MIPS_RESADDR:
			*unix_code = ILL_RESAD_FAULT;
			break;
		    default:
			return(FALSE);
		}
		break;

	    case EXC_ARITHMETIC:
	        *unix_signal = SIGFPE;
		switch (code) {
		    case EXC_MIPS_FLT_OVERFLOW:
			*unix_code = FPE_FLTOVF_FAULT;
			break;
		    case EXC_MIPS_FLT_DIVIDE0:
			*unix_code = FPE_FLTDIV_FAULT;
			break;
		    case EXC_MIPS_FLT_UNDERFLOW:
			*unix_code = FPE_FLTUND_FAULT;
			break;
		    case EXC_MIPS_FLT_UNIMP:
			*unix_code = FPE_UNIMP_FAULT;
			break;
		    case EXC_MIPS_FLT_INVALID:
			*unix_code = FPE_INVALID_FAULT;
			break;
		    case EXC_MIPS_FLT_INEXACT:
			*unix_code = FPE_INEXACT_FAULT;
			break;

		    default:
			return(FALSE);
		}
		break;

	    case EXC_SOFTWARE:
		switch (code) {
		    case EXC_MIPS_SOFT_SEGV:
		        *unix_signal = SIGSEGV;
			break;
		    case EXC_MIPS_SOFT_CPU:
		        *unix_signal = SIGILL;
			*unix_code = ILL_COPR_UNUSABLE;
			break;
		    default:
			return(FALSE);
		}
		break;

	    case EXC_BREAKPOINT:
		*unix_signal = SIGTRAP;
		*unix_code = code;
		break;

	    default:
		return(FALSE);
	}
	return(TRUE);
}
