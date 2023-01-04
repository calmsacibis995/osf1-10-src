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
static char	*sccsid = "@(#)$RCSfile: lint2.c,v $ $Revision: 4.3.7.3 $ (DEC) $Date: 1993/12/17 21:07:08 $";
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
#if !defined(lint) && !defined(_NOIDENT)

#endif
/*
 * COMPONENT_NAME: (CMDPROG) Programming Utilites
 *
 * FUNCTIONS: CheckSymbol, FtnUsage, RefDefSymbol, SameMembers, SameParameters,
	      SameTypes
 *
 * ORIGINS: 00 03 10 27 32
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 * OBJECT CODE ONLY SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * lint2.c	1.5  com/cmd/prog/lint/pass2,3.1,9013 10/30/89 18:19:12"; 
 */
#include "mfile1.h"
#include "lint2.h"


#include "lint_msg.h"
#define  MSGSTR(Num, Str) catgets(catd, MS_LINT, Num, Str)
nl_catd catd;
/*
** Check that the current symbol and symbol p are compatible types.
*/
CheckSymbol()
{
	int stat;

	stat = STORE;
#ifndef	DEBUG
	if (debug) {
		PrintSymbol("CHECKING OLD SYMBOL", prevSym);
		PrintSymbol("WITH NEW SYMBOL", curSym);
		printf("at %s (%s, %d-%d)\n", curPFname, curIFname, curDLine,
			curRLine);
        if (strcmp(curSym->sname, "asinf") == 0 ||  strcmp(curSym->sname, "sin") == 0) {
            printf("In Checksymbol with %s\n", curSym->sname);
        }
	}
#endif

	/* Check for multiple definitions. */
	if ((prevSym->usage & LINTDEF) && (curSym->usage & LINTDEF) &&
		!(prevSym->usage & LINTMBR)) {
		if (ISFTN(curSym->type))
			LERROR(WDECLAR, MSGSTR(M_MSG_304,
			       "function %s multiply defined"), curSym, CDUSE);
		else if (devdebug[ANSI_MODE]) 
			LERROR(WDECLAR, MSGSTR(M_MSG_305,
			       "symbol %s multiply defined"), curSym, CDUSE);
		stat = REJECT;
	}

	/* Check for compatible types. */
	if (!SameTypes(prevSym->type, curSym->type)) {
		stat = REJECT;
		if (!ISFTN(curSym->type))
			LERROR(WDECLAR, MSGSTR(M_MSG_306,
			       "symbol %s type inconsistent"), curSym, CRUSE);
		else if (prevSym->usage & LINTVRG || curSym->usage & LINTVRG)
			stat = CHANGE;
	}

	/* Check for possible struct/union/enum redefinition. */
	if (prevSym->nmbrs && !SameMembers(prevSym, curSym)) {
		LERROR(WDECLAR, MSGSTR(M_MSG_307,
			"struct/union/enum %s inconsistently redefined"),
			curSym, CDUSE);
		stat = REJECT;
	}

	/* End of REJECTion processing, now check for CHANGEs. */
	if (stat == REJECT)
		return (stat);

	/* New instance of symbol usage? */
	if ((prevSym->usage & (LINTREF|LINTDEF|LINTDCL|LINTRET)) !=
		(curSym->usage & (LINTREF|LINTDEF|LINTDCL|LINTRET)))
		stat = CHANGE;

	/* New file reference of symbol? */
	if (((prevSym->usage & LINTREF) && strcmp(prevSym->rpf, curPFname)) || 
	    ((prevSym->usage & LINTDEF) && strcmp(prevSym->dpf, curPFname)) &&
	    (curSym->usage & (LINTREF|LINTDCL)))
		stat = CHANGE;

	return (stat);
}

