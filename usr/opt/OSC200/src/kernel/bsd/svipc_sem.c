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
static char *rcsid = "@(#)$RCSfile: svipc_sem.c,v $ $Revision: 4.3.11.5 $ (DEC) $Date: 1993/09/02 12:28:24 $";
#endif
/* 
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0
 */
/*
 * Modification History
 *
 * 27-Oct-91	Fred Canter
 *	Make System V IPC definitions configurable.
 *
 */

/*
 * 	Inter-Process Communication Semaphore Facility. 
 */

#include <sys/secdefines.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/types.h>
#include <sys/param.h>
#include <sys/errno.h>
#include <sys/signal.h>
#include <sys/user.h>
#include <sys/proc.h>
#include <sys/uio.h>
#include <kern/kalloc.h>
#include <kern/zalloc.h>
#include <kern/macro_help.h>
#if	SEC_BASE
#include <sys/security.h>
#include <sys/audit.h>
#include <sys/secpolicy.h>
#endif

struct zone	*semundo_zone;		/* zone for semaphores undo structs */
struct zone	*undo_entry_zone;	/* zone for semaphores undo entries */
extern struct semid_ds sema[];		/* semaphore data structures */
extern struct sem      sem[];		/* semaphores */
struct sem_undo *semundo_head;		/* ptr to head of undo chaing for each
					   process in the system */
extern struct seminfo seminfo;		/* semaphore information structure */

extern union {
	u_short		semvals[SEMMSL];	/* set semaphore values */
	struct semid_ds ds;			/* set permission values */
	struct sembuf   semops[SEMOPM];		/* operation holding area */
} semtmp;

#if	SEC_ARCH
/*
 * Allocate space for the semaphore tag pools.  On systems that allocate
 * the semaphore structures dynamically, the tag pools should also be
 * dynamically allocated at the same time as the semaphores.
 */
extern tag_t		semtag[];
#endif


extern struct timeval   time;	/* system idea of date */

/*
 * find the undo struct in the list and remove it 
 */
#define UNLINK_PROC_UNDO(undop)			\
MACRO_BEGIN					\
    register struct sem_undo	*up;		\
						\
    for (up = semundo_head; up != NULL; up = up->un_nextp)\
        if (up->un_nextp == undop)		\
        {					\
	    up->un_nextp = undop->un_nextp;	\
	    break;				\
        }					\
MACRO_END

/*
 * point to next undo entry and free the current entry 
 */
#define REMOVE_UNDO_ENTRY(uentp)		\
MACRO_BEGIN					\
    struct undo_entry *rmuentp;			\
    rmuentp = uentp;				\
    uentp = uentp->uent_nextp;			\
    zfree(undo_entry_zone, (vm_offset_t) rmuentp);\
MACRO_END

/*
 * reverse semaphore operations already done this system call, 
 * return error if specified not to sleep, increment # waiting, and sleep 
 */
#define WAIT_FOR_SEM(awaitmember, pri)				\
MACRO_BEGIN							\
    if (i)							\
	semundo(semtmp.semops, i, uap->semid, sp);		\
    if (op->sem_flg & IPC_NOWAIT)				\
	return(EAGAIN);						\
    semp->awaitmember++;					\
    if (tsleep(&semp->awaitmember, PCATCH | pri, "sv_sem", 0))	\
    {								\
	if ((semp->awaitmember)-- <= 1)				\
	{							\
	    semp->awaitmember = 0;				\
	    wakeup(&semp->awaitmember);				\
	}							\
	return(EINTR);						\
    }								\
MACRO_END

/*
 *	semaoe - create or update adjust on exit entry 
 *		 (returns error on error, otherwise 0)
 */

