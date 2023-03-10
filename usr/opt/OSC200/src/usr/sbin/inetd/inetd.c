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
static char	*sccsid = "@(#)$RCSfile: inetd.c,v $ $Revision: 4.2.6.10 $ (DEC) $Date: 1993/12/10 17:29:14 $";
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
 * Copyright (c) 1983 Regents of the University of California.
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
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef lint
char copyright[] =
"@(#) Copyright (c) 1983 Regents of the University of California.\n\
 All rights reserved.\n";
#endif /* not lint */

/*
 * inetd.c
 *
 *	Revision History:
 *
 * 16-Apr-91    Mary Walker
 *      fix silent core dump when empty line in inetd.conf
 */

/*
 * Inetd - Internet super-server
 *
 * This program invokes all internet services as needed.
 * connection-oriented services are invoked each time a
 * connection is made, by creating a process.  This process
 * is passed the connection as file descriptor 0 and is
 * expected to do a getpeername to find out the source host
 * and port.
 *
 * Datagram oriented services are invoked when a datagram
 * arrives; a process is created and passed a pending message
 * on file descriptor 0.  Datagram servers may either connect
 * to their peer, freeing up the original socket for inetd
 * to receive further messages on, or ``take over the socket'',
 * processing all arriving datagrams and, eventually, timing
 * out.	 The first type of server is said to be ``multi-threaded'';
 * the second type of server ``single-threaded''. 
 *
 * Inetd uses a configuration file which is read at startup
 * and, possibly, at some later time in response to a hangup signal.
 * The configuration file is ``free format'' with fields given in the
 * order shown below.  Continuation lines for an entry must being with
 * a space or tab.  All fields must be present in each entry.
 *
 *	service name			must be in /etc/services
 *	socket type			stream/dgram/raw/rdm/seqpacket
 *	protocol			must be in /etc/protocols
 *	wait/nowait			single-threaded/multi-threaded
 *	user				user to run daemon as
 *	server program			full path name
 *	server program arguments	maximum of MAXARGS (20)
 *
 * for rpc services:
 *	service name/version		must be in /etc/rpc with version = 1-x
 *	socket type			stream/dgram/raw/rdm/seqpacket
 *	protocol			of the form rpc/protocol
 *	wait/nowait			single-threaded/multi-threaded
 *	user				user to run daemon as
 *	server program			full path name
 *	server program arguments	maximum of MAXARGS (5)
 *
 * Comment lines are indicated by a `#' in column 1.
 */
#include <ctype.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/file.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/resource.h>

#include <netinet/in.h>
#include <arpa/inet.h>

#include <errno.h>
#include <stdio.h>
#include <signal.h>
#include <netdb.h>
#include <sys/syslog.h>
#include <pwd.h>
#include <paths.h>
#include <rpc/netdb.h>

#include <nl_types.h>
#include <locale.h>
#include "inetd_msg.h"
#define MSGSTR(Num,Str) catgets(catd,MS_INETD,Num,Str)
nl_catd catd;

#define	TOOMANY		500		/* don't start more than TOOMANY */
#define	CNT_INTVL	60		/* servers in CNT_INTVL sec. */
#define	RETRYTIME	(60*10)		/* retry after bind or server fail */

#ifdef 0
#define	SIGBLOCK	(sigmask(SIGCHLD)|sigmask(SIGHUP)|sigmask(SIGALRM))
#else /* alpha */
#define	SIGBLOCK	((int)(sigmask(SIGCHLD)|sigmask(SIGHUP)|sigmask(SIGALRM)))
#endif /* alpha */

#define PIDFILE         "/var/run/inetd.pid"

extern	int errno;

void	config(), reapchild(), retry();
void	termserv();
void	unregister();

char	*index();
char	*malloc();
char	*strdup();

int	debug = 0;
int	ndescriptors;
int	nsock, maxsock;
fd_set	allsock;
int	options;
int	timingout;
int	nservers = 0;
int	maxservers;
struct	servent *sp;

/*
 * Each service requires one file descriptor for the socket we listen on
 * for requests for that service.  We don't allow more services than
 * we can allocate file descriptors for.  OTHERDESCRIPTORS is the number
 * of descriptors needed for other purposes; this number was determined
 * experimentally.
 */
#define	OTHERDESCRIPTORS	8

/*
 * Additional information carried in various fields:
 *
 * If "se_wait" is neither 0 nor 1, it's the PID of the server process
 * currently running for that service.
 * If "se_fd" is -1, it means that the service could not be set up,
 * e.g. the entry for that service couldn't be found in "/etc/services".
 */
struct	servtab {
	char	*se_service;		/* name of service */
	int	se_socktype;		/* type of socket to use */
	char	*se_proto;		/* protocol used */
	char	se_isrpc;		/* service is RPC-based */
	short	se_checked;		/* looked at during merge */
	short	se_wait;		/* single threaded server */
	char	*se_user;		/* user name to run as */
	struct	biltin *se_bi;		/* if built-in, description */
	char	*se_server;		/* server program */
#define MAXARGV 20
	char	*se_argv[MAXARGV+1];	/* program arguments */
	int	se_fd;			/* open descriptor */
	union {
		struct	sockaddr_in ctrladdr;	/* bound address */
		struct {
			unsigned prog;	/* program number */
			unsigned lowvers;	/* lowest version supported */
			unsigned highvers;	/* highest version supported */
		} rpcnum;
	} se_un;
	int	se_count;		/* number started since se_time */
	struct	timeval se_time;	/* start of se_count */
	struct	servtab *se_next;
} *servtab;

