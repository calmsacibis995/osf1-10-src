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
 *	@(#)$RCSfile: lock_types.h,v $ $Revision: 4.2.4.2 $ (DEC) $Date: 1992/03/18 18:46:22 $
 */ 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0
 */
/*
 * Copyright (C) 1988,1989 Encore Computer Corporation.
 * All Rights Reserved.
 *
 * Property of Encore Computer Corporation.
 * This software is made available solely pursuant to the terms of
 * a software license agreement which governs its use. Unauthorized
 * duplication, distribution or sale are strictly prohibited.
 *
 */

/*
 * lock_types.h:  distinguishing values for various locks.
 */

#ifndef	_SYS_LOCK_TYPES_H_
#define	_SYS_LOCK_TYPES_H_

#define	LTYPE_FS_START		0
#define	LTYPE_DEFAULT_UNI	(LTYPE_FS_START + 1)
#define	LTYPE_FILE_IO		(LTYPE_FS_START + 2)
#define	LTYPE_SPECH		(LTYPE_FS_START + 3)
#define	LTYPE_MOUNT_LOOKUP	(LTYPE_FS_START + 4)
#define	LTYPE_VNODE_AUX 	(LTYPE_FS_START + 5)
#define	LTYPE_BUF		(LTYPE_FS_START + 6)
#define	LTYPE_INODE_IO		(LTYPE_FS_START + 7)
#define	LTYPE_HOSTNAME		(LTYPE_FS_START + 8)
#define	LTYPE_PTY		(LTYPE_FS_START + 9)
#define	LTYPE_SCC_TTY		(LTYPE_FS_START + 10)
#define	LTYPE_FICHAIN		(LTYPE_FS_START + 11)
#define	LTYPE_VFSSW		(LTYPE_FS_START + 12)
#define	LTYPE_NFSNODE_IO	(LTYPE_FS_START + 13)
#define	LTYPE_CDEVSW		(LTYPE_FS_START + 14)
#define	LTYPE_BDEVSW		(LTYPE_FS_START + 15)
#define	LTYPE_NFSD		(LTYPE_FS_START + 16)
#define	LTYPE_DQUOT		(LTYPE_FS_START + 17)
#define	LTYPE_UMPQ		(LTYPE_FS_START + 18)
#define	LTYPE_FS_LAST		(LTYPE_FS_START + 18)

#define	LTYPE_INTR_START	(LTYPE_FS_LAST + 0)
#define	LTYPE_SWINTR		(LTYPE_INTR_START + 1)

/*
 * Network
 */
#define	LTYPE_NET_START		128
#define	LTYPE_TCP		(LTYPE_NET_START+0)
#define	LTYPE_INPCB		(LTYPE_NET_START+1)
#define	LTYPE_SOCKET		(LTYPE_NET_START+2)
#define	LTYPE_UDP		(LTYPE_NET_START+3)
#define	LTYPE_IP		(LTYPE_NET_START+4)
#define	LTYPE_RAW		(LTYPE_NET_START+5)
#define	LTYPE_ROUTE		(LTYPE_NET_START+6)
#define	LTYPE_ARP		(LTYPE_NET_START+7)
#define	LTYPE_SOCKBUF		(LTYPE_NET_START+8)
#define	LTYPE_DOMAIN		(LTYPE_NET_START+9)
#define	LTYPE_NFSREQ		(LTYPE_NET_START+10)
#define	LTYPE_NET_LAST		(LTYPE_NET_START+10)

#endif	/* _SYS_LOCK_TYPES_H_ */
