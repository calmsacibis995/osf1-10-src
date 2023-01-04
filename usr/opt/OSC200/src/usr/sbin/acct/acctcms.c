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
static char	*sccsid = "@(#)$RCSfile: acctcms.c,v $ $Revision: 4.2.2.2 $ (DEC) $Date: 1993/10/07 21:53:00 $";
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
#if !defined( lint) && !defined(_NOIDENT)

#endif

/*
 * COMPONENT_NAME: (CMDACCT) Command Accounting
 *
 * FUNCTIONS: ccmp, dofile, enter, fixjunk, kcmp, ncmp,outputa, outputc,
 *            pcmadd, pprint, print, prnt, squeeze, tccmp, tcmadd, tdofile,
 *            tenter, tfixjunk, tkcmp, tncmp, totprnt, toutpta, toutptc,
 *            tprint, tsqueeze
 *
 * ORIGINS: 3,9,27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 * OBJECT CODE ONLY SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 *	acctcms [-a] [-c] [-j] [-n] [-s] [-p] [-o] [-t] [file...]
 *	summarize per-process accounting
 *	-a	output in ascii, rather than [pt]cms.h format
 *	-c	sort by total cpu, rather than total kcore-minutes
 *	-j	anything used only once -> ***other
 *	-n	sort by number of processes
 *	-s	any following files already in pcms.h format
 *      -p      output prime time command summary (only with -a)
 *      -o      output non-prime time (offshift) command summary (only
 *		with -a option)
 *	-t	process records in total (old) style (tcms.h) format
 *	file	file in [pt]cms.h (if -s seen already) or acct.h (if not)
 *	expected use:
 *	acctcms /var/adm/pacct? > today; acctcms -s old today >new
 *	cp new old; rm new
 *	acctcms -a today; acctcms -a old
 */
#include <sys/types.h>
#include "acctdef.h"
#include <stdio.h>
#include <ctype.h>
#include <sys/acct.h> 


#define MYKIND(flag)	((flag & ACCTF) == 0)
#define TOTAL(a)        (a[PRIMETM] + a[NONPRIME])

#include <locale.h>
#include "acct_msg.h"
nl_catd catd;
#define MSGSTR(Num, Str) catgets(catd, MS_ACCT, Num, Str)

#define CSIZE 1000
int	csize;

#define CMDSIZE  (sizeof(((struct acct *)0)->ac_comm))

/*
 *  Total cms records format
 */
struct tcms {
	char	tcm_comm[CMDSIZE];	/* command name */
	long	tcm_pc;		/* number of processes */
	double	tcm_cpu;	/* cpu time(min) */
	double	tcm_real;	/* real time(min) */
	double	tcm_kcore;	/* kcore-minutes */
	double	tcm_io;		/* chars transfered */
	double	tcm_rw;		/* blocks read */
} ;
struct tcms	*tcm;
/*
 * prime/nonprime CMS record format
 */
struct pcms {
	char	pcm_comm[CMDSIZE];	/* command name */
	long	pcm_pc[2];	/* number of processes */
	double	pcm_cpu[2];	/* cpu time(min) */
	double	pcm_real[2];	/* real time(min) */
	double	pcm_kcore[2];	/* kcore-minutes */
	double	pcm_io[2];	/* chars transfered */
	double	pcm_rw[2];	/* blocks read */
} ;
struct pcms	*pcm;
#define PRIMETM         0
#define NONPRIME	1
struct	tcms	tcmtmp	= {"***other"};
struct	pcms	pcmtmp	= {"***other"};
int	aflg;
int	cflg;
int	jflg;
int	nflg;
int	sflg;
int	pflg;
int	oflg;
int	tflg;
int	errflg;

static int	ccmp(), kcmp(), ncmp();
static int	tccmp(), tkcmp(), tncmp();
static int	tdofile(), dofile();
static int	tenter(), enter();
static int	tfixjunk(), fixjunk();
static int	tcmadd(), pcmadd();
static int	tsqueeze(), squeeze();
static int 	toutpta(), toutptc();
static int	outputa(), outputc();
static int	print(), pprint(), prnt(), tprint(), totprnt();

/*  Format specification for ASCII printing */

