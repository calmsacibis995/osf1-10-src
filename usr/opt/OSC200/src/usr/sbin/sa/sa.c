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
static char	*sccsid = "@(#)$RCSfile: sa.c,v $ $Revision: 4.2.4.2 $ (DEC) $Date: 1993/10/08 16:01:40 $";
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
 * RESTRICTED RIGHTS LEGEND
 * Use, Duplication or Disclosure by the Government is subject to
 * restrictions as set forth in paragraph (b)(3)(B) of the rights in
 * Technical Data and Computer Software clause in DAR 7-104.9(a).
 */ 
/*
 * COMPONENT_NAME: (CMDSTAT) statistical commands....
 *
 * FUNCTIONS: sa
 *
 * ORIGINS: 26,27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 * OBJECT CODE ONLY SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

/*
 *	Extensive modifications to internal data structures
 *	to allow arbitrary number of different commands and users added.
 *
 *	Also allowed the digit option on the -v flag (interactive
 *	threshold compress) to be a digit string, so one can
 *	set the threshold > 9.
 *
 *	Also added the -f flag, to force no interactive threshold
 *	compression with the -v flag.
 *
 *	Robert Henry
 *	UC Berkeley
 *	31jan81
 */

#include <stdio.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/acct.h>
#include <signal.h>
#include <utmp.h>
#include <pwd.h>
#include <locale.h>

#include        <nl_types.h>
#include        "sa_msg.h"

nl_catd catd;
#define MSGSTR(Num,Str) catgets(catd, MS_SA, Num, Str)  /*MSG*/


/* interpret command time accounting */

#define	NC	sizeof(acctbuf.ac_comm)


struct acct acctbuf;
int	lflg;			/* Separate system and user time */
int	cflg;			/* percentage of total time      */
int	Dflg;			/* total number of disk i/o oper */
int	dflg;			/* sort by ave. num. disk i/o's  */
int	iflg;			/* Don't read in summary file.   */
int	jflg;			/* Give seconds instead of min.  */
int	Kflg;			/* Print&sort cpu-storage intgrl */
int	kflg;			/* sort by cpu time ave mem used */
int	nflg;			/* sort by num of calls          */
int	aflg;			/* print all command, even singl */
int	rflg;			/* Reverse the order of the sort */
int	oflg;			/* print u/s                     */
int	tflg;			/* ration of real vs. user+sys   */
int	vflg;			/* junk out command of n or fewr */
int	fflg;			/* force no inter threshold comp */
int	uflg;			/* print the user id of the comm.*/
int	sflg;			/* Merge the account file w/ save*/
int	bflg;			/* sort user sum div # of calls  */
int	mflg;			/* # of proc and cpu min.per/user*/
int	debug=0;		/* debugging flags		 */

struct	utmp	utmp;
#define	NAMELG	(sizeof(utmp.ut_name)+1)

struct 	Olduser{
	uid_t	Us_uid;	/*OSF*/
	int	Us_cnt;
	double	Us_ctime;
	double	Us_io;
	double	Us_imem;
};
	
struct	user {
	char	name[NC];		/* this is <\001><user id><\000> */
	struct	Olduser	oldu;
	char	us_name[NAMELG];
};
#define	us_uid		oldu.Us_uid /*OSF*/
#define	us_cnt		oldu.Us_cnt
#define	us_ctime	oldu.Us_ctime
#define	us_io		oldu.Us_io
#define	us_imem		oldu.Us_imem

/*
 *	We protect ourselves from preposterous user id's by looking
 *	through the passwd file for the highest uid allocated, and
 *	then adding 10 to that.
 *	This prevents the user structure from growing too large.
 */
/*
 *	There is a -2 uid for the standard nobody in NFS. To avoid
 * 	overflow of maxuser and to record the accounting information
 *	for nobody, I decided to delete the old USERSLOP. The user
 *	structure must therefore have a maximum of 2^(sizeof(uid_t)) entries. 
 *      As a consequence, I modified the program to save only nonzero user 
 *	entries if the -s option is used as we don't want a very large
 *	usracct file. So the additional Us_uid is inserted into 
 *	struct Olduser to restore the summary information for users. nm
#define	USERSLOP	10
 */
#define USERSLOP 	(uid_t)0

struct	process {
	char	name[NC];
	int	count;
	double	realt;
	double	cput;
	double	syst;
	double	imem;
	double	io;
};

