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
#ifndef lint
static char	*sccsid = "@(#)$RCSfile: state_load.c,v $ $Revision: 1.1.2.4 $ (DEC) $Date: 1992/05/26 09:36:11 $";
#endif 
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
#if !defined(lint) && !defined(_NOIDENT)

#endif
/*
 * OSF/1 Release 1.0
 */

/*
 * state_load.c
 *
 *	Modification History:
 *
 * 20-Apr-91	Fred Canter
 *	Bob Picco's fix for the MACH IPC memory leak.
 *	Initial creation of state_load.c.
 *
 */

#include <mach.h>
#include <mach/thread_status.h>

#define roundup(a,b)    ((((a) + (b) - 1) / (b)) * (b))

static struct alpha_thread_state alpha_state;

int
machine_state_load(state_type, state, state_size, start_pc)
	long *state_type;
	long **state;
	long *state_size;
	long start_pc;
{
	extern long _gp;
	extern long _curbrk;
	extern _exit();
	vm_offset_t addr, pagesize = getpagesize();

	(void) bzero(&alpha_state, sizeof (alpha_state));
	if (vm_allocate(task_self(), &addr, pagesize * 3, TRUE) != KERN_SUCCESS)
		return 1;

	if ((vm_protect(task_self(), addr, pagesize, 0, VM_PROT_NONE) !=
		KERN_SUCCESS) ||
		(vm_protect(task_self(), addr + pagesize * 2, pagesize,
			0, VM_PROT_NONE) != KERN_SUCCESS)) {
		return 1;
	}

	/*
	 * start stack at top minus a0-a3 save area
	 */

	alpha_state.r30 = addr + pagesize * 2 - 16; 

	alpha_state.r26 = (long)_exit;/* if this thread return, exit */
	alpha_state.r27 = start_pc;
	alpha_state.pc  = start_pc;
	alpha_state.r29 = (long)&_gp;

	*state_type = ALPHA_THREAD_STATE;
	*state_size = ALPHA_THREAD_STATE_COUNT;
	*state = (long *)&alpha_state;

	return 0; 
}
