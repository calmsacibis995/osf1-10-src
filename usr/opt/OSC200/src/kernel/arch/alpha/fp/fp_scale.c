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
static char *rcsid = "@(#)$RCSfile: fp_scale.c,v $ $Revision: 1.1.4.2 $ (DEC) $Date: 1993/03/22 22:11:15 $";
#endif

#ifndef KERNEL
#include <stdio.h>
#include <setjmp.h>
#endif /* KERNEL */
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

#define MODULE_NAME	scaling


unsigned long
ieee_renorm(
	fp_register_t	*operand,
	unsigned long	exponent_bias)
{
    /* renormalize an operand and return the scale need to do it */
    /* scale returned accounts for T format
     */
    unsigned long	fraction;
    unsigned long	scale;

    if (operand->t.exponent != 0 || operand->t.fraction == 0) {
	return 0;
    } /* if */

    fraction = operand->t.fraction;

    /* get first bit set */
    for (scale = 0; fraction != 0; (scale++), (fraction <<= 1)) {

	if (fraction&IEEE_T_FRACTION_HIGH_BIT) {

	    /* found implied one bit, shift out and break */
	    fraction <<= 1;
	    DPRINTF(("before mask: fraction=0x%016lx mask=0x%016lx\n", fraction, BITMASK(IEEE_T_FRACTION_SIZE)));
	    fraction = fraction&BITMASK(IEEE_T_FRACTION_SIZE);
	    DPRINTF(("after mask: fraction=0x%016lx\n", fraction));
	    scale++;
	    break;

	} /* if */
    } /* for */

    operand->t.exponent = EXPONENT_REGISTER_FORMAT(1, exponent_bias);
    operand->t.fraction = fraction;

    DPRINTF(("renormalizing operand by scaling up %ld\n", scale));
    return (scale);

} /* renorm */



unsigned long
ieee_t_scale(
	fp_register_t			*operand,
	long				scale_value, 
	alpha_fp_trap_context		*pfp_context,
	struct fp_attr			*pfp_attr)
{
    unsigned long	sign;
    fp_register_t	copy;
    long		exponent_value;
    long		scale;
    long		fraction_shift;
    unsigned long	fraction_fill;
    unsigned long	large_scale_down;	/* large than fraction size */
    unsigned long	bitmask;		/* used in scale down */
    unsigned long	renorm_scale_value;
    unsigned long	fraction;
    unsigned long	fraction_mask;		/* used for rounding overflow */
    unsigned long	original_fraction;
    unsigned long	cumulative;
    unsigned long	inexact;
    unsigned long	last_shifted;
    alpha_fp_trap_context		fp_context;

    /* scale operand up or down depending on sign of scale, update
     *	status registers in pfp_context if supplied and use fraction_fill
     *	to help deal with rounding on scale down to denorms of s format
     *	items (s format may not use this routine for scale up)
     *
     * all scales should account for T register format 
     *	(i.e. S format scales need to EXPONENT_REGISTER_FORMAT).
     *
     * on scales down, the context's fpcr must be accurate as if the operand
     *	was just created, otherwise rounding may not be done right.
     */

    /* initialization */


    /* fill optional parameters to refer to T format */

    if (pfp_context == 0) {
	pfp_context = &fp_context;
	pfp_context->rounding = IEEE_ROUND_TO_NEAREST;
	pfp_context->fpcr.fields.inexact_result = 0;
    }
    if (pfp_attr == 0) {
	pfp_attr = alpha_fp_attr_lookup(FORMAT_IEEE_T);
    } /* if */

    /* defaults for rounding */
    last_shifted = 0;
    cumulative = inexact = pfp_context->fpcr.fields.inexact_result;
    sign = operand->t.sign;

    /* locals used many times */
    fraction_fill = pfp_attr->fraction_fill;
    scale = EXPONENT_REGISTER_FORMAT(scale_value, pfp_attr->exponent_bias);
    exponent_value=BIASED_POWER_OF_2(operand->t.exponent,pfp_attr->exponent_bias);

