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
static char	*sccsid = "@(#)$RCSfile: print.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:11:45 $";
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
 *
 *	UNIX debugger
 *
 */
#include <i386/kdb/defs.h>

#include <kern/task.h>
#include <kern/thread.h>
#include <sys/proc.h>
#include <kern/kdb_print.h>

msg		BADMOD;

map		txtmap;
map		datmap;

long		lastframe;
long		callpc;

INT		infile;
INT		outfile;
char		*lp;
long		maxoff;
long		maxpos;
INT		radix;

/* symbol management */
long		localval;

/* breakpoints */
bkpt_t		bkpthead;

extern int	*kdb_regs;

REGLIST reglist [] = {
	"eip",	EIP,	0,
	"pc",	EIP,	0,

	"eax",	EAX,	0,
	"ax",	EAX,	0,

	"ebx",	EBX,	0,
	"bx",	EBX,	0,

	"ecx",	ECX,	0,
	"cx",	ECX,	0,

	"edx",	EDX,	0,
	"dx",	EDX,	0,

	"esi",	ESI,	0,
	"si",	ESI,	0,

	"edi",	EDI,	0,
	"di",	EDI,	0,

	"esp",	ESP,	0,
	"sp",	ESP,	0,

	"ebp",	EBP,	0,
	"bp",	EBP,	0,

	"cs",	CS,	0,
	"ds",	DS,	0,
	"es",	ES,	0,
	"fs",	FS,	0,
	"gs",	GS,	0,
	"err",	ERR,	0,
	"trapno",TRAPNO,0,

	"efl",	EFL,	0,
	"flags",EFL,	0
};

#define NREGS	(sizeof(reglist)/sizeof (REGLIST))

char		lastc;

INT		fcor;
string_t		errflg;
INT		signo;
INT		kdbsigcode;


long		dot;
long		var[];
string_t	symfil;
string_t	corfil;
INT		pid;
long		adrval;
INT		adrflg;
long		cntval;
INT		cntflg;

/* general printing routines ($) */

printtrace(modif)
{
	extern long	expv;
	INT		i;
	register bkpt_t	bkptr;
	int		dr7;
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
				kdbgetprocess(p);
				var[varchk('m')] = (long) curmap;
				pid = p->p_pid;
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
		flushbuf();

	case 'r':
	case 'R':
		printregs();
		return;

	case 'c':
	case 'C':
		oldtrace('c') ;
		break;

#if 0
	case 'n':
	case 'N':
		oldtrace('n') ;
		break;
#endif
		/*print externals*/
	case 'e':
	case 'E':
#ifdef	wheeze
#else	wheeze
		for (sp = symtab; sp < esymtab; sp++) {
			if (sp->n_type==(N_DATA|N_EXT) || sp->n_type==(N_BSS|N_EXT))
				printf("%s:%12t%R\n", sp->n_un.n_name, get(sp->n_value));
		}
#endif	wheeze
		break;

		/*print breakpoints*/
	case 'B': /* 386 style */
		printf("bkpt\taddress\t\ttype\tsize\n");
		dr7 = _dr7();
		for (i = 0; i < 4; i++)
			if (dr7 & (2 << (i + i)))
				prbkpt(i, (dr7 >> ((i << 2) + 16)) & 0xf);
		break;
	case 'b':
		printf("breakpoints:\ncount  bkpt address\tcommand\n");
		for (bkptr=bkpthead; bkptr; bkptr=bkptr->nxtbkpt)
			if (bkptr->flag) {
				printf("%-5.5d\t",bkptr->count);
				psymoff(bkptr->loc, ISYM, "\t");
				comptr=bkptr->comm;
				while ( *comptr ) {
					printc(*comptr++);
				}
			}
		break;
	case '<':
		printf("inb(%x) = %x\n", dot, inb(dot) & 0xff);
		break;
	case '>':
		if (expr(0) == 0)
			error("need value for outb");
		printf("outb(%x, %x) = %x\n", dot, expv, outb(dot, expv));
		break;
	default:
		printf("don't understand modifier 0%o\n", modif);
		error(BADMOD);
	}
}

prbkpt(n, t)
int n, t;
{
	register int pc;
        char *type, *size = "";

        switch (t & 3) {
        case 0:
                type = "inst";
                goto dr_reg;
        case 1:
                type = "wr";
                break;
        case 3:
                type = "rw";
                break;
        }
        switch ((t & 12) >> 2) {
        case 0:
                size = "byte";
                break;
        case 1:
                size = "word";
                break;
        case 3:
                size = "long";
                break;
        }
dr_reg:
	switch (n) {
	case 0:
		pc = _dr0();
		break;
	case 1:
		pc = _dr1();
		break;
	case 2:
		pc = _dr2();
		break;
	case 3:
		pc = _dr3();
		break;

	}

        printf("   %d\t", n);
	psymoff(pc, ISYM, "\t");
        printf("%s\t%s\n", type, size);
	return;
}

printregs()
{
	register REGPTR	p;
	long		v;
	int		i, n;

	printf("pc %9X, esp %9X, ebp %9X, flags %9X\n",
		kdb_regs[EIP], kdb_regs[ESP], kdb_regs[EBP],
		kdb_regs[EFL]);

	printf("eax %9X, ebx %9X, ecx %9X, edx %9X\n",
		kdb_regs[EAX], kdb_regs[EBX], kdb_regs[ECX], kdb_regs[EDX]);
	printf("esi %9X, edi %9X, err %9X, trapno %9X\n",
		kdb_regs[ESI], kdb_regs[EDI], kdb_regs[ERR], kdb_regs[TRAPNO]);
	printf("cs %4x, ss %4x, es %4x, ds %4x, fs %4x, gs %4x\n",
		kdb_regs[CS], kdb_regs[SS], kdb_regs[ES],
		kdb_regs[DS], kdb_regs[FS], kdb_regs[GS]);

	printpc();
}

int *
getreg(regnam)
{
	register REGPTR	p;
	register string_t	regptr;
	char	*olp=lp;

	for ( p=reglist; p < &reglist[NREGS]; p++ ) {
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
				return (&kdb_regs[p->roffs]);
			}
		}
	}
	lp=olp;
	return(0);
}

printpc()
{
	dot = kdb_regs[EIP];
	psymoff(dot,ISYM,":%16t");
	printins(0,ISP,chkget(dot));
	printc(EOR);
}

print_stack_trace(thread)
	thread_t	thread;
{
/*	label_t		save_kdb_save;*/
	int		save_jmpbuf[6];
	extern int	jmpbuf[6];
	int		*savekdb_regs = kdb_regs;
	int		hold_regs[20];
	int		saveadrflg = adrflg;

	bcopy(jmpbuf, save_jmpbuf, sizeof jmpbuf);

	if (thread != current_thread()) {
		kdb_regs = hold_regs;
		cntxt2regs(thread->pcb, kdb_regs);
	} else {
		extern int *active_regs;

		kdb_regs = active_regs;
	}

	printf("thread 0x%X, ", thread);
	if (!kdbsetjmp(jmpbuf)) {
		printf("W_EVENT ");
		psymoff((long)thread->wait_event, ISYM, "");
		printc(EOR);
		adrflg = 0;
		oldtrace('k');
	} else {
		errflg = 0;
		printf(" FAILED\n");
	}
	adrflg = saveadrflg;

	kdb_regs = savekdb_regs;
	bcopy(save_jmpbuf, jmpbuf, sizeof jmpbuf);
}
