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
static char	*sccsid = "@(#)$RCSfile: tftn.c,v $ $Revision: 4.2.4.3 $ (DEC) $Date: 1993/11/22 21:22:55 $";
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

/*
 * COMPONENT_NAME: (CMDPROG) tftn.c
 *
 * FUNCTIONS: FindBType, InitTypes, btype, comtypes, copyparms, copytype     
 *            defaultproto, incref, mkcomposite, modtype, newty, parmalloc    
 *            qualmember, qualtype, sameproto, setty, signedtype, tyencode    
 *            tynalloc, unqualtype                                            
 *
 * ORIGINS: 27 03 09 32 00 
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 * OBJECT CODE ONLY SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Changes for ANSI C were developed by HCR Corporation for IBM
 * Corporation under terms of a work made for hire contract.
 */

/* AIWS C compiler */
# include "mfile1.h"
# include "messages.h"

#define NUMBTYPES	(NRBTYPES+1)

struct tyinfo btypes[] = {
	{ TNULL, TNIL },
	{ TELLIPSIS, TNIL },
	{ FARG, TNIL },
	{ MOETY, TNIL },
	{ SIGNED, TNIL },
	{ UNDEF, TNIL },

	/*
	** "Real" basic types start here.
	*/

	{ TVOID, TNIL },
	{ CHAR, TNIL },
	{ SCHAR, TNIL },
	{ SHORT, TNIL },
	{ INT, TNIL },
	{ LONG, TNIL },
	{ LLONG, TNIL },
	{ FLOAT, TNIL },
	{ DOUBLE, TNIL },
	{ LDOUBLE, TNIL },
	{ STRTY, TNIL },
	{ UNIONTY, TNIL },
	{ ENUMTY, TNIL },
	{ UCHAR, TNIL },
	{ USHORT, TNIL },
	{ UNSIGNED, TNIL },
	{ ULONG, TNIL },
	{ ULLONG, TNIL },

	{ TSIGNED|INT, TNIL },			/* Fake type */
	{ CONST|TVOID, TNIL },
	{ CONST|CHAR, TNIL },
	{ CONST|SCHAR, TNIL },
	{ CONST|SHORT, TNIL },
	{ CONST|INT, TNIL },
	{ CONST|LONG, TNIL },
	{ CONST|LLONG, TNIL },
	{ CONST|FLOAT, TNIL },
	{ CONST|DOUBLE, TNIL },
	{ CONST|LDOUBLE, TNIL },
	{ CONST|STRTY, TNIL },
	{ CONST|UNIONTY, TNIL },
	{ CONST|ENUMTY, TNIL },
	{ CONST|UCHAR, TNIL },
	{ CONST|USHORT, TNIL },
	{ CONST|UNSIGNED, TNIL },
	{ CONST|ULONG, TNIL },
	{ CONST|ULLONG, TNIL },

	{ CONST|TSIGNED|INT, TNIL },		/* Fake type */

	{ VOLATILE|TVOID, TNIL },
	{ VOLATILE|CHAR, TNIL },
	{ VOLATILE|SCHAR, TNIL },
	{ VOLATILE|SHORT, TNIL },
	{ VOLATILE|INT, TNIL },
	{ VOLATILE|LONG, TNIL },
	{ VOLATILE|LLONG, TNIL },
	{ VOLATILE|FLOAT, TNIL },
	{ VOLATILE|DOUBLE, TNIL },
	{ VOLATILE|LDOUBLE, TNIL },
	{ VOLATILE|STRTY, TNIL },
	{ VOLATILE|UNIONTY, TNIL },
	{ VOLATILE|ENUMTY, TNIL },
	{ VOLATILE|UCHAR, TNIL },
	{ VOLATILE|USHORT, TNIL },
	{ VOLATILE|UNSIGNED, TNIL },
	{ VOLATILE|ULONG, TNIL },
	{ VOLATILE|ULLONG, TNIL },

	{ VOLATILE|TSIGNED|INT, TNIL },		/* Fake type */

