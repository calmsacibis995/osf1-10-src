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
static char	*sccsid = "@(#)$RCSfile: shutdown.c,v $ $Revision: 4.3.11.4 $ (DEC) $Date: 1993/11/05 21:04:48 $";
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
 * Copyright (c) 1983,1986 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

#ifndef lint
char copyright[] =
"@(#) Copyright (c) 1983,1986 Regents of the University of California.\n\
 All rights reserved.\n";
#endif not lint


#include <sys/secdefines.h>
#if SEC_BASE
#include <sys/security.h>

extern priv_t *privvec();
#endif

#include <stdio.h>
#include <ctype.h>
#include <signal.h>
#include <setjmp.h>
#include <utmp.h>
#include <pwd.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/param.h>
#include <sys/syslog.h>
#include <dec/binlog/binlog.h>      /*   binary event log sypport */

#ifdef NLS
#include <locale.h>
#endif

#ifdef MSG
#include "shutdown_msg.h" 
nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_SHUTDOWN,n,s) 
#ifdef SEC_BASE
#define MSGSTR_SEC(n,s) catgets(catd,MS_SHUTDOWN_SEC,n,s)
#endif
#else
#define MSGSTR(n,s) s
#endif

#if defined(KJI) || defined(NLS)
#include <NLchar.h>
#include <NLctype.h>
#endif


/*
 *	/usr/sbin/shutdown when [messages]
 *
 *	allow super users to tell users and remind users
 *	of iminent shutdown of unix
 *	and shut it down automatically
 *	and even reboot or halt the machine if they desire
 */

#define	REBOOT	"/sbin/reboot"
#define	HALT	"/sbin/halt"
#define MAXINTS 20
#define	HOURS	*3600
#define MINUTES	*60
#define SECONDS
#define NLOG		600		/* no of bytes possible for message */
#define	NOLOGTIME	5 MINUTES
#define IGNOREUSER	"sleeper"
#define CONSOLE         "/dev/console"

char	hostname[MAXHOSTNAMELEN];

int	timeout();
time_t	getsdt();
void	finish();

extern	char *ctime();
extern	struct tm *localtime();
extern	int time();

extern	char *strcpy();
extern	char *strncat();
extern	off_t lseek();

struct	utmp utmp;
int	sint;
int	stogo;
char	tpath[] =	"/dev/";
int	nlflag = 1;		/* nolog yet to be done */
int	killflg = 1;
int	doreboot = 0;
int	halt = 0;
int     fast = 0;
int     client_inform = 0;
char    *nosync = NULL;
char    nosyncflag[] = "-n";
char	term[sizeof tpath + sizeof utmp.ut_line];
char	tbuf[BUFSIZ];
char	nolog2[NLOG+1];
char    binlog_msg[1024];           /* binary event log sypport */
#ifdef	DEBUG
char	nologin[] = "nologin";
char    fastboot[] = "fastboot";
#else
char	nologin[] = "/etc/nologin";
char	fastboot[] = "/fastboot";
#endif
time_t	nowtime;
jmp_buf	alarmbuf;

struct interval {
	int stogo;
	int sint;
} interval[] = {
	4 HOURS,	1 HOURS,
	2 HOURS,	30 MINUTES,
	1 HOURS,	15 MINUTES,
	30 MINUTES,	10 MINUTES,
	15 MINUTES,	5 MINUTES,
	10 MINUTES,	5 MINUTES,
	5 MINUTES,	3 MINUTES,
	2 MINUTES,	1 MINUTES,
	1 MINUTES,	30 SECONDS,
	0 SECONDS,	0 SECONDS
};

char *shutter, *getlogin();

