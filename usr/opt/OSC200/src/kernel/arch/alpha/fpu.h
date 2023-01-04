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
 *	Alpha FP definitions
 */

#ifndef __ALPHA_FP_H__
#define __ALPHA_FP_H__

#ifdef __alpha
#if defined(__LANGUAGE_C__) || defined(__LANGUAGE_ASSEMBLER__)

#define IEEE_PLUS_SIGN		0
#define IEEE_MINUS_SIGN		1


/* ALPHA constants for exceptional values */
#define IEEE_PLUS_QUIET_NAN		0x7fffffffffffffffUL
#define IEEE_MINUS_QUIET_NAN		0xffffffffffffffffUL
#define IEEE_PLUS_SIGNALING_NAN		0x7ff7ffffffffffffUL
#define IEEE_MINUS_SIGNALING_NAN	0xfff7ffffffffffffUL


#define IEEE_PLUS_INFINITY		0x7ff0000000000000UL
#define IEEE_MINUS_INFINITY		0xfff0000000000000UL
#define IEEE_PLUS_LARGEST_NUMBER	0x7fefffffffffffffUL
#define IEEE_MINUS_LARGEST_NUMBER	0xffefffffffffffffUL
#define IEEE_PLUS_ZERO			0x0000000000000000UL
#define IEEE_MINUS_ZERO			0x8000000000000000UL
#define IEEE_PLUS_TWO			0x4000000000000000UL
/* ALPHA constants for integer values in fp registers */
#define IEEE_PLUS_LARGEST_QUAD_INTEGER	0x7fffffffffffffffUL
#define IEEE_MINUS_LARGEST_QUAD_INTEGER	0x8000000000000000UL
#define IEEE_PLUS_LARGEST_LONG_INTEGER	0x47ffffffe0000000UL
#define IEEE_MINUS_LARGEST_LONG_INTEGER	0x8000000000000000UL

#define IEEE_ZERO_EXPONENT		0
#define IEEE_ZERO_FRACTION		0
#define IEEE_DENORM_EXPONENT		0
#define IEEE_NAN_EXPONENT		0x7ff
#define IEEE_QUIET_NAN_MASK		0x0008000000000000UL

#define IEEE_FP_TRUE			IEEE_PLUS_TWO
#define IEEE_FP_FALSE			IEEE_PLUS_ZERO
#define IEEE_SIGN_BIT_MASK		(1UL<<63)

/* S format constants */
#define IEEE_S_FRACTION_SIZE		23
#define IEEE_S_EXPONENT_BIAS		127
#define IEEE_S_EXPONENT_MAX		254
#define IEEE_S_EXPONENT_SHIFT		3	/* in reg to make like t */

/* T format constants */
#define IEEE_T_FRACTION_SIZE		52
#define IEEE_T_EXPONENT_BIAS		1023
#define IEEE_T_EXPONENT_MAX		2046
#define IEEE_T_FRACTION_HIGH_BIT	(1UL<<51)
#define IEEE_T_EXPONENT_SHIFT		0

/* F format constants */
#define VAX_F_FRACTION_SIZE		23
#define VAX_F_EXPONENT_BIAS		127

/* G format constants */
#define VAX_G_FRACTION_SIZE		52
#define VAX_G_EXPONENT_BIAS		1023

/* D format constants */
#define VAX_D_FRACTION_SIZE		55
#define VAX_D_EXPONENT_BIAS	 	-127

/* Constants for fpcr bits in fc_struct.fc_word */
#define FPCR_INV	0x0010000000000000UL	/* invalid operation */
#define FPCR_DZE	0x0020000000000000UL	/* divide by zero */
#define FPCR_OVF	0x0040000000000000UL	/* overflow */
#define FPCR_UNF	0x0080000000000000UL	/* undeflow */
#define FPCR_INE	0x0100000000000000UL	/* inexact */
#define FPCR_IOV	0x0200000000000000UL	/* integer overflow */
#define FPCR_DYN_MASK	0x0c00000000000000UL	/* dynamic rounding mode mask */
#define FPCR_DYN_SHIFT	58			/* bit position of dyn_rm */
#define FPCR_DYN_CHOPPED 0x0000000000000000UL/* chopped rounding */
#define FPCR_DYN_MINUS	0x0400000000000000UL	/* minus inf rounding */
#define FPCR_DYN_NORMAL	0x0800000000000000UL	/* normal rounding */
#define FPCR_DYN_PLUS	0x0c00000000000000UL	/* plus inf rounding */
#define FPCR_SUM	0x8000000000000000UL	/* summary */

