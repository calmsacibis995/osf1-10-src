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
 * @(#)$RCSfile: local_ieee.h,v $ $Revision: 1.1.7.2 $ (DEC) $Date: 1993/06/14 17:42:11 $
 */

#ifndef _LOCAL_IEEE_H_
#define _LOCAL_IEEE_H_

#include <machine/inst.h>

/* this include file contains definitions which are only useful to the
 *	internal implementation of IEEE exception handling. None of
 *	these definitions is expected to be seen outside the implementation.
 *	(and if they need to be externalized they need to be renamed).
 */

/* the following macros help deal with S versus T in register exponent
 *	formats.  Whenever we want to go from an exponent number to what we have
 *	to add to the actual exponent field, we use EXPONENT_REGISTER_FORMAT. 
 *	When we	want to go from the exponent field to an actual exponent we 
 *	BIASED_POWER_OF_2 regardless of format.
 */
#define EXPONENT_REGISTER_FORMAT(s, bias) \
	((unsigned long)(((long)(s) - (long)(bias))+(long)IEEE_T_EXPONENT_BIAS))
#define BIASED_POWER_OF_2(e, bias) \
    (((long)(e) >= 0) ? \
     (((long)(e)-(long)IEEE_T_EXPONENT_BIAS)+(long)(bias)) : \
     -((-(long)(e)-(long)IEEE_T_EXPONENT_BIAS)+(long)(bias)))

#define NONE 0x00

/* set bits if rounding mode flag is set */
#define D 0xff				/* dynamic rounding */
#define M IEEE_ROUND_TO_MINUS_INFINITY	/* minus infinity */
#define C IEEE_ROUND_CHOPPED		/* chopped */
#define N IEEE_ROUND_NORMAL		/* nearest */

/* set bits if exception can occur and EV4 waiver */
#define E 0x80 /* EV4 waiver divide */
#define I 0x01 /* inexact result, optional */
#define O 0x02 /* invalid operation */
#define S 0x04 /* software, optional */
#define V 0x08 /* floating overflow */
#define U 0x10 /* underflow, optional */
#define W 0x20 /* integer overflow, optional */
#define Z 0x40 /* divide by zero */

/* describe operand & result format */
#define FORMAT_NONE	0	/* any format */
#define FORMAT_IEEE_S	1	/* ieee s */
#define FORMAT_IEEE_T	2	/* ieee t */
#define FORMAT_VAX_D	3	/* vax d */
#define FORMAT_VAX_F	4	/* vax f */
#define FORMAT_VAX_G	5	/* vax g */
#define FORMAT_LONG	6	/* longword */
#define FORMAT_QUAD	7	/* quadword */

/* structure containing floating point instruction info */
typedef struct float_entry {
	unsigned short		root_function_code;/* function code */
	unsigned short		function_code;	/* function code */
	unsigned char		rounding;	/* supported by opcode */
	unsigned char		exceptions;	/* possible with opcode */
	unsigned char		operands_format;/* operand formats */
	unsigned char		result_format;	/* result formats */
	double			(*routine)();	/* emulation routine */
	char			*string;	/* info on opcode */
} float_entry;

/* in case of reordering of fields in float_entry */
#define ENTRY(root,code,func,name,round,exceptions,operand_format,result_format)	\
    {root, code, round, exceptions,operand_format,result_format, func, name}
#define TENTRY(r,c,f,n,ro,e) ENTRY(r,c,f,n,ro,e,FORMAT_IEEE_T,FORMAT_IEEE_T)
#define TQENTRY(r,c,f,n,ro,e) ENTRY(r,c,f,n,ro,e,FORMAT_IEEE_T,FORMAT_QUAD)
#define GENTRY(r,c,f,n,ro,e) ENTRY(r,c,f,n,ro,e,FORMAT_VAX_G,FORMAT_VAX_G)

/* macros to make defining the lookup table easier */
#define IEEE_FP_ARITH_OP(ROOT,EXTRA,O,R)  \
ENTRY(flti_##ROOT, flti_##ROOT, __emulate_##ROOT, "ieee ROOT", N, O|V##EXTRA,O,R), \
ENTRY(flti_##ROOT, flti_##ROOT##c, __emulate_##ROOT##c, "ieee ROOT/C", C, O|V##EXTRA,O,R), \
ENTRY(flti_##ROOT, flti_##ROOT##m, __emulate_##ROOT##m, "ieee ROOT/M", N, O|V##EXTRA,O,R), \
ENTRY(flti_##ROOT, flti_##ROOT##d, __emulate_##ROOT##d, "ieee ROOT/D", N, O|V##EXTRA,O,R), \
ENTRY(flti_##ROOT, flti_##ROOT##u, __emulate_##ROOT##u, "ieee ROOT/U", N, O|U|V##EXTRA,O,R), \
ENTRY(flti_##ROOT, flti_##ROOT##uc, __emulate_##ROOT##uc, "ieee ROOT/U/C", C, O|U|V##EXTRA,O,R), \
ENTRY(flti_##ROOT, flti_##ROOT##um, __emulate_##ROOT##um, "ieee ROOT/U/M", M, O|U|V##EXTRA,O,R), \
ENTRY(flti_##ROOT, flti_##ROOT##ud, __emulate_##ROOT##ud, "ieee ROOT/U/D", D, O|V##EXTRA,O,R), \
ENTRY(flti_##ROOT, flti_##ROOT##su, __emulate_##ROOT##su, "ieee ROOT/S/U", N, O|S|U|V##EXTRA,O,R),\
ENTRY(flti_##ROOT, flti_##ROOT##suc, __emulate_##ROOT##suc, "ieee ROOT/S/U/C", C, O|S|U|V##EXTRA,O,R),\
ENTRY(flti_##ROOT, flti_##ROOT##sum, __emulate_##ROOT##sum, "ieee ROOT/S/U/M", M, O|S|U|V##EXTRA,O,R),\
ENTRY(flti_##ROOT, flti_##ROOT##sud, __emulate_##ROOT##sud, "ieee ROOT/S/U/D", D, O|S|U|V##EXTRA,O,R),\
ENTRY(flti_##ROOT, flti_##ROOT##sui, __emulate_##ROOT##sui, "ieee ROOT/S/U/I", N, I|O|S|U|V##EXTRA,O,R),\
ENTRY(flti_##ROOT, flti_##ROOT##suic, __emulate_##ROOT##suic, "ieee ROOT/S/U/I/C", C, I|O|S|U|V##EXTRA,O,R),\
ENTRY(flti_##ROOT, flti_##ROOT##suim, __emulate_##ROOT##suim, "ieee ROOT/S/U/I/M", M, I|O|S|U|V##EXTRA,O,R),\
ENTRY(flti_##ROOT, flti_##ROOT##suid, __emulate_##ROOT##suid, "ieee ROOT/S/U/I/D", D, I|O|S|U|V##EXTRA,O,R)

#define IEEE_FP_CVT_OP(ROOT,EXTRA1,EXTRA2,O,R)  \
ENTRY(flti_##ROOT, flti_##ROOT, __emulate_##ROOT, "ieee ROOT", N, NONE##EXTRA1,O,R), \
ENTRY(flti_##ROOT, flti_##ROOT##c, __emulate_##ROOT##c, "ieee ROOT/C", C, NONE##EXTRA1,O,R), \
ENTRY(flti_##ROOT, flti_##ROOT##m, __emulate_##ROOT##m, "ieee ROOT/M", N, NONE##EXTRA1,O,R), \
ENTRY(flti_##ROOT, flti_##ROOT##d, __emulate_##ROOT##d, "ieee ROOT/D", N, NONE##EXTRA1,O,R), \
ENTRY(flti_##ROOT, flti_##ROOT##sui, __emulate_##ROOT##sui, "ieee ROOT/S/U/I", N, I|S##EXTRA2,O,R), \
ENTRY(flti_##ROOT, flti_##ROOT##suic, __emulate_##ROOT##suic, "ieee ROOT/S/U/I/C", C, I|S##EXTRA2,O,R), \
ENTRY(flti_##ROOT, flti_##ROOT##suim, __emulate_##ROOT##suim, "ieee ROOT/S/U/I/M", M, I|S##EXTRA2,O,R), \
ENTRY(flti_##ROOT, flti_##ROOT##suid, __emulate_##ROOT##suid, "ieee ROOT/S/U/I/D", D, I|S##EXTRA2,O,R)

#define VAX_FP_ARITH_OP(ROOT,EXTRA,OV,ov,O,R)  \
ENTRY(fltv_##ROOT, fltv_##ROOT, __emulate_##ROOT, "vax ROOT", N, O|V##EXTRA,O,R), \
ENTRY(fltv_##ROOT, fltv_##ROOT##c, __emulate_##ROOT##c, "vax ROOT/C", C, O|V##EXTRA,O,R), \
ENTRY(fltv_##ROOT, fltv_##ROOT##ov, __emulate_##ROOT##ov, "vax ROOT/OV", N, O|OV|V##EXTRA,O,R), \
ENTRY(fltv_##ROOT, fltv_##ROOT##ov##c, __emulate_##ROOT##ov##c, "vax ROOT/OV/C", C, O|OV|V##EXTRA,O,R), \
ENTRY(fltv_##ROOT, fltv_##ROOT##s, __emulate_##ROOT##s, "vax ROOT/S", N, O|S|V##EXTRA,O,R), \
ENTRY(fltv_##ROOT, fltv_##ROOT##sc, __emulate_##ROOT##sc, "vax ROOT/S/C", C, O|S|V##EXTRA,O,R), \
ENTRY(fltv_##ROOT, fltv_##ROOT##s##ov, __emulate_##ROOT##s##ov, "vax ROOT/S/OV", N, O|S|OV|V##EXTRA,O,R),\
ENTRY(fltv_##ROOT, fltv_##ROOT##s##ov##c, __emulate_##ROOT##s##ov##c, "vax ROOT/S/OV/C", C, O|S|OV|V##EXTRA,O,R)


/* standard return arguments */
#define SFP_SUCCESS		1
#define SFP_FAILURE		0

/* debug macros */

#ifdef  DEBUG				/* DEBUGGING ON */

#define DSTUPID2 : ")

#ifndef KERNEL

unsigned long _ieee_skip_debug;
#define DSTUPID1  printf("
#define DPRINTF(args)	if (!_ieee_skip_debug) { DSTUPID1 MODULE_NAME DSTUPID2 ; printf args; fflush(stdout);}
#define EPRINTF(args)	printf ("ERROR: "); DSTUPID1 MODULE_NAME DSTUPID2 ; printf args; fflush(stdout); 
#define DPRINT_EXCSUM(excsum) if (!_ieee_skip_debug) alpha_print_excsum(stdout, excsum)
#define DPRINT_FP_CONTROL(fp_control) if (!_ieee_skip_debug) ieee_print_fp_control(stdout, fp_control)
#define DPRINT_FPCR(fpcr) if (!_ieee_skip_debug) alpha_print_fpcr(stdout, fpcr)
#define DPRINTC(args)	if (!_ieee_skip_debug) {printf(" "); printf args; fflush(stdout); }

#else /* KERNEL */

unsigned long _ieee_skip_debug;
#define DSTUPID1  printf("
#define DPRINTF(args)	if (!_ieee_skip_debug) { DSTUPID1 MODULE_NAME DSTUPID2 ; printf args;}
#define DPRINT_FPCR(fpcr) if (!_ieee_skip_debug) alpha_print_fpcr(fpcr)
#define DPRINTC(args)	if (!_ieee_skip_debug) {printf(" "); printf args;}
#define DPRINT_FP_CONTROL(fp_control) if (!_ieee_skip_debug) ieee_print_fp_control(fp_control)
#define DPRINT_EXCSUM(excsum) if (!_ieee_skip_debug) alpha_print_excsum(excsum)
#define EPRINTF(args)	printf ("FATAL IEEE FLOATING POINT EMULATION ERROR:\n"); DSTUPID1 MODULE_NAME DSTUPID2 ; printf args; psignal(u.u_procp,SIGKILL); psig()
#endif /* KERNEL */

#else /* DEBUG */		/* DEBUGGING OFF */

#ifndef KERNEL

#define EPRINTF(args)   printf ("SOFTFP ERROR: "); printf args;

#else /* KERNEL */

#define DSTUPID1  uprintf("
#define DSTUPID2 : ")
#define EPRINTF(args)	uprintf ("FATAL IEEE FLOATING POINT EMULATION ERROR:\n"); DSTUPID1 MODULE_NAME DSTUPID2 ; uprintf args; psignal(u.u_procp,SIGKILL); psig()

#endif /* KERNEL */

#define DPRINTF
#define DPRINT_EXCSUM(excsum)
#define DPRINT_FP_CONTROL(fp_control)
#define DPRINT_FPCR(fpcr)
#define DPRINTC(args)

#endif /* DEBUG */

/* long names for those of us who are born to be verbose */
#define	software_completion	swc
#define	invalid_operation	inv
#define	divide_by_zero		dze
#define	floating_overflow	ovf
#define	floating_underflow	unf
#define	inexact_result		ine
#define	integer_overflow	iov
#define	dynamic_rounding	dyn
#define	summary			sum

#define	enable_invalid_operation	enable_inv
#define	enable_divide_by_zero		enable_dze
#define	enable_floating_overflow	enable_ovf
#define	enable_floating_underflow	enable_unf
#define	enable_inexact_result		enable_ine


/* structure to pass around fp trap information */
typedef struct alpha_fp_trap_context {
	float_entry			*pfloat_entry;	/* table entry */
	fp_register_t			original_operand1;
	fp_register_t			original_operand2;
	fp_register_t			operand1;
	fp_register_t			operand2;
	fp_register_t			result;		/* for default */
	unsigned int			rounding;	/* at time of trap */
	unsigned int			invalid_shadow;	/* as it says */
	ieee_fp_control_t		fp_control;	/* to be or'ed */
	excsum_t			isolated_excsum;/* isolated */
	excsum_t			actual_excsum;	/* after defaults */
	union alpha_instruction		*pc;		/* trigger pc */
	union alpha_instruction		inst;		/* trigger inst */
	fpcr_t				fpcr;		/* hw control reg */
	unsigned long			target_exception;/* we're resolving */
	unsigned long			regmask;	/* a1 on entry */
	unsigned long                   dont_reset_inexact_result;
} alpha_fp_trap_context;

/* the following data structure is used to hand around the attributes
 *	associated with various floating point formats in one easy
 *	clump.
 */
typedef struct fp_attr {
	unsigned long		format;		/* operand format- s,t, etc. */
	unsigned long		exponent_max;	/* contains max finit exp */
	unsigned long		exponent_bias;	/* to exp of 0 */
	unsigned long		exponent_shift;	/* to get to reg format */
	unsigned long		fraction_fill;	/* diff in s fraction size */
	unsigned long		fraction_size;	/* size of fraction */
	unsigned long           largest_number; /* largest positive number */
} fp_attr_t;

extern fp_attr_t *
alpha_fp_attr_lookup(
	unsigned long	format);	/* format of operands */

extern float_entry *
alpha_float_entry_lookup(
	unsigned long	opcode,		/* opcode to lookup */
	unsigned long	function_code,	/* function code we're looking for */
	unsigned long	rounding,	/* if set look for root with rounding */
					/* unset means N */
	unsigned long	exceptions);	/* if set look for root with traps */
					/* unset is NONE */

/* chaning power of two, possibly denorming */
extern unsigned long
ieee_t_scale(
	fp_register_t			*operand,
	long				scale,		/* in field format */
	alpha_fp_trap_context		*pfp_context,
	struct fp_attr			*pfp_attr);

/* returns scale required (in field format) */
extern unsigned long
ieee_renorm(
	fp_register_t	*operand,
	unsigned long	exponent_bias);

#endif
