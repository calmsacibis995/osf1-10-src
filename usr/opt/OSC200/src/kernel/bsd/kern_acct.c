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
static char	*sccsid = "@(#)$RCSfile: kern_acct.c,v $ $Revision: 4.3.7.3 $ (DEC) $Date: 1993/10/19 20:27:54 $";
#endif 
/*
 */
/*
 * (c) Copyright 1990, 1991, OPEN SOFTWARE FOUNDATION, INC.
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
 * OSF/1 Release 1.0.1
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
 * Copyright (c) 1982, 1986, 1989 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice and this paragraph are
 * duplicated in all such forms and that any documentation,
 * advertising materials, and other materials related to such
 * distribution and use acknowledge that the software was developed
 * by the University of California, Berkeley.  The name of the
 * University may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 *
 *
 *	Revision History:
 *
 * 30-May-91	prs
 *	Merged in 1.0.1 bug fixes from OSF.
 *
 * 03-May-91	Peter H. Smith
 *	Replace hardcoded priority assignment with a constant.
 */
#include <rt_sched_rq.h>

#include <kern/sched.h>

#include <sys/unix_defs.h>
#include <sys/secdefines.h>
#include <sys/param.h>
#include <sys/systm.h>
#include <sys/time.h>
#include <sys/proc.h>
#include <sys/user.h>
#include <sys/vnode.h>
#include <sys/specdev.h>
#include <sys/mount.h>
#include <sys/kernel.h>
#include <sys/file.h>
#include <sys/acct.h>
#include <sys/uio.h>
#include <sys/syslog.h>
#include <kern/parallel.h>
#if     SEC_BASE
#include <sys/security.h>
#endif  


/*
 * Values associated with enabling and disabling accounting
 */
int	acctsuspend = 2;	/* stop accounting when < 2% free space left */
int	acctresume = 4;		/* resume when free space risen to > 4% */
struct	timeval chk = { 15, 0 };/* frequency to check space for accounting */

/*
 * SHOULD REPLACE THIS WITH A DRIVER THAT CAN BE READ TO SIMPLIFY.
 */
struct vnode	*acctp;
int		acct_suspended = 0;

/*
 * The accounting variables are guarded with a lock.
 *
 * However, the acctwatch routine itself uses too many
 * filesystem locks to be
 * permitted to run at interrupt level.  (Vnode locks,
 * mount locks, possibly others through VFS_STATFS.)
 *
 * We will pay the price of checking accounting every so
 * often even when accounting hasn't been enabled.
 */
udecl_simple_lock_data(,accounting_lock)
#define	ACCT_LOCK()		usimple_lock(&accounting_lock)
#define	ACCT_UNLOCK()		usimple_unlock(&accounting_lock)


/*
 * Perform process accounting functions.
 */
/* ARGSUSED */
sysacct(p, args, retval)
	struct proc *p;
	void *args;
	long *retval;
{
	register struct vnode *vp;
	register struct args {
		char	*fname;
	} *uap = (struct args *)args;
	register struct nameidata *ndp = &u.u_nd;
	int error;
	struct vnode *oacctp;
	int flag;
	enum vtype type;

#if     SEC_BASE
	if (!privileged(SEC_ACCT, EPERM))
		return(EPERM);
#else
	if (error = suser(u.u_cred, &u.u_acflag))
		return(error);
#endif
	if (uap->fname == NULL) {
		ACCT_LOCK();
		acct_suspended = 0;
		if (vp = acctp) {
			acctp = NULL;
			ACCT_UNLOCK();
			vrele(vp);
		} else
			ACCT_UNLOCK();
		return (0);
	}
	ndp->ni_segflg = UIO_USERSPACE;
	ndp->ni_dirp = uap->fname;
	if (error = vn_open(ndp, FWRITE, 0644))
		return (error == EISDIR ? EACCES : error);
	vp = ndp->ni_vp;
	BM(VN_LOCK(vp));
	type = vp->v_type;
	BM(VN_UNLOCK(vp));
	if (type != VREG) {
		vrele(vp);
		return (EACCES);
	}
	ACCT_LOCK();
	oacctp = acctp;
	acctp = vp;
	acct_suspended = 0;
	ACCT_UNLOCK();
	acctwatch(); 
	if (oacctp) {
		vrele(oacctp);
		return (EBUSY);
	}
	return (0);
}

/*
 * Periodically check the file system to see if accounting
 * should be turned on or off.
 *
 */
