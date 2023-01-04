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
static char *rcsid = "@(#)$RCSfile: kern_mman.c,v $ $Revision: 4.5.14.5 $ (DEC) $Date: 1993/10/19 19:45:36 $";
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
 * Copyright (C) 1988,1989 Encore Computer Corporation.  All Rights Reserved
 *
 * Property of Encore Computer Corporation.
 * This software is made available solely pursuant to the terms of
 * a software license agreement which governs its use. Unauthorized
 * duplication, distribution or sale are strictly prohibited.
 *
 */
/*
 * Copyright (c) 1982, 1986 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *

 */

/* TODO:
 *	msync, madvise, mincore
 *	add support for PROT_NONE
 *	some error status returns are still suspect
 */

#include <sys/secdefines.h>
#include <sys/security.h>
#include <machine/reg.h>
#if	!defined(ibmrt) && !defined(mips)
#include <machine/psl.h>
#endif

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/map.h>
#include <sys/user.h>
#include <sys/proc.h>
#include <sys/mount.h>
#include <sys/vnode.h>
#include <sys/specdev.h>
#include <ufs/inode.h>
#include <sys/acct.h>
#include <sys/wait.h>
#include <sys/vm.h>
#include <sys/file.h>
#include <sys/lock.h>
#include <sys/vadvise.h>
#include <sys/trace.h>
#include <sys/mman.h>
#include <sys/conf.h>
#include <sys/addrconf.h>
#include <machine/addrconf.h>
#include <mach/kern_return.h>
#include <machine/pmap_lw.h>
#include <machine/cpu.h>
#include <kern/sched_prim.h>
#include <kern/task.h>
#include <mach/vm_param.h>
#include <vm/vm_map.h>
#include <vm/vm_mmap.h>
#include <vm/vm_lock.h>
#include <vm/vm_control.h>
#include <rt_pml.h>
#include <rt_sem.h>

#ifdef RT_SEM
#include <binsem.h>
#endif


extern kern_return_t lw_wire();
extern kern_return_t lw_unwire();
kern_return_t decide_on_buffer();


static vm_prot_t vm_prots[] = {
	VM_PROT_NONE,
	VM_PROT_READ,
	VM_PROT_READ|VM_PROT_WRITE,
	VM_PROT_READ|VM_PROT_WRITE,
	VM_PROT_READ|VM_PROT_EXECUTE,
	VM_PROT_READ|VM_PROT_EXECUTE,
	VM_PROT_READ|VM_PROT_WRITE|VM_PROT_EXECUTE,
	VM_PROT_READ|VM_PROT_WRITE|VM_PROT_EXECUTE
};

#define MAX_PROT (PROT_EXEC|PROT_WRITE|PROT_READ)


/* ARGSUSED */
sbrk(p, args, retval)
	struct proc *p;
	void *args;
	long *retval;
{
	struct args {
		long	incr;
	} *uap = (struct args *)args;

	/* Not yet implemented */
	return (EOPNOTSUPP);

}

/* ARGSUSED */
sstk(p, args, retval)
	struct proc *p;
	void *args;
	long *retval;
{
	struct args {
		long	incr;
	} *uap = (struct args *)args;

	/* Not yet implemented */
	return (EOPNOTSUPP);


}

/* ARGSUSED */
getpagesize(p, args, retval)
	struct proc *p;
	void *args;
	long *retval;
{

	*retval = PAGE_SIZE;
	return (0);
}

