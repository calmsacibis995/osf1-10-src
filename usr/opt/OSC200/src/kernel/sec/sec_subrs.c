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
static char *rcsid = "@(#)$RCSfile: sec_subrs.c,v $ $Revision: 4.2.13.2 $ (DEC) $Date: 1993/04/01 20:06:15 $";
#endif
/*
 * (c) Copyright 1990, 1991, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0.1
 */
/*
 * Copyright (c) 1987-1990 SecureWare, Inc.  All rights reserved.
 *
 * This Module contains Proprietary Information of SecureWare, Inc. and
 * should be treated as Confidential.
 */

/*#ident "@(#)sec_subrs.c	3.4 22:54:07 6/12/90 SecureWare"*/
/*
 * Based on:	@(#)sec_subrs.c	2.13.2.3 12:53:16 2/2/90
 */

#include <sec/include_sec>
#include <sys/acct.h>

#if SEC_BASE

extern int	dead_lookup(),
		dead_open(),
		dead_strategy(),
		dead_select(),
		dead_rdwr(),
		dead_ebadf(),
		dead_badop(),
		dead_nullop();

int		sec_dead_ioctl(),
		sec_dead_read(),
		sec_dead_write();

struct vnodeops sec_deadvnops = {
	dead_lookup,	/* lookup */
	dead_badop,	/* create */
	dead_badop,	/* mknod */
	dead_open,	/* open */
	dead_nullop,	/* close */
	dead_ebadf,	/* access */
	dead_ebadf,	/* getattr */
	dead_ebadf,	/* setattr */
	sec_dead_read,	/* read */
	sec_dead_write,	/* write */
	sec_dead_ioctl,	/* ioctl */
	dead_select,	/* select */
	dead_badop,	/* mmap */
	dead_nullop,	/* fsync */
	dead_nullop,	/* seek */
	dead_badop,	/* remove */
	dead_badop,	/* link */
	dead_badop,	/* rename */
	dead_badop,	/* mkdir */
	dead_badop,	/* rmdir */
	dead_badop,	/* symlink */
	dead_ebadf,	/* readdir */
	dead_ebadf,	/* readlink */
	dead_badop,	/* abortop */
	dead_nullop,	/* inactive */
	dead_nullop,	/* reclaim */
	dead_rdwr,	/* bmap */
	dead_strategy,	/* strategy */
	dead_badop,	/* print */
	dead_badop,	/* page_read */
	dead_badop,	/* page_write */
};

/*
 * Declare some system-wide locks.
 * XXX Add description of what each controls.
 */

decl_simple_lock_data(, sec_alloc_object_lock)

decl_simple_lock_data(, sip_tag_lock)
decl_simple_lock_data(, sip_attr_lock)
decl_simple_lock_data(, sip_audit_lock)
decl_simple_lock_data(, sip_audit_flag_lock)
decl_simple_lock_data(, audit_stats_lock)

privvec_t	su_excluded_privs;
#if SEC_PRIV
privvec_t	su_included_privs;
#else
privvec_t	su_all_privs;

/* privileged group id, set in security_switch() */
uid_t 	sec_priv_gid=0;

/* security runtime on/off flags */
int 	check_luid=0;
int 	check_privileges=0;
int 	security_is_on=0;

#endif

/*
 * Called from main() after basic u fields are set and before the
 * newproc(s) are invoked, this sets basic default security parameters.
 * Although most values are implicitly set due to the zero'ing of the
 * bss area beforehand, the 0 assignments here are merely safeguards
 * against an MMU that didn't do the zeroing.
 * MP Note:  We assume that this is called before the 2nd and subsequent
 * processors are started, and therefore ignore the locking that would
 * otherwise be required in many places.
 */
