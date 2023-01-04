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
static char	*sccsid = "@(#)$RCSfile: finger.c,v $ $Revision: 4.3.5.2 $ (DEC) $Date: 1993/10/11 17:07:42 $";
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
/*
 * modifications
 * 
 * Apr-16-91  Mary Walker
 *            -return phone string unformatted    
 *            -only open .plan and .project if they are regular files with
 *             a link count of 1.
 */
/*
 * This is a finger program.  It prints out useful information about users
 * by digging it up from various system files.  It is not very portable
 * because the most useful parts of the information (the full user name,
 * office, and phone numbers) are all stored in the VAX-unused gecos field
 * of /etc/passwd, which, unfortunately, other UNIXes use for other things.
 *
 * There are three output formats, all of which give login name, teletype
 * line number, and login time.  The short output format is reminiscent
 * of finger on ITS, and gives one line of information per user containing
 * in addition to the minimum basic requirements (MBR), the full name of
 * the user, his idle time and office location and phone number.  The
 * quick style output is UNIX who-like, giving only name, teletype and
 * login time.  Finally, the long style output give the same information
 * as the short (in more legible format), the home directory and shell
 * of the user, and, if it exits, a copy of the file .plan in the users
 * home directory.  Finger may be called with or without a list of people
 * to finger -- if no list is given, all the people currently logged in
 * are fingered.
 *
 * The program is validly called by one of the following:
 *
 *	finger			{short form list of users}
 *	finger -l		{long form list of users}
 *	finger -b		{briefer long form list of users}
 *	finger -q		{quick list of users}
 *	finger -i		{quick list of users with idle times}
 *	finger namelist		{long format list of specified users}
 *	finger -s namelist	{short format list of specified users}
 *	finger -w namelist	{narrow short format list of specified users}
 *
 * where 'namelist' is a list of users login names.
 * The other options can all be given after one '-', or each can have its
 * own '-'.  The -f option disables the printing of headers for short and
 * quick outputs.  The -b option briefens long format outputs.  The -p
 * option turns off plans for long format outputs.
 */

#include <sys/secdefines.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <utmp.h>
#include <sys/signal.h>
#include <pwd.h>
#include <stdio.h>
#include <lastlog.h>
#include <ctype.h>
#include <time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <locale.h>

#include "finger_msg.h"
nl_catd catd;
#define MSGSTR(Num, Str) catgets(catd, MS_FINGER, Num, Str)

#define ASTERISK	'*'		/* ignore this in real name */
#define COMMA		','		/* separator in pw_gecos field */
#define COMMAND		'-'		/* command line flag char */
#define SAMENAME	'&'		/* repeat login name in real name */
#define TALKABLE	0220		/* tty is writable if 220 mode */

struct utmp user;
#define NWIDTH	8		/* MAX width of printed login name */
#define LWIDTH	8		/* MAX width of printed line name */
#define HWIDTH	16		/* MAX width of printed line name */
#define NMAX sizeof(user.ut_name)
#define LMAX sizeof(user.ut_line)
#define HMAX sizeof(user.ut_host)

struct person {			/* one for each person fingered */
	char *name;			/* name */
	char tty[LMAX+1];		/* null terminated tty line */
	char host[HMAX+1];		/* null terminated remote host name */
	long loginat;			/* time of (last) login */
	long idletime;			/* how long idle (if logged in) */
	char *realname;			/* pointer to full name */
	char *office;			/* pointer to office name */
	char *officephone;		/* pointer to office phone no. */
	char *homephone;		/* pointer to home phone no. */
	char *random;			/* for any random stuff in pw_gecos */
	struct passwd *pwd;		/* structure of /etc/passwd stuff */
	char loggedin;			/* person is logged in */
	char writable;			/* tty is writable */
	char original;			/* this is not a duplicate entry */
	struct person *link;		/* link to next person */
};

char LASTLOG[] = "/usr/adm/lastlog";	/* last login info */
char USERLOG[] = UTMP_FILE;		/* who is logged in */
char PLAN[] = "/.plan";			/* what plan file is */
char PROJ[] = "/.project";		/* what project file */
	
int unbrief = 1;			/* -b option default */
int header = 1;				/* -f option default */
int hack = 1;				/* -h option default */
int idle = 0;				/* -i option default */
int large = 0;				/* -l option default */
int match = 1;				/* -m option default */
int plan = 1;				/* -p option default */
int unquick = 1;			/* -q option default */
int small = 0;				/* -s option default */
int wide = 1;				/* -w option default */

