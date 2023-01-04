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
static char	*sccsid = "@(#)$RCSfile: syscall_emulation.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:27:12 $";
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
 * Copyright (c) 1988 Carnegie-Mellon University
 * Copyright (c) 1987 Carnegie-Mellon University
 * All rights reserved.  The CMU software License Agreement specifies
 * the terms and conditions for use and redistribution.
 */
/*
 * OSF/1 Release 1.0
 */

#include <mach_emulation.h>

#include <mach/mach_types.h>
#include <kern/syscall_emulation.h>

#if	MACH_EMULATION
#include <mach/error.h>
#include <kern/task.h>
#include <kern/zalloc.h>
#include <mach/vm_param.h>

zone_t		eml_zone;
int		eml_max_emulate_count;
/*
 *  eml_init:	initialize user space emulation code
 */
eml_init()
{
	extern int nsysent;
	
	eml_max_emulate_count = nsysent;

	eml_zone = zinit( sizeof(struct eml_dispatch)+((eml_max_emulate_count-1) * sizeof (eml_routine_t)),
		 PAGE_SIZE, PAGE_SIZE, "emulation routines" );
		 
}

/*
 *   task_set_emulation:  [Server Entry]
 *   set up for user space emulation of syscalls within this task.
 */
kern_return_t task_set_emulation( task, routine_entry_pt, routine_number)
	task_t		task;
	eml_routine_t 	routine_entry_pt;
	int		routine_number;
{
    	int i;
    
	if ( task == TASK_NULL )
	        return( EML_BAD_TASK );

	if ( routine_number >= eml_max_emulate_count
	     || routine_number < 0 )
		return( EML_BAD_CNT );
/*
 * If either the task does not have an emulation vector, or if
 * it points to a common one, then give it a new one.
 */
	if (task->eml_dispatch != EML_DISPATCH_NULL) {
		/* XXX lock the old dispatch structure */
	}

	if ((task->eml_dispatch == EML_DISPATCH_NULL)
	     || (task->eml_dispatch->eml_ref > 1)) {
	        eml_dispatch_t old_vec = task->eml_dispatch;

		task->eml_dispatch = (eml_dispatch_t)zalloc( eml_zone );
		task->eml_dispatch->eml_ref = 1;
		task->eml_dispatch->disp_count = eml_max_emulate_count;
		/* XXX lock the new dispatch structure */

		if( old_vec == EML_DISPATCH_NULL ) {
			/*  zero the vector */
			for ( i = 0; i < task->eml_dispatch->disp_count; i++)
				task->eml_dispatch->disp_vector[i] = EML_ROUTINE_NULL;
		} else {
			old_vec->eml_ref--;

			/*  copy the parent's vector */
			for ( i = 0; i < task->eml_dispatch->disp_count; i++)
				task->eml_dispatch->disp_vector[i] = old_vec->disp_vector[i];
	     	}
	}

	task->eml_dispatch->disp_vector[routine_number] = routine_entry_pt;

	/* XXX unlock the dispatch structure */

	return( KERN_SUCCESS );
}

/*
 * eml_task_fork() [Exported]
 *
 *	Bumps the reference count on the common emulation
 *	vector.
 */

kern_return_t eml_task_fork(parent,child)
task_t parent,child;
{
	if( parent->eml_dispatch != EML_DISPATCH_NULL ) {
		/* XXX lock the dispatch structure */
		parent->eml_dispatch->eml_ref++;
	}

	child->eml_dispatch = parent->eml_dispatch;
	return(KERN_SUCCESS);    
}


/*
 * eml_task_exit() [Exported]
 *
 *	Cleans up after the emulation code when a process exits.
 */
 
kern_return_t eml_task_exit(task)
task_t task;
{
	if (task->eml_dispatch != EML_DISPATCH_NULL) {
		/* XXX lock the dispatch structure */
		if (--task->eml_dispatch->eml_ref == 0 ) 
			zfree(eml_zone,task->eml_dispatch);
		return(KERN_SUCCESS);
	}

    return(KERN_FAILURE);
}
#else

/*
 *   task_set_emulation:  [Server Entry]
 *   set up for user space emulation of syscalls within this task.
 */
kern_return_t task_set_emulation( task, routine_entry_pt, routine_number)
	task_t		task;
	eml_routine_t 	routine_entry_pt;
	int		routine_number;
{
	return( KERN_FAILURE );
}
#endif	MACH_EMULATION
