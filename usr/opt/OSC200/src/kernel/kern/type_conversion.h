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
 *	@(#)$RCSfile: type_conversion.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:28:19 $
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
 * All rights reserved.  The CMU software License Agreement specifies
 * the terms and conditions for use and redistribution.
 */
/*
 * OSF/1 Release 1.0
 */
#ifndef	_KERN_TYPE_CONVERSION_H_
#define	_KERN_TYPE_CONVERSION_H_

#include <mach_km.h>

#include <mach/port.h>
#include <kern/task.h> 
#include <kern/thread.h>
#include <vm/vm_map.h>
#include <kern/host.h>
#include <kern/processor.h>

/*
 *	Conversion routines, to let Matchmaker do this for
 *	us automagically.
 */

extern task_t convert_port_to_task( /* port_t x */ );
extern thread_t convert_port_to_thread( /* port_t x */ );
extern vm_map_t convert_port_to_map( /* port_t x */ );
extern port_t convert_task_to_port( /* task_t x */ );
extern port_t convert_thread_to_port( /* thread_t x */ );

extern host_t convert_port_to_host( /* port_t x */ );
extern host_t convert_port_to_host_priv( /* port_t x */ );
extern processor_t convert_port_to_processor( /* port_t x */ );
extern processor_set_t convert_port_to_pset( /* port_t x */ );
extern processor_set_t convert_port_to_pset_name( /* port_t x */ );
extern port_t convert_host_to_port( /* host_t x */ );
extern port_t convert_processor_to_port( /* processor_t x */ );
extern port_t convert_pset_to_port( /* processor_set_t x */ );
extern port_t convert_pset_name_to_port( /* processor_set_t x */ );

#if     MACH_KM
#include <kern/kern_mon.h>

extern monitor_t convert_port_to_monitor();
extern port_t convert_monitor_to_port();
#endif /*  MACH_KM */

#endif	/* _KERN_TYPE_CONVERSION_H_ */
