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
/*	
 *	@(#)$RCSfile: NLregexp.h,v $ $Revision: 4.2.2.2 $ (DEC) $Date: 93/03/01 14:04:22 $
 */ 
/*
 */
/*
 * (c) Copyright 1990, 1991, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0.1
 */
/*
 * COMPONENT_NAME: (LIBCNLS) Standard C Library National Language Support
 *
 * FUNCTIONS:
 *
 * ORIGINS: 3 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1984 AT&T
 * All Rights Reserved
 *
 * THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
 * The copyright notice above does not evidence any
 * actual or intended publication of such source code.
 */

#ifndef _NLREGEXP_H_
#define _NLREGEXP_H_

#include <NLchar.h>
#include <values.h>


#define RE_DUP_MAX 255  /* Perhaps this should be in a different include file? */
/* This is the new (or large charset) version of regexp.h.              */
/* The main differences are that the bitmap previously used for         */
/* the [bracketed] range expression is replaced by a straight list      */
/* (due to the requirement for an 16,000+ bit maps), and that a         */
/* [:charclass:] definition is allowed within brackets. Ranges are      */
/* handled as a "substring" of entries, with an "or" rather than        */
/* "and" relationship. Note also that compares within brackets          */
/* is done on character values, except for dashranges, where collation  */
/* values are used.                                                     */
/* The normal method for defining character classes ([a-z]) does not    */
/* work well in an international environment; the new charclass element */
/* (with syntax "[:" name ":]", e.g. [:upper:]) provides the needed     */
/* capability.                                                          */
/*                                                                      */
/* Hex compile codes:                                                   */
/****************************************************/
/* function               normal  normal  normal    */
/*                                + STAR  +INTVL    */
/* CPAR   start group \(    02                      */
/* CCHR   normal char       04      05      06      */
/* CDOT   dot in this pos   08      09      0a      */
/* CENT   end group \)      0c      0d      0e      */
/* CBACK  \[1-9] indicator  10      11      12      */
/* CDOL   EOL anchor ($)    14                      */
/* CCEOF  end compiled pat  16                      */
/* CBRA   new [ string      18      19      1a      */
/* CNEG   new [^ string     1c      1d      1e      */
/* CLASS  [:cclass:]        20                      */
/* CEQV   [=x=] (not [=.=]) 22                      */
/* CELEM  [.xx.]            24                      */
/* CRNGE  new range (a-z)   26                      */
/* CKET   new ]             28                      */
/****************************************************/
/* A "typical" regular expression, e.g.                                   */
/*      'ab*[a[.LL.]c-f[:digit:]]\(.*\)'  (LANG set to Sp_SP)             */
/* would be compiled into (hex values):                                   */
/*                                                                        */
/*       04 61 05 62 18 00 10 04 00 61 24 01 7d 26                        */
/*       01 50 01 65 20 03 28 02 00 09 0c 00 16                           */
/*                                                                        */
/* which is:                                                              */
/*                                                                        */
/*      a       b*    [                 a         [.LL.]      c -         */
/*   04  61  05  62  18    00 10     04  00  61  24  01  7d  26   01  50  */
/*  CCHR a  CCHR b   CBRA  length   CCHR   a    CELEM  LL   CRNGE   c     */
/*          STAR           in bytes                                       */
/*                                                                        */
/*      f        [:digit:]       \(          .*    \)                     */
/*   01  65   20     03   28     02   00     09    0c  00      16         */
/*      f    CLASS digit CKET   CPAR group  CDOT  CENT group  CCEOF       */
/*                                   zero   STAR       zero               */
/*                                                                        */
/*                                                                        */
/* Note that character values are one or two bytes outside brackets,      */
/* two bytes inside brackets.                                             */
/*                                                                        */
/* The error numbers generated have the following meaning:                */
/*                      Note that 70 is new!!!!                           */
/*      ERROR(11)       Interval endpoint too large                       */
/*      ERROR(16)       Bad number                                        */
/*      ERROR(25)       "\digit" out of range                             */
/*      ERROR(36)       Illegal or missing delimiter                      */
/*      ERROR(41)       No remembered match string                        */
/*      ERROR(42)       \( \) imbalance                                   */
/*      ERROR(43)       Too many \(                                       */
/*      ERROR(44)       More than 2 numbers given in interval             */
/*      ERROR(45)       } expected after \                                */
/*      ERROR(46)       First number exceeds second in interval           */
/*      ERROR(49)       [] imbalance                                      */
/*      ERROR(50)       Regular expression overflow                       */
/*      ERROR(70)       Invalid endpoint in range                         */
/*                                                                        */


