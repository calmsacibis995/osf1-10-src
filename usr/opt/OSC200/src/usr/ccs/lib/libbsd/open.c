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
static char *rcsid = "@(#)$RCSfile: open.c,v $ $Revision: 4.2.3.2 $ (DEC) $Date: 1992/01/28 19:58:46 $";
#endif
/*
 * (c) Copyright 1990, 1991, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0.2
 */
/* 10/03/91 - Tom Peterson
 *       This function was added to the libbsd.a library so that the
 *       window(1) command would operate properly.
 */

#include <syscall.h>

#define SYS_o_open	5	/* The OLD Berkeley open */

extern int syscall(int, ...);

/*
 * This routine exists to allow Berkeley programs (which assumed that
 * opening /dev/tty would give you a controlling terminal) to work.
 *
 * n.b. that the correct way to work in OSF/1 would be to do the following:
 *
 *	setsid();
 *	fd = open("/dev/tty", O_RDWR );
 *	ioctl(fd, TIOCSCTTY, 0);
 */

int open( const char *path, int flags, int mode)
{
  return syscall(SYS_o_open, path, flags, mode);
}
