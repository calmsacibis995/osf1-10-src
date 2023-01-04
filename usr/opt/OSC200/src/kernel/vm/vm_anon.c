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
static char *rcsid = "@(#)$RCSfile: vm_anon.c,v $ $Revision: 1.1.13.2 $ (DEC) $Date: 1993/10/18 17:17:14 $";
#endif
#include <sys/unix_defs.h>
#include <vm/vm_map.h>
#include <vm/vm_swap.h>
#include <vm/vm_anon.h>
#include <vm/vm_page.h>
#include <vm/vm_anonpage.h>
#include <vm/vm_tune.h>
#include <kern/zalloc.h>
#include <sys/buf.h>
#include <vm/heap_kmem.h>
#include <vm/vm_debug.h>
#include <vm/vm_perf.h>
#include <sys/kernel.h>

int anon_klshift; 
int anon_klpages = 0;
int anon_klpagesize;
int anon_pagesinkl;
vm_offset_t anon_klsize;


struct vm_anon_swaphash *anon_swaphash;
vm_offset_t hanon_mask;
int nhanon = 0;
zone_t anon_lazy_zone;
extern int nproc;


#if	UNIX_LOCKS

/*
 * For anon locks on an MP machine
 */

simple_lock_t anon_lanon;
vm_offset_t anon_lanon_mask;
int anon_nlanon = 0;

#endif	/* UNIX_LOCKS */

a_init()
{
	register int i;
#if	UNIX_LOCKS
	register simple_lock_t alp;

	if ((1 << A_LSHIFT) < sizeof (struct vm_anon))
		printf("a_init: warning, A_LSHIFT wrong\n");
	if (anon_nlanon) {
		if (anon_nlanon & (anon_nlanon - 1)) {
			printf("a_init: anon lock size not power of two\n");
			anon_nlanon = ANON_NLANON;
		}
	}	
	else anon_nlanon = ANON_NLANON;
	anon_lanon_mask = anon_nlanon - 1;
	alp = anon_lanon = (simple_lock_t) 
		h_kmem_zalloc(sizeof (simple_lock_data_t) * anon_nlanon);
	for (i = anon_nlanon; i--; alp++) simple_lock_init(alp);
#endif	/* UNIX_LOCKS */

	anon_klshift = vm_tune_value(anonklshift);
	if (anon_klshift < page_shift) {
		printf("a_init: vt_anonklshift less than page shift 0x%x\n",
			anon_klshift);
		anon_klshift = ANON_KLSHIFT;
	}
	anon_klsize = 1 << anon_klshift;
	anon_pagesinkl = atop(anon_klsize);

	if (!anon_klpages) {
		if (vm_tune_value(anonklpages)) 
			anon_klpages = vm_tune_value(anonklpages);
		else anon_klpages = ANON_KLPAGES;
	}

	if (anon_klpages < 1) anon_klpages = ANON_KLPAGES;

	if (anon_klpages > atop(1 << anon_klshift)) 
		anon_klpages = atop(1 << anon_klshift);
	anon_klpagesize = ptoa(anon_klpages);

	/*
	 * Configure swap hash if not in eager mode
	 */

	if (!vm_swap_eager) {
		register struct vm_anon_swaphash *ahp;

		anon_lazy_zone = zinit((vm_size_t) sizeof(struct vm_anon),
				sizeof(struct vm_anon) * nproc * 40,
				PAGE_SIZE, "anon lazy cells");
		if (nhanon) {
			for (i = 1; i < nhanon; i <<= 1);
			nhanon = i;
		}
		else nhanon = A_SWAPHASH_SIZE;
		hanon_mask = nhanon - 1;
		anon_swaphash = (struct vm_anon_swaphash *)
			h_kmem_zalloc(sizeof (struct vm_anon_swaphash) 
				* nhanon);
		for (ahp = anon_swaphash, i = 0; i < nhanon; i++, ahp++)
			usimple_lock_init(&ahp->ah_lock);
	}
}

/*
 * Page allocation for a kernel anon cell
 * This is very simple minded because no klustering
 * is currently attempt for the kernel. zfod is only
 * done when the virtual page is owned by the pkernel_object.
 * Hence its currently not an issue.  
 */

