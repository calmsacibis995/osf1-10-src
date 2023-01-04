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
 *	@(#)$RCSfile: Ovfork.s,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 02:57:12 $
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

 * C library -- vfork
 */

/*
 * pid = vfork();
 *
 * r1 == 0 in parent process, r1 == 1 in child process.
 * r0 == pid of child in parent, r0 == pid of parent in child.
 *
 * trickery here, due to keith sklower, uses ret to clear the stack,
 * and then returns with a jump indirect, since only one person can return
 * with a ret off this stack... we do the ret before we vfork!
 * except on the n16, all we need do is get the ret address off the
 * stack, there is nothing else to "clear".  (thanks anyways, keith).
 */

#include "SYS.h"

ENTRY(vfork)
	movd	tos,r2		#get return pc
	addr	@SYS_vfork,r0	#set up for svc
	svc			#do the vfork
	bcs	.Lerror		#branch if not successful
	cmpqd	$0,r1		#are we the child?
	beq	.Ldone		#no
	movqd	$0,r0		#yes, clear return
.Ldone:	jump	r2		#return to caller

.Lerror:
	jsr     @cerror		#set error if not
	jump	r2		#return