void
sec_init()
{
	register int scan;
	register int priv_vec_size = SEC_SPRIVVEC_SIZE;
	register int max_priv = SEC_MAX_SPRIV;
	register int base;

	simple_lock_init(&sip_tag_lock);
	simple_lock_init(&sip_attr_lock);
	simple_lock_init(&sip_audit_lock);
	simple_lock_init(&sip_audit_flag_lock);
	simple_lock_init(&audit_stats_lock);

	/*
	 * Initialize the security attributes of process 0.
	 * These will be inherited by init.
	 */

	/*
	 * Turn on all privileges.
	 */
	bzero(SIP->si_mpriv, sizeof SIP->si_mpriv);
	bzero(SIP->si_bpriv, sizeof SIP->si_bpriv);
	bzero(SIP->si_epriv, sizeof SIP->si_epriv);
#if !SEC_PRIV
	bzero(su_all_privs, sizeof su_all_privs);
#endif

	for (scan = 0; scan <= SEC_MAX_SPRIV; scan++)  {
		ADDBIT(SIP->si_mpriv, scan);
		ADDBIT(SIP->si_bpriv, scan);
		ADDBIT(SIP->si_epriv, scan);
#if !SEC_PRIV
		ADDBIT(su_all_privs, scan);
#endif
	}

	/*
	 * Remove the superuser compatibility privileges from
	 * the base and effective privilege sets to prevent
	 * primordial processes from getting superuser treatment.
	 */
	RMBIT(SIP->si_epriv, SEC_SUCOMPAT);
	RMBIT(SIP->si_bpriv, SEC_SUCOMPAT);
	RMBIT(SIP->si_epriv, SEC_SUPROPAGATE);
	RMBIT(SIP->si_bpriv, SEC_SUPROPAGATE);

	/*
	 * Set up the vector of privileges that are independent
	 * of superuser.
	 */
	bzero(su_excluded_privs, sizeof su_excluded_privs);
	ADDBIT(su_excluded_privs, SEC_CHPRIV);
#if SEC_MAC
	ADDBIT(su_excluded_privs, SEC_MULTILEVELDIR);
#endif

#if SEC_PRIV
	/*
	 * Set up the vector of privileges that are granted based
	 * on either being superuser or having the bit in si_epriv.
	 */
	bzero(su_included_privs, sizeof su_included_privs);
	ADDBIT(su_included_privs, SEC_CHMODSUGID);
	ADDBIT(su_included_privs, SEC_EXECSUID);
#endif

#if SEC_ARCH
	/*
	 * Set all tags to the WILDCARD value.
	 */
	for (scan = 0; scan < SEC_TAG_COUNT; ++scan)
		SIP->si_tag[scan] = SEC_WILDCARD_TAG_VALUE;
#endif

	/*
	 * In standard UNIX, the default mask is 0, providing creation
	 * of mode 0777 files.  We want to begin with mode SEC_DEFAULT_MODE
	 * files and let the user increase the mode as he sees fit.  This way,
	 * the system provides tight security be default and programs
	 * and users must explicitly relax privileges.
	 */
	u.u_cmask = (~SEC_DEFAULT_MODE) & 0777;

	/*
	 * Mark the luid as not set.
	 */
	SIP->si_luid_set = 0;
	SIP->si_luid = 0;

#if SEC_MAC
	/*
	 * Zero the diversion name used for multilevel directories.  If
	 * ever a process without the SEC_MULTILEVELDIR privilege and
	 * without being assigned a security level tries to use a
	 * multllevel directory, it will fail.
	 */
	bzero(SIP->si_diversion, sizeof(SIP->si_diversion));
#endif /* SEC_MAC */

	printf("Portions Copyright (c) 1987-1990 SecureWare, Inc.  ");
	printf("All Rights Reserved.\n\n");

	sec_switch_security(0);/*OFF by default*/
}

#if SecureWare
secureware_banner()
{
	/*
	 * Let the world know of secure UNIX:
	 */

#if !defined(SMP_CONFIGURED) && SEC_MAC
	printf("\n\t*** SecureWare SMP+ B1 Kernel ***\n\n");
#define SMP_CONFIGURED
#endif

#if !defined(SMP_CONFIGURED)
	printf("\n\t*** SecureWare SMP C2 Kernel ***\n\n");
#define SMP_CONFIGURED
#endif
}
#endif SecureWare

/*
 * Modify the process group of the daemon to that of Shutdown
 * This is called by the SPIOC_IMMUNE and AUDIOC_SHUTDOWN ioctls.
 * PORT NOTE: System specific how this gets done!
 */