#define _CPAR   0x02            /* start group - \( - next is group #     */
#define _CCHR   0x04            /* normal char follows                    */
#define _CDOT   0x08            /* dot: any char...                       */
#define _CENT   0x0c            /* end group - \) - here                  */
#define _CBACK  0x10            /* \number; n follows                     */
#define _CDOL   0x14            /* end-of-line anchor ($)                 */
#define _CCEOF  0x16            /* end-of-line seen                       */
#define _CBRA   0x18            /* start new []; count & items follow     */
#define _CNEG   0x1c            /* start [^; count & items follow         */
#define _CLASS  0x20            /* charclass follows                      */
#define _CEQV   0x22            /* equiv class value follows              */
#define _CELEM  0x24            /* collation element follows              */
#define _CRNGE  0x26            /* range start and end chars follow       */
#define _CKET   0x28            /* end new brackets                       */

#define _STAR   0x01            /* asterisk, i.e., 0 or more            */
#define _INTVL  0x02            /* range \{m,n\} follows                */
#define _NEG    0x04            /* bracket expr negation                */

#define _NBRA   9               /* count of groups \(..\) */

#define _PLACE(c)       *ep++ = (c >> 8), *ep++ = c
#define _GETWC(sp)      (((unsigned char)sp[0] << 8) | (unsigned char)sp[1])

                                /* The following macro will return a wchar_t
                                   and place the char * pointer pas the
                                   converted single- or multibyte character
                                   It is used by _GETVAL.
                                 */
#define _CHNEXT(s)          (NCisshift( *(unsigned char*)s) ? s+=2, _NCd2(((unsigned char *)s)[-2], ((unsigned char *)s)[-1]) : *(unsigned char *)s++)

                                /* The following macro is called with a
                                   char * pointer and returns a collating
                                   value and a coluniq value. The char *
                                   is bumped to past the element (e.g.,
                                   past the "ch"). If 1-to-n, return -1
                                   and the coluniq value for the "repla-
                                   ced" character.
                                 */
#define _GETVAL(co,cu,s,p,ch) \
{ \
        int co_int = NCcollate(ch = _CHNEXT(s)); \
        if (co_int < 0) { \
                co = _NLxcolu(co_int, &s, &p, &cu); \
        } \
        else { \
                co = co_int; \
                cu = NCcoluniq(ch); \
        } \
}

/* Following variable names required by spec. */
char *loc1, *loc2;
int           circf;
int           sed, nbra;

/* Following variable names are undocumented, but required by sed. */
int      nodelim;
char *locs;
char *braslist[_NBRA];
char *braelist[_NBRA];

#include <NLctype.h>

#define __CHECK_FOR_NULL(character,eof,errornum) \
                         {   if (!(character) && (character) != (eof) ) \
                                    ERROR((errornum));                    }


/* As the "is" functions aren't functions, but macros, we cannot put    */
/* the "function" in the array below; thus another layer of indirection */

