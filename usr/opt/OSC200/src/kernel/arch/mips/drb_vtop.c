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
static char *rcsid = "@(#)$RCSfile: drb_vtop.c,v $ $Revision: 1.1.3.2 $ (DEC) $Date: 1992/03/18 15:03:05 $";
#endif
#include <mach/boolean.h>
#include <mach/vm_attributes.h>
#include <mach/vm_param.h>
#include <kern/macro_help.h>
#include <kern/lock.h>
#include <kern/thread.h>
#include <kern/sched_prim.h>
#include <vm/vm_kern.h>
#include <vm/vm_page.h>
#include <sys/types.h>
#include <machine/cpu.h>
#include <hal/entrypt.h>

drb_vtop_init()
{
	extern unsigned long drb_vtop();
	register unsigned long *_drb_vtop_addr = 
		(unsigned long *) (RB_BPADDR+4);

	*_drb_vtop_addr = (unsigned long) &drb_vtop;
}

unsigned long 
drb_vtop(unsigned long v_addr)
{
	pt_entry_t *pte_p;
	union {
		unsigned uint;
		pt_entry_t pte;
	} pte_u;
	pt_entry_t *drb_vchktopte(), *drb_get_rootpte();
	register vm_offset_t pa;


	if (IS_KSEG0(v_addr)) return v_addr;
	else if (IS_KSEG1(v_addr)) return v_addr;

	pte_p = 0;

	if (v_addr >= KPTEADDR) return -1;
	else if (v_addr >= VM_MIN_KERNEL_ADDRESS) {
		pte_p = drb_get_rootpte(v_addr);
	}
	else if (IS_KUSEG(v_addr)) 
		pte_p = drb_vchktopte(v_addr);
	else return -1;


	if (pte_p) {
		pte_u.uint = *(unsigned *)pte_p;
		if (!pte_u.pte.pg_v) return -1;
		else {
			pa = (unsigned long) mips_ptob(pte_u.pte.pg_pfnum) | 
			(v_addr & PGOFSET);
			pa = PHYS_TO_K0(pa);
			return pa;
		}
	} else return -1;
}

#define mipspte_index(VA) ((mips_btop((VA)) & ((MIPS_PGBYTES >> 2) - 1)) << 2)
pt_entry_t *
drb_get_rootpte(vm_offset_t kva)
{
	pt_entry_t *kpte;
	union {
		unsigned uint;
		pt_entry_t pte;
	} pte_u;
	vm_offset_t pa, kpteaddr;

	kpteaddr = (vm_offset_t) pmap_pte(kernel_pmap, kva);
	kpte = pmap_root_pte(kpteaddr);
	pte_u.uint = * (unsigned *) kpte;
	if (!pte_u.pte.pg_v) return (pt_entry_t *) 0;
	pa = PHYS_TO_K0(mips_ptob(pte_u.pte.pg_pfnum) + mipspte_index(kva));
	return (pt_entry_t *) pa;
}

pt_entry_t *
drb_vchktopte(register unsigned long va)
{
	register vm_offset_t v, kva;
	pt_entry_t *pte;
	union {
		unsigned uint;
		pt_entry_t pte;
	} pte_u;
	vm_offset_t pa;

	if (va < VM_MIN_ADDRESS) return (pt_entry_t *) 0;
	v = mips_btop(va);
	pte = drb_get_rootpte(active_pmap->ptebase + (v << 2));
	if (pte == (pt_entry_t *) 0) return pte;
	else pte_u.uint = *(unsigned *) pte;
	if (!pte_u.pte.pg_v) return (pt_entry_t *) 0;
	pa = PHYS_TO_K0(mips_ptob(pte_u.pte.pg_pfnum) + mipspte_index(va));
	return (pt_entry_t *) pa;
}
