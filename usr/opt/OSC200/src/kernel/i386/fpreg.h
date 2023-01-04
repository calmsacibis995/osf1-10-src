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
/*	
 *	@(#)$RCSfile: fpreg.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:17:48 $
 */ 
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
 *  ================================================================
 *  Copyright 1988, 1989 by Intel Corporation
 *
 *              intel corporation proprietary information
 *  this software  is  supplied  under  the  terms  of  a  license
 *  agreement  or non-disclosure agreement  with intel corporation
 *  and  may not be copied nor disclosed except in accordance with
 *  the terms of that agreement.                                  
 *  ================================================================
 */

#ifndef _I386_FPREG_H_
#define _I386_FPREG_H_

#ifndef ASSEMBLER

#include <sys/types.h>

extern char   fp_kind;			/* what fp support do we have */
extern struct thread  *fp_thread;     /* thread that own the fp unit */

#endif /* ASSEMBLER */


/*
 * values that go into fp_kind
 */
#define FP_NO   0       /* no fp chip, no emulator (no fp support)      */
#define FP_SW   1       /* no fp chip, using software emulator          */
#define FP_HW   2       /* chip present bit                             */
#define FP_287  2       /* 80287 chip present                           */
#define FP_387  3       /* 80387 chip present                           */


/*
 * 80287/80387 floating point processor definitions
 */

/* Is the 80287 state compatible with this definition? */

#ifndef ASSEMBLER

/* 80387 register */
typedef u_char i387_reg[10];
/* 
 * mantissa : 64;
 * exponent: 15;
 * sign : 1;
 */

/* 80387 "environment" */
struct i387_env {
	u_int	control	: 16;
	u_int		: 16;
	u_int	status	: 16;
	u_int		: 16;
	u_int	tags	: 16;
	u_int		: 16;
	u_int	ip	: 32;
	u_int	cs_sel	: 16;		/* CS selector */
	u_int	opcode	: 11;
	u_int		: 5;
	u_int	data	: 32;		/* data operand offset */
	u_int	data_sel : 16;		/* data operand selector */
	u_int		: 16;
};

/* 
 * Actual saved state.  Note that the first register in "stack" is the 
 * top of stack; it is not necessarily physical register 0.
 */

struct i387_state {
	struct i387_env	env;
	i387_reg stack[8];
};

#define FP_STATE_BYTES	sizeof(struct i387_state)

#endif /* ASSEMBLER */


/*
 * masks for 80387 control word
 */
#define FPINV   0x00000001      /* invalid operation                    */
#define FPDNO   0x00000002      /* denormalized operand                 */
#define FPZDIV  0x00000004      /* zero divide                          */
#define FPOVR   0x00000008      /* overflow                             */
#define FPUNR   0x00000010      /* underflow                            */
#define FPPRE   0x00000020      /* precision                            */
#define FPPC    0x00000300      /* precision control                    */
#define FPRC    0x00000C00      /* rounding control                     */
#define FPIC    0x00001000      /* infinity control                     */
#define WFPDE   0x00000080      /* data chain exception                 */

/*
 * precision, rounding, and infinity options in control word
 */
#define FPSIG24 0x00000000      /* 24-bit significand precision (short) */
#define FPSIG53 0x00000200      /* 53-bit significand precision (long)  */
#define FPSIG64 0x00000300      /* 64-bit significand precision (temp)  */
#define FPRTN   0x00000000      /* round to nearest or even             */
#define FPRD    0x00000400      /* round down                           */
#define FPRU    0x00000800      /* round up                             */
#define FPCHOP  0x00000C00      /* chop (truncate toward zero)          */
#define FPP     0x00000000      /* projective infinity                  */
#define FPA     0x00001000      /* affine infinity                      */
#define WFPB17  0x00020000      /* bit 17                               */
#define WFPB24  0x01000000      /* bit 24                               */

/*
 * masks for 80387 status word
 */
#define FPS_ES	0x00000080      /* error summary bit                    */

#endif /* _I386_FPREG_H_ */