_ALPHA(c) {return(NCisalpha(c));}
_UPPER(c) {return(NCisupper(c));}
_LOWER(c) {return(NCislower(c));}
_DIGIT(c) {return(NCisdigit(c));}
_ALNUM(c) {return(NCisalnum(c));}
_SPACE(c) {return(NCisspace(c));}
_PRINT(c) {return(NCisprint(c));}
_PUNCT(c) {return(NCispunct(c));}
_XDIGIT(c) {return(isascii(c) && isxdigit(c));}
_CNTRL(c) {return(NCiscntrl(c));}
_GRAPH(c) {return(NCisgraph(c));}
#ifdef _KJI
_JALPHA(c) {return(isjalpha(c));}
_JDIGIT(c) {return(isjdigit(c));}
_JSPACE(c) {return(isjspace(c));}
_JPUNCT(c) {return(isjpunct(c));}
_JPAREN(c) {return(isjparen(c));}
_JKANJI(c) {return(isjkanji(c));}
_JHIRA(c) {return(isjhira(c));}
_JKATA(c) {return(isjkata(c));}
_JXDIGIT(c) {return(isjxdigit(c));}
#endif
static struct __isarray {
        char *isstr;
        int (*isfunc)();
} __istab[] = {
        { "alpha", _ALPHA },
        { "upper", _UPPER },
        { "lower", _LOWER },
        { "digit", _DIGIT },
        { "alnum", _ALNUM },
        { "space", _SPACE },
        { "print", _PRINT },
        { "punct", _PUNCT },
        { "xdigit", _XDIGIT },
        { "cntrl", _CNTRL },
        { "graph", _GRAPH }

#ifdef _KJI
                                ,
        { "jalpha", _JALPHA },
        { "jdigit", _JDIGIT },
        { "jspace", _JSPACE },
        { "jpunct", _JPUNCT },
        { "jparen", _JPAREN },
        { "jkanji", _JKANJI },
        { "jhira", _JHIRA },
        { "jkata", _JKATA },
        { "jxdigit", _JXDIGIT }
#endif

#define _NISTAB (sizeof(__istab) / sizeof(struct __isarray))
};
#define _IFBUFLEN 16
        static unsigned char  __ifbuf[_IFBUFLEN];

static int      __ebra;
static int      __low;
static int      __ssize;
static int	__getintvl();
static int	__ecmp();
static int	__isthere();

char *
#ifdef _NO_PROTO
compile(instring, ep, endbuf, eof)
register  char *ep;
char *instring, *endbuf;
int eof;
#else
compile( char *instring,
        register char *ep,
        char *endbuf,
        int eof)
