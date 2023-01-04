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
static char	*sccsid = "@(#)$RCSfile: cdfs_config.c,v $ $Revision: 4.3.3.2 $ (DEC) $Date: 1992/05/01 16:10:48 $";
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
/*********************************************************************
 *			Modification History
 *
 * 23-May-91 -- prs
 *	Initial Creation.
 *
 *********************************************************************/
#include <cdfs.h>
#include <sys/mount.h>
#include <sys/errno.h>
#include <sys/sysconfig.h>

extern  struct vfsops cdfs_vfsops;

/*
 * ROUTINE:	cdfs_configure()
 *
 * FUNCTION:	ISO 9660 File System - Configuration entry point
 *
 * ALGORITH:	Branch on valid operations, else immediately return error.
 * 		Configure:
 *		1. Call VFS framework vfssw_add() routine to register us.
 *		   Note: If successful, it will have called our cdfs_init()).
 *		   Note: Will fail if CDFS is already registered.
 * 		Deconfigure:
 *		1. Call VFS framework vfssw_del() routine to deregister us.
 *		   Note:  Will fail if still referenced by mounts.
 *		   Note: Will fail if CDFS is not already registered.
 *
 * RETURNS:	0 if operation successful. 
 *		Otherwise, a non-zero error is returned.
 */

int
cdfs_configure(op, indata, indatalen, outdata, outdatalen )
	sysconfig_op_t		op;
	filesys_config_t * 	indata;
	size_t			indatalen;
	filesys_config_t * 	outdata;
	size_t			outdatalen;
{
        int     	ret;

        switch (op) {
		case SYSCONFIG_CONFIGURE:
			if ((ret=vfssw_add(MOUNT_CDFS, &cdfs_vfsops)) != 0)
				return ret;
			break;
		case SYSCONFIG_UNCONFIGURE:
			if ((ret=vfssw_del(MOUNT_CDFS)) != 0)
				return ret;
			break;
		default:
			return EINVAL;
			break;
        }
	if (outdata != NULL && outdatalen == sizeof(filesys_config_t)) {
		outdata->fc_version = OSF_FILESYS_CONFIG_10;
		outdata->fc_type = MOUNT_CDFS;
	}
        return 0;
}

