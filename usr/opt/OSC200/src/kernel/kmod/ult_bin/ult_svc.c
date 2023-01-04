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
static char *rcsid = "@(#)$RCSfile: ult_svc.c,v $ $Revision: 1.1.9.2 $ (DEC) $Date: 1993/04/01 20:05:27 $";
#endif
#ifndef lint
static char	*sccsid = "@(#)ult_svc.c	3.1	(ULTRIX/OSF)	8/8/91";
#endif 
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
 * OSF/1 Release 1.0
 */

/*
 * ult_svc.c
 *
 * Modification History:
 *
 * 10-Oct-91    Philip Cameron
 *      Created file. Fix some compatability problems and added debug 
 *	and tracing functions.
 *
 */

#include <sys/secdefines.h>
#include <sys/proc.h>
#include <sys/user.h>
#include <sys/file.h>
#include <sys/fcntl.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/systm.h>
#include "ult_kmod.h"

#if SEC_BASE
#include <sys/security.h>
#include <sys/audit.h>
#include <sys/secpolicy.h>
#endif

#define STATIC static

extern	time_t		time;		/* system idea of date */


/*
 * This file contains system call code that is needed by the Ultrix
 *	Binary Compatability module. The calls bridge the gap between Ultrix
 *	expectations and OSF code.
 */


/*
 * The file control system call.
 *	This is a complete replacement for the OSF call.
 *
 *	From bsd/kern_descrip.c
 */
