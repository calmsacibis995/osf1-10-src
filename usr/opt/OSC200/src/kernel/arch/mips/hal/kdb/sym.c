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
static char	*sccsid = "@(#)$RCSfile: sym.c,v $ $Revision: 1.2 $ (DEC) $Date: 1992/01/15 01:13:02 $";
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
 * derived from sym.c	2.1	(ULTRIX/OSF)	12/3/90";
 */
/*
 * adb-like symtab accesses for COFF files (MIPS compiler)
 */
#include <hal/kdb/defs.h>
#include <hal/kdb/mips_trace.h>
#include <machine/reg.h>
#include <syms.h>

#define IS_DATA(c,t)	((c==scData)||(c==scSData)||(c==scCommon)||(c==scSCommon))
#define IS_BSS(c,t)	((c==scBss)||(c==scSBss))
#define IS_TEXT(c,t)	((c==scText)||(c==scRData))
#define IS_LOCAL(c,t)	(t==stLocal)
#define IS_PARAM(c,t)	(t==stParam)
#define IS_ABS(c,t)	(c==scAbs)

extern HDRR *symtab;

struct nlist *cursym;
unsigned long localval;

char *last_thing_found;
EXTR *last_external_symbol;
PDR  *last_procedure_symbol;
SYMR *last_local_symbol;


/* lookup a symbol by name */
struct nlist *
lookup(symstr)
char *symstr;
{
	static struct nlist ret;	/* one at a time */
	register int i;
	EXTR	*es;
	PDR	*pr;
	SYMR	*sp;
	/*
	 * This is a question possibly with ambiguous answers.
	 * We first look through external symbols, and failing that
	 * we try the procedure table.  If that fails too we
	 * try the local symbols (which is likely to give more or
	 * less bogus answers)
	 */
	if (symtab) {
		for (es = (EXTR*)symtab->cbExtOffset, i = 0;
		     i < symtab->iextMax; i++, es++)
		     	if (strcmp(es->asym.iss, symstr) == 0) {
				ret.n_name  = (char*)es->asym.iss;
				ret.n_type  = es->asym.st;
				ret.reserved = es->asym.sc;
				ret.n_value = es->asym.value;
				last_external_symbol = es;
				last_thing_found = (char*)es;
				return &ret;				
			}
		for (pr = (PDR*)symtab->cbPdOffset, i = 0;
		     i < symtab->ipdMax; i++, pr++)
			/* assume the first local sym describes it */
			sp = (SYMR*)pr->isym;
		     	if (strcmp(sp->iss, symstr) == 0) {
				ret.n_name  = (char*)sp->iss;
				ret.n_type  = sp->st;
				ret.reserved = sp->sc;
				ret.n_value = sp->value;
				last_procedure_symbol = pr;
				last_thing_found = (char*)pr;
				return &ret;				
			}
		for (sp = (SYMR*)symtab->cbSymOffset, i = 0;
		     i < symtab->isymMax; i++, sp++)
		     	if (strcmp(sp->iss, symstr) == 0) {
				ret.n_name  = (char*)sp->iss;
				ret.n_type  = sp->st;
				ret.reserved = sp->sc;
				ret.n_value = sp->value;
				last_local_symbol = sp;
				last_thing_found = (char*)sp;
				return &ret;				
			}
	}
	return 0;
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
	register SYMR *sp;
	register EXTR *es;
	register PDR  *pr;
	int i;
	static struct nlist ret;

	cursym = 0;
	diff = 0x7fffffff;
	if (type == NSYM || symtab == 0)
		return (diff);

	/* See above for default search strategy */
	if (type == PSYM)
		goto proc_only;

	for (es = (EXTR*)symtab->cbExtOffset, i = 0;
	     i < symtab->iextMax; i++, es++) {
		if (val - es->asym.value < diff && val >= es->asym.value) {
			diff = val - es->asym.value;
			ret.n_name  = (char*)es->asym.iss;
			ret.n_type  = es->asym.st;
			ret.reserved = es->asym.sc;
			ret.n_value = es->asym.value;
			cursym = &ret;
			last_external_symbol = es;
			last_thing_found = (char*)es;
			if (diff == 0)
				return 0;
		}
	}

proc_only:
	for (pr = (PDR*)symtab->cbPdOffset, i = 0;
	     i < symtab->ipdMax; i++, pr++) {
		/* assume the first local sym describes it */
		sp = (SYMR*)pr->isym;
		if (val - sp->value < diff && val >= sp->value) {
			diff = val - sp->value;
			ret.n_name  = (char*)sp->iss;
			ret.n_type  = sp->st;
			ret.reserved = sp->sc;
			ret.n_value = sp->value;
			cursym = &ret;
			last_procedure_symbol = pr;
			last_thing_found = (char*)pr;
			if (diff == 0)
				return 0;
		}
	}
	if (type == PSYM)
		return diff;

	for (sp = (SYMR*)symtab->cbSymOffset, i = 0;
	     i < symtab->isymMax; i++, sp++) {
		if (val - sp->value < diff && val >= sp->value) {
			diff = val - sp->value;
			ret.n_name  = (char*)sp->iss;
			ret.n_type  = sp->st;
			ret.reserved = sp->sc;
			ret.n_value = sp->value;
			cursym = &ret;
			last_local_symbol = sp;
			last_thing_found = (char*)sp;
			if (diff == 0)
				break;
		}
	}
	return (diff);
}

/*
 * Advance cursym to the next local variable.
 * Leave its value in localval as a side effect.
 * Return 0 at end of file.
 */
