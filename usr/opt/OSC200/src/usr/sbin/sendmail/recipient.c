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
static char	*sccsid = "@(#)$RCSfile: recipient.c,v $ $Revision: 4.2.6.3 $ (DEC) $Date: 1993/12/09 15:52:26 $";
#endif 
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0
 */
#if !defined(lint) && !defined(_NOIDENT)

#endif
/*
 * Copyright (c) 1983 Eric P. Allman
 * Copyright (c) 1988 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted provided
 * that: (1) source distributions retain this entire copyright notice and
 * comment, and (2) distributions including binaries display the following
 * acknowledgement:  ``This product includes software developed by the
 * University of California, Berkeley and its contributors'' in the
 * documentation or other materials provided with the distribution and in
 * all advertising materials mentioning features or use of this software.
 * Neither the name of the University nor the names of its contributors may
 * be used to endorse or promote products derived from this software without
 * specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */
/*
#ifndef lint

#endif 
*/ 
# include <sys/param.h>
# include <sys/types.h>
# include <sys/stat.h>
# include <sys/file.h>
# include <pwd.h>
# include "sendmail.h"

/*
**  SENDTOLIST -- Designate a send list.
**
**	The parameter is a comma-separated list of people to send to.
**	This routine arranges to send to all of them.
**
**	Parameters:
**		list -- the send list.
**		ctladdr -- the address template for the person to
**			send to -- effective uid/gid are important.
**			This is typically the alias that caused this
**			expansion.
**		sendq -- a pointer to the head of a queue to put
**			these people into.
**
**	Returns:
**		none
**
**	Side Effects:
**		none.
*/

# define MAXRCRSN	10

sendtolist(list, ctladdr, sendq)
	char *list;
	ADDRESS *ctladdr;
	ADDRESS **sendq;
{
	register char *p;
	register ADDRESS *al;	/* list of addresses to send to */
	bool firstone;		/* set on first address sent */
	bool selfref;		/* set if this list includes ctladdr */
	char delimiter;		/* the address delimiter */

	if (tTd(25, 1))
	{
		printf("sendto: %s\n   ctladdr=", list);
		printaddr(ctladdr, FALSE);
	}

	/* heuristic to determine old versus new style addresses */
	if (ctladdr == NULL &&
	    (index(list, ',') != NULL || index(list, ';') != NULL ||
	     index(list, '<') != NULL || index(list, '(') != NULL))
		CurEnv->e_flags &= ~EF_OLDSTYLE;
	delimiter = ' ';
	if (!bitset(EF_OLDSTYLE, CurEnv->e_flags) || ctladdr != NULL)
		delimiter = ',';

	firstone = TRUE;
	selfref = FALSE;
	al = NULL;

	for (p = list; *p != '\0'; )
	{
		register ADDRESS *a;
		extern char *DelimChar;		/* defined in prescan */

		/* parse the address */
		while (isspace(*p) || *p == ',')
			p++;
		a = parseaddr(p, (ADDRESS *) NULL, 1, delimiter);
		p = DelimChar;
		if (a == NULL)
			continue;
		a->q_next = al;
		a->q_alias = ctladdr;

		/* see if this should be marked as a primary address */
		if (ctladdr == NULL ||
		    (firstone && *p == '\0' && bitset(QPRIMARY, ctladdr->q_flags)))
			a->q_flags |= QPRIMARY;

		/* put on send queue or suppress self-reference */
		if (ctladdr != NULL && sameaddr(ctladdr, a))
			selfref = TRUE;
		else
			al = a;
		firstone = FALSE;
	}

	/* if this alias doesn't include itself, delete ctladdr */
	if (!selfref && ctladdr != NULL)
		ctladdr->q_flags |= QDONTSEND;

	/* arrange to send to everyone on the local send list */
	while (al != NULL)
	{
		register ADDRESS *a = al;
		extern ADDRESS *recipient();

		al = a->q_next;
		setctladdr(a);
		a = recipient(a, sendq);

		/* arrange to inherit full name */
		if (a->q_fullname == NULL && ctladdr != NULL)
			a->q_fullname = ctladdr->q_fullname;

		/*
		**  If we have a bad recipient that is the result of
		**  an alias, mark the address that this recipient
		**  resulted from as a bad alias.
		*/
		if (ctladdr != NULL && bitset(QISALIAS|QISFORWD, ctladdr->q_flags) &&
		    bitset(QBADADDR|QBADALIAS, a->q_flags))
			ctladdr->q_flags |= QBADALIAS;
	}

	CurEnv->e_to = NULL;
}
/*
**  RECIPIENT -- Designate a message recipient
**
**	Saves the named person for future mailing.
**
**	Parameters:
**		a -- the (preparsed) address header for the recipient.
**		sendq -- a pointer to the head of a queue to put the
**			recipient in.  Duplicate supression is done
**			in this queue.
**
**	Returns:
**		The actual address in the queue.  This will be "a" if
**		the address is not a duplicate, else the original address.
**
**	Side Effects:
**		none.
*/

