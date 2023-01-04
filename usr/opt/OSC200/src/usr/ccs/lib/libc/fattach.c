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
static char *rcsid = "@(#)$RCSfile: fattach.c,v $ $Revision: 1.1.5.6 $ (DEC) $Date: 1993/08/02 20:43:17 $";
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
#define fattach __fattach
#pragma weak fattach = __fattach
#endif

#include <sys/stat.h>
#include <sys/mount.h>
#include <sys/errno.h>
#include <sys/syscall.h>
#include <stropts.h>
#include "ts_supp.h"

/*
 * NAME: fattach
 *                                                                    
 * FUNCTION: Attaches a stream to a filesystem node
 *                                                                    
 * NOTES:
 *
 * RETURNS: on error, returns -1 with error in errno
 *
 */

int fattach(filedes, path)
int filedes;
char *path;
{
	int error;
	struct ffm_args fargs;
	struct stat statb;

        if (ioctl(filedes, I_ISASTREAM, (caddr_t)0) == -1) {
		if(TS_GETERR() != EBADF)
			TS_SETERR(EINVAL);
                /*
                 * Not a stream or bad FD.
                 */
                return -1;
        }

	fargs.ffm_flags = FFM_FD; /* and implicitly !FFM_CLONE */
	fargs.ffm_filedesc = filedes;

	error = fstat(filedes, &statb);
	if (error)
		return(error);
	/* if it's a character special or pipe (FIFO) */
	if ((statb.st_mode & S_IFMT) == S_IFCHR || 
	    (statb.st_mode & S_IFMT) == S_IFIFO)
		fargs.ffm_flags |= FFM_CLONE;
	error = syscall(SYS_mount, MOUNT_FFM, path, 0, (caddr_t) &fargs);

	return(error);

}
