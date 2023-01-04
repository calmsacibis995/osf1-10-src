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
static char	*sccsid = "@(#)$RCSfile: ssignal.c,v $ $Revision: 4.3.5.2 $ (DEC) $Date: 1993/06/07 22:52:05 $";
#endif 
/*
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 */
/*
 * OSF/1 1.2
 */
#if !defined(lint) && !defined(_NOIDENT)
#endif
/*
 * COMPONENT_NAME: (LIBCGEN) Standard C Library General Functions
 *
 * FUNCTIONS: ssignal, gsignal
 *
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
 * ssignal.c	1.10  com/lib/c/gen,3.1,8943 10/24/89 10:31:08
 */

/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#if defined(_THREAD_SAFE)
#pragma weak gsignal_r = __gsignal_r
#pragma weak ssignal_r = __ssignal_r
#endif
#if !defined(_THREAD_SAFE)
#pragma weak gsignal = __gsignal
#pragma weak ssignal = __ssignal
#endif
#endif
#include <signal.h>		/* for SIG_DFL */

/* Highest allowable user signal number */
#define MAXSIG 16

/* Lowest allowable signal number (lowest user number is always 1) */
#define MINSIG (-4)

#ifndef _THREAD_SAFE
/* Table of signal values */
static void (*sigs[MAXSIG-MINSIG+1])(int);
#endif

/*
 * NAME:	ssignal
 *                                                                    
 * FUNCTION:	set software signal
 *                                                                    
 * NOTES:	Ssignal sets a software signal.  The signal can
 *		either be set to SIG_DFL, SIG_IGN or can be set
 *		to call a function when the signal is raised.
 *
 * DATA STRUCTURES:	'sigs' is modified
 *
 * RETURN VALUE DESCRIPTION:	The previous value of the signal
 *		is returned.
 */  

void
#ifdef _THREAD_SAFE
(*ssignal_r(int sig, void (*fn)(int), void (*sigs[])(int)))(int)
#else
(*ssignal(int sig, void (*fn)(int)))(int)
#endif
{
	void (*savefn)(int);	/* previous value of 'sig'	*/

	if (sig >= MINSIG && sig <= MAXSIG) {
		savefn = sigs[sig-MINSIG];
		sigs[sig-MINSIG] = fn;
	} else
		savefn = SIG_DFL;

	return(savefn);
}

/*
 * NAME:	gsignal
 *                                                                    
 * FUNCTION:	gsignal - raise a software signal.
 *                                                                    
 * NOTES:	Gsignal 'raises' a software signal.  In effect, the
 *		software signal is then processed.  If the value of
 *		signal is currently a function, the function is called.
 *
 * RETURN VALUE DESCRIPTION:	0 if the value of the function is SIG_DFL,
 *		1 if the value is SIG_IGN, else call the function and
 *		return the value returned by the function.
 */  

int
#ifdef _THREAD_SAFE
gsignal_r(int sig, void (*sigs[])(int))
#else
gsignal(sig)
int sig;
#endif
{
	int (*sigfn)(int);

	if (sig < MINSIG || sig > MAXSIG
	 || (sigfn = (int (*)(int))sigs[sig-MINSIG]) == (int (*)(int))SIG_DFL)
		return(0);
	else if (sigfn == (int (*)(int))SIG_IGN)
		return(1);
	else {
		sigs[sig-MINSIG] = SIG_DFL;
		return((*sigfn)(sig));
	}
}