#endif
{
        INIT    /* Dependent declarations and initializations */
        register c;
        wchar_t  wchr;
        wchar_t *p;
        unsigned char *lastep = 0;      /* addr of start of simple r-e,  */
                                /* for or-ing _INTVL or _STAR flag  */
        int cclcnt;
        unsigned char bracket[_NBRA], *bracketp;
        unsigned char *ib;
        unsigned char *cnclptr;         /* addr of _CBRA's count bytes */
        int dashfl;
        struct nextelt {
            char      class;
            char      rangeable;
            wchar_t     cvalue;
            wchar_t     uvalue;
        } next, prev;

        int closed;
        char neg;
        int lc;
        int i, cflg;

        c = GETC();
        __CHECK_FOR_NULL(c,eof,36);

        if(c == eof || c == '\n') {
                if(c == '\n') {
                                /* This apparently superfluous logic
                                 * is required by sed
                                 */
                        UNGETC(c);
                        nodelim = 1;
                }
                if(*(unsigned char *)ep == 0 && !sed)    /* WRONG  *ep uninitialized! */
                        ERROR(41);
                RETURN(ep);
        }
        bracketp = bracket;
        circf = closed = nbra = __ebra = 0;
        if(c == '^')
                circf++;
        else
                UNGETC(c);
        while(1) {
                        /* Will we overflow ep with this element?
                         * Aside from bracket lists, one r.e. element
                         * can produce no more than 3 bytes of compiled text.
                         */
            if(ep >= endbuf-3) ERROR(50);

            c = GETC();
            __CHECK_FOR_NULL(c,eof,36);

            if(c != '*' && ((c != '\\') || (PEEKC() != '{')))     /*}*/
                    lastep = (void *) ep;
            if(c == eof) {
                    *ep++ = _CCEOF;
                    RETURN(ep);
            }
            switch(c) {

        case '.':
                *ep++ = _CDOT;
                continue;

        case '\n':
                if(!sed) {
                        UNGETC(c);
                        *ep++ = _CCEOF;
                        nodelim = 1;
                        RETURN(ep);
                }
                else ERROR(36);
        case '*':
                        /* Accept * as ordinary character if first in
                         * pattern or if following \(.
                         * Undocumented, possibly POSIX-conflicting:
                         * also accept * following \).
                         */
                if(lastep == 0 || *lastep == _CPAR || *lastep == _CENT)
                        goto defchar;
                *lastep |= _STAR;
                continue;

        case '$':
                if(PEEKC() != eof && PEEKC() != '\n')
                        goto defchar;
                *ep++ = _CDOL;
                continue;

        case '[':
        /*
         *      Support for Posix NL bracket extensions, including
         *      equivalence classes and collating symbols.
         *      Syntactic rules for dash ranges are simplified: '-' is
         *      ordinary after '[', before ']', and immediately following
         *      a dashrange '-'.
         *      Any element may appear syntactically as a dashrange endpoint,
         *      including those that turn out to be semantically illegal:
         *      noncollating char; [:class:]; start>end; or previous endpoint
         *      as starting point, e.g. a-m-z.
         */
                if((c = PEEKC()) == '^') {
                    *ep++ = _CBRA|_NEG;     /* Bracket-^ start  */
                    GETC();
                }
                else
                    *ep++ = _CBRA;              /* Bracket start, no ^  */

                cnclptr = (void *)ep;
                *ep++ = 0;              /* Space for count,   */
                *ep++ = 0;              /* filled in at ]     */
                prev.class = 0;
                next.class = 0;
                dashfl = 0;
                if ((c = PEEKC()) == '-' || c == ']') {
                    prev.class = _CCHR;
                    prev.rangeable = 1;
                    prev.uvalue = c;
                    GETC();
                }

                while (1) {             /* Iterate over elements of bracket list */
                    if(ep >= endbuf-6)
                        ERROR(50);

                                /* Worst case: 6 bytes could be added to
                                 * ep in the case of  ... - ]
                                 */

                    c = GETC();
                    __CHECK_FOR_NULL(c,eof,49);

                    if (c == '\0') ERROR(49); /* Stop when NUL is found*/
                    if (c == ']') {
                        if (prev.class != 0) {
                            UNGETC(c);
                            goto stuffp;
                        }
                        if (dashfl) {
                                /* Trailing dash is ordinary character */
                            *ep++ = _CCHR;
                            wchr = '-';
                            _PLACE(wchr);
                        }
                        break;
                    }
                    else if (c == '-' && !dashfl) {
                        dashfl = 1;
                        continue;
                    }
                                /* Get next element into structure
                                   next.  It may be a:

                                    _CLASS  [:class:]
                                    _CEQV    [=collating-element=]
                                    _CDOT    [=.=]
                                    _CELEM   [.xx.] (collating-symbol)
                                    _CCHR    character
                                 */
                    else if (c == '[' &&
                        ((lc=PEEKC()) == ':' || lc == '.' || lc == '=')) {
                        ib = __ifbuf;
                        GETC();
                        while ( (c = GETC()) != lc || PEEKC() != ']') {
                            __CHECK_FOR_NULL(c,eof,49);

                            if (c == '\n' || c == eof) ERROR(49);
                            *ib++ = c;
                            if (NCisshift(c)) *ib++ = GETC();
                            if (ib>__ifbuf+_IFBUFLEN-2)
                                ib-=2;
                                        /* ifbuf is long enough that if we
                                         * discard characters here, the contents
                                         * are already known to be invalid.
                                         */

                        }
                        *ib = '\0';
                        ib = __ifbuf;
                        GETC();         /* Advance over trailing ]      */
                        if (lc == ':') {
                            for (i = 0; i < _NISTAB; i++) {
                                if((strcmp((char *)__ifbuf,__istab[i].isstr))==0)
                                    break;
                            }
                            if (i >= _NISTAB) ERROR(49);
                            next.class = _CLASS;
                            next.rangeable = 0;
                            next.uvalue = i;
                        }
                        else if (lc == '.') {
                            next.class = _CELEM;
                            next.rangeable = 1;

                            _GETVAL(next.cvalue,next.uvalue,ib,p,wchr);
                            if ((next.cvalue == 0) || (ib[0] != '\0'))
                                ERROR(36);
                        }
                        else {
                                        /* Equivalence class.  Special-case '.'
                                         * to mean any char with a collating value;
                                         * represent as CDOT in compiled string.
                                         */
                            if ((__ifbuf[0] == '.') && (__ifbuf[1] == '\0')){
                                next.class = _CDOT;
                                next.rangeable = 0;
                            }
                            else {
                                next.class = _CEQV;
                                _GETVAL(next.cvalue,next.uvalue,ib,p,wchr);
                                next.rangeable = 1;
                                if ((next.cvalue == 0) || (ib[0] != '\0'))
                                    ERROR(36);
                                if (next.cvalue == next.uvalue)
                                        next.class = _CELEM;


                            }
                        }
                    }
                    else {                      /* Ordinary character,
                                                 * including [ followed by
                                                 * anything but :=.
                                                 */
                        next.class = _CCHR;
                        next.rangeable = 1;
                        if (NCisshift(c))
                             _NCdec2(c, GETC(), c);
                        next.uvalue = c;
                    }
        /* Next element has been built and placed in next.
         * Now dispose of it.                                   */
                    if (dashfl) {
                        dashfl = 0;
                        /*
                         * '-' seen, not immediately following '['.
                         * The element preceding '-' is in struct prev and
                         * the element following is in struct next.
                         * It's legal if both prev and next are collatable
                         * and prev <= next.
                         */
                        if (prev.class == 0 ||
                                (!prev.rangeable || !next.rangeable))
                            ERROR(70);
                                        /* one end of range was char-class
                                         * or noncollating char, or 'start'
                                         * of range was really endpoint of
                                         * a preceding range, e.g. [a-m-z]
                                         */
                        prev.rangeable = 0;
                                        /* Inhibit [a-m-z]              */
                        if (prev.class == _CCHR) {
                            ib = __ifbuf;
                            _NCe2(prev.uvalue, ib[0], ib[1]);
                            _GETVAL(prev.cvalue,prev.uvalue,ib,p,wchr);
                            if (prev.cvalue == 0)
                                ERROR(70);
                        }
                        if (next.class == _CCHR) {
                            ib = __ifbuf;
                            _NCe2(next.uvalue, ib[0], ib[1]);
                            _GETVAL(next.cvalue,next.uvalue,ib,p,wchr);
                            if (next.cvalue == 0)
                                ERROR(70);
                        }
                        if (next.uvalue < prev.uvalue)
                                ERROR(70);

                        *ep++ = _CRNGE;
                        if (prev.class == _CEQV)
                            _PLACE(prev.cvalue);
                        else
                            _PLACE(prev.uvalue);
                        _PLACE(next.uvalue);
                        if (next.class == _CEQV) {
                            *ep++ = _CEQV;
                            _PLACE(next.cvalue);
                        }
                        prev.class = 0;
                    }
                    else {              /* not a range */
                        if (prev.class != 0) {
                                        /* Insert class and value in ep.
                                         * If [:class:], 1 byte of value;
                                         * if [=.=], no value;
                                         * otherwise 2 bytes of value.
                                         */
        stuffp:             *ep++ = prev.class;
                            switch (prev.class) {
                              case _CLASS:
                                *ep++ = _NCbot(prev.uvalue);
                                break;
                              case _CEQV:
                                _PLACE(prev.cvalue);
                                break;
                              case _CELEM:
                              case _CCHR:
                                _PLACE(prev.uvalue);
                              case _CDOT:
                                break;
                            }
                        }
                        prev=next;
                        next.class = 0;
                    }
                }

                *ep++ = _CKET;			/* trailing sentinel          */
                wchr = ((unsigned char *)ep)-cnclptr;	/* Store [] string length     */
                *cnclptr = _NCtop(wchr);     	/* at head of string            */
                *(cnclptr+1) = _NCbot(wchr);

                continue;

            case '\\':
                if ((c = GETC()) == '\0') ERROR(36);
                switch(c) {

                case '(':
                    if(nbra >= _NBRA)
                            ERROR(43);
                    *bracketp++ = nbra;
                    *ep++ = _CPAR;
                    *ep++ = nbra++;
                    continue;

                case ')':
                    if(bracketp <= bracket )
                            ERROR(42);
                    *ep++ = _CENT;
                    *ep++ = *--bracketp;
                    closed++;
                    continue;

                case '{':                                       /*}*/
                    if(lastep == 0)
                            goto defchar;
                    *lastep |= _INTVL;
                    cflg = 0;
                    c = GETC();
            nlim:
                    i = 0;
                    do {
                            if('0' <= c && c <= '9')
                                    i = 10 * i + c - '0';
                            else
                                    ERROR(16);
                    } while(((c = GETC()) != '\\') && (c != ','));
                    if(i > RE_DUP_MAX)
                            ERROR(11);
                    *ep++ = i;
                    if(c == ',') {
                            if(cflg++)
                                    ERROR(44);
                            if((c = GETC()) == '\\') {
                              cflg = -1;
                              if(i == 0) {
                                *(ep-1) = 2;
                                *ep++ = 0;
                              } else
                                *ep++ = i - 1;
                            }
                            else goto nlim;              /* get 2'nd number */
                    }                           /*{*/
                    if(GETC() != '}')
                            ERROR(45);
                    if(!cflg)   /* one number */
                            *ep++ = i;
                    else if( (cflg != -1) && (*((unsigned char *)ep -1)
                            < *((unsigned char *)ep -2) ) )
                            ERROR(46);
                    continue;

                case '\n':
                    ERROR(36);

                case 'n':
                    c = '\n';
                    goto defchar;

                default:
                    if(c >= '1' && c <= '9') {
                            if((c -= '1') >= closed)
                                    ERROR(25);
                            *ep++ = _CBACK;
                            *ep++ = c;
                            continue;
                    }
                }
/* Drop through to default to use \ to turn off special chars */
            defchar:
            default:
                lastep = (void *)ep;
                *ep++ = _CCHR;
                *ep++ = c;
                if (NCisshift(c))
                     *ep++ = GETC();
            }
        }
}

