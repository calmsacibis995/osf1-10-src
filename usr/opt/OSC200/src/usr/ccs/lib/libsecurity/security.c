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
static char *rcsid = "@(#)$RCSfile: security.c,v $ $Revision: 4.2.9.3 $ (DEC) $Date: 1993/04/08 19:21:59 $";
#endif
/*
 * @OSF_COPYRIGHT@
 */
/*
 * HISTORY
 * $OSF_Log:	security.c,v $
 * Revision 1.1.1.2  92/03/31  00:45:00  devrcs
 *  *** OSF1_1B25 version ***
 * 
 * Revision 1.7.2.2  1992/02/11  17:37:24  hosking
 * 	bug 4520: a bit more type widening (setluid() takes 'uid_t', not 'ushort')
 * 	[1992/02/11  17:36:29  hosking]
 *
 * Revision 1.5.2.3  91/03/11  15:42:33  seiden
 * 	Merge fixes up from 1.0.1
 * 	[91/03/11  15:26:06  seiden]
 * 
 * Revision 1.4.3.2  91/02/22  11:20:00  seiden
 * 	Build statpriv and chpriv under SEC_PRIV, not SEC_ARCH.
 * 
 * Revision 1.4  90/10/07  20:08:44  devrcs
 * 	Added EndLog Marker.
 * 	[90/09/28  20:17:20  gm]
 * 
 * Revision 1.3  90/07/17  12:21:20  devrcs
 * 	Internationalized
 * 	[90/07/05  07:44:32  staffan]
 * 
 * Revision 1.2  90/06/22  21:47:59  devrcs
 * 	Initial version from SecureWare
 * 	[90/05/31  11:41:55  staffan]
 * 
 * $OSF_EndLog$
 */
/*
 * Copyright (c) 1988, 1989 SecureWare, Inc.  All rights reserved.
 *
 * This Module contains Proprietary Information of SecureWare, Inc. and
 * should be treated as Confidential.
 */

/* #ident "@(#)security.c	6.4 08:38:17 8/21/91 SecureWare" */

/*LINTLIBRARY*/

#include <sys/secdefines.h>

/* #if SEC_BASE */ /*{*/

/*
 * This file contains a set of routines used to make programs
 * more secure.  Specifically, it contains system all interface
 * routines for security.
 */

#include <sys/types.h>

#include <sys/security.h>
#include <sys/audit.h>
#include <sys/sec_objects.h>
#if SEC_ARCH
#include <sys/secpolicy.h>
#endif
#include <prot.h>
#include <errno.h>

/*
 * Adjunct error number that allows analysis of security call failures.
 * The value of this external is set from the second byte of rval1.
 * The low order bits are used for the UNIX error number;
 * SEC_ERRNO_MASK is a mask of where in the errno field the sec_errno
 * value is placed.  SEC_ERRNO_SHIFT tells how far over to shift
 * the sec_errno value to get it right-justified.  Be careful if the
 * mask extends to the highest bit of errno and the machine sign extends
 * on a right shift.
 */

int sec_errno;
extern int errno;

#define SET_SEC_ERRNO()	{\
	sec_errno = (((long) errno) & SEC_ERRNO_MASK) >> SEC_ERRNO_SHIFT; \
	errno = ((long) errno) & ~SEC_ERRNO_MASK; \
	}

/*
 * The routines herein are user library stubs for the
 * security() sysent entry point.  The first argument to security() is
 * the system call code of the call and the remaining arguments
 * are call-specific.  Specify additional arguments as 0.
 *
 * Note that some return actual values (getluid(), getpriv())
 * while the others return 0 for success.  All return -1 for failure.
 */

/*
 * Some of the syscall-handling can be short-circuited if we know that
 * we have a kernel without SEC_BASE.
 */
static int sec_conf_known = 0, sec_conf_stat;

/*
 * Force all open files in the system to the filename provided to become
 * invalid.  On the next read/write/ioctl access, send a signal to the
 * effected process and return an error.
 */
int
stopio(filename)
	caddr_t filename;
{
	register int ret;

	if (security_is_on())
		ret = security(SEC_STOPIO, filename, NULL, NULL, NULL, NULL);
	else
		ret = revoke(filename);
	if (ret != -1)
		ret = 0;
	else
		SET_SEC_ERRNO();

	return ret;
}


/*
 * Get the system's notion of the login UID associated with the process.
 * If it has not been set yet, return an error.
 */
int
getluid()
{
	register int ret;

	ret = security(SEC_GETLUID, NULL, NULL, NULL, NULL, NULL);

	if (ret == -1)
		SET_SEC_ERRNO();

	return ret;
}