    /* scale the resulting exponent as far as possible.
     *	We'll take the left over and shift the result by that
     *	amount. For add and subtract this should be at most
     *	a bit (TODO do we need to round?). For multiple it
     *	may be more and we should check if we are loosing
     *	accuracy and set the inexact bit (TODO do we need
     *	to see if the user is trapping this?).
     */
    DPRINTF(("scaling result by %ld\n", scale_value));
    DPRINTF(("exp=%ld(0x%lx) exp_value=%ld(0x%lx)\n", operand->t.exponent, operand->t.exponent, exponent_value, exponent_value));
    DPRINTF(("scale=%ld(0x%lx) scale_value=%ld(0x%lx)\n", scale, scale, scale_value, scale_value));

    switch (operand->qval) {
    case IEEE_PLUS_INFINITY:
    case IEEE_MINUS_INFINITY:
    case IEEE_PLUS_ZERO:
    case IEEE_MINUS_ZERO:
	    DPRINTF(("zeros and infinities skip scaling\n"));
	    return SFP_SUCCESS;
    } /* switch */

    if (operand->t.exponent == IEEE_NAN_EXPONENT) {
	    DPRINTF(("nans skip scaling\n"));
	    return SFP_SUCCESS;
    } /* if */

    if (operand->t.exponent == IEEE_DENORM_EXPONENT) {
	DPRINTF(("scaling denorm\n"));
	if (scale_value > pfp_attr->exponent_bias) {
	    /* scaling up */
	    copy = *operand;
	    renorm_scale_value = ieee_renorm(&copy, pfp_attr->exponent_bias);
	    if (renorm_scale_value > scale_value) {
		    /* just shift fraction up */
		    operand->t.fraction <<= scale_value;
		    fraction = operand->t.fraction;
		    goto round;
	    } else {
		    /* just scale the exponent a bit more and return renorm */
		    copy.t.exponent += EXPONENT_REGISTER_FORMAT(scale-renorm_scale_value, pfp_attr->exponent_bias);
		    *operand = copy;
		    fraction = operand->t.fraction;
		    goto round;
	    }
	} else {
		fraction_shift = -scale_value;
		fraction = operand->t.fraction;
		goto shift_fraction;
	} /* if */
    } /* if */

    /* check for overflow */
    if (scale_value+exponent_value > (long)pfp_attr->exponent_max) {

	DPRINTF(("scaling to infinity\n"));
	/* depends on rounding, see spec */
	switch (pfp_context->rounding) {
	case IEEE_ROUND_TO_ZERO:
	    operand->qval = pfp_attr->largest_number;
	    operand->t.sign = sign;
	    break;
	case IEEE_ROUND_TO_NEAREST:
	    operand->qval = IEEE_PLUS_INFINITY;
	    operand->t.sign = sign;
	    break;
	case IEEE_ROUND_TO_PLUS_INFINITY:
	    operand->qval = (operand->t.sign == IEEE_PLUS_SIGN) ?
		    IEEE_PLUS_INFINITY : 
		    (pfp_attr->largest_number|IEEE_MINUS_ZERO);
	    break;
	case IEEE_ROUND_TO_MINUS_INFINITY:
	    operand->qval = (operand->t.sign != IEEE_PLUS_SIGN) ?
		    IEEE_MINUS_INFINITY : pfp_attr->largest_number;
	    break;
	} /* switch */
	pfp_context->fp_control.fields.floating_overflow = 1;
	/* per the rules-- overflow without a trap gets inexact too */
	pfp_context->isolated_excsum.fields.inexact_result = 1;
	return SFP_SUCCESS;

    } /* if */

    if (scale_value + exponent_value > 0L) {

	DPRINTF(("result is finite exponent is %ld + %ld\n", exponent_value, scale_value));
	operand->t.exponent += scale_value;
	return SFP_SUCCESS;

    } /* if */

    /* if we get here, the result is denorm */

    /* calculate amount to shift fraction */
    fraction_shift = - (scale_value + exponent_value);
    /* account for low bits */
    fraction_shift += fraction_fill;

    operand->t.exponent = IEEE_DENORM_EXPONENT;

