
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
static char *rcsid = "@(#)$RCSfile: statvfs.c,v $ $Revision: 1.1.3.2 $ (DEC) $Date: 1993/08/23 20:37:34 $";
#endif

/*
 * FUNCTIONS: statvfs, fstatvfs
 *
 * DESCRIPTION:
 *	System V compatible statvfs and fstatvfs. This library routine
 * takes SVID 3 compliant calls to statvfs anf fstatvfs and translates
 * them into calls to statfs and fstatfs
 */

#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#pragma weak statvfs = __statvfs
#pragma weak fstatvfs = __fstatvfs
#endif
#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/mount.h>
#include <sys/statvfs.h>
#include <unistd.h>
#include <errno.h>
#define mnt_names _My_mnt_names
#include <sys/fs_types.h>

int
statvfs(const char *path, struct statvfs *buf)
{
	struct statfs tmp_buf;
	int ret;

	ret=syscall(SYS_statfs,path,&tmp_buf,sizeof(tmp_buf));
	if (ret) return(ret);

	buf->f_namemax=(long)pathconf((char *)path, _PC_NAME_MAX) ;

	ret=fs_to_vfs(&tmp_buf, buf);
	return(ret);
}	

int
fstatvfs(int fildes, struct statvfs *buf)
{
       struct statfs tmp_buf;
       int ret;

	ret=syscall(SYS_fstatfs,fildes,&tmp_buf,sizeof(tmp_buf));
        if(ret) return(ret);

	buf->f_namemax=(long)fpathconf(fildes, _PC_NAME_MAX) ;

	ret=fs_to_vfs(&tmp_buf, buf);
	return(ret);
}

static int
fs_to_vfs(struct statfs *AES_buf, struct statvfs *SVID_buf)
{
	SVID_buf->f_bsize = (ulong)(AES_buf->f_bsize);
	SVID_buf->f_frsize = (ulong)(AES_buf->f_fsize);
	SVID_buf->f_blocks = (ulong)(AES_buf->f_blocks);
	SVID_buf->f_bfree = (ulong)(AES_buf->f_bfree);
	SVID_buf->f_bavail = (ulong)(AES_buf->f_bavail);
	SVID_buf->f_files = (ulong)(AES_buf->f_files);
	SVID_buf->f_ffree = (ulong)(AES_buf->f_ffree);
	SVID_buf->f_favail = (ulong)(AES_buf->f_ffree);
	SVID_buf->f_fsid = (ulong)(AES_buf->f_fsid.val[0]);
	strcpy(SVID_buf->f_basetype,mnt_names[AES_buf->f_type]);
	SVID_buf->f_flag = (ulong)(AES_buf->f_flags);
	SVID_buf->f_fstr[0] = '\0';
	
	return(0);
}
