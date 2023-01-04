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
static char	*sccsid = "@(#)$RCSfile: alias.c,v $ $Revision: 4.2.3.6 $ (DEC) $Date: 1992/10/30 15:05:49 $";
#endif 
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0
 */

# include "sendmail.h"

#if !defined(lint) && !defined(_NOIDENT)
#ifdef DBM

#else

#endif
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
#ifdef DBM
static char sccsid[] = "alias.c	5.21 (Berkeley) 6/1/90 (with DBM)";
#else
static char sccsid[] = "alias.c	5.21 (Berkeley) 6/1/90 (without DBM)";
#endif
#endif 
*/

# include <pwd.h>
# include <sys/types.h>
# include <sys/stat.h>
# include <signal.h>
#ifdef SVC_FILE
# include <sys/svcinfo.h>
#endif /* SVC_FILE */
# include <errno.h>
# include <sys/param.h>
# include <sys/file.h>
#ifdef LOCKF
# include <unistd.h>
#endif /* LOCKF */

/*
**  ALIAS -- Compute aliases.
**
**	Scans the alias file for an alias for the given address.
**	If found, it arranges to deliver to the alias list instead.
**	Uses libdbm database if -DDBM.
**
**	Parameters:
**		a -- address to alias.
**		sendq -- a pointer to the head of the send queue
**			to put the aliases in.
**
**	Returns:
**		none
**
**	Side Effects:
**		Aliases found are expanded.
**
**	Notes:
**		If NoAlias (the "-n" flag) is set, no aliasing is
**			done.
**
**	Deficiencies:
**		It should complain about names that are aliased to
**			nothing.
*/


alias(a, sendq)
	register ADDRESS *a;
	ADDRESS **sendq;
{
	register char *p;
	extern char *aliaslookup();

	if (tTd(27, 1))
		printf("alias(%s)\n", a->q_user);

	/* don't realias already aliased names */
	if (bitset(QDONTSEND, a->q_flags))
		return;

	CurEnv->e_to = a->q_paddr;

	/*
	**  Look up this name
	*/

	if (NoAlias)
		p = NULL;
	else
		p = aliaslookup(a->q_user);
	if (p == NULL)
		return;

	/*
	**  Have done successful aliaslookup() so mark
	**  original address as being an alias.
	*/
	a->q_flags |= QISALIAS;

	/*
	**  Match on Alias.
	**	Deliver to the target list.
	*/
	if (tTd(27, 1))
		printf("%s (%s, %s) aliased to %s\n",
		    a->q_paddr, a->q_host, a->q_user, p);
	message(Arpa_Info, "aliased to %s", p);
	AliasLevel++;
	sendtolist(p, a, sendq);
#ifdef DBM
	free(p);
#endif /* DBM */
	AliasLevel--;
}
/*
**  ALIASLOOKUP -- look up a name in the alias file.
**
**	Parameters:
**		name -- the name to look up.
**
**	Returns:
**		the value of name.
**		NULL if unknown.
**
**	Side Effects:
**		none.
**
**	Warnings:
**		The return value will be trashed across calls
**		unless NDBM is defined and we're using mapkey().
*/

