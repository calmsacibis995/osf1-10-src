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
 *	@(#)$RCSfile: __udivsi3.s,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 02:57:18 $
 */ 
/*
 */
#
# (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
# ALL RIGHTS RESERVED
#
#
# OSF/1 Release 1.0
	.file	"udivsi3.c"
# ccom  -O -X22 -X50 -X64 -X74 -X80 -X83 -X85 -X88 -X102 -X151 -X154 -X183
#	 -X193 -X211 -X230
	.text
	.align	16
___udivsi3:
	enter	[],$8

	movd	8(fp),-8(fp)
	movqd	$0,-4(fp)
	deid	12(fp),-8(fp)
	movd	-4(fp),r0

	exit	[]
	ret	$0
	.data
#_arg1	8(fp)	local
#_arg2	12(fp)	local
	.text
	.data
	.globl	___udivsi3
	.text