semaoe(val, id, num)
long           val;	/* operation value to be adjusted on exit */
long           num;	/* semaphore # */
long           id;	/* semid */
{
	register struct undo_entry 	*uentp;		/* ptr to entry to update */
	register struct sem_undo 	*undop;		/* ptr to process undo struct */
	register struct undo_entry 	*new_uentp;	/* new entry to be added */
	register struct undo_entry 	*puentp;	/* ptr to predecessor to entry */
	register int    		found;		/* matching entry found flag */

	if (val == 0)
		return (0);

	if (val > seminfo.semaem || val < -seminfo.semaem)
		return (ERANGE);

	/* if there is no undo structure for this process, allocate one */
	if ((undop = u.u_semundo) == NULL) {
		if (undop = (struct sem_undo *) zalloc(semundo_zone)) {
			undop->un_nextp = NULL;
			undop->un_entp = NULL;
			undop->un_cnt = 0;		       
			u.u_semundo = undop;
		} 
		else
			return (ENOSPC);
	}

	/*
	 * search the process undo structures for one with the same semaphore 
	 * ID and num as the semop operation. 
	 * (undo entries are ordered with ascending semaphore ids and numbers) 
	 */
	found = FALSE;
	puentp = NULL;
	uentp = undop->un_entp;

	while (uentp != NULL) {
		if (uentp->uent_id < id
		    || ((uentp->uent_id == id) && (uentp->uent_num < num))) {
			puentp = uentp;
			uentp = uentp->uent_nextp;
			continue;
		}

		if (uentp->uent_id == id && uentp->uent_num == num)
			found = TRUE;

		break;
	}

	/* if there is no matching undo entry, add one */
	if (!found) {
		if (undop->un_cnt < seminfo.semume)
			new_uentp = (struct undo_entry *) zalloc(undo_entry_zone);
		else
			return (EINVAL);

		/* set up undo values and make adjustment */
		new_uentp->uent_id = id;
		new_uentp->uent_num = num;
		new_uentp->uent_aoe = -val;

		undop->un_cnt++;

		/* insert new entry at appropriate place in the list */

		if (undop->un_entp == NULL) {
			/* first entry for this process */
			undop->un_entp = new_uentp;
			new_uentp->uent_nextp = NULL;

			/* add process to beginning of undo list */
			undop->un_nextp = semundo_head;
			semundo_head = undop;
		} 
		else if (puentp == NULL) {
			/* put new entry at head of entry list */
			new_uentp->uent_nextp = undop->un_entp;
			undop->un_entp = new_uentp;
		} 
		else {
			new_uentp->uent_nextp = uentp;
			puentp->uent_nextp = new_uentp;
		}

		return (0);
	}  /* if (!found) */

	/*
	 * found a matching entry: 
	 * subtract value of semaphore operation from adjustment val 
	 */
	uentp->uent_aoe -= val;

	/*
	 * if the adjustment value is out of max range, 
	 * undo the last adjustment and error return 
	 */
	if (uentp->uent_aoe > seminfo.semaem || uentp->uent_aoe < -seminfo.semaem) {
		uentp->uent_aoe += val;
		return (ERANGE);
	}

	/* if the adjustment value is 0, remove the undo entry */
	if (uentp->uent_aoe == 0) {
		/* free the entry */
		if (puentp != NULL)
			puentp->uent_nextp = uentp->uent_nextp;
		else
			undop->un_entp = uentp->uent_nextp;

		zfree(undo_entry_zone, (vm_offset_t) uentp);

		undop->un_cnt--;

		/*
		 * if there are no undo entries, 
		 * remove process (undop) from undo list 
		 */
		if (undop->un_entp == NULL) {
			if (semundo_head == undop)
				semundo_head = undop->un_nextp;
			else
				UNLINK_PROC_UNDO(undop);
		}
	}

	return (0);
}

/*
 *	semconv - convert user supplied semid into a ptr to the associated
 *		  semaphore header 
 */

struct semid_ds *
semconv(s)
register long    s;	/* semaphore id */
{
	register struct semid_ds *sp;	/* ptr to associated header */

	if (s < 0)
		return (NULL);

	sp = &sema[s % seminfo.semmni];

	if ((sp->sem_perm.mode & IPC_ALLOC) == 0 ||
	    s / seminfo.semmni != sp->sem_perm.seq) {
		return (NULL);
	}

	return (sp);
}

/*
 *	semctl - semctl system call: perform semaphore control operations 
 */

