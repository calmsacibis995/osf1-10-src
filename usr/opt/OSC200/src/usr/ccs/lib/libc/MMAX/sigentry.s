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
 *	@(#)$RCSfile: sigentry.s,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 03:08:01 $
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
 **********************************************************************
 *
 *	 Copyright (C) 1984 Hydra Computer Systems, Inc.
 *
 * ALL RIGHTS RESERVED. Licensed Material - Property of Hydra Computer
 * Systems, Inc. This software is made available solely pursuant to the
 * terms of a software license agreement which governs its use.
 * Unauthorized duplication, distribution or sale are strictly 
 * prohibited.
 *
 * Module Function:
 *	Sigcleanup system call (internal only!)
 *
 * Original Author: Tony Anzelmo	Created on: 85/01/14
 *
 * Revision Control Information:
 *
 * Revision History:
 *
 * $ Log:	sigentry.s,v $
 * Revision 3.2  85/10/30  17:48:45  ptw
 * Type-oh
 * 
 * Revision 3.1  85/10/30  17:40:43  ptw
 * Add back some code from multi to deal with the way they are still 
 * returning to the user program by constructing a procedure return 
 * on the stack.  The r5 will never execute this code, because after
 * the sigcleanup svc, it returns directly to the interrupted program.
 * 
 * Revision 1.1  85/05/16  12:12:32  ptw
 * Initial revision
 * 
 * Revision 1.2  85/01/14  21:07:15  anzelmo
 *	Added RCS header and system call macro
 *
 *
 * This file is currently under revision by:
 *
 * $ Locker:  $
 *
 **********************************************************************
 * 
 *
 *#define RCSDATE $ Date: 87/11/04 10:06:00 $
 *#define RCSREV	$ Revision: 10.1 $
 *
 * sigentry is an intermediate subroutine returned to by the kernel when
 *	sending any signal.  It saves the PSR and the temporary register,
 *	copies the signal handlers arguments, and calls the user signal
 *	handler.  When the user signal handler returns sigentry does an svc
 *	to the signal cleaner (sigcleanup).  Finally sigentry restores the
 *	PSR and temporary registers and returns to the interrupted process.
 */ 

	.set	sigcleanup,139
	.globl	sigentry

sigentry:
	jsr	0(12(sp))		#call signal handler
	addr	@sigcleanup,r0		#make sigcleanup system call
	svc
						#Only the multi kernel comes back here
						#the r5 kernel does a rti directly to 
						#the previously interrupted program
	lprd	upsr,6(sp)	#restore old psr flags
	rxp		$0			#return to interrupted routine