char	*fmtcmd =	"%-8.8s",
	*fmtcnt =	" %7ld",
	*fmtkcore =	" %11.2f",
	*fmtcpu =	" %9.2f",
	*fmtreal =	" %12.2f",
	*fmtmsz =	" %7.2f",
	*fmtmcpu =	" %7.2f",
	*fmthog =	" %7.2f",
	*fmtcharx =	" %12.0f",
	*fmtblkx =	" %10.0f" ;

main(int argc, char **argv)
{
	int	c;
	extern	int	optind;
	extern	char	*optarg;
	extern char *calloc();
	extern void exit();

	(void) setlocale (LC_ALL,"");
	catd = catopen(MF_ACCT,NL_CAT_LOCALE);

	while((c = getopt(argc, argv, "acjnspot")) != EOF)
	switch(c) {
		case 'a':
			aflg++;
			continue;
		case 'c':
			cflg++;
			continue;
		case 'j':
			jflg++;
			continue;
		case 'n':
			nflg++;
			continue;
		case 's':
			sflg++;
			continue;
		case 'p':
			pflg++;
			continue;
		case 'o':
			oflg++;
			continue;
		case 't':
			tflg++;
			continue;
		default:
			errflg++;
			continue;
	}
	if(errflg){
		(void)fprintf(stderr, 
			MSGSTR( USAGECMS, "Usage: %s [-acjnspot] [file ...]\n"),
			argv[0]);
		exit(1);
	}
	if(tflg) {
		if( (tcm = (struct tcms *)calloc(CSIZE, sizeof(struct tcms))) 
								== NULL) {
			(void)fprintf(stderr, 
				MSGSTR( NOMEM, "%s: Cannot allocate memory\n"),
				argv[0]);
			exit(5);
		}
		for(; optind < argc; optind++)
			tdofile(argv[optind]);
		if (jflg)
			tfixjunk();
		tsqueeze();
		qsort(tcm, csize, sizeof(tcm[0]), nflg? tncmp: (cflg? tccmp: tkcmp));
		if (aflg)
			toutpta();
		else
			toutptc();
	} else {
		if( (pcm = (struct pcms *)calloc(CSIZE, sizeof(struct pcms))) == NULL) {
			(void)fprintf(stderr, 
				MSGSTR( NOMEM, "%s: Cannot allocate memory\n"),
				argv[0]);
			exit(6);
		}
		for(; optind < argc; optind++)
			dofile(argv[optind]);
		if (jflg)
			fixjunk();
		squeeze();
		qsort(pcm, csize, sizeof(pcm[0]), nflg? ncmp: (cflg? ccmp: kcmp));
		if (aflg)
			outputa();
		else
			outputc();
	}

	return(0);
}

static
tdofile(fname)
char *fname;
{
	struct tcms cmt;
	struct acct ab;

	if (freopen(fname, "r", stdin) == NULL) {
		(void)fprintf(stderr, MSGSTR( CANTOPEN, "%s: Cannot open %s\n"), 
				"acctcms", fname);
		return;
	}
	if (sflg)
		while (fread(&cmt, sizeof(cmt), 1, stdin) == 1)
			(void)tenter(&cmt);
	else
		while (fread(&ab, sizeof(ab), 1, stdin) == 1) {
			(void)CPYN(cmt.tcm_comm, ab.ac_comm);
			cmt.tcm_pc = 1;
			cmt.tcm_cpu = MINS(expacct(ab.ac_stime)+expacct(ab.ac_utime));
			cmt.tcm_real = MINS(expacct(ab.ac_etime));
			cmt.tcm_kcore = KCORE(ab.ac_mem)*cmt.tcm_cpu;
			cmt.tcm_io = expacct(ab.ac_io);
			cmt.tcm_rw = expacct(ab.ac_rw);
			(void)tenter(&cmt);
		}
}