set_shutdown_pgrp(p)
struct proc *p;
{	
}
/*
 * Return error code if the user is not allowed to execute
 * SUID programs and this is a SUID program.  The user is not
 * allowed if he does not have the privilege or the program is
 * SUID/SGID to another user and the login user is not yet set.
 * Otherwise, merely return 0 and cause no side-effects.  Note that
 * when the SUID program is owned by the current user, we treat
 * it as a normal program.
 */
int
exec_allowed(vap)
	register struct vattr *vap;
{
	uid_t uid;
	gid_t gid;

	uid = vap->va_mode & VSUID ? vap->va_uid: u.u_uid;
	gid = vap->va_mode & VSGID ? vap->va_gid: u.u_gid;

	if(check_luid && !SIP->si_luid_set && !SEC_SUSER(uid,gid))
		if (uid != u.u_uid || gid != u.u_gid) {
			return(EACCES);
		}

	/* Handle privilege call separately for EPERM generation */

	if (uid != u.u_uid &&
#if !SEC_PRIV
	    ! (SEC_SUSER(uid,gid)) &&
#endif
	    !(privileged(SEC_EXECSUID, EPERM)))
		return(EPERM);

	return(0);
}

/*
 * At exec() time, recompute the effective privilege to be:
 *
 *	E = UNION(B, G)
 *
 * Also, save the potential privileges of the program now being executed.
 * The second argument indicates whether or not to ignore the privileges
 * associated with the file; it is set if the process is being traced or
 * on systems configured without file privilege sets if the executed file
 * is not setuid to root or setgid to SEC_GID.
 * 
 * On systems configured without file privileges, the granted (G) set is taken
 * to be empty, and the potential set is taken to be the complete set of
 * defined privileges.
 *
 * MP Note:
 * SIP_ATTR_LOCK() may be overkill here, since this is currently only
 * called at exec time, when (presumably) the process will be single-threaded
 * and privileges thus won't be changing, but it's cheap insurance.
 */
compute_subject_privileges(vp, ignore)
	struct vnode *vp;
	int ignore;
{
	register int	i, error;

#if SEC_PRIV
	struct vsecattr	vsattr;

	vsattr.vsa_valid = VSA_PPRIV | VSA_GPRIV;

	if (!ignore && VHASSECOPS(vp) &&
	    (VOP_GETSECATTR(vp, &vsattr, u.u_cred, error)) == 0) {
		SIP_ATTR_LOCK();
		for (i = 0; i < SEC_SPRIVVEC_SIZE; ++i) {
			SIP->si_epriv[i] = SIP->si_bpriv[i]
				| vsattr.vsa_gpriv[i];
			SIP->si_spriv[i] = vsattr.vsa_ppriv[i];
		}
		SIP_ATTR_UNLOCK();
	}
#else /* !SEC_PRIV */
	if (!ignore) {
		SIP_ATTR_LOCK();
		for (i = 0; i < SEC_SPRIVVEC_SIZE; ++i) {
			SIP->si_epriv[i] = SIP->si_bpriv[i];
			SIP->si_spriv[i] = su_all_privs[i];
		}
		SIP_ATTR_UNLOCK();
	}
#endif /* !SEC_PRIV */
	else {
		SIP_ATTR_LOCK();
		for (i = 0; i < SEC_SPRIVVEC_SIZE; ++i) {
			SIP->si_epriv[i] = SIP->si_bpriv[i];
			SIP->si_spriv[i] = 0;
		}
		SIP_ATTR_UNLOCK();
	}
}

/*
 * Check to see if the login uid is set.  If not, set the EPERM
 * error and return 0.  If set, return 1 and everything is
 * OK.  This is to be used like owner() and suser() to
 * make sure that operations that need to be done only after
 * setluid() are forced to be done then only, like all forms
 * of identity changes.  It is currently called only by code that
 * changes uids and gids.
 */
int
issetluid()
{
	register int luid_set;

	if (!check_luid)
		return (1);

	return(SIP->si_luid_set);
}

