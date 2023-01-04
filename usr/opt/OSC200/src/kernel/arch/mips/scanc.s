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
 * @(#)$RCSfile: scanc.s,v $ $Revision: 1.1.3.2 $ (DEC) $Date: 1992/04/14 13:22:00 $
 */


#include  <machine/regdef.h>
#include  <machine/asm.h>
#include  <machine/cpu.h>

/*
 * scanc(count, cp, table, mask)
 * Like VAX instruction
 */
LEAF(scanc)
	move	v0,a0
	b	2f

1:	subu	v0,1		# decr count
2:	beq	v0,zero,3f	# count exhausted
	lbu	v1,0(a1)	# get char at cp
	addu	a1,1		# incr cp
	addu	t8,a2,v1	# offset into table
	lbu	t9,0(t8)	# load table entry
	and	t9,a3		# mask table entry
	beq	t9,zero,1b	# masked bit set
3:	j	ra
	END(scanc)

