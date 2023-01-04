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
static char *rcsid = "@(#)$RCSfile: kern_prot.c,v $ $Revision: 4.4.13.6 $ (DEC) $Date: 1993/10/05 21:13:20 $";
#endif 
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
 * kern_prot.c
 *
 * Modification History:
 *
 *  6-Nov-91     Paula Long
 *      Merged in OSF V1.0.1 bug fixes (1692 and 1693).  Fixes include:
 *          1. use uid_t and gid_t instead  int for uid and gid type
 *          2. added range checking for uid and gid fields.
 *       
 *  8-Oct-91     Philip Cameron
 *      Changed setpgrp to use POSIX rules instead of BSD rules. 
 *
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
 *	@(#)kern_prot.c	7.1 (Berkeley) 6/5/86
 */

#include <sys/secdefines.h>

/*
 * System calls related to processes and protection
 */

#include <machine/reg.h>

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/acct.h>
#include <sys/user.h>
#include <sys/vnode.h>
#include <sys/proc.h>
#include <sys/timeb.h>
#include <sys/times.h>
#include <sys/reboot.h>
#include <ufs/fs.h>
#include <sys/acct.h>
#include <sys/mount.h>
#include <kern/zalloc.h>
#include <sys/uswitch.h>
#if     SEC_BASE
#include <sys/security.h>
#endif  
#if     SEC_ARCH
#include <sys/secpolicy.h>
#endif 

#include <dcedfs.h>

zone_t cred_zone;

/*
 * A reminder about credentials locking.  The system call handler
 * stashes a pointer in the uthread structure to the task's
 * credentials, which normally live in the proc structure.  At
 * the time the syscall handler grabs the pointer, under PROC_LOCK,
 * it also increments the credentials' reference count so that
 * they will not later disappear.  At the end of the system call,
 * the handler disposes of the uthread's reference to the
 * credentials, potentially deallocating the credentials as well.
 *
 * Each invocation of a system call is thereby guaranteed a copy
 * of the credentials that will not change while the system call
 * executes.  Furthermore, in the typical case no locks need to
 * be taken when examining credentials because we never change
 * credentials in place.
 *
 * When changing credentials, however, we generally ignore the
 * credentials supplied by the system call handler (u.u_cred)
 * in favor of using the actual credentials in the proc structure
 * (p_rcred).  The PROC_LOCK guards the p_rcred field and the
 * miscellaneous credentials in the proc structure (p_r{g,u}id
 * and p_sv{g,u}id).  These miscellaneous credentials are a special
 * case and must always be examined and modified under PROC_LOCK.
 */

/*
 * Macros to determine whether POSIX setuid/setgid calls apply to real as
 * well as effective. For now these are just tests for the caller being 
 * super-user, but they could be changed in the future.
 */

#if     SEC_BASE
#define CAN_SET_REAL_UGID()    (privileged(SEC_SETPROCIDENT, 0))
#else
#define CAN_SET_REAL_UGID() (suser(u.u_cred,&u.u_acflag)==0)
#endif


/* ARGSUSED */
getpid(p, uap, retval)
	struct proc *p;
	void *uap;
	long *retval;
{

	*retval = p->p_pid;
	retval[1] = p->p_ppid;
	return (0);
}

/* ARGSUSED */
int
getpgid(p, args, retval)
	struct proc *p;
	void *args;
	long *retval;		/* returns type pid_t */
{
	struct args {
		long pid;	/* real type: 'pid_t' */
	} *uap = (struct args *) args;
	int usecp = ((pid_t)uap->pid == 0);

	if (!usecp) {
		if ((p = pfind(uap->pid)) == 0) {
			return (ESRCH);
		}
	}
	PROC_LOCK(p);
	*retval = (long)p->p_pgrp->pg_id;
	PROC_UNLOCK(p);
	return(0);
}

getpgrp(p, args, retval)
	struct proc *p;
	void *args;
	long *retval;
{
	struct args {
		long pid;
	} *uap = (struct args *) args;

	*retval = p->p_pgrp->pg_id;
	return (0);
}

#if	COMPAT_43
ogetpgrp(p, args, retval)
	struct proc *p;
	void *args;
	long *retval;
{
	struct args {
		long pid;
	} *uap = (struct args *) args;

	if (uap->pid != 0 && (p = pfind(uap->pid)) == 0)
		return (ESRCH);
	*retval = p->p_pgrp->pg_id;
	return(0);
}
#endif

#if     !SEC_BASE
/*
 * Test if the current user is the super user.
 */
suser(cred, ac_flag)
	struct ucred *cred;
	struct flag_field *ac_flag;
{
	if (cred->cr_uid == 0) {
		if (ac_flag) {
			FLAG_LOCK(ac_flag);
			ac_flag->fi_flag |= ASU;
			FLAG_UNLOCK(ac_flag);
		}
		return (0);
	}
	return (EPERM);
}
#endif

/* ARGSUSED */
getuid(p, uap, retval)
	struct proc *p;
	void *uap;
	long *retval;
{
	BM(PROC_LOCK(p));
	*retval = p->p_ruid;
	BM(PROC_UNLOCK(p));
	retval[1] = u.u_cred->cr_uid;
	return (0);
}

/* ARGSUSED */
getgid(p, uap, retval)
	struct proc *p;
	void *uap;
	long *retval;
{
	BM(PROC_LOCK(p));
	*retval = p->p_rgid;
	BM(PROC_UNLOCK(p));
	retval[1] = u.u_cred->cr_gid;
	return (0);
}

getgroups(p, args, retval)
	struct proc *p;
	void *args;
	long *retval;
{
	struct args {
		long	gidsetsize;
		int	*gidset;
	} *uap = (struct args *)args;
	register gid_t *gp;
	register int *lp;
	int groups[NGROUPS];
	int error;
	register struct ucred *cr;

	cr = u.u_cred;
	if (uap->gidsetsize == 0) {
		*retval = cr->cr_ngroups;
		return (0);
	}
	if (uap->gidsetsize < cr->cr_ngroups)
		return (EINVAL);
	uap->gidsetsize = cr->cr_ngroups;
	gp = cr->cr_groups;
	for (lp = groups; lp < &groups[uap->gidsetsize]; )
		*lp++ = *gp++;
	if (error = copyout((caddr_t)groups, (caddr_t)uap->gidset,
	    uap->gidsetsize * sizeof (groups[0])))
		return (error);
	*retval = uap->gidsetsize;
	return (0);
}

/* ARGSUSED */
setsid(p, uap, retval)
        struct proc *p;
        void *uap;
        long *retval;
{

	ASSERT(syscall_on_master());
	if ((p->p_pgid == p->p_pid) || pgfind(p->p_pid))
		return (EPERM);
	else {
		pgmv(p, p->p_pid, 1);
		*retval = p->p_pid;
		return (0);
	}
}

/*
 * set process group
 *
 * caller does setpgrp(pid, pgid)
 *
 * pid must be caller or child of caller (ESRCH)
 * if a child
 *	pid must be in same session (EPERM)
 *	pid can't have done an exec (EACCES)
 * if pgid != pid
 * 	there must exist some pid in same session having pgid (EPERM)
 * pid must not be session leader (EPERM)
 */
#ifdef COMPAT_43
/* ARGSUSED */
setpgrp(p, args, retval)
	struct proc *p;
	void *args;
	long *retval;
{
	ASSERT(syscall_on_master());
	return(setpgrp1(p, args, 1));
}
#endif

/* ARGSUSED */
setpgid(p, args, retval)
	struct proc *p;
	void *args;
	long *retval;
{
	ASSERT(syscall_on_master());
	return(setpgrp1(p, args, 0));
}

setpgrp1(cp, args, compat)
	struct proc *cp;
	void *args;
	int compat;
{
        register struct args {
                long     pid;
                long     pgid;
        } *uap = (struct args *)args;

	register struct proc *p;
	register struct pgrp *pgrp;
	int pflag;

	if (uap->pid != 0) {
		if ((p = pfind(uap->pid)) == 0 || !inferior(p))
			return (ESRCH);
		if (p->p_session != cp->p_session)
			return (EPERM);
		if (p != cp) {
			BM(PROC_LOCK(p));
			pflag = p->p_flag;
			BM(PROC_UNLOCK(p));
			if (pflag&SEXEC)
				return (EACCES);
		}
	} else
		p = cp;
	if (SESS_LEADER(p))
		return (EPERM);
	if (uap->pgid < 0)
		return (EINVAL);
	if (uap->pgid == 0) {
               uap->pgid = p->p_pid;
	} else if ((uap->pgid != p->p_pid) &&
		(((pgrp = pgfind(uap->pgid)) == 0) || 
		   pgrp->pg_mem == NULL ||
	           pgrp->pg_session != u.u_procp->p_session))
		return (EPERM);
	/*
	 * done checking, now doit
	 */
	pgmv(p, uap->pgid, 0);
	return (0);
}

/*
 * internal functions to set uids and gids
 */

/*
 * Grab all of our credentials immediately.  Use the
 * current proc's credentials rather than the credentials
 * reserved for this thread by the syscall handler.
 */
fetch_real_creds(p, pruidp, psvuidp, prgidp, psvgidp, crp)
	struct proc *p;
	uid_t *pruidp, *psvuidp;
	gid_t *prgidp, *psvgidp;
	struct ucred **crp;
{
	PROC_LOCK(p);
	if (pruidp)
		*pruidp = p->p_ruid;
	if (psvuidp)
		*psvuidp = p->p_svuid;
	if (prgidp)
		*prgidp = p->p_rgid;
	if (psvgidp)
		*psvgidp = p->p_svgid;
	*crp = p->p_rcred;
	crhold(*crp);
	PROC_UNLOCK(p);
}

/*
 * In SVR4 a credential structure is called a cred.  In OSF/1, it is
 * called a ucred.  A cred contains everything in a ucred as well as the
 * real and saved user and group ids.  According to the DDI/DKI Reference
 * Manual, a driver can examine all of the fields defined in a cred. In
 * order to support the DDI/DKI interfaces, it was necessary to add these
 * fields to the OSF/1 ucred. 
 *
 * In OSF/1, the real and saved  user and group ids were stored only in
 * the proc structure.  This function is used to make any changes to those
 * values.  Code was added to make sure that the extended ucred contains the
 * same values for real and saved user and group ids as the proc structure.
 */

substitute_real_creds(p, pruid, psvuid, prgid, psvgid, newcr)
	struct proc *p;
	uid_t pruid, psvuid;
	gid_t prgid, psvgid;
	struct ucred *newcr;
{
	register struct ucred *opcred, *oucred;

	crhold(newcr);			/* account for current thread */
	PROC_LOCK(p);
	if (pruid != NOUID) {
		p->p_ruid = pruid;
		newcr->_cr_ruid = pruid;
	}
	if (psvuid != NOUID) {
		p->p_svuid = psvuid;
		newcr->_cr_suid = psvuid;
	}
	if (prgid != NOGID) {
		p->p_rgid = prgid;
		newcr->_cr_rgid = prgid;
	}
	if (psvgid != NOGID) {
		p->p_svgid = psvgid;
		newcr->_cr_sgid = psvgid;
	}
	opcred = p->p_rcred;
	oucred = u.u_cred;
	u.u_cred = p->p_rcred = newcr;
	PROC_UNLOCK(p);
	crfree(opcred);
	crfree(oucred);
}

set_uids(p, ruid, euid, svuid)
	struct proc *p;
	uid_t ruid;
	uid_t euid;
	uid_t svuid;
{
	struct ucred	*cr, *newcr, *ocred;
	uid_t		pruid, psvuid;
	int	error;

	fetch_real_creds(p, &pruid, &psvuid, (gid_t *) 0, (gid_t *) 0, &cr);
	if (ruid == NOUID)
		ruid = pruid;

	if (ruid != pruid &&
#if	COMPAT_43
	    ruid != cr->cr_uid &&			/* XXX */
#endif
#if     SEC_BASE
	    (!privileged(SEC_SETPROCIDENT, EPERM))) {
		crfree(cr);
		return (EPERM);
	}
#else
	    (error = suser(cr, &u.u_acflag))) {
		crfree(cr);
		return (error);
	}
#endif
	if (euid == NOUID)
		euid = cr->cr_uid;
#if     SEC_BASE
	if (euid != cr->cr_uid && euid != pruid && euid != psvuid &&
	    (!privileged(SEC_SETPROCIDENT, EPERM))) {
		crfree(cr);
		return (EPERM);
	}
#else
	if (euid != cr->cr_uid && euid != pruid && euid != psvuid &&
	    (error = suser(cr, &u.u_acflag))) {
		crfree(cr);
		return (error);
	}
#endif

	/*
	 * Everything's okay, do it.
	 */

	/*
	 * Copy credentials so other references do not
	 * see our changes.
	 */
	newcr = crcopy(cr);
	newcr->cr_uid = euid;
	newcr->cr_ruid = ruid;	/* Only used by NFS at this writing */
	u.u_set_uids++;
	substitute_real_creds(p, ruid, svuid, NOGID, NOGID, newcr);
#if     SEC_ARCH
	SP_CHANGE_SUBJECT();
#endif
	return (0);
}