	{ UNALIGNED|TVOID, TNIL },
	{ UNALIGNED|CHAR, TNIL },
	{ UNALIGNED|SCHAR, TNIL },
	{ UNALIGNED|SHORT, TNIL },
	{ UNALIGNED|INT, TNIL },
	{ UNALIGNED|LONG, TNIL },
	{ UNALIGNED|LLONG, TNIL },
	{ UNALIGNED|FLOAT, TNIL },
	{ UNALIGNED|DOUBLE, TNIL },
	{ UNALIGNED|LDOUBLE, TNIL },
	{ UNALIGNED|STRTY, TNIL },
	{ UNALIGNED|UNIONTY, TNIL },
	{ UNALIGNED|ENUMTY, TNIL },
	{ UNALIGNED|UCHAR, TNIL },
	{ UNALIGNED|USHORT, TNIL },
	{ UNALIGNED|UNSIGNED, TNIL },
	{ UNALIGNED|ULONG, TNIL },
	{ UNALIGNED|ULLONG, TNIL },

	{ CONST|UNALIGNED|TSIGNED|INT, TNIL },		/* Fake type */
	{ CONST|UNALIGNED|TVOID, TNIL },
	{ CONST|UNALIGNED|CHAR, TNIL },
	{ CONST|UNALIGNED|SCHAR, TNIL },
	{ CONST|UNALIGNED|SHORT, TNIL },
	{ CONST|UNALIGNED|INT, TNIL },
	{ CONST|UNALIGNED|LONG, TNIL },
	{ CONST|UNALIGNED|LLONG, TNIL },
	{ CONST|UNALIGNED|FLOAT, TNIL },
	{ CONST|UNALIGNED|DOUBLE, TNIL },
	{ CONST|UNALIGNED|LDOUBLE, TNIL },
	{ CONST|UNALIGNED|STRTY, TNIL },
	{ CONST|UNALIGNED|UNIONTY, TNIL },
	{ CONST|UNALIGNED|ENUMTY, TNIL },
	{ CONST|UNALIGNED|UCHAR, TNIL },
	{ CONST|UNALIGNED|USHORT, TNIL },
	{ CONST|UNALIGNED|UNSIGNED, TNIL },
	{ CONST|UNALIGNED|ULONG, TNIL },
	{ CONST|UNALIGNED|ULLONG, TNIL },

	{ CONST|UNALIGNED|TSIGNED|INT, TNIL },		/* Fake type */

	{ VOLATILE|UNALIGNED|TVOID, TNIL },
	{ VOLATILE|UNALIGNED|CHAR, TNIL },
	{ VOLATILE|UNALIGNED|SCHAR, TNIL },
	{ VOLATILE|UNALIGNED|SHORT, TNIL },
	{ VOLATILE|UNALIGNED|INT, TNIL },
	{ VOLATILE|UNALIGNED|LONG, TNIL },
	{ VOLATILE|UNALIGNED|LLONG, TNIL },
	{ VOLATILE|UNALIGNED|FLOAT, TNIL },
	{ VOLATILE|UNALIGNED|DOUBLE, TNIL },
	{ VOLATILE|UNALIGNED|LDOUBLE, TNIL },
	{ VOLATILE|UNALIGNED|STRTY, TNIL },
	{ VOLATILE|UNALIGNED|UNIONTY, TNIL },
	{ VOLATILE|UNALIGNED|ENUMTY, TNIL },
	{ VOLATILE|UNALIGNED|UCHAR, TNIL },
	{ VOLATILE|UNALIGNED|USHORT, TNIL },
	{ VOLATILE|UNALIGNED|UNSIGNED, TNIL },
	{ VOLATILE|UNALIGNED|ULONG, TNIL },
	{ VOLATILE|UNALIGNED|ULLONG, TNIL },

	{ VOLATILE|UNALIGNED|TSIGNED|INT, TNIL },		/* Fake type */

	{ CONST|VOLATILE|TVOID, TNIL },
	{ CONST|VOLATILE|CHAR, TNIL },
	{ CONST|VOLATILE|SCHAR, TNIL },
	{ CONST|VOLATILE|SHORT, TNIL },
	{ CONST|VOLATILE|INT, TNIL },
	{ CONST|VOLATILE|LONG, TNIL },
	{ CONST|VOLATILE|LLONG, TNIL },
	{ CONST|VOLATILE|FLOAT, TNIL },
	{ CONST|VOLATILE|DOUBLE, TNIL },
	{ CONST|VOLATILE|LDOUBLE, TNIL },
	{ CONST|VOLATILE|STRTY, TNIL },
	{ CONST|VOLATILE|UNIONTY, TNIL },
	{ CONST|VOLATILE|ENUMTY, TNIL },
	{ CONST|VOLATILE|UCHAR, TNIL },
	{ CONST|VOLATILE|USHORT, TNIL },
	{ CONST|VOLATILE|UNSIGNED, TNIL },
	{ CONST|VOLATILE|ULONG, TNIL },
	{ CONST|VOLATILE|ULLONG, TNIL },
	{ CONST|VOLATILE|TSIGNED|INT, TNIL },	/* Fake type */