/*
 * These constants are for the dynamic
 * rounding mode
 */

/* Constants for dyn_rm field of the fpcr */
#define FE_TOWARDZERO	0	/* chopped */
#define FE_DOWNWARD	1	/* minus infinity */
#define FE_TONEAREST	2	/* normal */
#define FE_UPWARD	3	/* plus infinity */

/* ieee dynamic rounding mode field value names */
#define IEEE_ROUND_CHOPPED		FE_TOWARDZERO
#define IEEE_ROUND_TO_ZERO		IEEE_ROUND_CHOPPED	/* ieee name */
#define	IEEE_ROUND_TO_MINUS_INFINITY	FE_DOWNWARD
#define IEEE_ROUND_NORMAL		FE_TONEAREST
#define IEEE_ROUND_TO_NEAREST		IEEE_ROUND_NORMAL	/* ieee name */
#define IEEE_ROUND_TO_PLUS_INFINITY	FE_UPWARD

/* exception summary bits */
#define	EXCSUM_SWC	0x01
#define	EXCSUM_INV	0x02
#define	EXCSUM_DZE	0x04
#define	EXCSUM_OVF	0x08
#define	EXCSUM_UNF	0x10
#define	EXCSUM_INE	0x20
#define	EXCSUM_IOV	0x40
#define EXCSUM_MASK	0x7e

/*
 *	We implement the IEEE trap enables with a software register
 *	mask. The mask is 64 bits wide.
 */

/* read/write flags */
#define IEEE_TRAP_ENABLE_INV    0x000002   /* invalid operation */
#define IEEE_TRAP_ENABLE_DZE    0x000004   /* divide by 0 */
#define IEEE_TRAP_ENABLE_OVF    0x000008   /* overflow */
#define IEEE_TRAP_ENABLE_UNF    0x000010   /* underflow */
#define IEEE_TRAP_ENABLE_INE    0x000020   /* inexact */

#define IEEE_TRAP_ENABLE_MASK	0x00003e   /* if set, we handle */

#define IEEE_MAP_UMZ		0x002000   /* infinities map to largest num */

/* read only flags */
#define IEEE_STATUS_INV    	0x020000   /* invalid operation */
#define IEEE_STATUS_DZE    	0x040000   /* divide by 0 */
#define IEEE_STATUS_OVF    	0x080000   /* overflow */
#define IEEE_STATUS_UNF    	0x100000   /* underflow */
#define IEEE_STATUS_INE    	0x200000   /* inexact */
#define IEEE_STATUS_MASK	0x3f0000   /* mask of all sticky bits */
#define IEEE_STATUS_TO_EXCSUM_SHIFT	16 /* shift to line up with EXCSUM */

#define IEEE_INHERIT	0x8000000000000000 /* on fork or thread create */

#ifdef __LANGUAGE_C__

typedef unsigned long	_fp_ulong_64;	/* identify large fields */

/* in register formats for alpha floating point types */

typedef struct fp_s_register {
	_fp_ulong_64		mbz:32;		/* must be zero */
	unsigned long		fraction:20;
	unsigned long		exponent:11;
	unsigned long		sign:1;
} fp_s_register_t;				/* ieee single precision */

typedef struct fp_f_register {
	_fp_ulong_64		mbz:32;		/* must be zero */
	unsigned long		fraction:20;
	unsigned long		exponent:11;
	unsigned long		sign:1;
} fp_f_register_t;				/* vax single precision */

typedef struct fp_t_register {
	_fp_ulong_64		fraction:52;
	unsigned long		exponent:11;
	unsigned long		sign:1;
} fp_t_register_t;				/* ieee double precision */

typedef struct fp_g_register {
	_fp_ulong_64		fraction:52;
	unsigned long		exponent:11;
	unsigned long		sign:1;
} fp_g_register_t;				/* vax double precision */

typedef struct fp_d_register {
	_fp_ulong_64		fraction:55;
	unsigned long		exponent:8;
	unsigned long		sign:1;
} fp_d_register_t;				/* vax double precision */

typedef struct fp_l_register {
	_fp_ulong_64		mbz:29;
	unsigned long		low_bits:30;
	unsigned long		ignored:3;
	unsigned long		hi_bit:1;
	unsigned long		sign:1;
} fp_l_register_t;				/* longword format */

