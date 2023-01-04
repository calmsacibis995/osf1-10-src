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
 *	@(#)$RCSfile: kern_msg.h,v $ $Revision: 4.3.3.2 $ (DEC) $Date: 1992/04/06 15:18:56 $
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
/*
 * File:	kern_msg.h
 * Purpose:
 *	Kernel internal message structure.
 *
 *	Revision History:
 *
 * 8-Apr-91	Ron Widyono
 *	Delay inclusion of sys/preempt.h (for RT_PREEMPT) to avoid circular
 *	include file problem.
 *
 */

#ifndef	_KERN_KERN_MSG_H_
#define _KERN_KERN_MSG_H_

#include <rt_preempt.h>

#if	RT_PREEMPT
#ifndef	_SKIP_PREEMPT_H_
#define _SKIP_PREEMPT_H_
#define	_KERN_KERN_MSG_H_PREEMPT_
#endif
#endif

#include <mach/message.h>
#include <kern/task.h>
#include <kern/queue.h>
#include <kern/zalloc.h>
#include <kern/ipc_netport.h>

/* 
 * Note: kmsg_header must be the last element in kern_msg.  The mig places
 * other structures and possibly the actual message at after kmsg_header.
 *
 */
typedef struct kern_msg {
		queue_chain_t		queue_head;	/* must be first */
		struct port_hash *	sender_entry;
		zone_t			home_zone;
		boolean_t		kernel_message;
		msg_header_t		kmsg_header;
} *kern_msg_t;

#define		KERN_MSG_NULL	((kern_msg_t) 0)

#if	RT_PREEMPT
#ifdef	_KERN_KERN_MSG_H_PREEMPT_
#include <sys/preempt.h>
#endif
#endif

#endif	/* _KERN_KERN_MSG_H_ */
