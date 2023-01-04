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
static char *sccsid = "@(#)$RCSfile: capsar_is_body_type.c,v $ $Revision: 4.2.3.2 $ (DEC) $Date: 1992/03/03 15:25:27 $";
#endif lint

/*
 * capsar_is_body_type.c    01/14/88
 * Lynn C Wood
 *
 * This routine is provided as part of the capsar library for handling
 * of Multiple Body Part messages in Ultrix Mail
 *
 * syntax : int capsar_is_body_type ( MM *m , char *desc)
 *
 */

#include <stdio.h>
#include <ctype.h>
#include <capsar.h>

int capsar_is_body_type(m,desc)
MM	*m;	/* pointer to message structure */
char	*desc;
{
/*------*\
  Locals
\*------*/

	if(!m || m->body_type == NULL) return(0);
	if(compare(m->body_type,desc,strlen(desc)) == 0)
			return(1);

	else return(0);

}
