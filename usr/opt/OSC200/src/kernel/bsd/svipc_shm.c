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
static char *rcsid = "@(#)$RCSfile: svipc_shm.c,v $ $Revision: 4.3.14.10 $ (DEC) $Date: 1993/10/04 13:17:13 $";
#endif 
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/* 
 * OSF/1 Release 1.0
 */
/*
 * svipc_shm.c
 *
 *      Modification History:
 *
 * 04-Dec-91    Marian Macartney
 *      Ensure that shmdt detaches only at a segment boundary.
 *
 * 29-Oct-91	Fred Canter
 *	Make system V IPC definitions configurable.
 *
 * 18-Aug-91	Fred Canter
 *	Bob Picco's fix for the VM IPC short circuit code not working
 *	for shared memory. The system would panic if shared memory was
 *	in use and paging to a raw disk instead of a paging file, because
 *	paging I/O is not synchronized thru the buffer cache if paging raw.
 *
 */

#include <sys/secdefines.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/param.h>
#include <sys/errno.h>
#include <sys/user.h>
#include <sys/proc.h>
#include <mach/vm_param.h>
#include <mach/vm_prot.h>
#include <mach/kern_return.h>
#include <mach/machine/vm_types.h>
#include <vm/vm_object.h>
#include <vm/vm_map.h>
#include <vm/vm_anon.h>
#if	SEC_BASE
#include <sys/security.h>
#include <sys/audit.h>
#include <sys/secpolicy.h>
#endif

#include <rt_pml.h>

#define SHM_REMOVED	02000

extern struct shminfo shminfo;	/* shared memory info structure */

extern struct shmid_internal	shmem[];	/* shared memory headers */
extern	struct timeval		time;		/* system idea of date */

lock_data_t			shm_attch_lock; /* protects attach array */

#if	SEC_ARCH
/*
 * Allocate space for the shared memory tag pools.  On systems that allocate
 * the shared memory structures dynamically, the tag pools should also be
 * dynamically allocated at the same time.
 */
extern tag_t		shmtag[];
#endif	
/*
 * Called by shared memory object deallocation handler when attach count
 reached zero.
 */

shm_free_entry(sp)
register struct shmid_internal  *sp;
{
        sp->shm_object = (vm_shm_object_t)VM_OBJECT_NULL;
        sp->s.shm_perm.mode = 0;
        sp->s.shm_segsz = 0;
        if (((int)(++(sp->s.shm_perm.seq) * shminfo.shmmni + (sp - shmem
))) < 0)
            sp->s.shm_perm.seq = 0;
}

/*
 * convert user supplied shmid into a ptr to the associated
 * shared memory table entry.
 */
struct shmid_internal *
shmconv(s)
register long	s;	/* shmid */
{
	register struct shmid_internal	*sp;	/* shared memory table entry */

	if (s < 0)
		return (NULL);

	sp = &shmem[s % shminfo.shmmni];

	if (!(sp->s.shm_perm.mode & IPC_ALLOC)
	    || s / shminfo.shmmni != sp->s.shm_perm.seq) {
		return(NULL);
	}

	return(sp);
}


/* 
 * shmat system call - attach a shared memory segment.
 */ 
/* ARGSUSED */
shmat(p, args, retval)
	struct proc *p;
	void *args;
	long *retval;
{
	register struct args { 		
		long	shmid;	/* shared mem id, returned from shmget call */
 		caddr_t	addr;	/* virtual address requested for attach */
 		long	flag;	/* valid flags: SHM_RND, SHM_RDONLY */
 	}	*uap = (struct args *)args;
 	register struct shmid_internal	*sp;	/* shared memory table entry */
 	register boolean_t		findspace;
	register vm_prot_t		prot;	/* protection */
 	register kern_return_t		result;	/* return value from vm_map() */
	vm_offset_t			addr;	/* virtual address */
	int				error;

	if ((sp = shmconv(uap->shmid)) == NULL)
		return(EINVAL);
	if (!sp->shm_object)
		return(EINVAL);
	if (sp->shm_object->so_refcnt <= sp->s.shm_nattch)
                return(EINVAL);

#if	SEC_ARCH
	if (sec_ipcaccess(&sp->s.shm_perm, SHMTAG(sp, 0), SP_READACC))
#else	
	if (ipcaccess(&sp->s.shm_perm, SHM_R))
#endif	
		return(EACCES);

	if ((uap->flag & SHM_RDONLY) == 0)
#if	SEC_ARCH
		if (sec_ipcaccess(&sp->s.shm_perm, SHMTAG(sp, 0), SP_WRITEACC))
#else	
		if (ipcaccess(&sp->s.shm_perm, SHM_W))
#endif
			return(EACCES);

	/*
	 * check we don't exceed per-process limit on attaches
	 */
	if (u.u_shmsegs >= shminfo.shmseg)
		return(EMFILE);

	/* 
	 * kernel chooses virtual address if 0 specified 
	 */
	if (uap->addr == 0) {
		addr = VM_MIN_ADDRESS;
		findspace = TRUE;
	} else {
		/* 
		 * round user-supplied address 
		 */
		addr = uap->flag & SHM_RND? (vm_offset_t)uap->addr & ~(SHMLBA - 1):
		 			    (vm_offset_t)uap->addr;
		findspace = FALSE;
	}

	/*
	 * set protection
	 */
	prot =  uap->flag & SHM_RDONLY? VM_PROT_READ: 
		                        VM_PROT_READ|VM_PROT_WRITE;

	/* 
	 * allocate a range in the current_task map, referring to the
	 * memory defined by the memory object 
         */

	vm_object_reference(sp->shm_object);
	result = u_map_enter(current_task()->map, &addr, 
			round_page(sp->s.shm_segsz), (vm_offset_t)0,
			findspace, sp->shm_object, (vm_offset_t)0, FALSE,
			prot, prot | VM_PROT_EXECUTE, VM_INHERIT_SHARE);


	if (result != KERN_SUCCESS) {
		switch (result) {

		      case KERN_NO_SPACE:
		      case KERN_MEMORY_FAILURE:
		      case KERN_MEMORY_ERROR:
			return(ENOMEM);

		      default:
			return(EINVAL);
		}
	} else (void)u_shm_attach(u.u_procp->p_pid, sp->shm_object, current_task()->map);

	u.u_shmsegs++;
	*retval = (long) addr;

	sp->s.shm_atime = time.tv_sec;
	sp->s.shm_lpid = u.u_procp->p_pid;
	sp->s.shm_nattch++;
	return(0);
}


