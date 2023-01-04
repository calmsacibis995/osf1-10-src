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
static char	*sccsid = "@(#)$RCSfile: dumpoptr.c,v $ $Revision: 4.2.3.2 $ (DEC) $Date: 1992/12/18 15:38:13 $";
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
 * dumpoptr.c
 *
 *	Modification History:
 *
 * 01-Apr-91	Fred Canter
 *	MIPS C 2.20+, changes for -std
 *
 */
#if !defined( lint) && !defined(_NOIDENT)

#endif

/*
 * This module contains IBM CONFIDENTIAL code. -- (IBM Confidential Restricted
 * when combined with the aggregated modules for this product) OBJECT CODE ONLY
 * SOURCE MATERIALS (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or disclosure
 * restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * Copyright (c) 1980 Regents of the University of California. All
 * rights reserved.  The Berkeley software License Agreement specifies the
 * terms and conditions for redistribution.
 */

#include	"dump.h"

/*
 * Query the operator; This piece of code requires an exact response.
 * It is intended to protect dump aborting by inquisitive people
 * banging on the console terminal to see what is happening which might cause
 * dump to croak, destroying a large number of hours of work.
 *
 * Every 2 minutes we reprint the message, alerting others that dump needs
 * attention.
 */

static void		alarmcatch();
static void		pose_question();
static void		sendmes();
static struct fstab    *allocfsent();

static char	       *attn_message;	/* attention message */

int
query(question)
	char	       *question;
{

#if	EDUMP

	msg(MSGSTR(CNTBH, "We cannot be here in query() in edump!\n"));
	abort_dump();

	/* NOTREACHED */

#else	! EDUMP

	char		replybuffer[256];
	int		answer;
	FILE	       *tty_fp;
	int		i;
	char	       *ptr;
	void	      (*save_alarm)();

	if ((tty_fp = fopen("/dev/tty", "r")) == NULL)
	{
		msg(MSGSTR(FOPENTTY, "Cannot fopen /dev/tty for reading\n"));
		dump_perror("query(): fopen()");
		abort_dump();

		/* NOTREACHED */
	}

	attn_message = question;

	save_alarm = signal(SIGALRM, alarmcatch);

	for (;;)
	{
		pose_question();
		(void) alarm(2 * MINUTE);

		if (fgets(replybuffer, sizeof(replybuffer) - 1, tty_fp) == NULL && ferror(tty_fp) != 0)
		{
			clearerr(tty_fp);
			continue;
		}

		ptr = (char *) strchr(replybuffer, '\n');
		if (ptr != NULL)
		{
			*ptr = '\0';
		}

		/*
		 * use NLyesno only if both the environment strings are
		 * defined.
		 */

		if (yes_flag == TRUE && no_flag == TRUE)
		{
			i = NLyesno(replybuffer);
		}
		else
		{
			if (strcmp(replybuffer, yes_str) == 0)
			{
				i = 1;
			}
			else if (strcmp(replybuffer, no_str) == 0)
			{
				i = 0;
			}
			else
			{
				i = -1;
			}
		}

		if (i == 1)
		{
			answer = YES;
			break;
		}
		else if (i == 0)
		{
			answer = NO;
			break;
		}

		msg(MSGSTR(YESNO1, "\"%s\" or \"%s\"?\n\n"), yes_str, no_str);

	}

	/*
	 * Turn off the alarm, and reset the signal to what it was before
	 */

	(void) alarm(0);
	(void) signal(SIGALRM, save_alarm);

	(void) fclose(tty_fp);

	return(answer);

#endif	! EDUMP

}

/*
 * Alert the console operator, and enable the alarm clock to sleep for 2
 * minutes in case nobody comes to satisfy dump
 */

static void
alarmcatch()
{
	/* if this routine has been called, then the two minute alarm */
	/* has expired and all operators should be notified */

	void alarmcatch();

	if (notify_flag == TRUE)
	{
		broadcast(MSGSTR(NEEDATTB, "DUMP NEEDS ATTENTION!\7\7\n"));
	}

	msgtail("\n");
	pose_question();

	(void) signal(SIGALRM, alarmcatch);
	(void) alarm(2 * MINUTE);
}

static void
pose_question()
{

	msg(MSGSTR(NEEDATT1, "NEEDS ATTENTION: %s?: (\"%s\" or \"%s\") "), attn_message, yes_str, no_str);

}

/*
 * This is from /usr/include/grp.h That defined struct group, which
 * conflicts with the struct group defined in param.h
 */

struct Group
{				/* see getgrent(3) */
	char	       *gr_name;
	char	       *gr_passwd;
	int		gr_gid;
	char	      **gr_mem;
};

/*
 * The following variables and routines manage alerting operators to the status
 * of dump. This works much like wall(1) does.
 */

extern struct Group    *getgrnam();

static struct Group    *gp = NULL;

/*
 * Get the names from the group entry "operator" to notify.
 */

int
set_operators()
{
	gp = getgrnam(OPERATOR_GROUP);
	(void) endgrent();

	if (gp == NULL)
	{
		msg(MSGSTR(NOGRP, "No entry in /etc/group for %s\n"), OPERATOR_GROUP);
		return(-1);
	}
	return(0);
}