	{ UNALIGNED|CONST|VOLATILE|TVOID, TNIL },
	{ UNALIGNED|CONST|VOLATILE|CHAR, TNIL },
	{ UNALIGNED|CONST|VOLATILE|SCHAR, TNIL },
	{ UNALIGNED|CONST|VOLATILE|SHORT, TNIL },
	{ UNALIGNED|CONST|VOLATILE|INT, TNIL },
	{ UNALIGNED|CONST|VOLATILE|LONG, TNIL },
	{ UNALIGNED|CONST|VOLATILE|LLONG, TNIL },
	{ UNALIGNED|CONST|VOLATILE|FLOAT, TNIL },
	{ UNALIGNED|CONST|VOLATILE|DOUBLE, TNIL },
	{ UNALIGNED|CONST|VOLATILE|LDOUBLE, TNIL },
	{ UNALIGNED|CONST|VOLATILE|STRTY, TNIL },
	{ UNALIGNED|CONST|VOLATILE|UNIONTY, TNIL },
	{ UNALIGNED|CONST|VOLATILE|ENUMTY, TNIL },
	{ UNALIGNED|CONST|VOLATILE|UCHAR, TNIL },
	{ UNALIGNED|CONST|VOLATILE|USHORT, TNIL },
	{ UNALIGNED|CONST|VOLATILE|UNSIGNED, TNIL },
	{ UNALIGNED|CONST|VOLATILE|ULONG, TNIL },
	{ UNALIGNED|CONST|VOLATILE|ULLONG, TNIL },
	{ UNALIGNED|CONST|VOLATILE|TSIGNED|INT, TNIL },	/* Fake type */
};

#define NTABTYPES	(sizeof(btypes) / sizeof(struct tyinfo))
extern void printtyinfo( TPTR typtr, int flag);

static TPTR curGlobTy;	/* Next global type to be allocated */
static TPTR curLoclTy;	/* Next local type to be allocated */

InitTypes ()
{
    register TPTR t;

    curGlobTy = globTyTab = (TPTR)getmem(nGlobTyEnts * sizeof(struct tyinfo));
    curLoclTy = loclTyTab = (TPTR)getmem(nLoclTyEnts * sizeof(struct tyinfo));

    for (t = &btypes[0]; t < &btypes[NTABTYPES]; t++) {
	t->typ_size = TOPTYPE(t);
    }

    labltype.tword = ARY;
    labltype.next = tyalloc(INT);
    labltype.ary_size = 0;
}


TPTR
btype (t)
    register TPTR t;
{
    while (!ISBTYPE(t)) {
	t = t->next;
    }
    return (t);
}


