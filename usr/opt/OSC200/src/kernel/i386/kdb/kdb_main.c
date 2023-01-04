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
static char	*sccsid = "@(#)$RCSfile: kdb_main.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:11:07 $";
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
 * Copyright (c) 1988 Carnegie-Mellon University
 * Copyright (c) 1987 Carnegie-Mellon University
 * All rights reserved.  The CMU software License Agreement specifies
 * the terms and conditions for use and redistribution.
 */
/*
 * OSF/1 Release 1.0
 */

/*
 * adb - main command loop and error/interrupt handling
 */
#include <mouse.h>
#include <i386/kdb/defs.h>
#include <i386/kdb/pcs.h>
#include <mach/boolean.h>

#include <sys/proc.h>
#include <mach/i386/thread_status.h>

msg		NOEOR;

INT		mkfault;
INT		executing;
INT		infile;
char		*lp;
long		maxoff;
long		maxpos;
long		sigint;
long		sigqit;
INT		wtflag;
string_t		errflg;
long		exitflg;

char		lastc;
INT		eof;

INT		lastcom;
long	maxoff = MAXOFF;
long	maxpos = MAXPOS;

/* 
 * This flag is used to tell the trap handler that kdb is responsible 
 * for the single-step trap in system mode.
 */
boolean_t kdb_singlestep = FALSE;
int kdbtrapok = 0;
int kdbreturn = 1;
int kdbactive = 0;
int step_thru_bpt = 0;
int runmode;
int kdbtrapflag = 0;
extern	short	doreinput;

long	dot;
int	jmpbuf[6];		/* should be label_t, later */
int	tregs[20];		/* reg holder if kdb called directly */
int	foreign_regs[20];	/* used by getprocess */
int	*active_regs;
int	*kdb_regs;		/* ptr to registers on stack */

/*
 * kdb_nofault - dismiss an addressing fault to allow kdb to continue.
 */

kdb_nofault()
{
	kdberror("faulted within kdb -- continuing");
}


/*
 * Kdb entry point. Be careful calling this directly. It doesn't like
 * having a NULL trapsp. If you need kdb use Bpt().
 */

kdb(type, trapsp, flag)
        int *trapsp;
{
        extern int mouse_in_use;
        
        /* Don't invoke kdb recusively */
        if (kdbactive) {
                switch (type) {
                case T_PGFLT:
                case T_SEGNPFLT:
                        trapsp[EIP] = (int)kdb_nofault;
                        return(flag);
                }
        }
        
        kdbtrapflag = flag;
	kdbactive++;

#if	NMOUSE > 0
	if (mouse_in_use) {
                cnpollc(1);
		kdb_main(type, trapsp);
                cnpollc(0);
	} else
#endif
                kdb_main(type, trapsp);

	kdbactive--;
        return(flag);
}

