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
static char	*sccsid = "@(#)$RCSfile: subr_mcount.c,v $ $Revision: 4.3.3.3 $ (DEC) $Date: 1992/12/09 06:54:32 $";
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
 * Copyright (C) 1988,1989 Encore Computer Corporation.  All Rights Reserved
 *
 * Property of Encore Computer Corporation.
 * This software is made available solely pursuant to the terms of
 * a software license agreement which governs its use. Unauthorized
 * duplication, distribution or sale are strictly prohibited.
 *
 */
/*
 * Copyright (c) 1982, 1986 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *

 */

#include <cputypes.h>

/* last integrated from: gmon.c	4.10 (Berkeley) 1/14/83 */

#if PROFILING
#include <sys/gprof.h>
#include <sys/param.h>
#include <sys/systm.h>
#include <machine/cpu.h>
#include <machine/regdef.h>

int profiling = 1;

#if !defined(mips) && !defined(__alpha)

/*
 * Froms is actually a bunch of unsigned shorts indexing tos
 */
u_short *froms;
struct tostruct *tos = NULL;
long tolimit = 0L;

#ifdef	i386
char	*s_lowpc = (char *)0xc0000000;
#define TOSFRACTION	100
#endif

#ifdef	vax
char	*s_lowpc = (char *)0x80000000;
#define TOSFRACTION	100
#endif

#ifdef	ibmrt
char    *s_lowpc = (char *)0xe0000000;
#define TOSFRACTION	200
#endif	ibmrt

#ifdef	sun
char	*s_lowpc = (char *)0x0e000000;
#define TOSFRACTION	100
#endif	sun

#ifdef	ns32000
char 	*s_lowpc = (char *) 0x0;
#define TOSFRACTION	100

#include <vm/vm_kern.h>
#define calloc(size) ((caddr_t)kmem_alloc(kernel_map, size, TRUE))
#endif	ns32000

#if	defined(sun)
#define spl_high()	np_splhigh()
#define spl_x(x)	np_splx(x)
#else	defined(sun)
#define spl_high() 	splhigh()
#define spl_x(x)	splx(x)
#endif	defined(sun)

extern char etext;

char *s_highpc = &etext;
u_long	s_textsize = 0;

int ssiz;
u_short	*sbuf;
u_short	*kcount;

kmstartup()
{
	u_long	fromssize, tossize;

	/*
	 *	round lowpc and highpc to multiples of the density we're using
	 *	so the rest of the scaling (here and in gprof) stays in ints.
	 */
	s_lowpc = (char *)
	    ROUNDDOWN((unsigned)s_lowpc, HISTFRACTION*sizeof(HISTCOUNTER));
	s_highpc = (char *)
	    ROUNDUP((unsigned)s_highpc, HISTFRACTION*sizeof(HISTCOUNTER));
	s_textsize = s_highpc - s_lowpc;
	ssiz = (s_textsize / HISTFRACTION) + sizeof(struct phdr);
	sbuf = (u_short *)calloc(ssiz);
	if (sbuf == 0) {
		printf("No space for monitor buffer(s)\n");
		return;
	}
	blkclr((caddr_t)sbuf, ssiz);
#ifndef	i386
	fromssize = s_textsize / HASHFRACTION;
	froms = (u_short *)calloc(fromssize);
	if (froms == 0) {
		printf("No space for monitor buffer(s)\n");
		cfreemem(sbuf, ssiz);
		sbuf = 0;
		return;
	}
	blkclr((caddr_t)froms, fromssize);
 
	tolimit = s_textsize * ARCDENSITY / TOSFRACTION;
	if (tolimit < MINARCS) {
		tolimit = MINARCS;
	} else if (tolimit > 65534) {
		tolimit = 65534;
	}
	tossize = tolimit * sizeof(struct tostruct);
	tos = (struct tostruct *)calloc(tossize);
	if (tos == 0) {
		printf("No space for monitor buffer(s)\n");
		cfreemem(sbuf, ssiz);
		sbuf = 0;
		cfreemem(froms, fromssize);
		froms = 0;
		return;
	}
	blkclr((caddr_t)tos, tossize);
	tos[0].link = 0;
#endif	i386
	((struct phdr *)sbuf)->lpc = s_lowpc;
	((struct phdr *)sbuf)->hpc = s_highpc;
#ifndef	i386
	((struct phdr *)sbuf)->ncnt = ssiz;
#endif	i386
	kcount = (u_short *)(&sbuf[sizeof(struct phdr)]);
#ifdef	notdef
	/*
	 *	profiling is what mcount checks to see if
	 *	all the data structures are ready!!!
	 */
	profiling = 0;		/* patch by hand when you're ready */
#endif
}

