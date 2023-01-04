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
static char	*sccsid = "@(#)$RCSfile: doname.c,v $ $Revision: 4.2.3.2 $ (DEC) $Date: 1992/01/29 17:59:14 $";
#endif 
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0
 */
/*
 * Copyright (c) 1989 Carnegie-Mellon University
 * All rights reserved.  The CMU software License Agreement specifies
 * the terms and conditions for use and redistribution.
 *
 * doname.c	4.9 (Berkeley) 87/06/18";
 *
 */

#include "defs.h"

#ifndef _BLD
#include "make_msg.h"
extern nl_catd  catd;
#define MSGSTR(Num, Str) catgets(catd, MS_MAKE, Num, Str)
#include <NLctype.h>
#else
#define MSGSTR(Num, Str) Str
#include <ctype.h>
#endif

FSTATIC int docom1();
FSTATIC tsetvars();
FSTATIC int cmds();
FSTATIC expand();

extern char *ctime();
#ifndef strcpy
extern char *strcpy();
#endif

/*
 * BASIC PROCEDURE.  RECURSIVE.
 *
 * p->done = 0   don't know what to do yet
 * p->done = 1   file in process of being updated
 * p->done = 2   file already exists in current state
 * p->done = 3   file make failed
 */
int
doname(p, reclev, tval, ochain)
	register struct nameblock *p;
	int reclev;
	time_t *tval;
	struct chain **ochain;
{
	register struct depblock *q;
	register struct lineblock *lp;
	int errstat, okdel1, didwork, found;
	time_t td, td1, tdep, ptime, ptime1;
	struct depblock *qtemp, *suffp;
	struct nameblock *p1, *p2;
	struct lineblock *implcom, *explcom, *lp2;
	char prefix[BUFSIZ], pct[BUFSIZ], concsuff[20]; /* XXX */
	char *pnamep, *cp, *setimpl, *inobj;
	struct chain *achain, *qchain, *cochain;
	static char abuf[QBUFMAX];
	extern time_t prestime();
	extern struct depblock *srchdir();
	extern struct chain *appendq();
	extern char *mkqlist();

	if (p == 0) {
		*tval = 0;
		return 0;
	}

	if (dbgflag)
		printf(MSGSTR(DONAME,"doname(%s, %d)\n"), p->namep, reclev);

	if (p->done) {
		/*
		 * if we want to check-out RCS files, and we have previously
		 * determined that we can, then append it to the previous
		 * level's cochain.
		 */
		if (p->RCSnamep)
			*ochain = appendq(*ochain, p->namep);
		*tval = p->modtime;
		if (dbgflag)
			printf(MSGSTR(TIME1,"TIME1(%s)=%s"),
				       p->namep, ctime(tval)+4);
		return p->done == 3;
	}

	cochain = 0;
	errstat = 0;
	tdep = 0;
	implcom = 0;
	explcom = 0;
	ptime = p->modtime = exists(p, ochain, 1);
	ptime1 = 0;
	didwork = NO;
	p->done = 1;	/* avoid infinite loops */
	achain = 0;
	qchain = 0;

	/*
	 * Perform runtime dependency translations.
	 */
	if (p->rundep == 0) {
		tsetvars(p->tmacp);
		okdel1 = okdel;
		okdel = NO;
		setvar("@", p->namep, 1);
		dynamicdep(p, pct);
		setvar("@", (char *) 0, 1);
		okdel = okdel1;
	}

	/*
	 * Expand any names that have embedded metacharaters. Must be
	 * done after dynamic dependencies because the dyndep symbols
	 * ($(*D)) may contain shell meta characters.
	 */
	for (lp = p->linep; lp; lp = lp->nxtlineblock)
		for (q = lp->depp; q; q = qtemp) {
			qtemp = q->nxtdepblock;
			expand(q);
		}

