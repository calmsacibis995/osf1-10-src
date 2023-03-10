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
static char	*sccsid = "@(#)$RCSfile: parseaddr.c,v $ $Revision: 4.2.3.3 $ (DEC) $Date: 1992/10/14 12:03:23 $";
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
# include "sendmail.h"

/*
**  PARSEADDR -- Parse an address
**
**	Parses an address and breaks it up into three parts: a
**	net to transmit the message on, the host to transmit it
**	to, and a user on that host.  These are loaded into an
**	ADDRESS header with the values squirreled away if necessary.
**	The "user" part may not be a real user; the process may
**	just reoccur on that machine.  For example, on a machine
**	with an arpanet connection, the address
**		csvax.bill@berkeley
**	will break up to a "user" of 'csvax.bill' and a host
**	of 'berkeley' -- to be transmitted over the arpanet.
**
**	Parameters:
**		addr -- the address to parse.
**		a -- a pointer to the address descriptor buffer.
**			If NULL, a header will be created.
**		copyf -- determines what shall be copied:
**			-1 -- don't copy anything.  The printname
**				(q_paddr) is just addr, and the
**				user & host are allocated internally
**				to parse.
**			0 -- copy out the parsed user & host, but
**				don't copy the printname.
**			+1 -- copy everything.
**		delim -- the character to terminate the address, passed
**			to prescan.
**
**	Returns:
**		A pointer to the address descriptor header (`a' if
**			`a' is non-NULL).
**		NULL on error.
**
**	Side Effects:
**		none
*/

/* following delimiters are inherent to the internal algorithms */
# define DELIMCHARS	"\001()<>,;\\\"\r\n"	/* word delimiters */

ADDRESS *
parseaddr(addr, a, copyf, delim)
	char *addr;
	register ADDRESS *a;
	int copyf;
	char delim;
{
	register char **pvp;
	register struct mailer *m;
	char pvpbuf[PSBUFSIZE];
	extern char **prescan();
	extern ADDRESS *buildaddr();

	/*
	**  Initialize and prescan address.
	*/

	CurEnv->e_to = addr;

	/*
	** Turning on this line has strange side effects in
	** received mail when there is an unknown user error
	** when mailing to an alias
	**
	** ie .. >>> MAIL From:<user@host>
	**	 <<<
	**	 >>> RCPT To:<user@host> (if .forward present)
	**	 <<<
	*/
	if (tTd(20, 1))
		printf("\n--parseaddr(%s)\n", addr);

	pvp = prescan(addr, delim, pvpbuf);
	if (pvp == NULL)
		return (NULL);

	/*
	**  Apply rewriting rules.
	**	Ruleset 0 does basic parsing.  It must resolve.
	*/

	rewrite(pvp, 3);
	rewrite(pvp, 0);

	/*
	**  See if we resolved to a real mailer.
	*/

	if (pvp[0][0] != CANONNET)
	{
		setstat(EX_USAGE);
		usrerr(MSGSTR(PA_RESLV, "cannot resolve name"));
		return (NULL);
	}

	/*
	**  Build canonical address from pvp.
	*/

	a = buildaddr(pvp, a);
	if (a == NULL)
		return (NULL);
	m = a->q_mailer;

	/*
	**  Make local copies of the host & user and then
	**  transport them out.
	*/

	if (copyf > 0)
	{
		extern char *DelimChar;
		char savec = *DelimChar;

		*DelimChar = '\0';
		a->q_paddr = newstr(addr);
		*DelimChar = savec;
	}
	else
		a->q_paddr = addr;

	if (a->q_user == NULL)
		a->q_user = "";
	if (a->q_host == NULL)
		a->q_host = "";

	if (copyf >= 0)
	{
		a->q_host = newstr(a->q_host);
		if (a->q_user != a->q_paddr)
			a->q_user = newstr(a->q_user);
	}

	/*
	**  Convert host name to lower case if requested.
	**	User name will be done later.
	*/

	if (!bitnset(M_HST_UPPER, m->m_flags))
		makelower(a->q_host);

	/*
	**  Compute return value.
	*/

	if (tTd(20, 1))
	{
		printf("parseaddr-->");
		printaddr(a, FALSE);
	}

	return (a);
}
/*
**  LOWERADDR -- map UPPER->lower case on addresses as requested.
**
**	Parameters:
**		a -- address to be mapped.
**
**	Returns:
**		none.
**
**	Side Effects:
**		none.
*/

