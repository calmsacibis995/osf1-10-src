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
/*
 * (c) Copyright 1990, 1991, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0.1
 */

#if !defined(lint) && !defined(_NOIDENT)
static char rcsid[] = "@(#)$RCSfile: mktemp.c,v $ $Revision: 4.2.2.3 $ (DEC) $Date: 1993/07/15 15:02:07 $";
#endif

/*
 * FUNCTIONS: mktemp
 *
 * DESCRIPTION:
 *	SVID2 compliant mktemp()
 */

#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <sys/habitat.h>

#ifdef	_THREAD_SAFE
#include <lib_lock.h>

extern lib_lock_functions_t	libc_lock_funcs;
#endif	/* _THREAD_SAFE */

#define	TRAILER	6	/* number of trailing chars to be replaced */

/*---------------------------------------------------------------------------*
 * Synopsis	: generate a unique filename
 * Parameters	:
 *		inout	template for filename
 * Return	: template or NULL if name cannot be generated
 * Fatal Errors	: none
 * Description	:
 *		Replace the trailing 6 'X's in the template string with
 *		a unique filename using a letter and the pid.
 * Notes	:
 *		SVID2 requires this (restricted) particular implementation.
 *		Making mktemp() `thread safe' would replace getpid() which we
 *		must have for SVID2, so caller must beware.
 *		Assumes the pid is less than 6 digits.
 *---------------------------------------------------------------------------*/

#pragma weak mktemp = __mktemp

char *
__mktemp(char *template)		
{
	int	offset;		/* offset to start of trailer */
	char	*s = template;
	char	*s0;
	pid_t	pid;
	int	i;

	/* check template size */
	if ((offset = strlen(template) - TRAILER) < 0) {
		*template = NULL;
		return (template);
	}
	s += offset;

	/* check trailer is all 'X's */
	for (s0 = s; *s0 == 'X'; s0++);
	if (*s0) {	/* found a non 'X' before end */
		*template = NULL;
		return (template);
	}

#ifdef	_THREAD_SAFE
	/*
	 * pid is not unique, use thread id from the threads provider
	 * we cannot use the kernel thread id as this may be a coroutine
	 * threads package or we may be have many threads multiplexed on
	 * fewer kernel threads.
	   -DAL001 also hash thread id to fit within 15 bits. */
	{ unsigned long x, y;
		x = (unsigned long)lib_thread_id(libc_lock_funcs);
		y = (x&0xffff) + ((x>>16)&0xffff);
#ifdef	LONG_BIT>32
		y += ((x>>48)&0xffff) + ((x>>32)&0xffff);
#endif
	pid = (((y & 0x7fff)<<15) + getpid()) & 0x3fffffff;
	}
#else	/* _THREAD_SAFE */
	pid = getpid();
#endif	/* _THREAD_SAFE */
	/* overwrite last 'X's with pid as a 5 digit number */
	for (i = 0, s0--; i < TRAILER; i++, s0--) {	/* for speed */
		*s0 = pid % 10 + '0';
		pid /= 10;
	}

	/* loop trying names with different letters */
	for (*s = 'a'; *s <= 'z'; ++*s) {
		if (access(template, F_OK) == -1)
			return (template);
	}

	/* all names used */
	*template = NULL;
	return (template);
}
