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
#if !defined(lint) && !defined(_NOIDENT)
static char rcsid[] = "@(#)$RCSfile: optim.c,v $ $Revision: 4.2.9.2 $ (DEC) $Date: 1993/08/02 18:20:45 $";
#endif
/*
 * HISTORY
 */
/*
 * OSF/1 Release 1.0
 */
/* 
 * COMPONENT_NAME: CMDMAILX optim.c
 * 
 * FUNCTIONS: MSGSTR, arpafix, best, inithost, little, minit, mlook, 
 *            mstash, mtype, name, netkind, netlook, netmap, netname, 
 *            nettype, ntype, optiboth, optim, optim1, optimex, 
 *            optimimp, prefer, revarpa, rpair, stradd, xlocate, 
 *            yyinit, yylex 
 *
 * ORIGINS: 10  26  27 
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 *	optim.c      5.5 (Berkeley) 11/2/85
 */

/*
 * Mail -- a program for sending and receiving mail.
 *
 * Network name modification routines.
 */

#include "rcv.h"
#include "configdefs.h"
#include <ctype.h>

#include "Mail_msg.h" 
extern nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_MAILX,n,s) 

char *unuucp();		/* SVID-2 */
char *makeremote();

char host[256];		/* moved from inithost() as static - for SVID-2 */
char domain[128];	/* added for SVID-2 */

/*
 * Map a name into the correct network "view" of the
 * name.  This is done by prepending the name with the
 * network address of the sender, then optimizing away
 * nonsense.
 */

char *
netmap(name, from)
	char name[], from[];
{
	char nbuf[BUFSIZ], ret[BUFSIZ];
	register char *cp;

	if (strlen(from) == 0)
		return(name);
	if (any('@', name) || any('%', name))
		return(savestr(arpafix(name, from)));

	/* heavy additions/modifications for SVID-2 in this section */

	if (any('@', from) || any('%', from))
		return(unuucp(makeremote(name, from)));
	if (value("onehop") && any('!', name))
		strcpy(nbuf, name);
	else {
		cp = revarpa(from);
		if (cp == NULLSTR)
			return(unuucp(name));
		strcpy(nbuf, cp);
		cp = &nbuf[strlen(nbuf) - 1];
		while (!any(*cp, metanet) && cp > nbuf)
			cp--;
		if (cp == nbuf)
			return(unuucp(name));
		*++cp = 0;
		strcat(nbuf, revarpa(name));
	}

	optim(nbuf, ret);
	cp = revarpa(ret);
	if (!icequal(name, cp))
                return(unuucp((char *) savestr(cp)));	/* mod for SVID-2 */
        return(unuucp(name));				/* mod for SVID-2 */
}

/* added for SVID-2 */
/*
 * Optionally translate an old format uucp name into a new one, e.g.
 * "mach1!mach2!user" becomes "user@mach2.UUCP".  This optional because
 * some information is necessarily lost (e.g. the route it got here
 * via) and if we don't have the host in our routing tables, we lose.
 */
char *
unuucp(name)
char *name;
{
	register char *np, *hp, *cp;
	char result[100];
	char tname[300];

	if((cp = value("conv"))==NULLSTR || strcmp(cp, "internet"))
		return name;
	if (debug) fprintf(stderr, MSGSTR(UNUUCP, "unuucp(%s)\n"), name);
	strcpy(tname, name);
	np = strrchr(tname, '!');
	if (np == NULLSTR)
		return name;
	*np++ = 0;
	hp = strrchr(tname, '!');
	if (hp == NULLSTR)
		hp = tname;
	else
		*hp++ = 0;
	cp = strchr(np, '@');
	if (cp == NULLSTR)
		cp = strchr(np, '%');
	if (cp)
		*cp = 0;
	if (debug) fprintf(stderr, MSGSTR(HOST, "host %s, name %s\n"), hp, np);
	sprintf(result, "%s@%s.UUCP", np, hp);
	if (debug) fprintf(stderr,MSGSTR(UNRET, "unuucp returns %s\n"), result);
	return savestr(result);
}

/*
 * Turn a network machine name into a unique character
 */