/*
 * shmctl system call - perform shared memory control operation.
 */
/* ARGSUSED */
shmctl(p, args, retval)
	struct proc *p;
	void *args;
	long *retval;
{
	register struct args {
		long		shmid,	/* shared mem id, returned by shmget */
				cmd;	/* IPC_RMID, IPC_STAT, or IPC_SET */
		struct shmid_ds	*arg;  

	}	*uap = (struct args *)args;
	register struct shmid_internal	*sp;	/* shared memory table entry */
	struct shmid_ds			ds;	/* hold area for IPC_SET */
	int				error;
#if	SEC_ARCH
	dac_t				dac;
	int				ret;
#endif	

	if ((sp = shmconv(uap->shmid)) == NULL)
		return(EINVAL);

	switch(uap->cmd) {

	/* 
	 * Remove the shared memory identifier
	 */
	case IPC_RMID:
#if	SEC_BASE
		if (!sec_owner(sp->s.shm_perm.uid, sp->s.shm_perm.cuid))
			return(EPERM);
#else	
		if ((u.u_uid != sp->s.shm_perm.uid) && (u.u_uid != sp->s.shm_perm.cuid)
		    && (error = suser(u.u_cred, &u.u_acflag)))
			return(error);
#endif	

#if	SEC_ARCH
		if (sec_ipcaccess(&sp->s.shm_perm, SHMTAG(sp, 0), SP_DELETEACC))
			return(EACCES);
#endif

		/*
		 * decrement object ref_count so it will be removed
		 * now if there are no attached to it otherwise 
		 * after the last detach.
		 */
                vm_object_deallocate(sp->shm_object);

		sp->s.shm_perm.key = IPC_PRIVATE;
		sp->s.shm_perm.mode |= SHM_REMOVED; 

		break;

	/* 
	 * Set ownership and permissions
	 */
	case IPC_SET:
#if	SEC_BASE
		if (!sec_owner(sp->s.shm_perm.uid, sp->s.shm_perm.cuid))
			return(EPERM);
#else
		if ((u.u_uid != sp->s.shm_perm.uid) && (u.u_uid != sp->s.shm_perm.cuid)
		    && (error = suser(u.u_cred, &u.u_acflag)))
			return(error);
#endif

#if	SEC_ARCH
		if (sec_ipcaccess(&sp->s.shm_perm, SHMTAG(sp, 0), SP_SETATTRACC))
			return(EACCES);
#endif	

		if (copyin(uap->arg, &ds, sizeof(ds)))
			return(EFAULT);
#if	SEC_BASE
                if (!sec_owner_change_permitted(sp->s.shm_perm.uid,
                                sp->s.shm_perm.gid, ds.shm_perm.uid,
                                ds.shm_perm.gid))
                        return(EPERM);
#endif	
		sp->s.shm_perm.uid = ds.shm_perm.uid;
		sp->s.shm_perm.gid = ds.shm_perm.gid;
		sp->s.shm_perm.mode = (ds.shm_perm.mode & 0777) |
			(sp->s.shm_perm.mode & ~0777);
#if	SEC_ARCH
		dac.uid = sp->s.shm_perm.uid;
		dac.gid = sp->s.shm_perm.gid;
		dac.mode = sp->s.shm_perm.mode;
		ret = SP_CHANGE_OBJECT(SHMTAG(sp, 0), &dac,
				SEC_NEW_UID|SEC_NEW_GID|SEC_NEW_MODE);
		if (ret) {
			if (ret & SEC_NEW_UID)
				sp->s.shm_perm.uid = dac.uid;
			if (ret & SEC_NEW_GID)
				sp->s.shm_perm.gid = dac.gid;
			if (ret & SEC_NEW_MODE)
				sp->s.shm_perm.mode =
					(sp->s.shm_perm.mode & ~0777)
						 | (dac.mode & 0777);
		}
#endif
		sp->s.shm_ctime = time.tv_sec;
		break;

	/*
	 * Get shared memory data structure
	 */
 	case IPC_STAT:

#if	SEC_ARCH
		if (sec_ipcaccess(&sp->s.shm_perm, SHMTAG(sp, 0), SP_READACC))
#else	
		if (ipcaccess(&sp->s.shm_perm, SHM_R))
#endif
			return(EACCES);

		if (copyout(&sp->s, uap->arg, sizeof(sp->s)))
			return(EFAULT);

		break;

	default:
		return(EINVAL);
	}
	return(0);
}