int unshort;
#if SEC_BASE
int lf = -1;				/* LASTLOG flag */
#else
int lf;					/* LASTLOG file descriptor */
#endif
struct person *person1;			/* list of people */
long tloc;				/* current time */

struct passwd *pwdcopy();
char *strcpy();
char *malloc();
char *ctime();
struct person *sort();

main(argc, argv)
	int argc;
	register char **argv;
{
	register char *s;

	(void) setlocale(LC_ALL, "");
	catd = catopen (MF_FINGER, NL_CAT_LOCALE);

#if SEC_BASE
	set_auth_parameters(argc, argv);
	initprivs();
#endif
	/* parse command line for (optional) arguments */
	while (*++argv && **argv == COMMAND)
		for (s = *argv + 1; *s; s++)
			switch (*s) {
			case 'b':
				unbrief = 0;
				break;
			case 'f':
				header = 0;
				break;
			case 'h':
				hack = 0;
				break;
			case 'i':
				idle = 1;
				unquick = 0;
				break;
			case 'l':
				large = 1;
				break;
			case 'm':
				match = 0;
				break;
			case 'p':
				plan = 0;
				break;
			case 'q':
				unquick = 0;
				break;
			case 's':
				small = 1;
				break;
			case 'w':
				wide = 0;
				break;
			default:
				fprintf(stderr, MSGSTR(USAGE,"Usage: finger [-bfhilmpqsw] [login1 [login2 ...] ]\n"));
				exit(1);
			}
	if (unquick || idle)
		time(&tloc);
	/*
	 * *argv == 0 means no names given
	 */
	if (*argv == 0)
		doall();
	else
		donames(argv);
	if (person1)
		print();
	exit(0);
}

doall()
{
	register struct person *p;
	register struct passwd *pw;
	int uf;
	char name[NMAX + 1];

	unshort = large;
	if ((uf = open(USERLOG, 0)) < 0) {
		fprintf(stderr, MSGSTR(ERR_OPEN,"finger: error opening %s\n"), USERLOG);
		exit(2);
	}
	if (unquick) {
		extern _pw_stayopen;

		setpwent();
		_pw_stayopen = 1;
		fwopen();
	}
	while (read(uf, (char *)&user, sizeof user) == sizeof user) {
		if ((user.ut_name[0] == 0) || (user.ut_type != USER_PROCESS))
			continue;
		if (person1 == 0)
			p = person1 = (struct person *) malloc(sizeof *p);
		else {
			p->link = (struct person *) malloc(sizeof *p);
			p = p->link;
		}
		bcopy(user.ut_name, name, NMAX);
		name[NMAX] = 0;
		bcopy(user.ut_line, p->tty, LMAX);
		p->tty[LMAX] = 0;
		bcopy(user.ut_host, p->host, HMAX);
		p->host[HMAX] = 0;
		p->loginat = user.ut_time;
		p->pwd = 0;
		p->loggedin = 1;
		if (unquick && (pw = getpwnam(name))) {
			p->pwd = pwdcopy(pw);
			decode(p);
			p->name = p->pwd->pw_name;
		} else
			p->name = strcpy(malloc(strlen(name) + 1), name);
	}
	if (unquick) {
		fwclose();
		endpwent();
	}
	close(uf);
	if (person1 == 0) {
		printf(MSGSTR(NOLOG, "No one logged on\n"));
		return;
	}
	p->link = 0;
}

