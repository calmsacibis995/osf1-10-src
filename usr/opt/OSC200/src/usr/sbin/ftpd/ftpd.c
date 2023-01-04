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
static char	*sccsid = "@(#)$RCSfile: ftpd.c,v $ $Revision: 4.2.8.5 $ (DEC) $Date: 1993/10/11 16:43:59 $";
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
/*
 * ftpd.c
 *
 *	Revision History:
 *
 * 01-Apr-91	Fred Canter
 *	MIPS C 2.20+, changes for -std.
 *
 * 12-Mar-91	Fred Canter
 *	MIPS C 2.20+
 *
 */
#if !defined(lint) && !defined(_NOIDENT)

#endif
/*

*/
/* 
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1985, 1988 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice and this paragraph are
 * duplicated in all such forms and that any documentation,
 * advertising materials, and other materials related to such
 * distribution and use acknowledge that the software was developed
 * by the University of California, Berkeley.  The name of the
 * University may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */
/*
#ifndef lint
char copyright[] =
" Copyright (c) 1985, 1988 Regents of the University of California.\n\
 All rights reserved.\n";
#endif 

#ifndef lint
static char sccsid[] = "ftpd.c	5.37	(Berkeley) 6/27/90";
#endif  not lint */

/*
 * FTP server.
 */
#include <sys/param.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/file.h>
#include <sys/wait.h>
#include <sys/dir.h>

#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <netinet/ip.h>

#define FTP_NAMES
#include <arpa/ftp.h>
#include <arpa/inet.h>
#include <arpa/telnet.h>

#include <ctype.h>
#include <stdio.h>
#include <signal.h>
#include <pwd.h>
#include <setjmp.h>
#include <netdb.h>
#include <errno.h>
#include <strings.h>
#include <sys/syslog.h>
#ifdef	__STDC__
#include <stdarg.h>
#else
#include <varargs.h>
#endif
#include "pathnames.h"

#include <nl_types.h>
#include <locale.h>
#include "ftpd_msg.h" 
#include <sia.h>				/* SIA */
nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_FTPD,n,s) 

/*
 * File containing login names
 * NOT to be used on this machine.
 * Commonly used to disallow uucp.
 */

extern	int errno;
extern	char *sys_errlist[];
extern	int sys_nerr;
extern	char *crypt();
extern	char version[];
extern	char *home;		/* pointer to home directory for glob */
extern	FILE *ftpd_popen(), *fopen(), *freopen();
extern	int  ftpd_pclose(), fclose();
extern	char *getline();
extern	char cbuf[];
extern  off_t restart_point;
extern  void old_openlog(), old_syslog();

struct	sockaddr_in ctrl_addr;
struct	sockaddr_in data_source;
struct	sockaddr_in data_dest;
struct	sockaddr_in his_addr;
struct	sockaddr_in pasv_addr;

int	data;
jmp_buf	errcatch, urgcatch;
int	logged_in;
struct	passwd *pw;
int	debug;
int	timeout = 900;    /* timeout after 15 minutes of inactivity */
int	maxtimeout = 7200;/* don't allow idle time to be set beyond 2 hours */
int	logging;
int	guest;
int	type;
int	form;
int	stru;			/* avoid C keyword */
int	mode;
int	usedefault = 1;		/* for data transfers */
int	pdata = -1;		/* for passive mode */
int	transflag;
off_t	file_size;
off_t	byte_count;
#if !defined(CMASK) || CMASK == 0
#undef CMASK
#define CMASK 027
#endif
int	defumask = CMASK;		/* default umask value */
char	tmpline[7];
char	hostname[MAXHOSTNAMELEN];
char	remotehost[MAXHOSTNAMELEN];

/*
 * Timeout intervals for retrying connections
 * to hosts that don't accept PORT cmds.  This
 * is a kludge, but given the problems with TCP...
 */
#define	SWAITMAX	90	/* wait at most 90 seconds */
#define	SWAITINT	5	/* interval between retries */

int	swaitmax = SWAITMAX;
int	swaitint = SWAITINT;

void lostconn();
void	myoob();
FILE	*getdatasock(), *dataconn();

#ifdef SETPROCTITLE
char	**Argv = NULL;		/* pointer to argument vector */
char	*LastArgv = NULL;	/* end of argv */
char	proctitle[BUFSIZ];	/* initial part of title */
#endif /* SETPROCTITLE */

#ifndef	_NO_PROTO
void reply(int n, char *s, ...);
#else
void reply();
#endif

#ifndef	_NO_PROTO
void lreply(int n, char *s, ...);
#else
void lreply();
#endif

#ifdef IP_TOS
#define MAXALIASES	35 
#define BSIZ		512

static char TOSDB[] = "/etc/iptos" ;
static FILE *tosf = NULL;
struct tosent {
        char    *t_name;        /* name */
        char    **t_aliases;    /* alias list */
        char    *t_proto;       /* protocol */
        int     t_tos;          /* Type Of Service bits */
};


struct tosent *tp ;

static char * getcommon_any();
static struct tosent *interpret_local();
static struct tosent *gettosbyname(); 
static struct tosent *gettosent_local(); 
static int stayopen;

static struct tosent *
gettosbyname_default(name,proto)
char *name, *proto;
{
	static struct tosent tos;
	static char *aliasp = 0;

	tos.t_name = name;
	tos.t_proto = proto;
	tos.t_aliases = &aliasp;
	if (strcmp(name, "ftp-data") == 0)
		tos.t_tos = IPTOS_THROUGHPUT;
	else
		tos.t_tos = IPTOS_LOWDELAY;
	return(&tos);
}

struct tosent *	
gettosbyname(name,proto)

   char *name, *proto ;

{
	register struct tosent *p;
	register char **cp;
	
	settosent_local(0);
	while(p = gettosent_local()) {
        if (strcmp(name,p->t_name) == 0)
		goto gotname ;
	for (cp = p->t_aliases; *cp; cp++)
		if (strcmp(name, *cp) == 0)
			goto gotname ;
	continue ;
gotname:
		if ((proto == 0) || (*p->t_proto == '*') ||
		    (strcmp(p->t_proto, proto) == 0))
		         break ;
       }
       endtosent_local();
       return(p) ;
}
struct tosent *
gettosent_local()

{

	static char line1[BSIZ+1];
 
        if (tosf == NULL && (tosf = fopen(TOSDB,"r")) == NULL)
		return(NULL);
	if (fgets(line1, BSIZ, tosf) == NULL)
		return (NULL) ;
	return interpret_local(line1, strlen(line1));
}

static struct tosent *
interpret_local(val,len)
	char *val;
	int len;
{
	static char *tos_aliases[MAXALIASES];
	static struct tosent tos ;
	static char line[BSIZ+1];
	char *p = NULL;
	register char *cp, **q;

	strncpy(line, val, len) ;
	p = line;
	line[len] = '\n';
	if (*p == '#')
		return(gettosent_local());
	cp = getcommon_any(p, "#\n");
	if (cp == NULL)
		return(gettosent_local());
	*cp = '\0' ;
	tos.t_name = p;			/* get application name */
	p = getcommon_any(p, " \t") ;
	if (p == NULL)
		return(gettosent_local());
	*p++ = '\0' ;
	while (*p == ' ' || *p == '\t')
		p++ ;
        tos.t_proto = p;		/* get protocol */
        p = getcommon_any(p, " \t") ;
	if (p == NULL)
		return (gettosent_local());
        *p++ = '\0' ;
	while (*p == ' ' || *p == '\t')
		p++ ;
	tos.t_tos = strtol(p,NULL,0);		/* get TOS bits */
	q = tos.t_aliases = tos_aliases ;
	cp = getcommon_any(p, " \t") ;
	if (cp != NULL)
		*cp++ = '\0' ;
	while (cp && *cp) {
		if (*cp == ' ' || *cp == '\t') {
			cp++ ;
			continue;
		}
		if (q < &tos_aliases[MAXALIASES - 1])
			*q++ = cp ;		/* get aliases */
		cp = getcommon_any(cp, " \t") ;
		if (cp != NULL) 
			*cp++ = '\0';
	}
	*q = NULL;
	return(&tos) ;
}