	/*
	 * make sure all dependents are up to date
	 */
	for (lp = p->linep; lp; lp = lp->nxtlineblock) {
		td = 0;
		for (q = lp->depp; q; q = q->nxtdepblock) {
			if (q->depname == 0)
				continue;
			errstat += doname(q->depname, reclev+1, &td1, &cochain);
                        if(dbgflag)
                            printf(MSGSTR(TIME1,"TIME1(%s)=%s"), q->depname->namep, ctime(&td1));
			if (td1 > td)
				td = td1;
			achain = appendq(achain, q->depname->namep);
			if (ptime < td1)
				qchain = appendq(qchain, q->depname->namep);
		}
		if (p->septype != SOMEDEPS) {
			if (lp->shp) {
				if (explcom)
					fprintf(stderr,
						MSGSTR(CMDLINES,
						       "Too many command lines for `%s'\n"), p->namep);
				else
					explcom = lp;
			}
			if (td > tdep)
				tdep = td;
			continue;
		}
		if (lp->shp && (ptime < td || (ptime == 0 && td == 0) || lp->depp == 0)) {
			okdel1 = okdel;
			okdel = NO;  /* why? */
			if (!questflag) {
				tsetvars(p->tmacp);
				if (cochain)
					co(cochain);
				setvar("@", p->namep, 1);
				setvar("%", pct, 1);
				setvar(">", strcpy(abuf, mkqlist(achain)), 1);
				setvar("?", mkqlist(qchain), 1);
				errstat += docom(lp);
				setvar("@", (char *) 0, 1);
			}
			achain = 0;
			qchain = 0;
			cochain = 0;
			okdel = okdel1;
			ptime1 = prestime();
			didwork = YES;
		}
	}

	found = 0;

	/*
	 * Look for implicit dependents, using suffix rules
	 */
	setimpl = 0;
	for (lp = sufflist; lp && !found; lp = lp->nxtlineblock)
	for (suffp = lp->depp; suffp; suffp = suffp->nxtdepblock) {
		if (suffp->depname == 0)
			continue;
		pnamep = suffp->depname->namep;
		if (!suffix(p->namep, pnamep, prefix))
			continue;
		if (dosrch(prefix, pnamep, &p1, &p2)) {
			found = 1;
			errstat += doname(p2, reclev+1, &td, &cochain);
			if (td > tdep)
				tdep = td;
			achain = appendq(achain, p2->namep);
			if (ptime < td)
				qchain = appendq(qchain, p2->namep);
                               if(dbgflag)
                               {
                                   time_t foo= exists(p);
                                   blprt(reclev);
                                   printf(MSGSTR(TIME3,"TIME3(%s)=%s")
                                   ,p->namep,ctime(&foo));

                                   if(td > foo)
                                   {

                                       blprt(reclev);
                                       printf(MSGSTR(INFRULE,"Building %s using inference rule %s, because it is out of date relative to %s\n"), p->namep, concsuff, p2 ->namep);
                                   }
                               }
			for (lp2 = p1->linep; lp2; lp2 = lp2->nxtlineblock)
				if (lp2->shp) {
					implcom = lp2;
					setimpl = p2->namep;
					break;
				}
			break;
		}
		setvar("*", (cp = rindex(prefix, '/')) ? cp+1 : prefix, 1);
	}

	/*
	 * look for a single suffix type rule.
	 * only possible if no shell rules are found, and nothing
	 * has been done so far (previously, `make' would exit
	 * with 'Don't know how to make ...' message).
	 */
	if (!found) {
		for (lp = p->linep; lp; lp = lp->nxtlineblock)
			if (lp->shp)
				break;
		if (lp == 0) {
			if (dbgflag)
				printf(MSGSTR(SUFFIX,
					"Looking for Single suffix rule.\n"));
			(void) concat(p->namep, "", prefix);
			if (dosrch(prefix, "", &p1, &p2)) {
				errstat += doname(p2, reclev+1, &td, &cochain);
				if (td > tdep)
					tdep = td;
				achain = appendq(achain, p2->namep);
				if (ptime < td)
					qchain = appendq(qchain, p2->namep);
				for (lp2 = p1->linep; lp2; lp2 = lp2->nxtlineblock)
					if (lp2->shp) {
						implcom = lp2;
						setimpl = p2->namep;
						break;
					}
			}
		}
	}

	inobj = 0;

	/*
	 * if we would normally say 'is up to date' and the target
	 * should have a version in the object directory, then we
	 * set things up so that an out of date object version will
	 * fire the "inobjdir" rule.
	 */
	if (!noconfig && tdep == 0 && errstat == 0
	&& explcom == 0 && implcom == 0 && ptime != 0
	&& didwork == NO && (lp = isdependent(p->namep, inobjdir))) {
		if (p->alias == 0) {
			if (dbgflag)
				printf(MSGSTR(SOURCE,"Looking for source version.\n"));
			tdep = exists(p, ochain, 0);
		} else {
			struct stat sbuf;

			if (dbgflag)
				printf(MSGSTR(OBJECT,"Looking for object version.\n"));
			tdep = ptime;
			ptime = (stat(p->namep, &sbuf) == -1)
					? 0 : sbuf.st_mtime;
		}
		if (inobjdir->septype == SOMEDEPS) {
			if (lp->shp) {
				implcom = lp;
				inobj = p->namep;
			}
		} else {
			for (lp = inobjdir->linep; lp; lp = lp->nxtlineblock)
				if (lp->shp) {
					implcom = lp;
					inobj = p->namep;
					break;
				}
		}
	}