netlook(machine, attnet)
	char machine[];
{
	register struct netmach *np;
	register char *cp, *cp2;
	char nbuf[BUFSIZ];

	/*
	 * Make into lower case.
	 */

	for (cp = machine, cp2 = nbuf; *cp; *cp2++ = little(*cp++))
		if (cp2 >= &nbuf[sizeof(nbuf)-1])
			break;
	*cp2 = 0;

	/*
	 * If a single letter machine, look through those first.
	 */

	if (strlen(nbuf) == 1)
		for (np = netmach; np->nt_mid != 0; np++)
			if (np->nt_mid == nbuf[0])
				return(nbuf[0]);

	/*
	 * Look for usual name
	 */

	for (np = netmach; np->nt_mid != 0; np++)
		if (strcmp(np->nt_machine, nbuf) == 0)
			return(np->nt_mid);

	/*
	 * Look in side hash table.
	 */

	return(mstash(nbuf, attnet));
}

/*
 * Make a little character.
 */

little(c)
	register int c;
{

	if (c >= 'A' && c <= 'Z')
		c += 'a' - 'A';
	return(c);
}

/*
 * Turn a network unique character identifier into a network name.
 */

char *
netname(mid)
{
	register struct netmach *np;
	char *mlook();

	if (mid & 0200)
		return(mlook(mid));
	for (np = netmach; np->nt_mid != 0; np++)
		if (np->nt_mid == mid)
			return(np->nt_machine);
	return(NULLSTR);
}

/*
 * Deal with arpa net addresses.  The way this is done is strange.
 * In particular, if the destination arpa net host is not Berkeley,
 * then the address is correct as stands.  Otherwise, we strip off
 * the trailing @Berkeley, then cook up a phony person for it to
 * be from and optimize the result.
 */
char *
arpafix(name, from)
	char name[];
	char from[];
{
	register char *cp;
	register int arpamach;
	char newname[BUFSIZ];
	char fake[5];
	char fakepath[20];

	if (debug) {
		fprintf(stderr,MSGSTR(ARPFIX, "arpafix(%s, %s)\n"), name, from);
	}
	cp = rindex(name, '@');
	if (cp == NULLSTR)
		cp = rindex(name, '%');
	if (cp == NULLSTR) {
		fprintf(stderr, MSGSTR(AMISS, "Somethings amiss -- no @ or % in arpafix\n")); /*MSG*/
		return(name);
	}
	cp++;
	arpamach = netlook(cp, '@');
	if (arpamach == 0) {
		if (debug)
			fprintf(stderr, MSGSTR(MACHUNK,
			"machine %s unknown, uses: %s\n"), cp, name);
		return(name);
	}
	if (((nettype(arpamach) & nettype(LOCAL)) & ~AN) == 0) {
		if (debug)
			fprintf(stderr, MSGSTR(MACHKN,
			"machine %s known but remote, uses: %s\n"), cp, name);
		return(name);
	}
	strcpy(newname, name);
	cp = rindex(newname, '@');
	if (cp == NULLSTR)
		cp = rindex(newname, '%');
	*cp = 0;
	fake[0] = arpamach;
	fake[1] = ':';
	fake[2] = LOCAL;
	fake[3] = ':';
	fake[4] = 0;
	prefer(fake);
	strcpy(fakepath, netname(fake[0]));
	stradd(fakepath, fake[1]);
	strcat(fakepath, "daemon");
	if (debug)
		fprintf(stderr, MSGSTR(MACHL,
		"machine local, call netmap(%s, %s)\n"), newname, fakepath);
	return(netmap(newname, fakepath));
}

/* added for SVID-2 */
/*
 * We have name with no @'s in it, and from with @'s.
 * Assume that name is meaningful only on the site in from.
 */
char *
makeremote(name, from)
	char name[];
	char from[];
{
	register char *cp;
	static char rbuf[200];

	if (debug) fprintf(stderr, MSGSTR(MAKREM,
		   "makeremote(%s, %s) returns "), name, from);
	strcpy(rbuf, name);
	cp = strrchr(from, '@');
	if (cp == NULLSTR)
		cp = strrchr(from, '%');
	strcat(rbuf, cp);
	if (debug) fprintf(stderr, "%s\n", rbuf);
	return rbuf;
}