#define se_ctrladdr se_un.ctrladdr
#define se_rpc se_un.rpcnum

int echo_stream(), discard_stream(), machtime_stream();
int daytime_stream(), chargen_stream();
int echo_dg(), discard_dg(), machtime_dg(), daytime_dg(), chargen_dg();

struct biltin {
	char	*bi_service;		/* internally provided service name */
	int	bi_socktype;		/* type of socket supported */
	short	bi_fork;		/* 1 if should fork before call */
	short	bi_wait;		/* 1 if should wait for child */
	int	(*bi_fn)();		/* function which performs it */
} biltins[] = {
	/* Echo received data */
	"echo",		SOCK_STREAM,	1, 0,	echo_stream,
	"echo",		SOCK_DGRAM,	0, 0,	echo_dg,

	/* Internet /dev/null */
	"discard",	SOCK_STREAM,	1, 0,	discard_stream,
	"discard",	SOCK_DGRAM,	0, 0,	discard_dg,

	/* Return 32 bit time since 1970 */
	"time",		SOCK_STREAM,	0, 0,	machtime_stream,
	"time",		SOCK_DGRAM,	0, 0,	machtime_dg,

	/* Return human-readable time */
	"daytime",	SOCK_STREAM,	0, 0,	daytime_stream,
	"daytime",	SOCK_DGRAM,	0, 0,	daytime_dg,

	/* Familiar character generator */
	"chargen",	SOCK_STREAM,	1, 0,	chargen_stream,
	"chargen",	SOCK_DGRAM,	0, 0,	chargen_dg,
	0
};

#define NUMINT	(sizeof(intab) / sizeof(struct inent))
char	*CONFIG = "/etc/inetd.conf";
char	**Argv;
char 	*LastArg;

