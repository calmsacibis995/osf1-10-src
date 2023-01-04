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
static char	*sccsid = "@(#)$RCSfile: daemon.c,v $ $Revision: 4.2.3.8 $ (DEC) $Date: 1992/10/14 12:02:56 $";
#endif 
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0
 */

#include "sendmail.h"

#if !defined(lint) && !defined(_NOIDENT)
#ifdef DAEMON

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

#include <errno.h>
/*
#ifndef lint
#ifdef DAEMON

#else

#endif
#endif
*/
int la;	/* load average */

#ifdef DAEMON

# include <netdb.h>
# include <sys/signal.h>
# include <sys/wait.h>
# include <sys/time.h>
# include <sys/resource.h>
# include <sys/file.h>
# include <sys/stat.h>
#ifdef LOCKF
# include <unistd.h>
#endif /* LOCKF */
# ifdef NAMED_BIND
# include <arpa/nameser.h>
# include <resolv.h>
# endif /* NAMED_BIND */

/*
**  DAEMON.C -- routines to use when running as a daemon.
**
**	This entire file is highly dependent on the 4.2 BSD
**	interprocess communication primitives.  No attempt has
**	been made to make this file portable to Version 7,
**	Version 6, MPX files, etc.  If you should try such a
**	thing yourself, I recommend chucking the entire file
**	and starting from scratch.  Basic semantics are:
**
**	getrequests()
**		Opens a port and initiates a connection.
**		Returns in a child.  Must set InChannel and
**		OutChannel appropriately.
**	clrdaemon()
**		Close any open files associated with getting
**		the connection; this is used when running the queue,
**		etc., to avoid having extra file descriptors during
**		the queue run and to avoid confusing the network
**		code (if it cares).
**	makeconnection(host, port, outfile, infile)
**		Make a connection to the named host on the given
**		port.  Set *outfile and *infile to the files
**		appropriate for communication.  Returns zero on
**		success, else an exit status describing the
**		error.
**	maphostname(hbuf, hbufsize)
**		Convert the entry in hbuf into a canonical form.  It
**		may not be larger than hbufsize.
**
**	mapinit(c)
**		Open and initialize a dbm database.  Reopen if the current
**		file descriptor is out of date.
**
**	mapkey(c, key, keysiz, arg)
**		Search a database for a match to the given key, sprintf'ing
**		the argument through the result if found.
*/
/*
**  GETREQUESTS -- open mail IPC port and get requests.
**
**	Parameters:
**		none.
**
**	Returns:
**		none.
**
**	Side Effects:
**		Waits until some interesting activity occurs.  When
**		it does, a child is created to process it, and the
**		parent waits for completion.  Return from this
**		routine is always in the child.  The file pointers
**		"InChannel" and "OutChannel" should be set to point
**		to the communication channel.
*/

struct sockaddr_in	SendmailAddress;/* internet address of sendmail */

int	DaemonSocket	= -1;		/* fd describing socket */
char	*NetName;			/* name of home (local?) network */

