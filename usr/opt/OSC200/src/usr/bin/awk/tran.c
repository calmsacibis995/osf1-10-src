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
static char	*sccsid = "@(#)$RCSfile: tran.c,v $ $Revision: 4.2.7.2 $ (DEC) $Date: 1993/10/01 01:28:34 $";
#endif 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0
 */

/************************************************************************
 *	Modification History
 *      --------------------
 *
 *	31-May-89		Tim N
 *		Added this header.  Changed setfval and setsval to check
 *		top see if the cell is a field and if so to call the
 *		routine num_of_fields() in lib.c which updates the
 *		number of fields if needed.
 ************************************************************************/

#include <stdio.h>
#include <stdio.h>
#include <ctype.h>
#include <locale.h>
#include <nl_types.h>

#include "awk.def"
#include "awk.h"
#include "awk_msg.h"

#define MSGSTR(Num, Str) catgets(catd,NL_SETD, Num, Str)

extern nl_catd catd;

cell *symtab[MAXSYM];	/* symbol table pointers */

char	**FS;	/* initial field sep */
char	**RS;	/* initial record sep */
char	**OFS;	/* output field sep */
char	**ORS;	/* output record sep */
char	**OFMT;	/*output format for numbers*/
awkfloat *NF;	/* number of fields in current record */
awkfloat *NR;	/* number of current record */
char	**FILENAME;	/* current filename argument */

cell	*recloc;	/* location of record */
cell	*nrloc;		/* NR */
cell	*nfloc;		/* NF */


syminit()
{
	setsymtab("0", tostring("0"), 0.0, NUM|STR|CON|FLD, symtab);
	/* this one is used for if(x)... tests: */
	setsymtab("$zero&null", tostring(""), 0.0, NUM|STR|CON|FLD, symtab);
	recloc = setsymtab("$record", record, 0.0, STR|FLD, symtab);
	dprintf("recloc %o lookup %o\n", recloc, lookup("$record", symtab, 0), NULL);
	FS = &setsymtab("FS", tostring(" "), 0.0, STR|FLD, symtab)->sval;
	RS = &setsymtab("RS", tostring("\n"), 0.0, STR|FLD, symtab)->sval;
	OFS = &setsymtab("OFS", tostring(" "), 0.0, STR|FLD, symtab)->sval;
	ORS = &setsymtab("ORS", tostring("\n"), 0.0, STR|FLD, symtab)->sval;
	OFMT = &setsymtab("OFMT", tostring("%.6g"), 0.0, STR|FLD, symtab)->sval;
	FILENAME = &setsymtab("FILENAME", EMPTY, 0.0, STR|FLD, symtab)->sval;
	nfloc = setsymtab("NF", EMPTY, 0.0, NUM, symtab);
	NF = &nfloc->fval;
	nrloc = setsymtab("NR", EMPTY, 0.0, NUM, symtab);
	NR = &nrloc->fval;
}

cell **makesymtab()
{
	int i;
	cell **cp;

	cp = (cell **) malloc(MAXSYM * sizeof(cell *));
	if (cp == NULL)
		error(FATAL, MSGSTR(NOSPACE1, "out of space in makesymtab"));

	for (i = 0; i < MAXSYM; i++)
		cp[i] = 0;
	return(cp);
}

freesymtab(cell *ap)	/* free symbol table */
{
	cell *cp, **tp;
	int i;

	if (!(ap->tval & ARR))
		return;
	tp = (cell **) ap->sval;
	for (i = 0; i < MAXSYM; i++) {
		for (cp = tp[i]; cp != NULL; cp = cp->nextval) {
			strfree(cp->nval);
			strfree(cp->sval);
			free(cp);
		}
	}
	xfree(tp);
}

cell *setsymtab(char *n, char *s, awkfloat f, unsigned t, cell **tab)
{
	register h;
	register cell *p;
	cell *lookup();

	if (n != NULL && (p = lookup(n, tab, 0)) != NULL) {
		if (s != EMPTY ) xfree(s); /* careful here */
		dprintf("setsymtab found %o: %s", p, p->nval, NULL);
		dprintf(" %s %g %o\n", p->sval, p->fval, p->tval);
		return(p);
	}
	p = (cell *) malloc(sizeof(cell));
	if (p == NULL)
		error(FATAL,
                     MSGSTR(SYMTABOF, "symbol table overflow at %s"), n);
	p->nval = tostring(n);
	p->sval = s;
	p->fval = f;
	p->tval = t;
	p->field_num = 0;
	h = hash(n);
	p->nextval = tab[h];
	tab[h] = p;
	dprintf("setsymtab set %o: %s", p, p->nval, NULL);
	dprintf(" %s %g %o\n", p->sval, p->fval, p->tval);
	return(p);
}