static
dofile(fname)
char *fname;
{
	struct acct ab;
	struct pcms pcmt;
	double	ratio;
	long	elaps[2];
	long	etime;
	double	dtmp;
	time_t	ltmp;

	if (freopen(fname, "r", stdin) == NULL) {
		(void)fprintf(stderr, MSGSTR( CANTOPEN, "%s: Cannot open %s\n"), 
			"acctcms", fname);
		return;
	}
	if (sflg)
		while (fread(&pcmt, sizeof(pcmt), 1, stdin) == 1)
			(void)enter(&pcmt);
	else
		while (fread(&ab, sizeof(ab), 1, stdin) == 1) {
			(void)CPYN(pcmt.pcm_comm, ab.ac_comm);
	/*
	 * Approximate P/NP split as same as elapsed time
 	 */
			if((etime = (long)expacct(ab.ac_etime)) == 0)
				etime = 1;
			pnpsplit(ab.ac_btime, etime, elaps);
			ratio = (double)elaps[PRIMETM]/(double)etime;
			if(elaps[PRIMETM] > elaps[NONPRIME]) {
				pcmt.pcm_pc[PRIMETM] = 1;
				pcmt.pcm_pc[NONPRIME] = 0;
			} else {
				pcmt.pcm_pc[PRIMETM] = 0;
				pcmt.pcm_pc[NONPRIME] = 1;
			}
			dtmp = MINS(expacct(ab.ac_stime)+expacct(ab.ac_utime));
			pcmt.pcm_cpu[PRIMETM] = dtmp * ratio;
			pcmt.pcm_cpu[NONPRIME] = (ratio == 1.0) ? 0.0 : (dtmp - pcmt.pcm_cpu[PRIMETM]);
			dtmp = MINS(expacct(ab.ac_etime));
			pcmt.pcm_real[PRIMETM] = dtmp * ratio;
			pcmt.pcm_real[NONPRIME] = (ratio == 1.0) ? 0.0 : (dtmp - pcmt.pcm_real[PRIMETM]);
			dtmp = KCORE(ab.ac_mem)*TOTAL(pcmt.pcm_cpu);
			pcmt.pcm_kcore[PRIMETM] = dtmp * ratio;
			pcmt.pcm_kcore[NONPRIME] = (ratio == 1.0) ? 0.0 : (dtmp - pcmt.pcm_kcore[PRIMETM]);
			ltmp = expacct(ab.ac_io);
			pcmt.pcm_io[PRIMETM] = (double)ltmp * ratio;
			pcmt.pcm_io[NONPRIME] = (ratio == 1.0) ? 0.0 : ((double)ltmp - pcmt.pcm_io[PRIMETM]);
			ltmp = expacct(ab.ac_rw);
			pcmt.pcm_rw[PRIMETM] = (double)ltmp * ratio;
			pcmt.pcm_rw[NONPRIME] = (ratio == 1.0) ? 0.0 : ((double)ltmp - pcmt.pcm_rw[PRIMETM]);
			(void)enter(&pcmt);
		}
}

static
tenter(p)
register struct tcms *p;
{
	register i;
	register j;
	for (i = j = 0; j < sizeof(p->tcm_comm); j++) {
		if (p->tcm_comm[j] && p->tcm_comm[j] <= 037)
			p->tcm_comm[j] = '?';
		i = i*7 + p->tcm_comm[j];	/* hash function */
	}
	if (i < 0)
		i = -i;
	for (i %= CSIZE, j = 0; tcm[i].tcm_comm[0] && j != CSIZE; i = (i+1)%CSIZE, j++)
		if (EQN(p->tcm_comm, tcm[i].tcm_comm))
			break;
	if(j == CSIZE) {
		(void)fprintf(stderr, MSGSTR( HASHOVR, 
			     "acctcms: Hash table overflow. Increase CSIZE\n"));
		return(-1);
	}
	if (tcm[i].tcm_comm[0] == 0)
		(void)CPYN(tcm[i].tcm_comm, p->tcm_comm);
	tcmadd(&tcm[i], p);
	return(i);
}

static
enter(p)
register struct pcms *p;
{
	register i;
	register j;
	for (i = j = 0; j < sizeof(p->pcm_comm); j++) {
		if (p->pcm_comm[j] && p->pcm_comm[j] <= 037)
			p->pcm_comm[j] = '?';
		i = i*7 + p->pcm_comm[j];	/* hash function */
	}
	if (i < 0)
		i = -i;
	for (i %= CSIZE, j = 0; pcm[i].pcm_comm[0] && j != CSIZE; i = (i+1)%CSIZE, j++)
		if (EQN(p->pcm_comm, pcm[i].pcm_comm))
			break;
	if(j == CSIZE) {
		(void)fprintf(stderr, MSGSTR( HASHOVR, 
			     "acctcms: Hash table overflow. Increase CSIZE\n"));
		return(-1);
	}
	if (pcm[i].pcm_comm[0] == 0)
		(void)CPYN(pcm[i].pcm_comm, p->pcm_comm);
	pcmadd(&pcm[i], p);
	return(i);
}