/*
 * Returns 1 if the mode can be changed given the user's privilege
 * set, and 0 if the mode cannot be changed.  The user cannot set the
 * SUID or SGID bits without the CHMODSUGID privilege, which is
 * helpful to prevent Trojan horses from setting SUID shells and the
 * like.  This is called only from chmod() code.
 */
int
sec_mode_change_permitted(new_mode)
	int new_mode;
{
	return 	((new_mode & (ISUID|ISGID)) == 0) ||
	    	privileged(SEC_CHMODSUGID, EPERM);
}


/*
 * sec_owner_change_permitted
 *
 * Return 1 if the specified change in ownership (uid and gid) is allowed.
 * Otherwise return 0.  If either of the new
 * owner and group ids differs from the corresponding old value, the CHOWN
 * privilege is required.  On systems that support supplementary groups,
 * the gid can be changed without the privilege as long as the new gid is
 * among the process's supplementary group list.
 */
int
sec_owner_change_permitted(oown, ogrp, nown, ngrp)
	uid_t oown;
	gid_t ogrp;
	uid_t nown;
	gid_t ngrp;
{
	/*
	 * XXX possible races here with group changes
	 */
	return 	nown == oown && 
		(ngrp == ogrp || groupmember(ngrp, u.u_cred)) ||
	    	privileged(SEC_CHOWN, EPERM);
}


/*
 * Used to see if process has the effective privilege specified.
 * Returns 0 if the process does not have privilege, 1 if it does.
 * If the error_code is 0, no side affects are done.  Otherwise, make
 * sure that the attempt to use the privilege is audited.
 */
int
privileged(privilege, error_code)
	int privilege;
	register int error_code;
{
	register int has_privilege;
	int is_suser = (u.u_uid == SEC_UID);
	struct flag_field *ac_flag;

	if (is_suser) {
		ac_flag = &u.u_acflag; 	/* accounting */
		FLAG_LOCK(ac_flag);
		ac_flag->fi_flag |= ASU;
		FLAG_UNLOCK(ac_flag);
	}

	if(! check_privileges) {
		/* checking for privileges is OFF, so just do what suser() does
		 * note: the return value of suser() is in the reverse
		 */
		if (is_suser)
			return (1);
		else
			return (0);
	}

#if SEC_PRIV
	if (ISBITSET(SIP->si_epriv, SEC_SUCOMPAT))
#endif
	{
		if (ISBITSET(su_excluded_privs, privilege))
			has_privilege = ISBITSET(SIP->si_epriv, privilege);
#if SEC_PRIV
		else if (!ISBITSET(su_included_privs, privilege))
			has_privilege = is_suser;
#endif
		else
			has_privilege = is_suser ||
					ISBITSET(SIP->si_epriv, privilege);
	}
#if SEC_PRIV
	else
		has_privilege = ISBITSET(SIP->si_epriv, privilege);
#endif

	return(has_privilege);
}


/*
 * Copy the security info structure of the parent to the child for fork(2)
 * This is called only from newproc().
 */
void
secinfo_dup(pp, cp)
struct proc *pp, *cp;
{
	register struct security_info *psip, *csip;
	mask_t *subj_bits_ptr;

	/* Set the parent and child security_info[] pointers and copy */

	psip = &secinfo[pp - proc];	/* parent SIP */
	csip = &secinfo[cp - proc];	/* child  SIP */

	if (!security_is_on) {
			register int i;

		for (i = 0; i < SEC_SPRIVVEC_SIZE; ++i) {
			csip->si_bpriv[i] = csip->si_epriv[i]
				= csip->si_mpriv[i] = csip->si_spriv[i]
					= su_all_privs[i];
		}
		csip->si_luid_set = psip->si_luid_set;
		csip->si_luid = psip->si_luid;
		return;
	} 

	/*
	 * Give the child an exact copy of the parent's SIP.
	 * This leaves a few fields in the child wrong, but
	 * we'll fix that momentarily.  We must hold SIP_ILB_LOCK
	 * to protect against someone sneaking in during that window.
	 */

	bcopy(psip,csip,sizeof(struct security_info));

#if SEC_ARCH
	/*
	 * Do process tag inheritance-special case handling after bcopy.
	 * This is just a bunch of non-pendable tag copying.
	 */