#if	!defined (LINTP2) && !defined (CFLOW2)
int
comtypes(t1, t2, o)
    register TPTR t1, t2;
    int o;
{
    /*
    ** This routine makes sure that the types t1 and t2 are compatible.
    ** Error messages assuming that these are dereferenced pointer types
    ** are emitted when o!=0 and has the value of the operation performed.
    ** If o!=0, the qualifiers of the top type are not checked.
    */
    char *operation;

    if (o != 0)
	operation = opst[o];
    else if (QUALIFIERS(t1) != QUALIFIERS(t2))
	    return (0);

    if (TOPTYPE(t1) != TOPTYPE(t2)) {
	if ( devdebug[COMPATIBLE] &&
		(TOPTYPE(t1) == INT && TOPTYPE(t2) == ENUMTY) ||
		(TOPTYPE(t2) == INT && TOPTYPE(t1) == ENUMTY))
	    return (1);

	if (o != 0)
	    /* "illegal pointer combination, op %s" */
	    WERROR( devdebug[COMPATIBLE], MESSAGE(66), operation );
	return (0);
    }

    while (!ISBTYPE(t1)) {
	switch (TOPTYPE(t1)) {

	case FTN:
	    if (!sameproto(t1, t2)) {
		if (o != 0) {
		    /* "incompatible function prototype combination, op %s" */
#ifdef LINT
		  if (AL_MIGCHK && AL_PROTO) AL_PRINTIT = 1;
#endif
		    WERROR( devdebug[COMPATIBLE], MESSAGE(172), operation );
		  };
		return(0);
	    }
	    break;

	case ARY:
	    if (t1->ary_size != 0 && t2->ary_size != 0 &&
		    t1->ary_size != t2->ary_size) {
		if (o != 0)
		    /* "illegal array size combination, op %s" */
		    WERROR( devdebug[COMPATIBLE], MESSAGE(49), operation );
		return (0);
	    }
	    break;
	case PTR:
	    /* Check for compatible types between void * and incomplete
		or object types. Sec. 3.2.2.3 of ANSI Standard */
	    if ((TOPTYPE(t1->next) == TVOID && TOPTYPE(t2->next) != FTN) ||
		(TOPTYPE(t2->next) == TVOID && TOPTYPE(t1->next) != FTN))
		return(1);
	    break;
	}

	t1 = t1->next;
	t2 = t2->next;

	if (TOPQTYPE(t1) != TOPQTYPE(t2)) {
	    if  ( devdebug[COMPATIBLE] && QUALIFIERS(t1) == QUALIFIERS(t2) &&
		    ((TOPTYPE(t1) == INT && TOPTYPE(t2) == ENUMTY) ||
		    (TOPTYPE(t2) == INT && TOPTYPE(t1) == ENUMTY)))
		return (1);
	    if (o != 0)
		/* "illegal pointer combination, op %s" */
		WERROR( devdebug[COMPATIBLE], MESSAGE(66), operation );
	    return (0);
	}
    }

    if (t1->typ_size != t2->typ_size) {
	if (o != 0) {
	    if (TOPTYPE(t1) == ENUMTY && TOPTYPE(t2) == ENUMTY)
		/* "enumeration type clash, op %s" */
		WERROR( devdebug[COMPATIBLE], MESSAGE(37), operation );
	    else {
#ifdef    LINT
	        if ((AL_MIGCHK && AL_STRPTR)) AL_PRINTIT = 1;
#endif
		/* "illegal structure type combination, op %s" */
		WERROR( devdebug[COMPATIBLE], MESSAGE(69), operation );
	      }
	}
	return (0);
    }

    return (1);
}

int
sameproto (t1,t2)
    register TPTR t1,t2;
{
	register PPTR p1,p2;


	if ( t1->ftn_parm == PNIL && t2->ftn_parm == PNIL ) {
		/* neither type has a prototype list, don't
		 * check anything.
		 */
		return ( 1 );
	}

	/* if either type has an empty prototype list, check
	 * the other to see that each entry is compatible with its default
	 * promotion type.
	 */
	if ( ( t1->ftn_parm == PNIL && t2->ftn_parm != PNIL ) ||
		( t2->ftn_parm == PNIL && t1->ftn_parm != PNIL ) )
		/* "mix of old and new style function declaration" */
		WARNING( WPROTO, MESSAGE(178) );

	if ( t1->ftn_parm == PNIL ) {
#ifdef COMPAT
/************************************************************************
** NOTE:  This must not be used if the compiler is modified to pass	*
** <4 byte parameters, or else bad code will result!			*
************************************************************************/
		return ( ( devdebug[KLUDGE] && !devdebug[COMPATIBLE] ) ||
			defaultproto(t2->ftn_parm) );
#else
		return ( defaultproto(t2->ftn_parm) );
#endif
	}

	if ( t2->ftn_parm == PNIL ) {
#ifdef COMPAT
/************************************************************************
** NOTE:  This must not be used if the compiler is modified to pass	*
** <4 byte parameters, or else bad code will result!			*
************************************************************************/
		return ( ( devdebug[KLUDGE] && !devdebug[COMPATIBLE] ) ||
			defaultproto(t1->ftn_parm) );
#else
		return ( defaultproto(t1->ftn_parm) );
#endif
	}

	/* check each entry on both lists to be sure they are
	 * compatible.  If not return a failure for the function.
	 */
	for (p1 = t1->ftn_parm, p2 = t2->ftn_parm ;
	     p1 != PNIL && p2 != PNIL;
	     p1 = p1->next, p2 = p2->next) {
		if (!comtypes(p1->type,p2->type,0)) return(0);
	}

	/* if both parameter lists don't end simultaneously, the
	 * number of arguments is mismatched.
	 */
	if ( p1 == PNIL && p2 == PNIL) {
		return(1);
	}
	else {
		return(0);
	}
}

