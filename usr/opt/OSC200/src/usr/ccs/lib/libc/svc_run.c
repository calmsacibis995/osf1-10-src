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
static char     *sccsid = "@(#)$RCSfile: svc_run.c,v $ $Revision: 4.2.5.2 $ (DEC) $Date: 1993/06/07 23:11:01 $";
#endif
/*
 */


/* 
 * Copyright (c) 1988 by Sun Microsystems, Inc.
 *  1.2 88/02/08 
 */


/*
 * This is the rpc server side idle loop
 * Wait for input, call server program.
 */
/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#if !defined(_THREAD_SAFE)
#pragma weak svc_run = __svc_run
#endif
#endif
#include <rpc/rpc.h>
#include <sys/errno.h>
#include <sys/syslog.h>

void
svc_run()
{
	fd_set readfds;
	int dtbsize = _rpc_dtablesize();
	extern int errno;

	for (;;) {
		readfds = svc_fdset;
		switch (select(dtbsize, &readfds, (fd_set *)0,
			(fd_set *)0, (struct timeval *)0)) {
		case -1:
			/*
			 * We ignore all other errors except EBADF.  For all
			 * other errors, we just continue with the assumption
			 * that it was set by the signal handlers (or any
			 * other outside event) and not caused by select().
			 */
			if (errno != EBADF) {
				continue;
			}
			(void) syslog(LOG_ERR, "svc_run: - select failed: %m");
			return;
		case 0:
			continue;
		default:
			svc_getreqset(&readfds);
		}
	}
}
