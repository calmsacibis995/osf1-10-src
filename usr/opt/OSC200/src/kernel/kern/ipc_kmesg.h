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
 *	@(#)$RCSfile: ipc_kmesg.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:24:13 $
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

#ifndef	_KERN_IPC_KMESG_H_
#define _KERN_IPC_KMESG_H_

#include <mach_xp_fpd.h>

#include <mach/boolean.h>
#include <mach/port.h>
#include <mach/message.h>
#include <mach_debug/ipc_statistics.h>
#include <kern/task.h>
#include <kern/kern_msg.h>
#include <kern/zalloc.h>
#include <kern/ipc_globals.h>
#include <kern/ipc_copyout.h>
#include <kern/macro_help.h>

/*
 *	Kernel message allocation
 *
 *	The IPC system allocates messages in one of two sizes: small or large.
 *	Other kernel subsystems may allocate messages on their own; the
 *	IPC system will return a message to its "home_zone" once it has been
 *	dequeued.
 *
 *	The internal allocation routines are noticably macros: their
 *	argument is the by-reference result of the allocation.
 *
 *	To destroy a message involves deallocating the resources
 *	associated with it; this is the normal case.
 */

#define kern_msg_allocate_small(kmsg)			\
MACRO_BEGIN						\
	ZALLOC(kmsg_zone, (kmsg), kern_msg_t); 		\
	(kmsg)->home_zone = kmsg_zone; 			\
	(kmsg)->kernel_message = FALSE;			\
	ipc_event(current); 				\
MACRO_END

#define kern_msg_allocate_large(kmsg)			\
MACRO_BEGIN						\
	ZALLOC(kmsg_zone_large, (kmsg), kern_msg_t);	\
	(kmsg)->home_zone = kmsg_zone_large; 		\
	(kmsg)->kernel_message = FALSE;			\
	ipc_event(current); 				\
MACRO_END

#define kern_msg_free(kmsg)				\
MACRO_BEGIN						\
	ZFREE(kmsg->home_zone, (vm_offset_t) kmsg);	\
	ipc_event_count(current, -1);			\
MACRO_END

#define kern_msg_destroy(kmsgptr)					     \
MACRO_BEGIN							     	     \
	if ((kmsgptr)->kmsg_header.msg_simple) {			     \
		register kern_port_t port;				     \
									     \
		port = (kern_port_t) (kmsgptr)->kmsg_header.msg_remote_port; \
		if (port != KERN_PORT_NULL)				     \
			port_release_macro(port);			     \
									     \
		port = (kern_port_t) (kmsgptr)->kmsg_header.msg_local_port;  \
		if (port != KERN_PORT_NULL)				     \
			port_release_macro(port);			     \
									     \
		kern_msg_free(kmsgptr); 				     \
	} else 								     \
		msg_destroy(kmsgptr);					     \
MACRO_END

/*
 * 	Macro:	move_msg_data
 *	
 *	Purpose:
 *		Determine if a move from user to kernel space or kernel
 *		to user space is needed for data of a particular type
 *		in a particular message.
 *
 *	Rationale:
 *		It is faster to sometimes avoid the extra copy into the
 *		kernel when we know the kernel will just re-copy the data
 *		anyway.  This is a sleazy hack for now.  A more general
 *		purpose solution must follow.
 */

#ifdef	lint
int	_MACH_XP_FPD_;
#else	/* lint */
#define _MACH_XP_FPD_	MACH_XP_FPD
#endif	/* lint */

#define fast_pager_data(kmsg)				\
	(_MACH_XP_FPD_ &&				\
	 ((kmsg)->kmsg_header.msg_id == 2038) &&	\
	 (kmsg)->kernel_message)

#define move_msg_data(kmsgptr, tn) 			\
	(! fast_pager_data(kmsgptr) &&			\
	 (tn != MSG_TYPE_INTERNAL_MEMORY))

#endif	/* _KERN_IPC_KMESG_H_ */
