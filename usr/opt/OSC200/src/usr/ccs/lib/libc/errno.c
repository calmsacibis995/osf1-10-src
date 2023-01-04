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
static char *rcsid = "@(#)$RCSfile: errno.c,v $ $Revision: 1.1.6.2 $ (DEC) $Date: 1993/06/07 22:46:53 $";
#endif
/*
 * Stub routines for non-threadsafe libc adapted from OSF/1 1.2.
 */
/*
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 1.2
 */
/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#if !defined(_THREAD_SAFE)
#pragma weak geterrno = __geterrno
#pragma weak seterrno = __seterrno
#endif
#endif
#include <errno.h>

#ifndef	_THREAD_SAFE
/*
 * Function:
 *	geterrno
 *
 * Return value:
 *	value of the global errno
 *
 * Description:
 *	For libraries only. Retrieve the value of errno from the per-thread
 *	or glabal variable.
 */
int
geterrno()
{
	return errno;
}

/*
 * Function:
 *	seterrno
 *
 * Parameters:
 *	error	- value to set errno to
 *
 * Return value:
 *	new value of errno
 *
 * Description:
 *	For libraries only. Set the global errno.
 */
int
seterrno(int error)
{
	errno = error;
	return error;
}
#endif	/* _THREAD_SAFE */
