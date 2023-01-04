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
static char	*sccsid = "@(#)$RCSfile: closedir.c,v $ $Revision: 4.2.8.2 $ (DEC) $Date: 1993/06/07 22:41:45 $";
#endif 
/*
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 */
/*
 * OSF/1 1.2
 */

#if !defined(lint) && !defined(_NOIDENT)
#endif

/*
 */ 
/*
 * COMPONENT_NAME: (LIBCIO) Standard C Library I/O Functions 
 *
 * FUNCTIONS: closedir 
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1983 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 *  "closedir.c        1.12  com/lib/c/io,3.1,9021 4/24/90 15:55:09";
 */

/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#pragma weak closedir = __closedir
#endif
#include <sys/limits.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <dirent.h>
#include <errno.h>

#include "ts_supp.h"

#ifdef _THREAD_SAFE
#include "rec_mutex.h"
#endif	/* _THREAD_SAFE */


/*
 * close a directory.
 */
int
closedir(DIR *dirp)
{
#ifdef _THREAD_SAFE
	rec_mutex_t     lock;
#endif	/* _THREAD_SAFE */

	if ( !dirp ) {		/* NULL pointer? */
	    _Seterrno(EBADF);
	    return(-1);
	}

        TS_LOCK(dirp->dd_lock);
        if ((dirp->dd_fd >= 0) && (close(dirp->dd_fd) == 0) ) {
		dirp->dd_fd = -1;
		dirp->dd_loc = 0;
		dirp->dd_size = 0; /* so readdir will fail */
		free(dirp->dd_buf);
#ifdef _THREAD_SAFE
                lock = (rec_mutex_t)dirp->dd_lock;
		dirp->dd_lock = (rec_mutex_t) 0;
#endif	/* _THREAD_SAFE */
		free(dirp);
		TS_UNLOCK(lock);
#ifdef _THREAD_SAFE
		_rec_mutex_free(lock);
#endif	/* _THREAD_SAFE */
		return(0);
	}
	else {	/* dirp does not refer to an open directory stream */
                TS_UNLOCK(dirp->dd_lock);
		_Seterrno(EBADF);
		return(-1);
	}
}
