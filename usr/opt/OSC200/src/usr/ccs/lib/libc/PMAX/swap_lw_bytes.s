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
 * @(#)$RCSfile: swap_lw_bytes.s,v $ $Revision: 4.2.3.2 $ (DEC) $Date: 1992/01/28 17:29:45 $
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
  *		Created this file, and the lw byte swap routine for
  *          	user level VME support.  
  */

#include <mips/asm.h>
#include <mips/regdef.h>

/***************************************************************************
 *
 *   swap_lw_bytes(buffer);
 *   unsigned long buffer;
 *
 *   RETURNS: resulting byte swapped long
 *
 ***************************************************************************
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


LEAF(swap_lw_bytes)
#ifdef MIPSEL
	# a0 has the input, unsigned 32 bit quanity - abcd
	sll	v0,a0,24	# shift left 24     - d000  in v0
	srl     v1,a0,24   	# shift right 24    - 000a  in v1
	or	v0,v1 		# v0 <- v0 | v1     - d00a  in v0  *
	srl	t0,a0,8         # shift right 8     - 0abc  in t0
	and     t1,t0,0xff00    # mask out b        - 00b0  in t1
	and 	v1,t0,0xff      # mask out c        - 000c  in v1
	sll	v1,16		# shift left 16     - 0c00  in v1
	or	v1,t1		# v1 <- v1 | t1     - 0cb0  in v1  *
	or 	v0,v1		# v0 <- v0 | v1     - dcba  in v0
#else
	move	v0,a0		# move v0 into ret value
#endif
	j	ra		# return
.end swap_lw_bytes

