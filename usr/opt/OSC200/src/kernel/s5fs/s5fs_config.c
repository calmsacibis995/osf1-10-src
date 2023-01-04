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
static char	*sccsid = "@(#)$RCSfile: s5fs_config.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:51:45 $";
#endif 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * History:
 *
 *	August 20, 1991: vsp
 *		OSF/1 Release 1.0.1 bug fixes.
 *
 * 	ADK:
 *		original OSF/1 Release 1.0
 *
 *
 */

#include <sysv_fs.h>
#include <sys/mount.h>
#include <sys/errno.h>
#include <sys/sysconfig.h>

extern  struct vfsops s5fs_vfsops;

/*
 * ROUTINE:	configure_S5FS()
 *
 * FUNCTION:	SysV File System - Configuration entry point
 *
 * ALGORITH:	Branch on valid operations, else immediately return error.
 * 		Configure:
 *		1. Call VFS framework vfssw_add() routine to register us.
 *		   Note: If successful, it will have called our s5fs_init()).
 *		   Note: Will fail if NFS is already registered.
 * 		Deconfigure:
 *		1. Call VFS framework vfssw_del() routine to deregister us.
 *		   Note:  Will fail if still referenced by mounts.
 *		   Note: Will fail if NFS is not already registered.
 *		2. Deallocate allocated memory.
 *
 * RETURNS:	0 if operation successful. 
 *		Otherwise, a non-zero error is returned.
 */

int
sysv_fs_configure( op, indata, indatalen, outdata, outdatalen )
	sysconfig_op_t		op;
	filesys_config_t * 	indata;
	size_t			indatalen;
	filesys_config_t * 	outdata;
	size_t			outdatalen;
{
        int     	ret;

        switch (op) {
		case SYSCONFIG_CONFIGURE:
			if ((ret=vfssw_add(MOUNT_S5FS, &s5fs_vfsops)) != 0)
				return ret;
			break;
		case SYSCONFIG_UNCONFIGURE:
			if ((ret=vfssw_del(MOUNT_S5FS)) != 0)
				return ret;
			s5fs_uninit();
			break;
		default:
			return EINVAL;
			break;
        }
	if (outdata != NULL && outdatalen == sizeof(filesys_config_t)) {
		outdata->fc_version = OSF_FILESYS_CONFIG_10;
		outdata->fc_type = MOUNT_S5FS;
	}
        return 0;
}

