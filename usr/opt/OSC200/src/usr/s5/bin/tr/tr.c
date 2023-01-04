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
static char	*sccsid = "@(#)$RCSfile: tr.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 06:57:56 $";
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
 * COMPONENT_NAME: (CMDFILES) commands that manipulate files
 *
 * FUNCTIONS: tr
 *
 * ORIGINS: 3, 26, 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 * OBJECT CODE ONLY SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * tr.c	1.8  com/cmd/files,3.1,9013 2/26/90 15:11:47
 */
#include <stdio.h>
#include <stdlib.h>
#include <locale.h>

/* tr - transliterate data stream */

#include "tr_msg.h"

nl_catd catd;
#define MSGSTR(Num, Str) catgets(catd, MS_TR, Num, Str)

#include <NLchar.h>

#ifndef KJI
#define	NCHARS	NLCHARMAX
#else
#define NCHARS  NLCOLMAX
#endif
typedef NLchar          CHAR;
int endchar = NCHARS;
#define	GETC()	(Aflag ? getchar() : NCgetchar())

/** the following should be in the library instead **/
#define	NLsgetc(s)	(_NCis2((s)[0], (s)[1]) ? (s)+=2, _NCd2((unsigned char)((s)[-2]), ((unsigned char)((s)[-1]))) : (unsigned char)*(s)++)
#define	NCgetchar()	NCgetc(stdin)
#define	NCputchar(c)	NCputc(c, stdout)
#define NUMASCIIBITS	7
#define KANJIMASK	0x80

#ifndef KJI
/*
 * NAME: NCgetc
 *
 * FUNCTION: get the next character, if it is a double byte
 *           character, then get the second byte.
 */
int NCgetc(iop) register FILE *iop; {
    register int c0, c1;
    c0 = getc(iop);
    if (!NCisshift(c0)) {
	return (c0);
    } else if ((c1 = getc(iop)) == EOF || (c1 & KANJIMASK) == 0) {
	(void)ungetc(c1, iop);
	return (c0);
    } else {
	return (_NCd2(c0, c1));
    }
}

/*
 * NAME: NCputc
 *
 * FUNCTION: put character out to iop, if double byte character put out
 *           second byte too.
 */
int NCputc(c, iop) NLchar c; register FILE *iop; {
    if (c <= 0xff) {
	return (putc((int)c, iop));
    } else if (putc((int)(unsigned char)(0x21 - (c >> NUMASCIIBITS)), iop) != EOF &&
	    putc((int)(unsigned char)(c | KANJIMASK), iop) != EOF) {
	return (c);
    } else {
	return (EOF);
    }
}
#else /* KJI */
#define NCgetc(iop)     getwc(iop)
#define NCputc(c,iop)   putwc(c, iop)
#endif
/** end library **/

#ifdef STRIPNULLS
#define	EOS	0
#else
#define	EOS	NCHARS
#endif

#ifdef STRIPNULLS
typedef	unsigned char	Char;
#else
typedef	unsigned short	Char;
#endif

int dflag = 0;
int sflag = 0;
int cflag = 0;
int Aflag = 0;
Char save = 0;
Char code[NCHARS];
char squeez[NCHARS];
Char vect[NCHARS];
struct string { int last, max, rep; char *p; } string1, string2;

/*
 * NAME: tr [-cdsA] [string 1 [string2]]
 *                                                                    
 * FUNCTION: copies charcters from the standard input to the 
 *    standard output with substitution or deletion of selected
 *    characters.  Input characters from string1 are replaced with
 *    the corresponding characters in string2.
 */  
