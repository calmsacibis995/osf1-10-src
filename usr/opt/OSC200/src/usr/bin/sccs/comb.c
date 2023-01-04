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
static char	*sccsid = "@(#)$RCSfile: comb.c,v $ $Revision: 4.2.3.2 $ (DEC) $Date: 1993/10/11 19:06:41 $";
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
/*
 * comb.c
 *
 *	Revision History:
 *
 * 12-Mar-91	Fred Canter
 *	MIPS C 2.20+
 *
 */
#if !defined(lint) && !defined(_NOIDENT)

#endif
/*
 * COMPONENT_NAME: CMDSCCS      Source Code Control System (sccs)
 *
 * FUNCTIONS: clean_up, comb, enter, escdodelt, fredck, getpred,
 *            main, prtget
 *
 * ORIGINS: 3, 10, 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 * OBJECT CODE ONLY SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * comb.c 1.4 com/cmd/sccs/cmd,3.1,9021 9/15/89 13:34:30";
 */

#include  <locale.h>
#include  "defines.h"
#include  "had.h"
#include  "comb_msg.h"

#define MSGSTR(Num, Str) catgets(catd, MS_COMB, Num, Str)

struct stat Statbuf;
char Error[128];

struct packet gpkt;
struct sid sid;
int	num_files;
int	Do_prs;
char	had[26];
char	*clist;
char	*Val_ptr;
char	Blank[]    =    " ";
int	*Cvec;
int	Cnt;
FILE	*iop;
char    *fmalloc();

nl_catd catd;

main(argc,argv)
int argc;
register char *argv[];
{
	register int i;
	register char *p;
	char c;
	char *sid_ab();
	int testmore;
	extern comb();
	extern int Fcnt;

	(void) setlocale(LC_ALL,"");
	catd = catopen(MF_SCCS,NL_CAT_LOCALE);

	Fflags = FTLEXIT | FTLMSG | FTLCLN;
	for(i = 1; i < argc; i++)
		if(argv[i][0] == '-' && (c=argv[i][1])) {
			p = &argv[i][2];
			testmore = 0;
			switch (c) {

			case 'p':
				if (!p[0]) {
					argv[i] = 0;
					continue;
				}
				chksid(sid_ab(p,&sid),&sid);
				break;
			case 'c':
				clist = p;
				break;
			case 'o':
				testmore++;
				break;
			case 's':
				testmore++;
				break;
			default:
				sprintf(Error,MSGSTR(UNKKEYLTR, 
				    "\nFlag -%c is not valid.(cm1)\n"),c);  /* MSG */
				fatal(Error);
			}

			if (testmore) {
				testmore = 0;
				if (*p) {
				        sprintf(Error,MSGSTR(VALAFTKEY,"\nDo not supply a value for the -%c flag.(cm8)\n"),c);  /* MSG */
					fatal(Error);
				}
			}
			if (had[c - 'a']++) {
				sprintf(Error,MSGSTR(KEYLTRTWC, 
				 "\nUse the -%c flag only once.(cm2)\n"),c);/* MSG */
				fatal(Error);
			}
			argv[i] = 0;
		}
		else num_files++;

	if(num_files == 0)
		fatal(MSGSTR(MISSFLNAM,"\nSpecify the file to process.(cm3)\n"));  /* MSG */
	if (HADP && HADC)
		fatal(MSGSTR(NOPANDC, "\nThe -p flag and the -c flag are mutually exclusive.(cb2)\n"));  /* MSG */
	setsig();
	Fflags &= ~FTLEXIT;
	Fflags |= FTLJMP;
	iop = stdout;
	for (i = 1; i < argc; i++)
		if (p=argv[i])
			do_file(p,comb);
	fclose(iop);
	exit(Fcnt ? 1 : 0);
}


