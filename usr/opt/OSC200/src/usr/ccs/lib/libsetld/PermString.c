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
static  char *sccsid = "@(#)$RCSfile: PermString.c,v $ $Revision: 4.2.2.3 $ (DEC) $Date: 1992/10/13 14:59:31 $";
#endif
/*
 */
#if !defined( lint) && !defined(_NOIDENT)

#endif
/*	PermString.c -
 *		libsetld.a module for xlating mode information into
 *		rw-r---r-- style strings.
 *
 *	mods:
 *	000	??-mar-1989	ccb
 *		New
 *	001	16-jun-1989	ccb
 *		lint
*/

#include	<sys/types.h>
#include	<sys/stat.h>
#include	<string.h>

/*LINTLIBRARY*/

#ifdef TESTMAIN
/* Compile with TESTMAIN defined to test
 *  the subr as a free standing unit.
*/
#include	<stdio.h>

main( argc, argv )
int argc;
char *argv[];
{
	int perms;
	char buf[80];
	char *PermString();

	while( gets( buf ) != NULL )
	{
		(void) sscanf( buf, "%o", &perms );
		(void) printf( "%s\n", PermString( (mode_t) perms ) );
	}
}
#endif



/*	PermString() -
 *		translate perms to a rw-rw-r-- type string
 *
 *	given:	permission bits to translate
 *	does:	assemble a permission string that represents the bits
 *	return:	a pointer to the string
*/

char *PermString( perms )
mode_t perms;
{
	static char	*xtab[] =	/* perm stringlets */
			{
				"---", "--x", "-w-", "-wx",
				"r--", "r-x", "rw-", "rwx"
			};

	static char	outbuf[10];	/* "rw-rw-r--\0" */
	int		i;		/* loop control */
	mode_t		p;		/* temp perms storage */

	for( p = perms, i = 6; i >= 0; i -= 3 )
	{
		(void) strncpy( outbuf+i, xtab[p & 07], 3 );
		p >>= 3;
	}
	/* doctor in the other bits */
	if( perms & S_ISUID )
		outbuf[2] = 's';	/* set-uid */

	if( perms & S_ISGID )
		outbuf[5] = 's';	/* set-gid */

	if( perms & S_ISVTX )
		outbuf[8] = 't';	/* sticky */

	return( outbuf );
}