union	Tab{
	struct	process	p;
	struct	user	u;
};

typedef	union Tab cell;

int	(*cmp)();	/* compares 2 cells; set to appropriate func */
cell	*enter();
struct	user *finduser();
struct	user *wasuser();

/*
 *	Table elements are keyed by the name of the file exec'ed.
 *	Because on large systems, many files can be exec'ed,
 *	a static table size may grow to be too large.
 *
 *	Table elements are allocated in chunks dynamically, linked
 *	together so that they may be retrieved sequentially.
 *
 *	An index into the table structure is provided by hashing through
 *	a seperate hash table.
 *	The hash table is segmented, and dynamically extendable.
 *	Realize that the hash table and accounting information is kept
 *	in different segments!
 *
 *	We have a linked list of hash table segments; within each
 *	segment we use a quadratic rehash that touches no more than 1/2
 *	of the buckets in the hash table when probing.
 *	If the probe does not find the desired symbol, it moves to the
 *	next segment, or allocates a new segment.
 *
 *	Hash table segments are kept on the linked list with the first
 *	segment always first (that will probably contain the
 *	most frequently executed commands) and
 *	the last added segment immediately after the first segment,
 *	to hopefully gain something by locality of reference.
 *
 *	We store the per user information in the same structure as
 *	the per exec'ed file information.  This allows us to use the
 *	same managers for both, as the number of user id's may be very
 *	large.
 *	User information is keyed by the first character in the name
 *	being a '\001', followed by four bytes of (long extended)
 *	user id number, followed by a null byte.
 *	The actual user names are kept in a seperate field of the
 *	user structure, and is filled in upon demand later.
 *	Iteration through all users by low user id to high user id
 *	is done by just probing the table, which is gross.
 */
#define	USERKEY	'\001'
#define	ISPROCESS(tp)	(tp->p.name[0] && (tp->p.name[0] != USERKEY))
#define	ISUSER(tp)	(tp->p.name[0] && (tp->p.name[0] == USERKEY))

#define	TABDALLOP	500
struct 	allocbox{
	struct	allocbox	*nextalloc;
	cell			tabslots[TABDALLOP];
};

struct	allocbox	*allochead;	/*head of chunk list*/
struct	allocbox	*alloctail;	/*tail*/
struct	allocbox	*newbox;	/*for creating a new chunk*/
cell			*nexttab;	/*next table element that is free*/
int			tabsleft;	/*slots left in current chunk*/
int			ntabs;
/*
 *	Iterate through all symbols in the symbol table in declaration
 *	order.
 *	struct	allocbox	*allocwalk;
 *	cell			*sp, *ub;
 *
 *	sp points to the desired item, allocwalk and ub are there
 *	to make the iteration go.
 */

#define DECLITERATE(allocwalk, walkpointer, ubpointer) \
	for(allocwalk = allochead; \
	    allocwalk != 0; \
	    allocwalk = allocwalk->nextalloc) \
		for (walkpointer = &allocwalk->tabslots[0],\
		        ubpointer = &allocwalk->tabslots[TABDALLOP], \
		        ubpointer = ubpointer > ( (cell *)alloctail) \
				 ? nexttab : ubpointer ;\
		     walkpointer < ubpointer; \
		     walkpointer++ )

#define TABCHUNKS(allocwalk, tabptr, size) \
	for (allocwalk = allochead; \
	    allocwalk != 0; \
	    allocwalk = allocwalk->nextalloc) \
	    if ( \
		(tabptr = &allocwalk->tabslots[0]), \
		(size = \
		 (   (&allocwalk->tabslots[TABDALLOP]) \
		   > ((cell *)alloctail) \
		 ) \
		   ? (nexttab - tabptr) : TABDALLOP \
		), \
		1 \
	    )
#define	PROCESSITERATE(allocwalk, walkpointer, ubpointer) \
	DECLITERATE(allocwalk, walkpointer, ubpointer) \
	if (ISPROCESS(walkpointer))

#define	USERITERATE(allocwalk, walkpointer, ubpointer) \
	DECLITERATE(allocwalk, walkpointer, ubpointer) \
	if (ISUSER(walkpointer))
/*
 *	When we have to sort the segmented accounting table, we
 *	create a vector of sorted queues that is merged
 *	to sort the entire accounting table.
 */
