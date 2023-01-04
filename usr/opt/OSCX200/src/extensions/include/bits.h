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
**************************************************************************
**                   DIGITAL EQUIPMENT CORPORATION                      **
**                         CONFIDENTIAL                                 **
**    NOT FOR MODIFICATION OR REDISTRIBUTION IN ANY MANNER WHATSOEVER   **
**************************************************************************
*/
/*
 * This include file is designed to be a portable way for systems to define
 * bit field manipulation of arrays of bits.
 */
#ifndef __BITS__
#define __BITS__ "@(#)bits.h	1.6 - 90/09/18  "
/* RCSID = $Header: /alphabits/u3/x11/ode/rcs/x11/src/extensions/include/bits.h,v 1.1.2.2 92/02/06 13:44:37 Jim_Ludwig Exp $" */

/*****************************************************************************
Copyright 1987, 1988, 1989, 1990 by Digital Equipment Corporation, Maynard, MA

Permission to use, copy, modify, and distribute this software and its 
documentation for any purpose and without fee is hereby granted, 
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in 
supporting documentation, and that the name of Digital not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.  

DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.

*****************************************************************************/
/*
 *
 *  CONTRIBUTORS:
 *
 *      Dick Annicchiarico
 *      Robert Chesler
 *      Dan Coutu
 *      Gene Durso
 *      Marc Evans
 *      Alan Jamison
 *      Mark Henry
 *      Ken Miller
 *
 */
typedef unsigned char *UByteP;  /* Pointer to an unsigned byte array */
#define BitsInByte    8L        /* The number of bits in a byte */

#define BitInByte(bit)	        /* Returns the bit mask of a byte */ \
    (1L << (((bit) % BitsInByte)))

#define BitInWord(bit)	        /* Returns the bit mask of a word */ \
    (1L << (((bit) % (BitsInByte * 2L))))

#define BitInLong(bit)	        /* Returns the bit mask of a long */ \
    (1L << (((bit) % (BitsInByte * 4L))))

#define ByteInArray(array,bit)	/* Returns the byte offset to get to a bit */ \
    (((UByteP)(array))[(bit) / BitsInByte])

#define BitIsTrue(array,bit)    /* Test to see if a specific bit is True */ \
    (ByteInArray(array,bit) & BitInByte(bit))

#define BitIsFalse(array,bit)   /* Test to see if a specific bit is False */ \
    (!(BitIsTrue(array,bit)))

#define BitTrue(array,bit)      /* Set a specific bit to be True */ \
    (ByteInArray(array,bit) |= BitInByte(bit))

#define BitFalse(array,bit)     /* Set a specific bit to be False */ \
    (ByteInArray(array,bit) &= ~BitInByte(bit))

#define BitToggle(array,bit)    /* Toggle a specific bit */ \
    (ByteInArray(array,bit) ^= BitInByte(bit))

#define BitCopy(dest,src,bit)   /* Copy a specific bit */ \
    BitIsTrue((src),(bit)) ? BitTrue((dest),(bit)) : BitFalse((dest),(bit))

#define BitValue(array,bit)     /* Return True or False depending on bit */ \
    (BitIsTrue((array),(bit)) ? True : False)

#define BitSet(array,bit,value) /* Set bit to given value in array */ \
    (value) ? BitTrue((array),(bit)) : BitFalse((array),(bit))

#endif /* __BITS__ */
