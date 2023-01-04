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
static char *rcsid = "@(#)$RCSfile: atof.c,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1994/01/10 23:27:42 $";
#endif

/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#endif
#include <sys/localedef.h>
#include <ctype.h>
#include <float.h>
#include <fp.h>
#include <errno.h>
#include <math.h>
#include <machine/endian.h>
#include <langinfo.h>
#include <nl_types.h>
#include <stdio.h>
#include <pdsc.h>
/*
 * #include <machine/fpu.h>
 * This is a little messing, but the fpu.h file must have 64 bit
 * longs in its data structures for execption handling to work.
 * for other reasons this file must be compiled as a 32-bit
 * routine in for the 32 bit libc. So for now just include
 * fpu.h in this routine and change the long to long long.
 */
/*
 * *****************************************************************
 * *                                                               *
 * *    Copyright (c) Digital Equipment Corporation, 1991, 1993    *
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

typedef unsigned long long	_fp_ulong_64;	/* identify large fields */

/* in register formats for alpha floating point types */

typedef struct fp_s_register {
	_fp_ulong_64		mbz:32;		/* must be zero */
	unsigned long long		fraction:20;
	unsigned long long		exponent:11;
	unsigned long long		sign:1;
} fp_s_register_t;				/* ieee single precision */

typedef struct fp_f_register {
	_fp_ulong_64		mbz:32;		/* must be zero */
	unsigned  long long		fraction:20;
	unsigned long long		exponent:11;
	unsigned long long		sign:1;
} fp_f_register_t;				/* vax single precision */

typedef struct fp_t_register {
	_fp_ulong_64		fraction:52;
	unsigned long long		exponent:11;
	unsigned long long		sign:1;
} fp_t_register_t;				/* ieee double precision */

typedef struct fp_g_register {
	_fp_ulong_64		fraction:52;
	unsigned long long		exponent:11;
	unsigned long long		sign:1;
} fp_g_register_t;				/* vax double precision */

typedef struct fp_d_register {
	_fp_ulong_64		fraction:55;
	unsigned long long		exponent:8;
	unsigned long long		sign:1;
} fp_d_register_t;				/* vax double precision */

typedef struct fp_l_register {
	_fp_ulong_64		mbz:29;
	unsigned long long		low_bits:30;
	unsigned long long		ignored:3;
	unsigned long long		hi_bit:1;
	unsigned long long		sign:1;
} fp_l_register_t;				/* longword format */

typedef struct fp_q_register {
	_fp_ulong_64		bits:63;
	unsigned long long		sign:1;
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
		unsigned long long inv : 1;
		unsigned long long dze : 1;
		unsigned long long ovf : 1;
		unsigned long long unf : 1;
		unsigned long long ine : 1;
		unsigned long long iov : 1;
		unsigned long long dyn : 2;
		unsigned long long reserved2 : 3;
		unsigned long long sum : 1;
	} fields;
} fpcr_t;

