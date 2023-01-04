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
 * @(#)$RCSfile: gentrap.h,v $ $Revision: 1.1.5.2 $ (DEC) $Date: 1993/06/14 17:36:38 $
 */

/*
 * gentrap PAL call arguments (passed in a0)
 */

#ifndef _GENTRAP_H_
#define _GENTRAP_H_

#define GEN_INTOVF	-1	/* integer overflow */
#define GEN_INTDIV	-2	/* integer division by zero */
#define GEN_FLTOVF	-3	/* floating overflow */
#define GEN_FLTDIV	-4	/* floating divide by zero */
#define GEN_FLTUND	-5	/* floating underflow */
#define GEN_FLTINV	-6	/* floating invalid operand */
#define GEN_FLTINE	-7	/* floating inexact result */
#define GEN_DECOVF	-8	/* decimal overflow */
#define GEN_DECDIV	-9	/* decimal divide by zero */
#define GEN_DECINV	-10	/* decimal invalid operand */
#define GEN_ROPRAND	-11	/* reserved operand */
#define GEN_ASSERTERR	-12	/* assertion error */
#define GEN_NULPTRERR	-13	/* null pointer error */
#define GEN_STKOVF	-14	/* stack overflow */
#define GEN_STRLENERR	-15	/* string length error */
#define GEN_SUBSTRERR	-16	/* substring error */
#define GEN_RANGEERR	-17	/* range error */
#define GEN_SUBRNG	-18	/* subscript range error */
#define GEN_SUBRNG1	-19	/* subscript 1 range error */
#define GEN_SUBRNG2	-20	/* subscript 2 range error */
#define GEN_SUBRNG3	-21	/* subscript 3 range error */
#define GEN_SUBRNG4	-22	/* subscript 4 range error */
#define GEN_SUBRNG5	-23	/* subscript 5 range error */
#define GEN_SUBRNG6	-24	/* subscript 6 range error */
#define GEN_SUBRNG7	-25	/* subscript 7 range error */
/*
 * -26..-1023 are reserved
 */
#endif