getrequests()
{
	int t;
	register struct servent *sp;
	int on = 1;
	enum {accepting, refusing} state = refusing;
	extern reapchild();

	/*
	**  Set up the address for the mailer.
	*/

	sp = getservbyname("smtp", "tcp");
	if (sp == NULL)
	{
		syserr(MSGSTR(DM_UKN, "server \"smtp\" unknown"));
		goto severe;
	}
	SendmailAddress.sin_family = AF_INET;
	SendmailAddress.sin_addr.s_addr = INADDR_ANY;
	SendmailAddress.sin_port = sp->s_port;

	/*
	**  Try to actually open the connection.
	*/

	if (tTd(15, 1))
		printf("getrequests: port 0x%x\n", SendmailAddress.sin_port);

	/* get a socket for the SMTP connection */
	DaemonSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (DaemonSocket < 0)
	{
		/* probably another daemon already */
		syserr(MSGSTR(DM_SOCK, "getrequests: can't create socket"));
	  severe:
# ifdef LOG
		if (LogLevel > 0)
			syslog(LOG_ALERT, MSGSTR(DM_NOCONN, "cannot get connection"));
# endif /* LOG */
		finis();
	}

	/* turn on network debugging? */
	if (tTd(15, 15))
		(void) setsockopt(DaemonSocket, SOL_SOCKET, SO_DEBUG, (char *)&on, sizeof on);

	(void) setsockopt(DaemonSocket, SOL_SOCKET, SO_REUSEADDR, (char *)&on, sizeof on);
	(void) setsockopt(DaemonSocket, SOL_SOCKET, SO_KEEPALIVE, (char *)&on, sizeof on);

	if (bind(DaemonSocket, &SendmailAddress, sizeof SendmailAddress) < 0)
	{
		syserr(MSGSTR(DM_BIND, "getrequests: cannot bind"));
		(void) close(DaemonSocket);
		goto severe;
	}

	(void) signal(SIGCHLD, (void (*)(int))reapchild);

	if (tTd(15, 1))
		printf("getrequests: %d\n", DaemonSocket);

	for (;;)
	{
		register int pid;
		auto int lotherend;
		extern int RefuseLA;

		/* see if we are rejecting connections */
		while ((la = getla()) > RefuseLA)
		{
			/* if we were accepting, don't even bother */
			/* listening until the load average drops  */
			if (state == accepting) {
				listen(DaemonSocket, 0);
				state = refusing;
			}
			setproctitle(MSGSTR(DM_REJCON, "rejecting connections: load average: %.2f"), (double)la);
			if (tTd(15, 15))
				printf("rejecting, loadav=%d\n", la);
			sleep(5);
		}
		if (tTd(15, 15))
			printf("accepting, loadav=%d\n", la);

		/* if we were refusing, make sure we are now listening */
		if (state == refusing) {
			if (listen(DaemonSocket, 10) < 0)
			{
				syserr("getrequests: cannot listen");
				(void) close(DaemonSocket);
				goto severe;
			}
			setproctitle("accepting connections");
			state = accepting;
		}

		/* wait for a connection */
		do
		{
			errno = 0;
			lotherend = sizeof RealHostAddr;
			t = accept(DaemonSocket, &RealHostAddr, &lotherend);
		} while (t < 0 && errno == EINTR);
		if (t < 0)
		{
			syserr("getrequests: accept");
			sleep(5);
			continue;
		}

		/*
		**  Create a subprocess to process the mail.
		*/

		if (tTd(15, 2))
			printf("getrequests: forking (fd = %d)\n", t);

		pid = fork();
		if (pid < 0)
		{
			syserr(MSGSTR(DM_NOFORK, "daemon: cannot fork"));
			sleep(10);
			(void) close(t);
			continue;
		}

		if (pid == 0)
		{
			extern struct hostent *gethostbyaddr();
			register struct hostent *hp;
			char buf[MAXNAME];

			/*
			**  CHILD -- return to caller.
			**	Collect verified idea of sending host.
			**	Verify calling user id if possible here.
			*/

			(void) signal(SIGCHLD, SIG_DFL);

			/* determine host name */
			hp = gethostbyaddr((char *) &RealHostAddr.sin_addr, sizeof RealHostAddr.sin_addr, AF_INET);
			if (hp != NULL)
				(void) strcpy(buf, hp->h_name);
			else
			{
				extern char *inet_ntoa();

				/* produce a dotted quad */
				(void) sprintf(buf, "[%s]",
					inet_ntoa(RealHostAddr.sin_addr));
			}

			/* should we check for illegal connection here? XXX */

			RealHostName = newstr(buf);

			(void) close(DaemonSocket);
			InChannel = fdopen(t, "r");
			OutChannel = fdopen(dup(t), "w");
			if (tTd(15, 2))
				printf("getreq: returning\n");
# ifdef LOG
			if (LogLevel > 11)
				syslog(LOG_DEBUG, "connected, pid=%d", getpid());
# endif /* LOG */
			return;
		}

		/* close the port so that others will hang (for a while) */
		(void) close(t);
	}
	/*NOTREACHED*/
}
/*
**  CLRDAEMON -- reset the daemon connection
**
**	Parameters:
**		none.
**
**	Returns:
**		none.
**
**	Side Effects:
**		releases any resources used by the passive daemon.
*/

