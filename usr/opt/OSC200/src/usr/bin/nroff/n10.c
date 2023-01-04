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
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
#if !defined(lint) && !defined(_NOIDENT)
static char rcsid[] = "@(#)$RCSfile: n10.c,v $ $Revision: 4.2.9.3 $ (DEC) $Date: 1993/10/14 04:17:18 $";
#endif
/*
 * HISTORY
 */
/*
 * OSF/1 Release 1.0
 */
/*
 * n10.c	4.3 4/27/88
 *
 */

/*
 * Modification History:
 *
 * 001 21-May-91	gws
 *	this came from usr/local/nosupport/nroff n10.c -r3.1
 *	cleaned up history comments; removed extraneous ifndef lint/endif
 *
 */

#include "tdef.h"
#include <sgtty.h>
extern
#include "d.h"
extern
#include "v.h"
extern
#include "tw.h"
/*
nroff10.c

Device interfaces
*/

extern int lss;
extern char obuf[];
extern char *obufp;
extern int xfont;
extern int esc;
extern int lead;
extern int oline[];
extern int *olinep;
extern int ulfont;
extern int esct;
extern int sps;
extern int ics;
extern int ttysave;
extern struct sgttyb ttys;
extern char termtab[];
extern int ptid;
extern int waitf;
extern int pipeflg;
extern int eqflg;
extern int hflg;
extern int tabtab[];
extern int ascii;
extern int xxx;
int dtab;
int bdmode;
int plotmode;

/* +++ WL001 +++ */
#ifdef	mips
static void tab_parse();
#endif
extern nl_catd catd;
char *Gen_Codeptr() ;	/* Function to return a pseudo char code string ptr */
extern int mb_lang;
/* --- WL001 --- */

