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
static char	*sccsid = "@(#)$RCSfile: sym.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:12:01 $";
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
 * adb - symbol table routines
 */
#include <i386/kdb/defs.h>

#ifdef	wheeze
/*
 * findsym looks thru symtable to find the routine name which begins
 * closest to value and returns the difference between value
 * and the symbol found.  Leaves a pointer to the symbol found
 * in cursym.  The format of symtable is a list of
 * (long, null-terminated string) pairs; the list is sorted
 * on the longs.  symtable is populated by patching the kernel using
 * a program called unixsyms.
 */

#define SYMSIZE	0x11000
char symtable[SYMSIZE] = {0};     /* initialize to get it into .data section */

findsym(value, tell)
unsigned long value;
int tell;   /* flag to print name and location */
{
	extern struct nlist *cursym;
	char *p = symtable;
	char *oldp = p;

	while (*(unsigned long *)p <= value)
	{
		oldp = p;
		p += sizeof(long);	/* jump past long */
		while (*p++)		/* jump past string */
			if (p >= &symtable[SYMSIZE])
				break;
	}
	p = oldp + sizeof(long);        /* name string */
	cursym = (struct nlist *)(p - sizeof(long));
	return(value - cursym->n_value);
}

/*
 * lookupsym looks thru symtable to find the address of name.
 */

struct nlist *
lookup(name)
char *name;
{
	char *p = symtable + sizeof(long);
	char *oldp = p;
	char *namep;

	while (*p)
	{
		oldp = p;
		for (namep = name; *namep && *p; namep++, p++)
			if (*namep != *p)
				break;
		if (!*namep && !*p)
			return((struct nlist *)(oldp - sizeof(long)));
		while (*p++);		/* jump past rest of name */
		p += sizeof(long);	/* jump past long */
	}
	return((struct nlist *)0);
}

/*
 * Advance cursym to the next local variable.
 * Leave its value in localval as a side effect.
 * Return 0 at end of file.
 */
localsym(cframe, cargp)
long cframe, cargp;
{
	char *p;

	p = (char *)0;
	if (cursym) {
		p = (char *)cursym + sizeof(long);
		while (*p++)
			;
	}
	cursym = (struct nlist *)p;
	if (cursym && cursym->n_name[0] != '\0') {
		localval = cursym->n_value;
		return(1);
	}
	return(0);
}

/*
 * Print value v and then the string s.
 * If v is not zero, then we look for a nearby symbol
 * and print name+offset if we find a symbol for which
 * offset is small enough.
 *
 * For values which are just into kernel address space
 * that they match exactly or that they be more than maxoff
 * bytes into kernel space.
 */
psymoff(v, type, s)
long v;
int type;
char *s;
{
	unsigned long w;

	if (v)
		w = (unsigned long)findsym(v, type);
#if	1
	if (v==0 || w >= (unsigned long)maxoff)
#else	1
		if (v==0 || w >= maxoff || (KVTOPH(v) < maxoff && w))
#endif	1
			printf(LPRMODE, v);
		else {
			printf("%s", cursym->n_name);
			if (w)
				printf(OFFMODE, w);
		}
	printf(s);
}

/*
 * Print value v symbolically if it has a reasonable
 * interpretation as name+offset.  If not, print nothing.
 * Used in printing out registers $r.
 */
valpr(v, idsp)
long v;
{
	off_t d;

	d = findsym(v, idsp);
	if (d >= maxoff)
		return;
	printf("%s", cursym->n_name);
	if (d)
		printf(OFFMODE, d);
}
#else	wheeze
#include <stab.h>

/*
 * Lookup a symbol by name.
 */
struct nlist *
lookup(symstr)
char *symstr;
{
	register struct nlist *sp;

	cursym = 0;
	if (symtab)
		for (sp = symtab; sp < esymtab; sp++) {
			if (!sp->n_un.n_strx)
				continue;
			/* SHOULD DO SOME OF EQSYM INLINE TO SAVE TIME */
			if ((sp->n_type&N_STAB)==0 && eqsym(sp->n_un.n_name, symstr, '_'))
				return(cursym = sp);
		}
	return (0);
}

/*
 * Find the closest symbol to val, and return
 * the difference between val and the symbol found.
 * Leave a pointer to the symbol found as cursym.
 */

findsym(val, type)
long val;
int type;
{
	long diff;
	register struct nlist *sp;

	cursym = 0;
	diff = MAXINT;
	if (type == NSYM || symtab == 0)
		return (diff);
	for (sp = symtab; sp < esymtab; sp++) {
		if (!sp->n_un.n_strx)
			continue;
		if (sp->n_type&N_STAB || (sp->n_type&N_EXT)==0)
			continue;
		/*
		 *	Ignore symbols outside kernel.
		 */
		if ((sp->n_value & 0x80000000) == 0)
			continue;
		if (val - sp->n_value < diff && val >= sp->n_value) {
			diff = val - sp->n_value;
			cursym = sp;
			if (diff == 0)
				break;
		}
	}
	return (((int)cursym) ? diff : MAXINT);
}

/*
 * Advance cursym to the next local variable.
 * Leave its value in localval as a side effect.
 * Return 0 at end of file.
 */
localsym(cframe, cargp)
long cframe, cargp;
{
	register struct nlist *sp;

	if (cursym)
		for (sp = cursym; ++sp < esymtab; ) {
			if (!sp->n_un.n_strx)
				continue;
			if (sp->n_un.n_name[0] =='_' || sp->n_type == N_FN)
				return (0);
			switch (sp->n_type) {

			case N_TEXT:
			case N_TEXT|N_EXT:
			case N_DATA:
			case N_DATA|N_EXT:
			case N_BSS:
			case N_BSS|N_EXT:
				localval = sp->n_value;
				cursym = sp;
				return (1);

			case N_LSYM:
				localval = cframe - sp->n_value;
				cursym = sp;
				return (1);

			case N_PSYM:
				/* code below works since n_value > 0 */
			case N_ABS:
				localval = cargp + sp->n_value;
				cursym = sp;
				return (1);
			}
		}
	cursym = 0;
	return (0);
}

/*
 * Print value v and then the string s.
 * If v is not zero, then we look for a nearby symbol
 * and print name+offset if we find a symbol for which
 * offset is small enough.
 *
 * For values which are just into kernel address space
 * that they match exactly or that they be more than maxoff
 * bytes into kernel space.
 */
psymoff(v, type, s)
long v;
int type;
char *s;
{
	long w;

	if (v)
		w = findsym(v, type);
	if (v==0 || w >= maxoff)
			printf(LPRMODE, v);
		else {
			printf("%s", cursym->n_un.n_name);
			if (w)
				printf(OFFMODE, w);
		}
	printf(s);
}

/*
 * Print value v symbolically if it has a reasonable
 * interpretation as name+offset.  If not, print nothing.
 * Used in printing out registers $r.
 */
valpr(v, idsp)
long v;
{
	off_t d;

	d = findsym(v, idsp);
	if (d >= maxoff)
		return;
	printf("%s", cursym->n_un.n_name);
	if (d)
		printf(OFFMODE, d);
}
#endif	wheeze
