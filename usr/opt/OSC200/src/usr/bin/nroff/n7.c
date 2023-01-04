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
static char rcsid[] = "@(#)$RCSfile: n7.c,v $ $Revision: 1.1.4.3 $ (DEC) $Date: 1993/10/14 04:17:40 $";
#endif
/*
 * HISTORY
 */
/*
 * OSF/1 Release 1.0
 */
/*
 * n7.c	4.4 3/7/90
 *
 */

/*
 * Modification History:
 *
 * 001 21-May-91	gws
 *	this came from usr/local/nosupport/troff n7.c -r3.1
 *	cleaned up history comments; removed extraneous ifndef lint/endif
 *
 */

#include "tdef.h"
extern
#include "d.h"
extern
#include "v.h"
#ifdef NROFF
extern
#include "tw.h"
#endif
#include "sdef.h"
#ifdef NROFF
#define GETCH gettch
#endif
#ifndef NROFF
#define GETCH getch
#endif

/*
troff7.c

text
*/

extern struct s *frame, *stk;
extern struct s *ejl;

extern int pl;
extern int trap;
extern int flss;
extern int npnflg;
extern int npn;
extern int stop;
extern int nflush;
extern int ejf;
extern int ascii;
extern int donef;
extern int nc;
extern int wch;
extern int dpn;
extern int ndone;
extern int lss;
extern int pto;
extern int pfrom;
extern int print;
/* +++ WW001 +++ */
extern nl_catd catd;
/* --- WW001 --- */
extern int nroff_nlist[NTRAP]; /* 001-kak */
extern int mlist[NTRAP];
extern int *pnp;
extern int nb;
extern int ic;
extern int icf;
extern int ics;
extern int ne;
extern int ll;
extern int un;
extern int un1;
extern int in;
extern int ls;
extern int spread;
extern int totout;
extern int nwd;
extern int *pendw;
extern int *linep;
extern int line[];
extern int lastl;
extern int ch;
extern int ce;
extern int fi;
extern int nlflg;
extern int pendt;
extern int sps;
extern int adsp;
extern int pendnf;
extern int over;
extern int adrem;
extern int nel;
extern int ad;
extern int ohc;
extern int hyoff;
extern int nhyp;
extern int spflg;
extern int word[];
extern int *wordp;
extern int wne;
extern int chbits;
extern int cwidth;
extern int widthp;
extern int hyf;
extern int xbitf;
extern int vflag;
extern int ul;
extern int cu;
extern int font;
extern int sfont;
extern int it;
extern int itmac;
extern int *hyptr[NHYP];
extern int **hyp;
extern int *wdstart, *wdend;
extern int lnmod;
extern int admod;
extern int nn;
extern int nms;
extern int ndf;
extern int ni;
extern int nform;
extern int lnsize;
extern int po;
extern int ulbit;
extern int *vlist;
extern int nrbits;
extern int nmbits;
extern char trtab[];
extern int xxx;
int brflg;
/* +++ WL001 +++ */
extern PeekStack peek ;	/* Character peek stack */
extern int mb_lang;
/* --- WL001 --- */

