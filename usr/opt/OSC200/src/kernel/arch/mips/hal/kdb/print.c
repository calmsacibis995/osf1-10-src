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
static char	*sccsid = "@(#)$RCSfile: print.c,v $ $Revision: 1.2 $ (DEC) $Date: 1992/01/15 01:12:50 $";
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
 * derived from print.c	2.1	(ULTRIX/OSF)	12/3/90";
 */

/*
 *
 *	UNIX debugger
 *
 */

#include <cpus.h>
#include <hal/kdb/defs.h>
#include <kern/task.h>
#include <kern/thread.h>
#include <sys/proc.h>
#include <kern/kdb_print.h>

extern	label_t	kdb_save;
extern	int	kdb_setjmp_val;

extern msg		BADMOD;

extern bkpt_t		bkpthead;


long		lastframe;
long		callpc;

char		*lp;
long		maxpos;
short		radix;

static int	mustbezero = 0;

REGLIST reglist [] = {
        /* name roffs   rkern */
        "zero", 0,      &mustbezero,
        "at",   R1,     &pcb.pcb_regs[PCB_AT],
        "v0",   R2,     &pcb.pcb_regs[PCB_V0],
        "v1",   R3,     &pcb.pcb_regs[PCB_V1],
        "a0",   R4,     &pcb.pcb_regs[PCB_A0],
        "a1",   R5,     &pcb.pcb_regs[PCB_A1],
        "a2",   R6,     &pcb.pcb_regs[PCB_A2],
        "a3",   R7,     &pcb.pcb_regs[PCB_A3],
        "t0",   R8,     &pcb.pcb_regs[PCB_T0],
        "t1",   R9,     &pcb.pcb_regs[PCB_T1],
        "t2",   R10,    &pcb.pcb_regs[PCB_T2],
        "t3",   R11,    &pcb.pcb_regs[PCB_T3],
        "t4",   R12,    &pcb.pcb_regs[PCB_T4],
        "t5",   R13,    &pcb.pcb_regs[PCB_T5],
        "t6",   R14,    &pcb.pcb_regs[PCB_T6],
        "t7",   R15,    &pcb.pcb_regs[PCB_T7],
        "s0",   R16,    &pcb.pcb_regs[PCB_S0],
        "s1",   R17,    &pcb.pcb_regs[PCB_S1],
        "s2",   R18,    &pcb.pcb_regs[PCB_S2],
        "s3",   R19,    &pcb.pcb_regs[PCB_S3],
        "s4",   R20,    &pcb.pcb_regs[PCB_S4],
        "s5",   R21,    &pcb.pcb_regs[PCB_S5],
        "s6",   R22,    &pcb.pcb_regs[PCB_S6],
        "s7",   R23,    &pcb.pcb_regs[PCB_S7],
        "t8",   R24,    &pcb.pcb_regs[PCB_T8],
        "t9",   R25,    &pcb.pcb_regs[PCB_T9],
        "k0",   R26,    &pcb.pcb_regs[PCB_K0],
        "k1",   R27,    &pcb.pcb_regs[PCB_K1],
        "gp",   R28,    &pcb.pcb_regs[PCB_GP],
        "sp",   R29,    &pcb.pcb_regs[PCB_SP],
        "s8",   R30,    &pcb.pcb_regs[PCB_FP],
        "ra",   R31,    &pcb.pcb_regs[PCB_RA],
/* special regs */
        "sr",   R_SR,   &pcb.pcb_regs[PCB_SR],
        "lo",   R_LO,   &pcb.pcb_regs[PCB_LO],
        "hi",   R_HI,   &pcb.pcb_regs[PCB_HI],
        "badv", R_BADV, &pcb.pcb_regs[PCB_BAD],
        "cs",   R_CS,   &pcb.pcb_regs[PCB_CS],
        "pc",   R_PC,   &pcb.pcb_regs[PCB_PC],
        0,0,0
};

short reglist_size = sizeof reglist / sizeof reglist[0];

