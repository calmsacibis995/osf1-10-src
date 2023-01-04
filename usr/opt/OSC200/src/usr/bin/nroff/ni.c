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
static char rcsid[] = "@(#)$RCSfile: ni.c,v $ $Revision: 1.1.4.3 $ (DEC) $Date: 1993/10/14 04:17:55 $";
#endif
/*
 * HISTORY
 */
/*
 * OSF/1 Release 1.0
 */
/*
 * ni.c	4.4 6/2/90
 *
 */

/*
 * Modification History:
 *
 * 001 21-May-91	gws
 *	this came from usr/local/nosupport/troff ni.c -r3.1
 *	cleaned up history comments; removed extraneous ifndef lint/endif
 *
 */

#include "tdef.h"
char obuf[OBUFSZ];
char *obufp = obuf;
/* +++ WL001 +++ */
#undef  PAIR
#define PAIR(ch1, ch2)  FORM_PAIR(ch1, ch2)

#if 0
extern PeekStack	peek ;	/* It is defined in na.c */
#endif
/* --- WL001 --- */
int r[NN] = {
	PAIR('%',0),
	PAIR('n','l'),
	PAIR('y','r'),
	PAIR('h','p'),
	PAIR('c','t'),
	PAIR('d','n'),
	PAIR('m','o'),
	PAIR('d','y'),
	PAIR('d','w'),
	PAIR('l','n'),
	PAIR('d','l'),
	PAIR('s','t'),
	PAIR('s','b'),
	PAIR('c','.')};
int pto = 10000;
int pfrom = 1;
int print = 1;
char nextf[NS] = "/usr/share/lib/tmac/tmac.";
int nfi = 25;
#ifdef NROFF
char termtab[NS] = "/usr/share/lib/term/tab";
int tti = 23;
#endif
#ifndef NROFF
int oldbits = -1;
#endif
int init = 1;
int fc = IMP;
int eschar = '\\';
int pl = 11*INCH;
int po = PO;
int dfact = 1;
int dfactd = 1;
int res = 1;
int smnt = 4;
int ascii = ASCII;
int ptid = PTID;
char ptname[] = "/dev/cat";
int lg = LG;
int pnlist[NPN] = {-1};
int *pnp = pnlist;
int npn = 1;
int npnflg = 1;
int xflg = 1;
int dpn = -1;
int totout = 1;
int ulfont = 1;
int ulbit = 1<<9;
int tabch = TAB;
int ldrch = LEADER;
int xxx;
extern caseds(), caseas(), casesp(), caseft(), caseps(), casevs(),
casenr(), caseif(), casepo(), casetl(), casetm(), casebp(), casech(),
casepn(), tbreak(), caseti(), casene(), casenf(), casece(), casefi(),
casein(), caseli(), casell(), casens(), casemk(), casert(), caseam(),
casede(), casedi(), caseda(), casewh(), casedt(), caseit(), caserm(),
casern(), casead(), casers(), casena(), casepl(), caseta(), casetr(),
caseul(), caselt(), casenx(), caseso(), caseig(), casetc(), casefc(),
caseec(), caseeo(), caselc(), caseev(), caserd(), caseab(), casefl(),
done(), casess(), casefp(), casecs(), casebd(), caselg(), casehc(),
casehy(), casenh(), casenm(), casenn(), casesv(), caseos(), casels(),
casecc(), casec2(), caseem(), caseaf(), casehw(), casemc(), casepm(),
casecu(), casepi(), caserr(), caseuf(), caseie(), caseel(), casepc(),
caseht();
/* specific case routines for Kanji */
extern caseki(), caseko(), casekl();
#ifndef NROFF
extern casefz();
#endif
extern casecf();
struct contab {
	int rq;
/*
	union {
 */
		int (*f)();
/*
		unsigned mx;
	}x;
 */
}contab[NM]= {
	PAIR('d','s'),caseds,
	PAIR('a','s'),caseas,
	PAIR('s','p'),casesp,
	PAIR('f','t'),caseft,
	PAIR('p','s'),caseps,
	PAIR('v','s'),casevs,
	PAIR('n','r'),casenr,
	PAIR('i','f'),caseif,
	PAIR('i','e'),caseie,
	PAIR('e','l'),caseel,
	PAIR('p','o'),casepo,
	PAIR('t','l'),casetl,
	PAIR('t','m'),casetm,
	PAIR('b','p'),casebp,
	PAIR('c','h'),casech,
	PAIR('p','n'),casepn,
	PAIR('b','r'),tbreak,
	PAIR('t','i'),caseti,
	PAIR('n','e'),casene,
	PAIR('n','f'),casenf,
	PAIR('c','e'),casece,
	PAIR('f','i'),casefi,
	PAIR('i','n'),casein,
	PAIR('l','i'),caseli,
	PAIR('l','l'),casell,
	PAIR('n','s'),casens,
	PAIR('m','k'),casemk,
	PAIR('r','t'),casert,
	PAIR('a','m'),caseam,
	PAIR('d','e'),casede,
	PAIR('d','i'),casedi,
	PAIR('d','a'),caseda,
	PAIR('w','h'),casewh,
	PAIR('d','t'),casedt,
	PAIR('i','t'),caseit,
	PAIR('r','m'),caserm,
	PAIR('r','r'),caserr,
	PAIR('r','n'),casern,
	PAIR('a','d'),casead,
	PAIR('r','s'),casers,
	PAIR('n','a'),casena,
	PAIR('p','l'),casepl,
	PAIR('t','a'),caseta,
	PAIR('t','r'),casetr,
	PAIR('u','l'),caseul,
	PAIR('c','u'),casecu,
	PAIR('l','t'),caselt,
	PAIR('n','x'),casenx,
	PAIR('s','o'),caseso,
	PAIR('i','g'),caseig,
	PAIR('t','c'),casetc,
	PAIR('f','c'),casefc,
	PAIR('e','c'),caseec,
	PAIR('e','o'),caseeo,
	PAIR('l','c'),caselc,
	PAIR('e','v'),caseev,
	PAIR('r','d'),caserd,
	PAIR('a','b'),caseab,
	PAIR('f','l'),casefl,
	PAIR('e','x'),done,
	PAIR('s','s'),casess,
	PAIR('f','p'),casefp,
	PAIR('c','s'),casecs,
	PAIR('b','d'),casebd,
	PAIR('l','g'),caselg,
	PAIR('h','c'),casehc,
	PAIR('h','y'),casehy,
	PAIR('n','h'),casenh,
	PAIR('n','m'),casenm,
	PAIR('n','n'),casenn,
	PAIR('s','v'),casesv,
	PAIR('o','s'),caseos,
	PAIR('l','s'),casels,
	PAIR('c','c'),casecc,
	PAIR('c','2'),casec2,
	PAIR('e','m'),caseem,
	PAIR('a','f'),caseaf,
	PAIR('h','w'),casehw,
	PAIR('m','c'),casemc,
	PAIR('p','m'),casepm,
#ifdef NROFF
	PAIR('p','i'),casepi,
#endif
	PAIR('u','f'),caseuf,
	PAIR('p','c'),casepc,
	PAIR('h','t'),caseht,
#ifndef NROFF
	PAIR('f','z'),casefz,
#endif
	PAIR('c', 'f'),casecf,
	PAIR('k', 'i'),caseki,
	PAIR('k', 'o'),caseko,
	PAIR('k', 'l'),casekl,
};