main(argc,argv)
	int argc;
	char **argv;
{
	register i, ufd;
	register char *f;
	char *ts;
	time_t sdt;
	int h, m;
	int first;
	FILE *termf;
	struct passwd *pw, *getpwuid();
	extern char *strcat();
	extern uid_t geteuid();
	int sawconsole;
	int orig_client_inform;

#ifdef NLS
	(void) setlocale( LC_ALL, "" );
#endif

#ifdef MSG
	catd = catopen(MF_SHUTDOWN,NL_CAT_LOCALE);
#endif

#if SEC_BASE
        set_auth_parameters(argc, argv);
        initprivs();
#endif

	shutter = getlogin();
        if ((shutter == 0 || *shutter == '\0') && (pw = getpwuid(getuid())))
		shutter = pw->pw_name;
	if (shutter == 0)
		shutter = "???";
	(void) gethostname(hostname, sizeof (hostname));
	openlog("shutdown", 0, LOG_AUTH);
	argc--, argv++;
	while (argc > 0 && (f = argv[0], *f++ == '-')) {
		while (i = *f++) switch (i) {
		case 'k':
			killflg = 0;
			continue;
		case 'n':
			nosync = nosyncflag;
			continue;
		case 'f':
			fast = 1;
			continue;
		case 'r':
			doreboot = 1;
			continue;
  	        case 'h':
			halt = 1;
			continue;
		case 'b':
			client_inform = 1;
			continue;
		default:
			fprintf(stderr, MSGSTR(UNKNOWN, "shutdown: '%c' - unknown flag\n"), i);
			exit(1);
		}
		argc--, argv++;
	}
	if (argc < 1) {
	        /* argv[0] is not available after the argument handling. */
		printf(MSGSTR(USAGE, "Usage: shutdown [ -krhfn ] shutdowntime [ message ]\n"));
		exit(1);
	}
	if (fast && (nosync == nosyncflag)) {
	        printf (MSGSTR(INCOMP, "shutdown: Incompatible switches 'fast' & 'nosync'\n"));
		exit(1);
	}
#if SEC_BASE
        if (!authorized_user("sysadmin")) {
                fprintf(stderr, MSGSTR_SEC(SYS_AUTH,"shutdown: need sysadmin authorization\n"));
                exit(1);
        }
        if (forceprivs(privvec(SEC_KILL, SEC_LIMIT, SEC_ALLOWDACACCESS,
#if SEC_MAC
                                SEC_ALLOWMACACCESS,
#endif
#if SEC_ILB
                                SEC_ILNOFLOAT,
#endif
#if SEC_NCAV
                                SEC_ALLOWNCAVACCESS,
#endif
                                -1), (priv_t *) 0)) {
                fprintf(stderr, MSGSTR_SEC(INSUFF_PRIV, "shutdown: insufficient privileges\n"));
                exit(1);
        }
#else /* !SEC_BASE */
	if (geteuid()) {
		fprintf(stderr, MSGSTR(NO_ROOT, "NOT super-user\n"));
		exit(1);
	}
#endif /* !SEC_BASE */
	nowtime = time((long *)0);
	sdt = getsdt(argv[0]);
	argc--, argv++;
	nolog2[0] = '\0';
	while (argc-- > 0) {
		(void) strcat(nolog2, " ");
		(void) strcat(nolog2, *argv++);
	}
	m = ((stogo = sdt - nowtime) + 30)/60;
	h = m/60; 
	m %= 60;
	ts = ctime(&sdt);
	printf(MSGSTR(MSG_AT, "Shutdown at %5.5s (in "), ts+11);
	if (h > 0)
		printf(MSGSTR(MSG_TIME_1, "%d hour%s "), h, h != 1 ? MSGSTR(MSG_TIME_2, "s") : "");
	printf(MSGSTR(MSG_TIME_3, "%d minute%s) "), m, m != 1 ? MSGSTR(MSG_TIME_2, "s") : "");
#ifndef DEBUG
	(void) signal(SIGHUP, SIG_IGN);
	(void) signal(SIGQUIT, SIG_IGN);
	(void) signal(SIGINT, SIG_IGN);
#endif
	(void) signal(SIGTTOU, SIG_IGN);
	(void) signal(SIGTERM, finish);
	(void) signal(SIGALRM, (void (*) (int)) timeout);
	(void) setpriority(PRIO_PROCESS, 0, PRIO_MIN);
	(void) fflush(stdout);
#ifndef DEBUG
	if (i = fork()) {
		printf(MSGSTR(MSG_PID, "[pid %d]\n"), i);
		exit(0);
	}
#else
	(void) putc('\n', stdout);
#endif
	sint = 1 HOURS;
	f = "";
	ufd = open(UTMP_FILE,0);
	if (ufd < 0) {
		perror(MSGSTR(10, "shutdown: /var/adm/utmp"));
		exit(1);
	}
	first = 1;
	for (;;) {
		for (i = 0; stogo <= interval[i].stogo && interval[i].sint; i++)
			sint = interval[i].sint;
		if (stogo > 0 && (stogo-sint) < interval[i].stogo)
			sint = stogo - interval[i].stogo;
		if (stogo <= NOLOGTIME && nlflag) {
			nlflag = 0;
			nolog(sdt);
		}
		if (sint >= stogo || sint == 0)
			f = MSGSTR(MSG_FINAL, "FINAL ");
		nowtime = time((long *)0);
		(void) lseek(ufd, 0L, 0);
		sawconsole = 0;
		orig_client_inform = client_inform;
		while (read(ufd,(char *)&utmp,sizeof utmp)==sizeof utmp)
		if (utmp.ut_name[0] &&
		    strncmp(utmp.ut_name, IGNOREUSER, sizeof(utmp.ut_name))) {
			if (setjmp(alarmbuf))
				continue;
			(void) strcpy(term, tpath);
			(void) strncat(term, utmp.ut_line, sizeof utmp.ut_line);
                        if (strcmp(term, CONSOLE) == 0)
                                sawconsole++;
                        TellHim(sdt, nowtime, first, f);
			client_inform = 0;
		}
		client_inform = orig_client_inform;
		if (stogo <= 0) {
                    if (setjmp(alarmbuf) == 0)
                    {
                        (void) alarm(10);

			printf(MSGSTR(ARRIVED, "\n\007\007System shutdown time has arrived\007\007\n"));
                        (void) alarm(0);
                    }

			syslog(LOG_CRIT, MSGSTR(MSG_BY, "%s by %s: %s"),
			    doreboot ? "reboot" : halt ? "halt" : "shutdown",
			    shutter, nolog2);

                        /* put the message into the binary event log */
			sprintf(binlog_msg, " System %s by %s: %s",
			   doreboot ? "rebooted" : halt ? "halted" : "shutdown",
			            shutter, nolog2);
			binlogmsg(ELMSGT_SD, binlog_msg);

			close(ufd);
			closelog();

			sleep(2);
			(void) unlink(nologin);
			if (!killflg) {
				printf(MSGSTR(YOURSELF, "but you'll have to do it yourself\n"));
				finish();
			}
			if (fast)
				doitfast();
#ifndef DEBUG
			if (doreboot)
				execle(REBOOT, "reboot", "-l", nosync, 0, 0);
			if (halt)
				execle(HALT, "halt", "-l", nosync, 0, 0);
			(void) kill(1, SIGBUS);		/* to single user */
#else
			if (doreboot)
				printf(MSGSTR(MSG_REBOOT, "REBOOT"));
			if (halt)
				printf(MSGSTR(MSG_HALT, " HALT"));
			if (fast)
				printf(MSGSTR(NO_FSCK, " -l %s (without fsck's)\n"), nosync);
			else
				printf(" -l %s\n", nosync);
			else
				printf(MSGSTR(KILL_HUP, "kill -HUP 1\n"));

#endif
			finish();
		}
		stogo = sdt - time((long *) 0);
		if (stogo > 0 && sint > 0)
			sleep((unsigned)(sint<stogo ? sint : stogo));
		stogo -= sint;
		first = 0;
	}
}

