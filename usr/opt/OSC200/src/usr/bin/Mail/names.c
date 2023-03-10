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
static char rcsid[] = "@(#)$RCSfile: names.c,v $ $Revision: 4.2.12.3 $ (DEC) $Date: 1993/09/29 13:26:59 $";
#endif
/*
 * HISTORY
 */
/*
 * OSF/1 Release 1.0
 */
/* 
 * COMPONENT_NAME: CMDMAILX names.c
 * 
 * FUNCTIONS: MSGSTR, cat, cmpdomain, count, delname, detract, elide, 
 *            extract, gexpand, isfileaddr, lengthof, mapf, mechk, 
 *            nalloc, nstrcmp, outof, prettyprint, put, tailof, 
 *            unpack, usermap, verify, yankword 
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
 *	names.c      5.3 (Berkeley) 11/10/85
 */

/*
 * Mail -- a mail program
 *
 * Handle name lists.
 */

#include "rcv.h"

#include "Mail_msg.h" 
extern nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_MAILX,n,s) 

/*
 * Allocate a single element of a name list,
 * initialize its name field to the passed
 * name and return it.
 */

struct name *
nalloc(str)
	char str[];
{
	register struct name *np;

	np = (struct name *) salloc(sizeof *np);
	np->n_flink = NIL;
	np->n_blink = NIL;
	np->n_type = -1;
	np->n_name = savestr(str);
	return(np);
}

/*
 * Find the tail of a list and return it.
 */

struct name *
tailof(name)
	struct name *name;
{
	register struct name *np;

	np = name;
	if (np == NIL)
		return(NIL);
	while (np->n_flink != NIL)
		np = np->n_flink;
	return(np);
}

/*
 * Extract a list of names from a line,
 * and make a list of names from it.
 * Return the list or NIL if none found.
 */

struct name *
extract(line, ntype)
	char line[];
{
	register char *cp;
	register struct name *top, *np, *t;
	char nbuf[BUFSIZ], abuf[BUFSIZ];
	char flatname[BUFSIZ];

	if (line == NULLSTR || strlen(line) == 0)
		return(NIL);
	top = NIL;
	np = NIL;
	cp = line;
	while ((cp = yankword(cp, nbuf)) != NULLSTR) {
		if (np != NIL && equal(nbuf, "at")) {
			strcpy(abuf, nbuf);
			if ((cp = yankword(cp, nbuf)) == NULLSTR) {
				strcpy(nbuf, abuf);
				goto normal;
			}
#ifndef ASIAN_I18N
			NLflatstr (np->n_name, flatname, BUFSIZ);
			strcpy(abuf, flatname);
#else /* ASIAN_I18N */
			strcpy(abuf, np->n_name);
#endif /* ASIAN_I18N */
			stradd(abuf, '@');
			strcat(abuf, nbuf);
			np->n_name = savestr(abuf);
			continue;
		}
normal:
		t = nalloc(nbuf);
		t->n_type = ntype;
		if (top == NIL)
			top = t;
		else
			np->n_flink = t;
		t->n_blink = np;
		np = t;
	}
	return(top);
}

/*
 * Turn a list of names into a string of the same names.
 */

char *
detract(np, ntype)
	register struct name *np;
{
	register int s;
	register char *cp, *top;
	register struct name *p;
	register int comma;
	char flatname[BUFSIZ];

	comma = ntype & GCOMMA;
	if (np == NIL)
		return(NULLSTR);
	ntype &= ~GCOMMA;
	s = 0;
	if (debug && comma)
		fprintf(stderr, MSGSTR(DETRACT,
		"detract asked to insert commas\n"));
	for (p = np; p != NIL; p = p->n_flink) {
		if (ntype && (p->n_type & GMASK) != ntype)
			continue;
		s += strlen(p->n_name) + 1;
		if (comma)
			s++;
	}
	if (s == 0)
		return(NULLSTR);
	s += 2;
	top = salloc(s);
	cp = top;
	for (p = np; p != NIL; p = p->n_flink) {
		if (ntype && (p->n_type & GMASK) != ntype)
			continue;
#ifndef ASIAN_I18N
		NLflatstr (p->n_name, flatname, BUFSIZ);
		cp = copy(flatname, cp);
#else /* ASIAN_I18N */
		cp = copy(p->n_name, cp);
#endif /* ASIAN_I18N */
		if (comma && p->n_flink != NIL)
			*cp++ = ',';
		*cp++ = ' ';
	}
	*--cp = 0;
	if (comma && *--cp == ',')
		*cp = 0;
	return(top);
}

