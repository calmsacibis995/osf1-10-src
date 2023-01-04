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
static char *rcsid = "@(#)$RCSfile: kern_memcntl.c,v $ $Revision: 1.1.2.4 $ (DEC) $Date: 1993/09/22 13:22:33 $";
#endif
/*
 *  Module Name:
 *	kern_memcntl.c
 *  Description:
 *	implements the SVR4 memcntl() system call.  lock/unlock
 *	and sync and invalidate pages of primary memory.
 *  Origin:
 *	memcntl() system call was designed and developed
 *	based on AT&T SVID Third Edition Volume III and OSF/1.
 *  Algorithm:
 *	lock the map entry to avoid races while scanning vm_map_entry.
 *	Find out the range of address space where a locking, unlocking,
 *	sync'ing or invalidation is to be done. If MC_LOCKAS or MC_UNLOCKAS
 *	is specified range is the entire address space. 
 *	Get the vm_map_entry for the starting address. Check if the 
 *	attributes specified in the 'attr' argument matches with that of
 *	vm_map_entry. If a match is found one of the above operation is
 *	executed.   A match may	fail if user specified an illegal address,
 *	or tried to do MC_SYNC on an address space not backed by permanent
 *	storage like anonymous memory, system V shared memory or simply
 *	because the attributes do match with vm_map_entry. If no match
 *	is found the vm_map_entry is skipped. This is repeated until the
 *	end of the address range.
 *	If a failure occurs during a lock/unlock operation, all the 
 *	operations done is completely undone. Also the um_lock_future flag
 *	is restored with it's original value. Undoing sync is not attepted.
 * 
 */

#include <sys/param.h>
#include <mach/vm_param.h>
#include <sys/errno.h>
#include <sys/user.h>
#include <sys/proc.h>
#include <sys/vnode.h>
#include <sys/vm.h>
#include <vm/vm_map.h>
#include <vm/vm_umap.h>
#include <vm/u_mape_seg.h>
#include <vm/vm_anon.h>
#include <vm/vm_object.h>
#include <vm/vm_page.h>
#include <sys/vmmac.h>
#include <sys/mman.h>
#include <svr4/mman.h>

#if SEC_BASE
#include <sys/security.h>
#include <sys/audit.h>
#endif


memcntl(p, args, retval)
	struct proc *p;
	void *args;
	long *retval;
	