/*
 * We fork a child to do the actual broadcasting, so that the process control
 * groups are not messed up
 */

void
broadcast(message)
	char	       *message;
{
	FILE	       *utmp_fp;
	struct utmp	utmp;
	char	      **grp_mem_ptr;
	int		pid;
	int		dummy;

	switch (pid = fork())
	{
	case -1:
		msg(MSGSTR(CNTFK, "Cannot fork to broadcast message\n"));
		dump_perror("broadcast(): fork()");
		return;

	case 0:
		break;

	default:
		while (wait(&dummy) != pid)
		{
			;
		}
		return;
	}

	if ((utmp_fp = fopen(UTMP_FILE, "r")) == NULL)
	{
		msg(MSGSTR(COUTMP, "Cannot open %s for reading\n"),UTMP_FILE);
		dump_perror("broadcast(): fopen()");
		Exit(X_FINBAD);

		/* NOTREACHED */
	}

	while (!feof(utmp_fp))
	{
		if (fread(&utmp, sizeof(struct utmp), 1, utmp_fp) != 1)
		{
			break;
		}

		if (utmp.ut_name[0] == '\0')
		{
			continue;
		}

		/*
		 * Do not send messages to operators on dialups
		 */

		if (strncmp(utmp.ut_line, DIALUP_PREFIX, strlen(DIALUP_PREFIX)) == 0)
		{
			continue;
		}

		for (grp_mem_ptr = gp->gr_mem; *grp_mem_ptr != NULL; ++grp_mem_ptr)
		{
			if (strncmp(*grp_mem_ptr, utmp.ut_name, sizeof(utmp.ut_name)) != 0)
			{
				continue;
			}

#if	TDEBUG

			msg("Message to %s at %s\n", utmp.ut_name, utmp.ut_line);

#endif	TDEBUG

			sendmes(utmp.ut_line, message);
		}
	}

	(void) fclose(utmp_fp);

	Exit(X_FINOK);		/* the wait in this same routine will catch
				 * this */
	/* NOTREACHED */
}

extern struct tm       *localtime();

static void
sendmes(tty_name, message)
	char	       *tty_name, *message;
{
	time_t		clock;
	struct tm      *localclock;
	char		tty_path[256];
	FILE	       *tty_fp;
	char		buf[BUFSIZ];
	register char  *cp;

	clock = time(NULL);
	localclock = localtime(&clock);

	(void) strcpy(tty_path, "/dev/");
	(void) strcat(tty_path, tty_name);

	if ((tty_fp = fopen(tty_path, "w")) != NULL)
	{
		setbuf(tty_fp, buf);

		(void) fprintf(tty_fp, "\a\a\a\n");
		(void) fprintf(tty_fp, MSGSTR(MFD1, "Message from the dump program to all operators "));
		(void) fprintf(tty_fp, MSGSTR(MFD2, "at %d:%02d ... \r\n\n"), localclock->tm_hour, localclock->tm_min);

		for (cp = message; *cp != '\0'; ++cp)
		{
			if (*cp == '\n')
			{
				(void) putc('\r', tty_fp);
			}
			(void) putc(*cp, tty_fp);
		}

		(void) fclose(tty_fp);
	}
}

/*
 * Tell the operator what has to be done; we don't actually do it
 */

static struct fstab   *
allocfsent(fs)
	register struct fstab *fs;
{
	register struct fstab *new;
	register char	      *cp;

	new = (struct fstab *) malloc(sizeof(struct fstab));
	cp = (char *) malloc((unsigned) strlen(fs->fs_file) + 1);
	(void) strcpy(cp, fs->fs_file);
	new->fs_file = cp;
	cp = (char *) malloc((unsigned) strlen(fs->fs_type) + 1);
	(void) strcpy(cp, fs->fs_type);
	new->fs_type = cp;
	cp = (char *) malloc((unsigned) strlen(fs->fs_spec) + 1);
	(void) strcpy(cp, fs->fs_spec);
	new->fs_spec = cp;
	new->fs_passno = fs->fs_passno;
	new->fs_freq = fs->fs_freq;
	return(new);
}

struct pfstab
{
	struct pfstab  *pf_next;
	struct fstab   *pf_fstab;
};

static struct pfstab *table = NULL;

void
getfstab()
{
	register struct fstab *fs;
	register struct pfstab *pf;

	if (setfsent() == 0)
	{
		msg(MSGSTR(CANTODT, "Cannot open fstab file %s\n"), FSTAB);
		dump_perror("getfstab(): setfsent()");
		return;
	}
	while ((fs = getfsent()) != NULL)
	{
		if (strcmp(fs->fs_type, FSTAB_RW) != 0 &&
		    strcmp(fs->fs_type, FSTAB_RO) != 0 &&
		    strcmp(fs->fs_type, FSTAB_RQ) != 0)
		{
			continue;
		}
		fs = allocfsent(fs);
		pf = (struct pfstab *) malloc(sizeof(struct pfstab));
		pf->pf_fstab = fs;
		pf->pf_next = table;
		table = pf;
	}
	(void) endfsent();
}