a_kpage_alloc(register struct vm_anon *ap)
{
	vm_page_t plist[1];
	vm_page_t pp;
	struct vm_swap_object *sop;
	register struct vm_swap *sp;
	vm_offset_t soffset;

	sp = a_aptosp(ap, &soffset);
	sop = sp->vs_object;
	while (1) {
		pp = vm_anon_kpage_alloc(sop, soffset);
		if (pp == VM_PAGE_NULL) {
			a_unlock(ap);
			vm_wait();
			a_lock(ap);
			if (ap->an_page) {
				vm_page_hold(ap->an_page);
				a_unlock(ap);
				return;
			}
			continue;
		}
		else {
			ap->an_page = pp;
			pp->pg_hold = 1;
			a_unlock(ap);
			AN_TRACK_READ(ap);
			plist[0] = pp;
			pp->pg_pnext = VM_PAGE_NULL;
			vm_swap_io(plist, B_READ);
			break;
		}
	}
}

a_anon_getpage(register struct vm_anon_object *aop,
	vm_offset_t offset,
	vm_map_entry_t ep,
	vm_offset_t addr,
	alock_t lp,
	vm_page_t *pl,
	int pcnt,
	register struct vm_anon *ap)
{
	vm_offset_t forward, back;
	register int npages;
	register vm_offset_t noffset;
	register struct vm_anon **app;
	register vm_page_t pp;
	vm_page_t plist[1];

	npages = 1;
	a_lock(ap);
	if (ap->an_page != VM_PAGE_NULL) {
		vm_page_hold(ap->an_page);
		a_unlock(ap);
		pl[0] = ap->an_page;
		goto done;
	}
	else if (ap->an_refcnt > 1) {
		pp = vm_anon_page_alloc(aop, offset, lp, ap, TRUE, TRUE);
		pl[0] = ap->an_page;
		if (pp == VM_PAGE_NULL) {
			vm_page_hold(ap->an_page);
			a_unlock(ap);
			goto done;
		}
		else a_unlock(ap);
	}
	else {
		a_unlock(ap);
		ap = a_swap_xlate(ap, aop->ao_anon + atop(offset));
		pp = vm_anon_page_alloc(aop, offset, lp, ap, FALSE, TRUE);
		pl[0] = pp;
	}
	plist[0] = ap->an_page;
	pp->pg_pnext = VM_PAGE_NULL;	

	AN_TRACK_READ(ap);

	if (pcnt == 1 || (*ep->vme_kluster)(ep, addr, pcnt, &forward, &back))
		goto startio;

	/*
	 * We have to bring in at least the original page of interest.
	 */

	if (forward > offset) for(noffset = offset + PAGE_SIZE; 
			noffset <= forward; noffset += PAGE_SIZE) {
		app = aop->ao_anon + atop(noffset);
		ap = *app;
		a_lock(ap);	
		if (ap->an_page) {
			a_unlock(ap);
			break;
		}
		else if (ap->an_refcnt == 1) {
			a_unlock(ap);
			ap = a_swap_xlate(ap, app);
			pp = vm_anon_page_alloc(aop, noffset, lp, ap, 
				FALSE, FALSE);
		}
		else {
			pp = vm_anon_page_alloc(aop, noffset, lp, ap, 
				FALSE, FALSE);
			a_unlock(ap);
		}
		if (pp == VM_PAGE_NULL) goto startio;
		pl[npages++] = ap->an_page;
		vm_swap_sort_add(plist, ap->an_page);
		AN_TRACK_READ(ap);
	}
	
	if (back < offset) for(noffset = offset -= PAGE_SIZE; noffset >= back; 
			noffset -= PAGE_SIZE) {
		app = aop->ao_anon + atop(noffset);
		ap = *app;
		a_lock(ap);
		if (ap->an_page) {
			a_unlock(ap);
			break;
		}
		else if (ap->an_refcnt == 1) {
			a_unlock(ap);
			ap = a_swap_xlate(ap, app);
			pp = vm_anon_page_alloc(aop, noffset, lp, ap, 
				FALSE, FALSE);
		}
		else {
			pp = vm_anon_page_alloc(aop, noffset, lp, ap, 
				FALSE, FALSE);
			a_unlock(ap);
		}
		if (pp == VM_PAGE_NULL) goto startio;
		pl[npages++] = ap->an_page;
		vm_swap_sort_add(plist, ap->an_page);
		AN_TRACK_READ(ap);
	}
startio:
	pl[npages] = VM_PAGE_NULL;
	vm_swap_io(plist, B_READ);
	return 0;
done:
	pl[npages] = VM_PAGE_NULL;
	return 0;
	
}
	
