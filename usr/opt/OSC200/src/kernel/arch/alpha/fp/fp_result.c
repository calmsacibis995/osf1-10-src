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
static char *rcsid = "@(#)$RCSfile: fp_result.c,v $ $Revision: 1.1.12.4 $ (DEC) $Date: 1993/12/17 15:03:49 $";
#endif

#ifndef KERNEL
#include <stdio.h>
#include <setjmp.h>
#endif
#include <sys/types.h>
#include <sys/buf.h>
#include <sys/user.h>
#include <machine/trap.h>
#include <mach/machine/exception.h>
#include <machine/reg.h>
#include <machine/softfp.h>
#include <sys/signal.h>
#include <machine/inst.h>
#include <arch/alpha/emulate.h>
#include <arch/alpha/fpu.h>
#include <arch/alpha/local_ieee.h>

#define BITMASK(x) ((1L << (x)) - 1)

#define MODULE_NAME	ieee_default_value

#define FP_EPRINTF(s)   EPRINTF((" \042%s\n   pc:\
    0x%lx\n  ins: 0x%x\n  op1: 0x%lx\n  op2: 0x%lx\n",s \
    ,pfp_context->pc \
    ,pfp_context->inst \
    ,pfp_context->original_operand1 \
    ,pfp_context->original_operand2 \
    ));



static unsigned long
do_op_and_rescale(
	alpha_fp_trap_context		*pfp_context,
	long				exponent_scale, 
	struct fp_attr			*pfp_attr,
	unsigned long			round)
{
    float_entry		*rounded_float_entry;
    excsum_t		excsum;
    fpcr_t		fpcr;

    /* function executes an opcode on normalized operands and then
     *	denormalizes the result based on arguments
     */
    rounded_float_entry = alpha_float_entry_lookup(op_flti, 
	pfp_context->pfloat_entry->root_function_code, 
	round, S|U);

    if (rounded_float_entry == 0) {
	DPRINTF(("cannot find entry for root function in IEEE default value routine\n"));
	return SFP_FAILURE;
    } /* if */


    /* now we've scaled the operands, exec the op w/o exceptions set 
     *	and check for inexact result
     */
    if (pfp_context->dont_reset_inexact_result == 0) {
	fpcr.qval = _get_fpcr();
	if (fpcr.fields.inexact_result) {
	    fpcr.fields.inexact_result = 0;	/* clear sticky bit */
	    _set_fpcr(fpcr.qval);
	} /* if */
    } /* if */
    excsum.qval = 0;
    DPRINTF(("emulating instruction with renormalized operands\n"));
    if (alpha_emulate_fp_instruction(rounded_float_entry, pfp_context->operand1,
		pfp_context->operand2, &pfp_context->result, &excsum) == 
		SFP_FAILURE) {
	/* got an exception */
	DPRINTF(("got exception (0x%lx) during IEEE default value calculation\n", excsum));
	pfp_context->isolated_excsum.qval |= excsum.qval;
	return SFP_SUCCESS;
    } /* if */

    /* fetch fpcr so we can test inexact bit */
    pfp_context->fpcr.qval = _get_fpcr();
    DPRINTF(("after finite operand emulation fpcr= "));
    DPRINT_FPCR(pfp_context->fpcr);
    DPRINTF(("\n"));
    DPRINTF(("after finite operand emulation result= %e(0x%016lx) fraction = 0x%013lx\n", pfp_context->result.dval, pfp_context->result.qval, pfp_context->result.t.fraction));

    if (pfp_context->fpcr.fields.inexact_result) {
	/* if inexact is set, pass it on to the exception summary */
	pfp_context->isolated_excsum.fields.inexact_result = 1;
    } /* if */

    return ieee_t_scale(&pfp_context->result, -(long)exponent_scale, 
	pfp_context, pfp_attr);
} /* do_op_and_rescale */

void
add_smallest_denorm(
	alpha_fp_trap_context		*pfp_context,
	struct fp_attr			*pfp_attr)
{
    unsigned long	fraction_mask;

    DPRINTF(("adding smallest denorm\n"));
    if (pfp_context->result.t.sign != IEEE_PLUS_SIGN) {
	/* subtract one fraction bit for negative numbers to add */
	if (pfp_context->result.t.fraction == 0) {

	/* set fraction to all ones and bump exponent based on sign */
	fraction_mask =  (1L << IEEE_T_FRACTION_SIZE) - 1L;
	fraction_mask &= ~((1L << pfp_attr->fraction_fill) - 1L);
	pfp_context->result.t.fraction = fraction_mask;
	pfp_context->result.t.exponent -= 1;

	} else {
	    pfp_context->result.t.fraction -= (1 << pfp_attr->fraction_fill);
	} /* if */
    } else {
	/* add one */
	fraction_mask =  (1L << IEEE_T_FRACTION_SIZE) - 1L;
	fraction_mask &= ~((1L << pfp_attr->fraction_fill) - 1L);
	if (pfp_context->result.t.fraction == fraction_mask) {
	    if (pfp_context->result.t.exponent + 1 > pfp_attr->exponent_max) {
	        if (pfp_context->rounding == IEEE_ROUND_TO_ZERO ||
		   pfp_context->rounding == IEEE_ROUND_TO_MINUS_INFINITY) {

		    /* certain rounding modes cannot go to +infinity */
		    return;
		} /* if */
		pfp_context->fp_control.fields.floating_overflow = 1;
		pfp_context->isolated_excsum.fields.inexact_result = 1;
	    } /* if */
	    pfp_context->result.t.fraction = 0;
	    pfp_context->result.t.exponent += 1;
	} else {
	    pfp_context->result.t.fraction += (1 << pfp_attr->fraction_fill);
	} /* if */
    } /* if */
} /* sub_smallest_denorm */

void
sub_smallest_denorm(
	alpha_fp_trap_context		*pfp_context,
	struct fp_attr			*pfp_attr)
{
    unsigned long	fraction_mask;

