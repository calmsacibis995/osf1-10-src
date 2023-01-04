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
static char *rcsid = "@(#)$RCSfile: ult_trace.c,v $ $Revision: 1.1.3.3 $ (DEC) $Date: 1992/04/17 19:50:40 $";
#endif
#ifndef lint
static char	*sccsid = "@(#)ult_trace.c	3.1	(ULTRIX/OSF)	8/8/91";
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
 * ult_trace.c
 *	Collection of system call tracing entry points.
 *
 * Modification History:
 *
 * 10-Oct-91    Philip Cameron
 *      Created file. Functions that are call traced.
 *
 */

#include <sys/proc.h>
#include <sys/user.h>
#include <sys/param.h>
#include <sys/sem.h>
#include <sys/systm.h>
#include "ult_kmod.h"




/*
 * close
 *	Calls bsd/kern_descrip.c close()
 */
/* ARGSUSED */
ult_close(p, args, retval)
	struct proc *p;
	void *args;
	long *retval;
{
	struct args {
		long	i;
	} *uap = (struct args *)args;
	int error;

	if(ULT_TRACE)
	    printf("(%s %d) close: fdes %d\n", u.u_comm, u.u_procp->p_pid, 
		uap->i);

	error = close(p, args, retval);

	return(error);
}



/*
 * ofstat
 *	calls bsd/kern_descrip.c ofstat()
 */
/* ARGSUSED */
ult_fstat(p, args, retval)
	struct proc *p;
	void *args;
	long *retval;
{
	struct file *fp;
	register struct args {
		long	fdes;
		struct	stat *sb;
       	} *uap = (struct args *)args;
	int error = 0;

	if(ULT_TRACE)
		printf("(%s %d) fstat: fdes %d, sb 0x%x\n", u.u_comm,
		    u.u_procp->p_pid, uap->fdes, uap->sb);

	error = ofstat(p, args, retval);

	return(error);
}


/*
 * exec system call, with and without environments.
 */
struct execa {
	char	*fname;
	char	**argp;
	char	**envp;
};


/*
 * excev
 *	Calls bsd/kern_exec.c excev
 */
ult_execv(p, args, retval)
	struct proc *p;
	void *args;
	long *retval;
{
int	error;
register struct execa *uap = (struct execa *) args;

	if(ULT_TRACE)
		ult_trace_name("excev", uap->fname);
	error = execv(p, args, retval);

	return(error);
}


/*
 * exceve
 *	Calls bsd/kern_exec.c exceve
 */
ult_execve(p, args, retval)
	struct proc *p;
	void *args;
	long *retval;
{
int	error;
register struct execa *uap = (struct execa *) args;

	if(ULT_TRACE)
		ult_trace_name("excev", uap->fname);
	error = execve(p, args, retval);

	return(error);
}




/*
 * obreak
 *	Calls bsd/kern_mman.c obreak
 */
/* BEGIN DEFUNCT */
/* ARGSUSED */
ult_obreak(p, args, retval)
	struct proc *p;
	void *args;
	long *retval;
{
	int error;
	struct args {
		char	*nsiz;
	} *uap = (struct args *)args;

	if(ULT_TRACE)
		printf("(%s %d) obreak: nsiz 0x%x\n", 
			u.u_comm, u.u_procp->p_pid, uap->nsiz);

	error = obreak(p, args, retval);

	return(error);
}



/*
 * atomic_op
 *	Calls dec/machine/mips/machdep.c atomic_op
 */
ult_atomic_op(p, args, retval)
        struct proc *p;
        void *args;
        long *retval;
{
	int	error;
        register struct args {
                long opcode;                /* ATOMIC_SET or ATOMIC_CLEAR */
                int *address;              /* target address of the operation */
        } *uap = (struct args *) args;

	if(ULT_TRACE)
		printf("(%s %d) atomic_op: op %d, addr 0x%x\n", 
			u.u_comm, u.u_procp->p_pid, uap->opcode, uap->address);

	error = atomic_op(p, args, retval);

	return(error);
}



char *rlimit_lits[] = {"CPU", "FSIZE", "DATA", "STACK", "CORE", "RSS"};
/*
 * setrlimit
 *	Calls bsd/kern_resource.c setrlimit
 */
ult_setrlimit(p, args, retval)
	struct proc *p;
	void *args;
	long *retval;
{
	register struct args {
		u_long	which;
		struct	rlimit *lim;
	} *uap = (struct args *) args;
	struct rlimit alim;
	register struct rlimit *alimp;
	int error, trace = 0;

	if(ULT_TRACE) {
		trace = 1;
		ASSERT(syscall_on_master());
		if (uap->which >= RLIM_NLIMITS)
			return (EINVAL);
		alimp = &u.u_rlimit[uap->which];
		if (error = 
	    		copyin((caddr_t)uap->lim, 
				(caddr_t)&alim, sizeof (struct rlimit)))
			return (error);

	   printf("(%s %d) setrlimit: change %s, s/h 0x%x/0x%x, to 0x%x/0x%x\n",
		u.u_comm, u.u_procp->p_pid, rlimit_lits[uap->which], 
		alimp->rlim_cur, alimp->rlim_max, 
		alim.rlim_cur, alim.rlim_max);
	}

	/* make the real call */
	error = setrlimit(p, args, retval);

	return(error);
}



/*
 * getrlimit
 *	Replaces bsd/kern_resource.c getrlimit
 */
