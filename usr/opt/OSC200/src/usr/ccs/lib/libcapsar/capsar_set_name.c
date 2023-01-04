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
/*LINTLIBRARY*/

#ifndef lint
static char *sccsid = "@(#)$RCSfile: capsar_set_name.c,v $ $Revision: 4.2.3.2 $ (DEC) $Date: 1992/03/03 15:26:38 $";
#endif lint

/*
 * capsar_set_name.c    01/14/88
 * Lynn C Wood
 *
 * This routine is provided as part of the capsar library for handling
 * of Multiple Body Part messages in Ultrix Mail
 *
 * syntax : capsar_set_name (MM *m, char *name )
 *
 * For m that is eihter a SIMPLE MESSAGE or COMPOUND MESSAGE , set the name
 * of the message to name. The name is optional: if missing (ie null),
 * it is not included in the flattened representation of the message and
 * a sequence number is used in the hierachical form of the message
 *
 */

#include <stdio.h>
#include <capsar.h>

capsar_set_name(m,name)
MM	*m;	/* pointer to message structure */
char	*name;	/* message name */
{
/*------*\
  Locals
\*------*/

	if(m==NULL)
		return (NOTOK);
	
	if(m->message_type == SIMPLE_MESSAGE || m->message_type == COMPOUND_MESSAGE){
		if(m->name != NULL)free(m->name);
		m->name = getcpy(name);
	}
	return (OK);
}
