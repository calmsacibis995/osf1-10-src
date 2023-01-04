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
 *	@(#)$RCSfile: biostats.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:56:37 $
 */ 
/*
 */
/*
 * Buffer cache statistics.
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0
 */

#ifndef	_SYS_BIOSTATS_H_
#define _SYS_BIOSTATS_H_

#if	BUFCACHE_STATS
struct bio_stats {
	int	getblk_hits;
	int	getblk_misses;
	int	getblk_research;
	int	getblk_dupbuf;
	int	getnewbuf_calls;
	int	getnewbuf_buflocked;
	int	vflushbuf_lockskips;
	int	mntflushbuf_misses;
	int	mntinvalbuf_misses;
	int	vinvalbuf_misses;
	int	allocbuf_buflocked;
	int	ufssync_misses;
};

#ifdef	_KERNEL
extern struct bio_stats 	bio_stats;
#define	BUF_STATS(clause)	STATS_ACTION(&bio_stats_lock, (clause))
vdecl_simple_lock_data(,bio_stats_lock)
#endif

#else	/* BUFCACHE_STATS */

#ifdef	_KERNEL
#define	BUF_STATS(clause)
#endif

#endif	/* BUFCACHE_STATS */
#endif	/* _SYS_BIOSTATS_H_ */