/* ARGSUSED */
semctl(p, args, retval)
	struct proc *p;
	void *args;
	long *retval;
{
	register struct args {
		long             semid;
		long		semnum;
		long             cmd;
		long             arg;
	}	*uap = (struct args *) args;

	register struct semid_ds	*sp;		/* ptr to semaphore header */
	register struct sem 		*semp;		/* ptr to semaphore */
	register int    		i;		/* loop control */
	register int    		movesize;	/* size of semaphore values to move */
	struct uio      		suio;		/* I/O info */
	struct iovec    		siov;		/* I/O vectors */
	int				error;
#if	SEC_ARCH
	dac_t				dac;
	int				ret;
#endif	
#if	SEC_ILB
	tag_t				ntag;
#endif	


	if ((sp = semconv(uap->semid)) == NULL)
		return(EINVAL);

	*retval = 0;

	switch (uap->cmd) {
	/* remove semaphore set */
	case IPC_RMID:
#if	SEC_BASE
		if (!sec_owner(sp->sem_perm.uid, sp->sem_perm.cuid))
			return(EPERM);
#else	
		if (u.u_uid != sp->sem_perm.uid && u.u_uid != sp->sem_perm.cuid
		    && (error = suser(u.u_cred, &u.u_acflag)))
			return(error);
#endif	

#if	SEC_ARCH
		if (sec_ipcaccess(&sp->sem_perm, SEMTAG(sp, 0), SP_DELETEACC))
			return(EACCES);
#endif	
		/* clear all the undo entries in this set */
		semunrm(uap->semid, 0, sp->sem_nsems);

		for (i = sp->sem_nsems, semp = sp->sem_base; i--; semp++) {
			semp->semval = semp->sempid = 0;
			if (semp->semncnt) {
				wakeup(&semp->semncnt);
				semp->semncnt = 0;
			}

			if (semp->semzcnt) {
				wakeup(&semp->semzcnt);
				semp->semzcnt = 0;
			}
		}

		kfree(sp->sem_base, sp->sem_nsems * sizeof(struct sem));

		if (uap->semid + seminfo.semmni < 0)
			sp->sem_perm.seq = 0;
		else
			sp->sem_perm.seq++;

		sp->sem_perm.mode = 0;

		return(0);

	/* set ownership and permissions */
	case IPC_SET:
#if	SEC_BASE
		if (!sec_owner(sp->sem_perm.uid, sp->sem_perm.cuid))
			return(EPERM);
#else	
		if (u.u_uid != sp->sem_perm.uid && u.u_uid != sp->sem_perm.cuid
		    && (error = suser(u.u_cred, &u.u_acflag)))
			return(error);
#endif	

#if	SEC_ARCH
		if (sec_ipcaccess(&sp->sem_perm, SEMTAG(sp, 0), SP_SETATTRACC))
			return(EACCES);
#endif

		if (copyin(uap->arg, &semtmp.ds, sizeof(semtmp.ds)))
			return(EFAULT);
#if	SEC_BASE
                if (!sec_owner_change_permitted(sp->sem_perm.uid,
                                sp->sem_perm.gid, semtmp.ds.sem_perm.uid,
                                semtmp.ds.sem_perm.gid))
                        return(EPERM);
#endif	

		sp->sem_perm.uid = semtmp.ds.sem_perm.uid;
		sp->sem_perm.gid = semtmp.ds.sem_perm.gid;
		sp->sem_perm.mode = semtmp.ds.sem_perm.mode & 0777 | IPC_ALLOC;
#if	SEC_ARCH
		dac.uid = sp->sem_perm.uid;
		dac.gid = sp->sem_perm.gid;
		dac.mode = sp->sem_perm.mode;
		ret = SP_CHANGE_OBJECT(SEMTAG(sp, 0), &dac,
				SEC_NEW_UID|SEC_NEW_GID|SEC_NEW_MODE);
		if (ret) {
			if (ret & SEC_NEW_UID)
				sp->sem_perm.uid = dac.uid;
			if (ret & SEC_NEW_GID)
				sp->sem_perm.gid = dac.gid;
			if (ret & SEC_NEW_MODE)
				sp->sem_perm.mode = (sp->sem_perm.mode & ~0777)
						  | (dac.mode & 0777);
		}
#endif	
		sp->sem_ctime = time.tv_sec;

		return(0);

	/* get semaphore data structure */
	case IPC_STAT:
#if	SEC_ARCH
                if (sec_ipcaccess(&sp->sem_perm, SEMTAG(sp, 0), SP_READACC))
#else	
		if (ipcaccess(&sp->sem_perm, SEM_R))
#endif
			return(EACCES);

		if (copyout(sp, uap->arg, sizeof(*sp)))
			return(EFAULT);

		return(0);

	/* get # of processes sleeping for greater semval */
	case GETNCNT:
#if	SEC_ARCH
		if (sec_ipcaccess(&sp->sem_perm, SEMTAG(sp, 0), SP_READACC))
#else	
		if (ipcaccess(&sp->sem_perm, SEM_R))
#endif
			return(EACCES);

		if (uap->semnum >= sp->sem_nsems || uap->semnum < 0)
			return(EINVAL);

		*retval = (sp->sem_base + uap->semnum)->semncnt;
		return(0);

	/* get pid of last process to operate on semaphore */
	case GETPID:
#if	SEC_ARCH
		if (sec_ipcaccess(&sp->sem_perm, SEMTAG(sp, 0), SP_READACC))
#else	
		if (ipcaccess(&sp->sem_perm, SEM_R))
#endif
			return(EACCES);

		if (uap->semnum >= sp->sem_nsems || uap->semnum < 0)
			return(EINVAL);

		*retval = (sp->sem_base + uap->semnum)->sempid;
		return(0);

	/* get semval of one semaphore */
	case GETVAL:

#if	SEC_ARCH
                if (sec_ipcaccess(&sp->sem_perm, SEMTAG(sp, 0), SP_READACC))
#else	
		if (ipcaccess(&sp->sem_perm, SEM_R))
#endif	
			return(EACCES);

		if (uap->semnum >= sp->sem_nsems || uap->semnum < 0)
			return(EINVAL);
#if	SEC_ILB
		if (SP_CHECK_FLOAT(UIO_READ, SEMTAG(sp, 0), &ntag)) {
			error = u.u_error;
			return(error);
		}
		SP_DO_FLOAT(UIO_READ, SEMTAG(sp, 0), &ntag);
#endif	

		*retval = (sp->sem_base + uap->semnum)->semval;
		return(0);

	/* get all semvals in set */
	case GETALL:
#if	SEC_ARCH
		if (sec_ipcaccess(&sp->sem_perm, SEMTAG(sp, 0), SP_READACC))
#else	
		if (ipcaccess(&sp->sem_perm, SEM_R))
#endif	
			return(EACCES);
#if	SEC_ILB
		if (SP_CHECK_FLOAT(UIO_READ, SEMTAG(sp, 0), &ntag)) {
			error = u.u_error;
			return(error);
		}
#endif	
		suio.uio_iov = &siov;
		suio.uio_iovcnt = 1;
		suio.uio_offset = 0;
		suio.uio_segflg = UIO_USERSPACE;
		siov.iov_base = (caddr_t) uap->arg;

		for (i = sp->sem_nsems, semp = sp->sem_base; i--; semp++) {
			siov.iov_len = suio.uio_resid = sizeof(semp->semval);
			suio.uio_rw = UIO_READ;
			if (error = uiomove((caddr_t) & semp->semval, sizeof(semp->semval), &suio))
#if	SEC_ILB
			{
				if (i != sp->sem_nsems)
					SP_DO_FLOAT(UIO_READ, SEMTAG(sp, 0), &ntag);
				return(error);
			}
#else	
				return(error);
#endif
		}
#if	SEC_ILB
		SP_DO_FLOAT(UIO_READ, SEMTAG(sp, 0), &ntag);
#endif	

		return(0);

	/* get # of processes sleeping for semval to become zero */
	case GETZCNT:
#if	SEC_ARCH
		if (sec_ipcaccess(&sp->sem_perm, SEMTAG(sp, 0), SP_READACC))
#else
		if (ipcaccess(&sp->sem_perm, SEM_R))
#endif
			return(EACCES);

		if (uap->semnum >= sp->sem_nsems || uap->semnum < 0)
			return(EINVAL);

		*retval = (sp->sem_base + uap->semnum)->semzcnt;
		return(0);

	/* set semval of one semaphore */
	case SETVAL:
#if	SEC_ARCH
		if (sec_ipcaccess(&sp->sem_perm, SEMTAG(sp, 0), SP_WRITEACC))
#else
		if (ipcaccess(&sp->sem_perm, SEM_A))
#endif
			return(EACCES);

		if (uap->semnum >= sp->sem_nsems || uap->semnum < 0)
			return(EINVAL);

                /* 
		 * verify that the semaphore value is not greater than the
		 * system defined maximum nor < 0
		 */
		if ((unsigned) uap->arg > seminfo.semvmx)
			return(ERANGE);
#if	SEC_ILB
                if (SP_CHECK_FLOAT(UIO_WRITE, SEMTAG(sp, 0), &ntag)) {
			error = u.u_error;
                        return(error);
		}
		SP_DO_FLOAT(UIO_WRITE, SEMTAG(sp, 0), &ntag);
#endif	

		if ((semp = sp->sem_base + uap->semnum)->semval = uap->arg) {
			if (semp->semncnt) {
				semp->semncnt = 0;
				wakeup(&semp->semncnt);
			}
		} 
		else if (semp->semzcnt) {
			semp->semzcnt = 0;
			wakeup(&semp->semzcnt);
		}

		semp->sempid = u.u_procp->p_pid;

		/* clear the undo entries for this semaphore */
		semunrm(uap->semid, uap->semnum, uap->semnum);

		return(0);

	/* set semvals of all semaphores in set */
	case SETALL:
#if	SEC_ARCH
		if (sec_ipcaccess(&sp->sem_perm, SEMTAG(sp, 0), SP_WRITEACC))
#else
		if (ipcaccess(&sp->sem_perm, SEM_A))
#endif	
			return(EACCES);
#if	SEC_ILB
		if (SP_CHECK_FLOAT(UIO_WRITE, SEMTAG(sp, 0), &ntag)) {
			error = u.u_error;
			return(error);
		}
#endif

		movesize = sizeof(semp->semval) * sp->sem_nsems;

		suio.uio_iov = &siov;
		suio.uio_iovcnt = 1;
		suio.uio_offset = 0;
		suio.uio_segflg = UIO_USERSPACE;
		suio.uio_resid = siov.iov_len = movesize;
		siov.iov_base = (caddr_t) uap->arg;
		suio.uio_rw = UIO_WRITE;
		if (error = uiomove((caddr_t) semtmp.semvals, movesize, &suio))
			return(error);

		for (i = 0; i < sp->sem_nsems;)
			if (semtmp.semvals[i++] > seminfo.semvmx)
				return(ERANGE);

		/* clear all the undo entries in this set */
		semunrm(uap->semid, 0, sp->sem_nsems);

		for (i = 0, semp = sp->sem_base; i < sp->sem_nsems;
		     (semp++)->sempid = u.u_procp->p_pid) {
			if (semp->semval = semtmp.semvals[i++]) {
				if (semp->semncnt) {
					semp->semncnt = 0;
					wakeup(&semp->semncnt);
				}
			} 
			else if (semp->semzcnt) {
				semp->semzcnt = 0;
				wakeup(&semp->semzcnt);
			}
		}
#if	SEC_ILB
		SP_DO_FLOAT(UIO_WRITE, SEMTAG(sp, 0), &ntag);
#endif	

		return(0);
	default:
		return(EINVAL);
	}
}

