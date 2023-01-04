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
	.rdata
	.asciiz "@(#)$RCSfile: byte_swap.s,v $ $Revision: 1.1.7.2 $ (DEC) $Date: 1993/06/24 22:28:01 $"
	.text

#include <machine/asm.h>
#include <machine/regdef.h>
#include "assym.s"

/*
 * nuxi_16(word)	Swap bytes in a word
 */
LEAF(nuxi_16)				# a0 = ??ab
	  extbl   a0,1,t0		# t0 =    a
	  extbl   a0,0,t1		# t1 =    b
	  sll     t1,8,t1		# t1 =   b0
	  or      t1,t0,v0		# v0 =   ba
	  ret     zero,(ra)
	  END(nuxi_16)
/*
 * nuxi_32(longword)	Swap bytes in a word
 */
/***************************************************************************
 *
 *   swap long word bytes: will operate on one unsigned 32 bit quantity
 *
 *
 *  			31               0
 *			 +---+---+---+---+
 *	start with:	 | a | b | c | d |
 *			 +---+---+---+---+
 *
 *			31               0
 *	end with:        +---+---+---+---+
 *			 | d | c | b | a |
 *			 +---+---+---+---+
 *
 ***************************************************************************/

LEAF(nuxi_32)				# a0 = abcd
XLEAF(swap_lw_bytes)
	  extbl   a0,1,t0		# t0 =    c
	  extbl   a0,0,t1		# t1 =    d
	  sll     t1,24,t1		# t1 = d000
	  sll	  t0,16,t0		# t0 =  c00
	  or      t1,t0,v0		# v0 = dc00
	  extbl	  a0,2,t0		# t0 =    b
	  sll     t0,8,t0		# t0 =   b0
	  or	  t0,v0,v0		# v0 = dcb0
	  extbl   a0,3,t0		# t0 =    a
	  addl	  t0,v0,v0		# v0 = dcba (canonicalize too)
	  ret     zero,(ra)
	  END(nuxi_32)

/*
 ***************************************************************************
 *
 *   swap word bytes: will operate on one unsigned 32 bit quantity
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
 *			 | b | a | d | c |
 *			 +---+---+---+---+
 *                               |
 *
 ***************************************************************************/


LEAF(swap_word_bytes)			# a0 = abcd

	  extbl   a0,1,t0		# t0 =    c
	  extbl   a0,0,t1		# t1 =    d
	  sll     t1,8,t1		# t1 = 00d0
	  or      t1,t0,v0		# v0 = 00dc
	  extbl	  a0,2,t0		# t0 =    b
	  sll     t0,24,t0		# t0 = b000
	  or	  t0,v0,v0		# v0 = b0dc
	  extbl   a0,3,t0		# t0 =    a
	  sll     t0,16,t0		# t0 = 0a00
	  addl	  t0,v0,v0		# v0 = badc (canonicalize too)
	  ret     zero,(ra)
	  END(swap_word_bytes)

/***************************************************************************
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


LEAF(swap_words)			# a0 = abcd

	  extwl   a0,0,t0		# t0 = 00cd
	  sll     t0,16,t0		# t0 = cd00
	  extwl	  a0,2,t1		# t1 =   ab
	  addl	  t0,t1,v0		# v0 = cdab (canonicalize too)
	  ret     zero,(ra)
	  END(swap_words)