{
	struct args {
		long 	addr;
		long	len;
		long	cmd;
		long	arg;
		long 	attr;
		long 	mask;
	} *uap = (struct args *)args;

	register caddr_t addr;
	register size_t len;
	register vm_size_t size;
	register vm_map_entry_t entry;
	register struct u_map_private *up;
	register vm_offset_t first_addr, start, end;
	struct pg_lk_stat *lk_stat_head, *lsp, *plsp;
	int attr, cmd, arg, mask, lockop;
	long sync_opt;
	vm_map_entry_t first_entry, last_entry;
	boolean_t allspace, lock_future, lock_current;
	boolean_t orig_lockfuture;
	kern_return_t ret;
	vm_seg_t segp;
	vm_map_t map;
	
	/* 
	 * System calls on alpah gets user arguments as long.
	 * Convert them to the type that of the SVID as below
	 * addr ---> caddr_t
	 * len  ---> size_t
	 * cmd, arg, attr, mask ---> int
	 */
	addr = (caddr_t) uap->addr;
	len = (size_t) uap->len;
	cmd = (int) uap->cmd;
	arg = (int) uap->arg;
	attr = (int) uap->attr;
	mask = (int) uap->mask;

	ret = KERN_SUCCESS;
	lk_stat_head = (struct pg_lk_stat *)0;
	lock_current = lock_future = allspace = (boolean_t)0;
	map = current_task()->map;
	up = (struct u_map_private *) (map->vm_private);
	orig_lockfuture = up->um_lock_future; 
	if ((cmd==MC_LOCK) || (cmd==MC_LOCKAS))  lockop = MC_LOCK;
	else if ((cmd==MC_UNLOCK) || (cmd==MC_UNLOCKAS))  lockop = MC_UNLOCK;
	else lockop = 0;

	/* make sure it is a valid command */
	if (cmd != MC_LOCK && cmd != MC_LOCKAS && cmd != MC_SYNC &&
				cmd != MC_UNLOCK && cmd != MC_UNLOCKAS)
		return(EINVAL);
        /*
         * Must be privileged to lock/unlock.
         */

	if (cmd != MC_SYNC) {
#if SEC_BASE
        	if (!privileged(SEC_LOCK, EPERM)) 
                	return (EPERM);
#else
        	if (ret = suser(u.u_cred, &u.u_acflag))
                	return (ret);
#endif
	}

	/* SVID requires mask to be zero always 		*/
	if (mask != 0)
		return(EINVAL);
	
	if (((vm_offset_t)addr > VM_MAX_ADDRESS) || (len > VM_MAX_ADDRESS) ||
			  	(((vm_offset_t)addr + len) > VM_MAX_ADDRESS))
		return(ENOMEM);

        if (!page_aligned(addr))
                return(EINVAL);

	/* A page cannot be private and shared at a time 	*/
	if ((attr & (PRIVATE|SHARED)) == (PRIVATE|SHARED))
		return(EINVAL);

	/*
	 * Gain the write lock and keep it until we're finished.
	 */

	vm_map_lock(map);
        vm_map_lock_set_recursive(map);

	/*
	 *  Find out the address range we're going to operate on
	 *  while checking possible user errors.
	 */

	switch (cmd) {
	case MC_SYNC:
		switch(arg) {
		case MS_SYNC:
		case MS_ASYNC:
		case MS_INVALIDATE:
		case (MS_INVALIDATE|MS_SYNC):
		case (MS_INVALIDATE|MS_ASYNC):
			break;
		default:
			ret = EINVAL;
			goto unlock_out;
		}
		sync_opt = arg;

		/* FALLS THROUGH */
	case MC_LOCK:
	case MC_UNLOCK:
		if (attr & ~(SHARED|PRIVATE|PROT_READ|PROT_WRITE|PROT_EXEC)) {
			ret = EINVAL;
			goto unlock_out;
		}
		first_addr = start = trunc_page(addr);
		end = round_page(addr + len);
		if (!vm_map_lookup_entry(map, start, &first_entry)) {
			ret = ENOMEM;
			goto unlock_out;
		}
		else entry = first_entry;
		break;

	case MC_LOCKAS:
		if ((arg & ~(MCL_CURRENT|MCL_FUTURE)) ||
		    !(arg & (MCL_CURRENT|MCL_FUTURE))) {
			ret = EINVAL;
			goto unlock_out;
		}
		lock_current = ((arg & MCL_CURRENT) != 0);
		lock_future = ((arg & MCL_FUTURE) != 0);
		if (lock_future) {
			up->um_lock_future = 1;
			if(!lock_current) {
				ret = KERN_SUCCESS;
				goto unlock_out;
			}
		}

		/* FALLS THROUGH */
	case MC_UNLOCKAS:
		if (addr || len) {
			ret = EINVAL;
			goto unlock_out;
		}
		if ((cmd == MC_UNLOCKAS) && arg) {
			ret = EINVAL;
			goto unlock_out;
		}
		entry = first_entry = vm_map_first_entry(map);
		if (entry == vm_map_to_entry(map)) {
			ret = ENOMEM;
			goto unlock_out;
		}
		first_addr = start = trunc_page(entry->vme_start);
		end = round_page(vm_map_last_entry(map)->vme_end);		
		allspace = 1;
		break;

	default:
		ret = EINVAL;
		goto unlock_out;
	}


	/*
	 * We got the range (start & end) that we can operate on.
	 */

	while (start < end) {
		if (start < entry->vme_start) {
			if (!allspace) {
				ret = ENOMEM;
				goto failed;
			}
		}
		else if (vm_map_to_entry(map) == entry) {
			if (!allspace) {
				ret = ENOMEM;
				goto failed;
			}
			else break;
		}
			
		size = MIN(end, entry->vme_end) - start;

		/*
		 * Wait for faults to complete
		 */
		vm_mape_faultlock(entry);
		
		/*
		 * The segment in an entry can be smaller than entry itself.
		 */
		if (vm_object_type(entry->vme_object) == OT_SEG) {
			segp = entry->vme_seg;
			if (allspace) {
				size = segp->seg_size;
				start = trunc_seg(entry->vme_start) +
				        segp->seg_start;
			}
			if (u_seg_bad_addr(entry, start, size)) {
			/* 
			 * User gave an invalid address range 
			 */
				ret = ENOMEM;
				goto failed;
			}
		}

		/* 
		 * Save the status of each page of the map entry so that
		 * if a lock/unlock operation fails at any point we can 
		 * undo the operation.
		 */

		if (lockop) {
			if ((lsp = save_pgstat(entry, start, size))
			     == (struct pg_lk_stat *)0) {
				ret = ENOMEM;
				goto failed;
			}
			lsp->st_next = (struct pg_lk_stat *)0;

			if (lk_stat_head == (struct pg_lk_stat *)0)
				plsp = lk_stat_head = lsp;
			else {
				plsp->st_next = lsp;
				plsp = lsp;
			}
		}

		/* 
		 * check if user given attributes match for the entry
		 * given the command.
		 */

		switch (attr_match(entry, attr, cmd)) {
		case ESUCCESS:
			break;
		case EACCES:
			/*
			 * cmd is Illegal on this entry, error out if we
			 * were not doing "allspace", else skip this entry
			 */
			if (!allspace) {
				ret = EPERM;
				goto failed;
			}

			/* FALLS THROUGH */
		case ENOENT:
			/* No match, skip this map entry */
			entry = entry->vme_next;
			if (allspace) start = entry->vme_start;
			else start += size;
			continue; 		/* while start < end */
		case ENOSYS:
			/* An invalid operation on entry was specified. */
			if (!allspace) {
				ret = ENOSYS;
				goto failed;
			}
			entry = entry->vme_next;
			if (allspace) start = entry->vme_start;
			else start += size;
			continue; 		/* while start < end */
		default:
			ret = EPERM;
			goto failed;
				
		}

		/*
		 * We got a map_entry that we can operate on, safely try
		 * locking unlocking or sync'ing of the pages.
		 */

		switch (cmd) {
		case MC_LOCK:
		case MC_LOCKAS:
			if ((*entry->vme_lockop)(entry, start, size,
			    VM_WIRE) != KERN_SUCCESS) {
				ret = EAGAIN;
				goto failed;
			} 
			break;

		case MC_UNLOCK:
		case MC_UNLOCKAS:
			if ((*entry->vme_lockop)(entry, start, size,
			VM_UNWIRE) != KERN_SUCCESS) {
				ret = EAGAIN;
				goto failed;
			} 
			if(cmd == MC_UNLOCKAS)
				up->um_lock_future = 0;
			break;

		case MC_SYNC:
			if ((*entry->vme_msync)(entry, start, size,
			    sync_opt) != KERN_SUCCESS) {
				ret = EBUSY;
				goto failed;
			}
		}

		entry = entry->vme_next;
		if (allspace) start = entry->vme_start;
		else start += size;
	}

	/* Free up all the memory used to save status of pages */
	while (lk_stat_head != (struct pg_lk_stat *)0) {
		lsp = lk_stat_head;
		lk_stat_head = lk_stat_head->st_next;
		kfree (lsp, lsp->st_size);
	}

        vm_map_lock_clear_recursive(map);
	vm_map_unlock(map);
	return KERN_SUCCESS;

failed:
	/*
	 * If we were locking undo all the locks taken
	 */

	if (lockop && (start != first_addr)) {
		lsp = lk_stat_head;
		end = start; 
		entry = first_entry;
		start = first_addr;
		up->um_lock_future = orig_lockfuture; 

		while (start < end) {
			if (lsp == (struct pg_lk_stat *)0) break;
			if (entry == vm_map_to_entry(map)) break;
			size = MIN(end, entry->vme_end) - start;
			if (vm_object_type(entry->vme_object) == OT_SEG) {
				segp = entry->vme_seg;
				if (allspace) {
					size = segp->seg_size;
					start = trunc_seg(entry->vme_start) +
				        	segp->seg_start;
				}
			}
			undo_lockop(entry, start, size, lsp, lockop);
			plsp = lsp;
			lsp = (struct pg_lk_stat *)lsp->st_next;
			kfree (plsp, plsp->st_size);
			entry = entry->vme_next;
			if (allspace) start = entry->vme_start;
			else start += size;
		}
	}

unlock_out:
        vm_map_lock_clear_recursive(map);
        vm_map_unlock(map);
	return (ret);
}
	