loweraddr(a)
	register ADDRESS *a;
{
	register MAILER *m = a->q_mailer;

	if (!bitnset(M_USR_UPPER, m->m_flags))
		makelower(a->q_user);
}
/*
**  PRESCAN -- Prescan name and make it canonical
**
**	Scans a name and turns it into a set of tokens.  This process
**	deletes blanks and comments (in parentheses).
**
**	This routine knows about quoted strings and angle brackets.
**
**	There are certain subtleties to this routine.  The one that
**	comes to mind now is that backslashes on the ends of names
**	are silently stripped off; this is intentional.  The problem
**	is that some versions of sndmsg (like at LBL) set the kill
**	character to something other than @ when reading addresses;
**	so people type "csvax.eric\@berkeley" -- which screws up the
**	berknet mailer.
**
**	Parameters:
**		addr -- the name to chomp.
**		delim -- the delimiter for the address, normally
**			'\0' or ','; \0 is accepted in any case.
**			If '\t' then we are reading the .cf file.
**		pvpbuf -- place to put the saved text -- note that
**			the pointers are static.
**
**	Returns:
**		A pointer to a vector of tokens.
**		NULL on error.
**
**	Side Effects:
**		sets DelimChar to point to the character matching 'delim'.
*/

/* states and character types */
# define OPR		0	/* operator */
# define ATM		1	/* atom */
# define QST		2	/* in quoted string */
# define SPC		3	/* chewing up spaces */
# define ONE		4	/* pick up one character */

# define NSTATES	5	/* number of states */
# define TYPE		017	/* mask to select state type */

/* meta bits for table */
# define M		020	/* meta character; don't pass through */
# define B		040	/* cause a break */
# define MB		M|B	/* meta-break */

static short StateTab[NSTATES][NSTATES] =
{
   /*	oldst	chtype>	OPR	ATM	QST	SPC	ONE	*/
	/*OPR*/		OPR|B,	ATM|B,	QST|B,	SPC|MB,	ONE|B,
	/*ATM*/		OPR|B,	ATM,	QST|B,	SPC|MB,	ONE|B,
	/*QST*/		QST,	QST,	OPR,	QST,	QST,
	/*SPC*/		OPR,	ATM,	QST,	SPC|M,	ONE,
	/*ONE*/		OPR,	OPR,	OPR,	OPR,	OPR,
};

# define NOCHAR		-1	/* signal nothing in lookahead token */

char	*DelimChar;		/* set to point to the delimiter */

char **
prescan(addr, delim, pvpbuf)
	char *addr;
	char delim;
	char pvpbuf[];
{
	register char *p;
	register char *q;
	register int c;
	char **avp;
	bool bslashmode;
	int cmntcnt;
	int anglecnt;
	char *tok;
	int state;
	int newstate;
	static char *av[MAXATOM+1];
	extern int errno;

	/* make sure error messages don't have garbage on them */
	errno = 0;

	q = pvpbuf;
	bslashmode = FALSE;
	cmntcnt = 0;
	anglecnt = 0;
	avp = av;
	state = OPR;
	c = NOCHAR;
	p = addr;
	if (tTd(22, 45))
	{
		printf("prescan: ");
		xputs(p);
		(void) putchar('\n');
	}

	do
	{
		/* read a token */
		tok = q;
		for (;;)
		{
			/* store away any old lookahead character */
			if (c != NOCHAR)
			{
				/* see if there is room */
				if (q >= &pvpbuf[PSBUFSIZE - 5])
				{
					usrerr(MSGSTR(PA_TOLNG, "Address too long"));
					DelimChar = p;
					return (NULL);
				}

				/* squirrel it away */
				*q++ = c;
			}

			/* read a new input character */
			c = *p++;
			if (c == '\0')
				break;
# ifdef EIGHTBIT
			if (!Allow8bit)
# endif /* EIGHTBIT */
				c &= ~0200;

			if (tTd(22, 101))
				printf("c=%c, s=%d; ", c, state);

			/* chew up special characters */
			*q = '\0';
			if (bslashmode)
			{
# ifdef EIGHTBIT
				if (!Allow8bit) 
# endif /* EIGHTBIT */
					/* kludge \! for naive users */
					if (c != '!')
						c |= 0200;
				bslashmode = FALSE;
			}
			else if (c == '\\')
			{
				bslashmode = TRUE;
				c = NOCHAR;
			}
			else if (state == QST)
			{
				/* do nothing, just avoid next clauses */
			}
			else if (c == '(')
			{
				cmntcnt++;
				c = NOCHAR;
			}
			else if (c == ')')
			{
				if (cmntcnt <= 0)
				{
					usrerr(MSGSTR(PA_PAREN, "Unbalanced ')'"));
					DelimChar = p;
					return (NULL);
				}
				else
					cmntcnt--;
			}
			else if (cmntcnt > 0)
				c = NOCHAR;
			else if (c == '<')
				anglecnt++;
			else if (c == '>')
			{
				if (anglecnt <= 0)
				{
					usrerr(MSGSTR(PA_GR, "Unbalanced '>'"));
					DelimChar = p;
					return (NULL);
				}
				anglecnt--;
			}
			else if (delim == ' ' && isspace(c))
				c = ' ';

			if (c == NOCHAR)
				continue;

			/* see if this is end of input */
			if (c == delim && anglecnt <= 0 && state != QST)
				break;

			newstate = StateTab[state][toktype(c)];
			if (tTd(22, 101))
				printf("ns=%02o\n", newstate);
			state = newstate & TYPE;
			if (bitset(M, newstate))
				c = NOCHAR;
			if (bitset(B, newstate))
				break;
		}

		/* new token */
		if (tok != q)
		{
			*q++ = '\0';
			if (tTd(22, 36))
			{
				printf("tok=");
				xputs(tok);
				(void) putchar('\n');
			}
			if (avp >= &av[MAXATOM])
			{
				syserr(MSGSTR(PA_SCAN, "prescan: too many tokens"));
				DelimChar = p;
				return (NULL);
			}
			*avp++ = tok;
		}
	} while (c != '\0' && (c != delim || anglecnt > 0));
	*avp = NULL;
	DelimChar = --p;
	if (cmntcnt > 0)
		usrerr(MSGSTR(PA_PAREN2,"Unbalanced '('"));
	else if (anglecnt > 0)
		usrerr(MSGSTR(PA_LT, "Unbalanced '<'"));
	else if (state == QST)
		usrerr(MSGSTR(PS_SLASH,"Unbalanced '\"'"));
	else if (av[0] != NULL)
		return (av);
	return (NULL);
}
/*
**  TOKTYPE -- return token type
**
**	Parameters:
**		c -- the character in question.
**
**	Returns:
**		Its type.
**
**	Side Effects:
**		none.
*/

