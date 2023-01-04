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
static char	*sccsid = "@(#)sec_calls.c	9.3	(ULTRIX/OSF)	10/23/91";
#endif 
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0
 */
/*
 * Copyright (c) 1988-1990 SecureWare, Inc.  All rights reserved.
 *
 * This module contains proprietary information of SecureWare, Inc.
 * and should be treated as confidential.
 * 
 * sec/sec_calls.c (functions related to the security() system call)
 */

#include <sec/include_sec>
#include <sys/sec_objects.h>
#include <sys/syscall.h>

/*
 * This file contains the code for the security() system call and
 * all functions it implements (the routines declared immediately
 * below this comment).
 */


#if SEC_BASE

int stopio(), getluid(), setluid(), statpriv(), chpriv();
int security_switch();
#if SEC_ARCH
int eaccess(), setlabel(), getlabel(), lmount(), islabeledfs();
#endif
#if SEC_MAC
int mkmultdir(), rmmultdir(), ismultdir();
#endif

extern struct msqid_ds *msgconv(), msgque[];
extern struct semid_ds *semconv(), sema[];
extern struct shmid_internal *shmconv(), shmem[];

/*
 * The dispatch for the syscall now lives in data/sec_data.c
 */

#if SEC_ARCH
/* 
 * Return 1 if vp is for a file on a secure file system.
 * Note that NFS-mounted secure file systems are NOT considered secure.
 */

is_secure_filesystem(vp)
register struct vnode *vp;
{
	return(VHASSECOPS(vp) && VSECURE(vp));
}
#endif

/*
 * Determine whether a file system is mounted read only
 */

is_read_only_filesystem(vp)
register struct vnode *vp;
{
	return((vp->v_mount->m_flag & M_RDONLY) != 0);
}

/*
 * Return true if the currently executing process is the owner of
 * the file whose vnode is pointed to by vp, either explicitly or
 * through possession of the SEC_OWNER privilege.
 */

sec_file_owner(vp)
struct vnode *vp;
{
	int owns_file; 
	struct vattr vattr;
	int error;

	VOP_GETATTR(vp, &vattr, u.u_cred, error);
	if ( error == 0) {
		if (!sec_owner(vattr.va_uid, vattr.va_uid))
			error = EPERM;
	}
	return(error);
}

/*
 * These are definitions for the "restrictions" bit mask of path_to_vp().
 */

#define		SEC_FS_ONLY	    0x001 /* must be on secure file system  */
#define		SEC_RW_FS_ONLY      0x002 /* SEC_FS_ONLY mounted read/write */
#define		SEC_OWNER_ONLY	    0x004 /* caller must own file	    */

/*
 * Get a vnode pointer for the file whose pathname is pointed to by path.
 * Also do common checking for read-only file systems, file systems
 * that aren't secure, etc.  Such code tends to be very machine-specific,
 * so we don't wish to replicate it many times.
 */


struct vnode *
path_to_vp(path, namei_flags, restrictions, access, error_code)
char *path;
int namei_flags, restrictions, access;
int *error_code;
{
	register struct nameidata *ndp = &u.u_nd;
	struct vnode *vp;
	ndp->ni_nameiop = LOOKUP | namei_flags;
	ndp->ni_segflg = UIO_USERSPACE;
	ndp->ni_dirp = path;
	if (*error_code = namei(ndp))
		return(NULL);
	vp = ndp->ni_vp;
	ASSERT(vp);

	/*
	 * The file exists, and we've got a vnode pointer to it.
	 * Many operations don't make sense on file systems that
	 * aren't secure, or that are mounted read-only.
	 * Enforce these restrictions here, so that our callers don't
	 * have to fill their code with #ifdefs for each OS.
	 * Each of these routines sets appropriate audit flags.
	 */ 

	if (restrictions) {
#if SEC_ARCH
		if (restrictions & (SEC_FS_ONLY | SEC_RW_FS_ONLY))
			if (!is_secure_filesystem(vp)) {
				*error_code = EINVAL;
				goto error;
			}
#endif

		if (restrictions & SEC_RW_FS_ONLY)
			if (is_read_only_filesystem(vp)) {
				*error_code = EROFS;
				goto error;
			}

		if (restrictions & SEC_OWNER_ONLY)
			if (*error_code = sec_file_owner(vp))
				goto error;
	}

#if SEC_ARCH
	/* Does caller have the appropriate type of access to this file ? */

	if (access) {
		/*
		 * This should really be made table driven some day...
		 * Note that some callers require multiple types of access.
		 */

		if (access & ~(SP_STATACC|SP_SETATTRACC))
			panic("path_to_vp: unknown access check");

		if (access & SP_SETATTRACC)
			if (!has_file_access(vp, SP_SETATTRACC, error_code))
				goto error;

		if (access & SP_STATACC)
			if (!has_file_access(vp, SP_STATACC, error_code))
				goto error;
	}
#endif

	return(vp);

	/*
	 * Something failed in one of the sanity checks above.
	 * Just release the vnode pointer and tell our caller the bad news.
	 */

error:	vrele(vp);
	return(NULL);
}

/*
 * Return a Boolean indicating whether the caller has "access" access
 * to the file associated with vp.
 */

has_file_access(vp, access, error)
struct vnode *vp;
int access;
int *error;
{
	VOP_ACCESS(vp, access, u.u_cred, *error);
	return(*error == 0);
}

/*
 * Get a copy of the security_info structure for a process.
 * statpriv() and getlabel() are the only callers.
 * This is somewhat inefficient, in that callers always want just a
 * piece of the ~90 byte structure, but MP life is easier if we can localize
 * the locking needed to access the SIP.  If performance ever becomes a
 * problem, this could be modified to include an offset and byte count.
 */

int 
get_process_sip(pid, buffer)
struct security_info *buffer;
{
	struct security_info *rsip;

	/*
	 * Get pointer to the secinfo structure for the specified pid.
	 */

	if (pid == u.u_procp -> p_pid) {
		rsip = SIP;
	} else {
		register struct proc	*p;
		p = pfind(pid);

		if (p == (struct proc *) 0)
			return(ESRCH);
		rsip = &secinfo[p - proc];

		/*
		 * If asking for another process's privileges, must be
		 * owner and have SP_STATACC access.
		 */
		if (!sec_owner(p->p_rcred->cr_uid, p->p_rcred->cr_uid))
			return(EPERM);
#if SEC_ARCH
		if (!sec_can_stat(rsip->si_tag))
			return(EACCES);
#endif
	}
	/* XXX LOCK SIP */
	bcopy(rsip, buffer, sizeof(struct security_info));
	/* XXX UNLOCK SIP */
	return(0);
}