    /* initialize rounding mode variables */
    cumulative = 0;
    last_shifted = 0;
    inexact = 0;

    /* do the shift and or in implied one found in finite
     *	operands (our inputs) but not in denorms.
     */
    DPRINTF(("before adding implied 1 bit: faction=0x%013lx, fraction_shift=%ld\n", operand->t.fraction, fraction_shift));
    original_fraction = fraction = 
	operand->t.fraction | (1L << (IEEE_T_FRACTION_SIZE));
    fraction_shift++;	/* unwanted bit displaced by implied 1 bit */

shift_fraction:

    /* determine inexact, cumulative (all but last bit shift out) and last
     *	bit shifted out.
     */
    large_scale_down = fraction_shift > IEEE_T_FRACTION_SIZE;
    if (large_scale_down) {
	bitmask = BITMASK(IEEE_T_FRACTION_SIZE);
	fraction_shift = IEEE_T_FRACTION_SIZE;
    } else {
	bitmask = BITMASK(fraction_shift);
    } /* if */

    if (pfp_context->fpcr.fields.inexact_result || (fraction&bitmask)) {

	DPRINTF(("about to shift out non-zero bits while denorming underflow operation result\n"));

	pfp_context->isolated_excsum.fields.inexact_result = 1;
	inexact = 1;

	/* set cumulative bit */
	if (pfp_context->fpcr.fields.inexact_result || 
	    ((fraction_shift > 0) && ((fraction<<1L)&bitmask))) {
	    cumulative = 1;
	} /* if */

	/* set last_shifted bit */
	if (!large_scale_down && fraction_shift &&
		((fraction>>(fraction_shift-1))&1)) {
	    last_shifted = 1;
	} /* if */

    } /* if */

    DPRINTF(("after adding implied 1 bit: fraction=0x%013lx, fraction_shift=%ld, fraction_fill=%ld\n", fraction, fraction_shift, fraction_fill));

    if (large_scale_down) {

	fraction = 0;
	DPRINTF(("setting fraction to 0 because scale down is so large\n"));

    } else if (fraction_shift > 0) {

	fraction >>= fraction_shift;

	if (pfp_attr->fraction_fill) {
	    /* cleanup lower stuff */
	    fraction <<= fraction_fill;
	} /* if */

	DPRINTF(("fraction = 0x%013lx\n", fraction));

    } /* if */

    if (original_fraction!=0L && fraction==0L) {

	/* underflow */
	DPRINTF(("enabling inexact and setting underflow\n"));

	/* if we were going after underflow set the sticky bits, 
	 *	else set the exception
	 */
	pfp_context->fp_control.fields.floating_underflow = 1;
	inexact = pfp_context->isolated_excsum.fields.inexact_result = 1;

    } else if (original_fraction == 0 || operand->t.exponent != 0) {

	EPRINTF(("unexpected original fraction(%ld) or new exponent(%ld)\n", original_fraction, operand->t.exponent));
    } /* if */