/*
 *	semexit - called by exit (kern_exit.c) to clean up on process exit 
 */

semexit()
{
	register struct undo_entry 	*uentp;	/* undo entry ptr */
	register struct sem_undo 	*undop;	/* process undo struct ptr */
	register long   		val;	/* adjusted value */
	register struct sem 		*semp;	/* semaphore ptr */
	register struct semid_ds 	*sp;	/* semid being undone ptr */
	int				error;


	if ((undop = u.u_semundo) == NULL)
		return;

	if (undop->un_entp == NULL)
		goto cleanup;

	/* for all undo entries, adjust semaphore value and remove entry */
	uentp = undop->un_entp;
	while (uentp != NULL) {
		if ((sp = semconv(uentp->uent_id)) == NULL) {
			REMOVE_UNDO_ENTRY(uentp);
			continue;
		}

		val = (long) (semp = sp->sem_base + uentp->uent_num)->semval
			+ uentp->uent_aoe;

		if ((val < 0) || (val > seminfo.semvmx)) {
			REMOVE_UNDO_ENTRY(uentp);
			continue;
		}

		semp->semval = val;

		if (val == 0 && semp->semzcnt) {
			semp->semzcnt = 0;
			wakeup(&semp->semzcnt);
		}

		if (uentp->uent_aoe > 0 && semp->semncnt) {
			semp->semncnt = 0;
			wakeup(&semp->semncnt);
		}

		REMOVE_UNDO_ENTRY(uentp);
	}  /* while (uentp != NULL) */

	/* remove this process undo structure from the list */
	if (semundo_head == undop)
		semundo_head = undop->un_nextp;
	else
		UNLINK_PROC_UNDO(undop);

cleanup:
	/* free and clear the undo struct for this process */
	zfree(semundo_zone, (vm_offset_t) undop);
	u.u_semundo = NULL;
}

