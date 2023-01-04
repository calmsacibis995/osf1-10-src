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
static char	*sccsid = "@(#)$RCSfile: w.c,v $ $Revision: 4.2.7.3 $ (DEC) $Date: 1993/11/23 16:33:08 $";
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
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

#ifndef lint
char copyright[] =
"@(#) Copyright (c) 1980 Regents of the University of California.\n\
 All rights reserved.\n";
#endif	/* not lint */

/*
 * w - print system status (who and what)
 */

#include <sys/secdefines.h>
#if SEC_BASE
#include <sys/security.h>
#include <prot.h>

extern priv_t *privvec();
#endif

#include <sys/types.h>
#include <dirent.h>
#include <sys/table.h>
#include <sys/stat.h>
#include <sys/user.h>
#include <sys/proc.h>
#include <sys/ioctl.h>
#include <sys/fcntl.h>
#include <stdlib.h>
#include <mach.h>
#include <time.h>
#include <stdio.h>
#include <utmp.h>
#include <locale.h>
#include <string.h>
#ifdef KJI
#include <NLchar.h>
#endif

#include <nl_types.h>
#include "w_msg.h"

nl_catd	catd;

#define	CATGETS(num, str)	catgets(catd, MS_W, num, str)
#ifdef SEC_BASE
#define CATGETS_SEC(num,str)	catgets(catd,MS_W_SEC,num,str)
#endif /* SEC_BASE */

#define NMAX sizeof(utmp->ut_name)
#define LMAX sizeof(utmp->ut_line)

#define ARGWIDTH	33	/* # chars left on 80 col crt for args */

struct pr {
	short	w_pid;			/* proc.p_pid */
	char	w_flag;			/* proc.p_flag */
	short	w_size;			/* proc.p_size */
	long	w_seekaddr;		/* where to find args */
	long	w_lastpg;		/* disk address of stack */
	int	w_igintr;		/* INTR+3*QUIT, 0=die, 1=ign, 2=catch */
	time_t	w_time;			/* CPU time used by this process */
	time_t	w_ctime;		/* CPU time used by children */
	dev_t	w_tty;			/* tty device of process */
	int	w_uid;			/* uid of process */
	char	w_comm[15];		/* user.u_comm, null terminated */
	char	w_args[ARGWIDTH+1];	/* args if interesting process */
} *pr;
int	nproc;

dev_t	tty;
int	uid;
char	doing[520];		/* process attached to terminal */
time_t	proctime;		/* cpu time of process in doing */
#define	LSCALE		1000	/* Taken from h/kernel.h */

#define	DIV60(t)	((t+30)/60)    /* x/60 rounded */ 
#define	IGINT		(1+3*1)		/* ignoring both SIGINT & SIGQUIT */

int	userfmt;
int	ttyfmt;
int	ttylfmt;
int	fromfmt;
int	loginfmt;
int	idlefmt;
int	jcpufmt;
int	pcpufmt;
int	whatfmt;

#define	USERFMT		8
#define	TTYFMT		3
#define	TTYLFMT		10
#define	FROMFMT		16
#define	LOGINFMT	7
#define	IDLEFMT		6
#define	JCPUFMT		6
#define	PCPUFMT		6
#define	WHATFMT		4

#ifdef DEBUG
#define	WOPTSTR	"dfhlsuwm"
#else
#define	WOPTSTR	"fhlsuwm"
#endif
#define UPOPTSTR "mw"

char	*getargs();
time_t	findidle();
void	filarray();

#ifdef DEBUG
int	debug;			/* true if -d flag: debugging output */
#endif
int	ttywidth = 80;		/* width of tty */
int	header = 1;		/* true if -h flag: don't print heading */
int	lflag = 1;		/* true if -l flag: long style output */
int	prfrom = 1;		/* true if not -f flag: print host from */
int	login;			/* true if invoked as login shell */
time_t	idle;			/* number of minutes user is idle */
int	nusers;			/* number of users logged in now */
char	*sel_user;		/* login of particular user selected */
char	firstchar;		/* first char of name of prog invoked as */
time_t	jobtime;		/* total cpu time visible */
time_t	now;			/* the current time of day */
time_t	uptime;			/* time of last reboot & elapsed time since */
int	np;			/* number of processes currently active */
struct	utmp *utmp;
struct	proc mproc;
int	use_mach_factor;
struct	user	up;