static char *
getcommon_any(cp,match) 
	register char *cp;
	char *match;
{
        register char *mp, c;

        while (c = *cp) {
                for (mp = match; *mp; mp++)
                        if (*mp == c)
                                return (cp);
                cp++;
        }
        return ((char *)0);
}

settosent_local(f)
        int f;
{
        if (tosf == NULL)
                tosf = fopen(TOSDB, "r");
        else
                rewind(tosf);
        stayopen |= f;
}
endtosent_local()
{
        if (tosf && !stayopen) {
                fclose(tosf);
                tosf = NULL;
        }
}
#endif /* IP_TOS */


int sia_argc;					 /* SIA */
char **sia_argv ;				 /* SIA */
SIAENTITY *entity=NULL ;			 /* SIA */

main(argc, argv, envp)
	int argc;
	char *argv[];
	char **envp;
{
	extern int optind;
	extern char *optarg;
	int addrlen, on = 1;
#ifdef IP_TOS
	struct tosent *tos;
#endif
	int ch; 
	pid_t pid;

        sia_argc = argc ;			 /* SIA */
	sia_argv = argv ;			 /* SIA */

	setlocale(LC_ALL, "");
	catd = NLcatopen (MF_FTPD, NL_CAT_LOCALE);

	addrlen = sizeof (his_addr);
	if (getpeername(0, (struct sockaddr *)&his_addr, &addrlen) < 0) {
		local_syslog(LOG_ERR, MSGSTR(PEERNAME, "getpeername (%s): %m"),argv[0]);
		exit(1);
	}
	addrlen = sizeof (ctrl_addr);
	if (getsockname(0, (struct sockaddr *)&ctrl_addr, &addrlen) < 0) {
	    local_syslog(LOG_ERR, MSGSTR(SOCKNAME,"getsockname (%s): %m"),argv[0]);
		exit(1);
	}
#ifdef IP_TOS
	if ((tos = gettosbyname("ftp-control", "tcp")) == NULL)
		tos = gettosbyname_default("ftp-control", "tcp");
        if (setsockopt(0, IPPROTO_IP, IP_TOS, (char *)&tos->t_tos, sizeof(int)) < 0)
                local_syslog(LOG_WARNING, MSGSTR( FTPD_SETSOCK_TOS,
			"setsockopt (IP_TOS): %m"));
#endif
	data_source.sin_port = htons(ntohs(ctrl_addr.sin_port) - 1);
	debug = 0;
	openlog("ftpd", LOG_PID, LOG_DAEMON);
	old_openlog("ftpd", LOG_PID, LOG_DAEMON);
#ifdef SETPROCTITLE
	/*
	 *  Save start and extent of argv for setproctitle.
	 */
	Argv = argv;
	while (*envp)
		envp++;
	LastArgv = envp[-1] + strlen(envp[-1]);
#endif /* SETPROCTITLE */

        while ((ch = getopt(argc, argv, "dlsT:t:vu:")) != EOF)
                switch((char)ch) {
                case 'd':
		case 'v':
			debug = 1;
			break;

		case 'l':
			logging = 1;
			break;

		case 't':
			timeout = atoi(optarg);
			if (maxtimeout < timeout)
				maxtimeout = timeout;
			break;

		case 'T':
			maxtimeout = atoi(optarg);
			if (timeout > maxtimeout)
				timeout = maxtimeout;
			break;

		case 'u':
			{
				int n;

				/* Verify that all the digits are octal */
				for (n = 0; n < strlen (optarg); n++)
				   if ((optarg[n] < '0') || (optarg[n] > '7'))
				      break;

				if (n == strlen (optarg))
					sscanf (optarg, "%o", &defumask);
				else
					fprintf (stderr, 
				  MSGSTR(BADVALUE, "ftpd: Bad value for -u\n"));

			}
			break;

                default:
			fprintf(stderr, 
	       MSGSTR(UNKFLAG, "ftpd: Unknown flag -%c ignored.\n"), (char) ch);
                }
        argc -= optind;
        argv += optind;


	(void) freopen(_PATH_DEVNULL, "w", stderr);
	(void) signal(SIGPIPE, lostconn);
	(void) signal(SIGCHLD, SIG_IGN);
	if ((int)signal(SIGURG, myoob) < 0)
		local_syslog(LOG_ERR, MSGSTR(SIGNAL, "signal: %m"));

	/* Try to handle urgent data inline */
#ifdef SO_OOBINLINE
	if (setsockopt(0, SOL_SOCKET, SO_OOBINLINE, (char *)&on, sizeof(on)) < 0)
		local_syslog(LOG_ERR, MSGSTR(SOCKOPT, "setsockopt: %m"));
#endif

#ifdef  F_SETOWN
        if (fcntl(fileno(stdin), F_SETOWN, getpid()) == -1)
                local_syslog(LOG_ERR, "fcntl F_SETOWN: %m");
#endif /* F_SETOWN */

	dolog(&his_addr);
	/*
	 * Set up default state
	 */
	data = -1;
	type = TYPE_A;
	form = FORM_N;
	stru = STRU_F;
	mode = MODE_S;
	tmpline[0] = '\0';
	(void) gethostname(hostname, sizeof (hostname));
	reply(220, MSGSTR(SERVREADY, "%s FTP server (OSF/1 %s) ready."), hostname, version);
	(void) setjmp(errcatch);
	for (;;)
		(void) yyparse();
	/* NOTREACHED */
}

void
lostconn()
{

	if (debug)
		local_syslog(LOG_DEBUG, MSGSTR(LOSTCONN, "lost connection"));
	dologout(-1);
}

static char ttyline[20];

/*
 * Helper function for sgetpwnam().
 */
char *
sgetsave(s)
	char *s;
{
	char *new = malloc((unsigned) strlen(s) + 1);

	if (new == NULL) {
		perror_reply(421, MSGSTR(MALLOCERR, "Local resource failure: malloc"));
		dologout(1);
		/* NOTREACHED */
	}
	(void) strcpy(new, s);
	return (new);
}

/*
 * Save the result of a getpwnam.  Used for USER command, since
 * the data returned must not be clobbered by any other command
 * (e.g., globbing).
 */
struct passwd *
sgetpwnam(name)
	char *name;
{
	static struct passwd save;
	register struct passwd *p;
	char *sgetsave();

	if ((p = getpwnam(name)) == NULL)
		return (p);
	if (save.pw_name) {
		free(save.pw_name);
		free(save.pw_passwd);
		free(save.pw_gecos);
		free(save.pw_dir);
		free(save.pw_shell);
	}
	save = *p;
	save.pw_name = sgetsave(p->pw_name);
	save.pw_passwd = sgetsave(p->pw_passwd);
	save.pw_gecos = sgetsave(p->pw_gecos);
	save.pw_dir = sgetsave(p->pw_dir);
	save.pw_shell = sgetsave(p->pw_shell);
	return (&save);
}

int login_attempts;		/* number of failed login attempts */
int askpasswd;			/* had user command, ask for passwd */

