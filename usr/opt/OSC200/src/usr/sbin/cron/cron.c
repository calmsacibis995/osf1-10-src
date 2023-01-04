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
static char rcsid[] = "@(#)$RCSfile: cron.c,v $ $Revision: 4.2.7.6 $ (DEC) $Date: 1993/12/21 21:11:53 $";
#endif
/*
 * HISTORY
 */
/*
 * OSF/1 1.2
 */
/*
 * COMPONENT_NAME: (CMDOPER) commands needed for basic system needs
 *
 * FUNCTIONS: cron 
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * 1.30  com/cmd/cntl/cron/cron.c, , bos320, 9134320c 8/18/91 20:41:06
 * cron.c	4.1 10:13:55 7/12/90 SecureWare 
 */
 
/*
 *  cron is a daemon that runs jobs at requested times.
 */                                                                   

#include <sys/secdefines.h>

#include <sys/types.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <dirent.h>
#include <pwd.h>
#include <grp.h>
#include <stdio.h>
#include <locale.h>
#include <langinfo.h>
#include <fcntl.h>
#include <time.h>
#include <sys/time.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <ctype.h>
#include <nl_types.h>
#include <string.h>

#include "cron.h"
#include "cron_msg.h"
nl_catd catd;
#define	MSGSTR(Num,Str) catgets(catd,MS_CRON,Num,Str)
#ifdef SEC_BASE
#define MSGSTR_SEC(Num,Str) catgets(catd,MS_CRON_SEC,Num,Str)
#endif

#if SEC_BASE
#include <sys/security.h>
#include <prot.h>

#if SEC_MAC
extern char	*cron_jobname();
#endif
#if SEC_MAC || SEC_NCAV
extern char	*cron_getlevel();
#endif
#endif /* SEC_BASE */

#define MAIL		"/usr/bin/mail" /* mail program to use */
#define CONSOLE		"/dev/console"	/* where to write error messages when cron dies	*/

#define TMPINFILE	"/tmp/crinXXXXXX"  /* file to put stdin in for cmd  */
#define	TMPDIR		"/tmp"
#define	PFX		"crout"
#define TMPOUTFILE	"/tmp/croutXXXXXX" /* file to place stdout, stderr */

#define INMODE		00400		/* mode for stdin file	*/
#define OUTMODE		00600		/* mode for stdout file */
#define ISUID		06000		/* mode for verifing at jobs */

#define INFINITY	2147483647L	/* upper bound on time	*/
#define CUSHION		120L
#define	MAXRUN		25		/* max total jobs allowed in system */
#define ZOMB		100		/* proc slot used for mailing output */

#define	JOBF		'j'
#define	NICEF		'n'
#define	USERF		'u'
#define WAITF		'w'

#define	DEFAULT		0
#define	LOAD		1

#define BADCD		"can't change directory to the crontab directory."
#define NOREADDIR	"can't read the crontab directory."

#define BADJOBOPEN	"unable to read your at job."
#define BADSHELL	"because your login shell isn't /usr/bin/sh, you can't use cron."
#define BADSTAT		"can't access your crontab file.  Resubmit it."
#define CANTCDHOME	"can't change directory to your home directory.\nYour commands will not be executed."
#define CANTEXECSH	"unable to exec the shell for one of your commands."
#define EOLN		"unexpected end of line"
#define NOREAD		"can't read your crontab file.  Resubmit it."
#define NOSTDIN		"unable to create a standard input file for one of your crontab commands.\nThat command was not executed."
#define OUTOFBOUND	"number too large or too small for field"
#define STDERRMSG	"\n\n*************************************************\nCron: The previous message is the standard output\n      and standard error of one of your cron commands.\n"
#define STDOUTERR	"one of your commands generated output or errors, but cron was unable to mail you this output.\nRemember to redirect standard output and standard error for each of your commands."
#define UNEXPECT	"unexpected symbol found"
#undef	sleep

struct event {	
	time_t time;	/* time of the event	*/
	short etype;	/* what type of event; 0=cron, 1=at	*/
	char *cmd;	/* command for cron, job name for at	*/
	struct usr *u;	/* ptr to the owner (usr) of this event	*/
	struct event *link; 	/* ptr to another event for this user */
	union { 
		struct { /* for crontab events */
			char *minute;	/*  (these	*/
			char *hour;	/*   fields	*/
			char *daymon;	/*   are	*/
			char *month;	/*   from	*/
			char *dayweek;	/*   crontab)	*/
			char *login;	/* login id 	*/
			char *input;	/* ptr to stdin	*/
		} ct;
		struct { /* for at events */
			short exists;	/* for revising at events	*/
			int eventid;	/* for el_remove-ing at events	*/
		} at;
	} of; 
};

struct usr {	
	char *name;	/* name of user (e.g. "kew")	*/
	char *home;	/* home directory for user	*/
	int uid;	/* user id	*/
	int gid;	/* group id	*/
#ifdef ATLIMIT
	int aruncnt;	/* counter for running jobs per uid */
#endif
#ifdef CRONLIMIT
	int cruncnt;	/* counter for running cron jobs per uid */
#endif
	int ctid;	/* for el_remove-ing crontab events */
	short ctexists;	/* for revising crontab events	*/
	struct event *ctevents;	/* list of this usr's crontab events */
	struct event *atevents;	/* list of this usr's at events */
	struct usr *nextusr; 
#if SEC_MAC || SEC_NCAV
	char *seclevel_ir;	/* security level of job */
#endif
};	/* ptr to next user	*/

struct	queue
{
	int njob;	/* limit */
	int nice;	/* nice for execution */
	int nwait;	/* wait time to next execution attempt */
	int nrun;	/* number running */
}	
	qd = {100, 2, 60},		/* default values for queue defs */
	qt[NQUEUE];

struct	queue	qq;
int	wait_time = 60;

struct	runinfo
{
	pid_t	pid;
	short	que;
	struct  usr *rusr;		/* pointer to usr struct */
	char 	*outfile;	/* file where stdout & stderr are trapped */
}	rt[MAXRUN];

int msgfd;		/* file descriptor for fifo queue */
int ecid=1;		/* for giving event classes distinguishable id names 
			   for el_remove'ing them.  MUST be initialized to 1 */
short jobtype;		/* at or batch job */
int delayed;		/* is job being rescheduled or did it run first time */
int notexpired;		/* time for next job has not come */
int cwd;		/* current working directory */
int running;		/* zero when no jobs are executing */
struct event *next_event;	/* the next event to execute	*/
struct usr *uhead;	/* ptr to the list of users	*/
struct usr *ulast;	/* ptr to last usr table entry */
struct usr *find_usr();
void timeout(int);
time_t init_time,num();
extern char *xmalloc();
extern void putenv();
time_t next_time();
extern daemon();	/* From libutil */

/* user's default environment for the shell */
char homedir[100]="HOME=";
char logname[50]="LOGNAME=";
char *envinit[]={
	homedir,logname,"PATH=:/usr/bin","SHELL=/usr/bin/sh",0};
extern char **environ;


/*
 * NAME: cron
 *
 * FUNCTION: executes commands automatically 
 * 		The jobs which are executed by the cron daemon are:
 * 			at jobs, batch jobs, cron jobs, sync
 * 			ksh at jobs, csh at jobs
 * EXECUTION ENVIRONMENT:
 *      cron daemon is executed at the system startup time and never exits.
 *
 * RETURNS: none
 */