ADDRESS *
recipient(a, sendq)
	register ADDRESS *a;
	register ADDRESS **sendq;
{
	register ADDRESS *q;
	ADDRESS **pq;
	register struct mailer *m;
	register char *p;
	bool quoted = FALSE;		/* set if the addr has a quote bit */
	char buf[MAXNAME];		/* unquoted image of the user name */
	extern ADDRESS *getctladdr();
	extern bool safefile();

	CurEnv->e_to = a->q_paddr;
	m = a->q_mailer;
	errno = 0;
	if (tTd(26, 1))
	{
		printf("\nrecipient: ");
		printaddr(a, FALSE);
	}

	/* break aliasing loops */
	if (AliasLevel > MAXRCRSN)
	{
		usrerr(MSGSTR(RE_LOOP, "aliasing/forwarding loop broken"));
		return (a);
	}

	/*
	**  Finish setting up address structure.
	*/

	/* set the queue timeout */
	a->q_timeout = TimeOut;

	/* map user & host to lower case if requested on non-aliases */
	if (a->q_alias == NULL)
		loweraddr(a);

	/* get unquoted user for file, program or user.name check */
	(void) strcpy(buf, a->q_user);
	for (p = buf; *p != '\0' && !quoted; p++)
	{
		if (!isascii(*p) && (*p & 0377) != (SpaceSub & 0377))
			quoted = TRUE;
	}
	stripquotes(buf, TRUE);

	/* do sickly crude mapping for program mailing, etc. */
	if (m == LocalMailer && buf[0] == '|')
	{
		a->q_mailer = m = ProgMailer;
		a->q_user++;
#if	CERT1193
		/*  Verify that this is being exec'd due to an alias, or
		**  if directly attributable or .forward.  If not, an
		**  error.
		*/
		if (a->q_alias == NULL && !bitset(QGOODUID, a->q_flags) && !ForceMail)
#else
		if (a->q_alias == NULL && !QueueRun && !ForceMail)
#endif
		{
			a->q_flags |= QDONTSEND|QBADADDR;
			usrerr(MSGSTR(RE_NOPROG, "Cannot mail directly to programs"));
		}
	}

	/*
	**  Look up this person in the recipient list.
	**	If they are there already, return, otherwise continue.
	**	If the list is empty, just add it.  Notice the cute
	**	hack to make from addresses suppress things correctly:
	**	the QDONTSEND bit will be set in the send list.
	**	[Please note: the emphasis is on "hack."]
	*/

	for (pq = sendq; (q = *pq) != NULL; pq = &q->q_next)
	{
		if (!ForceMail && sameaddr(q, a))
		{
			if (tTd(26, 1))
			{
				printf("%s in sendq: ", a->q_paddr);
				printaddr(q, FALSE);
			}
			if (!bitset(QDONTSEND, a->q_flags))
				message(Arpa_Info, MSGSTR(RE_DUP, "duplicate suppressed"));
			if (!bitset(QPRIMARY, q->q_flags))
				q->q_flags |= a->q_flags;
			return (q);
		}
	}

	/* add address on list */
	*pq = a;
	a->q_next = NULL;
	CurEnv->e_nrcpts++;

	/*
	**  Alias the name and handle :include: specs.
	*/

	if (m == LocalMailer && !bitset(QDONTSEND, a->q_flags))
	{
		if (strncmp(a->q_user, ":include:", 9) == 0)
		{
			a->q_flags |= QDONTSEND;
			/*
			** Only allow :include: from inside /etc/aliases
			*/
#if	CERT1193
			/*  Verify that this is being exec'd due to an alias, or
			**  if directly attributable or .forward.  If not, an
			**  error.
			*/
			if (!(a->q_alias != NULL && bitset(QISALIAS, a->q_alias->q_flags)) &&
			    !bitset(QGOODUID, a->q_flags) && !ForceMail)
#else
			if (!(a->q_alias != NULL && bitset(QISALIAS, a->q_alias->q_flags)) &&
			    !QueueRun && !ForceMail)
#endif
			{
				a->q_flags |= QBADADDR;
				usrerr(MSGSTR(RE_INC, "Cannot mail directly to :include:s"));
			}
			else
			{
				/*
				** including an alias list from a file across
				** NFS will probably fail due to our effective
				** uid being root
				*/
				message(Arpa_Info, MSGSTR(RE_INC2, "including file %s"), &a->q_user[9]);
				include(open(&a->q_user[9], O_RDONLY, 0), &a->q_user[9], " sending", a, sendq);
			}
		}
		else
			alias(a, sendq);
	}

	/*
	**  If the user is local and still being sent, verify that
	**  the address is good.  If it is, try to forward.
	**  If the address is already good, we have a forwarding
	**  loop.  This can be broken by just sending directly to
	**  the user (which is probably correct anyway).
	*/

	if (!bitset(QDONTSEND, a->q_flags) && m == LocalMailer)
	{
		struct stat stb;
		extern bool writable();

		/* see if this is to a file */
		if (buf[0] == '/')
		{
			p = rindex(buf, '/');
			/*
			** try to check if the file is writable or
			** creatable - this will probably crash and
			** burn over NFS as our euid is root.
			*/
#if	CERT1193
			/*  Verify that this is being exec'd due to an alias, or
			**  if directly attributable or .forward.  If not, an
			**  error.
			*/
			if (a->q_alias == NULL && !bitset(QGOODUID, a->q_flags) && !ForceMail)
#else
			if (a->q_alias == NULL && !QueueRun && !ForceMail)
#endif
			{
				a->q_flags |= QDONTSEND|QBADADDR;
				usrerr(MSGSTR(RE_FILES, "Cannot mail directly to files"));
			}
			else if ((stat(buf, &stb) >= 0) ? (!writable(&stb)) :
			    (*p = '\0', !safefile(buf, getruid(), S_IWRITE|S_IEXEC)))
			{
				a->q_flags |= QBADADDR;
				giveresponse(EX_CANTCREAT, m, CurEnv);
			}
		}
		else
		{
			register struct passwd *pw;
			extern struct passwd *finduser();

			/* warning -- finduser may trash buf */
			pw = finduser(buf);
			if (pw == NULL)
			{
				a->q_flags |= QBADADDR;
				errno = 0;	/* no special error */
				/*
				** postpone response if talking SMTP to
				** allow checking for an owner- alias.
				*/
				if (SmtpPhase &&
				    (strcmp(SmtpPhase, "RCPT") == 0 ||
				     strcmp(SmtpPhase, "EXPN") == 0 ||
				     strcmp(SmtpPhase, "VRFY") == 0))
				{
					CurEnv->e_flags |= EF_FATALERRS;
					if(LogLevel > 2)
						logdelivery("User unknown");
					if (CurEnv->e_xfp != NULL)
						fprintf(CurEnv->e_xfp, "%s: User unknown\n", buf);
				}
				else
					giveresponse(EX_NOUSER, m, CurEnv);
			}
			else
			{
				char nbuf[MAXNAME];

				if (strcmp(a->q_user, pw->pw_name) != 0)
				{
					a->q_user = newstr(pw->pw_name);
					(void) strcpy(buf, pw->pw_name);
				}
				a->q_home = newstr(pw->pw_dir);
				a->q_uid = pw->pw_uid;
				a->q_gid = pw->pw_gid;
				a->q_flags |= QGOODUID;
				buildfname(pw->pw_gecos, pw->pw_name, nbuf);
				if (nbuf[0] != '\0')
					a->q_fullname = newstr(nbuf);
				if (!quoted)
				{
#ifdef FUZZY
					alias(a, sendq); /* Re-check if user has an alias */
					if (!bitset(QDONTSEND, a->q_flags) && m == LocalMailer)
#endif /* FUZZY */
						forward(a, sendq);
				}
			}
		}
	}
	return (a);
}
/*
**  FINDUSER -- find the password entry for a user.
**
**	This looks a lot like getpwnam, except that it may want to
**	do some fancier pattern matching in /etc/passwd.
**
**	This routine contains most of the time of many sendmail runs.
**	It deserves to be optimized.
**
**	Parameters:
**		name -- the name to match against.
**
**	Returns:
**		A pointer to a pw struct.
**		NULL if name is unknown or ambiguous.
**
**	Side Effects:
**		may modify name.
*/