    DPRINTF(("subtracting smallest denorm\n"));
    if (pfp_context->result.t.sign == IEEE_PLUS_SIGN) {
	/* subtract one fraction bit and check if we need to modify exp too */
	if (pfp_context->result.t.fraction == 0) {

	/* set fraction to all ones and bump exponent based on sign */
	fraction_mask =  (1L << IEEE_T_FRACTION_SIZE) - 1L;
	fraction_mask &= ~((1L << pfp_attr->fraction_fill) - 1L);
	pfp_context->result.t.fraction = fraction_mask;
	pfp_context->result.t.exponent -= 1;

	} else {
	    pfp_context->result.t.fraction -= (1 << pfp_attr->fraction_fill);
	} /* if */
    } else {
	/* if a negative number add one to subtract */
	DPRINTF(("add to negative\n"));
	fraction_mask =  (1L << IEEE_T_FRACTION_SIZE) - 1L;
	fraction_mask &= ~((1L << pfp_attr->fraction_fill) - 1L);
	if (pfp_context->result.t.fraction == fraction_mask) {
	    if (pfp_context->result.t.exponent + 1 > pfp_attr->exponent_max) {
	        if (pfp_context->rounding == IEEE_ROUND_TO_ZERO ||
		   pfp_context->rounding == IEEE_ROUND_TO_PLUS_INFINITY) {

		    /* certain rounding modes cannot go to -infinity */
		    DPRINTF(("avoid infinity\n"));
		    return;
		} /* if */
		pfp_context->fp_control.fields.floating_overflow = 1;
		pfp_context->isolated_excsum.fields.inexact_result = 1;
	    } /* if */
	    pfp_context->result.t.fraction = 0;
	    pfp_context->result.t.exponent += 1;
	} else {
	    pfp_context->result.t.fraction += (1 << pfp_attr->fraction_fill);
	} /* if */
    } /* if */
} /* sub_smallest_denorm */

