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
static  char *sccsid = "@(#)$RCSfile: Assign.c,v $ $Revision: 4.2.5.2 $ (DEC) $Date: 1993/10/29 18:13:31 $";
#endif
/*
 */
#if !defined( lint) && !defined(_NOIDENT)

#endif
/*	Assign.c -
 *		mainipulate AssignT
 *
 *	mods:
 *	000	890606	ccb	New
 *	001	24-jul-1989	ccb
 *		Include <sys/dir.h> for setld.h
 *		Rename AssignParse() to AssignScan() to conform to libsetld
 *			conventions
 *		Fix AssignScan() bug, value fields we comming back with
 *			unwanted trailing newlines.
 *
*/

#include	<sys/param.h>
#include	<sys/types.h>
#include	<sys/dir.h>
#include	<stdio.h>
#include	<setld/setld.h>


/*LINTLIBRARY*/


/*	char	*AssignGetName() -
 *		return name portion of a AssignT
 *
 *	given:	AssignT *a - assignment
 *	does:	get the name portion
 *	return:	a pointer to the name portion
*/

char *AssignGetName( a )
AssignT *a;
{
	return( a->a_name );
}


/*	char	*AssignGetVal() -
 *		get AssignT( value )
 *
 *	given:	AssignT *a - assign to use
 *	does:	finds the value associated with the assign
 *	return:	a pointer to the value (a string)
*/

char *AssignGetVal( a )
AssignT *a;
{
	return( a->a_val );
}


/*	AssignT	*AssignScan() -
 *		xlate string data into an assign
 *
 *	given:	char *s - string to format
 *	does:	breaks the name=value string into an assign.
 *	return:	a pointer to a static AssignT representing the assign. Will
 *		return NULL for syntax error ( =foo, baz )
*/
 
AssignT *AssignScan( s )
char *s;
{
	static AssignT	a;
	char		*t;

	/* two part operation -
	 *  copy name.
	*/
	for( t = a.a_name; *s && *s != '=' && *s != '\n'; ++s, ++t )
	{
		*t = *s;
	}
	*t = '\0';

	if( t == a.a_name || *s != '=' )
	{
		/* (t == a.a_name) - assign to nothing
		 * (*s != '=') - no equal sign
		*/
		return( NULL );
	}

	/* copy value
	*/
	++s;

	for( t = a.a_val; *s && *s != '\n'; ++s, ++t )
	{
		*t = *s;
	}
	*t = '\0';

	return( &a );
}



