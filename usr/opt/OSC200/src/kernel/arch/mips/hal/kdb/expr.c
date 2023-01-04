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
static char	*sccsid = "@(#)$RCSfile: expr.c,v $ $Revision: 1.2 $ (DEC) $Date: 1992/01/15 01:11:20 $";
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
 * derived from expr.c	2.1	(ULTRIX/OSF)	12/3/90";
 */

/*
 *
 *	UNIX debugger
 *
 */


#include <hal/kdb/defs.h>

extern msg		BADSYM;
extern msg		BADVAR;
extern msg		BADKET;
extern msg		BADSYN;
extern msg		NOCFN;
extern msg		NOADR;
extern msg		BADLOC;

long		lastframe;
long		savlastf;
long		savframe;
long		savpc;
long		callpc;



char		*lp;
short		radix;
string_t		errflg;
char		isymbol[1024];

char		lastc,peekc;

long		dot;
long		ditto;
short		dotinc;
long		var[];
long		expv;




expr(a)
{	/* term | term dyadic expr |  */
	short		rc;
	long		lhs;

	rdc();
	lp--;
	rc=term(a);

	while ( rc ) {
		lhs = expv;

		switch ((int)readchar()) {

		case '+':
			term(a|1);
			expv += lhs;
			break;

		case '-':
			term(a|1);
			expv = lhs - expv;
			break;

		case '#':
			term(a|1);
			expv = round(lhs,expv);
			break;

		case '*':
			term(a|1);
			expv *= lhs;
			break;

		case '%':
			term(a|1);
			expv = lhs/expv;
			break;

		case '&':
			term(a|1);
			expv &= lhs;
			break;

		case '|':
			term(a|1);
			expv |= lhs;
			break;

		case ')':
			if ( (a&2)==0 ) {
				error(BADKET);
			}

		default:
			lp--;
			return(rc);
		}
	}
	return(rc);
}

term(a)
{	/* item | monadic item | (expr) | */

	switch ((int)readchar()) {

	case '*':
		term(a|1);
		expv=chkget(expv,DSP);
		return(1);

	case '@':
		term(a|1);
		expv=chkget(expv,ISP);
		return(1);

	case '-':
		term(a|1);
		expv = -expv;
		return(1);

	case '~':
		term(a|1);
		expv = ~expv;
		return(1);

	case '#':
		term(a|1);
		expv = !expv;
		return(1);

	case '(':
		expr(2);
		if ( *lp!=')' ) {
			error(BADSYN);
		} else {
			lp++;
			return(1);
		}

	default:
		lp--;
		return(item(a));
	}
}

item(a)
{	/* name [ . local ] | number | . | ^ | <var | <register | 'x | | */
	short		base, d;
	char		savc;
	char		hex;
	long		frame;
	register struct nlist *symp;
	int regptr;

	hex = FALSE;

	readchar();
	if ( symchar(0) ) {
		readsym();
		if ( lastc=='.' ) {
/* Bingo		frame= *(long *)(((long)&u)+FP);
			lastframe=0;
			callpc= *(long *)(((long)&u)+PC);
			while ( errflg==0 ) {
				savpc=callpc;
				findsym(callpc,ISYM);
				if (  eqsym(cursym->n_name,isymbol,'~') ) {
					break;
				}
				callpc=get(frame+16, DSP);
				lastframe=frame;
				frame=get(frame+12,DSP)&EVEN;
				if ( frame==0 ) {
					error(NOCFN);
				}
			}
			savlastf=lastframe;
			savframe=frame;
*/
printf("Bingo: item.\n");
lastframe=savlastf=savframe=0;
			readchar();
			if ( symchar(0) ) {
				chkloc(expv=frame);
			}
		} else if ( (symp=lookup(isymbol))==0 ) {
			error(BADSYM);
		} else {
			expv = symp->n_value;
		}
		lp--;

	} else if ( getnum(readchar) ) {
		;
	} else if ( lastc=='.' ) {
		readchar();
		if ( symchar(0) ) {
			lastframe=savlastf;
			callpc=savpc;
			chkloc(savframe);
		} else {
			expv=dot;
		}
		lp--;

	} else if ( lastc=='"' ) {
		expv=ditto;

	} else if ( lastc=='+' ) {
		expv=inkdot(dotinc);

	} else if ( lastc=='^' ) {
		expv=inkdot(-dotinc);

	} else if ( lastc=='<' ) {
		savc=rdc();
		if ( regptr=getreg(savc) ) {
			expv = *(int *)regptr;
		} else if ( (base=varchk(savc)) != -1 ) {
			expv=var[base];
		} else {
			error(BADVAR);
		}

	} else if ( lastc=='\'' ) {
		d=4;
		expv=0;
		while ( quotchar() ) {
			if ( d-- ) {
				expv = (expv << 8) | lastc;
			} else {
				error(BADSYN);
			}
		}

	} else if ( a ) {
		error(NOADR);
	} else {
		lp--;
		return(0);
	}
	return(1);
}