toktype(c)
	register char c;
{
	static char buf[50];
	static bool firstime = TRUE;

	if (firstime)
	{
		firstime = FALSE;
		expand("\001o", buf, &buf[sizeof buf - 1], CurEnv);
		(void) strcat(buf, DELIMCHARS);
	}
	if (c == MATCHCLASS || c == MATCHREPL || c == MATCHNCLASS)
		return (ONE);
#ifdef MACVALUE
	if (c == MACVALUE)
		return(ONE);
#endif /* MACVALUE */
	if (c == '"')
		return (QST);
	if (!isascii(c))
		return (ATM);
	if (isspace(c) || c == ')')
		return (SPC);
	if (iscntrl(c) || index(buf, c) != NULL)
		return (OPR);
	return (ATM);
}
/*
**  REWRITE -- apply rewrite rules to token vector.
**
**	This routine is an ordered production system.  Each rewrite
**	rule has a LHS (called the pattern) and a RHS (called the
**	rewrite); 'rwr' points the the current rewrite rule.
**
**	For each rewrite rule, 'avp' points the address vector we
**	are trying to match against, and 'pvp' points to the pattern.
**	If pvp points to a special match value (MATCHZANY, MATCHANY,
**	MATCHONE, MATCHCLASS, MATCHNCLASS) then the address in avp
**	matched is saved away in the match vector (pointed to by 'mvp').
**
**	When a match between avp & pvp does not match, we try to
**	back out.  If we back up over MATCHONE, MATCHCLASS, or MATCHNCLASS
**	we must also back out the match in mvp.  If we reach a
**	MATCHANY or MATCHZANY we just extend the match and start
**	over again.
**
**	When we finally match, we rewrite the address vector
**	and try over again.
**
**	Parameters:
**		pvp -- pointer to token vector.
**
**	Returns:
**		none.
**
**	Side Effects:
**		pvp is modified.
*/

struct match
{
	char	**first;	/* first token matched */
	char	**last;		/* last token matched */
};

# define MAXMATCH	9	/* max params per rewrite */

static int nrw;

rewrite(pvp, ruleset)
	char **pvp;
	int ruleset;
{
	nrw = 0;
	_rewrite(pvp, ruleset);
}