TellHim(sdt, nowtime, first, f)
        time_t sdt, nowtime;
        int first;
        char *f;
{
        FILE *termf;

        (void) alarm(3);
#ifdef DEBUG
        if ((termf = stdout) != NULL)
#else
 	if ((termf = fopen(term, "r")) != NULL) { /* make sure the term exists: fix for qar 10095 */
		fclose(termf);
 		if ((termf = fopen(term, "w")) != NULL)
#endif
		{
 			(void) alarm(0);
			setbuf(termf, tbuf);
 			fprintf(termf, "\n\r\n");
			warn(termf, sdt, nowtime, f);
 			if (first || sdt - nowtime > 1 MINUTES) {
 				if (*nolog2)
					fprintf(termf, "\t...%s", nolog2);
			}
			(void) fputc('\r', termf);
 			(void) fputc('\n', termf);
 			(void) alarm(5);
#ifdef DEBUG
			(void) fflush(termf);
#else
 			(void) fclose(termf);
#endif
 			(void) alarm(0);
		}
	}
 	(void) alarm(0);
}


time_t
getsdt(s)
	register char *s;
{
	time_t t, t1, tim;
	register char c;
	struct tm *lt;

	if (strcmp(s, "now") == 0)
		return(nowtime);
	if (*s == '+') {
		++s; 
		t = 0;
		for (;;) {
			c = *s++;
			if (!isdigit(c))
				break;
			t = t * 10 + c - '0';
		}
		if (t <= 0)
			t = 5;
		t *= 60;
		tim = time((long *) 0) + t;
		return(tim);
	}
	/* At this point we should have a valid time in the buffer
	 * of the form "hh:mm".  Check this just in case forgot
	 * to type in a time and this is the message option.
	 */
	if ((*s != ':') && !isdigit(*s))
		goto badform;
	t = 0;
	while (strlen(s) > 2 && isdigit(*s))
		t = t * 10 + *s++ - '0';
	if (*s == ':')
		s++;
	if (t > 23)
		goto badform;
	tim = t*60;
	t = 0;
	while (isdigit(*s))
		t = t * 10 + *s++ - '0';
	if (t > 59)
		goto badform;
	tim += t; 
	tim *= 60;
	t1 = time((long *) 0);
	lt = localtime(&t1);
	t = lt->tm_sec + lt->tm_min*60 + lt->tm_hour*3600;
	if (tim < t || tim >= (24*3600)) {
		/* before now or after midnight */
		printf(MSGSTR(MSG_WAIT, "That must be tomorrow\nCan't you wait till then?\n"));
		finish();
	}
	return (t1 + tim - t);
badform:
	printf(MSGSTR(BAD_TIME, "Bad time format\n"));
	finish();
	/*NOTREACHED*/
}