/* ARGSUSED */
smmap(p, args, retval)
	struct proc *p;
	void *args;
	long *retval;
{
	/*
	 *	Map in special device (must be SHARED) or file
	 */
	register struct args {
		caddr_t	addr;
		u_long	len;		/* real type: 'size_t' */
		long	prot;		/* real type: 'int' */
		long	flags;		/* real type: 'int' */
		long	fd;		/* real type: 'int' */
		u_long	pos;		/* real type: 'off_t' */

	} *uap = (struct args *)args;
	struct file *fp;
	register struct vnode *vp;
	struct		vattr vattr;
	vm_map_t	user_map = current_task()->map;
	kern_return_t	result;
	vm_offset_t	user_addr = (vm_offset_t) uap->addr;
	vm_size_t	user_size = (vm_size_t) uap->len;
	off_t		file_pos = (off_t)uap->pos;
	vm_prot_t	user_prot;
	vm_prot_t	max_prot;
	int		realshare = (uap->flags & (MAP_SHARED|MAP_PRIVATE));
					/* For winmmap kluge */
	int		error = 0;
	struct ufile_state *ufp = &u.u_file_state;
	boolean_t	anywhere;
	enum vtype	vtype;
	int		flag, ret;
        dev_t           dev;
#if	MACH_XP
#else
	extern memory_object_t	memory_object_special();
#endif
	struct file	*oldfp;


	if ((uap->prot < 0) || (uap->prot > MAX_PROT))
		return (EINVAL);
	user_prot = vm_prots[uap->prot];

	/* Check the flags argument. */
	if ((uap->flags & MAP_SHARED) && (uap->flags & MAP_PRIVATE))
		return (EINVAL);
	/* Impossible to check both MAP_FIXED and MAP_VARIABLE being defined */
	/* Impossible to check both MAP_FILE and MAP_VARIABLE being defined. */

	/* Round the end addresses to the nearest page boundary. */
	anywhere = (uap->flags & MAP_FIXED) ? FALSE : TRUE;

	if (!anywhere && !page_aligned(user_addr))
		return (EINVAL);
	if (!(uap->flags & MAP_UNALIGNED) && !page_aligned(file_pos))
		return (EINVAL);
        if ((off_t)user_size == 0)
                return (ENXIO);
	if (user_size + vm_map_vsize(current_task()->map)
		> u.u_rlimit[RLIMIT_AS].rlim_cur)
			return(ENOMEM);

/*
 * According to the DDI/DKI Reference Manual, "a character driver which needs
 * to support the mmap system call supplies either a single mmap entry point,
 * or both an mmap and a segmap entry point routine."  If a segmap entry point
 * is specified in the cdevsw table, the mmap system call must invoke it. If
 * such an entry point is not specified, a default kernel action is taken.
 * In OSF/1, the default is device_pager_create(), provided the kernel is
 * built with MACH_XP.  At the time that this change was made,
 * memory_object_special() didn't seem to exist.
 *
 * According to the DDI/DKI spec, the default mapper provided by the system 
 * can't handle private maps, which is true here in OSF.  The driver supported 
 * segmap entry is supposed to be able to check the flags parameter, and 
 * return appropriate error code if it doesn't support the specified type 
 * of map.
 */
	if ((uap->flags & MAP_TYPE) == MAP_FILE) {
		if (error = getvnode(uap->fd, &fp))
                        return (error);

                vp = (struct vnode *)fp->f_data;
                BM(VN_LOCK(vp));
		vtype = vp->v_type;
                BM(VN_UNLOCK(vp));

		if (vtype == VCHR) {
			/*
			 * Do not attempt to access v_rdev field until certain
			 * that we're dealing with a character device.  
			 * Otherwise, given the arrangement of data structures 
			 * we will end up in hyperspace.
			 */
			dev = vp->v_rdev;
			if (cdevsw[major(dev)].d_segmap != NULL) {
			    CDEVSW_SEGMAP(major(dev), dev, (off_t)(file_pos),
					  user_map, &((caddr_t)user_addr),
					  (off_t)user_size, uap->prot, 
					  max_prot, uap->flags, u.u_cred,
					  error);
			    if (error == KERN_SUCCESS)
				*retval = (long)user_addr;
			    return(error);
			} /*end d_segmap != 0*/	
		} /*end vtype == VCHR*/

		/* we have to do this if getvnode() succeeds*/
		if (fp)
                	FP_UNREF(fp);

	} /*end MAP_FILE*/
	
	/*
	 * See if default address (addr = 0).  Use the address
	 * configuration record: read-only executable file mappings to
	 * MMAP_TEXT, other file mappings to MMAP_DATA, anon mappings to
	 * MMAP_BSS.
	 */
	if (user_addr == (vm_offset_t)0) {
		int     kind;

		if ((uap->flags & MAP_TYPE) == MAP_FILE) {

			if (user_prot == (VM_PROT_READ|VM_PROT_EXECUTE))
				kind = AC_MMAP_TEXT;
			else
				kind = AC_MMAP_DATA;
		} else 
			kind = AC_MMAP_BSS;

		user_addr = (vm_offset_t)(addressconf[kind].ac_base);
	}


	if ((uap->flags & MAP_TYPE) == MAP_ANON) {
	

		/* anonymous memory -- fd must be -1.  No inode; use
		 * MEMORY_OBJECT_NULL.  Maximum protection will
		 * be ALL.
		 */

		if (uap->fd != -1)
			return (EBADF);

		fp = NULL;
		vp = NULL;
		max_prot = VM_PROT_ALL;

	} else  {

		if (error = getvnode(uap->fd, &fp))
			return (error);

		vp = (struct vnode *)fp->f_data;
		
		/*
		 * Maximum protections are based on file descriptor protections
		 * for shared mapping; for private mapping, all access allowed.
		 * However, file must be opened for reading even for private
		 * mapping.
		 */
		BM(FP_LOCK(fp));
		flag = fp->f_flag;
		BM(FP_UNLOCK(fp));	
		if ((uap->flags & (MAP_SHARED|MAP_PRIVATE)) == MAP_SHARED) {

			max_prot = VM_PROT_NONE;
			if (flag & FREAD)
				max_prot |= VM_PROT_READ | VM_PROT_EXECUTE;
			if (flag & FWRITE)
				max_prot |= VM_PROT_WRITE;
		} else {
			if (!(flag & FREAD)) {
				error = EACCES;
				goto out;
			}
			max_prot = VM_PROT_ALL;
		}
		/* Requested protections must be within maximums */

		if ((user_prot & ~max_prot) != VM_PROT_NONE) {
			error = EACCES;
			goto out;
		}


		BM(VN_LOCK(vp));
		vtype = vp->v_type;
		BM(VN_UNLOCK(vp));
	}

	if ((uap->flags & MAP_TYPE) == MAP_ANON) {
		struct vp_mmap_args margs;
		register struct vp_mmap_args *margsp;

		user_size = round_page(user_size);
		margsp = &margs;
		margsp->a_vaddr = &user_addr;
		margsp->a_size = user_size;
		margsp->a_prot = user_prot;
		margsp->a_maxprot = max_prot;
		margsp->a_flags = (uap->flags &
				(MAP_SHARED | MAP_PRIVATE | MAP_FIXED)),
		result = u_anon_create(user_map, VM_OBJECT_NULL, margsp);
	}
	else {
		switch (vtype) {
		case VREG :
			/* Requested file-pos + length must not extend 	*/
			/* beyond end-of-file				*/

			VOP_GETATTR(vp, &vattr, u.u_cred, error);
			if (error)
				goto out;
			if ((off_t)vattr.va_size < (file_pos + (off_t)user_size)) {
				error = ENXIO;
				goto out;
				}
		
		case VCHR :
			user_size = round_page(user_size);
			VOP_MMAP(vp, trunc_page(file_pos), user_map,
					&user_addr,  user_size,
					user_prot, max_prot, 
					(uap->flags & 
					(MAP_SHARED | MAP_PRIVATE | MAP_FIXED)),
					u.u_cred, result);
			if (result != ENODEV) break;
		default :
			error = ENODEV;
			goto out;
		}
	}

	
	if (result == KERN_SUCCESS) *retval = (vm_offset_t) user_addr;
	else switch (result) {

	      case KERN_NO_SPACE:
	      case KERN_INVALID_ADDRESS:
	      case KERN_RESOURCE_SHORTAGE:
		error = ENOMEM;
		break;

	      case KERN_MEMORY_FAILURE:
	      case KERN_MEMORY_ERROR:
		error = EIO;
		break;

	      default:
		error = EINVAL;
		break;

	}

	/*
	 *	Handle MAP_INHERIT (keep-on-exec) regions here
	 */
	if ((result == KERN_SUCCESS) && (uap->flags & MAP_INHERIT)){
		user_size = round_page(user_size);
		(void) vm_keep_on_exec(user_addr, user_size);
	}
	/*
	 *	Throw away references to the appropriate pager
	 */

	if ((uap->flags & MAP_TYPE) != MAP_ANON) {
		if (result != KERN_SUCCESS) {
			goto out;
		}
		
		/*
		 * HACK attack.  XXX
		 * If another thread in this task re-allocated the file
		 * descriptor slot, blow him away.  Probably should free
		 * up all resources and return an error to this thread,
		 * or better yet mark the descriptor slot as reserved at
		 * the beginning.
		 */
		oldfp = NULL;
		U_FDTABLE_LOCK(ufp);
		if ((U_OFILE(uap->fd, ufp)) != fp) {
			oldfp = U_OFILE(uap->fd, ufp);
			U_OFILE_SET(uap->fd, fp, ufp);
		}
		U_POFILE_SET(uap->fd, U_POFILE(uap->fd, ufp) | UF_MAPPED, ufp);
		U_FDTABLE_UNLOCK(ufp);
		if (oldfp)
			FP_UNREF(oldfp);
	}
out:
	if (fp)
		FP_UNREF(fp);
	return (error);

}
long lww_spot1 = 0;
long lww_spot2 = 0;
long lww_spot3 = 0;
long lww_spot4 = 0;
long lww_spot5 = 0;
long lww_spot6 = 0;
long lww_spot7 = 0;
int lww_error = 0;