ptinit(){
/* +++ WL001-1 +++ */
#ifdef	mips
	register int i;
#else	/* vax */
	register long i, j;
	register char **p;
	char *q;
	int x[8];
	extern char *setbrk();
#endif
/* --- WL001-1 --- */

	if(((i=open(termtab,0)) < 0) && (i=open("/usr/lib/term/tablpr",0)) < 0){
		prstr(catgets(catd, 1, 5, "Cannot open "));
		prstr(termtab);
		prstr("\n");
		exit(-1);
	}

/* +++ WL001-1 +++ */
#ifdef	mips
	/*
	 * Read in ASCII format terminal table
	 */

	tab_parse(i);
	(void)close(i);
#else
	read(i,(char *)x,8*sizeof(int));
	read(i,(char *)&t.bset,j = sizeof(t));
	x[2] -= j;
	q = setbrk(x[2]);
	i = read(i,q,x[2]);
	j = q - t.twinit;
	for(p = &t.twinit; p < &t.zzz; p++){
		if(*p)*p += j;else *p = "";
	}
#endif
/* --- WL001-1 --- */

	sps = EM;
	ics = EM*2;
	dtab = 8 * t.Em;
	for(i=0; i<16; i++)tabtab[i] = dtab * (i+1);
	if(eqflg)t.Adj = t.Hor;
}
twdone(){
	obufp = obuf;
	oputs(t.twrest);
	flusho();
	if(pipeflg){
		close(ptid);
		wait(&waitf);
	}
	if(ttysave != -1) {
		ttys.sg_flags = ttysave;
		stty(1, &ttys);
	}
}
ptout(i)
int i;
{
	*olinep++ = i;
	if(olinep >= &oline[LNSIZE])olinep--;
	if((i&CMASK) != '\n')return;
	olinep--;
	lead += dip->blss + lss - t.Newline;
	dip->blss = 0;
	esct = esc = 0;
	if(olinep>oline){
		move();
		ptout1();
		oputs(t.twnl);
	}else{
		lead += t.Newline;
		move();
	}
	lead += dip->alss;
	dip->alss = 0;
	olinep = oline;
}
ptout1()
{
	register i, k;
	register char *codep;
/* +++ WW001 +++ */
	register int num_bytes;
	register int mb=mb_lang;
/* --- WW001 --- */
	extern char *plot();
	int *q, w, j, phyw;

	for(q=oline; q<olinep; q++){
	if((i = *q) & MOT){
		j = i & ~MOTV;
		if(i & NMOT)j = -j;
		if(i & VMOT)lead += j;
		else esc += j;
		continue;
	}
	if((k = (i & CMASK)) <= 040){
		switch(k){
			case ' ': /*space*/
				esc += t.Char;
				break;
		}
		continue;
	}
	if (mb) {
		if (IS_ENCODED_ACHAR(k))
			codep = Gen_Codeptr(k)  ;
		else
			codep = t.codetab[k-32] ;
		if (!codep) {
			prstr(catgets(catd, 1, 28, 
				"Illegal data in input file.\n")) ;
			exit (1) ;
		}
		w = t.Char * (num_bytes = (*codep++ & 0177));
	}
	else {
		codep = t.codetab[k-32];
		w = t.Char * (*codep++ & 0177);
	}
	phyw = w;
	if(i&ZBIT)w = 0;
	if(*codep && (esc || lead))move();
	esct += w;
	if(i&074000)xfont = (i>>9) & 03;
	if(*t.bdon & 0377){
		if(!bdmode && (xfont == 2)){
			oputs(t.bdon);
			bdmode++;
		}
		if(bdmode && (xfont != 2)){
			oputs(t.bdoff);
			bdmode = 0;
		}
	}

	if(xfont == ulfont){
		for(k=w/t.Char;k>0;k--)oput('_');
		for(k=w/t.Char;k>0;k--)oput('\b');
	}
	while(*codep != 0){
/* +++ WL001 +++ */
		if (!IS_ENCODED_ACHAR(i) && (*codep & 0200)) {
/* --- WL001 --- */
			codep = plot(codep);
			oputs(t.plotoff);
			oput(' ');
		}else{
			if(plotmode)oputs(t.plotoff);
			/*
			 * simulate bold font as overstrike if no t.bdon
			 */
/* +++ WW001 +++ */
			if (mb) {	/* for efficiency */
				if (xfont == 2 && !(*t.bdon & 0377) &&
				    (*codep != '\b')) {
					for(k=w/t.Char;k>0;k--)oput(*codep++);
					for(k=w/t.Char;k>0;k--)oput('\b');
					codep-=(w/t.Char);
				}
				/* make sure to have enough space left */
				if(obufp >= (obuf + OBUFSZ + ascii - num_bytes))
					flusho();
				bcopy(codep,obufp,num_bytes);
				obufp+=num_bytes;
				codep+=num_bytes;
			}
			else {
				if (xfont == 2 && !(*t.bdon & 0377) &&
				    (*codep != '\b')) {
					oput(*codep);
					oput('\b');
				}
				*obufp++ = *codep++;
			}
/* --- WW001 --- */
			if(obufp == (obuf + OBUFSZ + ascii - 1))flusho();
/*			oput(*codep++);*/
		}
	}
	if(!w)for(k=phyw/t.Char;k>0;k--)oput('\b');
	}
}
char *plot(x)
char *x;
{
	register int i;
	register char *j, *k;

	if(!plotmode)oputs(t.ploton);
	k = x;
	if((*k & 0377) == 0200)k++;
	for(; *k; k++){
		if(*k & 0200){
			if(*k & 0100){
				if(*k & 040)j = t.up; else j = t.down;
			}else{
				if(*k & 040)j = t.left; else j = t.right;
			}
			if(!(i = *k & 037))return(++k);
			while(i--)oputs(j);
		}else oput(*k);
	}
	return(k);
}
move(){
	register k;
	register char *i, *j;
	char *p, *q;
	int iesct, dt;

	iesct = esct;
	if(esct += esc)i = "\0"; else i = "\n\0";
	j = t.hlf;
	p = t.right;
	q = t.down;
	if(lead){
		if(lead < 0){
			lead = -lead;
			i = t.flr;
		/*	if(!esct)i = t.flr; else i = "\0";*/
			j = t.hlr;
			q = t.up;
		}
		if(*i & 0377){
			k = lead/t.Newline;
			lead = lead%t.Newline;
			while(k--)oputs(i);
		}
		if(*j & 0377){
			k = lead/t.Halfline;
			lead = lead%t.Halfline;
			while(k--)oputs(j);
		}
		else { /* no half-line forward, not at line begining */
			k = lead/t.Newline;
			lead = lead%t.Newline;
			if (k>0) esc=esct;
			i = "\n";
			while (k--) oputs(i);
		}
	}
	if(esc){
		if(esc < 0){
			esc = -esc;
			j = "\b";
			p = t.left;
		}else{
			j = " ";
			if(hflg)while((dt = dtab - (iesct%dtab)) <= esc){
				/* 002-kak Changed dt==t.Em to    */
				/* dt<t.Em to make sure that      */
				/* if the width of a space t.Em   */
				/* is less than the current point */
				/* size, because if it is equal   */
				/* to it you will not be able to  */
				/* get the hard tabs needed when  */
				/* the program encounters letters */
				/* equal to the space             */
				if((dt%t.Em) || (dt<t.Em))break;
				oput(TAB);
				esc -= dt;
				iesct += dt;
			}
		}
		k = esc/t.Em;
		esc = esc%t.Em;
		while(k--)oputs(j);
	}
	if((*t.ploton & 0377) && (esc || lead)){
		if(!plotmode)oputs(t.ploton);
		esc /= t.Hor;
		lead /= t.Vert;
		while(esc--)oputs(p);
		while(lead--)oputs(q);
		oputs(t.plotoff);
	}
	esc = lead = 0;
}
ptlead(){move();}
dostop(){
	char junk;

	flusho();
	read(2,&junk,1);
}

