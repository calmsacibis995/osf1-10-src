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
 *	@(#)$RCSfile: values.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 05:05:10 $
 */ 
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
 * RESTRICTED RIGHTS LEGEND
 * Use, Duplication or Disclosure by the Government is subject to
 * restrictions as set forth in paragraph (b)(3)(B) of the rights in
 * Technical Data and Computer Software clause in DAR 7-104.9(a).
 */ 

/*
 * COMPONENT_NAME: (values.h) header file of common values
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1988, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#ifndef _VALUES_H_
#define	_VALUES_H_

#include <limits.h>

#define BITSPERBYTE	CHAR_BIT
#define BITS(type)	(BITSPERBYTE * sizeof(type))

/* short, regular and long ints with only the high-order bit turned on */
#define HIBITS	 ((short)(1 << BITS(short) - 1)) 
#define HIBITI	 (1 << BITS(int) - 1) 
#define HIBITL	 (1L << BITS(long) - 1)

/* largest short, regular and long int */
#define MAXSHORT ((short)~HIBITS)
#define MAXINT   (~HIBITI)
#define MAXLONG  (~HIBITL)

/* various values that describe the binary floating-point representation
 * DMAXEXP 	- the maximum exponent of a double (as returned by frexp())
 * FMAXEXP 	- the maximum exponent of a float  (as returned by frexp())
 * DMINEXP 	- the minimum exponent of a double (as returned by frexp())
 * FMINEXP 	- the minimum exponent of a float  (as returned by frexp())
 * MAXDOUBLE	- the largest double
			((_EXPBASE ** DMAXEXP) * (1 - (_EXPBASE ** -DSIGNIF)))
 * MAXFLOAT	- the largest float
			((_EXPBASE ** FMAXEXP) * (1 - (_EXPBASE ** -FSIGNIF)))
 * MINDOUBLE	- the smallest double (_EXPBASE ** (DMINEXP - 1))
 * MINFLOAT	- the smallest float (_EXPBASE ** (FMINEXP - 1))
 * DSIGNIF	- the number of significant bits in a double
 * FSIGNIF	- the number of significant bits in a float
 * DMAXPOWTWO	- the largest power of two exactly representable as a double
 * FMAXPOWTWO	- the largest power of two exactly representable as a float
 * LN_MAXDOUBLE	- the natural log of the largest double  -- log(MAXDOUBLE)
 * LN_MINDOUBLE	- the natural log of the smallest double -- log(MINDOUBLE)
 * _DEXPLEN	- the number of bits for the exponent of a double (11)
 * _FEXPLEN	- the number of bits for the exponent of a float (8)
 *
 *  These values are no longer defined, however, they are reference in other
 *  defines to show how they were calculated.
 *
 * _EXPBASE	- the exponent base (2)
 * _IEEE	- 1 if IEEE standard representation is used (1)
 * _LENBASE     - the number of bits in the exponent base (1 for binary)
 * _HIDDENBIT	- 1 if high-significance bit of mantissa is implicit
 */
/* these are for the IEEE format machines */
#define MAXDOUBLE     1.7976931348623157e+308

#ifndef MAXFLOAT
#define MAXFLOAT      ((float)3.40282346638528860e+38)
#endif

#define MINDOUBLE     4.94065645841246544e-324
#define MINFLOAT      ((float)1.40129846432481707e-45)
#define _IEEE           1
#define _DEXPLEN        11
#define _HIDDENBIT      1
#define DMINEXP       (-(DMAXEXP + DSIGNIF - _HIDDENBIT - 3))
#define FMINEXP       (-(FMAXEXP + FSIGNIF - _HIDDENBIT - 3))
#define DSIGNIF       (BITS(double) - _DEXPLEN + _HIDDENBIT - 1)
#define FSIGNIF       (BITS(float)  - _FEXPLEN + _HIDDENBIT - 1)
#define DMAXPOWTWO    ((double)(1L << BITS(long) - 2) * \
		               (1L<<DSIGNIF-BITS(long)+1)) 
#define FMAXPOWTWO    ((float)(1L << FSIGNIF - 1))
#define DMAXEXP       ((1 << _DEXPLEN - 1) - 1 + _IEEE) 
#define FMAXEXP       ((1 << _FEXPLEN - 1) - 1 + _IEEE) 
#define LN_MAXDOUBLE  (M_LN2 * DMAXEXP)
#define LN_MINDOUBLE  (M_LN2 * (DMINEXP - 1))

#define _DEXPLEN      11
#define _FEXPLEN      8

#define H_PREC        (DSIGNIF % 2 ? (1L << DSIGNIF/2) * M_SQRT2 : 1L << DSIGNIF/2)

#define X_PLOSS       ((double)(long)(M_PI * H_PREC))
#define X_TLOSS       (M_PI * DMAXPOWTWO)

/* The next values are duplicated in math.h. They have to be    */
/* here too for to keep from having to include math.h.		*/
#define M_LN2      6.9314718055994530942E-1 /*Hex  2^-1 * 1.62E42FEFA39EF */
#define M_PI       3.1415926535897931160E0  /*Hex  2^ 1 * 1.921FB54442D18 */
#define M_SQRT2    1.4142135623730951455E0  /*Hex  2^ 0 * 1.6A09E667F3BCD */

#endif	/* _VALUES_H_ */
