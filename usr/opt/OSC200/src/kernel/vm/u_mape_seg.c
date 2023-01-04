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
static char *rcsid = "@(#)$RCSfile: u_mape_seg.c,v $ $Revision: 1.1.12.12 $ (DEC) $Date: 1993/12/02 22:07:23 $";
#endif
#include <sys/unix_defs.h>
#include <sys/buf.h>
#include <mach/kern_return.h>
#include <mach/boolean.h>
#include <mach/machine/vm_types.h>
#include <mach/vm_prot.h>
#include <mach/vm_inherit.h>
#include <vm/vm_object.h>
#include <vm/vm_swap.h>
#include <vm/vm_anon.h>
#include <vm/vm_page.h>
#include <vm/vm_vppage.h>
#include <sys/vp_swap.h>
#include <sys/time.h>
#include <sys/vnode.h>
#include <sys/specdev.h>
#include <sys/mount.h>
#include <sys/buf.h>
#include <sys/uio.h>
#include <sys/user.h>
#include <vm/vm_tune.h>
#include <vm/vm_vp.h>
#include <vm/vm_mmap.h>
#include <vm/vm_umap.h>
#include <vm/u_mape_seg.h>
#include <sys/mman.h>
#include <sys/stat.h>

extern int
	u_seg_fault(),
	u_seg_dup(),
	u_seg_unmap(),
	u_seg_msync(),
	u_seg_lockop(),
	u_seg_swap(),
	u_seg_core(),
	u_seg_control(),
	u_seg_protect(),
	u_seg_check_protect(),
	u_seg_kluster(),
	u_seg_copy(),
	u_seg_grow();

struct vm_map_entry_ops u_seg_mape_ops = {
	&u_seg_fault,		/* fault */
	&u_seg_dup,		/* dup */
	&u_seg_unmap,		/* unmap */
	&u_seg_msync,		/* msync */
	&u_seg_lockop,		/* lockop */
	&u_seg_swap,		/* swap */
	&u_seg_core,		/* corefile */
	&u_seg_control,		/* control */
	&u_seg_protect,		/* protect */
	&u_seg_check_protect,	/* check protection */
	&u_seg_kluster,		/* kluster */
	&u_seg_copy,		/* copy */
	&u_seg_grow,		/* grow */
};


udecl_simple_lock_data(, u_seg_cache_lock)

#define	U_SEG_CACHE_MAX		50
int u_seg_cache_max = 0;
int u_seg_cached;
vm_seg_object_t u_seg_cache_head;

/*
 * Hash queue of referenced seg objects.
 */

zone_t u_seg_zone;
#define	U_SEG_NHASH	32			/* Default hash size */
int u_seg_nhash = 0;
#define	USEG_HASH(VOP,SEGNUM)						\
	((((vm_offset_t)(VOP)) + (SEGNUM)) & (u_seg_nhash - 1))

struct u_seg_hash_lock {
	udecl_simple_lock_data(, hl_slock)
	unsigned int
		hl_lock : 1,
		hl_want : 31;
};

typedef struct u_seg_hash_lock *hlock_t;
hlock_t u_seg_hash_lock;

#define	u_seg_hash_release(VOP,SEGNUM) {				\
	register hlock_t HL;						\
	HL = u_seg_hash_lock + USEG_HASH(VOP, SEGNUM);			\
	usimple_lock(&HL->hl_slock);					\
	if (HL->hl_want) {						\
		HL->hl_want = 0;					\
		thread_wakeup((vm_offset_t) HL);			\
	}								\
	HL->hl_lock = 0;						\
	usimple_unlock(&HL->hl_slock);					\
}

boolean_t u_seg_enabled = TRUE;
vm_offset_t u_seg_start, u_seg_end;
vm_size_t u_seg_size, u_seg_mask, u_seg_shift;

#define	round_seg(VA)	(((VA) + u_seg_mask) & ~u_seg_mask)
#define	trunc_seg(VA)	((VA) & ~u_seg_mask)
#define	atos(VA)	((VA) >> u_seg_shift)
#define	stoa(S)		((S) << u_seg_shift)

/*
 * Call by the pmap to establish whether
 * address segmentation is supported.
 */

u_seg_init(vm_offset_t start, 
	vm_offset_t end, 
	vm_size_t size,
	vm_offset_t mask)
{

	u_seg_start = start;
	u_seg_end = end;
	u_seg_size = size;
	u_seg_mask = mask;

}

u_seg_vm_init()
{
	register int i;
	register hlock_t hlp;

	u_seg_enabled = vm_tune_value(segmentation);

	if (u_seg_enabled == FALSE) return;

	usimple_lock_init(&u_seg_cache_lock);
	if (!u_seg_cache_max) u_seg_cache_max = U_SEG_CACHE_MAX;

	u_seg_zone = zinit((vm_size_t) sizeof(struct vm_seg),
		sizeof(struct vm_seg) * 1024, PAGE_SIZE, "segment map");
	if (!u_seg_nhash) u_seg_nhash = U_SEG_NHASH;
	else {
		for (i = 1; i < u_seg_nhash; i << 1);
			u_seg_nhash = i;
	}


	u_seg_hash_lock = (hlock_t)
		kalloc(u_seg_nhash * sizeof (struct u_seg_hash_lock));
	for (hlp = u_seg_hash_lock, i = 0; i < u_seg_nhash; i++, hlp++) {
		hlp->hl_lock = 0;
		hlp->hl_want = 0;
		usimple_lock_init(&hlp->hl_slock);
	}
	return;
}

extern void u_seg_cache_trim();

