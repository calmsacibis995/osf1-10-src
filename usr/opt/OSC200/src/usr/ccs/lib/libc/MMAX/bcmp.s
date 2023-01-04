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
 *	@(#)$RCSfile: bcmp.s,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 02:58:07 $
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
/* bcmp:		Compare len bytes in b1 to b2
 * Calling sequence:	res = bcmp (b1, b2, len);
 * Returns:	0 if equal
 *		Nonzero in unequal
 */
#include "SYS.h"

	.file	"bcmp.s"
	.text
	.align	1
ENTRY(bcmp)
	movd	SP(4),r1	# b1
	movd	SP(8),r2	# b2
	movd	SP(12),r0	# len
	cmpsb			# do the cmp
	bne	.Lendit		# Not equal - use nonzero value in r0
				#   from cmpsb as return value
	movqd	$0,r0		# Else were equal
.Lendit:
	EXIT
	ret	$0
