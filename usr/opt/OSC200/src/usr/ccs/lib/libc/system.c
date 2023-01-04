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
static char	*sccsid = "@(#)$RCSfile: system.c,v $ $Revision: 4.3.9.3 $ (DEC) $Date: 1993/06/29 18:19:20 $";
#endif 
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0
 */
/*
 * COMPONENT_NAME: (LIBCIO) Standard C Library I/O Functions 
 *
 * FUNCTIONS: system 
 *
 * ORIGINS: 3, 27 
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
 */

/*LINTLIBRARY*/
/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#endif
#include	<signal.h>
#include	<sys/types.h>
#include	<unistd.h>	/* for access */
#include	<sys/wait.h>
#include        <errno.h>

#define USRSHELL	"/usr/bin/sh"
#define SINGLESHELL	"/sbin/sh"

int 	
system(const char *string)
{
	sigset_t old,new;
	int     status, w;
	pid_t   pid;
	void	(*oldint)();
	void	(*oldquit)();
	char   *shell, *argstr;
	extern  char _DYNAMIC[];	/* magic symbol defined by ld:
					 *      set to 0 for -non_shared
					 *      set to X for -call_shared
					 *      set to X for -shared
					 *
					 * where X is the (non-zero) location
					 * of the .dynamic section.
					 */

	if(string == NULL) {
	  if (access(USRSHELL, X_OK) && access(SINGLESHELL, X_OK))
	      return(0);
	  else
	      return(1);
	}

	sigemptyset( &new );
	sigemptyset( &old );
	sigaddset( &new, SIGCHLD);
	sigprocmask( SIG_BLOCK, &new, &old);
	oldint = signal(SIGINT, SIG_IGN);
	oldquit = signal(SIGQUIT, SIG_IGN);

	if ((pid = fork()) == -1) {
		/* fork() will have set errno */
		sigprocmask( SIG_SETMASK, &old, NULL);
		signal(SIGINT, oldint);
		signal(SIGQUIT, oldquit);
		return((string) ? -1 : 0);/* return quickly so as not to hide
					   any errno set by "fork" in the
					   normal case */
	}

	if (!pid) { /* child */
	        sigprocmask( SIG_SETMASK, &old, NULL);
		signal(SIGINT, oldint);
		signal(SIGQUIT, oldquit);
		/*
		 * If the terminal is in a trusted state then
		 * maintain trustedness by running the trusted shell.
		 * Also, if string is NULL, pass an empty one.
		 * This prevents an error message from being generated
		 * by "sh" if presented a NULL string after "-c".
		 * It assumes that "sh" will return 0 if presented
		 * with a command string that is empty.
		 */
		shell = (access(USRSHELL, X_OK)==0 && _DYNAMIC)
				? USRSHELL : SINGLESHELL;

		argstr = (string) ? string : "";

		(void) execl(shell, "sh", "-c", argstr, 0);

		_exit(127);	/* when "execl" fails... */
	}
	/* parent */

	do {
	    w = waitpid( pid, &status, 0);
	} while (w == -1 && errno == EINTR);

	sigprocmask( SIG_SETMASK, &old, NULL);
	signal(SIGINT, oldint);
	signal(SIGQUIT, oldquit);

	if (w == -1)
		return((string) ? -1 : 0);

	return((string) ? status : (status == 0));
}
