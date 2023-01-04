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
static char	*sccsid = "@(#)$RCSfile: wcstod.c,v $ $Revision: 1.1.2.3 $ (DEC) $Date: 1993/10/07 18:33:00 $";
#endif
/*
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 */
/*
 * OSF/1 1.2
 */
/* The original code in this file came from D. Gay's net posting.

 * The major changes made to it were:
 *
 *	macros
 *		reordered
 *			default is IEEE, that is, not (IBM or VAX)
 *		removed where redundant
 *			many retained for portability - may go in the future
 *	cb'd and tidied code
 *	added comments
 *	locking for threads
 *	I18N
 *	used macros for INF/ NaN checks
 *	dtoa renamed to _dtoa and last arg is now in-out buffer

 * Assorted magic:
 *	unsigned longs are 32 bits
 *	9 decimal digits fit in a 32 binary bit number
 *	22 is the largest power of ten which fits in a double (IEEE)

 * Note
 *	long doubles are not supported in this implementation
 */

/****************************************************************
 *
 * The author of this software is David M. Gay.
 *
 * Copyright (c) 1991 by AT&T.
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose without fee is hereby granted, provided that this entire notice
 * is included in all copies of any software which is or includes a copy
 * or modification of this software and in all copies of the supporting
 * documentation for such software.
 *
 * THIS SOFTWARE IS BEING PROVIDED "AS IS", WITHOUT ANY EXPRESS OR IMPLIED
 * WARRANTY.  IN PARTICULAR, NEITHER THE AUTHOR NOR AT&T MAKES ANY
 * REPRESENTATION OR WARRANTY OF ANY KIND CONCERNING THE MERCHANTABILITY
 * OF THIS SOFTWARE OR ITS FITNESS FOR ANY PARTICULAR PURPOSE.
 *
 ***************************************************************/


/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#if defined(_SHARED_LIBRARIES) || (!defined(_SHARED_LIBRARIES) && !defined(_THREAD_SAFE))
#pragma weak wcstod = __wcstod
#endif
#endif
#ifdef DEBUG
#include <stdio.h>
#define Bug(cond, msg)	if (cond) {fprintf(stderr, "%s\n", msg); exit(1);}

#else	/* DEBUG */
#define Bug(cond, msg)
#endif	/* DEBUG */

#include <stdlib.h>
#include <string.h>
#include <float.h>
#include <math.h>
#include <values.h>
#include <fp.h>
#include <errno.h>
#include <langinfo.h>
#include <sys/localedef.h>
#include "ts_supp.h"
#include "dtoa_ieee.h"

#ifdef _THREAD_SAFE
#include "rec_mutex.h"

/* This lock protects both the Bigint freelist[] and the pow5mult() p5s list.
 * Though these are disjoint objects it doesn't seem worth two locks.
 * Secondly, the lock is very coarse grained; it is held throughout the
 * entrypoint functions.
 */
extern struct rec_mutex	_atof_rmutex;
#endif	/* _THREAD_SAFE */

#ifdef UNSIGNED_SHIFTS
#define Sign_Extend(a,b)	if (b < 0) a = (short)a;

#else	/* UNSIGNED_SHIFTS */
#define Sign_Extend(a,b)	/*no-op*/
#endif	/* UNSIGNED_SHIFTS */

#define word0(x)	VALH(x)
#define word1(x)	VALL(x)

#if BYTE_ORDER == BIG_ENDIAN
#define Storeinc(a,b,c) (((unsigned short *)a)[0] = (unsigned short)b, \
	((unsigned short *)a)[1] = (unsigned short)c, a++)

#else	/* BYTE_ORDER == BIG_ENDIAN */
#define Storeinc(a,b,c) (((unsigned short *)a)[1] = (unsigned short)b, \
	((unsigned short *)a)[0] = (unsigned short)c, a++)
#endif	/* BYTE_ORDER == BIG_ENDIAN */

/* DIGITSINLONG is the number of decimal digits which can be represented
 * exactly with a 32 digit binary number (ie an unsigned long).
 * Hence, two u_longs can hold more decimal digits (18) than a
 * double (DBL_DIG == 15).
 */
#define DIGITSINLONG	9	/* 10^9 < 2^32 */


#ifdef RND_PRODQUOT
#define Rounded_product(a,b)	a = rnd_prod(a, b)
#define Rounded_quotient(a,b)	a = rnd_quot(a, b)
extern double	rnd_prod(double, double), rnd_quot(double, double);

#else	/* RND_PRODQUOT */
#define Rounded_product(a,b)	a *= b
#define Rounded_quotient(a,b)	a /= b
#endif	/* RND_PRODQUOT */

#define BIG0 (FRAC_MASK1 | EXP_MSK1*(DBL_MAX_EXP+BIAS-1))
#define BIG1 0xffffffff

/* Copy the interesting parts of a Bigint to another.
 */
#define Bcopy(x, y)	memcpy((char *)&x->sign, (char *)&y->sign, \
				y->wds*sizeof(int) + 2*sizeof(int))

#define Kmax 15		/* max bucket size : 2^15 * 32 bits */

typedef struct Bigint {
	struct Bigint	*next;
	int		k;	/* bucket index */
	int		maxwds;	/* max words */
	int		sign;	/* 0 +ve, 1 -ve !! do not reorder !! */
	int		wds;	/* words used */
	unsigned int	x[1];	/* words least to most significant */
} Bigint;

/* Use of this cache is for performance.
 * It is unclear whether this is really a big win for the average user.
 */
static Bigint *freelist[Kmax+1];


/* Allocate and init a 2^k * 32 bit Bigint.
 */
static Bigint *
Balloc(int k)
{
	int	x;
	Bigint	*rv;

	if (rv = freelist[k]) {
		freelist[k] = rv->next;
	} else {
		x = 1 << k;
		rv = (Bigint * )malloc(sizeof(Bigint) + (x - 1) * sizeof(int));
		rv->k = k;
		rv->maxwds = x;
	}
	rv->sign = rv->wds = 0;
	return (rv);
}