/*
 * Search in the fstab for a file name. This file name can be either the
 * special or the path file name.
 *
 * The entries in the fstab are the BLOCK special names, not the character
 * special names. The caller of fstabsearch assures that the character device
 * is dumped (that is much faster)
 *
 * The file name can omit the leading '/'.
 */

struct fstab   *
fstabsearch(key)
	char	       *key;
{
	register struct pfstab *pf;
	register struct fstab *fs;

	if (table == NULL)
	{
		return(NULL);
	}
	for (pf = table; pf; pf = pf->pf_next)
	{
		fs = pf->pf_fstab;
		if (strcmp(fs->fs_file, key) == 0)
		{
			return(fs);
		}
		if (strcmp(fs->fs_spec, key) == 0)
		{
			return(fs);
		}
		if (strcmp(rawname(fs->fs_spec), key) == 0)
		{
			return(fs);
		}
		if (key[0] != '/')
		{
			if (*fs->fs_spec == '/' && strcmp(fs->fs_spec + 1, key) == 0)
			{
				return(fs);
			}
			if (*fs->fs_file == '/' && strcmp(fs->fs_file + 1, key) == 0)
			{
				return(fs);
			}
		}
	}
	return(NULL);
}

/*
 * Tell the operator what to do
 */

void
lastdump(arg)
	int		arg;	/* w ==> just what to do; W ==> most recent
				 * dumps */
{
	char	       *previous_name;
	char	       *date;
	register int	i;
	time_t		time_now;
	register struct fstab *dt;
	int		dumpme;
	register struct idates *itwalk;
	char		strbuf[256];
	struct tm      *tp;

	(void) time(&time_now);

	getfstab();		/* /etc/fstab input */
	inititimes();		/* /etc/dumpdates input */

	if (arg == 'w')
	{
		(void) fprintf(stdout, MSGSTR(DUMPFSYS, "Dump these file systems:\n"));
	}
	else
	{
		(void) fprintf(stdout, MSGSTR(LASTDU, "Last dump(s) done (Dump '>' file systems):\n"));
	}

	previous_name = "";
	for (i = 0; i < num_idate_records; ++i)
	{
		itwalk = idate_array[i];
		if (strncmp(previous_name, itwalk->id_name, sizeof(itwalk->id_name)) == 0)
		{
			continue;
		}

		tp = localtime(&itwalk->id_ddate);
		strftime(strbuf, 64, "%a %sD %sT\0", tp);
		strbuf[64] = '\0';
		date = strbuf;

		previous_name = itwalk->id_name;
		dt = fstabsearch(itwalk->id_name);
		dumpme = (dt != NULL && dt->fs_freq != 0 && itwalk->id_ddate < time_now - dt->fs_freq * DAY)? YES: NO;
		if (arg != 'w' || dumpme == YES)
		{
			(void) fprintf(stdout,
				       MSGSTR(DUMPLL, "%c %8s\t(%6s) Last dump: Level %c, Date %s\n"),
				       (dumpme == YES && arg != 'w')? '>' : ' ',
				       itwalk->id_name,
				       (dt != NULL)? dt->fs_file : "",
				       itwalk->id_incno,
				       date);
		}
	}
}

/*
 * simple routine to send in the correct direction additional information
 */

/* VARARGS1 */

void
smsg(va_alist)
	va_dcl
{
	va_list		ap;
	char	       *fmt;
	char		buf[256];

	va_start(ap);
	fmt = va_arg(ap, char *);
	(void) vsprintf(buf, fmt, ap);
	va_end(ap);

#if	EDUMP

	rmtsmsg(buf);

#else	! EDUMP

	msg("%s\n", buf);

#endif	EDUMP

}

/* VARARGS1 */

void
msg(va_alist)
	va_dcl
{
	va_list		ap;
	char	       *fmt;

#if	EDUMP

	(void) fprintf(stderr, "edump: ");

#else	! EDUMP

#if	REMOTE

	(void) fprintf(stderr, "rdump: ");

#else	! REMOTE

	(void) fprintf(stderr, "dump: ");

#endif	! REMOTE

#endif	! EDUMP

#if	TDEBUG

	(void) fprintf(stderr, "pid %d: ", getpid());

#endif	TDEBUG

	va_start(ap);
	fmt = va_arg(ap, char *);
	(void) vfprintf(stderr, fmt, ap);
	va_end(ap);
	(void) fflush(stderr);
}

/* VARARGS1 */

void
msgtail(va_alist)
	va_dcl
{
	va_list		ap;
	char	       *fmt;

	va_start(ap);
	fmt = va_arg(ap, char *);
	(void) vfprintf(stderr, fmt, ap);
	va_end(ap);
	(void) fflush(stderr);
}

void
dump_perror(where)
	char	       *where;
{

#if	EDUMP

	msg("%s: %s\n", where, errmsg(errno));

#else	! EDUMP

	perror(where);

#endif	! EDUMP

}