main(argc,argv)
char **argv;
{
	time_t t,t_old;
	time_t last_time;
	time_t ne_time;		/* amt of time until next event execution */
	time_t lastmtime = 0L;
	struct usr *u,*u2;
	struct event *e,*e2,*eprev;
	struct stat buf;
	long seconds;
	pid_t rfork;
	int id;			/* event id */
	int time_zone;		/* time zone */
	struct tm *time_data;

begin:					/* detach from console */
	if (daemon(1,1) == -1) {
		sleep((unsigned)30);
		goto begin; 
	}

	(void) setlocale(LC_ALL,"");
	catd = catopen(MF_CRON,NL_CAT_LOCALE);

#if SEC_BASE
	cron_init(argc, argv);
	cron_secure_mask();
#else
	umask((mode_t)022);
#endif
	signal(SIGHUP, SIG_IGN);
	signal(SIGINT, SIG_IGN);
	signal(SIGQUIT, SIG_IGN);

	initialize();
	quedefs(DEFAULT);	/* load default queue definitions */
	msg(MSGSTR(MS_CRSTART, "*** cron started ***   pid = %ld"),getpid());
	timeout(0);	/* set up alarm clock trap */
	t_old = time((time_t *) 0);
	last_time = t_old;
	time_data = localtime(&t_old);
	time_zone = time_data->tm_isdst;

	while (TRUE) {			/* MAIN LOOP	*/
		t = time((time_t *) 0);
		time_data = localtime(&t);
		if((t_old > t) || (t-last_time > CUSHION) ||
			( time_zone != time_data->tm_isdst)) {
			/* the time was set backwards or forward */
			/* or daylight savings time was changed */
			time_zone = time_data->tm_isdst;
			el_delete();
			u = uhead;
			while (u!=NULL) {
				rm_ctevents(u);
				e = u->atevents;
				while (e!=NULL) {
					free((void *)e->cmd);
					e2 = e->link;
					free((void *)e);
					e = e2; 
				}
				u2 = u->nextusr;
				u = u2; 
			}
			close(msgfd);
			initialize();
			t = time((time_t *) 0); 
		}
		t_old = t;
		if (next_event == NULL)
			if (el_empty()) ne_time = INFINITY;
			else {	
				next_event = (struct event *) el_first();
				ne_time = next_event->time - t; 
			}
		else ne_time = next_event->time - t;
#ifdef DEBUGX
		if(next_event != NULL)
			printf("next_time=%ld  %s",
				next_event->time,ctime(&next_event->time));
#endif
		seconds = (ne_time < (long) 0) ? (long) 0 : ne_time;
		if(ne_time > (long) 0)
			idle(seconds);
		if(notexpired) {
			notexpired = 0;
			last_time = INFINITY;
			continue;
		}
		if(stat(QUEDEFS,&buf))
			msg(MSGSTR(MS_NOQUEDEFS,"cannot stat QUEDEFS file"));
		else
			if(lastmtime != buf.st_mtime) {
				quedefs(LOAD);
				lastmtime = buf.st_mtime;
			}
		last_time = next_event->time;	/* save execution time */
		ex(next_event);
		switch(next_event->etype) {
		/* add cron or sync event back into the main event list */
		case CRONEVENT:
#ifdef SYNC
		case SYNCEVENT:
#endif
			if(delayed) {
				delayed = 0;
				break;
			}
			next_event->time = next_time(next_event, (time_t)0);
                        if (next_event->time == last_time) {
                        /* Don't schedule event for same time twice.    */
                        /* Schedule it for next next time.              */
                        next_event->time = next_time(next_event,
                                (time_t)last_time + 15);
                        }
			if (next_event->etype == CRONEVENT)
				id = (next_event->u)->ctid; 
			else
				id = 0;
			el_add(next_event, next_event->time, id);
			break;
		/* remove at or batch job from system */
		default:
			eprev=NULL;
			e=(next_event->u)->atevents;
			while (e != NULL)
				if (e == next_event) {
					if (eprev == NULL)
						(e->u)->atevents = e->link;
					else	eprev->link = e->link;
					free((void *)e->cmd);
					free((void *)e);
					break;	
				}
				else {	
					eprev = e;
					e = e->link; 
				}
			break;
		}
		next_event = NULL; 
	}
}



/*
 * NAME: initialize
 *
 * FUNCTION: initialization of the date areas which are used by the cron.
 * 		o initialize event queue.
 * 		o add sync event
 * 		o check FIFO queue status
 *              o read directories, create user list and add events to the
 *                main event list.
 * 		o log file open.
 * RETURNS: none
 */

initialize()
{

	static int flag = 0;

#ifdef DEBUGX
	printf("in initialize\n");
#endif
	init_time = time((time_t *) 0);
	el_init(8,init_time,(long)(60*60*24),10);


#ifdef SYNC
	 /* Add sync event that will get executed once a minute.
	  */
	sync_event();
#endif


#if SEC_BASE
	cron_set_communications(FIFO, 0);
#else
	if(access(FIFO,04)==-1 && mknod(FIFO,S_IFIFO|0600,0)!=0)
		crabort(MSGSTR(MS_NOFIFO1,"cannot access fifo queue"));
#endif
	if((msgfd = open(FIFO, O_RDWR)) < 0) {
		perror(MSGSTR(MS_BANGOPEN,"! open"));
		crabort(MSGSTR(MS_NOFIFO2,"cannot create fifo queue"));
	}

	/* read directories, create users list,
	   and add events to the main event list	*/
	uhead = NULL;
	read_dirs();
	next_event = NULL;
	if(flag)
		return;
#if SEC_BASE
	create_file_securely(ACCTFILE, AUTH_SILENT, "cron event log");
#endif
    /* cron should write to log ONLY if it exists */
        if(freopen(ACCTFILE,"a",stdout) == NULL)
		fprintf(stderr,MSGSTR(MS_NOWRITE,"cannot write to %s\n"),ACCTFILE);
	close((int)fileno(stderr));
	dup(1);
	/* this must be done to make popen work....i dont know why */
	freopen("/dev/null","r",stdin);
	flag = 1;
}


/*
 * NAME: read_dirs
 *
 * FUNCTION: read directories, create user list and add main event list.
 *
 * EXECUTION ENVIRONMENT:
 * 		o /var/spool/cron/crontabs
 * 		o /var/spool/cron/atjobs
 * RETURNS: none
 */

read_dirs()
{
	DIR *dir;
	int mod_ctab(), mod_atjob();

#ifdef DEBUGX
	printf("Read Directories\n");
#endif
	if (chdir(CRONDIR) == -1) crabort(MSGSTR(MS_BADCD,BADCD));
	cwd = CRON;
#if SEC_MAC
	cron_existing_jobs(MSGSTR(MS_NOREADDIR, NOREADDIR), mod_ctab, 1);
#else
	if ((dir=opendir("."))==NULL)
		crabort(MSGSTR(MS_NOREADDIR,NOREADDIR));
	dscan(dir,mod_ctab);
	closedir(dir);
#endif
	if(chdir(ATDIR) == -1) {
		msg(MSGSTR(MS_NOCHDIR,"Cannot change to the atjobs directory."));
		return;
	}
	cwd = AT;
#if SEC_MAC
	cron_existing_jobs(MSGSTR(MS_NOREADAT, "cannot read atjobs directory"),
				mod_atjob, 0);
#else
	if ((dir=opendir("."))==NULL) {
		msg(MSGSTR(MS_NOREADAT,"cannot read atjobs directory"));
		return; 
	}
	dscan(dir,mod_atjob);
	closedir(dir);
#endif
}

#if !SEC_MAC
/*
 * NAME: dscan
 *
 * FUNCTION: read directories specified by the df, 
 *           create user list and add main event list.
 *
 * RETURNS: none
 */
dscan(df,fp)
DIR	*df;
int	(*fp)();
{

	struct dirent *dn;
	char		name[PATH_MAX+1];

/*	rewinddir(df);*/         /* I don't think this is necessary */
	dn = readdir(df);     /* read . */
	dn = readdir(df);     /* read .. */
	while(( dn = readdir(df) ) != NULL ) {
		if(dn->d_fileno == 0)
			continue;
		strncpy(name,dn->d_name,dn->d_namlen);
		name[dn->d_namlen] = '\0'; /* just in case */
		(*fp)(name);
	}
}
#endif /* !SEC_MAC */