#if	defined(vax) || defined(balance) || defined(sun)
/*
 * This routine is massaged so that it may be jsb'ed to
 */
asm(".text");
asm("#the beginning of mcount()");
asm(".data");
#endif	defined(vax) || defined(balance) || defined(sun)

#if	defined(vax) || defined(ibmrt) || defined(ns32000) || defined(sun)

#if	multimax
mmax_mcount(selfpc, frompcindex)
register char		*selfpc;
register unsigned short	*frompcindex;
{
	register struct tostruct	*top;
	register struct tostruct	*prevtop;
	register long			toindex;
#else	multimax
mcount()
{
	register char			*selfpc;	/* r11 => r5 */
	register unsigned short		*frompcindex;	/* r10 => r4 */
	register struct tostruct	*top;		/* r9  => r3 */
	register struct tostruct	*prevtop;	/* r8  => r2 */
	register long			toindex;	/* r7  => r1 */
#endif	multimax
#ifdef	balance
	int s;
#else	balance
	static int s;
#endif	balance

	if (profiling)
		goto out;
	if (cpu_number() != master_cpu)
		goto out;
#ifdef	lint
	selfpc = (char *)0;
	frompcindex = 0;
#else	not lint
	/*
	 *	find the return address for mcount,
	 *	and the return address for mcount's caller.
	 */
#ifdef	vax	 
	asm("	.text");		/* make sure we're in text space */
	asm("	movl (sp), r11");	/* selfpc = ... (jsb frame) */
	asm("	movl 16(fp), r10");	/* frompcindex =     (calls frame) */
#endif	vax	
#if	defined(balance)
	asm("	.text");		/* make sure we're in text space */
	asm("	movd 4(fp), r7");
	asm("	movd 4(0(fp)), r6");
#endif	defined(balance)
#if	defined(sun)
	asm("	.text");		/* make sure we're in text space */
	asm("	movl a6@(4), a5");	/* selfpc */
	asm("	movl a6@,a0");
	asm("	movl a0@(4), a4");	/* frompc */
#endif	defined(sun)
#endif	not lint
	/*
	 *	Moved to above
	 */
#ifdef	ibmrt
	/*
	 *	and that we aren't recursively invoked.
	 */
	profiling++;
#endif	ibmrt	
	/*
	 *	insure that we cannot be recursively invoked.
	 *	this requires that splhigh() and splx() below
	 *	do NOT call mcount!
	 */
	s = spl_high();
	/*
	 *	check that frompcindex is a reasonable pc value.
	 *	for example:	signal catchers get called from the stack,
	 *			not from text space.  too bad.
	 */
	frompcindex = (unsigned short *)((long)frompcindex - (long)s_lowpc);
	if ((unsigned long)frompcindex > s_textsize) {
		goto done;
	}
	frompcindex =
	    &froms[((long)frompcindex) / (HASHFRACTION * sizeof(*froms))];
	toindex = *frompcindex;
	if (toindex == 0) {
		/*
		 *	first time traversing this arc
		 */
		toindex = ++tos[0].link;
		if (toindex >= tolimit) {
			goto overflow;
		}
		*frompcindex = toindex;
		top = &tos[toindex];
		top->selfpc = selfpc;
		top->count = 1;
		top->link = 0;
		goto done;
	}
	top = &tos[toindex];
	if (top->selfpc == selfpc) {
		/*
		 *	arc at front of chain; usual case.
		 */
		top->count++;
		goto done;
	}
	/*
	 *	have to go looking down chain for it.
	 *	top points to what we are looking at,
	 *	prevtop points to previous top.
	 *	we know it is not at the head of the chain.
	 */
	for (; /* goto done */; ) {
		if (top->link == 0) {
			/*
			 *	top is end of the chain and none of the chain
			 *	had top->selfpc == selfpc.
			 *	so we allocate a new tostruct
			 *	and link it to the head of the chain.
			 */
			toindex = ++tos[0].link;
			if (toindex >= tolimit) {
				goto overflow;
			}
			top = &tos[toindex];
			top->selfpc = selfpc;
			top->count = 1;
			top->link = *frompcindex;
			*frompcindex = toindex;
			goto done;
		}
		/*
		 *	otherwise, check the next arc on the chain.
		 */
		prevtop = top;
		top = &tos[top->link];
		if (top->selfpc == selfpc) {
			/*
			 *	there it is.
			 *	increment its count
			 *	move it to the head of the chain.
			 */
			top->count++;
			toindex = prevtop->link;
			prevtop->link = top->link;
			top->link = *frompcindex;
			*frompcindex = toindex;
			goto done;
		}

	}
done:
#ifdef	ibmrt
	profiling--;
#endif	ibmrt	
	spl_x(s);
	/* and fall through */
out:

#ifdef	vax
	asm("	rsb");
#endif	vax

#if	defined(ibmrt) || defined(ns32000) || defined(sun)
	return;
#endif	defined(ibmrt) || defined(ns32000) || defined(sun)

overflow:
	profiling = 3;
	printf("mcount: tos overflow\n");
	spl_x(s);
	goto out;
}
#if	defined(vax) || defined(balance) || defined(sun)
asm(".text");
asm("#the end of mcount()");
asm(".data");
#endif	defined(vax) || defined(balance) || defined(sun)
#endif	defined(vax) || defined(ibmrt) || defined(ns32000) || defined(sun)

#endif	!(defined(mips) || defined(__alpha))
#if defined(mips) || defined(__alpha)

#if	PROFTYPE < 1 || PROFTYPE > 4
#include "error: PROFTYPE incorrectly defined (must have value from 0 to 4)"
#endif

#include <sys/time.h>
#include <sys/kernel.h>

u_int *kcount;
char *s_lowpc;
u_long s_textsize;
struct phdr phdr;

kmstartup()
{
	if (phdr.pc_buf == 0) {
		printf("No space for monitor buffer(s)\n");
		s_textsize = 0;
		return;
	}
	bzero((caddr_t)phdr.pc_buf, phdr.pc_bytes);

	phdr.proftype = PC_SAMPLES;
	phdr.sample_hz = phz ? phz : hz;
	kcount = (u_int *)phdr.pc_buf;

#if	PROFTYPE == 1
	printf("Profiling type 1 (PC samples only)\n");
#endif	PROFTYPE == 1

#if	PROFTYPE == 2 || PROFTYPE == 3
	prof_startup();
#endif	PROFTYPE == 2 || PROFTYPE == 3
#if	PROFTYPE == 4
	gprof_startup();
#endif	PROFTYPE == 4
}

#if	PROFTYPE == 2 || PROFTYPE == 3

/*
 * "_mcount" adds (return_address >> 1) to this to find the corresponding
 * counter within the array of counters. Don't set this nonzero till after
 * we've allocated storage for the counters, since it also prevents _mcount
 * from being called from within sbrk before we're ready.
 */
char * _mcountoff;

prof_startup()
{
	if (phdr.bb_buf == 0) {
		printf("No space for bb counts\n");
		return;
	}
	bzero((caddr_t)phdr.bb_buf, phdr.bb_bytes);

#if	PROFTYPE == 2
	printf("Profiling type 2 (Invocation counts)\n");
	phdr.proftype |= INV_COUNTS;
#else	/* PROFTYPE == 3 */
	printf("Profiling type 3 (Basic Block counts)\n");
	phdr.proftype |= BB_COUNTS;
#endif	/* PROFTYPE */

	_mcountoff = 0;
	_mcountoff = phdr.bb_buf/* - (((unsigned) phdr.lowpc) >> 1)*/; 
}
#endif	/* PROFTYPE == 2 || PROFTYPE == 3 */

#if	PROFTYPE == 4
/*
 * GPROF profiling initialization
 *
 * Froms is actually a bunch of unsigned shorts indexing tos
 */
u_short	*sbuf = NULL;
u_short *froms = NULL;
struct tostruct *tos = NULL;
long tolimit = 0L;
long ssiz = 0L;

gprof_startup()
{
	u_long	fromssize, tossize;

	froms = (u_short *)phdr.froms_buf;
	if (froms == 0) {
		printf("No space for froms buffer\n");
		s_textsize = 0;
		return;
	}
	bzero((caddr_t)phdr.froms_buf, phdr.froms_bytes);
	tos = (struct tostruct *)phdr.tos_buf;
	if (tos == 0) {
		printf("No space for tos buffer(s)\n");
		cfreemem(froms, phdr.froms_bytes);
		s_textsize = 0;
		return;
	}
	bzero((caddr_t)phdr.tos_buf, phdr.tos_bytes);
	tos[0].link = 0;
	phdr.proftype |= GPROF_COUNTS;
        ssiz = phdr.pc_bytes;
        sbuf = (u_short *)phdr.pc_buf;
	printf("Profiling type 4 (GPROF)\n");
}
#endif	/* PROFTYPE == 4 */
#endif	/* __mips__ || __alpha */
#endif	/* PROFILING */