unsigned long
ieee_default_value(
	alpha_fp_trap_context		*pfp_context)
{

    unsigned long			op1_is_nan;
    unsigned long			op1_is_signaling_nan;
    unsigned long			op1_is_infinity;
    unsigned long			op1_is_zero;
    unsigned long			op1_is_denorm;
    unsigned long			op1_scale;

    unsigned long			op2_is_nan;
    unsigned long			op2_is_signaling_nan;
    unsigned long			op2_is_infinity;
    unsigned long			op2_is_zero;
    unsigned long			op2_is_denorm;
    unsigned long			op2_scale;

    unsigned long			op1_eq_op2;

    struct fp_attr			*pfp_attr;
    unsigned long			sign;
    unsigned long			other_sign;
    unsigned long			renorm_scale;
    unsigned long			exponent_scale;
    unsigned long			exponent_value;
    unsigned long			actual_subtract;
    long				scale_value;
    unsigned long			fraction_size_exponent;
    long				expected_exponent;
    long				exponent_bias;
    long				mul_div; /* 1/-1 if mul/div for exp */

    fp_register_t			operand1 = pfp_context->original_operand1;
    fp_register_t			operand2 = pfp_context->original_operand2;
    alpha_fp_trap_context		fp_context;	/* for EV4 waiver */
    unsigned long			rounding = pfp_context->rounding;
    float_entry				*pfloat_entry = 
						pfp_context->pfloat_entry;
    excsum_t				exception_summary = 
						pfp_context->isolated_excsum;


    /* this routine returns the default IEEE value given the alpha
     *	exception information.
     *
     * the operand fields of fp_context is transient
     *	All cases not needing to modify a copy of operand should use
     *	the locals which are set to the original operands.
     */

    /* often used local values */
    pfp_attr = alpha_fp_attr_lookup(pfloat_entry->operands_format);
    if (pfp_attr)
	exponent_bias = pfp_attr->exponent_bias;

    if (exception_summary.fields.invalid_operation) {

#undef MODULE_NAME
#define MODULE_NAME invalid_operation
	pfp_context->isolated_excsum.fields.invalid_operation = 0;
	pfp_context->target_exception = EXCSUM_INV;

	/* all non-finites come here on alpha */

	/* as we need to compute certain relationships we will save
	 *	the results of those relationships for future use
	 */

	/* only set bit when it is a legitimate invalid */
#define SET_INVALID pfp_context->fp_control.fields.invalid_operation = 1;

	/* ieee invalid operation items which result in NaNs */

	op1_is_nan = (operand1.t.exponent == IEEE_NAN_EXPONENT && 
		operand1.t.fraction != 0);
	op2_is_nan = (operand2.t.exponent == IEEE_NAN_EXPONENT && 
		operand2.t.fraction != 0);

	/* rule 1: any operation on a signaling NaN */
	/* rule 7: conversion of fp to integer or decimal format
	 *  - decimal implemented in library
	 *  - integer overflow comes in as integer overflow exception and
	 *	not invalid op
	 *  - infinities handled with rule 2
	 */
	/* rule 8: cmp < or > return false, unordered true, = gets compared */
	/* handle rest of nans too */
	if (op2_is_nan || op1_is_nan) {


	    op1_is_signaling_nan = (op1_is_nan && 
		(!(operand1.t.fraction&IEEE_QUIET_NAN_MASK)));
	    op2_is_signaling_nan = (op2_is_nan && 
		(!(operand2.t.fraction&IEEE_QUIET_NAN_MASK)));
	    
	    if (op1_is_signaling_nan || op2_is_signaling_nan) {

		SET_INVALID;
		DPRINTF(("found signaling nan\n"));

	    } /* if */

	    switch (pfloat_entry->root_function_code) {

	    case flti_cmptun:
		DPRINTF(("setting result to true\n"));
		pfp_context->result.qval = IEEE_FP_TRUE;
		return SFP_SUCCESS;

	    case flti_cmpteq:
		pfp_context->result.qval = IEEE_FP_FALSE;
		DPRINTF(("setting result to false\n"));
		return SFP_SUCCESS;

	    case flti_cmptle:
	    case flti_cmptlt:
		SET_INVALID;
		pfp_context->result.qval = IEEE_FP_FALSE;
		DPRINTF(("setting result to false\n"));
		return SFP_SUCCESS;

	    case flti_cvttq:
		SET_INVALID;
		pfp_context->result.qval = IEEE_PLUS_LARGEST_QUAD_INTEGER;
		DPRINTF(("setting result to largest integer\n"));
		return SFP_SUCCESS;

	    default:
		if (op1_is_signaling_nan) {
		    pfp_context->result.qval = (op2_is_nan && 
			!op2_is_signaling_nan ? operand2.qval :
			IEEE_PLUS_QUIET_NAN);
		} else if (op2_is_signaling_nan) {
		    pfp_context->result.qval = (op1_is_nan && 
			!op1_is_signaling_nan ? operand1.qval :
			IEEE_PLUS_QUIET_NAN);
                /* op1 and op2 are quiet NaN, return op2 */
                } else if (op1_is_nan && !op1_is_signaling_nan
                        && op2_is_nan && !op2_is_signaling_nan){
		    pfp_context->result.qval = operand2.qval;
		} else {
		    /* check what I'm really doing */
		    pfp_context->result.qval = (op1_is_nan ? operand1.qval : 
			operand2.qval);
		} /* if */
		DPRINTF(("setting result to appropriate nan\n"));
		return SFP_SUCCESS;
	    } /* switch */


	} /* if */
	
	op1_is_infinity = (operand1.qval == IEEE_PLUS_INFINITY ||
	      operand1.qval == IEEE_MINUS_INFINITY);
	op2_is_infinity = (operand2.qval == IEEE_PLUS_INFINITY ||
	      operand2.qval == IEEE_MINUS_INFINITY);
	op1_eq_op2 = (operand1.qval == operand2.qval);
	op1_is_zero = (operand1.qval == IEEE_PLUS_ZERO ||
	      operand1.qval == IEEE_MINUS_ZERO);
	op2_is_zero = (operand2.qval == IEEE_PLUS_ZERO ||
	      operand2.qval == IEEE_MINUS_ZERO);

	/* rule 2: magnitude subtraction of infinities */
	/* rule 3: multiplication of 0 by infinity */
	/* rule 4: infinity/infinity */
	/* handle the rest of the infinities too */
	if (op1_is_infinity || op2_is_infinity) {

	    switch (pfloat_entry->root_function_code) {
	    case flti_adds:
	    case flti_addt:
		if (op1_is_infinity && op2_is_infinity && !op1_eq_op2) {

		    SET_INVALID;
		    pfp_context->result.qval = IEEE_PLUS_QUIET_NAN;
		    DPRINTF(("setting result to quiet nan\n"));
		
		} else {

		    pfp_context->result.qval=(op1_is_infinity?operand1.qval:operand2.qval);
		    DPRINTF(("setting result to operand%s\n", op1_is_infinity ? "1" : "2"));


		} /* if */
		return SFP_SUCCESS;

	    case flti_subs:
	    case flti_subt:
		if (op1_is_infinity && op2_is_infinity && op1_eq_op2) {

		    SET_INVALID;
		    pfp_context->result.qval = IEEE_PLUS_QUIET_NAN;
		    DPRINTF(("setting result to quiet nan\n"));

		} else {

		    pfp_context->result.qval=(op1_is_infinity ? operand1.qval:
			(operand2.qval ^ IEEE_SIGN_BIT_MASK));
		    DPRINTF(("setting result to %s\n", op1_is_infinity ? "operand1" : "- operand2"));

		} /* if */
		return SFP_SUCCESS;

	    case flti_muls:
	    case flti_mult:
		if ((op1_is_zero && op2_is_infinity) ||
		    (op1_is_infinity && op2_is_zero)) {

		    SET_INVALID;
		    pfp_context->result.qval = IEEE_PLUS_QUIET_NAN;
		    DPRINTF(("setting result to quiet nan\n"));

		} else {

		    pfp_context->result.qval = IEEE_PLUS_INFINITY;
		    DPRINTF(("setting result to sign appropriate infinity\n"));
		    pfp_context->result.t.sign = operand1.t.sign ^ operand2.t.sign;

		} /* if */
		return SFP_SUCCESS;

	    case flti_divs:
	    case flti_divt:
		if (op1_is_infinity) {
		    if (op2_is_infinity) {

			/* rule 4 */
			SET_INVALID;
			pfp_context->result.qval = IEEE_PLUS_QUIET_NAN;
			DPRINTF(("setting result to quiet nan\n"));

		    } else {

			/* infinity / finite == infinity */
			pfp_context->result.qval = IEEE_PLUS_INFINITY;
			DPRINTF(("setting result to sign appropriate infinity\n"));
			pfp_context->result.t.sign = 
				operand1.t.sign ^ operand2.t.sign;
		    } /* if */

		} else {

		    /* finite / infinity == close to zero */
		    pfp_context->result.qval = IEEE_PLUS_ZERO;
		    DPRINTF(("setting result to sign appropriate zero\n"));
		    pfp_context->result.t.sign = 
			    operand1.t.sign ^ operand2.t.sign;

		} /* if */
		return SFP_SUCCESS;

	    case flti_cvttq:
		SET_INVALID;
		if (operand2.t.sign)
		    pfp_context->result.qval = IEEE_MINUS_LARGEST_QUAD_INTEGER;
		else
		    pfp_context->result.qval = IEEE_PLUS_LARGEST_QUAD_INTEGER;
		DPRINTF(("setting result to largest integer\n"));
		return SFP_SUCCESS;

	    case flti_cvtts:
#if INF_CVTTS
		DPRINTF(("setting to infinity\n"));
		/* depends on rounding, see spec */
		sign = pfp_context->operand2.t.sign;
		pfp_attr = alpha_fp_attr_lookup(pfloat_entry->result_format);
		switch (pfp_context->rounding) {
		case IEEE_ROUND_TO_ZERO:
		    pfp_context->result.qval = pfp_attr->largest_number;
		    pfp_context->result.t.sign = sign;
		    break;
		case IEEE_ROUND_TO_NEAREST:
		    pfp_context->result.qval = IEEE_PLUS_INFINITY;
		    pfp_context->result.t.sign = sign;
		    break;
		case IEEE_ROUND_TO_PLUS_INFINITY:
		    pfp_context->result.qval = (sign == IEEE_PLUS_SIGN) ?
			    IEEE_PLUS_INFINITY : 
			    (pfp_attr->largest_number|IEEE_MINUS_ZERO);
		    break;
		case IEEE_ROUND_TO_MINUS_INFINITY:
		    pfp_context->result.qval = (sign != IEEE_PLUS_SIGN) ?
			    IEEE_MINUS_INFINITY : pfp_attr->largest_number;
		    break;
		} /* switch */
		return SFP_SUCCESS;
#endif

#ifdef flti_cvtst
	    case flti_cvtst:
#endif
		pfp_context->result.qval = operand2.qval;
		/* bottom bits are clear already for cvtts */
		DPRINTF(("setting result to operand\n"));
		return SFP_SUCCESS;

	    case flti_cmptle:
		if (op1_eq_op2) {
		    pfp_context->result.qval = IEEE_FP_TRUE;
		    break;
		} /* if */
		/* Fall Through */

	    case flti_cmptlt:
		/* if not eq, all numbers are greater than minus infinity
		 *	and less than plus infinity-- so we check the
		 *	sign.
		 */
		if (op1_eq_op2) {

		    pfp_context->result.qval = IEEE_FP_FALSE;

		} else if (op1_is_infinity) {

		    pfp_context->result.qval = 
			((operand1.t.sign == IEEE_MINUS_SIGN) ?
			IEEE_FP_TRUE : IEEE_FP_FALSE);

		} else {

		    pfp_context->result.qval = 
			((operand2.t.sign == IEEE_PLUS_SIGN) ?
			IEEE_FP_TRUE : IEEE_FP_FALSE);

		} /* if */
		break;


	    case flti_cmpteq:
		if (op1_eq_op2) {
		    pfp_context->result.qval = IEEE_FP_TRUE;
		} else {
		    pfp_context->result.qval = IEEE_FP_FALSE;
		} /* if */
		break;

	    case flti_cmptun:
		if (!op1_eq_op2) {
		    pfp_context->result.qval = IEEE_FP_TRUE;
		} else {
		    pfp_context->result.qval = IEEE_FP_FALSE;
		} /* if */
		break;

	    default:
		FP_EPRINTF("unexpected opcode in infinity operation");
		return SFP_FAILURE;

	    } /* switch */

	    DPRINTF(("setting result to %s\n", (pfp_context->result.qval == IEEE_FP_TRUE) ? "true" : "false"));
	    return SFP_SUCCESS;

	} /* if */


	/* rule 4: division of 0/0 */
	if ((pfloat_entry->root_function_code == flti_divs ||
	     pfloat_entry->root_function_code == flti_divt) &&
	    ((op1_is_zero && op2_is_zero))) {

	    SET_INVALID;
	    pfp_context->result.qval = IEEE_PLUS_QUIET_NAN;
	    return SFP_SUCCESS;

	} /* if */

	/* rule 5: x rem y where y is zero or x is infinite */
	/* implemented in a library */

	/* rule 6: sqrt if the operand is less than 0 */
	/* implemented in a library */

	/* if we got here, one of our ops is a denorm */
	op1_is_denorm = (operand1.t.exponent == IEEE_DENORM_EXPONENT && operand1.t.fraction != 0);
	op2_is_denorm = (operand2.t.exponent == IEEE_DENORM_EXPONENT && operand2.t.fraction != 0);
	/* set up copies to modify */
	pfp_context->operand1.qval = operand1.qval;
	pfp_context->operand2.qval = operand2.qval;

	switch (pfloat_entry->root_function_code) {

	case flti_subs:
	case flti_subt:
	case flti_adds:
	case flti_addt:
	    /*
	     * cases
	     * op1/op2	denorm		small		medium		large
	     * ----	-------------	-----------	----------	-----
	     * denorm	scale+op	scale+op	op		large
	     */

	    fraction_size_exponent = 2 +	/* slop */
		EXPONENT_REGISTER_FORMAT(pfp_attr->fraction_size, 
		exponent_bias);

	    /* need to avoid overflow */

	    if (op1_is_denorm != op2_is_denorm) {

		if (!op2_is_denorm && 
		    operand2.t.exponent > fraction_size_exponent) {

		    /* number so large, denorm has no affect */
		    pfp_context->isolated_excsum.fields.inexact_result = 1;
		    pfp_context->fpcr.fields.inexact_result = 1;
		    pfp_context->result.qval = pfp_context->operand2.qval;

		    if (pfloat_entry->root_function_code == flti_subs ||
		        pfloat_entry->root_function_code == flti_subt) {
			/* subtract requires sign fixup */
			pfp_context->result.t.sign= ~pfp_context->result.t.sign;
		    } /* if */

		    other_sign = operand1.t.sign;


		    DPRINTF(("scaling result to get round(denorm has little effect)\n"));


		} else if (!op1_is_denorm && 
		    operand1.t.exponent > fraction_size_exponent) {

		    /* number so large, denorm has no affect */

		    pfp_context->isolated_excsum.fields.inexact_result = 1;
		    pfp_context->fpcr.fields.inexact_result = 1;
		    pfp_context->result.qval = pfp_context->operand1.qval;

		    if (pfloat_entry->root_function_code == flti_subs ||
		        pfloat_entry->root_function_code == flti_subt) {
			/* subtract requires sign fixup */
			operand2.t.sign = ~operand2.t.sign;
		    } /* if */
		    other_sign = operand2.t.sign;

		} else {
		    goto do_underflow_code;

		} /* if */


		DPRINTF(("denorm has little effect\n"));
		switch (pfp_context->rounding) {
		case IEEE_ROUND_TO_PLUS_INFINITY:
		    if (other_sign == IEEE_PLUS_SIGN) {
		       add_smallest_denorm(pfp_context,pfp_attr);
		    } /* if */
		    break;
		case IEEE_ROUND_TO_MINUS_INFINITY:
		    if (other_sign != IEEE_PLUS_SIGN) {
		       sub_smallest_denorm(pfp_context,pfp_attr);
		    } /* if */
		    break;
		case IEEE_ROUND_TO_ZERO:
		    if (pfp_context->result.t.sign != IEEE_PLUS_SIGN &&
			other_sign == IEEE_PLUS_SIGN) {
		       add_smallest_denorm(pfp_context,pfp_attr);
		    } else if (pfp_context->result.t.sign == IEEE_PLUS_SIGN &&
			other_sign != IEEE_PLUS_SIGN) {
		       sub_smallest_denorm(pfp_context,pfp_attr);
		    } /* if */
		    break;
		} /* case */
		return SFP_SUCCESS;


	    } /* if */
do_underflow_code:

	    op1_scale = ieee_renorm(&pfp_context->operand1, exponent_bias);
	    op2_scale = ieee_renorm(&pfp_context->operand2, exponent_bias);

	    DPRINTF(("add/sub denorm op1_scale = %ld, op2_scale =%ld\n", op1_scale, op2_scale));
	    if (op1_scale <= op2_scale) {
		renorm_scale = op2_scale;
		if (!op1_is_zero) {
		    pfp_context->operand1.t.exponent += op2_scale - op1_scale;
		} /* if */
	    } else {
		renorm_scale = op1_scale;
		if (!op2_is_zero) {
		    pfp_context->operand2.t.exponent += op1_scale - op2_scale;
		} /* if */
	    } /* if */
	    goto underflow;

	case flti_divs:
	case flti_divt:
	    /* cases
	     *	x/y
	     *
	     * 1) x is denorm and y is finite
	     * 2) x is finit and y is denorm
	     * 3) x is denorm and y is denorm
	     * 4) x is small and y is large
	     * 5) y is zero
	     *
	     * 1 and 3 are easy we just renorm, do the op and scale back
	     * 2 is harder since we might get overflow and we do the same thing
	     *	except note the overflow.
	     * 1 and 4 require us to reduce the difference between the
	     *	operands so we don't get underflow and the underflow code
	     *	will take care of that.
	     */
	    if (op2_is_zero) {
		
		goto divide_by_0;

	    } /* if */
	    /* Fall Through */

	case flti_muls:
	case flti_mult:
	    op1_scale = ieee_renorm(&pfp_context->operand1, exponent_bias);
	    op2_scale = ieee_renorm(&pfp_context->operand2, exponent_bias);
	    /* check for overflow done in do_op_and_rescale */
	    goto underflow;

	case flti_cvttq:
	    /* conversion of denormal number depends on rounding mode */
	    pfp_context->fp_control.fields.inexact_result = 1;
	    switch (pfp_context->rounding) {
	    case IEEE_ROUND_TO_PLUS_INFINITY:
		pfp_context->result.qval = (pfp_context->operand2.t.sign == IEEE_MINUS_SIGN) ? 0 : 1;
		break;
	    case IEEE_ROUND_TO_MINUS_INFINITY:
		pfp_context->result.qval = (pfp_context->operand2.t.sign == IEEE_MINUS_SIGN) ? -1 : 0;
		break;
	    case IEEE_ROUND_TO_ZERO:
	    case IEEE_ROUND_TO_NEAREST:
		pfp_context->result.qval = 0;
		break;
	    } /* case */

	    return SFP_SUCCESS;

	case flti_cvtts:
	    /* underflow will check for inexact and for underflow */
	    op2_scale = ieee_renorm(&pfp_context->operand2, exponent_bias);
	    goto underflow;

#ifdef flti_cvtst
	case flti_cvtst:
#endif
	    /* renorm op as S format and scale back down as T,
	     *	which will leave the result finite
	     */
	    pfp_context->result.qval = operand2.qval;
	    DPRINTF(("setting result to operand\n"));

	    op2_scale = ieee_renorm(&pfp_context->result, exponent_bias);

	    pfp_attr = alpha_fp_attr_lookup(FORMAT_IEEE_T); /* only result */
	    /* scale needs inexact result to be correct. */
	    pfp_context->fpcr.fields.inexact_result = 0;
	    return ieee_t_scale(&pfp_context->result, -(long)op2_scale, 
		pfp_context, pfp_attr);

        case flti_cmptle:
            if (op1_eq_op2) {
                pfp_context->result.qval = IEEE_FP_TRUE;
		DPRINTF(("setting result to true\n"));
                return SFP_SUCCESS;
            } /* if */
	    /* Fall Through */

        case flti_cmptlt:

	    /*
	     * op1/op2	+finite	-finite	+0	-0	+denorm	-denorm
	     * -----------------------------------------------------------
	     * +finite	X	X	X	X	F	F
	     * -finite	X	X	X	X	T	T
	     * +0	X	X	X	X	T	F
	     * -0	X	X	X	X	T	F
	     * +denorm	T	F	F	F	C	F
	     * -denorm	T	F	T	T	T	C
	     *
	     * where:
	     *	X	don't care one op must be denorm to get here
	     *	T	true
	     *	F	false
	     *	C	sign based compare of fraction
	     *
	     */
	    if (op1_is_zero) {

		pfp_context->result.qval = (operand2.t.sign == IEEE_PLUS_SIGN) ?
			IEEE_FP_TRUE : IEEE_FP_FALSE;

	    } else if (op2_is_zero) {

		pfp_context->result.qval = (operand1.t.sign == IEEE_MINUS_SIGN)?
			IEEE_FP_TRUE : IEEE_FP_FALSE;

	    } else if (operand1.t.sign != operand2.t.sign) {

		pfp_context->result.qval = (operand1.t.sign == IEEE_PLUS_SIGN) ?
			IEEE_FP_FALSE : IEEE_FP_TRUE;

            } else if (op1_is_denorm) {

                if (op2_is_denorm) {

		    if (op1_eq_op2) {

			pfp_context->result.qval = IEEE_FP_FALSE;

                    } else if (operand1.t.fraction < operand2.t.fraction) {

			pfp_context->result.qval = 
			    (operand1.t.sign == IEEE_PLUS_SIGN) ? 
			    IEEE_FP_TRUE : IEEE_FP_FALSE;

                    } else {

			pfp_context->result.qval = 
			    (operand1.t.sign == IEEE_MINUS_SIGN) ? 
			    IEEE_FP_TRUE : IEEE_FP_FALSE;

                    } /* if */

		} else {

		    pfp_context->result.qval = 
			(operand2.t.sign == IEEE_PLUS_SIGN) ? 
			IEEE_FP_TRUE : IEEE_FP_FALSE;

                } /* if */

	    } else {

		/* op1 isn't denorm but op2 is */
		pfp_context->result.qval = (operand1.t.sign == IEEE_MINUS_SIGN) 
			? IEEE_FP_TRUE : IEEE_FP_FALSE;

            } /* if */
	    DPRINTF(("setting result to %s\n", (pfp_context->result.qval == IEEE_FP_TRUE) ? "true" : "false"));
	    return SFP_SUCCESS;


	case flti_cmptun:
	    pfp_context->result.qval = op1_eq_op2 ? 
		IEEE_FP_FALSE : IEEE_FP_TRUE;
	    DPRINTF(("setting result to %s\n", op1_eq_op2 ? "false" : "true"));
	    return SFP_SUCCESS;

	case flti_cmpteq:
	    pfp_context->result.qval = op1_eq_op2 ? 
		IEEE_FP_TRUE : IEEE_FP_FALSE;
	    DPRINTF(("setting result to %s\n", op1_eq_op2 ? "true" : "false"));
	    return SFP_SUCCESS;

	default:
	    FP_EPRINTF("unexpected opcode in invalid operation code");
	    return SFP_FAILURE;

	} /* switch */

    } else if (exception_summary.fields.floating_overflow) {
	
#undef MODULE_NAME
#define MODULE_NAME floating_overflow
	pfp_context->target_exception = EXCSUM_OVF;
	pfp_context->isolated_excsum.fields.floating_overflow = 0;
	pfp_context->fp_control.fields.floating_overflow = 1;
	/* as per spec need to set inexact on non-trapping overflow */
	pfp_context->isolated_excsum.fields.inexact_result = 1;

	/* compute sign of intermediate result knowing we had an overflow */
	switch (pfloat_entry->root_function_code) {
	case flti_adds:
	case flti_addt:
	    /* possible combos: (plus + plus) or (minus + minus) */
	    sign = operand1.t.sign; /* which has to == operand2.sign */
	    break;

	case flti_subs:
	case flti_subt:
	    /* possible combos: (plus - minus) or (minus - plus) */
	    sign = operand1.t.sign; /* which has to != operand2.sign */
	    break;

	case flti_muls:
	case flti_mult:
	case flti_divs:
	case flti_divt:
	    /* any combos */
	    sign = operand1.t.sign ^ operand2.t.sign;
	    break;

	case flti_cvtts:
#ifdef flti_cvtst
	case flti_cvtst:
#endif
	    /* just take sign of what we are converting (Fb) */
	    sign = operand2.t.sign;
	    pfp_attr = alpha_fp_attr_lookup(pfloat_entry->result_format);
	    break;

	default:
	    FP_EPRINTF("unexpected opcode in floating overflow");
	    return SFP_FAILURE;
	} /* switch */


	/* as per spec set result based on rounding mode */
	switch (rounding) {

	case IEEE_ROUND_TO_NEAREST:
		pfp_context->result.qval = IEEE_PLUS_INFINITY;
		pfp_context->result.t.sign = sign;
		DPRINTF(("setting result to sign appropriate infinity\n"));
		break;

	case IEEE_ROUND_TO_ZERO:
		pfp_context->result.qval = pfp_attr->largest_number;
		pfp_context->result.t.sign = sign;
		DPRINTF(("setting result to sign appropriate largest number\n"));
		break;

	case IEEE_ROUND_TO_MINUS_INFINITY:
		pfp_context->result.qval = (sign == IEEE_PLUS_SIGN ? 
			pfp_attr->largest_number : IEEE_MINUS_INFINITY);
		DPRINTF(((sign == IEEE_PLUS_SIGN) ? "setting result to plus largest number" : "setting result to minus infinity"));
		break;

	case IEEE_ROUND_TO_PLUS_INFINITY:
		pfp_context->result.qval = 
		    (sign == IEEE_PLUS_SIGN ? IEEE_PLUS_INFINITY :
		    pfp_attr->largest_number);
		pfp_context->result.t.sign = sign;
		DPRINTF(((sign == IEEE_PLUS_SIGN) ? "setting result to plus infinity" : "setting result to minus largest number"));
		break;

	default:
	    FP_EPRINTF("unexpected rounding mode in overflow code");
	    return SFP_FAILURE;

	} /* switch */

	return SFP_SUCCESS;

    } else if (exception_summary.fields.divide_by_zero) {
	
#undef MODULE_NAME
#define MODULE_NAME divide_by_zero

divide_by_0:
	pfp_context->target_exception = EXCSUM_DZE;
	if (pfloat_entry->root_function_code != flti_divt &&
	    pfloat_entry->root_function_code != flti_divs) {
	    FP_EPRINTF("unexpected opcode in divide by zero code");
	    return SFP_FAILURE;
	} /* if */
	pfp_context->isolated_excsum.fields.divide_by_zero = 0;
	pfp_context->fp_control.fields.divide_by_zero = 1;

	pfp_context->result.qval = IEEE_PLUS_INFINITY;
	pfp_context->result.t.sign = operand1.t.sign ^ operand2.t.sign;
	DPRINTF(("setting result to sign appropriate infinity\n"));
	return SFP_SUCCESS;

    } else if (exception_summary.fields.floating_underflow) {

#undef MODULE_NAME
#define MODULE_NAME floating_underflow

	pfp_context->isolated_excsum.fields.floating_underflow = 0;

	if (pfp_context->target_exception != 0) {

	    /* we had an another trap who gave us the right answer but had to
	     *	raise underflow (probably going to 0) since we might have
	     *	had to trap the to the user. Since we're here we know we 
	     *	didn't have to trap to the user and we have
	     *	the right answer just return, the previous exception should have
	     *	set the sticky bit.
	     */
	    FP_EPRINTF("UNEXPECTED 2nd execution calls underflow");
	    return SFP_FAILURE;

	} /* if */
	pfp_context->target_exception = EXCSUM_UNF;

	/* set fp_control sticky bits in the inexact code as per spec */

	/* only case is two finites causing a denorm:
	 *
	 *	add	two small numbers with different signs
	 *	sub	two small numbers with the same sign
	 *	mul	two small numbers
	 *	div	big divisor
	 *
	 *	plan
	 *	add	scale exp up on both operands, operate and scale down
	 *	sub	scale exp up on both operands, operate and scale down
	 *	mul	scale exp up on first operand, operate and scale down
	 *	div	scale exp up on first operand, operate chopped and
	 *			scale down caclulating botom bit.
	 */

	/* set up copies to modify */
	pfp_context->operand1.qval = operand1.qval;
	pfp_context->operand2.qval = operand2.qval;

	op1_is_zero = (operand1.qval == IEEE_PLUS_ZERO ||
	      operand1.qval == IEEE_MINUS_ZERO);
	op2_is_zero = (operand2.qval == IEEE_PLUS_ZERO ||
	      operand2.qval == IEEE_MINUS_ZERO);

	/* set by invalid op for denorm input */
	renorm_scale = 0;
	op1_scale = 0;
	op2_scale = 0;

	/* denorm inputs do some scaling and jump here to share code */
underflow:

	mul_div = 1;
	switch (pfloat_entry->root_function_code) {

	case flti_adds:
	case flti_subs:
	case flti_addt:
	case flti_subt:
	    /* just scale up to the bias (taking into account S vs. T format)
	     *	and do the op.
	     */
	    exponent_value = BIASED_POWER_OF_2(MAX(
		pfp_context->operand1.t.exponent, 
		pfp_context->operand2.t.exponent), exponent_bias);

	    DPRINTF(("add/sub exponent = %ld\n", exponent_value));

	    if (exponent_value < exponent_bias) {
		scale_value = ((exponent_bias-exponent_value) + 6);
		exponent_scale = scale_value;
		DPRINTF(("add/sub exponent scale = %ld\n", exponent_scale));
		if (!op1_is_zero) {
		    pfp_context->operand1.t.exponent += exponent_scale;
		} /* if */
		if (!op2_is_zero) {
		    pfp_context->operand2.t.exponent += exponent_scale;
		} /* if */
	    } else {
		/* no need to scale and it might screw up format */
		scale_value = 0;
	    } /* if */
	    return do_op_and_rescale(pfp_context, renorm_scale+scale_value,
		pfp_attr, pfloat_entry->rounding);


	case flti_divs:
	case flti_divt:
	    mul_div = -1;
	    /* EV4 waiver issue, we need to set inexact here, inexact code
	     *	will determine if it is accurate.
	     */
	    pfp_context->isolated_excsum.fields.inexact_result = 1;
	    /* Fall through */

	case flti_muls:
	case flti_mult:
		/* the plan here is to scale operand 1 up so we neither
		 *	overflow nor underflow
		 *	and do the op in Chopped mode. once the op
		 *	is done we scale the result back by the amount
		 *	we scaled it up. When we do so we have to
		 *	round by hand and calculate the inexact bit
		 *
		 * The only way to get underflow from two finit numbers is
		 *	for their exponents to add/subtract to less than
		 *	-bias for each format. This means that the first
		 *	operand exponent must be between 0 & -bias (note
		 *	that the second operand can be > 0 for divide).
		 *
		 * We also share the code with code which handles denorm
		 *	inputs, see that code for comments.
		 */
	    expected_exponent = (long)(pfp_context->operand1.t.exponent - 
					IEEE_T_EXPONENT_BIAS) + 
		(mul_div * (long)(pfp_context->operand2.t.exponent - 
					IEEE_T_EXPONENT_BIAS));
	    
	    /* will it cause a denorm? calculate by how much  */
	    if (expected_exponent <= (-exponent_bias) + 1) {

		scale_value = 4L-((long)exponent_bias+(long)expected_exponent);
		DPRINTF(("scale_value=%ld\n", scale_value));
		exponent_scale = scale_value;
		rounding = C;	/* hopped,we will calc by hand */
	    } else {
		scale_value = exponent_scale = 0;
	    } /* if */

	    DPRINTF(("expected=%ld, scale=%ld, op1_scale=%ld, op2_scale=%ld\n", expected_exponent, exponent_scale, op1_scale, op2_scale));
	    if (!op1_is_zero) {
		pfp_context->operand1.t.exponent += exponent_scale;
	    } /* if */

	    /* make the call calculating the effect a previous renorm may have
	     *	had.
	     */
	    return do_op_and_rescale(pfp_context, (scale_value+op1_scale) + 
		((mul_div) * op2_scale), pfp_attr, pfloat_entry->rounding);

	case flti_cvtts:
	    /* we use the result attributes here, scale the exponent like
	     *	a T but modify the scale as if it were an S so do_op_and_rescale
	     *	can do it's job.
	     * we assume we cannot get a zero operand.
	     */
	    if (operand2.t.exponent < IEEE_T_EXPONENT_BIAS)
		scale_value = exponent_scale = 
		    ((IEEE_T_EXPONENT_BIAS-pfp_context->operand2.t.exponent)+6);
	    else
		exponent_scale = 0;
	    pfp_context->operand2.t.exponent += exponent_scale;
	    pfp_attr = alpha_fp_attr_lookup(FORMAT_IEEE_S); /* only result */
	    /* just assign and scale */
	    DPRINTF(("scale=%ld, op2_scale=%ld\n", exponent_scale, op2_scale));
	    return do_op_and_rescale(pfp_context, scale_value+op2_scale,
		pfp_attr, C);

	default:
	    FP_EPRINTF("unexpected opcode in underflow code");
	    return SFP_FAILURE;
	} /* switch */

    } else if (exception_summary.fields.integer_overflow) {
	
#undef MODULE_NAME
#define MODULE_NAME integer_overflow
	pfp_context->target_exception = EXCSUM_IOV;
	pfp_context->isolated_excsum.fields.integer_overflow = 0;
	SET_INVALID;

	/* this is where conversion overflows are delivered (rule 7) */

	if ((pfloat_entry->root_function_code == flti_cvttq) 
	    || (pfloat_entry->root_function_code == fltl_cvtql)) {
          
            /* illegal operation can not be inexact also */
            DPRINTF(("setting inexact to 0 in isolated_excsum\n"));
            pfp_context->isolated_excsum.fields.ine = 0;

	} else {

	    FP_EPRINTF("unexpected opcode in integer overflow code");
	    return SFP_FAILURE;

	} /* if */

	return SFP_SUCCESS;

    } else if (exception_summary.fields.inexact_result) {

#undef MODULE_NAME
#define MODULE_NAME inexact_result

	pfp_context->target_exception = EXCSUM_INE;
	pfp_context->isolated_excsum.fields.inexact_result = 0;
	if (pfp_context->fp_control.fields.inexact_result == 0) {

	    if ((pfloat_entry->exceptions&I) == 0 ||
		(pfloat_entry->exceptions&E) == 0) {

		/* didn't set inexact bit in instruction, just pass
		 *	on hardware information. EV4 is broken
		 *	here divide and may not always have the correct result.
		 * or did set inexact but not a divide waiver item.
		 */
		DPRINTF(("setting inexact bit\n"));
		pfp_context->fp_control.fields.inexact_result = 1;

	    } else {

		/* EV4 divide Waiver. we must remultiply the result
		 *	by the original 2nd operand. We'll do it
		 *	the safe way by using the underflow code.
		 */
		if (pfp_context->result.t.exponent == IEEE_ZERO_EXPONENT &&
		    pfp_context->result.t.fraction == 
		    IEEE_ZERO_FRACTION) {

		    /* do 0 by hand */
		    if (pfp_context->original_operand1.t.exponent != 
			IEEE_ZERO_EXPONENT ||
			pfp_context->original_operand1.t.fraction != 
			IEEE_ZERO_FRACTION) {

			pfp_context->fp_control.fields.inexact_result = 1;

		    } /* if */

		    return SFP_SUCCESS;

		} /* if */

		/* set up fp_context for multiply */
#ifndef KERNEL
		memset(&fp_context, 0, sizeof(fp_context));
#else
		bzero(&fp_context, sizeof(fp_context));
#endif
		fp_context.operand1 = pfp_context->result;
		fp_context.operand2 = pfp_context->original_operand2;
		fp_context.pfloat_entry = alpha_float_entry_lookup(op_flti,
		    (pfloat_entry->root_function_code == flti_divt) ?
		    flti_mult : flti_muls, pfloat_entry->rounding, S|U);

		/* in case either op isn't normalized */
		op1_scale = ieee_renorm(&fp_context.operand1, exponent_bias);
		op2_scale = ieee_renorm(&fp_context.operand2, exponent_bias);
		op1_is_denorm = (operand1.t.exponent == IEEE_DENORM_EXPONENT &&
			operand1.t.fraction != 0);

		expected_exponent = (long)(fp_context.operand1.t.exponent - 
					    IEEE_T_EXPONENT_BIAS) + 
		    ( (long)(fp_context.operand2.t.exponent - 
					    IEEE_T_EXPONENT_BIAS));
		
		/* will it cause a denorm? calculate by how much + 4 */
		if ((op1_scale != 0 || op2_scale != 0 || op1_is_denorm) &&
		    expected_exponent <= (-exponent_bias) + 1) {
		    scale_value = 4L-((long)exponent_bias+(long)expected_exponent);
		    DPRINTF(("scale_value=%ld\n", scale_value));
		    exponent_scale = scale_value;
		    rounding = C;	/* hopped,we will calc by hand */
		} else
		    scale_value = exponent_scale = 0;


		DPRINTF(("expected=%ld, scale=%ld, op1_scale=%ld, op2_scale=%ld\n", expected_exponent, exponent_scale, op1_scale, op2_scale));

		fp_context.operand1.t.exponent += exponent_scale;

		/* make the call calculating the effect a previous renorm may
		 *	have had.
		 */
		do_op_and_rescale(&fp_context, (scale_value+op1_scale) + 
		    op2_scale, pfp_attr, pfloat_entry->rounding);

		if (fp_context.fpcr.fields.inexact_result || 
		    fp_context.result.qval != 
			pfp_context->original_operand1.qval) {

		    DPRINTF(("setting waiver inexact bit\n"));
		    pfp_context->fp_control.fields.inexact_result = 1;

		} else {
		    /* don't want to underflow on denorm (next if) so return */
		    return SFP_SUCCESS;
		} /* if */
	    } /* if */
	} /* if */


	if (pfp_context->result.t.exponent == IEEE_DENORM_EXPONENT &&
	    pfp_context->result.t.fraction != 0) {

	    /* as per spec only if inexact is set do we set underflow */
	    DPRINTF(("setting underflow bit\n"));
	    pfp_context->fp_control.fields.floating_underflow = 1;

	} /* if */

	return SFP_SUCCESS;

    } /* if */

    FP_EPRINTF("unexpected fall through");
    return SFP_FAILURE;

} /* ieee_default_value */

double
alpha_sw_cvtst(double f)
{
    long		op2_scale;
    fp_register_t	operand;

    /* renorm op as S format and scale back down as T,
     *	which will leave the result finite
     */
    operand.dval = f;
    op2_scale = ieee_renorm(&operand, IEEE_S_EXPONENT_BIAS);
    if (op2_scale == 0)
	return f;

    /* scale needs inexact result to be correct. */
    ieee_t_scale(&operand, -(long)op2_scale, 0, 0);
    return operand.dval;
} /* alpha_sw_cvtst */
