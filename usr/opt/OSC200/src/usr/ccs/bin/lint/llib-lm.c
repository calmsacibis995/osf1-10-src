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
static char	*sccsid = "@(#)$RCSfile: llib-lm.c,v $ $Revision: 4.2.5.2 $ (DEC) $Date: 1993/12/17 21:07:06 $";
#endif 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0
 */

/*
 * COMPONENT_NAME: (CMDPROG) Programming Utilities
 *
 * FUNCTIONS: 
 *
 * ORIGINS: 27; 10
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*NOTUSED*/
/*NOTDEFINED*/

#include <math.h>

int signgam;

/* BESSEL(3M) */
double	j0(x) double x; { return (x); }
double	j1(x) double x; { return (x); }
double	jn(n, x) int n; double x; { return (x); }
double	y0(x) double x; { return (x); }
double	y1(x) double x; { return (x); }
double	yn(n, x) int n; double x; { return (x); }
/* single precision */
float	j0f(x) float x; { return (x); }
float	j1f(x) float x; { return (x); }
float	jnf(n,x) int n; float x; { return (x); }
float	y0f(x) float x; { return (x); }
float	y1f(x) float x; { return (x); }
float	ynf(n,x) int n; float x; { return (x); }
/* ERF(3M) */
double	erf(x) double x; { return (x); }
double	erfc(x) double x; { return (x); }
/* single precision */
float	erff(x) float x; { return (x); }
float	erfcf(x) float x; { return (x); }
/* EXP(3M) */
double	exp(x) double x; { return (x); }
double	expm1(x) double x; { return (x); }
double	log(x) double x; { return (x); }
double	log10(x) double x; { return (x); }
double	pow(x, y) double x; double y;  { return (x); }
double	sqrt(x) double x; { return (x); }

/* Single precision */
float	expf(x) float x; { return (x); }
float	expm1f(x) float x; { return (x); }
float	logf(x) float x; { return (x); }
float	log10f(x) float x; { return (x); }
float	powf(x, y) float x; float y;  { return (x); }
float	sqrtf(x) float x; { return (x); }
/* FLOOR(3M) */
double	floor(x) double x; { return (x); }
double	ceil(x) double x; { return (x); }
double	drem(x, y) double x; double y; { return (x); }
double	fmod(x, y) double x; double y; { return (x); }
double	fabs(x) double x; { return (x); }
double  cabs(x, y) double x; double y; { return (x); }
double	modf(x, n) double x; double *n; { return (x); }
double	remainder(x, y) double x; double y; { return (x); }
double  rint(x) double x; { return (x); }

/* Single precision */
float	floorf(x) float x; { return (x); }
float	ceilf(x) float x; { return (x); }
float	dremf(x, y) float x; float y; { return (x); }
float	fmodf(x, y) float x; float y; { return (x); }
float	fabsf(x) float x; { return (x); }
float   cabsf(x, y) float x; float y; { return (x); }
float	modff(x, n) float x; float *n; { return (x); }
float	remainderf(x, y) float x; float y; { return (x); }
float	rintf(x) float x; { return (x); }
/* FREXP(3M) */
double  frexp(x, i) double x; int *i; { return (x); }
float   frexpf(x, i) float x; int *i; { return (x); }
double  ldexp(x, i) double x; int i; { return (x); }
float   ldexpf(x, i) float x; int i; { return (x); }
double  logb(x) double x; { return (x); }
float   logbf(x) float x; { return (x); }
double  scalb(x, i) double x; int i; { return (x); }
float   scalbf(x, i) float x; int i; { return (x); }
/* GAMMA(3M) */
double	gamma(x) double x; { return (x); }
float	gammaf(x) float x; { return (x); }
double	lgamma(x) double x; { return (x); }
float	lgammaf(x) float x; { return (x); }
/* HYPOT(3M) */
double	hypot(x, y) double x; double y; { return (x); }
float	hypotf(x, y) float x; float y; { return (x); }
/* MATHERR(3M) */
int	matherr(struct exception *p) { return (0); }
/* SINH(3M) */
double	sinh(x) double x; { return (x); }
double	cosh(x) double x; { return (x); }
double	tanh(x) double x; { return (x); }
/* single precision */
float	sinhf(x) float x; { return (x); }
float	coshf(x) float x; { return (x); }
float	tanhf(x) float x; { return (x); }
/* TRIG(3M) */
double	sin(x) double x; { return (x); }
double	cos(x) double x; { return (x); }
double	tan(x) double x; { return (x); }
double	asin(x) double x; { return (x); }
float	acosf(x) float x; { return (x); }
double	acos(x) double x; { return (x); }
double	atan(x) double x; { return (x); }
double	atan2(x, y) double x; double y; { return (x); }

