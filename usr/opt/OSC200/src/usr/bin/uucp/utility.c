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
static char rcsid[] = "@(#)$RCSfile: utility.c,v $ $Revision: 4.3.6.2 $ (DEC) $Date: 1993/09/07 16:08:34 $";
#endif
/* 
 * COMPONENT_NAME: UUCP utility.c
 * 
 * FUNCTIONS: assert, errent, logError, timeStamp 
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/* 
1.4  /com/cmd/uucp/utility.c, bos320 6/16/90 00:01:24";
 */
/* VERSION( utility.c	5.3 -  -  ); */

#include "uucp.h"
static void logError();

#define TY_ASSERT	1
#define TY_ERROR	2

/*
 *	produce an assert error message
 * input:
 *	s1 - string 1
 *	s2 - string 2
 *	i1 - integer 1 (usually errno)
 *	sid - rcsid string of calling module
 *	file - __FILE of calling module
 *	line - __LINE__ of calling module
 */
void
assert(s1, s2, i1, sid, file, line)
char *s1, *s2, *file, *sid;
{
	logError(s1, s2, i1, TY_ASSERT, sid, file, line);
}


/*
 *	produce an assert error message
 * input: -- same as assert
 */
void
errent(s1, s2, i1, sid, file, line)
char *s1, *s2, *sid, *file;
{
	logError(s1, s2, i1, TY_ERROR, sid, file, line);
}

#define UEFORMAT	"%sERROR (%.9s)  pid: %d (%s) %s %s (%d) [SCCSID: %s, FILE: %s, LINE: %d]\n"

static
void
logError(s1, s2, i1, type, sid, file, line)
char *s1, *s2, *sid, *file;
{
	register FILE *errlog;
	char text[BUFSIZ];
	int pid;

	if (Debug)
		errlog = stderr;
	else {
		errlog = fopen(ERRLOG, "a");
		(void) chmod(ERRLOG, 0666);
	}
	if (errlog == NULL)
		return;

	pid = getpid();

	(void) fprintf(errlog, UEFORMAT, type == TY_ASSERT ? "ASSERT " : " ",
	    Progname, pid, timeStamp(), s1, s2, i1, sid, file, line);

	(void) fclose(errlog);
	(void) sprintf(text, " %sERROR %.100s %.100s (%.9s)",
	    type == TY_ASSERT ? "ASSERT " : " ",
	    s1, s2, Progname);
	if (type == TY_ASSERT)
	    systat(Rmtname, SS_ASSERT_ERROR, text, Retrytime);
	return;
}


/* timeStamp - create standard time string
 * return
 *	pointer to time string
 */

char *
timeStamp()
{
	time_t clock;
	struct tm *tp;
	static char timbuf[128]; /* Leave lots of space */
	size_t strftime();
	struct tm *localtime();

	(void) time(&clock);
	tp = localtime(&clock);
	strftime( timbuf, sizeof(timbuf), "%sD %T %Y", tp);
	return(timbuf);
}
