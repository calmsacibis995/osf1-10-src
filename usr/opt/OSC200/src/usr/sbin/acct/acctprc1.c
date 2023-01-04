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
static char	*sccsid = "@(#)$RCSfile: acctprc1.c,v $ $Revision: 4.2.2.2 $ (DEC) $Date: 1993/05/25 20:28:39 $";
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
 * FUNCTIONS: addurec, getmem, getnamc, getty, readctmp, addsrec
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
 *	acctprc1 [ctmpfile]
 *	reads std. input (acct.h format), adds login names
 *	writes std. output (ptmp.h/ascii format)
 *      if ctmpfile is given, it is expected have ctmp.h/ascii data, sorted
 *      by terminal and time.
 *      it is used to make better guesses at login names
 */

#include <sys/types.h>
#include "acctdef.h"
#include <sys/acct.h>
#include <stdio.h>
#include <pwd.h>
#include "ctmp.h"
#include "ptmp.h"
#include "tacct.h"
#include "table.h" 

#include <locale.h>
#include "acct_msg.h"
#define	MSGSTR(Num, Str)	NLgetamsg(MF_ACCT, MS_ACCT, Num, Str)

#define	ACCTF	0300		/* record type: 00 = acct */
#define MYKIND(flag)	((flag & ACCTF) == 0)

struct	acct	ab;
struct	ctmp	cb;
struct	ptmp	pb;

struct urec {				/* 1 for each distinct uid/name */
	uid_t	ur_uid;			/* sorted by uid/name */
	char	ur_name[NSZ];
	ushort  ur_chain;               /* common hash chain index */
} *ur;                                  /* NULL ur means no (usable) "ctmp" */

struct srec {				/* 1 for each distinct session */
	dev_t	sr_tty;			/* dev, used to connect with process*/
	ushort  sr_urec;                /* index of urec for user */
	time_t	sr_start;		/* start time of session */
} *sr;

struct trec {                           /* 1 for each distinct tty */
	dev_t   tr_tty;                 /* device id */
	ushort  tr_first;               /* index of first srec for tty */
	ushort  tr_last;                /* index of last srec for tty */
	ushort  tr_cur;                 /* current - to speed search */
	ushort  tr_chain;               /* hash chain */
} *tr;

char	*malloc();
char	*uidtonam();
static int	readctmp(), getmem();
static int	addurec(), addsrec();
static char	*getnamc();
static struct trec 	*getty();

static  char *prog;     /* program name for error messages */

main(int argc, char **argv)
{
	register char *p;
	long	elaps[2];
	long	etime, stime;

	(void) setlocale (LC_ALL,"");
	prog = argv[0];

	if (argc > 1)
		readctmp(argv[1]);

	while (fread((char *)&ab, sizeof(ab), 1, stdin) == 1) {
		if (!MYKIND(ab.ac_flag))
			continue;
		pb.pt_uid = ab.ac_uid;
		/* Use name from session if available; otherwise, passwd */
		if (ur==NULL ||
		    (p = getnamc(ab.ac_uid, ab.ac_tty, ab.ac_btime))==NULL)
			p = uidtonam(ab.ac_uid);

		if ( !p || !*p )   /* no one seems to know who this was, */
			p = "?";   /* but SOMETHING must be passed       */

		(void)CPYN(pb.pt_name, p);
		/*
		 * approximate cpu P/NP split same as elapsed time
		 */
		if ((etime = (long)expacct(ab.ac_etime)) == 0)
			etime = 1;
		stime = (long)(expacct(ab.ac_stime) + expacct(ab.ac_utime));

		pb.pt_mem = (unsigned)KCORE(ab.ac_mem); 
		pnpsplit(ab.ac_btime, etime, elaps);
		pb.pt_cpu[0] = (double)stime * (double)elaps[0] / etime;
		pb.pt_cpu[1] = (stime > pb.pt_cpu[0]) ?
					    stime - pb.pt_cpu[0] : 0;
		pb.pt_io = (long)expacct(ab.ac_io);
		pb.pt_rw = (long)expacct(ab.ac_rw);

		(void)printf("%-11lu %-.8s %lu %lu %lu %lu %u\n",
			pb.pt_uid,
			pb.pt_name,
			pb.pt_cpu[0], pb.pt_cpu[1],
			pb.pt_io, pb.pt_rw,
			pb.pt_mem);
	}
	return(0);
}

