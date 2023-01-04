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
static char	*sccsid = "@(#)$RCSfile: ferror.c,v $ $Revision: 4.2.5.2 $ (DEC) $Date: 1993/06/07 22:49:45 $";
#endif 
/*
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 */
/*
 * OSF/1 1.2
 */
/*
 */ 
#if !defined(lint) && !defined(_NOIDENT)

#endif
/*
 * FUNCTIONS: ferror 
 *
 * (C) COPYRIGHT International Business Machines Corp. 1989 
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * ferror.c	1.8  com/lib/c/io,3.1,8943 9/13/89 09:50:37
 */

/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#endif
#include <stdio.h>

#include "ts_supp.h"

#ifdef	_THREAD_SAFE
#include "stdio_lock.h"
#endif	/* _THREAD_SAFE */
#undef ferror

/*                                                                    
 * FUNCTION:	A subroutine version of the macro ferror.  This function was
 *	        created to meet ANSI C standards.  The ferror function tests
 *		for the error indicator for the stream pointed to by stream.
 *
 * RETURN VALUE DESCRIPTION:	
 *		The ferror function returns non-zero if and only if the error
 *		indicator is set for stream.
 *
 */  

int 	
ferror(FILE *stream)
{
	register int err;
	TS_FDECLARELOCK(filelock)

	TS_FLOCK(filelock, stream);
	err = (stream)->_flag & _IOERR;
	TS_FUNLOCK(filelock);

	return err;
}