struct chunkdesc   {
	cell	*chunk_tp;
	int	chunk_n;
};

/*
 *	Hash table segments and manager
 */
#define	NHASH	1103
struct hashdallop {
	int	h_nused;
	struct	hashdallop	*h_next;
	cell		*h_tab[NHASH];
};
struct	hashdallop	*htab;	/* head of the list */

double	treal;
double	tcpu;
double	tsys;
double	tio;
double	timem;
char	*sname;
double	ncom;
extern double	expacct();

/*
 *	usracct saves records of type Olduser.
 *	There is one record for every possible uid less than
 *	the largest uid seen in the previous usracct or in savacct.
 *
 *	In the old version of sa, uid's greater than 999 were not handled
 *	properly; this system will do that.
 */

#define	USRACCT "/var/adm/usracct"
#define	SAVACCT	"/var/adm/savacct"
#define	ACCT	"/var/adm/pacct"
#define USAGE_STR	"Usage: %s [-abcdDfijkKlmnorstu] [-v Number] [-S SaveFile] [-U UserFile] [File]\n"

char *usracct = USRACCT;
char *savacct = SAVACCT;

int	cellcmp();
cell	*junkp = 0;
/*
 *	If the threshold is zero after processing argv, it is set to 1
 */
int	thres = 0;	
int	htabinstall = 1;	 /* install the symbol */
uid_t	maxuser = (uid_t)0;
static uid_t getmaxuid();

/*	convert clicks to Kbytes (ac_mem, see kernel/bsd/kern_acct.c) */
#define	KCORE(clicks)	((double) (clicks << CLSIZELOG2)*(getpagesize()/1024))

extern	tcmp(), ncmp(), bcmp(), dcmp(), Dcmp(), kcmp(), Kcmp();
extern	double sum();