#define u_seg_cache_remove(SOP) {					\
	u_seg_cached--;							\
	(SOP)->sego_flags &= ~OSEG_CACHED;				\
	if ((SOP)->sego_cfl == (SOP))		 			\
		u_seg_cache_head = VM_SEG_OBJECT_NULL;			\
	else {								\
		if ((SOP) == u_seg_cache_head)				\
			u_seg_cache_head = (SOP)->sego_cfl;		\
		(SOP)->sego_cfl->sego_cbl = (SOP)->sego_cbl;		\
		(SOP)->sego_cbl->sego_cfl = (SOP)->sego_cfl;		\
	}								\
}

/*
 */

u_seg_cache_object(register vm_seg_object_t segobjp)
{
	register vm_seg_object_t sop;

	usimple_lock(&u_seg_cache_lock);
	u_seg_cached++;
	segobjp->sego_flags |= OSEG_CACHED;
	if (u_seg_cache_head == VM_SEG_OBJECT_NULL) {
		u_seg_cache_head =  segobjp;
		segobjp->sego_cfl = segobjp->sego_cbl = segobjp;
	}
	else {
		sop = u_seg_cache_head;
		segobjp->sego_cfl = sop;
		segobjp->sego_cbl = sop->sego_cbl;
		sop->sego_cbl->sego_cfl = segobjp;
		sop->sego_cbl = segobjp;
	}

	vm_object_unlock(segobjp);
	if (u_seg_cached >= u_seg_cache_max) u_seg_cache_trim(segobjp);
	usimple_unlock(&u_seg_cache_lock);
}

/*
 * Remove all cached segments referencing the given vnode and
 * indicate that the seg object should not be cached later if
 * it is currently in use.
 */

void
u_seg_uncache_vnode(struct vnode *vp)
{
	register vm_seg_object_t sop;
	register struct vm_vp_object *vop;
	register vm_seg_t seg;

	if (vp->v_object && vm_object_type(vp->v_object) == OT_VP) {
		vop = (struct vm_vp_object *)vp->v_object;
		usimple_lock(&vop->vo_seglock);
		seg = vop->vo_seglist;

		while (seg) {

			sop = seg->seg_obj;
			vm_object_lock(sop);
			sop->sego_ref_count++;
			sop->sego_res_count++;
			sop->sego_flags |= OSEG_ERROR; /* prevent later caching */
			vm_object_unlock(sop);
			if (!u_seg_global_destroy(sop)) break;
			seg = seg->seg_vnext;
		}
		usimple_unlock(&vop->vo_seglock);
	}
}

/*
 * Clear the cache of all segments referencing 
 * vnodes on this mount point.  Called by doumount
 * to remove cached segments which reference vnodes
 * on this mount point.
 */

int 
u_seg_cache_clear(register struct mount *mp)
{
	register vm_seg_t sp;
	register vm_seg_object_t sop, sopl, sopn;

top:
	usimple_lock(&u_seg_cache_lock);
	sopn = u_seg_cache_head;
	if (sopn != VM_SEG_OBJECT_NULL) sopl = sopn->sego_cbl;
	sop = VM_SEG_OBJECT_NULL;
	for (;;) {
		if (sopn == VM_SEG_OBJECT_NULL || sop == sopl) break;
		else {
			sop = sopn;
			sopn = sop->sego_cfl;
		}
		if (!vm_object_lock_try(sop)) continue;
		
		for (sp = sop->sego_seglist; sp != VM_SEG_NULL; 
			sp = sp->seg_onext) 
			if (sp->seg_vop->vo_vp->v_mount == mp) break;
		if (sp == VM_SEG_NULL) {
			vm_object_unlock(sop);
			continue;
		}
		sop->sego_ref_count++;
		sop->sego_res_count++;
		vm_object_unlock(sop);
		usimple_unlock(&u_seg_cache_lock);
		if (!u_seg_global_destroy(sop)) goto top;
		else return;
	}
	usimple_unlock(&u_seg_cache_lock);
	return;
}

/*
 * Enter and exit with cache lock held
 */

void
u_seg_cache_trim(register vm_seg_object_t segobjp)
{
	register vm_seg_t sp;
	register vm_seg_object_t sop, sopl;

	sop = u_seg_cache_head;
	if (sop != VM_SEG_OBJECT_NULL) sopl = sop->sego_cbl;
	else return;
	for (; u_seg_cached >= u_seg_cache_max;) {
		if ((sop == segobjp) || !vm_object_lock_try(sop)) {
			if (sop == sopl) break;
			else {
				sop = sop->sego_cfl;
				continue;
			}
		}
		usimple_unlock(&u_seg_cache_lock);
		sop->sego_ref_count++;
		sop->sego_res_count++;
		vm_object_unlock(sop);
		if (u_seg_global_destroy(sop)) {
			usimple_lock(&u_seg_cache_lock);
			return;
		}
		else usimple_lock(&u_seg_cache_lock);
		sop = u_seg_cache_head;
		if (sop != VM_SEG_OBJECT_NULL)
			sopl = sop->sego_cbl;
		else
			return;
	}
}

/*
 * Remove segp from vop.
 */

void
u_seg_vopseg_remove(register struct vm_vp_object *vop,
	register vm_seg_t sp)
{
	register vm_seg_t psp, csp;

	for (psp = VM_SEG_NULL, csp = vop->vo_seglist; csp != VM_SEG_NULL;
		psp = csp, csp = csp->seg_vnext) {
		if (csp != sp) continue;
		else if (csp == vop->vo_seglist) 
			vop->vo_seglist = csp->seg_vnext;
		else psp->seg_vnext = csp->seg_vnext;
		return;
	}
	panic("u_seg_vopseg_remove: seg not found in vop");
}

