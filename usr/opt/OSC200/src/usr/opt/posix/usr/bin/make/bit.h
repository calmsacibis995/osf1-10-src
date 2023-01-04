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
 * @(#)$RCSfile: bit.h,v $ $Revision: 1.1.2.3 $ (DEC) $Date: 1993/10/29 22:01:18 $
 */

/*
 * Copyright (c) 1988, 1989, 1990 The Regents of the University of California.
 * Copyright (c) 1988, 1989 by Adam de Boor
 * Copyright (c) 1989 by Berkeley Softworks
 * All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Adam de Boor.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that: (1) source distributions retain this entire copyright
 * notice and comment, and (2) distributions including binaries display
 * the following acknowledgement:  ``This product includes software
 * developed by the University of California, Berkeley and its contributors''
 * in the documentation or other materials provided with the distribution
 * and in all advertising materials mentioning features or use of this
 * software. Neither the name of the University nor the names of its
 * contributors may be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 *
 *	@(#)bit.h	5.3 (Berkeley) 6/1/90
 */

/*
 * bit.h --
 *
 *	Definition of macros for setting and clearing bits in an array
 *	of integers.
 *
 *	It is assumed that "int" is 32 bits wide.
 */

#ifndef _BIT
#define _BIT

#include "alphaosf.h"

#define BIT_NUM_BITS_PER_INT	32
#define BIT_NUM_BITS_PER_BYTE	8

#define Bit_NumInts(numBits)	\
	(((numBits)+BIT_NUM_BITS_PER_INT -1)/BIT_NUM_BITS_PER_INT)

#define Bit_NumBytes(numBits)	\
	(Bit_NumInts(numBits) * sizeof(int))

#define Bit_Alloc(numBits, bitArrayPtr)  	\
        bitArrayPtr = (int *)malloc((unsigned)Bit_NumBytes(numBits)); \
        Bit_Zero((numBits), (bitArrayPtr))

#define Bit_Free(bitArrayPtr)	\
        free((char *)bitArrayPtr)

#define Bit_Set(numBits, bitArrayPtr) \
	((bitArrayPtr)[(numBits)/BIT_NUM_BITS_PER_INT] |= \
				(1 << ((numBits) % BIT_NUM_BITS_PER_INT)))

#define Bit_IsSet(numBits, bitArrayPtr) \
	((bitArrayPtr)[(numBits)/BIT_NUM_BITS_PER_INT] & \
				(1 << ((numBits) % BIT_NUM_BITS_PER_INT)))

#define Bit_Clear(numBits, bitArrayPtr) \
	((bitArrayPtr)[(numBits)/BIT_NUM_BITS_PER_INT] &= \
				~(1 << ((numBits) % BIT_NUM_BITS_PER_INT)))

#define Bit_IsClear(numBits, bitArrayPtr) \
	(!(Bit_IsSet((numBits), (bitArrayPtr))))

#define Bit_Copy(numBits, srcArrayPtr, destArrayPtr) \
	bcopy((char *)(srcArrayPtr), (char *)(destArrayPtr), \
		Bit_NumBytes(numBits))

#define Bit_Zero(numBits, bitArrayPtr) \
	bzero((char *)(bitArrayPtr), Bit_NumBytes(numBits))
	 
#endif /* _BIT */