set_gids(p, rgid, egid, svgid)
	struct proc *p;
	gid_t rgid;
	gid_t egid;
	gid_t svgid;
{
	struct ucred	*newcr, *cr;
	gid_t		prgid, psvgid;
	int	error;

	fetch_real_creds(p, (uid_t *) 0, (uid_t *) 0, &prgid, &psvgid, &cr);
	if (rgid == NOGID)
		rgid = prgid;
	if (rgid != prgid &&
#if	COMPAT_43_XXX
	    rgid != cr->cr_gid &&		/* XXX */
#endif
#if     SEC_BASE
	    (!privileged(SEC_SETPROCIDENT, EPERM))) {
		crfree(cr);
		return (EPERM);
	}
#else
	    (error = suser(cr, &u.u_acflag))) {
		crfree(cr);
		return (error);
	}
#endif
	if (egid == NOGID)
		egid = cr->cr_gid;
	if (egid != cr->cr_gid && egid != prgid && egid != psvgid &&
#if     SEC_BASE
	   (!privileged(SEC_SETPROCIDENT, EPERM))) {
		crfree(cr);
		return (EPERM);
	}
#else
	   (error = suser(cr, &u.u_acflag))) {
		crfree(cr);
		return (error);
	}
#endif
	newcr = crcopy(cr);
	newcr->cr_gid = egid;
	substitute_real_creds(p, NOUID, NOUID, rgid, svgid, newcr);
