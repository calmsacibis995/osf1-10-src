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
static char	*sccsid = "@(#)$RCSfile: rcmd.c,v $ $Revision: 4.3.10.6 $ (DEC) $Date: 1993/11/23 22:32:09 $";
#endif 
/*
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 */
/*
 * OSF/1 1.2
 */
#if !defined(lint) && !defined(_NOIDENT)
#endif
/*
 * COMPONENT_NAME: LIBCNET rcmd.c
 *
 * FUNCTIONS: rcmd, rresvport, ruserok, _validuser, _checkhost, _special_case
 *
 *
 * Copyright (c) 1983 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that: (1) source distributions retain this entire copyright
 * notice and comment, and (2) distributions including binaries display
 * the following acknowledgement:  ``This product includes software
 * developed by the University of California, Berkeley and its contributors''
 * in the documentation or other materials provided with the distribution
 * and in all advertising materials mentioning features or use of this
 * software. Neither the name of the University nor the names of its
 * contributors may be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
rcmd.c	5.22 (Berkeley) 6/1/90";
 *
 * 1.13  com/lib/c/net/rcmd.c, libcnet, bos320, 9125320 6/6/91 13:56:33
 */


/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#pragma weak rresvport = __rresvport
#pragma weak ruserok = __ruserok
#if defined(_THREAD_SAFE)
#pragma weak rcmd_r = __rcmd_r
#endif
#if !defined(_THREAD_SAFE)
#pragma weak rcmd = __rcmd
#endif
#endif
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <pwd.h>
#include <sys/param.h>
#include <sys/file.h>
#include <sys/signal.h>
#include <sys/socket.h>
#include <sys/stat.h>

#include <netinet/in.h>

#include <netdb.h>
#include <errno.h>
#include <nl_types.h>

#include "libc_msg.h"
#include "ts_supp.h"

#ifdef _THREAD_SAFE
#include "rec_mutex.h"

extern struct rec_mutex	_domain_rmutex;
#endif	/* _THREAD_SAFE */

static nl_catd	catd;

char		*index();
static int	_checkhost();


#ifdef _THREAD_SAFE
rcmd_r(char **ahost, u_short rport, char *locuser, char *remuser,
     char *cmd, int *fd2p, struct hostent_data *host_data)
#else
rcmd(char **ahost, u_short rport, char *locuser, char *remuser,
     char *cmd, int *fd2p)