clrdaemon()
{
	if (DaemonSocket >= 0)
		(void) close(DaemonSocket);
	DaemonSocket = -1;
}
/*
**  MAKECONNECTION -- make a connection to an SMTP socket on another machine.
**
**	Parameters:
**		host -- the name of the host.
**		port -- the port number to connect to.
**		outfile -- a pointer to a place to put the outfile
**			descriptor.
**		infile -- ditto for infile.
**
**	Returns:
**		An exit code telling whether the connection could be
**			made and if not why not.
**
**	Side Effects:
**		none.
*/

makeconnection(host, port, outfile, infile)
	char *host;
	u_short port;
	FILE **outfile;
	FILE **infile;
{
	register int i = 0;
	register int s;
	register struct hostent *hp = (struct hostent *)NULL;
	extern char *inet_ntoa();
	int sav_errno;
#ifdef NAMED_BIND
	extern int h_errno;

	/*
	**  Don't do recursive domain searches.  An example why not:
	**  Machine cerl.cecer.army.mil has a UUCP connection to
	**  osiris.cso.uiuc.edu.  Also at UIUC is a machine called
	**  uinova.cerl.uiuc.edu that accepts mail for the its parent domain
	**  cerl.uiuc.edu.  Sending mail to cerl!user with recursion on
	**  will select cerl.uiuc.edu which maps to uinova.cerl.uiuc.edu.
	**  We leave RES_DEFNAMES on so single names in the current domain
	**  still work.
	**
	**  Paul Pomes, CSO, UIUC       17-Oct-88
	*/

	_res.options &= (~RES_DNSRCH & 0xffff);

	h_errno = 0;
#endif
	/*
	**  Set up the address for the mailer.
	**	Accept "[a.b.c.d]" syntax for host name.
	*/
	errno = 0;

	if (host[0] == '[')
	{
		int hid = -1;
		register char *p = index(host, ']');

		if (p != NULL)
		{
			*p = '\0';
			hid = inet_addr(&host[1]);
			*p = ']';
		}
		if (p == NULL || hid == -1)
		{
			usrerr(MSGSTR(DM_INV, "Invalid numeric domain spec \"%s\""), host);
			return (EX_NOHOST);
		}
		SendmailAddress.sin_addr.s_addr = hid;
	}
	else
	{
		hp = gethostbyname(host);
		if (hp == NULL)
		{
# ifdef NAMED_BIND
			/* if we timed out, our resolver
			library suggest to try again,
			assume temporary fail */

			if (errno == ETIMEDOUT || h_errno == TRY_AGAIN)
				return (EX_TEMPFAIL);

			/* connection refused, but name server
				    specified */
			if (UseNameServer && errno == ECONNREFUSED)
			        return (EX_TEMPFAIL);
# endif /* NAMED_BIND */
			return (EX_NOHOST);
		}

		bcopy(hp->h_addr, (char *) &SendmailAddress.sin_addr,
		      hp->h_length);
		i = 1;
	}

	/*
	**  Determine the port number.
	*/

	if (port != 0)
		SendmailAddress.sin_port = htons(port);
	else
	{
		register struct servent *sp = getservbyname("smtp", "tcp");

		if (sp == NULL)
		{
			syserr(MSGSTR(DM_SRVUKN, "makeconnection: server \"smtp\" unknown"));
			return (EX_OSFILE);
		}
		SendmailAddress.sin_port = sp->s_port;
	}

	/*
	**  Try to actually open the connection.
	*/

again:
	if (tTd(16, 1))
		printf("makeconnection (%s [%s])\n", host,
		    inet_ntoa(SendmailAddress.sin_addr));

	s = socket(AF_INET, SOCK_STREAM, 0);
	if (s < 0)
	{
		syserr("makeconnection: no socket");
		sav_errno = errno;
		goto failure;
	}

	if (tTd(16, 1))
		printf("makeconnection: %d\n", s);

	/* turn on network debugging? */
	if (tTd(16, 14))
	{
		int on = 1;
		(void) setsockopt(DaemonSocket, SOL_SOCKET, SO_DEBUG, (char *)&on, sizeof on);
	}
	if (CurEnv->e_xfp != NULL)
		(void) fflush(CurEnv->e_xfp);		/* for debugging */
	errno = 0;					/* for debugging */
	SendmailAddress.sin_family = AF_INET;
	if (connect(s, &SendmailAddress, sizeof SendmailAddress) < 0)
	{
		sav_errno = errno;
		(void) close(s);
		if (hp && hp->h_addr_list[i])
		{
			bcopy(hp->h_addr_list[i++],
			    (char *)&SendmailAddress.sin_addr, hp->h_length);
			goto again;
		}

		/* failure, decide if temporary or not */
	failure:
		errno = sav_errno;
		switch (errno)
		{
		  case EISCONN:
		  case ETIMEDOUT:
		  case EINPROGRESS:
		  case EALREADY:
		  case EADDRINUSE:
		  case EHOSTDOWN:
		  case ENETDOWN:
		  case ENETRESET:
		  case ENOBUFS:
		  case ECONNREFUSED:
		  case ECONNRESET:
		  case EHOSTUNREACH:
		  case ENETUNREACH:
			/* there are others, I'm sure..... */
			return (EX_TEMPFAIL);

		  case EPERM:
			/* why is this happening? */
			syserr("makeconnection: funny failure, addr=%lx, port=%x",
				SendmailAddress.sin_addr.s_addr, SendmailAddress.sin_port);
			return (EX_TEMPFAIL);

		  default:
			{
				extern char *errstring();

				message(Arpa_Info, "%s", errstring(sav_errno));
				return (EX_UNAVAILABLE);
			}
		}
	}

	/* connection ok, put it into canonical form */
	*outfile = fdopen(s, "w");
	*infile = fdopen(s, "r");

	return (EX_OK);
}
/*
**  MYHOSTNAME -- return the name of this host.
**
**	Parameters:
**		hostbuf -- a place to return the name of this host.
**		size -- the size of hostbuf.
**
**	Returns:
**		A list of aliases for this host.
**
**	Side Effects:
**		none.
*/