/*
 * Take a network machine descriptor and find the types of connected
 * nets and return it.
 */

nettype(mid)
{
	register struct netmach *np;

	if (mid & 0200)
		return(mtype(mid));
	for (np = netmach; np->nt_mid != 0; np++)
		if (np->nt_mid == mid)
			return(np->nt_type);
	return(0);
}

/*
 * Hashing routines to salt away machines seen scanning
 * networks paths that we don't know about.
 */

#define	XHSIZE		19		/* Size of extra hash table */
#define	NXMID		(XHSIZE*3/4)	/* Max extra machines */

struct xtrahash {
	char	*xh_name;		/* Name of machine */
	short	xh_mid;			/* Machine ID */
	short	xh_attnet;		/* Attached networks */
} xtrahash[XHSIZE];

struct xtrahash	*xtab[XHSIZE];		/* F: mid-->machine name */

short	midfree;			/* Next free machine id */

/*
 * Initialize the extra host hash table.
 * Called by sreset.
 */

minit()
{
	register struct xtrahash *xp, **tp;
	register int i;

	midfree = 0;
	tp = &xtab[0];
	for (xp = &xtrahash[0]; xp < &xtrahash[XHSIZE]; xp++) {
		xp->xh_name = NULLSTR;
		xp->xh_mid = 0;
		xp->xh_attnet = 0;
		*tp++ = (struct xtrahash *) 0;
	}
}

/*
 * Stash a net name in the extra host hash table.
 * If a new entry is put in the hash table, deduce what
 * net the machine is attached to from the net character.
 *
 * If the machine is already known, add the given attached
 * net to those already known.
 */

mstash(name, attnet)
	char name[];
{
	register struct xtrahash *xp;
	struct xtrahash *xlocate();
	int x;

	xp = xlocate(name);
	if (xp == (struct xtrahash *) 0) {
		printf(MSGSTR(OUTOFSPTS, "Ran out of machine id spots\n")); /*MSG*/
		return(0);
	}
	if (xp->xh_name == NULLSTR) {
		if (midfree >= XHSIZE) {
			printf(MSGSTR(NOIDS, "Out of machine ids\n")); /*MSG*/
			return(0);
		}
		xtab[midfree] = xp;
		xp->xh_name = savestr(name);
		xp->xh_mid = 0200 + midfree++;
	}
	x = ntype(attnet);
	if (x == 0)
		xp->xh_attnet |= SN;
	else
		xp->xh_attnet |= x;
	return(xp->xh_mid);
}

/*
 * Search for the given name in the hash table
 * and return the pointer to it if found, or to the first
 * empty slot if not found.
 *
 * If no free slots can be found, return 0.
 */

struct xtrahash *
xlocate(name)
	char name[];
{
	register int h, q, i;
	register char *cp;
	register struct xtrahash *xp;

	for (h = 0, cp = name; *cp; h = (h << 2) + *cp++)
		;
	if (h < 0 && (h = -h) < 0)
		h = 0;
	h = h % XHSIZE;
	cp = name;
	for (i = 0, q = 0; q < XHSIZE; i++, q = i * i) {
		xp = &xtrahash[(h + q) % XHSIZE];
		if (xp->xh_name == NULLSTR)
			return(xp);
		if (strcmp(cp, xp->xh_name) == 0)
			return(xp);
		if (h - q < 0)
			h += XHSIZE;
		xp = &xtrahash[(h - q) % XHSIZE];
		if (xp->xh_name == NULLSTR)
			return(xp);
		if (strcmp(cp, xp->xh_name) == 0)
			return(xp);
	}
	return((struct xtrahash *) 0);
}

/*
 * Return the name from the extra host hash table corresponding
 * to the passed machine id.
 */

char *
mlook(mid)
{
	register int m;

	if ((mid & 0200) == 0)
		return(NULLSTR);
	m = mid & 0177;
	if (m >= midfree) {
		printf(MSGSTR(UNDID, "Use made of undefined machine id\n")); /*MSG*/
		return(NULLSTR);
	}
	return(xtab[m]->xh_name);
}

