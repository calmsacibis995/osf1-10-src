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
static char	*sccsid = "@(#)$RCSfile: trbsd.c,v $ $Revision: 4.2.2.2 $ (DEC) $Date: 1992/09/15 13:49:56 $";
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
 * COMPONENT_NAME: CMDFILES: tr
 *                                                                    
 * ORIGIN: IBM, BSD
 *
 * IBM CONFIDENTIAL
 * Copyright International Business Machines Corp. 1985, 1988
 * Unpublished Work
 * All Rights Reserved
 * Licensed Material - Property of IBM
 *
 * RESTRICTED RIGHTS LEGEND
 * Use, Duplication or Disclosure by the Government is subject to
 * restrictions as set forth in paragraph (b)(3)(B) of the Rights in
 * Technical Data and Computer Software clause in DAR 7-104.9(a).
 * 
 * tr.c	1.9  com/cmd/files/trbsd,3.1,9013 3/15/90 17:05:28";
 */                                                                   
/*
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */
#include <stdio.h>
#include <locale.h>
extern void exit();

/* trbsd - transliterate data stream */

#include "trbsd_msg.h"
#define MSGSTR(Num, Str) NLgetamsg(MF_TRBSD, MS_TRBSD, Num, Str)

#include <NLchar.h>
#ifndef KJI
#define	NCHARS	NLCHARMAX
#else
#define NCHARS  NLCOLMAX
#endif
typedef	int		CHAR;
int endchar = NCHARS;
#define	NCgetchar()	getwc(stdin)
#define	NCputchar(c)	putwc(c, stdout)
#define	GETC()	(Aflag ? getchar() : NCgetchar())

/** the following should be in the library instead **/
#define	NLsgetc(s)	(_NCis2((s)[0], (s)[1]) ? (s)+=2, _NCd2((unsigned char)((s)[-2]), ((unsigned char)((s)[-1]))) : (unsigned char)*(s)++)
/** end library **/

#ifdef STRIPNULLS
#define	EOS	0
#else
#define	EOS	NCHARS
#endif

int dflag = 0;
int sflag = 0;
int cflag = 0;
int Aflag = 0;
CHAR save = 0;
CHAR code[NCHARS];
char squeez[NCHARS];
CHAR vect[NCHARS];
struct string { int last, max; char *p; } string1, string2;
int String2found;       /* nonzero iff nonnull String2 was specified */ /* 001 - tomp */

/*
 * NAME: trbsd [-Acs] String1 String2
 *       trbsd -d [-Ac] String1
 *
 * FUNCTION:  translate characters
 * FLAGS:
 *   -A       Translates on a byte-by-byte basis.  When you specify
 *            this flag, trbsd does not support extended characters.
 *   -c       Complements the set of characters in string1 with respect to 
 *            string2.
 *   -d       Deletes all input characters in string1.
 *   -s       Squeezes all strings of repeated output characters that are in 
 *            string2 to single characters.
 */
