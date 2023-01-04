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
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 */
/*
 * OSF/1 1.2
 */
/** Copyright (c) 1989-1991  Mentat Inc. **/

#include <fcntl.h>
#include <sys/errno.h>
#include <stropts.h>

/*
 * Streams pipes are a special device which has no minor number.
 *
 * These pipes are bidirectional, but fds[0] is canonically the
 * "write side." In OSF1.1 an fstat() on either side will always
 * return a stat associated with this direction. FIONREAD, etc.
 * on the other hand will return the value associated with the
 * proper side.
 */
#define PIPEDEV	"/dev/streams/pipe"
#define PIPEMOD	"pipemod"

#pragma weak pipe = __pipe
int
__pipe (fds)
	int	* fds;
{
	int	sav_errno;

	if ((fds[0] = open(PIPEDEV, O_RDWR)) == -1)
		return -1;
	if ((fds[1] = open(PIPEDEV, O_RDWR)) == -1) {
		sav_errno = _Geterrno();
		(void) close(fds[0]);
		_Seterrno(sav_errno);
		return -1;
	}
	if (ioctl(fds[0], I_PIPE, (caddr_t)fds[1]) != 0) {
		sav_errno = _Geterrno();
		(void) close(fds[0]);
		(void) close(fds[1]);
		_Seterrno(sav_errno);
		return -1;
	}
	(void) ioctl(fds[0], I_PUSH, PIPEMOD);
	return 0;
}
