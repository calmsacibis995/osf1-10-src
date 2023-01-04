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
 *	@(#)$RCSfile: utctime.h,v $ $Revision: 4.2.3.5 $ (DEC) $Date:
92/11/30 16:53:41 $
 */ 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/************************************************************************
 *									*
 *			Copyright (c) 1990 by				*
 *		Digital Equipment Corporation, Maynard, MA		*
 *			All rights reserved.				*
 *									*
 *   This software is furnished under a license and may be used and	*
 *   copied  only  in accordance with the terms of such license and	*
 *   with the  inclusion  of  the  above  copyright  notice.   This	*
 *   software  or  any  other copies thereof may not be provided or	*
 *   otherwise made available to any other person.  No title to and	*
 *   ownership of the software is hereby transferred.			*
 *									*
 *   The information in this software is subject to change  without	*
 *   notice  and should not be construed as a commitment by Digital	*
 *   Equipment Corporation.						*
 *									*
 *   Digital assumes no responsibility for the use  or  reliability	*
 *   of its software on equipment which is not supplied by Digital.	*
 *									*
 ************************************************************************/
/*
 * OSF/1 Release 1.0
 */

#ifndef _UTCTIME_H
#define _UTCTIME_H

#ifndef BYTE_ORDER
#include <machine/endian.h>
#endif

/*
 * Structure definitions for utctime used by DECdts
 */

#if BYTE_ORDER == BIG_ENDIAN
struct int64 {
	unsigned long hi;
	unsigned long lo;
};
#else /* BYTE_ORDER == LITTLE_ENDIAN || BYTE_ORDER == PDP_ENDIAN  or ALPHA */
#ifdef	__alpha
struct int64 {
        unsigned long lo;
};
#else
struct int64 {
	unsigned long lo;
	unsigned long hi;
};
#endif 
#endif 

/*
 * Define the utc timestamp structure - This is defined to be in the
 * natural byte order for this endian processor. See the comments in
 * the DTSS API (Application Programmer Interface). Note for PDP-11's
 * straight Little Endian is used! This is somewhat unnatural on the PDP's
 * but since there are only two form of the timestamp permitted, it must
 * be converted into one of the two forms. (Besides is anyone really
 * interested in a PDP?)
 * This structure MUSt be 128 bits long.
 */

#ifndef _UTC_T_
#define _UTC_T_
#if BYTE_ORDER == LITTLE_ENDIAN
typedef struct utc {
#ifdef	__alpha
	struct int64	time;
	unsigned int inacclo;
	unsigned short int inacchi;
	int tdf : 12;
	unsigned vers : 3;
	int big_endian : 1;	/* This field must be the most
				   significant bit of the last byte */
#else
	struct int64	time;
	unsigned long int inacclo;
	unsigned short int inacchi;
	int tdf : 12;
	unsigned vers : 3;
	int big_endian : 1;	/* This field must be the most
				   significant bit of the last byte */
#endif
} utc_t;
#endif
#if BYTE_ORDER == BIG_ENDIAN
typedef struct utc {
	struct int64	time;
	unsigned short int inacchi;
	unsigned short int inaccmid;
	unsigned short int inacclo;
	char tdfhi;
	int big_endian : 1;	/* This field must be the most
				   significant bit of the last byte */
	unsigned vers : 3;
	unsigned int tdflo : 4;
} utc_t;
#endif
#if BYTE_ORDER == PDP_ENDIAN
typedef struct utc {
	unsigned short int timelolo;
	unsigned short int timelohi;
	unsigned short int timehilo;
	unsigned short int timehihi;
	unsigned short int inacclo;
	unsigned short int inaccmid;
	unsigned short int inacchi;
	int tdf : 12;
	unsigned vers : 3;
	int big_endian : 1;	/* This field must be the most
				   significant bit of the last byte */
} utc_t;
#endif

/* For a 64-bit little endian:
typedef struct utc {
	long int time;
	long int inacc : 48;
	int tdf : 12;
	unsigned vers : 3;
	int big_endian : 1;	   This field must be the most
				   significant bit of the last byte
}; */
/* For a 64-bit big endian?
typedef struct utc {
	long int time;
	int big_endian : 1;	   This field must be the most
				   significant bit of the last byte
	unsigned vers : 3;
	unsigned int tdflo : 4;
	int tdfhi : 8;
	long int inacc : 48;
}; */
#endif         /* _UTC_T_ */

/*
 * Define random other constants
 */
#define K_UTC_VERSION	(1)
#define K_100NS_PER_SEC (10000000)
#define K_US_PER_SEC    (1000000)
#define K_NS_PER_SEC    (1000000000)

enum adjmode {
	settime,  	/* Set time */
	adjusttime,	/* Adjust time */
	endadjust,	/* End adjust time */
	getresolution,  /* Get resolution of clock */
	getdrift,       /* Get drift reciprocal */
	setfreq,	/* Set (tweek) the clock frequency */
	getfreq,	/* Get the clock frequency */
	lastmode	/* Used as limit test */
};

#ifndef _TIMESPEC
#define _TIMESPEC

#ifndef _TIME_T
#define _TIME_T
typedef long            time_t;
#endif /* _TIME_T */ 

typedef struct timespec {
        time_t  tv_sec;         /* seconds */
        long    tv_nsec;        /* microseconds */
} timespec_t;

#endif /* _TIMESPEC */

union adjunion {
    struct adjargs {
	struct timespec	a_adjustment; 	/* Amount to adjust or change */
	struct timespec	a_comptime;	/* Time which corresponds to base
					   inaccuracy */
	struct int64	a_baseinacc;	/* Base inaccuracy */
	struct int64	a_leaptime;	/* Time of next potential leap
					   second */
	long int	a_adjrate;	/* Rate at which to adjust
						1000 = .1% (.0768% on PMAX)
						100 = 1% (.999% on PMAX)
						10 = 10%, etc.
					   Ignored for set time */
	long int	a_curtdf;	/* Current timezone */
	long int	a_nextdf;	/* Next timezone (eg. Daylight time) */
	long int	a_tdftime;	/* Time of next timezone change */
    } adjargs;				/* Adustment args */
    long int		resolution;	/* Resolution of clock in nanosecs */
    unsigned long int	*maxdrift;	/* Maximun drift rate of clock */
    struct trimargs {
	long int	t_frequency;	/* New frequency trim of clock */
	long int	t_maxdrift;	/* New maximun drift rate of clock */
    } trimargs;
    long int		frequency;	/* Current frequency trim of clock */
};

#endif /* _UTCTIME_H */