extern u_int printstate;
int lww[20];
int lww_debug1 = 0;
extern kern_return_t vm_lw_fault();
u_long lww_time[50];
u_long lww_event[100];
extern u_int printstate;
int lww_debug6 = 0;
kern_return_t
lw_wire(va, n_pages, user_buf)
     vm_offset_t    va;
     u_long            n_pages;
     u_long         *user_buf;
{
     u_long           *buf;
     int              error, i;
     vm_map_t         map = current_task()->map;
     pmap_t           pmap = map->vm_pmap;
     vm_lw_trans_t    t;
     u_long           temp;
     int ret;


#ifdef LWW_EVENTS
     ++lww_event[20];
#endif
     if (u.u_uid != 0)
       return KERN_FAILURE;

     t = (vm_lw_trans_t)0;
     simple_lock(&vm_lw_trans_queue_lock);
     if(lw_free_queue) {
         t =  lw_free_queue;
	 lw_free_queue = t->next;
     }
     simple_unlock(&vm_lw_trans_queue_lock);
     if(t == (vm_lw_trans_t)0) {
     ++lww_event[21];
         ZALLOC(vm_lw_trans_zone, t, vm_lw_trans_t);
         if(t == (vm_lw_trans_t)0)
	     return KERN_FAILURE;
     }

     ret = 0;
     t->map = map;
     t->va = va;
     t->n_pages = n_pages;

#ifdef LWW_AUD1
     current_pcb->pcb_nofault = NF_LWERR;
     error = pmap_lw_wire(pmap, va, n_pages, user_buf, t);
#else
     current_pcb->pcb_nofault = NF_LWERR_ASS;
     error = pmap_lw_wire_ass(pmap, va, n_pages, user_buf, t);
#endif
     current_pcb->pcb_nofault = 0;

     if(error == 0) {
#ifdef LWW_EVENTS
     ++lww_event[22];
#endif
         simple_lock(&vm_lw_trans_queue_lock);
	 t->next = lw_trans_queue;
	 lw_trans_queue = t;
	 simple_unlock(&vm_lw_trans_queue_lock);
         return KERN_SUCCESS;;
     }

     lww_error = error;
     simple_lock(&vm_lw_trans_queue_lock);     
     t->next = lw_free_queue;
     lw_free_queue = t;
     simple_unlock(&vm_lw_trans_queue_lock);     

     ++lww_event[60];
     current_pcb->pcb_nofault = NF_LW_UNERR;
     pmap_lw_unwire_ass(pmap, va, n_pages);
     current_pcb->pcb_nofault = 0;
     ++lww_event[61];

     ZALLOC(vm_lw_trans_zone, t, vm_lw_trans_t);
     if(t == (vm_lw_trans_t)0)
         return KERN_FAILURE;

     t->map = map;
     t->va = va;
     t->n_pages = n_pages | VM_FULL_WIRE;

     buf = (u_long *) kalloc((n_pages + 1) * sizeof(u_long));

     if(buf == (u_long *)0)  {
         ++lww_event[62];
	 ret = 62;
         goto bad;
     }
     if((error = vm_map_pageable(map, va, va + (n_pages * NBPG), 
				 VM_PROT_READ)) !=
	  KERN_SUCCESS) {
          ++lww_event[63];
	  ret = error;
          goto bad;
	}

     *(vm_lw_trans_t *)buf = t;
     buf++;
     pmap_get_pfns(pmap, va, n_pages, buf);
     buf--;

     ++lww_event[64];
     error = copyout(buf, user_buf, (n_pages + 1) * sizeof(u_long));
     if(error) {
          vm_map_pageable(map, va, va + (n_pages * NBPG), VM_PROT_NONE);
	  ++lww_event[65];
	  ret = 65;
	  goto bad;
	}

     ++lww_event[66];

     simple_lock(&vm_lw_trans_queue_lock);
     t->next = lw_trans_queue;
     lw_trans_queue = t;
     simple_unlock(&vm_lw_trans_queue_lock);
     kfree(buf, (n_pages + 1) * sizeof(u_long));

     return KERN_SUCCESS;

   bad:
     ++lww_event[67];
     if(buf != (u_long *)0)
       kfree(buf, (n_pages + 1) * sizeof(u_long));

     ++lww_event[68];
     ZFREE(vm_lw_trans_zone, (vm_offset_t)t);
     ++lww_event[69];
     if (ret)
       return ret;
     else
       return KERN_FAILURE;
   }