main(argc, argv, envp)
	int argc;
	char *argv[], *envp[];
{
	extern char *optarg;
        extern int optind;
        extern uid_t geteuid();
	register struct servtab *sep;
        register struct passwd *pwd;
        register int tmpint;
        struct sigvec sv;
        int ch, dofork;
        pid_t pid, i_pid;
        char buf[50];
	register int i;
	FILE *fp;


	setlocale(LC_ALL, "");
        catd = NLcatopen(MF_INETD, NL_CAT_LOCALE);
	Argv = argv; 
	if (envp == 0 || *envp == 0)
		envp = argv;
	while (*envp)
		envp++;
	LastArg = envp[-1] + strlen(envp[-1]);

	if (geteuid()) {
                fprintf(stderr, MSGSTR(NO_ROOT, "NOT super-user\n"));
		exit(1);
	}

	while ((ch = getopt(argc, argv, "d")) != EOF)
                switch(ch) {
                case 'd':
                        debug = 1;
                        options |= SO_DEBUG;
                        break;
                case '?':
                default:
			fprintf(stderr, MSGSTR(USAGE, "usage: inetd [-d]"));
                        exit(1);
		}
	argc -= optind;
        argv += optind;
			
	if (argc > 0)
		CONFIG = argv[0];
	if (debug == 0) 
		daemon(0,0);

	i_pid = getpid();
        fp = fopen(PIDFILE,"w");
	if (fp != NULL) {
	   fprintf(fp,"%d\n",i_pid);
	   (void) fclose(fp);
	}
	ndescriptors = getdtablesize();
	maxservers = ndescriptors - OTHERDESCRIPTORS;
	openlog("inetd", LOG_PID | LOG_NOWAIT|LOG_CONS, LOG_ERR|LOG_DAEMON);
	bzero((char *)&sv, sizeof(sv));
	sv.sv_mask = SIGBLOCK;
	sv.sv_handler = retry;
	sigvec(SIGALRM, &sv, (struct sigvec *)0);
	config();
	sv.sv_handler = config;
	sigvec(SIGHUP, &sv, (struct sigvec *)0);
	sv.sv_handler = reapchild;
	sigvec(SIGCHLD, &sv, (struct sigvec *)0);

	for (;;) {
	    int ctrl, n;
	    fd_set readable;
	    struct sockaddr_in his_addr;
	    int hisaddrlen = sizeof (his_addr);

	    if (nsock == 0) {
		(void) sigblock(SIGBLOCK);
		while (nsock == 0)
		    sigpause(0L);
		(void) sigsetmask(0L);
	    }
	    readable = allsock;
	    if ((n = select(maxsock + 1, (fd_set *)&readable, (fd_set *)0,
		(fd_set *)0, (struct timeval *)0)) <= 0) {
		    if (n < 0 && errno != EINTR)
			syslog(LOG_WARNING, MSGSTR(SELECTWRN, 
				"select: %m\n")); /*MSG*/
		    continue;
	    }
	    for (sep = servtab; n && sep; sep = sep->se_next){
		if (sep->se_fd == -1)
		   continue;
	    if (FD_ISSET(sep->se_fd, &readable)) {
		n--;
		if (debug)
			syslog(LOG_DEBUG, MSGSTR(NEEDSERV, 
				"someone wants %s fd %d \n"), sep->se_service, 
		sep->se_fd ); /*MSG*/
		if (!sep->se_wait && sep->se_socktype == SOCK_STREAM) {
			ctrl = accept(sep->se_fd, 
				(struct sockaddr *)&his_addr, 
				(int *)&hisaddrlen);
			if (debug)
				syslog(LOG_DEBUG, MSGSTR(ACCEPT, 
					"accept, ctrl %d fd %d\n"), ctrl, sep->se_fd); /*MSG*/
			if (ctrl < 0) {
				if (errno == EINTR)
					continue;
				syslog(LOG_WARNING, MSGSTR(ACCEPTWRN, 
					"accept: %m")); /*MSG*/
				continue;
			}
		} else
			ctrl = sep->se_fd;
		(void) sigblock(SIGBLOCK);
		pid = 0;
		dofork = (sep->se_bi == 0 || sep->se_bi->bi_fork);
		if (dofork) {
			if (sep->se_count++ == 0)
			    (void)gettimeofday(&sep->se_time,
			        (struct timezone *)0);
			else if (sep->se_count >= TOOMANY) {
				struct timeval now;

				(void)gettimeofday(&now, (struct timezone *)0);
				if ((now.tv_sec - sep->se_time.tv_sec >
				    CNT_INTVL) ||
				    (now.tv_sec <= sep->se_time.tv_sec))
				{
					sep->se_time = now;
					sep->se_count = 1;
				} else {
					syslog(LOG_ERR, MSGSTR( SERVFAIL, 
			"%s/%s server failing (looping), service terminated\n"),
					    sep->se_service, sep->se_proto);
					termserv(sep);
					sep->se_count = 0;
					(void) sigsetmask(0L);
					if (!timingout) {
						timingout = 1;
						alarm(RETRYTIME);
					}
					continue;
				}
			}
			pid = fork();
		}
		if (pid < 0) {
			if (!sep->se_wait && sep->se_socktype == SOCK_STREAM)
				(void) close(ctrl);
			(void) sigsetmask(0L);
			sleep(1);
			continue;
		}
		if (pid && sep->se_wait) {
			sep->se_wait = pid;
			FD_CLR(sep->se_fd, &allsock);
			nsock--;
		}
		(void) sigsetmask(0L);
		if (pid == 0) {
			char addrbuf[32];

			sprintf(addrbuf,"%x.%d",ntohl(his_addr.sin_addr.s_addr),
					ntohs(his_addr.sin_port));
                        if (dofork) {
				setsid();
                                for (i = ndescriptors; --i > 2; )
                                        if (i != ctrl)
                                                close(i);
			}
			if (sep->se_bi)
				(*sep->se_bi->bi_fn)(ctrl, sep);
			else {
				(void) dup2(ctrl, 0);
				(void) close(ctrl);
				(void) dup2(0, 1);
				(void) dup2(0, 2);

				if ((pwd = getpwnam(sep->se_user)) == NULL) {
					syslog(LOG_ERR, MSGSTR(PWNAM, 
						"getpwnam: %s: No such user"), 
						sep->se_user);
					if (sep->se_socktype != SOCK_STREAM)
						recv(0, buf, sizeof (buf), 0);
					_exit(1);
				}
				if (pwd->pw_uid) {
					if (setgid((gid_t)pwd->pw_gid) == -1) {
						syslog(LOG_ERR, MSGSTR(SETGID,
						       "setgid(%d): %m"), 
						       pwd->pw_gid); 
						if (sep->se_socktype != 
						    SOCK_STREAM)
							recv(0, buf, 
							     sizeof (buf), 0);
						_exit(1);
					}
					initgroups(pwd->pw_name, pwd->pw_gid);
					if (setuid((uid_t)pwd->pw_uid) == -1) {
						syslog(LOG_ERR, MSGSTR(SETUID,
						       "setuid(%d): %m"), 
						       pwd->pw_uid); 
						if (sep->se_socktype != 
						    SOCK_STREAM)
							recv(0, buf, 
							     sizeof (buf), 0);
						_exit(1);
					}
				}
				if (debug)
					syslog(LOG_DEBUG, MSGSTR(EXECL, 
						"%d execl %s\n"), /*MSG*/
					    getpid(), sep->se_server);
				if (sep->se_argv[0] != NULL) {
				    if (!strcmp(sep->se_argv[0], "%A"))
					execl(sep->se_server,
						rindex(sep->se_server, '/')+1,
						sep->se_socktype == SOCK_DGRAM
						? (char *)0 : addrbuf, (char *)0);
				    else
					execv(sep->se_server, sep->se_argv);
			        } else
					execv(sep->se_server, sep->se_argv);
				if (sep->se_socktype != SOCK_STREAM)
					(void) recv(0, buf, sizeof (buf), 0);
				syslog(LOG_ERR, MSGSTR(EXECV, 
					"execv %s: %m"), sep->se_server); /*MSG*/
				_exit(1);
			}
		}
		if (!sep->se_wait && sep->se_socktype == SOCK_STREAM)
			(void) close(ctrl);
	    }
	  }
	}
}

