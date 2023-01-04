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
 * @(#)$RCSfile: mount.h,v $ $Revision: 1.1.3.2 $ (DEC) $Date: 1992/03/18 18:36:23 $
 */
/*	@(#)mount.h	1.3 90/07/19 4.1NFSSRC SMI	*/

/* 
 * Copyright (c) 1988,1990 by Sun Microsystems, Inc.
 */

#ifndef _rpcsvc_mount_h
#define _rpcsvc_mount_h

#define MOUNTPROG 100005
#define MOUNTPROC_MNT		1
#define MOUNTPROC_DUMP		2
#define MOUNTPROC_UMNT		3
#define MOUNTPROC_UMNTALL	4
#define MOUNTPROC_EXPORT	5
#define MOUNTPROC_EXPORTALL	6
#define MOUNTPROC_PATHCONF	7
#define MOUNTVERS		1
#define MOUNTVERS_POSIX		2

#ifndef svc_getcaller
#define svc_getcaller(x) (&(x)->xp_raddr)
#endif


bool_t xdr_fhstatus();
#ifndef KERNEL
bool_t xdr_path();
bool_t xdr_fhandle();
bool_t xdr_mountlist();
bool_t xdr_exports();
bool_t xdr_pathcnf();
#endif /* ndef KERNEL */

struct mountlist {		/* what is mounted */
	char *ml_name;
	char *ml_path;
	struct mountlist *ml_nxt;
};

struct fhstatus {
	int fhs_status;
	nfsv2fh_t fhs_fh;
};

/*
 * List of exported directories
 * An export entry with ex_groups
 * NULL indicates an entry which is exported to the world.
 */
struct exports {
	dev_t		  ex_dev;	/* dev of directory */
	char		 *ex_name;	/* name of directory */
	struct groups	 *ex_groups;	/* groups allowed to mount this entry */
	struct exports	 *ex_next;
	short		  ex_rootmap;	/* id to map root requests to */
	short		  ex_flags;	/* bits to mask off file mode */
};

struct groups {
	char		*g_name;
	struct groups	*g_next;
};

#endif /*!_rpcsvc_mount_h*/
