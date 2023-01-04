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
static char	*sccsid = "@(#)$RCSfile: getlogin.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 03:01:29 $";
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
/* getlogin.c -- library stub to transform calling sequence
 *
 * user program calls:
 * 	char *name = getlogin();
 *
 * kernel-side interface:
 *	getlogin(char *namebuf, int namelen);
 *
 */

#include <sys/param.h>
#ifdef _THREAD_SAFE
#include <errno.h>
#endif

#define MAXLOGNAME 12		/* for now; too hard to find include files */

extern int _getlogin(char *namebuf, int namelen);

#ifdef _THREAD_SAFE
int
getlogin_r(char *logname, int len)
{
	if ((logname == NULL) || (len == 0)) {
		seterrno(EINVAL);
		return(-1);
	}

	if (_getlogin(logname, len) < 0)
		return (-1);
	else
		return (0);
}
#else
char *
getlogin()
{
	static char logname[MAXLOGNAME];

	if (_getlogin(logname, sizeof(logname)) < 0)
		return (NULL);
	else
		return (logname);
}
#endif	/* _THREAD_SAFE */