long lww_use_aud = 0;
kern_return_t
lw_unwire(t)
     vm_lw_trans_t t;
{
     vm_lw_trans_t   tr, last;
     vm_offset_t     va;
     pmap_t          pmap;
     u_long pages;
     int error;
     u_long dirty_pages;

#ifdef LWW_EVENTS
     ++lww_event[45];
#endif

     if((u_long)t & 0x3) {
         dirty_pages = (u_long)t & 0x3;
	 t = (vm_lw_trans_t)((u_long)t & ~0x3);
     }
     else
         dirty_pages = 0;
     if(!lw_trans_queue)
         return KERN_FAILURE;
     simple_lock(&vm_lw_trans_queue_lock);
     if (t == lw_trans_queue)
         lw_trans_queue = t->next;
     else {
         for(last = 0, tr = lw_trans_queue;
	       tr != (vm_lw_trans_t)0; last = tr, tr = tr->next) {
	     if(tr == t) {
	         if(last == 0) {
		     lw_trans_queue = tr->next;
		 }
		 else
		       last->next = t->next;
		 break;
	     }
	 }
	 if (tr == (vm_lw_trans_t)0) {
	     ++lww_event[46];
	     simple_unlock(&vm_lw_trans_queue_lock);     
	     return KERN_FAILURE;
	 }
     }
     simple_unlock(&vm_lw_trans_queue_lock);

     if(dirty_pages) {
          error = 0;
          if(dirty_pages == 1) {
	      ++lww_event[49];
	      pmap_lw_set_modify(t->map->vm_pmap, t->va, t->n_pages & ~VM_FULL_WIRE);
	  }
	  else if (dirty_pages == 2){
	      ++lww_event[50];
	      error =
		  pmap_lw_check_modify(t->map->vm_pmap, t->va,
				  t->n_pages & ~VM_FULL_WIRE);
	  }
	  else {
	      ++lww_event[51];
	      pmap_lw_clear_modify(t->map->vm_pmap, t->va,
				  t->n_pages & ~VM_FULL_WIRE);
	  }
          simple_lock(&vm_lw_trans_queue_lock);
	  t->next = lw_trans_queue;
	  lw_trans_queue = t;
	  simple_unlock(&vm_lw_trans_queue_lock);
	  return error;
     }

     if((t->n_pages & VM_FULL_WIRE) == 0) {
#ifdef LWW_EVENTS
          ++lww_event[47];
#endif

	  pmap = t->map->vm_pmap;
	  va = t->va;
	  pages = t->n_pages;

#ifdef LWW_AUD2
	  current_pcb->pcb_nofault = NF_LW_UNERR_AUD;
          error = pmap_lw_unwire_aud(pmap, va, pages);
#else
	  current_pcb->pcb_nofault = NF_LW_UNERR;
          error = pmap_lw_unwire_ass(pmap, va, pages);
#endif
	  current_pcb->pcb_nofault = 0;
	  if(error != 0)
	       panic("lw_unwire\n");

	  simple_lock(&vm_lw_trans_queue_lock);     
	  t->next = lw_free_queue;
	  lw_free_queue = t;
	  simple_unlock(&vm_lw_trans_queue_lock);     
     }
     else {
          ++lww_event[48];
          vm_map_pageable(t->map, t->va,
		  t->va + (t->n_pages & ~VM_FULL_WIRE) * NBPG,
		  VM_PROT_NONE);
	  ZFREE(vm_lw_trans_zone, (vm_offset_t)t);
     }
     if (lw_waiters) {
         thread_wakeup((vm_offset_t)&lw_waiters);
	 lw_waiters = 0;
     }
     return KERN_SUCCESS;

}

void lw_delete_wirings(map)
     vm_map_t   map;
{
     vm_lw_trans_t    t;

     if(!lw_trans_queue)
           return;

   restart:

     simple_lock(&vm_lw_trans_queue_lock);
     for (t = lw_trans_queue; t != (vm_lw_trans_t)0; t = t->next)
          if(t->map == map) {
		  simple_unlock(&vm_lw_trans_queue_lock);     
		  lw_unwire(t);
		  goto restart;
	  }
     simple_unlock(&vm_lw_trans_queue_lock);     
     return;
}

/*
 * Is this virtual address range 
 * (lw)wired at any single page.
 */


boolean_t lw_is_wired(ep, start, end)
     vm_map_entry_t ep;
     vm_offset_t start;
     vm_offset_t end;
{
     vm_lw_trans_t    t;
     vm_offset_t wired_start;
     vm_offset_t wired_end;
     boolean_t   found;

     found = FALSE;

     if(lw_trans_queue) {
         simple_lock(&vm_lw_trans_queue_lock);
	 for (t = lw_trans_queue; t != (vm_lw_trans_t)0; t = t->next)
	      if(t->map == ep->vme_map) {
		   wired_start = t->va;
		   wired_end = t->va + t->n_pages * NBPG;
		   if ((start >= wired_start && start <= wired_end) ||
		       (end >= wired_start && end <= wired_end) ||
		       (start < wired_start && end > wired_end)) {
		        found = TRUE;
			break;
		   }
	      }
	 simple_unlock(&vm_lw_trans_queue_lock);
     }
     return found;
}

void lw_remove(map)
     vm_map_t   map;
{
     vm_lw_trans_t    t;

     if(lw_trans_queue) {
         simple_lock(&vm_lw_trans_queue_lock);
	 for (t = lw_trans_queue; t != (vm_lw_trans_t)0; t = t->next)
	      if(t->map == map)
		   panic("lw transaction found");
	 simple_unlock(&vm_lw_trans_queue_lock);
     }
     return;
}




/* ARGSUSED */
mremap(p, args, retval)
	struct proc *p;
	void *args;
	long *retval;
{
	register struct args {
		char	*addr;
		long	len;
	} *uap = (struct args *)args;

	/* Not yet implemented */
	return (EOPNOTSUPP);
}

/* ARGSUSED */
munmap(p, args, retval)
	struct proc *p;
	void *args;
	long *retval;
{
	register struct args {
		caddr_t	addr;
		u_long	len;		/* real type: 'size_t' */
	} *uap = (struct args *)args;
	vm_offset_t	user_addr;
	vm_size_t	user_size;
	kern_return_t	result;

	user_addr = (vm_offset_t) uap->addr;
	user_size = (vm_size_t) uap->len;
	/*
	 *	We bend a little - round the end addresses
	 *	to the nearest page boundary.
	 */
	if (!page_aligned(user_addr))
		return (EINVAL);
	user_size = round_page(user_size);
	result = vm_map_delete(current_task()->map, user_addr, 
		user_addr + user_size, FALSE);
	if (result != KERN_SUCCESS)
		return (EINVAL);
	return(result);
}

#ifdef notdef
munmapfd(fd)
{
	u.u_pofile[fd] &= ~UF_MAPPED;
}
#endif