/*
 * Return the bit mask of net's that the given extra host machine
 * id has so far.
 */

mtype(mid)
{
	register int m;

	if ((mid & 0200) == 0)
		return(0);
	m = mid & 0177;
	if (m >= midfree) {
		printf(MSGSTR(UNDID, "Use made of undefined machine id\n")); /*MSG*/
		return(0);
	}
	return(xtab[m]->xh_attnet);
}

/*
 * Take a network name and optimize it.  This gloriously messy
 * operation takes place as follows:  the name with machine names
 * in it is tokenized by mapping each machine name into a single
 * character machine id (netlook).  The separator characters (network
 * metacharacters) are left intact.  The last component of the network
 * name is stripped off and assumed to be the destination user name --
 * it does not participate in the optimization.  As an example, the
 * name "research!vax135!research!ucbvax!bill" becomes, tokenized,
 * "r!x!r!v!" and "bill"  A low level routine, optim1, fixes up the
 * network part (eg, "r!x!r!v!"), then we convert back to network
 * machine names and tack the user name on the end.
 *
 * The result of this is copied into the parameter "name"
 */

optim(net, name)
	char net[], name[];
{
	char netcomp[BUFSIZ], netstr[40], xfstr[40];
	register char *cp, *cp2;
	register int c;

	strcpy(netstr, "");
	cp = net;
	for (;;) {
		/*
		 * Rip off next path component into netcomp
		 */
		cp2 = netcomp;
		while (*cp && !any(*cp, metanet))
			*cp2++ = *cp++;
		*cp2 = 0;
		/*
		 * If we hit null byte, then we just scanned
		 * the destination user name.  Go off and optimize
		 * if its so.
		 */
		if (*cp == 0)
			break;
		if ((c = netlook(netcomp, *cp)) == 0) {
			printf(MSGSTR(NOHOST, "No host named \"%s\"\n"), netcomp); /*MSG*/
err:
			strcpy(name, net);
			return;
		}
		stradd(netstr, c);
		stradd(netstr, *cp++);
		/*
		 * If multiple network separators given,
		 * throw away the extras.
		 */
		while (any(*cp, metanet))
			cp++;
	}
	if (strlen(netcomp) == 0) {
		printf(MSGSTR(NMSYN, "net name syntax\n")); /*MSG*/
		goto err;
	}
	optim1(netstr, xfstr);

	/*
	 * Convert back to machine names.
	 */

	cp = xfstr;
	strcpy(name, "");
	while (*cp) {
		if ((cp2 = netname(*cp++)) == NULLSTR) {
			printf(MSGSTR(BADNAME, "Made up bad net name\n")); /*MSG*/
			printf(MSGSTR(CODE, "Machine code %c (0%o)\n"), cp[-1], cp[-1]); /*MSG*/
			printf(MSGSTR(DUMPING, "Sorry -- dumping now.  Alert K. Shoens\n")); /*MSG*/
			core(0);
			goto err;
		}
		strcat(name, cp2);
		stradd(name, *cp++);
	}
	strcat(name, netcomp);
}

/*
 * Take a string of network machine id's and separators and
 * optimize them.  We process these by pulling off maximal
 * leading strings of the same type, passing these to the appropriate
 * optimizer and concatenating the results.
 */

optim1(netstr, name)
	char netstr[], name[];
{
	char path[40], rpath[40];
	register char *cp, *cp2;
	register int tp, nc;

	cp = netstr;
	prefer(cp);
	strcpy(name, "");
	/*
	 * If the address ultimately points back to us,
	 * just return a null network path.
	 */
	if (strlen(cp) > 1 && cp[strlen(cp) - 2] == LOCAL)
		return;
	while (*cp != 0) {
		strcpy(path, "");
		tp = ntype(cp[1]);
		nc = cp[1];
		while (*cp && tp == ntype(cp[1])) {
			stradd(path, *cp++);
			cp++;
		}
		switch (netkind(tp)) {
		default:
			strcpy(rpath, path);
			break;

		case IMPLICIT:
			optimimp(path, rpath);
			break;

		case EXPLICIT:
			optimex(path, rpath);
			break;
		}
		for (cp2 = rpath; *cp2 != 0; cp2++) {
			stradd(name, *cp2);
			stradd(name, nc);
		}
	}
	optiboth(name);
	prefer(name);
}

