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
static char *rcsid = "@(#)$RCSfile: fp_tables.c,v $ $Revision: 1.1.4.2 $ (DEC) $Date: 1993/03/22 22:11:20 $";
#endif

/* this file contains a routine to lookup information about fp instructions.
 */

#include <machine/inst.h>
#include <arch/alpha/emulate.h>
#include <arch/alpha/fpu.h>
#include <arch/alpha/local_ieee.h>

#define OLD_SIGCONTEXT

/* the following data definitions are used to define three
 *	tables with a ALPHA floating point instruction function code
 *	as the key for lookups. There is one table for the common floating
 *	point instructons (fltl), the vax floating point instructions (fltv)
 *	and the ieee floating point instructions (flti).
 *
 *	Right now these are only used to map the function code to an
 *	emulator routines and rounding modes.
 */



static
float_entry fltl_table [] =
{
/*
 * op_fltl: datatype independent floating point group. These instructions
 * are floating point operate format (f_format) instructions with the function
 * code encoded in the 11-bit 'function' field.
 */
ENTRY(NONE, fltl_cvtlq, __emulate_cvtlq, "long to quad", N, NONE, NONE, NONE),
ENTRY(NONE, fltl_cpys, __emulate_cpys, "copy sign", N, NONE, NONE, NONE),
ENTRY(NONE, fltl_cpysn, __emulate_cpysn, "copy sign negate", N, NONE, NONE, NONE),
ENTRY(NONE, fltl_cpyse, __emulate_cpyse, "copy sign and exp", N, NONE, NONE, NONE),
ENTRY(NONE, fltl_mt_fpcr, __emulate_mt_fpcr, "move to fpcr", N, NONE, NONE, NONE),
ENTRY(NONE, fltl_mf_fpcr, __emulate_mf_fpcr, "move from fpcr", N, NONE, NONE, NONE),
ENTRY(NONE, fltl_fcmoveq, __emulate_fcmoveq, "move if eq", N, NONE, NONE, NONE),
ENTRY(NONE, fltl_fcmovne, __emulate_fcmovne, "move if ne", N, NONE, NONE, NONE),
ENTRY(NONE, fltl_fcmovlt, __emulate_fcmovlt, "move if lt", N, NONE, NONE, NONE),
ENTRY(NONE, fltl_fcmovge, __emulate_fcmovge, "move if ge", N, NONE, NONE, NONE),
ENTRY(NONE, fltl_fcmovle, __emulate_fcmovle, "move if le", N, NONE, NONE, NONE),
ENTRY(NONE, fltl_fcmovgt, __emulate_fcmovgt, "move if gt", N, NONE, NONE, NONE),
ENTRY(NONE, fltl_cvtql, __emulate_cvtql, "quad to long", N, NONE, NONE, NONE),
ENTRY(NONE, fltl_cvtqlv, __emulate_cvtqlv, "quad to long/V", N, W, NONE, NONE),
ENTRY(NONE, fltl_cvtqlsv, __emulate_cvtqlsv, "quad to long/S/V", N, S|W, NONE, NONE),
ENTRY(0, 0, (double (*)())0, (char *)0, NONE, NONE, NONE, NONE)
};

