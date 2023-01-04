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
static char *rcsid = "@(#)$RCSfile: vm_machdep.c,v $ $Revision: 1.2.4.9 $ (DEC) $Date: 1993/01/08 17:16:43 $";
#endif
#ifndef lint
static char	*sccsid = "@(#)vm_machdep.c	9.1	(ULTRIX/OSF)	10/21/91";
#endif	lint

/************************************************************************
 *									*
 *			Copyright (c) 1990 by				*
 *		Digital Equipment Corporation, Maynard, MA		*
 *			All rights reserved.				*
 *									*
 *   This software is furnished under a license and may be used and	*
 *   copied  only  in accordance with the terms of such license and	*
 *   with the  inclusion  of  the  above  copyright  notice.   This	*
 *   software  or  any  other copies thereof may not be provided or	*
 *   otherwise made available to any other person.  No title to and	*
 *   ownership of the software is hereby transferred.			*
 *									*
 *   The information in this software is subject to change  without	*
 *   notice  and should not be construed as a commitment by Digital	*
 *   Equipment Corporation.						*
 *									*
 *   Digital assumes no responsibility for the use  or  reliability	*
 *   of its software on equipment which is not supplied by Digital.	*
 *									*
 ************************************************************************/

/*
 * Modification History: alpha/vm_machdep.c
 *
 *  28-May-91 -- afd
 *	Gutted this file.  The only thing left for OSF is pagemove().
 */

#include <machine/cpu.h>
#include <machine/pmap.h>
#include <sys/buf.h>
#include <sys/unix_defs.h>
#include <mach/mach_types.h>
#include <vm/vm_kern.h>
#include <vm/vm_page.h>
#include <vm/vm_vppage.h>
#include <vm/heap_kmem.h>
#include <sys/map.h>
#include <vm/vm_tune.h>
#include <vm/vm_debug.h>

/*
 * Patchable items.
 */

vm_offset_t stackinc = 0;

#define	BP_MAP_PERCENT 80			/* percent mapped for I/O */
int bp_map_percent = 0;
vm_size_t bp_map_size = 0;
int bp_map_maxpercent = 20;			/* Maximum kernel_map */

/*
 * This value is used to reduce the resource map size.
 * It is primarily a guess at how well the swap subsystem
 * and file systems will kluster a page list into a single I/O.
 * For example with the default parameters the resource map size
 * on a 32 bit machine with a 4K page size is less than two pages
 * when the machine has 16 Mb of physical memory.
 */

#define	BP_MAP_KLUSTER	4
int bp_map_kluster = 0;


static struct map *bp_map_map;
static int bp_map_want;
static vm_offset_t bp_map_addr;
static vm_map_t bp_mapin_submap;
udecl_simple_lock_data(,bp_map_lock)
extern void bp_map_free();
extern vm_offset_t bp_map_alloc();

/*
 * Number of cluster buffers
 */

static int cluster_bp_max;
static int cluster_bp_allocated;
static int cluster_bp_active;
static struct buf *cluster_bp;
udecl_simple_lock_data(, cluster_bp_lock)
extern int vm_managed_pages;

