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
static char	*sccsid = "@(#)$RCSfile: signal.c,v $ $Revision: 4.3.5.2 $ (DEC) $Date: 1993/06/07 22:44:02 $";
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
#if !defined(lint) && !defined(_NOIDENT)

#endif
/*
 * COMPONENT_NAME: (LIBCGEN) Standard C Library General Functions
 *
 * FUNCTION: signal
 *
 * ORIGINS: 3 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 * OBJECT CODE ONLY SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1990 
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
 * signal.c	1.20  com/lib/c/gen,3.1,9013 3/8/90 13:52:45
 */

/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#endif
#include <signal.h>
#include <errno.h>

/*
 * NOTES: This file is included by the _signal.c file with __SIGNAL defined
 *	to create _signal().  See _signal.c for more details.
 */


/*
 *
 * FUNCTION: sets action for a signal, compatibility interface to sigaction()
 *
 * NOTES: signal() allows only a subset of the function of sigaction().
 *	  It is provided for compatibility with old source code, and
 *	  for comformance to POSIX and ANSI "C" standards.  
 *
 * RETURN VALUES: 
 *	SIGERR         => failed, errno is set to specify the cause of failure
 *      errno = EINVAL => invalid signal number
 *	      = EINVAL => attempt to ignore or catch SIGKILL,SIGSTOP,SIGCONT
 *	Any other return value implies success, and the return value is
 *	a the previous signal action handler value (see sigaction()).
 */

void
#ifdef __STDC__
#ifdef __SIGNAL
(*_signal(int signo, void (*fun)(int)))(int)
#else
(*signal(int signo, void (*fun)(int)))(int)
#endif
#else /* not ansi c */
#ifdef __SIGNAL
(*_signal(signo, fun))()
#else
(*signal(signo, fun))()
int signo;
void (*fun)(int);
#endif
#endif

{
	struct sigaction act;	/* new signal action structure */
	struct sigaction oact;  /* returned signal action structure */ 

	/*   Setup a sigaction struct */

 	act.sa_handler = fun;        /* Handler is function passed */
	sigemptyset(&(act.sa_mask)); /* No signal to mask while in handler */
	act.sa_flags = 0;
	
	/* use the sigaction() system call to set new and get old action */

	if(sigaction(signo, &act, &oact))
		/* If sigaction failed return SIG_ERR */
	    return(SIG_ERR);
	else
        	/* use the previous signal handler as a return value */
	    return(oact.sa_handler);
}
