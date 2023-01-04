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
static char *rcsid = "@(#)$RCSfile: sec_data.c,v $ $Revision: 1.1.4.4 $ (DEC) $Date: 1993/05/14 19:53:00 $";
#endif

#include <sec/include_sec>
#include <sys/syscall.h>

/*
 * This file contains the dispatch code for the security() system
 * call and those functions which must be implemented even when
 * SEC_BASE is turned off.
 */


int security_switch(), getluid(), setluid(), notinsys();

extern int nosys();

#if SEC_BASE

int stopio(), statpriv(), chpriv();
#if SEC_ARCH
int eaccess(), setlabel(), getlabel(), lmount(), islabeledfs();
#endif
#if SEC_MAC
int mkmultdir(), rmmultdir(), ismultdir();
#endif

#define	BASEsyscall(func)	func

#else	/* SEC_BASE */

#define BASEsyscall(func)	notinsys

#endif	/* SEC_BASE */

#if SEC_ARCH
#define	ARCHsyscall(func)	func
#else
#define	ARCHsyscall(func)	nosys
#endif
#if SEC_MAC
#define	MACsyscall(func)	func
#else
#define	MACsyscall(func)	nosys
#endif
#define	CHOTSsyscall(func)	nosys
#define	WISsyscall(func)	func

int (*sec_funcs[])() = {
	nosys,				/*  0 */
	BASEsyscall(stopio),		/*  1 */
	getluid,			/*  2 */
	setluid,			/*  3 */
	BASEsyscall(statpriv),		/*  4 */
	BASEsyscall(chpriv),		/*  5 */
	ARCHsyscall(eaccess),		/*  6 */
	MACsyscall(mkmultdir),		/*  7 */
	MACsyscall(rmmultdir),		/*  8 */
	MACsyscall(ismultdir),		/*  9 */
	ARCHsyscall(getlabel),		/* 10 */
	ARCHsyscall(setlabel),		/* 11 */
	ARCHsyscall(lmount),		/* 12 */
	ARCHsyscall(islabeledfs),	/* 13 */
	nosys,		/* 14 mk2person */
	nosys,		/* 15 rm2person */
	nosys,		/* 16 is2person */
	nosys,		/* 17 setsession */
	nosys,		/* 18 killsession */
	security_switch,		/* 19 */
};
#define	NUMSECFUNCS	(sizeof sec_funcs / sizeof sec_funcs[0])

/*
 * security system call
 *
 * Depending on the first argument, perform the real system call.
 * The reason we work off of one actual system call is to minimize
 * the entries into sysent[].  On some implementations, there is little
 * room for more sysent[] indices, so we take up only one and
 * subdivide the system calls based on the first argument.
 */

int sec_syscalls[NUMSECFUNCS];	/* perf. metering only for curiosity */

security(p, args, retval)
	struct proc *p;
	void *args;
	long *retval;
{
	register unsigned long *uap = (unsigned long *) args;
	long function = uap[0];
	int error;

	if (function >= NUMSECFUNCS)
		return (nosys(p, args, retval));
	else {
		error = (*sec_funcs[function])(p, &uap[1], retval);
		sec_syscalls[function]++;
		return (error);
	}
}

/*
 * Filler routine for functions we want to fail without sending SIGSYS.
 */

/* ARGSUSED */
notinsys(p, args, retval)
	struct proc *p;
	void *args;
	long *retval;
{
	AUDIT_CALL(SYS_security, ENOSYS, ((long *)args)-1, *retval, AUD_HPR,
			(uchar *) "00000000");
	return ENOSYS;
}

#if !SEC_BASE

/*
 * Get the login user ID for the current process.  However, if it is
 * not set yet, return an error instead.
 * MP Note: Locking here is mostly overkill, except on BM systems.
 */

getluid(p, args, retval)
	struct proc *p;
	void *args;
	long *retval;
{
	int error;
	uid_t auid;
	static uchar aud_parm[] = "a0000000";

	BM(PROC_LOCK(p));
	auid = p->p_auid;
	BM(PROC_UNLOCK(p));
	if (auid == NOUID)
		error = EPERM;
	else {
		error = 0;
		*retval = auid;
	}
	AUDIT_CALL(SYS_security, error, ((long *)args)-1, *retval, AUD_HPR,
			aud_parm);
	return(error);
}

/*
 * Set the login uid of the process, which gets inherited by all the
 * children.  Only the superuser can do this.  Once set, do not allow a change.
 * This must precede setuid(), since the logical login session begins
 * before the process is set to a user's ownership.  This is checked in
 * setuid() and setgid() by issetluid() .
 */

setluid(p, args, retval)
	struct proc *p;
	void *args;
	long *retval;
{
	register struct args {
		long uid;
	} *uap = (struct args *) args;
	int error;
	uid_t oauid, nauid;
	static uchar aud_parm[] = "al000000";

	nauid = uap->uid;
	if (!(error = suser(u.u_cred, &u.u_acflag))) {
		PROC_LOCK(p);
		oauid = p->p_auid;
		/* maybe check for additional reset privs here? */
		p->p_auid = nauid;
		PROC_UNLOCK(p);
		*retval = oauid;
	}
	AUDIT_CALL(SYS_security, error, ((long *)args)-1, *retval, AUD_HPR,
			aud_parm);
	return(error);
}

/*
 * test/set security code on/off at runtime
 */

security_switch(p, args, retval)
	struct proc *p;
	void *args;
	long *retval;
{
	register struct args {
		long swfunc;
		long groupid;
	} *uap = (struct args *) args;
	int error=0;
	static char done_once=0;
	static char *aud_parms[] = {
		"aa000000",
		"aam00000"
	};
	uchar *aud_parm = (uchar *)aud_parms[uap->swfunc == SEC_SWITCH_ON];

	switch (uap->swfunc) {
	  case SEC_SWITCH_STAT:
	  case SEC_SWITCH_CONF:
		*retval = 0;
		break;
	  case SEC_SWITCH_ON:
		error=ENOSYS;
		break;
	  case SEC_SWITCH_OFF:
		if (!(error = suser(u.u_cred, &u.u_acflag))) {
			if (done_once)
				error=EACCES;
			else
				done_once = 1;
		}
		break;
	  default:
		error=EINVAL;
		break;
	}
	AUDIT_CALL(SYS_security, error, ((long *)args)-1, *retval, AUD_HPR,
			aud_parm);
	return(error);
}

#endif /* !SEC_BASE */