u_seg_destroy(register vm_seg_t segp)
{
#define	u_seg_destroy(SEGP) ZFREE(u_seg_zone, (SEGP))
	u_seg_destroy(segp);
}

/*
 * Global segment destruction.
 */

int
u_seg_global_destroy(register vm_seg_object_t segobjp)
{
	register vm_seg_t segp, nsegp;
	register struct vm_vp_object *vop;

	segp = segobjp->sego_seglist;
	vop = segp->seg_vop;

	usimple_lock(&vop->vo_seglock);
	vm_object_lock(segobjp);
	
	if (segobjp->sego_ref_count > 1) {
		segobjp->sego_ref_count--;
		segobjp->sego_res_count--;
		vm_object_unlock(segobjp);
		usimple_unlock(&vop->vo_seglock);
		return 1;
	}
	else {
		u_seg_vopseg_remove(vop, segp);
		vm_object_unlock(segobjp);
		usimple_unlock(&vop->vo_seglock);
		vrele(vop->vo_vp);
		nsegp = segp->seg_onext;
		segobjp->sego_nseg--;
		u_seg_destroy(segp);
		segp = nsegp;
	}

	/*
	 * This global segment can no longer be looked up
	 * Free the other member segments now.
	 */


	for  (; segp != VM_SEG_NULL; segp = nsegp, segobjp->sego_nseg--) {
		nsegp = segp->seg_onext;
		vop = segp->seg_vop;
		usimple_lock(&vop->vo_seglock);
		u_seg_vopseg_remove(vop, segp);
		usimple_unlock(&vop->vo_seglock);
		vrele(vop->vo_vp);
		u_seg_destroy(segp);
	}
	
	if (segobjp->sego_flags & OSEG_CACHED) {
		usimple_lock(&u_seg_cache_lock);
		u_seg_cache_remove(segobjp);
		usimple_unlock(&u_seg_cache_lock);
	}
	pmap_seg_destroy(segobjp->sego_pmap);
	vm_object_free(segobjp);
	return 0;
}

kern_return_t
u_seg_global_allocate(
	vm_seg_object_t *segobjpp,
	vm_seg_t *segpp,
	vm_offset_t segstart,
	vm_offset_t segoff,
	struct vm_vp_object *vop, 
	vm_offset_t offset,
	vm_size_t segsize,
	int segnum,
	vm_prot_t prot,
	int flags)
{
	kern_return_t ret;
	register vm_seg_object_t segobjp;
	vm_seg_object_t segobj;
	register vm_seg_t segp;
	pmap_seg_t psegp;
	register hlock_t hl;
	register int hindex;
	struct stat sb;

	if ((ret = vm_object_allocate(OT_SEG, u_seg_size, (char *) 0, 
		(vm_object_t *) &segobj)) != KERN_SUCCESS) return ret;

	if ((ret = pmap_seg_alloc(&psegp)) != KERN_SUCCESS) {
		vm_object_deallocate(segobj);
		return ret;
	}

	segobjp = segobj;
	segobjp->sego_flags = (flags & MAP_FIXED) ? OSEG_FIXED : 0;
	segobjp->sego_segbase = segstart;
	segobjp->sego_pmap = psegp;
	segobjp->sego_nseg = 1;

	ZALLOC(u_seg_zone, segp, vm_seg_t);
	(void) bzero(segp, sizeof (struct vm_seg));
	segobjp->sego_nseg = 1;
	segobjp->sego_seglist = segp;

	segp->seg_obj = segobj;
	segp->seg_vop = vop;
	segp->seg_offset = offset;
	segp->seg_start = segoff;
	segp->seg_size = segsize;
	segp->seg_prot = prot;

	/*
	 * The hash lock has to be taken again
	 * to prevent other mapper from discovering
	 * the object we are about to associate with this vop.
	 */

	VREF(vop->vo_vp);
	hindex = USEG_HASH(vop, segnum);
	hl = u_seg_hash_lock + hindex;
	usimple_lock(&hl->hl_slock);
	usimple_lock(&vop->vo_seglock);
	segp->seg_vnext = vop->vo_seglist;
	vop->vo_seglist = segp;
	usimple_unlock(&vop->vo_seglock);
	usimple_unlock(&hl->hl_slock);

	*segobjpp = segobjp;
	*segpp = segp;
	return KERN_SUCCESS;
	
}

/*
 * Note will return with the hash sleep lock bit
 * set for a creator when we're unable to find the segment.
 * It is the caller's repsonsiblity to release the lock by
 * calling u_seg_hash_release.  Having this lock on the hash
 * queue prevents other seg object creations targeted to this
 * hash bucket.  Note lookups are still allowed on the hash bucket.
 */
 
vm_seg_object_t
u_seg_global_lookup(struct vm_vp_object *vop, 
		vm_seg_t *segp,
		vm_offset_t segbase,
		vm_offset_t segoff,
		vm_offset_t offset,
		vm_size_t segsize,
		int segnum,
		vm_prot_t prot,
		int flags)
{
	register vm_seg_object_t sop;
	register vm_seg_t sp;
	register hlock_t hl;
	register int hindex;
	struct stat sb;

	hindex = USEG_HASH(vop, segnum);
	hl = u_seg_hash_lock + hindex;
	usimple_lock(&hl->hl_slock);
top:
	usimple_lock(&vop->vo_seglock);
	for (sp = vop->vo_seglist; sp; sp = sp->seg_vnext)
		if (sp->seg_offset == offset &&
			sp->seg_start == segoff && 
			sp->seg_size == segsize &&
			sp->seg_prot == prot) {
			sop = sp->seg_obj;
			if ((sop->sego_seglist != sp) || (flags & MAP_FIXED && 
				sop->sego_segbase != segbase)) continue;
			vm_object_lock(sop);
			sop->sego_ref_count++;
			sop->sego_res_count++;
			usimple_unlock(&vop->vo_seglock);
			usimple_unlock(hl);
			if (sop->sego_flags & OSEG_CACHED) {
				usimple_lock(&u_seg_cache_lock);
				u_seg_cache_remove(sop);
				usimple_unlock(&u_seg_cache_lock);
			}
			vm_object_unlock(sop);
			*segp = sp;
			return sop;
		}
	usimple_unlock(&vop->vo_seglock);
	if (hl->hl_lock) {
		do {
			assert_wait((vm_offset_t) hl, FALSE);
			hl->hl_want = 1;
			usimple_unlock(hl);
			thread_block();
			usimple_lock(hl);
		} while (hl->hl_lock);
		goto top;
	}
	hl->hl_lock = 1;
	usimple_unlock(hl);
	return VM_SEG_OBJECT_NULL;
	
}