REGLIST fp_reglist [] = {
        "f0",   FR0,    &pcb.pcb_fpregs[0],
        "f1",   FR1,    &pcb.pcb_fpregs[1],
        "f2",   FR2,    &pcb.pcb_fpregs[2],
        "f3",   FR3,    &pcb.pcb_fpregs[3],
        "f4",   FR4,    &pcb.pcb_fpregs[4],
        "f5",   FR5,    &pcb.pcb_fpregs[5],
        "f6",   FR6,    &pcb.pcb_fpregs[6],
        "f7",   FR7,    &pcb.pcb_fpregs[7],
        "f8",   FR8,    &pcb.pcb_fpregs[8],
        "f9",   FR9,    &pcb.pcb_fpregs[9],
        "f10",  FR10,   &pcb.pcb_fpregs[10],
        "f11",  FR11,   &pcb.pcb_fpregs[11],
        "f12",  FR12,   &pcb.pcb_fpregs[12],
        "f13",  FR13,   &pcb.pcb_fpregs[13],
        "f14",  FR14,   &pcb.pcb_fpregs[14],
        "f15",  FR15,   &pcb.pcb_fpregs[15],
        "f16",  FR16,   &pcb.pcb_fpregs[16],
        "f17",  FR17,   &pcb.pcb_fpregs[17],
        "f18",  FR18,   &pcb.pcb_fpregs[18],
        "f19",  FR19,   &pcb.pcb_fpregs[19],
        "f20",  FR20,   &pcb.pcb_fpregs[20],
        "f21",  FR21,   &pcb.pcb_fpregs[21],
        "f22",  FR22,   &pcb.pcb_fpregs[22],
        "f23",  FR23,   &pcb.pcb_fpregs[23],
        "f24",  FR24,   &pcb.pcb_fpregs[24],
        "f25",  FR25,   &pcb.pcb_fpregs[25],
        "f26",  FR26,   &pcb.pcb_fpregs[26],
        "f27",  FR27,   &pcb.pcb_fpregs[27],
        "f28",  FR28,   &pcb.pcb_fpregs[28],
        "f29",  FR29,   &pcb.pcb_fpregs[29],
        "f30",  FR30,   &pcb.pcb_fpregs[30],
        "f31",  FR31,   &pcb.pcb_fpregs[31],
/*
        "fpc_csr",      FR0,    &pcb.pcb_fpc_csr,
        "fpc_eir",      FR0,    &pcb.pcb_fpc_eir,
        "fpc_used",     FR0,    &pcb.pcb_ownedfp,
*/
        0,0,0
};
short fp_reglist_size = sizeof fp_reglist / sizeof fp_reglist[0];



char		lastc;

string_t		errflg;
short		signo;
short		kdbsigcode;


long		dot;
long		var[];
short		pid;
long		adrval;
short		adrflg;
long		cntval;
short		cntflg;

string_t	signals[] = {
	"",
	"hangup",
	"interrupt",
	"quit",
	"illegal instruction",
	"trace/BPT",
	"IOT",
	"EMT",
	"floating exception",
	"killed",
	"bus error",
	"memory fault",
	"bad system call",
	"broken pipe",
	"alarm call",
	"terminated",
	"signal 16",
	"stop (signal)",
	"stop (tty)",
	"continue (signal)",
	"child termination",
	"stop (tty input)",
	"stop (tty output)",
	"input available (signal)",
	"cpu timelimit",
	"file sizelimit",
	"signal 26",
	"signal 27",
	"signal 28",
	"signal 29",
	"signal 30",
	"signal 31",
};

/* general printing routines ($) */

printtrace(modif)
{
	short		i;
	register bkpt_t	bkptr;
	string_t	comptr;
	register struct nlist *sp;

	if ( cntflg==0 ) {
		cntval = -1;
	}

	switch (modif) {

	case 'p':
		/*
		 *	pid$p	sets process to pid
		 *	$p	sets process to the one that invoked
		 *		kdb
		 */
		{
			struct proc 	*p;
			extern struct proc *	pfind();

			extern void	kdbgetprocess();

			if (adrflg) {
				p = pfind(dot);
			} else {
				p = pfind(curpid);	/* pid at entry */
			}
			if (p) {
				printf("proc entry at 0x%X\n", p);
				kdbgetprocess(p, &curmap, &curpcb);
				var[varchk('m')] = (long) curmap;
				pid = p->p_pid;
				if (pid == curpid) {
					/*
					 * pcb for entry process is saved
					 * register set
					 */
					curpcb = &pcb;
				}
			} else {
				printf("no such process");
			}
		}
		break;

	KDB_PRINT_CASES


	case 'd':
		if (adrflg) {
			if (adrval<2 || adrval>16) {
				printf("must have 2 <= radix <= 16");
				break;
			}
			printf("radix=%d base ten",radix=adrval);
		}
		break;


	case 'w':
	case 'W':
		maxpos=(adrflg?adrval:MAXPOS);
		break;

	case 's':
	case 'S':
		maxoff=(adrflg?adrval:MAXOFF);
		break;

	case 'v':
		prints("variables\n");
		for ( i=0;i<=35;i++ ) {
			    if ( var[i] ) {
				printc((i<=9 ? '0' : 'a'-10) + i);
				printf(" = %X\n",var[i]);
			}
		}
		break;


	case 0:
	case '?':
		if ( pid ) {
			printf("pcs id = %d\n",pid);
		} else {
			prints("no process\n");
		}
		sigprint();
		flushbuf();

	case 'r':
	case 'R':
		printregs();
		return;

	case 'c':
	case 'C':
		mips_printtrace(modif) ;
		break;

	case 'n':
	case 'N':
		mips_printtrace('n') ;
		break;


		/*print externals*/
	case 'e':
	case 'E':
		print_external_symbols();
		break;

	case 'a':
	case 'A':
		error ("No algol 68 on MIPS neither");
		/*NOTREACHED*/

		/*print breakpoints*/
	case 'b':
	case 'B':
		printf("breakpoints\ncount%8tbkpt%24tcommand\n");
		for (bkptr=bkpthead; bkptr; bkptr=bkptr->nxtbkpt)
			if (bkptr->flag) {
				printf("%-8.8d",bkptr->count);
				psymoff(bkptr->loc,ISYM,"%24t");
				comptr=bkptr->comm;
				while ( *comptr ) {
					printc(*comptr++);
				}
			}
		break;

	default:
		error(BADMOD);
	}
}