#define max(a, b)	((a) > (b) ? (a) : (b))

main(argc, argv)
int	argc;
char	**argv;
{
	int		days, hrs, mins;
	register int	i, j;
	char		*cp;
	register int	curpid, empty;
	struct winsize	win;
	int		c;
	int		opterror = 0;
	struct tbl_loadavg load;
	struct tbl_sysinfo sysinfo;
	extern int	optind;
	char		*optstr;

	catd = catopen(MF_W, NL_CAT_LOCALE);

	(void)setlocale(LC_ALL, "");

#if SEC_BASE
	set_auth_parameters(argc, argv);
	initprivs();
	if (forceprivs(privvec(SEC_DEBUG, -1), (priv_t *) 0)) {
		fprintf(stderr, CATGETS_SEC(PRIV,
			"%s: insufficient privileges\n"), command_name);
		exit(1);
	}
#endif
	login = (argv[0][0] == '-');
	cp = rindex(argv[0], '/');
	firstchar = login ? argv[0][1] : (cp==0) ? argv[0][0] : cp[1];
	cp = argv[0];	/* for Usage */
	if (firstchar == 'w')
		optstr = WOPTSTR;
	else
		optstr = UPOPTSTR;

	while ((c = getopt(argc, argv, optstr)) != EOF)
		switch (c) {
#ifdef DEBUG
		case 'd':
			debug++;
			break;
#endif
		case 'f':
			prfrom = !prfrom;
			break;

		case 'h':
			header = 0;
			break;

		case 'l':
			lflag++;
			break;

		case 's':
			lflag = 0;
			break;

		case 'u':
			firstchar = c;
			optstr = UPOPTSTR;
			break;

		case 'w':
			firstchar = c;
			optstr = WOPTSTR;
			break;

		case 'm':
			use_mach_factor = 1;
			break;

		default:
			opterror++;
		}

	switch ((argc - optind)) {
	case	0:
		break;
	case	1:
		sel_user = argv[optind];
		break;
	default:
		opterror++;
	}

	if (opterror) {
		if (firstchar == 'w')
			fprintf(stderr,CATGETS(WUSAGE, "Usage: %s [ -hlsfmuw ] [ user ]\n"), cp);
		else
			fprintf(stderr,CATGETS(UPUSAGE, "Usage: %s [ -m ]\n"), cp);
		exit(1);
	}

	if (firstchar == 'w') {
		readpr();
		if (ioctl(1, TIOCGWINSZ, &win) != -1 && win.ws_col > 70)
			ttywidth = win.ws_col;
	}

	/*
	 * calculate the field sizes for the information we are going
	 * print out
	 */
	userfmt = strlen(CATGETS(H_USER, "User"));
	userfmt = max(userfmt, USERFMT);

	ttyfmt = strlen(CATGETS(H_TTY, "tty"));
	ttyfmt = max(ttyfmt, TTYFMT);
	ttylfmt = max(ttyfmt, TTYLFMT);

	fromfmt = strlen(CATGETS(H_FROM, "from"));
	fromfmt = max(fromfmt, FROMFMT);

	loginfmt = strlen(CATGETS(H_LOGIN, "login@"));
	loginfmt = max(loginfmt, LOGINFMT);

	idlefmt = strlen(CATGETS(H_IDLE, "idle"));
	idlefmt = max(idlefmt, IDLEFMT);

	jcpufmt = strlen(CATGETS(H_JCPU, "JCPU"));
	jcpufmt = max(jcpufmt, JCPUFMT);

	pcpufmt = strlen(CATGETS(H_PCPU, "PCPU"));
	pcpufmt = max(pcpufmt, PCPUFMT);

	whatfmt = strlen(CATGETS(H_WHAT, "what"));
	whatfmt = max(whatfmt, WHATFMT);

	time(&now);
	if (header) {
		struct utmp	boot_time;

		/* Print time of day */
		(void)prtat(&now);

		/*
		 * Print how long system has been up.
		 */
		if (table(TBL_SYSINFO, 0, &sysinfo, 1, sizeof(sysinfo)) < 0) {
			fprintf(stderr,CATGETS(NOBOOT, "Cannot find boot time\n"));
			exit(1);
		}

		uptime = now - sysinfo.si_boottime;

		uptime += 30;
		days = uptime / (60*60*24);
		uptime %= (60*60*24);
		hrs = uptime / (60*60);
		uptime %= (60*60);
		mins = uptime / 60;

		printf("  %s", CATGETS(UP, "up"));
		switch (days) {
		case	0:
			break;
		case	1:
			printf(" %d %s,", days, CATGETS(DAY, "day"));
			break;
		default:
			printf(" %d %s,", days, CATGETS(DAYS, "days"));
		}
		if (hrs > 0 && mins > 0) {
			printf(" %2d:%02d,", hrs, mins);
		} else {
			switch (hrs) {
			case	0:
				break;
			case	1:
				printf(" %d %s,", hrs, CATGETS(HOUR, "hr"));
				break;
			default:
				printf(" %d %s,", hrs, CATGETS(HOURS, "hrs"));
			}
			switch (mins) {
			case	0:
				break;
			case	1:
				printf(" %d %s,", mins, CATGETS(MINUTE, "min"));
				break;
			default:
				printf(" %d %s,", mins, CATGETS(MINUTES, "mins"));
			}
		}

		/* Print number of users logged in to system */
		while ((utmp = getutent()) != NULL) {
			if (utmp->ut_type == USER_PROCESS)
				nusers++;
		}
		if (nusers == 1)
			printf("  %d %s", nusers, CATGETS(USER, "user"));
		else
			printf("  %d %s", nusers, CATGETS(USERS, "users"));

		/*
		 * Print 1, 5, and 15 minute load averages.
		 */

		if (table(TBL_LOADAVG, 0, (char *)&load, 1, sizeof(load)) != -1) {
			long	*mf;
			union avenrun {
				long	l[3];
				double	d[3];
			} *ave;

			if (use_mach_factor) {
				printf(",  %s:", CATGETS(MACHFACTOR, "Mach factor"));
				mf = load.tl_mach_factor;
				for (i = 0; i < 3; i++) {
					if (i > 0)
						printf(",");
					printf(" %.2f", (float)(*(mf + i)) / (float)LSCALE);
				}
			} else {
				printf(",  %s:", CATGETS(LOADAVE, "load average"));
				ave = (union avenrun *)&load.tl_avenrun;
				for (i = 0; i < 3; i++) {
					if (i > 0)
						printf(",");
					if (load.tl_lscale != 0)
						printf(" %.2f", (float)ave->l[i] / (float)load.tl_lscale);
					else
						printf(" %.2f", ave->d[i]);
				}
			}
		}

		printf("\n");
		if (firstchar == 'u')	/* if this was uptime(1), finished */
			exit(0);

		/* Headers for rest of output */
		printf("%-*.*s", userfmt, userfmt, CATGETS(H_USER, "User"));

		if (lflag && !prfrom)
			printf(" %-*.*s", ttylfmt, ttylfmt,
							CATGETS(H_TTY, "tty"));
		else
			printf(" %-*.*s", ttyfmt, ttyfmt, CATGETS(H_TTY, "tty"));

		if (prfrom)
			printf(" %-*.*s", fromfmt, fromfmt,
							CATGETS(H_FROM, "from"));
		if (lflag)
			printf(" %-*.*s", loginfmt, loginfmt,
						CATGETS(H_LOGIN, "login@"));

		printf(" %*.*s", idlefmt, idlefmt, CATGETS(H_IDLE, "idle"));

		if (lflag)
			printf(" %*.*s %*.*s", jcpufmt, jcpufmt,
						 CATGETS(H_JCPU, "JCPU"),
						 pcpufmt, pcpufmt,
						 CATGETS(H_PCPU, "PCPU"));

		printf(" %-*.*s\n", whatfmt, whatfmt, CATGETS(H_WHAT, "what"));

		fflush(stdout);
	}


	setutent();
	for (;;) {	/* for each entry in utmp */
		if ((utmp = getutent()) == NULL) {
			exit(0);
		}
		if (utmp->ut_type != USER_PROCESS)
			continue;

		if (sel_user && strncmp(utmp->ut_name, sel_user, NMAX) != 0)
			continue;	/* we wanted only somebody else */

#if SEC_MAC
		if (gettty() == -1)
			continue;
#else
		gettty();
#endif
		jobtime = 0;
		proctime = 0;
		strcpy(doing, "-");	/* default act: normally never prints */
		empty = 1;
		curpid = -1;
		idle = findidle();
		for (i=0; i<np; i++) {	/* for each process on this tty */
			if (tty != pr[i].w_tty)
				continue;
			jobtime += pr[i].w_time + pr[i].w_ctime;
			proctime += pr[i].w_time;
#ifdef DEBUG
			/* 
			 * Meaning of debug fields following proc name is:
			 * & by itself: ignoring both SIGINT and QUIT.
			 *		(==> this proc is not a candidate.)
			 * & <i> <q>:   i is SIGINT status, q is quit.
			 *		0 == DFL, 1 == IGN, 2 == caught.
			 * *:		proc pgrp == tty pgrp.
			 */
			 if (debug) {
				printf("\t\t%d\t%s", pr[i].w_pid, pr[i].w_args);
				if ((j=pr[i].w_igintr) > 0)
					if (j==IGINT)
						printf(" &");
					else
						printf(" & %d %d", j%3, j/3);
				printf("\n");
			}
#endif
			if (empty && pr[i].w_igintr!=IGINT) {
				empty = 0;
				curpid = -1;
			}
			if (pr[i].w_pid>curpid &&
			   (pr[i].w_igintr != IGINT || empty)){
				curpid = pr[i].w_pid;
				strcpy(doing, lflag ? pr[i].w_args : pr[i].w_comm);
			}
		}
		putline();
	}
}