/*
 *	semget - semget system call: creates an array of semaphores 
 */

/* ARGSUSED */
semget(p, args, retval)
	struct proc *p;
	void *args;
	long *retval;
{
	register struct args {
		long             key;
		long             nsems;
		long             semflg;
	}	*uap = (struct args *) args;
	struct semid_ds *sp;	/* semaphore header ptr */
	int             s;	/* ipcget status return */
	int		error;

	if (error = ipcget((key_t)uap->key, uap->semflg, sema, seminfo.semmni,
			   (long)sizeof(*sp), &s, (struct ipc_perm **)&sp))
		return(error);

	if (s) {
		/* this is a new semaphore set; finish initialization */
		if (uap->nsems <= 0 || uap->nsems > seminfo.semmsl) {
			sp->sem_perm.mode = 0;
			return(EINVAL);
		}
		/* allocate semaphore array of specified size and initialize */
		sp->sem_base = (struct sem *) kalloc(uap->nsems * sizeof(struct sem));
		bzero((caddr_t)sp->sem_base, uap->nsems * sizeof(struct sem));
		sp->sem_nsems = uap->nsems;
		sp->sem_ctime = time.tv_sec;
		sp->sem_otime = 0;

#if	SEC_ARCH
		sec_svipc_object_create(SEMTAG(sp, 0));
#endif
#if	SEC_ILB
		sp_init_obj_bits(SEMTAG(sp, 0));
#endif	
	} 
	else if (uap->nsems < 0 || (uap->nsems && sp->sem_nsems < uap->nsems))
		return(EINVAL);

	*retval = sp->sem_perm.seq * seminfo.semmni + (sp - sema);
	return(0);
}

