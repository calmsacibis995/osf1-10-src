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
 * @(#)$RCSfile: ufs_ubc.h,v $ $Revision: 1.1.3.2 $ (DEC) $Date: 1992/03/18 18:54:11 $
 */
#ifndef	__UFS_UBC__
#define	__UFS_UBC__	1


struct ufs_realloc {
	int 	ur_flags;			/* Flag for UBC */
	union {
		daddr_t _ur_blkno;		/* New block allocated */
		struct buf **_ur_bpp;		/* Buffer pointer returned */
	} ur_u;
};

#define	ur_blkno	ur_u._ur_blkno
#define	ur_bpp		ur_u._ur_bpp

#endif	/* !__UFS_UBC__ */