void
reapchild()
{
	union wait status;
	pid_t pid;
	register struct servtab *sep;

	for (;;) {
		pid = wait3(&status, WNOHANG, (struct rusage *)0);
		if (pid <= 0)
			break;
		if (debug)
			syslog(LOG_DEBUG, MSGSTR(REAPED,
				"%d reaped\n"), pid); 
		for (sep = servtab; sep; sep = sep->se_next)
			if (sep->se_wait == pid) {
				if (status.w_status)
					syslog(LOG_WARNING, MSGSTR(EXITSTAT, 
						"%s: exit status 0x%x"), /*MSG*/
					    sep->se_server, status);
				if (debug)
					syslog(LOG_DEBUG, MSGSTR(RESTORED, 
						"restored %s, fd %d\n"), /*MSG*/
					    sep->se_service, sep->se_fd);
				FD_SET(sep->se_fd, &allsock);
				nsock++;
				sep->se_wait = 1;
			}
	}
}

void
config()
{
	register struct servtab  *sep, *cp, **sepp;
	struct servtab *getconfigent(), *enter();
#ifdef 0
	long omask;
#else /* alpha */
	int omask;
#endif /* alpha */

	if (!setconfig()) {
		syslog(LOG_ERR, MSGSTR( CONFMSG, "%s: %m"), CONFIG); /*MSG*/
		return;
	}
	for (sep = servtab; sep; sep = sep->se_next)
		sep->se_checked = 0;
	while (cp = getconfigent()) {
		for (sep = servtab; sep; sep = sep->se_next) {
			if (sep->se_isrpc) {
				if (cp->se_isrpc
				    && sep->se_rpc.prog == cp->se_rpc.prog
				    && strcmp(sep->se_proto, cp->se_proto) == 0)
					break;
			} else {
				if (!cp->se_isrpc
				    && strcmp(sep->se_service, cp->se_service) == 0
				    && strcmp(sep->se_proto, cp->se_proto) == 0)
					break;
			}
		}
		if (sep != 0) {
			int i;

			omask = sigblock(SIGBLOCK);
			/*
                         * sep->se_wait may be holding the pid of a daemon
                         * that we're waiting for.  If so, don't overwrite
                         * it unless the config file explicitly says don't
                         * wait.
                         */
			if (cp->se_bi == 0 &&
			    (sep->se_wait == 1 || cp->se_wait == 0))
				sep->se_wait = cp->se_wait;
#define SWAP(a, b) { char *c = a; a = b; b = c; }
			if (cp->se_user)
				SWAP(sep->se_user, cp->se_user);
			if (cp->se_server)
				SWAP(sep->se_server, cp->se_server);
			for (i = 0; i < MAXARGV; i++)
				SWAP(sep->se_argv[i], cp->se_argv[i]);
			(void) sigsetmask(omask);
			freeconfig(cp);
			if (debug)
				print_service( MSGSTR(REDO, "REDO"), sep); /*MSG*/
		} else {
			sep = enter(cp);
			if (debug)
				print_service( MSGSTR(ADD, "ADD "), sep); /*MSG*/
		}
		sep->se_checked = 1;
		if (sep->se_isrpc) {
			if (sep->se_fd != -1)
				termserv(sep);
			else
				unregister(sep);	/* just in case */
			if (nservers >= maxservers) {
				syslog(LOG_ERR, MSGSTR(TOOMNYSVC,
				    "%s/%s: too many services (max %d)"),
				    sep->se_service, sep->se_proto,
				    maxservers);	
				sep->se_fd = -1;
				continue;
			}
			setuprpc(sep);
		} else {
			sp = getservbyname(sep->se_service, sep->se_proto);
			if (sp == NULL) {
				syslog(LOG_ERR, MSGSTR(UNKSERV, 
					"%s/%s: unknown service"), /*MSG*/
			    		sep->se_service, sep->se_proto);
				if (sep->se_fd != -1)
					termserv(sep);
				sep->se_checked = 0;
				continue;
			}
			if (sp->s_port != sep->se_ctrladdr.sin_port) {
				sep->se_ctrladdr.sin_port = sp->s_port;
				if (sep->se_fd != -1)
					termserv(sep);
			}
			if (sep->se_fd == -1) {
				if (nservers >= maxservers) {
					syslog(LOG_ERR, MSGSTR(TOOMNYSVC,
				    	   "%s/%s: too many services (max %d)"),
				    	   sep->se_service, sep->se_proto,
				    	   maxservers);	
					sep->se_fd = -1;
					continue;
				}
				setup(sep);
			}
		}
	}
	endconfig();
	/*
	 * Purge anything not looked at above.
	 * XXX - if we add a service and delete one, the new service
	 * will be added to the count of services before the deleted one
	 * is subtracted.  This means you may run out of services if this
	 * happens; however, we can't do much about that, since we get
	 * the socket for the new one before we close the one for the
	 * deleted service, and if we allowed that extra service we might
	 * run out of descriptors.
	 */
	omask = sigblock(SIGBLOCK);
	sepp = &servtab;
	while (sep = *sepp) {
		if (sep->se_checked) {
			sepp = &sep->se_next;
			continue;
		}
		*sepp = sep->se_next;
		if (sep->se_fd != -1) 
			termserv(sep);
		if (debug)
			print_service( MSGSTR( FREE, "FREE"), sep); 
		freeconfig(sep);
		free((void *)sep);
	}
	(void) sigsetmask(omask);
}