/*
 * Grab a single word (liberal word)
 * Throw away things between ()'s.
 */

char *
yankword(ap, wbuf)
	char *ap, wbuf[];
{
	register char *cp, *cp2;
#ifdef ASIAN_I18N
	int mb;
#endif

	cp = ap;
	do {
#ifdef ASIAN_I18N
		while (*cp && mb_any(cp, " \t,", &mb))
			cp++;
#else
		while (*cp && any(*cp, " \t,"))
			cp++;
#endif
		if (*cp == '(') {
			register int nesting = 0;

			while (*cp != '\0') {
				switch (*cp++) {
				case '(':
					nesting++;
					break;
				case ')':
					--nesting;
					break;
				}
				if (nesting <= 0)
					break;
			}
		}
		if (*cp == '\0')
			return(NULLSTR);
#ifdef ASIAN_I18N
	} while (mb_any(cp, " \t,(", &mb));
	for (cp2 = wbuf; *cp && !mb_any(cp, " \t,(", &mb); *cp2++ = *cp++)
#else
	} while (any(*cp, " \t,("));
	for (cp2 = wbuf; *cp && !any(*cp, " \t,("); *cp2++ = *cp++)
#endif
		;
	*cp2 = '\0';
	return(cp);
}

/*
 * Verify that all the users in the list of names are
 * legitimate.  Bitch about and delink those who aren't.
 */

struct name *
verify(names)
	struct name *names;
{
	register struct name *np, *top, *t, *x;
	register char *cp;

#ifdef SENDMAIL
	return(names);
#else
	top = names;
	np = names;
	while (np != NIL) {
		if (np->n_type & GDEL) {
			np = np->n_flink;
			continue;
		}
		for (cp = "!:@^"; *cp; cp++)
			if (any(*cp, np->n_name))
				break;
		if (*cp != 0) {
			np = np->n_flink;
			continue;
		}
		cp = np->n_name;
		while (*cp == '\\')
			cp++;
		if (equal(cp, "msgs") ||
		    getuserid(cp) != -1) {
			np = np->n_flink;
			continue;
		}
		fprintf(stderr, MSGSTR(NOSEND, "Can't send to %s\n"), np->n_name); /*MSG*/
		senderr++;
		if (np == top) {
			top = np->n_flink;
			if (top != NIL)
				top->n_blink = NIL;
			np = top;
			continue;
		}
		x = np->n_blink;
		t = np->n_flink;
		x->n_flink = t;
		if (t != NIL)
			t->n_blink = x;
		np = t;
	}
	return(top);
#endif
}

/*
 * For each recipient in the passed name list with a /
 * in the name, append the message to the end of the named file
 * and remove him from the recipient list.
 *
 * Recipients whose name begins with | are piped through the given
 * program and removed.
 */

struct name *
outof(names, fo, hp)
	struct name *names;
	FILE *fo;
	struct header *hp;
{
	register int c;
	register struct name *np, *top, *t, *x;
	long now;
	char *date, *fname, *shell, *ctime();
	FILE *fout, *fin;
	int ispipe, s, pid;
	extern char tempEdit[];