#endif	/* _THREAD_SAFE */
{
	int		s, timo = 1, pid;
	long		oldmask;
	char		c;
	int		lport = IPPORT_RESERVED - 1;
	struct hostent	*hp;
	fd_set		reads;
	struct sockaddr_in	sin, from;

#ifdef	_THREAD_SAFE
	struct hostent host;
	hp = &host;
#endif	/* _THREAD_SAFE */

	pid = getpid();

#ifdef	_THREAD_SAFE
	if (gethostbyname_r(*ahost, &host, host_data) < 0)
#else
	hp = gethostbyname(*ahost);
	if (hp == 0)
#endif	/* _THREAD_SAFE */
	{
		herror(*ahost);
		return (-1);
	}
	*ahost = hp->h_name;

#ifndef _THREAD_SAFE
	/*
	 * For thread safe case we do _not_ mess with the signal
	 * settings - note default SIGURG is to ignore it.
	 */
	oldmask = sigblock(sigmask(SIGURG));
#endif	/* _THREAD_SAFE */

	for (;;) {
		s = rresvport(&lport);
		if (s < 0) {
			if (_Geterrno() == EAGAIN) {
				catd = catopen(MF_LIBC,NL_CAT_LOCALE);
				fprintf(stderr, catgets(catd, LIBCNET, NET23,
						"socket: All ports in use\n"));
				(void)catclose(catd);
			}
			else
				perror("rcmd: socket");
#ifndef _THREAD_SAFE
			sigsetmask(oldmask);
#endif	/* _THREAD_SAFE */
			return (-1);
		}
		fcntl(s, F_SETOWN, pid);
		sin.sin_family = hp->h_addrtype;
		bcopy(hp->h_addr_list[0], (caddr_t)&sin.sin_addr, hp->h_length);
		sin.sin_port = rport;
		if (connect(s, (struct sockaddr *)&sin, sizeof (sin)) >= 0)
			break;
		(void) close(s);
		if (_Geterrno() == EADDRINUSE) {
			lport--;
			continue;
		}
		if (_Geterrno() == ECONNREFUSED && timo <= 16) {
			sleep(timo);
			timo *= 2;
			continue;
		}
		if (hp->h_addr_list[1] != NULL) {
			int oerrno = _Geterrno();

			catd = catopen(MF_LIBC,NL_CAT_LOCALE);
			fprintf(stderr, catgets(catd, LIBCNET, NET24,
					"connect to address %s: "),
					inet_ntoa(sin.sin_addr));
			_Seterrno(oerrno);
			perror(0);
			hp->h_addr_list++;
			bcopy(hp->h_addr_list[0], (caddr_t)&sin.sin_addr,
			      hp->h_length);
			fprintf(stderr, catgets(catd,LIBCNET,NET25,
					"Trying %s...\n"),
					inet_ntoa(sin.sin_addr));
			(void)catclose(catd);
			continue;
		}
		perror(hp->h_name);
#ifndef _THREAD_SAFE
		sigsetmask(oldmask);
#endif	/* _THREAD_SAFE */
		return (-1);
	}
	lport--;
	if (fd2p == 0) {
		write(s, "", 1);
		lport = 0;
	} else {
		char num[8];
		int s2 = rresvport(&lport), s3;
		int len = sizeof (from);

		if (s2 < 0)
			goto bad;
		listen(s2, 1);
		(void) sprintf(num, "%d", lport);
		if (write(s, num, strlen(num)+1) != strlen(num)+1) {
			catd = catopen(MF_LIBC,NL_CAT_LOCALE);
			perror(catgets(catd, LIBCNET, NET26,
				"write: setting up stderr"));
			(void)catclose(catd);
			(void) close(s2);
			goto bad;
		}
		FD_ZERO(&reads);
		FD_SET(s, &reads);
		FD_SET(s2, &reads);
		_Seterrno(0);
		if (select(32, &reads, (fd_set *)0, (fd_set *)0, (struct timeval *)0) < 1 ||
		    !FD_ISSET(s2, &reads)) {
			if (_Geterrno() != 0)
				perror(catgets(catd,LIBCNET,NET26,
						"write: setting up stderr"));
			else
				fprintf(stderr, catgets(catd,LIBCNET,NET27,
"socket: protocol failure in circuit setup.\n"));
			(void) close(s2);
			goto bad;
		}
		s3 = accept(s2, (struct sockaddr *)&from, &len);
		(void) close(s2);
		if (s3 < 0) {
			perror("accept");
			lport = 0;
			goto bad;
		}
		*fd2p = s3;
		from.sin_port = ntohs((u_short)from.sin_port);
		if (from.sin_family != AF_INET
		    || from.sin_port >= IPPORT_RESERVED
		    || from.sin_port < IPPORT_RESERVED / 2) {
			catd = catopen(MF_LIBC,NL_CAT_LOCALE);
			fprintf(stderr, catgets(catd, LIBCNET, NET27,
"socket: protocol failure in circuit setup.\n"));
			(void)catclose(catd);
			goto bad2;
		}
	}
	(void) write(s, locuser, strlen(locuser)+1);
	(void) write(s, remuser, strlen(remuser)+1);
	(void) write(s, cmd, strlen(cmd)+1);
	if (read(s, &c, 1) != 1) {
		perror(*ahost);
		goto bad2;
	}
	if (c != 0) {
 		/*
 		 *   Two different protocols seem to be used;
 		 *   one prepends a "message" byte with a "small"
 		 *   number; the other one just sends the message
 		 */
 		if (isalnum(c))
 			(void) write(2, &c, 1);

		while (read(s, &c, 1) == 1) {
			(void) write(2, &c, 1);
			if (c == '\n')
				break;
		}
		goto bad2;
	}
#ifndef _THREAD_SAFE
	sigsetmask(oldmask);
#endif	/* _THREAD_SAFE */
	return (s);
bad2:
	if (lport)
		(void) close(*fd2p);
bad:
	(void) close(s);
#ifndef _THREAD_SAFE
	sigsetmask(oldmask);
#endif	/* _THREAD_SAFE */
	return (-1);
}