/* Cache Bigint in bucket chain.
 */
static void
Bfree(Bigint *v)
{
	if (v) {
		v->next = freelist[v->k];
		freelist[v->k] = v;
	}
}


/* Multiply b by m and add a.
 */
static Bigint *
multadd(Bigint *b, int m, int a)
{
	int		i, wds;
	unsigned int	*x, y;
	unsigned int	xi, z;
	Bigint		*b1;

	wds = b->wds;
	x = b->x;
	i = 0;
	/* step through b multiplying and adding each half word */
	do {
		xi = *x;
		y = (xi & 0xffff) * m + a;	/* low half word */
		z = (xi >> 16) * m + (y >> 16);	/* high half word + carry */
		a = (int)(z >> 16);		/* carry over to next word */
		*x++ = (z << 16) + (y & 0xffff);
	} while (++i < wds);

	/* did we run out of words ? */
	if (a) {
		/* alloc more if necessary */
		if (wds >= b->maxwds) {
			b1 = Balloc(b->k + 1);
			Bcopy(b1, b);
			Bfree(b);
			b = b1;
		}
		b->x[wds++] = a;
		b->wds = wds;
	}
	return (b);
}


/* Allocate Bigint for number string and fill it in.
 * [ Currently unused due to the char * restriction this code
 *   remains for completeness of Bigint manipulation routines ]
 */
static Bigint *
s2b(const char *s, int nd0, int rad, int nd, unsigned int upper)
{
	Bigint	*b;
	int	i, k;
	int	x, y;

	/* allocate Bigint big enough */
	x = (nd + DIGITSINLONG - 1) / DIGITSINLONG;
	for (k = 0, y = 1; x > y; y <<= 1, k++) 
		;
	b = Balloc(k);

	/* fill in first long's worth */
	b->x[0] = upper;
	b->wds = 1;

	/* add the part before the radix */
	i = DIGITSINLONG;
	if (DIGITSINLONG < nd0) {
		s += DIGITSINLONG;
		do {
			b = multadd(b, 10, *s++ - '0');
		} while (++i < nd0);
		s += rad;			/* skip radix */
	} else
		s += DIGITSINLONG + rad;	/* skip radix */

	/* add the rest */
	for (; i < nd; i++)
		b = multadd(b, 10, *s++ - '0');
	return (b);
}


/* Count leading zeros in x.
 */
static int
hi0bits(register unsigned int x)
{
	register int	k = 0;

	if (!(x & 0xffff0000)) {
		k = 16;
		x <<= 16;
	}
	if (!(x & 0xff000000)) {
		k += 8;
		x <<= 8;
	}
	if (!(x & 0xf0000000)) {
		k += 4;
		x <<= 4;
	}
	if (!(x & 0xc0000000)) {
		k += 2;
		x <<= 2;
	}
	if (!(x & 0x80000000)) {
		k++;
		if (!(x & 0x40000000))
			return (32);
	}
	return (k);
}


/* Count low zeros in y and shift y right by that amount.
 */
static int
lo0bits(unsigned int *y)
{
	register int		k;
	register unsigned int	x = *y;

	if (x & 7) {
		if (x & 1)
			return (0);
		if (x & 2) {
			*y = x >> 1;
			return (1);
		}
		*y = x >> 2;
		return (2);
	}
	k = 0;
	if (!(x & 0xffff)) {
		k = 16;
		x >>= 16;
	}
	if (!(x & 0xff)) {
		k += 8;
		x >>= 8;
	}
	if (!(x & 0xf)) {
		k += 4;
		x >>= 4;
	}
	if (!(x & 0x3)) {
		k += 2;
		x >>= 2;
	}
	if (!(x & 1)) {
		k++;
		x >>= 1;
		if (!x & 1)
			return (32);
	}
	*y = x;
	return (k);
}


/* Convert integer to Bigint.
 */
static Bigint *
i2b(int i)
{
	Bigint	*b;

	b = Balloc(1);	/* allocate 2 words */
	b->x[0] = i;
	b->wds = 1;
	return (b);
}


/* Multiply two Bigints and return result in new Bigint.
 */
static Bigint *
mult(Bigint *a, Bigint *b)
{
	Bigint		*c;
	int		k, wa, wb, wc;
	unsigned int	carry, y, z;
	unsigned int	*x, *xa, *xae, *xb, *xbe, *xc, *xc0;
	unsigned int	z2;

	if (a->wds < b->wds) {	/* order so a is biggest */
		c = a;
		a = b;
		b = c;
	}
	k = a->k;
	wa = a->wds;
	wb = b->wds;
	wc = wa + wb;
	if (wc > a->maxwds)
		k++;
	c = Balloc(k);
	for (x = c->x, xa = x + wc; x < xa; x++)
		*x = 0;
	/* find endpoints */
	xa = a->x;
	xae = xa + wa;
	xb = b->x;
	xbe = xb + wb;
	xc0 = c->x;

	/* Step through b in words multiplying each half word by a
	 * and accumulating the result in c.
	 */
	for (; xb < xbe; xb++, xc0++) {
		if (y = *xb & 0xffff) {	/* low half word */
			x = xa;
			xc = xc0;
			carry = 0;
			/* multiply by a */
			do {
				z = (*x & 0xffff) * y + (*xc & 0xffff) + carry;
				carry = z >> 16;
				z2 = (*x++ >> 16) * y + (*xc >> 16) + carry;
				carry = z2 >> 16;
				Storeinc(xc, z2, z);
			} while (x < xae);
			*xc = carry;
		}
		if (y = *xb >> 16) {	/* high half word */
			x = xa;
			xc = xc0;
			carry = 0;
			z2 = *xc;
			/* multiply by a */
			do {
				z = (*x & 0xffff) * y + (*xc >> 16) + carry;
				carry = z >> 16;
				Storeinc(xc, z, z2);
				z2 = (*x++ >> 16) * y + (*xc & 0xffff) + carry;
				carry = z2 >> 16;
			} while (x < xae);
			*xc = z2;
		}
	}
	/* trim zeroed words */
	for (xc0 = c->x, xc = xc0 + wc; wc > 0 && !*--xc; --wc) 
		;
	c->wds = wc;
	return (c);
}


