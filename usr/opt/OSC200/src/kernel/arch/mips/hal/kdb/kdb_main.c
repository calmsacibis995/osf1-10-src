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
static char	*sccsid = "@(#)$RCSfile: kdb_main.c,v $ $Revision: 1.2 $ (DEC) $Date: 1992/01/15 01:11:49 $";
#endif 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * Mach Operating System
 * Copyright (c) 1989 Carnegie-Mellon University
 * All rights reserved.  The CMU software License Agreement specifies
 * the terms and conditions for use and redistribution.
 */
/*
 * OSF/1 Release 1.0
 */
/* 
 * derived from kdb_main.c	2.1	(ULTRIX/OSF)	12/3/90";
 */

#define DEBUG 0

/*
 * adb - main command loop and error/interrupt handling
 */

#include <hal/kdb/defs.h>
#include <sys/proc.h>


/*
 * KERNEL setexit/reset become setjmp/longjmp of
 * the label below.
 */
label_t	kdb_save;
int	kdb_setjmp_val;

extern msg		NOEOR;

short		executing;
char		*lp;
string_t	errflg;

char		lastc;
short		eof;
short		lastcom;
extern	short	doreinput;
extern	short	kdbsstep;

unsigned long	maxoff = MAXOFF;
long	maxpos = MAXPOS;
long	dot;

kdb(type, trapsp, cur_thread)
int		type;
int		*trapsp;
thread_t	cur_thread;
{
	extern long userpc;
	extern short pid;

	userpc = dot = pcb.pcb_regs[PCB_PC];
	if (cur_thread) {
		register task_t	cur_task;
		register int	proc_index;
		/*
		 *	Get the map for the current thread
		 */
		cur_task = cur_thread->task;
		curmap = cur_task->map;
		proc_index = cur_task->proc_index;
		if (proc_index != 0) {
		    curpid = proc[proc_index].p_pid;
		}
		else {
		    curpid = 1;	/* fake it */
		}
		var[varchk('m')] = (long) curmap;
	} else {
		/*
		 *	if there's no process...
		 */
		curmap = NULL;	/* take our chances */
		curpid = 1;	/* fake */
	}
	/*
	 *	But the pcb is the saved set of registers
	 */
	curpcb = &pcb;
	pid    = curpid;
	var[varchk('t')] = (int)trapsp;

	flushbuf();
#if	DEBUG
	{
		extern long loopcnt;
		printf("loop=%d\n", loopcnt);
	}
#endif	DEBUG
	switch (setexit())
	{
	case SINGLE:
#if	DEBUG
		printf("STEP=>\n");
#endif	DEBUG
		kdbsstep_set();
		doreinput = 1;
		return(1);

	case CONTIN:
#if	DEBUG
		printf("CONT=>\n");
#endif	DEBUG
		doreinput = 0;
		lastcom = 0;
		return(1);
	case 0:
#if	DEBUG
		printf("nextpcs(%d)\n", type);
#endif	DEBUG
		if ( nextpcs(type, 0) ) {
			/* stopped by breakpoint halt */
			printf("breakpoint%16t");
		} else {
			/* stopped by single step or CALL/RET */
			/* (also keyboard interrupt */
			printf("stopped at%16t");
		}
		printpc();
	}
	if (executing)
		delbp();
	executing = 0;
	for (;;) {
		extern int sstepmode;

		sstepmode = -1;
		flushbuf();
		if (errflg) {
			printf("%s\n", errflg);
			errflg = 0;
		}
		lp=0;
		rdc();
		lp--;
		if (eof)
			return(1);

		command(0, lastcom);
		if (lp && lastc!='\n')
			error(NOEOR);
	}
}


long
round(a,b)
register long a, b;
{
	register long w;
	w = (a/b)*b;
	if ( a!=w ) {
		w += b;
	}
	return(w);
}

/*
 * If there has been an error or a fault, take the error.
 */
chkerr()
{
	if (errflg)
		error(errflg);
}

/*
 * An error occurred; save the message for later printing,
 * close open files, and reset to main command loop.
 */
error(n)
char *n;
{
	errflg = n;
	doreinput = 0;
	lastcom = 0;
	reset(EXIT);
}