/*
 * It is assumed the caller has in some way
 * prevented access to these anon pages.
 * The pages are returned held.
 * Its assumed the allocation is within the same kluster.
 */

struct vm_anon *
a_anon_pagezero_alloc(register struct vm_anon_object *aop,
	register vm_offset_t offset, 
	register alock_t lp)
{
	register struct vm_anon *ap;
	struct vm_anon **app;
	register vm_page_t pp;

	ap = a_anon_allocate();
	app = aop->ao_anon + atop(offset);
	*app = ap;
	pp = vm_anon_zeroed_page_alloc(aop, offset, lp, ap, FALSE, TRUE);
	lp->akl_anon++;
	vpf_ladd(zfod,1);
	vm_pageout_activate(pp, FALSE);
	return ap;
}

/*
 * Used to allocate ap and page for ap.
 * Its assumed space is already reserved.
 */


struct vm_anon *
a_anon_appage_alloc(struct vm_anon_object *aop, 
	vm_offset_t offset,
	register alock_t lp,
	register struct vm_anon **app)
{
	register struct vm_anon *ap;
	register vm_page_t pp;

	ap = a_anon_allocate();
	if (ap == NULL) panic("a_anon_appage_alloc: no anon");
	*app = ap;
	pp = vm_anon_page_alloc(aop, offset, lp, ap, FALSE, TRUE);
	lp->akl_anon++;
	vm_pageout_activate(pp, FALSE);
	return ap;
}

/*
 * Enters with cap locked and exits with it locked.
 */

struct vm_anon *
a_anon_cowpage_alloc(register struct vm_anon_object *aop, 
	register vm_offset_t offset,
	register alock_t lp,
	register struct vm_anon *cap,
	register struct vm_anon **app)
{
	register struct vm_anon *ap;
	register vm_page_t pp;

	/*
	 * If owner of this page then
	 * release ownership and remove from 
	 * kluster lock resident set.
	 */

	if (cap->an_page->pg_owner == aop) {
						
		cap->an_page->pg_owner = (struct vm_anon_object *) 0;
		a_unlock(cap);
		lk_slock(lp);	
		lp->akl_rpages--;
		vm_page_remove_object(&lp->akl_pagelist, cap->an_page);
		lk_sunlock(lp);
	}
	else a_unlock(cap);

	ap = a_anon_allocate();
	if (ap == NULL) panic("a_anon_cowpage_alloc: no anon");
	*app = ap;
	pp = vm_anon_page_alloc(aop, offset, lp, ap, FALSE, TRUE);

	a_lock(cap);
	if (cap->an_refcnt == 1) {
		lk_slock(lp);
		vm_anon_page_free(aop, offset, ap);
		lk_sunlock(lp);
		a_anon_free(ap);
		*app = cap;
		cap->an_page->pg_owner = aop;
		cap->an_page->pg_roffset = (long) offset - aop->ao_rbase;
		lk_slock(lp);
		vm_page_insert_object(&lp->akl_pagelist, cap->an_page);
		lp->akl_rpages++;
		lk_sunlock(lp);
		return (struct vm_anon *) 0;
	}
	else return ap;
}

long a_swap_warn_time;
long a_swap_warn_interval = 10;			/* Every N seconds */
long a_swap_warn_percent = 10;			/* Lower than 10 % */
char a_swap_warn_message[] = "swap space below %d percent free\n";
char a_swap_fail_message[] = "Unable to obtain requested swap space\n";

#define	a_swap_warn() 							\
	if (vm_swap_space < 						\
		((vm_total_swap_space * a_swap_warn_percent)/100)) 	\
		a_swap_warn_interval();

#define	a_swap_warn_interval() {					\
		if ((time.tv_sec - a_swap_warn_time) > 			\
			a_swap_warn_interval) {				\
			uprintf(a_swap_warn_message, 			\
				a_swap_warn_percent);			\
			printf(a_swap_warn_message, 			\
				a_swap_warn_percent);			\
			a_swap_warn_time = time.tv_sec;			\
		}							\
}	

