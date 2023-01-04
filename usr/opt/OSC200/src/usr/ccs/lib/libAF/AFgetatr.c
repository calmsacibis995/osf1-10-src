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
static char	*sccsid = "@(#)$RCSfile: AFgetatr.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 02:41:18 $";
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
 * IBM CONFIDENTIAL
 * Copyright International Business Machines Corp. 1985, 1989
 * Unpublished Work
 * All Rights Reserved
 * Licensed Material - Property of IBM
 *
 * RESTRICTED RIGHTS LEGEND
 * Use, Duplication or Disclosure by the Government is subject to
 * restrictions as set forth in paragraph (b)(3)(B) of the Rights in
 * Technical Data and Computer Software clause in DAR 7-104.9(a).
 */

/*
 *
 * NAME:	AFgetstr
 * FUNCTION:	Get the attribute value matching name.
 * RETURN VALUE DESCRIPTION: Returns a pointer to the value of name.
 */

#include <stdio.h>
#include <string.h>
#include <AFdefs.h>

ATTR_t
AFgetatr( ENT_t entry, char * name )
{
	if (entry == NULL || name == NULL)
		return(NULL);

	if ((entry->EN_natr=entry->EN_catr) == NULL)		/* Null list */
		return(NULL);

	for ( ;entry->EN_natr->AT_name != NULL; entry->EN_natr++) {
	    
	    if (strcmp(entry->EN_natr->AT_name, name) == 0) {
		    entry->EN_natr->AT_nvalue = NULL;
		    return(entry->EN_natr);
	    }

	}
	return(NULL);
}
