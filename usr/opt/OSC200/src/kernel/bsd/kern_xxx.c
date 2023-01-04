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
static	char	*rcsid = "@(#)$RCSfile: kern_xxx.c,v $ $Revision: 4.3.10.5 $ (DEC) $Date: 1993/10/14 13:54:41 $";
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

 */

#include <cputypes.h>
#include <quota.h>
#include <sys/secdefines.h>

#include <sys/unix_defs.h>
#include <sys/param.h>
#include <sys/systm.h>
#include <sys/user.h>
#include <sys/kernel.h>
#include <sys/proc.h>
#include <sys/reboot.h>
#include <sys/utsname.h>
#include <sys/file.h>
#include <sys/stat.h>
#if	SEC_BASE
#include <sys/security.h>
#endif

gethostid(p, args, retval)
	struct proc *p;
	void *args;
	long *retval;
{

	HOSTNAME_READ_LOCK();
	*retval = hostid;
	HOSTNAME_READ_UNLOCK();
	return (0);
}

sethostid(p, args, retval)
	struct proc *p;
	void *args;	
	long *retval;
{
	struct args {
		long	hostid;		/* real type: 'int' */
	} *uap = (struct args *) args;
	int error;

#if	SEC_BASE
	if (!privileged(SEC_SYSATTR, EPERM))
		return (EPERM);
#else 
	if ((error = suser(u.u_cred, &u.u_acflag)))
		return (error);
#endif
	HOSTNAME_WRITE_LOCK();
	hostid = (int)uap->hostid;
	HOSTNAME_WRITE_UNLOCK();
	return (0);
}

gethostname(p, args, retval)
	struct proc *p;
	void *args;	
	long *retval;
{
	struct args {
		char	*hostname;	/* real type: 'char *' */
		u_long	len;		/* real type: 'int' */
	} *uap = (struct args *) args;
	int error;

	HOSTNAME_READ_LOCK();
	if ((int)uap->len > hostnamelen + 1)
		uap->len = hostnamelen + 1;
	error = copyout((caddr_t)hostname, (caddr_t)uap->hostname, (int)uap->len);
	HOSTNAME_READ_UNLOCK();
	return (error);
}

sethostname(p, args, retval)
	struct proc *p;
	void *args;	
	long *retval;
{
	struct args {
		char	*hostname;	/* real type: 'char *' */
		u_long	len;		/* real type: 'int' */
	} *uap = (struct args *) args;
	int error;
#ifdef CJXXX
	extern int unixauth_init;
#endif /* CJXXX */

#if	SEC_BASE
	if (!privileged(SEC_SYSATTR, EPERM))
		return (EPERM);
#else   
	if (error = suser(u.u_cred, &u.u_acflag))
		return (error);
#endif
	if ((int)uap->len > sizeof (hostname) - 1)
		return (EINVAL);
	HOSTNAME_WRITE_LOCK();
	hostnamelen = (int)uap->len;
	error = copyin((caddr_t)uap->hostname, hostname, (int)uap->len);
	hostname[hostnamelen] = 0;
	if (!error) {
		bcopy(hostname, utsname.nodename, sizeof(utsname.nodename));
		utsname.nodename[sizeof(utsname.nodename)-1] = 0; 
#ifdef CJXXX
		unixauth_init = 0;	/* Tell NFS client about change */
#endif /* CJXXX */
	}
	HOSTNAME_WRITE_UNLOCK();
	return (error);
}

/*
 * Re-use the hostname lock for the domainname, too.  As the host and
 * domain names are typically read and almost never set, this lock
 * shouldn't be a bottleneck unless use becomes so frequent that the
 * spinlock itself becomes a contention point.
 */
getdomainname(p, args, retval)
	struct proc *p;
	void *args;	
	long *retval;
{
	struct args {
		char	*domainname;	/* real type: 'char *' */
		long	len;		/* real type: 'int' */
	} *uap = (struct args *) args;
	int error;

	HOSTNAME_READ_LOCK();
	if ((int)uap->len > domainnamelen + 1)
		uap->len = domainnamelen + 1;
	error = copyout((caddr_t)domainname,(caddr_t)uap->domainname,(int)uap->len);
	HOSTNAME_READ_UNLOCK();
	return (error);
}

