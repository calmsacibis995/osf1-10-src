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
static char	*sccsid = "@(#)$RCSfile: mmax_exception.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:42:07 $";
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

#include <mach/boolean.h>
#include <mach/exception.h>
#include <mach/kern_return.h>

#include <sys/signal.h>

/*
 *	machine_exception translates a mach exception to a unix exception
 *	and code.  This handles all the hardware-specific exceptions for
 *	the ns32000.  unix_exception() handles the machine-independent ones.
 */

boolean_t machine_exception(exception, code, subcode, unix_signal, unix_code)
int	exception, code, subcode;
int	*unix_signal, *unix_code;
{
	switch(exception) {

	    case EXC_BAD_INSTRUCTION:
		if (code == EXC_NS32K_FPU && subcode == EXC_NS32K_FPU_INVALID) {
		    /*
		     * Invalid operand (NAN or INF) or 0.0/0.0
		     */
		    *unix_signal = SIGFPE;
		    *unix_code = FPE_OPERAND_FAULT;
		} else {
		    *unix_signal = SIGILL;
		    *unix_code = ILL_PRIVIN_FAULT;
		}
		break;

	    case EXC_ARITHMETIC:
	        *unix_signal = SIGFPE;
		if (code == EXC_NS32K_DVZ)
		    *unix_code = FPE_INTDIV_TRAP;
		else switch (subcode) {
		    case EXC_NS32K_FPU_UNDERFLOW:
			*unix_code = FPE_FLTUND_FAULT;
			break;
		    case EXC_NS32K_FPU_OVERFLOW:
			*unix_code = FPE_FLTOVF_FAULT;
			break;
		    case EXC_NS32K_FPU_DVZ:
			*unix_code = FPE_FLTDIV_FAULT;
			break;
		    case EXC_NS32K_FPU_INEXACT:
			*unix_code = FPE_INEXACT_FAULT;
			break;
		    case EXC_NS32K_FPU_OPERAND:
			*unix_code = FPE_OPERAND_FAULT;
			break;
		    case EXC_NS32K_FPU_INTOVF:
			*unix_code = FPE_INTOVF_TRAP;
			break;
		}
		break;

	    case EXC_SOFTWARE:
		if (code !=  EXC_NS32K_FLG)
		    return(FALSE);
		/*
		 *	Flag trap is not supported.
		 */
		*unix_signal = SIGILL;
		*unix_code = ILL_FLAG_TRAP;
		break;

	    case EXC_BREAKPOINT:
		*unix_signal = SIGTRAP;
		break;

	    default:
		return(FALSE);
	}
	return(TRUE);
}
