/*
 * *********************************************************************
 * *                                                                   *
 * *       Modified by Digital Equipment Corporation, 1991, 1994       *
 * *                                                                   *
 * *       This file no longer matches the original Free Software      *
 * *       Foundation file.                                            *
 * *                                                                   *
 * *********************************************************************
 */
/*
 * HISTORY
 */
#ifndef lint
static char *rcsid = "@(#)$RCSfile: co.c,v $ $Revision: 1.1.7.2 $ (DEC) $Date: 1993/08/02 22:38:46 $";
#endif
/* Copyright (C) 1982, 1988, 1989 Walter Tichy
   Copyright 1990, 1991 by Paul Eggert
   Distributed under license by the Free Software Foundation, Inc.

This file is part of RCS.

RCS is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.

RCS is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with RCS; see the file COPYING.  If not, write to
the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.

Report problems and direct all questions to:

    rcs-bugs@cs.purdue.edu

*/

/*
 *                     RCS checkout operation
 */
/*****************************************************************************
 *                       check out revisions from RCS files
 *****************************************************************************
 */


/* 
 */




#include "rcsbase.h"

static char const *getancestor P((char const*,char const*));
static int buildjoin P((char const*));
static int preparejoin P((void));
static int rmlock P((struct hshentry const*));
static int rmworkfile P((void));
static void cleanup P((void));

static char const quietarg[] = "-q";

static char const *expandarg, *join, *suffixarg, *versionarg;
static char const *joinlist[joinlength]; /* revisions to be joined */
static FILE *neworkptr;
static int exitstatus;
static int forceflag;
static int lastjoin;			/* index of last element in joinlist  */
static int lockflag; /* -1 -> unlock, 0 -> do nothing, 1 -> lock */
static int mtimeflag;
static struct hshentries *gendeltas;	/* deltas to be generated	*/
static struct hshentry *targetdelta;	/* final delta to be generated	*/
static struct stat workstat;

