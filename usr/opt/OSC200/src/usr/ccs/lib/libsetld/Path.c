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
static  char *sccsid = "@(#)$RCSfile: Path.c,v $ $Revision: 4.2.3.2 $ (DEC) $Date: 1992/10/13 13:39:46 $";
#endif
/*
 */
#if !defined( lint) && !defined(_NOIDENT)

#endif
/*	Path.c -
 *		routine for manipulating PathT
 *
 *	mods:
 *	000	05-jun-1989	ccb
 *		New
*/

#include	<sys/param.h>
#include	<sys/types.h>
#include	<sys/dir.h>
#include	<string.h>
#include	<stdio.h>
#include	<setld/setld.h>

/*LINTLIBRARY*/

/*	PathMatch() -
 *		determine if a path matches a pattern
 *
 *	given:	PathT p - a path
 *		char *s - a pattern (uses shell *, ?, [...])
 *	does:	match the pattern to the path
 *	return:	0 if the pattern does not match, 1 if it does.
*/

PathMatch( p, s )
PathT p;
char *s;
{
	int	found;
	char	*t;

	for( ; *p && *s; ++s, ++p )
	{
		switch( *s )
		{
		case '?':
			break;
		case '[':
			found = 0;
			while( *++s != ']' )
			{
				if( !found && *p == *s )
					found = 1;
			}
			if( !found )
				return( 0 );
			break;
		case '*':
			for( t = p; *t; ++t )
			{
				if( PathMatch( t, s + 1 ) )
					return( 1 );
			}
			return( 0 );
		default:
			/* non-meta, must match exactly */
			if( *s != *p )
				return( 0 );
		}
	}
	return( *s == *p );	/* both should be '\0' */
}



/*	char *	PathSet() -
 *		set a path object to a value
 *
 *	given:	PathT p - the path object to set
 *		char *s - the value to use
 *	does:	set the PathT to the value
 *	return:	a pointer to the initial value
*/

char *PathSet( p, s )
PathT p;
char *s;
{
	(void) strncpy( p, s, MAXPATHLEN );
	p[MAXPATHLEN] = '\0';
	return( (char *) p );
}


char *PathStripExt( p )
char *p;
{
	static PathT	s;
	char		*t;

	t = s;
	while( *p && *p != '.' )
	{
		*t++ = *p++;
	}
	*t = '\0';
	return((char *) s);
}

