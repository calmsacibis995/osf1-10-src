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
static char	*sccsid = "@(#)$RCSfile: fileno.c,v $ $Revision: 4.2.5.2 $ (DEC) $Date: 1993/06/07 22:52:01 $";
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
 * FUNCTIONS: fileno 
 *
 * (C) COPYRIGHT International Business Machines Corp. 1989 
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * fileno.c	1.2  com/lib/c/io,3.1,8943 9/12/89 17:49:30
 */

/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#pragma weak fileno = __fileno
#endif
#include <stdio.h>
#include "ts_supp.h"

#ifdef _THREAD_SAFE
#include "stdio_lock.h"
#endif	/* _THREAD_SAFE */

#ifdef fileno
#undef fileno
#endif
#ifdef _NAME_SPACE_WEAK_STRONG
#define fileno __fileno
#endif

/*
 * FUNCTION:
 *	fileno() returns the integer file descriptor associated with a
 *	stream.
 * 
 * RETURNS:
 *	If an error occurs, a value of -1 is returned and errno is set to
 *	indicate the error.
 *
 */

int
fileno(FILE *stream)
{
	register int rc;
	TS_FDECLARELOCK(filelock)

	TS_FLOCK(filelock, stream);
	rc = stream->_file;
	TS_FUNLOCK(filelock);
	return (rc);
}