int
tyencode (t)
    register TPTR t;
{
    register int result = 0;
    int shift = BTSHIFT;

    while (!ISBTYPE(t)) {
	switch (TOPTYPE(t)) {
	case PTR:
	    result |= PTROUT << shift;
	    break;
	case FTN:
	    result |= FTNOUT << shift;
	    break;
	case ARY:
	    result |= ARYOUT << shift;
	    break;
	}
	shift += TSHIFT;
	t = t->next;
    }
    switch (TOPTYPE(t)) {
    case UNDEF:
	return (result | 0);
    case FARG:
	return (result | 1);
    case SCHAR:
	return (result | 2);
    case SHORT:
	return (result | 3);
    case LONG:
    case LLONG:
#if SZLONG != SZINT
	return (result | 5);
#endif
    case INT:
	return (result | 4);
    case FLOAT:
	return (result | 6);
    case DOUBLE:
    case LDOUBLE:
	return (result | 7);
    case STRTY:
	return (result | 8);
    case UNIONTY:
	return (result | 9);
    case ENUMTY:
	return (result | 10);
    case MOETY:
	return (result | 11);
    case UCHAR:
    case CHAR:
	return (result | 12);
    case USHORT:
	return (result | 13);
    case ULONG:
    case ULLONG:
#if SZLONG != SZINT
	return (result | 15);
#endif
    case UNSIGNED:
	return (result | 14);
    default:
	return (result);
    }
}
#endif

int
defaultproto(p)
register PPTR p;
{
	/* use default argument promotion on each basic
	 * type encountered on the prototype list and issue an
	 * error if it is incompatible with its original type or
	 * an ellipsis is encountered (see section 3.5.4.3).
	 *
	 * In practice, this means we issue an error if
	 * the basic type is char, short, float or ellipsis.
	 */

	for (/*null*/; p != PNIL; p = p->next) {
		switch ( TOPTYPE( p->type ) ) {
		      case CHAR:
		      case UCHAR:
		      case SCHAR:
		      case SHORT:
		      case USHORT:
		      case FLOAT:
		      case TELLIPSIS:
			return ( 0 );
		      default:
			break;
		}
	}

	return ( 1 );
}


static TPTR tyrec[BNEST+1];		/* keep track of block nesting levels */
static int maxLevel = 0;		/* maximum defined level */

void
setty ()
{
    if (paramlevel != 0)
	cerror(TOOLSTR(M_MSG_207, "outstanding prototype list at type table mark" ));
    if (blevel > BNEST+1)
	UERROR( ALWAYS, TOOLSTR(M_MSG_320, "block nesting too deep") );
    else if (blevel <= maxLevel)
	curLoclTy = tyrec[blevel-1];
    else if (blevel > maxLevel + 1)
	cerror(TOOLSTR(M_MSG_208, "missing type table mark" ));
    else
	tyrec[blevel-1] = curLoclTy;
    maxLevel = blevel;
}

static TPTR
newty (level)
    int level;
{
    level -= paramlevel;
    if (level <= 0) {
	/* Allocate from global table */
	if (curGlobTy >= &globTyTab[nGlobTyEnts]) {
	    globTyTab = (TPTR)getmem(nGlobTyEnts * sizeof(struct tyinfo));
	    curGlobTy = globTyTab;
	}
	return (curGlobTy++);
    } else {
	/* Allocate from local table */
	if (level != blevel-paramlevel) cerror(TOOLSTR(M_MSG_209, "illegal type table level" ));
	if (level < maxLevel) {
	    maxLevel = level;
	    curLoclTy = tyrec[level];
	}
	if (curLoclTy >= &loclTyTab[nLoclTyEnts]) {
	    cerror(TOOLSTR(M_MSG_210,
#ifndef LINT
		"out of type nodes; recompile with -Nlx option with x greater than %d"),
#else
		"out of type nodes; use lint  -Nlx option with x greater than %d"),
#endif
		    nLoclTyEnts );
	}
	return (curLoclTy++);
    }
}


