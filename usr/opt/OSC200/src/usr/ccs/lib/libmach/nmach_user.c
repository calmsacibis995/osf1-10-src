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
static char	*sccsid = "@(#)$RCSfile: nmach_user.c,v $ $Revision: 4.2.2.2 $ (DEC) $Date: 1993/01/15 18:03:56 $";
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
/*
 * OSF/1 Release 1.0
 */

#include <mach/mach_types.h>

/*
 * Support for old port_{en,dis}able interfaces.
 *
 * NOTE: There's no synchronization among multiple uses of these routines.
 */

port_t		PORT_ENABLED = (-1);

kern_return_t	port_enable(target_task, port)
	task_t		target_task;
	port_t		port;
{
	extern kern_return_t	port_set_add();
	extern kern_return_t	port_set_allocate();
	kern_return_t		kr;

	if (PORT_ENABLED == (-1) &&
	    (kr = port_set_allocate(target_task, &PORT_ENABLED))
							!= KERN_SUCCESS)
			return kr;

	return(port_set_add(target_task, PORT_ENABLED, port));
}

kern_return_t	port_disable(target_task, port)
	task_t		target_task;
	port_t		port;
{
	extern kern_return_t	port_set_remove();
	extern kern_return_t	port_set_allocate();
	kern_return_t		kr;

	if (PORT_ENABLED == (-1) &&
	    (kr = port_set_allocate(target_task, &PORT_ENABLED))
							!= KERN_SUCCESS)
			return kr;

	/*
	 * Return code compatiblity with old port_disable(). 
	 */
	if ((kr = port_set_remove(target_task, port)) == KERN_NOT_IN_SET)
		return KERN_SUCCESS;
	else
		return kr;
}

#include "mach_user.c"
