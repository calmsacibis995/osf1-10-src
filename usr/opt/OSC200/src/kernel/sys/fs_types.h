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
/*	
 *	@(#)$RCSfile: fs_types.h,v $ $Revision: 4.3.9.4 $ (DEC) $Date: 1993/08/27 14:49:19 $	
 */ 
/*
 */

#ifndef	_SYS_FS_TYPES_H_
#define _SYS_FS_TYPES_H_

#ifndef KERNEL
/* filesystem name strings for user utilities */

/* WARNING:
 * 	The constants defined in sys/mount.h are the indexes to the
 *	associated filesystem name string in this array.  Any changes
 *	in this array should be reflected in sys/mount.h so the
 *	filesystem name constants always index to the associated name in
 *	this array.
 */
#define MNT_NUMTYPES	128
char *mnt_names[MNT_NUMTYPES] = {
	"unknown", "ufs", "nfs", "mfs", "pcfs", "sysv", "cdfs", 
        "dfs", "efs", "procfs", "advfs", "ffm", "fdfs", "addon", 0
};
#endif

#endif	/* _SYS_FS_TYPES_H_ */
