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
static  char *sccsid = "@(#)$RCSfile: String.c,v $ $Revision: 4.2.3.2 $ (DEC) $Date: 1992/10/13 13:42:22 $";
#endif
/*
 */
#if !defined( lint) && !defined(_NOIDENT)

#endif
/*	String.c -
 *		routines for manipulating StringT
 *
 *	mods:
*/

#include	<sys/param.h>
#include	<sys/types.h>
#include	<sys/dir.h>
#include	<stdio.h>
#include	<string.h>
#include	<setld/setld.h>

/*LINTLIBRARY*/


char *StringSet( s, p )
StringT s;
char *p;
{
	(void) strncpy( s, p, STRINGLEN );
	s[STRINGLEN] = '\0';
	return( s );
}


char *StringUnquote( s )
register StringT s;
{
	register char	*t;
	char		*u;

	for( u = t = s; *s != '\0'; s++ )
	{
		if( *s == '"' )
			continue;
		*t++ = *s;
	}
	*t = '\0';
	return( u );
}

char *StringToken( s, t )
char *s, *t;
{
	return( strtok( s, t ) );
}


