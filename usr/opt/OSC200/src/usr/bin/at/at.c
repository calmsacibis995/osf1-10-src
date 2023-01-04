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
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
#if !defined(lint) && !defined(_NOIDENT)
static char rcsid[] = "@(#)$RCSfile: at.c,v $ $Revision: 4.2.7.5 $ (DEC) $Date: 1993/12/13 13:46:50 $";
#endif
/*
 * HISTORY
 */
/*
 * COMPONENT_NAME: (CMDOPER) commands needed for basic system needs
 *
 * FUNCTIONS: at
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 * 
 * at.c	 1.43  com/cmd/cntl/cron/at.c, , bos320, 9135320b 8/21/91 14:32:45
 * at.c	4.3 00:41:01 7/12/90 SecureWare 
 */

#include <sys/secdefines.h>
#if SEC_BASE
#include <sys/security.h>

extern priv_t *privvec();
#endif

#include <sys/types.h>
#include <sys/limits.h>
#include <sys/stat.h>
#include <sys/param.h>        /* for BSIZE needed in dirent.h */
#include <dirent.h>
#include <fcntl.h>
#include <pwd.h>
#include <grp.h>
#include <stdio.h>
#include <locale.h>
#include <ctype.h>
#include <stdlib.h>
#include <time.h>
#include <signal.h>
#include <errno.h>
#include <string.h>
#include <ulimit.h>
#include <unistd.h>
#include <nl_types.h>
#include <langinfo.h>
#include "cron.h"
#include "cron_msg.h"
nl_catd catd;
#define	MSGSTR(Num,Str) catgets(catd,MS_CRON,Num,Str)

/* gtime formats */
const char *POSIX_fmts[] = { "%Y"" %m"" %d"" %H"" %M", "%y"" %m"" %d"" %H"" %M", " %m"" %d"" %H"" %M", NULL };

#define YEAR_AND_CENTURY 1

#define TMPFILE		"_at"	/* prefix for temporary files	*/
#if SEC_BASE
#define MSGSTR_SEC(Num,Str) catgets(catd,MS_CRON_SEC,Num,Str)
#define ATMODE		06040	/* Mode for creating files in ATDIR. */
#else
#define ATMODE		06444	/* Mode for creating files in ATDIR. */
#endif
#define BUFSIZE		512	/* for copying files */
#define TIMESIZE	256	/* for making time string */
#define LINESIZE	130	/* for listing jobs */
#define	MAXTRYS		60	/* max trys to create at job file */
#define	MPWDTRYS	15	/* max trys to create at pipe for pwd */

#define BADDATE		"bad date specification"
#define BADMINUTES	"minutes field is too large"
#define CANTCD		"can't change directory to the at directory"
#define CANTCHOWN	"can't change the owner of your job to you"
#if SEC_BASE
#define	CANTCHMOD	"can't change the mode of your job"
#define	INSUFFPRIVS	"at: insufficient privileges"
#endif
#define CANTOPEN  	"can't open job file for you"
#define CANTLINK  	"can't link job file for you"
#define CANTCREATE	"can't create a job for you"
#define CANTCWD		"bad return code from the library routine: getcwd()"
#define CANTPEEK	"you may only look at your own jobs."
#define INVALIDUSER	"you are not a valid user (no entry in /etc/passwd)"
#define	NONUMBER	"proper syntax is:\n\tat -ln\nwhere n is a number"
#define ATNOREADDIR	"can't read the at directory"
#define NOTALLOWED	"you are not authorized to use at.  Sorry."
#define NOTHING		"nothing specified"
#define NOPROTO		"no prototype file"
#define PAST		"it's past that time"
#define CONFOPTS	"conflicting options"
#define NOQUE		"no queue specified"
#define BADQUE		"bad queue specification"
#define NOKSH 		"ksh is currently not available"
#define NOCSH		"csh is currently not available"
#define MALLOC		"out of space, can not continue"
#define BADCHOWN	"at: can not change atjob file owner"
#define ATRMDEL 	"at: only jobs belonging to user: %s may be deleted\n"
#define BSHPATH		"/usr/bin/sh"
#define KSHPATH		"/usr/bin/ksh"
#define CSHPATH		"/usr/bin/csh"

/* This message is mailed to the user with -m flag. */
#define MFLAG_BUFF      1024
#define MAILJOB         "at: Job %s was run.\n"