donames(argv)
	char **argv;
{
	register struct person *p;
	register struct passwd *pw;
	int uf;

	/*
	 * get names from command line and check to see if they're
	 * logged in
	 */
	unshort = !small;
	for (; *argv != 0; argv++) {
		if (netfinger(*argv))
			continue;
		if (person1 == 0)
			p = person1 = (struct person *) malloc(sizeof *p);
		else {
			p->link = (struct person *) malloc(sizeof *p);
			p = p->link;
		}
		p->name = *argv;
		p->loggedin = 0;
		p->original = 1;
		p->pwd = 0;
	}
	if (person1 == 0)
		return;
	p->link = 0;
	/*
	 * if we are doing it, read /etc/passwd for the useful info
	 */
	if (unquick) {
		setpwent();
		if (!match) {
			extern _pw_stayopen;

			_pw_stayopen = 1;
			for (p = person1; p != 0; p = p->link)
				if (pw = getpwnam(p->name))
					p->pwd = pwdcopy(pw);
		} else while ((pw = getpwent()) != 0) {
			for (p = person1; p != 0; p = p->link) {
				if (!p->original)
					continue;
				if (strcmp(p->name, pw->pw_name) != 0 &&
				    !matchcmp(pw->pw_gecos, pw->pw_name, p->name))
					continue;
				if (p->pwd == 0)
					p->pwd = pwdcopy(pw);
				else {
					struct person *new;
					/*
					 * handle multiple login names, insert
					 * new "duplicate" entry behind
					 */
					new = (struct person *)
						malloc(sizeof *new);
					new->pwd = pwdcopy(pw);
					new->name = p->name;
					new->original = 1;
					new->loggedin = 0;
					new->link = p->link;
					p->original = 0;
					p->link = new;
					p = new;
				}

			}
		}
		endpwent();
	}
	/* Now get login information */
	if ((uf = open(USERLOG, 0)) < 0) {
		fprintf(stderr, MSGSTR(ERR_OPEN,"finger: error opening %s\n"), USERLOG);
		exit(2);
	}
	while (read(uf, (char *)&user, sizeof user) == sizeof user) {
		if (*user.ut_name == 0)
			continue;
		for (p = person1; p != 0; p = p->link) {
			if (p->loggedin == 2)
				continue;
			if (strncmp(p->pwd ? p->pwd->pw_name : p->name,
				    user.ut_name, NMAX) != 0)
				continue;
			if (p->loggedin == 0) {
				bcopy(user.ut_line, p->tty, LMAX);
				p->tty[LMAX] = 0;
				bcopy(user.ut_host, p->host, HMAX);
				p->host[HMAX] = 0;
				p->loginat = user.ut_time;
				p->loggedin = 1;
			} else {	/* p->loggedin == 1 */
				struct person *new;
				new = (struct person *) malloc(sizeof *new);
				new->name = p->name;
				bcopy(user.ut_line, new->tty, LMAX);
				new->tty[LMAX] = 0;
				bcopy(user.ut_host, new->host, HMAX);
				new->host[HMAX] = 0;
				new->loginat = user.ut_time;
				new->pwd = p->pwd;
				new->loggedin = 1;
				new->original = 0;
				new->link = p->link;
				p->loggedin = 2;
				p->link = new;
				p = new;
			}
		}
	}
	close(uf);
	if (unquick) {
		fwopen();
		for (p = person1; p != 0; p = p->link)
			decode(p);
		fwclose();
	}
}

print()
{
	register FILE *fp;
	register struct person *p;
	register char *s;
	register c;
	struct stat statb;

	/*
	 * print out what we got
	 */
	if (header) {
		if (unquick) {
			if (!unshort)
				if (wide)
			 		printf(MSGSTR(HEADER1,"Login       Name              TTY Idle    When            Office\n"));
				else
			 		printf(MSGSTR(HEADER2,"Login    TTY Idle    When            Office\n"));
		} else {
			printf(MSGSTR(HEADER3,"Login      TTY            When"));
			if (idle)
				printf(MSGSTR(HEADER4,"             Idle"));
			putchar('\n');
		}
	}

	if (unquick && unshort)		/* CMU enhancment - sort the list */
		person1 = sort(person1);

	for (p = person1; p != 0; p = p->link) {
		if (!unquick) {
			quickprint(p);
			continue;
		}
		if (!unshort) {
			shortprint(p);
			continue;
		}
		personprint(p);

		/*
		 * CMU enhancement
		 * Don't print out the plan/project info until we have seen
		 * the last entry for a userid (in the sorted list).
		 */
		if (p->link != NULL && (p->pwd != NULL && p->link->pwd != NULL)
		    && strcmp(p->pwd->pw_name, p->link->pwd->pw_name) == 0)
			continue;

		if (p->pwd != 0) {
			if (hack) {
				s = malloc(strlen(p->pwd->pw_dir) +
					sizeof PROJ);
				strcpy(s, p->pwd->pw_dir);
				strcat(s, PROJ);
				/* Only display .project if:
				 * 	1. is a regular file
				 *	2. has a link count == 1
				 *
				 * note that this rules out symbolic
				 * links, special files, and directorys.
				 */
				if ((lstat(s, &statb) < 0) ||
				   ((statb.st_mode & S_IFMT) != S_IFREG) ||
				   (statb.st_nlink != 1))
					; /* nop */
				else if ((fp = fopen(s, "r")) != 0) {
					printf(MSGSTR(PROJECT,"Project: "));
					while ((c = getc(fp)) != EOF) {
						if (c == '\n')
							break;
						putchar(c);
					}
					fclose(fp);
					putchar('\n');
				}
				free(s);
			}
			if (plan) {
				s = malloc(strlen(p->pwd->pw_dir) +
					sizeof PLAN);
				strcpy(s, p->pwd->pw_dir);
				strcat(s, PLAN);
				/* see rules for .project (above) */
				if ((lstat(s, &statb) < 0) ||
				   ((statb.st_mode & S_IFMT) != S_IFREG) ||
				   (statb.st_nlink != 1) ||
				   ((fp = fopen(s, "r")) == 0))
					printf(MSGSTR(NOPLAN,"No Plan.\n"));
				else {
					printf(MSGSTR(PALN,"Plan:\n"));
					while ((c = getc(fp)) != EOF)
						putchar(c);
					fclose(fp);
				}
				free(s);
			}
		}
		if (p->link != 0)
			putchar('\n');
	}
}