#if     SEC_ARCH
	SP_CHANGE_SUBJECT();
#endif
	return (0);
}

/* ARGSUSED */
setreuid(p, args, retval)
	register struct proc *p;
	void *args;
	long *retval;
{
	struct args {
		long	ruid;
		long	euid;
	} *uap = (struct args *) args;

	if ((uap->ruid < -1) || (uap->euid < -1))
		return(EINVAL);

#if SEC_BASE
	if(!issetluid())
		return (EPERM);
#endif
	return (set_uids(p, (uid_t) uap->ruid, (uid_t) uap->euid, NOUID));
}

/*
 * The POSIX setuid system call. If privileged user, sets all of real,
 * effective, and saved uids; otherwise sets effective only (provided it
 * matches one of real, effective, or saved).
 */

/* ARGSUSED */
setuid(p, args, retval)
	register struct proc *p;
	void *args;
	long *retval;
{
	struct args {
		long	uid;		/* real type: 'uid_t' */
	} *uap = (struct args *)args;
	register uid_t uid;
	int error;

	uid = (uid_t) uap->uid;

	if (((int)uid < 0) || (uid > (uid_t)UID_MAX))
		return(EINVAL);

#if     SEC_BASE
	if (!issetluid())
		return (EPERM);
#endif
	if (CAN_SET_REAL_UGID())
		return(set_uids(p, uid, uid, uid));
	else
		return(set_uids(p, NOUID, uid, NOUID));
}