_rewrite(pvp, ruleset)
	char **pvp;
	int ruleset;
{
	register char *ap;		/* address pointer */
	register char *rp;		/* rewrite pointer */
	register char **avp;		/* address vector pointer */
	register char **rvp;		/* rewrite vector pointer */
	register struct match *mlp;	/* cur ptr into mlist */
	register struct rewrite *rwr;	/* pointer to current rewrite rule */
	struct match mlist[MAXMATCH];	/* stores match on LHS */
	char *npvp[MAXATOM+1];		/* temporary space for rebuild */
	char tokbuf[MAXNAME+1];         /* for concatenated class tokens */
	int nloops, nmatches = 0;       /* for looping rule checks */

	if (OpMode == MD_TEST || tTd(21, 2))
	{
		printf("rewrite: ruleset %2d   input:", ruleset);
		printcav(pvp);
	}
	if (pvp == NULL)
		return;

	if (++nrw > 100) {
		char buf[MAXLINE];

		buf[0] = buf[MAXLINE-1] = 0;
		while (*pvp)
			strncat(buf, *pvp++, sizeof buf);
		syserr(MSGSTR(PA_ADLOOP, "Address causes rewrite loop: <%s>"), buf);
		return;
	}

	/*
	**  Run through the list of rewrite rules, applying
	**	any that match.
	*/

	for (rwr = RewriteRules[ruleset]; rwr != NULL; )
	{
		if (tTd(21, 12))
		{
			printf("-----trying rule:");
			printcav(rwr->r_lhs);
		}

		/* try to match on this rule */
		mlp = mlist;
		rvp = rwr->r_lhs;
		avp = pvp;
		nloops = 0;
		while ((ap = *avp) != NULL || *rvp != NULL)
		{
			if (nloops++ > 200) {
				syserr(MSGSTR(PA_RSLOOP, "looping on ruleset %d, rule %d"),
					ruleset, rwr-RewriteRules[ruleset]);
				break;
			}
			rp = *rvp;
			if (tTd(21, 35))
			{
				printf("ap=");
				xputs(ap);
				printf(", rp=");
				xputs(rp);
				printf("\n");
			}
			if (rp == NULL)
			{
				/* end-of-pattern before end-of-address */
				goto backup;
			}
			if (ap == NULL && *rp != MATCHZANY)
			{
				/* end-of-input */
				break;
			}

			switch (*rp)
			{
				register STAB *s;
				char **oldavp;

			  case MATCHNCLASS:
				/* match any single token not in a class */
				s = stab(ap, ST_CLASS, ST_FIND);
				if (s != NULL && bitnset(rp[1], s->s_class))
					goto backup;

				/* match exactly one token */
				mlp->first = avp;
				mlp->last = avp++;
				mlp++;
				break;

			  case MATCHCLASS:
				/* match any token in a class */
				/* slow, concat version by lel@ida.liu.se */
				/* handles multi-token class matches, though */
				oldavp = avp;
				*tokbuf = NULL;
				do {
					if (*avp == NULL) {
						avp = oldavp;
					goto backup;
					}
					strcat(tokbuf, *avp++);
					s = stab(tokbuf, ST_CLASS, ST_FIND);
				} while (s == NULL || !bitnset(rp[1], s->s_class));

				mlp->first = oldavp;
				mlp->last = avp-1;
				mlp++;
				break;

			  case MATCHONE:
			  case MATCHANY:
				/* match exactly one token */
				mlp->first = avp;
				mlp->last = avp++;
				mlp++;
				break;

			  case MATCHZANY:
				/* match zero or more tokens */
				mlp->first = avp;
				mlp->last = avp - 1;
				mlp++;
				break;

#ifdef MACVALUE
			  case MACVALUE:
				{
				extern char *macvalue();
				char *p = macvalue(rp[1], CurEnv);
				if (tTd(21, 2))
					printf("expanding runtime macro '%c' to \"%s\"\n",
					    rp[1], p ? p : "(null)");
				oldavp = avp;
				if (p) while (*p) {
					if (*avp == NULL ||
					   strncasecmp(p,*avp,strlen(*avp))) {
						avp = oldavp;
						goto backup;
					   }
					p += strlen(*avp++);
					}
				mlp->first = oldavp;
				mlp->last = avp -1;
				}
				mlp++;
				break;
#endif MACVALUE

			  default:
				/* must have exact match */
				if (strcasecmp(rp, ap))
					goto backup;
				avp++;
				break;
			}

			/* successful match on this token */
			rvp++;
			continue;

		  backup:
			/* match failed -- back up */
			while (--rvp >= rwr->r_lhs)
			{
				rp = *rvp;
				if (*rp == MATCHCLASS) {
				  register STAB *s;
				  char **oldavp;

				  /* attempt to extend binding */
				  /* slow, concat version by lel@ida.liu.se */

				  oldavp = avp;
				  *tokbuf = NULL;
				  for (avp = mlp[-1].first;
				       avp <= mlp[-1].last; avp++)
				    strcat(tokbuf, *avp);

				  do {
				    if (*avp == NULL) {
				      /* back out binding */
				      avp = oldavp;
				      mlp--;
				      goto cantextend;
				    }
				    strcat(tokbuf, *avp++);
				    s = stab(tokbuf, ST_CLASS, ST_FIND);
				  } while (s == NULL ||
				           !bitnset(rp[1], s->s_class));

				  /* found an extension */
				  mlp[-1].last = avp-1;
				  rvp++;
				  break;
				}
			      cantextend:
				if (*rp == MATCHANY || *rp == MATCHZANY)
				{
					/* extend binding and continue */
					avp = ++mlp[-1].last;
					avp++;
					rvp++;
					break;
				}
				avp--;
				if (*rp == MATCHONE || *rp == MATCHNCLASS)
				{
					/* back out binding */
					mlp--;
				}
			}

			if (rvp < rwr->r_lhs)
			{
				/* total failure to match */
				break;
			}
		}

		/*
		**  See if we successfully matched
		*/

		if (rvp < rwr->r_lhs || *rvp != NULL)
		{
			if (tTd(21, 10))
				printf("----- rule fails\n");
			rwr = rwr->r_next;
			continue;
		}

		if (nmatches++ > 200) {
			syserr(MSGSTR(PA_RSLOOP2, "Loop in ruleset %d, rule %d (too many matches)"),
				ruleset, rwr-RewriteRules[ruleset]);
			rwr = rwr->r_next;
			nmatches = 0;
			continue;
		}

		rvp = rwr->r_rhs;
		if (tTd(21, 12))
		{
			printf("-----rule matches:");
			printcav(rvp);
		}

		rp = *rvp;
		if (*rp == CANONUSER)
		{
			rvp++;
			rwr = rwr->r_next;
			nmatches = 0;
		}
		else if (*rp == CANONHOST)
		{
			rvp++;
			rwr = NULL;
		}
		else if (*rp == CANONNET)
			rwr = NULL;

		/* substitute */
		for (avp = npvp; *rvp != NULL; rvp++)
		{
			register struct match *m;
			register char **pp;

			rp = *rvp;
			if (*rp == MATCHREPL)
			{
				/* substitute from LHS */
				m = &mlist[rp[1] - '1'];
				if (m >= mlp)
				{
					syserr(MSGSTR(PA_REWRITE, 
					"rewrite: ruleset %d: replacement out of bounds"), ruleset);
					return;
				}
				if (tTd(21, 15))
				{
					printf("$%c:", rp[1]);
					pp = m->first;
					while (pp <= m->last)
					{
						printf(" %x=\"", *pp);
						(void) fflush(stdout);
						printf("%s\"", *pp++);
					}
					printf("\n");
				}
				pp = m->first;
				while (pp <= m->last)
				{
					if (avp >= &npvp[MAXATOM])
					{
						syserr(MSGSTR(PA_TOLNG2, "rewrite: expansion too long"));
						return;
					}
					*avp++ = *pp++;
				}
			}
			else
			{
				/* vanilla replacement */
				if (avp >= &npvp[MAXATOM])
				{
	toolong:
					syserr(MSGSTR(PA_TOLNG, "rewrite: expansion too long"));
					return;
				}
#ifdef MACVALUE
				if (*rp == MACVALUE) {
					extern char *macvalue();
					char *p = macvalue(rp[1], CurEnv);
					if (tTd(21, 2))
						printf("expanding runtime macro '%c' to \"%s\"\n",
						rp[1], p ? p : "(null)");
					if (p)
						*avp++ = p;
				} else
#endif /* MACVALUE */
					*avp++ = rp;
			}
		}
		*avp++ = NULL;

		/*
		**  Check for any hostname lookups.
		*/

		for (rvp = npvp; *rvp != NULL; rvp++)
		{
			char **hbrvp, **ubrvp;
			char **xpvp;
			int trsize;
			char *olddelimchar;
			char hbuf[MAXNAME + 1], ubuf[MAXNAME + 1];
			char *pvpb1[MAXATOM + 1];
			char pvpbuf[PSBUFSIZE];
			bool match, defaultpart;
			char begintype;
			char db = '\0';
			extern char *DelimChar;

			if (**rvp != HOSTBEGIN && **rvp != KEYBEGIN)
				continue;

			/*
			**  Got a hostname or database lookup.
			**
			**	This could be optimized fairly easily.
			*/

			begintype = **rvp;
			hbrvp = rvp;
			ubrvp = NULL;

			/* read database name if that's what we're up for */
			if (begintype == KEYBEGIN) {
				if (*++rvp != NULL)
					db = **rvp;
			}

			/* extract the match part */
			if (begintype == HOSTBEGIN)
				while (*++rvp != NULL && **rvp != HOSTEND &&
				       **rvp != CANONUSER)
				    continue;
			else
			    while (*++rvp != NULL && **rvp != KEYEND &&
				   **rvp != CANONHOST && **rvp != CANONUSER)
				continue;
			/* got a sprintf argument? */
			if (**rvp == CANONHOST) {
			    *rvp = NULL;
			    ubrvp = rvp+1;
			    while (*++rvp != NULL && **rvp != KEYEND &&
				   **rvp != CANONUSER)
				continue;
			}
			defaultpart = **rvp == CANONUSER;
			if (*rvp != NULL)
				*rvp++ = NULL;

			/* Save the remainder of the input string */
			trsize = (int) (avp - rvp + 1) * sizeof *rvp;
			bcopy((char *) rvp, (char *) pvpb1, trsize);

			/* Look it up (lowercase version) */
			cataddr(hbrvp + (begintype == HOSTBEGIN ? 1 : 2),
				hbuf, sizeof hbuf);
			if (begintype == HOSTBEGIN)
			    match = maphostname(hbuf, sizeof hbuf);
			else {
			    if (ubrvp == NULL) {
				/* no sprintf argument part */
				match = mapkey(db, hbuf, sizeof hbuf, NULL);
			    } else {
				cataddr(ubrvp, ubuf, sizeof ubuf);
				match = mapkey(db, hbuf, sizeof hbuf, ubuf);
			    }
			}
			if (match || !defaultpart) {
				/* scan the new route/host name */
				olddelimchar = DelimChar;
				xpvp = prescan(hbuf, '\0', pvpbuf);
				DelimChar = olddelimchar;
				if (xpvp == NULL) {
					syserr("rewrite: cannot prescan %s: %s",
					       begintype == HOSTBEGIN ?
					       "new hostname" :
					       "dbm lookup result",
					       hbuf);
					return;
				}

				/* append it to the token list */
				for (avp = hbrvp; *xpvp != NULL; xpvp++) {
					*avp++ = newstr(*xpvp);
					if (avp >= &npvp[MAXATOM])
						goto toolong;
				}
			} else
				avp = hbrvp;

			/* restore the old trailing information */
			rvp = avp - 1;
			for (xpvp = pvpb1; *xpvp != NULL; xpvp++) {
			    if (defaultpart && (begintype == HOSTBEGIN ?
						**xpvp == HOSTEND :
						**xpvp == KEYEND)) {
				defaultpart = FALSE;
				rvp = avp - 1;
			    } else if (!defaultpart || !match)
				*avp++ = *xpvp;
			    if (avp >= &npvp[MAXATOM])
				goto toolong;
			}
			*avp++ = NULL;

			/*break;*/
		}

		/*
		**  Check for subroutine calls.
		**  Then copy vector back into original space.
		*/

		callsubr(npvp);

		for (avp = npvp; *avp++ != NULL;);
		bcopy((char *) npvp, (char *) pvp,
		      (int) (avp - npvp) * sizeof *avp);

		if (tTd(21, 4))
		{
			printf("rewritten as:");
			printcav(pvp);
		}
	}

	if (OpMode == MD_TEST || tTd(21, 2))
	{
		printf("rewrite: ruleset %2d returns:", ruleset);
		printcav(pvp);
	}
}
/*
**  CALLSUBR -- call subroutines in rewrite vector
**
**	Parameters:
**		pvp -- pointer to token vector.
**
**	Returns:
**		none.
**
**	Side Effects:
**		pvp is modified.
*/