/*
 * USER command.
 * Sets global passwd pointer pw if named account exists and is acceptable;
 * sets askpasswd if a PASS command is expected.  If logged in previously,
 * need to reset state.  If name is "ftp" or "anonymous", the name is not in
 * _PATH_FTPUSERS, and ftp account exists, set guest and pw, then just return.
 * If account doesn't exist, ask for passwd anyway.  Otherwise, check user
 * requesting login privileges.  Disallow anyone who does not have a standard
 * shell as returned by getusershell().  Disallow anyone mentioned in the file
 * _PATH_FTPUSERS to allow people such as root and uucp to be avoided.
 */
user(name)
	char *name;
{
	register char *cp;
	char *shell;
	char *getusershell();


	if (logged_in) {
		if (guest) {
			reply(530, MSGSTR(NOCHANGE, "Can't change user from guest login."));
			return;
		}
		end_login();
	}
	guest = 0;
	if (strcmp(name, "ftp") == 0 || strcmp(name, "anonymous") == 0) {
		if (checkuser("ftp") || checkuser("anonymous")) {
		    	reply(530, MSGSTR(DENIED, "User %s access denied."), name);
			if (logging)
				local_syslog(LOG_NOTICE,
				    MSGSTR(REFUSED, "FTP LOGIN REFUSED FROM %s, %s"), remotehost, name);
		} else if ((pw = sgetpwnam("ftp")) != NULL) {
			guest = 1;
			askpasswd = 1;
			reply(331, MSGSTR(LOGINOK, "Guest login ok, send ident as password."));
		} else {
			reply(530, MSGSTR(USRUNK, "User %s unknown."), name);
			if (logging)
				local_syslog(LOG_INFO, MSGSTR(LOGINFAIL4, "login from %s failed, user %s unknown"), remotehost, name);
		}
		return;
	}
	if (pw = sgetpwnam(name)) {
		if ((shell = pw->pw_shell) == NULL || *shell == 0)
			shell = _PATH_BSHELL;
		while ((cp = getusershell()) != NULL)
                        if (strcmp(cp, shell) == 0)
                                break;
                endusershell();
		if (cp == NULL || checkuser(name)) {
		    reply(530, MSGSTR(DENIED, "User %s access denied."), name);
			if (logging)
				local_syslog(LOG_NOTICE,
				    MSGSTR(REFUSED, "FTP LOGIN REFUSED FROM %s, %s"),
				    remotehost, name);
			pw = (struct passwd *) NULL;
			return;
		}
	}
	reply(331, MSGSTR(PWDREQ, "Password required for %s."), name);
	askpasswd = 1;
	/*
	 * Delay before reading passwd after first failed
	 * attempt to slow down passwd-guessing programs.
	 */

	if (login_attempts)
		sleep((unsigned) login_attempts);
}

/*
 * Check if a user is in the file _PATH_FTPUSERS
 */
checkuser(name)
        char *name;
{
        register FILE *fd;
        register char *p;
	char line[BUFSIZ];

        if ((fd = fopen(_PATH_FTPUSERS, "r")) != NULL) {
                while (fgets(line, sizeof (line), fd) != NULL) {
			if ((p = index(line, '\n')) != NULL)
                                *p = '\0';
			if (line[0] == '#')
				continue;
                        if (strcmp(line, name) == 0)
                                return (1);
                }
                (void) fclose(fd);
        }
	return(0);
}

/*
 * Terminate login as previous user, if any, resetting state;
 * used when USER command is given or login fails.
 */
end_login()
{

	(void) seteuid((uid_t)0);
	if (logged_in)
		logwtmp(ttyline, "", "");
	if (logging)
		local_syslog(LOG_INFO, MSGSTR(TERMLOGIN, "terminate FTP login as previous user %s"), pw?pw->pw_name:(char *)NULL);
	pw = NULL;
	logged_in = 0;
	guest = 0;
	
}

pass(passwd)
	char *passwd;
{
	char *xpasswd, *salt;
	uid_t seuid ;

	if (logged_in || askpasswd == 0) {
		reply(503, MSGSTR(LOGINUSER, "Login with USER first."));
		return;
	}
	askpasswd = 0;

	if (!guest) { /* "ftp" is only account allowed no password */
		if (passwd == NULL || *passwd == '\0' || pw == NULL) {
			reply(530, MSGSTR(LOGINERR,"Login incorrect.")) ;
			if (logging) {
			   if (pw == NULL)
				local_syslog(LOG_WARNING,
				   MSGSTR(LOGINFAIL1, "login from %s failed, user unknown"), remotehost);
			   else
				local_syslog(LOG_WARNING,
				   MSGSTR(LOGINFAIL2, "login %s from %s failed, no password"), pw->pw_name, remotehost);
			}
			if (login_attempts++ >= 5) {
		   	   local_syslog(LOG_NOTICE, MSGSTR(LOGINFAIL, "repeated login failures from %s."), remotehost);
		    	   exit(0) ;
			}
			return ;
		}
	}

	/* SIA - sia session initialization */

	if ((sia_ses_init(&entity,sia_argc,sia_argv,hostname,pw->pw_name,(char *)NULL,FALSE,(char *)NULL))
        != SIASUCCESS) {
             reply(530, "Initialization failure") ;
             return ;
        }


	/* SIA - session authentication */

	if (!guest) {
	    if ((sia_ses_authent(NULL,passwd,entity)) != SIASUCCESS) {
	        reply(530, MSGSTR(LOGINERR,"Login incorrect. ")) ;
		if (logging)
		   local_syslog(LOG_WARNING,
			  MSGSTR(LOGINFAIL3, "login %s from %s failed."),
				pw->pw_name, remotehost);
		if (login_attempts++ >= 5) {
		   local_syslog(LOG_NOTICE,
		    MSGSTR(LOGINFAIL, "repeated login failures from %s."), remotehost);
		    sia_ses_release(&entity) ;
		    exit(0) ;
		}
                sia_ses_release(&entity) ;
                return ;
	    }
	}

	/* SIA - sia session establishment */

	if ((sia_ses_estab(NULL,entity)) != SIASUCCESS) {
	   reply(530, MSGSTR(LOGINERR,"Login incorrect. ")) ;
           sia_ses_release(&entity) ;
	   return ;
        }

	/* SIA - sia session launching */

        if (sia_ses_launch(NULL,entity) != SIASUCCESS) {
           reply(530, MSGSTR(LOGINERR, "Login incorrect. ")) ;
	   sia_ses_release(&entity) ;
           return ;
        }

	login_attempts = 0 ;		/* this time successful */
	logged_in = 1 ;
	(void) sprintf(ttyline, "ftp%d", getpid());
	logwtmp(ttyline, pw->pw_name, remotehost);

	/* SIA - get the password entry structure from entity structure */

	strncpy(pw->pw_name,entity->pwd->pw_name,sizeof(pw->pw_name)) ;
        strncpy(pw->pw_passwd,entity->pwd->pw_passwd,sizeof(pw->pw_passwd)) ;
        strncpy(pw->pw_gecos,entity->pwd->pw_gecos,sizeof(pw->pw_gecos)) ;
        strncpy(pw->pw_dir,entity->pwd->pw_dir,sizeof(pw->pw_dir)) ;
        strncpy(pw->pw_shell,entity->pwd->pw_shell,sizeof(pw->pw_shell)) ;
	
	if (guest) {
 
		/* 
		 * SIA -ses_estab has set the euid to be that of
		 * guest.  Save this and then set the euid to be
		 * root.
		 */
		if ((seuid = geteuid()) < 0) {
		    reply(550, "Can't set guest privileges.") ;
		    goto bad ;
		}
		if (seteuid((uid_t)0) < 0) {
		   reply(550, "Can't set guest privileges.") ;
		   goto bad ;
		}
	        /*
		 * We must do a chdir() after the chroot.  Otherwise
		 * the old current directory will be accessible as "."
		 * outside the new root!
		 */
		if (chroot(pw->pw_dir) < 0 || chdir("/") < 0) {
			reply(550, "Can't set guest privileges.") ;
		(void)seteuid(seuid) ;
			goto bad ;
		}
	        if (seteuid(seuid) < 0) {    /* SIA - set back user's euid */
		   reply(550, "Can't set guest privileges.") ;
		   goto bad ;
		}
	}
	if (!guest) {		/* SIA - just to make sure */
		if (strcmp(pw->pw_dir,"/") == 0)
		{
		if (strcmp(pw->pw_name,"root") != 0)
		lreply(230, MSGSTR(NODIR, "No directory! Logging in with home.")) ;
		}
		else
		{
		if (chdir(pw->pw_dir) < 0) {
		   if (chdir("/") < 0) {
			reply(530, MSGSTR(DIRERR, "User %s: can't change directory."), pw->pw_name,pw->pw_dir) ;
			goto bad ;
		   } else
			lreply(230, MSGSTR(NODIR, "No directory! Logging in withhome.")) ;
		}
	      }
	}

	if (guest) {
		reply(230, MSGSTR(GUESTLOGIN, "Guest login ok, access restrictions apply."));
#ifdef SETPROCTITLE
		sprintf(proctitle, "%s: anonymous/%.*s", remotehost,
		    sizeof(proctitle) - sizeof(remotehost) -
		    sizeof(": anonymous/"), passwd);
		setproctitle(proctitle);
#endif /* SETPROCTITLE */
		if (logging)
			local_syslog(LOG_INFO, MSGSTR(ANONYMLOGIN, "ANONYMOUS FTP LOGIN FROM %s, id=%s"),
			    remotehost, passwd);
	} else {
		reply(230, MSGSTR(LOGIN, "User %s logged in."), pw->pw_name);
#ifdef SETPROCTITLE
		sprintf(proctitle, "%s: %s", remotehost, pw->pw_name);
		setproctitle(proctitle);
#endif /* SETPROCTITLE */
		if (logging)
			local_syslog(LOG_INFO, MSGSTR(LOGINFROM, "FTP LOGIN FROM %s, %s"),
			    remotehost, pw->pw_name);
	}

	home = pw->pw_dir;		/* home dir for globbing */
	(void) umask(defumask);
        sia_ses_release(&entity) ;	/* SIA */
	return;
bad:
        /* Forget all about it... after doing an audit write */
	if (logging)
		local_syslog(LOG_INFO, MSGSTR(ENDLOGIN, "End login %s: %m"), pw->pw_name);
	end_login();
	sia_ses_release(&entity) ;	/* SIA */
}