#if SEC_ARCH
/*
 * Copy a policy's tag for a file into the specified buffer.
 * If this is not a tagged filesystem, use the global
 * tags associated with the mount structure.
 * Otherwise, call the filesystem-specific code to
 * retrieve the desired tag from the file.
 */

tag_t
get_file_tag(vp, policy, tagnum, error)
struct vnode *vp;
int policy, tagnum;
int *error;
{
	struct vsecattr			vsattr;
	if (!VSECURE(vp)) {
		vsattr.vsa_tag =
		    vp->v_mount->m_tag[OBJECT_TAG(policy, tagnum)];
	} else {
		vsattr.vsa_valid = VSA_TAG;
		vsattr.vsa_policy = policy;
		vsattr.vsa_tagnum = tagnum;
		VOP_GETSECATTR(vp, &vsattr, u.u_cred, *error);
	}
	return(vsattr.vsa_tag);
}
#endif /* SEC_ARCH */



/*
 * Mark all open file entries in the file table that have the filename
 * argument open.  Any further reads, writes, or ioctls to the open
 * file (obtained through open, creat, dup, ...) will be considered an
 * error.  Future opens will work until stopio() is called again.
 * For now, stopio() only works on character special files and not
 * on remote files.  This system call returns -1 (and appropriate
 * errno) on error, and otherwise returns the number of file entries
 * disabled.  This call may only be invoked by the superuser or the
 * owner of the file.
 *
 * Note that there is no need to catch open references to /dev/tty for the
 * terminal in question.  That is because /dev/tty cannot write a terminal
 * once the process group of the terminal changes from the process group
 * having /dev/tty open.  Since stopio is meant to be called on session
 * transitions, the /dev/tty caveat is already done for us.
 * MP note:
 * There are potential race conditions here between stopio(2) and
 * io_allowed().  open(2), close(2), dup(2), etc. could be occurring
 * simultaneously.  Since stopio() is defined in such a way that
 * subsequent opens are permitted to succeed, this doesn't introduce a new
 * problem, and is is harmless in any practical sense.
 */

stopio(p, args, retval)
	struct proc *p;
	void *args;
	long *retval;
{
	register struct args {
		char *filename;
	} *uap = (struct args *) args;
	register struct vnode		*vp;
	extern struct vnodeops		sec_deadvnops;
	int error = 0;
	static char aud_parm[] = "aB000000";

	/* Look up the specified pathname and get a pointer to its vnode. */

	vp = path_to_vp(uap -> filename, FOLLOW, SEC_OWNER_ONLY, 0, &error);

	/*
	 * Verify that it is a character device.
	 * Call clearalias() to shut off access to all vnodes
	 * referencing this device.  Attach the sec_deadvnops
	 */

	if (! error) {
		ASSERT(vp);
		if (IS_CHAR_DEV(vp))
			clearalias(vp, &sec_deadvnops);
		else error = ENOTTY;

		vrele(vp);
	}
	AUDIT_CALL(SYS_security, error, ((long *)args)-1, *retval, AUD_HPR,
			aud_parm);
	return(error);

}


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
	int error = 0;
	static char aud_parm[] = "a0000000";

	BM(PROC_LOCK(p));
	*retval = p->p_auid;
	BM(PROC_UNLOCK(p));
	if (*retval == NOUID)
		error = EPERM;
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
 *
 * MP Note: Locking here is mostly overkill, except on BM systems.
 * Otherwise, we could really get away with just being careful about
 * the order of setting si_luid and si_luid_set.
 */

setluid(p, args, retval)
	struct proc *p;
	void *args;
	long *retval;
{
	register struct args {
		long uid;
	} *uap = (struct args *) args;
	int error = 0;
	uid_t oauid;
	static char aud_parm[] = "al000000";

	if (privileged(SEC_SETPROCIDENT, EPERM))
	{
#ifdef MAXUID
		if (uap->uid >= MAXUID) {
			error = EINVAL;
		} else
#endif
		{
			PROC_LOCK(p);
			oauid = p->p_auid;
			if (oauid==NOUID || privileged(SEC_SUSPEND_AUDIT, 0))
				p->p_auid = uap->uid;
			else
				error = EPERM;
			PROC_UNLOCK(p);
		}
	} else
		error = EPERM;
	if (!error)
		*retval = oauid;
	AUDIT_CALL(SYS_security, error, ((long *)args)-1, *retval, AUD_HPR,
		aud_parm);
	return(error);
}

/*
 * This entry point obtains the privileges on any object, including
 * other processes (normally viewed as subjects).  The user must
 * own the object and have SP_STATACC access to it.
 *
 * MP note: Races related to privilege changes are unavoidable,
 * even on a uniprocessor system.  All we can guarantee is that
 * the privilege vector we return is consistent at some given moment
 * in time.  We also need to be careful about races between pfind() and
 * process termination if the target of the statpriv() is a process.
 */
statpriv(p, args, retval)
	struct proc *p;
	void *args;
	long *retval;
{
	register struct args {
		long	privtype;
		priv_t	vec;
		long	objtype;
		obj_t	*objid;
	} *uap = (struct args *) args;
	priv_t				*vec;
	privvec_t			temp_vec;
	obj_t				kobj;
	register struct security_info	*rsip;
	struct security_info		temp_sip;
	int				error = 0;
	char				*aud_parm;
#if SEC_PRIV
	register struct vnode		*vp;
	struct vsecattr			vsattr;
#endif /* SEC_PRIV */
	static char *aud_parms[] = {
		"aaeau000",	/* unknown args */
		"aaeaq000",	/* OT_PROCESS */
		"aaeaB000"	/* OT_REGULAR */
	};

	aud_parm = aud_parms[0];	/* start off as unknown type */

	if (copyin(uap->objid, &kobj, sizeof kobj)) {
		error = EFAULT;
		goto out;
	}
	
	switch (uap->objtype) {

	    case OT_PROCESS:
	    {
		int pid = kobj.o_pid ? kobj.o_pid : u.u_procp -> p_pid;

		aud_parm = aud_parms[1];	/* known type is pid */

		if (!security_is_on) {
			/* save time, we know what vec is going to be */
			extern privvec_t su_all_privs;
			vec = su_all_privs;
			break;
		}

	  	error = get_process_sip(pid, &temp_sip);
		if (error)
			break;
		rsip = &temp_sip;
		switch (uap->privtype) {
		    case SEC_POTENTIAL_PRIV:
			vec = rsip->si_spriv;
			break;
		    case SEC_MAXIMUM_PRIV:
			vec = rsip->si_mpriv;
			break;
		    case SEC_BASE_PRIV:
			vec = rsip->si_bpriv;
			break;
		    case SEC_EFFECTIVE_PRIV:
			vec = rsip->si_epriv;
			break;
		    default:
			error = EINVAL;
			break;
		}
		break;
	    }
#if SEC_PRIV
	    case OT_REGULAR:

		aud_parm = aud_parms[2];	/* known type is file */

		/*
		 * Lookup the pathname and get a vnode pointer.
		 */
		vp = path_to_vp(kobj.o_file, FOLLOW,
			SEC_FS_ONLY, SP_STATACC, &error);
		if (vp == NULL)
			break;

		/*
		 * Retrieve the privilege vectors associated with the vnode.
		 */
		vsattr.vsa_valid = VSA_GPRIV | VSA_PPRIV;
		VOP_GETSECATTR(vp, &vsattr, u.u_cred, error);
		vrele(vp);
		if (error)
			break;

		switch (uap->privtype) {
		    case SEC_POTENTIAL_PRIV:
			vec = vsattr.vsa_ppriv;
			break;
		    case SEC_GRANTED_PRIV:
			vec = vsattr.vsa_gpriv;
			break;
		    default:
			error = EINVAL;
			break;
		}
		break;
#endif /* SEC_PRIV */

	    default:
		/*
		 * Privileges are not supported on other types of objects.
		 */
		error = EINVAL;
		break;
	}

	/*
	 * Return the vector to the user, making a temporary copy of it
	 * first to avoid possible page faults in copyout() whlie holding
	 * locks.
	 */

	if (! error) {
		SIP_ATTR_LOCK();
		bcopy(vec, temp_vec, sizeof(privvec_t));
		SIP_ATTR_UNLOCK();
		if (copyout(temp_vec, uap->vec, sizeof(privvec_t)))
			error = EFAULT;
	}
out:
	AUDIT_CALL(SYS_security, error, ((long *)args)-1, *retval, AUD_HPR,
			aud_parm);
	return(error);
}


