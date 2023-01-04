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
 * softfp.h -- constants for software floating point emulation
 */

#ifndef _SOFTFP_H_
#define _SOFTFP_H_
/*
 * These constants refer to floating-point values for all formats
 */
#define	SIGNBIT		0x80000000

#define	GUARDBIT	0x80000000

/*
 * These constants refer to word values
 */
#define	WORD_MIN	0x80000000
#define	WORD_MAX	0x7fffffff
#define	WEXP_MIN	-1
#define	WEXP_MAX	30
#define	WQUIETNAN_LEAST	0x7fffffff

/*
 * These constants refer to single format floating-point values
 */
#define	SEXP_SHIFT	23
#define	SEXP_MASK	0xff
#define	SEXP_NAN	0xff
#define	SEXP_INF	0xff
#define	SEXP_BIAS	127
#define	SEXP_MAX	127
#define	SEXP_MIN	-126
#define	SEXP_OU_ADJ	192
#define	SIMP_1BIT	0x00800000
#define	SFRAC_LEAD0S	8
#define	SFRAC_BITS	23
#define	SFRAC_MASK	0x007fffff
#define	SFRAC_LEAST_MAX	0x007fffff

#define	SSNANBIT_MASK	0x00400000
#define	SQUIETNAN_LEAST	0x7fbfffff

/*
 * These constants refer to double format floating-point values
 */

#define	DEXP_SHIFT	52
#define	DEXP_MASK	0x7ff
#define	DEXP_NAN	0x7ff
#define	DEXP_INF	0x7ff
#define	DEXP_BIAS	1023
#define	DEXP_MAX	1023
#define	DEXP_MIN	-1022
#define	DEXP_OU_ADJ	1536
#define	DIMP_1BIT	0x00100000
#define	DFRAC_LEAD0S	11
#define	DFRAC_BITS	52
#define	DFRAC_MASK	0xfffffffffffffL
#define	DFRAC_LESS_MAX	0x000fffff
#define	DFRAC_LEAST_MAX	0xffffffff

#define	DSNANBIT_MASK	0x00080000
#define	DQUIETNAN_LESS	0x7ff7ffff
#define	DQUIETNAN_LEAST	0xffffffff

/* floating point opcodes */
#define FP_OP_OPCODE	0x16		/* IEEE operate floating point opcode */
#define FP_VAX_OPCODE	0x15		/* VAX floating point opcode */

/* instruction func bits */
#define DYNAMIC_BIT	0x80

/* floating point function codes capable of generating +/- infinity errors */
#define	ADDSM		0x40 	
#define	ADDSD	 	0xc0
#define	ADDSUM 		0x140
#define	ADDSUD 		0x1c0
#define	ADDSSUM	 	0x540
#define	ADDSSUD 	0x5c0
#define	ADDSSUIM 	0x740
#define	ADDSSUID	0x7c0
#define	ADDTM		0x60 	
#define	ADDTD	 	0xe0
#define	ADDTUM 		0x160
#define	ADDTUD 		0x1e0
#define	ADDTSUM	 	0x560
#define	ADDTSUD 	0x5e0
#define	ADDTSUIM 	0x760
#define	ADDTSUID	0x7e0
	
#define CVTQSM		0x7c
#define CVTQSD		0xfc
#define CVTQSSUIM	0x77c
#define CVTQSSUID	0x7fc

#define CVTQTM		0x7e
#define CVTQTD		0xfe
#define CVTQTSUIM	0x77e
#define CVTQTSUID	0x7fe

#define CVTTQD		0xef
#define CVTTQVD		0x1ef
#define CVTTQSVD	0x5ef
#define CVTTQSVID	0x7ef
#define CVTTQM		0x6f
#define CVTTQVM		0x16f
#define CVTTQSVM	0x56f
#define CVTTQSVIM	0x76f

#define CVTTSM		0x6c
#define CVTTSD		0xec
#define CVTTSUM		0x16c
#define CVTTSUD		0x1ec
#define CVTTSSUM	0x56c
#define CVTTSSUD	0x5ec
#define CVTTSSUIM	0x76c
#define CVTTSSUID	0x7ec

#define	DIVSM		0x43 	
#define	DIVSD	 	0xc3
#define	DIVSUM 		0x143
#define	DIVSUD 		0x1c3
#define	DIVSSUM	 	0x543
#define	DIVSSUD 	0x5c3
#define	DIVSSUIM 	0x743
#define	DIVSSUID	0x7c3
#define	DIVTM		0x63 	
#define	DIVTD	 	0xe3
#define	DIVTUM 		0x163
#define	DIVTUD 		0x1e3
#define	DIVTSUM	 	0x563
#define	DIVTSUD 	0x5e3
#define	DIVTSUIM 	0x763
#define	DIVTSUID	0x7e3

#define	MULSM		0x42 	
#define	MULSD	 	0xc2
#define	MULSUM 		0x142
#define	MULSUD 		0x1c2
#define	MULSSUM	 	0x542
#define	MULSSUD 	0x5c2
#define	MULSSUIM 	0x742
#define	MULSSUID	0x7c2
#define	MULTM		0x62	
#define	MULTD	 	0xe2
#define	MULTUM 		0x162
#define	MULTUD 		0x1e2
#define	MULTSUM	 	0x562
#define	MULTSUD 	0x5e2
#define	MULTSUIM 	0x762
#define	MULTSUID	0x7e2

#define	SUBSM		0x41 	
#define	SUBSD	 	0xc1
#define	SUBSUM 		0x141
#define	SUBSUD 		0x1c1
#define	SUBSSUM	 	0x541
#define	SUBSSUD 	0x5c1
#define	SUBSSUIM 	0x741
#define	SUBSSUID	0x7c1
#define	SUBTM		0x61	
#define	SUBTD	 	0xe1
#define	SUBTUM 		0x161
#define	SUBTUD 		0x1e1
#define	SUBTSUM	 	0x561
#define	SUBTSUD 	0x5e1
#define	SUBTSUIM 	0x761
#define	SUBTSUID	0x7e1

/* fpcr fields and masks*/

/* masks */
#define DYNAMIC_MASK	0xc0
#define MINUS_MASK	0x40
#define DOUBLE_MASK	0x20

/* rounding modes (fpcr.dyn) */
#define ROUND_CHOPPED	0x0		/* Chopped Rounding mode */
#define ROUND_MINF	0x1		/* Minus Infinity Rounding mode */
#define ROUND_NORMAL	0x2		/* Normal Rounding mode */
#define ROUND_PINF	0x3		/* Plus Infinity Rounding mode */

/* fp_infinity() return codes */
#define FP_NOT_FP_INSTR		1
#define FP_NOT_INFINITY		2
#define FP_FIXED_INFINITY	3
#define FP_SIGNAL_USER		4

#endif
