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
static char *sccsid = "@(#)$RCSfile: capsar_get_body_type.c,v $ $Revision: 4.2.3.3 $ (DEC) $Date: 1992/06/03 17:29:00 $";
#endif lint

/*
 * capsar_get_body_type.c    01/14/88
 * Lynn C Wood
 *
 * This routine is provided as part of the capsar library for handling
 * of Multiple Body Part messages in Ultrix Mail
 *
 * syntax : char *capsar_get_body_type ( MM *m )
 *
 * For a SIMPLE_MESSAGE this routine returns the name of the format of
 * the message. For example a DDIF document has the name ddif.
 */

#include <stdio.h>
#include <ctype.h>
#include <capsar.h>
#include "./dtif.h"

char *capsar_get_body_type(m)
MM	*m;	/* pointer to message structure */
{
/*------*\
  Locals
\*------*/

	if(!m)return(NULL);
	if(m->start == NULL)
		return(BODY_TYPE_DEF);

	if(match(strlen(DDIFTAG),DDIFTAG,strlen(m->start),m->start))
		return(DDIFTAG);

	else if(match(strlen(DTIFTAG),DTIFTAG,strlen(m->start),m->start))
		return(DTIFTAG);

	else if(match(strlen(DOTSTAG),DOTSTAG,strlen(m->start),m->start))
		return(DOTSTAG);

	else return(BODY_TYPE_DEF);
}
