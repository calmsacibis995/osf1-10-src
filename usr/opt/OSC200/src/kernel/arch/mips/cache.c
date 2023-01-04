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
static char	*sccsid = "@(#)$RCSfile: cache.c,v $ $Revision: 1.2.4.2 $ (DEC) $Date: 1992/04/14 15:06:58 $";
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
 * derived from cache.c	2.1	(ULTRIX/OSF)	12/3/90";
 */

/* ------------------------------------------------------------------ */
/* | Copyright Unpublished, MIPS Computer Systems, Inc.  All Rights | */
/* | Reserved.  This software contains proprietary and confidential | */
/* | information of MIPS and its suppliers.  Use, disclosure or     | */
/* | reproduction is prohibited without the prior express written   | */
/* | consent of MIPS.                                               | */
/* ------------------------------------------------------------------ */

/*
 * cache.c -- MIPS cache control operations
 */

#include <machine/cpu.h>
#include <machine/pmap.h>

#include <vm/vm_map.h>
#include <sys/proc.h>
#include <sys/buf.h>


ptrace_user_flush( map, addr, bcnt)
	vm_map_t map;
	vm_offset_t addr;
{
	int rem;
	for (; bcnt; bcnt -= rem, addr += rem) {
		/*
		 * calculate bytes to flush in this page
		 */
		rem = MIPS_PGBYTES - (addr & VA_OFFMASK);
		if (rem > bcnt)
			rem = bcnt;

		/* Find physical address */
		addr = pmap_extract( vm_map_pmap(map), addr);
		if (addr == 0)
		        continue;
	
		clean_icache(PHYS_TO_K0(addr), rem);
	}
}

/*
 * cacheflush() -- cacheflush(addr, bcount, cache) system call routine
 */
int
cacheflush(p, args, retval)
	struct proc *p;
	void *args;
	int *retval;
{
	struct args {
		unsigned addr;
		unsigned bcount;
		int cache;
	} *uap = (struct args *) args;	

	return(user_flush(uap->addr, uap->bcount, uap->cache));
}

/*
 * user_flush(addr, bcnt, cache) -- flush caches indicated by cache for
 * user addresses addr .. addr+bcnt-1
 */
user_flush(addr, bcnt, cache)
	register vm_offset_t addr;
	register unsigned bcnt;
{

	if (!useracc((caddr_t)addr, bcnt, B_READ) && !grow(addr))
		return(EFAULT);
	pmap_cache_flush(active_pmap, addr, bcnt, cache, NV_OK);
	return(0);
}


/*
 * bufflush(bp) -- flush cache that aliases to buffer bp if necessary
 */
bufflush(bp)
	register struct buf *bp;
{
	vm_offset_t addr;
	vm_offset_t bcnt;
	register pmap_t	pmap;

	addr = (unsigned)bp->b_un.b_addr;
	bcnt = bp->b_bcount;
	if ((bp->b_flags & B_PHYS) && IS_KUSEG(addr))
		pmap = vm_map_pmap(bp->b_proc->task->map);
	else
		pmap = pmap_kernel();

	pmap_cache_flush(pmap, addr, bcnt, BCACHE, NV_NOTOK);
}

/*
 * page_flush(addr) -- flush i and d caches that aliases with page btop(addr)
 */
page_flush(addr)
	caddr_t addr;
{
	page_iflush(addr);
	page_dflush(addr);
}

/*
 * clean_cache(addr, bcnt) -- flush i and d caches lines that alias with
 * address range addr .. addr+bcnt-1
 */
clean_cache(addr, bcnt)
	caddr_t addr;
	unsigned bcnt;
{
	clean_icache(addr, bcnt);
	clean_dcache(addr, bcnt);
}

