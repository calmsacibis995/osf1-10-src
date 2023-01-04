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
static char	*sccsid = "@(#)$RCSfile: runpcs.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:11:48 $";
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

#undef	DEBUG

/*
 *
 *	UNIX debugger
 *
 */
#include <i386/kdb/defs.h>

extern	map	txtmap;

char		*lp;
long		sigint;
long		sigqit;

/* breakpoints */
bkpt_t		bkpthead;

REGLIST		reglist[];

char		lastc;

INT		fcor;
INT		fsym;
string_t	errflg;
INT		errno;
INT		signo;
INT		kdbsigcode;

long		dot;
string_t	symfil;
INT		wtflag;
INT		pid;
long		expv;
INT		adrflg;

getsig(sig)
{
	return(expr(0) ? expv : sig);
}

print_salutation(type)
{
	register bkpt_t	bp;
	int dr6;
	extern int dotinc;
	extern int *kdb_regs, *active_regs;
        extern int *kdbintrregs;
        
	switch (type) {
	case T_SGLSTP:
		dr6 = _dr6();
		kdb_regs[EFL] |= EFL_RF;
		if (!(dr6 & DBG_BS)) {	
			int i;
			for (i = 0; i < 4; i++)
				if (dr6 & (1 << i)) {
					printf("Break(#%d, %X) [%s]: bp = %X, cr3 = %X\n", 
						i, kdb_regs, u.u_comm, kdb_regs[EBP], get_cr3());
					break;
				}
			if (i == 4) 
				printf("SGLSTP(?, %X) [%s]: bp = %X, cr3 = %X\n", 
					kdb_regs, u.u_comm, kdb_regs[EBP], get_cr3());
		}
		break;
	case T_BPTFLT:
		/* look for int 3 break point */
		kdb_regs[EIP]--;		/* so we can retry bpt'd instruction */
		if (bp=scanbkpt(kdb_regs[EIP])) {
			if (bp->flag & BKPTTMP) {
				bp->flag = 0;
				break;
			}
			printf("Break Point(%X) [%s]: bp = %X, cr3 = %X\n", 
				kdb_regs, u.u_comm, kdb_regs[EBP], get_cr3());
			bp->count=bp->initcnt;
		} else {
			printf("Bpt (%X) [%s]: bp = %X, cr3 = %X\n", 
				kdb_regs, u.u_comm, kdb_regs[EBP], get_cr3());
			kdb_regs[EIP]++;		/* skip it */
                        /*active_regs = kdbintrregs;*/
		}
		break;
	case 0:
		printf("Kdb(%X) [%s]: fp = %X, cr3 = %X\n", 
			kdb_regs, u.u_comm, &type-2, get_cr3());
		break;
	default:
	    {
	 	extern char *trap_type[];
		extern int TRAP_TYPES;
		extern char *kdberrflg;
		register int code = kdb_regs[EFL-2];

		printf("Kdb kernel trap [%s]: ", u.u_comm);
		if ((unsigned)type >= TRAP_TYPES)
			printf("type %d", type);
		else
			printf("%s", trap_type[type]);
		printf(" trap, code=%X\n", code);
	    }
	}
	dot = kdb_regs[EIP];
	dotinc = 0;
	psymoff(dot, ISYM, ":");
	printins(0, ISP, chkget(dot, ISP));
}

#define BPOUT 0
#define BPIN 1
INT bpstate = BPOUT;

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
			if ( bp->flag ) {
				a=bp->loc;
				put(a, (bp->ins&0xff)|(get(a)&~0xff));
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
			if ( bp->flag ) {
				a = bp->loc;
				bp->ins = get(a);
				put(a, BPT | (bp->ins)&~0xff);
				if ( errno ) {
					prints("cannot set breakpoint: ");
					psymoff(bp->loc,ISYM,"\n");
				}
			}
		}
		bpstate=BPIN;
	}
}