vm_machdep_init()
{
	register vm_size_t size, mapsize;
	register int pages, mapentries;
	vm_offset_t min, max;
	struct buf *bp;

	if (stackinc == 0) stackinc = PAGE_SIZE * 4;

	usimple_lock_init(&bp_map_lock);
	usimple_lock_init(&cluster_bp_lock);

	if (!bp_map_size) {
		if (!bp_map_percent) bp_map_percent = BP_MAP_PERCENT;
		pages = vm_managed_pages * bp_map_percent / 100;
		bp_map_size = pages;
	}
	else pages = bp_map_size;

	/*
	 * Adjust kluster size.
	 */

	if (!bp_map_kluster) bp_map_kluster = BP_MAP_KLUSTER;

	/*
	 * Add in the map values for get_cluster_bp
	 */

	mapsize = ptoa(pages) + vm_tune_value(clustermap);
	pages += atop(vm_tune_value(clustermap));

	/*
	 * Don't allow ridiculous map size
	 */

	size = vm_map_max(kernel_map) - vm_map_min(kernel_map);
	size = (size * bp_map_maxpercent) / 100;
	if (mapsize > size) {
		mapsize = size;
	}

	bp_mapin_submap = kmem_suballoc(kernel_map, &min, &max,
				mapsize, FALSE);

	if (bp_mapin_submap == VM_MAP_NULL) 
		panic("vm_machdep_init: failed to create bp_mapin_submap");

	bp_map_addr = vm_map_min(bp_mapin_submap);

	mapentries = (pages / bp_map_kluster) + 2;
	bp_map_map = (struct map *) 
		h_kmem_zalloc_memory(mapentries * sizeof (struct map), FALSE);
	if (bp_map_map == (struct map *) 0)
		panic("vm_machdep_init: unable to allocate bp_map_map");

	rminit(bp_map_map, atop(mapsize), (long) 1, "bp_map", mapentries);


	cluster_bp_max = vm_tune_value(clustermap)/vm_tune_value(clustersize);
	cluster_bp_allocated = 1;
	bp = (struct buf *) h_kmem_fast_zalloc_memory(&cluster_bp, 
		sizeof (struct buf), 1, FALSE);
	if (bp == BUF_NULL) 
		panic("vm_machdep_init: unable to allocate cluster bp");
	else h_kmem_fast_free(&cluster_bp, bp);

	/*
	 * The strategy for TB maintenace is delayed until resource free time.
	 * Hence we must assure the TB is clean of our future potential
	 * translations at this point.
	 */

	pmap_unload(bp_map_addr, mapsize, TB_SYNC_ALL);
}

/*
 * Move pages from one kernel virtual address to another.
 * Both addresses are assumed to be valid,
 * and size must be a multiple of the machine page size.
 */
void
pagemove(from, to, size)
	vm_offset_t from, to;
	long size;
{
	pmap_pagemove(from, to, size);
}

#if	UBC_PAGE_CHECK || BP_MAPIN_CHECK
extern vm_offset_t vm_first_phys, vm_last_phys;
#endif

/*
 * The b_un.b_addr for each bp in a dp list if of 
 * the same memory type (kseg, or seg1).
 */