/*
 * This entry point resets the privileges on any object, including
 * other processes (normally viewed as subjects).  The user must
 * have write access to the object before being able to change the
 * privileges.
 */
chpriv(p, args, retval)
	struct proc *p;
	void *args;
	long *retval;
{
	register struct args {
		long	privtype;
		caddr_t	vec;
		long	objtype;
		caddr_t	objid;
	} *uap = (struct args *) args;
	register int			i;
	privvec_t			new;
	obj_t				kobj;
	int				error = 0;
	char				*aud_parm;
	static char *aud_parms[] = {
		"aauau000",	/* unknown type */
		"aauaq000",	/* OT_PROCESS */
		"aauaB000"	/* OT_REGULAR */
	};

	if (!security_is_on)
		return(0);

	aud_parm = aud_parms[0];	/* start as unknown type */

	if (copyin(uap->objid, &kobj, sizeof kobj) ||
	    copyin(uap->vec, new, sizeof new)) {
		error = EFAULT;
		goto out;
	}

	/*
	 * Do a bit of general sanity checking on the arguments.
	 * The target of chpriv() on a process must be the caller's PID.
	 * chpriv() on a file must specify either the granted or
	 * potential privilege set, and be done by a caller who has
	 * the SEC_CHPRIV privilege.
	 */

	switch(uap->objtype) {
	    case OT_PROCESS:
		aud_parm = aud_parms[1];	/* known type is pid */
		if (kobj.o_pid != 0 && kobj.o_pid != u.u_procp->p_pid)
			error = EINVAL;
		break;
	    case OT_REGULAR:
		aud_parm = aud_parms[2];	/* known type is file */
		if ((uap->privtype != SEC_GRANTED_PRIV) &&
		    (uap->privtype != SEC_POTENTIAL_PRIV))
			error = EINVAL;
		break;
	    default:	/* trying to chpriv something of unknown type */
		error = EINVAL;
	}

	if (error)
		goto out;

	/* The following code handles chpriv() for a process. */

	if (uap->objtype == OT_PROCESS) {

		SIP_ATTR_LOCK();
		switch (uap->privtype) {
		    case SEC_BASE_PRIV:
			error = chpriv_proc_base_priv(new);
			break;
		    case SEC_EFFECTIVE_PRIV:
			error = chpriv_proc_effective_priv(new);
			break;
		    case SEC_MAXIMUM_PRIV:
			error = chpriv_proc_max_priv(new);
			break;
		    default:
			error = EINVAL;
		}
		SIP_ATTR_UNLOCK();
	}

#if SEC_PRIV /*{*/
	/* * The following code handles chpriv() for a file. */

	if (uap->objtype == OT_REGULAR) {

		struct vnode *vp = (struct vnode *) NULL;

		if (!privileged(SEC_CHPRIV, 0)) {
			error = EPERM;
			goto chpriv_file_error;
		}
			
		/*
		 * Lookup the pathname and get a vnode pointer.
		 */

		vp = path_to_vp(kobj.o_file, FOLLOW,
			SEC_RW_FS_ONLY | SEC_OWNER_ONLY, SP_SETATTRACC, &error);
				
		if (vp == NULL)
			goto out;

		/*
		 * Don't allow chpriv on directories, sockets, special
		 * files, etc.  Doing so wouldn't really hurt, but it
		 * also wouldn't make much sense.
		 */

		if (!IS_REG_FILE(vp)) {
			error = EINVAL;
			goto chpriv_file_error;
		}
			
		/* Try to modify the appropriate privilege vector of file */

		if (uap->privtype == SEC_GRANTED_PRIV)
			error = chpriv_file_granted_priv(vp, new);
		else
			error = chpriv_file_potential_priv(vp, new);

chpriv_file_error:
		if (vp)
			vrele(vp);
	}
#endif /*} SEC_PRIV */
out:
	AUDIT_CALL(SYS_security, error, ((long *)args)-1, *retval, AUD_HPR,
			aud_parm);
	return(error);
}

/*
 * Try to change the base privilege vector for the calling process.
 * This is called only by chpriv().
 * MP Note: Caller must have done SIP_ATTR_LOCK()
 */

chpriv_proc_base_priv(new)
register priv_t *new;
{
	register struct security_info *rsip = SIP;
	register int i;

	/* Can't add anything not in (M intersect P) */

	for (i = 0; i < SEC_SPRIVVEC_SIZE; ++i)
		if (new[i] & ~((rsip->si_mpriv[i] &
				rsip->si_spriv[i]) |
				rsip->si_bpriv[i]))
			return(EPERM);

	/* Set the new vector, adjusting effective. */

	for (i = 0; i < SEC_SPRIVVEC_SIZE; ++i) {
		rsip->si_bpriv[i] = new[i];
		rsip->si_epriv[i] &= rsip->si_bpriv[i] |
				     rsip->si_spriv[i];
	}
	return(0);
}

/*
 * Try to change the effective privilege vector for the calling process.
 * This is called only by chpriv().
 * MP Note: Caller must have done SIP_ATTR_LOCK()
 */