/* ARGSUSED */
ult_fcntl(p, args, retval)
	struct proc *p;
	void *args;
	long *retval;
{
	struct file *fp;
	register struct args {
		long	fdes;
		long	cmd;
		long	arg;
	} *uap = (struct args *)args;
	int i, error = 0;
	struct flock bf;
	char poflags;
	register struct ufile_state *ufp = &u.u_file_state;
	int fflags;

	if (error = getf(&fp, uap->fdes, &poflags, ufp))
		return (error);

	if(ULT_TRACE)
	    printf("(%s %d) fcntl: fdes %d, cmd %d, arg 0x%x\n", u.u_comm,
		u.u_procp->p_pid, uap->fdes, uap->cmd, uap->arg);

	switch(uap->cmd) {
	case F_DUPFD:
		i = uap->arg;
		if (i < 0 || i >= u.u_rlimit[RLIMIT_NOFILE].rlim_cur) {
			error = EINVAL;
			goto out;
		}
		if (error = ufalloc(i, &i, ufp))
			goto out;
		*retval = i;
		dupit(i, fp, poflags &~ UF_EXCLOSE, ufp);
		break;

	case F_GETFD:
		*retval = poflags & 1;
		break;

	case F_SETFD:
		U_FDTABLE_LOCK(ufp);
		/*
		 * Only perform the requested operation if the file
		 * descriptor was not closed or re-allocated between
		 * the first lock during getf and this lock.  Ignore
		 * poflags, u.u_pofile[uap->fdes] might have changed.
		 */
		if (U_OFILE(uap->fdes, ufp) == fp) {
			U_POFILE_SET(uap->fdes, 
				(U_POFILE(uap->fdes, ufp) &~ FD_CLOEXEC) |
				(uap->arg & FD_CLOEXEC), ufp);
		}
		U_FDTABLE_UNLOCK(ufp);
		break;

	case F_GETFL:
		BM(FP_LOCK(fp));
		/*
		 * Convert the returned flags to Ultrix 4.2 bit positions 
		 */
		*retval = cvtFcntlOtoU(fp->f_flag+FOPEN);
		BM(FP_UNLOCK(fp));
		break;

	case F_SETFL:
		/*
		 * Guarantee that flags will match the request to
		 * the ioctl routine by holding a lock across both
		 * operations.  We can't hold f_incore_lock for a
		 * long time so we cheat by using the f_io_lock.
		 * This is only a problem for a multiprocessor or a
		 * pre-emptible kernel.
		 */
		MP_ONLY(FP_IO_LOCK(fp));
		FP_LOCK(fp);
		fflags = fp->f_flag & FASYNC;
		fp->f_flag &= FCNTLCANT;
		/*
		 * Convert the passed flags to OSF bit positions 
		 */
		fp->f_flag |= cvtFcntlUtoO(uap->arg-FOPEN) &~ FCNTLCANT;
                if ((fflags ^ fp->f_flag) & FASYNC) {
                        /* Call down only if FASYNC changes */
                        FP_UNLOCK(fp);
                        fflags ^= FASYNC;
                        if (error = fioctl(fp, FIOASYNC, (caddr_t)&fflags)) {
                                /* Reset FASYNC to previous on failure */
                                FP_LOCK(fp);
                                fp->f_flag ^= FASYNC;
                                FP_UNLOCK(fp);
                        }
                } else
                        FP_UNLOCK(fp);
		MP_ONLY(FP_IO_UNLOCK(fp));
		break;

	case F_GETOWN:
		error = fgetown(fp, retval);
		break;

	case F_SETOWN:
		error = fsetown(fp, uap->arg);
		break;

	case F_GETLK:
		/* get record lock */
		if (copyin(uap->arg, &bf, sizeof bf))
			error = EFAULT;
		else if ((i=checkflock(&bf)) != 0)
			 error = i;
		else if ((i=getflck(fp, &bf)) != 0)
			error = i;
		/* Ultrix 4.2 binary compatability hack
		 *	Ultrix defines the F_UNLCK command to be 3 whereas
		 *	OSF/1 defines it to be 8. Ultrix 4.2 programs
		 *	expect this command to return F_UNLCK as 3.
		 *	Make sure that this is an Ultrix 4.2 executable.
		 */
		else {
			if (bf.l_type == F_UNLCK)
				bf.l_type = 3;	/* Ultrix 4.2 F_UNLCK */
			if(ULT_TRACE)
printf("(%s %d) fcntl: F_GETLK type %x, whence %d, start %d, len %d, pid %d\n",
			    u.u_comm, u.u_procp->p_pid, 
			    bf.l_type, bf.l_whence, bf.l_start,
			    bf.l_len, bf.l_pid);
			if (copyout(&bf, uap->arg, sizeof bf))
				error = EFAULT;
		}
		break;

	case F_SETLK:
		/* set record lock and return if blocked */
		if (copyin(uap->arg, &bf, sizeof bf))
			error = EFAULT;
		/* Ultrix 4.2 binary compatability hack
		 *	Ultrix defines the F_UNLCK command to be 3 whereas
		 *	OSF/1 defines it to be 8. Further, OSF/1 returns
		 *	EINVAL when l_type is 3. The fix is to detect 
		 *	the Ultrix 4.2 F_UNLCK and set it to the OSF/1
		 *	version.
		 */
		else {
			if(ULT_TRACE)
printf("(%s %d) fcntl: F_SETLK type %x, whence %d, start %d, len %d, pid %d\n",
			    u.u_comm, u.u_procp->p_pid, 
			    bf.l_type, bf.l_whence, bf.l_start,
			    bf.l_len, bf.l_pid);
			if (bf.l_type == 3 /* Ultrix 4.2 F_UNLCK */)
				bf.l_type = F_UNLCK;
			if ((i=checkflock(&bf)) != 0)
				 error = i;
			else if ((i=setflck(fp, &bf, 0)) != 0)
				error = i;
		}
		break;

	case F_SETLKW:
		/* set record lock and wait if blocked */
		if (copyin(uap->arg, &bf, sizeof bf))
			error = EFAULT;
		else {
			if(ULT_TRACE)
printf("(%s %d) fcntl: F_SETLKW type %x, whence %d, start %d, len %d, pid %d\n",
			    u.u_comm, u.u_procp->p_pid, 
			    bf.l_type, bf.l_whence, bf.l_start,
			    bf.l_len, bf.l_pid);
			if (bf.l_type == 3 /* Ultrix 4.2 F_UNLCK */)
				bf.l_type = F_UNLCK;
			if ((i=checkflock(&bf)) != 0)
				 error = i;
			else if ((i=setflck(fp, &bf, 1)) != 0)
				error = i;
		}
		break;

	default:
		error = EINVAL;
	}
out:
	FP_UNREF(fp);
	return (error);
}