/* service routines for expression reading */
getnum(rdf)
	int (*rdf)();
{
	short base,d,frpt;
	char hex;
	long rv;

	if ( isdigit(lastc) || (hex=TRUE, lastc=='#' && isxdigit((*rdf)())) ) {
		expv = 0;
		base = (hex ? 16 : radix);
		while ( (base>10 ? isxdigit(lastc) : isdigit(lastc)) ) {
			expv = (base==16 ? expv<<4 : expv*base);
			if ( (d=convdig(lastc))>=base ) {
				error(BADSYN);
			}
			expv += d;
			(*rdf)();
			if ( expv==0 ) {
				if ( (lastc=='x' || lastc=='X') ) {
					hex=TRUE;
					base=16;
					(*rdf)();
				} else if ( (lastc=='t' || lastc=='T') ) {
					hex=FALSE;
					base=10;
					(*rdf)();
				} else if ( (lastc=='o' || lastc=='O') ) {
					hex=FALSE;
					base=8;
					(*rdf)();
				}
			}
		}
		if ( lastc=='.' && (base==10 || expv==0) && !hex ) {
			rv = expv;
			frpt=0;
			base=10;
			while ( isdigit((*rdf)()) ) {
				rv *= base;
				frpt++;
				rv += lastc-'0';
			}
			while ( frpt-- ) {
				rv /= base;
			}
			expv = rv;
		}
		peekc=lastc;
		/*		lp--; */
		return(1);
	} else {
		return(0);
	}
}

readsym()
{
	register char	*p;

	p = isymbol;
	do {
		if ( p < &isymbol[sizeof(isymbol)-1] ) {
			*p++ = lastc;
		}
		readchar();
	}
	while( symchar(1) ) ;
	*p++ = 0;
}

convdig(c)
char c;
{
	if ( isdigit(c) ) {
		return(c-'0');
	} else if ( isxdigit(c) ) {
		return(c-'a'+10);
	} else {
		return(17);
	}
}

symchar(dig)
{
	if ( lastc=='\\' ) {
		readchar();
		return(TRUE);
	}
	return( isalpha(lastc) || lastc=='_' || dig && isdigit(lastc) );
}

varchk(name)
{
	if ( isdigit(name) ) {
		return(name-'0');
	}
	if ( isalpha(name) ) {
		return((name&037)-1+10);
	}
	return(-1);
}

chkloc(frame)
long		frame;
{
	readsym();
	do {
		if ( localsym(frame)==0 ) {
			error(BADLOC);
		}
		expv=localval;
	}
	while( !eqsym(cursym->n_name,isymbol,'~') ) ;
}

eqsym(s1, s2, c)
register char *s1, *s2;
{
	if (s1 == 0)
		s1 = "";
	if (s2 == 0)
		s2 = "";

	if (!strcmp(s1,s2))
		return (1);
	if (*s1 == c && !strcmp(s1+1, s2))
		return (1);
	return (0);
}