/*
 *	seminit - called by main (init_main.c) to initialize the semaphore map 
 */

seminit()
{
	semundo_zone = zinit(sizeof(struct sem_undo),
			     nproc * sizeof(struct sem_undo),
		       10 * sizeof(struct sem_undo), "semundo zone");

	/* make zone exhaustible */
	zchange(semundo_zone, FALSE, FALSE, TRUE, FALSE);

	undo_entry_zone = zinit(sizeof(struct undo_entry),
				seminfo.semume * sizeof(struct undo_entry),
				10 * sizeof(struct undo_entry),
				"undo entry zone");
}

/*
 *	semop - semop system call: manipulate semaphores 
 */

/* ARGSUSED */
semop(p, args, retval)
	struct proc *p;
	void *args;
	long *retval;
{
	register struct args {
		long             semid;	/* semaphore descriptor from semget() */
		struct sembuf  *sops;	/* array of semaphore operations */
		u_long           nsops;	/* number of operations */
	}	*uap = (struct args *) args;
	register struct sembuf 		*op;	/* ptr to operation */
	register struct semid_ds 	*sp;	/* ptr to associated header */
	register struct sem 		*semp;	/* ptr to semaphore */
	register int    		i;	/* loop control */
	register int    		opslen;	/* size of operation buffers */
	register int    		again;
	struct uio      		suio;	/* I/O info */
	struct iovec    		siov;	/* I/O vectors */
	int				error = 0;
#if	SEC_ILB
	tag_t				ntag;
	int				need_to_float = 0;
#endif	

	if ((sp = semconv(uap->semid)) == NULL)
		return(EINVAL);
	if (uap->nsops > seminfo.semopm)
		return(E2BIG);
	opslen = uap->nsops * sizeof(*op);

	/* read array of semaphore operations from user to kernel space */
	suio.uio_iov = &siov;
	suio.uio_iovcnt = 1;
	suio.uio_offset = 0;
	suio.uio_segflg = UIO_USERSPACE;
	suio.uio_resid = siov.iov_len = opslen;
	siov.iov_base = (caddr_t) uap->sops;
	suio.uio_rw = UIO_WRITE;
	if (error = uiomove((caddr_t) semtmp.semops, opslen, &suio))
		return(error);

	/*
	 * check permissions for all semaphore operations 
	 * and verify that sem numbers are in range and permissions are granted 
	 */
	for (i = 0, op = semtmp.semops; i++ < uap->nsops; op++) {
#if	SEC_ARCH
		if (sec_ipcaccess(&sp->sem_perm, SEMTAG(sp, 0),
					op->sem_op ? SP_WRITEACC : SP_READACC))
#else	
		if (ipcaccess(&sp->sem_perm, op->sem_op ? SEM_A : SEM_R))
#endif
			return(EACCES);

#if	SEC_ILB
		/*
		 * If any sem_op is non-zero, consider it a write operation
		 * that must be checked for a float.
		 */
		if (op->sem_op)
			need_to_float = 1;
#endif	
		if (op->sem_num >= sp->sem_nsems)
			return(EFBIG);
	}

	again = FALSE;

check:

	/*
	 * loop waiting for the operations to be satisified atomically. 
	 * (do the operations and undo them if a wait is needed 
	 *  or an error is detected) 
	 */
	if (again) {
		/* verify that the semaphores haven't been removed */
		if (semconv(uap->semid) == NULL)
			return(EIDRM);

		/* copy in user semaphore operation list after sleep */
		suio.uio_iov = &siov;
		suio.uio_iovcnt = 1;
		suio.uio_offset = 0;
		suio.uio_segflg = UIO_USERSPACE;
		suio.uio_resid = siov.iov_len = opslen;
		siov.iov_base = (caddr_t) uap->sops;
		suio.uio_rw = UIO_WRITE;
		if (error = uiomove((caddr_t) semtmp.semops, opslen, &suio))
			return(error);
	}
	again = TRUE;
#if	SEC_ILB
	if (need_to_float) {
		if (SP_CHECK_FLOAT(UIO_WRITE, SEMTAG(sp, 0), &ntag)) {
			error = u.u_error;
			return(error);
		}
		SP_DO_FLOAT(UIO_WRITE, SEMTAG(sp, 0), &ntag);
        }
#endif

	for (i = 0, op = semtmp.semops; i < uap->nsops; i++, op++) {
		semp = sp->sem_base + op->sem_num;

		if (op->sem_op > 0) {
			/* positive semaphore operation */
			if (op->sem_op + (long) semp->semval > seminfo.semvmx ||
			    ((op->sem_flg & SEM_UNDO) &&
			     (error = semaoe(op->sem_op, 
					    uap->semid, op->sem_num)))) {
				if (error == 0)
					error = ERANGE;
				/*
				 * UNDO flag is set on semaphore operation,
				 * update process undo structure 
				 */
				if (i)
					semundo(semtmp.semops, i, 
						uap->semid, sp);
				return(error);
			}

			/* add semaphore operation to semaphore value */
			semp->semval += op->sem_op;

			/* wake semaphores waiting for value to increase */
			if (semp->semncnt) {
				semp->semncnt = 0;
				wakeup(&semp->semncnt);
			}

			continue;
		}   /* if (op->sem_op > 0) */

		if (op->sem_op < 0) {
			/* negative semaphore operation */
			if (semp->semval >= -op->sem_op) {
				if (op->sem_flg & SEM_UNDO &&
				    (error = semaoe(op->sem_op, 
						uap->semid, op->sem_num))) {
					/*
					 * UNDO flag is set on semaphore 
					 * operation, update process 
					 * undo structure 
					 */
					if (i)
						semundo(semtmp.semops, i, 
							uap->semid, sp);
					return(error);
				}

				/* add operation to semaphore value */
				semp->semval += op->sem_op;

				if (semp->semval == 0 && semp->semzcnt) {
					/*
					 * semapore value is 0, awaken
					 * waiting processes 
					 */
					semp->semzcnt = 0;
					wakeup(&semp->semzcnt);
				}

				continue;
			} 
			else {  
				/* operation + semaphore value < 0.
				 * reverse all semaphore ops done this call 
				 * and wait for value to increase 
				 * (returns on error) 
				 */
				WAIT_FOR_SEM(semncnt, PSEMN);
				goto check;
			}
		}  /* if (op->sem_op < 0) */

		/* semaphore operation is 0 */
		if (semp->semval) {
			/*
			 * reverse all semaphore ops done this call 
			 * and wait for value to increase 
			 * (returns on error) 
			 */
			WAIT_FOR_SEM(semzcnt, PSEMZ);
			goto check;
		}
	}  /* for */

	/*
	 * all operations succeeded: 
	 * set pid of last operation for accessed semaphores 
	 */
	for (i = 0, op = semtmp.semops; i++ < uap->nsops;
	     (sp->sem_base + (op++)->sem_num)->sempid = u.u_procp->p_pid);

	sp->sem_otime = time.tv_sec;
	*retval = 0;
        /* global table() system call counter (see sem.h) */
        seminfo.sema++;
	return(0);
}

