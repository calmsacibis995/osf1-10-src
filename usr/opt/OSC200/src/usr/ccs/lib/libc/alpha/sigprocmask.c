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
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0
 */
/* sigprocmask -- library stub to transform calling sequence */

/*
 * user program calls:
 *     success = sigprocmask(int how, sigset_t *imask, sigset_t *omask);
 *
 * kernel-side interface:
 *     sigset_t omask = sigprocmask(int how, sigset_t imask);
 *
 */

/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#pragma weak sigprocmask = __sigprocmask
#endif
#include <sys/signal.h>
#include <sys/param.h>
#include <sys/errno.h>

extern sigset_t _sigprocmask();

int
sigprocmask(how, imask, omask)
int 		how;
sigset_t	*imask, *omask;
{
	sigset_t new, old;

	if (imask == NULL) {
		how = SIG_BLOCK;
		new = 0;
	}

	else {
		new = *imask;
	}

	old = _sigprocmask(how, new);

	if (old == (sigset_t)-1) {
		return (-1);
	}
	else {
		if (omask != NULL)
			*omask = old;
		return (0);
	}
}
