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
static char	*sccsid = "@(#)$RCSfile: command.c,v $ $Revision: 1.2 $ (DEC) $Date: 1992/01/15 01:11:09 $";
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
 * derived from command.c	2.1	(ULTRIX/OSF)	12/3/90";
 */

/*
 *
 *	UNIX debugger
 *
 */


#include <hal/kdb/defs.h>

extern msg		BADEQ;
extern msg		NOMATCH;
extern msg		BADVAR;
extern msg		BADCOM;

short		executing;
short           doreinput;
string_t	errflg;

char		*lp;
char		lastc;
char		eqformat[512] = "z";
char		stformat[512] = "X\"= \"^i";

long		dot;
long		ditto;
short		dotinc;
short		lastcom;
long		var[];
long		locval;
long		locmsk;
long		expv;
long		adrval;
short		adrflg;
long		cntval;
short		cntflg;




/* command decoding */

command(buf,defcom)
string_t		buf;
char		defcom;
{
	short		itype, ptype, modifier;
	long		regptr;
	char		longpr, eqcom;
	char		wformat[1];
	char		savc;
	long		w, savdot;
	string_t	savlp=lp;
	if ( buf ) {
		if ( *buf==EOR ) {
			return(FALSE);
		} else {
			lp=buf;
		}
	}

	do {
		if ( adrflg=expr(0) ) {
			dot=expv;
			ditto=dot;
		}
		adrval=dot;
		if ( rdc()==',' && expr(0) ) {
			cntflg=TRUE;
			cntval=expv;
		} else {
			cntflg=FALSE;
			cntval=1;
			lp--;
		}

		if ( !eol(rdc()) ) {
			lastcom=lastc;
		} else {
			if ( adrflg==0 ) {
				dot=inkdot(dotinc);
			}
			lp--;
			lastcom=defcom;
		}

		switch(lastcom&0177) {

		case '/':
			itype=DSP;
			ptype=DSYM;
			goto trystar;

		case '=':
			itype=NSP;
			ptype=0;
			goto trypr;

		case '?':
			itype=ISP;
			ptype=ISYM;
			goto trystar;

trystar:
			if ( rdc()=='*' ) {
				lastcom |= QUOTE;
			} else {
				lp--;
			}
			if ( lastcom&QUOTE ) {
				itype |= STAR;
				ptype = (DSYM+ISYM)-ptype;
			}

trypr:
			longpr=FALSE;
			eqcom=lastcom=='=';
			switch (rdc()) {

			case 'm':
				{/* Obsolete: reset map data*/
				}
				break;

			case 'L':
				longpr=TRUE;
			case 'l':
				/*search for exp*/
				if ( eqcom ) {
					error(BADEQ);
				}
				dotinc=(longpr?4:2);
				savdot=dot;
				expr(1);
				locval=expv;
				if ( expr(0) ) {
					locmsk=expv;
				} else {
					locmsk = -1L;
				}
				if ( !longpr ) {
					locmsk &= 0xFFFF;
					locval &= 0xFFFF;
				}
				{ /* try to find a match */
				    int match = 0;
				    for (;;) {
 					if ( cntflg && cntval-- <= 0 ) {
 						break;
 					}
					w=get(dot,itype);
					if ( errflg || (w&locmsk)==locval ) {
						match = 1;
						break;
					}
					dot=inkdot(dotinc);
				    }
				    if (!match) {
					errflg=NOMATCH;
				    }
				}
				if ( errflg ) {
					dot=savdot;
					errflg=NOMATCH;
				}
				psymoff(dot,ptype,"");
				break;

			case 'W':
				longpr=TRUE;
			case 'w':
				if ( eqcom ) {
					error(BADEQ);
				}
				wformat[0]=lastc;
				expr(1);
				do {
					savdot=dot;
					psymoff(dot,ptype,":%16t");
					exform(1,wformat,itype,ptype);
					errflg=0;
					dot=savdot;
					if ( longpr ) {
						put(dot,itype,expv);
					} else {
						put(dot,itype,itol(get(dot+2,itype),expv));
					}
					savdot=dot;
					printf("=%8t");
					exform(1,wformat,itype,ptype);
					newline();
				}
				while(  expr(0) && errflg==0 ) ;
				dot=savdot;
				chkerr();
				break;

			default:
				lp--;
				getformat(eqcom ? eqformat : stformat);
				if ( !eqcom ) {
					psymoff(dot,ptype,":%16t");
				}
				scanform(cntval,(eqcom?eqformat:stformat),itype,ptype);
			}
			break;

		case '>':
			lastcom=0;
			savc=rdc();
			if ( regptr=getreg(savc) ) {
				*(int *)regptr = dot;
			} else if ( (modifier=varchk(savc)) != -1 ) {
				var[modifier]=dot;
			} else {
				error(BADVAR);
			}
			break;

		case '!':
			lastcom=0;
			shell();
			break;

		case '$':
			lastcom=0;
			printtrace(nextchar());
			break;

		case ':':
			lastcom=0;
			if ( !executing ) {
				executing=TRUE;
				subpcs(nextchar());
				/* this longjmp's if continue/single/etc. */
				executing=FALSE;
			}
			break;

		case 0:
	                if (doreinput) {
	                        re_input();
	                } else
	                        error (DBNAME);
	                break;

		default:
			error(BADCOM);
		}

		flushbuf();
		doreinput = 1;
	}
	while( rdc()==';' ) ;
	if ( buf ) {
		lp=savlp;
	} else {
		lp--;
	}
	return(adrflg && dot!=0);
}