char *
aliaslookup(name)
	char *name;
{
#ifdef DBM
# ifdef SVC_FILE
	static struct svcinfo *svcinfo;
# endif /* SVC_FILE */
	char *newname, *lookuptype;
	int i;

	if (tTd(27,3))
		printf("aliaslookup(\"%s\")...\n", name);

	newname = NULL;
# ifdef SVC_FILE
	/*
	**	Get order of lookup from the svc.conf file and lookup the
	**	alias in that order.
	*/
	if ((svcinfo = getsvc()) != NULL) 
		for (i=0; (svc_lastlookup = svcinfo->svcpath[SVC_ALIASES][i]) != SVC_LAST && newname == NULL; i++)
			switch(svc_lastlookup) {
			case SVC_LOCAL:
				if (newname = (char *) mapkey(DB_ALIAS, name, 0, 0))
					lookuptype = "LOCAL";
				break;
#ifdef YP
			case SVC_YP:
				if (newname = (char *) mapkey(YP_ALIAS, name, 0, 0))
					lookuptype = "YP";
				break;
#endif /* YP */
#ifdef HESIOD
			case SVC_BIND:
				if (newname = (char *) mapkey(HS_ALIAS, name, 0, 0))
					lookuptype = "HESIOD";
				break;
#endif /* HESIOD */
			}
	/*
	**	If error in reading svc.conf file - read local.
	*/
	else
		if (newname = (char *) mapkey(DB_ALIAS, name, 0, 0))
			lookuptype = "LOCAL";
# else /* !SVC_FILE */
	if (newname = (char *) mapkey(DB_ALIAS, name, 0, 0))
		lookuptype = "LOCAL";
# endif /* SVC_FILE */

	if (tTd(27,3))
	{
		printf("aliaslookup(\"%s\") => ", name);
		if (newname == NULL)
			printf("NOT_FOUND\n");
		else
			printf("%s (%s)\n", newname, lookuptype);
	}
	return(newname);

#else /* DBM */

	register STAB *s;

	s = stab(name, ST_ALIAS, ST_FIND);
	if (tTd(27,3))
		printf("aliaslookup(\"%s\") => %s\n", name,
			s == NULL ? "NOT_FOUND" : s->s_alias);
	if (s == NULL)
		return(NULL);
	return(s->s_alias);
#endif /* DBM */
}
/*
**  INITALIASES -- initialize for aliasing
**
**	Very different depending on whether we are running DBM or not.
**
**	Parameters:
**		init -- if set and if DBM, initialize the DBM files.
**
**	Returns:
**		none.
**
**	Side Effects:
**		initializes aliases:
**		if DBM:  opens the database.
**		if ~DBM: reads the aliases into the symbol table.
*/

# define DBMMODE	0644

initaliases(init)
	bool init;
{
#ifdef DBM
	int atcnt;
	time_t modtime;
	bool automatic = FALSE;
	char buf[MAXNAME];
#endif /* DBM */
	struct stat stb;
	static bool initialized = FALSE;

	if (initialized)
		return;
	initialized = TRUE;

	if (AliasFile == NULL ||
#ifdef YP
	    (AliasFile[0] != YPMARK &&
#endif /* YP */
	     stat(AliasFile, &stb) < 0)
#ifdef YP
	    )
#endif /* YP */
	{
		if (AliasFile != NULL && init)
			syserr(MSGSTR(AL_CANTOPEN, "Cannot open %s"),AliasFile);
		NoAlias = TRUE;
		errno = 0;
		return;
	}

# ifdef DBM
	/*
	**  Check to see that the alias file is complete.
	**	If not, we will assume that someone died, and it is up
	**	to us to rebuild it.
	*/

#ifndef NDBM
	if (!init)
		dbminit(AliasFile);
#endif /* !NDBM */
	atcnt = SafeAlias * 2;
	if (atcnt > 0)
		while (!init && atcnt-- >= 0 && mapkey(DB_ALIAS, "@", 0, 0) == NULL)
			sleep(30);
	else
		atcnt = 1;

	/*
	**  See if the DBM version of the file is out of date with
	**  the text version.  If so, go into 'init' mode automatically.
	**	This only happens if our effective userid owns the DBM.
	**	Note the unpalatable hack to see if the stat succeeded.
	*/

	modtime = stb.st_mtime;
	(void) strcpy(buf, AliasFile);
	(void) strcat(buf, DB_PAGEXT);
	stb.st_ino = 0;
	if (!init &&
#ifdef YP
	    AliasFile[0] != YPMARK &&
#endif /* YP */
	    (stat(buf, &stb) < 0 || stb.st_mtime < modtime || atcnt < 0))
	{
		errno = 0;
		if (AutoRebuild && stb.st_ino != 0 && stb.st_uid == geteuid())
		{
			init = TRUE;
			automatic = TRUE;
			message(Arpa_Info, MSGSTR(AL_REBUILD, "rebuilding alias database"));
#ifdef LOG
			if (LogLevel >= 7)
				syslog(LOG_INFO, MSGSTR(AL_REBUILD, "rebuilding alias database"));
#endif /* LOG */
		}
		else
		{
#ifdef LOG
			if (LogLevel >= 7)
				syslog(LOG_INFO, MSGSTR(AL_OFD, "alias database out of date"));
#endif /* LOG */
			message(Arpa_Info, MSGSTR(AL_OFD, "Warning: alias database out of date"));
		}
	}


	/*
	**  If necessary, load the DBM file.
	**	If running without DBM, load the symbol table.
	*/

	if (init)
	{
#ifdef LOG
		if (LogLevel >= 6)
		{
			extern char *username();

			syslog(LOG_NOTICE, 
				MSGSTR(AL_AUTO, "alias database rebuilt by %s"),
				username());
		}
#endif /* LOG */
		readaliases(TRUE);
	}
# else /* DBM */
	readaliases(init);
# endif /* DBM */
}
/*
**  READALIASES -- read and process the alias file.
**
**	This routine implements the part of initaliases that occurs
**	when we are not going to use the DBM stuff.
**
**	Parameters:
**		init -- if set, initialize the DBM stuff.
**
**	Returns:
**		none.
**
**	Side Effects:
**		Reads AliasFile into the symbol table.
**		Optionally, builds the .dir & .pag files.
*/

