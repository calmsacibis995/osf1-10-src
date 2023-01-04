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
 *	@(#)$RCSfile: execle.s,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 02:59:01 $
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
/* execle.c 4.1 82/12/04 */

#include "SYS.h"

ENTRY(execle)
	addr	SP(4),r0	# point at first entry of argument list
.Lloop:	cmpqd	$0,0(r0)	# check for end of list
	addqd	$4,r0		# advance to next entry of list
	bne	.Lloop		# loop until done with argument list
	movd	0(r0),tos	# put environment pointer on stack
	addr	SP(12),tos	# move addr of args to stack
	movd	SP(12),tos	# copy addr of file string to stack
	addr	@SYS_execve,r0	# get svc index
	addr	0(sp),r1	# point at arguments
	svc			# execute the program
	bcc	.Lnoerr		# only get here on failure
	adjspb	$-12		# remove arguments
	jump	cerror
.Lnoerr:
	adjspb	$-12		# remove arguments
	EXIT
	ret	$0		# execle(file, arg1, arg2, ..., env);
