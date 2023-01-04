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
static char rcsid[] = "@(#)$RCSfile: shio.c,v $ $Revision: 4.3.5.2 $ (DEC) $Date: 1993/09/07 16:07:46 $";
#endif
/* 
 * COMPONENT_NAME: UUCP shio.c
 * 
 * FUNCTIONS: shio 
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
shio.c	1.3  com/cmd/uucp,3.1,9013 10/10/89 13:47:43";
*/
/*	/sccs/src/cmd/uucp/s.shio.c
	shio.c	1.1	7/29/85 16:33:26
*/
#include "uucp.h"
/* VERSION( shio.c	5.2 -  -  ); */

/*
 * use shell to execute command with
 * fi, fo, and fe as standard input/output/error
 *	cmd 	-> command to execute
 *	fi 	-> standard input
 *	fo 	-> standard output
 *	fe 	-> standard error
 * return:
 *	0		-> success 
 *	non zero	-> failure  -  status from child
			(Note - -1 means the fork failed)
 */
shio(cmd, fi, fo, fe)
char *cmd, *fi, *fo, *fe;
{
	register int pid, ret;
	int status;

	if (fi == NULL)
		fi = "/dev/null";
	if (fo == NULL)
		fo = "/dev/null";
	if (fe == NULL)
		fe = "/dev/null";

	DEBUG(3, "shio - %s\n", cmd);
	if ((pid = fork()) == 0) {
		(void) signal(SIGINT, SIG_IGN);
		(void) signal(SIGHUP, SIG_IGN);
		(void) signal(SIGQUIT, SIG_IGN);
		closelog();
		(void) close(Ifn);	/* close connection fd's */
		(void) close(Ofn);
		(void) close(0);	/* get stdin from file fi */
		if (open(fi, 0) != 0)
			exit(errno);
		(void) close(1);	/* divert stdout to fo */
		if (creat(fo, 0666) != 1)
			exit(errno);
		(void) close(2);	/* divert stderr to fe */
		if (creat(fe, 0666) != 2)
			exit(errno);
		(void) execle(SHELL, "sh", "-c", cmd, 0, Env);
		exit(100);
	}

	/*
	 * the status returned from wait can never be -1
	 * see man page wait(2)
	 * So we use the -1 value to indicate fork failed
	 */
	if (pid == -1)
		return(-1);
	

	while ((ret = wait(&status)) != pid && ret != -1);
	DEBUG(3, "status %d\n", status);
	return(status);
}
