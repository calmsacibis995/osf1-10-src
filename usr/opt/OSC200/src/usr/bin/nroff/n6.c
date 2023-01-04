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
static char rcsid[] = "@(#)$RCSfile: n6.c,v $ $Revision: 4.2.4.3 $ (DEC) $Date: 1993/10/14 04:17:36 $";
#endif
/*
 * HISTORY
 */
/*
 * OSF/1 Release 1.0
 */
/*
 * n6.c	4.2 11/26/82
 *
 */

/*
 * Modification History:
 *
 * 001 21-May-91	gws
 *	this came from usr/local/nosupport/nroff n6.c -r3.1
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

/*
troff6.c

width functions, sizes and fonts
*/

extern  int     inchar[LNSIZE], *pinchar;       /* XXX */
extern int eschar;
extern int widthp;
extern int ohc;
extern int xfont;
extern int smnt;
extern int setwdf;
extern char trtab[];
extern int chbits;
extern int nonumb;
extern int noscale;
extern int font;
extern int font1;
extern int pts;
extern int sps;
extern int nlflg;
extern int nform;
extern int dfact;
extern int dfactd;
extern int lss;
extern int lss1;
extern int vflag;
extern int ch0;
extern int level;
extern int ch;
extern int res;
extern int xxx;
extern int mb_lang;
int fontlab[] = {'R','I','B','S',0};

width(c)
int c;
{
	register i,j,k;
	register mb=mb_lang;

	j = c;
	k = 0;
	if(j & MOT){
		if(j & VMOT)goto rtn;
		k = j & ~MOTV;
		if(j & NMOT)k = -k;
		goto rtn;
	}
	if((i = (j & CMASK)) == 010){
		k = -widthp;
		goto rtn;
	}
	if(i == PRESC)i = eschar;
	if((i == ohc) ||
/* +++ WL001 +++ */
	   (i == 0377))goto rtn;    /* or 0377 ? */
/* --- WL001 --- */
	if(j & ZBIT)goto rtn;
/* +++ WL001 +++ */
	if (mb) i = Translate(i) ;	/* Perform character translation */
	else i = (unsigned char)trtab[i] & BMASK;
/* --- WL001 --- */
	if(i < 040)goto rtn;
/* +++ WL001 +++ */
	if (mb && IS_ENCODED_ACHAR(i))
		k = Display_Width(i) * t.Char ;
	else
/* --- WL001 --- */
	if (t.codetab[i-32])
		k = (*(t.codetab[i-32]) & 0177) * t.Char;
	else
		k = 0;
	widthp = k;
rtn:
	return(k);
}
setch(){
	register i,*j,k;
	extern int chtab[];

	if((i = getrq()) == 0)return(0);
	for(j=chtab;*j != i;j++)if(*(j++) == 0)return(0);
	k = *(++j) | chbits;
	return(k);
}
find(i,j)
int i,j[];
{
	register k;

	if(((k = i-'0') >= 1) && (k <= 4) && (k != smnt))return(--k);
	for(k=0; j[k] != i; k++)if(j[k] == 0)return(-1);
	return(k);
}
mchbits(){
	chbits = (((pts)<<2) | font) << (BYTE + 1);
	sps = width(' ' | chbits);
}
setps(){
	register i,j;

	if((((i=getch() & CMASK) == '+')  || (i == '-')) &&
	  (((j=(ch = getch() & CMASK) - '0') >= 0) && (j <= 9))){
		ch = 0;
		return;
	}
	if((i -= '0') == 0){
		return;
	}
	if((i > 0) && (i <= 9)){
		if((i <= 3) &&
		  ((j=(ch = getch() & CMASK) - '0') >= 0) && (j <= 9)){
			i = 10*i +j;
			ch = 0;
		}
	}
}
caseft(){
	skip();
	setfont(1);
}
setfont(a)
int a;
{
	register i,j;

	if(a)i = getrq();
		else i = getsn();
	if(!i || (i == 'P')){
		j = font1;
		goto s0;
	}
	if(i == 'S')return;
	if((j = find(i,fontlab))  == -1)return;
s0:
	font1 = font;
	font = j;
	mchbits();
}
setwd(){
	register i, base, wid;
	int delim, em, k;
	int savlevel, savhp, savfont, savfont1;
	int *savpinchar, *p, *q, tempinchar[LNSIZE];	/* XXX */

	base = v.st = v.sb = wid = v.ct = 0;
	if((delim = getch() & CMASK) & MOT)return;
	savhp = v.hp;
	savpinchar = pinchar;	/* XXX */
	for (p=inchar, q=tempinchar; p < pinchar; )	/* XXX */
		*q++ = *p++;	/* XXX */
	pinchar = inchar;	/* XXX */
	savlevel = level;
	v.hp = level = 0;
	savfont = font;
	savfont1 = font1;
	setwdf++;
	while((((i = getch()) & CMASK) != delim) && !nlflg){
		wid += width(i);
		if(!(i & MOT)){
			em = 2*t.Halfline;
		}else if(i & VMOT){
			k = i & ~MOTV;
			if(i & NMOT)k = -k;
			base -= k;
			em = 0;
		}else continue;
		if(base < v.sb)v.sb = base;
		if((k=base + em) > v.st)v.st = k;
	}
	nform = 0;
	setn1(wid);
	v.hp = savhp;
	pinchar = savpinchar;	/* XXX */
	for (p=inchar, q=tempinchar; p < pinchar; )	/* XXX */
		*p++ = *q++;	/* XXX */
	level = savlevel;
	font = savfont;
	font1 = savfont1;
	mchbits();
	setwdf = 0;
}
vmot(){
	dfact = lss;
	vflag++;
	return(mot());
}
hmot(){
	dfact = EM;
	return(mot());
}
mot(){
	register i, j;

	j = HOR;
	getch(); /*eat delim*/
	if(i = atoi()){
		if(vflag)j = VERT;
		i = makem(quant(i,j));
	}
	getch();
	vflag = 0;
	dfact = 1;
	return(i);
}
sethl(k)
int k;
{
	register i;

	i = t.Halfline;
	if(k == 'u')i = -i;
	else if(k == 'r')i = -2*i;
	vflag++;
	i = makem(i);
	vflag = 0;
	return(i);
}
makem(i)
int i;
{
	register j;

	if((j = i) < 0)j = -j;
	j = (j & ~MOTV) | MOT;
	if(i < 0)j |= NMOT;
	if(vflag)j |= VMOT;
	return(j);
}
casefp(){
	register i, j;

	skip();
	if(((i = (getch() & CMASK) - '0' -1) < 0) || (i >3))return;
	if(skip() || !(j = getrq()))return;
	fontlab[i] = j;
}
casevs(){
	register i;

	skip();
	vflag++;
	dfact = INCH; /*default scaling is points!*/
	dfactd = 72;
	res = VERT;
	i = inumb(&lss);
	if(nonumb)i = lss1;
	if(i < VERT)i = VERT;
	lss1 = lss;
	lss = i;
}
xlss(){
	register i, j;

	getch();
	dfact = lss;
	i = quant(atoi(),VERT);
	dfact = 1;
	getch();
	if((j = i) < 0)j = -j;
	ch0 = ((j & 03700)<<3) | HX;
	if(i < 0)ch0 |= 040000;
	return(((j & 077)<<9) | LX);
}
casefz(){}
caseps(){}
caselg(){}
casecs(){}
casebd(){}
casess(){}
getlg(i)
int i;
{
	return(i);
}
