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
static char	*sccsid = "@(#)$RCSfile: stack.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:11:54 $";
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

#define	UNKNOWN_NUM_ARGS 5

#include <i386/kdb/defs.h>

long adrval;
short adrflg;
extern int *kdb_regs;

oldtrace(modif)
{
	if (adrflg == 0)
		adrval = kdb_regs[EBP];
        kdbtrace(adrval, kdb_regs[EIP], modif);
}

struct i386_frame {
	struct i386_frame	*ofp;
	int			raddr;
	int			parm[1];
};

#define IN_KERNEL_STACK(x)	((unsigned)(x) >= (unsigned)(VM_MIN_KERNEL_ADDRESS))
#define PAGE(a)	((unsigned int)a & ~0xfff)
#define TRAP 0x1
#define INTERRUPT 0x2
#define SYSCALL 0x3

kdbtrace(fp, ip, modif)
struct i386_frame *fp;
int ip;
{
	int n;
	int *argp;
	char *prf;
	int is_trap;
	int sym;

/*
	while (fp > (struct i386_frame *)VM_MIN_KERNEL_ADDRESS) {
 */
	for (;;) {
		if (modif == 'k')
			if (!IN_KERNEL_STACK(fp))
				break;
		if (modif == 'c')
			if (PAGE(fp) != PAGE(fp->ofp))
				break;
		is_trap = 0;
		printsym(fp->raddr);
#ifdef	wheeze
		findsym(ip, ISYM);
		printf(":\t%s(", cursym->n_name);
#else	wheeze
		if (sym = findsym(ip, ISYM) != MAXINT)
			printf(":\t%s(", cursym->n_un.n_name);
		else
			printf(":\t%X(", ip);
#endif	wheeze

		/* Get number of arguments and display their addresses. */
		n = kdbnumargs(fp);
		if (n == UNKNOWN_NUM_ARGS && sym) {
#ifdef	wheeze
			if (strcmp(cursym->n_name, "trap") == 0 ||
			    strcmp(cursym->n_name, "_trap") == 0)
#else	wheeze
			if (strcmp(cursym->n_un.n_name, "trap") == 0 ||
			    strcmp(cursym->n_un.n_name, "_trap") == 0)
#endif	wheeze

			{
				n = 1;
				is_trap = TRAP;
			}
			
#ifdef	wheeze
			if (strcmp(cursym->n_name, "syscall") == 0 ||
			    strcmp(cursym->n_name, "_syscall") == 0)
#else	wheeze
			if (strcmp(cursym->n_un.n_name, "syscall") == 0 ||
			    strcmp(cursym->n_un.n_name, "_syscall") == 0)
#endif	wheeze
			{
				is_trap = SYSCALL;
			}

#ifdef	wheeze
			if (strcmp(cursym->n_name, "kdintr") == 0 ||
			    strcmp(cursym->n_name, "_kdintr") == 0)
#else	wheeze
			if (strcmp(cursym->n_un.n_name, "kdintr") == 0 ||
			    strcmp(cursym->n_un.n_name, "_kdintr") == 0)
#endif	wheeze
			{
				is_trap = INTERRUPT;
			}
		}
		argp = &fp->parm[0];
		prf = "";
		while (n--) {
			printf("%s%X", prf, *argp++);
			prf = ", ";
		}
		printf(")\n");

		kdbnextframe(&fp, &ip, &fp->parm[0], is_trap);
	}
}

/* 
 * XXX - This should go in sym.c.
 */
printsym(addr)
	int addr;
{
	int n = findsym(addr, ISYM);

#ifdef	wheeze
	printf("%s+%X", cursym->n_name, n);
#else	wheeze
	if (!(int)cursym) {
		printf("%X", addr);
	} else {
		printf("%s+%X", cursym->n_un.n_name, n);
	}
#endif	wheeze
}

/* 
 * Figure out how many arguments were passed into the frame at "fp". 
 * XXX - will this work with gcc?
 */
int
kdbnumargs(fp)
	struct i386_frame *fp;
{
	extern int vstart(), etext();
	int *argp;
	int inst;			/* an i386 instruction */
	int args = UNKNOWN_NUM_ARGS;

	argp = (int *)fp->raddr;
	if (argp < (int *)vstart || argp > (int *)etext)
		args = UNKNOWN_NUM_ARGS;
	else {
		inst = *argp;
		if ((inst & 0xff) == 0x59) /* popl %ecx */
			args = 1;
		else if ((inst & 0xffff) == 0xc483) /* addl $n,%esp */
			args = ((inst >> 16) & 0xff) / 4;
		else
			args = UNKNOWN_NUM_ARGS;
	}
	return(args);
}


/* 
 * Figure out the next frame up in the call stack.  
 * For trap(), we print the address of the faulting instruction and 
 *   proceed with the calling frame.  We return the ip that faulted.
 *   If the trap was caused by jumping through a bogus pointer, then
 *   the next line in the backtrace will list some random function as 
 *   being called.  It should get the argument list correct, though.  
 *   It might be possible to dig out from the next frame up the name
 *   of the function that faulted, but that could get hairy.
 */

kdbnextframe(fp, ip, argp, is_trap)
	struct i386_frame **fp;		/* in/out */
	int *ip;			/* out */
	int *argp;			/* in */
	int is_trap;			/* in */
{
	int *saved_regs;
	char *trap_name;

	if (is_trap == 0) {
		*ip = (*fp)->raddr;
		*fp = (*fp)->ofp;
	} else {
		/* 
		 * We know that trap() has 1 argument and we know that
		 * it's an (int *).
		 */
		if (is_trap == INTERRUPT ) {
			saved_regs = (int *) ((*fp)->ofp);
			printf("--- interrupt (number 0x%x) ---\n",
			       saved_regs[TRAPNO] & 0xffff);
		} else if (is_trap == TRAP) {
			saved_regs = (int *)*argp;
			trap_name = (saved_regs[TRAPNO] < TRAP_TYPES) ? 
				trap_type[saved_regs[TRAPNO]] :
				"?????";
			printf("--- %s ---\n", trap_name);
			printsym(saved_regs[EIP]);
			printf(":\n");
		} else {
			extern char *syscallnames[];

			saved_regs = (int *)*argp;
			printf("--- SYSCALL %d (%s)---\n",
				saved_regs[EAX],
				syscallnames[saved_regs[EAX]]);
			printsym(saved_regs[EIP]);
			printf(":\n");
		}
		*fp = (struct i386_frame *)saved_regs[EBP];
		*ip = saved_regs[EIP];
	}

}
