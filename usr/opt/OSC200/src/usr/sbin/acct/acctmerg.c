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
static char	*sccsid = "@(#)$RCSfile: acctmerg.c,v $ $Revision: 4.2.2.2 $ (DEC) $Date: 1993/10/07 21:53:37 $";
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
 * FUNCTIONS: getlast, getnext, parse, prtacct, sumcurr, tacctadd
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
 *      acctmerg [-a] [-v] [-p] [-h] [-i] [-t] [-u] [file...]
 *	-i	input is in tacct.h/ascii (instead of tacct.h)
 *	-a	output in tacct.h/ascii (instead of tacct.h)
 *	-v	output in verbose tacct.h/ascii
 *	-p	print input files with no processing
 *                  -i, -a, -v, -p may be followed by a fieldspec
 *                             e.g., -a1-3,5,7,14-13
 *      -h      (with -a, -v, -p only) print column headings
 *	-t	output single record that totals all input
 *	-u	summarize by uid, rather than uid/name
 *	reads std input and 0-NFILE files, all in tacct.h format,
 *	sorted by uid/name.
 *	merge/adds all records with same uid/name (or same uid if -u,
 *      or all records if -t), writes to std. output
 *	(still in tacct.h format)
 *	note that this can be used to summarize the std input
 */

#include <sys/types.h>
#include "acctdef.h"
#include <sys/acct.h>
#include <stdio.h>
#include "tacct.h"

#include <locale.h>
#include "acct_msg.h"
nl_catd catd;
#define MSGSTR(Num, Str) catgets(catd, MS_ACCT, Num, Str)

#define CSIZE 1000
#define NFILE	10
int	nfile;			/* index of last used in fl */
FILE	*fl[NFILE]	= { stdin, };

struct	tacct tb[NFILE];	/* current record from each file */
struct	tacct	tt = {
	0,
	"TOTAL"
};

char    asciiout;
char    asciiinp;
char    printonly;
char    totalonly;
char    uidsum;
char    verbose;
char    headings;
static int	sumcurr(), tacctadd(), output();
static int	getnext(), prtacct(), parse();
static struct tacct	*getleast();

#define NONUID  65535   /* Nonexistent UID */
#define NFIELDS 18      /* Number of printable fields */

char ifields[NFIELDS+1];  /* ASCII input fields, with 0 terminator */

/* Allow space to permit printing fields (e.g., logname) more than once */
char ofields[2*NFIELDS];

char *prog;

main(int argc, char **argv)
{
	register i;
	register struct tacct *tp;
	static char badspec[] = "%s: bad fieldspec `%s'\n";
	static char onespec[] = "%s: only one input fieldspec allowed\n";

	(void) setlocale (LC_ALL,"");
	catd = catopen(MF_ACCT,NL_CAT_LOCALE);

	prog = *argv;

	while (--argc > 0) {
		if (**++argv == '-')
			switch (*++*argv) {
			case 'i':
				if (asciiinp++) {
				    eprintf(MSGSTR( ONESPEC, onespec), prog);
				    exit(1);
				}
				if (*++*argv) {
				    if (parse(*argv,ifields,NFIELDS)==0) {
					eprintf(MSGSTR( BADSPEC, badspec), prog,
							 *argv);
					exit(1);
				    }
				}
				else for (i = NFIELDS; i > 0; --i)
				    ifields[i-1] = i;
				continue;
			case 't':
				totalonly++;
				continue;
			case 'u':
				uidsum++;
				continue;
			case 'p':
				printonly++;
				goto fieldspec;
			case 'v':
				verbose++;
				goto fieldspec;
			case 'h':
				headings++;     /* implies asciiout */
			case 'a':
				asciiout++;
			fieldspec:
				if (*++*argv) { /* -[avph]fieldspec */
				  if (parse(*argv,ofields,sizeof ofields)==0){
					eprintf(MSGSTR( BADSPEC, badspec), prog,
							 *argv);
					exit(1);
				    }
				}
				continue;
			}
		else {
			if (++nfile >= NFILE) {
				eprintf(MSGSTR(TOOMANYFILES, "%s: >%d files\n"),
						prog, NFILE);
				exit(1);
			}
			if ((fl[nfile] = fopen(*argv, "r")) == NULL) {
				eprintf(MSGSTR(CANTOPEN, "%s: can't open %s\n"),
						prog, *argv);
				exit(1);
			}
		}
	}

	/* If no output field specifications, assume all fields */
	if (ofields[0] == 0)
		for (i = NFIELDS; i > 0; --i)
		    ofields[i-1] = i;

	if (headings)
		prtacct(NULL);

	if (printonly) {
		for (i = 0; i <= nfile; i++)
			while (getnext(0))
				prtacct(&tb[0]);
		exit(0);
	}

	for (i = 0; i <= nfile; i++)
		(void)getnext(i);

	while ((tp = getleast()) != NULL)	/* get least uid of all files, */
		sumcurr(tp);			/* sum all entries for that uid, */
	if (totalonly)				/* and write the 'summed' record */
		output(&tt);
	return(0);
}

