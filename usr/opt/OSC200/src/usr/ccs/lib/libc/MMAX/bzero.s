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
 *	@(#)$RCSfile: bzero.s,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 02:58:16 $
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
/* bzero:	Zero len bytes starting at blk
 * Calling sequence:	bzero (blk, len);
 */
#include "SYS.h"

	.file	"bzero.s"
	.text
	.align	1
ENTRY(bzero)
	movd	SP(8),r0	# len
	cmpqd	$0,r0		# If len==0, return at this point
	beq	.Lendit
	movd	SP(4),r1	# blk
	movqb	$0,0(r1)
	movd	r1,r2		# Set up "destination"
	addqd	$1,r2		# dest = blk+1
	addqd	$-1,r0		# and len--
	movsb			# do the copy

.Lendit:
	EXIT
	ret	$0
