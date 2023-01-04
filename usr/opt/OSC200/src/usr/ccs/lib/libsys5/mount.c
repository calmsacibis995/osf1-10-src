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
static char     *sccsid = "@(#)$RCSfile: mount.c,v $ $Revision: 4.2.2.3 $ (DEC) $Date: 1993/07/15 15:02:09 $";
#endif

/*
 * ALL RIGHTS RESERVED
 */

/*
 * OSF/1 Release 1.0
 */

/*
 * mount.c
 *
 *      Revision History:
 *
 * 12-May-91
 *      SVID 2 + SVID 3 + Vax System V features.
 *
 */

#if !defined(lint) && !defined(_NOIDENT)

#endif


/*
 * FUNCTIONS: mount
 *
 * DESCRIPTION:
 *	System V compatible mount.  This conforms to the SVID 3 interface
 * definition. As per that definiton, SVID 2 is also supported via
 * the MS_DATA flag. 
 *
 *      NFS file system types require that a filled in "nfs_args" structure
 * 	be passed through "dataptr". The "fs" parameter may be a NULL
 * 	pointer in this case.
 *
 *      MFS file system types require that a filled in "mfs_args" structure
 *  	be passed through "dataptr". The "fs" parameter may be a NULL 
 * 	pointer in this case.
 */

#include <sys/habitat.h>
#include <sys/syscall.h>
#include <sys/mount.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <string.h>

#pragma weak mount = __mount
int
__mount(fs, path, mflag, fstyp, dataptr, datalen)
	char	*fs;
	char	*path;
	int	mflag;
	char	*fstyp;
	char 	*dataptr;
	int	datalen;
{
struct	ufs_args	args;
	caddr_t		argp;
	int		fsnum,ret;

	/*
	 * SVVS fix: 'Mounting a named special file that does not exist.'
	 *	mount syscall returns ENODEV where ENOENT is wanted.
	 */
	{	struct stat sb;

		if (stat((char *)fs, &sb) == -1 && _Geterrno() == ENOENT)
			return(-1);
	}

	if((~(MS_DATA | MS_RDONLY | MS_NOSUID | MS_REMOUNT)) & mflag) {
		_Seterrno(EINVAL);
		return(-1);
	}

	if (!(MS_DATA & mflag)) {
		/*
		 * FSType not specified,  use root FSType
		 */
		struct statfs fs;
		
		if (statfs("/", &fs, sizeof(struct statfs)) == -1)
			return(-1);

		fsnum = fs.f_type;
	}
	else {
		mflag &= ~MS_DATA;	/* MS_DATA bit no longer needed */

		/*
		 * Heuristic that works for both SVR3, SVID-3 and SVR4:
		 * If fstype is in the range of FSType numbers, treat it
		 * as such (SVR3 & SVR4), otherwise treat it is as a string 
		 * containing the name of the FSType (SVID-3).
		 */
		if ((unsigned)fstyp <= MOUNT_MAXTYPE) {
			if (fstyp == MOUNT_NONE) {
				_Seterrno(EINVAL);
				return(-1);
			}
			fsnum = (int)fstyp;	
		}
		else {
			fsnum = MOUNT_NONE;
			if (!strcmp(fstyp,"ufs"))  fsnum = MOUNT_UFS;
			else if (!strcmp(fstyp,"nfs"))  fsnum = MOUNT_NFS;
			else if (!strcmp(fstyp,"mfs"))  fsnum = MOUNT_MFS;
			else if (!strcmp(fstyp,"s5fs")) fsnum = MOUNT_S5FS;
			else if (!strcmp(fstyp,"pc")) fsnum = MOUNT_PC;
		}
	}
	switch(fsnum) {
		case MOUNT_NONE:
		case MOUNT_UFS:
		case MOUNT_S5FS:
			args.fspec = fs;
			args.exflags = 0;
			args.exroot = 0;
			argp = (caddr_t)&args;
			break;
		case MOUNT_NFS:
		case MOUNT_MFS:
			argp = (caddr_t)dataptr;
			break;
	}

	ret = syscall(SYS_mount, fsnum, path, mflag, argp);

	if (ret == -1) {
		if ((_Geterrno() == ENOMEM) || (_Geterrno() == EMFILE)) 
			_Seterrno(EBUSY);
		else if (_Geterrno() == EDIRTY) 
			_Seterrno(ENOSPC);
	}
	return(ret);
}
