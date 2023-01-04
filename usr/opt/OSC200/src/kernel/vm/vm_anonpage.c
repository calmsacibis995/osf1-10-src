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
static char *rcsid = "@(#)$RCSfile: vm_anonpage.c,v $ $Revision: 1.1.3.4 $ (DEC) $Date: 1992/12/11 15:38:44 $";
#endif
#include <kern/assert.h>
#include <mach/machine/vm_types.h>
#include <vm/vm_page.h>
#include <vm/vm_object.h>
#include <vm/vm_anon.h>
#include <vm/vm_swap.h>
#include <vm/vm_anonpage.h>


vm_page_t
vm_anon_kpage_alloc(struct vm_swap_object *sop,
	vm_offset_t soffset)
{
	vm_page_t page;
	register vm_page_t pp;

	vm_pg_alloc(&page);
	if ((pp = page) == VM_PAGE_NULL) return pp;
	vm_object_lock(sop);
	pp->pg_offset = soffset;
	pp->pg_object = (vm_object_t) sop;
	vm_page_insert_bucket(pp, sop, soffset);
	vm_object_unlock(sop);
	return pp;
}
	
/*
 * This currently supports
 * umap and cmap style anon management.
 */

vm_page_t
vm_anon_page_alloc(register struct vm_anon_object *aop,
	vm_offset_t offset,
	register alock_t lp,
	register struct vm_anon *ap,
	boolean_t aplocked,
	boolean_t wait)
{
	struct vm_swap_object *sop;
	vm_offset_t soffset;
	vm_page_t page;
	register vm_page_t pp;
	register struct vm_swap *sp;

	while (vm_pg_alloc(&page), ((pp = page) == VM_PAGE_NULL)) {
		if (wait == FALSE) return VM_PAGE_NULL;
		if (aplocked == TRUE) a_unlock(ap);
		vm_wait();
		if (aplocked == TRUE) {
			a_lock(ap);
			if (ap->an_page != VM_PAGE_NULL) return VM_PAGE_NULL;
		}
	}

	sp = a_aptosp(ap, &soffset);
	ap->an_page = pp;
	pp->pg_hold = 1;

	if (sp) {
		sop = sp->vs_object;
		vm_object_lock(sop);
		pp->pg_offset = soffset;
		pp->pg_object = (vm_object_t) sop;
		vm_page_insert_bucket(pp, sop, soffset);
		vm_object_unlock(sop);
	}
	else {
		pp->pg_offset = (vm_offset_t) (ap - vm_swap_lazy->vs_anbase);
		pp->pg_object = (vm_object_t) vm_swap_lazy_object;
	}

	lk_slock(lp);
	vm_page_insert_object(&lp->akl_pagelist, pp);
	lp->akl_rpages++;
	lk_sunlock(lp);
	pp->pg_owner = aop;
	pp->pg_roffset = (long) offset - aop->ao_rbase;
	return pp;
}

/*
 * This currently supports
 * umap and cmap style anon management.
 */

vm_page_t
vm_anon_zeroed_page_alloc(register struct vm_anon_object *aop,
	vm_offset_t offset,
	register alock_t lp,
	register struct vm_anon *ap,
	boolean_t aplocked,
	boolean_t wait)
{
	struct vm_swap_object *sop;
	vm_offset_t soffset;
	vm_page_t page;
	register vm_page_t pp;
	register struct vm_swap *sp;

	while (vm_zeroed_pg_alloc(&page), ((pp = page) == VM_PAGE_NULL)) {
		if (wait == FALSE) return VM_PAGE_NULL;
		if (aplocked == TRUE) a_unlock(ap);
		vm_wait();
		if (aplocked == TRUE) {
			a_lock(ap);
			if (ap->an_page != VM_PAGE_NULL) return VM_PAGE_NULL;
		}
	}

	sp = a_aptosp(ap, &soffset);
	ap->an_page = pp;
	pp->pg_hold = 1;

	if (sp) {
		sop = sp->vs_object;
		vm_object_lock(sop);
		pp->pg_offset = soffset;
		pp->pg_object = (vm_object_t) sop;
		vm_page_insert_bucket(pp, sop, soffset);
		vm_object_unlock(sop);
	}
	else {
		pp->pg_offset = (vm_offset_t) (ap - vm_swap_lazy->vs_anbase);
		pp->pg_object = (vm_object_t) vm_swap_lazy_object;
	}

	lk_slock(lp);
	vm_page_insert_object(&lp->akl_pagelist, pp);
	lp->akl_rpages++;
	lk_sunlock(lp);
	pp->pg_owner = aop;
	pp->pg_roffset = (long) offset - aop->ao_rbase;
	return pp;
}

/*
 * If aop is passed then the kluster lock is
 * taken.
 */

vm_anon_page_free(register struct vm_anon_object *aop, 
	register vm_offset_t offset, 
	struct vm_anon *ap)
{
	register struct vm_swap_object *sop;
	register vm_page_t pp;
	register struct vm_swap *sp;
	vm_offset_t soffset;

	pp = ap->an_page;
	if (aop && pp->pg_owner == aop) {
		register alock_t lp;

		/*
		 * It is assumed lp is protected by the caller.
		 */

		lp = aop->ao_klock + anon_kl(anon_kltrunc(offset));
		vm_page_remove_object(&lp->akl_pagelist, pp);
		lp->akl_rpages--;
	}

	sp = a_aptosp(ap, &soffset);
	if (sp) {
		sop = sp->vs_object;
		vm_object_lock(sop);
		ap->an_page = VM_PAGE_NULL;
		vm_page_remove_bucket(pp);
		vm_object_unlock(sop);
	}
	else ap->an_page = VM_PAGE_NULL;
	vm_pg_free(pp);
}
