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
static char	*sccsid = "@(#)$RCSfile: sub1.c,v $ $Revision: 4.2.3.3 $ (DEC) $Date: 1992/10/31 09:40:25 $";
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
 * COMPONENT_NAME: (CMDLANG) Language Utilities
 *
 * FUNCTIONS: NLgch, allprint, is_alpha, cclinter, cpyact, ctrans,
		digit, dupl, error, gch, getl, index, lgate,
		lookup, mn0, mn1, mn2, munput, printable, scomp,
		scopy, sect1dump, sect2dump, siconv, slength, space,
		strpt, treedump, usescape
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 * OBJECT CODE ONLY SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * sub1.c	1.6  com/cmd/lang/lex,3.1,9021 12/14/89 17:03:29"; 
 */

# include "ldefs.h"

#ifndef _BLD
#include "lex_msg.h"
#define MSGSTR(Num,Str) catgets(catd,MS_LEX,Num,Str)
nl_catd catd;
#else
#define MSGSTR(Num,Str) Str
#endif

#ifdef KJI
#include <NLchar.h>
#endif

unsigned char *
getl(p)	/* return next line of input, throw away trailing '\n' */
	/* returns 0 if eof is had immediately */
  char *p;
	{
	register int c;
	register char *s, *t;
	t = s = p;
	while(((c = gch()) != 0) && c != '\n') {
		*t++ = c;
		if ((unsigned char *)t >= buf+sizeof(buf))
			error(MSGSTR(LINELONG, "Input line is too long."));
	}
	*t = 0;
	if(c == 0 && s == t) return((unsigned char *)0);
	prev = '\n';
	pres = '\n';
	return ( (unsigned char *)s );
	}

space(ch)
	{
	switch(ch)
		{
		case ' ':
		case '\t':
		case '\n':
			return(1);
		}
	return(0);
	}

digit(c)
{
	return(c>='0' && c <= '9');
}
error(s,p,d)
char *s;
long p, d;
	{
	fprintf(errorf,"lex: ");
	if(!eof && (fin != NULL))
		fprintf(errorf,MSGSTR(ERRLINE, "(Error at line %d) "),yyline);
	else
		fprintf(errorf,MSGSTR(ERR, "(Error) "));
	fprintf(errorf,s,p,d);
	putc('\n',errorf);
# ifdef DEBUG
	if(debug && sect != ENDSECTION) {
		sect1dump();
		sect2dump();
	}
# endif
	if(
# ifdef DEBUG
		debug ||
# endif
		report == 1) statistics();
	exit(1);	/* error return code */
	}

warning(s,p,d)
char *s;
long p, d;
	{
	fprintf(errorf,"lex: ");
	if(!eof && (fin != NULL))
		fprintf(errorf,MSGSTR(WARNLINE, "(Warning at line %d)"),yyline);
	else
		fprintf(errorf,MSGSTR(WARN, "(Warning) "));
	fprintf(errorf,s,p,d);
	putc('\n',errorf);
	fflush(errorf);
	if (fout) fflush(fout);
	fflush(stdout);
	}
index(a,s)
	char *s;
{
	register int k;
	for(k=0; s[k]; k++)
		if (s[k]== a)
			return(k);
	return(-1);
	}

is_alpha(c)
  int c; {
return('a' <= c && c <= 'z' || 'A' <= c && c <= 'Z');
}
printable(c)
{
return( c>033 && c < 0377);
}
lgate()
{
	char fname[20];
	if (lgatflg) return;
	lgatflg=1;
	if(fout == NULL){
		sprintf(fname, "lex.yy.%c", ratfor ? 'r' : 'c' );
		fout = fopen(fname, "w");
		}
	if(fout == NULL) error(MSGSTR(NOPEN, "Can't open %s"),fname);
	if(ratfor) fprintf( fout, "#\n");
	phead1();
	}
/* scopy(ptr to str, ptr to str) - copy first arg str to second */
/* returns ptr to second arg */
scopy(s,t)
  char *s, *t; {
	register char *i;
	i = t;
	while(*i++ = *s++);
	return;
	}
