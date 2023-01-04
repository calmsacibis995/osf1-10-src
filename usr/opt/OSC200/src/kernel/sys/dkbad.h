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
 *	@(#)$RCSfile: dkbad.h,v $ $Revision: 4.3 $ (DEC) $Date: 1992/01/15 02:10:07 $
 */ 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/* 
 * Mach Operating System
 * Copyright (c) 1989 Carnegie-Mellon University
 * All rights reserved.  The CMU software License Agreement specifies
 * the terms and conditions for use and redistribution.
 */
/*
 * OSF/1 Release 1.0
 */
/*
 * Copyright (c) 1982, 1986 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 */

#ifndef	_SYS_DKBAD_H_
#define _SYS_DKBAD_H_

#include <sys/types.h>

/*
 * Definitions needed to perform bad sector
 * revectoring ala DEC STD 144.
 *
 * The bad sector information is located in the
 * first 5 even numbered sectors of the last
 * track of the disk pack.  There are five
 * identical copies of the information, described
 * by the dkbad structure.
 *
 * Replacement sectors are allocated starting with
 * the first sector before the bad sector information
 * and working backwards towards the beginning of
 * the disk.  A maximum of 126 bad sectors are supported.
 * The position of the bad sector in the bad sector table
 * determines which replacement sector it corresponds to.
 *
 * The bad sector information and replacement sectors
 * are conventionally only accessible through the
 * 'c' file system partition of the disk.  If that
 * partition is used for a file system, the user is
 * responsible for making sure that it does not overlap
 * the bad sector information or any replacement sector.s
 */

struct dkbad {
	int	bt_csn;			/* cartridge serial number */
	u_short	bt_mbz;			/* unused; should be 0 */
	u_short	bt_flag;		/* -1 => alignment cartridge */
	struct bt_bad {
		u_short	bt_cyl;		/* cylinder number of bad sector */
		u_short	bt_trksec;	/* track and sector number */
	} bt_bad[126];
};

#define ECC	0
#define SSE	1
#define BSE	2
#define CONT	3

#endif	/* _SYS_DKBAD_H_ */