#define	u_seg_unlock_lookup(SOP) {					\
	vm_object_lock((vm_object_t) (SOP));				\
	if ((SOP)->sego_flags & OSEG_LWANT) {				\
		thread_wakeup((vm_offset_t) (SOP));			\
		(SOP)->sego_flags &= ~OSEG_LWANT;			\
	}								\
	(SOP)->sego_flags &= ~OSEG_LLOCK;				\
	vm_object_unlock((vm_object_t) (SOP));				\
}

/*
 * Assumes object can't change during lookup.
 */

enum seg_ret
u_seg_lookup(vm_seg_object_t segobj, 
	vm_seg_t *segprev, 
	struct vm_vp_object *vop,
	vm_offset_t segoff,
	vm_size_t segsize,
	vm_prot_t prot)
{
	register vm_seg_t sp, psp;
	register vm_offset_t segend, rsegend;
	struct stat sb;

	segend = round_page(segoff + segsize);
	vm_object_lock(segobj);
retry:
	for (sp = segobj->sego_seglist, psp = VM_SEG_NULL; sp != VM_SEG_NULL; 
		psp = sp, sp = sp->seg_onext) {
		rsegend = round_page(sp->seg_start + sp->seg_size);
		if (segoff == sp->seg_start) {
			if (((sp->seg_start + sp->seg_size) != 
				(segoff + segsize)) || (prot != sp->seg_prot) ||
				(vop != sp->seg_vop))
				goto conflict;
			vm_object_unlock(segobj);
			*segprev = sp;
			return SEG_LOADED;
		}
		else if (segoff < sp->seg_start) {
			if (segend > sp->seg_start) goto conflict;
			else break;
		}
		else if (segoff >= rsegend) continue;
		else goto conflict;
	}

	/*
	 * Found a position.  Now anchor the llookup.
	 */

	if (segobj->sego_flags & OSEG_LLOCK) {
		do {
			segobj->sego_flags |= OSEG_LWANT;
			assert_wait((vm_offset_t) segobj, FALSE);
			vm_object_unlock(segobj);
			thread_block();
			vm_object_lock(segobj);
		} while (segobj->sego_flags & OSEG_LLOCK);
		goto retry;
	}
	else {
		segobj->sego_flags |= OSEG_LLOCK;
		vm_object_unlock(segobj);
		*segprev = psp;
		return SEG_EMPTY;
	}
	
conflict:
	vm_object_unlock(segobj);
	return SEG_CONFLICT;
}

vm_seg_t
u_seg_attach(register vm_seg_object_t segobjp, 
	vm_seg_t segprev, 
	struct vm_vp_object *vop, 
	vm_offset_t segoff,
	vm_offset_t offset, 
	vm_size_t segsize,
	vm_prot_t prot)
{
	register vm_seg_t sp;
	struct stat sb;

	VREF(vop->vo_vp);
	ZALLOC(u_seg_zone, sp, vm_seg_t);
	(void) bzero(sp, sizeof (struct vm_seg));

	sp->seg_start = segoff;
	sp->seg_size = segsize;
	sp->seg_obj = segobjp;
	sp->seg_prot = prot;
	sp->seg_vop = vop;
	sp->seg_offset = offset;

	vm_object_lock(segobjp);
	segobjp->sego_nseg++;
	if (segprev == VM_SEG_NULL) {
		sp->seg_onext = segobjp->sego_seglist;
		segobjp->sego_seglist = sp;
	}
	else {
		sp->seg_onext = segprev->seg_onext;
		segprev->seg_onext = sp;
	}
	usimple_lock(&vop->vo_seglock);
	sp->seg_vnext = vop->vo_seglist;
	vop->vo_seglist = sp;
	usimple_unlock(&vop->vo_seglock);
	if (segobjp->sego_flags & OSEG_LWANT) 
		thread_wakeup((vm_offset_t) segobjp);
	segobjp->sego_flags &= ~(OSEG_LLOCK|OSEG_LWANT);
	vm_object_unlock(segobjp);
	return sp;
}

/*
 * See if its possible to attach this mapping at 
 * a shared libary segment.
 */


