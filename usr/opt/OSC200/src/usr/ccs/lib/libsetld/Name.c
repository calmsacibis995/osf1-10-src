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
static  char *sccsid = "@(#)$RCSfile: Name.c,v $ $Revision: 4.2.3.2 $ (DEC) $Date: 1992/10/13 13:38:43 $";
#endif
/*
 */
#if !defined( lint) && !defined(_NOIDENT)

#endif
/*	Name.c -
 *		libsetld. Routine for manipulating NameT
 *
 *	mods:
 *	001	890605	ccb	New.
 *	002	24-jul-1989	ccb
 *		wrote obligatory comment headers for all routines
 *		lint
*/

#include	<sys/param.h>
#include	<sys/types.h>
#include	<sys/dir.h>
#include	<string.h>
#include	<stdio.h>
#include	<setld/setld.h>

/*LINTLIBRARY*/

/*	char	*NameSet() -
 *		set NameT value
 *
 *	given:	NameT n - NameT to set
 *		char *s - value to use
 *	does:	sets value of n to s with all type specific checking
 *	return:	the value as set (may have been truncated)
*/

char *NameSet( n, s )
NameT n;
char *s;
{
	(void) strncpy( n, s, NAMELEN );
	n[NAMELEN] = '\0';
	return( n );
}


/*	char	*NameGetPcode() -
 *		get product code from a name
 *
 *	given:	NameT n - the name to use
 *	does:	extract the pcode section into a static buffer
 *	return:	a pointer to the static buffer
*/

char *NameGetPcode( n )
NameT n;
{
	static CodeT	c;

	(void) strncpy( c, n, CODELEN );
	c[CODELEN] = '\0';
	return( c );
}


/*	char	*NameGetVcode() -
 *		get version code from a name
 *
 *	given:	NameT n - the name to use
 *	does:	extract the vcode section of the name into a static buffer
 *	return:	a pointer to the static buffer
*/

char *NameGetVcode( n )
NameT n;
{
	static CodeT	c;

	(void) strncpy( c, n + strlen(n) - CODELEN, CODELEN );
	c[CODELEN] = '\0';
	return( c );
}