/* exception summary register */
typedef union excsum {
	_fp_ulong_64	qval;
	struct {
	    /* cumulative excption bits representing excptions in trap shadow */
	    unsigned long long	swc:1;
	    unsigned long long	inv:1;
	    unsigned long long	dze:1;
	    unsigned long long	ovf:1;
	    unsigned long long	unf:1;
	    unsigned long long	ine:1;
	    unsigned long long	iov:1;

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

	unsigned long long	reserved1:1;

	/* trap and action enables match excsum except for int overflow */
	unsigned long long	enable_inv:1;
	unsigned long long	enable_dze:1;
	unsigned long long	enable_ovf:1;
	unsigned long long	enable_unf:1;
	unsigned long long	enable_ine:1;

	unsigned long long	reserved2: 7;

	unsigned long long	map_underflows_to_zero:1;
	unsigned long long	reserved3:3;

	/* sticky status bits, only cleared by user */
	unsigned long long	inv:1;
	unsigned long long	dze:1;
	unsigned long long	ovf:1;
	unsigned long long	unf:1;
	unsigned long long	ine:1;

	unsigned long long	reserved4: 10;
	unsigned long long	reserved5: 31;
	unsigned long long	inherit:1;	/* on fork or thread create */

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
extern unsigned long long _get_fpcr();
extern unsigned long long _set_fpcr();
#else
extern unsigned long long _get_fpcr(void);
extern unsigned long long _set_fpcr(unsigned long fpcr);
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

/* end of fpu.h */

#include "ts_supp.h"

/* arg to identify I want my callers type */
#define MY_CALLER		1
#define MY_CALLERS_CALLER	2

/* ieee format (not implementation) macros */
#define IS_DENORM(fp) \
    (fp.t.exponent == IEEE_DENORM_EXPONENT && \
     fp.t.fraction != IEEE_ZERO_FRACTION)
#define IS_NAN(fp) \
    (fp.t.exponent == IEEE_NAN_EXPONENT && \
     fp.t.fraction != IEEE_ZERO_FRACTION)

#define MAXDIGITS 17

#if BYTE_ORDER == LITTLE_ENDIAN
#  define LSW 0
#  define MSW 1
static unsigned infinity[2] = { 0x00000000, 0x7ff00000 };
#endif
#if BYTE_ORDER == BIT_ENDIAN
#  define MSW 0
#  define LSW 1
static unsigned infinity[2] = { 0x7ff00000, 0x00000000 };
#endif
#ifndef HUGE_VAL
#define HUGE_VAL 1.8e308
#endif
#if STRTOD

double strtod(const char *s, char **endptr)

#else

/* Must be ATOF */

extern double _atod ();

/* currently strtod.c supplies strtod. The following changes must be
 *	removed if we use this  file to create strtod and you need
 *	to use MY_CALLER instead of MY_CALLERS_CALLER
 */

double atof (const char *s)
{
    double  __actual_atof();
    return __actual_atof(s);
}

double __actual_atof (char *s)
#endif
{
    register unsigned c;
    register unsigned negate, decimal_point;
    register char *d;
    register int exp;
    register double x;
	char *valid_addr;              /* a valid address for endptr to point */
    char *ssave = s;
	char   *radix;        /* radix character from locale info  */
    union {
	double d;
	unsigned w[2];
    } o;
    char digits[MAXDIGITS];
    fp_register_t	fp;	/* union of float formats */
    unsigned long long	type;	/* of fp result caller expects */

#if STRTOD
	/* If endptr points to a null then point it to a valid address so */
	/* we don't have to worry about it later.                         */
	if ( endptr == (char **)0) endptr = &valid_addr;
#else
	char **endptr = &valid_addr;
#endif
	radix = __lc_locale->nl_info[RADIXCHAR];
	if ((radix == 0) || !(*radix))
	     radix = ".";

   while (c = *s++, isspace(c)); /* Trim leading spaces */

    negate = 0;					/* Assume positve */
    if (c == '+') {
	c = *s++;
    }
    else if (c == '-') {
	negate = 1;				  /* not positive! Set flag */
	c = *s++;
    }
    d = digits;				 /* init pointer and Flags */
    decimal_point = 0;
    exp = 0;
    while (1) {
	c -= '0';				/* Not ascii now */
	if (c < 10) {
	    if (d == digits+MAXDIGITS) {
		    /* ignore more than 17 digits, but adjust exponent */
		    exp += (decimal_point ^ 1);
	        }
	        else {
		       if (c == 0 && d == digits) {
		    /* ignore leading zeros */
		       }
		       else {
		          *d++ = c;   /* num to buffer */
		       }
		       exp -= decimal_point; /* If not past decimal dec exp */
		    }
	}
	else if (c == *radix - '0' && !decimal_point) {
	    decimal_point = 1;  /* Found what ever is decimal pt */
	}
	else {
	    break; /* anything else we're done */
	}
	c = *s++; /* had a good char get the next... */
  }

    if (c == 'e'-'0' || c == 'E'-'0') { /* ahh the exponent! */
		register unsigned negate_exp = 0;  /* assume positve exp */
		register int e = 0;
		c = *s++;
		if (c == '+' || c == ' ') {
			c = *s++;
		}
		else if (c == '-') {
			negate_exp = 1;
			c = *s++;
		}
		while (c -= '0', c < 10 && e < 400) {
			e = e * 10 + c; /* add up the exp */
			c = *s++;
		}
		if (negate_exp) {
			e = -e;
		}
		exp += e;
    }
    if (d == digits) {
		/* Either not a legal numeric string or INF */
		register char *sptr = ssave;
		if ( (*sptr == '+') ||(*sptr == '-') ) sptr++; 
		if (   ( ( sptr[0] == 'I' ) || ( sptr[0] == 'i' ) )
		    && ( ( sptr[1] == 'N' ) || ( sptr[1] == 'n' ) )
		    && ( ( sptr[2] == 'F' ) || ( sptr[2] == 'f' ) )) {

			x = *(double *)infinity;
		}
		else {
			return 0.0;
		}
    } else if (exp < -340) {
		x = 0;
		TS_SETERR(ERANGE);
    }
    else if (exp > 308) {
		x = *(double *)infinity;
		TS_SETERR(ERANGE);
    }
    else {
		x = _atod (digits, d - digits, exp);
    }
    if (negate) {
		x = -x;
    }

    fp.dval = x;

    if (IS_DENORM(fp) || IS_NAN(fp) || fp.qval == IEEE_PLUS_INFINITY ||
	fp.qval == IEEE_MINUS_INFINITY) {

	type = __exc_get_fp_type(MY_CALLERS_CALLER);
	switch (type) {
	case PDSC_EXC_SILENT:			/* under->0, no signal */
	case PDSC_EXC_SIGNAL:			/* under->0, signal rest */
	case PDSC_EXC_SIGNAL_ALL:			/* signal all */
	    if (IS_DENORM(fp) || IS_NAN(fp)) {
		TS_SETERR(ERANGE);
		x = 0;
	    } else if (fp.qval == IEEE_PLUS_INFINITY) {
		TS_SETERR(ERANGE);
		x = DBL_MAX;
	    } else if (fp.qval == IEEE_MINUS_INFINITY) {
		TS_SETERR(ERANGE);
		x = -DBL_MAX;
	    } /* if */
	    break;
	    
	case PDSC_EXC_IEEE:	/* default values are ok */
	    break;
	} /* switch */

    } /* if */

    return x;
}