/*
troff environment block
*/

int block = 0;
int ics = ICS;
int ic = 0;
int icf = 0;
int chbits = 0;
int spbits = 0;
int nmbits = 0;
int apts = PS;
int apts1 = PS;
int pts = PS;
int pts1 = PS;
int font = FT;
int font1 = FT;
int sps = SPS;
int spacesz = SS;
int lss = VS;
int lss1 = VS;
int ls = 1;
int ls1 = 1;
int ll = LL;
int ll1 = LL;
int lt = LL;
int lt1 = LL;
int ad = 1;
int nms = 1;
int ndf = 1;
int fi = 1;
int cc = '.';
int c2 = '\'';
int ohc = OHC;
int tdelim = IMP;
int hyf = 1;
int hyoff = 0;
int un1 = -1;
int tabc = 0;
int dotc = '.';
int adsp = 0;
int adrem = 0;
int lastl = 0;
int nel = 0;
int admod = 0;
int *wordp = 0;
int spflg = 0;
int *linep = 0;
int *wdend = 0;
int *wdstart = 0;
int wne = 0;
int ne = 0;
int nc = 0;
int nb = 0;
int lnmod = 0;
int nwd = 0;
int nn = 0;
int ni = 0;
int ul = 0;
int cu = 0;
int ce = 0;
int in = 0;
int in1 = 0;
int un = 0;
int wch = 0;
int pendt = 0;
int *pendw = 0;
int pendnf = 0;
int spread = 0;
int it = 0;
int itmac = 0;
int lnsize = LNSIZE;
int *hyptr[NHYP] = {0};
int tabtab[NTAB] = {DTAB,DTAB*2,DTAB*3,DTAB*4,DTAB*5,DTAB*6,DTAB*7,DTAB*8,
	DTAB*9,DTAB*10,DTAB*11,DTAB*12,DTAB*13,DTAB*14,DTAB*15,0};
int line[LNSIZE] = {0};
int word[WDSIZE] = {0};
int blockxxx[EVS-68-NHYP-NTAB-WDSIZE-LNSIZE] = {0};
/*spare 5 words*/
int oline[LNSIZE+1];

#if 0
/* +++ WW001 +++ */
PeekStack ostack;
/* --- WW001 */
#endif