/* figure out the major/minor device # pair for this tty */
#if SEC_MAC
/* gettty may fail to be able to stat destination terminal due to MAC */
int
#endif
gettty()
{
	char ttybuf[20];
	struct stat statbuf;

	ttybuf[0] = 0;
	strcpy(ttybuf, "/dev/");
	strcat(ttybuf, utmp->ut_line);
#if SEC_MAC
	if (stat(ttybuf, &statbuf) < 0)
		return -1;
#else
	stat(ttybuf, &statbuf);
#endif
	tty = statbuf.st_rdev;
	uid = statbuf.st_uid;
#if SEC_MAC
	return 0;
#endif
}

/*
 * putline: print out the accumulated line of info about one user.
 */
putline()
{
	int	tm;
	int	len;
	int	width = ttywidth - 1;

	/* print login name of the user */
	printf("%-*.*s ", userfmt, userfmt, utmp->ut_name);
	width -= userfmt + 1;

	/* print tty user is on */
	if (lflag && !prfrom) {
		/* long form: all (up to) ttylfmt chars */
		printf("%-*.*s", ttylfmt, ttylfmt, utmp->ut_line);
		width -= ttylfmt;
	 } else {
		/* short form: skipping 'tty' if there */
		if (utmp->ut_line[0]=='t' && utmp->ut_line[1]=='t' && utmp->ut_line[2]=='y')
			printf("%-*.*s", ttyfmt, ttyfmt, &utmp->ut_line[3]);
		else
			printf("%-*.*s", ttyfmt, ttyfmt, utmp->ut_line);
		width -= ttyfmt;
	}

	if (prfrom) {
		printf(" %-*.*s", fromfmt, fromfmt, utmp->ut_host);
		width -= fromfmt + 1;
	}

	if (lflag) {
		/* print when the user logged in */
		printf(" ");
		len = prtat(&utmp->ut_time);
		if (len < loginfmt)
			printf("%*s", loginfmt - len, " ");
		width -= loginfmt + 1;
	}

	/* print idle time */
	printf(" ");
	if (idle >= 36 * 60)
		printf("%2d%s", (idle + 12 * 60) / (24 * 60), CATGETS(DAYS, "days"));
	else {
		len = prttime(idle);
		if (len < idlefmt)
			printf("%*s", idlefmt - len, " ");
	}
	width -= idlefmt + 1;

	if (lflag) {
		/* print CPU time for all processes & children */
		printf(" ");
		len = prttime(jobtime);
		if (len < jcpufmt)
			printf("%*s", jcpufmt - len, " ");
		width -= jcpufmt + 1;
		/* print cpu time for interesting process */
		printf(" ");
		len = prttime(proctime);
		if (len < pcpufmt)
			printf("%*s", pcpufmt - len, " ");
		width -= pcpufmt + 1;
	}

	/* what user is doing, either command tail or args */
	printf(" %-.*s\n", width-1, doing);
	fflush(stdout);
}

