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
static  char *sccsid = "@(#)$RCSfile: mi.c,v $ $Revision: 4.2.3.2 $ (DEC) $Date: 1992/10/13 13:47:56 $";
#endif
/*
 */
#if !defined( lint) && !defined(_NOIDENT)

#endif
/*	mi.c
 *
 *	mods:
 *	000	07-mar-1989	ccb
 *	New.
 *	001	14-jun-1989	ccb
 *		Type clean-up wrt setld.h
*/

/*!	Contructors, Recognizers, Operators
*/

#include	<sys/param.h>
#include	<sys/dir.h>
#include	<stdio.h>
#include	<setld/setld.h>

/*LINTLIBRARY*/

#define	MIRECMAX	(sizeof(PathT)+(sizeof(NameT)*2)+2)

static FILE	*MiTToFILE();

/*	MiT	*MiInit() -
 *		convert FILE to MiT
 *
 *	given:	FILE * p - a FILE pointer
 *	does:	converts it to an MiT
 *	return:	a pointer to the MiT, NULL if none available.
 *
 *	note:	currently does nothing but cast
*/

MiT *MiInit(p)
FILE *p;
{
	return( (MiT *) p );
}



/*	MiRecT	*MiRead() -
 *		read a master inventory record
 *
 *	given:	MiT *p - a pointer to a master inventory
 *	does:	read one record from the inventory
 *	return:	NULL on EOF, else a (static) MiRecT containing the information
 *		from the record.
*/

MiRecT *MiRead(p)
MiT *p;
{
	static MiRecT	r;			/* static record */
	static char	buf[MIRECMAX+1];	/* input buffer */
	static NameT	flgtmp;			/* tmp flag buffer */

	if( fgets( buf, sizeof(buf), MiTToFILE(p) ) == NULL )
		return(NULL);

	(void) sscanf( buf, "%s\t%s\t%s\n", flgtmp, r.mi_path, r.mi_subset );

	/* attempt to extract a hex number from the flgtmp string. this
	 *  is done in two phases so the the first sscanf doesn't choke
	 *  on old style inventories.
	*/
	(void) sscanf( flgtmp, "%x", &r.mi_flags );
	return( &r );
}


/*	FILE	*MiTToFILE() -
 *		xlate MiT * to FILE *
*/

static FILE *MiTToFILE(p)
FILE *p;
{
	return( (MiT *) p );
}