#define WORST_MATCH	-2		/* even worse than no match */
#define NO_UID		-999		/* any "impossible" uid will do */

struct passwd *
finduser(name)
	char *name;
{
	register struct passwd *pw;
	register char *p;
	int best_match = WORST_MATCH;
	int best_uid = NO_UID;
	extern struct passwd *getpwent();
	extern struct passwd *getpwnam();
	extern struct passwd *getpwuid();

	errno = 0;

	/* map upper => lower case */
	for (p = name; *p != '\0'; p++)
	{
		if (isascii(*p) && isupper(*p))
			*p = tolower(*p);
	}

	if (tTd(26, 6))
		printf("%s password entry for \"%s\"\n",
			getpwnam(name) ? "found" : "can't find", name);

	/* look up this login name using fast path */
	if ((pw = getpwnam(name)) != NULL)
		return (pw);
#ifndef FUZZY
	else
		return (NULL);
#else /* FUZZY */
	if (tTd(26, 6))
		printf("looking for partial match to \"%s\"\n", name);
	(void) setpwent();
	while ((pw = getpwent()) != NULL)
	{
		char buf[MAXNAME];
		register int this_match;

		if (strcasecmp(pw->pw_name, name) == 0) {
			if (tTd(26, 6))
				printf("found password entry for \"%s\" as \"%s\"\n",
					name, pw->pw_name);
			(void) endpwent();
			return (pw);
		}

		buildfname(pw->pw_gecos, pw->pw_name, buf);
		this_match = partialstring(buf, name);
		if (tTd(26, 6 && this_match >= 0))
			printf("matched on level %d with \"%s\"\n",
				this_match, buf);
		if (this_match < best_match)
			continue;
		else if (this_match > best_match) {
			best_match = this_match;
			best_uid = pw->pw_uid;
		} else if (best_uid != pw->pw_uid)
			best_uid = NO_UID;
	}
	(void) endpwent();
		if (tTd(26, 6))
			if (best_match == WORST_MATCH)
				printf("no match, failing...\n");
			else if (best_uid == NO_UID)
				printf("ambiguous match, failing...\n");
			else
				printf("succeding on level %d...\n",
					best_match);

	if (best_uid == NO_UID)
		return (NULL);

	pw = getpwuid(best_uid);
       	message(Arpa_Info, MSGSTR(RE_LOGIN, "sending to login name %s"), pw->pw_name);
	return (pw);
#endif /* !FUZZY */
}
/*
**  PARTIALSTRING -- is one string of words contained by another?
**
**	See if one string of words can be found as part of
**	another string of words.  All substrings delimited by
**	one or more non-alphanumeric characters are considered
**	"words", and a partial match is such that all the words
**	of the pattern string are either full prefixes
**	of the target string.  Upper or lower case letters are
**	considered equal.
**
**	Parameters:
**		target -- target string
**		pattern -- pattern string
**
**	Returns:
**		The number of fully matched words, or -1 if none.
**
**	Side Effects:
**		None.
**
*/