/* find & return number of minutes current tty has been idle */
time_t
findidle()
{
	struct stat stbuf;
	long lastaction;
	time_t diff;
	char ttyname[20];

	bzero ((char *)&stbuf,sizeof(struct stat));
	strcpy(ttyname, "/dev/");
	strncpy(ttyname+5, utmp->ut_line, LMAX);
	if (!strncmp(ttyname+5,":0",2))
		strncpy(ttyname+5,"console",8);
	stat(ttyname, &stbuf);
	time(&now);
	lastaction = stbuf.st_atime;
	diff = now - lastaction;
	diff = DIV60(diff);
	if (diff < 0) diff = 0;
	return(diff);
}

/*
 * prttime prints a time in hours and minutes or minutes and seconds.
 */
prttime(tim)
time_t tim;
{
	struct tm p;

	if (tim >= 60) {
		printf("%3d:%02d", tim/60, tim%60);
		return(6);
	}
	if (tim > 0) {
		printf("%4s%2d", " ", tim);
		return(6);
	}
	return(0);
}

/* prtat prints a 12 hour time given a pointer to a time of day */
prtat(time)
long *time;
{
	struct tm *p;
	register int hr, pm;
	char	s[TIMELEN];

	p = localtime(time);
	strftime(s, TIMELEN, "%H:%M", p);
	printf(s);
	return(strlen(s));
}