static
tfixjunk()	/* combine commands used only once */
{
	register i, j;
	j = enter(&tcmtmp);
	for (i = 0; i < CSIZE; i++)
		if (i != j && tcm[i].tcm_comm[0] && tcm[i].tcm_pc <= 1) {
			tcmadd(&tcm[j], &tcm[i]);
			tcm[i].tcm_comm[0] = 0;
		}
}

static
fixjunk()	/* combine commands used only once */
{
	register i, j;
	j = enter(&pcmtmp);
	for (i = 0; i < CSIZE; i++)
		if (i != j && pcm[i].pcm_comm[0] && (pcm[i].pcm_pc[PRIMETM] + pcm[i].pcm_pc[NONPRIME]) <= 1) {
			pcmadd(&pcm[j], &pcm[i]);
			pcm[i].pcm_comm[0] = 0;
		}
}


static
tcmadd(p1, p2)
register struct tcms *p1, *p2;
{
	p1->tcm_pc += p2->tcm_pc;
	p1->tcm_cpu = p1->tcm_cpu + p2->tcm_cpu;
	p1->tcm_real = p1->tcm_real + p2->tcm_real;
	p1->tcm_kcore = p1->tcm_kcore + p2->tcm_kcore;
	p1->tcm_io += p2->tcm_io;
	p1->tcm_rw += p2->tcm_rw;
}

static
pcmadd(p1, p2)
register struct pcms *p1, *p2;
{
	p1->pcm_pc[PRIMETM] += p2->pcm_pc[PRIMETM];
	p1->pcm_pc[NONPRIME] += p2->pcm_pc[NONPRIME];
	p1->pcm_cpu[PRIMETM] += p2->pcm_cpu[PRIMETM];
	p1->pcm_cpu[NONPRIME] += p2->pcm_cpu[NONPRIME];
	p1->pcm_real[PRIMETM] += p2->pcm_real[PRIMETM];
	p1->pcm_real[NONPRIME] += p2->pcm_real[NONPRIME];
	p1->pcm_kcore[PRIMETM] += p2->pcm_kcore[PRIMETM];
	p1->pcm_kcore[NONPRIME] += p2->pcm_kcore[NONPRIME];
	p1->pcm_io[PRIMETM] += p2->pcm_io[PRIMETM];
	p1->pcm_io[NONPRIME] += p2->pcm_io[NONPRIME];
	p1->pcm_rw[PRIMETM] += p2->pcm_rw[PRIMETM];
	p1->pcm_rw[NONPRIME] += p2->pcm_rw[NONPRIME];
}


static
tsqueeze()	/* get rid of holes in hash table */
{
	register i, k;

	for (i = k = 0; i < CSIZE; i++)
		if (tcm[i].tcm_comm[0]) {
			(void)CPYN(tcm[k].tcm_comm, tcm[i].tcm_comm);
			tcm[k].tcm_pc = tcm[i].tcm_pc;
			tcm[k].tcm_cpu = tcm[i].tcm_cpu;
			tcm[k].tcm_real = tcm[i].tcm_real;
			tcm[k].tcm_kcore = tcm[i].tcm_kcore;
			tcm[k].tcm_io = tcm[i].tcm_io;
			tcm[k].tcm_rw = tcm[i].tcm_rw;
			k++;
		}
	csize = k;
}

static
squeeze()	/* get rid of holes in hash table */
{
	register i, k;

	for (i = k = 0; i < CSIZE; i++)
		if (pcm[i].pcm_comm[0]) {
			(void)CPYN(pcm[k].pcm_comm, pcm[i].pcm_comm);
			pcm[k].pcm_pc[PRIMETM] = pcm[i].pcm_pc[PRIMETM];
			pcm[k].pcm_pc[NONPRIME] = pcm[i].pcm_pc[NONPRIME];
			pcm[k].pcm_cpu[PRIMETM] = pcm[i].pcm_cpu[PRIMETM];
			pcm[k].pcm_cpu[NONPRIME] = pcm[i].pcm_cpu[NONPRIME];
			pcm[k].pcm_real[PRIMETM] = pcm[i].pcm_real[PRIMETM];
			pcm[k].pcm_real[NONPRIME] = pcm[i].pcm_real[NONPRIME];
			pcm[k].pcm_kcore[PRIMETM] = pcm[i].pcm_kcore[PRIMETM];
			pcm[k].pcm_kcore[NONPRIME] = pcm[i].pcm_kcore[NONPRIME];
			pcm[k].pcm_io[PRIMETM] = pcm[i].pcm_io[PRIMETM];
			pcm[k].pcm_io[NONPRIME] = pcm[i].pcm_io[NONPRIME];
			pcm[k].pcm_rw[PRIMETM] = pcm[i].pcm_rw[PRIMETM];
			pcm[k].pcm_rw[NONPRIME] = pcm[i].pcm_rw[NONPRIME];
			k++;
		}
	csize = k;
}