comb(file)
char *file;
{
	register int i, n;
	register struct idel *rdp;
	struct idel *dodelt();
	char *p;
	char *auxf();
	char rarg[32], *sid_ba();
	int succnt;
	struct sid *sp, *prtget();
	extern char had_dir, had_standinp;
	extern char *Sflags[];
	struct stats stats;

	if (setjmp(Fjmp))
		return;
	sinit(&gpkt, file, 1);
	gpkt.p_verbose = -1;
	gpkt.p_stdout = stderr;
	if (gpkt.p_verbose && (num_files > 1 || had_dir || had_standinp))
		fprintf(gpkt.p_stdout,"\n%s:\n",gpkt.p_file);
	if (exists(auxf(gpkt.p_file, 'p')))
		fatal(MSGSTR(PFILEXSTS,  "\nCannot use the comb command while there is an outstanding\n\\tdelta to the file.\n \tUse the delta command or the unget command; then use the comb command. (cb1)\n"));  /* MSG */

	if (dodelt(&gpkt,&stats,(struct sid *) 0,0) == 0)
		fmterr(&gpkt);

	Cvec = (int *) fmalloc(n = ((maxser(&gpkt) + 1) * sizeof(*Cvec)));
	zero((char *)Cvec, n);
	Cnt = 0;

	if (HADP) {
		rdp = gpkt.p_idel;
		if (!(n = sidtoser(&sid, &gpkt)))
		        fatal(MSGCM(SIDNOEXIST, "\nThe SID you specified does not exist. \n\
\tUse the sact command to check the p-file for existing SID numbers. (cm20)\n"));  /* MSG */
		while (n <= maxser(&gpkt)) {
			if (rdp[n].i_sid.s_rel == 0 &&
			    rdp[n].i_sid.s_lev == 0 &&
			    rdp[n].i_sid.s_br == 0  &&
			    rdp[n].i_sid.s_seq == 0) {
				n++;
				continue;
			}
			Cvec[Cnt++] = n++;
		}
	}
	else if (HADC) {
		dolist(&gpkt, clist, 0);
	}
	else {
		rdp = gpkt.p_idel;
		for (i = 1; i <= maxser(&gpkt); i++) {
			succnt = 0;
			if (rdp[i].i_sid.s_rel == 0 &&
			    rdp[i].i_sid.s_lev == 0 &&
			    rdp[i].i_sid.s_br == 0  &&
			    rdp[i].i_sid.s_seq == 0)
				continue;
			for (n = i + 1; n <= maxser(&gpkt); n++)
				if (rdp[n].i_pred == i)
					succnt++;
			if (succnt != 1)
				Cvec[Cnt++] = i;
		}
	}
	finduser(&gpkt);
	doflags(&gpkt);
	fclose(gpkt.p_iop);
	gpkt.p_iop = 0;
	if (!Cnt)
		fatal(MSGSTR(NOTHTODO, "\nThe flags specified on the command line would create no changes\n\
\tto the SCCS file. (cb4)\n"));  /* MSG */
	rdp = gpkt.p_idel;
	Do_prs = 0;
	sp = prtget(rdp, Cvec[0], iop, gpkt.p_file);
	sid_ba(sp,rarg);
	if (!(Val_ptr = Sflags[VALFLAG - 'a']))
		Val_ptr = Blank;
	fprintf(iop, "admin -iCOMB$$ -r%s -fv%s -m '-yThis was COMBined' s.COMB$$\n", rarg,Val_ptr);
	Do_prs = 1;
	fprintf(iop, "rm -f COMB$$\n");
	for (i = 1; i < Cnt; i++) {
		n = getpred(rdp, Cvec, i);
		if (HADO)
			fprintf(iop, "get -s -r%d -g -e -t s.COMB$$\n",
				rdp[Cvec[i]].i_sid.s_rel);
		else
			fprintf(iop, "get -s -a%d -r%d -g -e s.COMB$$\n",
				n + 1, rdp[Cvec[i]].i_sid.s_rel);
		prtget(rdp, Cvec[i], iop, gpkt.p_file);
		fprintf(iop, "delta -s -m\"$b\" -y\"$a\" s.COMB$$\n");
	}
	fprintf(iop, "sed -n '/^%c%c$/,/^%c%c$/p' %s >comb$$\n",
		CTLCHAR, BUSERTXT, CTLCHAR, EUSERTXT, gpkt.p_file);
	fprintf(iop, "ed - comb$$ <<\\!\n");
	fprintf(iop, "1d\n");
	fprintf(iop, "$c\n");
	fprintf(iop, MSGSTR(DELTAMSG,"*** DELTA TABLE PRIOR TO COMBINE ***\n"));
	fprintf(iop, ".\n");
	fprintf(iop, "w\n");
	fprintf(iop, "q\n");
	fprintf(iop, "!\n");
	fprintf(iop, "prs -e %s >>comb$$\n", gpkt.p_file);
	fprintf(iop, "admin -tcomb$$ s.COMB$$\\\n");
	for (i = 0; i < NFLAGS; i++)
		if (p = Sflags[i])
			fprintf(iop, " -f%c%s\\\n", i + 'a', p);
	fprintf(iop, "\n");
	fprintf(iop, "sed -n '/^%c%c$/,/^%c%c$/p' %s >comb$$\n",
		CTLCHAR, BUSERNAM, CTLCHAR, EUSERNAM, gpkt.p_file);
	fprintf(iop, "ed - comb$$ <<\\!\n");
	fprintf(iop, "v/^%c/s/.*/ -a& \\\\/\n", CTLCHAR);
	fprintf(iop, "1c\n");
	fprintf(iop, "admin s.COMB$$\\\n");
	fprintf(iop, ".\n");
	fprintf(iop, "$c\n");
	fprintf(iop, "\n");
	fprintf(iop, ".\n");
	fprintf(iop, "w\n");
	fprintf(iop, "q\n");
	fprintf(iop, "!\n");
	fprintf(iop, ". comb$$\n");
	fprintf(iop, "rm comb$$\n");
	if (!HADS) {
		fprintf(iop, "rm -f %s\n", gpkt.p_file);
		fprintf(iop, "mv s.COMB$$ %s\n", gpkt.p_file);
		if (!Sflags[VALFLAG - 'a'])
			fprintf(iop, "admin -dv %s\n", gpkt.p_file);
	}
	else {
		fprintf(iop, "set `ls -st s.COMB$$ %s`\n",gpkt.p_file);
		fprintf(iop, "c=`expr 100 - 100 '*' $1 / $3`\n");
		fprintf(iop, "echo '%s\t' ${c}'%%\t' $1/$3\n", gpkt.p_file);
		fprintf(iop, "rm -f s.COMB$$\n");
	}
}


