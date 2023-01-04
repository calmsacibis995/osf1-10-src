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
 *	@(#)$RCSfile: ipc_signal.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:25:00 $
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

#ifndef	_KERN_IPC_SIGNAL_H_
#define _KERN_IPC_SIGNAL_H_

#include <mach_ipc_sighack.h>

#if	MACH_IPC_SIGHACK

#include <mach/boolean.h>
#include <kern/task.h>
#include <kern/macro_help.h>
#include <sys/signal.h>
#include <sys/proc.h>

#define PSIGNAL(task, emerg) \
	MACRO_BEGIN						\
	if ((task)->ipc_intr_msg)				\
		psignal(&proc[(task)->proc_index],		\
			(emerg) ? SIGEMSG : SIGMSG);		\
	MACRO_END

#else	/* MACH_IPC_SIGHACK */

#define PSIGNAL(task, emerg)

#endif	/* MACH_IPC_SIGHACK */

#endif	/* _KERN_IPC_SIGNAL_H_ */
