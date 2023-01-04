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
#if !defined(lint) && !defined(_NOIDENT)
static char rcsid[] = "@(#)$RCSfile: uucpd.c,v $ $Revision: 4.3.6.3 $ (DEC) $Date: 1993/10/11 19:31:18 $";
#endif
/* 
 * COMPONENT_NAME: UUCP uucpd.c
 * 
 * FUNCTIONS: Muucpd, SCPYN, doit, dologin, dologout, dosrcpacket, 
 *            readline, wait3 
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
uucpd.c	1.9  com/cmd/uucp,3.1,9013 3/16/90 10:45:14";
*/

/*
 * 4.2BSD or 2.9BSD TCP/IP server for uucico
 * uucico's TCP channel causes this server to be run at the remote end.
 */


#include "uucp.h"
#include <netdb.h>
#include <signal.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <sys/wait.h>
#include <pwd.h>
#include <lastlog.h>

#define BSDINETD 
#define BSD4_2

char lastlog[] = "/var/adm/lastlog";
struct	sockaddr_in hisctladdr;
int hisaddrlen = sizeof hisctladdr;
struct	sockaddr_in myctladdr;
int mypid;

static char Username[64];
char *nenv[] = {
	Username,
	NULL,
};
extern char **environ;

main(argc, argv)
int argc;
char **argv;
{
#ifndef BSDINETD
	register int s, tcp_socket;
	struct servent *sp;
#endif /* !BSDINETD */
	extern int errno;
	int dologout();

	setlocale(LC_ALL,"");
	catd = catopen(MF_UUCP, NL_CAT_LOCALE);


	environ = nenv;

#ifdef BSDINETD
	close(1); close(2);
	dup(0); dup(0);
	hisaddrlen = sizeof (hisctladdr);
	if (getpeername(0, (struct sockaddr *)&hisctladdr, &hisaddrlen) < 0) {
		fprintf(stderr, "%s: ", argv[0]);
		perror("getpeername");
		_exit(1);
	}
	if (fork() == 0)
		doit(&hisctladdr);
	dologout();
	exit(1);
#else /* !BSDINETD */
	sp = getservbyname("uucp", "tcp");
	if (sp == NULL){
		perror("uucpd: getservbyname");
		exit(1);
	}
 	if (fork())
 		exit(0);
	if ((s=open("/dev/tty", 2)) >= 0){
		ioctl(s, TIOCNOTTY, (char *)0);
		close(s);
	}
 	(void) setsid();

	bzero((char *)&myctladdr, sizeof (myctladdr));
	myctladdr.sin_family = AF_INET;
	myctladdr.sin_port = sp->s_port;
	tcp_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (tcp_socket < 0) {
		perror("uucpd: socket");
		exit(1);
	}
	if (bind(tcp_socket, (char *)&myctladdr, sizeof (myctladdr)) < 0) {
		perror("uucpd: bind");
		exit(1);
	}
	listen(tcp_socket, 3);	/* at most 3 simultaneuos uucp connections */
	signal(SIGCLD, (void(*)(int)) dologout);

	for(;;) {

		s = accept(tcp_socket, &hisctladdr, &hisaddrlen);
		if (s < 0){
			if (errno == EINTR) 
				continue;
			perror("uucpd: accept");
			exit(1);
		}
		if (fork() == 0) {
			close(0); close(1); close(2);
			dup(s); dup(s); dup(s);
			close(tcp_socket); close(s);
			doit(&hisctladdr);
			exit(1);
		}
		close(s);
	}
#endif /* BSDINETD */
}

doit(sinp)
struct sockaddr_in *sinp;
{
	char user[64], passwd[64];
	char *xpasswd, *crypt();
	struct passwd *pw, *getpwnam();