/* ARGSUSED */
setregid(p, args, retval)
	register struct proc *p;
	void *args;
	long *retval;
{
	register struct args {
		long	rgid;
		long	egid;
	} *uap = (struct args *)args;

	if ((uap->rgid < -1) || (uap->egid < -1))
		return(EINVAL);

#if SEC_BASE
	if(!issetluid())
		return (EPERM);
#endif
	return (set_gids(p, uap->rgid, uap->egid, NOGID));
}

/*
 * The POSIX setgid system call. If privileged user, sets all of real,
 * effective, and saved group gids; otherwise sets effective only (provided it
 * matches one of real, effective, or saved).
 */

/* ARGSUSED */
setgid(p, args, retval)
	register struct proc *p;
	void *args;
	long *retval;
{
	struct args {
		long	gid;		/* real type: 'gid_t' */
	} *uap = (struct args *)args;
	register gid_t gid;

	gid = (gid_t) uap->gid;

	if ((int)gid < 0)
		return(EINVAL);

#if     SEC_BASE
	if (!issetluid())
		return (EPERM);
#endif
	if (CAN_SET_REAL_UGID())
		return(set_gids(p, gid, gid, gid));
	else
		return(set_gids(p, NOGID, gid, NOGID));
}

/* ARGSUSED */
setgroups(p, args, retval)
	register struct proc *p;
	void *args;
	long *retval;
{
	/*
	 * All setgroups calls now intercepted by DCE DFS
	 */

#if  defined(DCEDFS) && DCEDFS
	return (dfs_xsetgroups (p, args, retval));
#else
	return (setgroups_base (p, args, retval));
#endif  /* DCEDFS */

}

