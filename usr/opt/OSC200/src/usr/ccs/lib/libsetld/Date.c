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
static  char *sccsid = "@(#)$RCSfile: Date.c,v $ $Revision: 4.2.3.2 $ (DEC) $Date: 1992/10/13 13:34:19 $";
#endif
/*
 */
#if !defined( lint) && !defined(_NOIDENT)

#endif
/*	Date.c -
 *		routines for DateT
 *
 *	mods:
 *	000	19-jun-1989	ccb
 *		pulled from inv.c
*/

#include	<sys/param.h>
#include	<sys/types.h>
#include	<sys/dir.h>
#include	<time.h>
#include	<stdio.h>
#include	<setld/setld.h>

#define	I_DATFMT	"%d/%d/%d"

/*LINTLIBRARY*/

/*	char	*DateFormat() -
 *		format a time_t as a date in inventory format.
 *
 *	given:	time_t t - the time to format
 *	does:	formats the time into a static buffer
 *	return:	a pointer to the (static) buffer
*/

char *DateFormat(t)
time_t t;
{
	struct tm	*tms;
	static DateT	ds;

	tms = localtime( (long *) &t );
	(void) sprintf( ds, I_DATFMT, tms->tm_mon + 1, tms->tm_mday,
		tms->tm_year );
	return( (char *) ds );
}

