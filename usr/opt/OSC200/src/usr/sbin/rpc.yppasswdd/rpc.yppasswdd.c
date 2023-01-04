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
static char     *sccsid = "@(#)$RCSfile: rpc.yppasswdd.c,v $ $Revision: 4.2.2.2 $ (DEC) $Date: 1992/03/25 12:50:57 $";
#endif
/*
 */


/*
 * Copyright (c) 1985 by Sun Microsystems, Inc.
 */

#include <stdio.h>
#include <signal.h>
#include <sys/file.h>
#include <rpc/rpc.h>
#include <pwd.h>
#include <rpcsvc/yppasswd.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <ctype.h>
#include <strings.h>
#include <syslog.h>
#include <sys/resource.h>

#define	pwmatch(name, line, cnt) \
	(line[cnt] == ':' && strncmp(name, line, cnt) == 0)

char *file;			/* file in which passwd's are found */
char *filea;			/* adjunct file in which passwd's are found */
char	temp[96];		/* lockfile for modifying 'file' */
int mflag;			/* do a make */

extern char *crypt();
extern int errno;
static void boilerplate();
int Argc;
char **Argv;

/* must match sizes in passwd */
#define	STRSIZE 100
#define	PWBUFSIZE 10

#define	PWSIZE (PWBUFSIZE - 1)
#define	CRYPTPWSIZE 13
#define	FINGERSIZE (4 * STRSIZE - 4)
#define	SHELLSIZE (STRSIZE - 2)

int Mstart;
int single = 1;
int nogecos = 0;
int noshell = 0;
int nopw = 0;

main(argc, argv)
	int argc;
	char **argv;
{
	SVCXPRT *transp;
	int s;
	char	*cp;	/* Temporary for building the temp file name */
	int i;

	Argc = argc;
	Argv = argv;
	if (argc < 2) {
		(void)fprintf(stderr,
			"usage: %s file [adjfile] [-nosingle] [-nopw] [-nogecos] [-noshell] [-m arg1 arg2 ...]\n",
			argv[0]);
		exit(1);
	}
	file = argv[1];
	if (access(file, W_OK) < 0) {
		(void)fprintf(stderr, "yppasswdd: can't write %s\n", file);
		exit(1);
	}
	if (argc > 2 && argv[2][0] == '-' && argv[2][1] == 'm')
		mflag++;
	else if (argc > 2) {
		/*
		 * see if they have specified an adjunct passwd file
		 */
		i = 2;
		for (; i<argc; i++){
			if (strcmp(argv[i], "-m") == 0) {
				mflag++;
				Mstart=i;
				break;
			} else if (strcmp(argv[i], "-single") == 0)
				single = 1;
			else if (strcmp(argv[i], "-nosingle") == 0)
				single = 0;
			else if (strcmp(argv[i], "-nogecos") == 0)
				nogecos = 1;
			else if (strcmp(argv[i], "-nopw") == 0)
				nopw = 1;
			else if (strcmp(argv[i], "-noshell") == 0)
				noshell = 1;
			else
				(void)fprintf(stderr,
				    "yppasswdd: unknown option %s ignored\n",
				argv[i]);
		}
	}

	if (chdir("/var/yp") < 0) {
		fprintf(stderr, "yppasswdd: can't chdir to /var/yp\n");
		exit(1);
	}

	/* make a temp file in the same dir as the passwd file */
	(void)strcpy(temp, file);

	/* find end of the path ... */
	for (cp = &(temp[strlen(temp)]); (cp != temp) && (*cp != '/'); cp--)
		;
	if (*cp == '/')
		(void)strcat(cp+1, ".ptmp");
	else
		(void)strcat(cp, ".ptmp");
	/* temp now has either '.ptmp' or 'filepath/.ptmp' in it */

#ifndef DEBUG
	if (fork())
		exit(0);
	{ int t;
	for (t = getdtablesize()-1; t >= 0; t--)
			(void) close(t);
	}
	(void) open("/dev/null", O_RDWR);
	(void) dup2(0, 1);
	(void) dup2(0, 2);
	{
		int tt = open("/dev/tty", O_RDWR);
		if (tt > 0) {
			(void)ioctl(tt, TIOCNOTTY, 0);
			(void)close(tt);
		}
	}
#endif
	openlog("yppasswdd", LOG_CONS| LOG_PID, LOG_AUTH);
	unlimit(RLIMIT_CPU);
	unlimit(RLIMIT_FSIZE);

	if ((s = rresvport()) < 0) {
		syslog(LOG_ERR,
		    "yppasswdd: can't bind to a privileged socket\n");
		exit(1);
	}
	transp = svcudp_create(s);
	if (transp == NULL) {
		syslog(LOG_ERR, "yppasswdd: couldn't create an RPC server\n");
		exit(1);
	}
	(void)pmap_unset(YPPASSWDPROG, YPPASSWDVERS);
	if (!svc_register(transp, YPPASSWDPROG, YPPASSWDVERS,
	    boilerplate, IPPROTO_UDP)) {
		syslog(LOG_ERR, "yppasswdd: couldn't register yppasswdd\n");
		exit(1);
	}

	svc_run();
	syslog(LOG_ERR, "yppasswdd: svc_run shouldn't have returned\n");
	exit(1);
	/* NOTREACHED */
}