    /*

    Right now I am rounding all ops, from the following message, it
    appears I can skip this for add and subtract.

    From steven::hobbs Thu Sep 17 11:05:54 1992
    To: GILROY::himel
    Cc: HOBBS
    Subject: RE: I guess I still don't understand why

    Mark:

    If the the result of ADDx or SUBx is going to be a denorm then the
    exponents of the two operands have to both be small and close in value
    to each other.  It turns out that if the result is going to be a
    denorm then the exponents are so close in value that after the add/sub
    is done and normalization is complete that no one bits will be lost.
    Try and come up with an counter-example consisting of two IEEE values
    whose difference is an inexact denorm.  Trying to find this
    counter-example should convince you that inexact is impossible with an
    add/sub that generates a denorm.  If you do come up with such a
    counter-example then I will have learned something and we will have a
    good program for your test system.

    --Steve

    *	now to figure out the lsb (round) since we did our calculation
    *	chopped (because we didn't want a double round).

    given the sign, lsb, an cumulative inexact bit excluding the last bit 
    shifted out, and the last shifted out bit we can determine the lsb of 
    the fraction.

    cumulative      1       0       1       0       1       0       1       0
    last shifted    0       0       1       1       0       0       1       1
    lsb             0       0       0       0       1       1       1       1

    mode
    -----------     -       -       -       -
    NEAREST         lsb     lsb     f+1     0       lsb     lsb     f+1     f+1




    inexact         0       1       0       1
    sign            0       0       1       1

    mode
    ------------
    + INFINITY      lsb     f+1     lsb     lsb
    - INFINITY      lsb     lsb     lsb     f+1

    CHOPPED is always       lsb

    'inexact'	is cumulative + last shifted,
    'f'		is the fraction, and
    'lsb'	is the original result's lsb.

    according to the alpha architecture since rounding is done
    before check for underflow, there should be room to add one
    here for the 'f+1' cases without danger of creating a normalized
    number since otherwise the hardware should not have generated
    an underflow. NOTE: the previous comment is now untrue because
    invalid operations come here as well as underflows.

    *
    */
round:
    DPRINTF(("recalculating round\n"));
    switch (pfp_context->rounding) {

    case IEEE_ROUND_TO_NEAREST:
	if (last_shifted != 0 && (cumulative != 0 || 
	    ((fraction>>fraction_fill)&1) != 0)) {
		goto rounding_add_1;
	} /* if */
	break;

    case IEEE_ROUND_TO_ZERO:
	    break;

    case IEEE_ROUND_TO_PLUS_INFINITY:
	    if (inexact == 1 && operand->t.sign == 0) {
		goto rounding_add_1;
	    } /* if */
	    break;

    case IEEE_ROUND_TO_MINUS_INFINITY:
	    if (inexact == 1 && operand->t.sign == 1) {

rounding_add_1:
		DPRINTF(("round adding one\n"));
		/* check for rounding causing overflow of the fraction field */
		fraction_mask =  (1L << IEEE_T_FRACTION_SIZE) - 1L;
		fraction_mask &= ~((1L << fraction_fill) - 1L);
		if ((fraction&fraction_mask) == fraction_mask) {

		    /* go back to pot to check if the exponent overflowed */
		    DPRINTF(("fraction overflow during rounding: fraction = 0x%lx\n", fraction));
		    exponent_value = BIASED_POWER_OF_2(operand->t.exponent, 
			pfp_attr->exponent_bias);

		    /* check for overflow */
		    if (1+exponent_value > (long)pfp_attr->exponent_max) {

			/* depends on rounding, see spec */
			switch (pfp_context->rounding) {
			case IEEE_ROUND_TO_ZERO:
			    operand->qval = pfp_attr->largest_number;
			    operand->t.sign = sign;
			    break;
			case IEEE_ROUND_TO_NEAREST:
			    operand->qval = IEEE_PLUS_INFINITY;
			    operand->t.sign = sign;
			    break;
			case IEEE_ROUND_TO_PLUS_INFINITY:
			    operand->qval = (operand->t.sign == IEEE_PLUS_SIGN) ?
				    IEEE_PLUS_INFINITY : 
				    (pfp_attr->largest_number|IEEE_MINUS_ZERO);
			    break;
			case IEEE_ROUND_TO_MINUS_INFINITY:
			    operand->qval = (operand->t.sign != IEEE_PLUS_SIGN) ?
				    IEEE_MINUS_INFINITY : pfp_attr->largest_number;
			    break;
			} /* switch */
			DPRINTF(("scaling to infinity\n"));
			pfp_context->fp_control.fields.floating_overflow = 1;
			/* per the rules-- overflow without a trap gets inexact too */
			pfp_context->isolated_excsum.fields.inexact_result = 1;
			return SFP_SUCCESS;

		    } /* if */

		    /* no exponent overflow just add one to the exponent and move on */
		    operand->t.exponent = EXPONENT_REGISTER_FORMAT(exponent_value+1, 
			pfp_attr->exponent_bias);
		    fraction = 0;
		} else {

		    /* add is safe */
		    fraction += (1L << fraction_fill);

		} /* if */
	    } /* if */
	    break;

    } /* switch */


    operand->t.fraction = fraction;
    return SFP_SUCCESS;

} /* ieee_t_scale */