/*
 * Duplicate a pwd entry.
 * Note: Only the useful things (what the program currently uses) are copied.
 */
struct passwd *
pwdcopy(pfrom)
	register struct passwd *pfrom;
{
	register struct passwd *pto;

	pto = (struct passwd *) malloc(sizeof *pto);
#define savestr(s) strcpy(malloc(strlen(s) + 1), s)
	pto->pw_name = savestr(pfrom->pw_name);
	pto->pw_uid = pfrom->pw_uid;
	pto->pw_gecos = savestr(pfrom->pw_gecos);
	pto->pw_dir = savestr(pfrom->pw_dir);
	pto->pw_shell = savestr(pfrom->pw_shell);
#undef savestr
	return pto;
}

/*
 * print out information on quick format giving just name, tty, login time
 * and idle time if idle is set.
 */
quickprint(pers)
	register struct person *pers;
{
	printf(MSGSTR(PNAME,"%-*.*s  "), NWIDTH , NWIDTH, pers->name);
	if (pers->loggedin) {
		if (idle) {
			findidle(pers);
			printf(MSGSTR(QUICKPRINT1,"%c%-*s %-16.16s"), pers->writable ? ' ' : '*',
				LWIDTH, pers->tty, ctime(&pers->loginat));
			ltimeprint("   ", &pers->idletime, "");
		} else
			printf(MSGSTR(QUICKPRINT2," %-*s %-16.16s"), LWIDTH,
				pers->tty, ctime(&pers->loginat));
		putchar('\n');
	} else
		printf(MSGSTR(NOTLOG,"          Not Logged In\n"));
}

/*
 * print out information in short format, giving login name, full name,
 * tty, idle time, login time, office location and phone.
 */
shortprint(pers)
	register struct person *pers;
{
	char *p;
	char dialup;

	if (pers->pwd == 0) {
		printf("%-15s       ???\n", pers->name);
		return;
	}
	printf("%-*s", NWIDTH, pers->pwd->pw_name);
	dialup = 0;
	if (wide) {
		if (pers->realname)
			printf(" %-20.20s", pers->realname);
		else
			printf("        ???          ");
	}
	putchar(' ');
	if (pers->loggedin && !pers->writable)
		putchar('*');
	else
		putchar(' ');
	if (*pers->tty) {
		if (pers->tty[0] == 't' && pers->tty[1] == 't' &&
		    pers->tty[2] == 'y') {
			if (pers->tty[3] == 'd' && pers->loggedin)
				dialup = 1;
			printf("%-2.2s ", pers->tty + 3);
		} else
			printf("%-2.2s ", pers->tty);
	} else
		printf("   ");
	p = ctime(&pers->loginat);
	if (pers->loggedin) {
		stimeprint(&pers->idletime);
		printf(" %3.3s %-5.5s ", p, p + 11);
	} else if (pers->loginat == 0)
		printf(" < .  .  .  . >");
	else if (tloc - pers->loginat >= 180 * 24 * 60 * 60)
		printf(" <%-6.6s, %-4.4s>", p + 4, p + 20);
	else
		printf(" <%-12.12s>", p + 4);
	if (dialup && pers->homephone)
		printf(" %20s", pers->homephone);
	else {
		if (pers->office)
			printf(" %-11.11s", pers->office);
		else if (pers->officephone || pers->homephone)
			printf("            ");
		if (pers->officephone)
			printf(" %s", pers->officephone);
		else if (pers->homephone)
			printf(" %s", pers->homephone);
	}
	putchar('\n');
}

/*
 * print out a person in long format giving all possible information.
 * directory and shell are inhibited if unbrief is clear.
 */