/* ARGSUSED */
mprotect(p, args, retval)
	struct proc *p;
	void *args;
	long *retval;
{
	register struct args {
		caddr_t	addr;
		u_long	len;		/* real type: 'size_t' */
		long	prot;		/* real type: 'int' */
	} *uap = (struct args *)args;
	vm_map_t map = current_task()->map;
	vm_offset_t	user_addr;
	vm_size_t	user_size;
	kern_return_t	result;
	vm_prot_t	user_prot;

	if ((uap->prot < 0) || (uap->prot > MAX_PROT))
		return (EINVAL);
	user_prot = vm_prots[uap->prot];
	user_addr = (vm_offset_t) uap->addr;
	user_size = (vm_size_t) uap->len;
	/*
	 *	We bend a little - round the end addresses
	 *	to the nearest page boundary.
	 */
	if (!page_aligned(user_addr))
		return (EINVAL);
	user_size = round_page(user_size);

	/* Note: it appears that vm_protect allows you to apply protections to
	 * non-existent regions of the address space, which is supposed to be
	 * an error according to the AES.  If so, mprotect will have to make
	 * some kind of validity check on the region being re-protected.
	 */

	result = vm_protect(map, user_addr, user_size, FALSE, user_prot);
	if (result != KERN_SUCCESS) switch (result) {

		case KERN_PROTECTION_FAILURE:
			return (EACCES);

		case KERN_NO_SPACE:
		case KERN_INVALID_ADDRESS:
		case KERN_RESOURCE_SHORTAGE:
			return (ENOMEM);

		default:
			return (EINVAL);
	}
	return (0);
}

/* ARGSUSED */
mvalid(p, args, retval)
	struct proc *p;
	void *args;
	long *retval;
{
	register struct args {
		caddr_t addr;
		u_long  len;		/* real type: 'size_t' */
		long     prot;		/* real type: 'int' */
	} *uap = (struct args *)args;
	vm_map_t map = current_task()->map;
	vm_offset_t     user_addr;
	vm_size_t       user_size;
	kern_return_t   result;
	vm_prot_t       user_prot;

	if ((uap->prot < 0) || (uap->prot > MAX_PROT))
		return (EINVAL);
	user_prot = vm_prots[uap->prot];
	user_addr = (vm_offset_t) uap->addr;
	user_size = (vm_size_t) uap->len;
	/*
	 *      We bend a little - round the end addresses
	 *      to the nearest page boundary.
	 */
	if (!page_aligned(user_addr))
		return (EINVAL);
	user_size = round_page(user_size);
	
	result = vm_map_check_protection(map, user_addr, user_addr + user_size,
					user_prot);
	if (!result)
		return (EACCES);
	else
		return (0);
}

/* ARGSUSED */
madvise(p, args, retval)
	struct proc *p;
	void *args;
	long *retval;
{
 	struct args {
		char    *addr;
		long     len;
		long     behav;
	} *uap = (struct args *)args;
        int error;

	/* just check for invalid behav argument */

	*retval = 0;

        switch (uap->behav) {

        case MADV_NORMAL:
        case MADV_RANDOM:
        case MADV_SEQUENTIAL:
        case MADV_WILLNEED:
        case MADV_DONTNEED:
        case MADV_SPACEAVAIL:
	  error = ESUCCESS;
          break;
        default:
          error = EINVAL;
        }

	return (error);

}

/* ARGSUSED */
mincore(p, args, retval)
	struct proc *p;
	void *args;
	long *retval;
{
 	struct args {
		char    *addr;
		long     len;
		char	*vec;
	} *uap = (struct args *)args;

	/* Not yet implemented */
	return (EOPNOTSUPP);

}

/* msleep - If semaphore is still locked when checked by checked put */
/* 	requestor to sleep, waiting for an mwakeup on the semaphore. */
/* ARGSUSED */
msleep(p, args, retval)
	struct proc *p;
	void *args;
	long *retval;
{
	struct args {
		msemaphore *sem;
	} *uap = (struct args *)args;
	kern_return_t result;

	result = u_map_control(current_task()->map, uap->sem,
		(vm_offset_t) 0, VMC_SEM_SLEEP, (long) 0);

	if (result == THREAD_INTERRUPTED)
		return (EINTR);
	else if (result != 0)
		return (EFAULT);
	return (0);
}

/* mwakeup - Wakeup any threads waiting on the semaphore.  Note the  */
/* 	semaphore may still be locked. */
/* ARGSUSED */
mwakeup(p, args, retval)
	struct proc *p;
	void *args;
	long *retval;
{
	struct args {
		msemaphore *sem;
	} *uap = (struct args *)args;
	kern_return_t result;

	result = u_map_control(current_task()->map, uap->sem,
		(vm_offset_t) 0, VMC_SEM_WAKEUP, (long) 0);

	if (result)
		return (EFAULT);
	return (0);
}

/* msync - Synchronize a mapped region with its backing object */
/* ARGSUSED */
msync(p, args, retval)
	struct proc *p;
	void *args;
	long *retval;
{
	struct args {
		caddr_t addr;
		u_long len;		/* real type: 'size_t' */
		long flags;		/* real type: 'int' */
	} *uap = (struct args *)args;
	vm_size_t size;
	kern_return_t result;

	if (!page_aligned(uap->addr))
		return (EINVAL);

	/* Flags can only be one of the following values. */
	switch ((int)uap->flags) {
	case MS_SYNC:
	case MS_ASYNC:
	case MS_INVALIDATE:
	case MS_SYNC|MS_INVALIDATE:
	case MS_ASYNC|MS_INVALIDATE:
		break;
	default:
		return (EINVAL);
	}
	
	size = round_page((size_t)uap->len);
	
	result = u_map_control(current_task()->map, uap->addr, uap->addr + size,
		VMC_MSYNC, uap->flags);

	if (result) switch(result) {
	case KERN_INVALID_ADDRESS:
		return (ENOMEM);
	case KERN_MEMORY_ERROR:
		return (EIO);
	default:
		return (EINVAL);
	}
	return (0);
}

	
       