chpriv_proc_effective_priv(new)
register priv_t *new;
{
	register struct security_info *rsip = SIP;
	register int i;

	/* Must not exceed (B union P) */

	for (i = 0; i < SEC_SPRIVVEC_SIZE; ++i)
		if (new[i] & ~(rsip->si_bpriv[i] | rsip->si_spriv[i]))
			return(EPERM);

	/* Set the new vector. */

	for (i = 0; i < SEC_SPRIVVEC_SIZE; ++i)
		rsip->si_epriv[i] = new[i];
	return(0);
}

/*
 * Try to change the max privilege vector for the calling process.
 * This is called only by chpriv().
 * MP Note: Caller must have done SIP_ATTR_LOCK()
 */

chpriv_proc_max_priv(new)
register priv_t *new;
{
	register struct security_info *rsip = SIP;
	register int i;

	/* Must not increase. */

	for (i = 0; i < SEC_SPRIVVEC_SIZE; ++i)
		if (new[i] & ~rsip->si_mpriv[i])
			return(EPERM);

	/* Set the new vector, adjusting base and effective. */

	for (i = 0; i < SEC_SPRIVVEC_SIZE; ++i) {
		rsip->si_mpriv[i] = new[i];
		rsip->si_bpriv[i] &= rsip->si_mpriv[i];
		rsip->si_epriv[i] &= rsip->si_bpriv[i] |
				     rsip->si_spriv[i];
	}
	return(0);
}

#if SEC_PRIV

/*
 * Change the granted privilege vector for a file.
 */

chpriv_file_granted_priv(vp, new)
struct vnode *vp;
register priv_t *new;
{
	struct vsecattr			vsattr;
	int error;
	int i;

	/*
	 * Must not exceed P
	 * Retrieve P in order to perform check
	 * XXX Need to make get/set atomic for MP
	 */

	vsattr.vsa_valid = VSA_PPRIV;
	if (VOP_GETSECATTR(vp, &vsattr, u.u_cred, error))
		return(error);

	for (i = 0; i < SEC_SPRIVVEC_SIZE; ++i) {
		if (new[i] & ~vsattr.vsa_ppriv[i])
			return(EPERM);
		vsattr.vsa_gpriv[i] = new[i];
	}
	vsattr.vsa_valid = VSA_GPRIV;
	VOP_SETSECATTR(vp, &vsattr, u.u_cred, error);
	return(error);
}

/*
 * Change the potential privilege vector for a file.
 */

chpriv_file_potential_priv(vp, new)
struct vnode *vp;
register priv_t *new;
{
	register struct security_info *rsip = SIP;
	struct vsecattr			vsattr;
	int error;
	int i;

	vsattr.vsa_valid = VSA_GPRIV | VSA_PPRIV;
	VOP_GETSECATTR(vm, &vsattr, u.u_cred, error);
	if (error)
		return(error);

	/* Must not exceed M  */

	SIP_ATTR_LOCK();
	for (i = 0; i < SEC_SPRIVVEC_SIZE; ++i) {
		if (new[i] & ~rsip->si_mpriv[i]) {
			SIP_ATTR_UNLOCK();
			return(EPERM);
		}
		vsattr.vsa_ppriv[i] = new[i];
	}
	
	/* can't ever let gpriv exceed ppriv */

	vsattr.vsa_gpriv[i] &= new[i];
	vsattr.vsa_ppriv[i]  = new[i];

	SIP_ATTR_UNLOCK();

	VOP_SETSECATTR(vp, &vsattr, u.u_cred, error);
	return(error);
}
#endif

#if SEC_ARCH

/*
 * Return the effective access on the file.  Take into account all
 * conditions, including a read-only file system and all security
 * policies.
 */
eaccess(p, args, retval)
	struct proc *p;
	void *args;
	long *retval;
{
	register struct args {
		char	*filename;
		long	mode;
	} *uap = (struct args *) args;
	register u_short		fmode;
	register struct vnode		*vp;
	int				error = 0;
	static char aud_parm[] = "aBa00000";

	/*
	 * Lookup pathname and get pointer to its vnode.
	 */

	vp = path_to_vp(uap -> filename, FOLLOW, 0, 0, &error);
	if (vp == NULL)
		goto out;

	fmode = 0;
	if (uap->mode) {
		if (uap->mode & R_OK)
			fmode |= VREAD;
		if (uap->mode & W_OK) {
			fmode |= VWRITE;
			error = vn_writechk(vp);
		}
		if (uap->mode & X_OK)
			fmode |= VEXEC;
		if (error == 0)
			(void) has_file_access(vp, fmode, &error);
	}
	vrele(vp);
out:
	AUDIT_CALL(SYS_security, error, ((long *)args)-1, *retval, AUD_HPR,
			aud_parm);
	return(error);
}

#endif /* SEC_ARCH */


#if SEC_MAC

/*
 * Set or clear the multilevel attribute of a directory.  The directory
 * must already exist and be empty, and the caller must own it and have
 * execute and write access to it and must have the MULTILEVELDIR privilege.
 * This is the heart of mkmultdir() and rmmultdir() and is only called
 * by them.
 */
static
chmultdir(args, flag)
	void *args;
	int	flag;
{
	register struct args {
		char	*filename;
	} *uap = (struct args *) args;
	register struct nameidata	*ndp = &u.u_nd;
	register struct vnode		*vp, *dvp;
	int	error;

	/* Verify caller's privilege. */

	if (!privileged(SEC_MULTILEVELDIR, EPERM))
		return(EPERM);

	/*
	 * Lookup the pathname and get a vnode pointer as well as a
	 * reference to its parent directory for the VOP_DIREMPTY check.
	 */

	ndp->ni_nameiop = LOOKUP | WANTPARENT | FOLLOW;
	ndp->ni_segflg = UIO_USERSPACE;
	ndp->ni_dirp = uap->filename;
	if (error = namei(ndp))
		return(error);
	vp = ndp->ni_vp;
	dvp = ndp->ni_dvp;

	if (!is_secure_filesystem(vp)) {
		error = EINVAL;
		goto out;
	}

	if (is_read_only_filesystem(vp)) {
		error = EROFS;
		goto out;
	}

	/*
	 * Caller must own the target directory and have SP_EXECACC and
	 * SP_SETATTRACC access to it.
	 */

	if ((error = sec_file_owner(vp)) 
		|| !has_file_access(vp, SP_EXECACC, &error)
		|| !has_file_access(vp, SP_SETATTRACC, &error))
		goto out;
		
	/*
	 * XXX This should become VOP_MULTDIR but I don't want to
	 * recompile the world right now.
	 */

	error = ufs_chmultdir(vp, dvp, flag, u.u_cred);
out:
	vrele(vp);
	vrele(dvp);
	return(error);
}

/* 
 * Make a regular directory into a multilevel directory
 */
mkmultdir(p, args, retval)
	struct proc *p;
	void *args;
	long *retval;
{
	return chmultdir(args, 1);
}