char **
myhostname(hostbuf, size)
	char hostbuf[];
	int size;
{
	extern struct hostent *gethostbyname();
	struct hostent *hp = (struct hostent *) NULL;

	if (gethostname(hostbuf, size) < 0)
		(void) strcpy(hostbuf, "localhost");
	
	hp = gethostbyname(hostbuf);
	
	if (hp != NULL)
	{
		(void) strcpy(hostbuf, hp->h_name);
		return (hp->h_aliases);
	}
	else
		return (NULL);
}

/*
**  MAPHOSTNAME -- turn a hostname into canonical form
**
**	Parameters:
**		hbuf -- a buffer containing a hostname.
**		hbsize -- the size of hbuf.
**
**	Returns:
**		An exit code telling if the hostname was found and
**		canonicalized.
**
**	Side Effects:
**		Looks up the host specified in hbuf.  If it is not
**		the canonical name for that host, replace it with
**		the canonical name.  If the name is unknown, or it
**		is already the canonical name, leave it unchanged.
*/
maphostname(hbuf, hbsize)
	char *hbuf;
	int hbsize;
{
	register struct hostent *hp = (struct hostent *) NULL;
	static char tmphbuf[MAXNAME];
	struct hostent *gethostbyaddr();
	struct hostent *gethostbyname();

	/*
	 * If first character is a bracket, then it is an address
	 * lookup.  Address is copied into a temporary buffer to
	 * strip the brackets and to preserve hbuf if address is
	 * unknown.
	 */
	if (*hbuf == '[') 
	{
		u_int in_addr;

		(void) strncpy(tmphbuf, hbuf + 1, strlen(hbuf) - 2);
		in_addr = inet_addr(tmphbuf);

		hp = gethostbyaddr((char *) &in_addr,
				sizeof(struct in_addr), AF_INET);
	} else {
		register int ret;

# ifdef NAMED_BIND
		/*
		** See note in makeconnection() above for why we disable
		** recursive domain matching.  -pbp
		*/
		_res.options &= (~RES_DNSRCH & 0xffff);

		/*
		** But make sure default domain qualification is enabled -
		** it may have been disabled in deliver.c.  -nr
		*/
		_res.options |= RES_DEFNAMES;
# endif /* NAMED_BIND */
		hp = gethostbyname(hbuf);
		if (hp == NULL) {
			/* try lowercase version */
			(void) strcpy(tmphbuf, hbuf);
			(void) makelower(tmphbuf);
			/* Could be just an MX record; look for anything */
			ret = getcanonname(tmphbuf,sizeof(tmphbuf));
			if (ret != TRUE) {
				if (tTd(9, 1))
					printf("maphostname(%s, %d) => %.*s\n",
					       hbuf, hbsize, hbsize-1,
					       hp ? hp->h_name : "NOT_FOUND");
				return FALSE;
			}
			strcpy(hbuf,tmphbuf);
			return TRUE;
		}
	}

	if (tTd(9, 1))
		printf("maphostname(\"%s\", %d) => %.*s\n",
		       hbuf, hbsize, hbsize - 1, hp ? hp->h_name : "NOT_FOUND");

	if (hp == NULL)
		return FALSE;

	if (strlen(hp->h_name) >= hbsize)
		hp->h_name[hbsize - 1] = '\0';
	(void) strcpy(hbuf, hp->h_name);
	return TRUE;
}

