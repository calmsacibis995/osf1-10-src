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
 * @(#)$RCSfile: tzs.h,v $ $Revision: 1.1.5.2 $ (DEC) $Date: 1993/10/05 21:03:38 $
 */
/************************************************************************
 *			Modification History				*
 *									*
 * 001	Ken Lesniak, 12-Apr-1989					*
 *	Time zone state structures for ctime.c and tzset.h		*
 ************************************************************************/

struct ttinfo {				/* time type information */
	long		tt_gmtoff;	/* GMT offset in seconds */
	int		tt_isdst;	/* used to set tm_isdst */
	int		tt_abbrind;	/* abbreviation list index */
};

struct state {
	int		timecnt;
	time_t		ats[TZ_MAX_TIMES];
	unsigned char	types[TZ_MAX_TIMES];
	struct ttinfo	ttis[TZ_MAX_TYPES];
	char		chars[TZNAME_MAX * 2 + 2];
};

/*
 * Size of retained TZ variable: Previous comment said "POSIX.1 allows
 * 54+TZNAME_MAX; leave a little slop."  However, this didn't take into
 * account 2 time zone names (std and dst) in a TZ, which is required
 * in XPG4 test suite.  In addition, this needs to be large enough to
 * hold a file name if the :<tzfile> format is used, so PATH_MAX + 2 is
 * large enough to hold all of these.
 */
#define	LAST_TZ_LEN	(PATH_MAX + 2)
