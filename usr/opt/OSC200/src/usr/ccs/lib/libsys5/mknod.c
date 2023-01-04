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
/*
 * (c) Copyright 1990, 1991, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0.1
 *
 */
#if !defined(lint) && !defined(_NOIDENT)
static char rcsid[] = "@(#)$RCSfile: mknod.c,v $ $Revision: 4.2.4.3 $ (DEC) $Date: 1993/07/15 15:02:04 $";
#endif

/*
 * FUNCTIONS: mknod
 *
 * DESCRIPTION:
 *	SVID.3 compatible mknod.
 *	Currently picks up S_IFREG and S_IFDIR
 *	and passes off the rest to the mknod system call.
 */
#include <sys/syscall.h>
#include <sys/mode.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/habitat.h>

#pragma weak mknod = __mknod

/*ARGSUSED*/
int
__mknod(char *path, int mode, int dev)
{
	/* need to be root unless creating FIFO */
	if (!S_ISFIFO(mode) && geteuid()) {
		_Seterrno(EPERM);
		return (-1);
	}
	switch (mode & S_IFMT) {
	case S_IFDIR:			/* Directory file	*/
		return (mkdir(path,(mode & 07777)));

	case 0:				/* FALLTHROUGH */
	case S_IFREG:			/* Regular file		*/
		if ((dev = open(path, O_WRONLY|O_CREAT|O_EXCL,
				(mode & 07777))) == -1)
			return (-1);
		close(dev);
		return (0);

	default:			/* Everything Else	*/
		return (syscall(SYS_mknod, path, mode, dev));
	}
}