printregs()
{
	register REGPTR	p;
	long		v;

	printf ("%s%6t%R %16t", reglist -> rname, 0);
	printc(EOR);
	for (p = &reglist[1]; p -> rname; p++) {
		v = *(int *)((int)p->rkern - (int)&pcb + (int)curpcb);
		printf("%s%6t%R %16t", p->rname, v);
		valpr(v,(p->roffs==R_PC?ISYM:DSYM));
		printc(EOR);
	}
	printpc();
}

kdbgetreg_val(regnum)
{
        register REGPTR p = &reglist[regnum];
        if (regnum == 0) return 0;
        return (*(int *)((int)p->rkern - (int)&pcb + (int)curpcb));
}

getreg(regnam)
{
	register REGPTR	p;
	register string_t	regptr;
	char	*olp;
	int	 first = 1;

	olp=lp;
	p = reglist;
again:
	for ( ; p->rname; p++ ) {
		regptr=p->rname;
		if ( (regnam == *regptr++) ) {
			while ( *regptr ) {
				if (readchar() != *regptr++) {
					--regptr;
					break;
				}
			}
			if ( *regptr ) {
				lp=olp;
			} else {
				return (int)p->rkern - (int)&pcb + (int)curpcb;
			}
		}
	}
	lp=olp;
	if (first--) {
		p = fp_reglist;
		goto again;
	}
	return(0);
}

printpc()
{
	psymoff(dot,ISYM,":%16t");
	printins(ISP,chkget(dot,ISP), dot, 1);
	printc(EOR);
}

char	*illinames[] = {
	"reserved addressing fault",
	"privileged instruction fault",
	"reserved operand fault"
};

char	*fpenames[] = {
	0,
	"integer overflow trap",
	"integer divide by zero trap",
	"floating overflow trap",
	"floating/decimal divide by zero trap",
	"floating underflow trap",
	"decimal overflow trap",
	"subscript out of range trap",
	"floating overflow fault",
	"floating divide by zero fault",
	"floating undeflow fault"
};

sigprint()
{
	if ( (signo>=0) && (signo<sizeof signals/sizeof signals[0]) ) {
		prints(signals[signo]);
	}
	switch (signo) {

	case SIGFPE:
		if ( (kdbsigcode > 0 &&
		    kdbsigcode < sizeof fpenames / sizeof fpenames[0]) ) {
			prints(" (");
			prints(fpenames[kdbsigcode]);
			prints(")");
		}
		break;

	case SIGILL:
		if ( (kdbsigcode >= 0 &&
		    kdbsigcode < sizeof illinames / sizeof illinames[0]) ) {
			prints(" (");
			prints(illinames[kdbsigcode]);
			prints(")");
		}
		break;
	}
}

print_stack_trace(thread)
	thread_t	thread;
{
	label_t		save_kdb_save;
	struct pcb	*savecurpcb = curpcb;
	int		saveadrflg = adrflg;

	save_kdb_save = kdb_save;

	if (thread != current_thread())
		curpcb = thread->pcb;

	printf("thread 0x%X,  pcb 0x%X, ", thread, curpcb);
	if (!setexit()) {
		printf("W_EVENT ");
		psymoff((long)thread->wait_event, ISYM, "");
		printc(EOR);
		adrflg = 0;
		mips_printtrace('k');
	} else {
		errflg = 0;
		printf(" FAILED\n");
	}
	adrflg = saveadrflg;

	curpcb = savecurpcb;
	kdb_save = save_kdb_save;
}