warn(term, sdt, now, type)
	FILE *term;
	time_t sdt, now;
	char *type;
{
	char *ts;
	char shutdown_msg[BUFSIZ];
	char part_shutdown_msg[BUFSIZ];
	register delay = sdt - now;

	if (delay > 8)
		while (delay % 5)
			delay++;

	fprintf(term,
	MSGSTR(SYS_MSG, "\r\n\007\007\t*** %sSystem shutdown message from %s@%s ***\r\n\n"),
		    type, shutter, hostname);
	if (client_inform){
	  sprintf(shutdown_msg,
		  MSGSTR(SYS_MSG, "\r\n\007\007\t*** %sSystem shutdown message from %s@%s ***\r\n\n"),
		  type, shutter, hostname);

	}
	  

	ts = ctime(&sdt);
	if (delay > 10 MINUTES) {
		fprintf(term, MSGSTR(DOWN_1, "System going down at %5.5s\r\n"), ts+11);
		if (client_inform){
			sprintf(part_shutdown_msg, MSGSTR(DOWN_1, "System going down at %5.5s\r\n"), ts+11);
			strcat(shutdown_msg, part_shutdown_msg);
		}
	} else if (delay > 95 SECONDS) {
		fprintf(term, MSGSTR(DOWN_2, "System going down in %d minute%s\r\n"),
		    (delay+30)/60, (delay+30)/60 != 1 ? MSGSTR(MSG_TIME_2, "s") : "");
		if (client_inform){
		  sprintf(part_shutdown_msg, MSGSTR(DOWN_2, "System going down in %d minute%s\r\n"),
			  (delay+30)/60, (delay+30)/60 != 1 ? MSGSTR(MSG_TIME_2, "s") : "");
		  strcat(shutdown_msg, part_shutdown_msg);
		}
	} else if (delay > 0) {
		fprintf(term, MSGSTR(DOWN_3, "System going down in %d second%s\r\n"),
		    delay, delay != 1 ? MSGSTR(MSG_TIME_2, "s") : "");
		if (client_inform){
		  sprintf(part_shutdown_msg, MSGSTR(DOWN_3, "System going down in %d second%s\r\n"),
		    delay, delay != 1 ? MSGSTR(MSG_TIME_2, "s") : "");
		  strcat(shutdown_msg, part_shutdown_msg);
		}
	} else {
	  fprintf(term, MSGSTR(DOWN_4, "System going down IMMEDIATELY\r\n"));
	  if (client_inform){
	    sprintf(part_shutdown_msg, MSGSTR(DOWN_4, "System going down IMMEDIATELY\r\n"));
	    strcat(shutdown_msg, part_shutdown_msg);
	  }
	}
   if(client_inform)
     	  inform_clients(shutdown_msg);
}

doitfast()
{
	FILE *fastd;

	if ((fastd = fopen(fastboot, "w")) != NULL) {
		putc('\n', fastd);
		(void) fclose(fastd);
	}
}