mainProg(coId, "co", "@(#)$RCSfile: co.c,v $ $Revision: 1.1.7.2 $ (DEC) $Date: 1993/08/02 22:38:46 $")
{
	static char const cmdusage[] =
		"\nco usage: co -{flpqru}[rev] -ddate -jjoinlist -sstate -w[login] -Vn file ...";

	char *a, **newargv;
	char const *author, *date, *rev, *state;
	char const *joinfilename, *newdate, *neworkfilename;
	int changelock;  /* 1 if a lock has been changed, -1 if error */
	int expmode, r, tostdout, workstatstat;
	struct buf numericrev;	/* expanded revision number	*/
	char finaldate[datesize];

	setrid();
	author = date = rev = state = nil;
	bufautobegin(&numericrev);
	expmode = -1;
	suffixes = X_DEFAULT;
	tostdout = false;

	argc = getRCSINIT(argc, argv, &newargv);
	argv = newargv;
	while (a = *++argv,  0<--argc && *a++=='-') {
		switch (*a++) {

                case 'r':
		revno:
			if (*a) {
				if (rev) warn("redefinition of revision number");
				rev = a;
                        }
                        break;

		case 'f':
			forceflag=true;
			goto revno;

                case 'l':
			if (lockflag < 0) {
                                warn("-l overrides -u.");
                        }
			lockflag = 1;
                        goto revno;

                case 'u':
			if (0 < lockflag) {
                                warn("-l overrides -u.");
                        }
			lockflag = -1;
                        goto revno;

                case 'p':
			tostdout = true;
                        goto revno;

		case 'I':
			interactiveflag = true;
			goto revno;

                case 'q':
                        quietflag=true;
                        goto revno;

                case 'd':
			if (date)
				redefined('d');
			str2date(a, finaldate);
                        date=finaldate;
                        break;

                case 'j':
			if (*a) {
				if (join) redefined('j');
				join = a;
                        }
                        break;

		case 'M':
			mtimeflag = true;
			goto revno;

                case 's':
			if (*a) {
				if (state) redefined('s');
				state = a;
                        }
                        break;

                case 'w':
			if (author) redefined('w');
			if (*a)
				author = a;
			else
				author = getcaller();
                        break;

		case 'x':
			suffixarg = *argv;
			suffixes = a;
			break;

		case 'V':
			versionarg = *argv;
			setRCSversion(versionarg);
			break;

		case 'k':    /*  set keyword expand mode  */
			expandarg = *argv;
			if (0 <= expmode) redefined('k');
			if (0 <= (expmode = str2expmode(a)))
			    break;
			/* fall into */
                default:
			faterror("unknown option: %s%s", *argv, cmdusage);

                };
        } /* end of option processing */

	if (argc<1) faterror("no input file%s", cmdusage);
	if (tostdout)
#	    if text_equals_binary_stdio || text_work_stdio
		workstdout = stdout;
#	    else
		if (!(workstdout = fdopen(STDOUT_FILENO, FOPEN_W_WORK)))
		    efaterror("stdout");
#	    endif

        /* now handle all filenames */
        do {
	ffree();

	if (pairfilenames(argc, argv, lockflag?rcswriteopen:rcsreadopen, true, false)  <=  0)
		continue;

        /* now RCSfilename contains the name of the RCS file, and finptr
	 * points at it.  workfilename contains the name of the working file.
	 * Also, RCSstat has been set.
         */
	diagnose("%s  -->  %s\n", RCSfilename,tostdout?"stdout":workfilename);

	workstatstat = -1;
	if (tostdout) {
		neworkfilename = 0;
		neworkptr = workstdout;
	} else {
		workstatstat = stat(workfilename, &workstat);
		neworkfilename = makedirtemp(workfilename, 1);
		if (!(neworkptr = fopen(neworkfilename, FOPEN_W_WORK))) {
			if (errno == EACCES)
				error("%s: parent directory isn't writable",
					workfilename
				);
			else
				eerror(neworkfilename);
			continue;
		}
	}

        gettree();  /* reads in the delta tree */

        if (Head==nil) {
                /* no revisions; create empty file */
		diagnose("no revisions present; generating empty revision 0.0\n");
		Ozclose(&fcopy);
		if (workstatstat == 0)
			if (!rmworkfile()) continue;
		changelock = 0;
		newdate = 0;
                /* Can't reserve a delta, so don't call addlock */
        } else {
                if (rev!=nil) {
                        /* expand symbolic revision number */
			if (!expandsym(rev, &numericrev))
                                continue;
		} else
			switch (lockflag<0 ? findlock(false,&targetdelta) : 0) {
			    default:
				continue;
			    case 0:
				bufscpy(&numericrev, Dbranch?Dbranch:"");
				break;
			    case 1:
				bufscpy(&numericrev, targetdelta->num);
				break;
			}
                /* get numbers of deltas to be generated */
		if (!(targetdelta=genrevs(numericrev.string,date,author,state,&gendeltas)))
                        continue;
                /* check reservations */
		changelock =
			lockflag < 0 ?
				rmlock(targetdelta)
			: lockflag == 0 ?
				0
			:
				addlock(targetdelta);

		if (
			changelock < 0 ||
			changelock && !checkaccesslist() ||
			!dorewrite(lockflag, changelock)
		)
			continue;

		if (0 <= expmode)
			Expand = expmode;
		if (0 < lockflag  &&  Expand == VAL_EXPAND) {
			error("cannot combine -kv and -l");
			continue;
		}

                if (join && !preparejoin()) continue;

		diagnose("revision %s%s\n",targetdelta->num,
			 0<lockflag ? " (locked)" :
			 lockflag<0 ? " (unlocked)" : "");

		/* Prepare to remove old working file if necessary.  */
		if (workstatstat == 0)
                        if (!rmworkfile()) continue;

                /* skip description */
                getdesc(false); /* don't echo*/

		locker_expansion = 0 < lockflag;
		joinfilename = buildrevision(
			gendeltas, targetdelta,
			join&&tostdout ? (FILE*)0 : neworkptr,
			Expand!=OLD_EXPAND
		);
#		if !large_memory
			if (fcopy == neworkptr)
				fcopy = 0;  /* Don't close it twice.  */
#		endif
		if_advise_access(changelock && gendeltas->first!=targetdelta,
			finptr, MADV_SEQUENTIAL
		);

		if (!donerewrite(changelock))
			continue;

		newdate = targetdelta->date;
		if (join) {
			newdate = 0;
			if (!joinfilename) {
				aflush(neworkptr);
				joinfilename = neworkfilename;
			}
			if (!buildjoin(joinfilename))
				continue;
		}
        }
	if (!tostdout) {
	    r = 0;
	    if (mtimeflag && newdate) {
		if (!join)
		    aflush(neworkptr);
		r = setfiledate(neworkfilename, newdate);
	    }
	    if (r == 0) {
		ignoreints();
		r = chnamemod(&neworkptr, neworkfilename, workfilename,
		  WORKMODE(RCSstat.st_mode,
		    !(Expand==VAL_EXPAND || lockflag<=0&&StrictLocks)
		  )
		);
		keepdirtemp(neworkfilename);
		restoreints();
	    }
	    if (r != 0) {
		eerror(workfilename);
		error("see %s", neworkfilename);
		continue;
	    }
	    diagnose("done\n");
	}
        } while (cleanup(),
                 ++argv, --argc >=1);

	tempunlink();
	Ofclose(workstdout);
	exitmain(exitstatus);

}       /* end of main (co) */

	static void
