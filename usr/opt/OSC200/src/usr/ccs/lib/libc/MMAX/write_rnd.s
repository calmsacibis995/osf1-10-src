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
 *	@(#)$RCSfile: write_rnd.s,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 03:09:29 $
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
 *  The bit patterns and corresponding rounding modes as specified by ANSI are:
 *	00	RZ	Round toward zero
 *	01	RN	Round toward nearest representable value
 *	10	RP	Round toward positive infinity
 *	11	RM	Round toward minus infinity
 *
 *  These are the same bit values as used by the National FP chip.  It
 *  just stores them
 */

#include "SYS.h"

	.file	"write_rnd.s"
	.text
	.align 1
ENTRY(write_rnd)
	sfsr	r1		# Get current FP state
	extsd	r1,r0,$7,$2	# Get FP rounding field (bits 7,8)
				#  as return value
	inssw	SP(4),r1,$7,$2	# Set new rounding mode
	lfsr	r1
	EXIT			
	ret	$0

ENTRY(read_rnd)
	sfsr	r1		# Get the FP state
	extsd	r1,r0,$7,$2	# Extract out the rounding mode bits
	EXIT			#  and return them to caller
	ret	$0


