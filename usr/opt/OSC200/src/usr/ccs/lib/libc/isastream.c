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
static char *rcsid = "@(#)$RCSfile: isastream.c,v $ $Revision: 1.1.5.3 $ (DEC) $Date: 1993/06/07 18:04:53 $";
#endif
/*
 * (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.2
 */

/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#define isastream __isastream
#pragma weak isastream = __isastream
#endif

/** Copyright (c) 1989-1991  Mentat Inc. **/

#include <sys/errno.h>
#include <stropts.h>
#include "ts_supp.h"

int
isastream (fd)
	int	fd;
{
	int	sav_errno;

	sav_errno = TS_GETERR();
	if ((fd = ioctl(fd, I_ISASTREAM, (caddr_t)0)) == -1) {
		if (TS_GETERR() == EBADF)
			return fd;
		/*
		 * Not a Stream, but valid fd: no error.
		 */
		TS_SETERR(sav_errno);
		return 0;
	}
	/*
	 * OSF1.1 returns I_ISASTREAM, I_PIPE, or I_FIFO in response,
	 * but to preserve compatibility with future changes, we look
	 * for any IOC_VOID ioctl in the Streams group.
	 */
	return	(IOCGROUP(fd) == 'S' &&
		(fd & IOC_DIRMASK) == IOC_VOID &&
		IOCPARM_LEN(fd) == 0);
}