rmmultdir(p, args, retval)
	struct proc *p;
	void *args;
	long *retval;
{
	return chmultdir(args, 0);
}

/*
 * Return 1 if the file is a multilevel directory, 0 if not, and
 * -1 if permission to the directory is not granted.
 */
ismultdir(p, args, retval)
	struct proc *p;
	void *args;
	long *retval;
{
	register struct args {
		char	*filename;
	} *uap = (struct args *) args;
	int error;
	int result = get_inode_type_flags(uap -> filename, &error);
	if (!error)
		*retval = (result & SEC_I_MLD) != 0;
	return(error);
}

/*
 * Return the vnode security attribute flags for the specified pathname.
 * On errors, NULL is returned.
 */

get_inode_type_flags(path, error)
char *path;
int *error;
{
	register struct vnode		*vp;
	struct vsecattr			vsattr;

	vp = path_to_vp(path, FOLLOW, SEC_FS_ONLY, SP_STATACC, error);
	vsattr.vsa_type_flags = 0;
	if (vp) {
		vsattr.vsa_valid = VSA_TYPE_FLAGS;
		VOP_GETSECATTR(vp, &vsattr, u.u_cred, &error);
		vrele(vp);
	}
	return(vsattr.vsa_type_flags);

}

#endif /* SEC_MAC */


#if SEC_ARCH

/*
 * Retrieve the internal representation for a security label.  The policy
 * argument indicates which of the configured policies the desired label
 * belongs to, and the tagnum argument specifies which of the labels
 * managed by that policy is desired.  The objtype argument indicates
 * whether a subject or object label is sought and serves as the discriminant
 * for the objid union.  The attr argument describes the user space buffer
 * into which the IR is to be stored.
 */
getlabel(p, args, retval)
	struct proc *p;
	void *args;
	long *retval;
{
	register struct args {
		u_long	policy;
		u_long	tagnum;
		attr_t	*attr;
		long	objtype;
		obj_t	*objid;
	} *uap = (struct args *) args;
	obj_t			kobj;
	register tag_t		*tags;
	register u_int		policy = uap->policy,
				tagnum = uap->tagnum;
	tag_t			tag_to_check;
	int	error;

	if (!security_is_on)
		return(EACCES);

	/* Make sure the policy number and tag offset are valid. */

	if (policy >= spolicy_cnt ||
	    tagnum >= (uap->objtype == OT_PROCESS ? SUBJECT_TAG_COUNT(policy)
					  : OBJECT_TAG_COUNT(policy))) {
		return(EINVAL);
	}

	/* Get the handle for the object (or subject) whose label is sought. */

	if (copyin(uap->objid, &kobj, sizeof kobj))
		return(EFAULT);

	/* Make tagnum relative to the base of the tag pool */

	if (uap->objtype == OT_PROCESS)
		tagnum = SUBJECT_TAG(policy, tagnum);
	else
		tagnum = OBJECT_TAG(policy, tagnum);

	/* Switch on type of entity whose attributes we're interested in. */

	switch(uap->objtype) {

	    case OT_PROCESS:
	    {		
		struct security_info temp_sip;
		struct security_info *rsip;
		int pid = kobj.o_pid ? kobj.o_pid : p -> p_pid;
	  	if (error = get_process_sip(pid, &temp_sip))
			return(error);
		rsip = &temp_sip;
		tags = rsip->si_tag;
		tag_to_check = tags[tagnum];
		break;

	    }
	    case OT_MESSAGE_QUEUE:
	    {
		register struct msqid_ds	*msgq = msgconv(kobj.o_msgid);

		if (msgq == (struct msqid_ds *) 0)
			return(EINVAL);

		/* XXX Lock the message queue here */

		tags = MSGTAG(msgq, 0);
		tag_to_check = tags[tagnum];

		/* XXX Unlock the message queue here */

		break;
	    }

	    case OT_SHARED_MEMORY:
	    {
                register struct shmid_internal  *shm = shmconv(kobj.o_shmid);

                if (shm == (struct shmid_internal *) 0)
			return(EINVAL);

		/* XXX Lock the segment here */

		tags = SHMTAG(shm, 0);
		tag_to_check = tags[tagnum];

		/* XXX Unlock the segment here */

		break;
	    }

	    case OT_SEMAPHORE:
	    {
		register struct semid_ds	*sem = semconv(kobj.o_semid);

		if (sem == (struct semid_ds *) 0)
			return(EINVAL);

		/* XXX Lock the semaphore here */

		tags = SEMTAG(sem, 0);
		tag_to_check = tags[tagnum];

		/* XXX Unlock the semaphore here */

		break;
	    }


	    case OT_REGULAR:
	    case OT_SYMLINK:
	    {
		register struct vnode		*vp;
		struct vsecattr			vsattr;

		/* Lookup the pathname and get a vnode pointer.	 */
		vp = path_to_vp(kobj.o_file,
			 uap -> objtype == OT_SYMLINK ? 0 : FOLLOW,
			 0, SP_STATACC, &error);

		if (vp == NULL)
			return(error);

		tag_to_check = get_file_tag(vp, policy, uap->tagnum, &error);
		vrele(vp);

		if (error)
			return(error);

		break;
	    }

	    case OT_FILE_DESCR:
	    {
		struct file	*fp;
		char		poflags;
		
		/* XXX Potential race here between GETF and close(2) ?	 */

		if (error = getf(&fp, kobj.o_fdes, &poflags, &u.u_file_state))
			return(error);

		if ((fp->f_type != DTYPE_SOCKET) &&
			fp->f_type != DTYPE_VNODE) {
			FP_UNREF(fp);
			return(EINVAL);
		}
		if (fp->f_type == DTYPE_SOCKET) {
			tags = ((struct socket *) fp->f_data)->so_tag;
			tag_to_check = tags[tagnum];
			FP_UNREF(fp);
			break;
		}
		if (fp->f_type == DTYPE_VNODE) {
			register struct vnode	*vp;
			struct vsecattr		vsattr;
			
			vp = (struct vnode *) fp->f_data;

			/*
			 * Verify SP_STATACC access to the vnode since the
			 * file may have been opened for writing only.
			 */
			if (has_file_access(vp, SP_STATACC, &error))
				tag_to_check =
					 get_file_tag(vp, policy, uap->tagnum, &error);
			FP_UNREF(fp);
			if (error)
				return(error);
			break;
		}
	    }
	    default:
		return(EINVAL);
	}

	/*
	 * Almost home...  The caller must have SP_STATACC access to the
	 * tags of the object whose label is being read.  Most types were
	 * previously checked in get_process_sip() or  has_file_access(),
	 * but we want to do some special processing here.
	 */

	switch(uap->objtype) {
	    case OT_PROCESS:
	    case OT_REGULAR:
	    case OT_SYMLINK:
	    case OT_FILE_DESCR:	/* except DTYPE_SOCKET ??? XXX */
		break;
	    case OT_SHARED_MEMORY:
	    case OT_SEMAPHORE:
	    case OT_MESSAGE_QUEUE:	
	    {
		/* XXX Make a copy of tags */

		if (!sec_can_stat(tags))
		    return(EACCES);
	    }
		break;
	    default:
		panic("getlabel: unknown object type");
	}

	/*
	 * And finally, the moment of truth.
	 * Call the policy-specific tag-to-ir function.
	 */

	return(SP_GETATTR(policy,
		uap->objtype == OT_PROCESS ? SEC_SUBJECT : SEC_OBJECT,
		tag_to_check, uap->attr));
}

