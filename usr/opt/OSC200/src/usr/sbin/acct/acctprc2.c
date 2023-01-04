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
static char	*sccsid = "@(#)$RCSfile: acctprc2.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 05:59:59 $";
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
 * FUNCTIONS: getuser, ucmp
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
 *	acctprc2 <ptmp1 >ptacct
 *	reads std. input (in ptmp.h/ascii format)
 *	hashes items with identical uid/name together, sums times
 *	sorts in uid/name order, writes tacct.h records to output
 */

#include <sys/types.h>
#include "acctdef.h"
#include <sys/acct.h>
#include <stdio.h>
#include "ptmp.h"
#include "tacct.h"
#include "table.h"

#include <locale.h>
#include "acct_msg.h"
#define	MSGSTR(Num, Str)	NLgetamsg(MF_ACCT, MS_ACCT, Num, Str)

struct	ptmp	pb;
struct	tacct	tb;

struct	utab	{
	ushort  ut_chain;       /* chain for hash table */
	uid_t	ut_uid;
	char	ut_user[NSZ];
	double	ut_cpu[2];	/* cpu time (mins) */
	double	ut_kcore[2];	/* kcore-mins */
	double  ut_io[2];       /* chars/512 */
	double  ut_rw[2];       /* blocks */
	long	ut_pc;		/* # processes */
} *ub, clrtab;

struct table utable = INITTABLE(ub, A_USIZE);
ushort uused = UHASH;

char    *extend();
unsigned hashuser();

static int	enter(), squeeze(), ucmp();
static int	output(), nomem();
static struct utab	*getuser();

char *prog;

main(int argc, char **argv)
{       register int i;

	(void) setlocale (LC_ALL,"");
	prog = argv[0];

	if (extend(&utable) == NULL)
		nomem();

	/* Clear hash table portion */
	for (i = UHASH; --i >= 0; ub[i] = clrtab) ;

	while (scanf("%lu %9s %lu %lu %lu %lu %u",
		&pb.pt_uid,
		pb.pt_name, /* May extend into pt_cpu, but OK */
		&pb.pt_cpu[0], &pb.pt_cpu[1],
		&pb.pt_io, &pb.pt_rw,
		&pb.pt_mem) != EOF)
			enter(&pb);
	squeeze();
	qsort(ub, uused, sizeof(struct utab), ucmp);
	output();
	return(0);
}

static
enter(p)
register struct ptmp *p;
{
	register struct utab *utp;
	double memk;
	double prfrac;
	long prime, tot;

	utp = getuser(p->pt_uid, p->pt_name);
	utp->ut_cpu[0] += MINS(p->pt_cpu[0]);
	utp->ut_cpu[1] += MINS(p->pt_cpu[1]);
	memk = (double)p->pt_mem;
	utp->ut_kcore[0] += memk * MINS(p->pt_cpu[0]);
	utp->ut_kcore[1] += memk * MINS(p->pt_cpu[1]);
	prfrac = p->pt_cpu[1] ?
		((double) p->pt_cpu[0]) / (p->pt_cpu[0] + p->pt_cpu[1]) : 1.0;
	tot = (p->pt_io + 255) >> 9;    /* I/O rounded to 512-blocks */
	utp->ut_io[0] += (prime = prfrac * tot);
	utp->ut_io[1] += tot - prime;
	utp->ut_rw[0] += (prime = prfrac * p->pt_rw);
	utp->ut_rw[1] += p->pt_rw - prime;
	utp->ut_pc++;
}

static
squeeze()		/*eliminate holes in hash table*/
{
	register struct utab *p, *q, *lim;

	lim = &ub[uused];
	for (p = q = ub; p < lim; p++)
		if (p->ut_user[0]) {
			*q = *p;
			++q;
		}
	uused = q - ub;
}

static
ucmp(p1, p2)
register struct utab *p1, *p2;
{
	if (p1->ut_uid != p2->ut_uid)
		return(p1->ut_uid - p2->ut_uid);
	return(strncmp(p1->ut_user, p2->ut_user, NSZ));
}

static
output()
{
	register i;
	register struct utab *p, *lim;

	for (p = ub, lim = &ub[uused]; p < lim; p++) {
		tb.ta_uid = p->ut_uid;
		(void)CPYN(tb.ta_name, p->ut_user);
		tb.ta_cpu[0] = p->ut_cpu[0];
		tb.ta_cpu[1] = p->ut_cpu[1];
		tb.ta_kcore[0] = p->ut_kcore[0];
		tb.ta_kcore[1] = p->ut_kcore[1];
		tb.ta_io[0] = p->ut_io[0];
		tb.ta_io[1] = p->ut_io[1];
		tb.ta_rw[0] = p->ut_rw[0];
		tb.ta_rw[1] = p->ut_rw[1];
		tb.ta_pc = p->ut_pc;
		(void)fwrite(&tb, sizeof(tb), 1, stdout);
	}
}

static struct utab *
getuser(uid, name)
uid_t uid;
char *name;
{       register ushort u, uh;
	register struct utab *up;

	u = uh = hashuser(uid, name);
	do {
	    up = &ub[u];
	    if (up->ut_uid == uid && EQN(up->ut_user,name))
		    return(up);
	} while (u = up->ut_chain);

	/* If already an entry in this slot (first hash), get another slot. */
	if (up->ut_user[0]) {
		if ((uused += 1) > utable.tb_nel) {
			utable.tb_nel += utable.tb_nel/3;
			if (extend(&utable) == NULL)
				nomem();
		}
		up = &ub[u = uused];
		*up = clrtab;   /* Clear entry */
	} else
		u = uh;

	up->ut_uid = uid;
	(void)CPYN(up->ut_user, name);
	if (u != uh) {  /* If not first entry, link on at head of chain */
		up->ut_chain = ub[uh].ut_chain;
		ub[uh].ut_chain = uused;
	}
	return(up);
}

static
nomem()
{       eprintf(MSGSTR( NOMEM, "%s: Cannot allocate memory\n"), prog);
	exit(1);
}
