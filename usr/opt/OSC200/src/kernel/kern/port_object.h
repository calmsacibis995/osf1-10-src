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
 *	@(#)$RCSfile: port_object.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:26:34 $
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
 * Port "kernel object" declarations
 *
 */

#ifndef	_KERN_PORT_OBJECT_H_
#define _KERN_PORT_OBJECT_H_

#include <mach_km.h>
#include <mach_net.h>

typedef	enum {
		PORT_OBJECT_NONE,
#if	MACH_NET
		PORT_OBJECT_NET,
#endif	/* MACH_NET */
		PORT_OBJECT_TASK,
		PORT_OBJECT_THREAD,
		PORT_OBJECT_PAGING_REQUEST,
		PORT_OBJECT_PAGER,
		PORT_OBJECT_HOST,
		PORT_OBJECT_HOST_PRIV,
		PORT_OBJECT_PROCESSOR,
		PORT_OBJECT_PSET,
		PORT_OBJECT_PSET_NAME,
#if	MACH_KM
 		PORT_OBJECT_MONITOR
#endif	/* MACH_KM */
} port_object_type_t;

typedef struct {
		port_object_type_t kp_type;
		int		kp_object;
} port_object_t;

#endif	/* _KERN_PORT_OBJECT_H_ */