retrieve(cmd, name)
	char *cmd, *name;
{
	FILE *fin, *dout;
	struct stat st;
	int (*closefunc)();
	char buf[MAXPATHLEN*2+20];
	extern char *getwd();

	if (cmd == 0) {
		fin = fopen(name, "r"), closefunc = fclose;
		if (logging) {
			char wd[MAXPATHLEN];

			if (name[0] == '/')
				(void) sprintf(buf, MSGSTR(RETRIEVE1, "retrieve %s "), name);
			else {
				getwd(wd);
				(void) sprintf(buf, MSGSTR(RETRIEVE2, "retrieve %s/%s "), (strcmp(wd, "/") ? wd : ""), name);
			}
		}
		st.st_size = 0;
	} else {
		char line[BUFSIZ];

		(void) sprintf(line, cmd, name), name = line;
		if (logging) {
			char wd[MAXPATHLEN];

			(void) sprintf(buf, 
				MSGSTR(CMD1, "%s (cwd=%s) "), line, getwd(wd)); 
		}
		fin = ftpd_popen(line, "r"), closefunc = ftpd_pclose;
		st.st_size = -1;
		st.st_blksize = BUFSIZ;
	}
	if (fin == NULL) {
		if (errno != 0) {
			perror_reply(550, name);
			if (logging) {
				strcat(buf, MSGSTR(FAILED1, "failed: %m"));
				local_syslog(guest?LOG_WARNING:LOG_INFO, buf);
			}
		}
		return;
	}
	if (cmd == 0 &&
	    (fstat(fileno(fin), &st) < 0 || (st.st_mode&S_IFMT) != S_IFREG)) {
		reply(550, MSGSTR(NOTPLAINF, "%s: not a plain file."), name);
		if (logging)
			strcat(buf, MSGSTR(FAILED2, "failed: not a plain file"));
		goto done;
	}
	if (restart_point) {
                if (type == TYPE_A) {
                        register int i, n, c;

                        n = restart_point;
                        i = 0;
			 while (i++ < n) {
                                if ((c=getc(fin)) == EOF) {
                                        perror_reply(550, name);
					if (logging)
						strcat(buf, MSGSTR(FAILED1, "failed: %m"));
                                        goto done;
                                }
                                if (c == '\n')
					i++;
			}
		} else if (lseek(fileno(fin), restart_point, L_SET) < 0) {
                        perror_reply(550, name);
			if (logging)
				strcat(buf, MSGSTR(FAILED1, "failed: %m"));
                        goto done;
                }
        }
	dout = dataconn(name, st.st_size, "w");
	if (dout == NULL) {
		if (logging)
			strcat(buf, MSGSTR(FAILED, "failed"));
		goto done;
	}
	if (send_data(fin, dout, st.st_blksize) == 0)
		if (logging)
			strcat(buf, MSGSTR(SUCCEEDED1, "succeeded, %d bytes."));
	else
		if (logging)
			strcat(buf, MSGSTR(FAILED, "failed"));
	(void) fclose(dout);
	data = -1;
	pdata = -1;
done:
	if (logging)
		local_syslog(guest?LOG_WARNING:LOG_INFO, buf, byte_count);
	(*closefunc)(fin);
}

store(name, mode, unique)
	char *name, *mode;
	int unique;
{
	FILE *fout, *din;
	struct stat st;
	int (*closefunc)();
	char *gunique();
	char buf[MAXPATHLEN*2+20];
	extern char *getwd();

	if (logging) {
		char wd[MAXPATHLEN];

		if (name[0] == '/')
			(void) sprintf(buf, MSGSTR(STORE1, "store %s "),name);
		else {
			getwd(wd);
			(void) sprintf(buf, MSGSTR(STORE2, "store %s/%s "), 
			(strcmp(wd, "/") ? wd : ""), name);
		}
	}
	if (unique && stat(name, &st) == 0 &&
	    (name = gunique(name)) == NULL)
		return;

	if (restart_point)
                mode = "r+w";
	fout = fopen(name, mode);
	closefunc = fclose;
	if (fout == NULL) {
		perror_reply(553, name);
		if (logging) {
			strcat(buf, MSGSTR(FAILED1, "failed: %m"));
			local_syslog(guest?LOG_WARNING:LOG_INFO, buf);
		}
		return;
	}
	if (restart_point) {
                if (type == TYPE_A) {
                        register int i, n, c;

                        n = restart_point;
                        i = 0;
                        while (i++ < n) {
				if ((c=getc(fout)) == EOF) {
                                        perror_reply(550, name);
					if (logging)
						strcat(buf, MSGSTR(FAILED1, "failed: %m"));	
                                        goto done;
                                }
                                if (c == '\n')
                               		i++;
			}
			/*
                         * We must do this seek to "current" position
                         * because we are changing from reading to
                         * writing.
                         */
			 if (fseek(fout, 0L, L_INCR) < 0) {
                                perror_reply(550, name);
				if (logging)
					strcat(buf, MSGSTR(FAILED1, "failed: %m"));	
                                goto done;
                        }
                } else if (lseek(fileno(fout), restart_point, L_SET) < 0) {
                        perror_reply(550, name);
			if (logging)
				strcat(buf, MSGSTR(FAILED1, "failed: %m"));
                        goto done;
                }
        }
	din = dataconn(name, (off_t)-1, "r");
	if (din == NULL) {
		if (logging)
			strcat(buf, MSGSTR(FAILED, "failed"));
		goto done;
	}
	if (receive_data(din, fout) == 0) {
		if (unique)
			reply(226, MSGSTR(TRANSCOMP, "Transfer complete (unique file name:%s)."), name);
		else
			reply(226, MSGSTR(TRANSOK, "Transfer complete."));
		if (logging)
			strcat(buf, MSGSTR(SUCCEEDED1, "succeeded, %d bytes."));
	} else
		if (logging)
			strcat(buf, MSGSTR(FAILED, "failed"));
	(void) fclose(din);
	data = -1;
	pdata = -1;
done:
	if (logging)
		local_syslog(guest?LOG_WARNING:LOG_INFO, buf, byte_count);
	(*closefunc)(fout);
}