	top = names;
	np = names;
	time(&now);
	date = ctime(&now);
	while (np != NIL) {
		if (!isfileaddr(np->n_name) && np->n_name[0] != '|') {
			np = np->n_flink;
			continue;
		}
		ispipe = np->n_name[0] == '|';
		if (ispipe)
			fname = np->n_name+1;
		else
			fname = expand(np->n_name);

		/*
		 * See if we have copied the complete message out yet.
		 * If not, do so.
		 */

		if (image < 0) {
			if ((fout = fopen(tempEdit, "a")) == NULL) {
				perror(tempEdit);
				senderr++;
				goto cant;
			}
			image = open(tempEdit, 2);
			unlink(tempEdit);
			if (image < 0) {
				perror(tempEdit);
				senderr++;
				goto cant;
			}
			else {
				rewind(fo);
				fprintf(fout, MSGSTR(FROM,
				"From %s %s"), myname, date);
				puthead(hp, fout, GTO|GSUBJECT|GCC|GNL);
				while ((c = getc(fo)) != EOF)
					putc(c, fout);
				rewind(fo);
				putc('\n', fout);
				fflush(fout);
				if (ferror(fout))
					perror(tempEdit);
				fclose(fout);
			}
		}

		/*
		 * Now either copy "image" to the desired file
		 * or give it as the standard input to the desired
		 * program as appropriate.
		 */

		if (ispipe) {
			wait(&s);
			switch (pid = fork()) {
			case 0:
				sigchild();
				signal(SIGHUP, SIG_IGN);
				signal(SIGINT, SIG_IGN);
				signal(SIGQUIT, SIG_IGN);
				close(0);
				dup(image);
				close(image);
				if ((shell = value("SHELL")) == NULLSTR)
					shell = SHELL;
				execl(shell, shell, "-c", fname, 0);
				perror(shell);
				exit(1);
				break;

			case -1:
				perror("fork");
				senderr++;
				goto cant;
			}
		}
		else {
			if ((fout = fopen(fname, "a")) == NULL) {
				perror(fname);
				senderr++;
				goto cant;
			}
			fin = Fdopen(image, "r");
			if (fin == NULL) {
				fprintf(stderr, MSGSTR(NOREOPEN, "Can't reopen image\n")); /*MSG*/
				fclose(fout);
				senderr++;
				goto cant;
			}
			rewind(fin);
			while ((c = getc(fin)) != EOF)
				putc(c, fout);
			if (ferror(fout))
				senderr++, perror(fname);
			fclose(fout);
			fclose(fin);
		}

cant:

		/*
		 * In days of old we removed the entry from the
		 * the list; now for sake of header expansion
		 * we leave it in and mark it as deleted.
		 */

#ifdef CRAZYWOW
		if (np == top) {
			top = np->n_flink;
			if (top != NIL)
				top->n_blink = NIL;
			np = top;
			continue;
		}
		x = np->n_blink;
		t = np->n_flink;
		x->n_flink = t;
		if (t != NIL)
			t->n_blink = x;
		np = t;
#endif

		np->n_type |= GDEL;
		np = np->n_flink;
	}
	if (image >= 0) {
		close(image);
		image = -1;
	}
	return(top);
}

/*
 * Determine if the passed address is a local "send to file" address.
 * If any of the network metacharacters precedes any slashes, it can't
 * be a filename.  We cheat with .'s to allow path names like ./...
 */
isfileaddr(name)
	char *name;
{
	register char *cp;
	extern char *metanet;

	if (any('@', name))
		return(0);
	if (*name == '+')
		return(1);
	for (cp = name; *cp; cp++) {
		if (*cp == '.')
			continue;
		if (any(*cp, metanet))
			return(0);
		if (*cp == '/')
			return(1);
	}
	return(0);
}

/*
 * Map all of the aliased users in the invoker's mailrc
 * file and insert them into the list.
 * Changed after all these months of service to recursively
 * expand names (2/14/80).
 */

struct name *
usermap(names)
	struct name *names;
{
	register struct name *new, *np, *cp;
	struct name *getto;
	struct grouphead *gh;
	register int metoo;

	new = NIL;
	np = names;
	getto = NIL;
	metoo = (value("metoo") != NULLSTR);
	while (np != NIL) {
		if (np->n_name[0] == '\\') {
			cp = np->n_flink;
			new = put(new, np);
			np = cp;
			continue;
		}
		gh = findgroup(np->n_name);
		cp = np->n_flink;
		if (gh != NOGRP)
			new = gexpand(new, gh, metoo, np->n_type);
		else
			new = put(new, np);
		np = cp;
	}
	return(new);
}

/*
 * Recursively expand a group name.  We limit the expansion to some
 * fixed level to keep things from going haywire.
 * Direct recursion is not expanded for convenience.
 */

struct name *
gexpand(nlist, gh, metoo, ntype)
	struct name *nlist;
	struct grouphead *gh;
{
	struct group *gp;
	struct grouphead *ngh;
	struct name *np;
	static int depth;
	char *cp;
	int allnet = (value("allnet")!=NULLSTR);