/*
** This routine makes sure that the types t1 and t2 are compatible.
** Error messages assuming that these are dereferenced pointer types.
*/
SameTypes(t1, t2)
	register TPTR t1, t2;
{
	TPTR ot1 = t1;
	TPTR ot2 = t2;

	/* Check qualifier top-type compatibility. */
	if (QUALIFIERS(t1) != QUALIFIERS(t2))
		return (0);

	/* INTs and ENUMs are compatible types. */
	if (TOPTYPE(t1) != TOPTYPE(t2)) {
		if ((TOPTYPE(t1) == INT && TOPTYPE(t2) == ENUMTY) ||
			(TOPTYPE(t2) == INT && TOPTYPE(t1) == ENUMTY))
			return (1);
		return (0);
	}

	/* Examine each level of type indirection. */
	while (!ISBTYPE(t1)) {
		switch (TOPTYPE(t1)) {
		case FTN:
			if (!SameParameters(t1, t2))
				return (0);
			break;

		case ARY:
			if (t1->ary_size != 0 && t2->ary_size != 0 &&
				t1->ary_size != t2->ary_size)
				return (0);
			break;
		}
		t1 = t1->next; t2 = t2->next;

		/* Check type specifier and qualifier. */
		if (TOPQTYPE(t1) != TOPQTYPE(t2)) {
			if (QUALIFIERS(t1) == QUALIFIERS(t2) &&
				((TOPTYPE(t1) == INT && TOPTYPE(t2) == ENUMTY)
				||
				(TOPTYPE(t2) == INT && TOPTYPE(t1) == ENUMTY)))
				return (1);
			if (ISFTN(ot1) || ISFTN(ot2))
				LERROR(WDECLAR, MSGSTR(M_MSG_308,
					"function %s type inconsistent"),
					curSym, CRUSE);
			return (0);
		}
	}

	if (t1->typ_size != t2->typ_size)
		return (0);

	return (1);
}

/*
** Check if both function argument lists are compatible.
*/
static int lcl_defaultproto(p)
register PPTR p;
{
	/* use default argument promotion on each basic
	 * type encountered on the prototype list and issue an
	 * error if it is incompatible with its original type or
	 * an ellipsis is encountered (see section 3.5.4.3).
	 *
	 * In practice, this means we issue an error if
	 * the basic type is char, short, float or ellipsis.
     * GAF: the original code for this is in tftn.c. 
	 */

	for (/*null*/; p != PNIL; p = p->next) {
		switch ( TOPTYPE( p->type ) ) {
		      case CHAR:
		      case UCHAR:
		      case SCHAR:
		      case SHORT:
		      case USHORT:
/*		      case FLOAT:	to prevent conflicts in llib-lm.c */
		      case TELLIPSIS:
			return ( 0 );
		      default:
			break;
		}
	}

	return ( 1 );
}

SameParameters(t1, t2)
	register TPTR t1,t2;
{
	register PPTR p1 = t1->ftn_parm;
	register PPTR p2 = t2->ftn_parm;

	/* Check for prototype lists. */
	if (p1 == PNIL && p2 == PNIL)
		return (1);

	/* Check for ellipsis, although strictly speaking ANSI does not
	   permit an ellipsis to be the only function parameter, it is
	   produced when a VARARGS0 appears before an old-style function
	   definition. */
	if ((p1 != PNIL && TOPTYPE(p1->type) == TELLIPSIS) || (p2 != PNIL &&
		TOPTYPE(p2->type) == TELLIPSIS))
		return (1);

	/* If either prototype is NIL, check for default compatibilty. */
	if (p1 == PNIL)
		return (lcl_defaultproto(p2));
	if (p2 == PNIL)
		return (lcl_defaultproto(p1));

	/* Check each entry on both lists to be sure they are
	   compatible.  If not return a failure for the function. */
	for (; p1 != PNIL && p2 != PNIL; p1 = p1->next, p2 = p2->next) {
		if (!SameTypes(p1->type, p2->type)) {
			LERROR(WDECLAR, MSGSTR(M_MSG_309,
				"function %s argument type inconsistent"),
				curSym, CRUSE);
			return (0);
		}
	}

	/* If both parameter lists don't end simultaneously, the
	   number of arguments is mismatched. */
	if (!(prevSym->usage & LINTVRG || curSym->usage & LINTVRG)) {
		if (!(p1 == PNIL && p2 == PNIL)) {
			LERROR(WDECLAR, MSGSTR(M_MSG_310,
				"function %s argument count mismatch"),
				curSym, CRUSE);
			return (0);
		}
	}
	return (1);
}