main(argc, argv)
	char **argv;
{
	FILE *ff;
	double ft;
	register struct	allocbox *allocwalk;
	register cell *tp, *ub;
	int i, j, size, nchunks, smallest;
	struct chunkdesc *chunkvector;

	int c;
	extern char *optarg;
	extern int optind, opterr;

	(void) setlocale (LC_ALL,"");
	catd = catopen(MF_SA,NL_CAT_LOCALE);

	maxuser = USERSLOP + getmaxuid();

	tabinit();
	cmp = tcmp;

	while ((c=getopt(argc, argv, "abcdDfijJkKlmnorstuv:S:U:")) != EOF) {
		switch(c) {

		case 'a':
			aflg++;
			break;

		case 'b':
			bflg++;
			cmp = bcmp;
			break;

		case 'c':
			cflg++;
			break;

		case 'd':
			dflg++;
			cmp = dcmp;
			break;

		case 'D':
			Dflg++;
			cmp = Dcmp;
			break;

		case 'f':
			fflg++;	
			break;

		case 'i':
			iflg++;
			break;

		case 'j':
			jflg++;
			break;

		case 'J':
			debug++;
			break;

		case 'k':
			kflg++;
			cmp = kcmp;
			break;

		case 'K':
			Kflg++;
			cmp = Kcmp;
			break;

		case 'l':
			lflg++;
			break;

		case 'm':
			mflg++;
			break;

		case 'n':
			nflg++;
			cmp = ncmp;
			break;

		case 'o':
			oflg++;
			break;

		case 'r':
			rflg++;
			break;

		case 's':
        		sflg++;
        		aflg++;
        		break;

		case 't':
        		tflg++;
        		break;

		case 'u':
        		uflg++;
        		break;

		case 'v':
        		vflg++;
        		thres = atoi( optarg );
        		break;

		case 'U':
			if ( optarg ) {
				usracct = optarg;
			} else {
				fprintf(stderr, MSGSTR(NOFILENM,
						"sa: Missing filename\n")); 
				exit(1); 
			}
			break;

		case 'S':
			if ( optarg ) {
				savacct = optarg;
			} else {
				fprintf(stderr, MSGSTR(NOFILENM,
						"sa: Missing filename\n")); 
				exit(1); 
			}
			break;

		default:
		    	fprintf(stderr, MSGSTR(SAUSAGE, USAGE_STR), argv[0]);
			exit(1);
		}
	}

	if (thres == 0)
		thres = 1;
	if (iflg==0)
		init();
	if (argc<=optind)
		doacct(ACCT);
	else 
		doacct(argv[optind]);
	if (uflg) {
		return(0);
	}

/*
 * cleanup pass
 * put junk together
 */

	if (vflg)
		strip();
	if(!aflg)
	PROCESSITERATE(allocwalk, tp, ub){
		for(j=0; j<NC; j++)
			if(tp->p.name[j] == '?')
				goto yes;
		if(tp->p.count != 1)
			continue;
	yes:
		if(junkp == 0)
			junkp = enter("***other");
		junkp->p.count += tp->p.count;
		junkp->p.realt += tp->p.realt;
		junkp->p.cput += tp->p.cput;
		junkp->p.syst += tp->p.syst;
		junkp->p.imem += tp->p.imem;
		junkp->p.io += tp->p.io;
		tp->p.name[0] = 0;
	}
	if (sflg) {
		(void)signal(SIGINT, SIG_IGN);
		if ((ff = fopen(usracct, "w")) != NULL) {
			register int i;
			register struct user	*up;
        		register struct hashdallop *hdp;
        		register cell *hp;

			for (hdp = htab; hdp != 0; hdp = hdp->h_next) {
				for ( i = 0; i < NHASH; i++ ) {
					if ( ((hp = hdp->h_tab[i]) != (cell *)0)
					    && (up = &(hp->u))) {
					fwrite((char  *)&(up->oldu),
						(size_t)sizeof(struct Olduser),(size_t)1,ff);
					}
				}
			}
			(void)fclose(ff);
		} else {
			fprintf(stderr, MSGSTR(CANTSAVE,
					"sa: Can't save to %s\n"), usracct);
			exit(1);
		}

		if ((ff = fopen(savacct, "w")) == NULL) {
			fprintf(stderr, MSGSTR(CANTSAVE,
					"sa: Can't save to %s\n"), savacct);
			exit(1);
		}
		PROCESSITERATE(allocwalk, tp, ub)
			fwrite((char *)&(tp->p), (size_t)sizeof(struct process), (size_t)1, ff);
		(void)fclose(ff);
		(void)creat(sname, 0644);
		(void)signal(SIGINT, SIG_DFL);
	}
/*
 * sort and print
 */
	if (mflg) {
		printmoney();
		return(0);
	}
	column(ncom, treal, tcpu, tsys, timem, tio);
	printf("\n");

	/*
	 *	the fragmented table is sorted by sorting each fragment
	 *	and then merging.
	 */
	nchunks = 0;
	TABCHUNKS(allocwalk, tp, size){
		qsort(tp, size, sizeof(cell), cellcmp);
		nchunks ++;
	}
	chunkvector = (struct chunkdesc *)calloc((unsigned)nchunks,
				(unsigned)sizeof(struct chunkdesc));
	nchunks = 0;
	TABCHUNKS(allocwalk, tp, size){
		chunkvector[nchunks].chunk_tp = tp;
		chunkvector[nchunks].chunk_n = size;
		nchunks++;
	}
	for(; nchunks; ){
		/*
		 *	Find the smallest element at the head of the queues.
		 */
		smallest = 0;
		for (i = 1; i < nchunks; i++){
			if (cellcmp(chunkvector[i].chunk_tp,
				chunkvector[smallest].chunk_tp) < 0)
					smallest = i;
		}
		tp = chunkvector[smallest].chunk_tp++;
		/*
		 *	If this queue is drained, drop the chunk count,
		 *	and readjust the queues.
		 */
		if (--chunkvector[smallest].chunk_n == 0){
			nchunks--;
			for (i = smallest; i < nchunks; i++)
				chunkvector[i] = chunkvector[i+1];
		}
		if (ISPROCESS(tp)){
			ft = tp->p.count;
			column(ft, tp->p.realt, tp->p.cput,
				tp->p.syst, tp->p.imem, tp->p.io);
			printf("   %.*s\n", NC, tp->p.name);
		}
	}	/* iterate to merge the lists */
	return(0);
}