# else /* DAEMON */
/* code for systems without sophisticated networking */

/*
**  MYHOSTNAME -- stub version for case of no daemon code.
**
**	Can't convert to upper case here because might be a UUCP name.
**
**	Mark, you can change this to be anything you want......
*/

char **
myhostname(hostbuf, size)
	char hostbuf[];
	int size;
{
	register FILE *f;

	hostbuf[0] = '\0';
	f = fopen("/usr/include/whoami", "r");
	if (f != NULL)
	{
		(void) fgets(hostbuf, size, f);
		fixcrlf(hostbuf, TRUE);
		(void) fclose(f);
	}
	return (NULL);
}
/*
**  MAPHOSTNAME -- turn a hostname into canonical form
**
**	Parameters:
**		hbuf -- a buffer containing a hostname.
**		hbsize -- the size of hbuf.
**
**	Returns:
**		An exit code telling if the hostname was found and
**		canonicalized.
**
**	Side Effects:
**		Looks up the host specified in hbuf.  If it is not
**		the canonical name for that host, replace it with
**		the canonical name.  If the name is unknown, or it
**		is already the canonical name, leave it unchanged.
*/

/*ARGSUSED*/
maphostname(hbuf, hbsize)
	char *hbuf;
	int hbsize;
{
	return FALSE;
}