siconv(t)	/* convert string t, return integer value */
  char *t; {
	register int i,sw;
	register char *s;
	s = t;
	while(!(('0' <= *s && *s <= '9') || *s == '-') && *s) s++;
	sw = 0;
	if(*s == '-'){	/* neg */
		sw = 1;
		s++;
		}
	i = 0;
	while('0' <= *s && *s <= '9')
		i = i * 10 + (*(s++)-'0');
	return(sw ? -i : i);
	}
/* slength(ptr to str) - return integer length of string arg */
/* excludes '\0' terminator */
slength(s)
  char *s; {
	register int n;
	register char *t;
	t = s;
	for (n = 0; *t++; n++);
	return(n);
	}
/* scomp(x,y) - return -1 if x < y,
		0 if x == y,
		return 1 if x > y, all lexicographically */
scomp(x,y)
  unsigned char *x,*y; {
	register unsigned char *a,*d;
	a = x;
	d = y;
	while(*a || *d){
		if(*a > *d)
			return(1);	/* greater */
		if(*a < *d)
			return(-1);	/* less */
		a++;
		d++;
		}
	return(0);	/* equal */
	}
ctrans(ss)
	unsigned char **ss;
{
	register int c, k;
	if ((c = **ss) != '\\')
		return(c);
	switch(c= *++*ss)
	{
	case 'n': c = '\n'; break;
	case 't': c = '\t'; break;
	case 'r': c = '\r'; break;
	case 'b': c = '\b'; break;
	case 'f': c = 014; break;		/* form feed for ascii */
	case '\\': c = '\\'; break;
	case '0': case '1': case '2': case '3':
	case '4': case '5': case '6': case '7':
		c -= '0';
		while ((k = *(*ss+1)) >= '0' && k <= '7')
			{
			c = c*8 + k - '0';
			(*ss)++;
			}
		break;
	}
	return(c);
}
cclinter(sw)
  int sw; {
		/* sw = 1 ==> ccl */
	register int i, j, k;
	int m;
	if(!sw){		/* is NCCL */
		for(i=1;i<NCH;i++)
			symbol[i] ^= 1;			/* reverse value */
		}
	for(i=1;i<NCH;i++)
		if(symbol[i]) break;
	if(i >= NCH) return;
	i = cindex[i];
	/* see if ccl is already in our table */
	j = 0;
	if(i){
		for(j=1;j<NCH;j++){
			if((symbol[j] && cindex[j] != i) ||
			   (!symbol[j] && cindex[j] == i)) break;
			}
		}
	if(j >= NCH) return;		/* already in */
	m = 0;
	k = 0;
	for(i=1;i<NCH;i++)
		if(symbol[i]){
			if(!cindex[i]){
				cindex[i] = ccount;
				symbol[i] = 0;
				m = 1;
				}
			else k = 1;
			}
			/* m == 1 implies last value of ccount has been used */
	if(m)ccount++;
	if(k == 0) return;	/* is now in as ccount wholly */
	/* intersection must be computed */
	for(i=1;i<NCH;i++){
		if(symbol[i]){
			m = 0;
			j = cindex[i];	/* will be non-zero */
			for(k=1;k<NCH;k++){
				if(cindex[k] == j){
					if(symbol[k]) symbol[k] = 0;
					else {
						cindex[k] = ccount;
						m = 1;
						}
					}
				}
			if(m)ccount++;
			}
		}
	return;
	}
usescape(c)
  int c; {
	register unsigned char d;
	switch(c){
	case 'n': c = '\n'; break;
	case 'r': c = '\r'; break;
	case 't': c = '\t'; break;
	case 'b': c = '\b'; break;
	case 'f': c = 014; break;		/* form feed for ascii */
	case '0': case '1': case '2': case '3':
	case '4': case '5': case '6': case '7':
		c -= '0';
		while('0' <= (d=gch()) && d <= '7'){
			c = c * 8 + (d-'0');
			if(!('0' <= peek && peek <= '7')) break;
			}
		break;
	}
	return(c);
	}