static
tccmp(p1, p2)
register struct tcms *p1, *p2;
{
	if (p1->tcm_cpu == p2->tcm_cpu)
		return(0);
	return ((p2->tcm_cpu > p1->tcm_cpu)? 1 : -1);
}


static
ccmp(p1, p2)
register struct pcms *p1, *p2;
{
	register int	index;

	if( (pflg && oflg) || (!pflg && !oflg) ) {
		if (p1->pcm_cpu[PRIMETM] + p1->pcm_cpu[NONPRIME] == p2->pcm_cpu[PRIMETM] + p2->pcm_cpu[NONPRIME])
			return(0);
		return ((p2->pcm_cpu[PRIMETM] + p2->pcm_cpu[NONPRIME] > p1->pcm_cpu[PRIMETM] + p1->pcm_cpu[NONPRIME])? 1 : -1);
	}
	index = pflg ? PRIMETM : NONPRIME;
	if (p1->pcm_cpu[index] == p2->pcm_cpu[index])
		return(0);
	return ((p2->pcm_cpu[index] > p1->pcm_cpu[index])? 1 : -1);
}


static
tkcmp(p1, p2)
register struct tcms *p1, *p2;
{
	if (p1->tcm_kcore == p2->tcm_kcore)
		return(0);
	return ((p2->tcm_kcore > p1->tcm_kcore)? 1 : -1);
}


static
kcmp(p1, p2)
register struct pcms *p1, *p2;
{
	register int	index;

	if( (pflg && oflg) || (!pflg && !oflg) ){
		if (p1->pcm_kcore[PRIMETM] + p1->pcm_kcore[NONPRIME] == p2->pcm_kcore[PRIMETM] + p2->pcm_kcore[NONPRIME])
			return(0);
		return ((p2->pcm_kcore[PRIMETM] + p2->pcm_kcore[NONPRIME] > p1->pcm_kcore[PRIMETM] + p1->pcm_kcore[NONPRIME])? 1 : -1);
	}
	index = pflg ? PRIMETM : NONPRIME;
	if (p1->pcm_kcore[index] == p2->pcm_kcore[index])
		return(0);
	return ((p2->pcm_kcore[index] > p1->pcm_kcore[index])? 1 : -1);
}


static
tncmp(p1, p2)
register struct tcms *p1, *p2;
{
	if (p1->tcm_pc == p2->tcm_pc)
		return(0);
	return ((p2->tcm_pc > p1->tcm_pc)? 1 : -1);
}


static
ncmp(p1, p2)
register struct pcms *p1, *p2;
{
	register int	index;

	if( (pflg && oflg) || (!pflg && !oflg) ) {
		if (p1->pcm_pc[PRIMETM] + p1->pcm_pc[NONPRIME] == p2->pcm_pc[PRIMETM] + p2->pcm_pc[NONPRIME])
			return(0);
		return ((p2->pcm_pc[PRIMETM] + p2->pcm_pc[NONPRIME] > p1->pcm_pc[PRIMETM] + p1->pcm_pc[NONPRIME])? 1 : -1);
	}
	index =  pflg ? PRIMETM : NONPRIME;
	if (p1->pcm_pc[index] == p2->pcm_pc[index])
		return(0);
	return ((p2->pcm_pc[index] > p1->pcm_pc[index])? 1 : -1);
}

char	thd1[] =
"COMMAND   NUMBER      TOTAL       TOTAL       TOTAL   MEAN     MEAN     HOG      CHARS        BLOCKS\n";
char	thd2[] =
"NAME        CMDS    KCOREMIN     CPU-MIN     REAL-MIN SIZE-K  CPU-MIN  FACTOR   TRNSFD         READ\n";

