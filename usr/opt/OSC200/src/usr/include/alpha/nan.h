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
/* --------------------------------------------------------- */
/* | Copyright (c) 1986, 1989 MIPS Computer Systems, Inc.  | */
/* | All Rights Reserved.                                  | */
/* --------------------------------------------------------- */
/* $Header: /usr/sde/alpha/rcs/alpha/src/./usr/include/alpha/nan.h,v 1.1.2.3 1992/07/28 16:16:47 Jin_Chen Exp $ */

/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/* Handling of Not_a_Number's (only in IEEE floating-point standard) */
#if _IEEE
typedef union 
{
         struct	
	 {
#if defined(MIPSEL) || defined(__alpha)
	    unsigned fraction_low:32;
            unsigned bits:20;
	    unsigned exponent :11;
	    unsigned sign     : 1;
#else
	    unsigned sign     : 1;
	    unsigned exponent :11;
            unsigned bits:20;
	    unsigned fraction_low:32;
#endif
         } inf_parts;
	 struct 
	 {
#if defined(MIPSEL) || defined(__alpha)
	    unsigned fraction_low: 32;
	    unsigned bits     :19;
	    unsigned qnan_bit : 1;
            unsigned exponent :11;
	    unsigned sign     : 1;
#else
	    unsigned sign     : 1;
            unsigned exponent :11;
	    unsigned qnan_bit : 1;
	    unsigned bits     :19;
	    unsigned fraction_low: 32;
#endif
         } nan_parts;
         double d;

} dnan; 

	/* IsNANorINF checks that exponent of double == 2047 *
	 * i.e. that number is a NaN or an infinity	     */
	
#define IsNANorINF(X)  (((dnan *)&(X))->nan_parts.exponent == 0x7ff)

	/* IsINF must be used after IsNANorINF		*
 	 * has checked the exponent 			*/

#define IsINF(X)  (((dnan *)&(X))->inf_parts.bits == 0 &&  \
                    ((dnan *)&(X))->inf_parts.fraction_low == 0)

	/* IsPosNAN and IsNegNAN can be used 		*
 	 * to check the sign of infinities too		*/

#define IsPosNAN(X)  (((dnan *)&(X))->nan_parts.sign == 0)

#define IsNegNAN(X)  (((dnan *)&(X))->nan_parts.sign == 1)

	/* GETNaNPC gets the leftmost 32 bits 		*	
	 * of the fraction part				*/

#define GETNaNPC(dval)   (((dnan *)&(dval))->inf_parts.bits << 12 | \
			  ((dnan *)&(dval))->nan_parts.fraction_low>> 20) 

#define KILLFPE()       (void) kill(getpid(), 8)
#define NaN(X)  (((dnan *)&(X))->nan_parts.exponent == 0x7ff)
#define KILLNaN(X)      if (NaN(X)) KILLFPE()

#else

typedef double dnan;
#define IsINF(X)   0
#define IsPINF(X)  0
#define IsNegNAN(X)  0
#define IsPosNAN(X)  0
#define IsNAN(X)   0
#define GETNaNPC   0L

#define Nan(X)  0
#define KILLNaN(X)
#endif