kern_return_t
u_seg_create(vm_map_t map,
	struct vm_vp_object *vop,
	register struct vp_mmap_args *ap)
{
	register int nseg, i;
	register vm_offset_t va, end, addr, offset;
	vm_offset_t segstart, segend, segoff, segsize, freeaddr;
	register vm_seg_object_t segobj;
	vm_seg_object_t segobjp;
	register vm_seg_t segp;
	vm_seg_t segprev;
	vm_map_entry_t tmp_entry;
	vm_map_entry_t entry, map_entry;
	kern_return_t ret;
	enum seg_ret segret;
	boolean_t addrloaded, notfixed;
	int segnum;

	if (((struct u_map_private *)(map->vm_private))->um_lock_future)
		return KERN_INVALID_ADDRESS;

	va = *(ap->a_vaddr);
	if ((ap->a_flags & MAP_FIXED) == 0) {
		notfixed = TRUE;
		if (va == (vm_offset_t) 0) return KERN_INVALID_ADDRESS;
	}
	else notfixed = FALSE;
	if (va < u_seg_start || (va + ap->a_size) > u_seg_end) 
			return KERN_INVALID_ADDRESS;
	segstart = trunc_seg(va); 
	segend = round_seg(va + ap->a_size);
	end = va + ap->a_size;
	nseg = atos(segend - segstart);
	addrloaded = FALSE;

	vm_map_lock(map);

	for(offset = ap->a_offset, addr = va, segnum = 0; segstart < segend; 
		segstart += u_seg_size, addr = round_seg(addr + 1), segnum++) {
		segoff = addr - segstart;
		segsize = MIN((segstart + u_seg_size), end) - addr;
		if (vm_map_lookup_entry(map, addr, &tmp_entry)) {
			map_entry = tmp_entry;
			if (vm_object_type(map_entry->vme_object) != OT_SEG) {
				ret = KERN_NO_SPACE;
				goto failed;
			}
			else {
				segobj = (struct vm_seg_object *)
					(map_entry->vme_object);

				vm_mape_faultlock(map_entry);

				segret = u_seg_lookup(segobj, &segprev, 
						vop, segoff,
						segsize, ap->a_prot);

				if (segret == SEG_CONFLICT) {
					ret = KERN_NO_SPACE;
					goto failed;
				}

				ret = vm_map_entry_create(map, &entry);
				
				if (entry == VM_MAP_ENTRY_NULL) {
					if (segret == SEG_EMPTY)
						u_seg_unlock_lookup(segobj);
					goto failed;
				}


				segp = map_entry->vme_seg;
				if ((segoff + segsize) <= segp->seg_start) {
					entry->vme_end = segstart +
						segp->seg_start;
					entry->vme_start = map_entry->vme_start;
					map_entry->vme_start = entry->vme_end;
					map_entry = map_entry->vme_prev;
				}
				else if (segoff >= 
					(segp->seg_start + segp->seg_size)) {
					entry->vme_end = map_entry->vme_end;
					entry->vme_start = segstart +
						segp->seg_start +
						round_page(segp->seg_size);
					map_entry->vme_end = entry->vme_start;
				}
				else {
					ret = KERN_NO_SPACE;
				 	goto failed1;
				}

				/*
				 * Attach this segment if its not
				 * already attached.
				 */

				if (segret == SEG_EMPTY)
					 segp = u_seg_attach(segobj, segprev, 
						vop, segoff,
						offset + (addr - va), segsize,
						ap->a_prot);
				else segp = segprev;
				vm_object_reference(segobj);
			}
		}
		else {
			if (notfixed == FALSE) {
				if (addr & u_seg_mask) {
					ret = KERN_INVALID_ADDRESS;
					goto failed;
				}
				if (((tmp_entry != vm_map_to_entry(map)) && (tmp_entry->vme_end > segstart)) ||
			    	((tmp_entry->vme_next != vm_map_to_entry(map)) && (tmp_entry->vme_next->vme_start < segend))) {
					ret = KERN_NO_SPACE;
					goto failed;
				}
			}

			if (ap->a_flags & MAP_FIXED) {
				ret = (*map->vm_delete_map)(map, segstart,
					segstart + u_seg_size, FALSE); 
				if (ret != KERN_SUCCESS) goto failed;
			}
			if (ret = u_map_grow_vas(map, u_seg_size)) {
				goto failed;
			}
			lock_set_recursive(&map->vm_lock);
			freeaddr = segstart;
			ret = vm_map_space(map, &freeaddr, segend-segstart, 
					u_seg_mask,  
					((ap->a_flags & MAP_FIXED) ||
					(notfixed == FALSE)) ? 
					FALSE : TRUE, &tmp_entry);

			if (ret != KERN_SUCCESS) {
				lock_clear_recursive(&map->vm_lock);
				goto failed2;
			}
			else {
				vm_map_unlock(map);
				lock_clear_recursive(&map->vm_lock);
				if (notfixed == TRUE) {

					/*
					 * See if we start from the hint
					 */

					if (freeaddr != segstart) {
						if (freeaddr < u_seg_start ||
						(freeaddr + ap->a_size) 
						 > u_seg_end)  {
							ret = KERN_NO_SPACE;
							goto failed2;
						}
						segstart = freeaddr;
						va = addr = segstart;
						segend = round_seg(va + ap->a_size);
						end = va + ap->a_size;
						nseg = atos(segend - segstart);
						segsize = MIN((segstart + u_seg_size), end) - addr;
					}
					notfixed = FALSE;
				}
				ret = vm_map_entry_create(map, &entry);
				if (entry == VM_MAP_ENTRY_NULL) goto failed2;
			}
				
			map_entry = tmp_entry;

			/*
			 * Find or create the global segment
			 */

			segobj = u_seg_global_lookup(vop,
				&segprev, segstart, addr-segstart,
				offset + (addr - va), segsize, segnum,
				ap->a_prot, ap->a_flags);

			if (segobj == VM_SEG_OBJECT_NULL) {
				ret = u_seg_global_allocate(&segobjp,
					&segprev, segstart, addr-segstart, vop,
					offset + (addr - va), segsize, segnum,
					ap->a_prot, ap->a_flags);

				segobj = segobjp;

				/*
				 * Release hash create lock
				 */

				u_seg_hash_release(vop, segnum);

				if (ret != KERN_SUCCESS) {
					u_map_free_vas(map, u_seg_size);
					goto failed1;
				}
			}

			segp = segprev;
			entry->vme_start = segstart;
			entry->vme_end = segstart + u_seg_size;
			pmap_seg_load(vm_map_pmap(map), segobj->sego_pmap,
				segstart);

		}

		entry->vme_map = map;
		entry->vme_keep_on_exec = FALSE;
		entry->vme_inheritance = VM_INHERIT_COPY;
		entry->vme_protection = ap->a_prot;
		entry->vme_maxprot = ap->a_maxprot;
		entry->vme_object = (vm_object_t) segobj;
		entry->vme_seg = segp;
		entry->vme_ops = &u_seg_mape_ops;

		vm_map_entry_link(map, map_entry, entry);

		if ((map->vm_first_free == map_entry) && 
			(((map_entry == vm_map_to_entry(map)) &&
			(map_entry->vme_start == entry->vme_start)) ||
			((map_entry != vm_map_to_entry(map)) &&
			(map_entry->vme_end >= entry->vme_start))))
				map->vm_first_free = entry;

		if (addrloaded == FALSE) {
			addrloaded = TRUE;
			*(ap->a_vaddr) = addr;
		}
	}

	
	vm_map_unlock(map);
	return KERN_SUCCESS;

failed1:
	vm_map_entry_dispose(map,entry);
	map->vm_nentries--;
failed2:
	u_map_free_vas(map, u_seg_size);
	
failed:

	if (va != addr) u_map_delete(map, va, addr, TRUE);
	vm_map_unlock(map);
	return(ret);
}

