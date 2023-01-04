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
 * @(#)$RCSfile: emulate.h,v $ $Revision: 1.1.2.3 $ (DEC) $Date: 1993/01/15 16:35:30 $
 */

/*
 * this file contains routines which will emulate floating point instructions
 *	with a trap barrier immediately behind them so that we can
 *	determine the exception cause since an exception summary may
 *	not uniquely identify which exception occured. The caller
 *	is responsible for setting up the state including the fpcr,
 *	trap enables and signal handlers.
 *
 * this file also creates the prototypes used by C to call these functions.
 */

#ifdef __LANGUAGE_C__

#define EMULATE_0_FPARG_OP(OPCODE)					\
	extern	double __emulate_##OPCODE();

#define EMULATE_1_FPARG_OP(OPCODE)					\
	extern	double __emulate_##OPCODE();

#define EMULATE_2_FPARG_OP(OPCODE)					\
	extern	double __emulate_##OPCODE();

#else	/* assembler */

#define EMULATE_0_FPARG_OP(OPCODE)					\
	.globl	__emulate_##OPCODE;					\
	.ent	__emulate_##OPCODE;					\
__emulate_##OPCODE:							\
	.frame  $sp, 0, $26;						\
	OPCODE	$f0;							\
	trapb;								\
	ret	$31, ($26), 1;						\
	.end	__emulate_##OPCODE;

#define EMULATE_1_FPARG_OP(OPCODE)					\
	.globl	__emulate_##OPCODE;					\
	.ent	__emulate_##OPCODE;					\
__emulate_##OPCODE:							\
	.frame  $sp, 0, $26;						\
	OPCODE	$f17,$f0;						\
	trapb;								\
	ret	$31, ($26), 1;						\
	.end	__emulate_##OPCODE;

#define EMULATE_2_FPARG_OP(OPCODE)					\
	.globl	__emulate_##OPCODE;					\
	.ent	__emulate_##OPCODE;					\
__emulate_##OPCODE:							\
	.frame  $sp, 0, $26;						\
	OPCODE	$f16,$f17,$f0;						\
	trapb;								\
	ret	$31, ($26), 1;						\
	.end	__emulate_##OPCODE;
#endif /* __LANGUAGE_C__ */

#define IEEE_ARITH_EMULATE(ROOT,OPERANDS)		\
	EMULATE_##OPERANDS##_FPARG_OP(ROOT)			\
	EMULATE_##OPERANDS##_FPARG_OP(ROOT##c)			\
	EMULATE_##OPERANDS##_FPARG_OP(ROOT##m)			\
	EMULATE_##OPERANDS##_FPARG_OP(ROOT##d)			\
	EMULATE_##OPERANDS##_FPARG_OP(ROOT##u)			\
	EMULATE_##OPERANDS##_FPARG_OP(ROOT##uc)			\
	EMULATE_##OPERANDS##_FPARG_OP(ROOT##um)			\
	EMULATE_##OPERANDS##_FPARG_OP(ROOT##ud)			\
	EMULATE_##OPERANDS##_FPARG_OP(ROOT##su)			\
	EMULATE_##OPERANDS##_FPARG_OP(ROOT##suc)			\
	EMULATE_##OPERANDS##_FPARG_OP(ROOT##sum)			\
	EMULATE_##OPERANDS##_FPARG_OP(ROOT##sud)			\
	EMULATE_##OPERANDS##_FPARG_OP(ROOT##sui)			\
	EMULATE_##OPERANDS##_FPARG_OP(ROOT##suic)			\
	EMULATE_##OPERANDS##_FPARG_OP(ROOT##suim)			\
	EMULATE_##OPERANDS##_FPARG_OP(ROOT##suid)

#define IEEE_FP_CVT_EMULATE(ROOT)				\
	EMULATE_1_FPARG_OP(ROOT)				\
	EMULATE_1_FPARG_OP(ROOT##c)				\
	EMULATE_1_FPARG_OP(ROOT##m)				\
	EMULATE_1_FPARG_OP(ROOT##d)				\
	EMULATE_1_FPARG_OP(ROOT##sui)				\
	EMULATE_1_FPARG_OP(ROOT##suic)				\
	EMULATE_1_FPARG_OP(ROOT##suim)				\
	EMULATE_1_FPARG_OP(ROOT##suid)

#define VAX_FP_ARITH_EMULATE(ROOT,OPERANDS,ov)			\
	EMULATE_##OPERANDS##_FPARG_OP(ROOT)			\
	EMULATE_##OPERANDS##_FPARG_OP(ROOT##c)			\
	EMULATE_##OPERANDS##_FPARG_OP(ROOT##ov)			\
	EMULATE_##OPERANDS##_FPARG_OP(ROOT##ov##c)		\
	EMULATE_##OPERANDS##_FPARG_OP(ROOT##s)			\
	EMULATE_##OPERANDS##_FPARG_OP(ROOT##sc)			\
	EMULATE_##OPERANDS##_FPARG_OP(ROOT##s##ov)		\
	EMULATE_##OPERANDS##_FPARG_OP(ROOT##s##ov##c)


/*
 * op_fltl: datatype independent floating point group. These instructions
 * are floating point operate format (f_format) instructions with the function
 * code encoded in the 11-bit 'function' field.
 */
	EMULATE_1_FPARG_OP(cvtlq)
	EMULATE_2_FPARG_OP(cpys)
	EMULATE_2_FPARG_OP(cpysn)
	EMULATE_2_FPARG_OP(cpyse)
	EMULATE_0_FPARG_OP(mt_fpcr)
	EMULATE_0_FPARG_OP(mf_fpcr)
	EMULATE_2_FPARG_OP(fcmoveq)
	EMULATE_2_FPARG_OP(fcmovne)
	EMULATE_2_FPARG_OP(fcmovlt)
	EMULATE_2_FPARG_OP(fcmovge)
	EMULATE_2_FPARG_OP(fcmovle)
	EMULATE_2_FPARG_OP(fcmovgt)
	EMULATE_1_FPARG_OP(cvtql)
	EMULATE_1_FPARG_OP(cvtqlv)
	EMULATE_1_FPARG_OP(cvtqlsv)

/*
 * op_flti: ieee floating point group. These instructions
 * are floating point operate format (f_format) instructions with the function
 * code encoded in the 11-bit 'function' field.
 */
	IEEE_ARITH_EMULATE(adds,2)
	IEEE_ARITH_EMULATE(subs,2)
	IEEE_ARITH_EMULATE(muls,2)
	IEEE_ARITH_EMULATE(divs,2)
	IEEE_ARITH_EMULATE(addt,2)
	IEEE_ARITH_EMULATE(subt,2)
	IEEE_ARITH_EMULATE(mult,2)
	IEEE_ARITH_EMULATE(divt,2)
	IEEE_ARITH_EMULATE(cvtts,1)
#ifdef flti_cvtst
	EMULATE_1_FPARG_OP(cvtst)
	EMULATE_1_FPARG_OP(cvtsts)
#endif

	EMULATE_2_FPARG_OP(cmptun)
	EMULATE_2_FPARG_OP(cmpteq)
	EMULATE_2_FPARG_OP(cmptlt)
	EMULATE_2_FPARG_OP(cmptle)
	EMULATE_2_FPARG_OP(cmptunsu)
	EMULATE_2_FPARG_OP(cmpteqsu)
	EMULATE_2_FPARG_OP(cmptltsu)
	EMULATE_2_FPARG_OP(cmptlesu)

	IEEE_FP_CVT_EMULATE(cvtqt)
	IEEE_FP_CVT_EMULATE(cvtqs)

	EMULATE_1_FPARG_OP(cvttq)
	EMULATE_1_FPARG_OP(cvttqc)
	EMULATE_1_FPARG_OP(cvttqv)
	EMULATE_1_FPARG_OP(cvttqvc)
	EMULATE_1_FPARG_OP(cvttqsv)
	EMULATE_1_FPARG_OP(cvttqsvc)
	EMULATE_1_FPARG_OP(cvttqsvi)
	EMULATE_1_FPARG_OP(cvttqsvic)
	EMULATE_1_FPARG_OP(cvttqd)
	EMULATE_1_FPARG_OP(cvttqvd)
	EMULATE_1_FPARG_OP(cvttqsvd)
	EMULATE_1_FPARG_OP(cvttqsvid)
	EMULATE_1_FPARG_OP(cvttqm)
	EMULATE_1_FPARG_OP(cvttqvm)
	EMULATE_1_FPARG_OP(cvttqsvm)
	EMULATE_1_FPARG_OP(cvttqsvim)

/*
 * op_fltv: vax floating point group. These instructions are
 * floating point operate format (f_format) instructions with the function
 * code encoded in the 11-bit 'function' field.
 */
	VAX_FP_ARITH_EMULATE(addf,2,u)
	VAX_FP_ARITH_EMULATE(subf,2,u)
	VAX_FP_ARITH_EMULATE(mulf,2,u)
	VAX_FP_ARITH_EMULATE(divf,2,u)
	VAX_FP_ARITH_EMULATE(addg,2,u)
	VAX_FP_ARITH_EMULATE(subg,2,u)
	VAX_FP_ARITH_EMULATE(mulg,2,u)
	VAX_FP_ARITH_EMULATE(divg,2,u)
	VAX_FP_ARITH_EMULATE(cvtdg,1,u)
	VAX_FP_ARITH_EMULATE(cvtgf,1,u)
	VAX_FP_ARITH_EMULATE(cvtgd,1,u)
	VAX_FP_ARITH_EMULATE(cvtgq,1,v)

	EMULATE_2_FPARG_OP(cmpgeq)
	EMULATE_2_FPARG_OP(cmpgeqs)
	EMULATE_2_FPARG_OP(cmpglt)
	EMULATE_2_FPARG_OP(cmpglts)
	EMULATE_2_FPARG_OP(cmpgle)
	EMULATE_2_FPARG_OP(cmpgles)
	EMULATE_1_FPARG_OP(cvtqf)
	EMULATE_1_FPARG_OP(cvtqfc)
	EMULATE_1_FPARG_OP(cvtqg)
	EMULATE_1_FPARG_OP(cvtqgc)