static void
boilerplate(rqstp, transp)
	struct svc_req *rqstp;
	SVCXPRT *transp;
{
	switch (rqstp->rq_proc) {
		case NULLPROC:
			if (!svc_sendreply(transp, xdr_void, (char *)0))
				syslog(LOG_WARNING,
				    "yppasswdd: couldn't reply to RPC call\n");
			break;
		case YPPASSWDPROC_UPDATE:
			changepasswd(rqstp, transp);
			break;
	}
}

/*ARGSUSED*/
changepasswd(rqstp, transp)
	struct svc_req *rqstp;
	SVCXPRT *transp;
{
	int tempfd, i, len;
	static int ans;
	FILE *tempfp, *filefp;
	char buf[256], *p;
	char cmdbuf[BUFSIZ];
	void (*f1)(), (*f2)(), (*f3)();
	struct passwd *oldpw, *newpw;
	struct passwd *getpwnam();
	struct passwd_adjunct *oldpwa;
	struct yppasswd yppasswd;
	union wait status;
	char *ptr;
	char str1[224];
	char str2[224];
	int changedpw = 0;
	int changedsh = 0;
	int changedgecos = 0;
	char *cp;

#ifdef EXTRA_PARANOID
	struct sockaddr_in *caller;

	caller = svc_getcaller(transp);
	if ((caller->sin_family != AF_INET) ||
	    (ntohs(caller->sin_port)) >= IPPORT_RESERVED ||
	    (! yp_valid_client(ntohl(caller->sin_addr.s_addr)))) {
		svcerr_weakauth(transp);
		return;
	}
#endif

	bzero((char *)&yppasswd, sizeof (yppasswd));
	if (!svc_getargs(transp, xdr_yppasswd, &yppasswd)) {
		svcerr_decode(transp);
		return;
	}

	if ((! validstr(yppasswd.oldpass, PWSIZE)) ||
	    (! validstr(yppasswd.newpw.pw_passwd, CRYPTPWSIZE)) ||
	    (! validstr(yppasswd.newpw.pw_gecos, FINGERSIZE)) ||
	    (! validstr(yppasswd.newpw.pw_shell, SHELLSIZE))) {
		svcerr_decode(transp);
		return;
	}

	/*
	 * Clean up from previous changepasswd() call
	 */
	while (wait3(&status, WNOHANG, (struct rusage *)0) > 0)
		continue;

	newpw = &yppasswd.newpw;
	ans = 2;
	changedsh = 0;
	changedpw = 0;
	changedgecos = 0;

	oldpw = getpwnam(newpw->pw_name);
	if (oldpw == NULL) {
		syslog(LOG_NOTICE, "yppasswdd: no passwd for %s\n",
		    newpw->pw_name);
		goto done;
	}

	if ((!nopw) && (strcmp(oldpw->pw_passwd, newpw->pw_passwd) != 0))
		changedpw = 1;

	if ((!noshell) &&
		(strcmp(oldpw->pw_shell, newpw->pw_shell) != 0)){
		if (single) changedpw = 0;
			changedsh = 1;
		}

	if ((!nogecos) &&
		(strcmp(oldpw->pw_gecos, newpw->pw_gecos) != 0)){
		if (single){
			changedpw = 0;
			changedsh = 0;
			}
		changedgecos = 1;
		}

	if (!(changedpw + changedgecos + changedsh)) {
		syslog(LOG_NOTICE, "yppasswdd: no change for %s\n",
		    newpw->pw_name);
		    ans = 3;
		    goto done;
	}

	if (/*changedpw && */oldpw->pw_passwd && *oldpw->pw_passwd &&
	    strcmp(crypt(yppasswd.oldpass, oldpw->pw_passwd),
	    oldpw->pw_passwd) != 0) {
		syslog(LOG_NOTICE,
			"%s: password incorrect\n", newpw->pw_name);
		ans = 7;
		goto done;
	}
	(void)printf("%d %d %d\n", changedsh, changedgecos, changedpw);
	(void)printf("%s %s %s\n", yppasswd.newpw.pw_shell,
			yppasswd.newpw.pw_gecos, yppasswd.newpw.pw_passwd);
	(void)printf("%s %s %s\n", oldpw->pw_shell, oldpw->pw_gecos,
			oldpw->pw_passwd);
	if (changedsh && !validloginshell(oldpw, yppasswd.newpw.pw_shell)) {
		goto done;
	}

	(void) umask(0);

	f1 = signal(SIGHUP, SIG_IGN);
	f2 = signal(SIGINT, SIG_IGN);
	f3 = signal(SIGQUIT, SIG_IGN);

changeothers:
	(void)strcpy(temp, file);
	/* find end of the path ... */
	for (cp = &(temp[strlen(temp)]); (cp != temp) && (*cp != '/'); cp--)
		;
	if (*cp == '/')
		(void)strcat(cp + 1, ".ptmp");
	else
		(void)strcat(cp, ".ptmp");
	/* temp now has either '.ptmp' or 'filepath/.ptmp' in it */

	tempfd = open(temp, O_WRONLY|O_CREAT|O_EXCL, 0644);
	if (tempfd < 0) {
		if (errno == EEXIST){
			syslog(LOG_WARNING,
				"password file busy - try again.\n");
			ans = 8;
		} else {
			ans = 9;
			syslog(LOG_ERR,
				"password temp file open bug %s %m.\n", temp);
			perror(temp);
		}
		goto cleanup;
	}
	(void)signal(SIGTSTP, SIG_IGN);
	if ((tempfp = fdopen(tempfd, "w")) == NULL) {
		syslog(LOG_ERR, "yppasswdd: %s fdopen failed\n", temp);
		ans = 10;
		goto cleanup;
	}
	/*
	 * Copy passwd to temp, replacing matching lines
	 * with new password.
	 */
	if ((filefp = fopen(file, "r")) == NULL) {
		syslog(LOG_CRIT, "fopen of %s failed %m\n",
		    file);
		ans = 12;
		goto cleanup;
	}
	len = strlen(newpw->pw_name);
	/*
	 * This fixes a really bogus security hole, basically anyone can
	 * call the rpc passwd daemon, give them their own passwd and a
	 * new one that consists of ':0:0:Im root now:/:/bin/csh^J' and
	 * give themselves root access. With this code it will simply make
	 * it impossible for them to login again, and as a bonus leave
	 * a cookie for the always vigilant system administrator to ferret
	 * them out.
	 */
	for (p = newpw->pw_name; (*p != '\0'); p++)
		if ((*p == ':') || !(isprint(*p)))
			*p = '$';	/* you lose buckwheat */
	for (p = newpw->pw_passwd; (*p != '\0'); p++)
		if ((*p == ':') || !(isprint(*p)))
			*p = '$';	/* you lose buckwheat */

	while (fgets(buf, sizeof (buf), filefp)) {
		p = index(buf, ':');
		if (p && p - buf == len &&
		    strncmp(newpw->pw_name, buf, p - buf) == 0) {
			(void)fprintf(tempfp, "%s:%s:%d:%d:%s:%s:%s\n",
				    oldpw->pw_name,
				    (changedpw ?
					newpw->pw_passwd: oldpw->pw_passwd),
				    oldpw->pw_uid,
				    oldpw->pw_gid,
				    (changedgecos ?
					newpw->pw_gecos : oldpw->pw_gecos),
				    oldpw->pw_dir,
				    (changedsh ?
					newpw->pw_shell  : oldpw->pw_shell));
		} else {
			if (fputs(buf, tempfp) == EOF) {
				syslog(LOG_CRIT, "%s: write error\n", "passwd");
				ans = 13;
				goto cleanup;
			}
		}
		if (ferror(tempfp)) {
			syslog(LOG_CRIT, "%s: write ferror set.\n", "passwd");
			ans = 14;
			goto cleanup;
		}
	}
	(void)fclose(filefp);
	(void)fflush(tempfp);
	if (ferror(tempfp)) {
		syslog(LOG_CRIT, "%s: fflush ferror set.\n", "passwd");
		ans = 15;
		goto cleanup;
	}
	(void)fclose(tempfp);
	tempfp = (FILE *) NULL;
	if (rename(temp, file) < 0) {
		syslog(LOG_CRIT, "yppasswdd: "); perror("rename");
		(void)unlink(temp);
		ans = 17;
		goto cleanup;
	}
	ans = 0;

	if (mflag && fork() == 0) {
		(void)strcpy(cmdbuf, "make");
		for (i = Mstart; i < Argc; i++) {
			(void)strcat(cmdbuf, " ");
			(void)strcat(cmdbuf, Argv[i]);
		}
#ifdef DEBUG
		syslog(LOG_ERR, "about to execute %s\n", cmdbuf);
#else
		(void)system(cmdbuf);
#endif
		exit(0);
	}
cleanup:
	if (tempfp)
		(void)fclose(tempfp);
	(void)signal(SIGHUP, f1);
	(void)signal(SIGINT, f2);
	(void)signal(SIGQUIT, f3);
done:
	if (!svc_sendreply(transp, xdr_int, (char *)&ans))
		syslog(LOG_WARNING, "yppasswdd: couldnt reply to RPC call\n");
}