#ifdef FUZZY
partialstring(target, pattern)
    char *target;
    char *pattern;
{
    register char *t, *p, *q;
    int full_words = 0;

    /* skip initial delimiters */
    for (t = target; *t != '\0' && !isalnum(*t); t++);
    for (p = pattern; *p != '\0' && !isalnum(*p); p++);
    q = p;

    while (*t != '\0' && *p != '\0') {
	/*
	 * if at end of pattern word, find next, remember it,
	 * and eat the current target word
	 */
	if (!isalnum(*p)) {
	    while (*p != '\0' && !isalnum(*p)) p++;
	    if (*p == '\0')
		continue;
	    q = p;
	    if (!isalnum(*t)) {
		full_words++;
	    }
	    while (*t != '\0' && isalnum(*t)) t++;
	    while (*t != '\0' && !isalnum(*t)) t++;
	    continue;
	}

	/*
	 * if match, advance both pointers
	 */
	if ((isupper(*t) ? tolower(*t) : *t) ==
	    (isupper(*p) ? tolower(*p) : *p)) {
	    t++, p++;
	    continue;
	}

	/*
	 * if no match, backtrack to last unmatched pattern word and
	 * eat current target word
	 */
	p = q;
	while (*t != '\0' && isalnum(*t)) t++;
	while (*t != '\0' && !isalnum(*t)) t++;
    }

    /*
     * now, the pattern should be fully consumed if there was a match
     */
    if (*p == '\0')
	return isalnum(*t) ? full_words : full_words + 1;
    else
	return -1;
}
#endif /* FUZZY */
/*
**  WRITABLE -- predicate returning if the file is writable.
**
**	This routine must duplicate the algorithm in sys/fio.c.
**	Unfortunately, we cannot use the access call since we
**	won't necessarily be the real uid when we try to
**	actually open the file.
**
**	Notice that ANY file with ANY execute bit is automatically
**	not writable.  This is also enforced by mailfile.
**
**	Parameters:
**		s -- pointer to a stat struct for the file.
**
**	Returns:
**		TRUE -- if we will be able to write this file.
**		FALSE -- if we cannot write this file.
**
**	Side Effects:
**		none.
*/