/*
 * NAME: mod_ctab
 *
 * FUNCTION: read crontab file specified by name.
 * 		o checks the status of the crontabfile(CRONDIR/name)
 * 		o If name is a new user then reserve a user data area.
 * 			and read crontab file.
 * 		o If 'name' user already exists, and if he didn't have a 
 * 		  cronevent entry , create it and read crontab file.
 * 		o If 'name' user already exists, and if he have a  cronevents
 * 		  already, remove the existing crontab enty and read crontab 
 *                file.
 *
 * RETURNS: none
 */
mod_ctab(name)
char	*name;
{

	struct	passwd	*pw;
	struct	stat	buf;
	struct  usr     *u;
	char	namebuf[PATH_MAX+1];
	char	*pname;

	if((pw=getpwnam(name)) == NULL)
		return;
#if SEC_MAC
	pname = cron_jobname(cwd, CRON, CRONDIR, name, namebuf);
#else
	if(cwd != CRON) {
		strcat(strcat(strcpy(namebuf, CRONDIR),"/"),name);
		pname = namebuf;
	} else
		pname = name;
#endif
	if(stat(pname,&buf)) {
		mail(name,MSGSTR(MS_BADSTAT,BADSTAT),2);
		unlink(pname);
		return;
	}

	if((u=find_usr(name)) == NULL) {
#ifdef DEBUGX
		printf("new user (%s) with a crontab\n",name);
#endif
		u = (struct usr *) xmalloc(sizeof(struct usr));
		u->name = xmalloc(strlen(name)+1);
		strcpy(u->name,name);
		u->home = xmalloc(strlen(pw->pw_dir)+1);
		strcpy(u->home,pw->pw_dir);
		u->uid = pw->pw_uid;
		u->gid = pw->pw_gid;
#if SEC_MAC || SEC_NCAV
		u->seclevel_ir = cron_getlevel();
#endif
		u->ctexists = TRUE;
		u->ctid = ecid++;
		u->ctevents = NULL;
		u->atevents = NULL;
#ifdef ATLIMIT
		u->aruncnt = 0;
#endif
#ifdef CRONLIMIT
		u->cruncnt = 0;
#endif
		u->nextusr = uhead;
		uhead = u;
		readcron(u);
	}
	else {
		u->uid = pw->pw_uid;
		u->gid = pw->pw_gid;
#if SEC_MAC || SEC_NCAV
		u->seclevel_ir = cron_getlevel();
#endif
		if(strcmp(u->home,pw->pw_dir) != 0) {
			free(u->home);
			u->home = xmalloc(strlen(pw->pw_dir)+1);
			strcpy(u->home,pw->pw_dir);
		}
		u->ctexists = TRUE;
		if(u->ctid == 0) {
#ifdef DEBUGX
			printf("%s now has a crontab\n",u->name);
#endif
			/* user didnt have a crontab last time */
			u->ctid = ecid++;
			u->ctevents = NULL;
			readcron(u);
			return;
		}
#ifdef DEBUGX
		printf("%s has revised his crontab\n",u->name);
#endif
		rm_ctevents(u);
		el_remove(u->ctid,0);
		readcron(u);
	}
}


/*
 * NAME: mod_atjob
 *
 * FUNCTION: read at job file specified by name.
 * 		o checks the status of the crontabfile(CRONDIR/name)
 * 		o If 'name' is a new user then reserve a user data area.
 * 			and read at jobs file.
 * 		o If 'name' user already exists,  then add new at jobs entry
 * 			and read at jobs file.
 *
 * RETURNS: none
 */
mod_atjob(name)
char	*name;
{

	char	*tmpptr, *ptr;
	time_t	tim;
	struct	passwd	*pw;
	struct	stat	buf;
	struct  usr     *u;
	struct	event	*e;
	char	namebuf[PATH_MAX+1];
	char	*pname;

	tmpptr = name;			/* name = root.61326488.a */
	ptr = strchr(tmpptr,'.');	/* bypass login id in name */
	if (ptr == NULL)
		return;
	ptr++;
	if(((tim=num(&ptr)) == 0) || (*ptr != '.'))
		return;
	ptr++;
	if(!isalpha((int)*ptr))
		return;
	jobtype = *ptr - 'a';
#if SEC_MAC
	pname = cron_jobname(cwd, AT, ATDIR, name, namebuf);
#else
	if(cwd != AT) {
		strcat(strcat(strcpy(namebuf,ATDIR),"/"),name);
		pname = namebuf;
	} else
		pname = name;
#endif

	if(stat(pname,&buf) || jobtype >= NQUEUE-1) {
		unlink(pname);
		return;
	}

	if(!(buf.st_mode & ISUID)) {
		unlink(pname);
		return;
	}

	if((pw=getpwuid(buf.st_uid)) == NULL)
		return;
	if((u=find_usr(pw->pw_name)) == NULL) {
#ifdef DEBUGX
		printf("new user (%s) with an at job = %s\n",pw->pw_name,name);
#endif
		u = (struct usr *) xmalloc(sizeof(struct usr));
		u->name = xmalloc(strlen(pw->pw_name)+1);
		strcpy(u->name,pw->pw_name);
		u->home = xmalloc(strlen(pw->pw_dir)+1);
		strcpy(u->home,pw->pw_dir);
		u->uid = pw->pw_uid;
		u->gid = pw->pw_gid;
#if SEC_MAC || SEC_NCAV
		u->seclevel_ir = cron_getlevel();
#endif
		u->ctexists = FALSE;
		u->ctid = 0;
		u->ctevents = NULL;
		u->atevents = NULL;
#ifdef ATLIMIT
		u->aruncnt = 0;
#endif
#ifdef CRONLIMIT
		u->cruncnt = 0;
#endif
		u->nextusr = uhead;
		uhead = u;
		add_atevent(u,name,tim);
	}
	else {
		u->uid = pw->pw_uid;
		u->gid = pw->pw_gid;
#if SEC_MAC || SEC_NCAV
		u->seclevel_ir = cron_getlevel();
#endif
		if(strcmp(u->home,pw->pw_dir) != 0) {
			free(u->home);
			u->home = xmalloc(strlen(pw->pw_dir)+1);
			strcpy(u->home,pw->pw_dir);
		}
		e = u->atevents;
		while(e != NULL)
			if(strcmp(e->cmd,name) == 0) {
				e->of.at.exists = TRUE;
				break;
			} else
				e = e->link;
		if (e == NULL) {
#ifdef DEBUGX
			printf("%s has a new at job = %s\n",u->name,name);
#endif
			add_atevent(u,name,tim);
		}
	}
}



/*
 * NAME: add_atevent
 *
 * FUNCTION: reserve a event data area and set the data.
 * 	     adda an event to the main event list(el_add)
 *
 * RETURNS: none
 */
add_atevent(u,job,tim)
struct usr *u;
char *job;
time_t tim;
{
	struct event *e;

	e=(struct event *) xmalloc(sizeof(struct event));
	e->etype = jobtype;
	e->cmd = xmalloc(strlen(job)+1);
	strcpy(e->cmd,job);
	e->u = u;
#ifdef DEBUGX
	printf("add_atevent: user=%s, job=%s, time=%ld\n",
		u->name,e->cmd, e->time);
#endif
	e->link = u->atevents;
	u->atevents = e;
	e->of.at.exists = TRUE;
	e->of.at.eventid = ecid++;
	if(tim < init_time)		/* old job */
		e->time = init_time;
	else
		e->time = tim;
	el_add(e, e->time, e->of.at.eventid); 
}


char line[CTLINESIZE];		/* holds a line from a crontab file	*/
int cursor;			/* cursor for the above line	*/

/*
 * NAME: readcron
 *
 * FUNCTION: readcron reads in a crontab file for a user (u).
 *           The list of events for user u is built, and 
 *           u->events is made to point to this list.
 *           Each event is also entered into the main event list. 
 *
 * RETURNS: none
 */
readcron(u)
struct usr *u;
{

	FILE *cf;		/* cf will be a user's crontab file */
	struct event *e;
	int start,i;
	char *next_field();
	char *get_login_name(), *login_name;
	char namebuf[PATH_MAX+1];
	char *pname;
	int line_num;