static
toutpta()
{
	register i;

	(void)fprintf(stdout, MSGSTR( THEADER1, thd1));
	(void)fprintf(stdout, MSGSTR( THEADER2, thd2));
	(void)printf("\n");
	for (i = 0; i < csize; i++)
		tcmadd(&tcmtmp, &tcm[i]);
	(void)CPYN(tcmtmp.tcm_comm, MSGSTR( TOTALS, "TOTALS"));
	tprint(&tcmtmp);
	(void)printf("\n");
	for (i = 0; i < csize; i++)
		tprint(&tcm[i]);
}

char tpr_f1[] = "%-8.8s %7ld %11.2f %10.2f %12.2f %6.2f %7.2f ";
char tpr_f2[] = "%7.2f %11ld %11ld\n";
 

static
tprint(p)
register struct tcms *p;
{
	long pc;
	double cpu, real;

	cpu  = (p->tcm_cpu <= PRECISION) ? PRECISION : p->tcm_cpu;
	real = (p->tcm_real <= PRECISION) ? PRECISION : p->tcm_real;
	pc   = (p->tcm_pc == 0) ? 1 : p->tcm_pc;

	(void)fprintf(stdout, MSGSTR(TPR_OUT1, tpr_f1),
		p->tcm_comm, p->tcm_pc, p->tcm_kcore, p->tcm_cpu, p->tcm_real,
		p->tcm_kcore/cpu, p->tcm_cpu/pc);
	(void)fprintf(stdout, MSGSTR(TPR_OUT2, tpr_f2), p->tcm_cpu/real,
		p->tcm_io, p->tcm_rw);
}


static
toutptc()
{
	register i;

	for (i = 0; i < csize; i++)
		(void)fwrite(&tcm[i], sizeof(tcm[i]), 1, stdout);
}

char	hd1[] =
"COMMAND   NUMBER      TOTAL       TOTAL       TOTAL   MEAN    MEAN     HOG         CHARS     BLOCKS\n";
char	hd2[] =
"NAME        CMDS    KCOREMIN     CPU-MIN   REAL-MIN  SIZE-K  CPU-MIN  FACTOR      TRNSFD      READ\n";
char	hd3[] =
"COMMAND        NUMBER         TOTAL          CPU-MIN                 REAL-MIN        MEAN    MEAN      HOG       CHARS       BLOCKS\n";
char	hd4[] =
"NAME         (P)    (NP)   KCOREMIN       (P)      (NP)          (P)         (NP)  SIZE-K  CPU-MIN   FACTOR     TRNSFD        READ\n";
char	hdprime[] =
"                                   PRIME TIME COMMAND SUMMARY\n";
char	hdnonprime[] =
"                                  NON-PRIME TIME COMMAND SUMMARY\n";
char	hdtot[] =
"                                     TOTAL COMMAND SUMMARY\n";
char	hdp[] =
"                                PRIME/NON-PRIME TIME COMMAND SUMMARY\n";


static
outputa()
{
	register i;

	if( pflg && oflg ) (void)printf( MSGSTR( HDP, hdp)); 
	else if(pflg) (void)printf( MSGSTR( HDPRIME, hdprime)); 
	else if(oflg) (void)printf( MSGSTR( HDNONPRIME, hdnonprime)); 
	else (void)printf( MSGSTR( HDTOT, hdtot)); 
	if( (!pflg && !oflg) || (pflg ^ oflg)) {
		(void)printf( MSGSTR( HD1, hd1)); 
		(void)printf( MSGSTR( HD2, hd2)); 
	}
	else {
		(void)printf( MSGSTR( HD3, hd3)); 
		(void)printf( MSGSTR( HD4, hd4)); 
	}
	(void)printf("\n");
	for (i = 0; i < csize; i++)
		pcmadd(&pcmtmp, &pcm[i]);
	(void)CPYN(pcmtmp.pcm_comm, MSGSTR( TOTALS, "TOTALS"));
	(void)print(&pcmtmp);
	(void)printf("\n");
	for (i = 0; i < csize; i++)
		print(&pcm[i]);
}


static
print(p)
register struct pcms *p;
{
	if(pflg && oflg) pprint(p);
	else if(pflg || oflg) prnt(p, pflg ? PRIMETM : NONPRIME);
	else totprnt(p);
}