typedef struct fp_q_register {
	_fp_ulong_64		bits:63;
	unsigned long		sign:1;
} fp_q_register_t;				/* quadword format */

typedef union fp_register {
	_fp_ulong_64		qval;		/* quad to access */
	double			dval;		/* double to print */
	fp_s_register_t		s;		/* ieee single precision */
	fp_t_register_t		t;		/* ieee double precision */
	fp_f_register_t		f;		/* vax single precision */
	fp_g_register_t		g;		/* vax double precision */
	fp_d_register_t		d;		/* vax double precision */
	fp_l_register_t		l;		/* longword format */
	fp_l_register_t		q;		/* quadword format */
} fp_register_t;

/*
 * Structure and constant definisions for the floating-point control
 * control and status register (fpcr).
 */
typedef union fpcr {
	_fp_ulong_64	 qval;
	struct {
		_fp_ulong_64 reserved1 : 52;
		unsigned long inv : 1;
		unsigned long dze : 1;
		unsigned long ovf : 1;
		unsigned long unf : 1;
		unsigned long ine : 1;
		unsigned long iov : 1;
		unsigned long dyn : 2;
		unsigned long reserved2 : 3;
		unsigned long sum : 1;
	} fields;
} fpcr_t;

/* exception summary register */
typedef union excsum {
	_fp_ulong_64	qval;
	struct {
	    /* cumulative excption bits representing excptions in trap shadow */
	    unsigned long	swc:1;
	    unsigned long	inv:1;
	    unsigned long	dze:1;
	    unsigned long	ovf:1;
	    unsigned long	unf:1;
	    unsigned long	ine:1;
	    unsigned long	iov:1;

	    _fp_ulong_64	reserved:57;		/* must be zero */
	} fields;
} excsum_t;

#define IEEE_TRAP_HIT(excsum, fp_control)				\
    ((((excsum).qval&EXCSUM_MASK) & 					\
       ((fp_control).qval&IEEE_TRAP_ENABLE_MASK)) ||			\
     ((excsum).fields.integer_overflow &&				\
      (fp_control).fields.invalid_operation))

typedef union ieee_fp_control {
    _fp_ulong_64		qval;
    struct {

	unsigned long	reserved1:1;

	/* trap and action enables match excsum except for int overflow */
	unsigned long	enable_inv:1;
	unsigned long	enable_dze:1;
	unsigned long	enable_ovf:1;
	unsigned long	enable_unf:1;
	unsigned long	enable_ine:1;

	unsigned long	reserved2: 7;

	unsigned long	map_underflows_to_zero:1;
	unsigned long	reserved3:3;

	/* sticky status bits, only cleared by user */
	unsigned long	inv:1;
	unsigned long	dze:1;
	unsigned long	ovf:1;
	unsigned long	unf:1;
	unsigned long	ine:1;

	unsigned long	reserved4: 10;
	unsigned long	reserved5: 31;
	unsigned long	inherit:1;	/* on fork or thread create */

    } fields;

} ieee_fp_control_t;

#ifdef _NO_PROTO
extern int fegetround();
extern int fesetround();
#else
extern int fegetround(void);
extern int fesetround(int round);
#endif

/*
 * These functions are used to get and set the floating point control
 * register (fpcr)
 */
#ifdef _NO_PROTO
extern unsigned long _get_fpcr();
extern unsigned long _set_fpcr();
#else
extern unsigned long _get_fpcr(void);
extern unsigned long _set_fpcr(unsigned long fpcr);
#endif

/* backward compatability macros */
#define fc_word		qval
#define fc_struct	fields
#define dyn_rm		dyn
#define FPCR_DYN_RM_MASK	FPCR_DYN_MASK
#define FPCR_DYN_RM_SHIFT	FPCR_DYN_SHIFT
#define FPCR_DYN_RM_CHOPPED	FPCR_DYN_CHOPPED
#define FPCR_DYN_RM_MINUS	FPCR_DYN_MINUS
#define FPCR_DYN_RM_NORMAL	FPCR_DYN_NORMAL
#define FPCR_DYN_RM_PLUS	FPCR_DYN_PLUS

#endif /* __LANGUAGE_C__ */
#endif /* __LANGUAGE_C__ || __LANGUAGE_ASSEMBLER__ */
#endif /* __alpha */
#endif /* __ALPHA_FP_H__ */