a_noswap_warn()
{
	a_swap_warn_interval();
}

/*
 * Free an ap
 */

a_anon_free(register struct vm_anon *ap)
{
	AN_TRACK_FREE(ap);
	if (ap->an_type == ANT_LAZY) {
		if (ap->an_hasswap) a_swap_free(a_swap_hash_free(ap), TRUE);
		ZFREE(anon_lazy_zone, ap);
	}
	else a_swap_free(ap, (ap->an_type == ANT_XLATESWAP));
}

/*
 * Recover lazy swap cell which is no longer
 * needed because at least one pageout
 * was made against it.  
 */

struct vm_anon *
a_swap_xlate(register struct vm_anon *ap,
	register struct vm_anon **app)
{
	register struct vm_anon *sap;

	assert((ap->an_page == VM_PAGE_NULL) && (ap->an_refcnt == 1));
	if ((ap->an_type != ANT_LAZY) || (!ap->an_hasswap)) return ap;
	else {
		sap = a_swap_hash_free(ap);
		*sap = *ap;
		sap->an_type = ANT_XLATESWAP;
		*app = sap;
		ZFREE(anon_lazy_zone, ap);
		return sap;
	}
}

struct vm_anon *
a_swap_alloc(boolean_t canwait,
	boolean_t reserved,
	int swaptype, struct vm_swap *swap_hint)
{
	register struct vm_anon *ap;
	register struct vm_swap *sp;

	assert(swaptype != ANT_LAZY);

	ap = (struct vm_anon *) 0;
	swap_read_lock();

	if (!swap_hint) {
		simple_lock(&vm_swap_circular_lock);
		swap_hint = vm_swap_circular;
		if (vm_swap_circular)
			vm_swap_circular = vm_swap_circular->vs_fl;
		simple_unlock(&vm_swap_circular_lock);
	}

	if (!(sp = swap_hint)) {
		swap_read_unlock();
		a_noswap_warn();
		return ap;
	}

	do {
		swap_vslock(sp);
		if (sp->vs_freespace) {
			swap_read_unlock();
			sp->vs_freespace--;
			if (!(ap = sp->vs_anfree)) while (1) {
				ap = (struct vm_anon *)
					h_kmem_fast_alloc_memory_(sp->vs_hinfo,
						&sp->vs_anfree, 
						sizeof (struct vm_anon),
						MIN(VS_SWAP_GROW,
						sp->vs_freespace+1), FALSE);
				if (ap) break;
				else if (canwait) {
					swap_vsunlock(sp);
					vm_wait();
					swap_vslock(sp);
					if (ap = sp->vs_anfree) break;
					else continue;
				} else {
					/* restore free space */
					sp->vs_freespace++;
					swap_vsunlock(sp);
					return ap;
				}
			}
			sp->vs_anfree = ap->an_next;
			swap_vsunlock(sp);
			ap->an_type = swaptype;	
			ap->an_page = VM_PAGE_NULL;
			ap->an_cowfaults = 0;
			ap->an_refcnt = 1;
			if (reserved == FALSE) {
				swap_space_alloc(1);
				a_swap_warn();
			}
			return ap;
		}
		else swap_vsunlock(sp);
	} while ((sp = sp->vs_fl) != swap_hint);
	swap_read_unlock();
	a_noswap_warn();
	return ap;
}

boolean_t
a_swap_lazy_alloc(struct vm_anon *ap,
	register struct vm_page *pp,
	boolean_t canwait, struct vm_swap *swap_hint)
{
	register struct vm_anon *sap;
	register struct vm_anon_swaphash *ahp;

	/*
	 * See if we already have swap
	 */

	if ((ap->an_type != ANT_LAZY) || ap->an_hasswap) return TRUE;

	sap = a_swap_alloc(canwait, FALSE, ANT_LAZYSWAP, swap_hint);
	if (!sap) return FALSE;
	sap->an_anon = ap - vm_swap_lazy->vs_anbase;
	ahp = a_hash(ap);
	usimple_lock(&ahp->ah_lock);
	sap->an_next = ahp->ah_next;
	ahp->ah_next = sap;
	usimple_unlock(&ahp->ah_lock);
	ap->an_hasswap = 1;

	if (pp != VM_PAGE_NULL) {
		struct vm_swap *sp;
		register struct vm_swap_object *sop;
		vm_offset_t soffset;

		sp = a_aptosp(sap, &soffset);
		sop = sp->vs_object;
		vm_object_lock(sop);
		pp->pg_offset = soffset;
		pp->pg_object = (vm_object_t) sop;
		vm_page_insert_bucket(pp, sop, soffset);
		vm_object_unlock(sop);
	}
	return TRUE;
}

