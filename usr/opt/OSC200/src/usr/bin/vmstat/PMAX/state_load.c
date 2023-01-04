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
static char	*sccsid = "@(#)$RCSfile: state_load.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 01:08:45 $";
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

static struct mips_thread_state mips_state;

int
machine_state_load(state_type, state, state_size, start_pc)
	int *state_type;
	int **state;
	int *state_size;
	int start_pc;
{
	extern int _gp[];
	vm_offset_t addr, pagesize = getpagesize();

	(void) bzero(&mips_state, sizeof (mips_state));
	if (vm_allocate(task_self(), &addr, pagesize * 3, 1)
		!= KERN_SUCCESS) return 1;
	else if (
		(vm_protect(task_self(), addr, pagesize, 0, VM_PROT_NONE) !=
			KERN_SUCCESS) ||
		(vm_protect(task_self(), addr + pagesize * 2, pagesize,
			0, VM_PROT_NONE) != KERN_SUCCESS)) return 1;
	/*
	 * start stack at top minus a0-a3 save area
	 */

	else mips_state.r29 = addr + pagesize * 2 - 16;

	mips_state.pc = start_pc;
	mips_state.r28 = &_gp;

	*state_type = MIPS_THREAD_STATE;
	*state_size = MIPS_THREAD_STATE_COUNT;
	*state = (int *)&mips_state;
	return 0;
}
