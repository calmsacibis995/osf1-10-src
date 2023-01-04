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
 * @(#)$RCSfile: swap_words.s,v $ $Revision: 4.2.3.2 $ (DEC) $Date: 1992/01/28 17:30:05 $
 */
/* ------------------------------------------------------------------ */
/* | Copyright Unpublished, MIPS Computer Systems, Inc.  All Rights | */
/* | Reserved.  This software contains proprietary and confidential | */
/* | information of MIPS and its suppliers.  Use, disclosure or     | */
/* | reproduction is prohibited without the prior express written   | */
/* | consent of MIPS.                                               | */
/* ------------------------------------------------------------------ */

 /*
  * Modification History:
  *
  *     
  * 	11-Oct-1991	Stuart Hollander
  *		Converted header comments and include paths for OSF-based version
  *     
  * 	12-May-1990	Paul Grist
  *		Created this file, and the word byte swap routine for
  *          	user level VME support.
  */

#include <mips/asm.h>
#include <mips/regdef.h>

/***************************************************************************
 *
 *   swap_words(buffer);
 *   unsigned long buffer;
 *
 *   RETURNS: resulting byte swapped long
 *
 ***************************************************************************
 *
 *   swap words: will operate on one unsigned 32 bit quantity
 *
 *
 *  			31       |       0
 *			 +---+---+---+---+
 *	start with:	 | a | b | c | d |
 *			 +---+---+---+---+
 *                               |
 *
 *
 *			31       |       0
 *	end with:        +---+---+---+---+
 *			 | c | d | a | b |
 *			 +---+---+---+---+
 *                               |
 *
 ***************************************************************************/

LEAF(swap_words)
#ifdef MIPSEL
	# a0 has the input, unsigned 32 bit quanity - abcd
	sll	v0,a0,16		# shift left 16      - cd00  in v0
	srl	v1,a0,16		# shift right 16     - 00ab  in v1
	or	v0,v1			# v0 <- v0 | v1	     - cdab  in v0
#else
	move	v0,a0			# move v0 into ret value
#endif
	j	ra			# return
.end swap_words

