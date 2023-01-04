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
static char	*sccsid = "@(#)$RCSfile: ttyslot.c,v $ $Revision: 4.2.5.2 $ (DEC) $Date: 1993/06/07 23:41:35 $";
#endif 
/*
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 */
/*
 * OSF/1 1.2
 */
/*
 * Copyright (c) 1984 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

#if !defined(lint) && !defined(_NOIDENT)
#endif

/* @(#)ttyslot.c	5.2 (Berkeley) 3/9/86 */

/*
 * Return the number of the slot in the utmp file
 * corresponding to the current user: try for file 0, 1, 2.
 * 
 */
/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#pragma weak ttyslot = __ttyslot
#endif
#include <utmp.h>
#include <paths.h>
#include <strings.h>

#ifdef _THREAD_SAFE
int	ttyname_r();
#else
char	*ttyname();
#endif	/* _THREAD_SAFE */

char	*rindex();

#define	NULL	0

#ifdef _THREAD_SAFE
#define	TTYNAME(f, b)	(ttyname_r(f, b, sizeof(buf)) == 0) 
#define	SETUTENT()	setutent_r(&utmp_data)
#define	GETUTENT(u)	(getutent_r(&u, &utmp_data) == 0)
#define	ENDUTENT()	endutent_r(&utmp_data)
#else
#define	TTYNAME(f, b)	((b = ttyname(f)) != NULL)
#define	SETUTENT()	setutent()
#define	GETUTENT(u)	(u = getutent())
#define	ENDUTENT()	endutent()
#endif	/* _THREAD_SAFE */

#define DEV_LEN (sizeof(_PATH_DEV)-1)

int
ttyslot(void)
{
#ifdef _THREAD_SAFE
	struct utmp_data	utmp_data = {-1};
	char			buf[100];
	register char		*tp = buf;
#else
	register char		*tp;
#endif	/* _THREAD_SAFE */
	register char		*p;
	register int		s;
	struct utmp		*ubuf;

	if (!TTYNAME(0, tp) && !TTYNAME(1, tp) && !TTYNAME(2, tp))
		return (0);

	/* set p to the tty under /dev (eg. pts/N) */
/* Allow display names as device names - DAL001.
	p = (strncmp(tp, _PATH_DEV, DEV_LEN)? tp: &tp[DEV_LEN]);
*/	if ((p = rindex(tp, '/')) == NULL)
		p = tp;
	else
		p++;

	SETUTENT();
	s = 0;
	while (GETUTENT(ubuf)) {
		if ((ubuf->ut_type == INIT_PROCESS ||
		     ubuf->ut_type == LOGIN_PROCESS ||
		     ubuf->ut_type == USER_PROCESS ||
		     ubuf->ut_type == DEAD_PROCESS ) &&
		     strncmp(p, ubuf->ut_line, sizeof(ubuf->ut_line)) == 0) {
			ENDUTENT();
			return(s);
		}
		s++;
	}
	ENDUTENT();
	return (0);
}