/* Multiply Bigint by 5^k.
 * Calculate additional powers of 5 as necessary.
 */
static Bigint *
pow5mult(Bigint *b, int k)
{
	Bigint		*b1, *p51;
	int		i;
	static int	p05[3] = { 5, 25, 125 };
	static Bigint	p5s = { 0, 0, 1, 0, 1, 625 };
	Bigint		*p5 = &p5s;

	if (i = k & 3)
		b = multadd(b, p05[i-1], 0);

	if (!(k >>= 2))
		return (b);
	for (; ; ) {
		if (k & 1) {
			b1 = mult(b, p5);
			Bfree(b);
			b = b1;
		}
		if (!(k >>= 1))
			break;
		/* get next power of 5 or calculate it */
		if (!(p51 = p5->next)) {
			p51 = p5->next = mult(p5, p5);	/* next power */
			p51->next = 0;
		}
		p5 = p51;
	}
	return (b);
}


/* Left shift Bigint and return result in new Bigint.
 */
static Bigint *
lshift(Bigint *b, int k)
{
	int		i, k1, n, n1;
	Bigint		*b1;
	unsigned int	*x, *x1, *xe, z;

	n = k >> 5;
	k1 = b->k;
	n1 = n + b->wds + 1;
	for (i = b->maxwds; n1 > i; i <<= 1)
		k1++;
	b1 = Balloc(k1);
	x1 = b1->x;
	for (i = 0; i < n; i++)
		*x1++ = 0;
	x = b->x;
	xe = x + b->wds;
	if (k &= 0x1f) {
		k1 = 32 - k;
		z = 0;
		do {
			*x1++ = *x << k | z;
			z = *x++ >> k1;
		} while (x < xe);
		if (*x1 = z)
			++n1;
	} else 
		do
			*x1++ = *x++;
		while (x < xe);
	b1->wds = n1 - 1;
	Bfree(b);
	return (b1);
}


/* Compare two Bigints return -1, 0 or 1 (a <, ==, > b).
 */
static int
cmp(Bigint *a, Bigint *b)
{
	unsigned int	*xa, *xa0, *xb, *xb0;
	int		i, j;

	i = a->wds;
	j = b->wds;

	Bug((i > 1 && !a->x[i-1]), "cmp called with a->x[a->wds-1] == 0");
	Bug((j > 1 && !b->x[j-1]), "cmp called with b->x[b->wds-1] == 0");

	if (i -= j)
		return (i);
	xa0 = a->x;
	xa = xa0 + j;
	xb0 = b->x;
	xb = xb0 + j;
	for (; ; ) {
		if (*--xa != *--xb)
			return (*xa < *xb ? -1 : 1);
		if (xa <= xa0)
			break;
	}
	return (0);
}


/* Subtract two Bigints and return difference as new Bigint.
 * (a and b are close)
 */
static Bigint *
diff(Bigint *a, Bigint *b)
{
	Bigint 		*c;
	int		i, wa, wb;
	int		borrow, y;	/* We need signed shifts here. */
	unsigned int	*xa, *xae, *xb, *xbe, *xc;
	int		z;

	i = cmp(a, b);
	if (!i) {	/* a == b */
		c = Balloc(0);
		c->wds = 1;
		c->x[0] = 0;
		return (c);
	}
	if (i < 0) {	/* a < b so swap and make result -ve */
		c = a;
		a = b;
		b = c;
		i = 1;
	} else
		i = 0;
	c = Balloc(a->k);
	c->sign = i;
	wa = a->wds;
	xa = a->x;
	xae = xa + wa;
	wb = b->wds;
	xb = b->x;
	xbe = xb + wb;
	xc = c->x;
	borrow = 0;
	/* subtract b from a */
	do {
		y = (*xa & 0xffff) - (*xb & 0xffff) + borrow;
		borrow = y >> 16;
		Sign_Extend(borrow, y);
		z = (*xa++ >> 16) - (*xb++ >> 16) + borrow;
		borrow = z >> 16;
		Sign_Extend(borrow, z);
		Storeinc(xc, z, y);
	} while (xb < xbe);
	/* propogate borrow through rest of a */
	while (xa < xae) {
		y = (*xa & 0xffff) + borrow;
		borrow = y >> 16;
		Sign_Extend(borrow, y);
		z = (*xa++ >> 16) + borrow;
		borrow = z >> 16;
		Sign_Extend(borrow, z);
		Storeinc(xc, z, y);
	}
	/* trim zeroed words */
	while (!*--xc)
		wa--;
	c->wds = wa;
	return (c);
}


/* Work out the smallest part (power of two) of double.
 */
static double
ulp(double x)
{
	register int	L;
	double		a;

	/* drop exponent to that of least significant bit */
	L = (word0(x) & EXP_MASK) - (DBL_MANT_DIG - 1) * EXP_MSK1;

#ifndef SUDDEN_UNDERFLOW
	/* is it still normalised ? */
	if (L > 0) {
#endif	/* !SUDDEN_UNDERFLOW */

		word0(a) = L;	/* fix exponent */
		word1(a) = 0;	/* mantissa is zero (use hidden bit) */

#ifndef SUDDEN_UNDERFLOW
	} else {
		/* value is denormalised so we need to calculate
		 * the new (single bit) mantissa (exponent is zero).
		 */
		L = -L >> EXP_SHIFT;
		if (L < EXP_SHIFT) {	/* bit in upper word */
			word0(a) = 0x80000 >> L;
			word1(a) = 0;
		} else {		/* bit in lower word */
			word0(a) = 0;
			L -= EXP_SHIFT;
			word1(a) = L >= 31 ? 1 : 1 << 31 - L;
		}
	}
#endif	/* !SUDDEN_UNDERFLOW */

	return (a);
}