/*
 * Try again to establish sockets on which to listen for requests
 * for non-RPC-based services (if the attempt failed before, it was either
 * because a socket could not be created, or more likely because the socket
 * could not be bound to the service's address - probably because there was
 * already a daemon out there with a socket bound to that address).
 */
void
retry()
{
	register struct servtab *sep;

	timingout = 0;
	for (sep = servtab; sep; sep = sep->se_next) {
		if (sep->se_fd == -1 && !sep->se_isrpc) {
			if (nservers < maxservers)
				setup(sep);
		}
	}
}

/*
 * Set up to accept requests for a non-RPC-based service.  Create a socket
 * to listen for connections or datagrams, bind it to the address of that
 * service, add its socket descriptor to the list of descriptors to poll,
 * and increment the number of socket descriptors and active services.
 */
setup(sep)
	register struct servtab *sep;
{
	int on = 1;

	if ((sep->se_fd = socket(AF_INET, sep->se_socktype, 0)) < 0) {
		syslog(LOG_ERR, MSGSTR(SOCKERR, "%s/%s: socket: %m"), /*MSG*/
		    sep->se_service, sep->se_proto);
		return;
	}
#define	turnon(fd, opt) \
setsockopt(fd, SOL_SOCKET, opt, (char *)&on, sizeof (on))
	if (strcmp(sep->se_proto, "tcp") == 0 && (options & SO_DEBUG) &&
	    turnon(sep->se_fd, SO_DEBUG) < 0)
		syslog(LOG_ERR, MSGSTR(SETSOCK, 
			"setsockopt (SO_DEBUG): %m")); /*MSG*/
	if (turnon(sep->se_fd, SO_REUSEADDR) < 0)
		syslog(LOG_ERR, MSGSTR(SETSOCKOPT,
			"setsockopt (SO_REUSEADDR): %m")); /*MSG*/
#undef turnon
	if (bind(sep->se_fd, &sep->se_ctrladdr,
	    sizeof (sep->se_ctrladdr)) < 0) {
		syslog(LOG_ERR, MSGSTR(BIND, "%s/%s: bind: %m"), /*MSG*/
		    sep->se_service, sep->se_proto);
		(void) close(sep->se_fd);
		sep->se_fd = -1;
		if (!timingout) {
			timingout = 1;
			(void) alarm(RETRYTIME);
		}
		return;
	}
	if (sep->se_socktype == SOCK_STREAM)
		(void) listen(sep->se_fd, 10);
	FD_SET(sep->se_fd, &allsock);
	nsock++;
	if (sep->se_fd > maxsock)
		maxsock = sep->se_fd;
	nservers++;
}

/*
 * Set up to accept requests for an RPC-based service.  Create a socket
 * to listen for connections or datagrams, bind it to a system-chosen
 * address, register it with the portmapper under that address, add its
 * socket descriptor to the list of descriptors to poll, and increment
 * the number of socket descriptors and active services.
 */
setuprpc(sep)
	register struct servtab *sep;
{
	register int i;
	struct sockaddr_in addr;
	int len = sizeof(struct sockaddr_in);
	
	if ((sep->se_fd = socket(AF_INET, sep->se_socktype, 0)) < 0) {
		syslog(LOG_ERR, MSGSTR(SOCKERR, "%s/%s: socket: %m"), /*MSG*/
		    sep->se_service, sep->se_proto);
		return;
	}
	addr.sin_family = AF_INET;
	addr.sin_port = 0;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	if (bind(sep->se_fd, &addr, sizeof(addr)) < 0) {
		syslog(LOG_ERR, MSGSTR(BIND, "%s/%s: bind: %m"), /*MSG*/
		    sep->se_service, sep->se_proto);
		(void) close(sep->se_fd);
		sep->se_fd = -1;
		return;
	}
	if (getsockname(sep->se_fd, &addr, &len) != 0) {
		syslog(LOG_ERR, MSGSTR(GETSOCKNAME, "%s/%s: getsockname: %m"),
		    sep->se_service, sep->se_proto); /* MSG */
		(void) close(sep->se_fd);
		sep->se_fd = -1;
		return;
	}
	for (i = sep->se_rpc.lowvers; i <= sep->se_rpc.highvers; i++) {
                pmap_set(sep->se_rpc.prog, i, (sep->se_socktype == SOCK_DGRAM) ?
		    IPPROTO_UDP : IPPROTO_TCP, ntohs(addr.sin_port));
	}
	if (sep->se_socktype == SOCK_STREAM)
		listen(sep->se_fd, 10);
	FD_SET(sep->se_fd, &allsock);
	nsock++;
	if (sep->se_fd > maxsock)
		maxsock = sep->se_fd;
	nservers++;
}

/*
 * Shut down a service.  Unregister it if it's an RPC service, remove
 * its socket descriptor from the list of descriptors to poll, close
 * that socket descriptor, set it to -1 (to indicate that it's shut down),
 * and decrement the number of socket descriptors and active services.
 */
