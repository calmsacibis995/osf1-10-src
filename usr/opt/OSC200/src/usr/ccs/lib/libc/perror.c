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
static char	*sccsid = "@(#)$RCSfile: perror.c,v $ $Revision: 4.2.7.4 $ (DEC) $Date: 1993/10/19 18:10:34 $";
#endif 
/*
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 */
/*
 * OSF/1 1.2
 */
/*
 * FUNCTIONS: perror
 *
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
 * perror.c	1.12  com/lib/c/gen,3.1,8943 10/16/89 08:46:59
 */

/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#endif
#include <string.h>
#include <errno.h>
#include <nl_types.h>
#include "ts_supp.h"

extern int sys_nerr, write();
extern char *sys_errlist[];

#include "libc_msg.h"
#include <limits.h>


/*
 * NAME:	perror
 *
 * FUNCTION:	perror writes a message to the standard error output
 *		describing the last error encountered by a system
 *		call or the last error encountered by a subroutine call
 *		that set 'errno'.
 *
 * NOTES:	Perror is very useful in describing a system error after
 *		a system call fails.
 *
 *		sys_nerr, errno and sys_errlist are all used
 *
 * RETURN VALUE DESCRIPTION:	none
 */

void 	
perror(const char *s)
{
	int n = 0;
	int e;
	char *c;		/* pointer to string to be printed	*/

	nl_catd	catd;
	char buf[NL_TEXTMAX];

        e = TS_GETERR();
	catd = catopen(MF_LIBC, NL_CAT_LOCALE);
	if(e < sys_nerr && e >= 0)   {
		c = catgets(catd, MS_ERRNO, e, sys_errlist[e]);
	        if(strcmp(c,"") == 0)
		    sprintf(c = &buf[0], catgets(catd, MS_LIBC, M_PERROR,
		    "Error %d occurred."), e);

	}
	else
		sprintf(c = &buf[0], catgets(catd, MS_LIBC, M_PERROR,
		    "Error %d occurred."), e);

	if(s) {
		n = strlen(s);
	}
	if(n) {
		(void) write(2, s, (unsigned) n);
		(void) write(2, ": ", 2);
	}
	(void) write(2, c, (unsigned) strlen(c));
	(void) write(2, "\n", 1);
	catclose(catd);
}
