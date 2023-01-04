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
 * @(#)$RCSfile: ffintr.s,v $ $Revision: 1.1.3.2 $ (DEC) $Date: 1992/04/14 15:15:50 $
 */

#include  <machine/regdef.h>
#include  <machine/asm.h>
#include  <machine/cpu.h>

/*
 * ffintr(cause_register) -- find first bit set in interrupt pending byte
 * bits are numbered as 8 most significant to 1 least significant,
 * search starts from most significant end, returns 0 in no bits set
 */
LEAF(ffintr)
	and	v0,a0,CAUSE_IPMASK
	srl	a0,v0,CAUSE_IPSHIFT+4	# shift to high nibble of IPEND bits
	bne	a0,zero,1f		# bits set in high nibble
	srl	a0,v0,CAUSE_IPSHIFT	# get 2nd nibble right
	add	a0,16			# to get to 2nd half of table
1:	lbu	v0,ffitbl(a0)		# get value from table
	j	ra
	END(ffintr)

	.data
ffitbl:
	.byte 0,5,6,6,7,7,7,7,8,8,8,8,8,8,8,8
	.byte 0,1,2,2,3,3,3,3,4,4,4,4,4,4,4,4
	.text


