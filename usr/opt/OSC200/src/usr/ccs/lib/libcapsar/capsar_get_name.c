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
static char *sccsid = "@(#)$RCSfile: capsar_get_name.c,v $ $Revision: 4.2.3.2 $ (DEC) $Date: 1992/03/03 15:25:13 $";
#endif lint

/*
 * capsar_get_name.c    01/14/88
 * Lynn C Wood
 *
 * This routine is provided as part of the capsar library for handling
 * of Multiple Body Part messages in Ultrix Mail
 *
 * syntax : char *capsar_get_name (MM *m)
 *
 * This routine retruns a pointer to the name associated with message m
 *
 */

#include <stdio.h>
#include <capsar.h>

char *capsar_get_name(m)
MM	*m;	/* pointer to message structure */
{
/*------*\
  Locals
\*------*/

	char	*sc;

	if(m==NULL)return(NULL);

	sc = getcpy(m->name);

	return(sc);
}