/*
 * Set the system's notion of the login UID for this process and all its
 * descendants.  Once set, it cannot be reset.
 */
int
setluid(uid)
	uid_t uid;
{
	register int ret;

	ret = security(SEC_SETLUID, uid, NULL, NULL, NULL, NULL);
	if (ret != -1)
		ret = 0;
	else
		SET_SEC_ERRNO();

	return ret;
}


/*
 * Get the system privileges for this process and store them in the supplied
 * buffer.  The size in bytes of the number of privileges is returned.
 * Privtype specifies either the maximum, base or effective privileges.
 */
int
getpriv(privtype, vec)
	long privtype;
	register priv_t *vec;
{
	obj_t objid;
	register int i;
	register uid_t ruid, euid;
	static privvec_t allprivs;
	static int done_once = 0;

	objid.o_pid = 0;

	if (security_is_avail())
		return statprivsys(privtype, vec, OT_PROCESS, &objid);
	(void) memset(vec, 0, sizeof(*vec));
	ruid = getuid();
	euid = geteuid();
	if (!ruid || !euid) {
		if (!done_once) {
			for (i = 0;  i <= SEC_MAX_SPRIV;  i++)
				ADDBIT(vec, i);
			(void) memmove(allprivs, vec, sizeof allprivs);
			done_once = 1;
		}
		else
			(void) memmove(vec, allprivs, sizeof allprivs);
	}
	return 0;
}


/*
 * Reset the system privileges for this process.  They cannot be more
 * than was already set.  Privtype specifies either the maximum,
 * base or effective privileges.
 */
int
setpriv(privtype, vec)
	long privtype;
	register priv_t *vec;
{
	obj_t objid;
	register int i;
	register int ruid, euid;

	objid.o_pid = 0;

	if (security_is_avail())
		return chprivsys(privtype, vec, OT_PROCESS, &objid);
	ruid = getuid();
	euid = geteuid();
	if (!ruid || !euid)
		return 0;
	for (i = 0;  i < SEC_SPRIVVEC_SIZE;  i++) {
		if (vec[i]) {
			errno = EPERM;
			SET_SEC_ERRNO();
			return -1;
		}
	}
	return 0;
}

/*
 * check if the system has security code turned on or not
 */
int
security_is_on()
{
	register int ret;

	if (!sec_conf_known)
		(void) security_is_avail();
	if (sec_conf_known && !sec_conf_stat)
		return 0;

	ret = security(SEC_SWITCH_CALL, SEC_SWITCH_STAT, NULL, NULL, NULL, NULL);

	if (ret == -1)
		SET_SEC_ERRNO();

	return ret;
}

/*
 * check if the system was built with SEC_BASE turned on
 */
int
security_is_avail()
{
	register int ret;

	if (sec_conf_known)
		return sec_conf_stat;
	ret = security(SEC_SWITCH_CALL, SEC_SWITCH_CONF, NULL, NULL, NULL, NULL);

	if (ret == -1) {
		SET_SEC_ERRNO();
	}
	else {
		sec_conf_known = 1;
		sec_conf_stat = ret;
	}

	return ret;
}

/*
 * turn security code on
 */
int
security_turn_on(gid)
	gid_t gid;
{
	register int ret;

	ret = security( SEC_SWITCH_CALL, SEC_SWITCH_ON, gid, NULL, NULL, NULL);

	if (ret == -1)
		SET_SEC_ERRNO();

	return ret;
}
/*
 * turn security code off
 */
int
security_turn_off()
{
	register int ret;

	ret = security( SEC_SWITCH_CALL, SEC_SWITCH_OFF, NULL, NULL, NULL, NULL);

	if (ret == -1)
		SET_SEC_ERRNO();

	return ret;
}

#if SEC_PRIV /*{*/
/*
 * Retrieve the specified privilege set for a file.
 * Privtype is one of SEC_POTENTIAL_PRIV or SEC_GRANTED_PRIV
 */

int
statpriv(file, privtype, vec)
	caddr_t file;
	long privtype;
	priv_t *vec;
{
	obj_t objid;

	objid.o_file = file;

	return statprivsys(privtype, vec, OT_REGULAR, &objid);
}

/*
 * Set the privilege set for a file.
 * Privtype is one of SEC_POTENTIAL_PRIV or SEC_GRANTED_PRIV
 */

chpriv(file, privtype, vec)
	caddr_t file;
	long privtype;
	priv_t *vec;
{
	obj_t objid;

	objid.o_file = file;

	return chprivsys(privtype, vec, OT_REGULAR, &objid);
}
#endif /*} SEC_PRIV */