acctwatch()
{
	struct vnode	*vp;
	struct mount	*mp;
	int		suspended, error, avail;
	long		blocks;
	struct timeval	atv;
	int		s;

	ACCT_LOCK();
	vp = acctp;
	if (vp == NULL) {
		ACCT_UNLOCK();
		return;
	}
	VREF(vp);
	suspended = acct_suspended;
	ACCT_UNLOCK();
	mp = vp->v_mount;
	if (suspended) {
		VFS_STATFS(mp, error);
		BM(MOUNT_LOCK(mp));
		avail = mp->m_stat.f_bavail;
		blocks = mp->m_stat.f_blocks;
		BM(MOUNT_UNLOCK(mp));
		if (avail > acctresume * blocks / 100) {
			ACCT_LOCK();
			if (vp == acctp)
				acct_suspended = 0;
			else
				suspended = 0;
			ACCT_UNLOCK();
			if (suspended)
				log(LOG_NOTICE, "Accounting resumed\n");
		}
	} else {
		VFS_STATFS(mp, error);
		BM(MOUNT_LOCK(mp));
		avail = mp->m_stat.f_bavail;
		blocks = mp->m_stat.f_blocks;
		BM(MOUNT_UNLOCK(mp));
		if (avail <= acctsuspend * blocks / 100) {
			ACCT_LOCK();
			if (vp == acctp)
				suspended = acct_suspended = 1;
			ACCT_UNLOCK();
			if (suspended)
				log(LOG_NOTICE, "Accounting suspended\n");
		}
	}
	vrele(vp);
}

acctwatch_thread()
{
	thread_t	thread;
	struct timeval	atv;
	extern int	wakeup();
	int		s;

	thread = current_thread();
/*
 * RT_SCHED_RQ:  Use a constant instead of hardcoding the assignment to
 * priority and sched_pri.  Simplifies remapping of priorities.  It is
 * acceptable to change sched_pri here because the thread is not in a
 * run queue if it is the current thread.
 */
	thread->priority = thread->sched_pri = BASEPRI_ACCTWATCH;
	thread_swappable(thread, FALSE);

	for (;;) {
		assert_wait((vm_offset_t)acctwatch, FALSE);
		atv = chk;
		s = splhigh();
		TIME_READ_LOCK();
		timevaladd(&atv, &time);
		TIME_READ_UNLOCK();
		splx(s);
		thread_set_timeout(hzto(&atv));
		thread_block();
		acctwatch();
	}
}

/*
 * On exit, write a record on the accounting file.
 * MACH:  assume only one thread in task is active, so no
 * locking required for task state.
 */
acct(status)
	int status;
{
	struct vnode *vp;
	register struct rusage *ru;
	struct timeval t;
	int i, s, error;
	struct acct acctbuf;
	register struct acct *ap = &acctbuf;
	register struct proc *p;

	ACCT_LOCK();
	if (((vp = acctp) == NULL) || acct_suspended) {
		ACCT_UNLOCK();
		return(0);
	}
	VREF(vp);			/* XXX */
	ACCT_UNLOCK();
	bcopy(u.u_comm, ap->ac_comm, sizeof(ap->ac_comm));
	ru = &u.u_ru;
	ap->ac_utime = compress(ru->ru_utime.tv_sec, ru->ru_utime.tv_usec);
	ap->ac_stime = compress(ru->ru_stime.tv_sec, ru->ru_stime.tv_usec);
	s = splhigh();
	TIME_READ_LOCK();
	t = time;
	TIME_READ_UNLOCK();
	splx(s);
	timevalsub(&t, &u.u_start);
	ap->ac_etime = compress(t.tv_sec, t.tv_usec);
	ap->ac_btime = u.u_start.tv_sec;
	p = u.u_procp;
	PROC_LOCK(p);
	ap->ac_uid = p->p_ruid;
	ap->ac_gid = p->p_rgid;
	PROC_UNLOCK(p);
	t = ru->ru_stime;
	timevaladd(&t, &ru->ru_utime);
	if (i = t.tv_sec * hz + t.tv_usec / tick)
		ap->ac_mem = (ru->ru_ixrss+ru->ru_idrss+ru->ru_isrss) / i;
	else
		ap->ac_mem = 0;
	ap->ac_rw = compress(ru->ru_inblock + ru->ru_oublock, (long)0);
	ap->ac_io = compress(u.u_ioch, (long)0);
	/*
         * no locks needed since process is exiting
         */
	if ((p->p_flag & SCTTY) && p->p_session->s_ttyvp != NULL)
		ap->ac_tty = p->p_session->s_ttyvp->v_rdev;
	else
		ap->ac_tty = NODEV;
	ap->ac_flag = (char) u.u_acflag.fi_flag; /* XXX */
	ap->ac_stat = status;
	error = vn_rdwr(UIO_WRITE, vp, (caddr_t)ap, sizeof (acctbuf),
		(off_t)0, UIO_SYSSPACE, IO_UNIT|IO_APPEND, u.u_cred, (int *)0);
	vrele(vp);
	return (error);
}

/*
 * Produce a pseudo-floating point representation
 * with 3 bits base-8 exponent, 13 bits fraction.
 */
compress(t, ut)
	register long t;
	long ut;
{
	register exp = 0, round = 0;

	t = t * AHZ;  /* compiler will convert only this format to a shift */
	if (ut)
		t += ut / (1000000 / AHZ);
	while (t >= 8192) {
		exp++;
		round = t&04;
		t >>= 3;
	}
	if (round) {
		t++;
		if (t >= 8192) {
			t >>= 3;
			exp++;
		}
	}
	return ((exp<<13) + t);
}
