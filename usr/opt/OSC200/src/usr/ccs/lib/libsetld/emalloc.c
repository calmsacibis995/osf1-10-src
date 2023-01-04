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
static  char *sccsid = "@(#)$RCSfile: emalloc.c,v $ $Revision: 4.2.3.2 $ (DEC) $Date: 1992/10/13 13:43:37 $";
#endif
/*
 */
#if !defined( lint) && !defined(_NOIDENT)

#endif
/*	unsigned emalloc() -
 *		allocate memory, die on error.
 *
 *	given:	number of bytes to allocate.
 *	does:	calls malloc checking malloc return value,
 *		printing error diagnostic on malloc failure and exiting.
 *	return:	pointer to allocated memory if successful.
 *
 *	mods:
 *	000	ccb	1/1/86
 *	001	29-apr-1989	ccb
 *		add declaration of exit() to silence lint.
*/

#include	<stdio.h>

/*LINTLIBRARY*/

extern void	exit();		/* exit(3) */
extern char	*malloc();	/* malloc(2) */

extern char	*prog;

char *emalloc(n)
unsigned n;
{
	char *p;

	if( (p = malloc(n)) == NULL )
	{
		(void) fprintf( stderr, "%s: out of memory.\n", prog );
		exit(1);
	}
	return(p);
}

