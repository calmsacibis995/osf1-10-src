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
 *	@(#)$RCSfile: ffs.s,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 02:59:38 $
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
/* ffs:		Find first bit set in an int (least significant == 1,
 *		most significant==32).  For example, ffs(1) == 1,
 *		ffs(0x80000000) == 32, ffs(0xf4) == 3.
 * Calling sequence:	bitno = ffs (val);
 * Returns:	bit set (1-based), or 0 if none set
 */
#include "SYS.h"

	.file	"ffs.s"
	.text
	.align	1
ENTRY(ffs)
	movqd	$0,r0		# Start search at offset of 0
	ffsd	SP(4),r0	# r0 gets 0,F=1 if no bits set
	bfs	.Lnotfound	# F=1? return 0
	addqd	$1,r0		# ffsd uses 0-based count, convert to 1-based
.Lnotfound:
	EXIT
	ret	$0
