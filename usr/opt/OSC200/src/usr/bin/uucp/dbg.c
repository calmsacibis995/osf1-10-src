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
static char rcsid[] = "@(#)$RCSfile: dbg.c,v $ $Revision: 4.3.5.2 $ (DEC) $Date: 1993/09/07 16:04:44 $";
#endif
/* 
 * COMPONENT_NAME: UUCP dbg.c
 * 
 * FUNCTIONS: dbgmsg, dbgopt, dbgtime, dbgtty 
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
1.3  /com/cmd/uucp/dbg.c, bos320 6/15/90 23:58:47";
*/

#include "dbg.h"
#include "uucp.h"
#include <ctype.h>
#include <stdio.h>

/* static char sccsid[] = "dbg.c	5.2 -  - "; */

char  dbgflag = 1;	/* 0-9 are legal values */
char *dbgfile = 0;	/* Name of dbg output file */
FILE *dbgout  = 0;	/* Write dbg output here */
char *dbgtime();	/* Returns ASCII timestamp */
char *dbgtimp = 0;	/* ASCII version of time */
time_t  dbgtimv = 0;	/* Unix 32-bit timestamp */
extern int   sys_nerr;

/* Debugging output routine.
*/
dbgmsg(fmt,x0,x1,x2,x3,x4,x5,x6,x7,x8,x9)
	char *fmt;
{	int   r;

	if (dbgout == 0) 
		return 0;
	r = fprintf(dbgout,fmt,x0,x1,x2,x3,x4,x5,x6,x7,x8,x9);
	r+= fprintf(dbgout,"\n");
	fflush(dbgout);
	return r;
}
/* This routine decodes a "debug" option, which consists of a single-digit
** debug level, followed by a file name.  The file is opened as dbgout, and
** all debugging output goes there.  Note that we expect to be passed the
** address of the digit, which will usually be embedded in a longer string
** like "-X5/aud/fubar".
*/
dbgopt(ptr)
	char *ptr;
{
	if (isdigit(*ptr))
		 dbgflag = *ptr++ - '0';
	else dbgflag++;
	if (*ptr) 				/* There may be a file name specified */
		dbgfile = ptr;		/* If not, */
	if (dbgfile) {			/* there may be a default name */
		if (dbgout) fclose(dbgout);
		dbgout = fopen(dbgfile,"a");
		if (dbgout == 0) {
			fprintf(stderr,MSGSTR(MSG_DBG1,
			"Can't append to \"%s\"	[errno=%d]\n"), dbgfile,errno);
			sleep(10);
			exit(1);
		}
		D1("+-----------------------+");
		D1("%s.",dbgtime());
		D3(MSGSTR(MSG_DBG2,"Opened dbgfile \"%s\""),dbgfile);
	} else {
		/* If no debug file was specified, we might want to sent the
		** debug output to stderr, or we might just want to discard
		** it entirely.
		*/
		dbgout = stderr;	/* Let's send it to stderr */
	}
}
dbgtty(s)
	char *s;
{	int fd, saverr;

	saverr = errno;
	errno = 0;
	fd = open("/dev/tty",0);
	D2("open(\"/dev/tty\",0)=%d [errno=%d]",fd,errno);
	if (errno > 0 && errno < sys_nerr)
		D1(MSGSTR(MSG_DBG3,"Can't open /dev/tty due to %s [%s]"),
		   sys_errlist,s);
	close(fd);
	errno = saverr;
}
/*
** Get the current local time, return pointer to the ASCII version.
*/
char *dbgtime()
{

	time(&dbgtimv);
	dbgtimp = ctime(&dbgtimv);
	dbgtimp[24] = '\0';		/* We don't want the newline */
	return dbgtimp;
}
