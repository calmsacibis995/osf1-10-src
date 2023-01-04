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
 *	@(#)$RCSfile: ipc_basics.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:23:35 $
 */ 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/* 
 * Mach Operating System
 * Copyright (c) 1989 Carnegie-Mellon University
 * Copyright (c) 1988 Carnegie-Mellon University
 * Copyright (c) 1987 Carnegie-Mellon University
 * All rights reserved.  The CMU software License Agreement specifies
 * the terms and conditions for use and redistribution.
 */
/*
 * OSF/1 Release 1.0
 */

#ifndef	_KERN_IPC_BASICS_H_
#define _KERN_IPC_BASICS_H_

#include <mach_ipc_xxxhack.h>

#include <kern/kern_msg.h>
#include <mach/message.h>

extern void send_notification();
extern void send_complex_notification();

extern kern_msg_t mach_msg();
extern msg_return_t msg_queue();

extern msg_return_t msg_send();
extern msg_return_t msg_send_from_kernel();
extern msg_return_t msg_receive();
extern msg_return_t msg_rpc();

extern msg_return_t msg_send_trap();
extern msg_return_t msg_receive_trap();
extern msg_return_t msg_rpc_trap();

#if	MACH_IPC_XXXHACK
extern msg_return_t msg_send_old();
extern msg_return_t msg_receive_old();
extern msg_return_t msg_rpc_old();
#endif	/* MACH_IPC_XXXHACK */

#endif	/* _KERN_IPC_BASICS_H_ */