	/* read the crontab file */
#if SEC_MAC
	pname = cron_jobname(cwd, CRON, CRONDIR, u->name, namebuf);
#else
	if(cwd != CRON) {
		strcat(strcat(strcpy(namebuf,CRONDIR),"/"),u->name);
		pname = namebuf;
	} else
		pname = u->name;
#endif
	if ((cf=fopen(pname,"r")) == NULL) {
		mail(u->name,MSGSTR(MS_NOREAD, NOREAD),2);
		return; 
	}

	line_num = 0;
	while (fgets(line,CTLINESIZE,cf) != NULL) {
		/* process a line of a crontab file */
		cursor = 0;
		while(line[cursor] == ' ' || line[cursor] == '\t')
			cursor++;
		if(line[cursor] == '#')
			continue;
		e = (struct event *) xmalloc(sizeof(struct event));
		e->etype = CRONEVENT;
		if (line_num == 0) {
			login_name = get_login_name(u->name);
			e->of.ct.login = login_name;
			line_num++;
			/* only login name on 1st line */
			if (strchr(line,' ') == NULL) 
				continue;
		}
		else
			e->of.ct.login = login_name;

		if ((e->of.ct.minute=next_field(0,59,u)) == NULL) goto badline;
		if ((e->of.ct.hour=next_field(0,23,u)) == NULL) goto badline;
		if ((e->of.ct.daymon=next_field(1,31,u)) == NULL) goto badline;
		if ((e->of.ct.month=next_field(1,12,u)) == NULL) goto badline;
		if ((e->of.ct.dayweek=next_field(0,6,u)) == NULL) goto badline;
		if (line[++cursor] == '\0') {
			mail(u->name,MSGSTR(MS_EOLN,EOLN),1);
			goto badline; 
		}
		/* get the command to execute	*/
		start = cursor;
again:
		while ((line[cursor]!='%')&&(line[cursor]!='\n')
		    &&(line[cursor]!='\0') && (line[cursor]!='\\')) cursor++;
		if(line[cursor] == '\\') {
			cursor += 2;
			goto again;
		}
		e->cmd = xmalloc(cursor-start+1);
		strncpy(e->cmd,line+start,cursor-start);
		e->cmd[cursor-start] = '\0';
		/* see if there is any standard input	*/
		if (line[cursor] == '%') {
			e->of.ct.input = xmalloc(strlen(line)-cursor+1);
			strcpy(e->of.ct.input,line+cursor+1);
			for (i=0; i<strlen(e->of.ct.input); i++)
				if (e->of.ct.input[i] == '%') e->of.ct.input[i] = '\n'; 
		}
		else e->of.ct.input = NULL;
		/* have the event point to it's owner	*/
		e->u = u;
		/* insert this event at the front of this user's event list   */
		e->link = u->ctevents;
		u->ctevents = e;
		/* set the time for the first occurance of this event	*/
		e->time = next_time(e, (time_t)0);
		/* finally, add this event to the main event list	*/
		el_add(e,e->time,u->ctid);
#ifdef DEBUGX
		printf("inserting cron event %s at %ld (%s)",
			e->cmd,e->time,ctime(&e->time));
#endif
		continue;

badline: 
		free((void *)e); 
	}

	fclose(cf);
}


/*
 * NAME: mail
 *
 * FUNCTION:  mail mails a user a message.
 *
 * NOTES:     format - 1 error message, 2 - normal message
 *
 * RETURNS: none
 */
mail(usrname,msg,format)
char *usrname,*msg;
int format;
{
	int	p[2]; 

	FILE 	*pip; 
	char 	*temp, *i,*strrchr ();
#if SEC_BASE
	int	child_pid;
	struct runinfo	*rp;
#endif
#if SEC_BASE
	for (rp = rt; rp < rt + MAXRUN; rp++) {
		if (rp->pid == 0)
			break;
	}
	if (rp >= rt + MAXRUN)
		return;

	child_pid = cron_mail_setup(0);
	if (child_pid < 0)
		return;
	
	if (child_pid > 0) {
		rp->pid = child_pid;
		rp->que = ZOMB;
		rp->rusr = (struct usr *) 0;
		rp->outfile = (char *) 0;

		/* decremented in idle() */
		running++;

		return;
	}
#endif /* SEC_BASE */

	temp = xmalloc(strlen(MAIL)+strlen(usrname)+2);
	pip =  popen(strcat(strcat(strcpy(temp,MAIL)," "),usrname),"w");
	if (pip !=NULL) {
		if (format == 1) {
			fprintf(pip,MSGSTR(MS_BADTAB, 
				"Your crontab file has an error in it.\n"));
			i = strrchr(line,'\n');
			if (i != NULL) *i = ' ';
			fprintf(pip, "\t%s\n\t%s\n",line,msg);
			fprintf(pip, MSGSTR(MS_IGNORED,
				"This entry has been ignored.\n")); 
		}
		else 
			fprintf(pip, "Cron: %s\n",msg);

		/* dont want to do pclose because pclose does a wait */
		fclose(pip); 
		/* decremented in idle() */
		running++;
	}
	free(temp);
#if SEC_BASE
	cron_mail_finish();
#endif
	return;
}


/*
 * NAME: next_field
 *
 * FUNCTION: next_field returns a pointer to a string which holds 
 *           the next field of a line of a crontab file.
 *           if (numbers in this field are out of range (lower..upper),
 *             or there is a syntax error) then
 *                 NULL is returned, and a mail message is sent to
 *                 the user telling him which line the error was in.
 *
 * RETURNS: none
 */
char *next_field(lower,upper,u)
int lower,upper;
struct usr *u;
{
	char *s;
	int num,num2,start;

	while ((line[cursor]==' ') || (line[cursor]=='\t')) cursor++;
	start = cursor;
	if (line[cursor] == '\0') {
		mail(u->name,MSGSTR(MS_EOLN,EOLN),1);
		return(NULL); 
	}
	if (line[cursor] == '*') {
		cursor++;
		if ((line[cursor]!=' ') && (line[cursor]!='\t')) {
			mail(u->name,MSGSTR(MS_UNEXPECT, UNEXPECT),1);
			return(NULL); 
		}
		s = xmalloc(2);
		strcpy(s,"*");
		return(s); 
	}
	while (TRUE) {
		if (!isdigit(line[cursor])) {
			mail(u->name,MSGSTR(MS_UNEXPECT, UNEXPECT),1);
			return(NULL); 
		}
		num = 0;
		do { 
			num = num*10 + (line[cursor]-'0'); 
		}			while (isdigit(line[++cursor]));
		if ((num<lower) || (num>upper)) {
			mail(u->name,MSGSTR(MS_OUTOFBOUND,OUTOFBOUND),1);
			return(NULL); 
		}
		if (line[cursor]=='-') {
			if (!isdigit(line[++cursor])) {
				mail(u->name,MSGSTR(MS_UNEXPECT, UNEXPECT),1);
				return(NULL); 
			}
			num2 = 0;
			do { 
				num2 = num2*10 + (line[cursor]-'0'); 
			}				while (isdigit(line[++cursor]));
			if ((num2<lower) || (num2>upper)) {
				mail(u->name,MSGSTR(MS_OUTOFBOUND,OUTOFBOUND),1);
				return(NULL); 
			}
		}
		if ((line[cursor]==' ') || (line[cursor]=='\t')) break;
		if (line[cursor]=='\0') {
			mail(u->name,MSGSTR(MS_EOLN, EOLN),1);
			return(NULL); 
		}
		if (line[cursor++]!=',') {
			mail(u->name,MSGSTR(MS_UNEXPECT, UNEXPECT),1);
			return(NULL); 
		}
	}
	s = xmalloc(cursor-start+1);
	strncpy(s,line+start,cursor-start);
	s[cursor-start] = '\0';
	return(s);
}


/*
 * NAME: next_time
 * 
 * FUNCTION: returns the integer time for the next occurance of event e.
 *           the following fields have ranges as indicated:
 *        PRGM  | min     hour    day of month    mon     day of week
 *        ------|-------------------------------------------------------
 *        cron  | 0-59    0-23        1-31        1-12    0-6 (0=sunday)
 *        time  | 0-59    0-23        1-31        0-11    0-6 (0=sunday)
 * NOTE: this routine is hard to understand.
 * 
 * RETURNS: none
 */
