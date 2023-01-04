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
static char	*sccsid = "@(#)$RCSfile: format.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:10:53 $";
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
 *
 *	UNIX debugger
 *
 */

#include <i386/kdb/defs.h>
#include <sys/reboot.h>

msg		BADMOD;
msg		ADWRAP;

INT		mkfault;
char		*lp;
long		maxoff;
long		sigint;
long		sigqit;
string_t	errflg;
char		lastc,peekc;
long		dot;
INT		dotinc;
long		expv;
long		var[];


string_t fphack;

rdfp()
{
	return(lastc= *fphack++);
}

scanform(icount,ifp,itype,ptype)
long		icount;
string_t		ifp;
{
	string_t		fp;
	char		modifier;
	INT		fcount, init=1;
	long		savdot;
	BOOL exact;
	BOOL doit = 1;

	while ( icount ) {
		fp=ifp;
		savdot=dot;
		init=0;

		if ( init==0 && (exact=(findsym(dot,ptype)==0)) && maxoff ) {
#ifdef	wheeze
			printf("\n%s:%16t",cursym->n_name);
#else	wheeze
			printf("\n%s:%16t",cursym->n_un.n_name);
#endif	wheeze
		}

		/*now loop over format*/
		while ( *fp && errflg==0 ) {
			if ( digit(modifier = *fp) ) {
				fcount = 0;
				while ( digit(modifier = *fp++) ) {
					fcount *= 10;
					fcount += modifier-'0';
				}
				fp--;
			} else {
				fcount = 1;
			}

			if ( *fp==0 ) {
				break;
			}
#ifdef	wheeze
			if ( exact && dot==savdot && itype==ISP && cursym->n_name[0]=='_' && *fp=='i' ) 
#else	wheeze
			if ( exact && dot==savdot && itype==ISP && cursym->n_un.n_name[0]=='_' && *fp=='i' )
#endif	wheeze
			{
				exform(1,"x",itype,ptype);
				fp++;
				printc(EOR); /* entry mask */
			} else {
				fp=exform(fcount,fp,itype,ptype);
			}
		}
		dotinc=dot-savdot;
		dot=savdot;

		if ( errflg ) {
			if ( icount<0 ) {
				errflg=0;
				break;
			} else {
				error(errflg);
			}
		}
		if ( --icount ) {
			dot=inkdot(dotinc);
		}
		if ( mkfault ) {
			error(0);
		}
	}
}

