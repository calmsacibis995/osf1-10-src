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
static char	*sccsid = "@(#)$RCSfile: lockf.c,v $ $Revision: 4.2.8.2 $ (DEC) $Date: 1993/06/07 23:25:58 $";
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
#if !defined(lint) && !defined(_NOIDENT)

#endif

/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#pragma weak lockf = __lockf
#endif
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include "ts_supp.h"

lockf(fildes, function, size)
long size;
int fildes, function;
{
	struct flock l;
	int rv;

	l.l_whence = 1;
	if (size < 0) {
		l.l_start = size;
		l.l_len = -size;
	} else {
		l.l_start = 0L;
		l.l_len = size;
	}
	switch (function) {
	case F_ULOCK:
		l.l_type = F_UNLCK;
		rv = fcntl(fildes, F_SETLK, &l);
		break;
	case F_LOCK:
		l.l_type = F_WRLCK;
		rv = fcntl(fildes, F_SETLKW, &l);
		break;
	case F_TLOCK:
		l.l_type = F_WRLCK;
		rv = fcntl(fildes, F_SETLK, &l);
		break;
	case F_TEST:
		l.l_type = F_WRLCK;
		rv = fcntl(fildes, F_GETLK, &l);
		if (rv != -1) {
			if (l.l_type == F_UNLCK)
				return (0);
			else {
				TS_SETERR(EACCES);
				return (-1);
			}
		}
		break;
	default:
		TS_SETERR(EINVAL);
		return (-1);
	}
	if (rv < 0) {
		switch(TS_GETERR()) {
		case EMFILE:
		case ENOSPC:
			/* A deadlock error is given if we run out of resources,
			 * in compliance with /usr/group standards.
			 */
			TS_SETERR(EDEADLK);
			break;
		default:
			break;
		}
	}
	return (rv);
}