/* Convert Bigint to double.
 */
static double
b2d(Bigint *a, int *e)
{
	unsigned int	*xa, *xa0, w, y, z;
	int		k;
	double		d;

#define d0 word0(d)
#define d1 word1(d)

	xa0 = a->x;
	xa = xa0 + a->wds;
	y = *--xa;	/* most significant bits */

	Bug((!y), "zero y in b2d");

	k = hi0bits(y);
	*e = 32 - k;
	/* is d0 (less sign+exponent bits) too small for bits in y ? */
	if (k < EBITS) {
		/* d0 is what will fit of y
		 * d1 is overflow of y plus the next Bigint word
		 */
		d0 = EXP_1 | (y >> EBITS - k);
		w = xa > xa0 ? *--xa : 0;
		d1 = (y << 32 - EBITS + k) | (w >> EBITS - k);
	} else {
		z = xa > xa0 ? *--xa : 0;
		if (k -= EBITS) {
			/* d0 is y plus what will fit of next Bigint word
			 * d1 is overflow plus the next Bigint word
			 */
			d0 = EXP_1 | (y << k) | (z >> 32 - k);
			y = xa > xa0 ? *--xa : 0;
			d1 = (z << k) | (y >> 32 - k);
		} else {
			/* exact fit of y in d0, next word in d1 */
			d0 = EXP_1 | y;
			d1 = z;
		}
	}

#undef d0
#undef d1

	return (d);
}


/* Convert double to Bigint returning exponent and bits in new Bigint.
 */
static Bigint *
d2b(double d, int *e, int *bits)
{
	Bigint 		*b;
	int		de, i, k;
	unsigned int	*x, y, z;

#define d0 word0(d)
#define d1 word1(d)

	b = Balloc(1);
	x = b->x;

	z = d0 & FRAC_MASK;
	d0 &= 0x7fffffff;	/* clear sign bit, which we ignore */

#ifdef SUDDEN_UNDERFLOW
	de = (int)(d0 >> EXP_SHIFT);
	z |= EXP_MSK11;
#else	/* SUDDEN_UNDERFLOW */
	if (de = (int)(d0 >> EXP_SHIFT))
		z |= EXP_MSK1;	/* add in the hidden bit (normalised) */
#endif	/* SUDDEN_UNDERFLOW */

	/* any bits in d1 ? */
	if (y = d1) {
		/* fill low word borrowing from d0 if necessary */
		if (k = lo0bits(&y)) {
			x[0] = y | (z << 32 - k);
			z >>= k;
		} else
			x[0] = y;
		i = b->wds = (x[1] = z) ? 2 : 1;
	} else {

		Bug((!z), "Zero passed to d2b");
		/* d1 is empty so just use frac part of d0 */
		k = lo0bits(&z);
		x[0] = z;
		i = b->wds = 1;
		k += 32;
	}

#ifndef SUDDEN_UNDERFLOW
	if (de) {	/* was there an exponent ? */
#endif	/* !SUDDEN_UNDERFLOW */

		*e = de - BIAS - (DBL_MANT_DIG - 1) + k;
		*bits = DBL_MANT_DIG - k;

#ifndef SUDDEN_UNDERFLOW
	} else {
		*e = de - BIAS - (DBL_MANT_DIG - 1) + 1 + k;	/* hidden bit */
		*bits = 32 * i - hi0bits(x[i-1]);
	}
#endif	/* !SUDDEN_UNDERFLOW */

#undef d0
#undef d1

	return (b);
}


/* Compute ratio of two Bigints as a double.
 */
static double
ratio(Bigint *a, Bigint *b)
{
	double	da, db;
	int	k, ka, kb;

	da = b2d(a, &ka);
	db = b2d(b, &kb);
	k = ka - kb + 32 * (a->wds - b->wds);	/* find diff in exponents */

	if (k > 0)
		word0(da) += k * EXP_MSK1;	/* scale a up */
	else {
		k = -k;
		word0(db) += k * EXP_MSK1;	/* scale b up */
	}

	return (da / db);
}


/* The following definition assumes the compiler generates
 * correct (ie exact) code for the given double values.
 */
static double	tens[] = {
	1e0, 1e1, 1e2, 1e3, 1e4, 1e5, 1e6, 1e7, 1e8, 1e9,
	1e10, 1e11, 1e12, 1e13, 1e14, 1e15, 1e16, 1e17, 1e18, 1e19,
	1e20, 1e21, 1e22
};

#define n_bigtens 5
static double	bigtens[] = { 1e16, 1e32, 1e64, 1e128, 1e256 };
static double	tinytens[] = { 1e-16, 1e-32, 1e-64, 1e-128, 1e-256 };


/* Compute quotient and remainder of two Bigints.
 * Remainder is returned in b, return value is quotient.
 * Assume b and S are reasonably close.
 */