time_t next_time(e, tflag)
struct event *e;
time_t tflag;
{
	struct tm *tm;
	int tm_mon,tm_mday,tm_wday,wday,m,min,h,hr,carry,day,days,
	d1,day1,carry1,d2,day2,carry2,daysahead,mon,yr,db,wd,today;
	time_t t;

        /* If tflag is non_zero,  this value is used as the current     */
        /* time.  This allows scheduling of events according to what    */
        /* time cron thinks it is,  and prevents events from being      */
        /* scheduled twice for the same time.                           */
	if (tflag)
		t = tflag;
	else
		t = time((time_t *) 0);
	tm = localtime(&t);

	tm_mon = next_ge(tm->tm_mon+1,e->of.ct.month) - 1;	/* 0-11 */
	tm_mday = next_ge(tm->tm_mday,e->of.ct.daymon);		/* 1-31 */
	tm_wday = next_ge(tm->tm_wday,e->of.ct.dayweek);	/* 0-6  */
	today = TRUE;
	if ( (strcmp(e->of.ct.daymon,"*")==0 && tm->tm_wday!=tm_wday)
	    || (strcmp(e->of.ct.dayweek,"*")==0 && tm->tm_mday!=tm_mday)
	    || (tm->tm_mday!=tm_mday && tm->tm_wday!=tm_wday)
	    || (tm->tm_mon!=tm_mon)) today = FALSE;

	m = tm->tm_min+1;
	min = next_ge(m%60,e->of.ct.minute);
	carry = (min < m) ? 1:0;
	h = tm->tm_hour+carry;
	hr = next_ge(h%24,e->of.ct.hour);
	carry = (hr < h) ? 1:0;
	if ((!carry) && today) {
		/* this event must occur today	*/
		if (tm->tm_min>min)
			t +=(time_t)(hr-tm->tm_hour-1)*HOUR + 
			    (time_t)(60-tm->tm_min+min)*MINUTE;
		else t += (time_t)(hr-tm->tm_hour)*HOUR +
			(time_t)(min-tm->tm_min)*MINUTE;
		return(t-(long)tm->tm_sec); 
	}

	min = next_ge(0,e->of.ct.minute);
	hr = next_ge(0,e->of.ct.hour);

	/* calculate the date of the next occurance of this event,
	   which will be on a different day than the current day.	*/

	/* check monthly day specification	*/
	d1 = tm->tm_mday+1;
	day1 = next_ge((d1-1)%days_in_mon(tm->tm_mon,tm->tm_year)+1,e->of.ct.daymon);
	carry1 = (day1 < d1) ? 1:0;

	/* check weekly day specification	*/
	d2 = tm->tm_wday+1;
	wday = next_ge(d2%7,e->of.ct.dayweek);
	if (wday < d2) daysahead = 7 - d2 + wday;
	else daysahead = wday - d2;
	day2 = (d1+daysahead-1)%days_in_mon(tm->tm_mon,tm->tm_year)+1;
	carry2 = (day2 < d1) ? 1:0;

	/* based on their respective specifications,
	   day1, and day2 give the day of the month
	   for the next occurance of this event.	*/

	if ((strcmp(e->of.ct.daymon,"*")==0) && (strcmp(e->of.ct.dayweek,"*")!=0)) {
		day1 = day2;
		carry1 = carry2; 
	}
	if ((strcmp(e->of.ct.daymon,"*")!=0) && (strcmp(e->of.ct.dayweek,"*")==0)) {
		day2 = day1;
		carry2 = carry1; 
	}

	yr = tm->tm_year;
	if ((carry1 && carry2) || (tm->tm_mon != tm_mon)) {
		/* event does not occur in this month	*/
		m = tm->tm_mon+1;
		mon = next_ge(m%12+1,e->of.ct.month)-1;		/* 0..11 */
		carry = (mon < m) ? 1:0;
		yr += carry;
		/* recompute day1 and day2	*/
		day1 = next_ge(1,e->of.ct.daymon);
		db = days_btwn(tm->tm_mon,tm->tm_mday,tm->tm_year,mon,1,yr) + 1;
		wd = (tm->tm_wday+db)%7;
		/* wd is the day of the week of the first of month mon	*/
		wday = next_ge(wd,e->of.ct.dayweek);
		if (wday < wd) day2 = 1 + 7 - wd + wday;
		else day2 = 1 + wday - wd;
		if ((strcmp(e->of.ct.daymon,"*")!=0) && (strcmp(e->of.ct.dayweek,"*")==0))
			day2 = day1;
		if ((strcmp(e->of.ct.daymon,"*")==0) && (strcmp(e->of.ct.dayweek,"*")!=0))
			day1 = day2;
		day = (day1 < day2) ? day1:day2; 
	}
	else { /* event occurs in this month	*/
		mon = tm->tm_mon;
		if (!carry1 && !carry2) day = (day1 < day2) ? day1 : day2;
		else if (!carry1) day = day1;
		else day = day2;
	}

	/* now that we have the min,hr,day,mon,yr of the next
	   event, figure out what time that turns out to be.	*/

	days = days_btwn(tm->tm_mon,tm->tm_mday,tm->tm_year,mon,day,yr);
	t += (time_t)(23-tm->tm_hour)*HOUR + (time_t)(60-tm->tm_min)*MINUTE
	    + (time_t)hr*HOUR + (time_t)min*MINUTE + (time_t)days*DAY;
	return(t-(long)tm->tm_sec);
}



#define	DUMMY	100
/*
 * NAME: next_ge
 *
 * FUNCTION: list is a character field as in a crontab file;
 *                for example: "40,20,50-10"
 *           next_ge returns the next number in the list that is
 *           greater than or equal to current.
 *           if no numbers of list are >= current, the smallest
 *           element of list is returned.
 * NOTE: current must be in the appropriate range.   
 *
 * RETURNS: none
 */
next_ge(current,list)
int current;
char *list;
{
	char *ptr;
	int n,n2,min,min_gt;

	if (strcmp(list,"*") == 0) return(current);
	ptr = list;
	min = DUMMY; 
	min_gt = DUMMY;
	while (TRUE) {
		if ((n=(int)num(&ptr))==current) return(current);
		if (n<min) min=n;
		if ((n>current)&&(n<min_gt)) min_gt=n;
		if (*ptr=='-') {
			ptr++;
			if ((n2=(int)num(&ptr))>n) {
				if ((current>n)&&(current<=n2))
					return(current); 
			}
			else {	/* range that wraps around */
				if (current>n) return(current);
				if (current<=n2) return(current); 
			}
		}
		if (*ptr=='\0') break;
		ptr += 1; 
	}
	if (min_gt!=DUMMY) return(min_gt);
	else return(min);
}

/*
 * NAME: del_atjobs
 *
 * FUNCTION:	  delete at jobs event.
 * 		  If the user (usrname) doesn't have any events, then delete
 * 		 the data area of the user and remove the user entry from the
 *  		 user list.
 *
 * RETURNS: none
 */
del_atjob(name,usrname)
char	*name;
char	*usrname;
{

	struct	event	*e, *eprev;
	struct	usr	*u;

	if((u = find_usr(usrname)) == NULL)
		return;
	e = u->atevents;
	eprev = NULL;
	while(e != NULL)
		if(strcmp(name,e->cmd) == 0) {
			if(next_event == e)
				next_event = NULL;
			if(eprev == NULL)
				u->atevents = e->link;
			else
				eprev->link = e->link;
			el_remove(e->of.at.eventid, 1);
			free((void *)e->cmd);
			free((void *)e);
			break;
		} else {
			eprev = e;
			e = e->link;
		}
	if(!u->ctexists && u->atevents == NULL) {
#ifdef DEBUGX
		printf("%s removed from usr list\n",usrname);
#endif
		if(ulast == NULL)
			uhead = u->nextusr;
		else
			ulast->nextusr = u->nextusr;
		free((void *)u->name);
		free((void *)u->home);
#if SEC_MAC || SEC_NCAV
		cron_release_ir(u->seclevel_ir);
#endif
		free((void *)u);
	}
}