	if (dbgflag && cochain)
		printf(MSGSTR(CHECKOUT,"CO(%s): %s\n"),
		       p->namep, mkqlist(cochain));
	if (p->RCSnamep && (explcom || (implcom && inobj == 0))) {
		if (!keepgoing)
			fatal(MSGSTR(RCSFILRUL,"%s has both an RCS file and rules"), p->namep);
		errstat++;
		printf(MSGSTR(RCSRULES,"%s has both an RCS file and rules\n"),
		       p->namep);
	}

	if (p->alias && (explcom || (implcom && inobj == 0))) {
		/*
		 * we could support shadow object trees by allowing
		 * aliases that "originate" from the OBJECTDIR list.
		 * the alias would only need to be cleared if the
		 * target was found to be out of date.
		 */
		if (dbgflag)
			fprintf(stderr,MSGSTR(TARTIGN, 
				"Warning: target `%s' found as `%s' ignored\n"),
				p->namep, p->alias);
		p->alias = 0;
		ptime = 0;
	} else if (p->alias)
		p->aliasok = YES;
	if (errstat == 0 && (ptime < tdep || (ptime == 0 && tdep == 0))) {
		ptime = (tdep > 0 ? tdep : prestime());
		tsetvars(p->tmacp);
		if (cochain)
			co(cochain);
		if (setimpl) {
			setvar("*", prefix, 1);
			setvar("<", setimpl, 1);
		} else if (inobj)
			setvar("<", inobj, 1);
		setvar("@", p->namep, 1);
		setvar("%", pct, 1);
		setvar(">", strcpy(abuf, mkqlist(achain)), 1);
		setvar("?", mkqlist(qchain), 1);
		if (explcom)
			errstat += docom(explcom);
		else if (implcom)
			errstat += docom(implcom);
		else if ((p->septype != SOMEDEPS && !botchflag)
				|| (p->septype == 0 && botchflag)) {
			if (p1 = srchname(".DEFAULT")) {
				setvar("<", p->namep, 1);
				for (lp2 = p1->linep; lp2; lp2 = lp2->nxtlineblock)
					if (lp2->shp) {
						implcom = lp2;
						errstat += docom(implcom);
						break;
					}
			} else if (keepgoing) {
				printf(MSGSTR(DONTMAKE,
				      "Don't know how to make %s\n"), p->namep);
				++errstat;
			} else
				fatal(MSGSTR(CANTMKE,
					"Don't know how to make %s"), p->namep);
		}
		setvar("@", (char *) 0, 1);
		if (cmds(explcom) || cmds(implcom))
			if (noexflag || (ptime = exists(p, (struct chain **) 0, 1)) == 0)
				ptime = prestime();
	}
	
	else if (errstat && reclev == 0)
		printf(MSGSTR(NOREMAKE,"`%s' not remade because of errors\n"),
		       p->namep);

	else if (!questflag && reclev == 0 && didwork == NO)
		printf(MSGSTR(NOTTODATE,"`%s' is up to date.\n"), p->namep);

	if (inobj)
		p->alias = 0;
	if (questflag && reclev == 0)
		quit(ndocoms > 0 ? 1 : 0);
	p->done = (errstat ? 3 : 2);
	if (ptime1 > ptime)
		ptime = ptime1;
	p->modtime = ptime;
	*tval = ptime;
	if (dbgflag)
		printf(MSGSTR(TIME1,"TIME1(%s)=%s"), p->namep, ctime(tval)+4);
	return errstat;
}


dosrch(prefix, pnamep, p1p, p2p)
	char *prefix, *pnamep;
	struct nameblock **p1p, **p2p;
{
	register struct depblock *suffp1;
	register struct lineblock *lp1, *lp2;
	char srcname[BUFSIZ], temp[BUFSIZ];  /* XXX */
	char concsuff[20]; /* XXX */
	char *p1namep;