bool
writable(s)
	register struct stat *s;
{
	int euid, egid;
	int bits;
	gid_t group[NGROUPS], *gp;
	int groups;

	if (bitset(0111, s->st_mode))
		return (FALSE);
	euid = getruid();
	egid = getrgid();
	if (geteuid() == 0)
	{
		if (bitset(S_ISUID, s->st_mode))
			euid = s->st_uid;
		if (bitset(S_ISGID, s->st_mode))
			egid = s->st_gid;
	}

	if (euid == 0)
		return (TRUE);
	bits = S_IWRITE;
	if (euid != s->st_uid)
	{
		bits >>= 3;
		if (egid == s->st_gid)
			goto found;
		groups = getgroups(NGROUPS, &group[0]);
		for (gp = &group[0]; groups-- > 0; gp++)
			if (*gp == s->st_gid)
				goto found;
		bits >>= 3;
found:
		;
	}
	return ((s->st_mode & bits) != 0);
}
/*
**  INCLUDE -- handle :include: specification.
**
**	Parameters:
**		fname -- filename to include.
**		msg -- message to print in verbose mode.
**		ctladdr -- address template to use to fill in these
**			addresses -- effective user/group id are
**			the important things.
**		sendq -- a pointer to the head of the send queue
**			to put these addresses in.
**
**	Returns:
**		none.
**
**	Side Effects:
**		reads the :include: file and sends to everyone
**		listed in that file.
*/

