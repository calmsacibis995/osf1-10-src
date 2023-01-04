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
static char     *sccsid = "@(#)$RCSfile: emalloc.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 04:40:23 $";
#endif
/*
 */

/*
 * emalloc - return new memory obtained from the system.  Belch if none.
 */
#include <stdio.h>
#include <syslog.h>

char *
emalloc(size)
	unsigned int size;
{
	char *mem;
	extern char *malloc();

	if ((mem = malloc(size)) == 0) {
		syslog(LOG_ERR, "No more memory!");
		exit(1);
	}
	return mem;
}