/*
 * Common function to change security labels of System V IPC objects.
 * Called only from setlabel() below
 * XXX locking between callers and here will be a pain due to long
 * pendable paths.
 */
static int
ipcsetlabel(objtype, objp, policy, tagnum, kattr)
	register int		objtype;
	register caddr_t	objp;
	int			policy, tagnum;
	register attr_t		*kattr;
{
	register struct ipc_perm	*perm;
	register tag_t			*tags;
	tag_t				new;
	udac_t				udac;
	register int			error = 0;

	ASSERT(objp);

	/*
	 * Get pointers to the object's ipc_perm structure
	 * and to its tag pool.
	 */
	switch (objtype) {
	    case OT_MESSAGE_QUEUE:
		perm = &((struct msqid_ds *) objp)->msg_perm;
		tags = MSGTAG((struct msqid_ds *) objp, 0);
		break;
	    case OT_SEMAPHORE:
		perm = &((struct semid_ds *) objp)->sem_perm;
		tags = SEMTAG((struct semid_ds *) objp, 0);
		break;
	    case OT_SHARED_MEMORY:
                perm = &((struct shmid_internal *) objp)->s.shm_perm;
                tags = SHMTAG((struct shmid_internal *) objp, 0);
		break;
	    default:
		panic("ipcsetlabel: impossible objtype");
	}

	/*
	 * Construct the standard structure describing the object's
	 * UNIX discretionary attributes.
	 */
	udac.cuid = perm->cuid;
	udac.cgid = perm->cgid;
	udac.uid = perm->uid;
	udac.gid = perm->gid;
	udac.mode = perm->mode & 0777;

	/*
	 * Verify that the caller has SP_SETATTRACC access to the object.
	 */
	if (error = SP_ACCESS(SIP->si_tag, tags, SP_SETATTRACC, &udac)) {
		return(error);
	}

	/*
	 * If the new label is not WILDCARD, map the IR to a tag
	 * POSIX ACLS  -- clear kattr->code for return value
	 */
	if (kattr->code == SEC_WILDCARD_TAG) {
		new = SEC_WILDCARD_TAG_VALUE;
		kattr->code = 0;
	} else {
		kattr->code = 0;
		error = SP_MAPTAG(policy, SEC_OBJECT, tagnum, kattr,
					&udac, &new);
	}

	/*
	 * Call the policy-specific function to check the validity
	 * of the new label based on other object-specific attributes.
	 */
	if (error == 0)
		error = SP_SETATTR_CHECK(policy, objtype, objp, 0,
			 tags, tagnum, new);

	/*
	 * Call the policy-specific function to assign the new label
	 * value to the process.
	 */
	if (error == 0)
		error = SP_SETATTR(policy, SEC_OBJECT, tags, tagnum, new);

	/*
	 * If the change was successful, update any of the object's
	 * UNIX discretionary attributes that may need to change to
	 * reflect the new label.
	 * XXX locking
	 * XXX SP_MAPTAG() doesn't currently pass back enough info
	 * to know whether we need to update these fields.  Fortunately,
	 * no currently supported policies require updating.
	 */
#ifdef notdef
	if (error == 0 && mapflags) {
		if (mapflags & SEC_NEW_UID)
			perm->uid = udac.uid;
		if (mapflags & SEC_NEW_GID)
			perm->gid = udac.gid;
		if (mapflags & SEC_NEW_MODE)
			perm->mode = udac.mode;
	}
#endif
        /*
         * POSIX ACLS  -- added the following check
         *                modified u.u_error to be error
         */
        if (error == 0) {
                perm->uid = udac.uid;
                perm->gid = udac.gid;
                perm->mode = (perm->mode & ~0777) | (udac.mode & 0777);
        }

	return(error);
}


/*
 * Change one of the security labels associated with a process or an object.
 * The policy argument indicates which of the configured policies the target
 * label belongs to, and the tagnum argument specifies which of the labels
 * managed by that policy is to be changed.  The objtype argument indicates
 * whether a subject or object label is being changed and serves as the
 * discriminant for the objid union.  The attr argument describes the user
 * space buffer from which the IR for the new label is to be read.
 */
