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
static char	*sccsid = "@(#)$RCSfile: strerror.c,v $ $Revision: 4.2.5.6 $ (DEC) $Date: 1993/12/09 20:57:02 $";
#endif 
/*
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 */
/*
 * OSF/1 1.2
 */
/*
 * FUNCTIONS: strerror
 *
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * strerror.c	1.10  com/lib/c/str,3.1,9013 11/28/89 16:12:03
 */

/*
 *
 * FUNCTION: Maps the error number in errnum to an error message string.
 *
 * NOTES:    The ANSI Programming Language C standard requires this routine.
 *
 * PARAMETERS:
 *	     int errnum - error number to be associated with error message
 *
 * RETURN VALUE DESCRIPTION: Returns a pointer to the error message
 *	     string associated with the error number.
 */
/*LINTLIBRARY*/

/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#if defined(_THREAD_SAFE)
#pragma weak strerror_r = __strerror_r
#endif
#endif
#include "libc_msg.h"
#include <limits.h>
#include <stdlib.h>
#include <mesg.h>
#include <errno.h>

extern int sys_nerr;
extern char *sys_errlist[];

#ifndef _THREAD_SAFE

static char *strerrbuf;		/* [NL_TEXTMAX] */

#endif

#ifdef _THREAD_SAFE
int
strerror_r(int errnum, char *buf, int buflen)
#else
char 	*
strerror(int errnum)
#endif
{
	char *c;
	nl_catd	catd;

#ifdef _THREAD_SAFE
	char	strerrbuf[NL_TEXTMAX];
	int	strerrlen;

	if ((buf == NULL) || (buflen <= 1)) {
		_Seterrno(EINVAL);
		return(-1);
	}
#else
	if (!strerrbuf)
		strerrbuf = malloc(NL_TEXTMAX);
#endif	/* _THREAD_SAFE */

	c = strerrbuf;

	catd = catopen(MF_LIBC, NL_CAT_LOCALE);
	if((errnum > 0) && (errnum < sys_nerr)) {
		c = catgets(catd, MS_ERRNO, errnum, sys_errlist[errnum]);
		if(strcmp(c,"") == 0) {
		  _Seterrno(EINVAL);
		  (void) sprintf(c,catgets(catd, MS_LIBC, M_PERROR, 
			      "Error %d occurred."), errnum);
		}
	}
	else {
		_Seterrno(EINVAL);
		(void) sprintf(c
			      ,catgets(catd, MS_LIBC, M_PERROR, "Error %d occurred.")
			      , errnum);
	}

#ifdef _THREAD_SAFE
	if ((strerrlen = strlen(c)) >= buflen) {
		strerrlen = buflen - 1;
		strncpy(buf, c, strerrlen);
		buf[strerrlen] = '\0';
	} else
		strcpy(buf, c);
#else
	if (c != strerrbuf) {
		strcpy(strerrbuf, c);
		c = strerrbuf;
	}
#endif	/* _THREAD_SAFE */

	catclose(catd);

#ifdef _THREAD_SAFE
	return (0);
#else
	return (c);
#endif	/* _THREAD_SAFE */

}