nolog(sdt)
	time_t sdt;
{
	FILE *nologf;

	(void) unlink(nologin);			/* in case linked to std file */
	if ((nologf = fopen(nologin, "w")) != NULL) {
		fprintf(nologf, MSGSTR(NO_LOGINS, "\n\nNO LOGINS: System going down at %5.5s\n\n"), (ctime(&sdt)) + 11);
		if (*nolog2)
			fprintf(nologf, "\t%s\n", nolog2 + 1);
		(void) fclose(nologf);
	}
}

void
finish()
{
	(void) signal(SIGTERM, SIG_IGN);
	(void) unlink(nologin);
	exit(0);
}

timeout()
{
	longjmp(alarmbuf, 1);
}



/*
 *
 * routines to figure out the clients (showmount) and to send
 * messages to those clients (rwall)
 *
 */

#include <stdio.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/file.h>
#include <sys/socket.h>
#include <sys/socketvar.h>  
#include <netdb.h>  
#include <rpc/rpc.h>
#include <rpc/pmap_clnt.h>
#include <rpc/pmap_prot.h>
#include <utmp.h>
#include <sys/time.h> 
#include <rpcsvc/rwall.h> 
#include <nfs/rpcv2.h> 


/* Constant defs */
#define	ALL	1
#define	DIRS	2
#define	USERS	128

#define	DODUMP		0x1
#define	DOEXPORTS	0x2

struct timeval TIMEOUT = { 25, 0 };

struct mountlist {
	struct mountlist *ml_left;
	struct mountlist *ml_right;
	char	ml_host[RPCMNT_NAMELEN+1];
	char	ml_dirp[RPCMNT_PATHLEN+1];
};

struct grouplist {
	struct grouplist *gr_next;
	char	gr_name[RPCMNT_NAMELEN+1];
};

struct exportslist {
	struct exportslist *ex_next;
	struct grouplist *ex_groups;
	char	ex_dirp[RPCMNT_PATHLEN+1];
};

static struct mountlist *mntdump;
static struct exportslist *exports;
static int type = 0;
char who[9] = "???";
int xdr_mntdump(), xdr_exports();

int
inform_clients(message_from_server)
char *message_from_server;
{
	register struct mountlist *mntp;
	register struct exportslist *exp;
	register struct grouplist *grp;
	struct	utmp utmp[USERS];
	extern char *optarg;
	extern int optind;
	register int rpcs = 0;
	char ch;
	char *host;
	int estat;
	int sline;
	CLIENT *cl;
	FILE *f;
	char broadcastmsg[BUFSIZ];
	char	hostname[256];


	host = "localhost";

	/*
	 * First try tcp, then drop back to udp if tcp is unavailable 
	 * (an old version of mountd perhaps).  Using tcp is preferred
	 * because it can handle arbitrarily long export lists.
	 */
	cl = clnt_create(host, RPCPROG_MNT, RPCMNT_VER1, "tcp");
	if (cl == NULL) {
		cl = clnt_create(host, RPCPROG_MNT, RPCMNT_VER1, "udp");
		if (cl == NULL) {
			clnt_pcreateerror(host);
			exit(1);
		}
	}


	if (estat = clnt_call(cl, RPCMNT_DUMP, xdr_void, 0, 
			      xdr_mntdump, (char *)&mntdump, TIMEOUT)) {
	  fprintf(stderr, "Can't do Mountdump rpc: %s\n",
		  clnt_sperrno(estat));
	  exit(1);
	}

	if((f = fopen("/var/adm/utmp", "r")) == NULL) {
		fprintf(stderr, "Cannot open /var/adm/utmp\n");
		exit(1);
	}
	sline = ttyslot(2); /* 'utmp' slot no. of sender */
	fread((char *)utmp, sizeof(struct utmp), USERS, f);
	fclose(f);
	if (sline)
		strncpy(who, utmp[sline].ut_name, sizeof(utmp[sline].ut_name));
	gethostname(hostname, sizeof (hostname));
	sprintf(broadcastmsg,"%s@%s: %s\n", who, hostname,  message_from_server);

	/*
	 * There could be a large delay trying to send message to
	 * a number of clients. So the message sending process is
	 * being detached from the parent process.
	 */
	if (becomeDaemon())
		/*
		 * msgs are sent using a child process to avoid
		 * shutdown from hanging due to communication delays
		 * when sending out the rwall message
		 */
		broadcast_msg(mntdump, broadcastmsg);
#if DEBUG
	printf("Parent exiting..\n");
#endif
      }

/*
 * Xdr routine for retrieving the mount dump list
 */
