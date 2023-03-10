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
static char	*sccsid = "@(#)$RCSfile: psignal.c,v $ $Revision: 4.2.5.2 $ (DEC) $Date: 1993/06/07 23:36:20 $";
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
 * FUNCTIONS: psignal
 *
 * ORIGINS: 26 27
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
 *
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 * psignal.c	1.6  com/lib/c/gen,3.1,8943 10/16/89 08:52:42
 */

/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#if !defined(_THREAD_SAFE)
#pragma weak psignal = __psignal
#endif
#endif
#include <signal.h>

/*
 * FUNCTION: Print the name of the signal indicated along with the supplied message.
 *
 */
/*LINTLIBRARY*/


extern	char *sys_siglist[];

#ifdef MSG
#include "libc_msg.h"
#endif

psignal(sig, s)
	unsigned sig;
	char *s;
{
	register char *c;
	register n;

#ifdef MSG
	if (sig < SIGMAX+1)
		c = NLgetamsg( MF_LIBC, MS_LIBC, (int)(M_PSIGNAL + sig),
			sys_siglist[sig] );
	else
		c = NLgetamsg( MF_LIBC, MS_LIBC, M_PSIGNAL + NSIG,
		 	"Unknown signal" );
#else
	if (sig < SIGMAX+1)
		c = sys_siglist[sig];
	else
		c = "Unknown signal";
#endif
	n = strlen(s);
	if (n) {
		write(2, s, n);
		write(2, ": ", 2);
	}
	write(2, c, strlen(c));
	write(2, "\n", 1);
}