lookup(s,t)
  char *s;
  char **t; {
	register int i;
	i = 0;
	while(*t){
		if(scomp(s,*t) == 0)
			return(i);
		i++;
		t++;
		}
	return(-1);
	}
cpyact(){ /* copy C action to the next ; or closing } */
	register int brac, c, mth;
	int savline, sw;

	brac = 0;
	sw = TRUE;

while(!eof){
	c = gch();
swt:
	switch( c ){

case '|':	if(brac == 0 && sw == TRUE){
			if(peek == '|')gch();	/* eat up an extra '|' */
			return(0);
			}
		break;

case ';':
		if( brac == 0 ){
			putc(c,fout);
			putc('\n',fout);
			return(1);
			}
		break;

case '{':
		brac++;
		savline=yyline;
		break;

case '}':
		brac--;
		if( brac == 0 ){
			putc(c,fout);
			putc('\n',fout);
			return(1);
			}
		break;

case '/':	/* look for comments */
		putc(c,fout);
		c = gch();
		if( c != '*' ) goto swt;

		/* it really is a comment */

		putc(c,fout);
		savline=yyline;
		while( c=gch() ){
			if( c=='*' ){
				putc(c,fout);
				if( (c=gch()) == '/' ) goto loop;
				}
			putc(c,fout);
			}
		yyline=savline;
		error(MSGSTR(EOFCOM, "EOF inside comment" ));

case '\'':	/* character constant */
		mth = '\'';
		goto string;

case '"':	/* character string */
		mth = '"';

	string:  /*@@*/

		putc(c,fout);

		while( c=gch() ){
			if( c=='\\' ){
				putc(c,fout);
				c=gch();
				}
			else if( c==mth ) goto loop;
			putc(c,fout);
			if (c == '\n')
				{
				yyline--;
				error(MSGSTR(ENDLESSTR,
				"Non-terminated string or character constant"));
				}
			}

		error(MSGSTR(EOFSTR, "EOF in string or character constant" ));

case '\0':
		yyline = savline;
		error(MSGSTR(NONTERM, "Action does not terminate"));
default:
		break;		/* usual character */
		}
loop:
	if(c != ' ' && c != '\t' && c != '\n') sw = FALSE;
	putc(c,fout);
	}
error(MSGSTR(PEOF, "Premature EOF"));
}


#ifndef KJI
char shft[4] = {28,29,30,31};
#endif

NLgch(){
	register int c;
	prev = pres;
	c = pres = peek;
	peek = pushptr > pushc ? *--pushptr : getc(fin);
	if(peek == EOF && sargc > 1){
		fclose(fin);
		fin = fopen(sargv[++fptr],"r");
		if(fin == NULL)
			error(MSGSTR(NOPEN2,"Cannot open file %s"),
			      sargv[fptr]);
		peek = getc(fin);
		sargc--;
		}
	if(c == EOF) {
		eof = TRUE;
		fclose(fin);
		return(0);
		}
	if(c == '\n')yyline++;
#ifdef KJI
	if ( NCisshift(c) )
#else /* NLS */
	if(c == shft[0] || c == shft[1] || c == shft[2] || c == shft[3])
#endif
		warning(MSGSTR(SHIFT, "Shift Character Found."));
	return(c);
	}
gch(){
	register int c;
	prev = pres;
	c = pres = peek;
	peek = pushptr > pushc ? *--pushptr : getc(fin);
	if(peek == EOF && sargc > 1){
		fclose(fin);
		fin = fopen(sargv[++fptr],"r");
		if(fin == NULL)
			error(MSGSTR(NOPEN2, "Cannot open file %s"),
			      sargv[fptr]);
		peek = getc(fin);
		sargc--;
		}
	if(c == EOF) {
		eof = TRUE;
		fclose(fin);
		return(0);
		}
	if(c == '\n')yyline++;
	return(c);
	}