/*
 * original, base setgroups call
 * now called from new setgroups entry point (above)
 */

/* ARGSUSED */
setgroups_base(p, args, retval)
	register struct proc *p;
	void *args;
	long *retval;
{
	register struct	args {
		u_long	gidsetsize;
		int	*gidset;
	} *uap = (struct args *)args;
	register gid_t *gp;
	register int *lp;
	int error, ngrp, groups[NGROUPS];
	struct ucred *newcr, *cr;

#if     SEC_BASE
	if (!issetluid() || !privileged(SEC_SETPROCIDENT, EPERM))
		return (EPERM);
#else 
	if (error = suser(u.u_cred, &u.u_acflag))
		return (error);
#endif
	ngrp = uap->gidsetsize;
	if (ngrp > NGROUPS)
		return (EINVAL);
	error = copyin((caddr_t)uap->gidset, (caddr_t)groups,
	    uap->gidsetsize * sizeof (groups[0]));
	if (error)
		return (error);
	cr = p->p_rcred;
	/*
	 * We need to account for our reference to this cred.  crcopy
	 * will decrement the credentials reference count and
	 * substitute_real_creds will decrement the reference count of
	 * both the u-area and proc creds.  So we bump the reference
	 * count here to account for the three unrefs.
	 */
	crhold(cr);
	newcr = crcopy(cr);
	gp = newcr->cr_groups;
	for (lp = groups; lp < &groups[uap->gidsetsize]; )
		*gp++ = *lp++;
	newcr->cr_ngroups = ngrp;
	substitute_real_creds(p, NOUID, NOUID, NOGID, NOGID, newcr);
#if     SEC_ARCH
	SP_CHANGE_SUBJECT();
#endif
	return (0);
}

/*
 * Check if gid is a member of the group set.
 * No locks needed because credentials are read-only.
 */
