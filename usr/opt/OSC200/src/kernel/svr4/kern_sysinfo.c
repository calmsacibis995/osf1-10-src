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
static char *rcsid = "@(#)$RCSfile: kern_sysinfo.c,v $ $Revision: 1.1.3.2 $ (DEC) $Date: 1993/05/27 21:36:57 $";
#endif
/*
 *  Module Name:
 *	kern_sysinfo.c
 *  Description:
 *	implements the SVR4 sysinfo() system call
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
#ifndef __alpha
/* 
 * This file does not exist on OSF/1 for the alpha.  The
 * definitions that are needed have been put in <svr4/systeminfo.h>.
 */
#include <machine/utsname.h>
#endif /* __alpha */
#include <sys/systeminfo.h>

/*
 * Function Name:
 *	sysinfo()
 *
 * Description:
 *	Implements SVR4 sysinfo() system call.
 *	SI_SRPC_DOMAIN and SI_SET_SRPC_DOMAIN (get and set
 *	secure RPC domain name) are not impelmented.
 *
 * Inputs:
 *	current proc pointer
 *	system call args
 *	pointer to system call return value array
 *
 * Outputs:
 *	System name
 *	OR hostnmae
 *	OR verion
 *	OR release 
 *	OR machine 
 *	OR architecture 
 *	OR hardware provider
 *	OR serial number
 *
 * Return value:
 *	Success or Failure (type of error is returned.  Based on
 *	this, syscall() sets errno).
 *
 * Called Functions:
 *	various minor functions...
 *
 * Called by:
 *	syscall()
 *
 * Side effects:
 *	sets new hostname
 *	OR None
 *	
 * Notes:
 *
 * Dependencies:
 *
 */
