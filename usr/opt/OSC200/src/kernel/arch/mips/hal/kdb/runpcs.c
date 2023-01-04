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
static char	*sccsid = "@(#)$RCSfile: runpcs.c,v $ $Revision: 1.2 $ (DEC) $Date: 1992/01/15 01:12:56 $";
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
 * derived from runpcs.c	2.1	(ULTRIX/OSF)	12/3/90";
 */

#define DEBUG 0

/*
 *
 *	UNIX debugger
 *
 */


#include <hal/kdb/defs.h>

extern	label_t	kdb_save;
extern	int	kdb_setjmp_val;

char		*lp;

extern msg		SZBKPT;

/* breakpoints */
extern bkpt_t		bkpthead;

extern REGLIST		reglist[];

char		lastc;

string_t	errflg;
short		signo;
short		kdbsstep;

long		dot;
short		pid;
long		expv;
short		adrflg;
long		loopcnt;

#include <hal/kdb/pcs.h>


#define SETBP(ins)      (0x0001000D)

/* service routines for sub process control */

getsig(sig)
{
	return(expr(0) ? expv : sig);
}

long userpc = 1;

runpcs(runmode,execsig)
{
	register bkpt_t	bp;
	if ( adrflg ) {
		userpc=dot;
	}
	if (execsig == 0)
		printf("kernel: running\n");
#if	DEBUG
	printf("\ncontinue %R %d %d\n",userpc,execsig,runmode);
#endif
	if ( runmode==SINGLE ) {
		delbp();
	} else { /* continuing from a breakpoint is hard */
		if ( bp=scanbkpt(userpc) ) {
			execbkpt(bp,execsig);
			execsig=0;
		}
		setbp();
	}
#if	DEBUG
	printf("reset(%d) from runpcs()\n", runmode);
#endif	DEBUG
	reset(runmode);
}

short execbkptf = 0;

	/*
	 * determines whether to stop, and what to print if so
	 * flag:	1 if entered by trace trap
	 * execsig:	(seems not to be used by kernel debugger)
	 *
	 * exits:
	 *	skipping breakpoint (execbkptf != 0):
	 *		runpcs(CONTIN)
	 *      next iteration of breakpoint:
	 *		runpcs(CONTIN)
	 *	next iteration of single-step:
	 *		runpcs(SINGLE)
	 *
	 *	stopped by breakpoint:
	 *		returns 1
	 *	stopped by single-step, or
	 *		by CALL/RET:
	 *		returns 0
	 *
	 *	normal return MUST reset sstepmode!
	 */

int kdbtryme = 0;

nextpcs(flag, execsig)
{
	short		rc;
	register bkpt_t	bp;

	kdbsstep_rm();

	signo = flag?SIGTRAP:0;
	delbp();
	if (execbkptf) {
		execbkptf = 0;
		runpcs(CONTIN, 1);
	}

	if ( (signo==0) && (bp=scanbkpt(userpc)) ) {
	     /* stopped by BPT instruction */
#if	DEBUG
		printf("\n BPT code; '%s'%o'%o'%d\n",
		bp->comm,bp->comm[0],EOR,bp->flag);
#endif
		dot=bp->loc;
		if ( bp->flag==BKPTEXEC
		    || ((bp->flag=BKPTEXEC)
		    && bp->comm[0]!=EOR
		    && command(bp->comm,':')
		    && --bp->count) ) {
			loopcnt++;
			execbkpt(bp,execsig);
			execsig=0;
		} else {
			bp->count=bp->initcnt;
			rc=1;
		}
	} else {
		execsig=signo;
		rc=0;
	}
	if (sstepmode == STEP_OVER){
		/*
		 *	Step over function calls
		 *
		 *	If the last instruction wasnt a funcall
		 *	behave just like STEP_NONE.
		 *	If it was, set a breakpoint
		 *	at the return address and continue.
		 */
		extern long lastpc;
		int	ins = chkget(lastpc,ISP);

		if (isa_call(ins)) {
#ifdef orig_broken_code
			long ra = kdbgetreg_val(31);
#else
			long ra = lastpc + 8;
#endif
			register bkpt_t bkptr;

			/* there might already be a bkpt there */
			if ((bkptr=scanbkpt(ra)) == 0) {
				bkptr = kdbbkpt_alloc(ra);
				bkptr->flag = BKPTSSTEP;
				bkptr->ins = get(ra, ISP);
				bkptr->comm[0] = 0;
				put( ra, ISP, SETBP(bkptr->ins));
				/* kdbsstep_insert_bkpt(bkptr,ra); */
			}
			/* Our task is completed, turn into a regular ":s" */
			sstepmode = STEP_NONE;
			if (kdbtryme) {
				loopcnt--;
				subpcs('c');
			}
			loopcnt++,rc=1;
		} else
			lastpc = userpc;
	}
	if (--loopcnt > 0) {
		if (sstepmode == STEP_PRINT){
			printf("%16t");
			printpc();
		}
		runpcs(rc?CONTIN:SINGLE, 1);
	}
	if (sstepmode == STEP_RETURN) {
		/*
		 *	Keep going until matching return
		 */

		int	ins = chkget(dot, ISP);

		if (isa_rei(ins) || (isa_ret(ins) && (--call_depth == 0)))
			printf("%d instructions executed\n", icount);
		else {
			if (isa_call(ins) || isa_ret(ins)) {
				register int i;

				printf("[after %6d]     ", icount);
				for (i = call_depth; --i > 0;)
					printf("  ");
				printpc();
			}
			if (isa_call(ins))
				call_depth++;

			loopcnt++;
			icount++;
			runpcs(SINGLE, 1);
		}
	}
	if (sstepmode == STEP_CALLT){
		/*
		 *	Keep going until CALL or RETURN
		 */

		int	ins = chkget(dot,ISP);

		if (isa_call(ins) || isa_ret(ins) || isa_rei(ins))
			printf("%d instructions executed\n", icount);
		else {
			loopcnt++;
			icount++;
			runpcs(SINGLE, 1);
		}
	}
	sstepmode = STEP_NONE;	/* don't wait for CALL/RET */
	return(rc);
}