/*
 * Obtain the privileges associated with an object, whether the object be
 * a process, file or IPC entity.
 */
int
statprivsys(privtype, vec, objtype, objid)
	long privtype;
	priv_t *vec;
	long objtype;
	obj_t *objid;
{
	register int ret;

	ret = security( SEC_STATPRIV, privtype, vec, objtype, objid, NULL);

	if (ret == -1)
		SET_SEC_ERRNO();

	return ret;
}


/*
 * Change the privileges associated with an object, whether the object be
 * a process, file or IPC entity.
 */
int
chprivsys(privtype, vec, objtype, objid)
	long privtype;
	priv_t *vec;
	long objtype;
	obj_t *objid;
{
	register int ret;

	ret = security( SEC_CHPRIVSYS, privtype, vec, objtype, objid, NULL);
	if (ret != -1)
		ret = 0;
	else
		SET_SEC_ERRNO();

	return ret;
}


#if SEC_ARCH
/*
 * Obtain the effective access for the object, considering ALL security
 * policies and UNIX constraints.
 */
int
eaccess(file, mode)
	caddr_t file;
	long mode;
{
	register int ret;

	ret = security(SEC_EACCESS, file, mode, NULL, NULL, NULL);
	if (ret != -1)
		ret = 0;
	else
		SET_SEC_ERRNO();

	return ret;
}
#endif

#if SEC_MAC /*{*/
/*
 * Make a multilevel directory.
 */
int
mkmultdir(file, mode)
	caddr_t file;
	int mode;
{
	register int ret;

	ret = security(SEC_MKMULTDIR, file, NULL, NULL, NULL, NULL);
	if (ret != -1)
		ret = 0;
	else
		SET_SEC_ERRNO();

	return ret;
}


/*
 * Remove a multilevel directory.
 */
int
rmmultdir(file, mode)
	caddr_t file;
	int mode;
{
	register int ret;

	ret = security(SEC_RMMULTDIR, file, NULL, NULL, NULL, NULL);
	if (ret != -1)
		ret = 0;
	else
		SET_SEC_ERRNO();

	return ret;
}


/*
 * Query a file to see if it is a multilevel directory.  It returns 1
 * if it is a multilevel directory, 0 if not, and -1 for errors.
 */
int
ismultdir(file, mode)
	caddr_t file;
	int mode;
{
	register int ret;

	ret = security(SEC_ISMULTDIR, file, NULL, NULL, NULL, NULL);

	if (ret == -1)
		SET_SEC_ERRNO();

	return ret;
}
#endif /*} SEC_MAC */



#if SEC_ARCH /*{*/

/*
 * Get a security label from one of the policies.
 */
int
getlabel(policy,tagnum,attr,objtype,objid)
	ulong_t	policy, tagnum;
	attr_t *attr;
	long objtype;
	obj_t *objid;
{
	register int ret;

	ret = security(SEC_GETLABEL, policy, tagnum, attr, objtype, objid);
	if (ret != -1)
		ret = 0;
	else
		SET_SEC_ERRNO();

	return ret;
}


/*
 * Set a security label from one of the policies.
 */
int
setlabel(policy, tagnum, attr, objtype, objid)
	ulong_t policy, tagnum;
	attr_t *attr;
	long objtype;
	obj_t *objid;
{
	register int ret;

	ret = security(SEC_SETLABEL, policy, tagnum, attr, objtype, objid);
	if (ret != -1)
		ret = 0;
	else
		SET_SEC_ERRNO();

	return ret;
}


/*
 * Label mount an insecure file system type, assigning all files therein to
 * single values (fixed by the labels) for the security policies.
 */
int
lmount(mnttype, name, flags, argp, attrs)
	caddr_t	mnttype;
	caddr_t	name;
	long flags;
	caddr_t argp;
	attr_t *attrs;
{
	register int ret;

	ret = security(SEC_LMOUNT, mnttype, name, flags, argp, attrs);
	if (ret != -1)
		ret = 0;
	else
		SET_SEC_ERRNO();

	return ret;
}

/*
 * Return TRUE if the file system supports labels and/or privileges
 */
int
islabeledfs(file)
	caddr_t file;
{
	register int ret;

	ret = security(SEC_ISLABELEDFS, file, NULL, NULL, NULL, NULL);

	if (ret == -1)
		SET_SEC_ERRNO();

	return ret;
}
#endif /*} SEC_ARCH */
/* #endif */ /*} SEC_BASE */