void
termserv(sep)
	register struct servtab *sep;
{
	if (sep->se_isrpc) {
		/*
		 * Have to do this here because there might be multiple
		 * registers of the same prognum.
		 */
		unregister(sep);
	}
	FD_CLR(sep->se_fd, &allsock);
	nsock--;
	(void) close(sep->se_fd);
	sep->se_fd = -1;
	nservers--;
}

/*
 * Unregister an RPC service.
 */
void
unregister(sep)
	register struct servtab *sep;
{
	register int i;
	register struct servtab *s;
	unsigned prog;

	/*
	 * Unregister the service only if there were no rpc programnum
	 * earlier, because then you would pmap_unset that too!
	 * Added to support multiple transports for the same prognum.
	 */
	prog = sep->se_rpc.prog;
	for (s = servtab; s; s = s->se_next) {
		if (s == sep)	/* Ignore this one */
			continue;
		if ((s->se_checked == 0) || !s->se_isrpc ||
			(prog != s->se_rpc.prog))
			continue;
		/* Found an existing entry for that prog number */
		return;
	}
	for (i = sep->se_rpc.lowvers; i <= sep->se_rpc.highvers; i++)
		pmap_unset(sep->se_rpc.prog, i);
}

struct servtab *
enter(cp)
	struct servtab *cp;
{
	register struct servtab *sep;
#ifdef 0
	long omask;
#else /* alpha */
	int omask;
#endif /* alpha */

	sep = (struct servtab *)malloc(sizeof (struct servtab) + 1);
	if (sep == (struct servtab *)0L) {
		syslog(LOG_ERR, MSGSTR(MEMERR, "Out of memory.")); 
		exit(-1);
	}
	*sep = *cp;
	sep->se_fd = -1;
	omask = sigblock(SIGBLOCK);
	sep->se_next = servtab;
	servtab = sep;
	(void) sigsetmask(omask);
	return (sep);
}
FILE	*fconfig = NULL;
struct	servtab serv;
char	line[256];
char	*skip(), *nextline();

setconfig()
{

	if (fconfig != NULL) {
		(void) fseek(fconfig, 0L, L_SET);
		return (1);
	}
	fconfig = fopen(CONFIG, "r");
	return (fconfig != NULL);
}

endconfig()
{

	if (fconfig) {
		(void) fclose(fconfig);
		fconfig = NULL;
	}
}

struct servtab *
getconfigent()
{
	register struct servtab *sep = &serv;
	char *cp, *arg, *tp, *strp;
	int argc;
	struct rpcent *rpc;

more:
	bzero((char *) sep, sizeof(struct servtab));
	/* skip over comment lines and blank lines */
	while ((cp = nextline(fconfig)) && (*cp == '#' || *cp == '\0'))
		;
	if (cp == NULL)
		return ((struct servtab *)0);
	sep->se_service = strdup(skip(&cp));
	arg = skip(&cp);
	if (strcmp(arg, "stream") == 0)
		sep->se_socktype = SOCK_STREAM;
	else if (strcmp(arg, "dgram") == 0)
		sep->se_socktype = SOCK_DGRAM;
	else if (strcmp(arg, "rdm") == 0)
		sep->se_socktype = SOCK_RDM;
	else if (strcmp(arg, "seqpacket") == 0)
		sep->se_socktype = SOCK_SEQPACKET;
	else if (strcmp(arg, "raw") == 0)
		sep->se_socktype = SOCK_RAW;
	else
		sep->se_socktype = -1;
	sep->se_proto = strdup(skip(&cp));
	if (strncmp(sep->se_proto, "rpc/", 4) == 0) {
		sep->se_isrpc = 1;
		sep->se_rpc.lowvers = 0;
		sep->se_rpc.highvers = 0;
		tp = sep->se_service;
		while (*tp != '/') {
			if (*tp == '\0') {
				sep->se_rpc.lowvers = 1;
				sep->se_rpc.highvers = 1;
				break;
			} else
				tp++;
		}
		*tp = '\0';
		if ((rpc = getrpcbyname(sep->se_service)) != NULL)
			sep->se_rpc.prog = rpc->r_number;
		else {
			strp = sep->se_service;
			sep->se_rpc.prog = strtol(strp, &strp, 10);
			if (strp != tp) {
				/*
				 * Service name isn't all-numeric, so
				 * since we didn't find it it must not
				 * be known.
				 */
				syslog(LOG_ERR, MSGSTR(UNKSERV, 
					"%s/%s: unknown service"), /*MSG*/
			    		sep->se_service, sep->se_proto);
				freeconfig(sep);
				goto more;
			}
		}
		if (sep->se_rpc.lowvers == 0) {
			/*
			 * The service name ended with a slash, so the
			 * version number(s) are explicitly specified.
			 */
			tp++;
			strp = tp;
			sep->se_rpc.lowvers = strtol(tp, &strp, 10);
			tp = strp;
			if (*tp == '-') {
				tp++;
				strp = tp;
				sep->se_rpc.highvers = strtol(tp, &strp, 10);
				if (*strp != '\0') {
					syslog(LOG_ERR, MSGSTR(BADHIVERS,/*MSG*/
					    "%s/%s: bad high version number"),
					    sep->se_service, sep->se_proto); 
					freeconfig(sep);
					goto more;
				}
			} else if (*tp == '\0')
				sep->se_rpc.highvers = sep->se_rpc.lowvers;
			else {
				syslog(LOG_ERR, MSGSTR(BADVERS, /* MSG */
				    "%s/%s: bad version number"),
				    sep->se_service, sep->se_proto); 
				freeconfig(sep);
				goto more;
			}
		}
	} else
		sep->se_isrpc = 0;

	arg = skip(&cp);
	sep->se_wait = strcmp(arg, "wait") == 0;
	sep->se_user = strdup(skip(&cp));
	sep->se_server = strdup(skip(&cp));
	if (strcmp(sep->se_server, "internal") == 0) {
		register struct biltin *bi;

		for (bi = biltins; bi->bi_service; bi++)
			if (bi->bi_socktype == sep->se_socktype &&
			    strcmp(bi->bi_service, sep->se_service) == 0)
				break;
		if (bi->bi_service == 0) {
			 syslog(LOG_ERR, MSGSTR(UNKINTSERV, 
				"internal service %s unknown\n"), /*MSG*/
				sep->se_service);
			freeconfig(sep);
			goto more;
		}
		sep->se_bi = bi;
		sep->se_wait = bi->bi_wait;
	} else
		sep->se_bi = NULL;
	argc = 0;
	for (arg = skip(&cp); cp; arg = skip(&cp))
		if (argc < MAXARGV)
			sep->se_argv[argc++] = strdup(arg);
	while (argc <= MAXARGV)
		sep->se_argv[argc++] = NULL;
	return (sep);
}

