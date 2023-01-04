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
static char	*sccsid = "@(#)$RCSfile: putchar.c,v $ $Revision: 4.2.5.2 $ (DEC) $Date: 1993/06/07 23:36:49 $";
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
 * FUNCTIONS: putchar 
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989 
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * putchar.c	1.10  com/lib/c/io,3.1,8943 9/9/89 13:30:14
 */

/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#if defined(_THREAD_SAFE)
#pragma weak putchar_unlocked = __putchar_unlocked
#endif
#endif
#include <stdio.h>
#ifdef	_THREAD_SAFE
#include "stdio_lock.h"
#undef putchar_unlocked
#ifdef _NAME_SPACE_WEAK_STRONG
#define putchar_unlocked __putchar_unlocked
#endif
#endif
#undef putchar

/*
 * FUNCTION:	A subroutine version of the macro putchar.  This function is
 *		created to meet ANSI C standards.  The putchar function writes
 *		the character specified by c (converted to an unsigned char)
 *		to stdout, at the position indicated by the assoicated file
 *		poistion indicator for the stream, and advances the indicator
 *		appropriately.
 *
 * RETURN VALUE DESCRIPTION:	
 *		The putchar function returns the character written.  If a write
 *		error occurs, the error indicator for the stream is set and
 * 		putchar returns EOF.
 *
 */                                                                   

int 	
putchar(int c)
{
#ifdef	_THREAD_SAFE
	return(putc_locked(c, stdout));
#else
	return(putc(c, stdout));
#endif
}

#ifdef	_THREAD_SAFE
int 	
putchar_unlocked(int c)
{
	return(putc_unlocked(c, stdout));
}
#endif