tbreak(){
	register *i, j, pad;
	int res;
	static int curr_achar=0, prev_achar=0;
	register int mb=mb_lang;

	trap = 0;
	if(nb)return;
	if((dip == d) && (v.nl == -1)){
		newline(1);
		return;
	}
	if(!nc){
		setnel();
/* +++ WL001 +++ */
		if ((wch == 0) && ((j = Pop_From_Stack()) != 0)) {
			/* Pop all the characters left in peek stack */
			i = word ;
			do {
				if (j != ASIAN_NL_PAD) {
					*i++ = j 	;
					wch++    	;
					wne += width(j) ;
				}
					
			} while ((j = Pop_From_Stack()) != 0) ;
			wordp = word ;
		}
/* --- WL001 --- */
		if(!wch)return;
		if(pendw)getword(1);
		movword();
	}else if(pendw && !brflg){
		getword(1);
		movword();
	}
	*linep = dip->nls = 0;
#ifdef NROFF
	if(dip == d)horiz(po);
#endif
	if(lnmod)donum();
	lastl = ne;
	if(brflg != 1){
		totout = 0;
	}else if(ad){
		if((lastl = (ll - un)) < ne)lastl = ne;
	}
	if(admod && ad && (brflg != 2)){
		lastl = ne;
		adsp = adrem = 0;
#ifdef NROFF
		if(admod == 1)un +=  quant(nel/2,t.Adj);
#endif
#ifndef NROFF
		if(admod == 1)un += nel/2;
#endif
		else if(admod ==2)un += nel;
	}
	totout++;
	brflg = 0;
	if((lastl+un) > dip->maxl)dip->maxl = (lastl+un);
	horiz(un);
#ifdef NROFF
	if(adrem%t.Adj)res = t.Hor; else res = t.Adj;
#endif
/* +++ WL001 +++ */
	curr_achar = mb && IS_ENCODED_ACHAR(*line);
	for(i = line;nc > 0;){
		if (mb) {
			prev_achar=curr_achar;
			curr_achar = IS_ENCODED_ACHAR(j=*i++);
		}
		else j=*i++;
		if (((j & CMASK) == ' ') || Is_MByte_Space(j)) {
			pad = 0;
			do{
				pad += width(j);
				nc--;
			  }while ((((j = *i++) & CMASK) == ' ') ||
				     Is_MByte_Space(j)) ;
/* no char after space is considered a boundary case */
			curr_achar = IS_ENCODED_ACHAR(*i);
/* --- WL001 --- */
			i--;
			pad += adsp;
			--nwd;
			if(adrem){
				if(adrem < 0){
#ifdef NROFF
					pad -= res;
					adrem += res;
				}else if((totout&01) ||
					((adrem/res)>=(nwd))){
					pad += res;
					adrem -= res;
#endif
#ifndef NROFF
					pad--;
					adrem++;
				}else{
					pad++;
					adrem--;
#endif
				}
			}
			horiz(pad);
		}else if (mb && curr_achar && CanSpaceBefore(j)){
			--nwd;
			if (i>line+1) {		/* avoid a leading space */
				pad=adsp;
				if(adrem){
					if(adrem < 0){
#ifdef NROFF
						pad -= res;
						adrem += res;
					}else if((totout&01) ||
						((adrem/res)>=(nwd))){
						pad += res;
						adrem -= res;
#endif
					}
				}
				horiz(pad);/* pad space before achar */
			}
			pchar(j);
			nc--;
		}else if (mb && ((prev_achar && !curr_achar) ||
			 (!prev_achar && curr_achar))) {
			/* ASCII/ASIAN boundary */
			--nwd;
			if (i>line+1) {		/* avoid leading space */
				pad=adsp;
				if(adrem){
					if(adrem < 0){
#ifdef NROFF
						pad -= res;
						adrem += res;
					}else if((totout&01) ||
						((adrem/res)>=(nwd))){
						pad += res;
						adrem -= res;
#endif
					}
				}
				horiz(pad);	/* pad space before achar */
			}
			pchar(j);
			nc--;
		}else{
			pchar(j);
			nc--;
		}
/* After preceeding space and char are put, check if trailing space needed */
		if (mb && curr_achar && CanSpaceAfter(j)) {
			--nwd;
			if (nc>0 &&   /* for safety, avoid trailing space */
			    (IS_ENCODED_ACHAR(*i) && !CanSpaceBefore(*i))) {
				pad=adsp;
				if(adrem){
					if(adrem < 0){
#ifdef NROFF
						pad -= res;
						adrem += res;
					}else if((totout&01) ||
						((adrem/res)>=(nwd))){
						pad += res;
						adrem -= res;
#endif
					}
				}
				horiz(pad);	/* pad space after achar */
			}
		}
	}
	if(ic){
		if((j = ll - un - lastl + ics) > 0)horiz(j);
		pchar(ic);
	}
	if(icf)icf++;
		else ic = 0;
	ne = nwd = 0;
	un = in;
	setnel();
	newline(0);
	if(dip != d){if(dip->dnl > dip->hnl)dip->hnl = dip->dnl;}
	else{if(v.nl > dip->hnl)dip->hnl = v.nl;}
	for(j=ls-1; (j >0) && !trap; j--)newline(0);
	spread = 0;
}
donum(){
	register i, nw;
	extern pchar();

	nrbits = nmbits;
	nw = width('1' | nrbits);
	if(nn){
		nn--;
		goto d1;
	}
	if(v.ln%ndf){
		v.ln++;
	d1:
		un += nw*(3+nms+ni);
		return;
	}
	i = 0;
	if(v.ln<100)i++;
	if(v.ln<10)i++;
	horiz(nw*(ni+i));
	nform = 0;
	fnumb(v.ln,pchar);
	un += nw*nms;
	v.ln++;
}
text(){
	register i;
	static int spcnt;

	nflush++;
	if((dip == d) && (v.nl == -1)){newline(1); return;}
	setnel();
	if(ce || !fi){
		nofill();
		return;
	}
	if(pendw)goto t4;
	if(pendt)if(spcnt)goto t2; else goto t3;
	pendt++;
	if(spcnt)goto t2;
/* +++ WL001 +++ */
	while ((((i = getch()) & CMASK) == ' ') || Is_MByte_Space(i))
		spcnt += ((i & CMASK) == ' ') ? 1 : 2 ;
/* --- WL001 --- */
	if(nlflg){
	t1:
		nflush = pendt = ch = spcnt = 0;
		callsp();
		return;
	}
	ch = i;
	if(spcnt){
	t2:
/* +++ WL001 +++ */
		Flush_Stack() ; /* Flush the stack before doing tbreak() */
/* --- WL001 --- */
		tbreak();
		if(nc || wch)goto rtn;
		un += spcnt*sps;
		spcnt = 0;
		setnel();
		if(trap)goto rtn;
		if(nlflg)goto t1;
	}
t3:
	if(spread)goto t5;
	if(pendw || !wch)
	t4:
		if(getword(0))goto t6;
	if(!movword())goto t3;
t5:
/* +++ WL001 +++ */
	/* Break line with no-first & no-last constraints in mind */
	Check_NoFirst_NoLast_Char() ;
/* --- WL001 --- */
	if(nlflg)pendt = 0;
	adsp = adrem = 0;
	if(ad){
/* jfr */	if (nwd==1) adsp=nel; else adsp=nel/(nwd-1);
#ifdef NROFF
		adsp = (adsp/t.Adj)*t.Adj;
#endif
		adrem = nel - adsp*(nwd-1);
	}
	brflg = 1;
	tbreak();
	spread = 0;
	if(!trap)goto t3;
	if(!nlflg)goto rtn;
t6:
	pendt = 0;
	ckul();
rtn:
	nflush = 0;
}
nofill(){
	register i, j;

	if(!pendnf){
		over = 0;
		tbreak();
		if(trap)goto rtn;
		if(nlflg){
			ch = nflush = 0;
			callsp();
			return;
		}
		adsp = adrem = 0;
		nwd = 10000;
	}
	while((j = ((i = GETCH()) & CMASK)) != '\n'){
		if(j == ohc)continue;
/* +++ WL001 +++ */
		if (j == ASIAN_NL_PAD) continue ;
/* --- WL001 --- */
		if(j == CONT){
			pendnf++;
			nflush = 0;
			flushi();
			ckul();
			return;
		}
		storeline(i,-1);
	}
	if(ce){
		ce--;
		if((i=quant(nel/2,HOR)) > 0)un += i;
	}
	if(!nc)storeline(FILLER,0);
	brflg = 2;
	tbreak();
	ckul();
rtn:
	pendnf = nflush = 0;
}
callsp(){
	register i;

	if(flss)i = flss; else i = lss;
	flss = 0;
	casesp(i);
}
ckul(){
/* +++ WL001 +++ */
	extern int check_trap ;

	check_trap = TRUE ;
/* --- WL001 --- */
	if(ul && (--ul == 0)){
			cu = 0;
			font = sfont;
			mchbits();
	}
	if(it && (--it == 0) && itmac)control(itmac,0);
/* +++ WL001 +++ */
	check_trap = FALSE ;
/* --- WL001 --- */
}
storeline(c,w){
	register i;
	int *p;

	if((c & CMASK) == JREG){
		if((i=findr(c>>BYTE)) != -1)vlist[i] = ne;
		return;
	}
	if(linep >= (line + lnsize - 1)){
		if(!over){
			prstrfl(catgets(catd, 1, 23, "Line overflow.\n"));
			over++;
		c = 0343;
		w = -1;
		goto s1;
		}
		return;
	}
s1:
	if(w == -1)w = width(c);
	ne += w;
	nel -= w;
/*
 *	if( cu && !(c & MOT) && (trtab[(c & CMASK)] == ' '))
 *		c = ((c & ~ulbit) & ~CMASK) | '_';
 */
	*linep++ = c;
	nc++;
}
newline(a)
int a;
{
	register i, j, nlss;
	int opn;
/* +++ WL001 +++ */
	extern int check_trap ;

	check_trap = TRUE ;
/* --- WL001 --- */

	if(a)goto nl1;
	if(dip != d){
		j = lss;
		pchar1(FLSS);
		if(flss)lss = flss;
		i = lss + dip->blss;
		dip->dnl += i;
		pchar1(i);
		pchar1('\n');
		lss = j;
		dip->blss = flss = 0;
		if(dip->alss){
			pchar1(FLSS);
			pchar1(dip->alss);
			pchar1('\n');
			dip->dnl += dip->alss;
			dip->alss = 0;
		}
		if(dip->ditrap && !dip->ditf &&
			(dip->dnl >= dip->ditrap) && dip->dimac)
			if(control(dip->dimac,0)){trap++; dip->ditf++;}
/* +++ WL001 +++ */
		check_trap = FALSE ;
/* --- WL001 --- */
		return;
	}
	j = lss;
	if(flss)lss = flss;
	nlss = dip->alss + dip->blss + lss;
	v.nl += nlss;
#ifndef NROFF
	if(ascii){dip->alss = dip->blss = 0;}
#endif
	pchar1('\n');
	flss = 0;
	lss = j;
	if(v.nl < pl)goto nl2;
nl1:
	ejf = dip->hnl = v.nl = 0;
	ejl = frame;
	if(donef == 1){
		if((!nc && !wch) || ndone)done1(0);
		ndone++;
		donef = 0;
		if(frame == stk)nflush++;
	}
	opn = v.pn;
	v.pn++;
	if(npnflg){
		v.pn = npn;
		npn = npnflg = 0;
	}
nlpn:
	if(v.pn == pfrom){
		print++;
	}else if(opn == pto || v.pn > pto){
		print = 0;
		chkpn();
		goto nlpn;
		}
	if(stop && print){
		dpn++;
		if(dpn >= stop){
			dpn = 0;
			dostop();
		}
	}
nl2:
	trap = 0;
	if(v.nl == 0){
		if((j = findn(0)) != NTRAP)
			trap = control(mlist[j],0);
	} else if((i = findt(v.nl-nlss)) <= nlss){
		if((j = findn1(v.nl-nlss+i)) == NTRAP){
			prstrfl(catgets(catd, 1, 24, "Trap botch.\n"));
			done2(-5);
		}
		trap = control(mlist[j],0);
	}
/* +++ WL001 +++ */
	check_trap = FALSE ;
/* --- WL001 --- */
}
findn1(a)
int a;
{
	register i, j;

	for(i=0; i<NTRAP; i++){
		if(mlist[i]){
			if((j = nroff_nlist[i]) < 0)j += pl; /* 001-kak */
			if(j == a)break;
		}
	}
	return(i);
}
chkpn(){
	pfrom = pto = *(pnp++);
	if(pto == -1){
		flusho();
		done1(0);
	}
	if(pto == -2){
		print++;
		pfrom = 0;
		pto = 10000;
	}
	else if(pto & MOT){
		print++;
		pto &= ~MOT;
		pfrom = 0;
	}
}
findt(a)
int a;
{
	register i, j, k;

	k = 32767;
	if(dip != d){
		if(dip->dimac && ((i = dip->ditrap -a) > 0))k = i;
		return(k);
	}
	for(i=0; i<NTRAP; i++){
		if(mlist[i]){
			if((j = nroff_nlist[i]) < 0)j += pl; /* 001-kak */
			if((j -= a)  <=  0)continue;
			if(j < k)k = j;
		}
	}
	i = pl - a;
	if(k > i)k = i;
	return(k);
}
findt1(){
	register i;

	if(dip != d)i = dip->dnl;
		else i = v.nl;
	return(findt(i));
}
eject(a)
struct s *a;
{
	register savlss;

