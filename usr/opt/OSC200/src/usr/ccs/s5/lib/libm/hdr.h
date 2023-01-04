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
 *	@(#)$RCSfile: hdr.h,v $ $Revision: 4.2.3.4 $ (DEC) $Date: 1992/05/05 16:31:14 $
 */ 
/*
 */


#include <float.h>
#include <math.h>
#include <machine/endian.h>

#if defined(vax)||defined(tahoe)||defined(mmax)
#	include <errno.h>
extern double infnan(int);
#endif

#ifdef mmax
#define national	1
#endif

/*
 * Define indices into the mantissa side and the exponent side of a fp number.
 *	n0 is the exponent side, n1 is the "other" part
 */
#if BYTE_ORDER == LITTLE_ENDIAN
enum long_order{n0=1,n1=0};
#else
enum long_order{n0=0,n1=1};
#endif

#if defined(vax) || defined(tahoe)
#	define	_0x(A,B)	0x##A##B
#	define  NAN(somenum)	0
#else
#	define	_0x(A,B)	0x##B##A
#	define	NAN(somenum)	(somenum != somenum)
#endif

/*
 * MEXP(lvalue) extracts the bits of exponent from a FP value
 */
#if defined(vax) || defined(tahoe)
#	define MEXP(lvalue) (((long int*) &lvalue)[n0] & 0x7f800000 )
#	define RESERVED(lvalue) ((((long int*) &lvalue)[n0] & 0xff800000) == 0x80000000)
#else
#	define MEXP(lvalue) (((long int*) &lvalue)[n0] & 0x7ff00000 )
#	define RESERVED(lvalue) (MEXP(lvalue) == 0x7ff00000 )
#endif

/*
 * SIGN(lvalue) isolates the sign bit but doesn't move it around
 */
#define SIGN(lvalue) (((long int*) &lvalue)[n0] & 0x80000000 )


/*
 * Define some magic constants for various architectures....
 */

#if defined(vax)||defined(tahoe)	/* VAX D format */

/* static double */
/* ln2hi  =  6.9314718055829871446E-1    , Hex  2^  0   *  .B17217F7D00000 */
/* ln2lo  =  1.6465949582897081279E-12   ; Hex  2^-39   *  .E7BCD5E4F1D9CC */
static long     ln2hix[] = { _0x(7217,4031), _0x(0000,f7d0)};
static long     ln2lox[] = { _0x(bcd5,2ce7), _0x(d9cc,e4f1)};
#define    ln2hi    (*(double*)ln2hix)
#define    ln2lo    (*(double*)ln2lox)

#else	/* defined(vax)||defined(tahoe) */

static double
ln2hi  =  6.9314718036912381649E-1    , /*Hex  2^ -1   *  1.62E42FEE00000 */
ln2lo  =  1.9082149292705877000E-10   ; /*Hex  2^-33   *  1.A39EF35793C76 */

#endif	/* defined(vax)||defined(tahoe) */

/*
 * Constants:
 * The hexadecimal values are the intended ones for the following constants.
 * The decimal values may be used, provided that the compiler will convert
 * from decimal to binary accurately enough to produce the hexadecimal values
 * shown.
 */

#if defined(vax)||defined(tahoe)	/* VAX D format */

/* static double */
/* r2p1hi =  2.4142135623730950345E0     , Hex  2^  2   *  .9A827999FCEF32 */
/* r2p1lo =  1.4349369327986523769E-17   , Hex  2^-55   *  .84597D89B3754B */
/* sqrt2  =  1.4142135623730950622E0     ; Hex  2^  1   *  .B504F333F9DE65 */
static long    r2p1hix[] = { _0x(8279,411a), _0x(ef32,99fc)};
static long    r2p1lox[] = { _0x(597d,2484), _0x(754b,89b3)};
static long     sqrt2x[] = { _0x(04f3,40b5), _0x(de65,33f9)};
#define   r2p1hi    (*(double*)r2p1hix)
#define   r2p1lo    (*(double*)r2p1lox)
#define    sqrt2    (*(double*)sqrt2x)
#else	/* defined(vax)||defined(tahoe)	*/
static double
r2p1hi =  2.4142135623730949234E0     , /*Hex  2^1     *  1.3504F333F9DE6 */
r2p1lo =  1.2537167179050217666E-16   , /*Hex  2^-53   *  1.21165F626CDD5 */
sqrt2  =  1.4142135623730951455E0     ; /*Hex  2^  0   *  1.6A09E667F3BCD */
#endif	/* defined(vax)||defined(tahoe)	*/


#if defined(vax)||defined(tahoe)
/* static double  */
/* mln2hi =  8.8029691931113054792E1     , Hex  2^  7   *  .B00F33C7E22BDB */
/* mln2lo = -4.9650192275318476525E-16   , Hex  2^-50   * -.8F1B60279E582A */
/* lnovfl =  8.8029691931113053016E1     ; Hex  2^  7   *  .B00F33C7E22BDA */
static long    mln2hix[] = { _0x(0f33,43b0), _0x(2bdb,c7e2)};
static long    mln2lox[] = { _0x(1b60,a70f), _0x(582a,279e)};
static long    lnovflx[] = { _0x(0f33,43b0), _0x(2bda,c7e2)};
#define   mln2hi    (*(double*)mln2hix)
#define   mln2lo    (*(double*)mln2lox)
#define   lnovfl    (*(double*)lnovflx)
#else	/* defined(vax)||defined(tahoe) */
static double
mln2hi =  7.0978271289338397310E2     , /*Hex  2^ 10   *  1.62E42FEFA39EF */
mln2lo =  2.3747039373786107478E-14   , /*Hex  2^-45   *  1.ABC9E3B39803F */
lnovfl =  7.0978271289338397310E2     ; /*Hex  2^  9   *  1.62E42FEFA39EF */
#endif	/* defined(vax)||defined(tahoe) */

extern double scalb ( double, double );
extern double copysign ( double, double );
extern int finite( double );
extern double _log__L(double);
extern double _exp__E(double,double);
