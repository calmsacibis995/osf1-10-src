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
static char	*sccsid = "@(#)$RCSfile: AFnxtatr.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 02:41:26 $";
#endif 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0
 */
/*
 * NAME:	AFnxtatr 
 * FUNCTION: 	Get the next attribute for Attribute File entry
 * RETURN VALUE DESCRIPTION: Returns a pointer to an Attribute structure.
 */

#include <stdio.h>
#include <string.h>
#include <AFdefs.h>

ATTR_t
AFnxtatr( ENT_t entry )
{
	if (entry == NULL || entry->EN_catr == NULL)
		return(NULL);

	if (entry->EN_natr == NULL )
		entry->EN_natr = entry->EN_catr;
	else if ( entry->EN_natr->AT_name != NULL )
		entry->EN_natr++;

	if ( entry->EN_natr->AT_name == NULL )
		return(NULL);
	entry->EN_natr->AT_nvalue = NULL;
	return(entry->EN_natr);
}