int
rresvport(int *alport)
{
	struct sockaddr_in	sin;
	int			s;

	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = INADDR_ANY;
	s = socket(AF_INET, SOCK_STREAM, 0);
	if (s < 0)
		return (-1);
	for (;;) {
		sin.sin_port = htons((u_short)*alport);
		if (bind(s, (struct sockaddr *)&sin, sizeof (sin)) >= 0)
			return (s);
		if (_Geterrno() != EADDRINUSE) {
			(void) close(s);
			return (-1);
		}
		(*alport)--;
		if (*alport == IPPORT_RESERVED/2) {
			(void) close(s);
			_Seterrno(EAGAIN);		/* close */
			return (-1);
		}
	}
}


/* WARNING: this is not thread safe
 *
 * It cannot be removed as both rlogind and rshd commands use it.
 * Yuk.
 */
int	_check_rhosts_file = 1;
#define DISALLOW_PLUS 	2

int
ruserok(char *rhost, int superuser, char *ruser, char *luser)
{
	register char	*sp, *p;
	FILE		*hostf;
	char		fhost[MAXHOSTNAMELEN];
	int		first = 1;
	int		baselen = -1;
	int		ret;
	char domain[256];
	int suid;
	int sgid;
	int group_list_size= -1;
	gid_t groups[NGROUPS_MAX];

	if (getdomainname(domain, sizeof(domain)) < 0) {
		fprintf(stderr, 
			catgets(catd,LIBCNET,NET28,
			"rcmd: getdomainname system call missing\n"));
		return(-1);
	}
	sp = rhost;
	p = fhost;
	while (*sp) {
		if (*sp == '.') {
			if (baselen == -1)
				baselen = sp - rhost;
			*p++ = *sp++;
		} else {
			*p++ = isupper(*sp) ? tolower(*sp++) : *sp++;
		}
	}
	*p = '\0';
	hostf = superuser ? (FILE *)0 : fopen(_PATH_HEQUIV, "r");
again:
	if (hostf) {
		ret = _validuser(hostf, fhost, luser, ruser, baselen, domain);
		if (ret) {
			(void) fclose(hostf);
			if (first == 0) {
			        (void) seteuid(suid);
				(void) setegid(sgid);
				if(group_list_size >= 0)
					(void) setgroups(group_list_size, groups);
			}
			if (ret == 1)
				return(0);
			else
				return(-1);
		}
		(void) fclose(hostf);
	}
	if (first == 1 && (_check_rhosts_file || superuser)) {
		struct stat sbuf;
		struct passwd *pwd;
		char pbuf[MAXPATHLEN];
#ifdef	_THREAD_SAFE
#define	PWD_LEN	1024
		char pwd_entry[PWD_LEN];
		struct passwd paswd;
		pwd = &paswd;

		if (getpwnam_r(luser, pwd, pwd_entry, PWD_LEN) < 0)
#else
		if ((pwd = getpwnam(luser)) == NULL)
#endif	/* _THREAD_SAFE */
			return(-1);
		first = 0;
		suid = geteuid();
		sgid = getegid();
		group_list_size = getgroups(NGROUPS_MAX, groups);
		if(setegid(pwd->pw_gid) >= 0)
			(void) initgroups(luser, pwd->pw_gid);
		(void) seteuid(pwd->pw_uid);
		(void)strcpy(pbuf, pwd->pw_dir);
		(void)strcat(pbuf, "/.rhosts");
		if ((hostf = fopen(pbuf, "r")) == NULL)
			goto bad;
		/*
		 * if owned by someone other than user or root or if
		 * writeable by anyone but the owner, quit
		 */
		if (fstat(fileno(hostf), &sbuf)
		    || sbuf.st_uid && sbuf.st_uid != pwd->pw_uid
		    || sbuf.st_mode&022) {
			fclose(hostf);
			goto bad;
		}
		goto again;
	}
bad:
	if (first == 0) {
	        (void) seteuid(suid);
		(void) setegid(sgid);
		if(group_list_size >= 0)
			(void) setgroups(group_list_size, groups);
	}
	return (-1);
}

/*    _validuser:
 *		  returns 1 if connection allowed
 *		 	 -1 if disallowed
 *			  0 if connection not allowed
 */

