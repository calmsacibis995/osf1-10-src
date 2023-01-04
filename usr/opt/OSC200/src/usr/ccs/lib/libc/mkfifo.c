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
static char	*sccsid = "@(#)$RCSfile: mkfifo.c,v $ $Revision: 4.2.7.2 $ (DEC) $Date: 1993/06/07 23:29:28 $";
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
#if !defined(lint) && !defined(_NOIDENT)

#endif
/*
 * COMPONENT_NAME: (LIBCGEN) Standard C Library General Functions
 *
 * FUNCTIONS: mkfifo
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 * OBJECT CODE ONLY SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 * 
 * mkfifo.c	1.9  com/lib/c/gen,3.1,8943 10/13/89 15:08:26
 */

/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#pragma weak mkfifo = __mkfifo
#endif
#include <sys/limits.h> 
#include <sys/types.h> 
#include <sys/stat.h> 
#include <sys/errno.h> 
#include "ts_supp.h"

/*
 *
 * SYNOPSIS: mkfifo() is given a pathname and a set of permissions for
 *	a pipe to be created. We then test against system limits and
 *	pass the request on to mknod(), a system call which will create
 *	the pipe, set the mode, and set up the file inode and time
 *	stats. We pick up the return code and errno (if any), then
 *	act accordingly.
 */

/* The mkfifo() function includes all the POSIX requirements */


int 
mkfifo(char *path, mode_t mode) 
{ 

  if((strlen(path)) > PATH_MAX)
  {
    TS_SETERR(ENAMETOOLONG);
    return(-1);
  }

/**********
  mknod() will do all the dirty work.
**********/
  mode |= S_IFIFO;	/* OR mode with the FIFO creation code */
  if((mknod(path, mode)) != 0)
    return(-1);

  return(0); 
}