setlabel(p, args, retval)
	struct proc *p;
	void *args;
	long *retval;
{
	register struct args {
		u_long	policy;
		u_long	tagnum;
		attr_t	*attr;
		long	objtype;
		obj_t	*objid;
	} *uap = (struct args *) args;
	tag_t			new;
	obj_t			kobj;
	attr_t			kattr;
	register u_int		policy = uap->policy,
				tagnum = uap->tagnum;
	int	error = 0;

	if (!security_is_on)
		return(EACCES);

	/*
	 * Make sure the policy number and tag offset are valid.
	 */
	if (policy >= spolicy_cnt ||
	    tagnum >= (uap->objtype == OT_PROCESS ? SUBJECT_TAG_COUNT(policy)
						  : OBJECT_TAG_COUNT(policy))) {
		return(EINVAL);
	}

	/*
	 * Fetch the handle for the object (or subject) to be
	 * affected, and the IR descriptor for the new label.
	 */
	if (copyin(uap->objid, &kobj, sizeof kobj) ||
	    copyin(uap->attr, &kattr, sizeof kattr)) {
		return(EFAULT);
	}

	/*
	 * Make sure the IR descriptor specifies an acceptable tag code
	 * and handle the WILDCARD tag code.
	 */
	if (kattr.code == SEC_WILDCARD_TAG)
		new = SEC_WILDCARD_TAG_VALUE;
	else if (kattr.code != SEC_ACTUAL_TAG) {
		return(EINVAL);
	}

	/*
	 * Handle each type of object (or subject) individually.
	 */
	switch(uap->objtype) {

	    case OT_PROCESS:

		/* Only permitted to set label on own process */

		if (kobj.o_pid == 0)
			kobj.o_pid = u.u_procp->p_pid;
		else if (kobj.o_pid != u.u_procp->p_pid) {
			return(EINVAL);
		}

		/*
		 * If the new label is not WILDCARD, map the IR to a tag
		 */
		if (kattr.code != SEC_WILDCARD_TAG &&
		    (error = SP_MAPTAG(policy, SEC_SUBJECT, tagnum, &kattr, 0, &new)) != 0)
		{
			return(error);
		}

		/*
		 * Call the policy-specific function to check the validity
		 * of the new label based on other process-specific attributes.
		 * Note: The acl policy does not allow OT_PROCESS.
		 */
		if (error = SP_SETATTR_CHECK(policy, uap->objtype, u.u_procp, 0,
					SIP->si_tag, tagnum, new))
			return(error);
		/*
		 * Call the policy-specific function to assign the new label
		 * value to the process.
		 */
		error = SP_SETATTR(policy, SEC_SUBJECT, SIP->si_tag, tagnum, new);
		break;

	    case OT_MESSAGE_QUEUE:
	    {
		register struct msqid_ds	*msgq = msgconv(kobj.o_msgid);

		if (msgq == (struct msqid_ds *) 0) {
			return(EINVAL);
		}

		/* XXX Lock the message queue here */

		error = ipcsetlabel(uap->objtype, msgq, policy, tagnum, &kattr);

		/*
		 * If the change succeeded, update the object's change time.
		 */
		if (error == 0)
			msgq->msg_ctime = SYSTEM_TIME;

		/* XXX Unlock the message queue here */

		break;
	    }

	    case OT_SHARED_MEMORY:
	    {
                register struct shmid_internal  *shm = shmconv(kobj.o_shmid);

                if (shm == (struct shmid_internal *) 0) {
			return(EINVAL);
		}

		/* XXX Lock the segment here */

		error = ipcsetlabel(uap->objtype, shm, policy, tagnum, &kattr);

		/*
		 * If the change succeeded, update the object's change time.
		 */
		if (error == 0)
                        shm->s.shm_ctime = SYSTEM_TIME;
		/* XXX Unlock the segment here */

		break;
	    }

	    case OT_SEMAPHORE:
	    {
		register struct semid_ds	*sem = semconv(kobj.o_semid);

		if (sem == (struct semid_ds *) 0)
			return(EINVAL);

		/* XXX Lock the semaphore set here */

		error = ipcsetlabel(uap->objtype, sem, policy, tagnum, &kattr);

		/*
		 * If the change succeeded, update the object's change time.
		 */
		if (error == 0)
			sem->sem_ctime = SYSTEM_TIME;

		/* XXX Unlock the semaphore set here */

		break;
	    }

	    case OT_REGULAR:
	    case OT_SYMLINK:
	    {
		register struct nameidata	*ndp = &u.u_nd;
		register struct vnode		*vp, *dvp;
		struct vattr			vattr;
		struct vsecattr			vsattr;
		udac_t				udac;
		int				error = 0;

		/*
		 * Lookup the pathname and get a vnode pointer for the
		 * target and a reference to its parent directory.
		 */
		ndp->ni_nameiop = LOOKUP | WANTPARENT;
		if (uap->objtype != OT_SYMLINK)
			ndp->ni_nameiop |= FOLLOW;
		ndp->ni_segflg = UIO_USERSPACE;
		ndp->ni_dirp = kobj.o_file;
		if (error = namei(ndp))
			return(error);
		vp = ndp->ni_vp;
		dvp = ndp->ni_dvp;

		if (!is_secure_filesystem(vp)) {
			error = EINVAL;
			goto out;
		}

		if (is_read_only_filesystem(vp)) {
			error = EROFS;
			goto out;
		}

		if (!has_file_access(vp, SP_SETATTRACC, &error))
			goto out;

		/*
		 * If the new label is not WILDCARD, map the IR to a tag
	 	 * POSIX ACLS -- Pass down the user, group and mode
	 	 * because they may contribute to the tag mapping
		 */
		if (kattr.code != SEC_WILDCARD_TAG) {
			VOP_GETATTR(vp, &vattr, u.u_cred, error);
			if (error)
				goto out;
			udac.cuid = SEC_NOCREATOR_UID;
                        udac.uid = vattr.va_uid;
                        udac.cgid = SEC_NOCREATOR_GID;
                        udac.gid = vattr.va_gid;
                        udac.mode = vattr.va_mode & 0777;

                        error = SP_MAPTAG(policy, SEC_OBJECT, tagnum, &kattr,
                                                &udac, &new);
		}
                /*
                 * Call the filesystem-specific function to set the new label.
                 * To reduce the risk of a mislabeled file in the event of
                 * a crash, we do a synchronous flush of any modified data
                 * blocks in the file before setting the new label, then
                 * synchronously flush the label after changing it.
                 *
                 * POSIX ACLS -- and the user, group and mode if they've
                 *               changed.  The user, group, and
                 *               mode are only changed if we're not setting a
                 *               WILDCARD tag.
                 *
                 * PORT NOTE:
                 * For systems undergoing formal evaluation, this is
                 * probably important to enable.  Otherwise, performance
                 * considerations may outweigh the small increase in safety.
                 */

                if (error == 0) {
                        vsattr.vsa_valid = VSA_TAG;
                        vsattr.vsa_policy = policy;
                        vsattr.vsa_tagnum = tagnum;
                        vsattr.vsa_parent = dvp;
                        vsattr.vsa_tag = new;

                        /*
                         * POSIX ACLS -- check for attributes that changed
                         */
                        if (kattr.code != SEC_WILDCARD_TAG) {
                                if (vattr.va_uid != udac.uid) {
                                        vsattr.vsa_valid |= VSA_UID;
                                        vsattr.vsa_uid = udac.uid;
                                }
                                if (vattr.va_gid != udac.gid) {
                                        vsattr.vsa_valid |= VSA_GID;
                                        vsattr.vsa_gid = udac.gid;
                                }
                                if (vattr.va_mode != udac.mode) {
                                        vsattr.vsa_valid |= VSA_MODE;
                                        vsattr.vsa_mode = udac.mode;
                                }
                        }

#if PARANOID
                        VOP_FSYNC(vp, FWRITE, u.u_cred, MNT_WAIT, error);
                        if (error == 0)
#endif
                                VOP_SETSECATTR(vp, &vsattr, u.u_cred, error);
#if PARANOID
                        if (error == 0)
                                VOP_FSYNC(vp, FWRITE, u.u_cred, MNT_WAIT,
                                          error);
#endif
                }

		/*
		 * If the change was successful, update any of the object's
		 * UNIX discretionary attributes that may need to change to
		 * reflect the new label.
		 * XXX SP_MAPTAG() doesn't currently pass back enough info
		 * to know whether we need to update these fields.
		 * Fortunately, no currently supported policies require
		 * updating.
		 */
#ifdef notdef
		if (error == 0 && mapflags) {
			/* Preserve the non-permission mode bits */
			udac.mode |= vattr.va_mode & ~0777;
			vattr_null(&vattr);
			if (mapflags & SEC_NEW_UID)
				vattr.va_uid = udac.uid;
			if (mapflags & SEC_NEW_GID)
				vattr.va_gid = udac.gid;
			if (mapflags & SEC_NEW_MODE)
				vattr.va_mode = udac.mode;
			VOP_SETATTR(vp, &vattr, u.u_cred, error);
		}
#endif

out:
		vrele(vp);
		if (vp != dvp)
			vrele(dvp);

		break;
	    }

	    default:
		error = EINVAL;
	}
	return(error);
}