struct buf *
get_cluster_bp(register struct buf *dp, int rw)
{
	register vm_size_t bsize, bcount;
	register struct buf *bp, *lbp;
	register int npf, s;
	vm_size_t size;
	kern_return_t ret;
	register vm_offset_t va, bpva, pa;
	register enum {kseg_mem, seg1_mem} mem_type;
	vm_prot_t prot;

	for (lbp = dp->av_forw, bcount = 0, bsize = 0; 
			lbp != dp; lbp = lbp->av_forw) {
		bsize += round_page(lbp->b_bcount);
		bcount += lbp->b_bcount;
	}

	if (!bsize || !bcount) panic("get_cluster_bp: illegal size");

	s = splbio();
	usimple_lock(&cluster_bp_lock);
	if (cluster_bp_allocated >= cluster_bp_max) {
		if (cluster_bp_allocated == cluster_bp_active) {
			usimple_unlock(&cluster_bp_lock);
			(void) splx(s);
			return (struct buf *) 0;
		}
		bp = (struct buf *) h_kmem_fast_zalloc_memory(&cluster_bp, 
				sizeof (struct buf), 1, FALSE);
	}
	else {
		bp = (struct buf *) h_kmem_fast_zalloc_memory(&cluster_bp, 
				sizeof (struct buf), 1, FALSE);
		if (bp == BUF_NULL) {
			usimple_unlock(&cluster_bp_lock);
			(void) splx(s);
			return (struct buf *) 0;
		}
		else if (cluster_bp_allocated == cluster_bp_active)
				cluster_bp_allocated++;
	}

	cluster_bp_active++;
	usimple_unlock(&cluster_bp_lock);
	(void) splx(s);

	/*
	 * Have bp now acquire map.
	 */

	npf = atop(bsize);
	va = bp_map_alloc(npf, FALSE);

	/*
	 * Unable to acquire map.
	 */

	if (va == (vm_offset_t) 0) {
		s = splbio();
		usimple_lock(&cluster_bp_lock);
		cluster_bp_active--;
		h_kmem_fast_free(&cluster_bp, bp);
		usimple_unlock(&cluster_bp_lock);
		(void) splx(s);
		return (struct buf *) 0;
	}


	/*
	 * Have resource and can now do the remappings.
	 */

	lbp = dp->av_forw;
	if (IS_KSEG_VA(lbp->b_un.b_addr)) mem_type = kseg_mem;
	else if (IS_SEG1_VA(lbp->b_un.b_addr)) mem_type = seg1_mem;
	else panic("get_cluster_bp: seg0 memory not supported");

	bp->b_un.b_addr = (caddr_t) (va + 
		((vm_offset_t) (lbp->b_un.b_addr) & page_mask));
	bp->b_bufsize = bsize;
	bp->b_bcount = bcount;

	if (!rw) prot = VM_PROT_WRITE|VM_PROT_READ;
	else prot = VM_PROT_READ;

	for (; lbp != dp; lbp = lbp->av_forw) {
		if (mem_type == seg1_mem)
			for (npf = atop(round_page(lbp->b_bcount)),
				bpva = trunc_page(lbp->b_un.b_addr);
				npf; 
				npf--, bpva += PAGE_SIZE, va += PAGE_SIZE) {
				pa = pmap_extract(kernel_pmap, bpva);
				if (pa == 0)
					panic("get_cluster_bp:invalid mapping");
				ret = pmap_load(va, pa, PAGE_SIZE, 
					prot, TB_SYNC_NONE);
				if (prot & VM_PROT_WRITE) pmap_clear_modify(pa);
				if (ret != KERN_SUCCESS) goto failed;
			}
		else {
			size = round_page(lbp->b_bcount);
			pa = trunc_page(lbp->b_un.b_addr);
			pa = KSEG_TO_PHYS(pa);
			ret = pmap_load(va, pa, size, prot, TB_SYNC_NONE);
			if (prot & VM_PROT_WRITE) pmap_clear_modify(pa);
			if (ret != KERN_SUCCESS) goto failed;
			va += size;
		}
				
	}
	return bp;

failed:
	bpva = trunc_page(bp->b_un.b_addr);
	size = va - bpva;
	if (size) pmap_unload(bpva, size, TB_SYNC_ALL);
	bp_map_free(bpva, atop(bp->b_bufsize));
	s = splbio();
	usimple_lock(&cluster_bp_lock);
	h_kmem_fast_free(&cluster_bp, bp);
	cluster_bp_active--;
	usimple_unlock(&cluster_bp_lock);
	(void) splx(s);
	return (struct buf *) 0;
}

void
put_cluster_bp(register struct buf *bp)
{
	register vm_offset_t offset;
	register vm_size_t size;

	offset = (vm_offset_t) (bp->b_un.b_addr) & page_mask;
	size = round_page(bp->b_bcount + offset);
	pmap_unload(trunc_page(bp->b_un.b_addr), size, TB_SYNC_ALL);
	bp_map_free(trunc_page(bp->b_un.b_addr), atop(size));
	usimple_lock(&cluster_bp_lock);
	cluster_bp_active--;
	h_kmem_fast_free(&cluster_bp, bp);
	usimple_unlock(&cluster_bp_lock);
}

bp_mapin(register struct buf *bp)
{
	register int npf;
	register vm_offset_t offset, va;
	kern_return_t ret;

	/*
	 * bp->b_un.b_addr must contain the correct intra-page offset
	 * in its low-order bits.  Usually, the value is 0.
	 */
	offset = (vm_offset_t) (bp->b_un.b_addr) & page_mask;
	npf = atop(round_page(bp->b_bcount + offset));

	if (npf == 1) {
#if	BP_MAPIN_CHECK
		if ((vm_offset_t) (bp->b_un.b_addr) < vm_first_phys || 
			(vm_offset_t) bp->b_un.b_addr > vm_last_phys)
			panic("bp_mapin: bp_mapin_check");
#endif	/* BP_MAPIN_CHECK */
		bp->b_un.b_addr
			= (caddr_t)(PHYS_TO_KSEG(page_to_phys(bp->b_pagelist))
				    + offset);
		return 0;
	}


	/*
	 * I/O spans a page boundary.
	 */

	va = bp_map_alloc(npf, TRUE);

	if (ret = bp_map(bp, va, npf)) {
		bp_map_free(va, npf);
		return ret;
	}
	bp->b_un.b_addr = (caddr_t) (va + offset);
	bp->b_flags |= B_REMAPPED;	
	return 0;
	
	
}