xdr_mntdump(xdrsp, mlp)
	XDR *xdrsp;
	struct mountlist **mlp;
{
	register struct mountlist *mp;
	register struct mountlist *tp;
	register struct mountlist **otp;
	int val, val2;
	int bool;
	char *strp;

	*mlp = (struct mountlist *)0;
	if (!xdr_bool(xdrsp, &bool))
		return (0);
	while (bool) {
		mp = (struct mountlist *)malloc(sizeof(struct mountlist));
		if (mp == NULL)
			return (0);
		mp->ml_left = mp->ml_right = (struct mountlist *)0;
		strp = mp->ml_host;
		if (!xdr_string(xdrsp, &strp, RPCMNT_NAMELEN))
			return (0);
		strp = mp->ml_dirp;
		if (!xdr_string(xdrsp, &strp, RPCMNT_PATHLEN))
			return (0);

		/*
		 * Build a binary tree on sorted order of either host or dirp.
		 * Drop any duplications.
		 */
		if (*mlp == NULL) {
			*mlp = mp;
		} else {
			tp = *mlp;
			while (tp) {
				val = strcmp(mp->ml_host, tp->ml_host);
				val2 = strcmp(mp->ml_dirp, tp->ml_dirp);
				switch (type) {
				case ALL:
					if (val == 0) {
						if (val2 == 0) {
							free((caddr_t)mp);
							goto next;
						}
						val = val2;
					}
					break;
				case DIRS:
					if (val2 == 0) {
						free((caddr_t)mp);
						goto next;
					}
					val = val2;
					break;
				default:
					if (val == 0) {
						free((caddr_t)mp);
						goto next;
					}
					break;
				};
				if (val < 0) {
					otp = &tp->ml_left;
					tp = tp->ml_left;
				} else {
					otp = &tp->ml_right;
					tp = tp->ml_right;
				}
			}
			*otp = mp;
		}
next:
		if (!xdr_bool(xdrsp, &bool))
			return (0);
	}
	return (1);
      }

/*
 * Xdr routine to retrieve exports list
 */
xdr_exports(xdrsp, exp)
	XDR *xdrsp;
	struct exportslist **exp;
{
	register struct exportslist *ep;
	register struct grouplist *gp;
	int bool, grpbool;
	char *strp;

	*exp = (struct exportslist *)0;
	if (!xdr_bool(xdrsp, &bool))
		return (0);
	while (bool) {
		ep = (struct exportslist *)malloc(sizeof(struct exportslist));
		if (ep == NULL)
			return (0);
		ep->ex_groups = (struct grouplist *)0;
		strp = ep->ex_dirp;
		if (!xdr_string(xdrsp, &strp, RPCMNT_PATHLEN))
			return (0);
		if (!xdr_bool(xdrsp, &grpbool))
			return (0);
		while (grpbool) {
			gp = (struct grouplist *)malloc(sizeof(struct grouplist));
			if (gp == NULL)
				return (0);
			strp = gp->gr_name;
			if (!xdr_string(xdrsp, &strp, RPCMNT_NAMELEN))
				return (0);
			gp->gr_next = ep->ex_groups;
			ep->ex_groups = gp;
			if (!xdr_bool(xdrsp, &grpbool))
				return (0);
		}
		ep->ex_next = *exp;
		*exp = ep;
		if (!xdr_bool(xdrsp, &bool))
			return (0);
	}
	return (1);
      }


/*
 * Print the binary tree in inorder so that output is sorted.
 */
broadcast_msg(mp, message)
     struct mountlist *mp;
     char *message;
{

  if (mp == NULL)
    return;
  if (mp->ml_left)
    broadcast_msg(mp->ml_left, message);
  
  rwall(mp->ml_host, message);
  
  if (mp->ml_right)
    broadcast_msg(mp->ml_right, message);
}




char *path;

rwall(client, msg)
char *client;
char *msg;
{
	int msize;
	char buf[BUFSIZ];
	register i;
	FILE *f;
	int hflag;
	char *machine, *user, *domain;
	struct hostent *hp;
	pid_t pid;

	sprintf(buf, "%s", msg);
	msize = strlen(buf);
	path = buf;
#ifdef DEBUG
	printf ("sending rwall message to  %s: \n",  client);
	printf ( "%s\n", path);
#endif
	hflag = 1;

	if ((hp = gethostbyname(client)) == NULL)
				fprintf(stderr, "%s is unknown host\n",
				    client);
	else 
		doit(hp);
	return;

      
}