static int
checkflock(bf)
struct flock *bf;
{
	short type;
	short whence;

	/* check l_type is valid */
	type = bf->l_type;
	if ((type != F_RDLCK) && (type != F_WRLCK) &&
	    (type != F_UNLCK))
		return EINVAL;

	/* check l_whence is valid */
	whence = bf->l_whence;
	if ((whence != L_SET) && (whence != L_INCR) &&
	    (whence != L_XTND))
		return EINVAL;

	/* check l_start is not before file start */
	/* Argh! off_t is unsigned - need casts */
	if (((long)bf->l_start < 0) &&
	    (whence == L_SET))
		return EINVAL;

	/* check that for -ve l_len, l_end is not before file start */
	if (((long)bf->l_len < 0) &&
	    (whence == L_SET)) {
		if (((long)(bf->l_start) - 1) < 0)
			return EINVAL;
	}

	return 0;
}



/*
 * This is the Ultrix 4.2 waitpid call.
 * It is really a call to waitf with compatability mode set and
 * a null last argument.
 */
ult_waitpid(p, args, retval)
        struct proc *p;
        void *args;
        long *retval;
{
        register struct args {
		long	 pid;		/* real type: 'pid_t' */
                int	*status;
                long     options;
        } *uap = (struct args *) args;
        struct args1 {
                long     pid;
                int     *status;
                long     options;
                struct  rusage_dev *rusage;
                long     compat;
        } uap1;
        uap1.pid = uap->pid;
        uap1.status = uap->status;
        uap1.options = uap->options;
        uap1.rusage = (struct rusage_dev *)0;
        uap1.compat = 1;

        return (waitf(p, &uap1, retval));
}



/* 
 * shmsys
 *	This supports the Ultrix 4.2 shared memory interface.
 *	It would have been in bsd/svipc_shm.c
 */

/* This code provides binary compatibility with Ultrix execuitables
 * that use svc 180 to access shared memory functions. 
 *
 * SHMSYS - System entry point for SHMAT, SHMCTL, SHMDT, and SHMGET
 *	system calls. This is a modified version of smsys() from 
 *	sys/uipc_smem.c in the Ultrix kernel. The smem struct is from
 *	h/shm.h
 *
 * NOTE: This code assumes that the Ultrix kernel is a POSIX kernel.
 */

#define SHMT_SHM        1 /* detach a segment created because smget   */
#define SHMT_MMAP       2 /* detach a segment created because of mmap */
#define IPC_MMAP        0020000 /* entry used for memory mapping  */
				/* device memory		  */
#define IPC_SYSTEM      0040000 /* share memory between kern/user */


#define FREE_ENTRY(sp)				\
MACRO_BEGIN					\
	sp->shm_object = VM_OBJECT_NULL;	\
	sp->s.shm_perm.mode = 0;		\
	sp->s.shm_segsz = 0;			\
	if (((int)(++(sp->s.shm_perm.seq) * shminfo.shmmni + (sp - shmem))) < 0) \
	    sp->s.shm_perm.seq = 0;		\
MACRO_END