static int
rresvport()
{
	struct sockaddr_in sin;
	int s, alport = IPPORT_RESERVED - 1;

	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = 0;
	s = socket(AF_INET, SOCK_DGRAM, 0);
	if (s < 0)
		return (-1);
	for (; ;) {
		sin.sin_port = htons((u_short)alport);
		if (bind(s, (struct sockaddr *)&sin, sizeof (sin)) >= 0)
			return (s);
		if (errno != EADDRINUSE && errno != EADDRNOTAVAIL) {
			perror("socket");
			syslog(LOG_ERR, "socket: %m\n");
			return (-1);
		}
		(alport)--;
		if (alport == IPPORT_RESERVED/2) {
			syslog(LOG_ERR, "socket: All ports in use\n");
			return (-1);
		}
	}
}

static char *
pwskip(p)
	register char *p;
{
	while (*p && *p != ':' && *p != '\n')
		++p;
	if (*p) *p++ = 0;
	return (p);
}

static struct passwd *
getpwnam(name)
	char *name;
{
	FILE *pwf;
	int cnt;
	char *p;
	static char line[BUFSIZ+1];
	static struct passwd passwd;

	pwf = fopen(file, "r");
	if (pwf == NULL)
		return (NULL);
	cnt = strlen(name);
	while ((p = fgets(line, BUFSIZ, pwf)) && !pwmatch(name, line, cnt))
		;
	if (p) {
		passwd.pw_name = p;
		p = pwskip(p);
		passwd.pw_passwd = p;
		p = pwskip(p);
		passwd.pw_uid = atoi(p);
		p = pwskip(p);
		passwd.pw_gid = atoi(p);
		passwd.pw_comment = "";
		p = pwskip(p);
		passwd.pw_gecos = p;
		p = pwskip(p);
		passwd.pw_dir = p;
		p = pwskip(p);
		passwd.pw_shell = p;
		while (*p && *p != '\n') p++;
		*p = '\0';
		(void)fclose(pwf);
		return (&passwd);
	} else {
		(void)fclose(pwf);
		return (NULL);
	}
}

