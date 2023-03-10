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
static char	*sccsid = "@(#)$RCSfile: AFnxtval.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 02:41:33 $";
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
 *
 * NAME:	AFnxtval
 * FUNCTION:	Get the next attribute value for current attribute.
 * RETURN VALUE DESCRIPTION: Returns a pointer the next string value in the
 *		attribute list.
 */

#include <stdio.h>
#include <string.h>
#include <AFdefs.h>

char *
AFnxtval( ATTR_t attr )
{
	if (attr == NULL || attr->AT_value == NULL)
		return(NULL);

	if (attr->AT_nvalue == NULL) {
		attr->AT_nvalue = attr->AT_value;
		return(attr->AT_nvalue);
	}

	for (;*attr->AT_nvalue != '\0'; attr->AT_nvalue++)
	;

	if ( *(attr->AT_nvalue+1) == '\0' )
		return(NULL);

	attr->AT_nvalue++;
	return(attr->AT_nvalue);
}
