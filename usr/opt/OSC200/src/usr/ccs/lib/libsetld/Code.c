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
static  char *sccsid = "@(#)$RCSfile: Code.c,v $ $Revision: 4.2.3.2 $ (DEC) $Date: 1992/10/13 13:31:10 $";
#endif
/*
 */
#if !defined( lint) && !defined(_NOIDENT)

#endif
/*	Code.c -
 *		CodeT routines
 *
 *	mods:
 *	000	20-jun-1989	ccb
 *		culled from inv.c
 *	001	16-aug-1989	ccb
 *		force '\0' string terminator in CodeSet
*/

#include	<sys/param.h>
#include	<sys/types.h>
#include	<sys/dir.h>
#include	<string.h>
#include	<stdio.h>
#include	<setld/setld.h>

/*LINTLIBRARY*/

/*	char	*CodeSet() -
 *		set a CodeT to a value
 *
 *	given:	CodeT s - code to set
 *		char *t - value to use
 *	does:	set s to value of t
 *	return:	address of s
*/

char *CodeSet( s, t )
CodeT s;
char *t;
{
	(void) strncpy( s, t, CODELEN );
	s[CODELEN] = '\0';
	return( s );
}


