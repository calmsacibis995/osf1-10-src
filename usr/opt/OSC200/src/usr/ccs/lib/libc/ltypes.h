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
 *	@(#)$RCSfile: ltypes.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 03:40:34 $
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

#ifndef _H_ltypes
#define _H_ltypes
/*
 * COMPONENT_NAME: (LIBCGEN) Standard C Library General Functions
 *
 * FUNCTIONS: ltypes.h
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 * OBJECT CODE ONLY SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 * quick set of typedefs
 */

typedef int int32;
typedef char int8;
typedef short int16;
typedef unsigned long uint32;
typedef unsigned char uint8;
typedef unsigned short uint16;

#define hipart(X) (((uint32 *)&X)[0])
#define lopart(X) (((uint32 *)&X)[1])

#define _absfp(X)	asm("	cau	15, 0xff02");\
			asm("	st	0,0x80*(X)+0x8*(X)+0xd000(15)");

#ifndef StoQDNaN
#define StoQDNaN
#endif /* StoQDNaN */

#ifndef StoQFNaN
#define StoQFNaN(X)	hipart((X)) |= BIT22
#endif  /* StoQFNaN */

#ifndef _QNaN
#define _QNaN
static uint32 FQNaN[] = {0x7fc00000};
static uint32 DQNaN[] = {0x7ff80000, 0x00000000};
#define _QNaN
#endif  /* _QNaN */


#define fregtomem(P, REGNO)\
{\
	uint32 *FPAptr = (uint32 *)0xff02f000;\
	uint32 *rp = (uint32 *)(P);\
	uint32 i = (REGNO)*2;\
	*rp = *(FPAptr + (i<<4));\
}

#define memtofreg(P, REGNO)\
{\
	uint32 *FPAptr = (uint32 *)0xff025000;\
	uint32 *rp = (uint32 *)(P);\
	uint32 i = (REGNO)*2;\
	*(FPAptr + i) = *rp;\
}

#define dregtomem(P, REGNO)\
{\
	register uint32 *FPAptr = (uint32 *)0xff02f000;\
	register uint32 *rp = (uint32 *)(P);\
	uint32 i = (REGNO)*2;\
	*rp++ = *(FPAptr + (i++<<4));\
	*rp = *(FPAptr + (i<<4));\
}

#define memtodreg(P, REGNO)\
{\
	register uint32 *FPAptr = (uint32 *)0xff025000;\
	register uint32 *rp = (uint32 *)(P);\
	uint32 i = (REGNO)*2;\
	*(FPAptr + i++) = *rp++;\
	*(FPAptr + i) = *rp;\
}

#define fFA_ret(reg)\
{\
	uint32 f1;\
	fregtomem(&f1, reg & OKREGBITS);\
	return(f1);\
}

#define dFA_ret(reg)\
{\
	double d1;\
	dregtomem(&d1, reg & OKREGBITS);\
	return(d1);\
}

#define fNaNA_ret(reg)\
	memtofreg(FQNaN, (reg)&OKREGBITS);\
	return( *(uint32 *)FQNaN );\
}

#define dNaNA_ret(reg)\
{\
	memtodreg(DQNaN, (reg)&OKREGBITS);\
	return( *(double *)DQNaN );\
}

#define FPfp(X)  (*(unsigned (*)())_fpfpf[((int)(X))])()
#define FPfp1(X,Y)  (*(unsigned (*)())_fpfpf[((int)(X))])((Y))
#define FPfp2(X,Y,Z)  (*(int (*)())_fpfpf[((int)(X))])((Y),(Z))

#define NORETBIT        0x08

#define _DOUBLE(x) (*(DOUBLE *)&(x))
#define MIN(X, Y) ((X) < (Y) ? (X) : (Y))

#endif  /* _H_ltypes */