/* BEGIN DEFUNCT */
/* ARGSUSED */
obreak(p, args, retval)
	struct proc *p;
	void *args;
	long *retval;
{
	struct args {
		char	*nsiz;
	} *uap = (struct args *)args;
	vm_offset_t	old, new;
	kern_return_t	ret;

	new = round_page(uap->nsiz);
	/* The calculation "new - u.u_data_start" is only valid
	 * if new > u.u_data_start. */
	if (new > (vm_offset_t)u.u_data_start
	    && (new - (vm_offset_t)u.u_data_start)
			> u.u_rlimit[RLIMIT_DATA].rlim_cur)
		return (ENOMEM);
	old = round_page(u.u_data_start + ctob(u.u_dsize));
	if (new > old) {
		if (new - old + vm_map_vsize(current_task()->map)
			> u.u_rlimit[RLIMIT_AS].rlim_cur)
			return(ENOMEM);
		ret = vm_allocate(current_task()->map, &old, new - old, FALSE);
		if (ret != KERN_SUCCESS)
			return (ENOMEM);
		else u.u_dsize += btoc(new - old);
	}
	return (0);
}

int	both;

/* ARGSUSED */
ovadvise(p, args, retval)
	struct proc *p;
	void *args;
	long *retval;
{
#ifdef	lint
	both = 0;
#endif
	return (EOPNOTSUPP);
}
/* END DEFUNCT */

#ifdef notdef
/*
 * grow the stack to include the SP
 * true return if successful.
 */
grow(sp)
	unsigned long sp;
{
	register long si;

	if (sp >= USRSTACK-ctob(u.u_ssize))
		return (0);
	si = round_page((USRSTACK - sp) - ctob(u.u_ssize) + ctob(SINCR));
	if ((unsigned long)si > u.u_rlimit[RLIMIT_STACK].rlim_cur)
		return (0);
	return (1);
}
#endif /* notdef */

/* ARGSUSED */
set_program_attributes(p, args, retval)
	struct proc *p;
	void *args;
	long *retval;
{
	struct args {
		caddr_t	text_start;
		u_long	tsize;		/* real type: 'size_t' */
		caddr_t	data_start;
		u_long	dsize;		/* real type: 'size_t' */
	} *uap = (struct args *)args;
	size_t ts, ds;

	ts = round_page((size_t)uap->tsize);
	ds = round_page((size_t)uap->dsize);

	if ((unsigned int)ds > u.u_rlimit[RLIMIT_DATA].rlim_cur)
		return (ENOMEM);
	u.u_text_start = uap->text_start;
	u.u_tsize = btoc(ts);
	u.u_data_start = uap->data_start;
	u.u_dsize = btoc(ds);
	return (0);
}


/* plock system call -- lock text and/or data in memory */
/* ARGSUSED */
plock(p, args, retval)
	struct proc *p;
	void *args;
	long *retval;
{
	register struct args {
		long cmd;
	} *uap = (struct args *)args;
	int error = 0;
	vm_map_t mymap = current_task()->map;

#if SEC_BASE
	/*
	 * Must have privilege
	 */
	if (!privileged(SEC_LOCK, EPERM))       /* XXX which priv needed? */
		return (EPERM);
#else
	if (error = suser(u.u_cred, &u.u_acflag))
		return (error);
#endif

	vm_map_lock(mymap);
	vm_map_lock_set_recursive(mymap);

	/*
	 * Note that the protections specified in the calls to
	 * vm_map_pageable must be a (possibly improper) subset
	 * of the protections specified in the vm_map calls made
	 * by exec.
	 */

	if ( (uap->cmd == TXTLOCK) || (uap->cmd == PROCLOCK) ) {

		if (u.u_lflags & UL_TXTLOCK) {
			error = EINVAL;
			goto out;
		}

		if ((error = u_map_lockvas(mymap, trunc_page(u.u_text_start),
			round_page(u.u_text_start + ctob(u.u_tsize)),
			VML_LOCK_RANGE)) != KERN_SUCCESS) {
			if (error == KERN_RESOURCE_SHORTAGE)
				error = EAGAIN;
			else
				error = EINVAL;
			goto out;
		}

		u.u_lflags |= UL_TXTLOCK;
	}

	if ( (uap->cmd == DATLOCK) || (uap->cmd == PROCLOCK) ) {

		if (u.u_lflags & UL_DATLOCK) {
			error = EINVAL;
			goto out;
		}

		if ((error = u_map_lockvas(mymap, trunc_page(u.u_data_start),
				round_page(u.u_data_start + ctob(u.u_dsize)),
				VML_LOCK_RANGE|VML_FUTURE)) != 
					KERN_SUCCESS) {
			if (uap->cmd == PROCLOCK) {
				u_map_lockvas(mymap,
				   trunc_page(u.u_text_start),
				   round_page(u.u_text_start + ctob(u.u_tsize)),
				   VML_UNLOCK_RANGE);
				u.u_lflags &= ~UL_TXTLOCK;
			}
			if (error == KERN_RESOURCE_SHORTAGE)
				error = EAGAIN;
			else
				error = EINVAL;
			goto out;
		}

		if ((error = u_map_lockvas(mymap, trunc_page(u.u_stack_start),
				round_page(u.u_stack_end),
				VML_LOCK_RANGE)) != KERN_SUCCESS) {

			u_map_lockvas(mymap, trunc_page(u.u_data_start),
				round_page(u.u_data_start + ctob(u.u_dsize)),
				VML_UNLOCK_RANGE);
			if (uap->cmd == PROCLOCK) {
				u_map_lockvas(mymap,
				   trunc_page(u.u_text_start),
				   round_page(u.u_text_start + ctob(u.u_tsize)),
				   VML_UNLOCK_RANGE);
				u.u_lflags &= ~UL_TXTLOCK;
			}
			if (error == KERN_RESOURCE_SHORTAGE)
				error = EAGAIN;
			else
				error = EINVAL;
			goto out;
		}

		u.u_lflags |= UL_DATLOCK;
	}

	else if (uap->cmd == UNLOCK) {

		/*
		 * We return an error only if none of the regions
		 * is wired.
		 */
		if ((u.u_lflags & (UL_TXTLOCK|UL_DATLOCK)) == 0) {
			error = EINVAL;
			goto out;
		}

		/*
		 * Following calls "can't" fail given the
		 * check we just made.
		 */
		if (u_map_lockvas(mymap, vm_map_min(mymap), vm_map_max(mymap),
				VML_UNLOCK_ALL) != KERN_SUCCESS) 
			error = EINVAL;

		u.u_lflags &= ~(UL_TXTLOCK|UL_DATLOCK);
	}
out:
	vm_map_lock_clear_recursive(mymap);
	vm_map_unlock(mymap);
	return (error);
}