#ifdef _NO_PROTO
step(p1, p2)
register char *p1, *p2;
#else
step(register char *p1, register char *p2)
#endif
{
        register unsigned c;

        if(circf) {
                loc1 = (void *) p1;
                return(advance(p1, p2));
        }
        /* fast check for first character */
        if(*(unsigned char *)p2 == _CCHR) {
                c = ((unsigned char *)p2)[1];
                do {
                        if (*(unsigned char*)p1==c)
                        if(advance(p1, p2)) {
                                loc1 = (void *) p1;
                                return(1);
                        }
                        if (NCisshift(*(unsigned char *)p1)) p1++;
                } while(*p1++);
                return(0);
        }
                /* regular algorithm */
        do {
                if(advance(p1, p2)) {
                        loc1 = (void *) p1;
                        return(1);
                }
                 if (NCisshift(*(unsigned char *)p1)) p1++;
        } while(*p1++);
        return(0);
}

#ifdef _NO_PROTO
advance(lp, ep)
char *lp;
register char *ep;
#else
advance(char *lp, register char *ep)
#endif
{
        char *curlp, *nxtep, *curwp;
        int c2, lc;
        register int c;
        char *bbeg;
        int ct;
        char *next_character;   /* This is used to point to the */
                                         /* next 'character' in the 'lp' */
                                         /* string. The __isthere()      */
                                         /* routine will pass this value */
                                         /* back since it knows how big  */
                                         /* the 'character' is.          */

        while(1) {
                switch(*(unsigned char *)ep++) {

            case _CCHR:
                    c = *(unsigned char *)ep++;
                    if(c == *(unsigned char *)lp++)
                            if (!NCisshift(c) || *(unsigned char *)ep++ == *(unsigned char *)lp++) continue;
                    return(0);

            case _CDOT:
                    if (*lp==0)
                            return(0);
                    else lp +=NLchrlen(lp);
                    continue;

            case _CDOL:
                    if(*lp == 0)
                            continue;
                    return(0);

            case _CCEOF:
                    loc2 = (void *) lp;
                    return(1);

            case _CPAR:
                    braslist[*(unsigned char *)ep++] = lp;
                    continue;

            case _CENT:
                    braelist[*(unsigned char *)ep++] = lp;
                    continue;

            case _CCHR | _INTVL:
                    c = *(unsigned char *)ep++;
                    if (NCisshift(c)) {
                            c2 = *(unsigned char *)ep++;
                            __getintvl(ep);
                            while(__low--) {
                                    if(*(unsigned char *)lp++ != c || *(unsigned char *)lp++ != c2)
                                            return(0);
                            }
                            curlp = lp;
                            while (__ssize-- && *(unsigned char *)lp == c && ((unsigned char *)lp)[1] == c2) lp += 2;
                    } else {
                            __getintvl(ep);
                            while(__low--) {
                                    if(*(unsigned char *)lp++ != c)
                                            return(0);
                            }
                            curlp = lp;
                            while (__ssize-- && *(unsigned char *)lp == c) lp++;
                    }
                    ep += 2;
                    goto star;

            case _CDOT | _INTVL:
                    __getintvl(ep);
                    while(__low--) {
                            if (NCisshift(*(unsigned char *)lp)) lp++;
                            if(*(unsigned char *)lp++ == '\0')
                                    return(0);
                    }
                    curlp = lp;
                    while(__ssize-- && *(unsigned char *)lp != '\0') {
                            lp += (NCisshift(*(unsigned char *)lp) ? 2 : 1);
                    }
                    ep += 2;
                    goto star;

            case _CBACK:
                    bbeg = braslist[*(unsigned char *)ep];
                    ct = braelist[*(unsigned char *)ep++] - bbeg;

                    if(__ecmp(bbeg, lp, ct)) {
                            lp += ct;
                            continue;
                    }
                    return(0);

            case _CBACK | _STAR:
                    bbeg = braslist[*(unsigned char *)ep];
                    ct = braelist[*(unsigned char *)ep++] - bbeg;
                    curlp = lp;
                    while(__ecmp(bbeg, lp, ct))
                            lp += ct;

                    while(lp >= curlp) {
                            if(advance(lp, ep)) return(1);
                            lp -= ct;
                    }
                    return(0);


            case _CDOT | _STAR:
                    curlp = lp;
                    while(*(unsigned char *)lp) lp++;
                    goto star;

            case _CCHR | _STAR:
                    curlp = lp;
                    c = *(unsigned char *)ep++;
                    if (NCisshift(c)){
                            c2 =  *(unsigned char *)ep++;
                            while(*(unsigned char *)lp == c && ((unsigned char *)lp)[1] == c2) lp += 2;
                    }else {
                            while (*(unsigned char *)lp == c) lp++;
                    }
                    goto star;

            case _CBRA:
            case _CBRA | _NEG:
                    nxtep = ep + _GETWC(ep);
                    ep += 2;
                    if(!__isthere(lp, ep,&next_character)) return(0);

                    lp = next_character;
                    ep = nxtep;
                    continue;

            case _CBRA | _INTVL:
            case _CBRA | _INTVL | _NEG:
                    nxtep = ep + _GETWC(ep);
                    ep += 2;
                    __getintvl(nxtep);
                    while (__low--) {
                            if(!__isthere(lp, ep,&next_character)) return(0);
                            lp = next_character;
                    }
                    curlp = lp;
                    while(__ssize-- && __isthere(lp, ep, &next_character))
                          lp =  next_character;
                    ep = nxtep += 2;
                    goto star;

            case _CBRA | _STAR:
            case _CBRA | _STAR | _NEG:
                    nxtep = ep + _GETWC(ep);
                    ep += 2;
                    curlp = lp;
                    while(__isthere(lp, ep,&next_character))
                          lp  = next_character;

                    ep = nxtep;
                    goto star;

            star:

/* The logic of the backtracking done in this routine is based on the
 * characteristics of the SJIS code set; where a single-byte character must
 * be in the range 0x00-0xff, 0xa0-0xdf, and the first byte ("shift byte")
 * of a 2-byte character must be in the range 0x80-0x9f, 0xe0-0xfc.
 *
 * Let "N" denote a non-shift byte (ASCII or katakana), and "S" denote a
 * shift byte.  If the byte stream ends with "...S", it must end with a
 * two-byte character (otherwise we would not be at this point in the stream).
 * Otherwise, "...NSS...SSN" parses as "...N(SS)...(SS)(N)" if there are an
 * even number (including zero) of shift bytes preceding the last N, or
 * "...N(SS)...(SS)(SN)" if there are an odd number.
 * (And similarly if we reach the anchor point, curlp, instead of finding
 * an N).
 * In the worst case, this algorithm has to back up all the way to the
 * beginning, but it will only have to do so once.  (After backstepping the
 * last character, the preceding string of (SS) characters can be
 * backstepped quickly.)  Thus, we can process an entire ".*" in linear time.
 *
 * Note that this routine also works well in the NLS case, as we never
 * will find a shift byte; so it will just step back once...
 */
                    while ((char *)lp != locs) {
                            if (advance(lp, ep)) return (1);
                            if (lp <= curlp) return (0);
                            --lp;
                            if (NCisshift(*(unsigned char *)lp)) lp--;
                            else {
                               for (curwp = lp;
                                    curwp > curlp && NCisshift(curwp[-1]);
                                    --curwp);
                               if ((lp-curwp) & 1) --lp;
                            }
                    }
                    return (0);

                }
        }
}
                        /* this routine gets the low (or only) value into
                         * __low, and the delta to the high value into
                         * __ssize (for \{m,\}, set _ssize to max.)
                         * RE_DUP_MAX is a POSIX variable.
                         */