/* +++ WL001-1 +++ */
#ifdef	mips
/*
 * Table parser. This command reads from the given file descriptor, and
 * parses the data into the terminal driver structure t.
 */

static int tabgetnum();
static char *tabgetstr();
static int get_character();

static int Eof_seen;			/* Set upon EOF			*/
static char String_space[8192];		/* Space for strings		*/
static char *String_place;		/* Current string place		*/
static char Empty_string[3];		/* 3 nulls			*/

static void
tab_parse(fd)
int fd;
{

	int tab_no;		/* Code table index			*/

	/*
	 * The following values are initialized for reading the table.
	 * Eof_seen is 1 when EOF is reached, and all subsequent integers
	 * are 0s, and strings will all be set to point to Empty_string, which
	 * is a string of 3 nulls (control, character, null terminator).
	 */

	Eof_seen = 0;
	String_place = &String_space[0];

	/*
	 * Read in the numbers.
	 */

	t.bset = tabgetnum(fd);
	t.breset = tabgetnum(fd);
	t.Hor = tabgetnum(fd);
	t.Vert = tabgetnum(fd);
	t.Newline = tabgetnum(fd);
	t.Char = tabgetnum(fd);
	t.Em = tabgetnum(fd);
	t.Halfline = tabgetnum(fd);
	t.Adj = tabgetnum(fd);

	/*
	 * Now, the known strings (everything but codetab).
	 */

	t.twinit = tabgetstr(fd);
	t.twrest = tabgetstr(fd);
	t.twnl = tabgetstr(fd);
	t.hlr = tabgetstr(fd);
	t.hlf = tabgetstr(fd);
	t.flr = tabgetstr(fd);
	t.bdon = tabgetstr(fd);
	t.bdoff = tabgetstr(fd);
	t.ploton = tabgetstr(fd);
	t.plotoff = tabgetstr(fd);
	t.up = tabgetstr(fd);
	t.down = tabgetstr(fd);
	t.right = tabgetstr(fd);
	t.left = tabgetstr(fd);

	/*
	 * Now, read each code table element.
	 */

	tab_no = 0;
	while (tab_no < CTABSIZE) {
		t.codetab[tab_no] = tabgetstr(fd);
		tab_no++;
	}
}

/*
 * The subroutine tabgetnum() reads a line from the given file descriptor,
 * and converts it to a number. The number is returned. An invalid number
 * does not cause an error message.
 */

static int
tabgetnum(fd)
int fd;
{
	
	int accum;	/* Accumulator					*/
	int c;		/* Input character				*/

	if (Eof_seen) {
		return 0;
	}

	/*
	 * Read past any comments.
	 */

	if ((c = get_character(fd)) == 0) {
		Eof_seen = 1;
		return 0;
	}
	while (c == '#') {
		while (c != '\n') {
			if ((c = get_character(fd)) == 0) {
				Eof_seen = 1;
				return 0;
			}
		}
		if ((c = get_character(fd)) == 0) {
			Eof_seen = 1;
			return 0;
		}
	}

	/*
	 * Get each digit.
	 */

	accum = 0;
	while (c != '\n') {
		accum *= 10;
		accum += c - '0';
		if ((c = get_character(fd)) == 0) {
			Eof_seen = 1;
			return accum;
		}
	}
	return accum;
}