/* ARGSUSED */
getaddressconf(p, args, retval)
	struct proc *p;
	void *args;
	long *retval;
{
	register struct args {
		struct addressconf *buf;
		u_long  size;		/* real type: 'size_t' */
	} *uap = (struct args *)args;
	size_t          copysize;
	int             error;

	copysize = (uap->size / sizeof(struct addressconf)) * sizeof(struct addressconf);
	if (copysize > sizeof(addressconf))
		copysize = sizeof(addressconf);
	error = copyout((caddr_t)addressconf, (caddr_t)(uap->buf), copysize);
	if (error)
		return (error);
	*retval = (long)copysize;
	return (error);
}

#if	RT_PML

/* memlk system call -- lock text, data, stack or entire process in memory */
/* ARGSUSED */
memlk(p, args, retval)
	struct proc *p;
	void *args;
	long *retval;
{
	register struct args {
		long 		type;		/* real type: 'int' */
		void		*addr;
		size_t		size; 
	} *uap = (struct args *)args;
	int error;
	register unsigned int type = uap->type;
	vm_offset_t     user_addr;
	vm_size_t       user_size;
	vm_map_t mymap = current_task()->map;
	
	user_addr = (vm_offset_t) uap->addr;
	user_size = (vm_size_t) uap->size;
        if (user_size == 0) user_size = 1;

#if SEC_BASE
	/*
	 * Must have privilege
	 */
	if (!privileged(SEC_LOCK, EPERM))       /* XXX which priv needed? */
		return (EPERM);
#else
	if (error = suser(u.u_cred, &u.u_acflag))
		return (error);
#endif
        error = 0;

	vm_map_lock(mymap);
	vm_map_lock_set_recursive(mymap);
	
	if ((type == 0) || (type & ~(MCL_CURRENT|MCL_FUTURE|REGLOCK))) {
		error = EINVAL;
		goto out;
	}

	if (type & REGLOCK) {
		if (!page_aligned(user_addr)) {
			error = EINVAL;
			goto out;
		}
		if((error = u_map_lockvas(mymap, trunc_page(user_addr),
		    round_page(user_addr + user_size), VML_LOCK_RANGE)) != KERN_SUCCESS) {
			if (error == KERN_INVALID_ADDRESS)
				error = ENOMEM;
			else
				error = EAGAIN;
		}
		goto out;
	} 

        if (type & MCL_FUTURE) {   
                if(u_map_lockvas(mymap, vm_map_min(mymap),
                        vm_map_max(mymap), VML_FUTURE) != KERN_SUCCESS) { 
                        error = EAGAIN;
                        goto out; 
                } else u.u_lflags |= UL_ALL_FUTURE;   
        }
	
        if (type & MCL_CURRENT) {
                 if(u_map_lockvas(mymap, vm_map_min(mymap),
                        vm_map_max(mymap), VML_LOCK_ALL) != KERN_SUCCESS) {
			if (type & MCL_FUTURE) {
				u_map_lockvas(mymap, vm_map_min(mymap),
                        		vm_map_max(mymap), VML_NOFUTURE);
				u.u_lflags &= ~UL_ALL_FUTURE;	
			}
               		error = EAGAIN;
                        goto out;
		} 
        }

out:
	vm_map_lock_clear_recursive(mymap);
	vm_map_unlock(mymap);
	return (error);
}


/* memunlk system call -- unlock text, data, stack or entire process */
/* ARGSUSED */
memunlk(p, args, retval)
	struct proc *p;
	void *args;
	long *retval;
{
	register struct args {
		long 		type;		/* real type: int */
		void		*addr;
		size_t		size;
	} *uap = (struct args *)args;
	int error;
	kern_return_t	ret;
	vm_offset_t     user_addr;
	vm_size_t       user_size;
	vm_map_t mymap = current_task()->map;
	register unsigned int type = uap->type;

	user_addr = (vm_offset_t) uap->addr;
	user_size = (vm_size_t) uap->size;
        if (user_size == 0) user_size = 1;	

#if SEC_BASE
	/*
	 * Must have privilege
	 */
	if (!privileged(SEC_LOCK, EPERM))       /* XXX which priv needed? */
		return (EPERM);
#else
	if (error = suser(u.u_cred, &u.u_acflag))
		return (error);
#endif
        error = 0;

	vm_map_lock(mymap);
	vm_map_lock_set_recursive(mymap);

        if ((type == 0) || (type & ~(MCL_CURRENT|MCL_FUTURE|REGLOCK))) {
                error = EINVAL;
                goto out;
        }

        if (type & REGLOCK) {
                if (!page_aligned(user_addr)) {
                        error = EINVAL;
                        goto out;
                }

                if((error = u_map_lockvas(mymap, trunc_page(user_addr),
                    round_page(user_addr + user_size), VML_UNLOCK_RANGE)) != KERN_SUCCESS)
                        if (error == KERN_INVALID_ADDRESS)
                                error = ENOMEM;
                        else
                                error = EAGAIN;
                goto out;
        }

        if (type & MCL_FUTURE) {
                if(u_map_lockvas(mymap, vm_map_min(mymap),
                        vm_map_max(mymap), VML_NOFUTURE) != KERN_SUCCESS) {
                        error = EAGAIN;
                        goto out;
                } else u.u_lflags &= ~UL_ALL_FUTURE;
        }

        if (type & MCL_CURRENT) {
                 if(u_map_lockvas(mymap, vm_map_min(mymap),
                        vm_map_max(mymap), VML_UNLOCK_ALL) != KERN_SUCCESS) {
                        error = EAGAIN;
                        goto out;
                }
        }

out:
		
	vm_map_lock_clear_recursive(mymap);
	vm_map_unlock(mymap);

	return (error);
}

#else	/* RT_PML */

/* ARGSUSED */
memlk(p, args, retval)
	struct proc *p;
	void *args;
	long *retval;
{
	struct args {
		long	incr;
	} *uap = (struct args *)args;

	/* Not yet implemented */
	return (ENOSYS);

}