/*
 *	getleast returns ptr to least (lowest uid)  element of current 
 *	avail, NULL if none left; always returns 1st of equals
 */
static struct tacct *
getleast()
{
	register struct tacct *tp, *least;
	register int i;

	least = NULL;
	for (tp = tb, i = 0; i <= nfile; tp++, i++) {
		if (fl[i] == NULL)
			continue;
		if (least == NULL ||
			tp->ta_uid < least->ta_uid ||
			((tp->ta_uid == least->ta_uid) &&
			!uidsum &&
			(strncmp(tp->ta_name, least->ta_name, NSZ) < 0)))
			least = tp;
	}
	return(least);
}

/*
 *	sumcurr sums all entries with same uid/name (into tp->tacct record)
 *	writes it out, gets new entry
 */
static
sumcurr(tp)
register struct tacct *tp;
{
	register int i = tp - &tb[0];
	struct tacct tc;

	tc = *tp;
	tacctadd(&tt, tp);	/* gets total of all uids */
	(void)getnext(i);       /* get next one in same file */
	while (i <= nfile)
		if (fl[i] != NULL &&
			tp->ta_uid == tc.ta_uid &&
			(uidsum || EQN(tp->ta_name, tc.ta_name))) {
			tacctadd(&tc, tp);
			tacctadd(&tt, tp);
			(void)getnext(i);
		} else {
			tp++;	/* look at next file */
			i++;
		}
	if (!totalonly)
		output(&tc);
}

static
tacctadd(t1, t2)
register struct tacct *t1, *t2;
{
	t1->ta_cpu[0] += t2->ta_cpu[0];
	t1->ta_cpu[1] += t2->ta_cpu[1];
	t1->ta_kcore[0] += t2->ta_kcore[0];
	t1->ta_kcore[1] += t2->ta_kcore[1];
	t1->ta_con[0] += t2->ta_con[0];
	t1->ta_con[1] += t2->ta_con[1];
	t1->ta_io[0] += t2->ta_io[0];
	t1->ta_io[1] += t2->ta_io[1];
	t1->ta_rw[0] += t2->ta_rw[0];
	t1->ta_rw[1] += t2->ta_rw[1];
	t1->ta_du += t2->ta_du;
	t1->ta_qsys += t2->ta_qsys;
	t1->ta_fee += t2->ta_fee;
	t1->ta_pc += t2->ta_pc;
	t1->ta_sc += t2->ta_sc;
	t1->ta_dc += t2->ta_dc;
}

static
output(tp)
register struct tacct *tp;
{
	if (asciiout)
		prtacct(tp);
	else
		(void)fwrite(tp, sizeof(*tp), 1, stdout);
}

/*
 *	getnext reads next record from stream i, returns 1 if one existed
 */
static
getnext(i)
int i;
{
	register struct tacct *tp;
	register int j, z;
	register FILE *fp;
	register int ret;
	char tmpname[sizeof(tp->ta_name)+1];
	ushort tmpshort;
	unsigned long tmplong;
	static char ufmt[] = " %hu";
	static char efmt[] = " %le";
	static char lufmt[] = " %lu";

	static struct tacct tproto = {NONUID,
				      '*','N','O','N','A','M','E','*'};

	if ((fp = fl[i]) == NULL)
	    return(0);
	tp = &tb[i];
	if (asciiinp) {
	    *tp = tproto;
	    for (j = 0; ; j++) {
		/* Terminates after ifields[j] == 0 */
		/* or when an expected field isn't there. */
		switch(ifields[j]) {
		    case 0:  while (((z = getc(fp)) != EOF) && (z != '\n'));
			     if (z != EOF) (void)ungetc('\n', fp);
			     return(1);

		    case 1:  if ((ret = fscanf(fp, lufmt, &tmplong)) == 1)
				 tp->ta_uid = tmplong;
			     break;
		    case 2:  if ((ret = fscanf(fp, " %9s", tmpname)) == 1)
				 (void)CPYN(tp->ta_name,tmpname);
			     break;
		    case 3:  ret = fscanf(fp, efmt, &tp->ta_cpu[0]); break;
		    case 4:  ret = fscanf(fp, efmt, &tp->ta_cpu[1]); break;
		    case 5:  ret = fscanf(fp, efmt, &tp->ta_kcore[0]); break;
		    case 6:  ret = fscanf(fp, efmt, &tp->ta_kcore[1]); break;
		    case 7:  ret = fscanf(fp, efmt, &tp->ta_con[0]); break;
		    case 8:  ret = fscanf(fp, efmt, &tp->ta_con[1]); break;
		    case 9:  ret = fscanf(fp, efmt, &tp->ta_io[0]); break;
		    case 10: ret = fscanf(fp, efmt, &tp->ta_io[1]); break;
		    case 11: ret = fscanf(fp, efmt, &tp->ta_rw[0]); break;
		    case 12: ret = fscanf(fp, efmt, &tp->ta_rw[1]); break;
		    case 13: ret = fscanf(fp, efmt, &tp->ta_du); break;
		    case 14: ret = fscanf(fp, lufmt, &tp->ta_qsys); break;
		    case 15: ret = fscanf(fp, efmt, &tp->ta_fee); break;
		    case 16: ret = fscanf(fp, lufmt, &tp->ta_pc); break;
		    case 17: if ((ret = fscanf(fp, ufmt, &tmpshort)) == 1)
				 tp->ta_sc = tmpshort;
			     break;
		    case 18: if ((ret = fscanf(fp, ufmt, &tmpshort)) == 1)
				 tp->ta_dc = tmpshort;
			     break;
		}
		if (ret != 1) break;
	    }
	} else {
		if (fread(tp, sizeof(*tp), 1, fp) == 1)
			return(1);
	}
	fl[i] = NULL;
	return(0);
}

