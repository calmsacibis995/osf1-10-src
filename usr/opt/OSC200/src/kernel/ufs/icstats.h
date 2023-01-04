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
 *	@(#)$RCSfile: icstats.h,v $ $Revision: 4.2.3.2 $ (DEC) $Date: 1993/07/14 18:19:45 $
 */ 
/*
 */
/*
 * Inode cache statistics.
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0
 */

#ifndef	_UFS_ICSTATS_H_
#define	_UFS_ICSTATS_H_

#if	INOCACHE_STATS
struct icache_stats {
	u_int	ic_iget_call;		/* times iget called */
	u_int	ic_iget_loop;		/* iget started over from scratch */
	u_int	ic_iget_research;	/* iget searched hash for duplicate */
	u_int	ic_iget_vget;		/* iget stumbled on funny vnode */
	u_int	ic_iget_hit;		/* cache hits */
	u_int	ic_iget_error;		/* error reading inode from disk */
	u_int	ic_iget_insert;		/* insertions into inode cache */
	u_int	ic_iget_reallocd;	/* freed, then reactivated */
};
extern struct icache_stats icache_stats;
#endif

#endif	/* _UFS_ICSTATS_H_ */