	if(dip != d)return;
	ejf++;
	if(a)ejl = a;
		else ejl = frame;
	if(trap)return;
e1:
	savlss = lss;
	lss = findt(v.nl);
	newline(0);
	lss = savlss;
	if(v.nl && !trap)goto e1;
}
movword(){
	register i, w, *wp;
	int savwch, hys;

/* +++ WL001 +++ */
	/* nwd will be incremented only if this flag is TRUE */
	short	preceded_by_space = FALSE ;
	static int curr_achar=0;
	static int prev_achar;
	static int inserted_space_after=0;	/* space ins in prev call */
/* --- WL001 --- */

	over = 0;
	wp = wordp;
	if(!nwd){
/* +++ WL001 +++ */
		preceded_by_space = TRUE ; /* Enforce incrementing nwd */
		while ((((i = *wp++) & CMASK) == ' ') || Is_MByte_Space(i)) {
/* --- WL001 --- */
			wch--;
			wne -= width(i);
		}
		wp--;
	}
/* +++ WL001 +++ */
	else {	/* Check if there is space in front of the word */
		if (((*wp & CMASK) == ' ') || Is_MByte_Space(*wp))
			preceded_by_space = TRUE ;
	}
	if((wne > nel) &&
	   !IS_ENCODED_ACHAR(*wp) &&	/* Do hyphen() if not Asian char */
/* --- WL001 --- */
	   !hyoff && hyf &&
	   (!nwd || (nel > 3*sps)) &&
	   (!(hyf & 02) || (findt1() > lss))
	  )hyphen(wp);
	savwch = wch;
	hyp = hyptr;
	nhyp = 0;
	while(*hyp && (*hyp <= wp))hyp++;
	while(wch){
		if((hyoff != 1) && (*hyp == wp)){
			hyp++;
			if(!wdstart ||
			   ((wp > (wdstart+1)) &&
			    (wp < wdend) &&
			    (!(hyf & 04) || (wp < (wdend-1))) &&
			    (!(hyf & 010) || (wp > (wdstart+2)))
			   )
			  ){
				nhyp++;
				storeline(IMP,0);
			}
		}
		i = *wp++;
		w = width(i);
		wne -= w;
		wch--;
		storeline(i,w);
	}
	if(nel >= 0){
/* +++ WL001 +++ */
/*
 * Count ndw wrt the number of places which can insert spaces, rather than
 * the actual number of spaces.  Note that Asian chars are read one by one
 * with each char as a word, so it's just necessary to check the last char
 * stored to be Asian or not.
 */
		prev_achar=curr_achar;
		curr_achar=IS_ENCODED_ACHAR(i);
/* count virtual spaces before */
		if (preceded_by_space ||
		    (curr_achar && CanSpaceBefore(i)) ||
		    (prev_achar && !curr_achar) ||
		    (!prev_achar && curr_achar)) nwd++;
/* count virtual spaces after */
		if (curr_achar && CanSpaceAfter(i)) nwd++;
/* --- WL001 --- */
		return(0);
	}
	xbitf = 1;
	hys = width(0200); /*hyphen*/
m1:
	if(!nhyp){
		if(!nwd)goto m3;
		if(wch == savwch)goto m4;
	}
	if(*--linep != IMP)goto m5;
	if(!(--nhyp))
		if(!nwd)goto m2;
	if(nel < hys){
		nc--;
		goto m1;
	}
m2:
	if(((i = *(linep-1) & CMASK) != '-') &&
	   (i != 0203)
	  ){
	*linep = (*(linep-1) & ~CMASK) | 0200;
	w = width(*linep);
	nel -= w;
	ne += w;
	linep++;
/*
	hsend();
*/
	}
m3:
	nwd++;
m4:
	wordp = wp;
	return(1);
m5:
	nc--;
	w = width(*linep);
	ne -= w;
	nel += w;
	wne += w;
	wch++;
	wp--;
	goto m1;
}
horiz(i)
int i;
{
	vflag = 0;
	if(i)pchar(makem(i));
}
setnel(){
/* +++ WL001 +++ */
	extern int  is_flushing_stack ;

	if (!nc && !is_flushing_stack) {
/* --- WL001 --- */
		linep = line;
		if(un1 >= 0){
			un = un1;
			un1 = -1;
		}
		nel = ll - un;
		ne = adsp = adrem = 0;
	}
}
/* +++ WL001 +++ */
/*
 * This routine extracts one word from the input stream. 
 * Because each Asian character is treated as a single word, its processing
 * is different from the normal ASCII characters. Originally this routine
 * removes 1 space from the end of a word and inserts 1 space immediately
 * before the next word. It doesn't work for Asian characters as spaces
 * should not be inserted between them. Instead, we don't discard space,
 * but save it in stack. We break word at every Asian-Asian and Asian-ASCII
 * character boundary unless it is glued together by motion characters.
 * One problem with this scheme is that the last word of a line will be
 * concatenated with the first word of the next line. So we have to insert
 * a space at the line break point unless it is an Asian-Asian character
 * boundary.
 */