callsubr(pvp)
    char **pvp;
{
    char **rvp;
    int subr;

    for (; *pvp != NULL; pvp++)
	if (**pvp == CALLSUBR) {

	    subr = atoi(pvp[1]);

	    if (tTd(21, 3))
		printf("-----callsubr %d\n", subr);

	    /*
	    **  Take care of possible inner calls.
	    */
	    callsubr(pvp+2);

	    /*
	    **  Move vector up over calling opcode.
	    */
	    for (rvp = pvp+2; *rvp != NULL; rvp++)
		rvp[-2] = rvp[0];
	    rvp[-2] = NULL;

	    /*
	    **  Call inferior ruleset.
	    */
	    _rewrite(pvp, subr);

	    break;
	}
}
/*
**  BUILDADDR -- build address from token vector.
**
**	Parameters:
**		tv -- token vector.
**		a -- pointer to address descriptor to fill.
**			If NULL, one will be allocated.
**
**	Returns:
**		NULL if there was an error.
**		'a' otherwise.
**
**	Side Effects:
**		fills in 'a'
*/

ADDRESS *
buildaddr(tv, a)
	register char **tv;
	register ADDRESS *a;
{
	static char buf[MAXNAME];
	struct mailer **mp;
	register struct mailer *m;

	if (a == NULL)
		a = (ADDRESS *) xalloc(sizeof *a);
	bzero((char *) a, sizeof *a);

	/* figure out what net/mailer to use */
	if (**tv != CANONNET)
	{
		syserr(MSGSTR(PA_NONET, "buildaddr: no net"));
		return (NULL);
	}
	tv++;
	if (!strcasecmp(*tv, "error"))
	{
		if (**++tv == CANONHOST)
		{
			setstat(atoi(*++tv));
			tv++;
		}
		if (**tv != CANONUSER)
			syserr(MSGSTR(PA_USR, "buildaddr: error: no user"));
		buf[0] = '\0';
		while (*++tv != NULL)
		{
			if (buf[0] != '\0')
				(void) strcat(buf, " ");
			(void) strcat(buf, *tv);
		}
		usrerr(buf);
		return (NULL);
	}
	for (mp = Mailer; (m = *mp++) != NULL; )
	{
		if (!strcasecmp(m->m_name, *tv))
			break;
	}
	if (m == NULL)
	{
		syserr(MSGSTR(PA_MAILER, "buildaddr: unknown mailer %s"), *tv);
		return (NULL);
	}
	a->q_mailer = m;

	/* figure out what host (if any) */
	if (**++tv != CANONHOST) {
		if (!bitnset(M_LOCAL, m->m_flags)) {
			syserr(MSGSTR(PA_NHOST, "buildaddr: no host"));
			return (NULL);
		} else
			a->q_host = NULL;
	} else {
		buf[0] = '\0';
		while (*++tv != NULL && **tv != CANONUSER)
			(void) strcat(buf, *tv);
		a->q_host = newstr(buf);
	}

	/* figure out the user */
	if (*tv == NULL || **tv != CANONUSER)
	{
		syserr(MSGSTR(PA_NUSER, "buildaddr: no user"));
		return (NULL);
	}

	/* define tohost before running mailer rulesets */
	define('h', a->q_host, CurEnv);

	/* rewrite according recipient mailer rewriting rules */
	rewrite(++tv, 2);
	if (m->m_re_rwset > 0)
		rewrite(tv, m->m_re_rwset);
	rewrite(tv, 4);

	/* save the result for the command line/RCPT argument */
	cataddr(tv, buf, sizeof buf);
	a->q_user = buf;

	return (a);
}
/*
**  CATADDR -- concatenate pieces of addresses (putting in <LWSP> subs)
**
**	Parameters:
**		pvp -- parameter vector to rebuild.
**		buf -- buffer to build the string into.
**		sz -- size of buf.
**
**	Returns:
**		none.
**
**	Side Effects:
**		Destroys buf.
*/