FILE *
getdatasock(mode)
	char *mode;
{
	int s, on = 1, tries;
#ifdef IP_TOS
	struct tosent *tos;
#endif

	if (data >= 0)
		return (fdopen(data, mode));
	s = socket(AF_INET, SOCK_STREAM, 0);
	if (s < 0)
		return (NULL);
	(void) seteuid((uid_t)0);
	if (setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (char *) &on, sizeof (on)) < 0)
		goto bad;
	/* anchor socket to avoid multi-homing problems */
	data_source.sin_family = AF_INET;
	data_source.sin_addr = ctrl_addr.sin_addr;
	for (tries = 1; ; tries++) {
                if (bind(s, (struct sockaddr *)&data_source,
                    sizeof (data_source)) >= 0)
                        break;
                if (errno != EADDRINUSE || tries > 10)
                        goto bad;
                sleep(tries);
	}
	(void) seteuid((uid_t)pw->pw_uid);
#ifdef IP_TOS
	if ((tos = gettosbyname("ftp-data", "tcp")) == NULL)
		tos = gettosbyname_default("ftp-data", "tcp");
        if (setsockopt(s, IPPROTO_IP, IP_TOS, (char *)&tos->t_tos, sizeof(int)) < 0)
                local_syslog(LOG_WARNING, MSGSTR( FTPD_SETSOCK_TOS, 
			"setsockopt (IP_TOS): %m"));
#endif
	return (fdopen(s, mode));
bad:
	(void) seteuid((uid_t)pw->pw_uid);
	(void) close(s);
	return (NULL);
}

FILE *
dataconn(name, size, mode)
	char *name;
	off_t size;
	char *mode;
{
	char sizebuf[32];
	FILE *file;
	int retry = 0;
#ifdef IP_TOS
	struct tosent *tos;
#endif

	file_size = size;
	byte_count = 0;
	if (size != (off_t) -1)
		(void) sprintf (sizebuf, MSGSTR(BYTES, " (%ld bytes)"), size);
	else
		(void) strcpy(sizebuf, "");
	if (pdata >= 0) {
		struct sockaddr_in from;
		int s, fromlen = sizeof(from);

		s = accept(pdata, (struct sockaddr *)&from, &fromlen);
		if (s < 0) {
			reply(425, MSGSTR(NOCONN, "Can't open data connection."));
			(void) close(pdata);
			pdata = -1;
			return(NULL);
		}
		(void) close(pdata);
		pdata = s;

#ifdef IP_TOS
	if ((tos = gettosbyname("ftp-data", "tcp")) == NULL)
		tos = gettosbyname_default("ftp-data", "tcp");
        if (setsockopt(s, IPPROTO_IP, IP_TOS, (char *)&tos->t_tos, sizeof(int)) < 0)
                local_syslog(LOG_WARNING, MSGSTR( FTPD_SETSOCK_TOS, 
			"setsockopt (IP_TOS): %m"));
#endif
		reply(150, MSGSTR(OPENCONN, "Opening %s mode data connection for %s (%s,%d)%s."),
	     (type == TYPE_A ? MSGSTR(ASCII, "ASCII") : MSGSTR(BINARY, "BINARY")), name, inet_ntoa(data_dest.sin_addr), ntohs(data_dest.sin_port), sizebuf); 
		return(fdopen(pdata, mode));
	}
	if (data >= 0) {
		reply(125, MSGSTR(USECONN, "Using existing data connection for %s%s."), name, sizebuf); 
		usedefault = 1;
		return (fdopen(data, mode));
	}
	if (usedefault)
		data_dest = his_addr;
	usedefault = 1;
	file = getdatasock(mode);
	if (file == NULL) {
		reply(425, MSGSTR(NODATASOCK, "Can't create data socket (%s,%d): %s."),
		    inet_ntoa(data_source.sin_addr),
		    ntohs(data_source.sin_port),
		    errno < sys_nerr ? sys_errlist[errno] : "unknown error");
		return (NULL);
	}
	data = fileno(file);
	while (connect(data, (struct sockaddr *)&data_dest,
	    sizeof (data_dest)) < 0) {
		if (errno == EADDRINUSE && retry < swaitmax) {
			sleep((unsigned) swaitint);
			retry += swaitint;
			continue;
		}
		perror_reply(425, MSGSTR(NODATACONN, "Can't build data connection"));
		(void) fclose(file);
		data = -1;
		return (NULL);
	}
	reply(150, MSGSTR(OPENCONN, "Opening %s mode data connection for %s (%s,%d)%s."),
	     type == TYPE_A ? MSGSTR(ASCII, "ASCII") : MSGSTR(BINARY, "BINARY"),  name, inet_ntoa(data_dest.sin_addr), ntohs(data_dest.sin_port), sizebuf); 

	return (file);
}

/*
 * Tranfer the contents of "instr" to
 * "outstr" peer using the appropriate
 * encapsulation of the data subject
 * to Mode, Structure, and Type.
 *
 * NB: Form isn't handled.
 */
int
send_data(instr, outstr, blksize)
	FILE *instr, *outstr;
	off_t blksize;
{
	register int c, cnt;
	register char *buf;
	int netfd, filefd;

	transflag++;
	if (setjmp(urgcatch)) {
		transflag = 0;
		return (-1);
	}
	switch (type) {

	case TYPE_A:
		while ((c = getc(instr)) != EOF) {
			byte_count++;
			if (c == '\n') {
				if (ferror(outstr))
					goto data_err;
				(void) putc('\r', outstr);
			}
			(void) putc(c, outstr);
		}
		fflush(outstr);
		transflag = 0;
		if (ferror(instr))
			goto file_err;
		if (ferror(outstr))
			goto data_err;
		reply(226, MSGSTR(TRANSOK, "Transfer complete."));
		return (0);

	case TYPE_I:
	case TYPE_L:
		if ((buf = malloc((u_int)blksize)) == NULL) {
			transflag = 0;
			perror_reply(451, MSGSTR(MALLOCERR, "Local resource failure: malloc"));
			return (-1);
		}
		netfd = fileno(outstr);
		filefd = fileno(instr);
		while ((cnt = read(filefd, buf, (u_int)blksize)) > 0 &&
		    write(netfd, buf, cnt) == cnt)
			byte_count += cnt;
		transflag = 0;
		(void)free(buf);
		if (cnt != 0) {
			if (cnt < 0)
				goto file_err;
			goto data_err;
		}
		reply(226, MSGSTR(TRANSOK, "Transfer complete."));
		return (0);
	default:
		transflag = 0;
		reply(550, MSGSTR(UNIMPTYPESEND, "Unimplemented TYPE %d in send_data"), type);
		return (-1);
	}

data_err:
	transflag = 0;
	perror_reply(426, MSGSTR(DATACONNERR, "Data connection"));
	return (-1);

file_err:
	transflag = 0;
	perror_reply(551, MSGSTR(ERRINPUT, "Error on input file"));
	return (-1);
}

