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
static char	*sccsid = "@(#)$RCSfile: rewinddir.c,v $ $Revision: 4.2.7.2 $ (DEC) $Date: 1993/06/07 22:27:40 $";
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
 * FUNCTIONS: rewinddir 
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
 * rewinddir.c	1.2  com/lib/c/io,3.1,8943 9/9/89 13:31:58
 */

/*
 * FUNCTION:
 *	resets the positiion of the directory stream to which 'dirp' refers
 *	to the beginning of the directory.
 *
 * RETURNS:
 *	rewinddir() does not return a value.
 *
 */

/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#pragma weak rewinddir = __rewinddir
#endif
#include <sys/param.h>
#include <dirent.h>
#include <sys/file.h>
#include <unistd.h>

#include "ts_supp.h"

#ifdef _THREAD_SAFE
#include "rec_mutex.h"
#endif	/* _THREAD_SAFE */

#ifdef rewinddir
#undef rewinddir
#endif
#ifdef _NAME_SPACE_WEAK_STRONG
#define rewinddir __rewinddir
#endif

/*
 * Since seekdir is unsafe, we need an explicit routine for rewinddir
 */
void
rewinddir(DIR *dirp)
{
	if (dirp == NULL || dirp->dd_buf == NULL)
		return;
	TS_LOCK(dirp->dd_lock);
	dirp->dd_loc = 0;
	(void) lseek(dirp->dd_fd, (off_t)0, SEEK_SET);
	TS_LOCK(dirp->dd_lock);
}
