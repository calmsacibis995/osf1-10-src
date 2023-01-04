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
#if !defined(lint) && !defined(_NOIDENT)
static char rcsid[] = "@(#)$RCSfile: tblock.c,v $ $Revision: 4.2.4.2 $ (DEC) $Date: 1993/03/16 23:20:01 $";
#endif

#include <fcntl.h>
#include <stropts.h>

int
t_blocking(
	int		fd)
{
	register int	status;

	/*
	 * Get descriptor's current flag settings
	 */
	if ((status = fcntl( fd, F_GETFL, 0 )) == -1)
		return status;

	/*
	 * Now ensure that the NO DELAY bit is cleared, ie. make
	 * descriptor do blocking I/O
	 */
	return fcntl( fd, F_SETFL, (status & ~O_NDELAY) );
}

/** Copyright (c) 1990  Mentat Inc.
 ** tcblock.c 2.1, last change 11/14/90
 **/

int
t_is_nonblocking (
	int		fd)
{
	int		flags;
	
	flags = fcntl(fd, F_GETFL, 0);
	if ( flags == -1 )
		return -1;
	return ((flags & O_NDELAY) || (flags & O_NONBLOCK)) ? 1 : 0;
}
