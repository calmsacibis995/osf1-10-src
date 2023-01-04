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
static char	*sccsid = "@(#)$RCSfile: vx_exception.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:20:39 $";
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

#include <mach/boolean.h>
#include <mach/exception.h>
#include <mach/kern_return.h>
#include <sys/signal.h>

/*
 *	machine_exception translates a mach exception to a unix exception
 *	and code.  This handles all the hardware-specific exceptions for
 *	the vax.  unix_exception() handles the machine-independent ones.
 */

boolean_t machine_exception(exception, code, subcode, unix_signal, unix_code)
int	exception, code, subcode;
int	*unix_signal, *unix_code;
{
	switch(exception) {

	    case EXC_BAD_INSTRUCTION:
	        *unix_signal = SIGILL;
		switch (code) {
		    case EXC_I386_INVOP:
			*unix_code = ILL_RESOP_FAULT;
			break;
		    default:
			return(FALSE);
		}
		break;

	    case EXC_ARITHMETIC:
	        *unix_signal = SIGFPE;
		switch (code) {
		    case EXC_I386_INTO:
			*unix_code = FPE_INTOVF_TRAP;
			break;
		    case EXC_I386_DIV:
			*unix_code = FPE_INTDIV_TRAP;
			break;
		    case EXC_I386_BOUND:
			*unix_code = FPE_SUBRNG_TRAP;
			break;
		    case EXC_I386_NOEXT:
		    case EXC_I386_EXTOVR:
		    case EXC_I386_EXTERR:
		    case EXC_I386_EMERR:
			*unix_code = 0;
			break;
		    default:
			return(FALSE);
		}
		break;

	    case EXC_BREAKPOINT:
		*unix_signal = SIGTRAP;
		break;

	    default:
		return(FALSE);
	}
	return(TRUE);
}