freeconfig(cp)
	register struct servtab *cp;
{
	int i;

	if (cp->se_service)
		free(cp->se_service);
	if (cp->se_proto)
		free(cp->se_proto);
	if (cp->se_user)
		free(cp->se_user);
	if (cp->se_server)
		free(cp->se_server);
	for (i = 0; i < MAXARGV; i++)
		if (cp->se_argv[i])
			free(cp->se_argv[i]);
}

char *
skip(cpp)
	char **cpp;
{
	register char *cp = *cpp;
	char *start;
	static char nullstring[1] = "";
	
	if (cp == (char *) 0L)
		return(nullstring);
again:
	while (*cp == ' ' || *cp == '\t')
		cp++;
	if (*cp == '\0' || *cp == '\\') {
		int i;
		if ((i = getc(fconfig)) != EOF) {
			char c = (char) i;
			ungetc(c, fconfig);
			/*
			 * A config line can be continued on subsequent
			 * indented lines.
			 */
			if (c == ' ' || c == '\t')
				if (cp = nextline(fconfig))
					goto again;
		}
		/*
		 * Signal end of line or file.  We can't just return the
		 * null pointer, though.  If the end occured in the middle
		 * of parsing a config line we have to continue returning valid
		 * string pointers for the benefit of code that might
		 * do strdup()'s or strcmp()'s against them.
		 */
		*cpp = (char *)0L;
		return(nullstring);
	}
	start = cp;
	while (*cp && *cp != ' ' && *cp != '\t')
		cp++;
	if (*cp != '\0')
		*cp++ = '\0';
	*cpp = cp;
	return (start);
}

char *
nextline(fd)
	FILE *fd;
{
	char *cp;

	if (fgets(line, sizeof (line), fd) == NULL)
		return ((char *)0);
	cp = index(line, '\n');
	if (cp)
		*cp = '\0';
	return (line);
}

char *
strdup(cp)
	char *cp;
{
	char *new;

	if (cp == NULL)
		cp = "";
	new = (char *)malloc((unsigned)(strlen(cp) + 1));
	if (new == (char *)0L) {
		syslog(LOG_ERR, MSGSTR(MEMERR, "Out of memory.")); /*MSG*/
		exit(-1);
	}
	(void)strcpy(new, cp);
	return (new);
}

setproctitle(a, s)
	char *a;
	int s;
{
	int size;
	register char *cp;
	struct sockaddr_in sin;
	char buf[80];

	cp = Argv[0];
	size = sizeof(sin);
	if (getpeername(s, &sin, &size) == 0)
		(void) sprintf(buf, "-%s [%s]", a, inet_ntoa(sin.sin_addr)); 
	else
		(void) sprintf(buf, "-%s", a); 
	(void) strncpy(cp, buf, LastArg - cp);
	cp += strlen(cp);
	while (cp < LastArg)
		*cp++ = ' ';
}

/*
 * Internet services provided internally by inetd:
 */

#define BUFSIZE	4096

/* ARGSUSED */
echo_stream(s, sep)		/* Echo service -- echo data back */
	int s;
	struct servtab *sep;
{
	char buffer[BUFSIZE];
	int i;

	setproctitle(sep->se_service, s);
	while ((i = read(s, buffer, sizeof(buffer))) > 0 &&
	    write(s, buffer, i) > 0)
		;
	exit(0);
}

/* ARGSUSED */
echo_dg(s, sep)			/* Echo service -- echo data back */
	int s;
	struct servtab *sep;
{
	char buffer[BUFSIZE];
	int i, size;
	struct sockaddr sa;

	size = sizeof(sa);
	if ((i = recvfrom(s, buffer, sizeof(buffer), 0, &sa, &size)) < 0)
		return;
	(void) sendto(s, buffer, i, 0, &sa, sizeof(sa));
}

