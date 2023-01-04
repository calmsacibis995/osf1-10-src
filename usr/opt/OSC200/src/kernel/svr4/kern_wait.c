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
static char *rcsid = "@(#)$RCSfile: kern_wait.c,v $ $Revision: 1.1.2.5 $ (DEC) $Date: 1993/10/14 19:55:57 $";
#endif

#include <kern/sched_prim.h>
#include <mach/time_value.h>
#if SEC_BASE
#include <sys/security.h>
#endif
#include <rt_timer.h>
#include <sys/time.h>
#include <sys/siginfo.h>
#include <sys/wait.h>
#include <sys/errno.h>
#include <sys/procset.h>

/*
 * This is the SVR4 waitid() system call.
 * It is really a call to waitf() with compatability mode set, and
 * with the arguments modified as appropriate for waitf().
 */
waitid(p, args, retval)
	struct proc *p;
	void *args;
	int *retval;
{
	struct args {
		long	idtype;		/* type is really idtype_t */
		long	id;		/* type is really id_t */
		siginfo_t *infop;
		long	options;	/* type is really int */
	} *uap = (struct args *) args;

	struct waitf_args {
		long	pid;
		int	*status;
		long	options;
		struct rusage_dev *rusage;
		long	compat;
	} uap1;

	/*
	 * This is a special case for SVVS4, which expects PIDs < 0
	 * to be invalid. Negative values are acceptable as PIDs
	 * for waitf(), so we need to check validity here, including
	 * the case of P_ALL, where the id is ignored altogether.
	 */
	if ((idtype_t)uap->idtype != P_ALL && (pid_t)uap->id < 0)
		return EINVAL;

	switch ((idtype_t)uap->idtype) {
	case P_PID:
		uap1.pid = uap->id;
		break;
	case P_PGID:
		/*
		 * waitf() recognizes pgids by
		 * the fact that they are negative.
		 */
		uap1.pid = -uap->id;
		break;
	case P_ALL:
		uap1.pid = WAIT_ANY;
		break;
	default:
		return EINVAL;
	}

	uap1.status = (int *)uap->infop;
	uap1.options = uap->options | WSIGINFO;
	
        uap1.rusage = (struct rusage_dev *)0;
	uap1.compat = 0;

	return(waitf(p, &uap1, retval));
}


/*
 * This is the SVR4 waitpid call.
 * It is really a call to waitf with compatability mode set and
 * a null last argument.
 */
sysv_waitpid(p, args, retval)
        struct proc *p;
        void *args;
        int *retval;
{
        register struct args {
		long 	pid;
                int	*status;
                long     options;
        } *uap = (struct args *) args;
        struct args1 {
                long    pid;
                int     *status;
                long    options;
                struct  rusage_dev *rusage;
                long    compat;
        } uap1;
        uap1.pid = uap->pid;
        uap1.status = uap->status;
        uap1.options = uap->options;
        uap1.rusage = (struct rusage_dev *)0;
        uap1.compat = 1;

	/*
	 * waitpid() must set WEXITED and WTRAPPED in uap->options
	 * before calling the version of waitf() that supports
	 * SVR4 wait behavior.
	 */
	uap1.options |= (WEXITED|WTRAPPED);
        return (waitf(p, &uap1, retval));
}

