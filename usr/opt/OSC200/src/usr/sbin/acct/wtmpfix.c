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
static char	*sccsid = "@(#)$RCSfile: wtmpfix.c,v $ $Revision: 4.2.3.2 $ (DEC) $Date: 1993/10/07 21:53:57 $";
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
 * wtmpfix.c
 *
 *	Modification History:
 *
 * 01-Apr-91	Fred Canter
 *	MIPS C 2.20+, changes for -std
 *
 */
#if !defined( lint) && !defined(_NOIDENT)

#endif

/*
 * COMPONENT_NAME: (CMDACCT) Command Accounting
 *
 * FUNCTIONS: adjust, err, intr, invalid, mkdtab, setdtab,
 *            winp, wout
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
 * wtmpfix - adjust wtmp file and remove date changes.
 *
 *	wtmpfix <wtmp1 >wtmp2
 *
 */

# include <stdio.h>
# include <varargs.h>
# include <ctype.h>
# include <sys/types.h>
# include "acctdef.h"
# include <sys/acct.h>
# include <signal.h>
# include <time.h>

#include <locale.h>
#include "acct_msg.h"
nl_catd catd;
#define MSGSTR(Num, Str) catgets(catd, MS_ACCT, Num, Str)

FILE	*Wtmp, *Opw;

char	Ofile[]	= "/tmp/wtmpfixXXXXXX";

struct	dtab
{
	long	d_off1;		/* file offset start */
	long	d_off2;		/* file offset stop */
	long	d_adj;		/* time adjustment */
	struct dtab *d_ndp;	/* next record */
};

struct	dtab	*Fdp;		/* list header */
struct	dtab	*Ldp;		/* list trailer */


long	ftell();
struct	utmp	wrec, wrec2;

char timbuf[BUFSIZ];
struct tm *localtime();

static void
intr()
{

	(void)signal(SIGINT,SIG_IGN);
	(void)unlink(Ofile);
	exit(1);
}

static int	err(), winp(), wout();
static int	mkdtab(), setdtab(), adjust(), invalid();

char inval[] = "wtmpfix: logname \"%*.*s\" changed to \"INVALID\"\n";

main(int argc, char **argv)
{

	void intr();
	static long	recno = 0;
	register struct dtab *dp;

	if(argc < 2){
		argv[argc] = "-";
		argc++;
	}

	if((int)signal(SIGINT,intr) == -1) {
		perror("signal");
		exit(1);
	}

	(void) setlocale (LC_ALL,"");
	catd = catopen(MF_ACCT,NL_CAT_LOCALE);

	(void)mktemp(Ofile);
	if((Opw=fopen(Ofile,"w"))==NULL)
		err( MSGSTR( CANTMKTMP, "cannot make temporary: %s"), Ofile);

	while(--argc > 0){
		argv++;
		if(strcmp(*argv,"-")==0)
			Wtmp = stdin;
		else if((Wtmp = fopen(*argv,"r"))==NULL)
			err(MSGSTR( CANTOPEN, "%s: Cannot open: %s"), 
					"wtmpfix", *argv);
		while(winp(Wtmp,&wrec)){
			if(recno == 0 || wrec.ut_type==BOOT_TIME){
				mkdtab(recno,&wrec);
			}
			if(invalid(wrec.ut_user)) {
				(void)fprintf(stderr,
					MSGSTR( LOGCHG, inval), NSZ, NSZ, wrec.ut_user);
				(void)strncpy(wrec.ut_user, "INVALID", NSZ);
			}
			if(wrec.ut_type==OLD_TIME){
				if(!winp(Wtmp,&wrec2))
					err(MSGSTR( INTRUC, "Input truncated at offset %ld"),recno);
				if(wrec2.ut_type!=NEW_TIME)
					err(MSGSTR( NEWDTEXPTED, "New date expected at offset %ld"),recno);
				setdtab(recno,&wrec,&wrec2);
				recno += (2 * sizeof(struct utmp));
				wout(Opw,&wrec);
				wout(Opw,&wrec2);
				continue;
			}
			wout(Opw,&wrec);
			recno += sizeof(struct utmp);
		}
		if(Wtmp!=stdin)
			(void)fclose(Wtmp);
	}
	(void)fclose(Opw);
	if((Opw=fopen(Ofile,"r"))==NULL)
		err(MSGSTR( CATREADTMP, "Cannot read from temp: %s"), Ofile);
	recno = 0;
	while(winp(Opw,&wrec)){
		adjust(recno,&wrec);
		recno += sizeof(struct utmp);
		if(wrec.ut_type==OLD_TIME || wrec.ut_type==NEW_TIME)
			continue;	
		wout(stdout,&wrec);
	}
	(void)fclose(Opw);
	(void)unlink(Ofile);
	return(0);
}

