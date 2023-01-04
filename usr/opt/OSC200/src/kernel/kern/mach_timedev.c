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
static char	*sccsid = "@(#)$RCSfile: mach_timedev.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:26:04 $";
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
/*
 *	File:	mach_timedev.c
 *	Author:	Joseph S. Barrera III, Randall Dean
 *
 *	Machine-independent time psuedo-device.
 */
#include <mach/vm_param.h>
#include <vm/vm_kern.h>
#include <sys/time.h>
#include <sys/errno.h>

struct timeval	*mach_tv = 0;
struct timezone	*mach_tz = 0;

int		mach_time_frame = 0;

timeopen()
{
	extern struct timeval time;
	extern struct timezone tz;
	struct tm {
		struct timeval	tv;
		struct timezone	tz;
	} *tm = 0;

	if (mach_tv) {
		return 0;
	}
	if (! (tm = (struct tm *) kmem_alloc(kernel_map, PAGE_SIZE))) {
		return ENOMEM;
	}
	mach_tv = &tm->tv;
	mach_tz = &tm->tz;
	*mach_tv = time;
	*mach_tz = tz;
	mach_time_frame = pmap_phys_to_frame(pmap_extract(pmap_kernel(), tm));
	return 0;
}

timeclose()
{
	return 0;
}

timemap(dev, offset, prot)
	dev_t		dev;
	vm_offset_t	offset;
	vm_prot_t	prot;
{
	if (prot & VM_PROT_WRITE) {
		return -1;
	}
	return mach_time_frame;
}
