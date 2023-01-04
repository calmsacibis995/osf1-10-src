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
static char *rcsid = "@(#)$RCSfile: truncate.c,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/11/22 22:03:35 $";
#endif
#include <sys/types.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>

/*
 * Truncate a file given its path name.
 *
 * The truncate() system call is already available.  However,
 * it doesn't open() the file being truncated.  When using
 * /dev/fd, this means that the /dev/fd/? file is truncated,
 * when actually the aliased file should be truncated.
 */

#pragma weak truncate = __truncate

int
__truncate(char *fname, off_t length)
{
	int fd, error, saved_errno;

	if ((fd = open(fname, O_RDWR)) == -1){
		/* errno is already set.  */
		return(fd);
	}

	if (error = ftruncate(fd, length)){
		saved_errno = errno;
	}

	close(fd);	/* N.B. This also sets errno. */

	if (error) {
		/*
     		* On multi-threaded systems, errno may actually
     		* be a procedure.  In that case, this won't work
     		* and another method for setting errno must be
     		* devised.
     		*/
		errno = saved_errno;
	}
	return (error);
}