cataddr(pvp, buf, sz)
	char **pvp;
	char *buf;
	register int sz;
{
	bool oatomtok = FALSE;
	bool natomtok = FALSE;
	register int i;
	register char *p;

	if (pvp == NULL)
	{
		(void) strcpy(buf, "");
		return;
	}
	p = buf;
	sz -= 2;
	while (*pvp != NULL && (i = strlen(*pvp)) < sz)
	{
		natomtok = (toktype(**pvp) == ATM);
		if (oatomtok && natomtok)
			*p++ = SpaceSub;
		(void) strcpy(p, *pvp);
		oatomtok = natomtok;
		p += i;
		sz -= i + 1;
		pvp++;
	}
	*p = '\0';
}
/*
**  SAMEADDR -- Determine if two addresses are the same
**
**	This is not just a straight comparison -- if the mailer doesn't
**	care about the host we just ignore it, etc.
**
**	Parameters:
**		a, b -- pointers to the internal forms to compare.
**
**	Returns:
**		TRUE -- they represent the same mailbox.
**		FALSE -- they don't.
**
**	Side Effects:
**		none.
*/

bool
sameaddr(a, b)
	register ADDRESS *a;
	register ADDRESS *b;
{
	/* if they don't have the same mailer, forget it */
	if (a->q_mailer != b->q_mailer)
		return (FALSE);

	/* if the user isn't the same, we can drop out */
	if (strcasecmp(a->q_user, b->q_user))
		return (FALSE);

	/* if the mailer ignores hosts, we have succeeded! */
	if (bitnset(M_LOCAL, a->q_mailer->m_flags))
		return (TRUE);

	/* otherwise compare hosts (but be careful for NULL ptrs) */
	if (a->q_host == NULL || b->q_host == NULL)
		return (FALSE);
	if (strcasecmp(a->q_host, b->q_host))
		return (FALSE);

	return (TRUE);
}
/*
**  PRINTADDR -- print address (for debugging)
**
**	Parameters:
**		a -- the address to print
**		follow -- follow the q_next chain.
**
**	Returns:
**		none.
**
**	Side Effects:
**		none.
*/