struct vm_anon *
a_swap_hash_free(register struct vm_anon *ap)
{
	register struct vm_anon_swaphash *ahp;
	register struct vm_anon *hap, *phap;
	register int i;

	assert(ap->an_type == ANT_LAZY);
	ahp = a_hash(ap);
	i = ap - vm_swap_lazy->vs_anbase;
	usimple_lock(&ahp->ah_lock);
	for (phap = (struct vm_anon *) 0, hap = ahp->ah_next; 
		hap; phap = hap, hap = hap->an_next)
		if (hap->an_anon == i) {
			if (phap) phap->an_next = hap->an_next;
			else ahp->ah_next = hap->an_next;
			usimple_unlock(&ahp->ah_lock);
			return hap;
		}
	usimple_unlock(&ahp->ah_lock);
	panic("a_swap_hash_free: ap not found");
}

struct vm_anon *
a_swap_hash_lookup(register struct vm_anon *ap)
{
	register struct vm_anon_swaphash *ahp;
	register struct vm_anon *hap;
	register int i;

	assert(ap->an_type == ANT_LAZY);
	ahp = a_hash(ap);
	i = ap - vm_swap_lazy->vs_anbase;
	usimple_lock(&ahp->ah_lock);
	for (hap = ahp->ah_next; hap; hap = hap->an_next)
		if (hap->an_anon == i) {
			usimple_unlock(&ahp->ah_lock);
			return hap;
		}
	usimple_unlock(&ahp->ah_lock);
	if (ap->an_hasswap) panic("a_swap_hash_lookup: no swap found");
	else return (struct vm_anon *) 0;
}


a_swap_free(register struct vm_anon *ap,
		boolean_t credit_space)
{
	register struct vm_swap *sp;
	vm_offset_t soffset;

	sp = a_aptosp(ap,&soffset);
	swap_vslock(sp);
	ap->an_next = sp->vs_anfree;
	sp->vs_anfree = ap;
	sp->vs_freespace++;
	sp->vs_anfree = ap;
	swap_vsunlock(sp);
	if (credit_space == TRUE) swap_space_free(1);
}

struct vm_swap *
a_aptosp(register struct vm_anon *ap,
	vm_offset_t *soffset)
{
	register struct vm_swap *sp, *lsp;

	if (ap->an_type == ANT_LAZY) {
		if (!ap->an_hasswap) return (struct vm_swap *) 0;
		ap = a_swap_hash_lookup(ap);
	}

	swap_read_lock();
	if ((sp = vm_swap_head) == (struct vm_swap *) 0) goto bad;
	lsp = sp->vs_bl;
	do {
		if (ap >= sp->vs_anbase && 
			ap < (sp->vs_anbase + sp->vs_swapsize)) {
			swap_read_unlock();
			*soffset = ptoa(ap - sp->vs_anbase);
			return sp;
		}
		if (sp != lsp) sp = sp->vs_fl;
		else break;
	} while (1);
bad:
	panic("a_aptosp: ap not found in swap cell");
}

struct vm_anon *
a_anon_allocate()
{
	register struct vm_anon *ap;

	if (!vm_swap_eager) {
		ZALLOC(anon_lazy_zone, ap, struct vm_anon *);
		if (ap == (struct vm_anon *) 0) 
			panic("a_anon_allocate: ZALLOC failure");
		ap->an_refcnt = 1;
		ap->an_cowfaults = 0;
		ap->an_hasswap = 0;
		ap->an_type = ANT_LAZY;
		ap->an_page = VM_PAGE_NULL;
		return ap;
	}
	else {
		ap = a_swap_alloc(TRUE, TRUE, ANT_SWAP, (struct vm_swap *) 0);
		if (!ap) panic("a_anon_allocate: swap not found");
		else return ap;
	}
}


/*
 * The entire anon array must be protected.
 */