ult_getrlimit(p, args, retval)
	struct proc *p;
	void *args;
	long *retval;
{
	register struct args {
		u_long	which;
		struct	rlimit *rlp;
	} *uap = (struct args *) args;
	ASSERT(syscall_on_master());
	if (uap->which >= RLIM_NLIMITS)
		return (EINVAL);

	if(ULT_TRACE){
	    printf("(%s %d) getrlimit: %s, soft 0x%x, hard 0x%x\n", u.u_comm, 
		u.u_procp->p_pid, rlimit_lits[uap->which], 
		u.u_rlimit[uap->which].rlim_cur, 
		u.u_rlimit[uap->which].rlim_max);
	    if (uap->which == RLIMIT_RSS)
		printf("(%s %d) getrlimit: p_maxrss 0x%x, NBPG 0x%x\n", 
			u.u_comm, u.u_procp->p_pid, u.u_procp->p_maxrss, NBPG);
	}

	return (copyout((caddr_t)&u.u_rlimit[uap->which], (caddr_t)uap->rlp,
	    sizeof (struct rlimit)));
}


/* 
 * semctl
 *	Front end for bsd/svipc_sem.c semctl
 */
/* ARGSUSED */
ult_semctl(p, args, retval)
	struct proc *p;
	void *args;
	long *retval;
{
	register struct args {
		long            semid;
		long		semnum;
		long            cmd;
		long            arg;
	}	*uap = (struct args *) args;
	register int error;

	if(ULT_TRACE) {
	    printf("(%s %d) semctl: semid %d, semnum %d, cmd 0%o, arg 0x%x\n",
		u.u_comm, u.u_procp->p_pid, uap->semid, 
		uap->semnum, uap->cmd, uap->arg);
	}

	/* call semctl */
	error = semctl(p, args, retval);

	return(error);
}



/*
 * semop
 *	Front end for bsd/svipc_sem.c semop()
 */
/* ARGSUSED */
ult_semop(p, args, retval)
	struct proc *p;
	void *args;
	long *retval;
{
	register struct args {
		long            semid;	/* semaphore descriptor from semget() */
		struct sembuf  *sops;	/* array of semaphore operations */
		u_long          nsops;	/* number of operations */
	}	*uap = (struct args *) args;

	union {
		u_short		semvals[SEMMSL]; /* set semaphore values */
		struct semid_ds ds;		 /* set permission values */
		struct sembuf   semops[SEMOPM];	 /* operation holding area */
	} semtmp;

	register struct sembuf 		*op;	/* ptr to operation */
	register struct semid_ds 	*sp;	/* ptr to associated header */
	register struct sem 		*semp;	/* ptr to semaphore */
	register int    		i;	/* loop control */
	register int    		opslen;	/* size of operation buffers */
	struct uio      		suio;	/* I/O info */
	struct iovec    		siov;	/* I/O vectors */
	int				error = 0;

	if(ULT_TRACE){
		printf("(%s %d) semop: semid %d, sops 0x%x, nsops %d\n",
		u.u_comm, u.u_procp->p_pid, uap->semid, uap->sops, uap->nsops);
	}

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

	if(ULT_TRACE){
	    for (i = 0, op = semtmp.semops; i++ < uap->nsops; op++) {
		printf("(%s %d) semop: i %d, num %d, op 0%o, flg 0%o\n",
			u.u_comm, u.u_procp->p_pid, i,
			op->sem_num, op->sem_op, op->sem_flg);
	    }
	}

	error = semop(p, args, retval);

	return(error);
}


/*
 * getsysinfo
 *	From dec/machine/mips/sys_sysinfo.c
 */
int
ult_getsysinfo(p, args, retval) /* 6 */		/* 256 = getsysinfo */
	struct proc *p;
	void *args;
	long *retval;
{
	register struct args {
		unsigned long	 op;
		char		*buffer;
		unsigned long	 nbytes;
		int		*start;
		char		*arg;
		unsigned long	 flag;
	} *uap = (struct args *) args;

	int error = 0;


	if(ULT_TRACE)
	    printf("(%s %d) getsysinfo: op %d\n", 
		u.u_comm, u.u_procp->p_pid, uap->op);

	error = getsysinfo(p, args, retval);

	return(error);
}



/*
 * setsysinfo
 *	From dec/machine/mips/sys_sysinfo.c
 */
int
ult_setsysinfo(cp, args, retval) /* 5 */	/* 257 = setsysinfo */
	struct proc *cp;
	void *args;
	long *retval;
{
	register struct args {
		unsigned long	op;
		caddr_t		buffer;
		unsigned long	nbytes;
		caddr_t		arg;
		unsigned long	flag;
	} *uap = (struct args *) args;

	int			error = 0;

	if(ULT_TRACE)
	    printf("(%s %d) setsysinfo: op %d\n", 
		u.u_comm, u.u_procp->p_pid, uap->op);

	error = setsysinfo(cp, args, retval);

	return(error);
}




/* ------------------------------------------------------------------ */

/* 
 * ult_trace_name
 *	Function that displays a file name.
 */
ult_trace_name(svcname, fname)
char *svcname;	/* svc that is making the call */
char *fname;	/* user space location of file name */
{
int error;
char    buf[128];

	if(! fname) {
		printf("(%s %d) %s: NULL name pointer\n",
			u.u_comm, u.u_procp->p_pid, svcname);
		return;
	}

	if((error = copyin(fname, buf, 128))==0) {
		buf[127] = 0;
		printf("(%s %d) %s: --%s--\n", u.u_comm, u.u_procp->p_pid, 
			svcname, buf);
	} else {
		printf("(%s %d) %s: ERROR in copyin of name\n", 
			u.u_comm, u.u_procp->p_pid, svcname);
	}

}