cleanup()
{
	if (nerror) exitstatus = EXIT_FAILURE;
	Izclose(&finptr);
	Ozclose(&frewrite);
#	if !large_memory
		if (fcopy!=workstdout) Ozclose(&fcopy);
#	endif
	if (neworkptr!=workstdout) Ozclose(&neworkptr);
	dirtempunlink();
}

#if lint
#	define exiterr coExit
#endif
	exiting void
exiterr()
{
	dirtempunlink();
	tempunlink();
	_exit(EXIT_FAILURE);
}


/*****************************************************************
 * The following routines are auxiliary routines
 *****************************************************************/

	static int
rmworkfile()
/* Function: prepares to remove workfilename, if it exists, and if
 * it is read-only.
 * Otherwise (file writable):
 *   if !quietmode asks the user whether to really delete it (default: fail);
 *   otherwise failure.
 * Returns true if permission is gotten.
 */
{
	if (workstat.st_mode&(S_IWUSR|S_IWGRP|S_IWOTH) && !forceflag) {
	    /* File is writable */
	    if (!yesorno(false, "writable %s exists%s; remove it? [ny](n): ",
			workfilename,
			myself(workstat.st_uid) ? "" : ", and you do not own it"
	    )) {
		error(!quietflag && ttystdin()
			? "checkout aborted"
			: "writable %s exists; checkout aborted", workfilename);
		return false;
            }
        }
	/* Actual unlink is done later by caller. */
	return true;
}


	static int
rmlock(delta)
	struct hshentry const *delta;
/* Function: removes the lock held by caller on delta.
 * Returns -1 if someone else holds the lock,
 * 0 if there is no lock on delta,
 * and 1 if a lock was found and removed.
 */
{       register struct lock * next, * trail;
	char const *num;
        struct lock dummy;
        int whomatch, nummatch;

        num=delta->num;
        dummy.nextlock=next=Locks;
        trail = &dummy;
        while (next!=nil) {
		whomatch = strcmp(getcaller(), next->login);
                nummatch=strcmp(num,next->delta->num);
                if ((whomatch==0) && (nummatch==0)) break;
			/*found a lock on delta by caller*/
                if ((whomatch!=0)&&(nummatch==0)) {
                    error("revision %s locked by %s; use co -r or rcs -u",num,next->login);
                    return -1;
                }
                trail=next;
                next=next->nextlock;
        }
        if (next!=nil) {
                /*found one; delete it */
                trail->nextlock=next->nextlock;
                Locks=dummy.nextlock;
                next->delta->lockedby=nil; /* reset locked-by */
                return 1; /*success*/
        } else  return 0; /*no lock on delta*/
}




/*****************************************************************
 * The rest of the routines are for handling joins
 *****************************************************************/


	static char const *
addjoin(joinrev)
	char *joinrev;
/* Add joinrev's number to joinlist, yielding address of char past joinrev,
 * or nil if no such revision exists.
 */
{
	register char *j;
	register struct hshentry const *d;
	char terminator;
	struct buf numrev;
	struct hshentries *joindeltas;

	j = joinrev;
	for (;;) {
	    switch (*j++) {
		default:
		    continue;
		case 0:
		case ' ': case '\t': case '\n':
		case ':': case ',': case ';':
		    break;
	    }
	    break;
	}
	terminator = *--j;
	*j = 0;
	bufautobegin(&numrev);
	d = 0;
	if (expandsym(joinrev, &numrev))
	    d = genrevs(numrev.string,(char*)nil,(char*)nil,(char*)nil,&joindeltas);
	bufautoend(&numrev);
	*j = terminator;
	if (d) {
		joinlist[++lastjoin] = d->num;
		return j;
	}
	return nil;
}

	static int
preparejoin()
/* Function: Parses a join list pointed to by join and places pointers to the
 * revision numbers into joinlist.
 */
{
	register char const *j;

        j=join;
        lastjoin= -1;
        for (;;) {
                while ((*j==' ')||(*j=='\t')||(*j==',')) j++;
                if (*j=='\0') break;
                if (lastjoin>=joinlength-2) {
                        error("too many joins");
                        return(false);
                }
		if (!(j = addjoin(j))) return false;
                while ((*j==' ') || (*j=='\t')) j++;
                if (*j == ':') {
                        j++;
                        while((*j==' ') || (*j=='\t')) j++;
                        if (*j!='\0') {
				if (!(j = addjoin(j))) return false;
                        } else {
                                error("join pair incomplete");
                                return false;
                        }
                } else {
                        if (lastjoin==0) { /* first pair */
                                /* common ancestor missing */
                                joinlist[1]=joinlist[0];
                                lastjoin=1;
                                /*derive common ancestor*/
				if (!(joinlist[0] = getancestor(targetdelta->num,joinlist[1])))
                                       return false;
                        } else {
                                error("join pair incomplete");
                                return false;
                        }
                }
        }
        if (lastjoin<1) {
                error("empty join");
                return false;
        } else  return true;
}



	static char const *
