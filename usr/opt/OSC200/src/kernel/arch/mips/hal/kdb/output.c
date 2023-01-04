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
static char	*sccsid = "@(#)$RCSfile: output.c,v $ $Revision: 1.2 $ (DEC) $Date: 1992/01/15 01:12:33 $";
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
 * derived from output.c	2.1	(ULTRIX/OSF)	12/3/90";
 */

/*
 *
 *	UNIX debugger
 *
 */


#include <hal/kdb/defs.h>
#include <machine/varargs.h>

long		maxpos;
short		radix = 16;

char		printbuf[MAXLIN];
char		*printptr = printbuf;
char		*digitptr;


eqstr(s1, s2)
register string_t	s1, s2;
{
	while ( *s1++ == *s2 ) {
		if ( *s2++ == 0 ) {
			return(1);
		}
	}
	return(0);
}

length(s)
register string_t		s;
{
	short		n = 0;
	while ( *s++ ) {
		n++;
	}
	return(n);
}

printc(c)
char		c;
{
	char		d;
	string_t		q;
	short		posn, tabs, p;

	if ( (*printptr=c)==EOR ) {
		tabs=0;
		posn=0;
		q=printbuf;
		for ( p=0; p<printptr-printbuf; p++ ) {
			    d=printbuf[p];
			if ( (p&7)==0 && posn ) {
				tabs++;
				posn=0;
			}
			if ( d==' ' ) {
				posn++;
			} else {
				while ( tabs>0 ) {
					*q++='	';
					tabs--;
				}
				while ( posn>0 ) {
					*q++=' ';
					posn--;
				}
				*q++=d;
			}
		}
		*q++=EOR;
		write(1,printbuf,q-printbuf);
		printptr=printbuf;
	} else if ( c=='\t' ) {
		*printptr++=' ';
		while ( (printptr-printbuf)&7 ) {
			*printptr++=' ';
		}
	} else if ( c ) {
		printptr++;
	}
	if ( printptr >= &printbuf[MAXLIN-9] ) {
		write(1, printbuf, printptr - printbuf);
		printptr = printbuf;
	}
}

charpos()
{
	return(printptr-printbuf);
}

flushbuf()
{
	if ( printptr!=printbuf ) {
		printc(EOR);
	}
}

printf(va_alist)
va_dcl
{
	va_list pvar;
	va_list vptr, dptr, rptr, tmp;
	short v; unsigned int d;/* L_REAL r;*/
	string_t		fptr, s;
	short		width, prec;
	char		c, adj;
	short		x, n;
	long		lx;
	char		digits[64];

	va_start(pvar);
	fptr = va_arg(pvar, char*);

	dptr = vptr = pvar;

	while ( c = *fptr++ ) {
		if ( c!='%' ) {
			printc(c);
		} else {
			if ( *fptr=='-' ) {
				adj='l';
				fptr++;
			} else {
				adj='r';
			}
			width=convert(&fptr);
			if ( *fptr=='.' ) {
				fptr++;
				prec=convert(&fptr);
			} else {
				prec = -1;
			}

			digitptr=digits;

		        rptr = dptr;
		        lx = va_arg(dptr, long);

			x = shorten(lx);
			s=0;
			switch (c = *fptr++) {

			case 'd':
			case 'u':
				printnum(x,c,10);
				break;
			case 'o':
				printoct(itol(0,x),0);
				break;
			case 'q':
				printoct(lx,-1);
				break;
			case 'x':
				printdbl(itol(0,x),c,16);
				break;
			case 'r':
				printdbl(lx=x,c,radix);
				break;
			case 'R':
				printdbl(lx,c,radix);
				v = va_arg(vptr, short);
				break;
			case 'Y':
				printdate(lx);
			        v = va_arg(vptr, short);
				break;
			case 'D':
			case 'U':
				printdbl(lx,c,10);
			        v = va_arg(vptr, short);
				break;
			case 'O':
				printoct(lx,0);
			        v = va_arg(vptr, short);
				break;
			case 'Q':
				printoct(lx,-1);
			        v = va_arg(vptr, short);
				break;
			case 'X':
				printdbl(lx,'x',16);
			        v = va_arg(vptr, short);
				break;
			case 'c':
				printc(x);
				break;
			case 's':
				s=(char *)lx;
				break;
			case 'm':
			        vptr = (va_list)((int)vptr - sizeof(short));
				break;
			case 'M':
				width=x;
				break;
			case 'T':
			case 't':
				if ( c=='T' )
					width=x;
				else
					dptr = (va_list)((int)dptr - sizeof(long));

				if ( width ) {
					width -= charpos()%width;
				}
				break;
			default:
				printc(c);
			        dptr = (va_list)((int)dptr - sizeof(long));
			}

			if ( s==0 ) {
				*digitptr=0;
				s=digits;
			}
			n=length(s);
			n=(prec<n && prec>=0 ? prec : n);
			width -= n;
			if ( adj=='r' ) {
				while ( width-- > 0 ) {
					printc(' ');
				}
			}
			while ( n-- ) {
				printc(*s++);
			}
			while ( width-- > 0 ) {
				printc(' ');
			}
			digitptr=digits;
		}
	}
	va_end(pvar);
}

