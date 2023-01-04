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
static char	*sccsid = "@(#)$RCSfile: mktemp.c,v $ $Revision: 4.2.6.3 $ (DEC) $Date: 1993/09/20 21:54:40 $";
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
 * COMPONENT_NAME: (LIBCGEN) Standard C Library General Functions
 *
 * FUNCTIONS: mktemp
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989, 1991 
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1984 AT&T	
 * All Rights Reserved  
 *
 * THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	
 * The copyright notice above does not evidence any   
 * actual or intended publication of such source code.
 *
 * 1.3  com/lib/c/env/mktemp.c, libcenv, bos320, bosarea.9125 5/1/91 18:32:13
 */

/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#pragma weak mktemp = __mktemp
#endif
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/file.h>
#ifdef	_THREAD_SAFE
#include <lib_lock.h>

extern lib_lock_functions_t	libc_lock_funcs;
#endif	/* _THREAD_SAFE */

/* Underscore is used twice in LETTERS to make 64.  Other special characters,
   like ',', could cause problems for parsers of file-names.
*/
#define	LETTERS	\
	"abcdefghijklmnopqrstuvwxyzABCDEF"
/* Old string.
	"abcdefghijklmnopqrstuvwxyz1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZ__"
*/
/*---------------------------------------------------------------------------*
 * Synopsis	: generate a unique filename
 * Parameters	:
 *		inout	template for filename
 * Return	: template or NULL if name cannot be generated
 * Fatal Errors	: none
 * Description	:
 *		Replace the trailing 'X's in the template string with
 *		a unique filename using a letter and an id.
 *		If all permutations are used return NULL and set the first
 *		element of the template to NULL.
 * Notes	:
 *---------------------------------------------------------------------------*/
char *
mktemp(char *template)
{
	char	*last = template;
	char	*s0;
	int	id;

	/*
	 * check for NULL
	 */
	if (template == NULL || template[0] == '\0')
		return (NULL);

	/*
	 * check for trailing 'X'
	 */
	if (*(last = template + strlen(template) - 1) != 'X') {
		*template = '\0';
		return (NULL);
	}

#ifdef	_THREAD_SAFE
/*	id = (pid_t)lib_thread_id(_libc_lock_funcs); */
	/*
	 * pid is not unique, use thread id from the threads provider
	 * we cannot use the kernel thread id as this may be a coroutine
	 * threads package or we may be have many threads multiplexed on
	 * fewer kernel threads.
	-DAL001 also hash thread id to fit within 15 bits. */
	{ unsigned long x, y;
		x = (unsigned long)lib_thread_id(libc_lock_funcs);
		y = (x&0xffff) + ((x>>16)&0xffff);
#if	LONG_BIT>32
		y += ((x>>48)&0xffff) + ((x>>32)&0xffff);
#endif
	id = (((y & 0x7fff)<<15) + getpid()) & 0x3fffffff;
	}
#else	/* _THREAD_SAFE */
	id = getpid();
#endif	/* _THREAD_SAFE */

	for (s0 = last - 1; s0 >= template && *s0 == 'X'; s0--) {
		*s0 = LETTERS[id & 0x1f];
		id >>= 5;
	}

	/*
	 * loop trying names with different letters
	 */
	for (s0 = LETTERS; *last = *s0; s0++)
		if (access(template, F_OK) == -1)
			return (template);

	/* all names used */
	*template = '\0';
	return (NULL);
}
