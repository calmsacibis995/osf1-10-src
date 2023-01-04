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
static char *rcsid = "@(#)$RCSfile: u_mape_shm.c,v $ $Revision: 1.1.21.5 $ (DEC) $Date: 1993/11/22 21:47:09 $";
#endif

/*
 * Supports system V shared memory regions for U maps
 */

#include <vm/vm_map.h>
#include <mach/kern_return.h>
#include <vm/vm_object.h>
#include <vm/vm_anon.h>
#include <vm/vm_perf.h>
#include <sys/shm.h>
#include <sys/errno.h>
#include <sys/user.h>
#include <sys/proc.h>
#include <vm/heap_kmem.h>
#include <mach/vm_param.h>
#include <kern/thread.h>

extern int
	u_anon_fault(),
        u_shm_dup(),
        u_shm_unmap(),
	u_anon_dup(),
        u_anon_unmap(),
        u_anon_msync(),
        u_anon_lockop(),
        u_anon_swap(),
        u_anon_core(),
        u_anon_control(),
        u_anon_protect(),
        u_anon_check_protect(),
        u_anon_kluster(),
        u_anon_copy(),
        u_anon_grow(),
	u_shm_oop_deallocate(),
	u_shm_allocate(),
	u_anon_oop_bad(),
	u_anon_oop_pageout(),
	u_anon_oop_swapout(),
	u_anon_oop_create(),
	u_anon_oop_lock_try(),
	u_anon_oop_unlock(),
	u_anon_oop_reference(),
	u_anon_oop_pagesteal();

struct vm_map_entry_ops u_mape_shm_ops = {
        &u_anon_fault,          /* fault */
        &u_shm_dup,             /* dup */
        &u_shm_unmap,           /* unmap */
        &u_anon_msync,          /* msync */
        &u_anon_lockop,         /* lockop */
        &u_anon_swap,           /* swap */
        &u_anon_core,           /* corefile */
        &u_anon_control,        /* control */
        &u_anon_protect,        /* protect */
        &u_anon_check_protect,  /* check protection */
        &u_anon_kluster,        /* kluster */
        &u_anon_copy,           /* copy */
        &u_anon_grow,           /* grow */
};

extern  struct timeval	time;
extern thread_t		reaper_thread_ptr;
extern lock_data_t	shm_attch_lock;		/* protects attach array */

/*
 * Allocate the shared memory.
 */

u_shm_allocate(vm_size_t size, vm_object_t *obj)
{
        struct vm_shm_object *sop;
        kern_return_t ret;

        size = round_page(size);
        if ((ret = vm_object_allocate(OT_SHM, size,
                FALSE, (vm_object_t *) &sop)) != KERN_SUCCESS) return ret;

        sop->so_flag |= AF_SHARED;

        if (a_reserve(sop, atop(size)) == FALSE) {
                vm_object_deallocate(sop);
                return KERN_RESOURCE_SHORTAGE;
        }
        *obj = (vm_object_t) sop;
        return 0;
}

/*
 * Deallocate the shared memory.
 */

u_shm_oop_deallocate(register struct vm_shm_object *sop)
{
        register struct vm_anon *app;
        register int npages;

        vm_object_lock(sop);

	if (sop->so_sp && sop->so_refcnt <= sop->so_sp->s.shm_nattch) {
                vm_object_unlock(sop);
                return 0;
        }

        sop->so_refcnt--;
	sop->so_rescnt--;
        sop->so_crefcnt--;

        if (sop->so_refcnt) {
                vm_object_unlock(sop);
                return 0;
        }

	if (sop->so_sp)
        	shm_free_entry(sop->so_sp);

        if (sop->so_oflags & OB_SWAPON) vm_object_swapoff(sop);
        vm_object_unlock(sop);


        /*
         * If any anon array and anon allocated then free each one
         * The anon is freed before the reservation is updated.
         * This will eliminate inconsistencies in what swap
         * space is actually available.
         */

        if (sop->so_anon) {
                register int npages;

                npages = atop(sop->so_size);
                u_anon_free(sop, (vm_offset_t) 0, npages);
                h_kmem_free(sop->so_anon, npages * sizeof (sop->so_anon));
        }

        if (sop->so_klock)
                h_kmem_free(sop->so_klock,
                        sop->so_nklock * sizeof (struct anon_klock));

        /*
         * If any anon reserved then free it.
         */

        if (sop->so_ranon) a_free(sop, sop->so_ranon);

        /*
         * If there is a backing object we must
         * remove our reference by deallocating it.
         */

        if (sop->so_bobject) vm_object_deallocate(sop->so_bobject);

        vm_object_free(sop);
}


/*
 * Dup the address space
 */

u_shm_dup(register vm_map_entry_t ep,
        vm_offset_t addr,
        register vm_size_t size,
        register vm_map_entry_t newep,
        vm_copy_t copy)
{
	int ret;

	if ((ret = u_anon_dup(ep, addr, size, newep, copy)) == KERN_SUCCESS) {
		if (copy==VM_COPYU) {
			((vm_shm_object_t)(ep->vme_object))->so_sp->s.shm_nattch++;
			u_shm_attach(u.u_procp->p_cptr->p_pid, ep->vme_object, newep->vme_map);
		}
	}
	return (ret);
}

