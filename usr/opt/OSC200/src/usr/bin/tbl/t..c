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
/*
static char rcsid[] = "@(#)$RCSfile: t..c,v $ $Revision: 4.2.6.2 $ (DEC) $Date: 1993/06/10 17:47:12 $";
*/

/* t..c : external declarations */

#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <locale.h>
#include <nl_types.h>

# define MAXLIN 200
# define MAXHEAD 100
# define MAXCOL 20
# define MAXCHS 2000
# define MAXRPT 100
# define CLLEN 10
# define SHORTLINE 4
extern int nlin, ncol, iline, nclin, nslin;
extern int style[MAXHEAD][MAXCOL];
extern int ctop[MAXHEAD][MAXCOL];
extern char font[MAXHEAD][MAXCOL][2];
extern char csize[MAXHEAD][MAXCOL][4];
extern char vsize[MAXHEAD][MAXCOL][4];
extern char cll[MAXCOL][CLLEN];
extern int stynum[];
extern int F1, F2;
extern int lefline[MAXHEAD][MAXCOL];
extern int fullbot[];
extern wchar_t *instead[];
extern int expflg;
extern int ctrflg;
extern int evenflg;
extern int evenup[];
extern int boxflg;
extern int dboxflg;
extern int linsize;
extern int tab;
extern int pr1403;
extern int linsize, delim1, delim2;
extern int allflg;
extern int textflg;
extern int left1flg;
extern int rightl;
struct colstr {
	wchar_t *col, *rcol;
};
extern struct colstr *table[];
extern wchar_t *cspace, *cstore;
extern wchar_t *exstore, *exlim;
extern int sep[];
extern int used[], lused[], rused[];
extern int linestop[];
extern wchar_t *leftover;
extern char *last, *ifile;
extern int texname;
extern int texct, texmax;
extern char texstr[];
extern int linstart;
extern wchar_t *chspace();


extern FILE *tabin, *tabout;
# define CRIGHT 80
# define CLEFT 40
# define CMID 60
# define S1 31
# define S2 32
# define TMP 38
# define SF 35
# define SL 34
# define LSIZE 33
# define SIND 37
# define SVS 36
/* this refers to the relative position of lines */
# define LEFT 1
# define RIGHT 2
# define THRU 3
# define TOP 1
# define BOT 2

#define CATFILE "tbl"
extern nl_catd catd;

#ifdef DEBUG
# define D(X) X
#else
# define D(X)
#endif
/*
 * HISTORY
 */
# define DX(X)

/* The following codes were extracted from csh.h. */
/*
 * In lines for frequently called functions
 */
#define XFREE(cp) { \
        extern char end[]; \
        char stack; \
        if ((cp) >= end && (cp) < &stack) \
                free(cp); \
}
char    *alloctmp;
#define xalloc(i) ((alloctmp = (char *)malloc(i)) ? alloctmp : (char *)nomem(i))

wchar_t *maknew();

char    **saveblk();
wchar_t **wsaveblk();
char    *savestr();
wchar_t *savewcs();
char    *strcat();
wchar_t *wcscat();
char    *strcpy();
wchar_t *wcscpy();
wchar_t *Towcs();
wchar_t **Towcsv();
char    *Tombs();
char    **Tombsv();

/*
 * Pointers to wide character string constants.
 */
extern wchar_t WNULLp[];
#define WNULL ((wchar_t *)0)
extern wchar_t *WCS_dTE;
extern wchar_t *WCS_dTC;
extern wchar_t *WCS_dTand;
extern wchar_t *WCS_Tlbrace;
extern wchar_t *WCS_Trbrace;
extern wchar_t *WCS_SPAN;

/*
 * Macros for i18n.
 */
#define towcs(mbs) Towcs("mbs")
#define tombs(wcs) Tombs("wcs")
#define wcsconst(mbs) WCS_mbs