int
_validuser(FILE *hostf, char *rhost, char *luser, char *ruser, int baselen, 
	   char *domain)
{
	register char	*p;
	char		*user;
	char		ahost[MAX_CANON];
	int		hostmatch, usermatch;

	while (fgets(ahost, sizeof (ahost), hostf)) {
		p = ahost;
		/*
		 * skip comment lines
		 */
		if (*p == '#')
			continue;
		/*
                 * check if to see match any 
		 * has been disabled
                 */
		if (!strncmp(p,"NO_PLUS",7)) {
			_check_rhosts_file = DISALLOW_PLUS; 
			continue;
		}
		while (*p != '\n' && *p != ' ' && *p != '\t' && *p != '\0') {
			*p = isupper(*p) ? tolower(*p) : *p;
			p++;
		}
		if (*p == ' ' || *p == '\t') {
			*p++ = '\0';
			while (*p == ' ' || *p == '\t')
				p++;
			user = p;
			while (*p != '\n' && *p != ' ' && *p != '\t' && *p != '\0')
				p++;
		} else
			user = p;
		*p = '\0';
		if (!(hostmatch = _special_case(rhost, ahost, NULL, domain))) {
			hostmatch = _checkhost(rhost, ahost, baselen);
		}
		if (*user) {
			if (!(usermatch = _special_case(NULL, user, ruser, domain))) 
				usermatch =  !strcmp(ruser, user);
		}
		else
			usermatch = !strcmp(ruser,luser);
		if (hostmatch && usermatch) {
			if (hostmatch == -1 || usermatch == -1)
				return(-1);
			return (1);
		}
	}
	return (0);
}


static int
_checkhost(char *rhost, char *lhost, int len)
{
	register char	*cp;
	/*
	 * These data are initialised once.
	 */
	static char	ldomain[MAXHOSTNAMELEN + 1];
	static char	*domainp = NULL;
	static int	nodomain = 0;
	int		gethostname();

	if (len == -1)
		return(!strcasecmp(rhost, lhost));
	if (strncasecmp(rhost, lhost, len))
		return(0);
	if (!strcasecmp(rhost, lhost))
		return(1);
	if (*(lhost + len) != '\0')
		return(0);

	/* Wait in case someone is initialising.
	 */
	TS_LOCK(&_domain_rmutex);
	if (nodomain) {
		TS_UNLOCK(&_domain_rmutex);
		return(0);
	}
	if (!domainp) {
		if (gethostname(ldomain, sizeof(ldomain)) == -1) {
			nodomain = 1;
			TS_UNLOCK(&_domain_rmutex);
			return(0);
		}
		ldomain[MAXHOSTNAMELEN] = '\0';
		if ((domainp = index(ldomain, '.')) == (char *)NULL) {
			nodomain = 1;
			TS_UNLOCK(&_domain_rmutex);
			return(0);
		}
		for (cp = ++domainp; *cp; ++cp)
			if (isupper(*cp))
				*cp = tolower(*cp);
	}

	TS_UNLOCK(&_domain_rmutex);	/* initialisation complete */
	return(!strcasecmp(domainp, rhost + len +1));
}

/*
 * _special_case - parses special cases
 *		  returns 1 if connection allowed
 *		 	 -1 if disallowed
 *			  0 No Match
 */
static int
_special_case(rhost, name, ruser, domain)
	char *rhost, *name, *ruser, *domain;
{
	int match = 0;
	int length;

	/*
  	 * check for match any
	 * host (if plus processing is allowed)
	 */
	if (_check_rhosts_file != DISALLOW_PLUS) 
		if (name[0] == '+' && name[1] == 0)
			return(1);

	/*
	 * Check for two yp netgroup cases
	 */
	if (name[0] == '+' && name[1] == '@' && domain != NULL) 
		match = innetgr(name + 2, rhost, ruser, domain);
	else if (name[0] == '-' && name[1] == '@' && domain != NULL) {
		if (innetgr(name + 2, rhost, ruser, domain))
			match = -1;
	}
	/*
	 * Check to disallow this host/user
	 */
 	else if (name[0] == '-') {
		if (rhost != NULL) {
 			if (!strcmp(rhost, name+1))
 				match = -1;
		 } else {
			if (!strcmp(ruser,name+1))
				match = -1;
		}
 	}
	return(match);
}
