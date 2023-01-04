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
static char	*sccsid = "@(#)$RCSfile: unctime.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 06:04:41 $";
#endif 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0
 */
#if !defined( lint) && !defined(_NOIDENT)

#endif

/*
 * This module contains IBM CONFIDENTIAL code. -- (IBM Confidential Restricted
 * when combined with the aggregated modules for this product) OBJECT CODE ONLY
 * SOURCE MATERIALS (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or disclosure
 * restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * Copyright (c) 1980 Regents of the University of California. All
 * rights reserved.  The Berkeley software License Agreement specifies the
 * terms and conditions for redistribution.
 */

#include	<sys/types.h>
#include	<sys/time.h>
#include	<stdio.h>

/*
 * Convert a ctime(3) format string into a system format date. Return the date
 * thus calculated.
 *
 * Return -1 if the string is not in ctime format.
 */

/*
 * Offsets into the ctime string to various parts.
 */

#define	E_MONTH		4
#define	E_DAY		8
#define	E_HOUR		11
#define	E_MINUTE	14
#define	E_SECOND	17
#define	E_YEAR		20

static int		dcmp();
static int		lookup();
static time_t		emitl();

time_t
unctime(str)
	char	       *str;
{
	struct tm	then;

	if ((then.tm_mon = lookup(&str[E_MONTH])) < 0)
	{
		return(-1);
	}
	then.tm_mday = atoi(&str[E_DAY]);
	then.tm_hour = atoi(&str[E_HOUR]);
	then.tm_min = atoi(&str[E_MINUTE]);
	then.tm_sec = atoi(&str[E_SECOND]);
	then.tm_year = atoi(&str[E_YEAR]) - 1900;
	return(emitl(&then));
}

static char	months[] = "JanFebMarAprMayJunJulAugSepOctNovDec";

static int
lookup(str)
	register char  *str;
{
	register char  *cp;

	for (cp = months; *cp != '\0'; cp += 3)
	{
		if (strncmp(cp, str, 3) == 0)
		{
			return((cp - months) / 3);
		}
	}
	return(-1);
}

/*
 * Routine to convert a localtime(3) format date back into a system format
 * date.
 *
 * Use a binary search.
 */


static time_t
emitl(dp)
	struct tm      *dp;
{
	time_t		conv;
	register int	i, bit;
	struct tm      *localtime();

	conv = 0;
	for (i = 30; i >= 0; --i)
	{
		bit = 1 << i;

		/* set the bit */

		conv |= bit;

		/* if this new time, conv, is greater than the
		 * one being tested */

		if (dcmp(localtime(&conv), dp) > 0)
		{
			/* unset the bit again */

			conv &= ~bit;
		}
	}
	return(conv);
}

/*
 * Compare two localtime dates, return result.
 */

#define	DECIDE(a) \
	if (dp1->a > dp2->a)	\
	{			\
		return(1);	\
	}			\
	if (dp1->a < dp2->a)	\
	{			\
		return(-1);	\
	}

static int
dcmp(dp1, dp2)
	register struct tm     *dp1, *dp2;
{

	DECIDE(tm_year);
	DECIDE(tm_mon);
	DECIDE(tm_mday);
	DECIDE(tm_hour);
	DECIDE(tm_min);
	DECIDE(tm_sec);
	return(0);
}
