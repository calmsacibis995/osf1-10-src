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
static char	*sccsid = "@(#)$RCSfile: sigops.c,v $ $Revision: 4.2.8.2 $ (DEC) $Date: 1993/06/07 22:44:10 $";
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
 * FUNCTIONS: sigaddset, sigdelset, sigfillset, sigismember,
 *	      sigemptyset (POSIX)
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 * OBJECT CODE ONLY SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * sigops.c	1.10  com/lib/c/gen,3.1,9013 10/23/89 13:38:39
 */

/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#pragma weak sigaddset = __sigaddset
#pragma weak sigdelset = __sigdelset
#pragma weak sigemptyset = __sigemptyset
#pragma weak sigfillset = __sigfillset
#pragma weak sigismember = __sigismember
#endif
#include <signal.h>
#include <errno.h>
#include "ts_supp.h"

/*
 * NAME: sigemptyset		(POSIX)
 *
 * FUNCTION: clears all bits in signal set, i.e. all signals excluded
 *
 * NOTES: This subroutine does not affect a processes signal mask,
 *	  it just initializes the specified mask data structure for
 *	  use in subsequent calls such as sigprocmask() that WOULD
 *	  affect a processes signal mask.
 *
 * RETURN VALUES: 
 * 	always return successfully with a value of 0
 *	upon return the specifies signal set is cleared
 *
 */
int
sigemptyset(sigset_t *setp)
{
	*setp = 0;
	return (0);
}

/*
 * NAME: sigfillset		(POSIX)
 *
 * FUNCTION: sets all bits in signal set, i.e. ALL signals blocked
 *
 * NOTES: This subroutine does not affect a processes signal mask,
 *	  it just initializes the specified mask data structure for
 *	  use in subsequent calls such as sigprocmask() that WOULD
 *	  affect a processes signal mask.
 *
 * RETURN VALUES: 
 * 	always return successfully with a value of 0
 *	upon return all signals in the specifies signal set are set
 */
/* setp is the pointer to signal set structure to be set */
int sigfillset(sigset_t *setp)
{
	*setp = ~0;
	return (0);
}
/*
 * NAME: sigaddset		(POSIX)
 *
 * FUNCTION: adds a signal to the specified set of signals to block
 *
 * NOTES: This subroutine does not affect a processes signal mask,
 *	  it just initializes the specified mask data structure for
 *	  use in subsequent calls such as sigprocmask() that WOULD
 *	  affect a processes signal mask.
 *
 * RETURN VALUES: 
 * 	0 => successfully altered signal set
 *	-1 => failed, did not alter signal set 
 *	errno = EINVAL if a -1 is returned => invalid signal number
 *	errno is unchanged if a 0 is returned
 */
/* setp is the pointer to signal set that will be changed */
/* signo is the signal to be added to "*setp" signal set */
  
int sigaddset(sigset_t *setp, int signo)
{

	/* check for invalid signal numbers - POSIX requires this */
	if ((signo <= 0) || (signo > SIGMAX))
	{	TS_SETERR(EINVAL);
		return (-1);
	}
	/* set appropriate mask bit for valid signal number */
	SIGADDSET(*setp,signo);
	return (0);
}
/*
 * NAME: sigdelset		(POSIX)
 *
 * FUNCTION: deletes a signal from the specified set of signals to block
 *
 * NOTES: This subroutine does not affect a processes signal mask,
 *	  it just initializes the specified mask data structure for
 *	  use in subsequent calls such as sigprocmask() that WOULD
 *	  affect a processes signal mask.
 *
 * RETURN VALUES: 
 * 	0 => successfully deleted signal from the signal set
 *	-1 => failed, did not alter signal set 
 *	errno = EINVAL if a -1 is returned => invalid signal number
 *	errno is unchanged if a 0 is returned
 */
int sigdelset(sigset_t *setp, int signo)
{
	/* check for invalid signal numbers - POSIX requires this */
	if ((signo <= 0) || (signo > SIGMAX))
	{	TS_SETERR(EINVAL);
		return (-1);
	}
	/* clear appropriate mask bit for valid signal number */ 
	SIGDELSET(*setp,signo);
	return (0);
}
/*
 * NAME: sigismember		(POSIX)
 *
 * FUNCTION: tests to see if a signal is in the specified set of signals
 *
 * NOTES: This subroutine does not affect a processes signal mask,
 *	  it just initializes the specified mask data structure for
 *	  use in subsequent calls such as sigprocmask() that WOULD
 *	  affect a processes signal mask.
 *
 * RETURN VALUES: 
 * 	0 => signal is NOT in the signal set
 *	1 => signal IS in the signal set
 *     -1 => failed, invalid signal number could not be tested
 *	errno = EINVAL if a -1 is returned => invalid signal number
 *	errno is unchanged if a 0 is returned
 */ 
int sigismember(const sigset_t *setp, int signo)
{
	/* check for invalid signal number */
	if ( (signo <= 0) || (signo > SIGMAX) )
	{	TS_SETERR(EINVAL);
		return (-1);
	}
	/* test bit corresponding to signal number; 1 => set */
	else
		return (SIGISMEMBER(*setp,signo));
}