	(void) srchdir(concat(prefix, "*", temp), NO, (struct depblock *) 0);
	if (coflag)
		srchRCS(temp);
	srchmachine(temp);
	for (lp1 = sufflist; lp1; lp1 = lp1->nxtlineblock)
	for (suffp1 = lp1->depp; suffp1; suffp1 = suffp1->nxtdepblock) {
		if (suffp1->depname == 0)
			continue;
		/*
		 * only use a single suffix if it really has rules
		 */
		if (*pnamep == 0) {
			for (lp2 = suffp1->depname->linep; lp2; lp2 = lp2->nxtlineblock)
				if (lp2->depp || lp2->shp)
					break;
			if (lp2 == 0)
				continue;
		}
		p1namep = suffp1->depname->namep;
		if ((*p1p = srchname(concat(p1namep, pnamep, concsuff)))
		&& (*p2p = srchname(concat(prefix, p1namep, srcname))))
			return 1;
	}
	return 0;
}


int
docom(p)
	struct lineblock *p;
{
	register struct shblock *q;
	register char *s;
	struct nameblock *np;
	int ign, nopr;
	char string[OUTMAX], string2[OUTMAX];  /* XXX */

	if (questflag) {
		if (!nocmds)
			++ndocoms;
		return 0;
	}

	if (touchflag) {
		if (nocmds)
			return 0;
		s = varptr("@")->varval;
		if (!silflag)
			printf(MSGSTR(NOFLAG,"%stouch %s\n"),
			   (noexflag ? "" : prompt), s);
		if (!noexflag)
			touch(NO, s);
		return 0;
	}

	if (p->lockp)
		printf("LOCK: %s\n", p->lockp->namep);  /* XXX */
	for (q = p->shp; q; q = q->nxtshblock) {
		if (nocmds && !q->exok)
			continue;
		(void) subst(q->shbp, string2);
		if (*(s = string2) == '[') {
			while (*++s != ']')
				if (*s == 0)
					fatal("bad lock name: %s", string2+1);
			*s++ = 0;
			np = makename(string2+1);
			printf("LOCK: %s\n", np->namep);  /* XXX */
		}
		fixname(s, string);
		ign = ignerr;
		nopr = NO;
		for (s = string; *s == '-' || *s == '@'; ++s)
			if (*s == '-')
				ign = YES;
			else
				nopr = YES;
		if (docom1(s, ign, nopr, q->exok) && !ign)
			return 1;
	}
	return 0;
}


FSTATIC int
docom1(comstring, nohalt, noprint, exok)
	register char *comstring;
	int nohalt, noprint, exok;
{
	if (!silflag && (!noprint || (noexflag && *comstring)))
		printf("%s%s\n", (noexflag ? "" : prompt),
			(*comstring ? comstring : "(null command)"));
	if (*comstring == 0 || (noexflag && !exok))
		return 0;
	return dosys(comstring, nohalt, exok);
}


FSTATIC
tsetvars(t)
	struct tmacblock *t;
{
	register struct tmacblock *p;

	for (p = t; p; p = p->nxttmacblock) {
		if (dbgflag)
			printf("%s\n", p->tmacbp);
		(void) eqsign(p->tmacbp, (char *) 0, 1);
	}
}


FSTATIC int
cmds(p)
	struct lineblock *p;
{
	register struct shblock *q;

	if (p == 0)
		return 0;
	for (q = p->shp; q; q = q->nxtshblock)
		if (*q->shbp)
			return 1;
	return 0;
}


/*
 * If there are any Shell meta characters in the name,
 * expand into a list, after searching directory
 */
FSTATIC
expand(q)
	register struct depblock *q;
{
	register char *s;
	char *s1;
	struct depblock *p;
	extern struct depblock *srchdir();

	if (q->depname == 0)
		return;
	s1 = s = q->depname->namep;
	for (;;) {
		switch (*s++) {
		case 0:
			return;
		case '*':
		case '?':
		case '[':
			if (p = srchdir(s1, YES, q->nxtdepblock)) {
				q->nxtdepblock = p;
				q->depname = 0;
			}
			return;
		}
	}
}

/*
 *      PRINT n BLANKS WHERE n IS THE CURRENT RECURSION LEVEL.
 */
blprt(n)
register int n;
{
        while(n--)
                printf(MSGSTR(SPACES,"  "));
}