enter(pkt,ch,n,sidp)
struct packet *pkt;
char ch;
int n;
struct sid *sidp;
{
	Cvec[Cnt++] = n;
}


struct sid *
prtget(idp, ser, fptr, file)
struct idel *idp;
int ser;
FILE *fptr;
char *file;
{
	char buf[32], *sid_ba();
	struct sid *sp;

	sid_ba(sp = &idp[ser].i_sid, buf);
	fprintf(fptr, "get -s -k -r%s -p %s > COMB$$\n", buf, file);
	if (Do_prs) {
		fprintf(fptr, "a=`prs -r%s -d:C: %s`\n",buf,file);
		fprintf(fptr, "b=`prs -r%s -d:MR: %s`\n",buf,file);
	}
	return(sp);
}


getpred(idp, vec, i)
struct idel *idp;
int *vec;
int i;
{
	int ser, pred, acpred;

	ser = vec[i];
	while (--i) {
		pred = vec[i];
		for (acpred = idp[ser].i_pred; acpred; acpred = idp[acpred].i_pred)
			if (pred == acpred)
				break;
		if (pred == acpred)
			break;
	}
	return(i);
}


clean_up(n)
{
	ffreeall();
}


escdodelt()	/* dummy for dodelt() */
{
}
#ifdef CASSI

fredck() /*dummy for dodelt() */
{
}
#endif