personprint(pers)
	register struct person *pers;
{

	/*
	 * CMU enhancement
	 */
	static struct person *last = 0;

	/*
	 * Only print out the header for a userid the first time we encounter
	 * their name.
	 */
	if (last != 0 && (last->pwd != NULL && pers->pwd != NULL) &&
	    strcmp(last->pwd->pw_name, pers->pwd->pw_name) == 0) {
		last = pers;
		goto loginfo;
	}
	last = pers;

	if (pers->pwd == 0) {
		printf(MSGSTR(LOGIN1,"Login name: %-10s\t\t\tIn real life: ???\n"), pers->name);
		return;
	}
	printf(MSGSTR(LOGIN2,"Login name: %-10s"), pers->pwd->pw_name);
	if (pers->loggedin && !pers->writable)
		printf(MSGSTR(MSGOFF,"	(messages off)	"));
	else
		printf("			");
	if (pers->realname)
		printf(MSGSTR(REALNAME, "In real life: %s"), pers->realname);
	if (pers->office) {
		printf(MSGSTR(OFFICE,"\nOffice: %-.11s"), pers->office);
		if (pers->officephone) {
			printf(", %s", pers->officephone);
			if (pers->homephone)
				printf(MSGSTR(HOMEPHONE1, "\t\tHome phone: %s"), pers->homephone);
			else if (pers->random)
				printf("\t\t%s", pers->random);
		} else
			if (pers->homephone)
				printf(MSGSTR(HOMEPHONE2, "\t\t\tHome phone: %s"), pers->homephone);
			else if (pers->random)
				printf("\t\t\t%s", pers->random);
	} else if (pers->officephone) {
		printf(MSGSTR(PHONE, "\nPhone: %s"), pers->officephone);
		if (pers->homephone)
			printf(", %s", pers->homephone);
		if (pers->random)
			printf(", %s", pers->random);
	} else if (pers->homephone) {
		printf(MSGSTR(PHONE, "\nPhone: %s"), pers->homephone);
		if (pers->random)
			printf(", %s", pers->random);
	} else if (pers->random)
		printf("\n%s", pers->random);
	if (unbrief) {
		printf(MSGSTR(DIR, "\nDirectory: %-25s"), pers->pwd->pw_dir);
		if (*pers->pwd->pw_shell)
			printf(MSGSTR(SHELL, "\tShell: %-s"), pers->pwd->pw_shell);
	}
loginfo:
	if (pers->loggedin) {
		/*
		 * CMU enhancement
		 * Make login and idle times look better when listed
		 */
		register char *ep = ctime(&pers->loginat);
		printf(MSGSTR(SINCE1, "\nOn since %15.15s"), &ep[4]);
		ltimeprint("\t\t", &pers->idletime, MSGSTR(IDLE, " Idle Time"));
		printf(MSGSTR(SINCE2, "\n   on %s"), pers->tty);
		if (*pers->host) 
			printf(MSGSTR(FROM, " from %s"), pers->host);

	} else if (pers->loginat == 0)
		printf(MSGSTR(NEVER, "\nNever logged in."));
	else if (tloc - pers->loginat > 180 * 24 * 60 * 60) {
		register char *ep = ctime(&pers->loginat);
		printf(MSGSTR(LASTLOGIN1, "\nLast login %10.10s, %4.4s on %s"),
			ep, ep+20, pers->tty);
		if (*pers->host)
			printf(MSGSTR(FROM, " from %s"), pers->host);
	} else {
		register char *ep = ctime(&pers->loginat);
		printf(MSGSTR(LASTLOGIN2, "\nLast login %16.16s on %s"), ep, pers->tty);
		if (*pers->host)
			printf(MSGSTR(FROM, " from %s"), pers->host);
	}

	/*
	 * CMU enhancement:
	 * Only print newline when we have seen the last entry for a userid.
	 */
	if (pers->link == 0 || (pers->pwd != NULL && pers->link->pwd != NULL)
	    && strcmp(pers->pwd->pw_name, pers->link->pwd->pw_name) != 0)
		putchar('\n');
}

/*
 *  phone - return phone string, unformatted (leave this up to each user)
 */
char  *phone(s)
char	*s;
{
	char	*p;

	if (strlen(s) == 0)	/* null phone field */
		return(0);
	if (strlen(s) == 4) {	/* 4 digit extension? */
		p = malloc(strlen(s) + 2);	/* leave space for 'x' */
		strcpy(p, "x");
		strcat(p, s);
		return(p);
	} else {
		p = malloc(strlen(s) + 1);
		strcpy(p, s);
		return(p);
	}
}