/*
 * readpr finds and reads in the array pr, containing the interesting
 * parts of the proc and user tables for each live process.
 * We only accept procs whos controlling tty has a pgrp equal to the
 * pgrp of the proc.  This accurately defines the notion of the current
 * process(s), but because of time skew, we always read in the tty struct
 * after reading the proc, even though the same tty struct may have been
 * read earlier on.
 */
readpr()
{
	int			pn, mf, addr, c;
	int			i;
	struct tbl_procinfo	pi;
	task_t			task;
	unsigned int		count;
	task_basic_info_data_t	ti;
	thread_array_t		thread_table;
	unsigned int		table_size;
	thread_basic_info_t	thi;
	thread_basic_info_data_t thi_data;

	/*
	 *	Try to use TBL_PROCINFO call instead of reading
	 *	proc table.
	 */
	nproc = table(TBL_PROCINFO, 0, (char *)0, 32767, 0);
	if (nproc == -1)
		nproc = 0;
	pr = (struct pr *)calloc(nproc, sizeof (struct pr));
	np = 0;

	for (pn=0; pn<nproc; pn++) {
		(void)table(TBL_PROCINFO, pn, (char *)&pi, 1,
				sizeof(pi));
		if (pi.pi_status == PI_EMPTY ||
		    pi.pi_status == PI_ZOMBIE ||
		    pi.pi_pgrp == 0 || pi.pi_ttyd == -1)
			continue;
#if SEC_MAC
		if (!w_proc_dominate(pi.pi_pid))
			continue;
#endif
		if (table(TBL_UAREA, pi.pi_pid, (char *)&up, 1,
				sizeof(struct user)) != 1)
			continue;
		pr[np].w_pid = pi.pi_pid;
		pr[np].w_flag = pi.pi_flag;

		pr[np].w_igintr = (((int)up.u_signal[2]==1) +
				  2*((int)up.u_signal[2]>1) +
				  3*((int)up.u_signal[3]==1)) +
				  6*((int)up.u_signal[3]>1);

		if (task_by_unix_pid(task_self(), pi.pi_pid, &task) != KERN_SUCCESS) {
			pi.pi_status = PI_ZOMBIE;
			continue;
		}

                count = TASK_BASIC_INFO_COUNT;
                if (task_info(task, TASK_BASIC_INFO, (task_info_t)&ti, &count)
                    != KERN_SUCCESS) {
                        pi.pi_status = PI_ZOMBIE;
			continue;
                }

		pr[np].w_time = ti.user_time.seconds + ti.system_time.seconds;

		(void)task_threads(task, &thread_table, &table_size);
		
		thi = &thi_data;

		for (i = 0; i < table_size; i++) {
			count = THREAD_BASIC_INFO_COUNT;
			if (thread_info(thread_table[i], THREAD_BASIC_INFO,
			    (thread_info_t)thi, &count) == KERN_SUCCESS) {
				pr[np].w_time += thi->user_time.seconds;
				pr[np].w_time += thi->system_time.seconds;
			}
		}

		pr[np].w_ctime = up.u_cru.ru_utime.tv_sec +
				 up.u_cru.ru_stime.tv_sec;

		pr[np].w_tty = pi.pi_ttyd;
		pr[np].w_uid = pi.pi_uid;
		strncpy(pr[np].w_comm, pi.pi_comm, sizeof(pr[np].w_comm));
		pr[np].w_comm[sizeof(pr[np].w_comm)] = '\0';
		/*
		 * Get args if there's a chance we'll print it.
		 * Cant just save pointer: getargs returns static place.
		 */
		pr[np].w_args[0] = 0;
		strncpy(pr[np].w_args, getargs(&pr[np]), ARGWIDTH);
		if (pr[np].w_args[0]==0 || pr[np].w_args[0]=='-' &&
		    pr[np].w_args[1]<=' ' || pr[np].w_args[0] == '?') {
			strcat(pr[np].w_args, " (");
			strcat(pr[np].w_args, pr[np].w_comm);
			strcat(pr[np].w_args, ")");
		}
		np++;
		(void) vm_deallocate(task_self(), (vm_address_t) thread_table,
				(vm_size_t) (table_size * sizeof(thread_t)));

	}
}

/*
 * getargs: given a pointer to a proc structure, use table to 
 * try to reconstruct the arguments.
 */
char *
getargs(p)
struct pr *p;
{
	int	arg_length;
	char	*cp, *ap;
	char	arguments_buf[4096];
	char	*arguments = arguments_buf;
	int	arguments_size = 4096;

	bzero(arguments, arguments_size);
	if ((arg_length = table(TBL_ARGUMENTS, p->w_pid, arguments, 1, arguments_size)) != 1) {
		arguments[0] = 0;
		return(arguments);
	}
	ap = arguments;

	/* Find length of total arguments */
	for (cp = &arguments[arguments_size - 1];
	     (*cp == '\0' || *cp == ' ') && cp > ap; --cp)
		;

	arg_length = cp - ap + 1;

	/* Concat arguments */
	for (cp = ap; cp < &arguments[arg_length]; cp++)
		if (*cp == '\0')
			*cp = ' ';

	if (ap[0] == '-' || ap[0] == '?' || ap[0] <= ' ') {
		arguments[0] = 0;
		return(arguments);
	} else {
		return(ap);
	}

}