static
prnt(p, hr)
register struct pcms *p;
register int	hr;
{
	if(p->pcm_pc[hr] == 0) return;
	(void)printf(fmtcmd, p->pcm_comm);
	(void)printf(fmtcnt, p->pcm_pc[hr]);
	(void)printf(fmtkcore, p->pcm_kcore[hr]);
	(void)printf(fmtcpu, p->pcm_cpu[hr]);
	(void)printf(fmtreal, p->pcm_real[hr]);
	if(p->pcm_cpu[hr] <= PRECISION)  p->pcm_cpu[hr] = PRECISION;
	(void)printf(fmtmsz, p->pcm_kcore[hr]/p->pcm_cpu[hr]);
	if(p->pcm_pc[hr] == 0)  p->pcm_pc[hr] = 1;
	(void)printf(fmtmcpu, p->pcm_cpu[hr]/p->pcm_pc[hr]);
	if (p->pcm_real[hr] <= PRECISION)
		p->pcm_real[hr] = PRECISION;
	(void)printf(fmthog, p->pcm_cpu[hr]/p->pcm_real[hr]);
	(void)printf(fmtcharx,p->pcm_io[hr]);
	(void)printf(fmtblkx,p->pcm_rw[hr]);
	(void)printf("\n");
}


static
pprint(p)
register struct pcms *p;
{
	(void)printf(fmtcmd, p->pcm_comm);
	(void)printf(fmtcnt, p->pcm_pc[PRIMETM]);
	(void)printf(fmtcnt, p->pcm_pc[NONPRIME]);
	(void)printf(fmtkcore, TOTAL(p->pcm_kcore));
	(void)printf(fmtcpu, p->pcm_cpu[PRIMETM]);
	(void)printf(fmtcpu, p->pcm_cpu[NONPRIME]);
	(void)printf(fmtreal, p->pcm_real[PRIMETM]);
	(void)printf(fmtreal, p->pcm_real[NONPRIME]);
	if(TOTAL(p->pcm_cpu) <= PRECISION)  p->pcm_cpu[PRIMETM] = PRECISION;
	(void)printf(fmtmsz, TOTAL(p->pcm_kcore)/TOTAL(p->pcm_cpu));
	if(TOTAL(p->pcm_pc) == 0)  p->pcm_pc[PRIMETM] = 1;
	(void)printf(fmtmcpu, TOTAL(p->pcm_cpu)/TOTAL(p->pcm_pc));
	if ( TOTAL(p->pcm_real) <= PRECISION)
		p->pcm_real[PRIMETM] = PRECISION;
	(void)printf(fmthog, TOTAL(p->pcm_cpu)/TOTAL(p->pcm_real));
	(void)printf(fmtcharx,TOTAL(p->pcm_io));
	(void)printf(fmtblkx, TOTAL(p->pcm_rw));
	(void)printf("\n");
}


static
totprnt(p)
register struct pcms *p;
{
	(void)printf(fmtcmd, p->pcm_comm);
	(void)printf(fmtcnt, TOTAL(p->pcm_pc));
	(void)printf(fmtkcore, TOTAL(p->pcm_kcore));
	(void)printf(fmtcpu, TOTAL(p->pcm_cpu));
	(void)printf(fmtreal, TOTAL(p->pcm_real));
	if(TOTAL(p->pcm_cpu) <= PRECISION)  p->pcm_cpu[PRIMETM] = PRECISION;
	(void)printf(fmtmsz, TOTAL(p->pcm_kcore)/TOTAL(p->pcm_cpu));
	if(TOTAL(p->pcm_pc) == 0)  p->pcm_pc[PRIMETM] = 1;
	(void)printf(fmtmcpu, TOTAL(p->pcm_cpu)/TOTAL(p->pcm_pc));
	if (TOTAL(p->pcm_real) <= PRECISION)
		p->pcm_real[PRIMETM] = PRECISION;
	(void)printf(fmthog, TOTAL(p->pcm_cpu)/TOTAL(p->pcm_real));
	(void)printf(fmtcharx,TOTAL(p->pcm_io));
	(void)printf(fmtblkx,TOTAL(p->pcm_rw));
	(void)printf("\n");
}

static
outputc()
{
	register i;

	for (i = 0; i < csize; i++)
		(void)fwrite(&pcm[i], sizeof(pcm[i]), 1, stdout);
}