sysinfo(p, args, retval)
	struct proc *p;
	void *args;
	long *retval;
{
	struct args {
#ifdef	__alpha
                long command;
#else	/* __alpha */
                int command;
#endif	/* __alpha */
		char *buf;
		long count;
	} *uap = (struct args *) args;
	int error = ESUCCESS;
	int id;
	long copylen;

	/*
	 * copybuf must be big enough to hold
	 * 	hostname (MAXXHOSTNAMELEN == 64)
	 *	sysname (_SYS_NMLN ==32)
	 *	hardware serial number string (int in string format)
	 */
	char copybuf[MAXHOSTNAMELEN];

	/*
	 * (*retval) contains the buffer size in bytes required to hold
	 * the complete value and the terminating null character.
	 * If this value is no greater than the value passed in count,
	 * the entire string was copied.  If this value is greater
	 * than count, the string copied into buf has been truncated
	 * to count-1 bytes plus a terminating null character.
	 */

	switch(uap->command) {
	case SI_SYSNAME:
		*retval = strlen(utsname.sysname) + 1;
		if( uap->count ) {
			copylen = min( uap->count, *retval ) - 1;
			/* copybuf is bigger than utsname.sysname (64>32) */
			bcopy(utsname.sysname, copybuf, copylen);
			copybuf[copylen] = 0;
			error = copyout( copybuf, uap->buf, copylen+1 );
		}
		return( error );

	case SI_HOSTNAME:
		HOSTNAME_READ_LOCK();
		*retval = hostnamelen + 1;
		if( uap->count ) {
			copylen = min( uap->count, *retval ) - 1;
			/* copybuf is the same size as hostname (64==64) */
			bcopy(hostname, copybuf, copylen);
			HOSTNAME_READ_UNLOCK();
			copybuf[copylen] = 0;
			error = copyout( copybuf, uap->buf, copylen+1 );
		} else {
			HOSTNAME_READ_UNLOCK();
		}
		return( error );


	case SI_SET_SYSNAME:
#if SEC_BASE
		if (!privileged(SEC_SYSATTR, EPERM))
			return (EPERM);
#else   
		if (error = suser(u.u_cred, &u.u_acflag))
			return (error);
#endif
		if( error = copyin((caddr_t)uap->buf, copybuf,
		  sizeof(utsname.sysname)) != 0 )
			return(error);

		/* make sure copied in name ends with a null */
		for(copylen = 0; copylen < sizeof(utsname.sysname); copylen++) { 
			if( copybuf[copylen] == 0 )
				break;
		}
		/* name too long or name too short */
		if( copylen == sizeof(utsname.sysname) || copylen == 0 ) {
			return(EINVAL);
		}
		copylen++;	/* add in ending NULL */

		HOSTNAME_WRITE_LOCK();
		bcopy(copybuf, utsname.sysname, copylen);
		HOSTNAME_WRITE_UNLOCK();
		return (error);


	case SI_SET_HOSTNAME:
#if SEC_BASE
		if (!privileged(SEC_SYSATTR, EPERM))
			return (EPERM);
#else   
		if (error = suser(u.u_cred, &u.u_acflag))
			return (error);
#endif
		/* copybuf is the same size as hostname, so will hold
		 * exactly as big a name as will fit in hostname
		 */
		error = copyin((caddr_t)uap->buf, copybuf, sizeof(copybuf));
		if( error )
			return(error);

		/* make sure copied in name ends with a null */
		for(copylen = 0; copylen < sizeof(copybuf); copylen++) { 
			if( copybuf[copylen] == 0 )
				break;
		}
		/* name too long or name too short */
		if( copylen == sizeof(copybuf) || copylen == 0 ) {
			return(EINVAL);
		}
		copylen++;	/* add in ending NULL */

		HOSTNAME_WRITE_LOCK();
		bcopy(copybuf, hostname, copylen);
		hostnamelen = copylen - 1;
		bcopy(hostname, utsname.nodename, sizeof(utsname.nodename));
		utsname.nodename[sizeof(utsname.nodename)-1] = 0; 
#if	SEC_BASE
		/*
		 * MP note: audstub_pathname() is pendable.  Hopefully we won't be
		 * calling this often enough for it to matter that we're holding a
		 * lock.  (We don't want to risk races with other hostname changes,
		 * and it's probably not worth the effort to do copying to a 
		 * temp buffer.)
		 */
		audstub_pathname(hostname, hostnamelen + 1);
#endif 
		HOSTNAME_WRITE_UNLOCK();
		return (error);

	case SI_RELEASE:
		*retval = strlen(utsname.release) + 1;
		if( uap->count ) {
			copylen = min( uap->count, *retval ) - 1;
			/* copybuf is bigger than utsname.release (64>32) */
			bcopy(utsname.release, copybuf, copylen);
			copybuf[copylen] = 0;
			error = copyout( copybuf, uap->buf, copylen+1 );
		}
		return( error );

	case SI_VERSION:
		*retval = strlen(utsname.version) + 1;
		if( uap->count ) {
			copylen = min( uap->count, *retval ) - 1;
			/* copybuf is bigger than utsname.version (64>32) */
			bcopy(utsname.version, copybuf, copylen);
			copybuf[copylen] = 0;
			error = copyout( copybuf, uap->buf, copylen+1 );
		}
		return( error );

	case SI_MACHINE:
		*retval = strlen(utsname.machine) + 1;
		if( uap->count ) {
			copylen = min( uap->count, *retval ) - 1;
			/* copybuf is bigger than utsname.machine (64>32) */
			bcopy(utsname.machine, copybuf, copylen);
			copybuf[copylen] = 0;
			error = copyout( copybuf, uap->buf, copylen+1 );
		}
		return( error );

	case SI_ARCHITECTURE:
		/*
		 * note - I 'm ASSUMING that ARCHITECTURE will
		 * not be longer than what will fit in copybuf,
		 * and forcing it to be this long if not.
		 */
		*retval = strlen(ARCHITECTURE) + 1;
		if( *retval > sizeof(copybuf) ) {
			*retval = sizeof(copybuf);
		}
		if( uap->count ) {
			copylen = min( uap->count, *retval ) - 1;
			bcopy(ARCHITECTURE, copybuf, copylen);
			copybuf[copylen] = 0;
			error = copyout( copybuf, uap->buf, copylen+1 );
		}
		return( error );

	case SI_HW_PROVIDER:
		/*
		 * note - I 'm ASSUMING that HW_PROVIDER_NAME will
		 * not be longer than what will fit in copybuf,
		 * and forcing it to be this long if not.
		 */
		*retval = strlen(HW_PROVIDER_NAME) + 1;
		if( *retval > sizeof(copybuf) ) {
			*retval = sizeof(copybuf);
		}
		if( uap->count ) {
			copylen = min( uap->count, *retval ) - 1;
			bcopy(HW_PROVIDER_NAME, copybuf, copylen);
			copybuf[copylen] = 0;
			error = copyout( copybuf, uap->buf, copylen+1 );
		}
		return( error );

	case SI_HW_SERIAL:
		/* id = machineid(); */
		id = 0;
		/* int translated into a string will fit into copybuf
		 * can be 63 characters long.
		 */
		itoa(id,copybuf);
		/* copy as much of string as fits into user buffer */
		*retval = strlen(copybuf) + 1;
		if( uap->count ) {
			copylen = min( uap->count, *retval ) - 1;
			copybuf[copylen] = 0;
			error = copyout( copybuf, uap->buf, copylen+1 );
		}
		return( error );

	case SI_SRPC_DOMAIN:
	case SI_SET_SRPC_DOMAIN:
		/*
		 * the domainname set and retrieved in this kernel
		 * refers to the domain used by yp and such, not
		 * to the domain used for Secure Remote Procedure
		 * Call.  This domain name is not available.
		 * Return ENOSYS to make this obvious.
		 */
		return(ENOSYS);
	default:
		return(EINVAL);
	}
	return(EINVAL);
}