static
prtacct(tp)
register struct tacct *tp;
{       register int i;
	register char *efmt;
	register char *lfmt;
	register char *sfmt;

	if (tp == NULL) {   /* Print headings */
	    efmt = "%-13s";
	    lfmt = "%-11s";
	    sfmt = "%-6s";
	    for (i = 0; i < NFIELDS; i++) {
	      switch(ofields[i]) {
		case 0:  break;
		case 1:  (void)printf(sfmt,   MSGSTR( ACCTUID, "UID")); break;
		case 2:  (void)printf("%-9s", MSGSTR( ACCTLOGNAME, "LOGNAME")); break;
		case 3:  (void)printf(efmt,   MSGSTR( ACCTPRI_CPU, "PRI_CPU")); break;
		case 4:  (void)printf(efmt,   MSGSTR( ACCTNPRI_CPU, "NPRI_CPU")); break;
		case 5:  (void)printf(efmt,   MSGSTR( ACCTPRI_MEM, "PRI_MEM")); break;
		case 6:  (void)printf(efmt,   MSGSTR( ACCTNPRI_MEM, "NPRI_MEM")); break;
		case 7:  (void)printf(efmt,   MSGSTR( ACCTPRI_RD_WR, "PRI_RD/WR")); break;
		case 8:  (void)printf(efmt,   MSGSTR( ACCTNPRI_RD_WR, "NPRI_RD/WR")); break;
		case 9:  (void)printf(efmt,   MSGSTR( ACCTPRI_BLKIO, "PRI_BLKIO")); break;
		case 10: (void)printf(efmt,   MSGSTR( ACCTNPRI_BLKIO, "NPRI_BLKIO")); break;
		case 11: (void)printf(efmt,   MSGSTR( ACCTPRI_CONNECT, "PRI_CONNECT")); break;
		case 12: (void)printf(efmt,   MSGSTR( ACCTNPRI_CONNECT, "NPRI_CONNECT")); break;
		case 13: (void)printf(efmt,   MSGSTR( ACCTDSK_BLOCKS, "DSK_BLOCKS")); break;
		case 14: (void)printf(lfmt,   MSGSTR( ACCTPRINT, "PRINT")); break;
		case 15: (void)printf(efmt,   MSGSTR( ACCTFEES, "FEES")); break;
		case 16: (void)printf(lfmt,   MSGSTR( ACCTPROCESSES, "PROCESSES")); break;
		case 17: (void)printf(sfmt,   MSGSTR( ACCTSESS, "SESS")); break;
		case 18: (void)printf(sfmt,   MSGSTR( ACCTDSAMPS, "DSAMPS")); break;
	      }
	    }
	} else {
	    efmt = (verbose ? "%-13.5e" : "%-13.0f");
	    lfmt = "%-11lu";
	    sfmt = "%-6u";
	    for (i = 0; i < NFIELDS; i++) {
	      switch(ofields[i]) {
		case 0:  break;
		case 1:  (void)printf(lfmt,   tp->ta_uid); break;
		case 2:  (void)printf("%-9.8s", tp->ta_name); break;
		case 3:  (void)printf(efmt,   tp->ta_cpu[0]); break;
		case 4:  (void)printf(efmt,   tp->ta_cpu[1]); break;
		case 5:  (void)printf(efmt,   tp->ta_kcore[0]); break;
		case 6:  (void)printf(efmt,   tp->ta_kcore[1]); break;
		case 7:  (void)printf(efmt,   tp->ta_io[0]); break;
		case 8:  (void)printf(efmt,   tp->ta_io[1]); break;
		case 9:  (void)printf(efmt,   tp->ta_rw[0]); break;
		case 10: (void)printf(efmt,   tp->ta_rw[1]); break;
		case 11: (void)printf(efmt,   tp->ta_con[0]); break;
		case 12: (void)printf(efmt,   tp->ta_con[1]); break;
		case 13: (void)printf(efmt,   tp->ta_du); break;
		case 14: (void)printf(lfmt,   tp->ta_qsys); break;
		case 15: (void)printf(efmt,   tp->ta_fee); break;
		case 16: (void)printf(lfmt,   tp->ta_pc); break;
		case 17: (void)printf(sfmt,   tp->ta_sc); break;
		case 18: (void)printf(sfmt,   tp->ta_dc); break;
	      }
	    }
	}
	(void)printf("\n");
}