printaddr(a, follow)
	register ADDRESS *a;
	bool follow;
{
	bool first = TRUE;

	while (a != NULL)
	{
		first = FALSE;
		printf("%x=", a);
		(void) fflush(stdout);
		printf("%s: mailer %d (%s), host `%s', user `%s', ruser `%s'\n",
			 a->q_paddr, a->q_mailer->m_mno, a->q_mailer->m_name,
			 a->q_host, a->q_user, a->q_ruser? a->q_ruser: "<null>");
		printf("\tnext=%x, flags=%o, alias %x\n", a->q_next, a->q_flags,
		       a->q_alias);
		printf("\thome=\"%s\", fullname=\"%s\"\n", a->q_home,
		       a->q_fullname);

		if (!follow)
			return;
		a = a->q_next;
	}
	if (first)
		printf("[NULL]\n");
}

/*
**  REMOTENAME -- return the name relative to the current mailer
**
**	Parameters:
**		name -- the name to translate.
**		m -- the mailer that we want to do rewriting relative
**			to.
**		senderaddress -- if set, uses the sender rewriting rules
**			rather than the recipient rewriting rules.
**		canonical -- if set, strip out any comment information,
**			etc.
**		headeraddress -- if set, use header specific rewriting
**			rulesets and uurelativize if M_RELATIVIZE is set.
**
**	Returns:
**		the text string representing this address relative to
**			the receiving mailer.
**
**	Side Effects:
**		none.
**
**	Warnings:
**		The text string returned is tucked away locally;
**			copy it if you intend to save it.
*/

