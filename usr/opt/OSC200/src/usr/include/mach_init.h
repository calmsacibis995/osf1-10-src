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
 *	@(#)$RCSfile: mach_init.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 05:17:05 $
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
 * All rights reserved.  The CMU software License Agreement specifies
 * the terms and conditions for use and redistribution.
 */
/*
 * OSF/1 Release 1.0
 */
/*
 *	Items provided by the Mach environment initialization.
 */

#ifndef	_MACH_INIT_H_
#define	_MACH_INIT_H_	1

#include <mach/mach_types.h>

/*
 *	Kernel-related ports; how a task/thread controls itself
 */

extern	port_t	task_self_;
extern	port_t	task_notify_;
extern  port_t	thread_reply_;
#define task_data_	thread_reply_

#define	task_self()	task_self_
#define	task_data()	thread_reply_
#define	thread_reply()	thread_reply_
#define	task_notify()	task_notify_

#define	current_task()	task_self()

/*
 *	Other important ports in the Mach user environment
 */

#define	NameServerPort	name_server_port	/* compatibility */

extern	port_t	name_server_port;
extern	port_t	environment_port;
extern	port_t	service_port;

/*
 *	Where these ports occur in the "mach_ports_register"
 *	collection... only servers or the runtime library need know.
 */

#if	MACH_INIT_SLOTS
#define	NAME_SERVER_SLOT	0
#define	ENVIRONMENT_SLOT	1
#define SERVICE_SLOT		2

#define	MACH_PORTS_SLOTS_USED	3

extern	port_array_t	mach_init_ports;
extern	unsigned int	mach_init_ports_count;
#endif	/* MACH_INIT_SLOTS */

/*
 *	Globally interesting numbers
 */

extern	vm_size_t	vm_page_size;

#define round_page(x)	((((vm_offset_t)(x) + (vm_page_size - 1)) / vm_page_size) * vm_page_size)
#define trunc_page(x)	((((vm_offset_t)(x)) / vm_page_size) * vm_page_size)

#endif	/* _MACH_INIT_H_ */