static int achar_flag ;

/* This achar_flag is set to one or a combination of the following */
/* values or is cleared when none of them is applicable.	   */
#define RD_ACHAR	1 	/* An Asian character read  */
#define RD_MOTION	2	/* A motion character read  */
#define RD_NEWLINE	4	/* A newline character read */

/* --- WL001 --- */
getword(x)
int x;
{
	register i, j, swp;
	int noword;

	noword = 0;
	if(x)if(pendw){
		*pendw = 0;
		goto rtn;
	}
	if(wordp = pendw)goto g1;
	hyp = hyptr;
	wordp = word;
	over = wne = wch = 0;
	hyoff = 0;
	while(1){
		j = (i = GETCH()) & CMASK;
		if(j == '\n'){
			wne = wch = 0;
			noword = 1;
/* +++ WL001 +++ */
			Push_To_Stack(ASIAN_NL_PAD) ; /* Remember \n read */
/* --- WL001 --- */
			goto rtn;
		}
		if(j == ohc){
			hyoff = 1;
			continue;
		}
/* +++ WL001 +++ */
		if (j == ASIAN_NL_PAD) {
			achar_flag |= RD_NEWLINE ;
			continue 		 ;
		}
		if ((j == ' ') || Is_MByte_Space(j)) {
/* --- WL001 --- */
			storeword(i,width(i));	/* XXX */
			continue;
		}
		break;
	}
	swp = widthp;
/* +++ WL001 +++ */
	/* 
	 * A space is inserted when a newline has just been read and 
	 *  1. the word buffer is empty, and
	 *  2. the previous character is not an Asian character, or
	 *  3. the coming is not an Asian character.
	 */
	if ((achar_flag & RD_NEWLINE) && (wordp == word) &&
	    (!(achar_flag & RD_ACHAR) || !IS_ENCODED_ACHAR(j)))
		storeword(' ' | chbits, -1) ;
/* --- WL001 --- */
	if(spflg){
		storeword(' ' | chbits, -1);
		spflg = 0;
	}
	widthp = swp;
g0:
	if(j == CONT){
		pendw = wordp;
		nflush = 0;
		flushi();
		return(1);
	}
	if(hyoff != 1){
		if(j == ohc){
			hyoff = 2;
			*hyp++ = wordp;
			if(hyp > (hyptr+NHYP-1))hyp = hyptr+NHYP-1;
			goto g1;
		}
		if((j == '-') ||
		   (j == 0203) /*3/4 Em dash*/
		  )if(wordp > word+1){
			hyoff = 2;
			*hyp++ = wordp + 1;
			if(hyp > (hyptr+NHYP-1))hyp = hyptr+NHYP-1;
		}
	}
	storeword(i,width(i));	/* XXX */
/* +++ WL001 +++ */
	if (mb_lang && IS_ENCODED_ACHAR(i)) {
		achar_flag = RD_ACHAR  ; /* An Asian character has been read */
	} else if (i & MOT) {
		achar_flag = RD_MOTION ; /* A motion character has been read */
	} else
		achar_flag = 0 ;
/* --- WL001 --- */
g1:
	j = (i = GETCH()) & CMASK;
/* +++ WL001 +++ */
#define	is_printable(c)	(((c) < 0xff) && ((c) != 0x20))

	if (j == ASIAN_NL_PAD) {
		register int prev_ch = *(wordp - 1) & CMASK ;

		/* Get character again until a non-pad character is found */
		while ((j = (i = GETCH()) & CMASK) == ASIAN_NL_PAD) ;
		if (!Is_MByte_Space(j)    &&
		  ((is_printable    (prev_ch) && is_printable    (j)) ||
		   (is_printable    (prev_ch) && IS_ENCODED_ACHAR(j)) ||
		   (IS_ENCODED_ACHAR(prev_ch) && is_printable    (j)))) {
		        /* We need to pad a space in between */
			Push_To_Stack(i) 	    ;
			Push_To_Stack(' ' | chbits) ;
			goto found_newline	    ;
		}
	}

	if ((achar_flag == RD_ACHAR) && !(i & MOT)) {
	/* Return if previous char is an Asian character & the following */
	/* one is not a motion character.				 */
		Push_To_Stack(i) ;  /* Save char in stack */
		goto a_rtn 	 ;
	}
	if ((j != ' ') && !Is_MByte_Space(j) && (j != ASIAN_NL_PAD)) {
		if (j != '\n') {
			if (IS_ENCODED_ACHAR(j) && (achar_flag != RD_MOTION)) {
			/* Return if previous char not a motion char */
				Push_To_Stack(i) ;
				goto a_rtn 	 ;
			} else
			/* Get more character */
				goto g0 ;
		}
		Push_To_Stack(ASIAN_NL_PAD) ; /* Record newline state */
found_newline:
/* --- WL001 --- */
		j = *(wordp-1) & CMASK;
		if((j == '.') ||
		   (j == '!') ||
		   (j == '?'))spflg++;
/* +++ WL001 +++ */
	} else {
		Push_To_Stack(i) ; /* Save character */
/* --- WL001 --- */
	}
/* +++ WL001 +++ */
a_rtn:
/* --- WL001 --- */
	*wordp = 0;
rtn:
	wdstart = 0;
	wordp = word;
	pendw = 0;
	*hyp++ = 0;
	setnel();
	return(noword);
}
storeword(c,w)
int c, w;
{

	if(wordp >= &word[WDSIZE - 1]){
		if(!over){
			prstrfl(catgets(catd, 1, 25, "Word overflow.\n"));
			over++;
			c = 0343;
			w = -1;
		goto s1;
		}
		return;
	}
s1:
	if(w == -1)w = width(c);
	wne += w;
	*wordp++ = c;
	wch++;
}
#ifdef NROFF
extern char trtab[];
gettch(){
	register int i, j;
/* +++ WL001 +++ */
	register int tmp 		;
	extern   int is_flushing_stack	;

	if ((i = Pop_From_Stack()) != 0)
		return (i) ;	/* Return the character from peek stack */
	else if (is_flushing_stack)
		return ('\n') ; /* Stack is empty now */
/* --- WL001 --- */
	if(!((i = getch()) & MOT) && (i & ulbit)){
		j = i&CMASK;
/* +++ WL001 +++ */
		if (mb_lang) {
			if (cu && Is_MByte_Space(tmp = Translate(j))) {
				/* A 2-byte space is converted into two '_' */
				i = ((i & ~ulbit) & ~CMASK) | '_';
				Push_To_Stack(i)		 ;
			} else if (cu && (tmp == ' '))
				i = ((i & ~ulbit)& ~CMASK) | '_';
		}
		else if(cu && (trtab[j] == ' '))
			i = ((i & ~ulbit)& ~CMASK) | '_';
/* --- WL001 --- */
		if(!cu && (j>32) && (j<0370) && !(*t.codetab[j-32] & 0200))
			i &= ~ulbit;
	}
	return(i);
}
#endif