char *
remotename(name, m, senderaddress, canonical, headeraddress)
	char *name;
	struct mailer *m;
	bool senderaddress;
	bool canonical;
	bool headeraddress;
{
	register char **pvp;
	char *fancy;
	extern char *macvalue();
	char *oldg = macvalue('g', CurEnv);
	static char buf[MAXNAME];
	char lbuf[MAXNAME];
	char pvpbuf[PSBUFSIZE];
	extern char **prescan();
	extern char *crackaddr();

	if (tTd(12, 1))
		printf("remotename(%s)\n", name);

	/* don't do anything if we are tagging it as special */
	if ((senderaddress ?
	     (headeraddress ? m->m_sh_rwset : m->m_se_rwset) :
	     (headeraddress ? m->m_rh_rwset : m->m_re_rwset)) < 0)
		return (name);

	/*
	**  Do a heuristic crack of this name to extract any comment info.
	**	This will leave the name as a comment and a $g macro.
	*/

	if (canonical)
		fancy = "\001g";
	else
		fancy = crackaddr(name);

	/*
	**  Turn the name into canonical form.
	**	Normally this will be RFC 822 style, i.e., "user@domain".
	**	If this only resolves to "user", and the "C" flag is
	**	specified in the sending mailer, then the sender's
	**	domain will be appended.
	*/

	pvp = prescan(name, '\0', pvpbuf);
	if (pvp == NULL)
		return (name);
	rewrite(pvp, 3);
	if (CurEnv->e_fromdomain != NULL)
	{
		/* append from domain to this address */
		register char **pxp = pvp;

		/* see if there is an "@domain" in the current name */
		while (*pxp != NULL && strcmp(*pxp, "@") != 0)
			pxp++;
		if (*pxp == NULL)
		{
			/* no.... append the "@domain" from the sender */
			register char **qxq = CurEnv->e_fromdomain;

			while ((*pxp++ = *qxq++) != NULL)
				continue;
			rewrite(pvp, 3);
		}
	}

	/*
	**  Do more specific rewriting.
	**	Rewrite using ruleset 1 or 2 for envelope addresses and
	**	5 or 6 for header addresses depending on whether this
	**	is a sender address or not.
	**	Then run it through any receiving-mailer-specific rulesets.
	*/

	if (senderaddress) {
		if (headeraddress) {
			rewrite(pvp, SplitRewriting ? 5 : 1);
			if (m->m_sh_rwset > 0)
				rewrite(pvp, m->m_sh_rwset);
		} else {
			rewrite(pvp, 1);
			if (m->m_se_rwset > 0)
				rewrite(pvp, m->m_se_rwset);
		}
	} else {
		if (headeraddress) {
			rewrite(pvp, SplitRewriting ? 6 : 2);
			if (m->m_rh_rwset > 0)
				rewrite(pvp, m->m_rh_rwset);
		} else {
			rewrite(pvp, 2);
			if (m->m_re_rwset > 0)
				rewrite(pvp, m->m_re_rwset);
		}
	}

	/*
	**  Do any final sanitation the address may require.
	**	This will normally be used to turn internal forms
	**	(e.g., user@host.LOCAL) into external form.  This
	**	may be used as a default to the above rules.
	*/

	rewrite(pvp, 4);

	/*
	**  Check if we're supposed to do make the address
	**  UUCP !-relative to the rcpt host vs ourselves.
	*/

	if (headeraddress && bitnset(M_RELATIVIZE, m->m_flags))
		uurelativize("\001k", "\001h", pvp);

	/*
	**  Now restore the comment information we had at the beginning.
	*/

	cataddr(pvp, lbuf, sizeof lbuf);
	define('g', lbuf, CurEnv);
	expand(fancy, buf, &buf[sizeof buf - 1], CurEnv);
	define('g', oldg, CurEnv);

	if (tTd(12, 1))
		printf("remotename => `%s'\n", buf);
	return (buf);
}
/*
**  UURELATIVIZE -- Make an address !-relative to recipient/sender nodes
**
**      Parameters:
**	      from -- the sending node (usually "$k" or "$w")
**	      to -- the receiving node (usually "$h")
**	      pvp -- address vector
**
**      Returns:
**	      none.
**
**      Side Effects:
**	      The pvp is rewritten to be relative the "to" node
**	      wrt the "from" node.  In other words, if the pvp
**	      is headed by "to!" that part is stripped; otherwise
**	      "from!" is prepended.  Exception: "to!user" addresses
**	      with no '!'s in the user part are sent as is.
**
**      Bugs:
**	      The pvp may overflow, but we don't catch it.
*/

uurelativize(from, to, pvp)
	char *from, *to;
	char **pvp;
{
	register char **pxp = pvp;
	char expfrom[MAXNAME], expto[MAXNAME];

	expand(from, expfrom, &expfrom[sizeof expfrom - 1], CurEnv);
	expand(to, expto, &expto[sizeof expto - 1], CurEnv);

	/*
	 * supposing that we've got something, should
	 * we add "from!" or remove "to!"?
	 */
	if (pvp[0] != NULL)
	    if (pvp[1] == NULL || strcmp(pvp[1], "!") != 0 ||
		    /*strcasecmp?*/ strcmp(pvp[0], expto) != 0) {
		/* either local name, no UUCP address, or not to "to!"  */
		/* ==> prepend address with "from!"			*/

		/* already there? */
		if (pvp[1] == NULL || strcmp(pvp[1], "!") != 0 ||
			/*strcasecmp?*/ strcmp(pvp[0], expfrom) != 0) {
		    /* no, put it there */
		    while (*pxp != NULL)
			pxp++;
		    do
			pxp[2] = *pxp;
		    while (pxp-- != pvp);
		    pvp[0] = newstr(expfrom);
		    pvp[1] = "!";
		}
	    } else {
		/* address is to "to!" -- remove if not "to!user" */
		for (pxp = &pvp[2]; *pxp != NULL && strcmp(*pxp, "!") != 0; pxp++);
		if (*pxp != NULL)
		    for (pxp = pvp; *pxp != NULL; pxp++)
			*pxp = pxp[2];
	    }
}