printdate(tvec)
long		tvec;
{
	register short		i;
	register string_t	timeptr;

	timeptr="????????????????????????";
	for ( i=20; i<24; i++ ) {
		*digitptr++ = *(timeptr+i);
	}
	for ( i=3; i<19; i++ ) {
		*digitptr++ = *(timeptr+i);
	}
}

prints(s)
char *s;
{
	printf("%s",s);
}

newline()
{
	printc(EOR);
}

convert(cp)
register string_t	*cp;
{
	register char	c;
	short		n;
	n=0;
	while ( ((c = *(*cp)++)>='0') && (c<='9') ) {
		n=n*10+c-'0';
	}
	(*cp)--;
	return(n);
}

printnum(n,fmat,base)
register short		n;
{
	register char	k;
	register short		*dptr;
	short		digs[15];
	dptr=digs;
	if ( n<0 && fmat=='d' ) {
		n = -n;
		*digitptr++ = '-';
	}
	n &= 0xffff;
	while ( n ) {
		*dptr++ = ((unsigned)(n&0xffff))%base;
		n=((unsigned)(n&0xffff))/base;
	}
	if ( dptr==digs ) {
		*dptr++=0;
	}
	while ( dptr!=digs ) {
		k = *--dptr;
		*digitptr++ = (k+(k<=9 ? '0' : 'a'-10));
	}
}

printoct(o,s)
long		o;
short		s;
{
	short		i;
	long		po = o;
	char		digs[12];

	if ( s ) {
		if ( po<0 ) {
			po = -po;
			*digitptr++='-';
		} else {
			if ( s>0 ) {
				*digitptr++='+';
			}
		}
	}
	for ( i=0;i<=11;i++ ) {
		    digs[i] = po&7;
		po >>= 3;
	}
	digs[10] &= 03;
	digs[11]=0;
	for ( i=11;i>=0;i-- ) {
		    if ( digs[i] ) {
			break;
		}
	}
	for ( i++;i>=0;i-- ) {
		    *digitptr++=digs[i]+'0';
	}
}


printdbl(lxy,fmat,base)
long lxy;
char fmat;
int base;
{
        int digs[20];
        int *dptr;
        char k;
        unsigned long f ,g;
        unsigned long q;
        short lx,ly;

        ly=lxy;
        lx=(lxy>>16)&0xFFFF;
        dptr=digs;

        if ( fmat=='D' || fmat=='r' ) {
                if ( lxy<0 ) {
                        *digitptr++='-';
                        lxy = -lxy;
                }
                f = lxy;
        } else {
                if ( lx==-1 ) {
                        *digitptr++='-';
                        f=leng(-ly);
                } else {
                        f = lxy;
                }
        }
        while ( f ) {
                q=f/base;
                g=q;
                *dptr++ = f-g*base;
                f=q;
        }
        if ( dptr==digs || dptr[-1]>9 ) {
                *dptr++=0;
        }
        while ( dptr!=digs ) {
                k = *--dptr;
                *digitptr++ = (k+(k<=9 ? '0' : 'a'-10));
        }
}


endline()
{

	if (maxpos <= charpos())
		printf("\n");
}
