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
 * @(#)$RCSfile: egrep.y,v $ $Revision: 4.2.11.2 $ (DEC) $Date: 93/02/08 09:47:46 $
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0
 */
/*
 * COMPONENT_NAME: (CMDSCAN) commands that scan files
 *
 * FUNCTIONS:
 *
 * ORIGINS: 3, 26, 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 * OBJECT CODE ONLY SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 * Copyright 1976, Bell Telephone Laboratories, Inc.
 * 
 * egrep.y	1.18  com/cmd/scan,3.1,9021 5/8/90 13:37:50 
 */
/*
 * egrep -- print lines containing (or not containing) a regular expression
 *
 *      status returns:
 *              0 - ok, and some matches
 *              1 - ok, but no matches
 *              2 - some error
 */

/* for 3.1, tokens are started at 16129 rather than the YACC default of 257
   If size of largest compacted character or largest allocated collation
   value ever goes beyond this number then this number must be changed
   (or else change YACC!!)

   1. Size of gotofn array varies depending on NLS or JLS. The maximum
      possible value for NLS is ~ 900; so we give it 1000. For JLS,
      it probably will not exceed 12000, but could, in theory, be
      up to 32K. 16128 will be used...

   2. Because ranges must be expressed in collating values and all other
      in character values,  new array (gotofn1) is implemented.
      Positions in this is in colval rather than in charval. If there
      are any values in this array, it will be searched before the
      standard one. This may cause some incompatibilities with previous
      precedence ordering.

   3. Character classes are supported within brackets.

   4. The intermediate format in the "chars" array is changed. Ranges
      are identified by the CCL token, followed by the two endpoints.
      Multi-char ranges (a-d-f-z) are broken up in their components.
      Character classes are identified by the CHAR token, followed by
      the index into the istab array. Both these formats are expanded
      to the normal format when moved to gotofn and gotofn1.

 */
/******************************************************************************* 
*
* Modification History
*
*              001     Dave Grey, October 1991
*                      Increase constant MAXPOS to 8000 (from 4000). Change some variables 
*                      to unsigned char to prevent segment faults on files with 8-bit 
*                      characters in them.
*
*              002     Michael Karp, 04-October-1991
*                      Changed char to unsigned char in key spots to allow egrep 
*                      to find 8-bit characters. Some variables had to be cast 
*                      to unsigned char also.
*
*              003     Tom Woodburn, 30-October-1991
*
*                      Made several fixes to get -i option working:
*
*                      a) Added parameter "int n" to function lower().
*                         lower() converts the input buffer to lowercase;
*                         the new parameter is the number of bytes to
*                         convert.  Before, lower() expected the buffer to
*                         be null-terminated.  It isn't, so lower() would
*                         go past the end of the buffer, converting who
*                         knows what to lowercase.
*
*                      b) Added parameter "char *save" to lower().
*                         lower() saves a copy of the input buffer in
*                         lcasebuf; the new parameter points to the place
*                         in lcasebuf where lower() should copy the buffer.
*                         Before, lower() would always copy the buffer to
*                         the beginning of lcasebuf.  That would overwrite
*                         any partial line left over from the previous
*                         buffer.  Now, those partial lines are appended
*                         to instead of overwritten.
*
*                      c) Fixed casting in execute()'s call to _NLxcolu().
*                         The compiler was warning about it.
*
*                      d) In execute(), made nlp a pointer just into buf.
*                         Before, it would point into either buf or into
*                         lcasebuf, depending on whether the -i option was
*                         set or not.  nlp is compared against p, which
*                         is always a pointer into buf, so that comparison
*                         was working right.
*
*              004     Tom Woodburn, 15-November-1991
*
*                      a) Fixed character ranges.  When yylex() sees a
*                         range like [a-d], it used to store the ASCII
*                         characters between a and d.  Now it stores
*                         a CCL token, a, and d.  That's expanded
*                         later in cgotofn() to all the collation
*                         values between a and d.
*
*                      b) Fixed yylex() so it accepts stuff like
*                         [[:digit:]a-b].  The variable 'last', used to
*                         keep track of the previous token read, wasn't
*                         always being updated.
*
*                      c) Added comments to cgotofn().
*
*                      d) In cgotofn(), fixed the parsing of not
*                         character classes, like [^[:punct:]].  In
*                         the chars array, those are represented by a
*                         CHAR token and an index into a table of 'is'
*                         functions.  A pointer into the chars array
*                         was being incremented only by one (instead
*                         of by two), so the index was being parsed
*                         twice:  once as an index, and then once as
*                         a distinct character.  Fixed this by
*                         incrementing the pointer by two.
*
*                      e) Printed usage message to stderr instead of to
*                         stdout.
*
*********************************************************************************/

%token CHAR 16129
%token DOT 16130
%token CCL 16131
%token NCCL 16132
%token OR  16133
%token CAT 16134
%token STAR 16135
%token PLUS 16136
%token QUEST 16137
%left OR
%left CHAR DOT CCL NCCL '('
%left CAT
%left STAR PLUS QUEST

%{
#include <stdio.h>
#include <locale.h>
#include <NLctype.h>

#define MAXPOS 8000			/* 001 - Was 4000 */
#define MAXLIN 1000
#ifndef  KJI
#define NSTATES 256
#define NCHARS 256
#define NTOP 1000
#else
#define NSTATES 256
#define NCHARS NLCOLMAX+NLCHARMAX
#define NTOP 16128
#endif
#define FINAL -1
#include <nl_types.h>
#include "egrep_msg.h"
nl_catd catd;
#define MSGSTR(n,s)     catgets(catd,MS_EGREP,n,s)
int nchars;
unsigned char gotofn[NSTATES][NTOP];	/* 001 - Was just char */
unsigned char gotofn1[NSTATES][NTOP];	/* 001 - Was just char */
wchar_t symbol[NTOP];
wchar_t colsym[NTOP];
int colis[NSTATES];
int state[NSTATES];
unsigned char out[NSTATES];		/* 001 - Was just char */
int line = 1;
int name[MAXLIN];
int left[MAXLIN];
int right[MAXLIN];
int parent[MAXLIN];
int foll[MAXLIN];
int positions[MAXPOS];
NLchar chars[MAXLIN];
int nxtpos;
int nxtchar = 0;
int tmpstat[MAXLIN];
int initstat[MAXLIN];
int xstate;
int count;
int icount;
unsigned char *input;
#define         BSIZE   4096	/* 005 */
long    lnum;           /* current line number.                        */
int     bflag;          /* print block number of matches               */
int     cflag;          /* print total line count of matches           */
int     fflag;          /* get pattern from a file.                    */
int     lflag;          /* print name of file once then quit searching */
int     nflag;          /* print line number of each match             */
int     hflag = 1;      /* don't print file name on grep of multi files*/
int 	iflag;		/* ignore case				       */
int     sflag;          /* silent mode. Don't print anything but errors*/
int     vflag;          /* everything but.  The NOT case.              */
int     eflag;          /* expression follows                          */
int     nfile;          /* Number of files in the argument list.       */
extern  char *optarg;
extern  int optind;
FILE    *expfile;
int     blkno;          /* current offset block in the file.           */
long    tln;            /* total number of lines that had a match      */
int     nsucc;          /* return code.                                */
char 	lcasebuf[BSIZE*2];	/* buffer with original line */ 
static void lower();	
yyerror(char *s);

#ifdef _NO_PROTO
wchar_t colval();
#else
wchar_t colval(const wchar_t ch);
int _NLxcolu(int desc, char **src, wchar_t **xstr, wchar_t *uval);
#endif

/* As the "is" functions aren't functions, but macros, we cannot put    */
/* the "function" in the array below; thus another layer of indirection */

ALPHA(c) {return(NCisalpha(c));}
UPPER(c) {return(NCisupper(c));}
LOWER(c) {return(NCislower(c));}
DIGIT(c) {return(NCisdigit(c));}
ALNUM(c) {return(NCisalnum(c));}
SPACE(c) {return(NCisspace(c));}
PRINT(c) {return(NCisprint(c));}
PUNCT(c) {return(NCispunct(c));}
XDIGIT(c) {return(NCisxdigit(c));}
CNTRL(c) {return(NCiscntrl(c));}
GRAPH(c) {return(NCisgraph(c));}
#ifdef KJI
JALPHA(c) {return(isjalpha(c));}
JDIGIT(c) {return(isjdigit(c));}
JSPACE(c) {return(isjspace(c));}
JPUNCT(c) {return(isjpunct(c));}
JPAREN(c) {return(isjparen(c));}
JKANJI(c) {return(isjkanji(c));}
JHIRA(c) {return(isjhira(c));}
JKATA(c) {return(isjkata(c));}
JXDIGIT(c) {return(isjxdigit(c));}
#endif
struct isarray {
        char *isstr;
        int (*isfunc)();
} istab[] = {
        { "[:alpha:]", ALPHA },
        { "[:upper:]", UPPER },
        { "[:lower:]", LOWER },
        { "[:digit:]", DIGIT },
        { "[:alnum:]", ALNUM },
        { "[:space:]", SPACE },
        { "[:print:]", PRINT },
        { "[:punct:]", PUNCT },
        { "[:xdigit:]", XDIGIT },
        { "[:cntrl:]", CNTRL },
        { "[:graph:]", GRAPH }


#ifdef KJI
                                ,
        { "[:jalpha:]", JALPHA },
        { "[:jdigit:]", JDIGIT },
        { "[:jspace:]", JSPACE },
        { "[:jpunct:]", JPUNCT },
        { "[:jparen:]", JPAREN },
        { "[:jkanji:]", JKANJI },
        { "[:jhira:]", JHIRA },
        { "[:jkata:]", JKATA },
        { "[:jxdigit:]", JXDIGIT }
#endif

#define NISTAB (sizeof(istab) / sizeof(struct isarray))
};
int     ix;

int     f;
%}

%%
s:      t
                ={ unary(FINAL, $1);
                  line--;
                }
        ;
t:      b r
                ={ $$ = node(CAT, $1, $2); }
        | OR b r OR
                ={ $$ = node(CAT, $2, $3); }
        | OR b r
                ={ $$ = node(CAT, $2, $3); }
        | b r OR
                ={ $$ = node(CAT, $1, $2); }
        ;
b:
                ={ $$ = enter(DOT);
                   $$ = unary(STAR, $$); }
        ;
r:      CHAR
                ={ $$ = enter($1); }
        | DOT
                ={ $$ = enter(DOT); }
        | CCL
                ={ $$ = cclenter(CCL); }
        | NCCL
                ={ $$ = cclenter(NCCL); }
        ;

r:      r OR r
                ={ $$ = node(OR, $1, $3); }
        | r r %prec CAT
                ={ $$ = node(CAT, $1, $2); }
        | r STAR
                ={ $$ = unary(STAR, $1); }
        | r PLUS
                ={ $$ = unary(PLUS, $1); }
        | r QUEST
                ={ $$ = unary(QUEST, $1); }
        | '(' r ')'
                ={ $$ = $2; }
        | error
        ;

%%
yyerror(s) char *s; {
        fprintf(stderr, "egrep: %s\n", s);
        exit(2);
}

yylex() {
        extern long yylval;
        int cclcnt, x;
        int last = 0;
        register int c, d;
        switch(c = nextch()) {
            case '$':
            case '^': c = '\n';
                goto defchar;
            case '|': return (OR);
            case '*': return (STAR);
            case '+': return (PLUS);
            case '?': return (QUEST);
            case '(': return (c);
            case ')': return (c);
            case '.': return (DOT);
            case '\0': return (0);
            case '\n': return (OR);
            case '[':
               x = CCL;
               cclcnt = 0;
               count = nxtchar++;
               if ((c = nextsym()) == '^') {
                       x = NCCL;
                       c = nextsym();
               }
               do {
                 if (c == '\0') synerror();
                 if (c == '-' && cclcnt > 0 && chars[nxtchar-1] != 0) {
                      if ((d = nextsym()) != 0) {
                          if ((last == CHAR) || (d == CHAR))
                               synerror();

			  /* 004a
			   *
			   * Save the range c-d in the chars array as
			   * CCL, c, and d.  c is already there.  If
			   * it wasn't put there as the endpoint of a
			   * previous range (as in, a-c-d), overwrite
			   * it.
			   */

			  c = _NCunmap(chars[nxtchar-1]);

			  if (last != CCL) {
			      nxtchar--;
			      cclcnt--;
			  }

			  if (nxtchar + 2 >= MAXLIN) overflo();
			  chars[nxtchar++] = CCL;
			  cclcnt++;
			  chars[nxtchar++] = _NCmap(c);
			  cclcnt++;
			  chars[nxtchar++] = _NCmap(d);
			  cclcnt++;

                           last = CCL;
                           continue;
                       }
                  }
                  if (c == CHAR) {
                      if (last == CCL) synerror();
                      chars[nxtchar++] = CHAR;
                      cclcnt++;
                      chars[nxtchar++] = ix;
                      cclcnt++;
                      last = CHAR;
                      continue;
                  }

                  if (nxtchar >= MAXLIN) overflo();
                  chars[nxtchar++] = _NCmap(c);
                  cclcnt++;
                  last = 0; /* 004b */
               } while ((c = nextsym()) != ']');
               chars[count] = cclcnt;
               return (x);
            case '\\':
                if ((c = nextch()) == '\0') synerror();
            defchar:
            default:
                yylval = _NCmap(c);
                last = 0;
                return (CHAR);
        }
}

#define IFBUFLEN 16

int
nextsym()
{
        register int c, d;
        static char ifbuf[IFBUFLEN];
        static char *ib = ifbuf;
        static char *eb = &ifbuf[IFBUFLEN-2];
        if      (*ib != '\0')
                        return(c = *ib++);
        else    if ((c = nextch()) != '[')
                        return (c);

        switch (c = nextch()) {
        case ':':
                ib = ifbuf;
                *ib++ = '[';
                do {
                   if ((c == '\0') || (c == '\n'))
                                synerror();
                   *ib++ = c;
                } while (((c = nextch()) != ']') &&
                        (c != '-') && (c != '[') &&
                        (ib < eb));
                *ib++ = c;
                *ib = '\0';
                for (ix = 0; ix < NISTAB; ix++)
                        if((strcmp(ifbuf,istab[ix].isstr))==0)
                                return(CHAR);
                if (c == ']') {
                        unnextch(c);
                        *--ib = '\0';
                }
                ib = ifbuf;
                return(c = *ib++);
        default:
                unnextch(c);
                return('[');
        }

}
int
nextch() {
        register int c;
        register int c1;
        if (fflag) {
#ifdef KJI
                if ((c = getwc(expfile)) == EOF) return 0;
#else
                if ((c = getc(expfile)) == EOF) return(0);
#endif
#ifndef KJI
                if (NCisshift (c)) {
                        if ((c1 = getc(expfile)) == EOF) return(0);
                        if (_NCdec2 (c, c1, c) == 1) ungetc (c1, expfile);
                }
#endif
        }
        else {
                c = *input++;
                if (NCisshift (c)) {
                        c1 = *input++;
                        if (_NCdec2 (c, c1, c) == 1) input--;
                }
        }
	if (iflag)	/* ignore case */
		c = NCtolower(c);
        return(c);
}
unnextch(c) int c; {
        if (fflag)
                ungetwc(c, expfile);
        else {
                if (c > 0xff)
                        input -= 2;
                else
                        input--;
        }
}

wchar_t
#ifdef _NO_PROTO
colval(ch)
wchar_t ch;
#else
colval(const wchar_t ch)
#endif
{
        char ifbuf[16];
        char *ib;
        int cv;
        wchar_t uv;

        ib = ifbuf;
        if (ch > 256)
                *ib++ = (ch >> 8);
        *ib++ = (ch & 0xff);
        *ib = '\0';
        uv = NCcoluniq(ch);
        cv = (((cv = NCcollate(ch)) < 0) &&
                                (_NLxcolu(cv, &ib, (wchar_t **)0, &uv)));
        return (uv);
}


synerror() {
        fprintf(stderr, MSGSTR(SYNERR,"egrep: syntax error\n"));        /*MSG*/
        exit(2);
}

enter(x) int x; {
        if(line >= MAXLIN) overflo();
        name[line] = x;
        left[line] = 0;
        right[line] = 0;
        return(line++);
}

cclenter(x) int x; {
        register int linno;
        linno = enter(x);
        right[linno] = count;
        return (linno);
}

node(x, l, r) {
        if(line >= MAXLIN) overflo();
        name[line] = x;
        left[line] = l;
        right[line] = r;
        parent[l] = line;
        parent[r] = line;
        return(line++);
}

unary(x, d) {
        if(line >= MAXLIN) overflo();
        name[line] = x;
        left[line] = d;
        right[line] = 0;
        parent[d] = line;
        return(line++);
}
/*
 * NAME: ovrflow
 *
 * FUNCTION: print error and exit
 *
 * RETURN VALUE:        none (exit)
 */

overflo() {
        fprintf(stderr,
        MSGSTR(NOSPACE,"egrep: regular expression requires too much space\n")); /*MSG*/
        exit(2);
}

cfoll(v) {
        register int i;
        if (left[v] == 0) {
                count = 0;
                for (i=1; i<=line; i++) tmpstat[i] = 0;
                follow(v);
                add(foll, v);
        }
        else if (right[v] == 0) cfoll(left[v]);
        else {
                cfoll(left[v]);
                cfoll(right[v]);
        }
}

/*
 * Compute the two transition functions, gotofn and gotofn1.
 *
 * Both are just tables.  Rows represent states; columns
 * represent character values in gotofn and collation values in
 * gotofn1.  Like this,
 *
 *	next_state = gotofn [current_state][character_value]
 *	next_state = gotofn1[current_state][collation_value]
 */

cgotofn()
{
        register int c, i, k;
        int n, s;
        int l, m, o;
        int j, nc, pc, pos;
        int curpos, num;
        int number, newpos;

	/*
	 * Compute the initial state.
	 *
	 * The initial state is the set of all positions in the re's
	 * syntax tree that could match the first symbol of a string
	 * matching the re.
	 */

        count = 0;
        for (n=3; n<=line; n++) tmpstat[n] = 0;
        if (cstate(line-1)==0) {
                tmpstat[line] = 1;
                count++;
                out[0] = 1;
        }
        for (n=3; n<=line; n++) initstat[n] = tmpstat[n];
        count--;                /*leave out position 1 */
        icount = count;
        tmpstat[1] = 0;
        add(state, 0);

	/*
	 * Build the transition tables row by row (state by state),
	 * starting with the initial state.  This involves 2 steps:
	 *
	 *	   1) Find the columns that need to be filled in.
	 *	   2) Fill in those columns with the next state.
	 */

        n = 0;
        for (s=0; s<=n; s++)  {
                if (out[s] == 1) continue;

		/*
		 * 1) Find the columns that need to be filled in.
		 *
		 * Those columns correspond to the symbols that would
		 * make us transition from this state to another one.
		 * The symbols are just the symbols at each position
		 * in the current state.
		 *
		 * symbol[] keeps track of the columns for gotofn,
		 * colsym[] keeps track of them for gotofn1.
		 */

                for (i=0; i<nchars; i++) {
                                symbol[i] = 0;
                                colsym[i] = 0;
                                }
                num = positions[state[s]];
                count = icount;
                for (i=3; i<=line; i++) tmpstat[i] = initstat[i];
                pos = state[s] + 1;
                for (i=0; i<num; i++) {
                        curpos = positions[pos];
                        if ((c = name[curpos]) >= 0) {
                                switch(c)  {
                                case DOT:
                                        for (k=0; k<nchars; k++) {
                                                if (k!=colval('\n'))
                                                        symbol[k] = 1;
                                        };
                                        break;
                                case CCL:
                                        nc = chars[right[curpos]];
                                        pc = right[curpos] + 1;
                                        for (k = 0; k < nc; k++) {
                                            switch(chars[pc]) {
                                            case CCL:
                                                 l = colval(_NCunmap(chars[pc+1]));
                                                 m = colval(_NCunmap(chars[pc+2]));
                                                 while (l <=  m)
                                                        colsym[l++] = 1;
                                                 k += 2;
                                                 pc += 3;
                                                 break;
                                            case CHAR:
                                                 pc++;
                                                 for(j=0; j<NCHARS; j++) {
                                                 l = _NCunmap(j);
                                                 if ((NCisNLchar(l) &&
                                                 (*istab[chars[pc]].isfunc)(l)))
                                                   symbol[j] = 1;
                                                 }
                                                 pc++;
                                                 k++;
                                                 break;
                                            default:
                                                 symbol[chars[pc++]] = 1;
                                                 break;
                                            }
                                        }
                                        break;
                                case NCCL:
                                        nc = chars[right[curpos]];
                                        for (j = 0; j < NCHARS; j++) {
                                                if (!NCisNLchar(j))
                                                        continue;
                                                else o = colval(j);
                                            pc = right[curpos] + 1;
                                            for (k = 0; k < nc; k++) {
                                              switch(chars[pc]) {
                                              case CCL:
                                                   k += 2;
                                                   l = colval(_NCunmap(chars[pc+1]));
                                                   m = colval(_NCunmap(chars[pc+2]));
                                                   if ((l <= o) && (o <= m)) {
                                                        pc +=3;
                                                        goto cont;
                                                   } else pc +=3;
                                                   break;
                                              case CHAR:
						   k++;  /* 004d */
                                                   pc++;
                                                   if ((*istab[chars[pc++]].isfunc)(_NCunmap(j))) /* 004d */
                                                        goto cont;
                                                   break;
                                              default:
                                                  if (j==chars[pc++])
                                                        goto cont;
                                                  break;
                                              }
                                            }
                                                if (o != colval('\n')) {
                                                        symbol[j] = 1;
                                                        colsym[o] = 1;
                                                }
                                                cont:;
                                        }
                                        break;
                                break;
                                default:
                                        if (c < nchars) symbol[c] = 1;
                /*                      else printf(MSGSTR(FUNNY,"something's funny\n"));       /*MSG*/
                                }
                        }
                        pos++;
                }

		/*
		 * 2) Fill in those columns with the next state.
		 *
		 * For each column that needs to be filled in, find
		 * the positions that the column's symbol could make
		 * us transition to.  That set of positions forms the
		 * next state.  Fill in the column with that state.
		 */

                for (c=0; c<nchars; c++) {
                        if ((symbol[c] == 1) || (colsym[c] == 1))
                                {
                                if (colsym[c] == 1) colis[s] = 1;

				/*
				 * For each position in the current
				 * state, find any positions this
				 * symbol could make us transition to.
				 * Mark those positions.
				 */

                                count = icount;
                                for (i=3;i <= line; i++) tmpstat[i]=initstat[i];
                                pos = state[s] + 1;
                                for (i=0; i<num; i++) {
                                    curpos = positions[pos];
                                    if ((k = name[curpos]) >= 0)
                                        if (
                                               (k == c)
                                            || (k == DOT)
                                            || (k == CCL &&
						member(c, colsym[c], right[curpos], 1))  /* 006 */
                                            || (k == NCCL &&
						member(c, colsym[c], right[curpos], 0))  /* 006 */
                                           ) {
                                                number=positions[foll[curpos]];
                                                newpos = foll[curpos] + 1;
                                                for (k=0; k<number; k++) {
                                                    if (tmpstat[positions[newpos]] != 1) {
                                                        tmpstat[positions[newpos]] = 1;
                                                        count++;
                                                    }
                                                newpos++;
                                                }
                                             }
                                        pos++;
                                } /* end nextstate */

				/*
				 * The marked positions form the next
				 * state.  Save that state if we
				 * haven't yet.  Then use it to fill
				 * in the column of the appropriate
				 * transition table.
				 */

                                if (notin(n)) {
                                        if (n >= NSTATES) overflo();
                                        add(state, ++n);
                                        if (tmpstat[line] == 1) out[n] = 1;
                                        if (symbol[c] == 1)
                                                gotofn[s][c] = n;
                                        if (colsym[c] == 1)
                                                gotofn1[s][c] = n;
                                }
                                else {
                                        if (symbol[c] == 1)
                                                gotofn[s][c] = xstate;
                                        if (colsym[c] == 1)
                                                gotofn1[s][c] = xstate;
                                }
                        }
                }
        }
}

cstate(v) {
        register int b;
        if (left[v] == 0) {
                if (tmpstat[v] != 1) {
                        tmpstat[v] = 1;
                        count++;
                }
                return(1);
        }
        else if (right[v] == 0) {
                if (cstate(left[v]) == 0) return (0);
                else if (name[v] == PLUS) return (1);
                else return (0);
        }
        else if (name[v] == CAT) {
                if (cstate(left[v]) == 0 && cstate(right[v]) == 0) return (0);
                else return (1);
        }
        else { /* name[v] == OR */
                b = cstate(right[v]);
                if (cstate(left[v]) == 0 || b == 0) return (0);
                else return (1);
        }
}


member(symb, iscolval, set, torf) {  /* 006 */
        register int i, j, num, pos, c;  /* 006 */
        num = chars[set];
        pos = set + 1;
        for (i=0; i<num; i++)
                switch(chars[pos]) {
                case CCL:               /* check if symb value is a colval */  /* 006 */
		             /* 008 - start of change */
                             if (!iscolval) {
			       if ((colval(chars[pos+1]) <= colval(symb)) &&
				   (colval(chars[pos+2]) >= colval(symb)))
				 return(torf);
			     }
			     else {
			       if ((colval(chars[pos+1]) <= symb) &&
				   (colval(chars[pos+2]) >= symb))
				 return(torf);
			     }
		             /* 008 - end of change */
                             i += 2;
                             pos += 3;
                             break;
                case CHAR:              /* symb is _NCmap(char) */
                             i++;
                             pos++;
                             if (!iscolval &&   /* 006 */
                                (*istab[chars[pos++]].isfunc)(_NCunmap(symb))) /* 006 */
                                                return(torf);
                             break;
                default:                /* 006  start */
                             c = chars[pos++];
                             if (iscolval) c = colval(c);
                             if (symb == c) return (torf);
                                        /* 006  end */
                             break;
                }
        return (!torf);
}


notin(n) {
        register int i, j, pos;
        for (i=0; i<=n; i++) {
                if (positions[state[i]] == count) {
                        pos = state[i] + 1;
                        for (j=0; j < count; j++)
                                if (tmpstat[positions[pos++]] != 1) goto nxt;
                        xstate = i;
                        return (0);
                }
                nxt: ;
        }
        return (1);
}

add(array, n) int *array; {
        register int i;
        if (nxtpos + count > MAXPOS) overflo();
        array[n] = nxtpos;
        positions[nxtpos++] = count;
        for (i=3; i <= line; i++) {
                if (tmpstat[i] == 1) {
                        positions[nxtpos++] = i;
                }
        }
}

/*
 * NAME: follow
 *
 * FUNCTION: check for a match
 *
 * RETURN VALUE: none
 */

follow(v) int v; {
        int p;
        if (v == line) return;
        p = parent[v];
        switch(name[p]) {
                case STAR:
                case PLUS:      cstate(v);
                                follow(p);
                                return;

                case OR:
                case QUEST:     follow(p);
                                return;

                case CAT:       if (v == left[p]) {
                                        if (cstate(right[p]) == 0) {
                                                follow(p);
                                                return;
                                        }
                                }
                                else follow(p);
                                return;
                case FINAL:     if (tmpstat[line] != 1) {
                                        tmpstat[line] = 1;
                                        count++;
                                }
                                return;
        }
}


main(argc, argv)
char **argv;
{
        register int c;
        int errflg = 0;

        extern nl_catd catd;
        catd = catopen(MF_EGREP,0);
        (void) setlocale(LC_ALL,"");

                while(( c = getopt(argc, argv, "bshice:f:lnv")) != EOF)
                        switch(c) {

                        case 'b':               /* display block numbers */
                                bflag++;
                                continue;

                        case 'c':               /* display count of matches */
                                cflag++;
                                continue;

                        case 'e':               /* pattern */
                                eflag++;
                                input = (unsigned char*)optarg;
                                continue;

                        case 'f':               /* get pattern from file */
                                fflag++;
                                expfile = fopen(optarg,"r");
                                if (expfile == NULL) {
                                     fprintf(stderr,
                                     MSGSTR(CANTOPEN,"egrep: can't open %s\n"),optarg); /*MSG*/
                                     exit(2);
                                }
                                continue;

                        case 'h':               /* don't print file names */
                                hflag = 0;
                                continue;

			case 'i':		/* ignore case */
				iflag++;
				continue;

                        case 'l':               /* list file once on match */
                                lflag++;
                                continue;

                        case 'n':               /* print line numbers */
                                nflag++;
                                continue;

                        case 's':               /* silent mode */
                                sflag++;
                                continue;

                        case 'v':               /* the NOT case */
                                vflag++;
                                continue;

                        case '?':
                                errflg++;
                }

                argc -= optind;
                if (errflg || ((argc <= 0) && !(fflag || eflag))) {
                        fprintf(stderr, MSGSTR(USGE,"usage: egrep [ -bchilnsv ] [ -e exp ] [ -f file ] [ strings ] [ file ] ...\n")); /*MSG*/ /* 004e */
                        exit(2);
                }
                if ( !eflag  && !fflag ) {
                        input = (unsigned char*)argv[optind];
                        optind++;
                        argc--;
        }
                nchars = NTOP;


        yyparse();

        cfoll(line-1);
        cgotofn();
        nfile = argc;
        argv = &argv[optind];
        if (argc<=0) {
                if (lflag) exit(1);
                execute((char *)NULL);
        }
        else
                while ( --argc >= 0 ) {
                        execute(*argv);
                        argv++;
                }
        exit((nsucc == 2) ? 2 : (nsucc == 0));
}

/*
 * NAME: execute
 *
 * FUNCTION: Look for a match line by line for the argument file.
 *
 * RETURN VALUE: none
 */

execute(file)
char *file;
{
        register unsigned char *p;                           /* 002 - was char */
	unsigned char *p1;                                 /* 002 - new variable to solve dereferncing problem */
        register int cstat;
        register int cstat1;
        int cvalue;
        wchar_t uvalue;
        unsigned char *q;                                    /* 002 - was char */
        register int ccount;
        unsigned char buf[BSIZE*2];                          /* 002 - was char */
        unsigned char *nlp;                                  /* 002 - was char */
	unsigned char *outbuf;                              /* 003d */
        int istat;
        int c;
        if (file) {
                if ((f = open(file, 0)) < 0) {
                      fprintf(stderr, MSGSTR(CANTOPEN,"egrep: can't open %s\n"), file); /*MSG*/
                        nsucc = 2;
                        return;
                }
        }
        else f = 0;
        ccount = 0;
        lnum = 1;
        tln = 0;
        blkno = 0;
        p = buf;
	nlp = buf;                                          /* 003d */
        if ((ccount = read_nl (f,p,BSIZE))<=0) goto done;   /* 009 */
	if (iflag) 
		lower(p, ccount, lcasebuf + (p - buf));     /* 003a,b */
        istat = cstat = gotofn[0][_NCmap('\n')];
        if (out[cstat]) goto found;
        for (;;) {
                if ((ccount == 1) && NCisshift(p[0])) {
                        if ((unsigned long) p < (unsigned long) &buf[BSIZE*2])      /* 002 - recast to matching datatype */
                                p++;
                        else  {
                                buf[0] = *p;
                                p = &buf[1];
                        }
                        if ((ccount = read_nl (f, p, 1)) == 1) { /* 009 */
                                ccount = 2;
                                p--;
                        } else  {
                                p--;
                                ccount = 1;
                        }
                }
                if (_NCdec2 (*p, p[1], c) == 2) {
                   p++;
                   --ccount;
                }
                if (colis[cstat]) {
                        q = p;
                        q++;
                        uvalue = NCcoluniq(c);
                        cvalue = (((cvalue = NCcollate(c)) < 0) &&
                                        (_NLxcolu(cvalue, (char **) &q, (wchar_t **)0, &uvalue)));
                                                                   /* 002, 003c - cast to matching datatype */
			/*
			 * cstat -> current state
			 * gotofn1 -> transition function for character class
			 * gotofn -> transition function 
			 * 
			 * If it's not obvious, this is using finite
			 * state machine algorithm. Regretable it's
			 * nondeterministic, so we check for other 
			 * transitions and always transition to the
			 * highest state.  This forces it to be
			 * deterministic (given input, only one
			 * transition).  PTM 40994
			 */

			if ( ( (cstat1 = gotofn[cstat][uvalue] ) || 
				( gotofn1[cstat][_NCmap(c)]) >=
				(gotofn[cstat][_NCmap(c)]) ) ) {
	                        if (cstat1 = gotofn1[cstat][uvalue]) {
       	                        	cstat = cstat1;
					ccount -= (q - p - 1);  /* 007 */
                                	p = --q;
                        	} else  cstat = gotofn[cstat][_NCmap(c)];
			} else cstat = gotofn[cstat][_NCmap(c)];
                } else cstat = gotofn[cstat][_NCmap(c)];
                if (out[cstat]) {
                found:  for(;;) {
                                if (*p++ == '\n') {
                                        if (vflag == 0) {
                                succeed: nsucc = (nsucc == 2) ? 2 : 1;
                                        if (cflag) tln++;
                                        else if (sflag)
                                                ;       /* silent mode */
                                        else if (lflag) {
                                                printf("%s\n", file);
                                                close(f);
                                                return;
                                        }
                                        else {
                                                if (nfile>1 && hflag) printf("%s:",file);
                                                if (bflag) printf("%d:", blkno);
                                                if (nflag) printf("%ld:", lnum);

	/* 003d
	 *
	 * Want to print the current line.  nlp points to where that line
	 * starts in buf, p points to the start of the next line.
	 *
	 * buf is a double buffer, so the current line may be split between
	 * the two halves of buf.  That is, the line may start near the end
	 * of the second half, and end near the start of the first half.
	 * If that's the case, p <= nlp.
	 *
	 * To print a split line, first need to print the beginning of the
	 * line from the second half of buf, and then print the end of the
	 * line from the first half.
	 *
	 * If the line isn't split, can just print it all at once.
	 */

						outbuf = iflag ?
							(unsigned char *) lcasebuf : buf;
                                                if (p <= nlp) {
							nlp = outbuf + (nlp - buf);
                                                        while (nlp < &outbuf[BSIZE*2])
                                                                putchar(*nlp++);
                                                        nlp = buf;
                                                }
						nlp = outbuf + (nlp - buf);
                                                while (*nlp != '\n') 
						    putchar(*nlp++);
						putchar('\n');
                                        }
                                }
                                        lnum++;
                                        nlp = p; /* 003d */
                                        if((out[(cstat=istat)]) == 0) goto brk2;
                                }
                                cfound:
                                if (--ccount <= 0) {
                                        if ((unsigned long) p <= (unsigned long) &buf[BSIZE]) {
					                              /* 002 - recast to matching datatype */
                                                if ((ccount = read_nl (f, p, BSIZE)) <= 0) goto done;  /* 009 */
                                        }
                                        else if ((unsigned long) p == (unsigned long) &buf[BSIZE*2]) {
                                                                      /* 002 - recast to matching datatype */
                                                p = buf;
                                                if ((ccount = read_nl (f, p, BSIZE)) <= 0) goto done;  /* 009 */
                                        }
                                        else {
                                                if ((ccount = read_nl (f, p, &buf[BSIZE*2]-p)) <= 0) goto done;  /* 009 */
                                        }
					if (iflag) 
						lower(p, ccount, lcasebuf + (p - buf));
                                                                     /* 003a,b */
                                        blkno++;
                                }
                        }
                }
		p1 = p++;                                             /* 002 - rework statement to prevent */
                if (*p1 == '\n') {                                   
                        if (vflag) goto succeed;
                        else {
                                lnum++;
                                nlp = p; /* 003d */
                                if (out[(cstat=istat)]) goto cfound;
                        }
                }
                brk2:
                if (--ccount <= 0) {
                        if ((unsigned long) p <= (unsigned long) &buf[BSIZE]) {        /* 002 - recast to matching datatype */
                                if ((ccount = read_nl (f, p, BSIZE)) <= 0) break;  /* 009 */
                        }
                        else if ((unsigned long) p == (unsigned long) &buf[BSIZE*2]) { /* 002 - recast to matching datatype */
                                p = buf;
                                if ((ccount = read_nl (f, p, BSIZE)) <= 0) break;  /* 009 */
                        }
                        else {
                                if ((ccount = read_nl (f, p, &buf[BSIZE*2] - p)) <= 0) break;  /* 009 */
                        }
			if (iflag)
				lower(p, ccount, lcasebuf + (p - buf));
                                                                     /* 003a,b */
                        blkno++;
                }
        }
done:   close(f);
        if (cflag) {
                if (nfile > 1)
                        printf("%s:", file);
                printf("%ld\n", tln);
        }
}

/*
 * Convert n bytes of buffer p to lower case.
 * Save a copy of the original in lcasebuf.
 */
static void
lower(char *p1, int n, char *save)		/* 003a,b */
{
	char *p1_end;				/* 003a */
	int cnt;
	char twochr[2];
	NLchar tmp;

	twochr[0] = twochr[1] = 0;

	p1_end = p1 + n;			/* 003a */
	while (p1 != p1_end)			/* 003a */
	{
		*save++ = twochr[0] = *p1;
		if (NCisshift((int)twochr[0]))
			*save++ = twochr[1] = *(p1+1);
		NCdecode (twochr,&tmp);
		tmp = NCtolower((int) tmp);
		cnt = NCencode (&tmp, twochr);
		*p1++ = twochr[0];
		if (cnt == 2)
			*p1++ = twochr[1];
	}
	*save = '\0';
}

/* 009 - start */
/* The "read_nl" routine is similar to "read", but ensures that the
 * last line of the input file will be terminated by a new-line char.
 */
static int
read_nl (int in_file, char *in_buffer, size_t in_max)
{
	int  count;			/* number of characters read	*/
	static char last_char = '\n';	/* last character of last read 	*/
	count = read (in_file, in_buffer, in_max);
	if (((count <=  0) && (last_char != '\n')) ||
	    ((count > 0) && (count < in_max) && (in_buffer [count-1] != '\n')))
	{				/* last read from file		*/
		in_buffer [count++] = '\n';  
		last_char = '\n';
	}				/* last read from file		*/
	else	last_char = in_buffer [count-1];
	return (count);
}
/* 009 - end */