/*
 * The subroutine tabgetstr() reads a line from the given file descriptor,
 * converts all escaped octal characters (\xxx) to corresponding characters,
 * converts \n, \b, \t, \r, and \f to the appropriate characters,
 * and returns a pointer to the null-terminated string.
 *
 * Note that \xxx is interpreted as "at most 3 digits", so \01x is interpreted
 * as \01 and x, but \011 is interpreted as \011.
 */

static char *
tabgetstr(fd)
int fd;
{

	int accum;	/* Value accumulator			*/
	int c;		/* Input character			*/
	char *retval;	/* Pointer to current string space	*/

	if (Eof_seen) {
		return Empty_string;
	}

	String_place++;
	retval = String_place;

	/*
	 * Read past any comments.
	 */

	if ((c = get_character(fd)) == 0) {
		Eof_seen = 1;
		return 0;
	}
	while (c == '#') {
		while (c != '\n') {
			if ((c = get_character(fd)) == 0) {
				Eof_seen = 1;
				return 0;
			}
		}
		if ((c = get_character(fd)) == 0) {
			Eof_seen = 1;
			return 0;
		}
	}

	/*
	 * Read characters up to a newline. Empty string isn empty line is a
	 * special case.
	 */

	if (c == '\n') {
		return Empty_string;
	}

	while (c != '\n') {
		if (c == '\\') {
			accum = 0;
			if ((c = get_character(fd)) == 0) {
				Eof_seen = 1;
				*String_place = (accum & 0xff);
				String_place++;
				*String_place = '\0';
				return retval;
			}
			if (c < '0' || c > '7') {
				switch (c) {

				case 'n':
					c = '\n';
					break;

				case 'r':
					c = '\r';
					break;

				case 'b':
					c = '\b';
					break;

				case 't':
					c = '\t';
					break;

				case 'f':
					c = '\f';
					break;
				}
				*String_place = c;
				String_place++;
				if ((c = get_character(fd)) == 0) {
					Eof_seen = 1;
					*String_place = '\0';
					return retval;
				}
				continue;
			}

			accum *= 8;
			accum += c - '0';

			/*
			 * Second digit
			 */

			if ((c = get_character(fd)) == 0) {
				Eof_seen = 1;
				*String_place = (accum & 0xff);
				String_place++;
				*String_place = '\0';
				return retval;
			}

			if (c < '0' || c > '7') {
				*String_place = (accum & 0xff);
				String_place++;
				continue;
			}

			accum *= 8;
			accum += c - '0';

			/*
			 * Third digit
			 */

			if ((c = get_character(fd)) == 0) {
				Eof_seen = 1;
				*String_place = (accum & 0xff);
				String_place++;
				*String_place = '\0';
				return retval;
			}

			if (c < '0' || c > '7') {
				*String_place = (accum & 0xff);
				String_place++;
				continue;
			}

			accum *= 8;
			accum += c - '0';
			*String_place = (accum & 0xff);
			String_place++;
		} else {	/* c != '\'	*/
			*String_place = c;
			String_place++;
		}

		if ((c = get_character(fd)) == 0) {
			Eof_seen = 1;
			*String_place = '\0';
			return retval;
		}
	}

	*String_place = '\0';
	return retval;
}

/*
 * The subrutine get_character() reads a character from the given file
 * descriptor and returns the character or a '\0' (for EOF). If the character
 * is null or non-ASCII, an error message is printed and nroff exits.
 */

static int
get_character(fd)
int fd;
{

	char c;		/* Input character			*/

	if (read(fd, &c, sizeof(char)) != sizeof(char)) {
		return '\0';
	}

	if (c == '\0' || (c &0x80)) {
		prstr(catgets(catd, 1, 7, "Null or non-ASCII character in terminal table file "));
		prstr(termtab);
		prstr("\n");
		exit(1);
	}

	return (int) (c & 0xff);
}
#endif	/* mips */
/* --- WL001-1 --- */