localsym(cframe, cargp)
long cframe, cargp;
{
	register int   sc, st;
	register SYMR *sp;
	register EXTR *es;
	register PDR  *pr;
	static struct nlist ret;

	sp = (SYMR*)last_thing_found;
	es = (EXTR*)sp;
	pr = (PDR*)es;

	while (cursym) {
		if ((EXTR*)last_thing_found == last_external_symbol) {
			if (++es > ((EXTR*)symtab->cbExtOffset + symtab->iextMax))
				return 0;
			ret.n_name = (char*)es->asym.iss;
			ret.n_type = es->asym.st;
			ret.reserved = es->asym.sc;
			ret.n_value = es->asym.value;
			last_thing_found = (char *)es;
		} else if ((PDR*)last_thing_found == last_procedure_symbol) {
			if (++pr > ((PDR*)symtab->cbPdOffset + symtab->ipdMax))
				return 0;
			sp = (SYMR*)pr->isym;
			ret.n_name = (char*)sp->iss;
			ret.n_type = sp->st;
			ret.reserved = sp->sc;
			ret.n_value = sp->value;
			last_thing_found = (char *)pr;
		} else if ((SYMR*)last_thing_found == last_local_symbol) {
			if (++sp > ((SYMR*)symtab->cbSymOffset + symtab->isymMax))
				return 0;
			ret.n_name = (char*)sp->iss;
			ret.n_type = sp->st;
			ret.reserved = sp->sc;
			ret.n_value = sp->value;
			last_thing_found = (char *)sp;
		} else
			return 0;

		sc = ret.reserved;
		st = ret.n_type;

		if (IS_TEXT(sc,st) || IS_DATA(sc,st) || IS_BSS(sc,st)) {
			localval = ret.n_value;
			cursym = &ret;
			return (1);
		}

		if (IS_LOCAL(sc,st)) {
			localval = cframe - ret.n_value;
			cursym = &ret;
			return (1);
		}

		if (IS_PARAM(sc,st) || IS_ABS(sc,st)) {
/* ??? */
			if (ret.n_value < 0)
				localval = cframe + ret.n_value;
			else
				localval = cargp + ret.n_value;
			cursym = &ret;
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
	long            w;

	if (symtab == 0) {
		printf ("%R", v);
		goto ret;
	}

	if (v)
		w = findsym (v, type);

	if (v == 0 || (w >= maxoff && type != PSYM))
		printf ("%R", v);
	else {
		printf ("%s", cursym->n_name);
		if (w)
			printf ("+%R", w);
	}
ret:
	printf (s);
}

/*
 * Print value v symbolically if it has a reasonable
 * interpretation as name+offset.  If not, print nothing.
 * Used in printing out registers $r.
 */
valpr(v, idsp)
long v;
{
	unsigned long   d;

	d = findsym(v, idsp);
	if (d >= maxoff)
		return;
	printf("%s", cursym->n_name);
	if (d)
		printf("+%R", d);
}

/*
 * Print all external symbols, and their values
 */
print_external_symbols()
{
	EXTR	*es;
	int 	 i;
	register int st, sc;

	if (symtab == 0)
		return;

	for (es = (EXTR*)symtab->cbExtOffset, i = 0;
	     i < symtab->iextMax; i++, es++) {
		st = es->asym.st;
		sc = es->asym.sc;
		if (IS_DATA(sc,st) || IS_BSS(sc,st))
		    printf ("%s:%12t%R\n", es -> asym.iss,
			    get (es -> asym.value, DSP));
	}
}

/*
 * Find the descriptor for the function starting at location val,
 * return all we know about it.
 */
findproc(val, fr)
unsigned val;
frame_info_t fr;
{
	register PDR  *pr;
	register SYMR *sp;
	int i,r;

	for (pr = (PDR*)symtab->cbPdOffset, i = 0;
	     i < symtab->ipdMax; i++, pr++) {
		/* assume the first local sym describes it */
		sp = (SYMR*)pr->isym;
		if (val == sp->value)
			goto ret;
	}
	error("\t!!findproc!!\n");
	/*NOTREACHED*/
ret:
#if DEBUG
	printf("[%s: %R %R %R %R : ",
			sp->iss,
			pr->regoffset,
			pr->frameoffset,
			pr->pcreg,
			pr->regmask);
		printf("%R %R]\n", 
			sp->value, 
			sp->index);
#endif
	i = -1;
	do {
		sp++, i++;
#if	DEBUG && notdef
		printf("[%s %R %R %R]\n",
			sp->iss, sp->value, sp->st, sp->sc);
#endif
	} while (sp->st == stParam);
	fr->narg = i;

	/* Leaf procedures do not save the ra */
	fr->isleaf = (pr->regmask & 0x80000000) == 0;
	fr->framereg = pr->framereg;
	fr->framesize = pr->frameoffset;
	fr->saved_pc_off = pr->regoffset;
	if (pr->frameoffset)
		fr->nloc = pr->frameoffset / 4 - i;
	else 
		fr->nloc = 0;
	fr->mod_sp = (pr->frameoffset != 0);

	fr->isvector = ((pr->pcreg == 0) && (fr->framesize == EF_SIZE));

	/*
	 * if the frame register is not SP then we need
	 * to find the offset of where the next fp is
	 * located on the current frame
	 */
	if (fr->framereg != SP_REGISTER) {
		/* initialize to first offset */
		fr->saved_fp_off = pr->regoffset;
		/* account for each register saved until fp */
        	for (r=31; r > fr->framereg; r--) {
			if (pr->regmask & (1<<r))
				fr->saved_fp_off -=4;
		}
	}
#if DEBUG
	printf("\tframe register  : %d\n",fr->framereg);
	printf("\tsaved fp offset : %x\n",fr->saved_fp_off); 
	printf("\tis vector       : %s\n",(fr->isvector?"true":"false"));
#endif
}
