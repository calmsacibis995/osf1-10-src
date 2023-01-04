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
static char	*sccsid = "@(#)$RCSfile: sigpending.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 02:55:38 $";
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
/* sigpending -- library stub to transform calling sequence */

/*
 * user program calls:
 *     success = sigpending(sigset_t *mask);
 *
 * kernel-side interface:
 *     sigset_t omask = sigpending();
 *
 */

#include <sys/signal.h>
#include <sys/param.h>

extern int errno;
extern sigset_t _sigpending();

int
sigpending(mask)
sigset_t	*mask;
{
	errno = 0;
	*mask = _sigpending();
	if (errno != 0) {
		return (-1);
	}
	else
		return (0);
}