/*
 * Mount an untagged filesystem.  The labels specified (in IR form)
 * in the attrs array are applied globally to all files residing on
 * the volume to be mounted.
 */
lmount(p, args, retval)
	struct proc *p;
	void *args;
	long *retval;
{
	register struct args {
		long	type;
		caddr_t	dir;
		long	flags;
		caddr_t	data;
		attr_t	*attrs;
	} *uap = (struct args *) args;
	register int	i, policy, tagnum;
	attr_t		attrs[SEC_TAG_COUNT];
	tag_t		tags[SEC_TAG_COUNT];
	int	error = 0;

	/* Need all privileges to do a labeled mount */

	if (uap->attrs) {
		if (!privileged(SEC_ALLOWDACACCESS, EPERM)
#if SEC_MAC
		 || !privileged(SEC_ALLOWMACACCESS, EPERM)
#endif
		   )
			return(EPERM);
	}

	/* For labeled mount, get the attributes and map them to tags */

	if (uap->attrs != (attr_t *) NULL) {
	    if (copyin(uap->attrs, attrs, sizeof attrs)) {
		return(EFAULT);
	    }

	    for (i = 0; i < SEC_TAG_COUNT && error == 0; ++i) {
		/*
		 * If no attribute specified for this slot,
		 * fill in a wildcard tag value.
		 */
		if (attrs[i].code != SEC_ACTUAL_TAG) {
			tags[i] = SEC_WILDCARD_TAG_VALUE;
			continue;
		}

		/*
		 * Convert tag pool offset to policy, tagnum.
		 */
		for (policy = 0; policy < spolicy_cnt; ++policy)
			if (i >= sp_switch[policy].sw_first_obj_tag &&
			    i < sp_switch[policy].sw_first_obj_tag
					+ sp_switch[policy].sw_obj_count) {
				tagnum = i - sp_switch[policy].sw_first_obj_tag;
				break;
			}

		/*
		 * Error if no policy has claimed this tag pool slot.
		 */
		if (policy >= spolicy_cnt) {
			error = EINVAL;
			break;
		}
		error = SP_MAPTAG(policy, SEC_OBJECT, tagnum, &attrs[i],
					(udac_t *) 0, &tags[i]);
	    }
		if (error == 0) 
			error = mount1(p, uap, retval, tags);
	} else 
		error = mount1(p, uap, retval, (tag_t *) NULL);
	
	return(error);
}


/*
 * Determine if the specified file resides on a labeled filesystem.
 */
islabeledfs(p, args, retval)
	struct proc *p;
	void *args;
	long *retval;
{
	register struct args {
		char	*filename;
	} *uap = (struct args *) args;
	int error;

	/*
	 * Lookup the pathname and get a vnode.
	 */
	struct vnode *vp = path_to_vp(uap -> filename, FOLLOW, 0, 0, &error);
	if (vp == NULL) {
		return(0);
	}
	if (VSECURE(vp))
		*retval = 1;
	vrele(vp);
	return(0);
}

#endif /* SEC_ARCH */

/*
 * turn security code on/off at runtime
 */
void
sec_switch_security(val)
register int val;
{
	/* take care of all the flags here */
	security_is_on 	 = val;
	check_luid 	 = val;
	check_privileges = val;
}

security_switch(p, args, retval)
	struct proc *p;
	void *args;
	long *retval;
{
	register struct args {
		long	towhat;
		long	groupid;
	} *uap = (struct args *) args;

	int error=0;
	int code = uap->towhat;
	static char done_once=0;
	static char *audit_parms[] = {
		"aa000000",
		"aam00000"
	};
	char *aud_parm = audit_parms[uap->towhat == SEC_SWITCH_ON];

	if (uap->towhat == SEC_SWITCH_CONF) {
		*retval = SEC_CONF_BASE	/* SEC_BASE is on if we got here */
#if SEC_PRIV
		| SEC_CONF_PRIV
#endif
#if SEC_MAC_OB
		| SEC_CONF_MAC_OB
#endif
#if SEC_CMW
		| SEC_CONF_CMW
#endif
#if SEC_SHW
		| SEC_CONF_SHW
#endif
#if SEC_ACL_SWARE
		| SEC_CONF_ACL_SWARE
#endif
#if SEC_ACL_POSIX
		| SEC_CONF_ACL_POSIX
#endif
#if SEC_NCAV
		| SEC_CONF_NCAV
#endif
		;	/* done building the mask */

		AUDIT_CALL(SYS_security, error, ((long *)args)-1, *retval,
				AUD_HPR, aud_parm);
		return(error);
	}
	if (uap->towhat == SEC_SWITCH_STAT) {
		if (security_is_on)
			*retval = sec_priv_gid ? sec_priv_gid : -1;
		else
			*retval = 0;
		AUDIT_CALL(SYS_security, error, ((long *)args)-1, *retval,
				AUD_HPR, aud_parm);
		return(error);
	}

	/* security-mode can only be changed at boot time
	 */
	if (done_once /* can change mode only once */
		|| (SIP->si_luid_set)
		|| (p->p_ppid == 1)
		|| (p->p_pgid != p->p_pptr->p_pptr->p_pgid)
		|| (!privileged(SEC_SUCOMPAT, EACCES))) {

		error = EACCES;
		AUDIT_CALL(SYS_security, error, ((long *)args)-1, *retval,
				AUD_HPR, aud_parm);
		return(error);
	}

	switch (uap->towhat) {
	  case SEC_SWITCH_ON:
		if (uap->groupid <= 0) {
			error = EINVAL;
			sec_priv_gid = -1;
		} else 
			sec_priv_gid = uap->groupid;
		done_once = 1;
		sec_switch_security(1);
		printf("OSFC2 Security turned on.\n");
		break;

	  case SEC_SWITCH_OFF:
		done_once = 1;
		sec_switch_security(0);
		printf("OSFC2 Security is not turned on.\n");
		break;

	  default:
		error = EINVAL;
		code = SEC_SWITCH_CONF+1;
		break;
	}

	AUDIT_CALL(SYS_security, error, ((long *)args)-1, *retval, AUD_HPR,
			aud_parm);
	return (error);
}

#endif /* SEC_BASE */
