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
static char	*sccsid = "@(#)$RCSfile: xcreat.c,v $ $Revision: 4.2.2.2 $ (DEC) $Date: 1993/09/23 18:28:55 $";
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
 * COMPONENT_NAME: (LIBPW) Programmers Workbench Library
 *
 * FUNCTIONS: xcreat
 *
 * ORIGINS: 3 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 * OBJECT CODE ONLY SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989 
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1984 AT&T	
 * All Rights Reserved  
 *
 * THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	
 * The copyright notice above does not evidence any   
 * actual or intended publication of such source code.
 *
 */


/*
 * FUNCTION: Create a file.
 *
 * RETURN VALUE DESCRIPTIONS:
 *	Returns file descriptor on success,
 *	fatal() on failure.
 */

#ifdef MSG
#include "pw_msg.h"
nl_catd	catd;
#define MSGSTR(Num, Str) catgets(catd, MS_PW, Num, Str)
#else
#define MSGSTR(Num, Str) Str
#endif

# include	"sys/types.h"
# include	"macros.h"

/*
	"Sensible" creat: write permission in directory is required in
	all cases, and created file is guaranteed to have specified mode
	and be owned by effective user.
	(It does this by first unlinking the file to be created.)
	Returns file descriptor on success,
	fatal() on failure.
*/

xcreat(name,mode)
char *name;
int mode;
{
	register int fd;
	char d[sizeof Error];
	extern char *dname();

	copy(name,d);
	if (!exists(dname(d))) {
#ifdef MSG
		catd = catopen(MF_PW, NL_CAT_LOCALE);
		sprintf(Error, MSGSTR(XNODIR, 
			"directory `%s' does not exist (ut1)"), d);
#else
		sprintf(Error,"directory `%s' does not exist (ut1)",d);
#endif
		return(fatal(Error));
	}
	unlink(name);
	if ((fd = creat(name,mode)) >= 0)
		return(fd);
	return(xmsg(name,"xcreat"));
}