/*
 * decode the information in the gecos field of /etc/passwd
 */
decode(pers)
	register struct person *pers;
{
	char buffer[256];
	register char *bp, *gp, *lp;
	int hasspace;
	int len;

	pers->realname = 0;
	pers->office = 0;
	pers->officephone = 0;
	pers->homephone = 0;
	pers->random = 0;
	if (pers->pwd == 0)
		return;
	gp = pers->pwd->pw_gecos;
	bp = buffer;
	if (*gp == ASTERISK)
		gp++;
	while (*gp && *gp != COMMA)			/* name */
		if (*gp == SAMENAME) {
			lp = pers->pwd->pw_name;
			if (islower(*lp))
				*bp++ = toupper(*lp++);
			while (*bp++ = *lp++)
				;
			bp--;
			gp++;
		} else
			*bp++ = *gp++;
	*bp++ = 0;
	if ((len = bp - buffer) > 1)
		pers->realname = strcpy(malloc(len), buffer);
	if (*gp == COMMA) {				/* office */
		gp++;
		hasspace = 0;
		bp = buffer;
		while (*gp && *gp != COMMA) {
			*bp = *gp++;
			if (*bp == ' ')
				hasspace = 1;
			/* leave 5 for Cory and Evans expansion */
			if (bp < buffer + sizeof buffer - 6)
				bp++;
		}
		*bp = 0;
		len = bp - buffer;
		bp--;			/* point to last character */
		len++;
		if (len > 1)
			pers->office = strcpy(malloc(len), buffer);
	}
	if (*gp == COMMA) {				/* office phone */
		gp++;
		bp = buffer;
		while (*gp && *gp != COMMA) {
			*bp = *gp++;
			if (bp < buffer + sizeof buffer - 1)
				bp++;
		}
		*bp = 0;
		pers->officephone = phone(buffer);
	}
	if (*gp == COMMA) {				/* home phone */
		gp++;
		bp = buffer;
		while (*gp && *gp != COMMA) {
			*bp = *gp++;
			if (bp < buffer + sizeof buffer - 1)
				bp++;
		}
		*bp = 0;
		pers->homephone = phone(buffer);
	}
	if (pers->loggedin)
		findidle(pers);
	else
		findwhen(pers);
}

/*
 * find the last log in of a user by checking the LASTLOG file.
 * the entry is indexed by the uid, so this can only be done if
 * the uid is known (which it isn't in quick mode)
 */

fwopen()
{
#if SEC_BASE
	/*
	 * Last login times are kept in the protected password file on
	 * secure systems; the lastlog file is not maintained.  Just
	 * set a flag here to tell findwhen() that we want to get the
	 * last login information.
	 */
	lf = 1;
#else
	if ((lf = open(LASTLOG, 0)) < 0)
		fprintf(stderr, MSGSTR(FWOPEN, "finger: %s open error\n"), LASTLOG);
#endif
}

findwhen(pers)
	register struct person *pers;
{
#if !SEC_BASE
	struct lastlog ll;
	int i;
#endif

	if (lf >= 0) {
#if SEC_BASE
		finger_findwhen(pers->name, pers->tty, LMAX, pers->host, HMAX,
				&pers->loginat);
#else
		lseek(lf, (long)pers->pwd->pw_uid * sizeof ll, 0);
		if ((i = read(lf, (char *)&ll, sizeof ll)) == sizeof ll) {
			bcopy(ll.ll_line, pers->tty, LMAX);
			pers->tty[LMAX] = 0;
			bcopy(ll.ll_host, pers->host, HMAX);
			pers->host[HMAX] = 0;
			pers->loginat = ll.ll_time;
		} else {
			if (i != 0)
				fprintf(stderr, 
				       MSGSTR(FWREAD,"finger: %s read error\n"),
					LASTLOG);
			pers->tty[0] = 0;
			pers->host[0] = 0;
			pers->loginat = 0L;
		}
#endif
	} else {
		pers->tty[0] = 0;
		pers->host[0] = 0;
		pers->loginat = 0L;
	}
}

fwclose()
{
#if SEC_BASE
	lf = -1;
#else
	if (lf >= 0)
		close(lf);
#endif
}

/*
 * find the idle time of a user by doing a stat on /dev/tty??,
 * where tty?? has been gotten from USERLOG, supposedly.
 */