/*
 * NAME: del_ctab
 *
 * FUNCTION:     delete crontabs event.
 *               If the user (usrname) doesn't have any events, then delete
 *               the data area of the user and remove the user entry from the
 *               user list.
 *
 * RETURNS: none
 */
del_ctab(name)
char	*name;
{

	struct	usr	*u;

	if((u = find_usr(name)) == NULL)
		return;
	rm_ctevents(u);
	el_remove(u->ctid, 0);
	u->ctid = 0;
	u->ctexists = 0;
	if(u->atevents == NULL) {
#ifdef DEBUGX
		printf("%s removed from usr list\n",name);
#endif
		if(ulast == NULL)
			uhead = u->nextusr;
		else
			ulast->nextusr = u->nextusr;
		free((void *)u->name);
		free((void *)u->home);
#if SEC_MAC || SEC_NCAV
		cron_release_ir(u->seclevel_ir);
#endif
		free((void *)u);
	}
}


/*
 * NAME: rm_ctevents
 *
 * FUNCTION:     delete crontabs event.
 *               free the crontab events entry
 *
 * RETURNS: none
 */
rm_ctevents(u)
struct usr *u;
{
	struct event *e2,*e3;

	/* see if the next event (to be run by cron)
	   is a cronevent owned by this user.		*/
	if ( (next_event!=NULL) && 
	    (next_event->etype==CRONEVENT) &&
	    (next_event->u==u) )
		next_event = NULL;
	e2 = u->ctevents;
	while (e2 != NULL) {
		free((void *)e2->cmd);
		free((void *)e2->of.ct.login);
		free((void *)e2->of.ct.minute);
		free((void *)e2->of.ct.hour);
		free((void *)e2->of.ct.daymon);
		free((void *)e2->of.ct.month);
		free((void *)e2->of.ct.dayweek);
		if (e2->of.ct.input != NULL) free((void *)e2->of.ct.input);
		e3 = e2->link;
		free((void *)e2);
		e2 = e3; 
	}
	u->ctevents = NULL;
}

/*
 * NAME: find_user
 * FUNCTION: find the user entry(uname) from the user list.(uhead)
 * RETURNS: none
 */
struct usr *find_usr(uname)
char *uname;
{
	struct usr *u;

	u = uhead;
	ulast = NULL;
	while (u != NULL) {
#if SEC_MAC
		if (cron_id_match(u->name, uname, u->seclevel_ir))
			return u;
#else
		if (strcmp(u->name,uname) == 0) return(u);
#endif
		ulast = u;
		u = u->nextusr; 
	}
	return(NULL);
}


/*
 * NAME: ex
 * FUNCTION: execute the events
 *           o check the limits
 *           o fork the child process
 *           o Parent process (return to the main )
 *           o child process
 *              If at event, open jobfile for stdin for input the shell script.
 *              If cron event, create a temporary input file for stdin.
 *              create a temporary output file for stdout(croutxxx)
 *              If at event, execute the job(setpenv).(read the cmd from stdin)
 *              If cron event, execute the job(setpenv).(get the cmd from event)
 * RETURNS: none
 */
ex(e)
struct event *e;
{

	register i,j;
	short sp_flag;
	int fd;
	pid_t rfork;
	char *at_cmdfile, *cron_infile;
	char *mktemp();
#if SEC_MAC || SEC_NCAV
	char *cron_tempnam();
#endif
	struct stat buf;
	struct queue *qp;
	struct runinfo *rp;
	char cmd[BUFSIZ];
	char tmpbuf[BUFSIZ];
	time_t	t = time ((time_t *) 0);
	int maxfiles;

#ifdef SYNC
	if (e->etype == SYNCEVENT)
	{
#ifdef DEBUGX
		msg("sync-ing disks");
#endif 
		sync();
		return;
	}
#endif

	qp = &qt[e->etype];	/* set pointer to queue defs */
	if(qp->nrun >= qp->njob) {
		msg(MSGSTR(MS_QLIMAX, "%c queue max run limit reached"),e->etype+'a');
		resched(qp->nwait);
		return;
	}
	for(rp=rt; rp < rt+MAXRUN; rp++) {
		if(rp->pid == 0)
			break;
	}
	if(rp >= rt+MAXRUN) {
		msg(MSGSTR(MS_MAXRUN, "MAXRUN (%d) procs reached"),MAXRUN);
		resched(qp->nwait);
		return;
	}
#ifdef ATLIMIT
	if((e->u)->uid != 0 && (e->u)->aruncnt >= ATLIMIT) {
		msg(MSGSTR(MS_ATLIMIT, "ATLIMIT (%d) reached for uid %d"),
		    ATLIMIT,(e->u)->uid);
		resched(qp->nwait);
		return;
	}
#endif
#ifdef CRONLIMIT
	if((e->u)->uid != 0 && (e->u)->cruncnt >= CRONLIMIT) {
		msg(MSGSTR(MS_CRONLIMIT, "CRONLIMIT (%d) reached for uid %d"),
		    CRONLIMIT,(e->u)->uid);
		resched(qp->nwait);
		return;
	}
#endif
#if SEC_MAC || SEC_NCAV
	rp->outfile = cron_tempnam(TMPDIR, PFX, e->u->seclevel_ir);
#else
	rp->outfile = tempnam(TMPDIR,PFX);
#endif
	if (rp->outfile == (char *) 0) {
		sprintf(tmpbuf, MSGSTR(MS_NOTEMP, "Cannot create output file in %s."), TMPDIR);
		msg(tmpbuf);
		mail((e->u)->name,tmpbuf,2);
		return;
	}

	if((rfork = fork()) == -1) {
		msg(MSGSTR(MS_NOFORK, "cannot fork"));
		resched(wait_time);
		sleep((unsigned)30);
		return;
	}
	if(rfork) {	/* parent process */
		++qp->nrun;
		++running;
		rp->pid = rfork;
		rp->que = e->etype;
#ifdef ATLIMIT
		if(e->etype != CRONEVENT)
			(e->u)->aruncnt++;
#endif
#if ATLIMIT && CRONLIMIT
		else
			(e->u)->cruncnt++;
#else
#ifdef CRONLIMIT
		if(e->etype == CRONEVENT)
			(e->u)->cruncnt++;
#endif
#endif
		rp->rusr = (e->u);
		return;
	}

#if SEC_BASE
	cron_close_files();
#else
	maxfiles = sysconf(_SC_OPEN_MAX);
	for (i=0; i<maxfiles; i++) close(i);
#endif

	if (e->etype != CRONEVENT ) {
		/* open jobfile as stdin to shell */
#if SEC_MAC
		/* the +16 below reflects the space needed to hold a
		 * mld subdirectory name plus trailing slash */
		at_cmdfile = xmalloc(strlen(ATDIR)+strlen(e->cmd)+16+2);
		(void) cron_jobname(cwd, "", ATDIR, e->cmd, at_cmdfile);
#else
		at_cmdfile = xmalloc(strlen(ATDIR)+strlen(e->cmd)+2);
		strcat(strcat(strcpy(at_cmdfile,ATDIR),"/"),e->cmd);
#endif
		if (stat(at_cmdfile,&buf)) exit(1);
		if (!(buf.st_mode&ISUID)) {
			/* if setuid bit off, original owner has 
			   given this file to someone else	*/
			unlink(at_cmdfile);
			exit(1); 
		}
		if (open(at_cmdfile,O_RDONLY) == -1) {
			mail((e->u)->name,MSGSTR(MS_BADJOBOPEN, BADJOBOPEN),2);
			unlink(at_cmdfile);
			exit(1); 
		}
		setbuf (stdin, NULL);	/* Don't buffer */
		unlink(at_cmdfile); 	/* remove at job file */
	}

#if SEC_MAC || SEC_NCAV
	cron_set_user_environment(e->u->seclevel_ir, e->u->name, e->u->uid);
#else
#if SEC_BASE
	cron_set_user_environment(e->u->name, e->u->uid);
#endif
#endif
	/* set correct user and group identification	*/
	if (setgid((e->u)->gid)<0)
		exit(1);
	(void) initgroups((e->u)->name, (e->u)->gid);
	(void) setlogin((e->u)->name);				/*FPM001*/
	if (setuid((e->u)->uid)<0)
		exit(1);