/*
 * The segmentation has been broken.
 * We revert to the vop & aop pair rule
 * for the entire segment.
 * Assumption:
 *	map has been write locked by the
 *	upper level operation.
 */

kern_return_t
u_seg_to_anon(register vm_map_entry_t ep)
{
	register vm_map_entry_t fep, cep;
	register vm_map_t map;
	vm_map_entry_t tmp_entry;
	register vm_offset_t segbegin, segend;
	register vm_seg_object_t segobjp;
	register vm_seg_t segp;
	register vm_size_t vassize, segsize;
	struct vm_vp_object *vop;
	struct vm_anon_object *aop;
	vm_offset_t start;

	segbegin = trunc_seg(ep->vme_start);
	if (!vm_map_lookup_entry(ep->vme_map, segbegin, &tmp_entry)) 
			panic("u_seg_to_anon: segmentation corruption");
	else fep = cep = tmp_entry;

	segobjp = (vm_seg_object_t) (cep->vme_object);
	segend = segbegin + segobjp->sego_size;

	map = ep->vme_map;

	for (; cep != vm_map_to_entry(map) && cep->vme_start < segend; 
		cep = cep->vme_next) 
		if (cep != ep) vm_mape_faultlock(ep);


	pmap_seg_unload(vm_map_pmap(ep->vme_map), 
		segobjp->sego_pmap, segbegin); 
		
	for (cep = fep; 
		cep != vm_map_to_entry(map) && 
		cep->vme_start < segend; cep = cep->vme_next) {

		segp = cep->vme_seg;
		vassize = cep->vme_end - cep->vme_start;
		segsize = round_page(segp->seg_size);
		cep->vme_start = segbegin + segp->seg_start;
		cep->vme_end = cep->vme_start + segsize;
		vop = segp->seg_vop;
		VREF(vop->vo_vp);
		vm_object_allocate(OT_ANON, segsize,
			(caddr_t) TRUE, (vm_object_t *) &aop);
		aop->ao_flags = AF_PRIVATE;
		cep->vme_object = (vm_object_t) aop;
		cep->vme_offset = 0;
		aop->ao_bobject = (vm_object_t) vop;
		aop->ao_boffset = segp->seg_offset;
		cep->vme_ops = vm_object_to_vmeops(aop);
		vassize -= (cep->vme_end - cep->vme_start);
		if (vassize) u_map_free_vas(cep->vme_map, vassize);
		vm_object_deallocate(segobjp);
	}

	return KERN_SUCCESS;
}

/*
 * Note the address could be invalid because
 * of the way segment spanding is done.
 */