static
readaliases(init)
	bool init;
{
	register char *p;
	char *rhs;
	bool skipping;
	int naliases, bytes, longest;
	FILE *af;
	void (*oldsigint)();
	ADDRESS al, bl;
	register STAB *s;
	char line[BUFSIZ];
	char longest_lhs[BUFSIZ];

# ifdef YP
	if (AliasFile[0] == YPMARK) {
	    if (tTd(27, 1))
		printf("Can't reinit YP databases: \"%s\"\n", AliasFile);
	    /* reuse old aliases */
	    errno = 0;
	    return;
	}
# endif /* YP */

#ifdef LOCKF
	if ((af = fopen(AliasFile, "r+")) == NULL) /* lockf needs it writable TK */
#else /* LOCKF */
	if ((af = fopen(AliasFile, "r")) == NULL)
#endif /* LOCKF */
	{
		if (tTd(27, 1))
			printf("Can't open %s\n", AliasFile);
		errno = 0;
		NoAlias++;
		return;
	}

# ifdef DBM
	/* see if someone else is rebuilding the alias file already */
#ifdef LOCKF
	if (lockf(fileno(af), F_TLOCK, (off_t)0) < 0 && errno == EACCES )
#else /* LOCKF */
	if (flock(fileno(af), LOCK_EX | LOCK_NB) < 0 && errno == EWOULDBLOCK)
#endif /* LOCKF */
	{
		/* yes, they are -- wait until done and then return */
		message(Arpa_Info, MSGSTR(AL_LOCK, "Alias file is already being rebuilt"));
		if (OpMode != MD_INITALIAS)
		{
			/* wait for other rebuild to complete */
#ifdef LOCKF
			(void) lockf(fileno(af), F_LOCK, (off_t)0);
#else /* LOCKF */
			(void) flock(fileno(af), LOCK_EX);
#endif /* LOCKF */
		}
		(void) fclose(af);
		errno = 0;
		return;
	}
# endif /* DBM */

	/*
	**  If initializing, create the new DBM files.
	*/

	if (init)
	{
		oldsigint = signal(SIGINT, SIG_IGN);
		(void) strcpy(line, AliasFile);
		(void) strcat(line, DB_PAGEXT);
		if (close(creat(line, DBMMODE)) < 0)
		{
			syserr(MSGSTR(AL_CANTMAKE, "cannot make %s"), line);
			(void) signal(SIGINT, oldsigint);
			return;
		}
		(void) strcpy(line, AliasFile);
		(void) strcat(line, DB_DIREXT);
		if (close(creat(line, DBMMODE)) < 0)
		{
			syserr(MSGSTR(AL_CANTMAKE, "cannot make %s"), line);
			(void) signal(SIGINT, oldsigint);
			return;
		}
# ifdef NDBM
		mapinit(DB_ALIAS);
# else /* NDBM */
		dbminit(AliasFile);
# endif /* NDBM */
	}

	/*
	**  Read and interpret lines
	*/

	FileName = AliasFile;
	LineNumber = 0;
	naliases = bytes = longest = 0;
	skipping = FALSE;
	while (fgets(line, sizeof (line), af) != NULL)
	{
		int lhssize, rhssize;

		LineNumber++;
		p = index(line, '\n');
		if (p != NULL)
			*p = '\0';
		switch (line[0])
		{
		  case '#':
		  case '\0':
			skipping = FALSE;
			continue;

		  case ' ':
		  case '\t':
			if (!skipping)
				syserr(MSGSTR(AL_NONC, "Non-continuation line starts with space"));
			skipping = TRUE;
			continue;
		}
		skipping = FALSE;

		/*
		**  Process the LHS
		**	Find the first colon, and parse the address.
		**	It should resolve to a local name -- this will
		**	be checked later (we want to optionally do
		**	parsing of the RHS first to maximize error
		**	detection).
		*/

		for (p = line; *p != '\0' && *p != ':' && *p != '\n'; p++)
			continue;
		if (*p++ != ':')
		{
			syserr(MSGSTR(AL_MC, "missing colon"));
			continue;
		}
		if (parseaddr(line, &al, 1, ':') == NULL)
		{
			syserr(MSGSTR(AL_IN, "illegal alias name"));
			continue;
		}
		loweraddr(&al);

		/*
		**  Process the RHS.
		**	'al' is the internal form of the LHS address.
		**	'p' points to the text of the RHS.
		*/

		rhs = p;
		for (;;)
		{
			register char c;

			if (init && CheckAliases)
			{
				/* do parsing & compression of addresses */
				while (*p != '\0')
				{
					extern char *DelimChar;

					while (isspace(*p) || *p == ',')
						p++;
					if (*p == '\0')
						break;
					if (parseaddr(p, &bl, -1, ',') == NULL)
						usrerr(MSGSTR(AL_BAD, 
							"%s... bad address"),p);
					p = DelimChar;
				}
			}
			else
			{
				p = &p[strlen(p)];
				if (p[-1] == '\n')
					*--p = '\0';
			}

			/* see if there should be a continuation line */
			c = fgetc(af);
			if (!feof(af))
				(void) ungetc(c, af);
			if (c != ' ' && c != '\t')
				break;

			/* read continuation line */
			if (fgets(p, sizeof line - (p - line), af) == NULL)
				break;
			LineNumber++;
		}
		if (al.q_mailer != LocalMailer)
		{
			syserr(MSGSTR(AL_NONL, "cannot alias non-local names"));
			continue;
		}

		/*
		**  Insert alias into symbol table or DBM file
		*/

		lhssize = strlen(al.q_user) + 1;
		rhssize = strlen(rhs) + 1;

# ifdef DBM
		if (init)
		{
			DATUM key, content;

			key.dsize = lhssize;
			key.dptr = al.q_user;
			content.dsize = rhssize;
			content.dptr = rhs;
			if (
# ifdef NDBM
			    dbm_store(AliasDbm, key, content, DBM_REPLACE)
# else /* NDBM */
			    store(key, content)
# endif /* NDBM */
			    < 0)
				syserr("DBM store of %s (size %d) failed", al.q_user, (lhssize+rhssize));
		}
		else
# endif /* DBM */
		{
			s = stab(al.q_user, ST_ALIAS, ST_ENTER);
			s->s_alias = newstr(rhs);
		}

		/* statistics */
		naliases++;
		bytes += lhssize + rhssize;
		if ((rhssize + lhssize) > longest) {
			longest = rhssize + lhssize;
			(void) strcpy(longest_lhs, al.q_user);
		}
	}

# ifdef DBM
	if (init)
	{
		/* add the distinquished alias "@" */
		DATUM key;
# ifdef YP
		DATUM content;
		char Now[MAXHOSTNAMELEN+1];

		/* add the YP stamps.  N.B., don't pad the lengths by 1! */
		gethostname (Now, MAXHOSTNAMELEN);
		key.dptr = "YP_MASTER_NAME";
		key.dsize = strlen(key.dptr);
		content.dsize = strlen(Now);
		content.dptr = Now;
# ifdef NDBM
		(void) dbm_store(AliasDbm, key, content, DBM_INSERT);
# else /* NDBM */
		store(key, content);
# endif /* NDBM */
		(void) sprintf (Now, "%010u", time(0));
		key.dptr = "YP_LAST_MODIFIED";
		key.dsize = strlen(key.dptr);
		content.dsize = strlen(Now);
		content.dptr = Now;
# ifdef NDBM
		(void) dbm_store(AliasDbm, key, content, DBM_INSERT);
# else /* NDBM */
		store(key, content);
# endif /* NDBM */
# endif /* YP */

		key.dsize = 2;
		key.dptr = "@";
# ifdef NDBM
		(void) dbm_store(AliasDbm, key, key, DBM_INSERT);
# else /* NDBM */
		store(key, key);
# endif /* NDBM */

		/* restore the old signal */
		(void) signal(SIGINT, oldsigint);
	}
# endif /* DBM */

	/* closing the alias file drops the lock */
	(void) fclose(af);
	CurEnv->e_to = NULL;
	FileName = NULL;
	message(Arpa_Info, MSGSTR(AL_SUM, "%d aliases, longest (%s) %d bytes, %d bytes total"),
			naliases, longest_lhs, longest, bytes);
# ifdef LOG
	if (LogLevel >= 8)
		syslog(LOG_INFO, MSGSTR(AL_SUM, "%d aliases, longest (%s) %d bytes, %d bytes total"),
			naliases, longest_lhs, longest, bytes);
# endif /* LOG */
}
/*
**  FORWARD -- Try to forward mail
**
**	This is similar but not identical to aliasing.
**
**	Parameters:
**		user -- the name of the user who's mail we would like
**			to forward to.  It must have been verified --
**			i.e., the q_home field must have been filled
**			in.
**		sendq -- a pointer to the head of the send queue to
**			put this user's aliases in.
**
**	Returns:
**		none.
**
**	Side Effects:
**		New names are added to send queues.
*/