/*
 * Return the network of the separator --
 *	AN for arpa net
 *	BN for Bell labs net
 *	SN for Schmidt (berkeley net)
 *	0 if we don't know.
 */

ntype(nc)
	register int nc;
{
	register struct ntypetab *np;

	for (np = ntypetab; np->nt_char != 0; np++)
		if (np->nt_char == nc)
			return(np->nt_bcode);
	return(0);
}

/*
 * Return the kind of routing used for the particular net
 * EXPLICIT means explicitly routed
 * IMPLICIT means implicitly routed
 * 0 means don't know
 */

netkind(nt)
	register int nt;
{
	register struct nkindtab *np;

	for (np = nkindtab; np->nk_type != 0; np++)
		if (np->nk_type == nt)
			return(np->nk_kind);
	return(0);
}

/*
 * Do name optimization for an explicitly routed network (eg BTL network).
 */

optimex(net, name)
	char net[], name[];
{
	register char *cp, *rp;
	register int m;

	strcpy(name, net);
	cp = name;
	if (strlen(cp) == 0)
		return(-1);
	if (cp[strlen(cp)-1] == LOCAL) {
		name[0] = 0;
		return(0);
	}
	for (cp = name; *cp; cp++) {
		m = *cp;
		rp = rindex(cp+1, m);
		if (rp != NULLSTR)
			strcpy(cp, rp);
	}
	return(0);
}

/*
 * Do name optimization for implicitly routed network (eg, arpanet,
 * Berkeley network)
 */

optimimp(net, name)
	char net[], name[];
{
	register char *cp;
	register int m;

	cp = net;
	if (strlen(cp) == 0)
		return(-1);
	m = cp[strlen(cp) - 1];
	if (m == LOCAL) {
		strcpy(name, "");
		return(0);
	}
	name[0] = m;
	name[1] = 0;
	return(0);
}

/*
 * Perform global optimization on the given network path.
 * The trick here is to look ahead to see if there are any loops
 * in the path and remove them.  The interpretation of loops is
 * more strict here than in optimex since both the machine and net
 * type must match.
 */

optiboth(net)
	char net[];
{
	register char *cp, *cp2;
	char *rpair();

	cp = net;
	if (strlen(cp) == 0)
		return;
	if ((strlen(cp) % 2) != 0) {
		printf(MSGSTR(BADARG, "Strange arg to optiboth\n")); /*MSG*/
		return;
	}
	while (*cp) {
		cp2 = rpair(cp+2, *cp);
		if (cp2 != NULLSTR)
			strcpy(cp, cp2);
		cp += 2;
	}
}

/*
 * Find the rightmost instance of the given (machine, type) pair.
 */

char *
rpair(str, mach)
	char str[];
{
	register char *cp, *last;

	cp = str;
	last = NULLSTR;
	while (*cp) {
		if (*cp == mach)
			last = cp;
		cp += 2;
	}
	return(last);
}

/*
 * Change the network separators in the given network path
 * to the preferred network transmission means.
 */

prefer(name)
	char name[];
{
	register char *cp;
	register int state, n;

	state = LOCAL;
	for (cp = name; *cp; cp += 2) {
		n = best(state, *cp);
		if (n)
			cp[1] = n;
		state = *cp;
	}
}

/*
 * Return the best network separator for the given machine pair.
 */

best(src, dest)
{
	register int dtype, stype;
	register struct netorder *np;

	stype = nettype(src);
	dtype = nettype(dest);
	fflush(stdout);
	if (stype == 0 || dtype == 0) {
		printf(MSGSTR(UNKID, "ERROR:  unknown internal machine id\n")); /*MSG*/
		return(0);
	}
	if ((stype & dtype) == 0)
		return(0);
	np = &netorder[0];
	while ((np->no_stat & stype & dtype) == 0)
		np++;
	return(np->no_char);
}

