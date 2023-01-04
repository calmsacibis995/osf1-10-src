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
/* tws.h */
/* @(#)$RCSfile: tws.h,v $ $Revision: 4.2.3.2 $ (DEC) $Date: 1992/06/03 16:33:28 $ devrcs Exp Locker: devbld $ */

#define	DSTXXX
			/* DST vs. GMT nonsense */

struct tws {
    int     tw_sec;
    int     tw_min;
    int     tw_hour;

    int     tw_mday;
    int     tw_mon;
    int     tw_year;

    int     tw_wday;
    int     tw_yday;

    int     tw_zone;

    long    tw_clock;

    int     tw_flags;
#define	TW_NULL	0x0000
#define	TW_SDAY	0x0003		/* how day-of-week was determined */
#define	  TW_SNIL	0x0000	/*   not given */
#define	  TW_SEXP	0x0001	/*   explicitly given */
#define	  TW_SIMP	0x0002	/*   implicitly given */
#define	TW_SZONE 0x0004		/* how timezone was determined */
#define	  TW_SZNIL	0x0000	/*   not given */
#define	  TW_SZEXP	0x0004	/*   explicitly given */
#define	TW_DST	0x0010		/* daylight savings time */
#define	TW_ZONE	0x0020		/* use numeric timezones only */
};

void    twscopy ();
int	twsort ();
long	twclock ();
char   *dasctime (), *dtimezone (), *dctime (), *dtimenow ();
struct tws *dgmtime(), *dlocaltime (), *dparsetime (), *dtwstime ();

#ifndef	ATZ
#define	dtime(cl)	dasctime (dlocaltime (cl), TW_ZONE)
#else	/* ATZ */
#define	dtime(cl)	dasctime (dlocaltime (cl), TW_NULL)
#endif	/* ATZ */
#define	dtwszone(tw)	dtimezone (tw -> tw_zone, tw -> tw_flags)


extern char   *tw_dotw[], *tw_ldotw[], *tw_moty[];