forward(user, sendq)
	ADDRESS *user;
	ADDRESS **sendq;
{
	char buf[BUFSIZ];
	int uid, gid, groups[NGROUPS], ngroups;
	int fd, sav_errno, ignore = FALSE;
	struct stat stb;

	if (tTd(27, 1))
		printf("forward(\"%s\")\n", user->q_paddr);

	if (user->q_mailer != LocalMailer || bitset(QBADADDR, user->q_flags))
		return;
	if (user->q_home == NULL)
		syserr(MSGSTR(AL_NOHOME, "forward: no home"));

	/* good address -- look for .forward file in home */
	define('z', user->q_home, CurEnv);
	expand("\001z/.forward", buf, &buf[sizeof buf - 1], CurEnv);

	/*
	** Since sendmail runs suid root, our effective uid here
	** should be root - this means we may not be able to
	** access a user's .forward since by default NFS doesn't
	** allow remote root equivalence.  The only way to be
	** sure of proper access (almost) is to assume the uid,
	** gid and groups of the user...
	*/
	
	/* save current real uid, gid and groups */
	uid = getuid(); gid = getgid();
	ngroups = getgroups(NGROUPS, &groups[0]);
	/* set real uid and gid from effective */
	setruid(geteuid()); setrgid(getegid());
	/* reset groups */
	initgroups(user->q_user, user->q_gid);
	/* set effective uid and gid to that of user->q_user */
	setegid(user->q_gid); seteuid(user->q_uid);

	/* now try opening the file */
	sav_errno = 0;
	fd = open(buf, O_RDONLY, 0);
	if (fd < 0)
		switch (errno)
		{
		    case ENOENT:
		    case EACCES:
		    case ENOTDIR:
		    case ENXIO:
			/* ignorable failures */
			ignore = TRUE;
			break;
		    case ENFILE:
		    case ESTALE:
		    case ETIMEDOUT:
			/* temporary failures */
			errno = 0;
		    default:
			/* permanent failures */
			sav_errno = errno;
		}
	else
	{
		if (fstat(fd, &stb) < 0)
		{
			syserr("forward: Cannot fstat %s!", buf);
			/* don't bother trying to read it */
			ignore = TRUE;
		}
		else
		{
			/* ignore .forward if not a regular file */
			if (stb.st_mode & S_IFMT != S_IFREG ||
			    stb.st_mode & S_IFMT != S_IFIFO)	/* debug */
				ignore = TRUE;
			/* ignore .forward if not owned by user or root */
			if (stb.st_uid != user->q_uid && stb.st_uid != 0)
				ignore = TRUE;
			/* not much point continuing if .forward is empty */
			if (stb.st_size == 0)
				ignore = TRUE;
		}
	}

	/*
	** Having opened the file we must change our uids etc. back
	** to what they were before calling include(), as it can call
	** sendtolist() which calls recipient() which can check on
	** permissions to mail to a file or call forward() again...
	*/

	/* reset effective uid and gid */
	seteuid(getuid()); setegid(getgid());
	/* reset groups */
	setgroups(ngroups, &groups[0]);
	/* reset real uid and gid */
	setrgid(gid); setruid(uid);

	if (ignore)
	{
		(void) close(fd);
		return;
	}

	/*
	** Mark user as being a forwarded address.
	*/
	user->q_flags |= QISFORWD;

	/*
	** Finally read in the .forward - it has only taken about
	** 22 system calls to get around NFS's broken concept of
	** security and open a file whilst effectively being root.
	*/
	errno = sav_errno;
	include(fd, buf, "forwarding", user, sendq);
}
