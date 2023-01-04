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
static char	*sccsid = "@(#)$RCSfile: exec_args.c,v $ $Revision: 4.2.7.2 $ (DEC) $Date: 1993/06/07 22:47:19 $";
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
 * FUNCTIONS: _exec_args, _exec_argv 
 *
 * (C) COPYRIGHT International Business Machines Corp. 1989 
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * exec_args.c	1.2  com/lib/c/sys,3.1,8943 9/13/89 11:52:19
 */

/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#endif
#include <stdio.h>
#include <stdarg.h>
#include <string.h>			/* for memcpy		*/
#include <stdlib.h>			/* for malloc/realloc	*/
#include <errno.h>			/* for errno		*/
#include <sys/types.h>			/* for size_t		*/

char **_exec_argv();		/* argv memory management */

/*
 * NAME:	_exec_args
 *
 * FUNCTION:	_exec_args - convert variable argument list and convert o
 *		argv[] format...
 *
 * NOTES:	exec_args takes a variable length argument list (like
 *		execl() is given) and converts it to a NULL terminated
 *		array of pointers.  If the second argument 'envp' is
 *		not NULL, it is assumed the environment comes immediately
 *		after the args.
 *		
 *
 * RETURN VALUE DESCRIPTION:	-1 on memory allocation error, else 0
 */

int
_exec_args(argvp, envp, args)
char ***argvp;				/* pointer to destination argv	*/
char ***envp;				/* pointer to destination env	*/
va_list args;				/* variable list of arguments	*/
{
	int argc;			/* arguments processed so far	*/
	int alloced;			/* allocated pointers so far	*/
	char **argp;			/* use to 'walk thru' *argvp	*/

	argc = alloced = 0;

	do {
		if (argc >= alloced) {		/* used up our space?	*/
			if ((*argvp = _exec_argv(&alloced)) == NULL)
				return (-1);
			argp = *argvp + argc;
		}
		argc++;

	/* copy in the pointer */
	} while ((*argp++ = va_arg(args, char *)) != NULL);

	if (envp != NULL)		/* get enviroment...  */
		*envp = va_arg(args, char **);
	return (0);
}

/*
 * NAME:	_exec_argv
 *
 * FUNCTION:	_exec_argv - expand and return a pointer suitable for
 *		use as an argv.
 *
 * NOTES:	exec_argv expands and returns a pointer suitable for
 *		use as an array of character pointers (ala argv).  The
 *		argument 'argvct' points to a counter which indicates
 *		how much memory we've previously allocated.  The counter
 *		should be set to 0 for the first call to exec_argv.
 *		exec_argv uses the value to determine whether to use
 *		static memory, malloc some memory, or realloc some previously
 *		allocated memory.  exec_argv will update that variable to
 *		always be the total number of pointers allocated.  exec_argv
 *		should be called again when that number of pointers is
 *		exhausted.
 *
 *		Since there is a chance that exec's could fail, the normal
 *		loop of filling in argv's could start over.  In that case,
 *		the program should again set the variable 'argvct' points
 *		to to 0 when it starts the loop again.
 *
 * RETURN VALUE DESCRIPTION:	NULL if a memory allocation error
 *		occurred, or if there were too many arguments (assuming
 *		a maximum limit is set), else a pointer to the new
 *		memory.
 */

/*
 *	ARGV_SIZ is used 2 ways:
 *
 *	1)	our initial static array of pointers is this size
 *	2)	each time we expand, we expand by this size.
 *
 *	This should be as small as possible, but big enuf to handle most
 *	'normal' exec calls.  If this is too small, we spend lots of time
 *	malloc'ing and realloc'ing memory.  If this is too big, we end up
 *	wasting memory.
 */

#ifndef ARGV_SIZ
# define	ARGV_SIZ	64
#endif

char **
_exec_argv(argvct)
int *argvct;
{
	size_t newsize;			/* new size to allocate		*/
	char **newargv;			/* new pointer			*/
	static int max_alloced = 0;	/* the max we've ever allocated	*/
	static char **oldargv = NULL;	/* last memory we allocated	*/
	static char *initial[ARGV_SIZ]; /* initial array for speed	*/

	/*
	 * if our allocated count is higher than our expansion target, just
	 * use the old memory and the allocated count... (this might occur
	 * if the loop is starting over and we've already got some space
	 * allocated...)
	 */
	if (max_alloced >= *argvct + ARGV_SIZ) {
		newargv = oldargv;
		*argvct = max_alloced;
		}

	else if (*argvct == 0) {	/* first time in this loop...	*/
		/* start with our static memory */
		newargv = initial;
		*argvct = ARGV_SIZ;
		}

	else {			/* second, etc.: need to malloc/realloc	*/
		if (max_alloced == 0) {
			/*
			 * first time in malloc loop - need to start with
			 * 2 * ARGV_SIZ (to take into account our initial
			 * static array plus expansion)
			 */
			max_alloced = 2 * ARGV_SIZ;
			newsize = (size_t) (max_alloced * sizeof(*newargv));
			if ((newargv = (char **) malloc(newsize)) != NULL)
				/*
				 * copy the static array into our new array
				 */
				(void) memcpy((void *) newargv,
				(void *) oldargv,
				(size_t) ((*argvct) * sizeof(*newargv)));
			}
		else {
			/*
			 * expand by ARGV_SIZ and realloc our old mem...
			 */
			max_alloced += ARGV_SIZ;
			newsize = (size_t) (max_alloced * sizeof(*newargv));
			newargv = (char **) realloc((void *) oldargv, newsize);
			}

		/* set errno to ENOMEM if our malloc/realloc failed */
		if (newargv == NULL)
			_Seterrno(ENOMEM);

		else
			*argvct = max_alloced;
		}

	return (oldargv = newargv);
}