hash(char *s)	/* form hash value for string s */
{
	register int hashval;

	for (hashval = 0; *s != '\0'; )
		hashval += *s++;
	return(hashval % MAXSYM);
}

/* look for s in tab, flag must match*/
cell *lookup(register char *s, cell **tab, int flag)
{
	register cell *p;

	for (p = tab[hash(s)]; p != NULL; p = p->nextval)
		if (strcmp(s, p->nval) == 0 &&
			(flag == 0 || flag == p->tval))
			return(p);	/* found it */
	return(NULL);	/* not found */
}

awkfloat setfval(register cell *vp, awkfloat f)
{
	extern void num_of_fields();

	dprintf("setfval: %o %g\n", vp, f, NULL);
	checkval(vp);
	if (vp == recloc)
		error(FATAL, "can't set $0");
	vp->tval &= ~STR;	/* mark string invalid */
	vp->tval |= NUM;	/* mark number ok */
	if ((vp->tval & FLD) && isnull(vp->nval))
		donerec = 0;
	if (vp->tval & FLD)
		num_of_fields(vp->field_num);
	return(vp->fval = f);
}

char *setsval( register cell *vp, char *s)
{
	extern void num_of_fields();

	dprintf("setsval: %o %s\n", vp, s, NULL);
	checkval(vp);
	if (vp == recloc)
		error(FATAL, MSGSTR(CANTSET, "can't set $0"));
	vp->tval &= ~NUM;
	vp->tval |= STR;
	if ((vp->tval & FLD) && isnull(vp->nval))
		donerec = 0;
	if (vp->tval & FLD)
		num_of_fields(vp->field_num);
	if (!(vp->tval&FLD))
		strfree(vp->sval);
	vp->tval &= ~FLD;
	return(vp->sval = tostring(s));
}

awkfloat getfval( register cell *vp)
{

	if (vp->sval == record && donerec == 0)
		recbld();
	dprintf("getfval: %o", vp, NULL, NULL);
	checkval(vp);
	if ((vp->tval & NUM) == 0) {
		/* the problem is to make non-numeric things */
		/* have unlikely numeric variables, so that */
		/* $1 == $2 comparisons sort of make sense when */
		/* one or the other is numeric */
		if (isnumber(vp->sval)) {
			vp->fval = atof(vp->sval);
			if (!(vp->tval & CON))	/* don't change type of a constant */
				vp->tval |= NUM;
		}
		else
			vp->fval = 0.0;	/* not a very good idea */
	}
	dprintf("  %g\n", vp->fval, NULL, NULL);
	return(vp->fval);
}

char *getsval( register cell *vp)
{
	char s[100];

	if (vp->sval == record && donerec == 0)
		recbld();
	dprintf("getsval: %o", vp, NULL, NULL);
	checkval(vp);
	if ((vp->tval & STR) == 0) {
		if (!(vp->tval&FLD))
			strfree(vp->sval);
		if ((long)vp->fval==vp->fval)
			(void)sprintf(s, "%.20g", vp->fval);
		else
			(void)sprintf(s, *OFMT, vp->fval);
		vp->sval = tostring(s);
		vp->tval &= ~FLD;
		vp->tval |= STR;
	}
	dprintf("  %s\n", vp->sval, NULL, NULL);
	return(vp->sval);
}

checkval( register cell *vp)
{
	if (vp->tval & ARR)
		error(FATAL,
                MSGSTR(ILLREFARRAY,"illegal reference to array %s"), vp->nval);
	if ((vp->tval & (NUM | STR)) == 0)
		error(FATAL, 
                      MSGSTR(FUNNYVAR,"funny variable %o: %s %s %g %o"),
                      vp, vp->nval, vp->sval, vp->fval, vp->tval);
}

char *tostring( register char *s)
{
	register char *p;

	if (s==NULL){
		p = (char *)malloc(1);
		if (p == NULL)
			error(FATAL, 
                        MSGSTR(TOSTRING, "out of space in tostring on %s"), s);
		*p = '\0';
	} else {
		p = (char *)malloc(strlen(s)+1);
		if (p == NULL)
			error(FATAL, 
                        MSGSTR(TOSTRING, "out of space in tostring on %s"), s);
		strcpy(p, s);
	}
	return(p);
}
#ifndef yfree
yfree(char *a)
{
	printf("%o\n", a);
	free(a);
}
#endif
#ifdef malloc
#undef malloc
char *ymalloc(unsigned u)
{	char *p;
	p = (char *) malloc(u);
	printf("%o %o\n", u, p);
	return(p);
}
#endif