/*	err() writes an error message to the standard error and then
 *	calls the interrupt routine to clean up the temporary file
 *	and exit. Varable argument lists are now supported.
 */

/*VARARGS*/

static
err(va_alist) va_dcl
{
        va_list arglist;
        register char *fmt;

        va_start(arglist);
        fmt = va_arg(arglist, char *);
        (void)fflush(stdout);
        (void)vfprintf(stderr, fmt, arglist);
	(void)fprintf(stderr,"\n");
        (void)fflush(stderr);
        va_end(arglist);

	intr();
}

/*	winp() reads a record from a utmp.h-type file pointed to
 *	by the stream pointer "f" into the structure whose address
 *	is given by the variable "w".
 *	This reading takes place in two stages: first the raw
 *      records from the utmp.h files are read in (usually /var/adm/wtmp)
 *	and written into the temporary file (Opw); then the records
 *	are read from the temporary file and placed on the standard 
 *	output.
 */
static
winp(f,w)
register FILE *f;
register struct utmp *w;
{
	if(fread(w,sizeof(struct utmp),1,f)!=1)
		return(0); 
	if((w->ut_type >= EMPTY) && (w->ut_type <= UTMAXTYPE))
		return ((unsigned)w);
	else {
		(void)fprintf(stderr,
			MSGSTR( BADFILOFF, "Bad file at offset %ld\n"),
			ftell(f)-sizeof(struct utmp));
		(void)strftime(timbuf, BUFSIZ, "%c %Z %n", 
				localtime(&w->ut_time));
		(void)fprintf(stderr,"%-*s %-*s %lu %s",
			LSZ,w->ut_line,NSZ,w->ut_user,w->ut_time,timbuf);
		intr();
	}
	return(0);
}

/*	wout() writes an output record of type utmp.h.  The
 *	variable "f" is a file descripter of either the temp
 *	file or the standard output.  The variable "w" is an
 *	address of the entry in the utmp.h structure.
 */
static
wout(f,w)
register FILE *f;
register struct utmp *w;
{

	(void)fwrite(w,sizeof(struct utmp),1,f);
}

static
mkdtab(p,w)
long	p;
register struct utmp *w;
{

	register struct dtab *dp;

	dp = Ldp;
	if(dp == NULL){
		dp = (struct dtab *)calloc(sizeof(struct dtab),1);
		if(dp == NULL)
			err(MSGSTR( NOCORE, "out of core"));
		Fdp = Ldp = dp;
	}
	dp->d_off1 = p;
}

static
setdtab(p,w1,w2)
long	p;
register struct utmp *w1, *w2;
{

	register struct dtab *dp;

	if((dp=Ldp)==NULL)
		err("no dtab");
	dp->d_off2 = p;
	dp->d_adj = w2->ut_time - w1->ut_time;
	if((Ldp=(struct dtab *)calloc(sizeof(struct dtab),1))==NULL)
		err(MSGSTR( NOCORE, "out of core"));
	Ldp->d_off1 = dp->d_off1;
	dp->d_ndp = Ldp;
}

static
adjust(p,w)
long	p;
register struct utmp *w;
{

	long pp;
	register struct dtab *dp;

	pp = p;

	for(dp=Fdp;dp!=NULL;dp=dp->d_ndp){
		if(dp->d_adj==0)
			continue;
		if(pp>=dp->d_off1 && pp < dp->d_off2)
			w->ut_time += dp->d_adj;
	}
}

/*
 *	invalid() determines whether the name field adheres to
 *	the criteria set forth in acctcon1.  If the name violates
 *	conventions, it returns a truth value meaning the name is
 *	invalid; if the name is okay, it returns false indicating
 *	the name is not invalid.
 */
#define	VALID	0
#define	INVALID	1

static
invalid(name)
char	*name;
{
	register int	i;

	for(i=0; i<NSZ; i++) {
		if(name[i] == '\0')
			return(VALID);
		if( ! (isalnum(name[i]) || (name[i] == '$')
			|| (name[i] == ' ') )) {
			return(INVALID);
		}
	}
	return(VALID);
}
