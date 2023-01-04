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
static char	*sccsid = "@(#)$RCSfile: xopen.c,v $ $Revision: 4.2.2.2 $ (DEC) $Date: 1993/09/23 18:29:23 $";
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
 * RESTRICTED RIGHTS LEGEND
 * Use, Duplication or Disclosure by the Government is subject to
 * restrictions as set forth in paragraph (b)(3)(B) of the rights in
 * Technical Data and Computer Software clause in DAR 7-104.9(a).
 */ 

/*
 * COMPONENT_NAME: (LIBPW) Programmers Workbench Library
 *
 * FUNCTIONS: xopen
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

# include "errno.h"

/*
 * FUNCTION: Interface to open(II).
 *
 * RETURN VALUE DESCRIPTIONS:
 *	Returns file descriptor on success,
 *	fatal() on failure.
 */
/*
	Interface to open(II) which differentiates among the various
	open errors.
	Returns file descriptor on success,
	fatal() on failure.
*/


#ifdef MSG
#include "pw_msg.h"
nl_catd	catd;
#define MSGSTR(Num, Str) catgets(catd, MS_PW, Num, Str)
#else
#define MSGSTR(Num, Str) Str
#endif

xopen(name,mode)
char name[];
int mode;
{
	register int fd;
	extern int errno;
	extern char Error[];

	if ((fd = open(name,mode)) < 0) {
		if(errno == EACCES) {
#ifdef MSG
			catd = catopen(MF_PW, NL_CAT_LOCALE);
#endif
			if(mode == 0)
#ifdef MSG
				sprintf(Error, MSGSTR(XNORD,
					"`%s' unreadable (ut5)"), name);
#else
				sprintf(Error,"`%s' unreadable (ut5)",name);
#endif
			else if(mode == 1)
#ifdef MSG
				sprintf(Error, MSGSTR(XNOWRT,
					"`%s' unwritable (ut6)"), name);
#else
				sprintf(Error,"`%s' unwritable (ut6)",name);
#endif
			else
#ifdef MSG
				sprintf(Error, MSGSTR(XNORDWRT,
					"`%s' unreadable or unwritable (ut7)"),
					name);
#else
				sprintf(Error,"`%s' unreadable or unwritable (ut7)",name);
#endif
			fd = fatal(Error);
		}
		else
			fd = xmsg(name,"xopen");
	}
	return(fd);
}