printmoney()
{
	register struct user	*up;
        register struct hashdallop *hdp;
        register cell *hp;
	register int i;

	getnames();		/* fetches all of the names! */

	for (hdp = htab; hdp != 0; hdp = hdp->h_next) {
		for ( i = 0; i < NHASH; i++ ) {
			if (   ((hp = hdp->h_tab[i]) != (cell *)0) 
			    && ISUSER(hp)
			    && (up = &(hp->u))  
			    && up->us_cnt     ) {
				if (up->us_name[0])
					printf("%-*s", (NC<NAMELG?NC:NAMELG), up->us_name);
				else 
					printf("%-*d", (NC<NAMELG?NC:NAMELG), up->us_uid);
				printf(" %7u %9.2fcpu %10.0ftio %12.0fk*sec\n",
					up->us_cnt, 
					(jflg ? up->us_ctime :up->us_ctime/60.),
					up->us_io,
					up->us_imem);
			}
		}
	}
}

column(n, a, b, c, d, e)
	double n, a, b, c, d, e;
{

	printf(" %8.0f", n);
	if(cflg)
		printf(" %8.2f%%", n==ncom ? 100. : 100.*n/ncom);
	col(n, a, treal, "re");
	if (oflg)
		printf(" %11.2f%s", (b/((b+c)!=0.0?(b+c):1.0)), "u/s"); 
	else if(lflg) {
		col(n, b, tcpu, "u");
		col(n, c, tsys, "s");
	} else
		col(n, b+c, tcpu+tsys, "cpu");
	if(tflg)
		printf(" %8.1fre/cpu", a/((b+c)!=0.0?(b+c):1.0));
	if(dflg || !Dflg)
		printf(" %10.0favio", e/(n?n:1));
	else
		printf(" %10.0ftio", e);
	if (kflg || !Kflg)
		printf(" %10.0fk", d/((b+c)!=0.0?(b+c):1.0));
	else
		printf(" %10.0fk*sec", d);
}

col(n, a, m, cp)
	double n, a, m;
	char *cp;
{

	if(jflg)
		printf(" %11.2f%s", a, cp); 
	else
                printf(" %11.2f%s", a/60. > .01? a/60. : 0.0  , cp);
/*		printf(" %11.2f%s", a/60., cp); */

	if(cflg)
		printf(" %8.2f%%", a==m ? 100. : 100.*a/m);
}


doacct(f)
char *f;
{
	FILE *ff;
	double x, y, z;
	struct acct fbuf;
	register char *cp;
	register int c;
	register struct	user *up;
	register cell *tp;
	int	nrecords = 0;

	if (sflg && sname) {
		fprintf(stderr, MSGSTR(ONEFILE,"sa: Only 1 file with -s\n"));
		exit(1);
	}
	if (sflg)
		sname = f;
	if ((ff = fopen(f, "r"))==NULL) {
		fprintf(stderr, MSGSTR(CANTOPEN,"sa: Can't open %s\n"), f);
		exit(1);
	}
	while (fread((char *)&fbuf, (size_t)sizeof(fbuf), (size_t)1, ff) == 1) {
		if (debug) {
			/*
			if (++nrecords % 1000 == 0)
			*/
			printf("Input record from %s number %d\n",
				f, ++nrecords);
			printf ("comm:%-*.*s io: %d rw: %d mem: %d\n",NC, NC,
				fbuf.ac_comm,fbuf.ac_io,fbuf.ac_rw,fbuf.ac_mem);
			printf ("utime: %d stime: %d etime: %d btime: %d\n",
				fbuf.ac_utime,fbuf.ac_stime,fbuf.ac_etime,
				fbuf.ac_btime);
			printf ("tty: 0x%x uid: %d gid: %d flag: 0x%x stat: 0x%x\n",
				fbuf.ac_tty,fbuf.ac_uid,fbuf.ac_gid,
				fbuf.ac_flag,fbuf.ac_stat);
			printf ("\tExpanded values:\n");
			printf ("io: %f rw: %f mem %f\n",
				expacct(fbuf.ac_io), expacct(fbuf.ac_rw),
				KCORE(fbuf.ac_mem) );
			printf ("utime: %f stime: %f etime: %f\n\n",
				expacct(fbuf.ac_utime), expacct(fbuf.ac_stime),
				expacct(fbuf.ac_etime) );
		}
		for (cp = fbuf.ac_comm; *cp && cp < &fbuf.ac_comm[NC]; cp++)
			if (!isascii((int)*cp) || iscntrl((int)*cp))
				*cp = '?';
		if (cp == fbuf.ac_comm)
			*cp++ = '?';
		if (fbuf.ac_flag&AFORK) {
			if (cp >= &fbuf.ac_comm[NC])
				cp = &fbuf.ac_comm[NC-1];
			*cp++ = '*';
		}
		if (cp < &fbuf.ac_comm[NC])
			*cp = '\0';
		x = expacct(fbuf.ac_utime) + expacct(fbuf.ac_stime);
		y = KCORE(fbuf.ac_mem);
		z = expacct(fbuf.ac_io);
		if (uflg) {
			printf("%3d %6.2fcpu %8.0fk %10.0ftio %.*s\n",
			    fbuf.ac_uid, x, y, z, NC, fbuf.ac_comm);
			continue;
		}
		up = finduser(fbuf.ac_uid);
		if (up == 0)
			continue;	/* preposterous user id */
		up->us_cnt++;
		up->us_ctime += x;
		up->us_imem += (x * y);
		up->us_io += z;
		up->us_uid = fbuf.ac_uid;
		ncom += 1.0;

		tp = enter(fbuf.ac_comm);
		tp->p.imem += (x * y);
		timem += (x * y);
		tp->p.count++;
		x = expacct(fbuf.ac_etime);
		tp->p.realt += x;
		treal += x;
		x = expacct(fbuf.ac_utime);
		tp->p.cput += x;
		tcpu += x;
		x = expacct(fbuf.ac_stime);
		tp->p.syst += x;
		tsys += x;
		tp->p.io += z;
		tio += z;
	}
	(void)fclose(ff);
}