double	asinh(x) double x; { return (x); }
double	acosh(x) double x; { return (x); }
double	atanh(x) double x; { return (x); }
double	cbrt(x) double x; { return (x); }
double	nearest(x) double x; { return (x); }
double	trunc(x) double x; { return (x); }
int	    ilogb(x) double x; { return (0); }
int	    isnan(x) double x; { return (0); }
double	log1p(x) double x; { return (x); }
double	copysign(x, y) double x; double y; { return (x); }
int     finite (x) double x; { return (0); }
int     fp_class (x) double x; { return (x); }
double	nextafter(x, y) double x; double y; { return (x); }
int	    unordered(x, y) double x; double y; { return (0); }
/* single precision */
float	sinf(x) float x; { return (x); }
float	cosf(x) float x; { return (x); }
float	tanf(x) float x; { return (x); }
float	asinf(x) float x; { return (x); }
float	atanf(x) float x; { return (x); }
float	atan2f(x, y) float x; float y; { return (x); }
float	asinhf(x) float x; { return (x); }
float	acoshf(x) float x; { return (x); }
float	atanhf(x) float x; { return (x); }
float	cbrtf(x) float x; { return (x); }
float	truncf(x) float x; { return (x); }
int	    isnanf(x) float x; { return (0); }
float	log1pf(x) float x; { return (x); }
float	copysignf(x, y) float x; float y; { return (x); }
int     finitef(x) float x; { return (0); }
int     fp_classf(x) float x; { return (x); }
float	nextafterf(x, y) float x; double y; { return (x); }
int	    unorderedf(x, y)  float x; float y; { return (0); }

/*
 * FAST Floating Point
 */
double F_acos (x) double x; { return (x); }
double F_asin (x) double x; { return (x); }
double F_atan (x) double x; { return (x); }
double F_atan2 (x, y) double x; double y; { return (x); }
double F_cos (x) double x; { return (x); }
double F_exp (x) double x; { return (x); }
double F_hypot (x, y) double x; double y; { return (x); }
double F_log (x) double x; { return (x); }
double F_log10 (x) double x; { return (x); }
double F_pow (x, y) double x; double y; { return (x); }
double F_sin (x) double x; { return (x); }
double F_sqrt (x) double x; { return (x); }
double F_tan (x) double x; { return (x); }
/*
 * FAST Floating Point single precision
 */
float F_acosf (x) float x; { return (x); }
float F_asinf (x) float x; { return (x); }
float F_atan2f (x, y) float x; float y; { return (x); }
float F_atanf (x) float x; { return (x); }
float F_cosf (x) float x; { return (x); }
float F_expf (x) float x; { return (x); }
float F_hypotf (x, y) float x; float y; { return (x); }
float F_log10f (x) float x; { return (x); }
float F_logf (x) float x; { return (x); }
float F_powf (x, y) float x; float y; { return (x); }
float F_sinf (x) float x; { return (x); }
float F_sqrtf (x) float x; { return (x); }
float F_tanf (x) float x; { return (x); }

/*
 * Additional Digital functions
 */
double acosd (x) double x; { return (x); }
double asind (x) double x; { return (x); }
double atand (x) double x; { return (x); }
double atand2 (x, y) double x; double y; { return (x); }
double cosd (x) double x; { return (x); }
double cot (x) double x; { return (x); }
double cotd (x) double x; { return (x); }
double log2 (x) double x; { return (x); }
double nint (x) double x; { return (x); }
double powi (x, i) double x; int i; { return (x); }
double sind (x) double x; { return (x); }
double tand (x) double x; { return (x); }

float acosdf (x) float x; { return (x); }
float asindf (x) float x; { return (x); }
float atand2f (x, y) float x; float y; { return (x); }
float atandf (x) float x; { return (x); }
float cosdf (x) float x; { return (x); }
float cotdf (x) float x; { return (x); }
float cotf (x) float x; { return (x); }
float log2f (x) float x; { return (x); }
float nintf (x) float x; { return (x); }
float powif ( x, y) float x; int y; { return x; }
float sindf (x) float x; { return (x); }
float tandf (x) float x; { return (x); }

int powii (x, y) int x, y; { return x; }