#define BPOUT 0
#define BPIN 1
short bpstate = BPOUT;

execbkpt(bp,execsig)
bkpt_t	bp;
{
#if	DEBUG
	printf("exbkpt: %d\n",bp->count);
#endif
	delbp();
	bp->flag=BKPTSET;
	execbkptf++;
#if	DEBUG
	printf("reset(%d) from execbkpt()\n", SINGLE);
#endif	DEBUG
	reset(SINGLE);
}

isa_break(ins)
{
	return (ins == SETBP(ins));
}

bkpt_t	scanbkpt(adr)
long adr;
{
	register bkpt_t	bp;
	for ( bp=bkpthead; bp; bp=bp->nxtbkpt ) {
		if ( bp->flag && bp->loc==adr ) {
			break;
		}
	}
	return(bp);
}


delbp()
{
	register long	a;
	register bkpt_t	bp;

	if ( bpstate!=BPOUT ) {
		for ( bp=bkpthead; bp; bp=bp->nxtbkpt ) {
			if ( bp->flag & (BKPTSET|BKPTEXEC) ) {
				a=bp->loc;
#if	DEBUG
				printf("[-bp@%R]\n", a);
#endif	DEBUG
				put(a, ISP, bp->ins);
			}
		}
		bpstate=BPOUT;
	}
}

setbp()
{
	register long		a;
	register bkpt_t	bp;

	if ( bpstate!=BPIN ) {
		for ( bp=bkpthead; bp; bp=bp->nxtbkpt ) {
			if ( bp->flag & (BKPTSET|BKPTEXEC) ) {
				a = bp->loc;
#if	DEBUG
			printf("[+bp@%R]\n", a);
#endif	DEBUG
				bp->ins = get(a, ISP);
				put(a, ISP, SETBP(bp->ins));
			}
		}
		bpstate=BPIN;
	}
}


kdbsstep_set()
{
	register long inst, brpc, pc;
	register bkpt_t	bkptr;

#if	DEBUG
	printf("sstep:S");
#endif	DEBUG
	kdbsstep = 1;
	/*
	 * assume user just executed at "userpc" [might not be true?]
	 */
	inst = get(pc=userpc, ISP);
	if (isa_branch(inst)) {
		brpc = branch_target(inst, pc, 0);
		if (brpc != pc) {	/* self-branches are hopeless */
			if ( (bkptr=scanbkpt(brpc)) == 0) {
				bkptr = kdbbkpt_alloc(brpc);
				bkptr->flag = BKPTSSTEP;
				bkptr->ins = get(brpc, ISP);
#if	DEBUG
				printf("[+bp@%R]", brpc);
#endif	DEBUG
				put( brpc, ISP, SETBP(bkptr->ins));
			}
		}
		pc += 4;
	}
	pc += 4;
	if ( (bkptr=scanbkpt(pc)) == 0) {
		bkptr = kdbbkpt_alloc(pc);
		bkptr->flag = BKPTSSTEP;
		bkptr->ins = get(pc, ISP);
#if	DEBUG
		printf("[+bp@%R]", pc);
#endif	DEBUG
		put( pc, ISP, SETBP(bkptr->ins));
	}
#if	DEBUG
	printf("\n");
#endif	DEBUG
}

kdbsstep_rm()
{
	register long	a;
	register bkpt_t	bp;

#if	DEBUG
	printf("sstep:R");
#endif	DEBUG
	kdbsstep = 0;
	for ( bp=bkpthead; bp; bp=bp->nxtbkpt ) {
		if ( bp->flag&BKPTSSTEP ) {
			if ((bp->flag &= ~BKPTSSTEP) == 0) {
				a = bp->loc;
#if	DEBUG
				printf("[-bp@%R]", a);
#endif	DEBUG
				put(a, ISP, bp->ins);
			}
		}
	}
#if	DEBUG
	printf("\n");
#endif	DEBUG
}


bkpt_t
kdbbkpt_alloc(loc)
long loc;
{
	register bkpt_t bkptr;

	for ( bkptr=bkpthead; bkptr; bkptr=bkptr->nxtbkpt )
		if ( bkptr->flag == 0 )
			break;

	if ( bkptr==0 )
		if ( (bkptr=(bkpt_t)sbrk(sizeof *bkptr)) == (bkpt_t)-1 )
			error(SZBKPT);
		else {
			bkptr->nxtbkpt=bkpthead;
			bkpthead=bkptr;
		}

	bkptr->loc = loc;
	return bkptr;
}