string_t
exform(fcount,ifp,itype,ptype)
INT		fcount;
string_t		ifp;
{
	/* execute single format item `fcount' times
	 * sets `dotinc' and moves `dot'
	 * returns address of next format item
	 */
	unsigned		w;
	long		savdot, wx;
	string_t		fp;
	char		c, modifier, longpr;
#ifdef	FLOAT
	double		fw;
#endif
	struct L_STRUCT {
		long	sa;
		INT	sb,sc;
	};

	while ( fcount>0 ) {
		fp = ifp;
		c = *fp;
		longpr=(c>='A')&(c<='Z')|(c=='f')|(c=='4')|(c=='p');
		if ( itype==NSP || *fp=='a' ) {
			wx=dot;
			w=dot;
		} else {
			w=get(dot);
			if ( longpr ) {
				wx=itol(get(inkdot(2)),w);
			} else {
				wx=w;
			}
		}
#ifdef	FLOAT
		if ( c=='F' ) {
			((struct L_STRUCT *)&fw)->sb=get(inkdot(4));
			((struct L_STRUCT *)&fw)->sc=get(inkdot(6));
		}
#endif
		if ( errflg ) {
			return(fp);
		}
		if ( mkfault ) {
			error(0);
		}
		var[0]=wx;
		modifier = *fp++;
		dotinc=(longpr?4:2);
		;

		if ( charpos()==0 && modifier!='a' ) {
			printf("%16m");
		}

		switch(modifier) {

		case ' ':
		case '	':
			break;

		case 't':
		case 'T':
			printf("%T",fcount);
			return(fp);

		case 'r':
		case 'R':
			printf("%M",fcount);
			return(fp);

		case 'a':
			psymoff(dot,ptype,":%16t");
			dotinc=0;
			break;

		case 'p':
			psymoff(var[0],ptype,"%16t");
			break;

		case 'u':
			printf("%-8u",w);
			break;

		case 'U':
			printf("%-16U",wx);
			break;

		case 'c':
		case 'C':
			if ( modifier=='C' ) {
				printesc(w&0377);
			} else {
				printc(w&0377);
			}
			dotinc=1;
			break;

		case 'b':
		case 'B':
			printf("%-8o", w&0377);
			dotinc=1;
			break;

		case '1':
			printf("%-8r", w&0377);
			dotinc=1;
			break;

		case '2':
		case 'w':
			printf("%-8r", w);
			break;

		case '4':
		case 'W':
			printf("%-16R", wx);
			break;

		case 's':
		case 'S':
			savdot=dot;
			dotinc=1;
			while ( (c=get(dot)&0377) && errflg==0 ) {
				dot=inkdot(1);
				if ( modifier == 'S' ) {
					printesc(c);
				} else {
					printc(c);
				}
				endline();
			}
			dotinc=dot-savdot+1;
			dot=savdot;
			break;

		case 'x':
			printf("%-8x",w);
			break;

		case 'X':
			printf("%-16X", wx);
			break;

		case 'Y':
			printf("%-24Y", wx);
			break;

		case 'q':
			printf("%-8q", w);
			break;

		case 'Q':
			printf("%-16Q", wx);
			break;

		case 'o':
			printf("%-8o", w);
			break;

		case 'O':
			printf("%-16O", wx);
			break;

		case 'i':
			printins(0,itype,w);
			printc(EOR);
			break;

		case 'd':
			printf("%-8d", w);
			break;

		case 'D':
			printf("%-16D", wx);
			break;

#ifdef	FLOAT
		case 'f':
			fw = 0;
			((struct L_STRUCT *)&fw)->sa = wx;
			printf("%-16.9f", fw);
			dotinc=4;
			break;

		case 'F':
			((struct L_STRUCT *)&fw)->sa = wx;
			printf("%-32.18F", fw);
			dotinc=8;
			break;
#endif

		case 'n':
		case 'N':
			printc('\n');
			dotinc=0;
			break;

		case '"':
			dotinc=0;
			while ( *fp != '"' && *fp ) {
				printc(*fp++);
			}
			if ( *fp ) {
				fp++;
			}
			break;

		case '^':
			dot=inkdot(-dotinc*fcount);
			return(fp);

		case '+':
			dot=inkdot(fcount);
			return(fp);

		case '-':
			dot=inkdot(-fcount);
			return(fp);

		default:
			printf("don't understand modifier 0%o\n", modifier);
			error(BADMOD);
		}
		if ( itype!=NSP ) {
			dot=inkdot(dotinc);
		}
		fcount--;
		endline();
	}

	return(fp);
}

shell()
{
	/*
	 * The closest thing to a shell for the kernel debugger
	 * is perhaps the ability to call functions. Or maybe not.
	 */
#define MAXARGS 11
	int args[MAXARGS], narg = 0;
	int ret;
	int (*func)();
	extern long adrval, cntval;
	extern short adrflg, cntflg;

	func = (int (*) ())dot;		/* address of target function */

	if ( !cntflg )			/* repeat count */
		cntval=1;

	while (expr(0)) {		/* arguments */
		args[narg++] = expv;
		if (narg == MAXARGS)
			error("Too many args");
	}

	while (narg < MAXARGS)		/* default to 0 */
		args[narg++] = 0;

	while (lastc != EOR)		/* skip any extra stuff */
		rdc();

	while (cntval-- > 0) {
		ret = (*func)(args[0], args[1], args[2], args[3], args[4],
			      args[5], args[6], args[7], args[8], args[9] );
		printf("!%R\n", ret);
	}
}

Q()
{
	outb(0x64, 0xfe);
	panic("KDB:CPU didn't reset\n");
}

R()
{
	boot(RB_PANIC, RB_AUTOBOOT);
}

printesc(c)
{
	c &= 0177;
	if ( c==0177 ) {
		printf("^?");
	} else if ( c<' ' ) {
		printf("^%c", c + '@');
	} else {
		printc(c);
	}
}

long	inkdot(incr)
{
	long		newdot;

	newdot=dot+incr;
	if ( (dot ^ newdot) >> 24 ) {
		error(ADWRAP);
	}
	return(newdot);
}

digit(c)
{
	return c >= '0' && c <= '9';
}
