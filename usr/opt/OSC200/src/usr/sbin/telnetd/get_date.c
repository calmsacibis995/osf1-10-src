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
static char	*sccsid = "@(#)$RCSfile: get_date.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 06:31:34 $";
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
/*
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

#ifndef lint

#endif not lint

#include <stdio.h>
#include <sys/time.h>

static char *days[] = {
	"Sun", "Mon", "Tues", "Wed", "Thurs", "Fri", "Sat"
};

static char *months[] = {
	"Jan", "Feb", "Mar", "Apr", "May", "June",
	"July", "Aug", "Sept", "Oct", "Nov", "Dec"
};

#define AM "am"
#define PM "pm"

get_date(datebuffer)
	char *datebuffer;
{
	struct tm *localtime(), *tmp;
	struct timeval tv;
	int realhour;
	char *zone;

	gettimeofday(&tv, 0);
	tmp = localtime(&tv.tv_sec);

	realhour = tmp->tm_hour;
	zone = AM;			/* default to morning */
	if (tmp->tm_hour == 0)
		realhour = 12;		/* midnight */
	else if (tmp->tm_hour == 12)
		zone = PM;		/* noon */
	else if (tmp->tm_hour >= 13 && tmp->tm_hour <= 23) { /* afternoon */
		realhour = realhour - 12;
		zone = PM;
	}
	
	/* format is '8:10pm on Sunday, 16 Sept 1973' */

	(void)sprintf(datebuffer, "%d:%02d%s on %s, %d %s %d",
		realhour,
		tmp->tm_min,
		zone,
		days[tmp->tm_wday],
		tmp->tm_mday,
		months[tmp->tm_mon],
		1900 + tmp->tm_year);
}
