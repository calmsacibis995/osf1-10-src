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
static char rcsid[] = "@(#)$RCSfile: nii.c,v $ $Revision: 1.1.4.3 $ (DEC) $Date: 1993/10/14 04:17:59 $";
#endif
/*
 * HISTORY
 */
/*
 * OSF/1 Release 1.0
 */
/*
 * nii.c	4.1 6/7/82
 *
 */

/*
 * Modification History:
 *
 * 001 21-May-91	gws
 *	this came from usr/local/nosupport/troff nii.c -r3.1
 *	cleaned up history comments; removed extraneous ifndef lint/endif
 *
 */

#include "tdef.h"
#ifdef NROFF
#include "tw.h"
#endif
#include "sdef.h"
#include "d.h"
#include "v.h"
#include <sgtty.h>

int *vlist = (int *)&v;
struct s *frame, *stk, *ejl;
struct s *nxf, *litlev;

#ifdef NROFF
int pipeflg;
int hflg;
int eqflg;
#endif

#ifndef NROFF
int xpts;
int verm;
int *pslp;
int psflg;
int ppts;
int pfont;
int paper;
int mpts;
int mfont;
int mcase;
int escm;
int cs;
int code;
int ccs;
int bd;
int back;
#endif

int level;
int stdi;
int waitf;
int nofeed;
int quiet;
int stop;
/* +++ WW001 +++ */
nl_catd catd;
/* Add space to store incomplete Asian character */
char ibuf[IBUFSZ + MB_MAX];
char xbuf[IBUFSZ + MB_MAX];
/* --- WW001 --- */
char *ibufp;
char *xbufp;
char *eibuf;
char *xeibuf;
int cbuf[NC];
int *cp;
int nx;
int mflg;
int ch = 0;
int cps;
int ibf;
int ttyod;
struct sgttyb ttys;
int iflg;
char *enda;
int rargc;
char **argp;
char trtab[256];
int lgf;
int copyf;
int ch0;
int cwidth;
filep ip;
int nlflg;
long *ap;
int donef;
int nflush;
int nchar;
int rchar;
int nfo;
int ifile;
int padc;
int raw;
int ifl[NSO];
int ifi;
int flss;
int nonumb;
int trap;
int tflg;
int ejf;
int lit;
int gflag;
int dilev;
int tlss;
filep offset;
int em;
int ds;
filep woff;
int app;
int ndone;
int lead;
int ralss;
filep nextb;
long *argtop;
int nrbits;
int nform;
int oldmn;
int newmn;
int macerr;
filep apptr;
int diflg;
filep roff;
int wbfi;
int inc[NN];
int fmt[NN];
int evi;
int vflag;
int noscale;
int po1;
int nroff_nlist[NTRAP]; /* 001-kak */
int mlist[NTRAP];
int evlist[EVLSZ];
int ev;
int tty;
int sfont;
int sv;
int esc;
int widthp;
int xfont;
int setwdf;
int xbitf;
int over;
int nhyp;
int **hyp;
int *olinep;
int esct;
int ttysave = -1;
int dotT;
char *unlkp;
int no_out;