/*
** Check if struct/union/enum members are identical.
*/
SameMembers(p, q)
	SMTAB *p, *q;
{
	register MBTAB *mp, *mq;
	TWORD bt;

	/* Same symbol check. */
	if (p == q)
		return (1);

	/* Member count check. */
	if (p->nmbrs != q->nmbrs)
		return (0);

	/* Compare each member. */
	mp = p->mbrs; mq = q->mbrs;
	while (mp) {
#ifndef	DEBUG
		if (debug)
			printf("\tcomparing %s vs. %s\n",mp->mname,mq->mname);
#endif
		if (strcmp(mp->mname, mq->mname))
			return (0);
		if (!SameTypes(mp->type, mq->type))
			return (0);
		if ((bt = BTYPE(mp->type)) == STRTY || bt == UNIONTY ||
			bt == ENUMTY)
			if ((mp->tagname == NULL && mq->tagname != NULL) ||
				(mp->tagname != NULL && mq->tagname == NULL) ||
				(mp->tagname != mq->tagname &&
				strcmp(mp->tagname, mq->tagname)))
				return (0);
		mp = mp->next; mq = mq->next;
	}
	return (1);
}

/*
** Examine symbol for proper reference/definitions usage.
*/
RefDefSymbol(h)
	register SMTAB *h;
{
#ifndef	DEBUG
		if (debug)
		  PrintSymbol("In RefDefSymbol", h);
#endif	
	switch (h->usage & (LINTREF|LINTDEF|LINTDCL)) {
	case LINTREF|LINTDCL:
		if (!(h->usage & LINTNDF)) {
			if (ISFTN(h->type))
				LERROR(WUNUSED, MSGSTR(M_MSG_311,
					"function %s used but not defined"), h, RUSE);
			else
				LERROR(WUNUSED, MSGSTR(M_MSG_312,
					"symbol %s used but not defined"), h, RUSE);
		} 
		break;
	case LINTDEF:
	case LINTDEF|LINTDCL:
		if (!(h->usage & LINTNOT)) {
			if (ISFTN(h->type)) {
				if (strcmp(h->sname, "main") && !(h->usage & LINTLIB))
					LERROR(WUNUSED, MSGSTR(M_MSG_313,
						"function %s defined but never used"),
						h, DUSE);
			}
			else
				LERROR(WUNUSED, MSGSTR(M_MSG_314,
					"symbol %s defined but never used"), h, DUSE);
		}
		break;
	case LINTDCL:
		if (!(h->usage & LINTNOT) && !(h->usage & LINTNDF)) {
			if (ISFTN(h->type))
				LERROR(WUDECLAR, MSGSTR(M_MSG_315,
			  	"function %s declared but never used or defined"),
			  	h, RUSE);
			else
				LERROR(WUDECLAR, MSGSTR(M_MSG_316,
			  	"symbol %s declared but never used or defined"),
			  	h, RUSE);
		}
		break;

	}
	/*  "symbol %s set but never used." Suppress with either -u or -wU */
	if (h->usage & LINTSTO)
	    LERROR(WUSAGE && WUNUSED, MSGSTR(M_MSG_323,
		   "symbol %s set but never used."), h, DUSE);
}

/*
** Examine function for proper/consistent usage.
*/
FtnUsage(h)
	register SMTAB *h;
{
	switch (h->usage & (LINTRET|LINTUSE|LINTIGN)) {
	case LINTUSE:
	case LINTUSE|LINTIGN:
		LERROR(WRETURN, MSGSTR(M_MSG_317,
			"function %s return value used, but none returned"),
			h, RUSE);
		break;

	case LINTRET|LINTIGN:
		LERROR(WRETURN, MSGSTR(M_MSG_318,
			"function %s return value is always ignored"),h,DUSE);
		break;

	case LINTRET|LINTUSE|LINTIGN:
		LERROR(WRETURN, MSGSTR(M_MSG_319,
			"function %s return value is sometimes ignored"),
			h, DUSE);
		break;
	}
}