setdomainname(p, args, retval)
	struct proc *p;
	void *args;	
	long *retval;
{
	struct args {
		char	*domainname;	/* real type: 'char *' */
		long	len;		/* real type: 'int' */
	} *uap = (struct args *) args;
	int error;

#if	SEC_BASE
	if (!privileged(SEC_SYSATTR, EPERM))
		return (EPERM);
#else  
	if (error = suser(u.u_cred, &u.u_acflag))
		return (error);
#endif
	if ((int)uap->len > sizeof (domainname) - 1)
		return (EINVAL);
	HOSTNAME_WRITE_LOCK();
	domainnamelen = (int)uap->len;
	error = copyin((caddr_t)uap->domainname, domainname, (int)uap->len);
	domainname[domainnamelen] = 0;
	HOSTNAME_WRITE_UNLOCK();
	return (error);
}

/* 
 *  get name of current operating system 
 */
uname(p, args, retval)
	struct proc *p;
	void *args;	
	long *retval;
{
	struct args {
		struct utsname *name;
	} *uap = (struct args *) args;
	int error;
	/*
	 * The utsname.nodename field is now set by sethostname.
	 */
	HOSTNAME_READ_LOCK();
	bcopy(hostname, utsname.nodename, sizeof(utsname.nodename));
	utsname.nodename[sizeof(utsname.nodename)-1] = 0;
	error = copyout(&utsname, uap->name, sizeof(struct utsname));
	HOSTNAME_READ_UNLOCK();
	return (error);
}

reboot(p, args, retval)
	struct proc *p;
	void *args;	
	long *retval;
{
	struct args {
		long	opt;
	} *uap = (struct args *) args;
	int error;

	/*
	 * call the presto NVRAM pseudo-driver to flush buffers;
	 * this routine is a stub (conf.c) if presto not present.
	 */
	presto_reboot();

#if	SEC_BASE
	if (!privileged(SEC_SHUTDOWN, EPERM))
		error = EPERM;
#else
	if (error = suser(u.u_cred, &u.u_acflag))
		;
#endif
	else {
#ifndef	multimax
#if	NCPUS > 1
#ifdef	notdef
		extern int new_printf_cpu_number;
#endif	/* notdef */

		printf("Reboot()ing.\n");
#ifdef	notdef
		new_printf_cpu_number = 0;
#endif	/* notdef */
#endif
#endif

		AUDIT_CALL2 ( u.u_event, 0, args, 0L, AUD_HPR|AUD_FLU, (char *)0 );
		boot(RB_BOOT, uap->opt);
	}

	AUDIT_CALL2 ( u.u_event, error, args, 0L, AUD_HPR, (char *)0 );
	return(error);
}

#ifdef COMPAT_43

struct  ostat
{
	short   ost_dev;                /* ID of device containing a directory*/
					/*   entry for this file.  File serial*/
					/*   no + device ID uniquely identify */
					/*   the file within the system */
	ino_t   ost_ino;                /* File serial number */
	u_short ost_mode;               /* File mode; see #define's below */
	short   ost_nlink;              /* Number of links */
	u_short ost_uid;                /* User ID of the file's owner */
	u_short ost_gid;                /* Group ID of the file's group */
	short   ost_rdev;               /* ID of device */
					/*   This entry is defined only for */
					/*   character or block special files */
	off_t   ost_size;               /* File size in bytes */
	time_t  ost_atime;              /* Time of last access */
	int     ost_spare1;
	time_t  ost_mtime;              /* Time of last data modification */
	int     ost_spare2;
	time_t  ost_ctime;              /* Time of last file status change */
	int     ost_spare3;
					/* Time measured in seconds since */
					/*   00:00:00 GMT, Jan. 1, 1970 */
	ulong_t ost_blksize;            /* Size of block in file */
	long    ost_blocks;             /* blocks allocated for file */
	u_long  ost_flags;              /* user defined flags for file */
	u_long  ost_gen;                /* file generation number */

};

ostat(p, args, retval)
	struct proc *p;
	void *args;
	long *retval;
{
	return (ostat1(p, args, retval, (long) FOLLOW));
}