static
float_entry flti_table [] =
{
/*
 * op_flti: ieee floating point group. These instructions
 * are floating point operate format (f_format) instructions with the function
 * code encoded in the 11-bit 'function' field.
 */
 IEEE_FP_ARITH_OP(adds,,FORMAT_IEEE_S,FORMAT_IEEE_S),
 IEEE_FP_ARITH_OP(subs,,FORMAT_IEEE_S,FORMAT_IEEE_S),
 IEEE_FP_ARITH_OP(muls,,FORMAT_IEEE_S,FORMAT_IEEE_S),
 IEEE_FP_ARITH_OP(divs,|Z|E,FORMAT_IEEE_S,FORMAT_IEEE_S),
 IEEE_FP_ARITH_OP(addt,,FORMAT_IEEE_T,FORMAT_IEEE_T),
 IEEE_FP_ARITH_OP(subt,,FORMAT_IEEE_T,FORMAT_IEEE_T),
 IEEE_FP_ARITH_OP(mult,,FORMAT_IEEE_T,FORMAT_IEEE_T),
 IEEE_FP_ARITH_OP(divt,|Z|E,FORMAT_IEEE_T,FORMAT_IEEE_T),
 IEEE_FP_ARITH_OP(cvtts,,FORMAT_IEEE_T,FORMAT_IEEE_S),
#if defined(flti_cvtst)
ENTRY(flti_cvtst, flti_cvtst, __emulate_cvtst, "convert s to t", N, NONE, FORMAT_IEEE_S,FORMAT_IEEE_T),
ENTRY(flti_cvtst, flti_cvtsts, __emulate_cvtsts, "convert s to t/s", N, S, FORMAT_IEEE_S,FORMAT_IEEE_T),
#endif
TENTRY(flti_cmptun, flti_cmptun, __emulate_cmptun, "ieee t cmp unordered", N, O),
TENTRY(flti_cmpteq, flti_cmpteq, __emulate_cmpteq, "ieee t cmp equal", N, O),
TENTRY(flti_cmptlt, flti_cmptlt, __emulate_cmptlt, "ieee t cmp less than", N, O),
TENTRY(flti_cmptle, flti_cmptle, __emulate_cmptle, "ieee t cmp le", N, O),
TENTRY(flti_cmptun, flti_cmptunsu, __emulate_cmptunsu, "ieee t cmp unordered", N, O|S),
TENTRY(flti_cmpteq, flti_cmpteqsu, __emulate_cmpteqsu, "ieee t cmp equal", N, O|S),
TENTRY(flti_cmptlt, flti_cmptltsu, __emulate_cmptltsu, "ieee t cmp less than", N, O|S),
TENTRY(flti_cmptle, flti_cmptlesu, __emulate_cmptlesu, "ieee t cmp le", N, O|S),
 IEEE_FP_CVT_OP(cvtqt,,,FORMAT_QUAD,FORMAT_IEEE_T),
 IEEE_FP_CVT_OP(cvtqs,,,FORMAT_QUAD,FORMAT_IEEE_S),
TQENTRY(flti_cvttq, flti_cvttq, __emulate_cvttq, "ieee t float to int", N, O),
TQENTRY(flti_cvttq, flti_cvttqc, __emulate_cvttqc, "ieee t float to int/C", C, O),
TQENTRY(flti_cvttq, flti_cvttqv, __emulate_cvttqv, "ieee t float to int/W", N, O|W),
TQENTRY(flti_cvttq, flti_cvttqvc, __emulate_cvttqvc, "ieee t float to int/C", C, O),
TQENTRY(flti_cvttq, flti_cvttqsv, __emulate_cvttqsv, "ieee t float to int/W", N, O|W),
TQENTRY(flti_cvttq, flti_cvttqsvc, __emulate_cvttqsvc, "ieee t float to int/C/S/W", C, O|S|W),
TQENTRY(flti_cvttq, flti_cvttqsvi, __emulate_cvttqsvi, "ieee t float to int/I/S/W", N, I|O|S|W),
TQENTRY(flti_cvttq, flti_cvttqsvic, __emulate_cvttqsvic, "ieee t float to int/C/I/S/W", C, I|O|S|W),
TQENTRY(flti_cvttq, flti_cvttqd, __emulate_cvttqd, "ieee t float to int/D", D, O),
TQENTRY(flti_cvttq, flti_cvttqvd, __emulate_cvttqvd, "ieee t float to int/D/W", D, O|W),
TQENTRY(flti_cvttq, flti_cvttqsvd, __emulate_cvttqsvd, "ieee t float to int/D/S/W", D, O|S|W),
TQENTRY(flti_cvttq, flti_cvttqsvid, __emulate_cvttqsvid, "ieee t float to int/D/I/S/W", D, I|O|S|W),
TQENTRY(flti_cvttq, flti_cvttqm, __emulate_cvttqm, "ieee t float to int/M", M, O),
TQENTRY(flti_cvttq, flti_cvttqvm, __emulate_cvttqvm, "ieee t float to int/M/W", M, O|W),
TQENTRY(flti_cvttq, flti_cvttqsvm, __emulate_cvttqsvm, "ieee t float to int/M/S/W", M, O|S|W),
TQENTRY(flti_cvttq, flti_cvttqsvim, __emulate_cvttqsvim, "ieee t float to int/I/M/S/W", M, I|O|S|W),
ENTRY(0, 0, (double (*)())0, (char *)0, NONE, NONE, NONE, NONE)
};