include(fd, fname, msg, ctladdr, sendq)
	int fd;
	char *fname;
	char *msg;
	ADDRESS *ctladdr;
	ADDRESS **sendq;
{
	char buf[MAXLINE];
	register FILE *fp;
	char *oldto = CurEnv->e_to;
	char *oldfilename = FileName;
	int oldlinenumber = LineNumber;
	extern ADDRESS *getctladdr();

	if (fd < 0 || (fp = fdopen(fd, "r")) == NULL)
	{
		usrerr(MSGSTR(RC_OPEN, "Cannot open %s"), fname);
		return;
	}
	if (getctladdr(ctladdr) == NULL)
	{
		struct stat st;

		if (fstat(fd, &st) < 0)
			syserr(MSGSTR(RE_STAT, "Cannot fstat %s!"), fname);
		ctladdr->q_uid = st.st_uid;
		ctladdr->q_gid = st.st_gid;
		ctladdr->q_flags |= QGOODUID;
	}

	/* read the file -- each line is a comma-separated list. */
	FileName = fname;
	LineNumber = 0;
	while (fgets(buf, sizeof buf, fp) != NULL)
	{
		register char *p = index(buf, '\n');

		LineNumber++;
		if (p != NULL)
			*p = '\0';

		/* ignore null lines and # comment lines */
		if ((buf[0] == '\0') || (buf[0] == '#'))
			continue;
		CurEnv->e_to = oldto;
		message(Arpa_Info, "%s to %s", msg, buf);
		AliasLevel++;
		sendtolist(buf, ctladdr, sendq);
		AliasLevel--;
	}

	(void) fclose(fp);
	FileName = oldfilename;
	LineNumber = oldlinenumber;
}
/*
**  SENDTOARGV -- send to an argument vector.
**
**	Parameters:
**		argv -- argument vector to send to.
**
**	Returns:
**		none.
**
**	Side Effects:
**		puts all addresses on the argument vector onto the
**			send queue.
*/

sendtoargv(argv)
	register char **argv;
{
	register char *p;

	while ((p = *argv++) != NULL)
	{
		if (argv[0] != NULL && argv[1] != NULL && !strcasecmp(argv[0], "at"))
		{
			char nbuf[MAXNAME];

			if (strlen(p) + strlen(argv[1]) + 2 > sizeof nbuf)
				usrerr(MSGSTR(RE_OVFL, "address overflow"));
			else
			{
				(void) strcpy(nbuf, p);
				(void) strcat(nbuf, "@");
				(void) strcat(nbuf, argv[1]);
				p = newstr(nbuf);
				argv += 2;
			}
		}
		sendtolist(p, (ADDRESS *) NULL, &CurEnv->e_sendqueue);
	}
}
/*
**  GETCTLADDR -- get controlling address from an address header.
**
**	If none, get one corresponding to the effective userid.
**
**	Parameters:
**		a -- the address to find the controller of.
**
**	Returns:
**		the controlling address.
**
**	Side Effects:
**		none.
*/

ADDRESS *
getctladdr(a)
	register ADDRESS *a;
{
	while (a != NULL && !bitset(QGOODUID, a->q_flags))
		a = a->q_alias;
	return (a);
}