/*
 * Transfer data from peer to
 * "outstr" using the appropriate
 * encapulation of the data subject
 * to Mode, Structure, and Type.
 *
 * N.B.: Form isn't handled.
 */
receive_data(instr, outstr)
	FILE *instr, *outstr;
{
	register int c;
	int cnt, bare_lfs = 0;
	char buf[BUFSIZ];

	transflag++;
	if (setjmp(urgcatch)) {
		transflag = 0;
		return (-1);
	}
	switch (type) {

	case TYPE_I:
	case TYPE_L:
		while ((cnt = read(fileno(instr), buf, sizeof buf)) > 0) {
			if (write(fileno(outstr), buf, cnt) != cnt)
				goto file_err;
			byte_count += cnt;
		}
		if (cnt < 0)
			goto data_err;
		transflag = 0;
		return (0);

	case TYPE_E:
		reply(553, MSGSTR(NOTYPEE, "TYPE E not implemented."));
		transflag = 0;
		return (-1);

	case TYPE_A:
		while ((c = getc(instr)) != EOF) {
			byte_count++;
			if (c == '\n')
                                bare_lfs++;
			while (c == '\r') {
				 if (ferror(outstr))
					goto data_err;
				if ((c = getc(instr)) != '\n') {
					(void) putc ('\r', outstr);
					if (c == '\0' || c == EOF)
                                                goto contin2;
				}
			}
			(void) putc(c, outstr);
	contin2:	;
		}
		fflush(outstr);
		if (ferror(instr))
			goto data_err;
		if (ferror(outstr))
			goto file_err;
		transflag = 0;
		if (bare_lfs) {
                        lreply(230, MSGSTR( FTPD_BARE_LFS, 
			"WARNING! %d bare linefeeds received in ASCII mode"), 
			bare_lfs);
                        printf(MSGSTR( FTPD_FILE_INC,
			 "   File may not have transferred correctly.\r\n"));
                }
		return (0);
	default:
		reply(550, MSGSTR(UNIMPTYPERECV, "Unimplemented TYPE %d in receive_data"), type);
		transflag = 0;
		return (-1);
	}

data_err:
	transflag = 0;
	perror_reply(426, MSGSTR(DATACONNERR, "Data connection"));
	return (-1);

file_err:
	transflag = 0;
	perror_reply(452, MSGSTR(ERRWRITE, "Error writing file"));
	return (-1);
}

statfilecmd(filename)
	char *filename;
{
	char line[BUFSIZ];
	FILE *fin;
	int c;
	struct stat stbuf;
	int rcode;

	(void) sprintf(line, "/bin/ls -lgA %s", filename);
	fin = ftpd_popen(line, "r");
	if (stat((char *)filename, &stbuf) == 0)
	   if ((stbuf.st_mode & S_IFMT) == S_IFDIR) {
		rcode = 212;
		lreply(rcode, MSGSTR(FTPD_STAT_FILE, "status of %s:"),filename);
	   } else {
		rcode = 213;
		lreply(rcode, MSGSTR(FTPD_STAT_FILE, "status of %s:"),filename);
	   }
	else {
	   rcode = 211;
	   lreply(rcode, MSGSTR(FTPD_STAT_FILE, "status of %s:"),filename);
	}
	while ((c = getc(fin)) != EOF) {
		if (c == '\n') {
			if (ferror(stdout)){
				perror_reply(421, MSGSTR(CNTRCONN, "control connection"));
				(void) ftpd_pclose(fin);
				dologout(1);
				/* NOTREACHED */
			}
			if (ferror(fin)) {
				perror_reply(551, filename);
				(void) ftpd_pclose(fin);
				return;
			}
			(void) putc('\r', stdout);
		}
		(void) putc(c, stdout);
	}
	(void) ftpd_pclose(fin);
	reply(rcode, MSGSTR(ENDSTATUS,"End of Status"));
}

statcmd()
{
	struct sockaddr_in *sin;
	u_char *a, *p;

	lreply(211, MSGSTR(SERVSTATUS, "%s FTP server status:"), hostname, version);
	printf(MSGSTR( FTPD_FMT1, "     %s\r\n"), version);
	printf(MSGSTR(CONN, "     Connected to %s"), remotehost);
	if (isdigit(remotehost[0]))
		printf(MSGSTR( FTPD_FMT2, " (%s)"), inet_ntoa(his_addr.sin_addr));
	printf(MSGSTR( FTPD_CRNL, "\r\n"));
	if (logged_in) {
		if (guest)
		    printf(MSGSTR(ANONYM, "     Logged in anonymously\r\n"));
		else
		    printf(MSGSTR(USERLOG,"     Logged in as %s\r\n"), pw->pw_name);
	} else if (askpasswd)
		printf(MSGSTR(WAITPWD, "     Waiting for password\r\n"));
	else
		printf(MSGSTR(WAITNAME, "     Waiting for user name\r\n"));
        printf(MSGSTR( FTPD_TYPE, "     TYPE: %s"), typenames[type]);
	if (type == TYPE_A || type == TYPE_E)
                printf(MSGSTR( FTPD_FORM, ", FORM: %s"), formnames[form]);
	if (type == TYPE_L)
#if NBBY == 8
		printf(MSGSTR( FTPD_FMT3, " %d"), NBBY);
#else
		printf(MSGSTR( FTPD_FMT3, " %d"), bytesize);	/* need definition! */
#endif
        printf(MSGSTR( FTPD_TRANS_MODE, 
		"; STRUcture: %s; transfer MODE: %s\r\n"),
            strunames[stru], modenames[mode]);
	if (data != -1)
		printf(MSGSTR(DATACONN1,"    211- Data connection open\r\n"));
	else if (pdata != -1) {
		printf(MSGSTR(PASSIVE, "   211- in Passive mode"));
		sin = &pasv_addr;
		goto printaddr;
	} else if (usedefault == 0) {
		printf(MSGSTR( FTPD_PORT, "     PORT"));
		sin = &data_dest;
printaddr:
		a = (u_char *) &sin->sin_addr;
		p = (u_char *) &sin->sin_port;
#define UC(b) (((int) b) & 0xff)
		printf(MSGSTR( FTPD_FMT4, " (%d,%d,%d,%d,%d,%d)\r\n"), 
			UC(a[0]), UC(a[1]), UC(a[2]), UC(a[3]), 
			UC(p[0]), UC(p[1]));
#undef UC
	} else
		printf(MSGSTR(NODATACONN, "   211- No data connection\r\n"));
	reply(211, MSGSTR(ENDSTATUS, "End of status"));
}

fatal(s)
	char *s;
{
	local_syslog(LOG_INFO, "In fatal routine\n") ;
	reply(451, MSGSTR(ERRSERV, "Error in server: %s\n"), s);
	reply(221, MSGSTR(CLOSECONN,"Closing connection due to server error."));
	dologout(0);
	/* NOTREACHED */
}