#endif /* DAEMON */
/*
**  MAPINIT -- Open and (re)initialize a dbm database
**
**	Parameters:
**		c -- the (one character) name of the database
**
**	Returns:
**		An exit code telling if we could open the database.
**
*/
#ifdef NDBM
bool
mapinit(c)
	char c;
{
	struct stat stb;
	struct dbm_table *db;
#ifdef LOCKF
	struct flock flck;
#endif /* LOCKF */
	char buf[MAXNAME];

	if (c < 32)
		db = &DbmTab[c];
	else
		db = &DbmTab[c - 32 + NSPECIALDB];

	/*
	 * Some of the first 32 databases are used for our own
	 * purposes.  Handle their initialisation seperately.
	 */
	if (c < 32)
		switch (c)
		{
	/* Hesiod and YP support seems incomplete so far (OSF1); so
	 * inactive by default.
	 */
#ifdef HESIOD
		    case HS_ALIAS:
		    case HS_PASSWD:
			/* no initialisation - fake database entries */
			return TRUE;
#endif /* HESIOD */
# ifdef YP
		    case YP_PASSWD:
			/* no initialisation - fake database entries */
			return TRUE;
		    case YP_ALIAS:
			/* fake it so it uses YP code in mapkey() */
			if (db->db_name == NULL)
				db->db_name = "%mail.aliases";
			return TRUE;
# endif /* YP */
		    default:
			/* don't know about this one */
			syserr("database '\\%#o' has not been defined", c);
			return FALSE;
		}

	if (db->db_name == NULL) {
		syserr("database '%c' has not been defined", c);
		return FALSE;
	}

#ifdef YP
	/*
	 * Yellow pages are always supposed to be ready
	 */
	if (db->db_name[0] == YPMARK)
		return TRUE;
#endif /* YP */

	/*
	 * Have we already (unsuccessfully) tried to open it?
	 */
	if (db->db_dbm == DB_NOSUCHFILE) {
		if (tTd(60, 1))
			printf("mapinit(%c) => NO_FILE\n", c);
		return FALSE;
	}

	/*
	 * If it already is open, check if it has been changed.
	 */
	(void) sprintf(buf, "%s%s", db->db_name, DB_DIREXT);
	if (db->db_dbm != DB_NOTYETOPEN) {
		if (stat(buf, &stb) < 0) {
			/* try once more */
			sleep(30);
			if (stat(buf, &stb) < 0) {
				syserr("somebody removed %s for db '%c'", buf, c);
				db->db_dbm = DB_NOSUCHFILE;
				if (tTd(60, 1))
					printf("mapinit(%c) => FILE_REMOVED\n", c);
				return FALSE;
			}
		}
		if (db->db_mtime != stb.st_mtime) {
			if (tTd(60, 1))
				printf("database '%c' [%s] has changed; reopening it\n",
				   c, db->db_name);
			(void) dbm_close(db->db_dbm);
			db->db_dbm = DB_NOTYETOPEN;
		}
	}

	/*
	 * Initialize database if not already open (r/w for aliases)
	 */
	if (db->db_dbm == DB_NOTYETOPEN) {
		db->db_dbm = dbm_open(db->db_name, c == DB_ALIAS ? O_RDWR : O_RDONLY, 0);
		if (db->db_dbm == DB_NOSUCHFILE) {
			/* try once more */
			sleep(30);
			db->db_dbm = dbm_open(db->db_name, c == DB_ALIAS ? O_RDWR : O_RDONLY, 0);
		}
		if (db->db_dbm == DB_NOSUCHFILE) {
			syserr("can't open database '%c' [%s]", c, db->db_name);
			if (tTd(60, 1))
				printf("mapinit(%c) => CAN'T OPEN %s\n", c, db->db_name);
			return FALSE;
		}
		if (stat(buf, &stb)) {
			/* try once more */
			sleep(30);
			if (stat(buf, &stb) < 0) {
				syserr("can't stat %s", buf);
				if (tTd(60, 1))
					printf("mapinit(%c) => FILE_REMOVED\n", c);
				return FALSE;
			}
		}
		db->db_mtime = stb.st_mtime;

		/*
		 * Make sure the database isn't being updated
		 */
#ifdef LOCKF
		flck.l_type = (c == DB_ALIAS ? F_WRLCK : F_RDLCK);
		flck.l_whence = SEEK_SET;
		flck.l_start = (off_t)0;
		flck.l_len = (off_t)0;
		if (fcntl(dbm_dirfno(db->db_dbm), F_SETLK, &flck) < 0)
			if (errno == EACCES) {
#else /* LOCKF */
		if (flock(dbm_dirfno(db->db_dbm), LOCK_EX | LOCK_NB) < 0)
			if (errno == EWOULDBLOCK) {
#endif /* LOCKF */
				if (tTd(60, 1))
					printf("%s%s is locked, waiting...\n",
						   db->db_name, DB_DIREXT);
#ifdef LOCKF
				(void) fcntl(dbm_dirfno(db->db_dbm), F_SETLKW, &flck);
			} else
				syserr("lockf failed for db %c [%s], fd %d",
				       c, db->db_name, dbm_dirfno(db->db_dbm));
			flck.l_type = F_UNLCK;
			(void) fcntl(dbm_dirfno(db->db_dbm), F_SETLKW, &flck);
#else /* LOCKF */
				(void) flock(dbm_dirfno(db->db_dbm), LOCK_EX);
			} else
				syserr("flock failed for db %c [%s], fd %d",
				       c, db->db_name, dbm_dirfno(db->db_dbm));
			(void) flock(dbm_dirfno(db->db_dbm), LOCK_UN);
#endif /* LOCKF */
	}
	return TRUE;
}
/*
**  MAPKEY -- Search a dbm database.
**
**	Search the named database using the given key.  If
**	a result is found, sprintf the argument through the
**	result back into the key and return TRUE;
**	otherwise return FALSE and do nothing.
**
**	Keysize may also be given as zero, in which case the
**	sprintf'ed result is returned if the key matched.
**
**	Parameters:
**		c -- the database
**		key -- search string
**		keysiz -- size of key
**		arg -- sprintf argument & result
**
**	Returns:
**		An exit code telling if there was a match.
**
**	Side Effects:
**		The argval is rewritten to reflect what was found
**		in the database.
*/