TPTR
tynalloc (bt)
    TWORD bt;
{
    register TPTR t;

    t = newty(blevel);
    t->tword = bt;
    t->next = TNIL;

    switch (bt) {
    case ARY:
    case PTR:
	t->ary_size = 0;
	break;
    case FTN:
	t->ftn_parm = PNIL;
	break;
    default:
	t->typ_size = bt & BTMASK;
	break;
    }

    return (t);
}


TPTR
incref (t, bt)
    TPTR t;
    TWORD bt;
{
    TPTR newt;

    newt = tynalloc(bt);
    newt->next = t;
    return (newt);
}


TPTR
modtype (t, bt)
    TPTR t;
    TWORD bt;
{
    register TPTR *t2 = &t;
    register TPTR t1 = t;
    register TPTR tNew;

    while (!ISBTYPE(t1)) {
	*t2 = tNew = tynalloc(t1->tword);
	tNew->info = t1->info;
	t1 = t1->next;
	t2 = &tNew->next;
    }

    tNew = tyalloc(bt);
    if (QUALIFIERS(t1)) {
	tNew = qualtype(tNew, QUALIFIERS(t1), 0);
    }
    *t2 = tNew;

    return (t);
}


TPTR
qualtype (t, qt, docopy)
    register TPTR t;
    TWORD qt;
    int docopy;
{
    if (t >= &btypes[TVOID] && t < &btypes[NTABTYPES]) {
	/*
	** It's a table entry.
	** ASSUME IT'S NOT ALREADY SO QUALIFIED!
	*/
	if (qt & CONST) {
	    t += NUMBTYPES;
	}
	if (qt & VOLATILE) {
	    t += 2*NUMBTYPES;
	}
	if (qt & UNALIGNED) {
	    t += 3*NUMBTYPES;
	}
	return (t);
    } else {
	/*
	** It's a constructed type.
	*/
	TPTR t2;

	/* Check if we have to copy the top node. */
	if (docopy) {
	    t2 = tynalloc(t->tword | qt);
	    t2->info = t->info;
	    t2->next = t->next;
	    return (t2);
	} else {
	    t->tword |= qt;
	    return (t);
	}
    }
}


TPTR
unqualtype (t)
    register TPTR t;
{
    if (QUALIFIERS(t) == 0) {
	return (t);
    }

    if (t >= &btypes[TVOID] && t < &btypes[NTABTYPES]) {
	/*
	** It's a table entry.
	*/
	if (ISCONST(t)) {
	    t -= NUMBTYPES;
	}
	if (ISVOLATILE(t)) {
	    t -= 2*NUMBTYPES;
	}
	if (ISUNALIGNED(t)) {
	    t += 3*NUMBTYPES;
	}
	return (t);
    } else {
	/*
	** It's a constructed type.
	*/
	TPTR t2;

	/* Assume we have to copy the top node. */
	t2 = tynalloc(t->tword & ~(CONST|VOLATILE|UNALIGNED));
	t2->info = t->info;
	t2->next = t->next;
	return (t2);
    }
}


TPTR
signedtype (t)
    register TPTR t;
{
    /*
    ** Assume this is called only once per type.
    */
    if (TOPTYPE(t) == INT) {
	/* Only "signed int" is meaningfully different. */
	t += NBTYPES - INT;
    }
    return (t);
}


TPTR
qualmember (t, qt)
    register TPTR t;
    TWORD qt;
{
    register TPTR newt;

    if (t >= &btypes[0] && t < &btypes[NTABTYPES]) {
	/*
	** It's a table entry.  This shouldn't happen.
	*/
	cerror(TOOLSTR(M_MSG_211, "trying to qualify generic structure" ));
    }
    t->tword |= qt;
    return (t);
}


TPTR
copytype (t, level)
    TPTR t;
    int level;
{
    register TPTR *t2 = &t;
    register TPTR t1 = t;
    register TPTR tNew;
    extern PPTR copyparms();

    while (!ISBTYPE(t1)) {
	*t2 = tNew = newty(level);
	tNew->tword = t1->tword;
	switch (TOPTYPE(t1)) {
	case FTN:
	    tNew->ftn_parm = copyparms(t1->ftn_parm, level);
	    break;
	case ARY:
	    tNew->ary_size = t1->ary_size;
	    break;
	}
	t1 = t1->next;
	t2 = &tNew->next;
    }

    if (level != blevel && (TOPTYPE(t1) == STRTY ||
	    TOPTYPE(t1) == UNIONTY || TOPTYPE(t1) == ENUMTY)) {
	*t2 = tNew = newty(level);
	tNew->tword = t1->tword;
	tNew->typ_size = t1->typ_size;
	tNew->next = TNIL;
    } else {
	*t2 = t1;
    }

    return (t);
}


