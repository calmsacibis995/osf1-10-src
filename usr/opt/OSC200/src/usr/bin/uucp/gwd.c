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
static char rcsid[] = "@(#)$RCSfile: gwd.c,v $ $Revision: 4.3.5.2 $ (DEC) $Date: 1993/09/07 16:06:14 $";
#endif
/* 
 * COMPONENT_NAME: UUCP gwd.c
 * 
 * FUNCTIONS: gwd, uidstat 
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

/*
gwd.c	1.5  com/cmd/uucp,3.1,9013 2/28/90 15:06:30";
*/
/*	/sccs/src/cmd/uucp/s.gwd.c
	gwd.c	1.3	7/29/85 16:33:09
*/
#include "uucp.h"
/* VERSION( gwd.c	5.2 -  -  ); */

#if 0
/*
 *	gwd - get working directory
 *	Uid and Euid are global
 *	return
 *		0 - ok
 *	 	FAIL - failed
 */

gwd(wkdir)
char *wkdir;
{
	FILE *fp;
	char cmd[BUFSIZ];

	*wkdir = '\0';
	(void) sprintf(cmd, "%s pwd 2>&-", PATH);

#ifndef V7
	(void) setuid(Uid);
	fp = popen(cmd, "r");
	(void) setuid(Euid);
#else
	fp = popen(cmd, "r");
#endif

	if (fp == NULL || fgets(wkdir, MAXFULLNAME, fp) == NULL) {
		(void) pclose(fp);
		return(FAIL);
	}
	if (wkdir[strlen(wkdir)-1] == '\n')
		wkdir[strlen(wkdir)-1] = '\0';
	(void) pclose(fp);
	return(0);
}

#endif

/*
**  Return the current working directory in a lot less time than
**  the original HDB version of gwd() [see above]. 
*/

gwd(wkdir)
char *wkdir;
{
	int gwdstat;
	(void) setuid(Uid);
	gwdstat = (getcwd(wkdir, MAXFULLNAME-1) == NULL);
	(void) setuid(Euid);
	return gwdstat;
}


/*
 * uidstat(file, &statbuf)
 * This is a stat call with the uid set from Euid to Uid.
 * Used from uucp.c and uux.c to permit file copies
 * from directories that may not be searchable by other.
 * return:
 *	same as stat()
 */

int
uidstat(file, buf)
char *file;
struct stat *buf;
{
#ifndef V7
	register ret;

	(void) setuid(Uid);
	ret = stat(file, buf);
	(void) setuid(Euid);
	return(ret);
#else
	int	ret;

	if (vfork() == 0) {
		(void) setuid(Uid);
		_exit(stat(file, buf));
	}
	wait(&ret);
	return(ret);
#endif
}