	if (depth > MAXEXP) {
		printf(MSGSTR(EXPALIAS, "Expanding alias to depth larger than %d\n"), MAXEXP); /*MSG*/
		return(nlist);
	}
	depth++;
	for (gp = gh->g_list; gp != NOGE; gp = gp->ge_link) {
		cp = gp->ge_name;
		if (*cp == '\\')
			goto quote;
		if (strcmp(cp, gh->g_name) == 0)
			goto quote;
		if ((ngh = findgroup(cp)) != NOGRP) {
			nlist = gexpand(nlist, ngh, metoo, ntype);
			continue;
		}
quote:
		np = nalloc(cp);
		np->n_type = ntype;
		/*
		 * At this point should allow to expand
		 * to self if only person in group
		 */
		if (gp == gh->g_list && gp->ge_link == NOGE)
			goto skip;

		/* modified for SVID-2 */
		if (!metoo){
			cp = gp->ge_name;
			if(allnet && (cp=strrchr(gp->ge_name,'!'))!=NULL)
				cp++;
			  else
				cp=gp->ge_name;
                        /*
                        if (debug) fprintf(stderr, MSGSTR(NAME,
			"name= %s\n"),cp); /* DEBUG */
			if (strcmp(cp, myname) == 0)
				np->n_type |= GDEL;
		}
skip:
		nlist = put(nlist, np);
	}
	depth--;
	return(nlist);
}



/*
 * Compute the length of the passed name list and
 * return it.
 */

lengthof(name)
	struct name *name;
{
	register struct name *np;
	register int c;

	for (c = 0, np = name; np != NIL; c++, np = np->n_flink)
		;
	return(c);
}

/*
 * Concatenate the two passed name lists, return the result.
 */

struct name *
cat(n1, n2)
	struct name *n1, *n2;
{
	register struct name *tail;

	if (n1 == NIL)
		return(n2);
	if (n2 == NIL)
		return(n1);
	tail = tailof(n1);
	tail->n_flink = n2;
	n2->n_blink = tail;
	return(n1);
}

/*
 * Unpack the name list onto a vector of strings.
 * Return an error if the name list won't fit.
 */

char **
unpack(np)
	struct name *np;
{
	register char **ap, **top;
	register struct name *n;
	char *cp;
	char hbuf[10];
	int t, extra, metoo, verbose;

	n = np;
	if ((t = lengthof(n)) == 0)
		panic(MSGSTR(NONUNPCK, "No names to unpack")); /*MSG*/

	/*
	 * Compute the number of extra arguments we will need.
	 * We need at least two extra -- one for "mail" and one for
	 * the terminating 0 pointer.  Additional spots may be needed
	 * to pass along -r and -f to the host mailer.
	 */

	extra = 2;
	if (rflag != NULLSTR)
		extra += 2;
#ifdef SENDMAIL
	extra++; /* -i */
	metoo = value("metoo") != NULLSTR;
	if (metoo)
		extra++;
	verbose = value("verbose") != NULLSTR;
	if (verbose)
		extra++;
#endif /*SENDMAIL*/
	if (hflag)
		extra += 2;
	top = (char **) calloc((t + extra), sizeof cp);
	ap = top;
	*ap++ = "send-mail";
	if (rflag != NULLSTR) {
		*ap++ = "-r";
		*ap++ = rflag;
	}
#ifdef SENDMAIL
	*ap++ = "-i";
	if (metoo)
		*ap++ = "-m";
	if (verbose)
		*ap++ = "-v";
#endif /*SENDMAIL*/
	if (hflag) {
		*ap++ = "-h";
		sprintf(hbuf, "%d", hflag);
		*ap++ = savestr(hbuf);
	}
	while (n != NIL) {
		if (n->n_type & GDEL) {
			n = n->n_flink;
			continue;
		}
		*ap++ = n->n_name;
		n = n->n_flink;
	}
	*ap = NULLSTR;
	return(top);
}

/*
 * See if the user named himself as a destination
 * for outgoing mail.  If so, set the global flag
 * selfsent so that we avoid removing his mailbox.
 */

mechk(names)
	struct name *names;
{
	register struct name *np;

	for (np = names; np != NIL; np = np->n_flink)
		if ((np->n_type & GDEL) == 0 && equal(np->n_name, myname)) {
			selfsent++;
			return;
		}
}

/*
 * Remove all of the duplicates from the passed name list by
 * insertion sorting them, then checking for dups.
 * Return the head of the new list.
 */

struct name *
elide(names)
	struct name *names;
{
	register struct name *np, *t, *new;
	struct name *x;

