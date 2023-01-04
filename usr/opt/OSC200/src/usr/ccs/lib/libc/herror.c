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
static char	*sccsid = "@(#)$RCSfile: herror.c,v $ $Revision: 4.2.5.4 $ (DEC) $Date: 1993/09/23 18:29:37 $";
#endif 
/*
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 */
/*
 * OSF/1 1.2
 */
/*
 * Copyright International Business Machines Corp. 1989
 * Unpublished Work
 * All Rights Reserved
 * Licensed Material - Property of IBM
 *
 * herror.c	1.2  com/lib/c/net,3.1,8943 7/21/89 08:05:42
 */
/*
 * Copyright (c) 1987 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */
/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#pragma weak herror = __herror
#endif
#include <errno.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <nl_types.h>
#include "libc_msg.h"
#ifdef	_THREAD_SAFE
#include <lib_data.h>
#endif	/* _THREAD_SAFE */
#include <netdb.h>
static nl_catd catd;

static char	*h_errlist[] = {
	"Error 0",
	"Unknown host",				/* 1 HOST_NOT_FOUND */
	"Host name lookup failure",		/* 2 TRY_AGAIN */
	"Unknown server error",			/* 3 NO_RECOVERY */
	"No address associated with name",	/* 4 NO_ADDRESS */
};
static const int	h_nerr = { sizeof(h_errlist)/sizeof(h_errlist[0]) };

/*
 * herror --
 *	print the error indicated by the h_errno value.
 */
int
herror(s)
	char *s;
{
	struct iovec iov[4];
	register struct iovec *v = iov;
	int _Get_h_errno();

	catd = catopen(MF_LIBC,NL_CAT_LOCALE);
	if (s && *s) {
		v->iov_base = s;
		v->iov_len = strlen(s);
		v++;
		v->iov_base = ": ";
		v->iov_len = 2;
		v++;
	}

	switch (_Get_h_errno()) {
		case 0:
			v->iov_base = catgets(catd,LIBCNET,NET13, h_errlist[0]);
			break;
		case 1:
			v->iov_base = catgets(catd,LIBCNET,NET14, h_errlist[1]);
			break;
		case 2:
			v->iov_base = catgets(catd,LIBCNET,NET15, h_errlist[2]);
			break;
		case 3:
			v->iov_base = catgets(catd,LIBCNET,NET16, h_errlist[3]);
			break;
		case 4:
			v->iov_base = catgets(catd,LIBCNET,NET17, h_errlist[4]);
			break;
		default:
			v->iov_base =
			catgets(catd,LIBCNET,NET12, "Unknown error");
			break;
	}
	v->iov_base = _Get_h_errno() < h_nerr ? h_errlist[_Get_h_errno()] : 
			"Unknown error";
	v->iov_len = strlen(v->iov_base);
	v++;
	v->iov_base = "\n";
	v->iov_len = 1;
	writev(fileno(stderr), iov, (v - iov) + 1);
	catclose(catd);
	return (0);
}

/*
 * Following are the functions for setting/getting h_errno.
 */

#ifdef  h_errno
#undef  h_errno
#endif  /* h_errno */

int	h_errno;

#ifdef	_THREAD_SAFE

#define Get_Error_Ref	_errno()

/*
 * Function:
 *	_h_errno
 *
 * Return value:
 *	address of the per-thread h_errno if available or else
 *	address of the global h_errno
 *
 * Description:
 *	Allow access to a per-thread h_errno. Returning the address enables
 *	existing l/r-value rules to set/get the correct value. Default to
 *	the global h_errno in case per-thread data is not available.
 */
int *
_h_errno()
{
	int	*err;

	return ((err = Get_Error_Ref) ? err : &h_errno);
}
#endif  /* _THREAD_SAFE */

/*
 * Function:
 *	_Get_h_errno
 *
 * Return value:
 *	value of the per-thread h_errno if available or else
 *	value of the global h_errno
 *
 * Description:
 *	For libraries only. Retrieve the value of h_errno from the per-thread
 *	or glabal variable.
 */
int
_Get_h_errno()
{
#ifdef  _THREAD_SAFE
	int	*err;

	return ((err = Get_Error_Ref) ? *err : h_errno);
#else
	return (h_errno);
#endif  /* _THREAD_SAFE */
}

/*
 * Function:
 *	_Set_h_errno
 *
 * Parameters:
 *	h_error	- value to set h_errno to
 *
 * Return value:
 *	new value of h_errno
 *
 * Description:
 *	For libraries only. Set both the per-thread and global h_errnos.
 *	_THREAD_SAFE case helps code which still uses the global h_errno.
 */
int
_Set_h_errno(int error)
{
#ifdef  _THREAD_SAFE
	int	*err;

	if ((err = Get_Error_Ref))
		*err = error;
#endif  /* _THREAD_SAFE */
	h_errno = error;
	return (error);
}
