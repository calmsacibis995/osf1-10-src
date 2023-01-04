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
static char	*sccsid = "@(#)$RCSfile: setsig.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 02:43:13 $";
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

/*
 * COMPONENT_NAME: (LIBPW) Programmers Workbench Library
 *
 * FUNCTIONS: setsig, setsig1
 *
 * ORIGINS: 3 27
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
 * Copyright (c) 1984 AT&T	
 * All Rights Reserved  
 *
 * THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	
 * The copyright notice above does not evidence any   
 * actual or intended publication of such source code.
 *
 */
 
# include	"signal.h"
# include	"sys/types.h"
# include	"macros.h"

/*
	General-purpose signal setting routine.
	All non-ignored, non-caught signals are caught.
	If a signal other than hangup, interrupt, or quit is caught,
	a "user-oriented" message is printed on file descriptor 2 with
	a number for help(I).
	If hangup, interrupt or quit is caught, that signal	
	is set to ignore.
	Termination is like that of "fatal",
	via "clean_up(sig)" (sig is the signal number)
	and "exit(userexit(1))".
 
	If the file "dump.core" exists in the current directory
	the function commits suicide to produce a core dump
	(after calling clean_up, but before calling userexit).
*/


char    *Mesg[]={
	0,
	0,	/* Hangup */
	0,	/* Interrupt */
	0,	/* Quit */
	"Illegal instruction",
	"Trace/BPT trap",
	"IOT trap",
	"Danger",
	"Arithmetic exception",
	"Killed",
	"Bus error",
	"Memory fault",
	"Bad system call",
	"Broken pipe",
	"Alarm clock",
	"Terminated"
};

#define NUMSIGS sizeof Mesg/sizeof *Mesg

setsig()
{
	extern int setsig1(int);
	register int signo;
	register void (* func)(int);

	for (signo = 1; signo < NUMSIGS; signo++) {
		if ((func = signal(signo, SIG_IGN)) == SIG_DFL)
			func = (void (*)(int))setsig1;
		signal(signo,(void (*)(int))func);
	}
}


static char preface[]="SIGNAL: ";
static char endmsg[]=" (ut12)\n";

setsig1(int sig)
{
	register char *p;

	if (sig < NUMSIGS && (p = Mesg[sig])) {
		write(2,preface,sizeof(preface)-1);
		write(2,p,length(p));
		write(2,endmsg,sizeof(endmsg)-1);
	}
	else
		signal(sig,SIG_IGN);
	clean_up(sig);
	if(open("dump.core",0) > 0) {
		signal(SIGIOT,SIG_DFL);
		abort();
	}
	exit(userexit(1));
}
