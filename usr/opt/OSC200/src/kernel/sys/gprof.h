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
 *	@(#)$RCSfile: gprof.h,v $ $Revision: 4.3.3.2 $ (DEC) $Date: 1992/12/09 07:08:19 $
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

#ifndef	_SYS_GPROF_H_
#define _SYS_GPROF_H_

#if defined(mips) || defined(__alpha)
/*
 * profiling header
 * used by kgmon to determine appropriate form of mon.out file
 */
struct phdr {
    int		proftype;	/* profile type, see below */
    char	*lowpc;		/* low text pc */
    char	*highpc;	/* high text pc */
    char	*pc_buf;	/* address of pc sample buckets */
    int		pc_bytes;	/* size of pc sample buckets in bytes */
    char	*bb_buf;	/* address of bb counters */
    int		bb_bytes;	/* size of invocation/bb counters in bytes */
    char	*tos_buf;	/* address of tos array */
    int		tos_bytes;	/* size of gprof tos array in bytes */
    char	*froms_buf;	/* address of froms array */
    int		froms_bytes;	/* size of gprof froms array in bytes */
    int		sample_hz;	/* frequency of sampling */
};

#else	/* mips || __alpha */

struct phdr {
    char	*lpc;
    char	*hpc;
    int		ncnt;
};

#endif	/* mips || __alpha */

    /*
     *	histogram counters are unsigned shorts (according to the kernel).
     */
#define HISTCOUNTER	unsigned short

    /*
     *	fraction of text space to allocate for histogram counters
     *	here, 1/2
     */
#define HISTFRACTION	2

    /*
     *	Fraction of text space to allocate for from hash buckets.
     *	The value of HASHFRACTION is based on the minimum number of bytes
     *	of separation between two subroutine call points in the object code.
     *	Given MIN_SUBR_SEPARATION bytes of separation the value of
     *	HASHFRACTION is calculated as:
     *
     *		HASHFRACTION = MIN_SUBR_SEPARATION / (2 * sizeof(short) - 1);
     *
     *	For the VAX, the shortest two call sequence is:
     *
     *		calls	$0,(r0)
     *		calls	$0,(r0)
     *
     *	which is separated by only three bytes, thus HASHFRACTION is 
     *	calculated as:
     *
     *		HASHFRACTION = 3 / (2 * 2 - 1) = 1
     *
     *	Note that the division above rounds down, thus if MIN_SUBR_FRACTION
     *	is less than three, this algorithm will not work!
     *
     *	NB: for the kernel we assert that the shortest two call sequence is:
     *
     *		calls	$0,_name
     *		calls	$0,_name
     *
     *	which is separated by seven bytes, thus HASHFRACTION is calculated as:
     *
     *		HASHFRACTION = 7 / (2 * 2 - 1) = 2
     *
     *	For Mips the shortest call sequence is:
     *
     *		jal	x
     *		jal	y
     *
     * which is separated by four bytes, thus HASHFRACTION is calculated as:
     *
     *		HASHFRACTION = 4 / (2 * 2 - 1) = 1
     */
#if defined(mips) || defined(__alpha)
#define HASHFRACTION	1
#else	/* mips */
#define HASHFRACTION	2
#endif	/* mips */

    /*
     *	percent of text space to allocate for tostructs
     *	with a minimum.
     */
#define ARCDENSITY	2
#define MINARCS		50

struct tostruct {
    char		*selfpc;
    long		count;
    unsigned short	link;
};

    /*
     *	a raw arc,
     *	    with pointers to the calling site and the called site
     *	    and a count.
     */
struct rawarc {
    unsigned long	raw_frompc;
    unsigned long	raw_selfpc;
    long		raw_count;
};

    /*
     *	general rounding functions.
     */
#define ROUNDDOWN(x,y)	(((x)/(y))*(y))
#define ROUNDUP(x,y)	((((x)+(y)-1)/(y))*(y))

#if defined(mips) || defined(__alpha)
/*
 * profile types
 */
#define PC_SAMPLES	0x1	/* pc samples taken */
#define INV_COUNTS	0x2	/* procedure invocations */
#define BB_COUNTS	0x4	/* basic block counts */
#define GPROF_COUNTS	0x8	/* gprof call graph info */

#define BB_SCALE	2	/* half of textsize for bb counts */

#endif	/* mips || __alpha */
#endif	/* _SYS_GPROF_H_ */