/*	
 * attr_match(): Check if the specified attributes in 'attr'
 * match for the map entry 'me' to do operation 'cmd'.
 * Return values:
 * 	ESUCCESS:	Attributes matches the map.
 * 	ENOENT:		Attributes don't match the map.
 * 	EACCES:		'cmd is illegal on this map.
 *	ENOSYS		'cmd' cannot be executed.
 */

int
attr_match (me, attr, cmd)
vm_map_entry_t me;
int attr, cmd;
{
	unsigned short ao_flags, p_prot, g_prot;
	int rval, prot_ok;

	rval = ENOENT;
	ao_flags = ((struct vm_anon_object *)(me->vme_object))->ao_flags;

	/*
         * Note:  PROC_TEXT and PROC_DATA are really just attribute
         * combinations as 
         * #define PROC_TEXT  (PROT_EXEC|PROT_READ)
         * #define PROC_DATA  (PROT_WRITE|PROT_READ)
	 * So we don't purticularly check for PROC_TEXT and PROC_DATA.
	 */

	p_prot = attr & (PROT_READ|PROT_WRITE|PROT_EXEC);
	g_prot = attr & (SHARED|PRIVATE);
        prot_ok = ((me->vme_protection & p_prot) == p_prot) ? 1 : 0;

	switch (vm_object_type(me->vme_object)) {
	case OT_ANON:
		if ((!attr) ||
		    ((g_prot == 0) && prot_ok) ||
		    ((g_prot & SHARED) && (ao_flags & AF_SHARED) && prot_ok) ||
		    ((g_prot & PRIVATE) && (ao_flags & AF_PRIVATE)&& prot_ok)) {
			/*
			 * OT_ANON is not backed by permanent storage. SVID
			 * requires data to be written to permanent storage
			 * with MC_SYNC.
			 */
			if (cmd == MC_SYNC) {
				rval = ENOSYS;
				break;
			}
			rval = ESUCCESS;
		}
		break;

	case OT_SHM:
		if ((!attr) ||
		    ((g_prot == 0) && prot_ok) ||
		    ((g_prot & SHARED) && prot_ok)) {
			/*
			 * OT_SHM is not backed by permanent storage.
			 */
			if (cmd == MC_SYNC) {
				rval = ENOSYS;
				break;
			}
			rval = ESUCCESS;
		}
		break;

	case OT_VP:
		/* 
		 * This is a (public) shared object.
		 */
		if ((!attr) ||
		    ((g_prot == 0) && prot_ok) ||
		    ((g_prot & SHARED) && prot_ok))
			rval = ESUCCESS;
		break;

	case OT_SEG:
		/* 
		 * Segments are private although no flag is set so.
		 * Right now map entry has the perms for the segs.
		 */
		if ((!attr) ||
		    ((g_prot == 0) && prot_ok) ||
		    ((g_prot & PRIVATE) && (prot_ok))) {
			/*
			 * MC_SYNC on OT_SEG is a nop in OSF.  So, even the
			 * MS_INVALIDATE won't free the page.
			 */
			if (cmd == MC_SYNC) {
				rval = ENOSYS;
				break;
			}
			rval = ESUCCESS;
		}
		break;

	case OT_NULL:
	case OT_SWAP:
	case OT_VPMAP:
	case OT_DEVMAP:
	case OT_KERNEL:
	case OT_PKERNEL:
	default:	
		rval = EACCES;
		break;

	}

	return (rval);
}