static int
quorem(Bigint *b, Bigint *S)
{
	int		n;
	int		borrow, y;
	unsigned int	carry, q, ys;
	unsigned int	*bx, *bxe, *sx, *sxe;
	int		z;
	unsigned int	si, zs;

	n = S->wds;

	Bug((b->wds > n), "oversize b in quorem");

	if (b->wds < n)		/* divisor is bigger than dividend */
		return (0);
	sx = S->x;
	sxe = sx + --n;
	bx = b->x;
	bxe = bx + n;
	q = *bxe / (*sxe + 1);	/* ensure q <= true quotient */

	Bug((q > DIGITSINLONG), "oversized quotient in quorem");

	/* first estimate of remainder is b = b - S * q */
	if (q) {
		borrow = 0;
		carry = 0;
		do {		/* subtract q * S from b */
			si = *sx++;
			ys = (si & 0xffff) * q + carry;
			zs = (si >> 16) * q + (ys >> 16);
			carry = zs >> 16;
			y = (*bx & 0xffff) - (ys & 0xffff) + borrow;
			borrow = y >> 16;
			Sign_Extend(borrow, y);
			z = (*bx >> 16) - (zs & 0xffff) + borrow;
			borrow = z >> 16;
			Sign_Extend(borrow, z);
			Storeinc(bx, z, y);
		} while (sx <= sxe);
		/* trim zeroed words */
		if (!*bxe) {
			bx = b->x;
			while (--bxe > bx && !*bxe)
				--n;
			b->wds = n;
		}
	}
	/* can we divide (once) again ? */
	if (cmp(b, S) >= 0) {
		q++;
		borrow = 0;
		carry = 0;
		bx = b->x;
		sx = S->x;
		do {		/* subtract S from b */
			si = *sx++;
			ys = (si & 0xffff) + carry;
			zs = (si >> 16) + (ys >> 16);
			carry = zs >> 16;
			y = (*bx & 0xffff) - (ys & 0xffff) + borrow;
			borrow = y >> 16;
			Sign_Extend(borrow, y);
			z = (*bx >> 16) - (zs & 0xffff) + borrow;
			borrow = z >> 16;
			Sign_Extend(borrow, z);
			Storeinc(bx, z, y);
		} while (sx <= sxe);
		bx = b->x;
		bxe = bx + n;
		/* trim zeroed words */
		if (!*bxe) {
			while (--bxe > bx && !*bxe)
				--n;
			b->wds = n;
		}
	}
	return (q);
}


/* get rid of any predefinitions */
#undef	CHAR
#undef	ISSPACE
#undef	ISDIGIT

#include <wchar.h>
#include <pdsc.h>
#include <machine/fpu.h>

#define CHAR	wchar_t
#define ISSPACE	iswspace
#define ISDIGIT	iswdigit

/* arg to identify I want my callers type */
#define MY_CALLER     1

double
wcstod(const wchar_t *s00, wchar_t **se)

