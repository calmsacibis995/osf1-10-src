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

#include <kern/thread.h>
#include <sys/types.h>
#include <machine/pmap.h>
#include <machine/machparam.h>
#include <hal/entrypt.h>

/*
 * These kdebug state variables must sync with those in kdebug/include/kdebug.h
 */
#define	KDEBUG_DISABLED	0x0
#define	KDEBUG_ENABLED	0x1
#define	KDEBUG_VTOP	0x2

#define kdebug_printf if (rb->rb_kdebug_printf) rb->rb_kdebug_printf

static struct restart_blk *rb = (struct restart_blk *) RESTART_ADDR;

pt_entry_t *
kdebug_get_pte(map, va)
    pmap_t map;
    vm_offset_t va;
{
    pt_entry_t *pte;

    kdebug_printf("kdebug_get_pte: va is 0x%x\n", va);

    /*
     * ensure that page tables are initialized
     */
    if (!(rb->rb_kdebug_state & KDEBUG_VTOP)) {
    	kdebug_printf("kdebug_get_pte: page tables not initialized\n");
	return(0);
    }

    /*
     * get the level 1 pte
     */
    pte = pmap_l1pte(map, va);
    kdebug_printf("kdebug_get_pte: l1 pte is 0x%x\n", pte);
    if (!pte->pg_v) {
    	kdebug_printf("kdebug_get_pte: got invalid l1 pte\n");
	return(0);
    }

    /*
     * get the level 2 pte
     */
    pte = pmap_l2pte(pte, va);
    kdebug_printf("kdebug_get_pte: l2 pte is 0x%x\n", pte);
    if (!pte->pg_v) {
    	kdebug_printf("kdebug_get_pte: got invalid l2 pte\n");
	return (0);
    }

    return(pte);
}

vm_offset_t
kdebug_vtop(va)
    vm_offset_t va;
{
    pt_entry_t *pte;
    vm_offset_t pa;
    extern char pcb_initial_space[];

    kdebug_printf("kdebug_vtop: mapping 0x%x\n", va);

    /*
     * we don't need to go through the mapping tables for K0 and K1
     */
    if (IS_KSEG0(va)) {
	return(va);
    }
    if (IS_KSEG1(va)) {
	return(va);
    }

    /* 
     * check to see if the address falls in the wired kernel stack
     */
    if ((va < VM_MIN_KERNEL_ADDRESS) &&
	(va >= VM_MIN_KERNEL_ADDRESS - KERNEL_STACK_SIZE)) {

    	kdebug_printf("kdebug_vtop: in kernel stack\n");

	/*
	 * calculate the address offset, and determine where the
	 * kernel stack is
	 */
	va -= (unsigned) (VM_MIN_KERNEL_ADDRESS - KERNEL_STACK_SIZE);

        if (u.uthread) {
	    va += (unsigned) current_thread()->kernel_stack;
	} else {
	    va += (unsigned) (((unsigned) pcb_initial_space + NBPG - 1)
		      & 0xfffff000);
	}

    	kdebug_printf("kdebug_vtop: translated to 0x%x\n", va);
	return(kdebug_vtop(va));
    }

    pte = 0;
    if (va >= KPTEADDR) {
    	kdebug_printf("kdebug_vtop: address > KPTEADDR\n");
	pte = pmap_root_pte(va);
    } else {
	if (IS_KSEG2(va)) {
    	    kdebug_printf("kdebug_vtop: in KSEG2\n");
	    pte = kdebug_get_pte(kernel_pmap, va);
	} else {
	    if (IS_KUSEG(va)) {
    	        kdebug_printf("kdebug_vtop: in KUSEG\n");
	        pte = kdebug_get_pte(active_pmap, va);
	    }
	}
    }

    kdebug_printf("kdebug_vtop: found pte 0x%x\n", pte);

    if (pte) {
	if (pte->pg_v) {
	    pa = PTETOPHYS(pte) + (va & VA_OFFMASK);
    	    kdebug_printf("kdebug_vtop: mapped to pa 0x%x\n", pa);
	    pa = PHYS_TO_K0(pa);
    	    kdebug_printf("kdebug_vtop: pa mapped to phys 0x%x\n", pa);
	    return(pa);
	} else {
    	    kdebug_printf("kdebug_vtop: invalid pte\n");
	}
    }

    return(-1);
}

/*
 * called by the kernel's serial line interrupt handler to force the kernel
 * to respond to dbx
 */

kdebug_fakebreak(epc)
    unsigned *epc;
{
    rb->rb_kdebug_fakebreak = 1;
    kdebug_printf("entering kdebug_fakebreak\n");
    gimmeabreak();
    kdebug_printf("leaving kdebug_fakebreak\n");
}

/*
 * Check for the -kdebug flag to the boot command.  If present,
 * flag that kdebug is active.  Perform any needed kdebug initialization
 * that can be done at this time.
 */

kdebug_init(argc, argv, environ, vector)
    int argc;
    char *argv[];
    char *environ[];
    char *vector[];
{
    int i;

    /*
     * see if kdebug is requested
     */
    rb->rb_kdebug_state = KDEBUG_DISABLED;
    for (i = 1; i < argc; i++) {
	if (!strcmp("-kdebug", argv[i])) {
    	    rb->rb_kdebug_state = KDEBUG_ENABLED;
    	    rb->rb_vtop = kdebug_vtop;
        }
    }
}

kdebug_vtop_init()
{
    /*
     * flag that the map tables are usable
     */
    if (rb->rb_kdebug_state & KDEBUG_ENABLED)
        rb->rb_kdebug_state |= KDEBUG_VTOP;
}

kdebug_state()
{
    return(rb->rb_kdebug_state);
}