main(argc,argv)
char **argv;
{

	extern int optind;   /* 001 - tomp */
        extern int optopt;   /* 001 - tomp */
        extern char *optarg; /* 001 - tomp */
        extern int opterr;   /* 001 - tomp */

        int oc, badopt;      /* 001 - tomp */
	register i;
	int j;
	register d;
	int c;
	CHAR *compl;
	int lastd;

	(void) setlocale( LC_ALL, "" );
	string1.last = string2.last = 0;
	string1.max = string2.max = 0;
/* 001-start - tomp */
	string1.p = string2.p = 0;

        badopt = 0;
        while ((oc = getopt(argc,argv,"cdsA")) != -1 ) {
                switch((unsigned char)oc) {
                case 'c':
                        cflag++;
                        break;
                case 'd':
                        dflag++;
                        break;
                case 's':
                        sflag++;
                        break;
                case 'A':
                        Aflag++;
                        endchar = 256;
                        break;
                default:
                        badopt++;       /* Option syntax or bad option.*/
                }
        }

                /* Get translation strings */
        switch(argc-optind) {
        case 2:
                string2.p = (char *) argv[optind+1];
        case 1:
                string1.p = (char *)argv[optind];
                break;
        default:
                badopt++;       /* Zero or (more than two) translation strings specified.*/
        }

        if (string2.p == NULL) {
                string2.p = (char *) "" ;
                String2found = 0;
        }
        else
          String2found = 1;

                /* Check for illegal combinations of options and String parameters:
                 *     -c  -d  -s    String1  String1,2
                 *      0   0   0       bad      ok
                 *      0   0   1       bad      ok
                 *      0   1   0       ok       bad
                 *      0   1   1       bad      bad
                 *      1   0   0       bad      ok
                 *      1   0   1       bad      ok
                 *      1   1   0       ok       bad
                 *      1   1   1       bad      bad
                 */
        if (  (!String2found
               && (  (!cflag && !dflag && !sflag)
                   ||(!cflag && !dflag &&  sflag)
                   ||(!cflag &&  dflag &&  sflag)
                   ||( cflag && !dflag && !sflag)
                   ||( cflag && !dflag &&  sflag)
                   ||( cflag &&  dflag &&  sflag) ) )
            ||(String2found
               && (  (!cflag &&  dflag && !sflag)
                   ||(!cflag &&  dflag &&  sflag)
                   ||( cflag &&  dflag && !sflag)
                   ||( cflag &&  dflag &&  sflag) ) ) ) {
                fprintf(stderr,MSGSTR(BADCOMB,"trbsd: Invalid combination of options and Strings.\n"));
                badopt++;
        }

                /* If any command errors detected, issue Usage message and terminate.*/
        if (badopt) {
                Usage();
        }
/* 001-finish - tomp */

	if(cflag) {
		for(i=0; i<endchar; i++)
			vect[i] = 0;
		while ((c = next(&string1)) != EOS) {
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
	lastd = 0;
	for(;;){

		if(cflag) c = *compl++;
		else c = next(&string1);
		if(c==EOS) break;
		d = next(&string2);
	 	if (d==EOS) 
			if (sflag && !lastd) d = c;
                        else d = lastd;
		else lastd = d;
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
			{
				int count = 0;
				char twochar[2];

				save = c;
				{
				NLchar nlc;
				nlc = c;
				count = NCenc(&nlc, twochar);
				}
				if (count > 1) {  
					putchar(twochar[0]);
					putchar(twochar[1]);
				} else
					putchar(c);
			}
#ifdef KJI
		}
#endif
	}
	exit(0);
/* NOTREACHED */
}


next(s)
struct string *s;
{
#ifdef KJI
int save;
#endif
again:
	if(s->max) {
#ifdef KJI
                save = s->last;
                s->last = next_kanji(s->last);
		if(save < s->max)
#else
		if(s->last++ < s->max)
#endif KJI
			return(s->last);
		s->max = s->last = 0;
	}
	if(s->last && *s->p=='-') {
		nextc(s);
		s->max = nextc(s);
		if(s->max==0) {
			s->p--;
			return('-');
		}
		if(s->max < s->last)  {
			s->last = s->max-1;
			return('-');
		}
		goto again;
	}
	return(s->last = nextc(s));
}
nextc(s)
struct string *s;
{
	register i, c, n;
#ifdef MSG
	char * msgstr;
#endif

	c = (Aflag ? (unsigned char)*s->p++ : NLsgetc(s->p));
	if (c == '\0') {
		--s->p;
		return (EOS);
	}
	if(c=='\\') {
		i = n = 0;
		while(i<3 && (c = *s->p)>='0' && c<='7') {
			n = n*8 + c - '0';
			i++;
			s->p++;
		}
		if(i>0) c = n;
		else c = *s->p++;
		if (c >= endchar) {
#ifdef MSG
			msgstr = MSGSTR(BADVAL, "Bad octal value\n");
			write(2, msgstr, strlen(msgstr));
#else
			write(2, "Bad octal value\n", 16);
#endif
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

/* 001-start - tomp */
/* NAME:        Usage
 * FUNCTION:    Issue Usage message to standard error and immediately terminate
 *              the trbsd command with return value 1.
 */
Usage()
{
        fprintf(stderr, MSGSTR(BADUSE,
"Usage: trbsd [-Acs] String1 String2\n\
\       trbsd -d [-Ac] String1\n") );
        exit(1);
}
/* 001-finish - tomp */
