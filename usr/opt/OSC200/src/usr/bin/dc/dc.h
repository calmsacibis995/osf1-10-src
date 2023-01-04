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
static char rcsid[] = "@(#)$RCSfile: dc.h,v $ $Revision: 4.2.5.2 $ (DEC) $Date: 1993/06/10 17:54:13 $";
 */
/*
 * HISTORY
 */
/*
 * OSF/1 1.2
 * COMPONENT_NAME: (CMDCALC) calculators
 *
 * FUNCTIONS: 
 *
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
 *
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 * 
 * 1.10  com/cmd/calc/dc.h, , bos320, 9134320c 8/20/91 16:01:56
 */

#define FATAL 0
#define NFATAL 1
#define BLK sizeof(struct blk)
#define PTRSZ sizeof(int *)
#define HEADSZ 1024
#define STKSZ 100
#define RDSKSZ 100
#define ARRAYST 0241
#define TBLSZ 256
#define MAXIND 2048
#define NL 1
#define NG 2
#define NE 3
#define length(p) ((p)->wt-(p)->beg)
#define rewind(p) (p)->rd=(p)->beg
#define create(p)	(p)->rd = (p)->wt = (p)->beg
#define fsfile(p)	(p)->rd = (p)->wt
#define truncate(p)	(p)->wt = (p)->rd
#define sfeof(p)	(((p)->rd==(p)->wt)?1:0)
#define sfbeg(p)	(((p)->rd==(p)->beg)?1:0)
#define sungetc(p,c)	*(--(p)->rd)=c
#define NEGBYTE 0200
#define MASK (-1 & ~0377)
#define sgetc(p)	( ((p)->rd==(p)->wt) ? EOF :( ((*(p)->rd & NEGBYTE) != 0) ? ( *(p)->rd++ | MASK): *(p)->rd++ ))
#define slookc(p)	( ((p)->rd==(p)->wt) ? EOF :( ((*(p)->rd & NEGBYTE) != 0) ? (*(p)->rd | MASK) : *(p)->rd ))
#define sbackc(p)	( ((p)->rd==(p)->beg) ? EOF :( ((*(--(p)->rd) & NEGBYTE) != 0) ? (*(p)->rd | MASK): *(p)->rd ))
#define sputc(p,c)	{if((p)->wt==(p)->last)more(p); *(p)->wt++ = c; }
#define salterc(p,c)	{if((p)->rd==(p)->last)more(p); *(p)->rd++ = c; if((p)->rd>(p)->wt)(p)->wt=(p)->rd;}
#define sunputc(p)	(*( (p)->rd = --(p)->wt))
#define zero(p)	for(pp=(p)->beg;pp<(p)->last;)*pp++='\0'
#define OUTC(x) {printf("%c",x); if(--count == 0){printf("\\\n"); count=ll;} }
#define TEST2	{if((count -= 2) <=0){printf("\\\n");count=ll;}}
#define EMPTY if(stkerr != 0){error(MSGSTR(EMPTSTK,"empty stack\n")) }
#define EMPTYR(x) if(stkerr!=0) {			\
				pushp(x);		\
				fprintf(stderr,MSGSTR(EMPTSTK,	\
						      "empty stack\n")); \
			        continue;}
#define EMPTYS if(stkerr != 0){fprintf(stderr,MSGSTR(EMPTSTK,"stack empty\n")); return(1);}
#define EMPTYSR(x) if(stkerr !=0){fprintf(stderr, MSGSTR(EMPTSTK,"stack empty\n"));pushp(x);return(1);}
#define error(p)	{fprintf(stderr,p); continue; }
#define errorrt(p)	{fprintf(stderr,p); return(1); }

struct blk {
	char	*rd;
	char	*wt;
	char	*beg;
	char	*last;
};
struct	blk *hfree;
struct	blk *getwd();
struct	blk *lookwd();
struct	blk *getdec();
struct	blk *morehd();

struct	blk *arg1, *arg2;
int	svargc;
int	savk;
char	**svargv;
int	dbg;
int	ifile;
FILE	*curfile;
struct	blk *scalptr, *basptr, *tenptr, *inbas;
struct	blk *sqtemp, *chptr, *strptr, *divxyz;
struct	blk *stack[STKSZ];
struct	blk **stkptr,**stkbeg;
struct	blk **stkend;
int	stkerr;
int	lastchar;
struct	blk *readstk[RDSKSZ];
struct	blk **readptr;
struct	blk *rem;
int	k;
struct	blk *irem;
int	skd,skr;
struct	blk *pop(),*readin(),*add0(),*mult();
struct	blk *scalint();
struct	blk *removc();
struct	blk *add(),*divn(),*removr();
struct	blk *exp();
struct	blk *sqrt_dc();
struct	blk *salloc(),*copy();
struct	blk *scale();
int	neg;
struct	sym {
	struct	sym *next;
	struct	blk *val;
} symlst[TBLSZ];
struct	sym *stable[TBLSZ];
struct	sym *sptr,*sfree;
struct	wblk {
	struct blk **rdw;
	struct blk **wtw;
	struct blk **begw;
	struct blk **lastw;
};
FILE	*fsave;
long	rel;
long	nbytes;
long	all;
long	headmor;
long	obase;
int	fw,fw1,ll;
int	(*outdit)();
int	bigot(),hexot();
int	logo;
int	log10;
int	count;
char	*pp;
int	onintr(void);
char	*nalloc();
char	*dummy;