kdb_main(type, regs)
int		type;
int		*regs;
{
	int dr6;
	extern char peekc;
	extern int dotinc;
	int s;
	extern INT pid;
	struct proc *curproc;
	extern long 	loopcnt;

	s = sploff();		/* ?? rvb */
	/*
	 * Dismiss break point instruction execution via stepping
	 */
	if (step_thru_bpt) {
		step_thru_bpt = 0;
		regs[EFL] &= ~EFL_TF;
		goto bptexit;
	}
	/*
	 * Just keep doing what we're into
	 */
	if (--loopcnt > 0) {
		if (sstepmode == STEP_PRINT){
			printf("%2d:", loopcnt);
			kdb_regs = regs;	/* needed by printpc */
			printpc();
		}
		goto exit;
	}
	/*
	 * Hairy stepping mode
	 */
	if (type == T_SGLSTP && ((dr6 = _dr6()) & DBG_BS)) {
	    if (sstepmode == STEP_RETURN) {
		/*
		 *	Keep going until matching return
		 */
		int	ins1 = chkget(regs[EIP]);
		int	ins = ins1 & 0xff;

		if (ins == 0xff) {
			ins1 &= 0x3800;
			if (ins1 == 0x1000 || ins1 == 0x1800) {
				ins = I_CALL;
			}
		}

		if ((ins == I_REI) || ((ins == I_RET) && (--call_depth <= 0)))
			printf("%d instructions executed\n", icount);
		else {
			if ((ins == I_CALL) || (ins == I_RET)) {
				register int i;
			printf("[after %4d]  ", icount);
				for (i = call_depth; --i > 0;)
					printf("  ");
				kdb_regs = regs;     /* needed by printpc */
				printpc();
			}
			if (ins == I_CALL)
				call_depth++;

			loopcnt++;
			icount++;
			goto exit;
		}
	    }
	    if (sstepmode == STEP_CALLT){
		/*
		 *	Keep going until CALL or RETURN
		 */
		int	ins1 = chkget(regs[EIP]);
		int	ins = ins1 & 0xff;

		if (ins == 0xff) {
			ins1 &= 0x3800;
			if (ins1 == 0x1000 || ins1 == 0x1800) {
				ins = I_CALL;
			}
		}

		if ((ins == I_CALL) || (ins == I_RET) || (ins == I_REI))
			printf("%d instructions executed\n", icount);
		else {
			loopcnt++;
			icount++;
			goto exit;
		}
	    }
	}
	/*
	 *	Prologue for simple command execution
	 */
	if (!regs)
		regs = tregs;
	active_regs = regs;
	curproc = u.u_procp;
	kdbgetprocess(curproc);
	curpid = curproc->p_pid;
	pid    = curpid;

	var[varchk('m')] = (long) curmap;
	var[varchk('p')] = (long) curpcb;
	var[varchk('t')] = (int)regs;

	/*
	 *	Special cases for special commands like
	 *	:s and :b
	 */
	delbp();
	kdb_regs[EFL] &= ~EFL_TF;
	{
#ifdef	USER_BREAKS
		int *sp = (int *) kdb_regs[ESP];
		if ((sp[3]&0xffff) != KCSSEL)
			kdb_regs[ESP] += 7 * sizeof (int);
		else
#endif	USER_BREAKS
			kdb_regs[ESP] += 5 * sizeof (int);
	}
	sstepmode = -1;
	kdb_singlestep = FALSE;

	print_salutation(type);
	/*
	 * 	Command Loop
	 */
	kdbsetjmp(jmpbuf);
	executing = 0;
	eof=0;
	peekc = 0;
	for (;;) {
		flushbuf();
		if (errflg) {
			printf("%s\n", errflg);
			exitflg = (int)errflg;
			errflg = 0;
		}
		if (mkfault) {
			mkfault=0;
			printc('\n');
			prints(DBNAME);
		}
		lp=0; rdc(); lp--;
		if (eof) {
			goto bptexit;
		} else
			exitflg = 0;
		runmode = -1;
		command(0, lastcom);
		switch (runmode) {
		case SINGLE:		/* continue for single step */
                        doreinput = 1;
single:
			kdb_regs[EFL] |= EFL_TF;
			kdb_singlestep = TRUE;
exit:
			splon(s);
			_wdr6(0);
			return;
		case CONTIN:		/* continue for break point */
		    {
			register bkpt_t	bp;

                        doreinput = 0;
                        lastcom = 0;
			if (bp = scanbkpt(kdb_regs[EIP])) {
				step_thru_bpt=1;
				goto single;
			}
bptexit:
			pb(-1);		/* reset playback buffer */
			setbp();
			splon(s);
			_wdr6(0);
			return;
		     }
		}
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
	if (errflg || mkfault)
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
	kdblongjmp(jmpbuf, 1);
}


/*
 *	Return the map and pcb for a process.
 */
kdbgetprocess(p)
	struct proc	*p;
{
	/*
	 *	special case for current process
	 */
	if (!p || p == u.u_procp) {
		curmap = current_task()->map;
		curpcb = current_thread()->pcb;
		kdb_regs = active_regs;
	} else {
		if (p->task)
			curmap = p->task->map;
		else
			panic("kdbgetprocess: no task");
		if (p->thread) {
			extern int foreign_regs[];

			regs2cntxt(kdb_regs, curpcb);
			curpcb = p->thread->pcb;
			kdb_regs = foreign_regs;
			/* cntxt2regs(p->thread->pcb, kdb_regs);*/
		} else
			panic("kdbgetprocess: no pcb");
	}
}

#define	O_EDI	0
#define	O_ESI	1
#define	O_EBX	2
#define	O_EBP	3
#define	O_ESP	4
#define O_EIP	5
#define	O_IPL	6

cntxt2regs(pcb, regs)
struct i386_saved_state *regs;
struct pcb *pcb;
{
	int temp	= chkget(&pcb->pcb_context[O_EIP]);	/* is it in memory */

	regs->eip	= pcb->pcb_context[O_EIP];
	regs->ebx	= pcb->pcb_context[O_EBX];
	regs->esp	= pcb->pcb_context[O_ESP];		/* kernel stack pointer ??? */
	regs->ebp	= pcb->pcb_context[O_EBP];
	regs->esi	= pcb->pcb_context[O_ESI];
	regs->edi	= pcb->pcb_context[O_EDI];
}

regs2cntxt(regs, pcb)
struct i386_saved_state *regs;
struct pcb *pcb;
{
	pcb->pcb_context[O_EIP] = regs->eip;
	pcb->pcb_context[O_EBX] = regs->ebx;
/*	pcb->pcb_context[O_ESP] = regs->esp; */
	pcb->pcb_context[O_EBP] = regs->ebp;
	pcb->pcb_context[O_ESI] = regs->esi;
	pcb->pcb_context[O_EDI] = regs->edi;
}