/*
 * save the status of each page in the address space specified
 * by 'va' (virtual addr) and 'len'. 'ep' points to the map entry
 * containing the address space.
 * Returns:  a pointer to structure pg_lk_stat containing status
 * of each page, locked or unlocked.
 */

struct pg_lk_stat *
save_pgstat(ep, va, len)
vm_map_entry_t ep;
caddr_t va;
size_t len;
{
        register vm_offset_t start, end;
        register struct vpage *vp;
	struct pg_lk_stat *lks;
	int bufsiz, i;

	bufsiz = sizeof(lks->st_flag[0]) * btop(len) + sizeof(struct pg_lk_stat);
	lks = (struct pg_lk_stat *) kalloc(bufsiz);
	if (lks == (struct pg_lk_stat *)0)  return(lks);

	lks->st_size = bufsiz;
        vp = (struct vpage *) (ep->vme_private);

	start = (vm_offset_t)va;
	end = (vm_offset_t)va + len;

	/*
	 * If no pages of map are in core, mark 'em as unlocked
	 */
	if (vp == (struct vpage *)0) {
		for (i=0; start < end; i++, start += PAGE_SIZE)
			lks->st_flag[i] = UNLOCKED;
		return(lks);
	}

	for (i=0; start < end; i++, vp++, start += PAGE_SIZE) {
		if (vp->vp_plock)
			lks->st_flag[i] = LOCKED;
		else
			lks->st_flag[i] = UNLOCKED;
	}

	return(lks);
}