/*
 * 	semundo - Undo work done up to finding an operation that can't be done. 
 */

semundo(op, n, id, sp)
register struct sembuf		*op;	/* first operation that was done ptr */
register long    		n;	/* # of operations that were done */
register long			id;	/* semaphore id */
register struct semid_ds 	*sp;	/* semaphore data structure ptr */
{
	register struct sem 		*semp;	/* semaphore ptr */

	for (op += n - 1; n--; op--) {
		if (op->sem_op == 0)
			continue;
		semp = sp->sem_base + op->sem_num;
		semp->semval -= op->sem_op;

		if (op->sem_flg & SEM_UNDO)
			/* generate the undo entry for this operation */
			(void) semaoe(-op->sem_op, id, op->sem_num);
	}
}

/*
 * 	semunrm - undo entry remover 
 *
 *	clears all undo entries for a set of semaphores that are being 
 * 	removed from the system or are being reset by SETVAL or 
 *	SETVALS commands to semctl 
 */

semunrm(id, low, high)
long		id;	/* semid */
u_long		low;	/* lowest semaphore being changed */
u_long		high;	/* highest semaphore being changed */
{
	register struct undo_entry 	*uentp;		/* ptr to undo entry */
	register struct sem_undo 	*undop;		/* ptr to process undo struct */
	register struct undo_entry 	*puentp;	/* ptr to predecessor to uentp */
	register struct sem_undo 	*pundop;	/* ptr to predecessor to undop */
	register struct undo_entry 	*rmuentp;	/* ptr to uentp to be removed */


	pundop = NULL;
	undop = semundo_head;

	while (undop != NULL) {
		/* search through undo entries for matching entries */
		puentp = NULL;
		uentp = undop->un_entp;

		while ((uentp != NULL) &&
		       (uentp->uent_id <= id) && (uentp->uent_num <= high)) {
			if ((uentp->uent_id == id) && (uentp->uent_num >= low)) {

				/* a match, remove it */
				rmuentp = uentp;

				if (puentp != NULL)
					uentp = puentp->uent_nextp = uentp->uent_nextp;
				else
					uentp = undop->un_entp = uentp->uent_nextp;

				zfree(undo_entry_zone, (vm_offset_t) rmuentp);
				undop->un_cnt--;
			} 
			else {
				puentp = uentp;
				uentp = uentp->uent_nextp;
			}
		}

		/*
		 * if there are now no entries for this undo struct, 
		 * remove the undo struct from the linked list 
		 */
		if (undop->un_entp == NULL) {
			if (pundop != NULL)
				undop = pundop->un_nextp = undop->un_nextp;
			else
				undop = semundo_head = undop->un_nextp;
		} 
		else {
			pundop = undop;
			undop = undop->un_nextp;
		}
	}  /* while (undop != NULL) */
}