main(argc,argv)
int argc;
char **argv;
{
	register int i;
	int j;
	register int c, d;
	Char *compl;

	(void ) setlocale(LC_ALL,"");
	catd = catopen(MF_TR, 0);
	string1.last = string2.last = 0;
	string1.max = string2.max = 0;
	string1.rep = string2.rep = 0;
	string1.p = string2.p = "";

	if(--argc>0) {
		argv++;
		if(*argv[0]=='-'&&argv[0][1]!=0) {
			while(*++argv[0])
				switch(*argv[0]) {
				case 'c':
					cflag++;
					Aflag++;	/* for SVID-2 compl */
					endchar = 256;	/* for SVID-2 compl */
					continue;
				case 'd':
					dflag++;
					continue;
				case 's':
					sflag++;
					continue;
				case 'A':
					Aflag++;
					endchar = 256;
					continue;

	    			default:
		    			fprintf(stderr, 
		       			MSGSTR(BADUSE,"usage:\ttr [ -c -cs -s -A ] string1 string2\n\
\ttr [ -d -cd ] string1\n") );
		    			exit(1);   
				}
			argc--;
			argv++;
		}
	}
	if(argc>0) string1.p = argv[0];
	if(argc>1) string2.p = argv[1];
	if(cflag) {
		for(i=0; i<endchar; i++)
			vect[i] = 0;
		while ((c = next(&string1)) != EOS)
		{
#ifdef KJI
			if (!Aflag) c = _NCmap(c);
#endif
			vect[c] = 1;
		}
		j = 0;
		for(i=0; i<endchar; i++)
#ifdef KJI
			if(vect[i]==0) vect[j++] = (Aflag ? i : _NCunmap(i));
#else
			if(vect[i]==0) vect[j++] = i;
#endif
		vect[j] = EOS;
		compl = vect;
	}
	for(i=0; i<endchar; i++) {
		code[i] = EOS;
		squeez[i] = 0;
	}
	for(;;){
		if(cflag) c = *compl++;
		else c = next(&string1);
		if(c==EOS) break;
		d = next(&string2);
		if(d==EOS) d = c;
#ifdef KJI
		if (!Aflag) c = _NCmap(c);
#endif
		code[c] = d;
#ifdef KJI
		if (!Aflag) d = _NCmap(d);
#endif
		squeez[d] = 1;
	}
#ifdef KJI
	while ((d = next(&string2)) != EOS) {
		if (!Aflag) d = _NCmap(d);
		squeez[d] = 1;
	}
#else
	while ((d = next(&string2)) != EOS)
		squeez[d] = 1;
#endif
	for(i=0;i<endchar;i++) {
#ifdef KJI
		if(code[i]==EOS) code[i] = (Aflag ? i : _NCunmap(i));
#else
		if(code[i]==EOS) code[i] = i;
#endif
		else if(dflag) code[i] = EOS;
	}

	while ((c = GETC()) != EOF) {
#ifdef STRIPNULLS
		if(c == 0) continue;
#endif
#ifdef KJI
		if (!Aflag) c = _NCmap(c);
		if ((c = code[c]) != EOS) {
			if (!Aflag) d = _NCmap(c);
			if(!sflag || c!=save || !squeez[d])
#else
		if ((c = code[c]) != EOS)
			if(!sflag || c!=save || !squeez[c])
#endif
				NCputchar(save = c);
#ifdef KJI
	}
#endif
	}
	exit(0);
/* NOTREACHED */
}

/*
 * NAME: next
 *
 * FUNCTION:  get the next charater from string s
 */
int next(s)
struct string *s;
{
	int a, b, c, n;
	int base;
	char *msgstr;

	if(--s->rep > 0) return(s->last);
#ifdef KJI
	if(s->last < s->max) return(s->last = next_kanji(s->last));
#else
	if(s->last < s->max) return(++s->last);
#endif KJI
	if(*s->p=='[') {
		nextc(s);
		s->last = a = nextc(s);
		s->max = 0;
		switch(nextc(s)) {
		case '-':
			b = nextc(s);
			if(b<a || *s->p++!=']')
				goto error;
			s->max = b;
			return(a);
		case '*':
			base = (*s->p=='0') ? 8 : 10;  /* which base */
			n = 0;
			while((c = *s->p)>='0' && c<'0'+base) {
				n = base*n + c - '0';
				s->p++;
			}
			if(*s->p++ != ']') goto error;
#ifdef KJI
			if(n==0) n = NCHARS-1; /* 1000 is too small for KANJI */
#else
			if(n==0) n = 1000;
#endif
			s->rep = n;
			return(a);
		default:
		error:
			msgstr = MSGSTR(BADSTR, "Bad string\n");
			write(2,msgstr,strlen(msgstr));
			exit(1);
		}
	}
	return(nextc(s));
}

/*
 * NAME: nextc
 *
 * FUNCTION: get the next character from string s
 */
int nextc(s)
struct string *s;
{
	register int c, i, n;
	char *msgstr;

	c = (Aflag ? (unsigned char)*s->p++ : NLsgetc(s->p));
	if (c == '\0') {
		--s->p;
		return (EOS);
	}
	if(c=='\\') {          /* convert string to octal number */
		i = n = 0;
		while(i<3 && (c = *s->p)>='0' && c<='7') {
			n = n*8 + c - '0';
			i++;
			s->p++;
		}
		if(i>0) c = n;
		else c = *s->p++;
		if (c >= endchar) {
			msgstr = MSGSTR(BADVAL, "Bad octal value\n");
			write(2, msgstr, strlen(msgstr));
			exit(1);
		}
#ifdef STRIPNULLS
		if(c==0) *--s->p = 0;
#endif
	}
	return(c);
}
#ifdef KJI
int next_kanji(k)
int k;
{
unsigned int c1, c2, p,ku,ten,low,high;

       if(k < 256) return(++k);
        c1 = (0xff00 & k)>>8;
        c2 = 0xff & k;
        p = 0;
        if (c2 < 0x7f)
              c2 -= 0x1f;
        else if (c2 < 0x9f)
              c2 -= 0x20;
        else {
              c2 -= 0x7e;
              p++;
              }
        if (c1 < 0xa0)
                ku = (((c1 - 0x81) << 1) + 0x21 + p) - 32;
        else
                ku = (((c1 - 0xe0) << 1) + 0x5f + p) - 32;
        ten = c2 -32 ;
        if(++ten > 94){ ++ku;ten=1;}
        c1= ku-1 + 32;
        c2 = ten + 32;
        if (c1 < 0x5e)
            high = (c1 >> 1) + 0x71;
        else
            high = (c1 >> 1) + 0xb1;
        if (c1 & 1)
            low = c2 + 0x7e;
        else if (c2 <= 0x5f)
            low = c2 + 0x1f;
        else
            low = c2 + 0x20;
        return(0xffff &(high <<8 | low));
}
#endif KJI