	alarm(60);
	printf("login: "); fflush(stdout);
	if (readline(user, sizeof user) < 0) {
		fprintf(stderr, MSGSTR(MSG_UUCPD_1, "user read\n"));
		return;
	}
	/* truncate username to 8 characters */
	user[8] = '\0';
	pw = getpwnam(user);
	if (pw == NULL) {
		fprintf(stderr, MSGSTR(MSG_UUCPD_2, "user unknown\n"));
		return;
	}
	if (strcmp(pw->pw_shell, UUCICO)) {
		fprintf(stderr, MSGSTR(MSG_UUCPD_3, "Login incorrect."));
		return;
	}
	if (pw->pw_passwd && *pw->pw_passwd != '\0') {
		printf("Password: "); fflush(stdout);
		if (readline(passwd, sizeof passwd) < 0) {
			fprintf(stderr, MSGSTR(MSG_UUCPD_4, "passwd read\n"));
			return;
		}
		xpasswd = crypt(passwd, pw->pw_passwd);
		if (strcmp(xpasswd, pw->pw_passwd)) {
		     fprintf(stderr, MSGSTR(MSG_UUCPD_5, "Login incorrect."));
		     return;
		}
	}
	alarm(0);
	sprintf(Username, "USER=%s", user);
	(void)putenv(Username);
	dologin(pw, sinp);
	setgid(pw->pw_gid);
#ifdef BSD4_2
	initgroups(pw->pw_name, pw->pw_gid);
#endif
	chdir(pw->pw_dir);
	setuid(pw->pw_uid);
#ifdef BSD4_2
	execl(UUCICO, "uucico", (char *)0);
#endif
#ifdef BSD2_9
	sprintf(passwd, "-h%s", inet_ntoa(sinp->sin_addr));
	execl(UUCICO, "uucico", passwd, (char *)0);
#endif
	perror("uucico server: execl");
}

readline(p, n)
register char *p;
register int n;
{
	char c;

	while (n-- > 0) {
		if (read(0, &c, 1) <= 0)
			return(-1);
		c &= 0177;
		if (c == '\n' || c == '\r') {
			*p = '\0';
			return(0);
		}
		*p++ = c;
	}
	return(-1);
}

#include <utmp.h>
#ifdef BSD4_2
#include <fcntl.h>
#endif

#define	SCPYN(a, b)	strncpy(a, b, sizeof (a))

struct	utmp utmp;

dologout()
{
	union wait status;
	int pid, wtmp;

#ifdef BSDINETD
	while ((pid=wait(&status)) > 0) {
#else
	while ((pid=wait3(&status,WNOHANG,0)) > 0) {
#endif
		wtmp = open(WTMP_FILE, O_WRONLY|O_APPEND);
		if (wtmp >= 0) {
			sprintf(utmp.ut_line, "uucp%.4d", pid);
			SCPYN(utmp.ut_name, "");
			SCPYN(utmp.ut_host, "");
			(void) time(&utmp.ut_time);
			(void) write(wtmp, (char *)&utmp, sizeof (utmp));
			(void) close(wtmp);
		}
	}
}

/*
 * Record login in wtmp file.
 */
dologin(pw, sin)
struct passwd *pw;
struct sockaddr_in *sin;
{
	char line[32];
	char remotehost[32];
	int wtmp, f;
	struct hostent *hp = gethostbyaddr((char *) &sin->sin_addr,
		sizeof (struct in_addr), AF_INET);

	if (hp) {
		strncpy(remotehost, hp->h_name, sizeof (remotehost));
		endhostent();
	} else
		strncpy(remotehost, inet_ntoa(sin->sin_addr),
		    sizeof (remotehost));
	wtmp = open(WTMP_FILE, O_WRONLY|O_APPEND);
	if (wtmp >= 0) {
		/* hack, but must be unique and no tty line */
		sprintf(line, "uucp%.4d", getpid());
		SCPYN(utmp.ut_line, line);
		SCPYN(utmp.ut_name, pw->pw_name);
		SCPYN(utmp.ut_host, remotehost);
		time(&utmp.ut_time);
		(void) write(wtmp, (char *)&utmp, sizeof (utmp));
		(void) close(wtmp);
	}
	if ((f = open(lastlog, 2)) >= 0) {
		struct lastlog ll;

		time(&ll.ll_time);
		lseek(f, (long)pw->pw_uid * sizeof(struct lastlog), 0);
		strcpy(line, remotehost);
		SCPYN(ll.ll_line, line);
		SCPYN(ll.ll_host, remotehost);
		(void) write(f, (char *) &ll, sizeof ll);
		(void) close(f);
	}
}