bp_map(register struct buf *bp, 
	vm_offset_t kva, 
	register int npf)
{
	register vm_page_t pp;
	register vm_offset_t phys, addr;
	register int i;
	kern_return_t ret;

	pp = bp->b_pagelist;

	for (i = 0, addr = kva; i < npf;  i++, addr += PAGE_SIZE) {
		phys = page_to_phys(pp);
		pp = pp->pg_pnext;
		ret = pmap_load(addr, phys, PAGE_SIZE,
			VM_PROT_READ|VM_PROT_WRITE, TB_SYNC_NONE);
		if (ret != KERN_SUCCESS) goto failed;
		pmap_clear_modify(phys);
	}
	return 0;
failed:
	if(i) pmap_unload(kva, ptoa(npf - i) , TB_SYNC_ALL);
	return ret;
}

bp_mapout(register struct buf *bp)
{
	register int npf;
	register vm_offset_t offset, va;

	if ((bp->b_flags & B_REMAPPED) == 0) return 0;

	offset = (vm_offset_t) (bp->b_un.b_addr) & page_mask;
	npf = atop(round_page(bp->b_bcount + offset));
	va = trunc_page(bp->b_un.b_addr);
	pmap_unload(va, ptoa(npf), TB_SYNC_ALL);
	bp_map_free(va, npf);
	return 0;
}

vm_offset_t
bp_map_alloc(int npf, boolean_t canwait) 
{
	register long pf;
	register int s;
	register vm_offset_t va;

	s = splbio();
	usimple_lock(&bp_map_lock);
	while ((pf = rmalloc(bp_map_map, npf)) == (long) 0) {
		if (canwait == FALSE) {
			usimple_unlock(&bp_map_lock);
			(void) splx(s);
			return (vm_offset_t) 0;
		}
		bp_map_want++;
		(void) mpsleep((vm_offset_t) &bp_map_want, PZERO,
			"BP_MAPIN", FALSE, simple_lock_addr(bp_map_lock),
			MS_LOCK_ON_ERROR|MS_LOCK_SIMPLE);	
	}
	usimple_unlock(&bp_map_lock);
	(void) splx(s);

	va =  bp_map_addr + ptoa(pf - 1);
	return va;
}

void
bp_map_free(register vm_offset_t addr,
	register int npf)
{
	register long maddr;
	register int s;

	maddr = atop((addr - bp_map_addr) + PAGE_SIZE);
	s = splbio();
	usimple_lock(&bp_map_lock);
	rmfree(bp_map_map, npf, maddr);
	if (bp_map_want) {
		bp_map_want = 0;
		thread_wakeup((vm_offset_t) &bp_map_want);
	}
	usimple_unlock(&bp_map_lock);
	(void) splx(s);
}

#if	UBC_PAGE_CHECK

ubc_load_page_check(vm_page_t pp,
	vm_offset_t offset,
	vm_size_t size)
{
	vm_offset_t phys;

	phys = KSEG_TO_PHYS(pp->pg_addr);
	if (phys < vm_first_phys || phys > vm_last_phys)
		panic("ubc_load_page_check:  bad pg_addr field");
	else return pp->pg_addr;
}

ubc_page_zero_page_check(vm_page_t pp,
	vm_offset_t offset,
	vm_size_t size)
{
	vm_offset_t phys;

	phys = KSEG_TO_PHYS(pp->pg_addr);
	if (phys < vm_first_phys || phys > vm_last_phys)
		panic("ubc_page_zero_page_check:  bad pg_addr field");
	else bzero(pp->pg_addr + offset, size);
}

#endif	/* UBC_PAGE_CHECK */