/*
 * Unmap all of an address space.
 */

u_shm_unmap(register vm_map_entry_t ep,
        vm_offset_t addr,
        vm_size_t len)
{

	if((ep->vme_start != addr) || (ep->vme_end != addr+len))
		return(EINVAL);
	((vm_shm_object_t)(ep->vme_object))->so_sp->s.shm_nattch--;
	((vm_shm_object_t)(ep->vme_object))->so_sp->s.shm_dtime=time.tv_sec;
	((vm_shm_object_t)(ep->vme_object))->so_sp->s.shm_lpid=
				u_shm_dettach(ep->vme_object, ep->vme_map);
	return u_anon_unmap(ep, addr, len);
}

/*
 * The attach array functions
 */

u_shm_attach(pid_t pid, vm_shm_object_t sop, vm_map_t map)
{
        register long	count, oldcount;
	register u_int	oldsize;
        vm_shm_attach_t attach;
	vm_offset_t	new_at;

	lock_write(&shm_attch_lock);
        if (sop->so_attach) {
                count = oldcount = *(long *)(sop->so_attach);
                for (attach=(vm_shm_attach_t)(sop->so_attach + sizeof(long));
                                                 count; count--,attach++) {
                        if (map == attach->at_map) {
                                attach->at_count++;
				lock_done(&shm_attch_lock);
                                return;
			}
                }
		oldsize = sizeof(long) +
			oldcount * sizeof(struct vm_shm_attach);
                new_at = (vm_offset_t)h_kmem_alloc(oldsize +
						sizeof(struct vm_shm_attach));
                bcopy(sop->so_attach, new_at, oldsize);
                h_kmem_free(sop->so_attach, oldsize);
		sop->so_attach = new_at;
                (*(long *)(sop->so_attach))++;
                attach=(vm_shm_attach_t)(sop->so_attach + oldsize);
        } else {
                sop->so_attach = (vm_offset_t)(h_kmem_alloc(sizeof(long) +
			sizeof(struct vm_shm_attach)));
		*(long *)(sop->so_attach)=1;
                attach=(vm_shm_attach_t)(sop->so_attach + sizeof(long));
        }
        attach->at_count = 1;
        attach->at_map = map;
        attach->at_pid = pid;
	lock_done(&shm_attch_lock);
        return;
}

pid_t
u_shm_dettach(vm_shm_object_t sop, vm_map_t map)
{
	register long	count, oldcount;
	vm_shm_attach_t	attach;
	vm_offset_t	new_at;
	pid_t		pid;

	if (current_thread() != reaper_thread_ptr) u.u_shmsegs--;

	lock_write(&shm_attch_lock);
	if (sop->so_attach) {
		count = oldcount = *(long *)sop->so_attach;
		for (attach=(vm_shm_attach_t)(sop->so_attach + sizeof(long));
						count; count--,attach++) {
			if (map == attach->at_map) {
				pid=attach->at_pid;
				if (--attach->at_count) {
					lock_done(&shm_attch_lock);
					return (pid);
				}
				if (oldcount == 1) {
					h_kmem_free(sop->so_attach, (sizeof(long) + sizeof(struct vm_shm_attach)));
					sop->so_attach = (vm_offset_t)0;
					lock_done(&shm_attch_lock);
					return (pid);
				}
				new_at = (vm_offset_t)h_kmem_alloc(sizeof(long) +
							(oldcount-1)*sizeof(struct vm_shm_attach));
				*(long *)new_at = oldcount-1;
				if(count != oldcount)
					bcopy((sop->so_attach+sizeof(long)), (new_at + sizeof(long)),
							(oldcount-count)*sizeof(struct vm_shm_attach));
				if(count != 1)
					bcopy((sop->so_attach+sizeof(long)+((oldcount-count+1)*sizeof(struct vm_shm_attach))),
					      (new_at+sizeof(long)+((oldcount-count)*sizeof(struct vm_shm_attach))),
					      ((count-1)*sizeof(struct vm_shm_attach)));
				h_kmem_free(sop->so_attach, (sizeof(long) + oldcount*sizeof(struct vm_shm_attach)));
				sop->so_attach = new_at;
				lock_done(&shm_attch_lock);
				return (pid);
			}
		}
	}
	panic("u_shm_dettach: detaching non-existent segment");
	lock_done(&shm_attch_lock);
}


struct vm_object_ops u_shm_oop = {
        &u_anon_oop_lock_try,           /* lock try */
        &u_anon_oop_unlock,             /* unlock */
        &u_anon_oop_reference,          /* reference */
        &u_shm_oop_deallocate,          /* deallcoate */
        &u_anon_oop_bad,                /* pagein */
        &u_anon_oop_pageout,            /* pageout */
        &u_anon_oop_swapout,            /* swapout */
        &u_anon_oop_bad,                /* control */
        &u_anon_oop_bad,                /* page control */
	&u_anon_oop_pagesteal,		/* page steal */
};


struct vm_object_config shm_object_conf = {
        &u_anon_oop_create,
        (int (*)()) 0,
        sizeof (struct vm_shm_object),
        &u_shm_oop,
        &u_mape_shm_ops,
};