/* ARGSUSED */
memunlk(p, args, retval)
	struct proc *p;
	void *args;
	long *retval;
{
	struct args {
		long	incr;
	} *uap = (struct args *)args;

	/* Not yet implemented */
	return (ENOSYS);

}

#endif	/* RT_PML */

#if RT_SEM

decl_simple_lock_data(,p4bsem_lock) /* Global lock used for binary semaphore */

#define       P4SEM_LOCK()                   simple_lock(&p4bsem_lock)
#define       P4SEM_UNLOCK()                 simple_unlock(&p4bsem_lock)


/*
 * This routine initializes the global lock used by the binary semaphore code.
 */

void rtbsem_init()
{
        simple_lock_init(&p4bsem_lock);
}

/*                                                                                  */
/* psx4_sem_sleep - If binary semaphore is still locked, then put                   */
/* the requestor to sleep, waiting for an psx4_sem_wakeup on the binary semaphore.  */



psx4_sem_sleep(p, args, retval)
	struct proc *p;
	void *args;
	long *retval;
{
        register struct args {
	     p4sem_t        *usem;		/* binary semaphore  */
             p4_key_entry_t *ke;
             p4semid_set_t  *obj;     
        }*uap = (struct args *) args;
       
        kern_return_t result = KERN_SUCCESS;
        int             my_lock = P4_MEM_LOCK;
        p4sem_t		sem, *usem;
        p4_key_entry_t  my_ke, *uke;
        p4semid_set_t   my_obj;
 	vm_offset_t     pa, offset;
        int             sts;
        vm_offset_t     uobj;
        vm_map_entry_t  entry;

    
	vm_map_t mymap = current_task()->map;        
        usem = uap->usem;
        uobj = (vm_offset_t)uap->obj;
        uke  = uap->ke;  
	P4SEM_LOCK();                    /* Lock global binary semaphore lock. */

        vm_map_lock(mymap);
        vm_map_lock_set_recursive(mymap);

 	if ((copyin((caddr_t)uap->usem, (caddr_t) &sem, sizeof(p4sem_t))) ||
            (copyin((caddr_t)uap->ke,   (caddr_t) &my_ke, sizeof(p4_key_entry_t))) ||	
            (copyin((caddr_t)uap->obj,  (caddr_t) &my_obj,sizeof(p4semid_set_t))))
	{
                result = EINVAL;
 		goto errout;

	}
     
	/*
        ** lock down the semaphore set into the physical memory if it has not done yet.
        */
       
        if((my_ke.lock != P4_MEM_LOCK) || (my_ke.pid !=  p->p_pid))
	  {

           	if (!page_aligned(uobj)) {
			result = EINVAL;
			goto errout;
		}
               
                if(!vm_map_lookup_entry(mymap, uobj, &entry)){
                        result = EINVAL;
                        goto errout;
		      }

                if(!ismmaper(entry))                
                {
                       result = EINVAL;
                       goto errout;
		}
                       
 
		if (u_map_lockvas(mymap, trunc_page(uobj),
				round_page((uobj+sizeof(p4semid_set_t))),
				VML_LOCK_RANGE) != KERN_SUCCESS) {
			result = EAGAIN;
			goto errout;				
		} 
              
                if(copyout((caddr_t)(&my_lock), (caddr_t)&(uke->lock), sizeof(int)) ||
                   copyout((caddr_t)(&p->p_pid), (caddr_t)&(uke->pid),  sizeof(pid_t)))
		 {
                    result = EINVAL;
                    goto errout;
                 }

  
         }		  
		  

	vm_map_lock_clear_recursive(mymap);
	vm_map_unlock(mymap);
               
       	/* Find the physical address of the binary  semaphore. */
        pa = pmap_extract(vm_map_pmap(mymap), uap->usem);
        if(!pa){
                result = EINVAL;
                goto out;
	}
        
	/* Wait for the semaphore to free up. */
 
        while(usem->semstate != SEM_UNLOCKED){
	  usem->semncnt++;
          sts = mpsleep(pa, PCATCH|PZERO, "p4_sem", 0,  
		simple_lock_addr(p4bsem_lock), 
          MS_LOCK_SIMPLE|MS_LOCK_ON_ERROR, p->p_pid); 
          usem->semncnt--;
	  if(sts){
            result = EINTR;
            goto out;
	  }
	}

         usem->semstate = SEM_LOCKED;
 
out:     P4SEM_UNLOCK();
         return(result);
 
errout:	 vm_map_lock_clear_recursive(mymap);
	 vm_map_unlock(mymap);
         P4SEM_UNLOCK();
	 return (result);

}


/* psx4_sem_wakeup - Wakeup the hightest priority process waiting on the binary semaphore */ 


psx4_sem_wakeup(p, args, retval)
	struct proc *p;
	void *args;
	long *retval;
{
   register struct args {
	  p4sem_t   *usem;                          /* binary semaphore  */
    }	*uap = (struct args *) args;


   kern_return_t        result = KERN_SUCCESS;
   p4sem_t 	        sem, *usem;
   vm_offset_t          pa;
   int                  s;
   vm_map_t 	        mymap = current_task()->map;
             
   usem = uap->usem;
   P4SEM_LOCK();                   /* Lock global semaphore lock. */
		  
 
  if (copyin((caddr_t)uap->usem, (caddr_t) &sem, sizeof(p4sem_t)))
	{
                result = EINVAL;
                goto out;

	}

  
   /* Find the physical address of the binary  semaphore. */
   vm_map_lock(mymap);
   vm_map_lock_set_recursive(mymap);
        
   pa = pmap_extract(vm_map_pmap(mymap), uap->usem);
   vm_map_lock_clear_recursive(mymap);
   vm_map_unlock(mymap);

   if(!pa){
         result = EINVAL;
         goto out;
     }
  
   if(usem->semncnt){
     s = splhigh(); 
     thread_wakeup_high((vm_offset_t)pa); 
     splx(s); 
   }

out:   P4SEM_UNLOCK();
       return(result);
 }


#else                   /* stub entry point */

psx4_sem_sleep(p, args, retval)
	struct proc *p;
	void *args;
	long *retval;
{

	return(ENOSYS);
}


psx4_sem_wakeup(p, args, retval)
	struct proc *p;
	void *args;
	long *retval;
{

	return(ENOSYS);
}


void rtbsem_init()
{
    return;
}


#endif     /* RT_SEM */