kern_return_t
u_seg_fault(register vm_map_entry_t ep,
	vm_offset_t va,
	register vm_size_t size,
	vm_prot_t access_type,
	vm_fault_t fault_type,
	vm_page_t *rpp)
{
	register vm_seg_t segp;
	vm_offset_t addr, segstart, segend, offset, end;
	vm_prot_t prot[VP_PAGELIST];
	vm_page_t plist[VP_PAGELIST+1], *pl; 
	register vm_page_t pp;
	register vm_prot_t *pprot;
	vm_size_t psize;
	int pcnt;
	vm_seg_object_t segobjp;
	kern_return_t ret;

	if (fault_type == VM_UNWIRE)
		panic("u_seg_fault: VM_UNWIRE operation");

	segp = ep->vme_seg;
	segstart = trunc_seg(ep->vme_start) + segp->seg_start;
	segend = segstart + segp->seg_size;

	if ((va < segstart || (va + size) > segend)) 
		return KERN_INVALID_ADDRESS;

	if ((access_type & ep->vme_protection) != access_type)
		return KERN_PROTECTION_FAILURE;

	if (fault_type == VM_WIRE) {
		ret = u_seg_to_anon(ep);
		if (ret != KERN_SUCCESS) return ret;
		else return (*ep->vme_fault)(ep, va, size, 
			access_type, fault_type, rpp);
	}

	segobjp = segp->seg_obj;

	offset = segp->seg_offset + (va - segstart);
	end = offset + size;
	for (addr = va; offset < end; offset += psize) {
		plist[0] = VM_PAGE_NULL;
		pcnt = (fault_type == VM_PAGEGET) ? 1 : VP_PAGELIST;
		if (size > VP_PAGELISTSIZE) 
			psize =  VP_PAGELISTSIZE;
		else psize = size;

		VOP_GETPAGE(segp->seg_vop->vo_vp, offset, 
			psize, prot, plist,
			pcnt, ep, addr, 1, u.u_cred, ret);
		if (ret) goto failed;
		else for (pprot = prot, pl = plist, pp = *pl; 
			pp != VM_PAGE_NULL; pl++, pprot++, pp = *pl) {
			if (pp->pg_offset >= offset && 
				pp->pg_offset < offset + psize) {

				vm_page_wait(pp);
				if (pp->pg_error) {
					ret = KERN_FAILURE;
					goto failed;
				}
				if ((ep->vme_protection & ~(*pprot)) !=
					ep->vme_protection) {
					ret = KERN_PROTECTION_FAILURE;
					goto failed;
				}
				else if (fault_type == VM_PAGEGET && 
					pp->pg_offset == offset) {
					*rpp = pp;
				}
				else {
					pmap_seg_enter(vm_map_pmap(ep->vme_map),
						segobjp->sego_pmap,
						addr, page_to_phys(pp),
						ep->vme_protection);
					ubc_page_release(pp, 0);
				}
				addr += PAGE_SIZE;
			}
			else ubc_page_release(pp, 0);
			*pl = VM_PAGE_EMPTY;
		}
	}

	return KERN_SUCCESS;

failed:
	vm_object_lock(segobjp);
	segobjp->sego_flags |= OSEG_ERROR;
	vm_object_unlock(segobjp);
	pl = plist;
	while (*pl != VM_PAGE_NULL) {
		pp = *pl;
		if (pp != VM_PAGE_EMPTY) {
			vm_page_wait(pp);
			ubc_page_release(pp, 0);
		}
		*pl++ = VM_PAGE_EMPTY;
	}
	return ret;
}

u_seg_unwire(register vm_map_entry_t ep,
	register vm_offset_t start,
	register vm_size_t len)
{
	panic("u_seg_unwire: not supported");
}

u_seg_bad_addr(register vm_map_entry_t ep,
	register vm_offset_t va,
	register vm_size_t len)
{
	register vm_seg_t sp;
	register vm_offset_t segstart;

	sp = ep->vme_seg;
	segstart = trunc_seg(ep->vme_start) + sp->seg_start;

#define u_seg_hole(SP,SEGSTART,VA,LEN)					\
	(((VA) < (SEGSTART) || 						\
	((VA) + (LEN)) > ((SEGSTART) + (SP)->seg_size)) ? 1 : 0)

#define	u_seg_bad_addr_(SP,SEGSTART,VA,LEN)				\
	(u_seg_hole(SP,SEGSTART,VA,LEN) ? KERN_INVALID_ADDRESS :	\
		KERN_SUCCESS)
	
	return u_seg_bad_addr_(sp,segstart,va,len);
}


u_seg_dup(register vm_map_entry_t ep,
	vm_offset_t addr,
	register vm_size_t size,
	register vm_map_entry_t newep,
	vm_copy_t copy)
{
	kern_return_t ret;

	switch (copy) {
	case VM_COPYMCOPY:
		if (ret = u_seg_bad_addr(ep, addr, size)) return ret;
		else if (ret = u_seg_to_anon(ep)) return ret;
		else return u_anon_dup(ep, addr, size, newep, copy);
	case VM_COPYU:
	{
		register vm_seg_object_t segobj;

		segobj = (vm_seg_object_t) (ep->vme_object);
		vm_object_reference(segobj);
		pmap_seg_load(vm_map_pmap(newep->vme_map), segobj->sego_pmap,
				trunc_seg(newep->vme_start));
		return KERN_SUCCESS;
	}
	default:
		return KERN_INVALID_ARGUMENT;
	}
}

/*
 * We rely on a vm_exec widget to tell us
 * whether the unload is for all address space.
 */

u_seg_unmap(register vm_map_entry_t ep,
	vm_offset_t addr,
	vm_size_t len)
{
	struct u_map_private *up = 
		(struct u_map_private *) (ep->vme_map->vm_private);
	vm_seg_object_t segobjp;
	register vm_offset_t start, end, vas;
	kern_return_t ret;

	if (!up->um_unload_all) {
		if (ret = u_seg_to_anon(ep)) return ret;
		
		/*
		 * The segment relationships have been destroyed.
		 * Note u_seg_to_anon restores the unused VAS resource.  
		 * This might have to be adjusted below.
		 */

		/*
		 * Totally consuming a segment hole.
		 */

		if ((addr < ep->vme_start && (addr + len) <= ep->vme_start) ||
			(addr >= ep->vme_end)) return KERN_SEGHOLE;
		/*
		 * Consuming all or part of the original segment part.
		 * A complex unmapping.
		 */

		else {
			if (addr < ep->vme_start) vas = ep->vme_start - addr;
			else vas = 0;
			if ((addr + len) > ep->vme_end)
				vas = vas + ((addr + len) - ep->vme_end );
			start = MAX(addr,ep->vme_start);
			end = MIN(addr + len, ep->vme_end);
			ret = u_anon_unmap(ep, start, end - start);
			if (ret == KERN_SUCCESS) {
				u_map_grow_vas(ep->vme_map, vas);
				return ret;
			}
			else return ret;
		}
	}
	else {
		segobjp = (vm_seg_object_t) (ep->vme_object);
		pmap_seg_unload(vm_map_pmap(ep->vme_map),
			segobjp->sego_pmap, addr);
	}
	return KERN_SUCCESS;
}