mn2(a,d,c)
  int a;
  long d,c;
	{
	name[tptr] = a;
	left[tptr] = d;
	right[tptr] = c;
	parent[tptr] = 0;
	nullstr[tptr] = 0;
	switch(a){
	case RSTR:
		parent[d] = tptr;
		break;
	case BAR:
	case RNEWE:
		if(nullstr[d] || nullstr[c]) nullstr[tptr] = TRUE;
		parent[d] = parent[c] = tptr;
		break;
	case RCAT:
	case DIV:
		if(nullstr[d] && nullstr[c])nullstr[tptr] = TRUE;
		parent[d] = parent[c] = tptr;
		break;
	case RSCON:
		parent[d] = tptr;
		nullstr[tptr] = nullstr[d];
		break;
# ifdef DEBUG
	default:
		warning("bad switch mn2 %d %d",a,d);
		break;
# endif
		}
	if(tptr > treesize)
		error(MSGSTR(BIGPAR, "Parse tree too big %s"),
			(treesize == TREESIZE?
				MSGSTR(BIGPAR2, "\nTry using %%e num"):""));
	return(tptr++);
	}
mn1(a,d)
  int a;
  long d;
	{
	name[tptr] = a;
	left[tptr] = d;
	parent[tptr] = 0;
	nullstr[tptr] = 0;
	switch(a){
	case RCCL:
	case RNCCL:
		if(slength(d) == 0) nullstr[tptr] = TRUE;
		break;
	case STAR:
	case QUEST:
		nullstr[tptr] = TRUE;
		parent[d] = tptr;
		break;
	case PLUS:
	case CARAT:
		nullstr[tptr] = nullstr[d];
		parent[d] = tptr;
		break;
	case S2FINAL:
		nullstr[tptr] = TRUE;
		break;
# ifdef DEBUG
	case FINAL:
	case S1FINAL:
		break;
	default:
		warning("bad switch mn1 %d %ld",a,d);
		break;
# endif
		}
	if(tptr > treesize)
		error(MSGSTR(BIGPAR, "Parse tree too big %s"),
			(treesize == TREESIZE?
			MSGSTR(BIGPAR2, "\nTry using %%e num"):""));
	return(tptr++);
	}
mn0(a)
  int a;
	{
	name[tptr] = a;
	parent[tptr] = 0;
	nullstr[tptr] = 0;
	if(a >= NCH) switch(a){
	case RNULLS: nullstr[tptr] = TRUE; break;
# ifdef DEBUG
	default:
		warning("bad switch mn0 %d",a);
		break;
# endif
	}
	if(tptr > treesize)
		error(MSGSTR(BIGPAR, "Parse tree too big %s"),
			(treesize == TREESIZE?
			MSGSTR(BIGPAR2, "\nTry using %%e num"):""));
	return(tptr++);
	}
munput(t,p)	/* implementation dependent */
  unsigned char *p;
  int t; {
	register int i,j;
	if(t == 'c'){
		*pushptr++ = peek;		/* watch out for this */
		peek = (int)p;
		}
	else if(t == 's'){
		*pushptr++ = peek;
		peek = p[0];
		i = slength(p);
		for(j = i-1; j>=1; j--)
			*pushptr++ = p[j];
		}
# ifdef DEBUG
	else error("Unrecognized munput option %c",t);
# endif
	if(pushptr >= pushc+TOKENSIZE)
		error(MSGSTR(CHARPUSH, "Too many characters pushed"));
	return;
	}

dupl(n)
  int n; {
	/* duplicate the subtree whose root is n, return ptr to it */
	register int i;
	i = name[n];
	if(i < NCH) return(mn0(i));
	switch(i){
	case RNULLS:
		return(mn0(i));
	case RCCL: case RNCCL: case FINAL: case S1FINAL: case S2FINAL:
		return(mn1(i,left[n]));
	case STAR: case QUEST: case PLUS: case CARAT:
		return(mn1(i,dupl(left[n])));
	case RSTR: case RSCON:
		return(mn2(i,dupl(left[n]),right[n]));
	case BAR: case RNEWE: case RCAT: case DIV:
		return(mn2(i,dupl(left[n]),dupl(right[n])));
# ifdef DEBUG
	default:
		warning("bad switch dupl %d",n);
# endif
	}
	return(0);
	}
