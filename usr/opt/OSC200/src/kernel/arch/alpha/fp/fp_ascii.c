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
static char *rcsid = "@(#)$RCSfile: fp_ascii.c,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1992/12/01 16:37:12 $";
#endif

#ifndef KERNEL
#include <stdio.h>
#endif
#include <arch/alpha/fpu.h>
#include <arch/alpha/local_ieee.h>

#ifndef KERNEL
#define FPRINTF(args) fprintf fd args; fflush(stdout);
#else /*KERNEL */
#define FPRINTF(args)	 printf args
#endif /*KERNEL */

#define PRINT_EXC(object, exc)			\
    if (object.fields.exc) {			\
	if (!first_one)				\
	    FPRINTF(("|"));			\
	FPRINTF(("exc", object.fields.exc));	\
	first_one = 0;				\
    } /* if */


extern void
#ifndef KERNEL
alpha_print_fpcr(FILE	*fd, fpcr_t	fpcr)
#else /* KERNEL */
alpha_print_fpcr(fpcr_t	fpcr)
#endif /* KERNEL */
{

    unsigned long first_one;

    FPRINTF(("[0x%016lx: ", fpcr.qval));

    first_one = 1;
    PRINT_EXC(fpcr, invalid_operation)
    PRINT_EXC(fpcr, divide_by_zero)
    PRINT_EXC(fpcr, floating_overflow)
    PRINT_EXC(fpcr, floating_underflow)
    PRINT_EXC(fpcr, inexact_result)
    PRINT_EXC(fpcr, integer_overflow)
    PRINT_EXC(fpcr, summary)

    if (first_one) {
	FPRINTF(("no_exceptions"));
    } /* if */

    switch (fpcr.fields.dynamic_rounding) {
	case IEEE_ROUND_CHOPPED:
	    FPRINTF((", chopped(round to zero)]"));
	    break;

	case	IEEE_ROUND_TO_MINUS_INFINITY:
	    FPRINTF((", round to minus infinity]"));
	    break;

	case IEEE_ROUND_NORMAL:
	    FPRINTF((", normal(round to nearest)]"));
	    break;

	case IEEE_ROUND_TO_PLUS_INFINITY:
	    FPRINTF((", round to plus infinity]"));
	    break;

    } /* switch */

} /* alpha_print_fpcr */


extern void
#ifndef KERNEL
alpha_print_excsum(FILE	*fd,	excsum_t	excsum)
#else /* KERNEL */
alpha_print_excsum(excsum_t	excsum)
#endif /* KERNEL */
{
    unsigned long	first_one;

    FPRINTF(("[0x%016lx: ", excsum.qval));

    first_one = 1;
    PRINT_EXC(excsum, software_completion)
    PRINT_EXC(excsum, invalid_operation)
    PRINT_EXC(excsum, divide_by_zero)
    PRINT_EXC(excsum, floating_overflow)
    PRINT_EXC(excsum, floating_underflow)
    PRINT_EXC(excsum, inexact_result)
    PRINT_EXC(excsum, integer_overflow)

    if (first_one) {
	FPRINTF(("no_exceptions"));
    } /* if */

    FPRINTF(("]"));
} /* alpha_print_excsum */


extern void
#ifndef KERNEL
ieee_print_fp_control(FILE	*fd,	ieee_fp_control_t	fp_control)
#else /* KERNEL */
ieee_print_fp_control(ieee_fp_control_t	fp_control)
#endif /* KERNEL */
{
    unsigned long	first_one;

    FPRINTF(("[0x%016lx: ", fp_control.qval));

    first_one = 1;
    PRINT_EXC(fp_control, inherit)
    if (first_one) {
	FPRINTF((", "));
    } /* if */

    FPRINTF(("enables = "));
    first_one = 1;
    PRINT_EXC(fp_control, enable_invalid_operation)
    PRINT_EXC(fp_control, enable_divide_by_zero)
    PRINT_EXC(fp_control, enable_floating_overflow)
    PRINT_EXC(fp_control, enable_floating_underflow)
    PRINT_EXC(fp_control, enable_inexact_result)

    if (first_one) {
	FPRINTF(("none"));
    } /* if */

    FPRINTF((", mappings = "));

    first_one = 1;
    PRINT_EXC(fp_control, map_underflows_to_zero)

    if (first_one) {
	FPRINTF(("none"));
    } /* if */

    FPRINTF((", sticky_bits= "));

    first_one = 1;
    PRINT_EXC(fp_control, invalid_operation)
    PRINT_EXC(fp_control, divide_by_zero)
    PRINT_EXC(fp_control, floating_overflow)
    PRINT_EXC(fp_control, floating_underflow)
    PRINT_EXC(fp_control, inexact_result)

    if (first_one) {
	FPRINTF(("none"));
    } /* if */


    FPRINTF(("]"));
} /* alpha_print_fp_control */