int
mkcomposite (t, t2, level)
    register TPTR t;
    register TPTR t2;
    int level;
{
    /*
    ** Merge t2 into t to get the composite type.
    ** ASSUME THE TYPES ARE COMPATIBLE.
    ** Return whether the new type had less information than the original.
    */
    register PPTR p, p2;
    extern PPTR copyparms();
    int lostInfo = 0;

    while (!ISBTYPE(t)) {
	switch (TOPTYPE(t)) {

	case FTN:
	    p = t->ftn_parm;
	    p2 = t2->ftn_parm;
	    if (p == PNIL) {
#ifdef COMPAT
		if (devdebug[KLUDGE] && !defaultproto(p2)) {
			/* "prototype not compatible with non-prototype declaration" */
#ifdef LINT
		  if (AL_MIGCHK && AL_PROTO) AL_PRINTIT = 1;
#endif
			WERROR( ALWAYS, MESSAGE(194) );
		};
#endif
		t->ftn_parm = copyparms(p2, level);
	    } else if (p2 != PNIL) {
		while (p != PNIL) {
		    if (mkcomposite(p->type, p2->type, level)) {
			lostInfo = 1;
		    }
		    p = p->next;
		    p2 = p2->next;
		}
	    } else {
#ifdef COMPAT
		if (devdebug[KLUDGE] && !defaultproto(p)) {
			/* "prototype not compatible with non-prototype declaration" */
#ifdef LINT
		  if (AL_MIGCHK && AL_PROTO) AL_PRINTIT = 1;
#endif
			WERROR( ALWAYS, MESSAGE(194) );
		};
#endif
		lostInfo = 1;
	    }
	    break;

	case ARY:
	    if (t2->ary_size != 0) {
		t->ary_size = t2->ary_size;
	    } else if (t->ary_size != 0) {
		lostInfo = 1;
	    }
	    break;
	}

	t = t->next;
	t2 = t2->next;
    }

    return (lostInfo);
}

PPTR
parmalloc ()
{
    static int bunchsize = 0;
    static PPTR parmbunch;

    if (bunchsize == 0) {
	/* Allocate another bunch of type nodes */
	parmbunch = (PPTR)getmem(MAXBUNCH * sizeof(struct parminfo));
	bunchsize = MAXBUNCH;
    }
    return (&parmbunch[--bunchsize]);
}

PPTR
copyparms (p, level)
	PPTR p;
	int level;
{
	register PPTR *p2 = &p;
	register PPTR p1 = p;
	register PPTR pNew;

	while (p1 != PNIL) {
		*p2 = pNew = parmalloc();
		pNew->type = copytype(p1->type, level);
		p1 = p1->next;
		p2 = &pNew->next;
	}
	*p2 = PNIL;

	return (p);
}
static int printtyinfo_recurse = 0;
void printtyinfo( TPTR typtr, int flag)
{
  printf("Tyinfo at %lx\n",typtr);
  printf("\t\t tword: %d, Spacer %d, next: %lx\n",typtr->tword,typtr->spacer32,typtr->next);
  printf("\t\t\t info: %lx\n",typtr->info.parms);
  if ((unsigned long)typtr->info.parms > 0xffffffffUL) {
    if (flag) {
      printf("\t\t\t\t Parminfo next:%lx Type %lx\n",typtr->info.parms->next,typtr->info.parms->type);
      if (((unsigned long)typtr->info.parms->type  > 0xffffffffUL) && printtyinfo_recurse) {
	printf ("\n\t\t\t\t ****** Recursing on %lx *********\n",typtr->info.parms->type);
	printtyinfo(typtr->info.parms->type,flag);
      }
    }
  }
} 
TPTR
FindBType(bt)
	TWORD bt;
{
	register int i;

	for (i = 0; i < NTABTYPES; i++)
		if (btypes[i].tword == bt)
			return (&btypes[i]);
	return (TNIL);
}