/*
 *	Generalized cell compare routine, to cast out users
 */
cellcmp(p1, p2)
	cell *p1, *p2;
{
	if (ISPROCESS(p1)){
		if (ISPROCESS(p2))
			return((*cmp)(p1, p2));
		return(-1);
	}
	if (ISPROCESS(p2))
		return(1);
	return(0);
}

ncmp(p1, p2)
	cell *p1, *p2;
{

	if(p1->p.count == p2->p.count)
		return(tcmp(p1, p2));
	if(rflg)
		return(p1->p.count - p2->p.count);
	return(p2->p.count - p1->p.count);
}

static
bcmp(p1, p2)
	cell *p1, *p2;
{
	double f1, f2;
	double sum();

	f1 = sum(p1)/p1->p.count;
	f2 = sum(p2)/p2->p.count;
	if(f1 < f2) {
		if(rflg)
			return(-1);
		return(1);
	}
	if(f1 > f2) {
		if(rflg)
			return(1);
		return(-1);
	}
	return(0);
}

Kcmp(p1, p2)
	cell *p1, *p2;
{

	if (p1->p.imem < p2->p.imem) {
		if(rflg)
			return(-1);
		return(1);
	}
	if (p1->p.imem > p2->p.imem) {
		if(rflg)
			return(1);
		return(-1);
	}
	return(0);
}

kcmp(p1, p2)
	cell *p1, *p2;
{
	double a1, a2;

	a1 = p1->p.imem / ((p1->p.cput+p1->p.syst)?(p1->p.cput+p1->p.syst):1);
	a2 = p2->p.imem / ((p2->p.cput+p2->p.syst)?(p2->p.cput+p2->p.syst):1);
	if (a1 < a2) {
		if(rflg)
			return(-1);
		return(1);
	}
	if (a1 > a2) {
		if(rflg)
			return(1);
		return(-1);
	}
	return(0);
}

dcmp(p1, p2)
	cell *p1, *p2;
{
	double a1, a2;

	a1 = p1->p.io / (p1->p.count?p1->p.count:1);
	a2 = p2->p.io / (p2->p.count?p2->p.count:1);
	if (a1 < a2) {
		if(rflg)
			return(-1);
		return(1);
	}
	if (a1 > a2) {
		if(rflg)
			return(1);
		return(-1);
	}
	return(0);
}

Dcmp(p1, p2)
	cell *p1, *p2;
{

	if (p1->p.io < p2->p.io) {
		if(rflg)
			return(-1);
		return(1);
	}
	if (p1->p.io > p2->p.io) {
		if(rflg)
			return(1);
		return(-1);
	}
	return(0);
}

tcmp(p1, p2)
	cell *p1, *p2;
{
	extern double sum();
	double f1, f2;

	f1 = sum(p1);
	f2 = sum(p2);
	if(f1 < f2) {
		if(rflg)
			return(-1);
		return(1);
	}
	if(f1 > f2) {
		if(rflg)
			return(1);
		return(-1);
	}
	return(0);
}