/*
 *	read ctmp file, build up urec-srec data structures for
 *	later use by getnamc
 */
static
readctmp(fname)
char *fname;
{
	register FILE *fp;
	register  int splast = 0;
	register int up;
		 long devtemp;
		 int    flag = 0;

	if ((fp = fopen(fname, "r")) == NULL) {
		eprintf(MSGSTR( CANTREAD, "%s: Can't read %s\n"), prog, fname);
		return;
	}

	if (getmem() != 0)
		ur = NULL;
	else
	while (fscanf(fp, "%lu\t%u\t%9s\t%*lu\t%*lu\t%lu\t%*[^\n]", /*cin001*/
		&devtemp,
		&cb.ct_uid,
		cb.ct_name, /* 8 chars wide, but ct_start is next field! */
		&cb.ct_start) != EOF) {

		cb.ct_tty = (dev_t)devtemp;
		flag = 1;   /* Remember that we've been thru here   */

		/* Get user record - create if necessary */
		if ((up = addurec(cb.ct_uid, cb.ct_name)) < 0) {
			ur = NULL;
			break;
		}
		/* Add session record if this is not just a "continuation",
		 * the same user on the same tty at the next login.
		 * This relies on the sort order established above.
		 */
		if (splast <= 0 ||
		    cb.ct_tty != sr[splast].sr_tty ||
		    (ushort) up != sr[splast].sr_urec) {
			if ((splast = addsrec(&cb, (ushort)up)) < 0) {
				ur = NULL;
				break;
			}
		}
	}
	if (ur == NULL)
		eprintf(MSGSTR( OUTOFMEM, "%s: Out of memory -- %s not used\n"),
				prog, fname);

	if (flag == 0)          /* Treat empty ctmp file as if it wasn't */
		ur = NULL;      /* ... specified at all.                 */

	(void)fclose(fp);
}

/*
 *	using urec-srec data (if any), make best guess at login name
 *	corresponding to uid, return ptr to the name.
 *	must match on tty; use start time to help guess
 *      for any urec having same uid as uid.
 *      give it to person of same uid who last used that terminal
 */
static char *
getnamc(uid, tty, start)
register uid_t uid;
dev_t	tty;
time_t	start;
{
	register struct srec *sp, *lim;
	register struct trec *tp;

	if ((tp = getty(tty,0)) == NULL)
		return(NULL);

	/* Search all session records, beginning with current one, for
	 * the last session for uid beginning before the given start time.
	 * Update tr_cur accordingly.  Return pointer to associated name.
	 */
	sp = &sr[tp->tr_cur];

	/* Scan forward in time to beyond process start time */
	for (lim = &sr[tp->tr_last]; ; ++sp, ++tp->tr_cur) {
		if (sp->sr_start > start || sp >= lim)
			break;
	}

	/* Scan backward to position at or before process start time */
	for (lim = &sr[tp->tr_first]; ; --sp, --tp->tr_cur) {
		if (sp->sr_start <= start || sp <= lim)
			break;
	}

	if (sp->sr_start > start)
		return(NULL);

	/* Scan backward for first record with matching uid */
	for ( ; ; --sp, --tp->tr_cur) {
		if (uid == ur[sp->sr_urec].ur_uid)
			return(ur[sp->sr_urec].ur_name);
		if (sp <= lim) /* Can't be less */
			return(NULL);
	}
}

/* Space for all tables is allocated dynamically.  The terminal and user
 * tables are hashed with chaining on collisions.  The session table is
 * linear.  Indexes rather than pointers are used both to save space
 * where pointers are longer than shorts) and to permit relocation.
 */

struct  table urtable = INITTABLE(ur, A_USIZE);
struct  table trtable = INITTABLE(tr, A_TSIZE);
struct  table srtable = INITTABLE(sr, A_SSIZE);