	if (names == NIL)
		return(NIL);
	new = names;
	np = names;
	np = np->n_flink;
	if (np != NIL)
		np->n_blink = NIL;
	new->n_flink = NIL;
	while (np != NIL) {
		t = new;
		while (nstrcmp(t->n_name, np->n_name) < 0) {
			if (t->n_flink == NIL)
				break;
			t = t->n_flink;
		}

		/*
		 * If we ran out of t's, put the new entry after
		 * the current value of t.
		 */

		if (nstrcmp(t->n_name, np->n_name) < 0) {
			t->n_flink = np;
			np->n_blink = t;
			t = np;
			np = np->n_flink;
			t->n_flink = NIL;
			continue;
		}

		/*
		 * Otherwise, put the new entry in front of the
		 * current t.  If at the front of the list,
		 * the new guy becomes the new head of the list.
		 */

		if (t == new) {
			t = np;
			np = np->n_flink;
			t->n_flink = new;
			new->n_blink = t;
			t->n_blink = NIL;
			new = t;
			continue;
		}

		/*
		 * The normal case -- we are inserting into the
		 * middle of the list.
		 */

		x = np;
		np = np->n_flink;
		x->n_flink = t;
		x->n_blink = t->n_blink;
		t->n_blink->n_flink = x;
		t->n_blink = x;
	}

	/*
	 * Now the list headed up by new is sorted.
	 * Go through it and remove duplicates.
	 */

	np = new;
	while (np != NIL) {
		t = np;
		while (t->n_flink!=NIL &&
		    icequal(np->n_name,t->n_flink->n_name))
			t = t->n_flink;
		if (t == np || t == NIL) {
			np = np->n_flink;
			continue;
		}
		
		/*
		 * Now t points to the last entry with the same name
		 * as np.  Make np point beyond t.
		 */

		np->n_flink = t->n_flink;
		if (t->n_flink != NIL)
			t->n_flink->n_blink = np;
		np = np->n_flink;
	}
	return(new);
}

/*
 * Version of strcmp which ignores case differences.
 */

nstrcmp(s1, s2)
	register char *s1, *s2;
{
	register int c1, c2;

	do {
		c1 = *s1++;
		c2 = *s2++;
	} while (c1 && c1 == c2);
	return(c1 - c2);
}

/*
 * Put another node onto a list of names and return
 * the list.
 */

struct name *
put(list, node)
	struct name *list, *node;
{
	node->n_flink = list;
	node->n_blink = NIL;
	if (list != NIL)
		list->n_blink = node;
	return(node);
}

/*
 * Determine the number of elements in
 * a name list and return it.
 */

count(np)
	register struct name *np;
{
	register int c = 0;

	while (np != NIL) {
		c++;
		np = np->n_flink;
	}
	return(c);
}

cmpdomain(name, dname)
	register char *name, *dname;
{
	char buf[BUFSIZ];

	strcpy(buf, dname);
	buf[strlen(name)] = '\0';
	return(icequal(name, buf));
}

/*
 * Delete the given name from a namelist, using the passed
 * function to compare the names.
 */
struct name *
delname(np, name, cmpfun)
	register struct name *np;
	char name[];
	int (* cmpfun)();
{
	register struct name *p;
	register char *cp;
        int allnet = (value("allnet")!=NULLSTR);


	for (p = np; p != NIL; p = p->n_flink)
	{
		cp = p->n_name;			/* modified for SVID-2 */
		if(allnet && (cp=strrchr(p->n_name,'!'))!=NULL)
			cp++;
		  else
			cp = p->n_name;
		if ((* cmpfun)(cp, name)) {
			if (p->n_blink == NIL) {
				if (p->n_flink != NIL)
					p->n_flink->n_blink = NIL;
				np = p->n_flink;
				continue;
			}
			if (p->n_flink == NIL) {
				if (p->n_blink != NIL)
					p->n_blink->n_flink = NIL;
				continue;
			}
			p->n_blink->n_flink = p->n_flink;
			p->n_flink->n_blink = p->n_blink;
		}
	}
	return(np);
}

/*
 * Call the given routine on each element of the name
 * list, replacing said value if need be.
 */

mapf(np, from)
	register struct name *np;
	char *from;
{
	register struct name *p;

	for (p = np; p != NIL; p = p->n_flink)
		p->n_name = netmap(p->n_name, from);
}

/*
 * Pretty print a name list
 * Uncomment it if you need it.
 */

prettyprint(name)
	struct name *name;
{
	register struct name *np;

	np = name;
	while (np != NIL) {
		fprintf(stderr, "%s(%d) ", np->n_name, np->n_type);
		np = np->n_flink;
	}
	fprintf(stderr, "\n");
}