/*
 * shmdt system call - detach a shared memory segment.
 */
/* ARGSUSED */
shmdt(p, args, retval)
	struct proc *p;
	void *args;
	long *retval;
{
	register struct args {
		caddr_t	addr;	/* virtual address of attached mem, from shmat call */
	} *uap = (struct args *)args;
	register struct shmid_internal *sp;	/* shared memory table entry */
	vm_map_entry_t 		map_entry;	/* map containing addr */
	struct vm_object	*object;	/* object */

	if( (vm_offset_t) uap->addr < VM_MIN_ADDRESS )
		return(EINVAL);
	/* 
	 * look for a map with the given starting address 
	 */

	if (!vm_map_lookup_entry(current_task()->map, uap->addr, &map_entry)) 
		return(EINVAL);		

	if ((vm_offset_t)map_entry->vme_start != (vm_offset_t)uap->addr)
		return(EINVAL);

	/* 
	 * find the shmem table entry whose object
	 * matches the one from the found map.
	 */
	
	for (sp = shmem; sp < &shmem[shminfo.shmmni]; sp++)
		if (sp->shm_object == (vm_shm_object_t)map_entry->vme_object)
			break;

	if (sp >= &shmem[shminfo.shmmni])
		/* the shmem table entry has already been removed */
		return(EINVAL);


	/* deallocate the address range in the current_task map */
	if ((vm_deallocate(current_task()->map, uap->addr, 
			   sp->s.shm_segsz)) != KERN_SUCCESS)
		return(EINVAL);

	sp->s.shm_dtime = time.tv_sec;
	sp->s.shm_lpid = u.u_procp->p_pid;

	assert(u.u_shmsegs > 0);

	return(0);
}


/*
 * shmget system call - get a shared memory segment.
 */
/* ARGSUSED */
shmget(p, args, retval)
	struct proc *p;
	void *args;
	long *retval;
{
	register struct args {
		long	key;	/* key to identify the shared memory segment */
		u_long	size,	/* minimum bytes to allocate */
			shmflg;	/* valid flags: IPC_CREAT, IPC_EXCL, access modes */
	}	*uap = (struct args *)args;
	struct shmid_internal		*sp;	/* shared memory table entry */
	int				s;	/* ipcget status */
	int				error;

	if (error = ipcget((key_t)uap->key, uap->shmflg, shmem, shminfo.shmmni, 
			 (long)sizeof(*sp), &s, (struct ipc_perm **)&sp))
		return(error);

	if (s) {
		vm_shm_object_t object;

		/*
		 * This is a new shared memory segment - init shmem table entry
		 */

		/*
		 * verify size is between system min and max 
		 */
		if (uap->size < shminfo.shmmin || uap->size > shminfo.shmmax) {
			sp->s.shm_perm.mode = 0;
			return(EINVAL);
		}

		if (u_shm_allocate(round_page(uap->size), &object)) {
			shm_free_entry(sp);
			return ENOMEM;
		}
		object->so_sp = sp;
		sp->shm_object = object;
		sp->s.shm_segsz = uap->size;
		sp->s.shm_ctime = time.tv_sec;
		sp->s.shm_cpid = u.u_procp->p_pid;
		sp->s.shm_atime = sp->s.shm_dtime = 0;
		sp->s.shm_lpid = 0;
		sp->s.shm_nattch = 0;
#if	SEC_ARCH
		sec_svipc_object_create(SHMTAG(sp, 0));
#endif	
	} else {
		/*
		 * found an existing segment - verify the size
		 */
		if (uap->size && (uap->size > sp->s.shm_segsz))
			return(EINVAL);
	}

	*retval = sp->s.shm_perm.seq * shminfo.shmmni + (sp - shmem);
	return(0);
}

/*
 * SAR: calculate the number of shared memory file table entries in use.
 */
long
svr4_shmem_cnt()
{
	register struct shmid_internal *sp;	/* shared memory table entry */
	long shmem_tbl_cnt;		 /* # of shared memory table entries */

	shmem_tbl_cnt = 0;

	for (sp = shmem; sp < &shmem[shminfo.shmmni]; sp++)
		if (sp->shm_object) 
			shmem_tbl_cnt++;
	return(shmem_tbl_cnt);
}

shminit()
{
	lock_init(&shm_attch_lock, TRUE);
}