validstr(str, size)
	char *str;
	int size;
{
	char c;

	if (str == NULL || strlen(str) > size || index(str, ':'))
		return (0);
	while (c = *str++) {
		if (iscntrl(c))
			return (0);
	}
	return (1);
}


#define	DEFSHELL "/bin/sh"
bool_t
validloginshell(pwd, arg)
	struct passwd *pwd;
	char *arg;
{
	static char newshell[STRSIZE];
	char *cp, *valid, *getusershell();

	setusershell();	/* XXX */
	if (pwd->pw_shell == 0 || *pwd->pw_shell == '\0')
		pwd->pw_shell = DEFSHELL;
		for (valid = getusershell(); valid; valid = getusershell())
			if (strcmp(pwd->pw_shell, valid) == 0)
				break;
		if (valid == NULL) {
			(void) syslog(LOG_ERR,
				"Cannot change from restricted shell %s\n",
				pwd->pw_shell);
				return (0);
		}
	if (arg != 0) {
		(void) strncpy(newshell, arg, sizeof newshell - 1);
		newshell[sizeof newshell - 1] = 0;
	} else {
		return (0);
	}
	/*
	 * Allow user to give shell name w/o preceding pathname.
	 */
	setusershell();	/* XXX */
	for (valid = getusershell(); valid; valid = getusershell()) {
		if (newshell[0] == '/') {
			cp = valid;
		} else {
			cp = rindex(valid, '/');
			if (cp == 0)
				cp = valid;
			else
				cp++;
			}
		if (strcmp(newshell, cp) == 0)
			break;
	}
	if (valid == 0) {
		(void) syslog(LOG_WARNING,
			"%s is unacceptable as a new shell.\n", newshell);
		    return (0);
	}
	if (access(valid, X_OK) < 0) {
		(void) syslog(LOG_WARNING, "%s is unavailable.\n", valid);
		return (0);
	}
	pwd->pw_shell =  newshell;
	return (1);
}

#ifdef EXTRA_PARANOID
/*ARGSUSED*/
int
yp_valid_client(a)
	unsigned long a;
{
	return (TRUE);
}
#endif

unlimit(lim)
{
	struct rlimit rlim;
	rlim.rlim_cur = rlim.rlim_max = RLIM_INFINITY;
	(void) setrlimit(lim, &rlim);
}
