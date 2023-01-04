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
static char *rcsid = "@(#)$RCSfile: nfs_common.c,v $ $Revision: 1.1.6.2 $ (DEC) $Date: 1993/06/09 12:43:36 $";
#endif
/*
 * Replacement for OSF/NFS server code built from ONC/NFS + ULTRIX/NFS code.
 */

#include <sys/param.h>
#include <sys/errno.h>
#include <sys/systm.h>
#include <sys/time.h>
#include <sys/user.h>
#include <sys/proc.h>
#include <sys/mount.h>
#include <sys/vnode.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <dirent.h>
#include <sys/mbuf.h>
#include <rpc/rpc.h>
#include <nfs/nfs.h>
#include <nfs/nfssys.h>
#include <ufs/inode.h>
#include <ufs/dir.h>
#include <sys/presto.h>
#include <sys/prestoioctl.h>
#include <sys/fs_types.h>	

#include <sys/uio.h>
#include <sys/ucred.h>
#include <sys/namei.h>
#include <sys/errno.h>

/*
 * General utilities
 */

/*
 * Returns the prefered transfer size in bytes based on
 * what network interfaces are available.
 */
nfstsize()
{
	return (8192);
}

vattr_to_nattr(vap, na)
	struct vattr *vap;
	struct nfsfattr *na;
{

	na->na_type = (enum nfsftype)vap->va_type;
        if (vap->va_mode == (unsigned short) -1)
                na->na_mode = (unsigned int) -1;
        else
                na->na_mode = MAKEIMODE(vap->va_type, vap->va_mode);
 
        if (vap->va_uid == (uid_t) -1)
                na->na_uid = (unsigned int) -1;
        else
                na->na_uid = vap->va_uid;

        if (vap->va_gid == (gid_t) -1)
                na->na_gid = (unsigned int) -1;
        else
                na->na_gid = vap->va_gid;

	na->na_fsid = vap->va_fsid;
	na->na_nodeid = vap->va_fileid;
	na->na_nlink = vap->va_nlink;
	/* For > 2GB files, return 2 GB */
	if (vap->va_size > 0x7fffffff)
		na->na_size = 0x7fffffff;
	else
		na->na_size = vap->va_size;
	na->na_atime.tv_sec  = vap->va_atime.tv_sec;
	na->na_atime.tv_usec = vap->va_atime.tv_usec;
	na->na_mtime.tv_sec  = vap->va_mtime.tv_sec;
	na->na_mtime.tv_usec = vap->va_mtime.tv_usec;
	na->na_ctime.tv_sec  = vap->va_ctime.tv_sec;
	na->na_ctime.tv_usec = vap->va_ctime.tv_usec;
	na->na_rdev = vap->va_rdev;
	na->na_blocks = (vap->va_bytes / NFS_FABLKSIZE);
	na->na_blocksize = vap->va_blocksize;

	/*
	 * This bit of ugliness is a *TEMPORARY* hack to preserve the
	 * over-the-wire protocols for named-pipe vnodes.  It remaps the
	 * VFIFO type to the special over-the-wire type. (see note in nfs.h)
	 *
	 * BUYER BEWARE:
	 *  If you are porting the NFS to a non-SUN server, you probably
	 *  don't want to include the following block of code.  The
	 *  over-the-wire special file types will be changing with the
	 *  NFS Protocol Revision.
	 */
	if (vap->va_type == VFIFO)
		NA_SETFIFO(na);
}

sattr_to_vattr(sa, vap)
	register struct nfssattr *sa;
	register struct vattr *vap;
{

	vattr_null(vap);
	vap->va_mode = sa->sa_mode;
	vap->va_uid = (uid_t)sa->sa_uid;
	vap->va_gid = (gid_t)sa->sa_gid;
	if (sa->sa_size == (u_int) -1)
		vap->va_size = VNOVAL;
	else
		vap->va_size = sa->sa_size;
	vap->va_atime.tv_sec  = sa->sa_atime.tv_sec;
	vap->va_atime.tv_usec = sa->sa_atime.tv_usec;
	vap->va_mtime.tv_sec  = sa->sa_mtime.tv_sec;
	vap->va_mtime.tv_usec = sa->sa_mtime.tv_usec;
}

/*
 * Print a file handle
 */
printfhandle(fh)
        caddr_t fh;
{
        int i;
        int fhint[NFS_FHSIZE / sizeof (int)];

        bcopy(fh, (caddr_t)fhint, NFS_FHSIZE);
        for (i = 0; i < (sizeof (fhint) / sizeof (int)); i++) {
                printf("%x ", fhint[i]);
        }
}