groupmember(gid, cred)
	gid_t gid;
	register struct ucred *cred;
{
	register gid_t *gp;
	gid_t *egp;

	if (gid == cred->cr_gid)
		return(1);
	egp = &(cred->cr_groups[cred->cr_ngroups]);
	for (gp = cred->cr_groups; gp < egp; gp++)
		if (*gp == gid)
			return (1);
	return (0);
}


/*
 * Routines to allocate and free credentials structures
 */

int	cractive = 0;
vdecl_simple_lock_data(,cractive_lock)
#define	CRSTAT(clause)		STATS_ACTION(&cractive_lock, (clause))


/*
 * Allocate a zeroed cred structure and reference it.
 */
struct ucred *
crget()
{
	register struct ucred *cr;

	ZALLOC(cred_zone, cr, struct ucred *);
	LASSERT(cr->cr_lock.slthread == 0);
	bzero((caddr_t)cr, sizeof(*cr));
	CR_LOCK_INIT(cr);
	ASSERT(cr->cr_ref == 0);
	cr->cr_ref = 1;
	CRSTAT(cractive++);
	return(cr);
}

/*
 * Free a cred structure.
 * Throws away space when ref count gets to 0.
 * This can never be done from interrupt level.
 */
crfree(cr)
	struct ucred *cr;
{
	CR_LOCK(cr);
	ASSERT(cr->cr_ref > 0);
	if (--cr->cr_ref != 0) {
		CR_UNLOCK(cr);
		return;
	}
#if	MACH_ASSERT
	ASSERT(cr->cr_dummy == 0);
#endif
	CR_UNLOCK(cr);
#if	MACH_LDEBUG
	cr->cr_lock.slthread = 0;
	cr->cr_lock.slck_addr = 0;
	cr->cr_lock.sunlck_addr = 0;
#endif
	ZFREE(cred_zone, cr);
	CRSTAT(cractive--);
}

/*
 * Copy cred structure to a new one and free the old one.
 * No lock needed as long as we hold cr referenced.
 */
struct ucred *
crcopy(cr)
	struct ucred *cr;
{
	struct ucred *newcr;
	int i;

	newcr = crget();
	*newcr = *cr;
	CR_LOCK_INIT(newcr);
	newcr->cr_ref = 1;
	crfree(cr);
	return(newcr);
}

/*
 * Dup cred struct to a new held one.
 * No lock needed as long as we hold cr referenced.
 */
struct ucred *
crdup(cr)
	struct ucred *cr;
{
	struct ucred *newcr;
	int i;

	newcr = crget();
	ASSERT(newcr->cr_ref == 1);
	*newcr = *cr;
	CR_LOCK_INIT(newcr);
	newcr->cr_ref = 1;
 	return(newcr);
}

#if	MACH_ASSERT
cr_ref(cr)
	struct ucred *cr;
{
	CR_LOCK(cr);
	if (cr->cr_dummy != 0) {
		printf("cr_ref: attempt to ref non-cred, cred = 0x%X\n", cr);
		panic("cr_ref");
	}
	(cr)->cr_ref++;
	CR_UNLOCK(cr);
}
#endif

/*
 * Obtain the first handle on credentials to be used by the
 * current thread.   Anyone else resetting the credentials makes
 * a new copy so the caller of this routine will always have
 * the same credentials it began with.  Used by uarea_init and syscall
 *
 * If the creds passed in are NOCRED, they get assigned to those of
 * the process with which the thread is associated.  If they are valid,
 * they are unreferenced and then reassigned.
 */
cr_threadinit(th)
	thread_t th;
{
	register struct proc *p;
	register struct uthread *uthread;

	uthread = th->u_address.uthread;
	if (uthread->uu_nd.ni_cred != NOCRED)
		crfree(uthread->uu_nd.ni_cred);
	p = th->u_address.utask->uu_procp;
	PROC_LOCK(p);
	uthread->uu_nd.ni_cred = p->p_rcred;
	crhold(p->p_rcred);
	ASSERT(p->p_rcred->cr_ref >= 2);
	PROC_UNLOCK(p);
}