/* ARGSUSED */
ult_shmsys(p, args, retval)
	struct proc *p;
	void *args;
	long *retval;
{
	register struct args {
		u_long	id;
		long    parm1,
		        parm2,
		        parm3;
	} *uap = (struct args *)args;
	register long *arg_pt;
	int retcode;
	int	shmat(),
		ult_svshmctl(),
		shmdt(),
		shmget();
	static int (*calls[])() = {shmat, ult_svshmctl, shmdt, shmget};

	arg_pt = &uap->parm1;
	switch ((int)uap->id) {
	      case 0:	/* shmat */
		if(ULT_TRACE)
		    printf("(%s %d) shmsys-at: id %d, addr 0x%x, flag 0x%x\n", 
			u.u_comm, u.u_procp->p_pid, uap->parm1, 
			uap->parm2, uap->parm3);

		retcode = (*calls[uap->id])(p, arg_pt, retval);

		if(ULT_TRACE)
			printf("(%s %d) shmsys-at: ret 0x%x, val 0x%x\n",
			u.u_comm, u.u_procp->p_pid, retcode, *retval);

		break;

	      case 1:	/* shmctl */
		retcode = (*calls[uap->id])(p, arg_pt, retval);
		break;

	      case 2:	/* shmdt */
		uap->parm2 = SHMT_SHM;
		if(ULT_TRACE)
		    printf("(%s %d) shmsys-dt: 0x%x, %d\n", 
			u.u_comm, u.u_procp->p_pid, *arg_pt, uap->parm2);

		retcode = (*calls[uap->id])(p, arg_pt, retval);

		break;

	      case 3:	/* shmget */
		/*
		 * 	do not permit users to pass IPC_SYSTEM flag 
		 *	or the IPC_MMAP flag which is used to implement
	    	 *	memory mapped devices.
		 *
		 */
		uap->parm3 &= ~(IPC_SYSTEM | IPC_MMAP);
		if(ULT_TRACE)
		    printf("(%s %d) shmsys-get: key %d, addr 0x%x, shmflg 0%o\n"
			, u.u_comm, u.u_procp->p_pid, uap->parm1, 
			uap->parm2, uap->parm3);

		retcode = (*calls[uap->id])(p, arg_pt, retval);

		break;

	      default:
		if(ULT_DEBUG)
		    printf("(%s %d) shmsys: Unknown command %d\n",
			u.u_comm, u.u_procp->p_pid, uap->id);
		if(uap->id > 3){
			return(EINVAL);
		}
		break;
	}
	return(retcode);
}



/*
 * struct smem is the struct definition that is used in Ultrix
 * execuitables that reference svc 180. shmctl is repeated here just
 * to support the old data struct. Since we don't have data for all 
 * of the fields, we provide 0 in the fields that don't exist for OSF/1
 */

struct dmap { int a; };
struct pte { int a; };
struct smem {
				/* SM_PERM must be the first	*/
				/* element in the structure.	*/
	struct ipc_perm	sm_perm;	/* permission struct		(s)*/
	struct dmap *sm_dmap; 	/* disk map of shm segment 		(p)*/
	long	sm_ptdaddr;	/* disk address of page table		(p)*/
	int	sm_size;	/* segment size (bytes)			(p)*/
	struct	proc *sm_caddr;	/* ptr to linked proc, if loaded	(s)*/
	struct	pte *sm_ptaddr;	/* ptr to assoc page table		(p)*/
	size_t	sm_rssize;	/* SM resource set size (pages)		(s)*/
	pid_t	sm_lpid;	/* pid of last smop			(.)*/
	pid_t	sm_cpid;	/* pid of creator			(p)*/
	unsigned short	sm_count;/* reference count			(x)*/
	unsigned short	sm_ccount;/* number of loaded references	(s)*/
	unsigned short	sm_lcount;/* number of processes locking SMS	(s)*/
	short	sm_flag;	/* traced, written flags	(see below)*/
	short	sm_poip;	/* page out in progress count		(s)*/
	time_t	sm_atime;	/* last smat time			(u)*/
	time_t	sm_dtime;	/* last smdt time			(u)*/
	time_t	sm_ctime;	/* last change time			(u)*/
};

/*
 * ult_svshmctl system call - perform shared memory control operation.
 *
 * This version uses the system V smem struct instead of the 
 * OSF shmid_ds struct. It also handles the SHM_LOCK and SHM_UNLOCK
 * commands.
 */
