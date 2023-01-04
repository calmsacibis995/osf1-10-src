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
static char	*sccsid = "@(#)$RCSfile: pcs.c,v $ $Revision: 1.2 $ (DEC) $Date: 1992/01/15 01:12:39 $";
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
 * derived from pcs.c	2.1	(ULTRIX/OSF)	12/3/90";
 */

/*
 *
 *	UNIX debugger
 *
 */

#include <hal/kdb/defs.h>


extern msg		NOBKPT;
extern msg		EXBKPT;
extern msg		BADMOD;

extern long		userpc;

/* breakpoints */
bkpt_t		bkpthead;

char		*lp;
char		lastc;

short		signo;
long		dot;
short		pid;
long		cntval;
long		loopcnt;

short		adrflg;

long		lastpc;

#include <hal/kdb/pcs.h>

extern int kdb_kill(), kdb_kill_nosync(), kdb_reboot(), kdb_dump();

/* sub process control */

subpcs(modif)
{
	register short		check;
	short		execsig,runmode;
	register bkpt_t	bkptr;
	string_t		comptr;
	long *regptr;
	int	s;

	execsig=0;
	loopcnt=cntval;

	switch (modif) {

		/* delete breakpoint */
	case 'd':
	case 'D':
		if ( (bkptr=scanbkpt(dot)) ) {
			bkptr->flag = (bkptr->flag&BKPTSSTEP) ? BKPTSSTEP : 0;
			return;
		} else {
			error(NOBKPT);
		}

		/* set breakpoint */
	case 'b':
	case 'B':
		if ( bkptr=scanbkpt(dot) )
			bkptr->flag=0;
		bkptr = kdbbkpt_alloc(dot);
		bkptr->initcnt = bkptr->count = cntval;
		bkptr->flag = BKPTSET;
		check=MAXCOM-1;
		comptr=bkptr->comm;
		rdc();
		lp--;
		do {
			*comptr++ = readchar();
		}
		while( check-- && lastc!=EOR ) ;
		*comptr=0;
		lp--;
		if ( check ) {
			return;
		} else {
			error(EXBKPT);
		}


		/* single step */
	case 's':
		/* same, but step over function calls */
	case 'S':
		runmode=SINGLE;
		execsig=getsig(signo);
		sstepmode = isupper(modif) ? STEP_OVER : STEP_NORM;
		lastpc = userpc;
		break;

	case 'p':
	case 'P':

		runmode = SINGLE;
		execsig = getsig(signo);
		sstepmode = STEP_PRINT;
		break;

	case 'j':
	case 'J':

		runmode = SINGLE;
		execsig = getsig(signo);
		sstepmode = isupper(modif) ? STEP_RETURN : STEP_CALLT;
		call_depth = 1;
		icount = 0;
		break;

		/* Kill the debugged process (i.e., the kernel) */
	case 'k':
        case 'K': /* No sync */
		runmode=CONTIN;
		execsig=getsig(signo);
		sstepmode = STEP_NONE;
		if (islower(modif))
                        pcb.pcb_regs[PCB_PC] = (long)kdb_kill;
                else
			pcb.pcb_regs[PCB_PC] = (long)kdb_kill_nosync;
		break;

		/* Run the debugged process, (i.e. the kernel) aka reboot */
	case 'r':
		runmode=CONTIN;
		execsig=getsig(signo);
		sstepmode = STEP_NONE;
		pcb.pcb_regs[PCB_PC] = (long)kdb_reboot;
		break;

		/* continue with optional signal */
	case 'c':
	case 'C':
		runmode=CONTIN;
		execsig=getsig(signo);
		sstepmode = STEP_NONE;
		break;

		/* z: force a dump and halt the system */
	case 'z':
		runmode=CONTIN;
		execsig=getsig(signo);
		sstepmode = STEP_NONE;
		pcb.pcb_regs[PCB_PC] = (long)kdb_dump;
		break;

		 /* Z: force a dump and return to kdb */
	case 'Z':
		s = splhigh();
		save_context_all();
		dumpsys();
		splx(s);
		break;

	default:
		error(BADMOD);
	}

	if ( loopcnt>0 ) {
		runpcs(runmode,0);
	}
}

#include <sys/reboot.h>
kdb_kill()
{
	boot(RB_BOOT,RB_HALT);
	/* NOTREACHED */
}

kdb_reboot()
{
	boot(RB_BOOT,0);
	/* NOTREACHED */
}

kdb_kill_nosync()
{
	boot(RB_BOOT,RB_HALT|RB_NOSYNC);
	/* NOTREACHED */
}

kdb_dump()
{
	boot(RB_PANIC,RB_NOSYNC);
	/* NOTREACHED */
}