/*
 * Undo unlock or lock operation given in 'op'.
 * 'ep' points to the map entry, 'va' and 'len' specifies 
 * the address space where 'op' to be undone. 'lks' contains
 * status (locked/unlocked) of each page before operation
 * was executed.  No error is returned.
 */ 

void
undo_lockop(ep, va, len, lks, lockop) 
vm_map_entry_t ep;
caddr_t va;
size_t len;
struct pg_lk_stat *lks;
int lockop;
{
        register vm_offset_t start, end;
        register struct vpage *vp;
	register vm_size_t size;
	register int i;
	register char cur_group_stat;

	start = (vm_offset_t)va;
	end = (vm_offset_t)va + len;
	cur_group_stat = lks->st_flag[0];

	if (start + PAGE_SIZE > end)
		return;

	/*
	 * Group the consecutive pages with the same status and call
	 * a single lock/unlock operation for it. 
	 */
	for (i=0, size=0; (start + size) < end; ) {
		if (lks->st_flag[i] == cur_group_stat) {
			size += PAGE_SIZE; i++;
			/* If reached end, do the operation and go out */
			if ((start + size) < end)
				continue;
		}

		if (cur_group_stat == LOCKED) {
			if (lockop == MC_UNLOCK) {
				if ((*ep->vme_lockop)(ep, start, size,
				     VM_WIRE) != KERN_SUCCESS)
					return;
			}
			start += size;
			cur_group_stat = UNLOCKED;
			size = 0;
			continue;
		}

		if (cur_group_stat != UNLOCKED)
			panic ("memcntl(): Corrupted status buffer\n");

		/* The pages were unlocked, so unlock 'em since we locked it */
		if (lockop == MC_LOCK) {
			if ((*ep->vme_lockop)(ep, start, size,
			     VM_UNWIRE) != KERN_SUCCESS)
				return;
		}
		cur_group_stat = LOCKED;
		start += size;
		size = 0;
	}
}