olstat(p, args, retval)
	struct proc *p;
	void *args;
	long *retval;
{
	return (ostat1(p, args, retval, (long) NOFOLLOW));
}

#include <sys/ioctl.h>

ofstat(p, args, retval)
	struct proc *p;
	void *args;	
	long *retval;
{
	struct args {
		long     fdes;
		struct  ostat *ub;
	} *uap = (struct args *) args;
	struct file *fp;
	struct stat sb;
	struct ostat osb;
	int error;

	if (error = getf(&fp, uap->fdes, FILE_FLAGS_NULL, &u.u_file_state))
		return (error);
	switch (fp->f_type) {
	case DTYPE_VNODE:
		FILE_FUNNEL(fp->f_funnel);
		error = vn_stat((struct vnode *)fp->f_data, &sb);
		if(!error && S_ISCHR(sb.st_mode)) {
			FOP_IOCTL(fp,I_ISASTREAM,0,retval,error);
			if(!error && (*retval == I_FIFO || *retval == I_PIPE)) {
				sb.st_mode &= ~S_IFMT;
				sb.st_mode |= S_IFIFO;
				sb.st_size = 0;
			}
			error = 0;
			retval = 0;
		}
		FILE_UNFUNNEL(fp->f_funnel);
		break;

	case DTYPE_SOCKET:
		error = soo_stat((struct socket *)fp->f_data, &sb);
		break;
	
	default:
		panic("ofstat");
		/*NOTREACHED*/
	}
	if (error == 0) {
		convert_stat(&osb, &sb);
		error = copyout((caddr_t)&osb, (caddr_t)uap->ub,
					sizeof (osb));
	}
	FP_UNREF(fp);
	return (error);
}

ostat1(p, args, retval, follow)
	struct proc *p;
	void *args;	
	long *retval;
	long follow;
{
	struct args {
		char    *fname;
		struct ostat *ub;
	} *uap = (struct args *) args;
	register struct nameidata *ndp = &u.u_nd;
	struct stat sb;
	struct ostat osb;
	int error;

	ndp->ni_nameiop = LOOKUP | follow;
	ndp->ni_segflg = UIO_USERSPACE;
	ndp->ni_dirp = uap->fname;
	if ((error = namei(ndp)) == 0) {
		error = vn_stat(ndp->ni_vp, &sb);
		vrele(ndp->ni_vp);
		if (error == 0) {
			convert_stat(&osb, &sb);
			error = copyout((caddr_t)&osb, (caddr_t)uap->ub, 
					 sizeof (osb));
		}
	}
	return (error);
}

static
convert_stat(osb, sb)
register struct ostat *osb;
register struct stat *sb;
{
	osb->ost_dev = sb->st_dev;
	osb->ost_ino = sb->st_ino;
	osb->ost_mode = sb->st_mode;
	osb->ost_nlink = sb->st_nlink;
	osb->ost_uid = sb->st_uid;
	osb->ost_gid = sb->st_gid;
	osb->ost_rdev = sb->st_rdev;
	osb->ost_size = sb->st_size;
	osb->ost_atime = sb->st_atime;
	osb->ost_spare1 = sb->st_spare1;
	osb->ost_mtime = sb->st_mtime;
	osb->ost_spare2 = sb->st_spare2;
	osb->ost_ctime = sb->st_ctime;
	osb->ost_spare3 = sb->st_spare3;
	osb->ost_blksize = sb->st_blksize;
	osb->ost_blocks = sb->st_blocks;
	osb->ost_flags = sb->st_flags;
	osb->ost_gen = sb->st_gen;
}
#endif /* COMPAT_43 */


/*
 * XXX should these be COMPAT_43 ???
 */
ovhangup(p, args, retval)
	struct proc *p;
	void *args;
	long *retval;
{

	return (EINVAL);
}

oldquota(p, args, retval)
	struct proc *p;
	void *args;
	long *retval;
{

#ifdef __alpha
	/*
	 *  FIX ME -- FIX ME -- FIX ME
	 */
	return(0);
#else
	return (EINVAL);
#endif
}
