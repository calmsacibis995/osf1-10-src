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
#if !defined(lint) && !defined(_NOIDENT)
static char rcsid[] = "@(#)$RCSfile: chremdir.c,v $ $Revision: 4.3.5.2 $ (DEC) $Date: 1993/09/07 16:04:09 $";
#endif
/* 
 * COMPONENT_NAME: UUCP chremdir.c
 * 
 * FUNCTIONS: chremdir, mkremdir 
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	/sccs/src/cmd/uucp/s.chremdir.c
	chremdir.c	1.1	7/29/85 16:32:42
*/
#include "uucp.h"
/* VERSION( chremdir.c	5.2 -  -  ); */
/*
chremdir.c	1.3  com/cmd/uucp,3.1,9013 10/10/89 13:34:15";
*/

/*
 * chremdir(sys)
 * char	*sys;
 *
 * create SPOOL/sys directory and chdir to it
 * side effect: set RemSpool
 */
void
chremdir(sys)
char	*sys;
{
	int	ret;

	mkremdir(sys);	/* set RemSpool, makes sure it exists */
	DEBUG(6, "chdir(%s)\n", RemSpool);
	ret = chdir(RemSpool);
	ASSERT(ret == 0, Ct_CHDIR, RemSpool, errno);
	(void) strcpy(Wrkdir, RemSpool);
	return;
}

/*
 * mkremdir(sys)
 * char	*sys;
 *
 * create SPOOL/sys directory
 */

void
mkremdir(sys)
char	*sys;
{
	(void) sprintf(RemSpool, "%s/%s", SPOOL, sys);
	(void) mkdirs2(RemSpool, DIRMASK);
	return;
}