static
float_entry fltv_table [] =
{
/*
 * op_fltv: vax floating point group. These instructions are
 * floating point operate format (f_format) instructions with the function
 * code encoded in the 11-bit 'function' field.
 */
 VAX_FP_ARITH_OP(addf,,U,u,FORMAT_VAX_F,FORMAT_VAX_F),
 VAX_FP_ARITH_OP(subf,,U,u,FORMAT_VAX_F,FORMAT_VAX_F),
 VAX_FP_ARITH_OP(mulf,,U,u,FORMAT_VAX_F,FORMAT_VAX_F),
 VAX_FP_ARITH_OP(divf,|Z,U,u,FORMAT_VAX_F,FORMAT_VAX_F),
 VAX_FP_ARITH_OP(addg,,U,u,FORMAT_VAX_G,FORMAT_VAX_G),
 VAX_FP_ARITH_OP(subg,,U,u,FORMAT_VAX_G,FORMAT_VAX_G),
 VAX_FP_ARITH_OP(mulg,,U,u,FORMAT_VAX_G,FORMAT_VAX_G),
 VAX_FP_ARITH_OP(divg,|Z,U,u,FORMAT_VAX_G,FORMAT_VAX_G),
 VAX_FP_ARITH_OP(cvtdg,,U,u,FORMAT_VAX_D,FORMAT_VAX_G),
 VAX_FP_ARITH_OP(cvtgf,,U,u,FORMAT_VAX_G,FORMAT_VAX_F),
 VAX_FP_ARITH_OP(cvtgd,,U,u,FORMAT_VAX_G,FORMAT_VAX_D),
 VAX_FP_ARITH_OP(cvtgq,,V,v,FORMAT_VAX_G,FORMAT_QUAD),
GENTRY(NONE, fltv_cmpgeq, __emulate_cmpgeq, "vax g cmp equal", N, O),
GENTRY(NONE, fltv_cmpgeq, __emulate_cmpgeq, "vax g cmp equal/S", N, O|S),
GENTRY(NONE, fltv_cmpglt, __emulate_cmpglt, "vax g cmp less than", N, O),
GENTRY(NONE, fltv_cmpglts, __emulate_cmpglts, "vax g cmp less than/S", N, O|S),
GENTRY(NONE, fltv_cmpgle, __emulate_cmpgle, "vax g cmp less than or equal", N, O),
GENTRY(NONE, fltv_cmpgles, __emulate_cmpgles, "vax g cmp less than or equal/S",N, O|S),
ENTRY(NONE, fltv_cvtqf, __emulate_cvtqf, "vax int to f float", N, NONE,FORMAT_QUAD,FORMAT_VAX_F),
ENTRY(NONE, fltv_cvtqfc, __emulate_cvtqfc, "vax int to f float/C", C, NONE,FORMAT_QUAD,FORMAT_VAX_F),
ENTRY(NONE, fltv_cvtqg, __emulate_cvtqg, "vax int to g float", N, NONE,FORMAT_QUAD,FORMAT_VAX_G),
ENTRY(NONE, fltv_cvtqgc, __emulate_cvtqgc, "vax int to g float/C", C, NONE,FORMAT_QUAD,FORMAT_VAX_G),
ENTRY(0, 0, (double (*)())0, (char *)0, NONE, NONE, NONE, NONE)
};

float_entry *
alpha_float_entry_lookup(
	unsigned long	opcode,		/* opcode to lookup */
	unsigned long	function_code,	/* function code we're looking for */
	unsigned long	rounding,	/* if set look for root with rounding */
	unsigned long	exceptions)	/* if set look for root with traps */
{
    /* lookup function code entry in [flti|fltv|fltl] specific table
     *	which is passed in. If no matching entry is found a zero is
     *	returned.
     */
    float_entry	*table;		/* table to lookup in */
    float_entry	*entry;

    switch (opcode) {
    case op_flti:
	table = flti_table;
	break;

    case op_fltv:
	table = fltv_table;
	break;

    case op_fltl:
	table = fltl_table;
	break;

    default:
	return 0;
    } /* switch */

    for (entry = table; entry->routine; entry++) {

	if (rounding == N && exceptions == NONE) {

	    if (entry->function_code == function_code) {

		return entry;

	    } else {

		continue;

	    } /* if */

	} /* if */

	if (entry->root_function_code != function_code) {

	    continue;

	} /* if */

	/* if rounding specifed check for match */
	if (rounding != N && entry->rounding != rounding) {

		continue;

	} /* if */

	/* if exceptions specifed check for match */
	if (exceptions != NONE&&(entry->exceptions&exceptions) != exceptions) {

		continue;

	} /* if */

	/* found a match */
	return entry;


    } /* for */

    return 0;	/* not found */
} /* alpha_float_entry_lookup */

/* the following table is used to look up attributes for various
 *	formats of operands or results.
 */

static fp_attr_t fp_attr[] = {
/* must match local_ieee.h order */
{FORMAT_NONE,0,0,0},
{FORMAT_IEEE_S, IEEE_S_EXPONENT_MAX, 
	IEEE_S_EXPONENT_BIAS, IEEE_S_EXPONENT_SHIFT,
	IEEE_T_FRACTION_SIZE - IEEE_S_FRACTION_SIZE, IEEE_S_FRACTION_SIZE,
	0x47efffffe0000000},
{FORMAT_IEEE_T, IEEE_T_EXPONENT_MAX, 
	IEEE_T_EXPONENT_BIAS, IEEE_T_EXPONENT_SHIFT,
	NONE, IEEE_T_FRACTION_SIZE,
	IEEE_PLUS_LARGEST_NUMBER},
};

extern fp_attr_t *
alpha_fp_attr_lookup(unsigned long format)
{
    /* return pointer to fp_attr entry which describes the attribuites
     *	of the format. we only supply thru IEEE ones
     */
    if (format > FORMAT_IEEE_T)
	return 0;
    return fp_attr+format;
} /* alpha_fp_attr_lookup */