/*
 * Get login name, if available.
 */
/* ARGSUSED */
getlogin(p, args, retval)
	struct proc *p;
	void *args;
	long *retval;
{
	struct args {
		char	*namebuf;
		u_long	namelen;
	} *uap = (struct args *)args;
	char buf[MAXLOGNAME];
	int error;

	/*
	 * N.B.: 4.3 reno has put u_logname into the proc structure, as
	 *	p_logname.  Do we want to do this?
	 */

	if (uap->namelen > sizeof (u.u_logname))
		uap->namelen = sizeof (u.u_logname);
	U_HANDY_LOCK();
	bcopy(u.u_logname, buf, MAXLOGNAME - 1);
	U_HANDY_UNLOCK();
	/* Error if no setlogin() has been done */
	if (buf[0] == '\0')
		error = EINVAL;
	else
		error = copyout((caddr_t)buf, (caddr_t)uap->namebuf,
				    uap->namelen);
	return (error);
}

/*
 * Set login name.
 */
/* ARGSUSED */
setlogin(p, args, retval)
	struct proc *p;
	void *args;
	long *retval;
{
	struct args {
		char	*namebuf;
	} *uap = (struct args *)args;
	char buf[MAXLOGNAME];
	int error;

#if     SEC_BASE
	if (!privileged(SEC_SETPROCIDENT, EPERM))
		return (EPERM);
#else
	if (error = suser(u.u_cred, &u.u_acflag))
		return (error);
#endif
	error = copyinstr((caddr_t)uap->namebuf, (caddr_t)buf,
	    MAXLOGNAME - 1, (int *) 0);
	if (!error) {
		U_HANDY_LOCK();
		bcopy(buf, u.u_logname, MAXLOGNAME - 1);
		U_HANDY_UNLOCK();
	} else if (error == ENOENT)	/* name too long */
		error = EINVAL;
	return (error);
}

kern_prot_init()
{
	cred_zone = zinit(sizeof(struct ucred), 1024*1024, 0, "ucred");
	VSTATS_LOCK_INIT(&cractive_lock);
}

int getsid(p, args, retval)
	struct proc *p;
	void *args;
	long *retval;
{
	struct args {
		long pid;		/* real type: 'pid_t' */
	} *uap = (struct args *) args;


	if (uap->pid != 0) {
		if ((p = pfind(uap->pid)) == 0) {
			return (ESRCH);
		}
	}
	if (p->p_session && p->p_session->s_leader)
		*retval = p->p_session->s_leader->p_pid;
	else
		*retval = 0;			

	return(0);
}

/*
 * Set/get user switches
 */

#ifdef __alpha
extern int map_zero_page_at_0();
#else
#define map_zero_page_at_0()	0
#endif /* __alpha */

int
uswitch(p, args, retval)
	struct proc *p;
	void *args;
	long *retval;		/* pid_t is returned */
{
	struct args {
		long cmd;
		long value;
	} *uap = (struct args *) args;
	int habitatval;

	switch(uap->cmd) {
		case(USC_SET):
			habitatval = USW_GETHAB(uap->value);
			if (habitatval < USW_MIN || habitatval > USW_MAX)
				return(EINVAL);
			if (USW_UNUSED(uap->value))
				return(EINVAL); /* unimplemented bits set*/
			U_HANDY_LOCK();
			u.u_uswitch = uap->value;
			p->p_habitat = habitatval;
			*retval	= u.u_uswitch;
			U_HANDY_UNLOCK();
			if (uap->value & USW_NULLP)
				return map_zero_page_at_0();
			break;
		case(USC_GET):
			/* return current value */
			U_HANDY_LOCK();
			*retval	= u.u_uswitch;
			U_HANDY_UNLOCK();
			break;
		default:
			return(EINVAL);
	}
	return(0);
}

/* used by tlb_trap.c to toggle NULL_P behavior */
emulate_null(){
	return(u.u_uswitch & USW_NULLP);
}