/* VARARGS2 */
#ifndef	_NO_PROTO
void
reply(int n, char *s, ...)
#else
void
reply(n, s, va_alist)
int n;
char *s;
va_dcl
#endif
{
	va_list	ap;

#ifdef __STDC__
	va_start(ap, s);
#else
	va_start(ap);
#endif /* __STDC__ */
	printf(MSGSTR( FTPD_FMT5, "%d "), n);
        vprintf(s, ap);
        printf(MSGSTR( FTPD_CRNL, "\r\n"));
        (void) fflush(stdout);
        va_end(ap);
	if (debug) {
#ifdef __STDC__
                va_start(ap, s);
#else
                va_start(ap);
#endif /* __STDC__ */
		 local_syslog(LOG_DEBUG, MSGSTR( FTPD_DEBUG, "<--- %d "), n);
                va_end(ap);
        }
	
}
/* VARARGS2 */
#ifndef	_NO_PROTO
void
lreply(int n, char *s, ...)
#else
void
lreply(n, s, va_alist)
int n;
char *s;
va_dcl
#endif
{
	va_list ap;
	
#ifdef __STDC__
	va_start(ap, s);
#else
	va_start(ap);
#endif /* __STDC__ */
	printf(MSGSTR( FTPD_FMT6, "%d-"), n);
        vprintf(s, ap);
        printf(MSGSTR( FTPD_CRNL, "\r\n"));
        (void) fflush(stdout);
        va_end(ap);
        if (debug) {
#ifdef __STDC__
		va_start(ap, s);
#else
		va_start(ap);
#endif /* __STDC__ */
		local_syslog(LOG_DEBUG, MSGSTR( FTPD_DEBUG2, "<--- %d- "), n);
                va_end(ap);
        }
}
ack(s)
	char *s;
{
	reply(250, MSGSTR(CMDOK, "%s command successful."), s);
}

nack(s)
	char *s;
{
	reply(502, MSGSTR(NOCMD, "%s command not implemented."), s);
}

/* ARGSUSED */
yyerror(s)
	char *s;
{
	char *cp;

	if (cp = index(cbuf,'\n'))
		*cp = '\0';
	/* send reply only if yy parsing finished to avoid breaking protocol */
	if (s == NULL)		
	    reply(500, MSGSTR(CMDUNK, "'%s': command not understood."), cbuf);
	else
	   if (debug)
		local_syslog(LOG_DEBUG, MSGSTR(YYERR, "yacc error: %s"), s);
}

delete(name)
	char *name;
{
	struct stat st;

	if (stat(name, &st) < 0) {
		perror_reply(550, name);
		return;
	}
	if ((st.st_mode&S_IFMT) == S_IFDIR) {
		if (rmdir(name) < 0) {
			perror_reply(550, name);
			return;
		}
		goto done;
	}
	if (unlink(name) < 0) {
		perror_reply(550, name);
		return;
	}
done:
	ack("DELE");
}

cwd(path)
	char *path;
{
	if (chdir(path) < 0)
		perror_reply(550, path);
	else
		ack("CWD");
}

makedir(name)
	char *name;
{
	if (mkdir(name, 0777) < 0)
		perror_reply(550, name);
	else
		reply(257, MSGSTR(MKD_OK, "MKD command successful."));
}

removedir(name)
	char *name;
{
	if (rmdir(name) < 0)
		perror_reply(550, name);
	else
		ack("RMD");
}

pwd()
{
	char path[MAXPATHLEN + 1];
	extern char *getwd();

	if (getwd(path) == (char *)NULL)
		reply(550, "%s.", path);
	else
		reply(257, MSGSTR(CURRDIR, "\"%s\" is current directory."), path);
}

char *
renamefrom(name)
	char *name;
{
	struct stat st;

	if (stat(name, &st) < 0) {
		perror_reply(550, name);
		return ((char *)0);
	}
	reply(350,MSGSTR(FILEEXIST, "File exists, ready for destination name"));
	return (name);
}

renamecmd(from, to)
	char *from, *to;
{
	if (rename(from, to) < 0)
		perror_reply(550, "rename");
	else
		ack("RNTO");
}

dolog(sin)
	struct sockaddr_in *sin;
{
	struct hostent *hp = gethostbyaddr((char *)&sin->sin_addr,
		sizeof (struct in_addr), AF_INET);
	time_t t, time();
	extern char *ctime();

	if (hp)
		(void) strncpy(remotehost, hp->h_name, sizeof (remotehost));
	else
		(void) strncpy(remotehost, inet_ntoa(sin->sin_addr),
		    sizeof (remotehost));
#ifdef SETPROCTITLE
	sprintf(proctitle, "%s: connected", remotehost);
	setproctitle(proctitle);
#endif /* SETPROCTITLE */

	if (logging) {
		t = time((time_t *) 0);
		local_syslog(LOG_INFO, MSGSTR( FTPD_CONN_FROM,
			"connection from %s at %s"),
		    remotehost, ctime(&t));
	}
}

/*
 * Record logout in wtmp file
 * and exit with supplied status.
 */
dologout(status)
	int status;
{
	if (logged_in) {
		(void) seteuid((uid_t)0);
		logwtmp(ttyline, "", "");
	}
	if (logging)
		local_syslog(guest?LOG_WARNING:LOG_INFO,
			  MSGSTR(LOGOUT, "FTP LOGOUT, %s"), pw->pw_name);
	/* beware of flushing buffers after a SIGPIPE */
	_exit(status);
}

void
myoob()
{
	char *cp;

	/* only process if transfer occurring */
	if (!transflag)
		return;
	cp = tmpline;
	if (getline(cp, 7, stdin) == NULL) {
		reply(221, MSGSTR(GOODBYE, "You could at least say goodbye."));
		dologout(0);
	}
	upper(cp);
	if (strcmp(cp, "ABOR\r\n") == 0) {
		tmpline[0] = '\0';
		reply(426, MSGSTR(CONNABORT, "Transfer aborted. Data connection closed."));
		reply(226, MSGSTR(ABORT_SUCCESS, "Abort successful"));
		longjmp(urgcatch, 1);
	}
	if (strcmp(cp, "STAT\r\n") == 0) {
		if (file_size != (off_t) -1)
			reply(213, MSGSTR(STATUS1, "Status: %lu of %lu bytes transferred"),
			    byte_count, file_size);
		else
			reply(213, MSGSTR(STATUS2, "Status: %lu bytes transferred"), byte_count);
	}
}

/*
 * Note: a response of 425 is not mentioned as a possible response to
 * 	the PASV command in RFC959. However, it has been blessed as
 * 	a legitimate response by Jon Postel in a telephone conversation
 *	with Rick Adams on 25 Jan 89.
 */
passive()
{
	int len;
	register char *p, *a;
	

	if (!logged_in){
            reply(530, MSGSTR(LOGUSERPASS,"Please login with USER and PASS."));
            return;
        }

	pdata = socket(AF_INET, SOCK_STREAM, 0);
	if (pdata < 0) {
		perror_reply(425, MSGSTR(NOPASSCONN, "Can't open passive connection"));
		return;
	}
	pasv_addr = ctrl_addr;
	pasv_addr.sin_port = 0;
	(void) seteuid((uid_t)0);
	if (bind(pdata, (struct sockaddr *)&pasv_addr, sizeof(pasv_addr)) < 0) {
		(void) seteuid((uid_t)pw->pw_uid);
		goto pasv_error;
	}
	(void) seteuid((uid_t)pw->pw_uid);
	len = sizeof(pasv_addr);
	if (getsockname(pdata, (struct sockaddr *) &pasv_addr, &len) < 0)
		goto pasv_error;
	if (listen(pdata, 1) < 0)
		goto pasv_error;
	a = (char *) &pasv_addr.sin_addr;
	p = (char *) &pasv_addr.sin_port;

#define UC(b) (((int) b) & 0xff)

	reply(227, MSGSTR(ENTRPASS, "Entering Passive Mode (%d,%d,%d,%d,%d,%d)"), UC(a[0]),
		UC(a[1]), UC(a[2]), UC(a[3]), UC(p[0]), UC(p[1]));
	return;

pasv_error:
	(void) close(pdata);
	pdata = -1;
	perror_reply(425, MSGSTR(NOPASSCONN, "Can't open passive connection"));
	return;
}