double sum(p)
	cell *p;
{

	if(p->p.name[0] == 0)
		return(0.0);
	return( p->p.cput + p->p.syst);
}

init()
{
	struct user userbuf;
	struct process	tbuf;
	register cell *tp;
	register struct user *up;
	int uid;
	FILE *f;

	if ((f = fopen(savacct, "r")) == NULL)
		goto gshm;
	while (fread((char *)&tbuf, (size_t)sizeof(struct process), (size_t)1, f) == 1) {
		tp = enter(tbuf.name);
		ncom += tbuf.count;
		tp->p.count = tbuf.count;
		treal += tbuf.realt;
		tp->p.realt = tbuf.realt;
		tcpu += tbuf.cput;
		tp->p.cput = tbuf.cput;
		tsys += tbuf.syst;
		tp->p.syst = tbuf.syst;
		tio += tbuf.io;
		tp->p.io = tbuf.io;
		timem += tbuf.imem;
		tp->p.imem = tbuf.imem;
	}
	(void)fclose(f);
 gshm:
	if ((f = fopen(usracct, "r")) == NULL)
		return;
	while (fread((char *)&(userbuf.oldu), (size_t)sizeof(struct Olduser), (size_t)1, f) == 1){
		if (userbuf.us_cnt){
			up = finduser(userbuf.us_uid);
			if (up == 0)
				continue;	/* preposterous user id */
			up->oldu = userbuf.oldu;
		}
	}
	(void)fclose(f);
}

strip()
{
	int c;
	register struct allocbox *allocwalk;
	register cell *tp, *ub, *jnkp;

	if (fflg)
		printf(MSGSTR(CATEGOR,"Categorizing commands used %d times or fewer as **junk**\n"),
			thres);
	jnkp = enter("**junk**");
	PROCESSITERATE(allocwalk, tp, ub){
		if (tp->p.name[0] && tp->p.count <= thres) {
			if (!fflg)
				printf("%.*s--", NC, tp->p.name);
			if (fflg || ((c=getchar())=='y')) {
				tp->p.name[0] = '\0';
				jnkp->p.count += tp->p.count;
				jnkp->p.realt += tp->p.realt;
				jnkp->p.cput += tp->p.cput;
				jnkp->p.syst += tp->p.syst;
				jnkp->p.imem += tp->p.imem;
				jnkp->p.io += tp->p.io;
			}
			if (!fflg)
				while (c && c!='\n')
					c = getchar();
		}
	}
}

static	char UserKey[NC + 2];

char *
makekey(uid)
	uid_t uid;
{
	sprintf(UserKey+1, "%04x", uid);
	UserKey[0] = USERKEY;
	return(UserKey);
}

struct user *
wasuser(uid)
	uid_t uid;
{
	struct user *tp;

	htabinstall = 0;
	tp = finduser(uid);
	htabinstall = 1;
	return(tp);
}

/*
 *	Only call this if you really want to insert it in the table!
 */
struct user *
finduser(uid)
	uid_t uid;
{

	if (uid > maxuser){
		fprintf(stderr, MSGSTR(BADID,
			"sa: Preposterous user id, %d: ignored\n"), uid); 
		return(0);
	}
	return((struct user*)enter(makekey(uid)));
}

/*
 *	Set the names of all users in the password file.
 *	We will later not print those that didn't do anything.
 */
getnames()
{
	register struct user *tp;
	register struct passwd *pw;
	struct passwd *getpwent();

	(void)setpwent();
	while (pw = getpwent()){
		/* use first name in passwd file for duplicate uid's */
		if ((tp = wasuser(pw->pw_uid)) != 0 && !isalpha((int)tp->us_name[0]))
			(void)strncpy(tp->us_name, pw->pw_name, NAMELG);
	}
	(void)endpwent();
}

static uid_t
getmaxuid()
{
	register struct user *tp;
	register struct passwd *pw;
	struct passwd *getpwent();
	uid_t maxuid = 0;

	(void)setpwent();
	while(pw = getpwent()){
		if (pw->pw_uid > maxuid)
			maxuid = pw->pw_uid;
	}
	(void)endpwent();
	return(maxuid);
}

