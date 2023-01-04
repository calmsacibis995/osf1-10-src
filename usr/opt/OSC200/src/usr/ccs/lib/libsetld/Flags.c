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
static  char *sccsid = "@(#)$RCSfile: Flags.c,v $ $Revision: 4.2.3.2 $ (DEC) $Date: 1992/10/13 13:37:28 $";
#endif
/*
 */
#if !defined( lint) && !defined(_NOIDENT)

#endif
/*	Flags.c
 *		routines for manipulating FlagsT
 *
 *	mods:
 *	000	20-jun-1989	ccb
 *		forgot to set this up before. Flags.c is about 2 weeks
 *			old now.
 *	001	24-jul-1989	ccb
 *		lint
*/

#include	<sys/param.h>
#include	<sys/types.h>
#include	<sys/dir.h>
#include	<stdio.h>
#include	<setld/setld.h>

/*LINTLIBRARY*/

/*	FlagsT	FlagsScan() -
 *		xlate string to flag
 *
 *	given:	StringT s
 *	does:	xlate value in s to a FlagsT
 *	return:	the FlagsT value
*/
 
FlagsT FlagsScan( s )
StringT s;
{
	FlagsT	f;

	(void) sscanf( s, "%x", &f );
	return( f );
}