#ifdef	GETHOST
/*
 * Initialize the network name of the current host.
 */
inithost()
{
	register struct netmach *np;

	if (gethostname(host, sizeof host))
		perror("inithost: gethostname");
	strcpy(domain, host);
	strcat(domain, MYDOMAIN);
	for (np = netmach; np->nt_machine != 0; np++)
		if (strcmp(np->nt_machine, EMPTY) == 0)
			break;
	if (np->nt_machine == 0) {
		printf(MSGSTR(NOSLOT, "Cannot find empty slot for dynamic host entry\n")); /*MSG*/
		exit(1);
	}
	np->nt_machine = host;
	np++;
	np->nt_machine = domain;
}
#endif /*GETHOST*/

/*
 * Code to twist around arpa net names.
 */

#define WORD 257			/* Token for a string */

static	char netbuf[256];
static	char *yylval;

/*
 * Reverse all of the arpa net addresses in the given name to
 * be of the form "host @ user" instead of "user @ host"
 * This function is its own inverse.
 */

char *
revarpa(str)
	char str[];
{

	if (yyinit(str) < 0)
		return(NULLSTR);
	if (name())
		return(NULLSTR);
	if (strcmp(str, netbuf) == 0)
		return(str);
	return(savestr(netbuf));
}

/*
 * Parse (by recursive descent) network names, using the following grammar:
 *	name:
 *		term {':' term}
 *		term {'^' term}
 *		term {'!' term}
 *		term '@' name
 *		term '%' name
 *
 *	term:
 *		string of characters.
 */

name()
{
	register int t;
	register char *cp;

	for (;;) {
		t = yylex();
		if (t != WORD)
			return(-1);
		cp = yylval;
		t = yylex();
		switch (t) {
		case 0:
			strcat(netbuf, cp);
			return(0);

		case '@':
		case '%':
			if (name())
				return(-1);
			stradd(netbuf, '@');
			strcat(netbuf, cp);
			return(0);

		case WORD:
			return(-1);

		default:
			strcat(netbuf, cp);
			stradd(netbuf, t);
		}
	}
}

/*
 * Scanner for network names.
 */

static	char *charp;			/* Current input pointer */
static	int nexttok;			/* Salted away next token */

/*
 * Initialize the network name scanner.
 */

yyinit(str)
	char str[];
{
	static char lexbuf[BUFSIZ];

	netbuf[0] = 0;
	if (strlen(str) >= sizeof lexbuf - 1)
		return(-1);
	nexttok = 0;
	strcpy(lexbuf, str);
	charp = lexbuf;
	return(0);
}

/*
 * Scan and return a single token.
 * yylval is set to point to a scanned string.
 */

yylex()
{
	register char *cp, *dot;
	register int s;
#ifdef ASIAN_I18N
	wchar_t wc;
	int mb;
#endif

	if (nexttok) {
		s = nexttok;
		nexttok = 0;
		return(s);
	}
	cp = charp;
#ifdef ASIAN_I18N
	while (*cp && ISSPACE(wc, cp, mb))
		cp+=mb;
#else
	while (*cp && isspace(*cp))
		cp++;
#endif
	if (*cp == 0)
		return(0);
	if (any(*cp, metanet)) {
		charp = cp+1;
		return(*cp);
	}
	dot = cp;
#ifdef ASIAN_I18N
	while (*cp && !any(*cp, metanet) && !mb_any(cp, " \t", &mb))
#else
	while (*cp && !any(*cp, metanet) && !any(*cp, " \t"))
#endif
		cp++;
	if (any(*cp, metanet))
		nexttok = *cp;
	if (*cp == 0)
		charp = cp;
	else
		charp = cp+1;
	*cp = 0;
	yylval = dot;
	return(WORD);
}

/*
 * Add a single character onto a string.
 */

stradd(str, c)
	register char *str;
	register int c;
{

	str += strlen(str);
	*str++ = c;
	*str = 0;
}