/* ARGSUSED */
ult_svshmctl(p, args, retval)
	struct proc *p;
	void *args;
	long *retval;
{
	register struct args {
		long		shmid,	/* shared mem id, returned by shmget */
				cmd;	/* IPC_RMID, IPC_STAT, or IPC_SET */
		struct smem	*arg;  

	}	*uap = (struct args *)args;
	register struct shmid_internal	*sp;	/* shared memory table entry */
	struct smem			ds;	/* hold area for IPC_SET */
	int				error;
	extern struct shmid_internal * shmconv();

#if	SEC_ARCH
	dac_t				dac;
	int				ret;
#endif	

	if ((sp = shmconv(uap->shmid)) == NULL)
		return(EINVAL);

#if	SEC_BASE
	/*
	 * Set audit event type based on function code
	 */
	AUDSTUB_IPC1(uap->cmd);
#endif
	switch((int)uap->cmd) {

	/* 
	 * Remove the shared memory identifier
	 */
	case IPC_RMID:
#if	SEC_BASE
		if (!sec_owner(sp->s.shm_perm.uid, sp->s.shm_perm.cuid))
			return(EPERM);
#else	
		if ((u.u_uid != sp->s.shm_perm.uid) 
		    && (u.u_uid != sp->s.shm_perm.cuid)
		    && (error = suser(u.u_cred, &u.u_acflag)))
			return(error);
#endif	

#if	SEC_ARCH
		if (sec_ipcaccess(&sp->s.shm_perm, SHMTAG(sp, 0), SP_DELETEACC))
			return(EACCES);
#endif

		if(ULT_TRACE)
		    printf("(%s %d) shmsys-ctl IPC_RMID: id %d\n", 
			u.u_comm, u.u_procp->p_pid, uap->shmid);
		/*
		 * decrement object ref_count so it will be removed
		 * now if there are no attached to it otherwise 
		 * after the last detach.
		 */
                vm_object_deallocate(sp->shm_object);

                FREE_ENTRY(sp);

		break;

	/* 
	 * Set ownership and permissions
	 */
	case IPC_SET:
#if	SEC_BASE
		if (!sec_owner(sp->s.shm_perm.uid, sp->s.shm_perm.cuid))
			return(EPERM);
#else
		if ((u.u_uid != sp->s.shm_perm.uid) 
		    && (u.u_uid != sp->s.shm_perm.cuid)
		    && (error = suser(u.u_cred, &u.u_acflag)))
			return(error);
#endif

#if	SEC_ARCH
		if(sec_ipcaccess(&sp->s.shm_perm, SHMTAG(sp, 0), SP_SETATTRACC))
			return(EACCES);
#endif	

		if (copyin(uap->arg, &ds, sizeof(ds)))
			return(EFAULT);
#if	SEC_BASE
                if (!sec_owner_change_permitted(sp->s.shm_perm.uid,
                                sp->s.shm_perm.gid, ds.sm_perm.uid,
                                ds.sm_perm.gid))
                        return(EPERM);
#endif	
		if(ULT_TRACE)
	printf("(%s %d) shmsys-ctl IPC_SET: id %d, uid %d, gid %d, mode %o\n", 
			u.u_comm, u.u_procp->p_pid, uap->shmid, 
			ds.sm_perm.uid, ds.sm_perm.gid, ds.sm_perm.mode);
		sp->s.shm_perm.uid = ds.sm_perm.uid;
		sp->s.shm_perm.gid = ds.sm_perm.gid;
		sp->s.shm_perm.mode = (ds.sm_perm.mode & 0777) |
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
#if	SEC_BASE
		AUDSTUB_DAC(1, sp->s.shm_perm.uid, sp->s.shm_perm.gid,
				sp->s.shm_perm.mode);
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

		bzero(&ds, sizeof(ds));	/* default fields to 0 */
		ds.sm_perm = sp->s.shm_perm;
		ds.sm_size = sp->s.shm_segsz;
		ds.sm_lpid = sp->s.shm_lpid;
		ds.sm_cpid = sp->s.shm_cpid;
		ds.sm_count = sp->s.shm_nattch;
		ds.sm_atime = sp->s.shm_atime;
		ds.sm_dtime = sp->s.shm_dtime;
		ds.sm_ctime = sp->s.shm_ctime;

		if(ULT_TRACE) {
	printf("(%s %d) shmsys-ctl IPC_STAT: id %d, uid %d, gid %d, mode %o\n", 
			u.u_comm, u.u_procp->p_pid, uap->shmid, 
			ds.sm_perm.uid, ds.sm_perm.gid,
			ds.sm_perm.mode);
	printf(
"(%s %d) shmsys-ctl IPC_STAT: segsz %d 0x%x, lpid %d, cpid %d, nattch %d\n",
			u.u_comm, u.u_procp->p_pid, ds.sm_size, 
			ds.sm_size, ds.sm_lpid, ds.sm_cpid,
			ds.sm_count);
	printf("(%s %d) shmsys-ctl IPC_STAT: atime %d, dtime %d, ctime %d\n",
			u.u_comm, u.u_procp->p_pid, ds.sm_atime, 
			ds.sm_dtime, ds.sm_ctime);
		}

		if (copyout(&ds, uap->arg, sizeof(ds)))
			return(EFAULT);

		break;

	case 3 /* SHM_LOCK */:
	case 4 /* SHM_UNLOCK */:
		printf("shmsys: shmctl: Lock/Unlock call\n");
		break;

	default:
		if(ULT_DEBUG)
		    printf("(%s %d) shmsys-ctl: ERROR cmd %d undefined\n", 
			u.u_comm, u.u_procp->p_pid, uap->cmd);
		return(EINVAL);
	}
	return(0);
}