        /* These 4 lines are a kludge - required because initgroups() */
        /* leaves some file descriptors open. Remove these lines of   */
        /* code when initgroups() is fixed                            */
        if (e->etype == CRONEVENT)                              /*FPM002*/
           close(0);                                            /*FPM002*/
        for (i=1; i<5; i++)                                     /*FPM002*/
           close(i);						/*FPM002*/
 
#if SEC_BASE
      	cron_adjust_privileges();
#endif
	sp_flag = FALSE;
	if (e->etype == CRONEVENT) {
		/* check for standard input to command	*/
		if (e->of.ct.input != NULL) {
			cron_infile = mktemp(TMPINFILE);
			if ((fd=creat(cron_infile,(mode_t)INMODE)) == -1) {
				mail((e->u)->name,MSGSTR(MS_NOSTDIN, NOSTDIN),2);
				exit(1); 
			}
			if (write(fd,e->of.ct.input,(unsigned)strlen(e->of.ct.input))
			    != strlen(e->of.ct.input)) {
				mail((e->u)->name,MSGSTR(MS_NOSTDIN, NOSTDIN),2);
				unlink(cron_infile);
				exit(1); 
			}
			close(fd);
			/* open tmp file as stdin input to sh	*/
			if (open(cron_infile,O_RDONLY)==-1) {
				mail((e->u)->name,MSGSTR(MS_NOSTDIN, NOSTDIN),2);
				unlink(cron_infile);
				exit(1); 
			}
			unlink(cron_infile); 
		} /* if stdin for this event is not NULL */
		else if (open("/dev/null",O_RDONLY)==-1) {
			open("/",O_RDONLY);	/* can't fail */
			sp_flag = TRUE;		/* close it later */
		}
	} /* if CRONEVENT */

#if SEC_MAC
	forcepriv(SEC_MULTILEVELDIR);
#endif
	if (creat(rp->outfile,(mode_t)OUTMODE)!=-1) 
		dup(1);		/* stdout, stderr go to outfle */
	else if (open("/dev/null",O_WRONLY)!=-1) 
		dup(1);		/* stdout, stderr go to null   */
	if (sp_flag)
		close(0);	/* stdin got opened on "/", so close it */

	strcat(homedir,(e->u)->home);
	strcat(logname,(e->u)->name);
	environ = envinit;
	if (chdir((e->u)->home)==-1) {
		mail((e->u)->name,MSGSTR(MS_CANTCDHOME, CANTCDHOME),2);
		exit(1); 
	}
	if((e->u)->uid != 0)
		nice(qp->nice);
	if (e->etype == CRONEVENT)
		execl(SHELL,"sh","-c",e->cmd,0);
	else /* type == ATEVENT */
		execl(SHELL,"sh",0);
	mail((e->u)->name,CANTEXECSH,2);
	exit(1);
}


/*
 * NAME: idle
 * FUNCTION: idle for a specified period.
 *           o wait for the specified time. This function divides
 *             the specified time( tyme) and set the alarm.
 *              If some job is running then wait for until it is finised.
 *                If not, wait for the message(queue)
 *              If no job is running wait for the message(queue), too.
 * RETURNS: none
 */
idle(tyme)
long tyme;
{

	long t;
	time_t	now;
	pid_t	pid;
	int	prc;
	long	alm;
	struct	runinfo	*rp;

	t = tyme;
	while(t > 0L) {
		if(running) {
			if(t > wait_time)
				alm = wait_time;
			else
				alm = t;
#ifdef DEBUGX
			printf("in idle - setting alarm for %ld sec\n",alm);
#endif
			alarm((unsigned) alm);
			pid = wait(&prc);
			alarm((unsigned)0);
			if(pid == -1) {
				if(msg_wait())
					return;
			} else {
				for(rp=rt;rp < rt+MAXRUN; rp++)
					if(rp->pid == pid)
						break;
				if(rp >= rt+MAXRUN) {
					msg(MSGSTR(MS_BADPID, "unexpected pid returned %d (ignored)"),pid);
					/* incremented in mail() */
					running--;
				}
				else
					if(rp->que == ZOMB) {
						running--;
						rp->pid = 0;
#if SEC_BASE
						if (rp->outfile) {
							unlink(rp->outfile);
							free(rp->outfile);
						}
#else
						unlink(rp->outfile);
						free(rp->outfile);
#endif
					}
					else
						cleanup(rp,prc);
			}
		} else {
			msg_wait();
			return;
		}
		now = time((time_t *) 0);
		t = next_event->time - now;
	}
}


/*
 * NAME: cleanup
 * FUNCTION: cleanup the standard output of the executed event.
 * RETURNS: none
 */
cleanup(pr,rc)
struct	runinfo	*pr;
{

	int	fd;
	struct	usr	*p;
	struct	stat	buf;
	time_t	t = time ((time_t *) 0);

	--qt[pr->que].nrun;
	pr->pid = 0;
	--running;
	p = pr->rusr;
#if SEC_MAC || SEC_NCAV
	cron_setlevel(p->seclevel_ir);
#endif
#ifdef ATLIMIT
	if(pr->que != CRONEVENT)
		--p->aruncnt;
#endif
#if ATLIMIT && CRONLIMIT
	else
		--p->cruncnt;
#else
#ifdef CRONLIMIT
	if(pr->que == CRONEVENT)
		--p->cruncnt;
#endif
#endif
	if(!stat(pr->outfile,&buf)) 
	{
		if(buf.st_size > 0) 
		{
			if((fd=open(pr->outfile,O_WRONLY)) == -1)
				mail(p->name,MSGSTR(MS_STDOUTERR,STDOUTERR),2);
			else
			{
				lseek(fd,(long) 0,2);
				write(fd,MSGSTR(MS_STDERRMSG, STDERRMSG),(unsigned)strlen(MSGSTR(MS_STDERRMSG, STDERRMSG)));
				close(fd);
				/* mail user stdout and stderr */
#if SEC_MAC
				cron_mail_line(line, MAIL, p->name,
						pr->outfile);
#else
				/*
				 * We have to stick a blank line at the top
				 * of the outfile so mail doesn't get
				 * error msgs confused with mail headers.
				 */
				sprintf(line,
					"(echo; /usr/bin/cat \"%s\") | %s %s\n",
					pr->outfile,MAIL,p->name);
#endif
				if((pr->pid = fork()) == 0) {
#if SEC_BASE
					(void) cron_mail_setup(1);
#endif
					execl(SHELL,"sh","-c",line,0);
					exit(127);

				}
				pr->que = ZOMB;
				running++;

			}
		} 
		else 
		{
			free((void *)pr->outfile);
			unlink(pr->outfile);
		}
	}
}

#define	MSGSIZE	sizeof(struct message)
/*
 * NAME: msg_wait
 * FUNCTION: wait for the message to the FIFO queue.
 * RETURNS: none
 */
