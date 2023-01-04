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
 *	@(#)$RCSfile: netmemory_defs.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 05:11:37 $
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
 * All rights reserved.  The CMU software License Agreement specifies
 * the terms and conditions for use and redistribution.
 */
/*
 * OSF/1 Release 1.0
 */
/*
 *	File:	netmemory_defs.h
 *	Author:	Joseph S. Barrera III
 *
 *	Copyright (C) 1989, Joseph S. Barrera III
 *
 *	C definitions for netmemory mig interface.
 *
 */

#ifndef	NETMEMORY_DEFS
#define	NETMEMORY_DEFS		1

#include <mach/error.h>
#include <servers/errorlib.h>


#define	NETMEMORY_SUCCESS		ERR_SUCCESS
#define NETMEMORY_INVALID_ARGUMENT	(SERV_NETMEMORY_MOD | 0x1)
#define	NETMEMORY_RESOURCE_SHORTAGE	(SERV_NETMEMORY_MOD | 0x2)

typedef mach_error_t netmemory_return_t;

typedef struct fault_info {
    int read_faults;
    int write_faults;
    int protection_faults;
} fault_info_t;

#endif	/* NETMEMORY_DEFS */