getancestor(r1, r2)
	char const *r1, *r2;
/* Yield the common ancestor of r1 and r2 if successful, nil otherwise.
 * Work reliably only if r1 and r2 are not branch numbers.
 */
{
	static struct buf t1, t2;

	unsigned l1, l2, l3;
	char const *r;

	l1 = countnumflds(r1);
	l2 = countnumflds(r2);
	if ((2<l1 || 2<l2)  &&  cmpnum(r1,r2)!=0) {
	    /* not on main trunk or identical */
	    l3 = 0;
	    while (cmpnumfld(r1, r2, l3+1)==0 && cmpnumfld(r1, r2, l3+2)==0)
		l3 += 2;
	    /* This will terminate since r1 and r2 are not the same; see above. */
	    if (l3==0) {
		/* no common prefix; common ancestor on main trunk */
		VOID partialno(&t1, r1, l1>2 ? (unsigned)2 : l1);
		VOID partialno(&t2, r2, l2>2 ? (unsigned)2 : l2);
		r = cmpnum(t1.string,t2.string)<0 ? t1.string : t2.string;
		if (cmpnum(r,r1)!=0 && cmpnum(r,r2)!=0)
			return r;
	    } else if (cmpnumfld(r1, r2, l3+1)!=0)
			return partialno(&t1,r1,l3);
	}
	error("common ancestor of %s and %s undefined", r1, r2);
	return nil;
}



	static int
buildjoin(initialfile)
	char const *initialfile;
/* Function: merge pairs of elements in joinlist into initialfile
 * If workstdout is set, copy result to stdout.
 * All unlinking of initialfile, rev2, and rev3 should be done by tempunlink().
 */
{
	struct buf commarg;
	struct buf subs;
	char const *rev2, *rev3;
        int i;
	char const *cov[10], *mergev[12];
	char const **p;

	bufautobegin(&commarg);
	bufautobegin(&subs);
	rev2 = maketemp(0);
	rev3 = maketemp(3); /* buildrevision() may use 1 and 2 */

	cov[0] = nil;
	/* cov[1] setup below */
	cov[2] = CO;
	/* cov[3] setup below */
	p = &cov[4];
	if (expandarg) *p++ = expandarg;
	if (suffixarg) *p++ = suffixarg;
	if (versionarg) *p++ = versionarg;
	*p++ = quietarg;
	*p++ = RCSfilename;
	*p = nil;

	mergev[0] = nil;
	mergev[1] = nil;
	mergev[2] = MERGE;
	mergev[3] = mergev[5] = "-L";
	/* rest of mergev setup below */

        i=0;
        while (i<lastjoin) {
                /*prepare marker for merge*/
                if (i==0)
			bufscpy(&subs, targetdelta->num);
		else {
			bufscat(&subs, ",");
			bufscat(&subs, joinlist[i-2]);
			bufscat(&subs, ":");
			bufscat(&subs, joinlist[i-1]);
		}
		diagnose("revision %s\n",joinlist[i]);
		bufscpy(&commarg, "-p");
		bufscat(&commarg, joinlist[i]);
		cov[1] = rev2;
		cov[3] = commarg.string;
		if (runv(cov))
			goto badmerge;
		diagnose("revision %s\n",joinlist[i+1]);
		bufscpy(&commarg, "-p");
		bufscat(&commarg, joinlist[i+1]);
		cov[1] = rev3;
		cov[3] = commarg.string;
		if (runv(cov))
			goto badmerge;
		diagnose("merging...\n");
		mergev[4] = subs.string;
		mergev[6] = joinlist[i+1];
		p = &mergev[7];
		if (quietflag) *p++ = quietarg;
		if (lastjoin<=i+2 && workstdout) *p++ = "-p";
		*p++ = initialfile;
		*p++ = rev2;
		*p++ = rev3;
		*p = nil;
		switch (runv(mergev)) {
		    case DIFF_FAILURE: case DIFF_SUCCESS:
			break;
		    default:
			goto badmerge;
		}
                i=i+2;
        }
	bufautoend(&commarg);
	bufautoend(&subs);
        return true;

    badmerge:
	nerror++;
	bufautoend(&commarg);
	bufautoend(&subs);
	return false;
}
