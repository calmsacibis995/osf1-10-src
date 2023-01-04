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
 *	@(#)$RCSfile: mode.h,v $ $Revision: 1.2 $ (DEC) $Date: 1992/01/15 01:12:26 $
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
 * derived from mode.h	2.1	(ULTRIX/OSF)	12/3/90
 */

/*
 * sdb/adb - common definitions for old srb style code
 */

/*	mode.h	4.2	81/05/14	*/

#include <hal/kdb/machine.h>
#define MAXCOM	64
#define MAXARG	32
#define LINSIZ	512

typedef	char		*string_t;
typedef	char		msg[];

typedef	struct bkpt	bkpt;
typedef	bkpt		*bkpt_t;

struct bkpt {
	long	loc;
	long	ins;
	short	count;
	short	initcnt;
	short	flag;
	char	comm[MAXCOM];
	bkpt	*nxtbkpt;
};

typedef	struct reglist	REGLIST;
typedef	REGLIST		*REGPTR;
struct reglist {
	string_t	rname;
	short	roffs;
	int	*rkern;
};