/*
	this data is used for parsing time
*/
#define	dysize(A) ((((1900+(A)) % 4) == 0 && (1900+(A)) % 100 != 0 || (1900)+(A) % 400 == 0) ? 366 : 365)

/* Current flags */
int	lflag = 0;		/* list atjob files */
int	rflag = 0;		/* remove atjobs files */
int	cflag =	0;		/* execute in c shell */
int	Fflag =	0;		/* suppress deletion info */
int	kflag =	0;		/* execute in korn shell */
int	nflag =	0;		/* list number of files */
int	iflag =	0;		/* interactive delete */
int	uflag =	0;		/* user flag */
int	oflag =	0;		/* list in schedule order */
int	shflag = 0;		/* bourne shell flag - default */
int	qflag = 0;		/* select queue */
int	gmtflag = 0;		/* greenwich mean time */
int	pathflg = 0;		/* pathname instead of standard input */
int	mflag = 0;		/* send mail to the invoking user */
int	tflag = 0;		/* submit the job according to time argument*/

extern int errno;
extern	char	*argp;

char	*nargp;
char	login[UNAMESIZE];
char	argpbuf[PATH_MAX+65];	/* buf desc of time 1023 + 65(time syntax) */
char	pname[PATH_MAX+1];
char	pname1[PATH_MAX+1];
char	jobname[PATH_MAX+1];
char	*job;			/* file name in var/spool/cron/atjobs */
time_t	when, now;
struct	tm	*tp, at, rt, *timeptr;
int	mday[12] =
{
	31,28,31,
	30,31,30,
	31,31,30,
	31,30,31,
};
int	mtab[12] =
{
	0,   31,  59,
	90,  120, 151,
	181, 212, 243,
	273, 304, 334,
};
int     dmsize[12] = {
	31,28,31,30,31,30,31,31,30,31,30,31};

/* end of time parser */

short	jobtype = ATEVENT;		/* set to 1 if batch job */
short	exeflag = FALSE;		/* execute the filename given? */
char	*tfname;			/* temporary file name */
char	*shell = BSHPATH;		/* shell to run under */
char	queue = '\0';			/* queue specified with -q flag */

extern	char *xmalloc();
time_t  num();

extern	int list_aj();
extern	int remove_aj();
extern	void strtolower();
extern	int send_msg();

static int gtime(char *, const char *[]);
static int is_exec( char* );


char* add_delimiters(int year_and_century_flg, char* tmp)
{
    char* new_buf;
    char* delim;
    int i, len, str_len;

    str_len = strlen(tmp);

    if ((new_buf = (char*) malloc(str_len + 8)) == NULL)
        atabort(MSGSTR(MS_MALLOC,MALLOC));

    if (year_and_century_flg) {
        strncpy(new_buf, tmp, 4);
	strcat(new_buf, " ");
	tmp += 4;
	str_len -= 4;
    }

    for(i = 0; i < str_len; i += len) {

	if (*tmp == '.') {
		len = 3;
		delim = "";
	}
	else {
		len = 2;
		delim = " ";
	}

	strncat(new_buf, tmp, len);
	strcat(new_buf, delim);
	tmp += len;
    }

    return (new_buf);
}