tabinit()
{
	allochead = 0;
	alloctail = 0;
	nexttab = 0;
	tabsleft = 0;
	htab = 0;
	ntabs = 0;
	htaballoc();		/* get the first part of the hash table */
}

#define ALLOCQTY 	sizeof (struct allocbox)
cell *
taballoc()
{

	if (tabsleft == 0){
		newbox = (struct allocbox *)calloc((unsigned)1, 
						(unsigned)ALLOCQTY);
		tabsleft = TABDALLOP;
		nexttab = &newbox->tabslots[0];
		if (alloctail == 0){
			allochead = alloctail = newbox;
		} else {
			alloctail->nextalloc = newbox;
			alloctail = newbox;
		}
	}
	--tabsleft;
	++ntabs;
#ifdef DEBUG_IT
	if (ntabs % 100 == 0)
		printf("##Accounting table slot # %d\n", ntabs);
#endif DEBUG_IT
	return(nexttab++);
}

htaballoc()
{
	register struct hashdallop *new;
#ifdef DEBUG_IT
	static int ntables = 0;

	printf("%%%New hash table chunk allocated, number %d\n", ++ntables);
#endif DEBUG_IT
	new = (struct hashdallop *)calloc((unsigned)1, 
				(unsigned)sizeof (struct hashdallop));
	if (htab == 0)
		htab = new;
	else {		/* add AFTER the 1st slot */
		new->h_next = htab->h_next;
		htab->h_next = new;
	}
}

#define 	HASHCLOGGED	(NHASH / 2)
/*
 *	Lookup a symbol passed in as the argument.
 *
 *	We take pains to avoid function calls; this function
 *	is called quite frequently, and the calling overhead
 *	contributes significantly to the overall execution speed of sa.
 */
cell *
enter(name)
	char *name;	
{
	static int initialprobe;
	register cell **hp;
	register char *from, *to;
	register int len, nprobes;
	static struct hashdallop *hdallop, *emptyhd;
	static cell **emptyslot, **hp_ub;

	emptyslot = 0;
	for (nprobes = 0, from = name, len = 0;
	     *from && len < NC;
	     nprobes <<= 2, nprobes += *from++, len++)
		continue;
	nprobes += from[-1] << 5;
	nprobes %= NHASH;
	if (nprobes < 0)
		nprobes += NHASH;

	initialprobe = nprobes;
	for (hdallop = htab; hdallop != 0; hdallop = hdallop->h_next){
		for (hp = &(hdallop->h_tab[initialprobe]),
				nprobes = 1,
				hp_ub = &(hdallop->h_tab[NHASH]);
		     (*hp) && (nprobes < NHASH);
				hp += nprobes,
				hp -= (hp >= hp_ub) ? NHASH:0,
				nprobes += 2)
		{
			from = name;
			to = (*hp)->p.name;

			for (len = 0; (len<NC) && *from; len++)
				if (*from++ != *to++)
					goto nextprobe;
			if (len >= NC)		/*both are maximal length*/
				return(*hp);
			if (*to == 0)		/*assert *from == 0*/
				return(*hp);
	nextprobe: ;
		}
		if (*hp == 0 && emptyslot == 0 &&
		    hdallop->h_nused < HASHCLOGGED) {
			emptyslot = hp;
			emptyhd = hdallop;
		}
	}
	if (emptyslot == 0) {
		htaballoc();
		hdallop = htab->h_next;		/* aren't we smart! */
		hp = &hdallop->h_tab[initialprobe];
	} else {
		hdallop = emptyhd;
		hp = emptyslot;
	}
	if (htabinstall){
		*hp = taballoc();
		hdallop->h_nused++;
		for(len = 0, from = name, to = (*hp)->p.name; (len<NC); len++)
			if ((*to++ = *from++) == '\0')
				break;
		return(*hp);
	}
	return(0);
}


#ifdef DEBUG_IT
static
print_htab()
{
	struct hashdallop *hdp;
	register int i;
	register cell *hp;

	for (hdp = htab; hdp != 0; hdp = hdp->h_next) {
		for ( i = 0; i < NHASH; i++ ) {
			if( ((hp = hdp->h_tab[i]) != (cell *)0) && ISUSER(hp)) {
				printf("%s (%d)\n",hp->u.us_name,hp->u.us_uid);
			}
		}
	}
}
#endif
