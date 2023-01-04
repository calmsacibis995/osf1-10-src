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
static char	*sccsid = "@(#)$RCSfile: svipc_ipc.c,v $ $Revision: 4.3.3.2 $ (DEC) $Date: 1992/01/30 23:50:05 $";
#endif 
/*
 */

/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0
 */

#include <sys/secdefines.h>
#include <sys/ipc.h>
#include <sys/errno.h>
#include <sys/types.h>
#include <sys/param.h>
#include <sys/signal.h>
#include <sys/proc.h>
#include <sys/user.h>
#if	SEC_BASE && !SEC_ARCH
#include <sys/security.h>
#endif	

/*
**	Common IPC routines.
*/

/*
**	Check message, semaphore, or shared memory access permissions.
**
**	This routine verifies the requested access permission for the current
**	process.  Super-user is always given permission.  Otherwise, the
**	appropriate bits are checked corresponding to owner, group, or
**	everyone.  Zero is returned on success.  On failure, u.u_error is
**	set to EACCES and one is returned.
**	The arguments must be set up as follows:
**		p - Pointer to permission structure to verify
**		mode - Desired access permissions
*/


#if	!SEC_ARCH
ipcaccess(p, mode)
register struct ipc_perm 	*p;
register u_long 		mode;
{
#if	!SEC_BASE
	if (u.u_uid == 0)
		return (0);
#endif

	if (u.u_uid != p->uid && u.u_uid != p->cuid) {
		mode >>= 3;
		if (u.u_gid != p->gid && u.u_gid != p->cgid)
			mode >>= 3;
	}

	if (mode & p->mode)
		return (0);

#if	SEC_BASE
	if (privileged(SEC_ALLOWDACACCESS, 0))
		return 0;
#endif

	return (1);
}
#endif	/* !SEC_ARCH */

/*
**	Get message, semaphore, or shared memory structure.
**
**	This routine searches for a matching key based on the given flags
**	and returns a pointer to the appropriate entry.  A structure is
**	allocated if the key doesn't exist and the flags call for it.
**	The arguments must be set up as follows:
**		key - Key to be used
**		flag - Creation flags and access modes
**		base - Base address of appropriate facility structure array
**		cnt - # of entries in facility structure array
**		size - sizeof(facility structure)
**		status - Pointer to status word: set on successful completion
**			only:	0 => existing entry found
**				1 => new entry created
**	Ipcget returns NULL with u.u_error set to an appropriate value if
**	it fails, or a pointer to the initialized entry if it succeeds.
*/

ipcget(key, flag, base, cnt, size, status, ret)
key_t           		key;
long             		flag;
register struct ipc_perm 	*base;
long             		cnt;
long 				size;
int				*status;
struct ipc_perm 		**ret;
{
	register struct ipc_perm	*a;	/* ptr to available entry */
	register int    		i;	/* loop control */

	*ret = NULL;
	if (key == IPC_PRIVATE) {
		for (i = 0; i++ < cnt;
		        base = (struct ipc_perm *) (((char *) base) + size)) {
			if (base->mode & IPC_ALLOC)
				continue;

			goto init;
		}

		return (ENOSPC);
	} 
	else {
		for (i = 0, a = NULL; i++ < cnt;
		     base = (struct ipc_perm *) (((char *) base) + size)) {

			if (base->mode & IPC_ALLOC) {

				/* this entry is allocated */
				if (base->key == key) {

					/* key matches */
					if ((flag & (IPC_CREAT | IPC_EXCL)) ==
					    (IPC_CREAT | IPC_EXCL))
						return (EEXIST);

					if ((flag & 0777) & ~base->mode)
						return (EACCES);

					*status = 0;
					*ret = base;
					return (0);
				}  
				continue;

			}

			/*
			 * entry not allocated, hold this space for a new entry 
			 */
			if (a == NULL)
				a = base;
		} /* for() */

		if (!(flag & IPC_CREAT))
			return (ENOENT);

		if (a == NULL)
			return (ENOSPC);

		base = a;
	}

init:
	*status = 1;

	base->mode = IPC_ALLOC | (flag & 0777);
	base->key = key;
	base->cuid = base->uid = u.u_uid;
	base->cgid = base->gid = u.u_gid;

	*ret = base;
	return (0);
}