/*
 * open
 *	Front end to vfs/vfs_syscalls.c open()
 */
ult_open(p, args, retval)
	struct proc *p;
	void *args;
	long *retval;
{
	register struct args {
		char	*fname;
		long	mode;
		long	crtmode;
	} *uap = (struct args *)  args;
	int error;

	/* convert mode flags */
	uap->mode = cvtOpenUtoO(uap->mode);

	if(ULT_TRACE)
		ult_trace_name("open", uap->fname);

	error = copen(p, args, retval, 1);

	return (error);
}



#ifdef PHIL
/*
 * The stat struct is different in Ultrix 4.2.
 *	The structs are the same length and only differ in the last 
 *	two fields. Conversions are done in place.
 */
struct  ustat
{
        dev_t           ust_dev;
        ino_t           ust_ino;
        unsigned short/*mode_t*/          ust_mode;
        short /*nlink_t*/         ust_nlink;
        short /*uid_t*/           ust_uid;
        short /*gid_t*/           ust_gid;
        dev_t           ust_rdev;
        off_t           ust_size;
        time_t          ust_atime;
        int             ust_spare1;
        time_t          ust_mtime;
        int             ust_spare2;
        time_t          ust_ctime;
        int             ust_spare3;
        long            ust_blksize;
        long            ust_blocks;
        unsigned long   ust_gennum; /* OSF st_flags */
        long            ust_spare4; /* OSF st_gen   */
};
#endif


/*
 * ostat
 * 	From bsd/kern_xxx.c ostat()
 */
ult_stat(p, args, retval)
	struct proc *p;
	void *args;
	long *retval;
{
	register struct args {
		char	*fname;
		struct stat *ub;
	} *uap = (struct args *) args;
	int	error;
	struct	ustat *us = (struct ustat *)uap->ub;

	if(ULT_TRACE)
		ult_trace_name("stat", uap->fname);

	error = ostat1(p, args, retval, FOLLOW);

	return(error);
}


/*
 * olstat
 * 	From bsd/kern_xxx.c olstat()
 */
ult_lstat(p, args, retval)
	struct proc *p;
	void *args;
	long *retval;
{
	register struct args {
		char	*fname;
		struct stat *ub;
	} *uap = (struct args *) args;
	int	error;
	struct	ustat *us = (struct ustat *)uap->ub;

	if(ULT_TRACE)
		ult_trace_name("lstat", uap->fname);

	error = ostat1(p, args, retval, NOFOLLOW);

	return(error);
}



