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
static  char *sccsid = "@(#)$RCSfile: verify.c,v $ $Revision: 4.2.3.2 $ (DEC) $Date: 1992/10/13 13:49:08 $";
#endif
/*
 */
#if !defined( lint) && !defined(_NOIDENT)

#endif
/*	verify.c
 *		library routines used for subset verification
 *
 *	mods:
 *	000	2-feb-1989	Chas Bennett
 *		new.
 *
 *	001	1-may-1989	Chas Bennett
 *		modify FVerify so that files of type 'l' will match
 *		any type.
*/

#include	<sys/param.h>
#include	<sys/types.h>
#include	<sys/stat.h>
#include	<sys/dir.h>
#include	<stdio.h>
#include	<setld/setld.h>


/*LINTLIBRARY*/

/*	InvRecT *FVerify() -
 *		Compares the stat information from a file against an inventory
 *	record.
 *
 *	given: 	InvRecT *p - a pointer to the record to be verified.
 *	does:	looks up the file in the filesystem. For each attribute
 *		requested that doesn't match the inventory record, a bit
 *		is set in a return mask which is included in p->i_flags
 *	return:	a pointer to the static InvRecT used in the verify
*/

InvRecT *FVerify( p )
InvRecT *p;
{
	unsigned	mask;	/* output mask */
	char		*path;	/* path info from InvRecT */
	InvRecT		*real;	/* inventory record for actual file */
	struct stat	stb;	/* stat buffer */
	char		type;	/* type info from InvRecT */

	path = p->i_path;
	if( lstat( path, &stb ) )
	{
		p->i_vflags |= I_PATH;
		return( NULL );
	}
	real = StatToInv( &stb );

	mask = 0;
	type = p->i_type;

	if( type == 'f' && p->i_size != real->i_size )
		mask |= I_SIZE;

	if( type == 'f' && p->i_sum != (real->i_sum = CheckSum(path)) )
		mask |= I_SUM;

	if( p->i_uid != real->i_uid )	/* user id */
		mask |= I_UID;

	if( p->i_gid != real->i_gid )	/* group id */
		mask |= I_GID;

	if( PERM( p->i_mode ) != PERM( real->i_mode ) )	/* permissions */
		mask |= I_PERM;

	if( strcmp( p->i_date, real->i_date  ) )
		mask |= I_DATE;

	if( type != real->i_type && type != 'l' )
		mask |= I_TYPE;

	/* referent checking is not done here, calling functions
	 *  which need it can implement checking there
	*/

	p->i_vflags = mask;
	return( real );
}

