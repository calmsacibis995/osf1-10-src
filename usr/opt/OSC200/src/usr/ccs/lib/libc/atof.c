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
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0
 */
/*
 * COMPONENT_NAME: LIBCCNV atof
 *
 * FUNCTIONS: atof
 */
/* ------------------------------------------------------------------ */
/* | Copyright Unpublished, MIPS Computer Systems, Inc.  All Rights | */
/* | Reserved.  This software contains proprietary and confidential | */
/* | information of MIPS and its suppliers.  Use, disclosure or     | */
/* | reproduction is prohibited without the prior express written   | */
/* | consent of MIPS.                                               | */
/* ------------------------------------------------------------------ */
/* $Header: /usr/sde/osf1/rcs/os/src/usr/ccs/lib/libc/atof.c,v 4.3.12.6 1993/11/09 20:53:33 Neil_OBrien Exp $ */
/*
 *	Modification History
 *
 * 1	Jon Reeves, 1989 June 14
 *	Internationalize: use _lc_radix instead of hardcoded '.'.
 *	Overlooked in the initial port.
 *
 * 2	Jon Reeves, 1989 Sep 19
 *	Fix error handling: detect absurdly large exponents, set ERANGE
 *
 * 3	Jon Reeves, 1989 Nov 14
 *	Fix error handling: set ERANGE on exponent underflow, too
 */

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
#include <machine/fpu.h>
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
    unsigned long	type;	/* of fp result caller expects */

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