a_grow(struct vm_anon_object *aop,
	vm_size_t increase,
	vm_size_t delta,
	as_grow_t direction,
	boolean_t canwait)
{
	register int newsize, oldsize;
	register struct vm_anon **napp;
	register struct vpage *nvp;

	if (aop->ao_flags & AF_NOGROW) return KERN_NO_SPACE;

	newsize = atop(increase);
	if (a_reserve(aop, newsize) == FALSE) return KERN_RESOURCE_SHORTAGE;
	oldsize = atop(aop->ao_size);
	if (aop->ao_anon) {
		
		napp = (struct vm_anon **) 
			h_kmem_zalloc_memory((newsize + oldsize) * 
				sizeof (aop->ao_anon), canwait);
		if (napp == (struct vm_anon **) 0) goto failed;
		if (direction == AS_GROWUP) 
			bcopy(aop->ao_anon + delta, napp, 
				oldsize * sizeof (aop->ao_anon));
		else 
			bcopy(aop->ao_anon + delta, napp + newsize,
				oldsize * sizeof (aop->ao_anon));
		h_kmem_free_memory(aop->ao_anon + delta, 
			oldsize * sizeof (aop->ao_anon), canwait);
		aop->ao_anon = napp;
	}
	else {
		aop->ao_anon = (struct vm_anon **)
			h_kmem_zalloc_memory(newsize * sizeof (aop->ao_anon), 
				canwait);
		if (aop->ao_anon == (struct vm_anon **) 0) goto failed;
	}
	aop->ao_size += increase;
	return KERN_SUCCESS;
failed:

	/*
	 * Release the reserved swap
	 */

	a_free(aop, newsize);
	return KERN_RESOURCE_SHORTAGE;
}


/*
 * Allocate an anon object and reserve all swap space.
 * For now nothing is done with the map allocating
 * the space.
 */

a_object_allocate(vm_map_t map, 
	struct vm_anon_object **object, 
	vm_size_t size)
{
	vm_object_allocate(OT_ANON, size, FALSE, (vm_object_t *) object);
	if (a_reserve(*object, atop(size)) == FALSE) {
		vm_object_deallocate((vm_object_t) (*object));
		return KERN_RESOURCE_SHORTAGE;
	}
	return KERN_SUCCESS;
}

/*
 * Free reserved anonymous memory
 */

void
a_free(register struct vm_anon_object *aop,
	vm_size_t size)
{
	if (vm_swap_eager) {
		swap_space_free(size);
		aop->ao_ranon -= size;
	}
}

/*
 * Reserve anonymous memory
 */

boolean_t
a_reserve(register struct vm_anon_object *aop,
	register vm_size_t size)
{
	boolean_t ret;

	if (vm_swap_eager) {
		ret = swap_space_alloc(size);
		if (ret == TRUE) { 
			aop->ao_ranon += size;
			a_swap_warn();
		}
		else {
			uprintf(a_swap_fail_message);
			printf(a_swap_fail_message);
		}
		return ret;
	}
	else return TRUE;
}

/*
 * anon kluster lock try
 */

boolean_t
anon_lk_lock_try(register alock_t lp)
{
	if ((simple_lock_try(&lp->akl_slock) == FALSE) ||
		lp->akl_lock == 1) return FALSE;
	lp->akl_lock = 1;
	simple_unlock(&lp->akl_slock);
	return TRUE;
}

#if	VM_ANON_TRACK


extern long get_caller_ra();

/*
 * Test for freeing non-dirty page
 */

an_track_written(register struct vm_anon *ap)
{
	register struct an_track *at;

	at = APTOAT(ap);
	if (!at->at_written) panic("an_track_written: ap not written");
}

an_track_write(register struct vm_anon *ap)
{
	register struct an_track *at;

	at = APTOAT(ap);
	at->at_written = 1;
	at->at_writes++;
	at->at_writepc = get_caller_ra();
}

an_track_read(register struct vm_anon *ap)
{
	register struct an_track *at;

	at = APTOAT(ap);
	at->at_readpc = get_caller_ra();
	at->at_reads++;
	if (!at->at_written) panic("an_track_read:");
}

an_track_free(register struct vm_anon *ap)
{
	register struct an_track *at;

	at = APTOAT(ap);
	bzero(at, sizeof (*at));
}

#endif	/* VM_ANON_TRACK */
