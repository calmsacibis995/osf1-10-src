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
static char *rcsid = "@(#)$RCSfile: utmpname.c,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/11/09 14:51:44 $";
#endif

/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#pragma weak utmpname = __utmpname
#endif

#include "ts_supp.h"
#ifdef	_THREAD_SAFE
#include "rec_mutex.h"

extern struct rec_mutex _utmp_rmutex;
#endif	/* _THREAD_SAFE */

#include <limits.h>
#include <utmp.h>
#ifdef	DEBUGX
#undef	UTMP_FILE
#define	UTMP_FILE	"utmp"
#endif	/* DEBUGX */
#define	MAXFILE	PATH_MAX + 1	/* max utmp filename length (inc. NULL) */

char	__utmpfile[MAXFILE] = UTMP_FILE;	/* Current utmp file */

void
utmpname(char *newfile)
{
	/*
	 * If the new filename will not fit, return.
	 */
	if (strlen(newfile) >= MAXFILE)
		return;

	TS_LOCK(&_utmp_rmutex);

	/* copy in the new file name */
	strcpy(__utmpfile, newfile);

	TS_UNLOCK(&_utmp_rmutex);

#ifndef _THREAD_SAFE
	/*
	 * Make sure everything is reset to the beginning state.
	 */
	endutent();
#endif	/* _THREAD_SAFE */
}