	SP_OBJECT_CREATE(&psip->si_tag[0],&csip->si_tag[0],
		(tag_t *) 0,SEC_SUBJECT, (dac_t *) 0, (mode_t) 0);
#endif
}

#if SEC_ARCH
/*
 * sec_can_stat() returns a Boolean to indicate whether the calling
 * process is authorized to stat the object whose tags are pointed
 * to by target_tags.
 * MP Note:
 * We make a temporary copy of the tags because we don't wish to keep the
 * SIP tag pool locked across the pendable call to SP_ACCESS.
 */

sec_can_stat(target_tags)
tag_t *target_tags;
{
        tag_t temp_tags[SEC_TAG_COUNT];
        /* XXX lock SIP tag pool */
        bcopy(SIP->si_tag, temp_tags, sizeof(tag_t) * SEC_TAG_COUNT);
        /* XXX unlock SIP tag pool */
        return (SP_ACCESS(temp_tags, target_tags, SP_STATACC, NULL) == 0);
}


/*
 * return 1 if the calling process can signal the specified process
 */

sec_can_kill(p)
struct proc *p;
{
      if (!security_is_on)
		return(1);
      if (p == u.u_procp)     /* you can always signal yourself */
              return(1);
      return (SP_ACCESS(SIP->si_tag, secinfo[p - proc].si_tag,
                              SP_KILLACC, NULL) == 0);
}

#endif	/* SEC_ARCH */


/*
 * Security related dead vnodeops which simply send a signal to the
 * calling process.  No need to check locking.  The vnode better not
 * be undergoing any transitions in type.  It's already dead if it got
 * here, so this should be a safe assumption.
 */
sec_dead_read(vp, uio, ioflag, cred)
	struct vnode *vp;
	struct uio *uio;
	int ioflag;
	struct ucred *cred;
{
	return(sec_dead_hangup(vp));
}

sec_dead_write(vp, uio, ioflag, cred)
	register struct vnode *vp;
	struct uio *uio;
	int ioflag;
	struct ucred *cred;
{
	return(sec_dead_hangup(vp));
}

sec_dead_ioctl(vp, com, data, fflag, cred)
	struct vnode *vp;
	register int com;
	caddr_t data;
	int fflag;
	struct ucred *cred;
{
	return(sec_dead_hangup(vp));
}

/* 
 * common code for sec_dead_read(), sec_dead_write(), and sec_dead_ioctl()
 */

sec_dead_hangup(vp)
struct vnode *vp;
{
	unix_master();
	psignal(u.u_procp, SIGHUP);
	unix_release();
	return (0);
}

/*
 * Following routines are an attempt to localize the places that know
 * about the "bogus memory" define "BM" that OSF uses.  The idea is
 * that some architectures can't do atomic word transfers.  In an MP
 * world, that leaves the possibility of getting bad data.  Cluttering
 * all the code with "BM" locking macros is ugly and gets uglier as
 * code grows, so we'll try to avoid it for now.  See security.h for
 * equivalent macros for non-BM systems.
 */

#ifdef BM
vnode_type(vp)			/* BM read of v_type field of struct vnode */
struct vnode *vp;
{
	int v_type;

	BM(VN_LOCK(vp));
	v_type = vp->v_type;
	BM(VN_UNLOCK(vp));
	return(v_type);
}

fp_flags(fp)			/* BM read of f_flag field of struct file */
struct file *fp;
{
	int f_flag;

	BM(FP_LOCK(fp));
	f_flag = fp->f_flag;
	BM(FP_UNLOCK(fp));
	return(f_flag);
}

mp_flags(mp)			/* BM read of m_flag field of struct mount */
struct mount *mp;
{
	int m_flag;

	BM(MP_LOCK(mp));
	m_flag = mp->m_flag;
	BM(MP_UNLOCK(mp));
	return(m_flag);
}

process_flags(p)		/* BM read of p_flag field of struct proc */
struct proc *p;
{
	int p_flag;

	BM(PROC_LOCK(p));
	p_flag = p->p_flag;
	BM(PROC_UNLOCK(p));
	return(p_flag);
}

#endif

#endif /* SEC_BASE */
