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
static char	*sccsid = "@(#)$RCSfile: pcs.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:11:37 $";
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

msg		NOBKPT;
msg		SZBKPT;
msg		EXBKPT;
msg		BADMOD;

/* breakpoints */
bkpt_t		bkpthead;

char		*lp;
char		lastc;

INT		signo;
long		dot;
INT		pid;
long		cntval;
long		loopcnt;

long		entrypt;
INT		adrflg;

int	runmode;

#include <i386/kdb/pcs.h>
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

/* sub process control */
/* DEBUG */
int kdb_debug = 0;

subpcs(modif)
{
	extern long	expv;
	register INT		check;
	INT		execsig;
	register bkpt_t	bkptr;
	string_t		comptr;
	char	c;
	int	s, i=0, shift, dr6, dr7;
	extern int *active_regs;
	extern int dotinc;
        
	execsig=0;
	loopcnt=cntval;

	switch (modif) {

		/* delete breakpoint */
	case 'D': /* 386 style */
		if (adrflg) {
			extern int adrval;
			if (_dr0() == adrval)
				i = 0;
			else if (_dr1() == adrval)
				i = 1;
			else if (_dr2() == adrval)
				i = 2;
			else if (_dr3() == adrval)
				i = 3;
			else {
				printf("breakpoint not found\n");
				return;
			}
		} else if (expr(0))
			i = expv;
		else
			i = 0;
		if (i < 0 || i > 3)
			i = 0;
		dr7 = _dr7();
		dr7 &= ~(3 << (i + i));	/* disable debug register i */
		_wdr7(dr7);
		return;
	case 'd':
		if ( (bkptr=scanbkpt(dot)) ) {
			bkptr->flag=0;
			return;
		} else {
			error(NOBKPT);
		}

		/* set breakpoint */
	case 'B': /* 386 style */ {
                int type = 0;
#define B_INTR  0
#define B_DWR   1
#define B_DRW   3
#define S_BYTE  0
#define S_WORD  4
#define S_LONG  12
#define GE      0x200
                while (c = nextchar())
                switch (c) {
                case 0:
                        break;
                case 'i':
                        type |= B_INTR;
                        break;
                case 'w':
                        type |= B_DWR;
                        break;
                case 'r':
                        type |= B_DRW;
                        break;
                case 'b':
                        type |= S_BYTE;
                        break;
                case 's':
                        if (type & B_INTR == 0)
                                type |= S_WORD;
                        break;
                case 'l':
                        if (type & B_INTR == 0)
                                type |= S_LONG;
                        break;
                case '0':
                case '1':
                case '2':
                case '3':
                        i = c - '0';
                        break;
                default:
                        goto bpt_usage;
                }
#if	1 /* DEBUG */
                if (kdb_debug)
                        printf("dr%d type %x\n", i, type);
#endif
#if 0
		if (expr(0))
			i = expv;
		else
			i = 0;
		if (i < 0 || i > 3) {
                        printf("invalid breakpoint register\n");
                        return;
                }
#endif
		dr7 = _dr7();
		dr7 |= 2 << (i + i);	/* enable debug register i */
		shift = (i << 2) + 16;
		dr7 &= ~(0xf << shift);	/* break condition */
                dr7 |= type << shift;
                if (type & B_DRW || type & B_DWR)
                        dr7 |= GE;
                else
                        dr7 &= ~GE;
#if	1 /* DEBUG */
                if (kdb_debug)
                        printf("dr7 %08x\n", dr7);
#endif
		_wdr7(dr7);

		switch (i) {		/* write addr into debug register */
		case 0:
			_wdr0(dot);
			break;
		case 1:
			_wdr1(dot);
			break;
		case 2:
			_wdr2(dot);
			break;
		case 3:
			_wdr3(dot);
			break;
		}
		return;
        bpt_usage:
                printf(":B usage:\n");
                printf("        :B[type][size][reg]\n");
                printf("        type = i|r|w\n");
                printf("        size = b|s|l\n");
                printf("        reg = dr0-3\n");
                return;
        }
	case 'b':
		if ( (bkptr=scanbkpt(dot)) ) {
			bkptr->flag=0;
		}
		for ( bkptr=bkpthead; bkptr; bkptr=bkptr->nxtbkpt ) {
			if ( bkptr->flag == 0 ) {
				break;
			}
		}
		if ( bkptr==0 ) {
			if ( (bkptr=(bkpt_t)sbrk(sizeof *bkptr)) == (bkpt_t)-1 )
				{
					error(SZBKPT);
				} else {
					bkptr->nxtbkpt=bkpthead;
					bkpthead=bkptr;
				}
		}
		bkptr->loc = dot;
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
		runmode=SINGLE;
		sstepmode = STEP_NORM;
		break;

	case 'p':
	case 'P':
		runmode = SINGLE;
		sstepmode = STEP_PRINT;
		break;

	case 'j':
	case 'J':

		runmode = SINGLE;
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
                        active_regs[EIP] = (long)kdb_kill;
                else
                        active_regs[EIP] = (long)kdb_kill_nosync;
		break;

		/* Run the debugged process, (i.e. the kernel) aka reboot */
	case 'r':
		runmode=CONTIN;
		execsig=getsig(signo);
		sstepmode = STEP_NONE;
                active_regs[EIP] = (long)kdb_reboot;
		break;

		/* continue with optional signal */
	case 'c':
	case 'C':
		runmode=CONTIN;
		sstepmode = STEP_NONE;
		break;

	case 'R':
		dot = ((int *)active_regs[EBP])[1];
		goto bkpttmp;
	case 'S':
		dot += dotinc;
bkpttmp:
		if ( (bkptr=scanbkpt(dot)) ) {
			return;
		}
		for ( bkptr=bkpthead; bkptr; bkptr=bkptr->nxtbkpt ) {
			if ( bkptr->flag == 0 ) {
				break;
			}
		}
		if ( bkptr==0 ) {
			if ( (bkptr=(bkpt_t)sbrk(sizeof *bkptr)) == (bkpt_t)-1 )
				{
					error(SZBKPT);
				} else {
					bkptr->nxtbkpt=bkpthead;
					bkpthead=bkptr;
				}
		}
		bkptr->loc = dot;
		bkptr->initcnt = bkptr->count = 1;
		bkptr->flag = BKPTTMP;

		runmode=CONTIN;
		sstepmode = STEP_NONE;
		break;

		/* z: force a dump and halt the system */
	case 'z':
		runmode=CONTIN;
		sstepmode = STEP_NONE;
                active_regs[EIP] = (long)kdb_dump;
		break;

#ifdef notyet
		 /* Z: force a dump and return to kdb */
		 /* this doesn't work on 386 yet, save_context_all undefined */
	case 'Z':
		save_context_all();
		dumpsys();
		break;
#endif
	default:
		printf("don't understand modifier 0%o\n", modif);
		error(BADMOD);
	}
}
