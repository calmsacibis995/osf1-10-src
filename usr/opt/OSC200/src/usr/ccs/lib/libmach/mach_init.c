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
static char	*sccsid = "@(#)$RCSfile: mach_init.c,v $ $Revision: 4.2.4.4 $ (DEC) $Date: 1993/12/15 20:31:22 $";
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
#if !defined(lint) && !defined(_NOIDENT)

#endif

#define	MACH_INIT_SLOTS		1
#include <mach_init.h>
#include <mach.h>

extern void init_reply_port();
extern void mig_init();

port_t		task_self_ =  PORT_NULL;
port_t		task_notify_ =  PORT_NULL;
port_t		thread_reply_ = PORT_NULL;

port_t		name_server_port = PORT_NULL;
port_t		environment_port = PORT_NULL;
port_t		service_port = PORT_NULL;

vm_size_t	vm_page_size;


port_array_t	mach_init_ports;
unsigned int	mach_init_ports_count;

int		mach_init()
{
	vm_statistics_data_t vm_stat;

	/*
	 * undefine the macros defined in mach_init.h so that we
	 * can make the real kernel calls
	 */

#undef task_self
#undef thread_reply
#undef task_notify


	/* Assure idempotency of mach_init() within the task */

	if(task_self_ != PORT_NULL)
		return;

	/*
	 *	Get the important ports into the cached values,
	 *	as required by "mach_init.h".
	 */
	 
	task_self_ = task_self();
	thread_reply_ = thread_reply();
	task_notify_ = task_notify();

	/*
	 *	Initialize the single mig reply port
	 */

	mig_init(0);

	/*
	 *	Cache some other valuable system constants
	 */

	vm_statistics(task_self_, &vm_stat);
	vm_page_size = vm_stat.pagesize;
	
	/*
	 *	Find those ports important to every task.
	 */
	 
	if (mach_ports_lookup(task_self_, &mach_init_ports, &mach_init_ports_count) != KERN_SUCCESS)
		mach_init_ports_count = 0;

	name_server_port = mach_init_ports_count > (unsigned int)NAME_SERVER_SLOT 
			? mach_init_ports[NAME_SERVER_SLOT] : PORT_NULL;
	environment_port = mach_init_ports_count > ENVIRONMENT_SLOT ? mach_init_ports[ENVIRONMENT_SLOT] : PORT_NULL;
	service_port     = mach_init_ports_count > SERVICE_SLOT ? mach_init_ports[SERVICE_SLOT] : PORT_NULL;
	

	/* get rid of out-of-line data so brk has a chance of working */
	(void) vm_deallocate(task_self(),(vm_address_t)mach_init_ports,
				vm_page_size);


	return(0);
}

int		(*mach_init_routine)() = mach_init;

#ifndef	lint
/*
 *	Routines which our library must suck in, to avoid
 *	a later library from referencing them and getting
 *	the wrong version.
 */
static replacements()
{
	fork();
}
#endif	lint

void catch_exception_raise()
{
}