/* wcstod for IEEE-, VAX-, and IBM-arithmetic machines.
 *
 * This wcstod returns a nearest machine number to the input decimal
 * string (or sets errno to ERANGE).  With IEEE arithmetic, ties are
 * broken by the IEEE round-even rule.  Otherwise ties are broken by
 * biased rounding (add half and chop).
 *
 * Inspired loosely by William D. Clinger's paper "How to Read Floating
 * Point Numbers Accurately" [Proc. ACM SIGPLAN '90, pp. 92-101].
 *
 * Modifications:
 *
 *	1. We only require IEEE, IBM, or VAX double-precision
 *		arithmetic (not IEEE double-extended).
 *	2. We get by with floating-point arithmetic in a case that
 *		Clinger missed -- when we're computing d * 10^n
 *		for a small integer d and the integer n is not too
 *		much larger than 22 (the maximum integer k for which
 *		we can represent 10^k exactly), we may be able to
 *		compute (d*10^k) * 10^(e-k) with just one roundoff.
 *	3. Rather than a bit-at-a-time adjustment of the binary
 *		result in the hard case, we use floating-point
 *		arithmetic to determine the adjustment to within
 *		one bit; only in really hard cases do we need to
 *		compute a second residual.
 *	4. Because of 3., we don't need a large table of powers of 10
 *		for ten-to-e (just some small tables, e.g. of 10^k
 *		for 0 <= k <= 22).
 */
{
	int	e, e1, esign, i, j, k, nz, sign;
	int	mant_len;	/* digits in mantissa */
	int	mant_int_len;	/* digits in integer part of mantissa */
	int	mant_frac_len;	/* digits in fraction part of mantissa */
	int	lead_zeros;	/* true if leading zeros */

	const CHAR	*s = s00, *s_tmp;
	const CHAR	*first_nonzero;
	CHAR		c;
	double		rv, rv0;
	int		L;
	unsigned int	upper, lower;
	Bigint 		*bb, *bb1, *bd, *bd0, *bs, *delta;
	char		*radix;
	size_t		radix_size;
#define	RADIX_LEN	4	/* ASSUMPTION: 4 chars is enough */
	CHAR		wc_radix[RADIX_LEN + 1];
	fp_register_t   fp;     /* union of float formats */
	unsigned long   type;   /* of fp result caller expects */

	sign = lead_zeros = nz = 0;
	rv = 0.;

	/* Skip white space.
	 */
	while (ISSPACE(*s)) s++;

	/* Check for mantissa sign.
	 */
	switch (*s) {
	case '-'	:
		sign = 1;
		/* FALLTHROUGH */
	case '+'	:
		s++;
		break;
	case '\0'	:
		s = s00;
		goto ret_nolock;
	}

	/* Skip leading zeros.
	 */
	if (*s == '0') {
		lead_zeros = 1;
		while (*++s == '0') 
			;
		if (!*s)
			goto ret_nolock;
	}

	/* Read in integer part of mantissa (pre-radix).
	 */
	first_nonzero = s;	/* assume start of mantissa */
	mant_frac_len = 0;
	upper = lower = 0;
	for (mant_len = 0; ISDIGIT(c = *s); mant_len++, s++)
		if (mant_len < DIGITSINLONG)	/* first long */
			upper = 10 * upper + c - '0';
		else if (mant_len < DBL_DIG + 1)	/* beyond precision */
			lower = 10 * lower + c - '0';
	mant_int_len = mant_len;

	/* Check for radix and skip it before reading
	 * fractional part of mantissa (post-radix).
	 */
	if ((radix = __lc_locale->nl_info[RADIXCHAR]) == 0 || !*radix)
		radix = ".";	/* X/Open default */
	/* Convert radix chars to wide chars,
	 * match against current input and skip it.
	 */
	if ((radix_size = mbstowcs(wc_radix, radix, RADIX_LEN + 1))
	    <= RADIX_LEN && !wcsncmp(wc_radix, s, radix_size))
	{
		c = *(s += radix_size);

		if (mant_len == 0) {	/* mantissa is zero so far */
			for (; c == '0'; c = *++s)	/* eat zeros */
				nz++;
			if (ISDIGIT(c)) {
				first_nonzero = s;
				mant_frac_len += nz;	/* put back zeros */
				nz = 0;
				goto have_dig;
			}
			goto dig_done;
		}

		for (; ISDIGIT(c); c = *++s) {
have_dig:
			nz++;
			if (c -= '0') {
				mant_frac_len += nz;
				for (i = 1; i < nz; i++, mant_len++)
					if (mant_len < DIGITSINLONG)
						upper *= 10;
					else if (mant_len < DBL_DIG + 1)
						lower *= 10;
				if (mant_len++ < DIGITSINLONG)
					upper = 10 * upper + c;
				else if (mant_len <= DBL_DIG + 1)
					lower = 10 * lower + c;
				nz = 0;
			}
		}
	}
dig_done:

	/* Read in exponent.
	 */
	e = 0;
	if (c == 'e' || c == 'E') {
		if (!mant_len && !nz && !lead_zeros) {
			s = s00;
			goto ret_nolock;
		}
		s00 = s;	/* number is ok up to here */
		esign = 0;
		switch (c = *++s) {
		case '-':
			esign = 1;
			/* FALLTHROUGH */
		case '+':
			c = *++s;
			break;
		}
		if (ISDIGIT(c)) {
			while (c == '0')
				c = *++s;
			if (ISDIGIT(c)) {
				L = c - '0';
				s_tmp = s;
				while (ISDIGIT(c = *++s))
					L = 10 * L + c - '0';
				if (s - s_tmp > DIGITSINLONG || L > 19999)

					/* Avoid confusion from exponents
					 * so large that e might overflow.
					 */
					e = 19999; /* safe for 16 bit ints */
				else
					e = (int)L;
				if (esign)
					e = -e;
			} else
				e = 0;
		} else
			s = s00;
	}
	if (!mant_len) {
		if (!nz && !lead_zeros)
			s = s00;
		goto ret_nolock;
	}
	e1 = (e -= mant_frac_len);	/* adjust exponent */
	k = mant_len < DBL_DIG + 1 ? mant_len : DBL_DIG + 1;
	rv = upper;
	if (k > DIGITSINLONG)
		rv = tens[k - DIGITSINLONG] * rv + lower;

	/* Now we have rv as a double representation of the most
	 * significant digits in the number, the exponent is held
	 * separately. We need to combine them.
	 *
	 * Check to see we can do that directly without losing accuracy.
	 */
	if (mant_len <= DBL_DIG

#ifndef RND_PRODQUOT
	     && FLT_ROUNDS == 1
#endif	/* RND_PRODQUOT */

	    ) {
		if (!e)
			goto ret_nolock;
		if (e > 0) {
			/* Is the exponent an exact double ? */
			if (e <= EXACTPOWTEN) {
				Rounded_product(rv, tens[e]);
				goto ret_nolock;
			}
			/* Can the exponent be scaled down to an exact double
			 * by scaling the mantissa up ?
			 */
			i = DBL_DIG - mant_len;
			if (e <= EXACTPOWTEN + i) {
				/* A fancier test would sometimes let us do
				 * this for larger i values.
				 */
				e -= i;		/* exponent <= EXACTPOWTEN */
				rv *= tens[i];	/* scale mantissa up */
				Rounded_product(rv, tens[e]);
				goto ret_nolock;
			}
		}
#ifndef INNACCURATE_DIVIDE
		/* Is the exponent an exact double ? */
		else if (e >= -EXACTPOWTEN) {
			Rounded_quotient(rv, tens[-e]);
			goto ret_nolock;
		}
#endif	/* !INNACCURATE_DIVIDE */
	    }

	/* We can't scale rv exactly so we go the long way around.
	 *
	 * First, scale rv anyway and get an approximate result.
	 */
	e1 += mant_len - k;

	if (e1 > 0) {
		if (i = e1 & 0xf)
			rv *= tens[i];
		if (e1 &= ~0xf) {
			if (e1 > DBL_MAX_10_EXP) {	/* too big */
ovfl:
				_Seterrno(ERANGE);
				rv = HUGE_VAL;
				goto ret_nolock;
			}
			if (e1 >>= 4) {
				int	exp_tmp;

				for (j = 0; e1 > 1; j++, e1 >>= 1)
					if (e1 & 1)
						rv *= bigtens[j];
				/* Scale down by a pow of 2 bigger than rv
				 * so we don't overflow (2^DBL_MANT_DIG)
				 */
				word0(rv) -= DBL_MANT_DIG * EXP_MSK1;
				rv *= bigtens[j];
				if ((exp_tmp = word0(rv) & EXP_MASK)
				     > EXP_MSK1
				       * (DBL_MAX_EXP+BIAS - DBL_MANT_DIG))
					goto ovfl;
				if (exp_tmp
				    > EXP_MSK1
				      * (DBL_MAX_EXP+BIAS - 1 - DBL_MANT_DIG)) {
					/* set to largest number */
					/* (Can't trust DBL_MAX) */
					word0(rv) = BIG0;
					word1(rv) = BIG1;
				} else
					/* Ok, scale back up */
					word0(rv) += DBL_MANT_DIG * EXP_MSK1;
			}

		}
	} else if (e1 < 0) {
		e1 = -e1;
		if (i = e1 & 0xf)
			rv /= tens[i];
		if (e1 &= ~0xf) {
			e1 >>= 4;
			if (e1 > (1 << n_bigtens)) {	/* too small */
undfl:
				_Seterrno(ERANGE);
				rv = 0.;
				goto ret_nolock;
			}
			for (j = 0; e1 > 1; j++, e1 >>= 1)
				if (e1 & 1)
					rv *= tinytens[j];
			/* could underflow */
			rv0 = rv;
			rv *= tinytens[j];
			if (!rv) {
				rv = 2. * rv0;
				rv *= tinytens[j];
				if (!rv)
					goto undfl;
				word0(rv) = TINY0;
				word1(rv) = TINY1;
				/* The refinement below will clean
				 * this approximation up.
				 */
			}
		}
	}

	TS_LOCK(&_atof_rmutex);	/* lock rest of function */

	/* Now the hard part -- adjusting rv to the correct value.
	 */

	/* Put digits into an arbitrary precision representation
	 * Bigint. Convert from the first non-zero char up to the radix,
	 * and then the rest. The first word of the Bigint is initialised
	 * with the upper word read in earlier. The rest of the digits in
	 * the string are added.
	 */
	if (mant_int_len == 0)
		mant_int_len = mant_len;

	{	/* allocate big enough Bigint */
		int	ksize;
		int	x, y;

		x = (mant_len + DIGITSINLONG - 1) / DIGITSINLONG;
		for (ksize = 0, y = 1; x > y; y <<= 1, ksize++) 
			;
		bd0 = Balloc(ksize);
	}

	/* initialise */
	bd0->x[0] = upper;
	bd0->wds = 1;

	/* add the part before the radix */
	if ((i = DIGITSINLONG) < mant_int_len) {
		first_nonzero += DIGITSINLONG;
		do {
			bd0 = multadd(bd0, 10, *first_nonzero++ - '0');
		} while (++i < mant_int_len);
		first_nonzero += radix_size;			/* skip radix */
	} else
		first_nonzero += DIGITSINLONG + radix_size;	/* skip radix */

	/* add the rest */
	for (; i < mant_len; i++)
		bd0 = multadd(bd0, 10, *first_nonzero++ - '0');

	/* Loop calculating how much rv differs from the exact representation
	 * and adjusting rv in the right direction.
	 *
	 * bd0 is the exact number
	 * bb is an exact representation of rv
	 * bs is the slop
	 */
	for (;;) {
		unsigned int	y, z;
		double		aadj, aadj1, adj;
		int		bb2, bb5, bbe, bd2, bd5, bbbits, bs2, dsign;

		bd = Balloc(bd0->k);
		Bcopy(bd, bd0);
		bb = d2b(rv, &bbe, &bbbits);	/* rv = bb * 2^bbe */
		bs = i2b(1);

		if (e >= 0) {
			bb2 = bb5 = 0;
			bd2 = bd5 = e;
		} else {
			bb2 = bb5 = -e;
			bd2 = bd5 = 0;
		}
		if (bbe >= 0)
			bb2 += bbe;
		else
			bd2 -= bbe;
		bs2 = bb2;

#ifdef SUDDEN_UNDERFLOW
		j = DBL_MANT_DIG + 1 - bbbits;
#else	/* SUDDEN_UNDERFLOW */
		i = bbe + bbbits - 1;	/* logb(rv) */
		if (i < EMIN)	/* denormal */
			j = bbe + (DBL_MANT_DIG - EMIN);
		else
			j = DBL_MANT_DIG + 1 - bbbits;
#endif	/* SUDDEN_UNDERFLOW */

		bb2 += j;
		bd2 += j;
		i = bb2 < bd2 ? bb2 : bd2;
		if (i > bs2)
			i = bs2;
		if (i > 0) {
			bb2 -= i;
			bd2 -= i;
			bs2 -= i;
		}
		if (bb5 > 0) {
			bs = pow5mult(bs, bb5);
			bb1 = mult(bs, bb);
			Bfree(bb);
			bb = bb1;
		}
		if (bb2 > 0)
			bb = lshift(bb, bb2);
		if (bd5 > 0)
			bd = pow5mult(bd, bd5);
		if (bd2 > 0)
			bd = lshift(bd, bd2);
		if (bs2 > 0)
			bs = lshift(bs, bs2);
		delta = diff(bb, bd);
		dsign = delta->sign;
		delta->sign = 0;
		i = cmp(delta, bs);
		if (i < 0) {
			/* Error is less than half an ulp -- check for
			 * special case of mantissa a power of two.
			 */
			if (dsign || word1(rv) || word0(rv) & BNDRY_MASK)
				break;
			delta = lshift(delta, LOG2P);
			if (cmp(delta, bs) > 0)
				goto drop_down;
			break;
		}
		if (i == 0) {
			/* exactly half-way between */
			if (dsign) {
				if ((word0(rv) & BNDRY_MASK1) == BNDRY_MASK1
				     &&  word1(rv) == 0xffffffff) {
					/*boundary case -- increment exponent*/
					word0(rv) = (word0(rv) & EXP_MASK)
						    + EXP_MSK1;
					word1(rv) = 0;
					break;
				}
			} else if (!(word0(rv) & BNDRY_MASK) && !word1(rv)) {
drop_down:
				/* boundary case -- decrement exponent */

#ifdef SUDDEN_UNDERFLOW
				L = word0(rv) & EXP_MASK;
				if (L <= EXP_MSK1)
					goto undfl;
				L -= EXP_MSK1;
#else	/* SUDDEN_UNDERFLOW */
				L = (word0(rv) & EXP_MASK) - EXP_MSK1;
#endif	/* SUDDEN_UNDERFLOW */

				word0(rv) = L | BNDRY_MASK1;
				word1(rv) = 0xffffffff;
				break;
			}
#ifndef ROUND_BIASED
			if (!(word1(rv) & LSB))
				break;
#endif	/* !ROUND_BIASED */
			if (dsign)
				rv += ulp(rv);
#ifndef ROUND_BIASED
			else {
				rv -= ulp(rv);
#ifndef SUDDEN_UNDERFLOW
				if (!rv)
					goto undfl;
#endif	/* !SUDDEN_UNDERFLOW */
			}
#endif	/* !ROUND_BIASED */

			break;
		}
		if ((aadj = ratio(delta, bs)) <= 2.) {	/* bs is never 0 */
			if (dsign)
				aadj = aadj1 = 1.;
			else if (word1(rv) || word0(rv) & BNDRY_MASK) {
#ifndef SUDDEN_UNDERFLOW
				if (word1(rv) == TINY1 && !word0(rv))
					goto undfl;
#endif	/* !SUDDEN_UNDERFLOW */
				aadj = 1.;
				aadj1 = -1.;
			} else {
				/* special case -- power of FLT_RADIX to be */
				/* rounded down... */

				if (aadj < 2. / FLT_RADIX)
					aadj = 1. / FLT_RADIX;
				else
					aadj *= 0.5;
				aadj1 = -aadj;
			}
		} else {
			aadj *= 0.5;
			aadj1 = dsign ? aadj : -aadj;
#ifdef CHECK_FLT_ROUNDS
			switch (FLT_ROUNDS) {
			case 2: /* towards +infinity */
				aadj1 -= 0.5;
				break;
			case 0: /* towards 0 */
			case 3: /* towards -infinity */
				aadj1 += 0.5;
			}
#else	/* CHECK_FLT_ROUNDS */
			if (FLT_ROUNDS == 0)
				aadj1 += 0.5;
#endif	/* CHECK_FLT_ROUNDS */
		}
		y = word0(rv) & EXP_MASK;

		/* Check for overflow */

		if (y == EXP_MSK1 * (DBL_MAX_EXP + BIAS - 1)) {
			rv0 = rv;
			word0(rv) -= DBL_MANT_DIG * EXP_MSK1;
			adj = aadj1 * ulp(rv);
			rv += adj;
			if ((word0(rv) & EXP_MASK) >= 
			    EXP_MSK1 * (DBL_MAX_EXP + BIAS - DBL_MANT_DIG)) {
				if (word0(rv0) == BIG0 && word1(rv0) == BIG1)
					goto ovfl;
				word0(rv) = BIG0;
				word1(rv) = BIG1;
				goto cont;
			} else
				word0(rv) += DBL_MANT_DIG * EXP_MSK1;
		} else {
#ifdef SUDDEN_UNDERFLOW
			if ((word0(rv) & EXP_MASK) <= DBL_MANT_DIG * EXP_MSK1) {
				rv0 = rv;
				word0(rv) += DBL_MANT_DIG * EXP_MSK1;
				adj = aadj1 * ulp(rv);
				rv += adj;
				if ((word0(rv) & EXP_MASK)
				    <= DBL_MANT_DIG * EXP_MSK1)
				{
					if (word0(rv0) == TINY0
					     && word1(rv0) == TINY1)
						goto undfl;
					word0(rv) = TINY0;
					word1(rv) = TINY1;
					goto cont;
				} else
					word0(rv) -= DBL_MANT_DIG * EXP_MSK1;
			} else {
				adj = aadj1 * ulp(rv);
				rv += adj;
			}
#else	/* SUDDEN_UNDERFLOW */
			/* Compute adj so that the IEEE rounding rules will
			 * correctly round rv + adj in some half-way cases.
			 * If rv * ulp(rv) is denormalized (i.e.,
			 * y <= (DBL_MANT_DIG-1)*EXP_MSK1),
			 * we must adjust aadj to avoid
			 * trouble from bits lost to denormalization;
			 * example: 1.2e-307 .
			 */
			if (y <= (DBL_MANT_DIG - 1) * EXP_MSK1 && aadj >= 1.) {
				aadj1 = (double)(int)(aadj + 0.5);
				if (!dsign)
					aadj1 = -aadj1;
			}
			adj = aadj1 * ulp(rv);
			rv += adj;
#endif	/* SUDDEN_UNDERFLOW */
		}
		z = word0(rv) & EXP_MASK;
		if (y == z) {
			/* Can we stop now? */
			L = aadj;
			aadj -= L;
			/* The tolerances below are conservative. */
			if (dsign || word1(rv) || word0(rv) & BNDRY_MASK) {
				if (aadj < .4999999 || aadj > .5000001)
					break;
			} else if (aadj < .4999999 / FLT_RADIX)
				break;
		}
cont:
		Bfree(bb);
		Bfree(bd);
		Bfree(bs);
		Bfree(delta);
	}
	Bfree(bb);
	Bfree(bd);
	Bfree(bs);
	Bfree(bd0);
	Bfree(delta);
ret:
	TS_UNLOCK(&_atof_rmutex);

ret_nolock:	/* return before lock is taken */

	if (se)
		*se = (CHAR *)s;

	if (sign)
		rv = -rv;

	type = __exc_get_fp_type(MY_CALLER);
	fp.dval = rv;
 
	if (fp.qval == IEEE_PLUS_INFINITY || fp.qval == IEEE_MINUS_INFINITY) {
 
		switch (type) {
		case PDSC_EXC_SILENT:               /* under->0, no signal */
		case PDSC_EXC_SIGNAL:               /* under->0, signal rest */
		case PDSC_EXC_SIGNAL_ALL:           /* signal all */
			if (fp.qval == IEEE_PLUS_INFINITY) {
				rv = DBL_MAX;
			} else if (fp.qval == IEEE_MINUS_INFINITY) {
				rv = -DBL_MAX;
			} /* if */
			break;
 
		case PDSC_EXC_IEEE: /* default values are ok */
			break;
		} /* switch */
 
	} /* if */

	return (rv);
}