/* Parse field specifications with a state machine */

/* States */
#define START   (0<<2)
#define NUM1    (1<<2)
#define BETW    (2<<2)
#define NUM2    (3<<2)

/* Character classes */
#define DIGIT   0
#define DASH    1
#define COMMA   2
#define END     3

/* Actions */
#define STNUM   (0<<4)      /* Start number */
#define CNUM    (1<<4)      /* Continue number */
#define ASUM1   (2<<4)      /* Assume 1 for first number */
#define SAVE1   (3<<4)      /* Save as first number */
#define ERR     (4<<4)      /* Too bad */
#define NOP     (5<<4)      /* Nothing to do (check for END)*/
#define FILL1   (6<<4)      /* Fill in one number and check for END */
#define FILLEND (7<<4)      /* Fill through last field and check for END */
#define FILL    (8<<4)      /* Fill range and check for END */

#define ACTION  (~0xF)      /* Mask for action part */
#define STATE   (0xC)       /* Mask for state */
#define INPUT   (0x3)       /* Mask for input character class */

/* Action/Nextstate Table */
static unsigned char nxtact[] =
{             /*  DIGIT           DASH            COMMA           END  */
/* START */     STNUM|NUM1,     ASUM1|BETW,     NOP|START,      NOP,
/* NUM1  */     CNUM|NUM1,      SAVE1|BETW,     FILL1|START,    FILL1,
/* BETW  */     STNUM|NUM2,     NOP|BETW,       FILLEND|START,  FILLEND,
/* NUM2  */     CNUM|NUM2,      ERR,            FILL|START,     FILL,
};

static
parse(line, flds, fldsize)
register char *line;
char *flds;
{
	register int i;
	register int state;
	register int nmbr, firstnum, secnum, fieldno;

	/* Set up for no fields */
	for (i = fldsize; --i >= 0; flds[i] = 0) ;

	fieldno = 0;

	for (state = START; ; ++line) {
	    /* Form state/char-class combo */
	    state |= ((*line >= '0' && *line <= '9') ? DIGIT :
		      (*line == ','                  ? COMMA :
		      (*line == '-'                  ? DASH  : END)));

	    switch(nxtact[state]&ACTION) {

		case STNUM:     /* Start number */
		    nmbr = *line - '0'; break;
		case CNUM:      /* Continue number */
		    nmbr = 10*nmbr + *line - '0'; break;
		case ASUM1:     /* Assume 1 for first number */
		    firstnum = 1; break;
		case SAVE1:     /* Save as first number */
		    firstnum = nmbr; break;
		case ERR:       /* Too bad */
		    return(0);
		case FILL1:     /* Fill in one number and check for END */
		    firstnum = secnum = nmbr; goto fill;
		case FILLEND:   /* Fill thru last field and check for END */
		    secnum = NFIELDS; goto fill;
		case FILL:      /* Fill range and check for END */
		    secnum = nmbr;
		fill:
		    if (firstnum > NFIELDS || secnum > NFIELDS)
			    return(0);
		    for (i = firstnum; ;i += (i < secnum ? 1 : -1)) {
			if (i > 0 && i <= NFIELDS && fieldno < fldsize)
			    flds[fieldno++] = i;
			else return(0);
			if (i == secnum) break;
		    }
		    /* Fall thru */
		case NOP:       /* (check for END) */
		    if ((state&INPUT) == END)
			return(*line == '\0');
	    }
	    state = nxtact[state]&STATE;
	}
}