findidle(pers)
	register struct person *pers;
{
	struct stat ttystatus;
	static char buffer[20] = "/dev/";
	long t;
#define TTYLEN 5

	strcpy(buffer + TTYLEN, pers->tty);
	buffer[TTYLEN+LMAX] = 0;
	if (stat(buffer, &ttystatus) < 0) {
		/*
		 * As a general rule, this should not be a fatal condition
		 * since the line field of a utmp entry need not contain
		 * an actual device name (e.g., consider window systems).
		 * On systems with mandatory access control or nationality
		 * caveats, we may not be able to stat a user's terminal.
		 * Just fill in the fields we need below with dummy values:
		 * 0 idle time and not writable.
		 */
		time(&ttystatus.st_atime);
		ttystatus.st_mode = S_IFCHR;
	}
	time(&t);
	if (t < ttystatus.st_atime)
		pers->idletime = 0L;
	else
		pers->idletime = t - ttystatus.st_atime;
	pers->writable = (ttystatus.st_mode & TALKABLE) == TALKABLE;
}

/*
 * print idle time in short format; this program always prints 4 characters;
 * if the idle time is zero, it prints 4 blanks.
 */
stimeprint(dt)
	long *dt;
{
	register struct tm *delta;

	delta = gmtime(dt);
	if (delta->tm_yday == 0)
		if (delta->tm_hour == 0)
			if (delta->tm_min == 0)
				printf("    ");
			else
				printf("  %2d", delta->tm_min);
		else
			if (delta->tm_hour >= 10)
				printf("%3d:", delta->tm_hour);
			else
				printf("%1d:%02d",
					delta->tm_hour, delta->tm_min);
	else
		printf("%3dd", delta->tm_yday);
}

/*
 * print idle time in long format with care being taken not to pluralize
 * 1 minutes or 1 hours or 1 days.
 * print "prefix" first.
 */
ltimeprint(before, dt, after)
	long *dt;
	char *before, *after;
{
	register struct tm *delta;

	delta = gmtime(dt);
	if (delta->tm_yday == 0 && delta->tm_hour == 0 && delta->tm_min == 0 &&
	    delta->tm_sec <= 10)
		return (0);
	printf("%s", before);
	if (delta->tm_yday >= 10)
		printf(MSGSTR(DAY, "%d days"), delta->tm_yday);
	else if (delta->tm_yday > 0)
		if (delta->tm_yday == 1)
		    if (delta->tm_hour > 1)
		        printf(MSGSTR(DAYHR1,"%d day %d hours"),delta->tm_yday,delta->tm_hour);
		    else
		        printf(MSGSTR(DAYHR2,"%d day %d hour"),delta->tm_yday,delta->tm_hour);
		else
		    if (delta->tm_hour > 1)
		        printf(MSGSTR(DAYHR3,"%d days %d hours"),delta->tm_yday,delta->tm_hour);
		    else
		        printf(MSGSTR(DAYHR4,"%d days %d hour"),delta->tm_yday,delta->tm_hour);
	else
		if (delta->tm_hour >= 10)
			printf(MSGSTR(HOUR,"%d hours"), delta->tm_hour);
		else if (delta->tm_hour > 0)
		   	if (delta->tm_hour == 1) 
			   if (delta->tm_min > 1) 
			      printf(MSGSTR(HRMIN1,"%d hour %d minutes"), delta->tm_hour, delta->tm_min);
			   else
			      printf(MSGSTR(HRMIN2,"%d hour %d minute"), delta->tm_hour, delta->tm_min);
			else
			   if (delta->tm_min > 1)
			      printf(MSGSTR(HRMIN3,"%d hours %d minutes"), delta->tm_hour, delta->tm_min);
			   else
			      printf(MSGSTR(HRMIN4,"%d hours %d minute"), delta->tm_hour, delta->tm_min);
		else
			if (delta->tm_min >= 10)
				printf(MSGSTR(MINUTE,"%2d minutes"), delta->tm_min);
			else if (delta->tm_min == 0)
				printf(MSGSTR(SECOND,"%2d seconds"), delta->tm_sec);
			else
			     if (delta->tm_min == 1)
				if (delta->tm_sec > 1)
				  printf(MSGSTR(MINSEC1,"%d minute %d seconds"), delta->tm_min, delta->tm_sec);
				else
				  printf(MSGSTR(MINSEC2,"%d minute %d second"), delta->tm_min, delta->tm_sec);
			     else
				if (delta->tm_sec > 1)
				  printf(MSGSTR(MINSEC3,"%d minutes %d seconds"), delta->tm_min, delta->tm_sec);
				else
				  printf(MSGSTR(MINSEC4,"%d minutes %d second"), delta->tm_min, delta->tm_sec);
		
	printf("%s", after);
}

