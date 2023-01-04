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
static char	*sccsid = "@(#)$RCSfile: parallel.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:26:26 $";
#endif 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * Copyright (C) 1988,1989 Encore Computer Corporation.  All Rights Reserved
 *
 * Property of Encore Computer Corporation.
 * This software is made available solely pursuant to the terms of
 * a software license agreement which governs its use. Unauthorized
 * duplication, distribution or sale are strictly prohibited.
 *
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


#include <cpus.h>
#include <mach_assert.h>
#include <mach_ldebug.h>
#include <ser_compat.h>

#if	NCPUS > 1

#include <kern/processor.h>
#include <kern/thread.h>
#include <kern/sched_prim.h>
#include <kern/parallel.h>
#include <kern/assert.h>

#if	SER_COMPAT
lock_data_t	default_uni_lock;
#endif

int	umast_calls = 0;
int	umast_to_master = 0;
int	umast_block = 0;
int	umast_release = 0;
int	umast_rel_cpu = 0;
int	umast_rel_block = 0;
int	umast_reset = 0;
int	umast_totreset = 0;
int	umast_force = 0;
int	umast_force_block = 0;

#define	UMAST_STAT(action)	(action)


void unix_master()
{
	register thread_t t = current_thread();
	
	UMAST_STAT(umast_calls++);
	if (! (++( t->unix_lock )))	{

		UMAST_STAT(umast_to_master++);
		/* thread_bind(t, master_processor); */
		t->bound_processor = master_processor;

		if (cpu_number() != master_cpu) {
			UMAST_STAT(umast_block++);
			t->interruptible = FALSE;
			thread_block();
		}
	}
	assert(cpu_number() == master_cpu &&
	    current_thread()->bound_processor == master_processor);
}

void unix_release()
{
	register thread_t t = current_thread();

	UMAST_STAT(umast_release++);
	t->unix_lock--;
	if (t->unix_lock < 0) {
		/* thread_bind(t, PROCESSOR_NULL); */
		t->bound_processor = PROCESSOR_NULL;
#if	notnow
		if (cpu_number() == master_cpu) {
			UMAST_STAT(umast_rel_block++);
			thread_block();
		}
#endif
		UMAST_STAT(umast_rel_cpu++);
	}
}

unix_release_force()
{
	register thread_t t = current_thread();

	umast_force++;
	t->unix_lock = -1;
	t->bound_processor = PROCESSOR_NULL;
	if (cpu_number() == master_cpu) {
		umast_force_block++;
		thread_block();
	}
}

void unix_reset()
{
	register thread_t	t = current_thread();

	UMAST_STAT(umast_reset++);
	if (t->unix_lock != -1)
		umast_totreset += t->unix_lock;
	if (t->unix_lock != -1)
		t->unix_lock = 0;
}

#endif	/* NCPUS > 1 */

#if	MACH_LDEBUG || MACH_ASSERT
int
syscall_on_master()
{
#if	NCPUS > 1
	return cpu_number() == master_cpu;
#else
	return 1;
#endif
}
#endif
