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
static char	*sccsid = "@(#)$RCSfile: execlp.c,v $ $Revision: 4.2.5.2 $ (DEC) $Date: 1993/06/07 22:47:36 $";
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
 * FUNCTIONS: execlp 
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989 
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * execlp.c	1.2  com/lib/c/sys,3.1,8943 9/13/89 11:56:05
 */

/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#pragma weak execlp = __execlp
#endif
#include <stdio.h>
#include <stdarg.h>
#include <errno.h>

#include "ts_supp.h"

#ifdef _THREAD_SAFE
#include "rec_mutex.h"

extern struct rec_mutex	_exec_rmutex;
#endif	/* _THREAD_SAFE */

extern int execvp(), _exec_args();

/*
 * NAME:	execlp
 *
 * FUNCTION:	exec a file, using the arguments to construct an
 *		argv for the new file, and PATH to find the file
 *		to exec.
 *
 * NOTES:	execlp(file, arg0 [, arg1, ...], (char *) 0)
 *		execs a file like 'execl', except that the
 *		PATH is searched for the file to execute.
 *
 * RETURN VALUE DESCRIPTION:	-1 if a memory allocation occurs,
 *		if too many arguments were given, or if the exec
 *		fails.  Else execlp does not return.
 */

/* VARARGS1 */
int
execlp(const char *file, ...)
{
	int error;		/* error status code	*/
	char **argv;		/* argv for `program'	*/
	va_list args;		/* varargs args		*/

#ifdef _THREAD_SAFE
	if (!_rec_mutex_trylock(&_exec_rmutex)) {
		_Seterrno(EAGAIN);
		return(-1);
	}
#endif	/* _THREAD_SAFE */

	/* set up the arg list...  */
	va_start(args, file);

	/* get the rest of the args into argv...  */
	error = _exec_args(&argv, (char ***) NULL, args);

	/* clean up the arg list...  */
	va_end(args);

	if (error == 0)
		/* do the exec...  */
		error = execvp(file, argv);

	TS_UNLOCK(&_exec_rmutex);
	return (error);
}