/*
 * struct rusage
 *	This struct is different in Ultrix 4.2 so a conversion must
 *	be done in moving data to Ultrix. The Ultrix struct is longer
 *	so conversion is done in-place.
 */
struct  urusage {
	struct timeval uru_utime;	/* user time used */
	struct timeval uru_stime;	/* system time used */
	long	uru_maxrss;
	long	uru_ixrss; 	/* integral shared text size */
	long	uru_ismrss; 	/* integral shared memory size*/
	long	uru_idrss; 	/* integral unshared data " */
	long	uru_isrss; 	/* integral unshared stack " */
	long	uru_minflt; 	/* page reclaims */
	long	uru_majflt; 	/* page faults */
	long	uru_nswap; 	/* swaps */
	long	uru_inblock; 	/* block input operations */
	long	uru_oublock; 	/* block output operations */
	long	uru_msgsnd; 	/* messages sent */
	long	uru_msgrcv; 	/* messages received */
	long	uru_nsignals; 	/* signals received */
	long	uru_nvcsw; 	/* voluntary context switches */
	long	uru_nivcsw; 	/* involuntary " */
};

STATIC void cvtRusageOtoU();


/*
 * getrusage
 *	Replacement for getrusage that converts returned struct to
 *	Ultrix 4.2 form 	bsd/kern_resuorce.c
 */
ult_getrusage(p, args, retval)
	struct proc *p;
	void *args;
	long *retval;
{
	register struct args {
		long	who;
		struct	rusage *rusage;
	} *uap = (struct args *) args;
	register struct rusage *rup;
	struct urusage urup;	/* Ultrix 4.2 struct */
	time_value_t		sys_time, user_time;
	register struct timeval	*tvp;

	ASSERT(syscall_on_master());
	switch (uap->who) {

	case RUSAGE_SELF:
		/*
		 *	This is the current_thread.  Don't need to lock it.
		 */
		thread_read_times(current_thread(), &user_time, &sys_time);
		tvp = &u.u_ru.ru_utime;
		tvp->tv_sec = user_time.seconds;
		tvp->tv_usec = user_time.microseconds;
		tvp = &u.u_ru.ru_stime;
		tvp->tv_sec = sys_time.seconds;
		tvp->tv_usec = sys_time.microseconds;
		rup = &u.u_ru;
		break;

	case RUSAGE_CHILDREN:
		rup = &u.u_cru;
		break;

	default:
		return (EINVAL);
	}

	/* convert struct from OSF to Ultrix format */
	cvtRusageOtoU(&urup, rup);

	/* copy result back to caller */
	return (copyout(&urup, (caddr_t)uap->rusage, sizeof (struct urusage)));
}



/* 
 * cvtRusageOtoU(uru)
 *	Convert input struct ru to output struct uru.
 */
STATIC
void
cvtRusageOtoU(uru, ru)
struct urusage *uru;
struct rusage *ru;
{
	/* These are in the same location */
	uru->uru_utime = ru->ru_utime;
	uru->uru_stime = ru->ru_stime;
	uru->uru_maxrss = ru->ru_maxrss;
	uru->uru_ixrss = ru->ru_ixrss;

	/* these must move */
	uru->uru_nivcsw = ru->ru_nivcsw;
	uru->uru_nvcsw = ru->ru_nvcsw;
	uru->uru_nsignals = ru->ru_nsignals;
	uru->uru_msgrcv = ru->ru_msgrcv;
	uru->uru_msgsnd = ru->ru_msgsnd;
	uru->uru_oublock = ru->ru_oublock;
	uru->uru_inblock = ru->ru_inblock;
	uru->uru_nswap = ru->ru_nswap;
	uru->uru_majflt = ru->ru_majflt;
	uru->uru_minflt = ru->ru_minflt;
	uru->uru_isrss = ru->ru_isrss;
	uru->uru_idrss = ru->ru_idrss;

	/* Ultrix field that is not in OSF */
	uru->uru_ismrss = 0;
}