main(argc, argv)
int  	argc;
char 	*argv[];
{
	struct passwd 	*nptr;
	int 		user, i, fd, catch(void), list_aj_flag;
	char 		pid[6], timestr[TIMESIZE + 1];
	char 		*pp;
	char 		*mkjobname(),*getuser();
	int  		rc ,st = 0;
	int		c;			/* getopt option */
	struct	tm	*tempdate;
	extern	char	*optarg;
	extern	int	optind, optopt;

	(void) setlocale(LC_ALL,"");
	catd = catopen(MF_CRON,NL_CAT_LOCALE);

	jobname[0] = '\0';
	getnls();

#if SEC_BASE
	set_auth_parameters(argc, argv);
	initprivs();

	if (forceprivs(privvec(SEC_CHMODSUGID,
#if SEC_MAC
				SEC_WRITEUPSYSHI,
#endif
#if SEC_ILB
				SEC_ILNOFLOAT,
#endif
#if SEC_NCAV
				SEC_ALLOWNCAVACCESS,
#endif
				-1), (priv_t *) 0))
		atabort(MSGSTR(MS_ATPRIVS, INSUFFPRIVS));
#if SEC_MAC
	disablepriv(SEC_MULTILEVELDIR);
#endif

	/* Accountability -- don't allow user to submit a job as somebody else */
	if ((user = getluid()) != geteuid())
		atabort(MSGSTR_SEC(AT_SEC_4,
			       "Login UID does not match effective UID.\n"));
	if ((nptr = getpwuid(user)) == NULL)
#else /* !SEC_BASE */
	if ((nptr=getpwuid(user=getuid())) == NULL) 
#endif /* !SEC_BASE */
		atabort(MSGSTR(MS_INVALIDUSER, INVALIDUSER));
	else 
		pp = nptr->pw_name;

	strcpy(login,pp);
	if (!allowed(login,ATALLOW,ATDENY)) 
		atabort(MSGSTR(MS_NOTALLOWED, NOTALLOWED));
	time(&now);

	/*
	 * Interpret command line flags if they exist.
	 */
	while ((c = getopt(argc, argv, ":cFf:iklmnoq:rstu")) != EOF) {
	    switch (c) {
	      case 'c' : /* Select csh */
		cflag = 1; 
	 	jobtype = CSHEVENT;
		shell = CSHPATH;
		break;
	      case 'k' : /* Select ksh */
	        kflag = 1; 
		jobtype = KSHEVENT;
		shell = KSHPATH;
		break;
	      case 's' : /* Select sh */
	 	shflag = 1;
		shell = BSHPATH;
		break;
	      case 'u' : /* delete all for specific user */
		uflag = 1;
		break;
	      case 'F' : /* suppress delete verification */
		Fflag = 1;
		break;
	      case 'f' : /* pathname instead of standard input 	    */
		pathflg = 1;
                                    /* modified to allow files with x perm */
                                    /* (no r perm necessary)           DWD */
		if ((access(optarg, X_OK) != -1) 
		     && (is_exec(optarg)))
			exeflag = TRUE;	/* execute this as a command */
		else
			exeflag = FALSE; /* copy the contents of
					    the file (ala BSD) */
		if (access(optarg, R_OK) != -1 || exeflag) 
			strncpy(jobname, optarg, strlen(optarg));
		break;
	      case 'i' : /* interactive delete */
		iflag = 1;
		break;
	      case 'l' : /* List entries */
		lflag = 1;
		break;
	      case 'n' : /* # of files */
		nflag = 1;
		break;
	      case 'o' : /* scheduled order */
		oflag = 1;
		break;
	      case 'r' : /* Remove entries */
		rflag = 1;
		break;
	      case 'q' : /* Select queue */
		qflag = 1;
		queue = *optarg;
		jobtype = queue - 'a';
		if ((jobtype < ATEVENT) || (jobtype > CSHEVENT))
		    atabort(MSGSTR(MS_BADQUE, BADQUE));
		break;
	      case 'm' : /* Send mail (BSD option, AIX default) */
		mflag = 1;
		break; 
	      case 't' :
		tflag = 1;
		break;

	      case '?':
	      default  : /* Unknown switch */
		usage();
	      }
	}

	argc -= optind;
	argv += optind;

	if (kflag && access(KSH, X_OK) == -1)
		atabort(MSGSTR(MS_NOKSH,NOKSH));

	if (cflag && access(CSH, X_OK) == -1)
		atabort(MSGSTR(MS_NOCSH,NOCSH));

	if ((lflag + rflag + shflag + cflag + kflag) > 1) 
		atabort(MSGSTR(MS_CONFOPTS,CONFOPTS));

	if (rflag) {
		/* remove jobs that are specified */
		if (argc == 0)
			atabort(MSGSTR(MS_NOTHING, NOTHING));
		i=0;
		do {
#if SEC_BASE
 			if (uflag && strcmp(argv[i], login) && !at_authorized())
#else
 			if (uflag && (user != ROOT) && (strcmp(argv[i], login)))
#endif
			{
				fprintf(stderr,MSGSTR(MS_ATRMDEL,ATRMDEL), login);
				exit(1);
			}
			if (iflag) {
				if (uflag)
					rc = remove_aj(CRON_PROMPT|CRON_USER,
						argv[i]);
				else
					if (argv[i] != NULL)
						rc = remove_aj(CRON_PROMPT,argv[i]);
			}
			else {
				if (Fflag)
					if (uflag)
					    rc = remove_aj(CRON_QUIET|CRON_USER,
						argv[i]);
					else
					    rc = remove_aj(CRON_QUIET,argv[i]);
				else
					if (uflag)
					    rc = remove_aj(CRON_NON_VERBOSE|
					      CRON_USER,argv[i]);
					else
					    rc = remove_aj(CRON_NON_VERBOSE,argv[i]);
			}
		}
		while (argv[++i]!=NULL);
		exit(rc); 
	}

	if (lflag) {
		if (*argv == NULL) 
#if SEC_BASE
			if (at_authorized()) {
#else
			if (user == ROOT) {
#endif
				if (oflag)
					rc = list_aj(CRON_SORT_M,NULL,queue);
				else
					rc = list_aj(CRON_SORT_E,NULL,queue);
			}
			else {
				if (oflag)
					rc = list_aj(CRON_SORT_M,login,queue);
				else
					rc = list_aj(CRON_SORT_E,login,queue);
			}
		else {
			rc = -1;
			for (i=0; argv[i]!=NULL; i++) {
#if SEC_BASE
				if (memcmp((void *)argv[i], (void *)login, (size_t)strlen(login)) && !at_authorized())
#else
				if (memcmp((void *)argv[i], (void *)login, (size_t)strlen(login)) && (user != ROOT))
#endif
					continue;
				if (oflag)
					list_aj_flag = CRON_SORT_M;
				else
					list_aj_flag = CRON_SORT_E;
				if ((strchr(argv[i], '.')) != NULL) 
					/* job#'s */
					list_aj_flag |= AT_JOB_ID;
				rc = list_aj(list_aj_flag,argv[i],queue);
			}
			if (rc == -1)
				atabort(MSGSTR(MS_CANTPEEK, CANTPEEK));
		}
		exit(rc); 
	}

	if (nflag) {
		if (*argv == NULL)
			rc = list_aj(CRON_COUNT_JOBS,login,queue);
		else
		{
			rc = -1;
			for (i=0; argv[i]!=NULL; i++) {
#ifdef SEC_BASE
 				if (strcmp(argv[i], login) && !at_authorized())
#else
 				if (strcmp(argv[i], login) && (user != ROOT))
#endif
					continue;
				rc = list_aj(CRON_COUNT_JOBS,argv[i],queue);
			}
			if (rc == -1)
				atabort(MSGSTR(MS_CANTPEEK, CANTPEEK));
		}
		 exit(rc); 
	}

	fflush(stdout);
	/* figure out what time to run the job */

	if(argc == 0 && jobtype != BATCHEVENT)
		atabort(MSGSTR(MS_NOTHING, NOTHING));
	time(&now);
	if(jobtype != BATCHEVENT) {	/* at job */
		nargp = argpbuf;
		i = st;
                                    /* modified to allow files with x perm */
                                    /* (no r perm necessary)           DWD */
		if (!pathflg) {
		    if ((access(argv[argc-1],X_OK) != -1) 
		          && (is_exec(argv[argc-1])))
			exeflag = TRUE;	/* execute this as a command */
		    else
			exeflag = FALSE; /* copy the contents of 
					    the file (ala BSD) */
		    if (access(argv[argc-1],R_OK) != -1 || exeflag) {
			strncpy(jobname,argv[argc-1], strlen(argv[argc-1]));
			argc--;
                    }
		}
		while(i < argc) {
			strcat(nargp,argv[i]);
			strcat(nargp, " ");
			i++;
		}
		if ((argp = malloc(strlen(nargp)+1)) == NULL)
			atabort(MSGSTR(MS_MALLOC,MALLOC));
		strtolower(argp,nargp);
		tp = localtime(&now);
		mday[1] = 28 + leap(tp->tm_year);
		if (tflag) {
			argp[strlen(argp)-1] = '\0';
			if (gtime(argp, POSIX_fmts))
				usage();
		}
		else {
			yyparse();
		}
		atime(&at, &rt);
		mday[1] = 28 + leap(tp->tm_year);
		when = Gtime(&at);
		if(!gmtflag) {
			when += timezone;
			if(localtime(&when)->tm_isdst) /* daylight sav tm */
				when -= 60 * 60;
		}
	} else		/* batch job */
		when = now;

	if(when < now)	/* time has already past */
		atabort(MSGSTR(MS_TOOLATE, "too late"));
	timeptr = localtime(&when);
	fflush(stdout);
	sprintf(pid,"%-5d",getpid());
	tfname=xmalloc(strlen(ATDIR)+strlen(TMPFILE)+7);
	strcpy(tfname, ATDIR);
	strcat(strcat(strcat(tfname, "/"), TMPFILE), pid);
	/* catch SIGINT, HUP, and QUIT signals */
	if (signal(SIGINT,(void(*)(int))catch)==SIG_IGN) signal(SIGINT,SIG_IGN);
	if (signal(SIGHUP,(void(*)(int))catch)==SIG_IGN) signal(SIGHUP,SIG_IGN);
	if (signal(SIGQUIT,(void(*)(int))catch)==SIG_IGN)signal(SIGQUIT,SIG_IGN);
	if (signal(SIGTERM,(void(*)(int))catch)==SIG_IGN)signal(SIGTERM,SIG_IGN);
	if ((fd = open(tfname, O_CREAT|O_EXCL|O_WRONLY, ATMODE)) < 0)
		atabort(MSGSTR(MS_CANTOPEN, CANTOPEN)); /*MSG*/

#if SEC_BASE
	if (fchown(fd, user, getegid()) != 0)
#else
	if (fchown(fd, user, getgid()) != 0)
#endif
	{
		unlink(tfname);
		atabort(MSGSTR(MS_CANTCHOWN, CANTCHOWN)); /*MSG*/
	}

	/* In the previous version, mkjobname() have been called in the back */
	/* of copy(). But, this version needed to call it in the front of    */
	/* copy(), because, copy() needed to know the job name.              */
	/* make a job name out of the time (when) and link it to tfname */
	/*   returns the last component of the job name 		*/
	job = mkjobname (tfname, when);

	close(1);
	dup(fd);		/* stdout is now our temp-file */
	close(fd);
	sprintf(pname, "%s", PROTO);
	sprintf(pname1, "%s.%c", PROTO, 'a'+jobtype);
	copy();
	fflush(stdout);	/* Bug fix, flush job file before sendmsg() */
#if SEC_BASE
	/*
	 * Reset the mode of the job file since the SUID and SGID
	 * bits will have been cleared by writes in copy().
	 */
	if (chmod(tfname, ATMODE)) {
		unlink(tfname);
		atabort(MSGSTR(MS_CANTCHMOD, CANTCHMOD));
	}
#endif
	unlink(tfname);
	send_msg(AT, ADD, job, login);
	(void)strftime(timestr, TIMESIZE, nl_langinfo(D_T_FMT), timeptr);
	fprintf(stderr,MSGSTR(MS_JOBAT, "job %s at %s\n"), job,timestr);
	exit(0);
}

/* 
 * is_exec()
 *   Returns 1 is file has any execute bit set, 0 otherwise.
 *   This is only important if we're being run by root.  Because
 *   access() always returns success for root, we need to confirm
 *   that the file is meant to be executable before we set
 *   the exeflag.
 *   For non-root users, this is wasted cycles.
 */
int
is_exec(char* file)
{
	struct stat statbuf;

	if (stat(file, &statbuf) < 0) {
		return(0);
	}
	return(statbuf.st_mode&(S_IXUSR|S_IXGRP|S_IXOTH));
}

/****************/
char *mkjobname(ttfname,t)
/****************/
char   *ttfname;
time_t t;
{
	int     i;
	char    *name;
	struct passwd *nptr;
	char *p,*getfilenam();
	char pp[UNAMESIZE];

	name = xmalloc(200);

	for (i = 0; i < MAXTRYS; i++) 
	{
		sprintf(name,"%s/%ld.%c", ATDIR, t, 'a'+jobtype);

		/* if file doesn't exist return */
		/*   if it does then try again  */

		if ((nptr=getpwuid(getuid())) == NULL) /* check the user id */
			atabort(MSGSTR(MS_INVALIDUSER, INVALIDUSER));
	
		strcpy(pp, nptr->pw_name);	/* user's crontab file */

		p = strrchr(name,'/');
		p++;
		strcat(pp,".");
		strcat(pp,p);
		*(strrchr (name, '/') + 1) = '\0';
		strcat (name, pp);

		if (link(ttfname,name) != -1)
			return (strrchr (name, '/') + 1);
			else if (errno != EEXIST)
		{
			unlink(ttfname);
			atabort(MSGSTR( MS_CANTLINK, CANTLINK)); /*MSG*/
		}
		t += 1;
	}
	atabort(MSGSTR(MS_QFULL, "queue full"));
}


/****************/
catch(void)
/****************/
{
	unlink(tfname);
	exit(1);
}


/****************/
atabort(msg)
/****************/
char *msg;
{
	fprintf(stderr,MSGSTR(MS_ATERROR,"at: %s\n"),msg);
	exit(1);
}

yywrap()
{
	return(1);
}

yyerror(s1)
char *s1;
{
	if (s1 == NULL)
		atabort(MSGSTR(MS_BADDATE, BADDATE));
	else
		atabort(s1);
}

/*
 * add time structures logically
 */
atime(a, b)
register
struct tm *a, *b;
{
	if ((a->tm_sec += b->tm_sec) >= 60) {
		b->tm_min += a->tm_sec / 60;
		a->tm_sec %= 60;
	}
	if ((a->tm_min += b->tm_min) >= 60) {
		b->tm_hour += a->tm_min / 60;
		a->tm_min %= 60;
	}
	if ((a->tm_hour += b->tm_hour) >= 24) {
		b->tm_mday += a->tm_hour / 24;
		a->tm_hour %= 24;
	}
	a->tm_year += b->tm_year;
	if ((a->tm_mon += b->tm_mon) >= 12) {
		a->tm_year += a->tm_mon / 12;
		a->tm_mon %= 12;
	}
	a->tm_mday += b->tm_mday;
	while (a->tm_mday > mday[a->tm_mon]) {
		a->tm_mday -= mday[a->tm_mon++];
		if (a->tm_mon > 11) {
			a->tm_mon = 0;
			mday[1] = 28 + leap(++a->tm_year);
		}
	}
}

leap(year)
{
	return(((1900+year) % 4 == 0 && (1900+year) % 100 != 0 || (1900+year) % 400 == 0));
}

/*
 * make job file from user enviro + stdin
 *
 * AIX code replaced by (almost) straight SYS V.2 code.
 */
copy()
{
	register int c;
	register FILE *pfp;
	register FILE *xfp;
	register char **ep;
	char	dirbuf[PATH_MAX+1];
	char	tmpchar;
	char *getwd();
	mode_t um;
	char *val;
	char *mailmsg1;		/* temp buffer for -m flag */
	extern char **environ;

	printf(": %s job\n",jobtype ? "batch" : "at");
	for (ep=environ; *ep; ep++) {
		if (strncmp(*ep, "TERMCAP=", 8) == 0)
			continue;
		if (strncmp(*ep, "TERM=", 5) == 0) {
			printf("TERM='dumb'\n");
			printf("export TERM\n");
			continue;
		}
		if ( strchr(*ep,'\'')!=NULL )
			continue;
		if ((val=strchr(*ep,'='))==NULL)
			continue;
		if (strncmp(*ep, "SHELL=", 6) == 0) {
			printf("export SHELL; SHELL='%s'\n", shell);
			continue;
		}
		*val++ = '\0';
		printf("export %s; %s='%s'\n",*ep,*ep,val);
		*--val = '=';
	}

	if (mflag) {
		/* If -m flag was specified, at mails that to the user. */
		if ((mailmsg1 = (char *)malloc(MFLAG_BUFF)) != NULL) {
        		sprintf(mailmsg1, MSGSTR(MS_MAILJOB, MAILJOB), job);
        		printf("echo \"%s\" | mail %s\n", mailmsg1, login);
		}
	}

	if((pfp = fopen(pname1,"r")) == NULL && (pfp=fopen(pname,"r"))==NULL)
		atabort(MSGSTR(MS_NOPROTO, NOPROTO));

	um = umask(0);
	(void) umask(um);
	while ((c = getc(pfp)) != EOF) {
		if (c != '$')
			putchar(c);
		else switch (c = getc(pfp)) {
		case EOF:
			goto out;
		case 'd':
			dirbuf[0] = '\0';
			if (getwd(dirbuf) == NULL)
				atabort(MSGSTR(MS_CANTCWD, CANTCWD));
			printf("%s", dirbuf);
			break;
		case 'l':
			printf("%ld",ulimit(UL_GETFSIZE,-1L));
			break;
		case 'm':
			printf("%o", um);
			break;
		case '<':
			printf("%s << 'QAZWSXEDCRFVTGBYHNUJMIKOLP'\n", shell);
			if (exeflag && (jobname[0] != '\0')) {
				printf("%s\n", jobname);
				break;
			}
			xfp = stdin;		/* job from stdin by default */
			if (jobname[0] != '\0') {
  				/* job from a file */
				if ((xfp=fopen(jobname, "r")) == NULL) {
					perror(jobname);
				}

			}
			while ((c = getc(xfp)) != EOF) {
				tmpchar = c;
				printf("%c",tmpchar);
			}
			/* don't put on single quotes, csh doesn't like it */
			printf("QAZWSXEDCRFVTGBYHNUJMIKOLP\n");
			break;
		case 't':
			printf(":%lu", when);
			break;
		default:
			putchar(c);
		}
	}
out:
	fclose(pfp);
}


/*
 * NAME: usage
 * FUNCTION: display a usage statement to the user
 */
usage()
{
	fprintf(stderr, MSGSTR(MS_USAGE1,
	 	"Usage: at [-c|-k|-s] [-m] [-f file] [-q{a|b|e|f}] \n\
		[ time [date] [inc] | -t [[CC]YY]MMDDhhmm[.SS] ]\n"));
	fprintf(stderr, MSGSTR(MS_USAGE2,
		"       at -l [-o] [-q{a|b|e|f}] [user ...]\n"));
	fprintf(stderr, MSGSTR(MS_USAGE3,
		"       at -r [-Fi] job ... | -u user\n"));
	exit(1);
}

/*
 * NAME: gtime
 *                                                                    
 * FUNCTION: 	Convert ascii time value on command line into a
 *		the number of seconds since 1970.
 *		Accepts several different formats using strptime()
 *
 * 		Returns (time_t) -1 on failure
 */  

int
gtime(char *timestring, const char *candidates[])
{
    char	*p = timestring;
    char	*q;
    char        *new_argp;
    int		century=0;	/* Century delta to add */
    char	*fmt;
    time_t	timbuf;

    for( fmt= (char *)*candidates++; fmt; fmt=(char *)*candidates++) {

        timbuf = time(NULL);	/* Start with now as modification time */
	at = *localtime(&timbuf);	/* Resets to right now */

	/* if the YEAR should contain the Century, then set the 
	   appropriate flag on the argument to add the delimiters in
	   the right place.
	*/
	if (strchr(fmt, 'Y'))
	    new_argp = (char*) add_delimiters(YEAR_AND_CENTURY, timestring);
	else
	    new_argp = (char*) add_delimiters(0, timestring);

	p = new_argp;
	q = strptime(p, fmt, &at);	/* Trial conversion */

	if (!q) continue;			/* Failed */

	if (*q == ' ') q++;

	if (*q == '.')				/* Expecting seconds */
	    q = strptime(q, ".%S", &at);

	if (!q || *q)				/* Format error or extra chars */
	  continue;

	/*
	 * Need to check tm_year for special posix values
	 */
	if ((at.tm_year < 70) &&	/* Can't express times before 1970 */
	    (!strchr(fmt,'Y')))		/* and they didn't use explicit century */
	    at.tm_year += 100;	/* Then assume they really meant 21st century */

	return (0);
    }

    /*
     * Only reach here if no format could effectively convert the time string
     */
    return (1);
}

/*
 * I had to take this code from Silver BL12 gtime() since osf1.2 'at' expects
 * osf1.2 feature in mktime(). In our environment we have to use Silver libc.
 * Function name was renamed to Gname(). - Akira 3/17/'93
 */
/*
 * return time from time structure
 * determine the number of seconds to target time from 1970
 */
time_t
Gtime(ttp)
struct	tm *ttp;
{ 	register time_t i;
	time_t	tv;
	tv = 0;

	for (i = 1970; i < ttp->tm_year+1900; i++)
		tv += dysize(i);
	if (dysize(ttp->tm_year) == 366 && ttp->tm_mon >= 2)
		++tv;
	for (i = 0; i < ttp->tm_mon; ++i)
		tv += dmsize[i];
	tv += ttp->tm_mday - 1;
	tv = 24 * tv + ttp->tm_hour;
	tv = 60 * tv + ttp->tm_min;
	tv = 60 * tv + ttp->tm_sec;
	return(tv);
}

