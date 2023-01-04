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
 *	@(#)$RCSfile: ipc_copyout.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:23:51 $
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

#ifndef	_KERN_IPC_COPYOUT_H_
#define _KERN_IPC_COPYOUT_H_

#include <mach_ipc_xxxhack.h>

#include <mach/message.h>

#if	MACH_IPC_XXXHACK
extern void port_destroy_receive();
extern void port_destroy_own();
#endif	/* MACH_IPC_XXXHACK */
extern void port_destroy_receive_own();

#if	MACH_IPC_XXXHACK
extern int port_copyout_receive();
extern int port_copyout_own();
#endif	/* MACH_IPC_XXXHACK */
extern int port_copyout_receive_own();

extern void object_destroy();
extern void object_copyout();

extern msg_return_t msg_copyout();
extern void msg_destroy();

#endif	/* _KERN_IPC_COPYOUT_H_ */
