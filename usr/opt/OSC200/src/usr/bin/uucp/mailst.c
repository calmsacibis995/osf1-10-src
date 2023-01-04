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
static char rcsid[] = "@(#)$RCSfile: mailst.c,v $ $Revision: 4.3.5.2 $ (DEC) $Date: 1993/09/07 16:06:45 $";
#endif
/* 
 * COMPONENT_NAME: UUCP mailst.c
 * 
 * FUNCTIONS: mailst, setuucp 
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */



/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/
/*
 *	1.5  /com/cmd/uucp/mailst.c, bos320 6/15/90 23:59:58";
 */
#include "uucp.h"
/* VERSION( mailst.c	5.3 -  -  ); */

/*
 * fork and execute a mail command sending 
 * string (str) to user (user).
 * If file is non-null, the file is also sent.
 * (this is used for mail returned to sender.)
 *	user	 -> user to send mail to
 *	str	 -> string mailed to user
 *	infile	 -> optional stdin mailed to user
 *	errfile	 -> optional stderr mailed to user
 */
mailst(user, str, infile, errfile)
char *user, *str, *infile, *errfile;
{
	register FILE *fp, *fi;
	char cmd[BUFSIZ];

	(void) sprintf(cmd, "%s mail %s", PATH, user);
	if ((fp = popen(cmd, "w")) == NULL)
		return;
	(void) fprintf(fp, "%s\n", str);

	/* copy back stderr */
	if (*errfile != '\0' && NOTEMPTY(errfile) && (fi = fopen(errfile, "r")) != NULL) {
		fprintf(fp, MSGSTR(MSG_MAILST2,
				"\n\t===== stderr was =====\n"));
		if (xfappend(fi, fp) != SUCCESS)
			fprintf(fp, MSGSTR(MSG_MAILST3, 
			   "\n\t===== well, i tried =====\n"));
		(void) fclose(fi);
		fprintf(fp, "\n");
	}

	/* copy back stdin */
	if (*infile != '\0' && NOTEMPTY(infile) && (fi = fopen(infile, "r")) != NULL) {
		fprintf(fp, MSGSTR(MSG_MAILST4, 
			"\n\t===== stdin was =====\n"));
		if (xfappend(fi, fp) != SUCCESS)
			fprintf(fp, MSGSTR(MSG_MAILST3, 
				"\n\t===== well, i tried =====\n"));
		(void) fclose(fi);
		fprintf(fp, "\n");
	}

	(void) pclose(fp);
}
#ifndef	V7
static char un[2*NAMESIZE];
setuucp(p)
char *p;
{
   char **envp;

    envp = Env;
    for ( ; *envp; envp++) {
	if(PREFIX("LOGNAME", *envp)) {
	    (void) sprintf(un, "LOGNAME=%s",p);
	    envp[0] = &un[0];
	}
    }
}
#else
/*ARGSUSED*/
setuucp(p) char	*p; {}
#endif
