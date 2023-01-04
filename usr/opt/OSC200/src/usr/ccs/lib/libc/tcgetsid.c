
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
static char *rcsid = "@(#)$RCSfile: tcgetsid.c,v $ $Revision: 1.1.3.2 $ (DEC) $Date: 1993/08/23 20:37:42 $";
#endif

/*
 * FUNCTION: tcgetsid
 *
 * DESCRIPTION:
 *	Get the session ID associated with the terminal whose file 
 *	descriptor is specified.  Required for SVID 3 compliance.
 */

#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#pragma weak tcgetsid = __tcgetsid
#endif
#include <stdio.h>
#include <sys/errno.h>
#include <sys/ioctl.h>
#include <sys/termios.h>
#include <sys/tty.h>
#include <sys/types.h>

pid_t tcgetsid(int fd)
{
    pid_t sid;

    return (isatty(fd) && !ioctl(fd, TIOCGSID, &sid)) ? sid : (pid_t)-1;
}