/* ARGSUSED */
discard_stream(s, sep)		/* Discard service -- ignore data */
	int s;
	struct servtab *sep;
{
	char buffer[BUFSIZE];

	setproctitle(sep->se_service, s);
	while (1) {
		while (read(s, buffer, sizeof(buffer)) > 0)
			;
		if (errno != EINTR)
			break;
	}
	exit(0);
}

/* ARGSUSED */
discard_dg(s, sep)		/* Discard service -- ignore data */
	int s;
	struct servtab *sep;
{
	char buffer[BUFSIZE];

	(void) read(s, buffer, sizeof(buffer));
}

#define LINESIZ 72
char ring[128];
char *endring;

initring()
{
	register int i;

	endring = ring;

	for (i = 0; i <= 128; ++i)
		if (isprint(i))
			*endring++ = i;
}

/* ARGSUSED */
chargen_stream(s, sep)		/* Character generator */
	int s;
	struct servtab *sep;
{
	register char *rs;
	int len;
	char text[LINESIZ+2];

	setproctitle(sep->se_service, s);

	if (!endring) {
		initring();
		rs = ring;
	}

	text[LINESIZ] = '\r';
	text[LINESIZ + 1] = '\n';
	for (rs = ring;;) {
		if ((len = endring - rs) >= LINESIZ)
			bcopy(rs, text, LINESIZ);
		else {
			bcopy(rs, text, len);
			bcopy(ring, text + len, LINESIZ - len);
		}
		if (++rs == endring)
			rs = ring;
		if (write(s, text, sizeof(text)) != sizeof(text))
			break;
	}
	exit(0);
}

/* ARGSUSED */
chargen_dg(s, sep)		/* Character generator */
	int s;
	struct servtab *sep;
{
	struct sockaddr sa;
	static char *rs;
	int len, size;
	char text[LINESIZ+2];

	if (endring == 0) {
		initring();
		rs = ring;
	}

	size = sizeof(sa);
	if (recvfrom(s, text, sizeof(text), 0, &sa, &size) < 0)
		return;

	if ((len = endring - rs) >= LINESIZ)
		bcopy(rs, text, LINESIZ);
	else {
		bcopy(rs, text, len);
		bcopy(ring, text + len, LINESIZ - len);
	}
	if (++rs == endring)
		rs = ring;
	text[LINESIZ] = '\r';
	text[LINESIZ + 1] = '\n';
	(void) sendto(s, text, sizeof(text), 0, &sa, sizeof(sa));
}

/*
 * Return a machine readable date and time, in the form of the
 * number of seconds since midnight, Jan 1, 1900.  Since gettimeofday
 * returns the number of seconds since midnight, Jan 1, 1970,
 * we must add 2208988800 seconds to this figure to make up for
 * some seventy years Bell Labs was asleep.
 */

unsigned int
machtime()
{
	struct timeval tv;

	if (gettimeofday(&tv, (struct timezone *)0) < 0) {
		syslog(LOG_DEBUG,MSGSTR(TIMERR, 
			"Unable to get time of day\n")); /*MSG*/
		return (0L);
	}
	return (htonl(tv.tv_sec + 2208988800));
}

/* ARGSUSED */
machtime_stream(s, sep)
	int s;
	struct servtab *sep;
{
	unsigned int result;

	result = machtime();
	(void) write(s, (char *) &result, sizeof(result));
}

/* ARGSUSED */
machtime_dg(s, sep)
	int s;
	struct servtab *sep;
{
	unsigned int result;
	struct sockaddr sa;
	int size;

	size = sizeof(sa);
	if (recvfrom(s, (char *)&result, sizeof(result), 0, &sa, &size) < 0)
		return;
	result = machtime();
	(void) sendto(s, (char *) &result, sizeof(result), 0, &sa, sizeof(sa));
}

/* ARGSUSED */
daytime_stream(s, sep)		/* Return human-readable time of day */
	int s;
	struct servtab *sep;
{
	char buffer[256];
	time_t time(), clock;
	char *ctime();

	clock = time((time_t *) 0L);

	(void) sprintf(buffer, "%.24s\r\n", ctime(&clock));
	(void) write(s, buffer, strlen(buffer));
}

/* ARGSUSED */
daytime_dg(s, sep)		/* Return human-readable time of day */
	int s;
	struct servtab *sep;
{
	char buffer[256];
	time_t time(), clock;
	struct sockaddr sa;
	int size;
	char *ctime();

	clock = time((time_t *) 0L);

	size = sizeof(sa);
	if (recvfrom(s, buffer, sizeof(buffer), 0, &sa, &size) < 0)
		return;
	(void) sprintf(buffer, "%.24s\r\n", ctime(&clock));
	(void) sendto(s, buffer, strlen(buffer), 0, &sa, sizeof(sa));
}

/*
 * print_service:
 *	Dump relevant information to stderr
 */
print_service(action, sep)
	char *action;
	struct servtab *sep;
{
	fprintf(stderr, MSGSTR( PRNTSERV,
	    "%s: %s proto=%s, wait=%d, user=%s builtin=%x server=%s\n"),
	    action, sep->se_service, sep->se_proto,
	    (long)sep->se_wait, sep->se_user, sep->se_bi, sep->se_server);
}