/* Initialize tables and clear hash areas */
static
getmem()
{       register int i;
	static struct urec u0;
	static struct trec t0;

	if (extend(&urtable) == NULL ||
	    extend(&trtable) == NULL ||
	    extend(&srtable) == NULL)
		return(1);

	for (i = 0; i < UHASH; i++)
		ur[i] = u0;

	for (i = 0; i < THASH; i++)
		tr[i] = t0;

	return(0);
}

/* Return index of user with given uid and name.  If no such user is in
 * table, add him, extending tables as necessary.  Maintain table using
 * hashing of initial size with chained extensions so the extensions can
 * grow contiguously.
 */
static
addurec(uid, name)
uid_t uid;
char *name;
{       register ushort u, uh;
	register struct urec *up;
	ushort hashuser();
	static uused = UHASH;

	u = uh = hashuser(uid, name);
	do {
	    up = &ur[u];
	    if (up->ur_uid == uid && EQN(up->ur_name,name))
		    return((int) u);
	} while (u = up->ur_chain);

	/* If already an entry in this slot (first hash), get another slot. */
	if (up->ur_name[0]) {
		if ((uused += 1) > urtable.tb_nel) {
			urtable.tb_nel += urtable.tb_nel/3;
			if (extend(&urtable) == NULL)
				return(-1);
		}
		up = &ur[u = uused];
	} else
		u = uh;
	up->ur_uid = uid;
	(void)CPYN(up->ur_name, name);
	if (u != uh) {  /* If not first entry, link on at head of chain */
		up->ur_chain = ur[uh].ur_chain;
		ur[uh].ur_chain = uused;
	}
	return((int) u);
}

/* Add session record for session given by cp, with user index in hash table
 * of up.  Extend tr (terminal record) and sr (session record) tables as
 * needed.  Return index of record.
 */
static
addsrec(cp, uindex)
struct ctmp *cp;
ushort uindex;
{       register struct trec *tp;
	register struct srec *sp;
	static int sused = 0;    /* 0 isn't used */

	if ((tp = getty(cp->ct_tty,1)) == NULL)
		return(-1);

	/* Assure that there's space for one more session */
	if ((sused+1) <= sused)         /* Check for wraparound */
		return(-1);
	sused += 1;
	if (sused != (ushort) sused)    /* Make sure it fits in a ushort */
		return(-1);
	if (sused > srtable.tb_nel) {
		srtable.tb_nel += srtable.tb_nel/3;
		/* Check for overflow */
		if (srtable.tb_nel < sused || extend(&srtable) == NULL)
			return(-1);
	}
	sp = &sr[sused];
	sp->sr_tty = cp->ct_tty;
	sp->sr_urec = uindex;
	sp->sr_start = cp->ct_start;
	if (tp->tr_first == 0)  /* first record for this tty */
	    tp->tr_first = tp->tr_cur = sused;
	return((int) (tp->tr_last = sused));
}

/* Obtain pointer to trec table entry for dev tty.  If record doesn't already
 * exist, create if create!=0 (with tr_first==0), else return NULL.
 */
static struct trec *
getty(tty, create)
register dev_t tty;
int create;
{       register struct trec *tp;
	register ushort t, th;
	static tused = THASH;

#define hashtty(x) (((ushort) tty) % THASH)

	t = th = hashtty(tty);
	do {
	    tp = &tr[t];
	    if (tp->tr_tty == tty)
		    return(tp);
	} while (t = tp->tr_chain);

	if (create == 0) return(NULL);

	/* If already an entry in this slot (first hash), get another slot.
	 * An empty slot is one that has tr_first non-zero, since sessions
	 * don't use index 0.
	 */
	if (tp->tr_first) {
		if ((tused += 1) > trtable.tb_nel) {
			trtable.tb_nel += 32;
			if (extend(&trtable) == NULL)
				return(NULL);
		}
		tp = &tr[t = tused];
	} else
		t = th;
	tp->tr_tty = tty;
	tp->tr_first = 0;
	if (t != th) {  /* If not first entry, link on at head of chain */
		tp->tr_chain = tr[th].tr_chain;
		tr[th].tr_chain = tused;
	}
	return(tp);
}
