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
 *	@(#)$RCSfile: msem_tas.s,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 03:09:47 $
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
 * msem_tas.s
 *
 *	Revision History:
 *
 * 28-Apr-91	Fred Canter
 *	Undo LANGUAGE_C hack.
 *
 * 12-Mar-91	Fred Canter
 *	MIPS C 2.20+
 *
 */


#include <mips/regdef.h>
#include <mips/asm.h>
#include <mips/inst.h>


	.set noreorder
LEAF(_msem_tas)
	move	v0,a0		#preserve a0s contents
	.word tas_op		#kernel emulated test and set instruction
				#It preforms a test and set on the location
				#pointed to by a0 and returns the previous
				#value in a0.  
	j	ra
	move	v0, a0		#Return the previous value of the lock
END(_msem_tas)