/*
 * Clnt_call to a host that is down has a very long timeout
 * waiting for the portmapper, so we use rmtcall instead.   Since pertry
 * value of rmtcall is 3 secs, make timeout here 4 secs so that
 * you get 1 try.
 */
doit(hp)
	struct hostent *hp;
{
	struct sockaddr_in server_addr;
	int socket, port;
	struct timeval timeout;
	enum clnt_stat stat;
	CLIENT *client;

	socket = RPC_ANYSOCK;
	timeout.tv_usec = 0;
	timeout.tv_sec = 4; /* change 4 to 8; for 2 tries */
	bcopy(hp->h_addr, &server_addr.sin_addr, hp->h_length);
	server_addr.sin_family = AF_INET;
	server_addr.sin_port =  0;
	stat = pmap_rmtcall(&server_addr, WALLPROG, WALLVERS, WALLPROC_WALL,
	    xdr_wrapstring, &path,  xdr_void, NULL, timeout, &port);
	if (stat != RPC_SUCCESS) {
		/*fprintf(stderr, "Couldn't contact %s:\n ", hp->h_name);*/
	        /* no associated terminal, no IO possible */
		clnt_perrno(stat);
	}
}



/*
 *
 * becomeDaemon... (imported from ASE)
 *
 * this routine turns the caller into a daemon
 * by forking and then disassociating the child from
 * the parent.  The parent then kills itself and the child becomes
 * the orphaned child of init(8) and returns.  
 *
 */

static pid_t pid;       /* pid from fork daemon */
static pid_t pgid;      /* process group ID of daemon */
static void sigExit();


#include <sys/resource.h>


int
becomeDaemon(void)
{

  int i;
  struct sigaction *pSigAction;


    /*
     * separate from the parent...
     */

    if ((pid = fork()) == -1)
      {
	fprintf(stderr, "becomeDaemon: fork failed");
        return(0);
      }
    else if (pid != 0)  /* I'm the parent, so uh, I'll be leaving now... */
      {
        return(0);
      }

    /*
     * if I made it here, I must be the child.
     */

    /*
     * Set umask so that files the daemon creates have limited access.
     * Also, the daemon changes it's working directory to root so it
     * won't stop the directory it's currently running on from failing
     * umount(8).
     */

#ifdef NOTYET           /* must get rid of ALL relative paths first */
    umask (077);

    if (chdir("/"))
      {
        frpintf(stderr, "daemon can't chdir to root");
        exit(1);
      }
#endif

    /*
     * Close all my parent's open files.
     */

    for (i = getdtablesize()-1; i >= 0; --i)
      close(i);
    /*
     * set up the daemon's signal handling
     */

    pSigAction = (struct sigaction *) malloc(sizeof(struct sigaction));

    pSigAction->sa_handler = SIG_IGN;
    pSigAction->sa_mask = (sigset_t) 0;
    pSigAction->sa_flags = 0;

    for (i = 1; i < NSIG; i++)
      sigaction(i, pSigAction, NULL);

    pSigAction->sa_handler = SIG_DFL;   /* default action for I/O */
    sigaction(SIGIO, pSigAction, NULL);

    /* exit on these... */

    pSigAction->sa_handler = sigExit;
    sigaction(SIGTERM, pSigAction, NULL);       /* softwre termination sig */
    sigaction(SIGHUP, pSigAction, NULL);        /* terminal disconnect sig */

    /*
     * run the daemon at a favorable priority.  0 = neutral
     */

    if (setpriority(PRIO_PROCESS, getpid(), 0) == -1)
      {
	fprintf(stderr, "daemon can't set process priority");
        free(pSigAction);
        return(1);
      }

    /*
     * The setsid(2) below begins a new session, meaning the daemon is
     * disassociated from it's parents process group and made lord
     * of a new one.
     *
     * Note: after the setsid(2) we should have no controlling tty to
     * bother us.  In all subsequent file opens this daemon should
     * include the open(2) flag O_NOCTTY to avoid tty control.
     */

    if ((pgid = setsid()) == -1)
      {
	fprintf(stderr, "daemon can't set process group");
        exit(1);
      }

    free(pSigAction);

    return(1);

} /* becomeDaemon */




static void
sigExit()
{

  exit(0);      /* no clean up for the moment... */

} /* sigExit */