void *mapkey(c, key, keysiz, arg)
	char c, *key, *arg;
	int keysiz;
{
	struct dbm_table *db;
	DATUM dkey, result;
	static char lowkey[MAXNAME];

	if (c < 32)
		db = &DbmTab[c];
	else
		db = &DbmTab[c - 32 + NSPECIALDB];

	if (tTd(60, 1))
		printf(c < 32 ? "mapkey('\\%#o', \"%s\", \"%s\") => "
					: "mapkey('%c', \"%s\", \"%s\") => ",
			c, key, arg ? arg : "--");

	/*
	 * Init the database; return if failure
	 */
	if (!mapinit(c))
		return NULL;

	/*
	 * Normalize key (ie turn it to lowercase)
	 */
	(void) strcpy(lowkey, key);
	(void) makelower(lowkey);

	/*
	 * Handle (most) of the first 32 databases separately
	 */
#ifdef YP
	if (c < 32 && c != YP_ALIAS)
#else
	if (c < 32)
#endif /* YP */
		switch (c)
		{
			char **happ;
			struct passwd *pp;

#ifdef HESIOD
		    case HS_ALIAS:
			happ = hes_resolve(key, "aliases");
			if (happ == NULL)
				result.dptr = NULL;
			else
				result.dptr = *happ;
			break;
		    case HS_PASSWD:
			happ = hes_resolve(key, "passwd"); /* Exists? */
			if (happ == NULL){
			        if (tTd(60,1))
					printf("NOT_FOUND\n");
				return NULL;
			}else{
				if (tTd(60, 1))
					printf("FOUND\n");
                                return TRUE;
			}
#endif /* HESIOD */

#ifdef YP
		    case YP_PASSWD:
			if (YPdomain == NULL)           /* Exists? */
				return NULL;

			if (yp_match(YPdomain, "passwd", key, strlen(key)+1,
				     &result.dptr, &result.dsize) != 0){
				if (tTd(60,1))
					printf("NOT_FOUND\n");
				return NULL;
			}else{
				if (tTd(60, 1))
					printf("FOUND\n");
			}
				return (void *)TRUE;
#endif /* YP */
		}
	else {
#ifdef YP
		/*
		 * Test for yellow page database first
		 */
		if (db->db_name[0] == YPMARK) {
			if (YPdomain == NULL)		/* is YP not running? */
				return NULL;

			/*
			 * We include the null after the string, but Sun doesn't
			 */
			if (yp_match(YPdomain, &db->db_name[1],
				     lowkey, strlen(key),
				     &result.dptr, &result.dsize) != 0 &&
			    yp_match(YPdomain, &db->db_name[1],
				     lowkey, strlen(key) + 1,
				     &result.dptr, &result.dsize) != 0)
				result.dptr = NULL;
			else
				/* smash newline */
				result.dptr[result.dsize] = '\0';
		} else {
#endif /* YP */
			/*
			 * Go look for matching dbm entry
			 */
			dkey.dptr = lowkey;
			dkey.dsize = strlen(dkey.dptr) + 1;
			result = dbm_fetch(db->db_dbm, dkey);
#ifdef YP
		}
#endif /* YP */
	}

	/*
	 * Well, were we successful?
	 */
	if (result.dptr == NULL) {
		if (tTd(60, 1))
			printf("NOT_FOUND\n");
		return NULL;
	}

	/*
	 * Yes, rewrite result if sprintf arg was given.
	 */
	if (arg == NULL)
		(void) strcpy(lowkey, result.dptr);
	else
		(void) sprintf(lowkey, result.dptr, arg);
	/* if keysiz is zero, we should return a string from the heap */
	if (keysiz == 0)
		key = newstr(lowkey);
	else {
		if (strlen(lowkey)+1 > keysiz) {
			syserr("mapkey: result \"%s\" too long after expansion\n",
			   lowkey, keysiz);
			lowkey[keysiz-1] = '\0';
		}
		(void) strcpy(key, lowkey);
	}
	if (tTd(60, 1))
		printf("%s\n", key);

	return key;
}

#else /* NDBM */

/* should really read the table into the stab instead */
mapkey(db, key, keysiz, arg)
	char db, *key, *arg;
	int keysiz;
{
	return FALSE;
}

#endif /* NDBM */