static int
#ifdef _NO_PROTO
__getintvl(str)
unsigned char *str;
#else
__getintvl(unsigned char *str)
#endif
{
        int  __high;

        __low = *str++;
       __high = *str;
        if(__low > __high) {
          __ssize = MAXINT;
          if((__low == 2) && (__high == 0))
            __low = 0;
        }
        else
          __ssize = __high - __low;
}

static __ecmp(a, b, count)
register char   *a, *b;
register        count;
{
        while(count--)
                if(*a++ != *b++)
                        return(0);
        return(1);
}

/* This routine replaces the _ISTHERE macro; it matches the pattern         */
/* within brackets (bp) against the char in sp. It will advance in the bp   */
/* expression until a match occurs or the pattern is empty.                 */
/* The _NEG case is handled by switching the return codes.                  */

static int
#ifdef _NO_PROTO
__isthere(sp, bp,next_character)
register unsigned char *sp, *bp, **next_character;
#else
__isthere(unsigned char *sp, unsigned char *bp, unsigned char **next_character)
#endif
{
                        int     c, lc;
                        wchar_t co;
                        wchar_t cu;
                        wchar_t w;
                        wchar_t *p;
                        int     ishere, nothere;
                        unsigned char *sa = sp;

                        nothere = (bp[-3] >> 2) & 1;
                        ishere = nothere ^ 1;

                        if(sp == NULL || *sp == '\0' ||
                              next_character == NULL)
                                  return(0);

                        *next_character = sp;

                       /* Set *next_character to the next 'character' */
                       /* in the string.  This is so that when there  */
                       /* are multiple character 'characters' like LL */
                       /* the string pointer can be properly incremented. */
                       /* The value *next_character is incemented here  */
                       /* so that if 'sa' is not incremented,           */
                       /* *next_character will still point to the next  */
                       /* 'character'.                                  */
          
                       _GETVAL(co, cu, (*next_character), p, w);  
                       

                        if(NCisshift(*sa))
                                c = _GETWC(sa);
                        else    c = sa[0];


                        do {

                            sa = sp;
                            switch(*bp++) {

                                case _CCHR:
                                        lc = _GETWC(bp);
                                        if(lc == c) {
                                                return(ishere);
                                        }
                                        bp += 2;
                                        break;

                                case _CRNGE:
                                        _GETVAL(co, cu, sa, p, w);
                                        lc = _GETWC(bp);
                                        if(cu >= lc) {
                                                lc = ((bp[2] << 8) | bp[3]);

                                                if(cu <= lc) {
                                                       *next_character = sa;
                                                        return(ishere);
                                                }
                                        }
                                        bp += 4;
                                        break;

                                case _CELEM:
                                        _GETVAL(co, cu, sa, p, w);
                                        if(cu == _GETWC(bp)) {
                                                *next_character = sa;
                                                return(ishere);
                                        }
                                        bp += 2;
                                        break;

                                case _CEQV:
                                        _GETVAL(co, cu, sa, p, w);
                                        if(co == _GETWC(bp)) {
                                                *next_character = sa;
                                                return(ishere);
                                        }
                                        bp += 2;
                                        break;

                                case _CDOT:
                                        _GETVAL(co, cu, sa, p, w);
                                        if (co != 0) {
                                                *next_character = sa;
                                                return(ishere);
                                        }
                                        break;

                                case _CLASS:
                                        if((*__istab[*bp++].isfunc)(c)) {
                                                return(ishere);
                                        }
                                        break;

                                default:
                                        break;
                                }

                        } while( *bp != _CKET);

                        /* If the pointer (sa) has been incremented, then */
                        /* set *next_character to point to the new location */
                        if (sa != sp)
                            *next_character = sa;

                        return(nothere);
}
#endif /* _NLREGEXP_H_ */