/*
 * Generate unique name for file with basename "local".
 * The file named "local" is already known to exist.
 * Generates failure reply on error.
 */
char *
gunique(local)
	char *local;
{
	static char new[MAXPATHLEN];
	struct stat st;
	char *cp = rindex(local, '/');
	int count = 0;

	if (cp)
		*cp = '\0';
	if (stat(cp ? local : ".", &st) < 0) {
		perror_reply(553, cp ? local : ".");
		return((char *) 0);
	}
	if (cp)
		*cp = '/';
	(void) strcpy(new, local);
	cp = new + strlen(new);
	*cp++ = '.';
	for (count = 1; count < 100; count++) {
		(void) sprintf(cp, "%d", count);
		if (stat(new, &st) < 0)
			return(new);
	}
	reply(452, MSGSTR(NOFILENAME, "Unique file name cannot be created."));
	return((char *) 0);
}

/*
 * Format and send reply containing system error number.
 */
perror_reply(code, string)
	int code;
	char *string;
{
	if (errno < sys_nerr)
		reply(code, "%s: %s.", string, sys_errlist[errno]);
	else
		reply(code, MSGSTR(UNKERR, "%s: unknown error %d."), string, errno);
}

static char *onefile[] = {
	"",
	0
};

send_file_list(whichfiles)
	char *whichfiles;
{
	struct stat st;
	DIR *dirp = NULL;
	struct direct *dir;
	FILE *dout = NULL;
	register char **dirlist, *dirname;
	int simple = 0;
	char *strpbrk();

	if (strpbrk(whichfiles, "~{[*?") != NULL) {
		extern char **glob(), *globerr;

		globerr = NULL;
		dirlist = glob(whichfiles);
		if (globerr != NULL) {
			reply(550, globerr);
			return;
		} else if (dirlist == NULL) {
			errno = ENOENT;
			perror_reply(550, whichfiles);
			return;
		}
	} else {
		onefile[0] = whichfiles;
		dirlist = onefile;
		simple = 1;
	}

	if (setjmp(urgcatch)) {
		transflag = 0;
		return;
	}
	while (dirname = *dirlist++) {
		if (logging) {
			if (dirname[0] == '/')
				local_syslog(LOG_INFO, 
					MSGSTR(NLST1, "NLST %s"), dirname);
			else {
				char wd[MAXPATHLEN];
				
				if (strcmp(getwd(wd), "/") == 0)
					wd[0] = '\0';
				local_syslog(LOG_INFO, 
					MSGSTR(NLST2, "NLST %s/%s"),wd,dirname);
			}
		}
		if (stat(dirname, &st) < 0) {
			/*
			 * If user typed "ls -l", etc, and the client
			 * used NLST, do what the user meant.
			 */
			if (dirname[0] == '-' && *dirlist == NULL &&
			    transflag == 0) {
				retrieve("/bin/ls %s", dirname);
				return;
			}
			perror_reply(550, whichfiles);
			if (dout != NULL) {
				(void) fclose(dout);
				transflag = 0;
				data = -1;
				pdata = -1;
			}
			return;
		}

		if ((st.st_mode&S_IFMT) == S_IFREG) {
			if (dout == NULL) {
				dout = dataconn("file list", (off_t)-1, "w");
				if (dout == NULL)
					return;
				transflag++;
			}
			fprintf(dout, MSGSTR( FTPD_DIRNAME, "%s\n"), dirname);
			byte_count += strlen(dirname) + 1;
			continue;
		} else if ((st.st_mode&S_IFMT) != S_IFDIR)
			continue;

		if ((dirp = opendir(dirname)) == NULL)
			continue;

		while ((dir = readdir(dirp)) != NULL) {
			char nbuf[MAXPATHLEN];

			if (dir->d_name[0] == '.' && dir->d_namlen == 1)
				continue;
			if (dir->d_name[0] == '.' && dir->d_name[1] == '.' &&
			    dir->d_namlen == 2)
				continue;

			sprintf(nbuf, "%s/%s", dirname, dir->d_name);

			/*
			 * We have to do a stat to insure it's
			 * not a directory or special file.
			 */
			if (simple || (stat(nbuf, &st) == 0) &&
			    (st.st_mode&S_IFMT) == S_IFREG) {
				if (dout == NULL) {
					dout = dataconn("file list", (off_t)-1,
						"w");
					if (dout == NULL)
						return;
					transflag++;
				}
				if (nbuf[0] == '.' && nbuf[1] == '/')
					fprintf(dout, MSGSTR( FTPD_FMT7,
					"%s%s\n"), &nbuf[2],
                                                type == TYPE_A ? "\r" : "");
				else
					fprintf(dout, MSGSTR( FTPD_FMT7,
					 "%s%s\n"), nbuf,
                                                type == TYPE_A ? "\r" : "");
				byte_count += strlen(nbuf) + 1;
			}
		}
		(void) closedir(dirp);
	}

	if (dout == NULL)
		reply(550, "No files found.");
	else if (ferror(dout) != 0)
		perror_reply(550, MSGSTR(DATACONNERR, "Data connection"));
	else
		reply(226, MSGSTR(TRANSOK, "Transfer complete."));

	transflag = 0;
	if (dout != NULL)
		(void) fclose(dout);
	data = -1;
	pdata = -1;
}

#ifdef SETPROCTITLE
/*
 * clobber argv so ps will show what we're doing.
 * (stolen from sendmail)
 * warning, since this is usually started from inetd.conf, it
 * often doesn't have much of an environment or arglist to overwrite.
 */

/*VARARGS1*/
#ifdef VA_ARGV_IS_RECAST
setproctitle(char *fmt, ...)
{
        va_list ap;
        register char *p, *bp, ch;
        register int i;
        char buf[BUFSIZ];

#ifdef __STDC__
        va_start(ap, fmt);
#else
        va_start(ap);
#endif /* __STDC__ */
	(void) vsprintf(buf, fmt, ap);
        va_end(ap);
	/* make ps print our process name */
	p = Argv[0];
	*p++ = '-';

	i = strlen(buf);
	if (i > LastArgv - p - 2) {
		i = LastArgv - p - 2;
		buf[i] = '\0';
	}
	bp = buf;
	while (ch = *bp++)
		if (ch != '\n' && ch != '\r')
			*p++ = ch;
	while (p < LastArgv)
		*p++ = ' ';
}
#else
setproctitle(fmt, a, b, c)
char *fmt;
long a, b, c;
{
	register char *p, *bp, ch;
	register int i;
	char buf[BUFSIZ];

	(void) sprintf(buf, fmt, a, b, c);

	/* make ps print our process name */
	p = Argv[0];
	*p++ = '-';

	i = strlen(buf);
	if (i > LastArgv - p - 2) {
		i = LastArgv - p - 2;
		buf[i] = '\0';
	}
	bp = buf;
	while (ch = *bp++)
		if (ch != '\n' && ch != '\r')
			*p++ = ch;
	while (p < LastArgv)
		*p++ = ' ';
}
#endif
#endif /* SETPROCTITLE */

/*
 * To seperate ftp anonymous account logging with regular logging.
 * ftp anonymous account uses old style syslog which sends log message
 * to syslogd through INET socket and is unaffected by chroot.
 */  
local_syslog(int pri, const char *fmt, va_list arg)
{
	if (guest)
		old_syslog(pri, fmt, arg);
	else
		syslog(pri, fmt, arg);
}