msg_wait()
{

	long	t;
	time_t	now;
	struct	stat msgstat;
	struct	message	*pmsg;
	int	cnt;

	if(fstat(msgfd,&msgstat) != 0)
		crabort(MSGSTR(MS_FIFO1, "cannot stat fifo queue"));
	if(msgstat.st_size == 0 && running)
		return(0);
	if(next_event == NULL)
		t = INFINITY;
	else {
		now = time((time_t *) 0);
		t = next_event->time - now;
		if(t <= 0L)
			t = 1L;
	}
#ifdef DEBUGX
	printf("in msg_wait - setting alarm for %ld sec\n", t);
#endif
#if SEC_MAC || SEC_NCAV
	pmsg = (struct message *) cron_set_message(sizeof *pmsg);
	if (pmsg == (struct message *) 0) {
		msg(MSGSTR(MS_MSGALLOC, "can't allocate message buffer"));
		notexpired = 1;
		return 1;
	}
#endif
	alarm((unsigned) t);
#if SEC_MAC || SEC_NCAV
	if (!cron_read_message(msgfd, (char *) pmsg, sizeof *pmsg, &cnt))
#else
	pmsg = &msgbuf;
	if((cnt=read(msgfd,(char *) pmsg,MSGSIZE)) != MSGSIZE)
#endif
	{
		if(errno != EINTR) {
			perror(MSGSTR(MS_READ, "! read"));
			notexpired = 1;
		}
		if(next_event == NULL)
			notexpired = 1;
		return(1);
	}
	alarm((unsigned)0);
	if(pmsg->etype != NULL) {
		switch(pmsg->etype) {
		case AT:
			if(pmsg->action == DELETE)
				del_atjob(pmsg->fname,pmsg->logname);
			else
				mod_atjob(pmsg->fname);
			break;
		case CRON:
			if(pmsg->action == DELETE)
				del_ctab(pmsg->fname);
			else
				mod_ctab(pmsg->fname);
			break;
		default:
			msg(MSGSTR(MS_BADFMT, "message received - bad format"));
			break;
		}
		if (next_event != NULL) {
			if (next_event->etype == CRONEVENT)
				el_add(next_event,next_event->time,(next_event->u)->ctid);
			else /* etype == ATEVENT */
				el_add(next_event,next_event->time,next_event->of.at.eventid);
			next_event = NULL;
		}
		fflush(stdout);
		pmsg->etype = NULL;
		notexpired = 1;
		return(1);
	}
}


/*
 * NAME: timeout
 * FUNCTION: reset the alarm
 * RETURNS: none
 */
void
timeout(int sig)
{
	/* don't really need to reset this, but also used for setup */
	signal(SIGALRM, timeout);
}

/*
 * NAME: crabort
 * FUNCTION: crabort handles exits out of cron
 * RETURNS: none
 */
crabort(mssg)
char *mssg;
{
	/* crabort handles exits out of cron */
	int c;

	/* write error msg to console */
	if ((c=open(CONSOLE,O_WRONLY))>=0) {
		write(c,MSGSTR(MS_ABORT, "cron aborted: "),(unsigned)strlen(MSGSTR(MS_ABORT, "cron aborted: ")));
		write(c,mssg,(unsigned)strlen(mssg));
		write(c,"\n",(unsigned)1);
		close(c); 
	}
	msg(mssg);
	msg(MSGSTR(MS_ABORTHDR, "******* CRON ABORTED ********"));
	exit(1);
}

/*
 * NAME: msg
 * FUNCTION: print out the message with time stamp
 * RETURNS: none
 */
msg(fmt,a,b)
char *fmt;
{

	time_t	t;
	char	timestr[255];

	t = time((time_t *) 0);
	printf("! ");
	printf(fmt,a,b);
	strftime(timestr, 255, nl_langinfo(D_T_FMT), localtime(&t));
	printf(" %s\n",timestr);
	fflush(stdout);
}

/*
 * NAME: resched
 * FUNCTION: reschedule the job to run at a later time(delay time)
 * RETURNS: none
 */
resched(delay)
int	delay;
{
	time_t	nt;

	/* run job at a later time */
	nt = next_event->time + delay;
	if(next_event->etype == CRONEVENT) {
		next_event->time = next_time(next_event, (time_t)0);
		if(nt < next_event->time)
			next_event->time = nt;
		el_add(next_event,next_event->time,(next_event->u)->ctid);
		delayed = 1;
		msg(MSGSTR(MS_CRRESCHED,"rescheduling a cron job"));
		return;
	}
	add_atevent(next_event->u, next_event->cmd, nt);
	msg(MSGSTR(MS_ATRESCHED, "rescheduling at job"));
}

#define	QBUFSIZ		80
/*
 * NAME: quedefs
 * FUNCTION: read or check the quedefs file.
 * ENVIRONMENT:    action DEFAULT - set up default queuedefinitions.
 *                        other   - read the quedef file and set up the queue
 *                                  def data.(qbuf)
 * RETURNS: none
 */
quedefs(action)
int	action;
{
	register i;
	int	j;
	char	name[PATH_MAX+1];
	char	qbuf[QBUFSIZ];
	FILE	*fd;

	/* set up default queue definitions */
	for(i=0;i<NQUEUE;i++) {
		qt[i].njob = qd.njob;
		qt[i].nice = qd.nice;
		qt[i].nwait = qd.nwait;
	}
	if(action == DEFAULT)
		return;
	if((fd = fopen(QUEDEFS,"r")) == NULL) {
		msg(MSGSTR(MS_NOQUEDEFS, "cannot open quedefs file"));
		msg(MSGSTR(MS_NOQDEFS2, "using default queue definitions"));
		return;
	}
	while(fgets(qbuf, QBUFSIZ, fd) != NULL) {
		if((j=qbuf[0]-'a') < 0 || j >= NQUEUE || qbuf[1] != '.')
			continue;
		i = 0;
		while(qbuf[i] != NULL)
		{
			name[i] = qbuf[i];
			i++;
		}
		parsqdef(&name[2]);
		qt[j].njob = qq.njob;
		qt[j].nice = qq.nice;
		qt[j].nwait = qq.nwait;
	}
	fclose(fd);
}

/*
 * NAME: parsqdef
 * FUNCTION:       set the qbuf date from the quedef entry.
 * ENVIRONMENT:    name - format EventType.[(Jobs)j][(Nice)n][(Wait)w]
 *                              ex.  a.2j2n90w
 * RETURNS: none
 */
parsqdef(name)
char *name;
{
	register i;

	qq = qd;
	while(*name) {
		i = 0;
		while(isdigit(*name)) {
			i *= 10;
			i += *name++ - '0';
		}
		switch(*name++) {
		case JOBF:
			qq.njob = i;
			break;
		case NICEF:
			qq.nice = i;
			break;
		case WAITF:
			qq.nwait = i;
			break;
		}
	}
}


#ifdef SYNC
/*
 * NAME: sync_event
 * FUNCTION: add a special event for doing syncs
 *           to cron's main event list. This guarantees that a sync
 *            will occur once per minute.
 *            This code obviates 'sync' entry in root's crontab file.
 * RETURNS: none
 */
sync_event()
{
    struct event *e;


    e = (struct event *) xmalloc(sizeof(struct event));
    e->etype = SYNCEVENT;

     /* Set up a once-a-minute event.
      */
    e->of.ct.minute  = "*";
    e->of.ct.hour    = "*";
    e->of.ct.daymon  = "*";
    e->of.ct.month   = "*";
    e->of.ct.dayweek = "*";

     /* Since sync event is handled specially,
      * many elements of the event structure are unused.
      * In particular, there is no usr structure because
      * the event isn't associated with any particular user.
      */
    e->u = NULL;		/* no user structure */
    e->cmd = NULL;		/* no command */
    e->of.ct.input = NULL;	/* no standard input */
    e->link = NULL;		/* no other events */

     /* Set the time for the first occurence of sync event,
      * then add it to the main event list.
      */
    e->time = next_time(e, (time_t)0);
    el_add(e, e->time, 0);	/* event id not used: just set to '0' */
}
#endif

/*
 * NAME: get_login_name
 * FUNCTION: process the first line of crontab file whether or not there
 *           is a login id there
 * RETURNS: none
 */
char *get_login_name(name)
char *name;
{
	char *s;
	
	/*
	 * process login id  - strchr will return NULL if only 1 
	 * token on line. Meaning login id else login id left off
	 * so use the name from the usr struct.
	 */

	if (strchr(line,' ') == NULL) {  /* only 1 token */
		s = xmalloc(strlen(line)+1);
		strcpy(s,line);

	}
	else {
		s = xmalloc(strlen(name)+1);
		strcpy(s,name);
	}
	return(s);
}