# ifdef DEBUG
allprint(c)
  unsigned char c; {
	switch(c){
		case 014:
			printf("\\f");
			charc++;
			break;
		case '\n':
			printf("\\n");
			charc++;
			break;
		case '\t':
			printf("\\t");
			charc++;
			break;
		case '\b':
			printf("\\b");
			charc++;
			break;
		case ' ':
			printf("\\\bb");
			break;
		default:
			if(!printable(c)){
				printf("\\%-3o",c);
				charc =+ 3;
				}
			else 
				putchar(c);
			break;
		}
	charc++;
	return;
	}
strpt(s)
  char *s; {
	charc = 0;
	while(*s){
		allprint(*s++);
		if(charc > LINESIZE){
			charc = 0;
			printf("\n\t");
			}
		}
	return;
	}
sect1dump(){
	register int i;
	printf("Sect 1:\n");
	if(def[0]){
		printf("str	trans\n");
		i = -1;
		while(def[++i])
			printf("%s\t%s\n",def[i],subs[i]);
		}
	if(sname[0]){
		printf("start names\n");
		i = -1;
		while(sname[++i])
			printf("%s\n",sname[i]);
		}
	if(chset == TRUE){
		printf("char set changed\n");
		for(i=1;i<NCH;i++){
			if(i != ctable[i]){
				allprint(i);
				putchar(' ');
				printable(ctable[i]) ? putchar(ctable[i]) :
					printf("%d",ctable[i]);
				putchar('\n');
				}
			}
		}
	}
sect2dump(){
	printf("Sect 2:\n");
	treedump();
	}
treedump()
	{
	register int t;
	register unsigned char *p;
	printf("treedump %d nodes:\n",tptr);
	for(t=0;t<tptr;t++){
		printf("%4d ",t);
		parent[t] ? printf("p=%4d",parent[t]) : printf("      ");
		printf("  ");
		if(name[t] < NCH) {
				allprint(name[t]);
				}
		else switch(name[t]){
			case RSTR:
				printf("%ld ",left[t]);
				allprint(right[t]);
				break;
			case RCCL:
				printf("ccl ");
				strpt(left[t]);
				break;
			case RNCCL:
				printf("nccl ");
				strpt(left[t]);
				break;
			case DIV:
				printf("/ %ld %ld",left[t],right[t]);
				break;
			case BAR:
				printf("| %ld %ld",left[t],right[t]);
				break;
			case RCAT:
				printf("cat %ld %ld",left[t],right[t]);
				break;
			case PLUS:
				printf("+ %ld",left[t]);
				break;
			case STAR:
				printf("* %ld",left[t]);
				break;
			case CARAT:
				printf("^ %ld",left[t]);
				break;
			case QUEST:
				printf("? %ld",left[t]);
				break;
			case RNULLS:
				printf("nullstring");
				break;
			case FINAL:
				printf("final %ld",left[t]);
				break;
			case S1FINAL:
				printf("s1final %ld",left[t]);	
				break;
			case S2FINAL:
				printf("s2final %ld",left[t]);
				break;
			case RNEWE:
				printf("new %ld %ld",left[t],right[t]);
				break;
			case RSCON:
				p = (unsigned char *) right[t];
				printf("start %s",sname[*p++-1]);
				while(*p)
					printf(", %s",sname[*p++-1]);
				printf(" %ld",left[t]);
				break;
			default:
				printf("unknown %d %ld %ld",name[t],left[t],
					right[t]);
				break;
			}
		if(nullstr[t])printf("\t(null poss.)");
		putchar('\n');
		}
	}
# endif
