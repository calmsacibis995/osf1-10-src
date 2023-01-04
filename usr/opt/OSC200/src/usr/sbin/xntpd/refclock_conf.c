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
#ifndef lint
static char     *sccsid = "@(#)$RCSfile: refclock_conf.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 06:39:27 $";
#endif
/*
 */

/*
 * refclock_conf.c - reference clock configuration
 */
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include <ntp/ntp_syslog.h>
#include <ntp/ntp_fp.h>
#include <ntp/ntp.h>
#include <ntp/ntp_refclock.h>

#ifdef REFCLOCK

#ifdef LOCAL_CLOCK
extern int local_start();
extern void local_shutdown(), local_poll(), local_control();
extern void local_init();
#endif

#ifdef PST
extern int pst_start();
extern void pst_shutdown(), pst_leap(), pst_control();
extern void pst_init(), pst_buginfo();
#endif

#ifdef CHU
extern int chu_start();
extern void chu_shutdown(), chu_poll(), chu_control();
extern void chu_init();
#endif

#ifdef WWVB
extern int wwvb_start();
extern void wwvb_shutdown(), wwvb_poll(), wwvb_leap(), wwvb_control();
extern void wwvb_init(), wwvb_buginfo();
#endif


/*
 * Order is clock_start(), clock_shutdown(), clock_poll(), clock_leap(),
 * clock_control(), clock_init(), clock_xmitinterval, clock_flags;
 *
 * Types are defined in ntp.h.  The index must match this.
 */
struct refclock refclock_conf[] = {
	{ noentry, noentry, noentry, noentry,	/* 0 REFCLOCK_NONE */
	  noentry, noentry, noentry, NOPOLL, NOFLAGS },

#ifdef LOCAL_CLOCK
	{ local_start, local_shutdown, local_poll, noentry,
	  local_control, local_init, noentry, STDPOLL, NOFLAGS },
#else
	{ noentry, noentry, noentry, noentry,	/* 1 REFCLOCK_LOCALCLOCK */
	  noentry, noentry, noentry, NOPOLL, NOFLAGS },
#endif

	{ noentry, noentry, noentry, noentry,	/* 2 REFCLOCK_WWV_HEATH */
	  noentry, noentry, noentry, NOPOLL, NOFLAGS },

#ifdef PST
	{ pst_start, pst_shutdown, noentry, pst_leap,
	  pst_control, pst_init, pst_buginfo, STDPOLL, NOFLAGS },
#else
	{ noentry, noentry, noentry, noentry,	/* 3 REFCLOCK_WWV_PST */
	  noentry, noentry, noentry, NOPOLL, NOFLAGS },
#endif

#ifdef WWVB
	{ wwvb_start, wwvb_shutdown, wwvb_poll, wwvb_leap,
	  wwvb_control, wwvb_init, wwvb_buginfo, STDPOLL, NOFLAGS },
#else
	{ noentry, noentry, noentry, noentry,	/* 4 REFCLOCK_WWVB_SPECTRACOM */
	  noentry, noentry, noentry, NOPOLL, NOFLAGS },
#endif

	{ noentry, noentry, noentry, noentry,	/* 5 REFCLOCK_GOES_TRUETIME */
	  noentry, noentry, noentry, NOPOLL, NOFLAGS },

	{ noentry, noentry, noentry, noentry,	/* 6 REFCLOCK_GOES_TRAK */
	  noentry, noentry, noentry, NOPOLL, NOFLAGS },

#ifdef CHU
	{ chu_start, chu_shutdown, chu_poll, noentry,
	  chu_control, chu_init, noentry, STDPOLL, NOFLAGS },
#else
	{ noentry, noentry, noentry, noentry,	/* 7 REFCLOCK_CHU */
	  noentry, noentry, noentry, NOPOLL, NOFLAGS },
#endif

	{ noentry, noentry, noentry, noentry,	/* extra, no comma for ANSI */
	  noentry, noentry, noentry, NOPOLL, NOFLAGS }
};

int num_refclock_conf = sizeof(refclock_conf)/sizeof(struct refclock);

#endif
