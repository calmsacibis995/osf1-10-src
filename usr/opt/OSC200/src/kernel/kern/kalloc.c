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
static char	*sccsid = "@(#)$RCSfile: kalloc.c,v $ $Revision: 4.2.10.3 $ (DEC) $Date: 1993/11/02 21:01:04 $";
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
/*
 *	File:	kern/kalloc.c
 *	Author:	Avadis Tevanian, Jr.
 *
 *	Copyright (C) 1985, Avadis Tevanian, Jr.
 *
 *	General kernel memory allocator.  This allocator is designed
 *	to be used by the kernel to manage dynamic memory fast.
 */

#include <sys/types.h>
#include <mach/vm_param.h>

#include <kern/zalloc.h>
#include <kern/kalloc.h>
#include <vm/vm_kern.h>
#include <vm/vm_object.h>
#ifdef MEMLOG
#include <sys/memlog.h>
#endif

/*
 *	All allocations of size less than PAGE_SIZE are rounded to the
 *	next highest power of 2.  This allocator is built on top of
 *	the zone allocator.  A zone is created for each potential size
 *	that we are willing to get in small blocks.
 *
 *	We assume that PAGE_SIZE is not greater than 32K;
 *	thus 16 is a safe array size for k_zone and k_zone_name.
 */

int first_k_zone = -1;
struct zone *k_zone[16];
static char *k_zone_name[16] = {
	"kalloc.1",		"kalloc.2",
	"kalloc.4",		"kalloc.8",
	"kalloc.16",		"kalloc.32",
	"kalloc.64",		"kalloc.128",
	"kalloc.256",		"kalloc.512",
	"kalloc.1024",		"kalloc.2048",
	"kalloc.4096",		"kalloc.8192",
	"kalloc.16384",		"kalloc.32768"
};

/*
 *  Max number of elements per zone.  zinit rounds things up correctly
 *  Doing things this way permits each zone to have a different maximum size
 *  based on need, rather than just guessing; it also
 *  means its patchable in case you're wrong!
 */
unsigned long k_zone_max[16] = {
      1024,		/*      1 Byte  */
      1024,		/*      2 Byte  */
      1024,		/*      4 Byte  */
      1024,		/*      8 Byte  */
      1024,		/*     16 Byte  */
      1024,		/*     32 Byte  */
      1024,		/*     64 Byte  */
      1024,		/*    128 Byte  */
      4096,		/*    256 Byte  */
      256,		/*    512 Byte  */
      128,		/*   1024 Byte  */
      128,		/*   2048 Byte  */
      64,		/*   4096 Byte  */
      64,		/*   8192 Byte  */
      64,		/*  16384 Byte  */
      64,		/*  32768 Byte  */
};

/*
 *	Initialize the memory allocator.  This should be called only
 *	once on a system wide basis (i.e. first processor to get here
 *	does the initialization).
 *
 *	This initializes all of the zones.
 */
/* kallocinitialized is a gross hack to allow us to initialize the 
   kalloc zones exactly when we need to. This depends on being called prior
   to going multithreaded. */
static int kallocinitialized = 0;

kallocinit()
{
	register int i, size;

	if (kallocinitialized) return;
	kallocinitialized++;

	/*
	 *	Allocate a zone for each size we are going to handle.
	 *	We specify non-paged memory.
	 */
	for (i = 0, size = 1; size <= PAGE_SIZE; i++, size <<= 1) {
		if (size < MINSIZE) {
			k_zone[i] = 0;
			continue;
		}
		if (size == MINSIZE) {
			first_k_zone = i;
		}
		k_zone[i] = zinit(size, k_zone_max[i] * size, PAGE_SIZE,
			k_zone_name[i]);
	}
}

caddr_t kalloc(size)
{
	register zindex, allocsize;
	caddr_t addr;

#ifdef MEMLOG
	if(memlog) {
		GET_CALLER(caller);
		memory_log(KALLOC_LOG, caller, size);
	}
#endif

	if (!kallocinitialized) kallocinit();

	/* compute the size of the block that we will actually allocate */

	allocsize = size;
	if (size <= PAGE_SIZE) {
		allocsize = MINSIZE;
		zindex = first_k_zone;
		while (allocsize < size) {
			allocsize <<= 1;
			zindex++;
		}
	}

	/*
	 * If our size is still small enough, check the queue for that size
	 * and allocate.
	 */

	if (allocsize <= PAGE_SIZE) {
		addr = (caddr_t) zalloc(k_zone[zindex]);
	} else {
		addr = (caddr_t) kmem_alloc(kernel_map, allocsize);
	}
	return(addr);
}

caddr_t kget(size)
{
	register zindex, allocsize;
	caddr_t addr;

#ifdef MEMLOG
	if(memlog) {
		GET_CALLER(caller);
		memory_log(KGET_LOG, caller, size);
	}
#endif
	/* compute the size of the block that we will actually allocate */

	allocsize = size;
	if (size <= PAGE_SIZE) {
		allocsize = MINSIZE;
		zindex = first_k_zone;
		while (allocsize < size) {
			allocsize <<= 1;
			zindex++;
		}
	}

	/*
	 * If our size is still small enough, check the queue for that size
	 * and allocate.
	 */

	if (allocsize <= PAGE_SIZE) {
		addr = (caddr_t) zget(k_zone[zindex]);
	} else {
		/* This will never work, so we might as well panic */
		panic("kget: oversized request");
	}
	return(addr);
}

kfree(data, size)
	caddr_t	data;
	long	size;
{
	register freesize, zindex;

#ifdef MEMLOG
	if(memlog) {
		GET_CALLER(caller);
		memory_log(KFREE_LOG, caller, size);
	}
#endif
	freesize = size;
	if (size <= PAGE_SIZE) {
		freesize = MINSIZE;
		zindex = first_k_zone;
		while (freesize < size) {
			freesize <<= 1;
			zindex++;
		}
	}

	if (freesize <= PAGE_SIZE) {
		zfree(k_zone[zindex], data);
	} else {
		kmem_free(kernel_map, data, freesize);
	}
}