u_seg_msync(register vm_map_entry_t ep,
	vm_offset_t addr,
	register vm_size_t size,
	int flags)
{
	return u_seg_bad_addr(ep, addr,size);
}

u_seg_lockop(vm_map_entry_t ep,
	vm_offset_t va,
	vm_size_t len,
	vm_fault_t wire)
{
	register vm_offset_t segstart;
	register vm_seg_t sp;
	kern_return_t ret;


	if (wire == VM_UNWIRE) return KERN_SUCCESS;
	sp = ep->vme_seg;
	segstart = trunc_seg(ep->vme_start) + sp->seg_start;
	if (ret = u_seg_to_anon(ep)) return ret;
	else return (*ep->vme_lockop)(ep, va, min(len, (ep->vme_end - va)), wire);
}

u_seg_swap(register vm_map_entry_t ep, int rw)
{
	return 0;
}

u_seg_core(register vm_map_entry_t ep,
	register unsigned next, 
	register char *vec, 
	register int *vecsize)
{
	*vecsize = -1;
	return 0;
}

u_seg_control(register vm_map_entry_t ep,
	register vm_offset_t addr,
	register vm_size_t size,
	vm_control_t control,
	int arg)
{
	kern_return_t ret;

	switch (control) {
	case VMC_INHERITANCE:
		if (ret = u_seg_bad_addr(ep, addr, size)) return ret; 
		else if (ret = u_seg_to_anon(ep)) return ret;
		else ep->vme_inheritance = arg;
		break;
	case VMC_KEEP_ON_EXEC:
		ep->vme_keep_on_exec = arg;
		break;
	default:
		return KERN_INVALID_ARGUMENT;
	}	

	return KERN_SUCCESS;
}

u_seg_protect(register vm_map_entry_t ep,
	register vm_offset_t addr,
	vm_size_t size,
	vm_prot_t prot)
{
	kern_return_t ret;

	if (prot == ep->vme_protection) return 0;
	else {
		if (ret = u_seg_to_anon(ep)) return ret;
		else return u_anon_protect(ep, addr, size, prot);
	}
}

boolean_t
u_seg_check_protect(register vm_map_entry_t ep,
	register vm_offset_t addr,
	register vm_size_t size,
	register vm_prot_t prot)
{
	if ((ep->vme_protection & prot) == prot) return TRUE;
	else return FALSE;
}

u_seg_kluster(register vm_map_entry_t ep,
	register vm_offset_t addr,
	int pcnt,
	vm_offset_t *back, 
	vm_offset_t *forward)
{
	register vm_offset_t offset, noffset;
	register vm_size_t regsize;
	int cluster;
	register vm_seg_t sp;

	sp = ep->vme_seg;
	addr = addr - (trunc_seg(addr) + sp->seg_start);
	offset = sp->seg_offset + addr;
	addr = trunc_page(addr);

	cluster = vm_page_free_count - vm_page_kluster;
	if (cluster <= 0 || (pcnt = MIN(pcnt, cluster)) == 1) return 1;
	regsize = ptoa(pcnt - 1);

	noffset  = MIN(regsize,	
		(round_page(sp->seg_size) - (addr + PAGE_SIZE)));
	if (noffset) {
		*forward = offset + noffset;
		noffset = regsize - noffset;
	}
	else {
		noffset = regsize;	
		*forward = offset;
	}

	if (noffset) {
		noffset = MIN((addr - sp->seg_start), noffset);
		*back = offset - noffset;
	}
	else *back = offset;
	return 0;
}

u_seg_copy(vm_map_entry_t ep, vm_copy_op_t op)
{
	panic("u_seg_copy: illegal operation");
}

u_seg_grow(vm_map_entry_t ep,
	vm_prot_t prot,
	register vm_size_t size,
	as_grow_t direction)
{
	return KERN_NO_SPACE;
}

u_seg_oop_bad()
{
	panic("u_seg_oop_bad: operation not supported");
}

kern_return_t
u_seg_oop_pagesteal()
{
        return KERN_FAILURE;
}


struct vm_object_ops u_seg_oop = {
	OOP_KLOCK_TRY_K
	&u_seg_oop_bad,			/* lock try */
	OOP_UNKLOCK_K
	&u_seg_oop_bad,			/* unlock */
	&vm_ops_reference_def,		/* reference */
	&vm_ops_deallocate_def,		/* deallcoate */
	OOP_PAGEIN_K
	&u_seg_oop_bad,			/* pagein */
	OOP_PAGEOUT_K
	&u_seg_oop_bad,			/* pageout */
	OOP_SWAP_K
	&u_seg_oop_bad,			/* swapout */
	OOP_CONTROL_K
	&u_seg_oop_bad,			/* control */
	OOP_PAGECTL_K
	&u_seg_oop_bad,			/* page control */
	OOP_PAGESTEAL_K
	&u_seg_oop_pagesteal,		/* page steal */
};

extern int u_seg_oop_terminate();

struct vm_object_config seg_object_conf = {
	(int (*)()) 0,
	u_seg_oop_terminate,
	sizeof (struct vm_seg_object),
	&u_seg_oop,
	&u_seg_mape_ops,
};


u_seg_oop_terminate(register vm_seg_object_t segobjp)
{
	if ((segobjp->sego_flags & OSEG_ERROR) || (u_seg_cache_max == 0)) {
		vm_object_unlock(segobjp);
		u_seg_global_destroy(segobjp);
	}
	else {
		segobjp->sego_ref_count--;
		segobjp->sego_res_count--;
		u_seg_cache_object(segobjp);
	}
}