matchcmp(gname, login, given)
	register char *gname;
	char *login;
	char *given;
{
	char buffer[100];
	register char *bp, *lp;
	register c;

	if (*gname == ASTERISK)
		gname++;
	lp = 0;
	bp = buffer;
	for (;;)
		switch (c = *gname++) {
		case SAMENAME:
			for (lp = login; bp < buffer + sizeof buffer
					 && (*bp++ = *lp++);)
				;
			bp--;
			break;
		case ' ':
		case COMMA:
		case '\0':
			*bp = 0;
			if (namecmp(buffer, given))
				return (1);
			if (c == COMMA || c == 0)
				return (0);
			bp = buffer;
			break;
		default:
			if (bp < buffer + sizeof buffer)
				*bp++ = c;
		}
	/*NOTREACHED*/
}

namecmp(name1, name2)
	register char *name1, *name2;
{
	register c1, c2;

	for (;;) {
		c1 = *name1++;
		if (islower(c1))
			c1 = toupper(c1);
		c2 = *name2++;
		if (islower(c2))
			c2 = toupper(c2);
		if (c1 != c2)
			break;
		if (c1 == 0)
			return (1);
	}
	if (!c1) {
		for (name2--; isdigit(*name2); name2++)
			;
		if (*name2 == 0)
			return (1);
	} else if (!c2) {
		for (name1--; isdigit(*name1); name1++)
			;
		if (*name2 == 0)
			return (1);
	}
	return (0);
}

netfinger(name)
	char *name;
{
	char *host;
	struct hostent *hp;
	struct servent *sp;
	struct sockaddr_in sin;
	int s;
	char *rindex();
	register FILE *f;
	register int c;
	register int lastc;

	if (name == NULL)
		return (0);
	host = rindex(name, '@');
	if (host == NULL)
		return (0);
	*host++ = 0;
	hp = gethostbyname(host);
	if (hp == NULL) {
		static struct hostent def;
		static struct in_addr defaddr;
		static char *alist[1];
		static char namebuf[128];
		int inet_addr();

		defaddr.s_addr = inet_addr(host);
		if (defaddr.s_addr == -1) {
			printf(MSGSTR(UNKHOST,"unknown host: %s\n"), host);
			return (1);
		}
		strcpy(namebuf, host);
		def.h_name = namebuf;
		def.h_addr_list = alist, def.h_addr = (char *)&defaddr;
		def.h_length = sizeof (struct in_addr);
		def.h_addrtype = AF_INET;
		def.h_aliases = 0;
		hp = &def;
	}
	sp = getservbyname("finger", "tcp");
	if (sp == 0) {
		printf(MSGSTR(UNKSERVICE,"tcp/finger: unknown service\n"));
		return (1);
	}
	sin.sin_family = hp->h_addrtype;
	bcopy(hp->h_addr, (char *)&sin.sin_addr, hp->h_length);
	sin.sin_port = sp->s_port;
	s = socket(hp->h_addrtype, SOCK_STREAM, 0);
	if (s < 0) {
		perror("socket");
		return (1);
	}
	printf("[%s]\n", hp->h_name);
	fflush(stdout);
	if (connect(s, (char *)&sin, sizeof (sin)) < 0) {
		perror("connect");
		close(s);
		return (1);
	}
	if (large) write(s, "/W ", 3);
	write(s, name, strlen(name));
	write(s, "\r\n", 2);
	f = fdopen(s, "r");
	while ((c = getc(f)) != EOF) {
		lastc = c;
		putchar(c);
	}
	if (lastc != '\n')
		putchar('\n');
	(void)fclose(f);
	return (1);
}

/*
 * CMU enhancement
 * Sort the list of people so we can ignore duplicates
 */
struct person *
sort(p)
	register struct person *p;
{
	register struct person *h, *q, *r;

	if ((h = p) == 0)
		return 0;
	q = p->link;
	h->link = 0;
	while ((p = q) != 0) {
		q = p->link;
		if (strcmp(p->name, h->name) < 0) {
			p->link = h;
			h = p;
			continue;
		}
		for (r = h; r->link != 0; r = r->link)
			if (strcmp(p->name, r->link->name) < 0)
				break;
		p->link = r->link;
		r->link = p;
	}
	return h;
}
