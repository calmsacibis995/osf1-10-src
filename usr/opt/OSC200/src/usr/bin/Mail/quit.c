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
static char rcsid[] = "@(#)$RCSfile: quit.c,v $ $Revision: 4.2.9.5 $ (DEC) $Date: 1993/12/21 21:36:40 $";
#endif
/*
 * HISTORY
 */
/*
 * OSF/1 Release 1.0
 */
/* 
 * COMPONENT_NAME: CMDMAILX quit.c
 * 
 * FUNCTIONS: MSGSTR, quit, writeback 
 *
 * ORIGINS: 10  26  27 
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 *	quit.c       5.3 (Berkeley) 3/6/86
 */
/*
 * Modification History:
 *
 * 00 Todd Kaehler Thu Jul 25 13:13:43 EDT 1991
 *    Added the ability to use the lockf(3) system call to lock the mail
 *    spool file.  lockf(3) has been added to make Mail compatible with
 *    other mailers that use lockf(3) and to get locking to works over nfs.
 *    Use the compile option LOCKF to use lockf(3) rather than flock(2).
 *
 */

/* 00 kaehler */
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <userpw.h>
#include <syslog.h>

#include "rcv.h"
#include <sys/stat.h>
#include <sys/file.h>

#include "Mail_msg.h" 
extern nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_MAILX,n,s) 

#define	DOUBLE_LK		/* User-configurable locking style - RW */
#define	PGMNAME		"mailx"


/*
 * Rcv -- receive mail rationally.
 *
 * Termination processing.
 */


/*
 * Save all of the undetermined messages at the top of "mbox"
 * Save all untouched messages back in the system mailbox.
 * Remove the system mailbox, if none saved there.
 */

quit()
{
	int mcount, p, modify, autohold, anystat, holdbit, nohold;
	FILE *ibuf, *obuf, *fbuf, *rbuf, *readstat, *abuf;
	register struct message *mp;
	register int c, ck;
	extern char tempQuit[], tempResid[];
	struct stat minfo;
	char *id;
	/* for SVID-2, use a local copy of mbox */
        char *mbox = expand(Getf("MBOX"));


	/*
	 * If we are read only, we can't do anything,
	 * so just return quickly.
	 */

	if (readonly)
		return;
	/*
	 * See if there any messages to save in mbox.  If no, we
	 * can save copying mbox to gettmpdir() and back.
	 *
	 * Check also to see if any files need to be preserved.
	 * Delete all untouched messages to keep them out of mbox.
	 * If all the messages are to be preserved, just exit with
	 * a message.
	 *
	 * If the luser has sent mail to himself, refuse to do
	 * anything with the mailbox, unless mail locking works.
	 */

	/*
	 * 00 kaehler
	 * NOTE: mailfile is open "r+" so lockf() locking works correctly.
	 * Do not attempt to write to fbuf in the quit() routine.
	 *
	 * R.W. Call lock() to lockf the file; spin until obtained or timeout.
	 */
	for(;;){
	    fbuf = fopen(mailname, "r+");
	    if (fbuf == NULL)
		    goto newmail;
	    if (lock(fileno(fbuf)))
		break;
	    fclose(fbuf);
	}
	lock_file(mailname);


#ifndef CANLOCK
	if (selfsent) {
		printf(MSGSTR(NEWMAIL, "You have new mail.\n")); /*MSG*/
		unlock_file();
		fclose(fbuf);
		return;
	}
#endif
	rbuf = NULL;
	if (fstat(fileno(fbuf), &minfo) >= 0 && minfo.st_size > mailsize) {
		printf(MSGSTR(MAILTIME, "New mail has arrived.\n")); /*MSG*/
		rbuf = fopen(tempResid, "w");
		if (rbuf == NULL || fbuf == NULL)
			goto newmail;
#ifdef APPEND
		fseek(fbuf, mailsize, 0);
		while ((c = getc(fbuf)) != EOF) {
			ck = putc(c, rbuf);
			if (ck == EOF) {
			perror(tempResid);
			fclose(rbuf);
			unlock_file();
			fclose(fbuf);
			return;
			}
		}
#else
		p = minfo.st_size - mailsize;
		while (p-- > 0) {
			c = getc(fbuf);
			if (c == EOF)
				goto newmail;
			ck = putc(c, rbuf);
			if (ck == EOF) {
			perror(tempResid);
			fclose(rbuf);
			unlock_file();
			fclose(fbuf);
			return;
			}
		}
#endif
		fclose(rbuf);
		if ((rbuf = fopen(tempResid, "r")) == NULL)
			goto newmail;
		remove(tempResid);
	}

	/*
	 * Adjust the message flags in each message.
	 */

	anystat = 0;
	autohold = value("hold") != NULLSTR;
	holdbit = autohold ? MPRESERVE : MBOX;
	nohold = MBOX|MSAVED|MDELETED|MPRESERVE;
	if (value("keepsave") != NULLSTR)
		nohold &= ~MSAVED;
	for (mp = &message[0]; mp < &message[msgCount]; mp++) {
		if (mp->m_flag & MNEW) {
			mp->m_flag &= ~MNEW;
			mp->m_flag |= MSTATUS;
		}
		if (mp->m_flag & MSTATUS)
			anystat++;
		if ((mp->m_flag & MTOUCH) == 0)
			mp->m_flag |= MPRESERVE;
		if ((mp->m_flag & nohold) == 0)
			mp->m_flag |= holdbit;
	}
	modify = 0;
	if (Tflag != NULLSTR) {
		if ((readstat = fopen(Tflag, "w")) == NULL)
			Tflag = NULLSTR;
	}
	for (c = 0, p = 0, mp = &message[0]; mp < &message[msgCount]; mp++) {
		if (mp->m_flag & MBOX)
			c++;
		if (mp->m_flag & MPRESERVE)
			p++;
		if (mp->m_flag & MODIFY)
			modify++;
		if (Tflag != NULLSTR && (mp->m_flag & (MREAD|MDELETED)) != 0) {
			id = hfield("article-id", mp);
			if (id != NULLSTR)
				fprintf(readstat, "%s\n", id);
		}
	}
	if (Tflag != NULLSTR)
		fclose(readstat);
	if (p == msgCount && !modify && !anystat) {
		if (p == 1)
			printf(MSGSTR(MSGHELD, "Held 1 message in %s\n"), mailname); /*MSG*/
		else
			printf(MSGSTR(MSGSHELD, "Held %2d messages in %s\n"), p, mailname); /*MSG*/
		unlock_file();
		fclose(fbuf);
		return;
	}
	if (c == 0) {
		if (p != 0) {
			writeback(rbuf);
			unlock_file();
			fclose(fbuf);
			return;
		}
		goto cream;
	}

	/*
	 * Create another temporary file and copy user's mbox file
	 * darin.  If there is no mbox, copy nothing.
	 * If he has specified "append" don't copy his mailbox,
	 * just copy saveable entries at the end.
	 */

	mcount = c;
	if (value("append") == NULLSTR) {
		if ((obuf = fopen(tempQuit, "w")) == NULL) {
			perror(tempQuit);
			unlock_file();
			fclose(fbuf);
			return;
		}
		if ((ibuf = fopen(tempQuit, "r")) == NULL) {
			perror(tempQuit);
			remove(tempQuit);
			fclose(obuf);
			unlock_file();
			fclose(fbuf);
			return;
		}
		remove(tempQuit);
		if ((abuf = fopen(mbox, "r")) != NULL) {
			while ((c = getc(abuf)) != EOF) {
				ck = putc(c, obuf);
				if (ck == EOF) {
				perror(tempQuit);
				fclose(ibuf);
				fclose(obuf);
				unlock_file();
				fclose(fbuf);
				return;
				}
			}
			fclose(abuf);
		}
		if (ferror(obuf)) {
			perror(tempQuit);
			fclose(ibuf);
			fclose(obuf);
			unlock_file();
			fclose(fbuf);
			return;
		}
		fclose(obuf);
		close(creat(mbox, 0600));
		if ((obuf = fopen(mbox, "r+")) == NULL) {
			perror(mbox);
			fclose(ibuf);
			unlock_file();
			fclose(fbuf);
			return;
		}
	}
	if (value("append") != NULLSTR) {

		if ((obuf = fopen(mbox, "a")) == NULL) {
			perror(mbox);
			unlock_file();
			fclose(fbuf);
			return;
		}

		/* this is to compensate for the open() behavior that
		   causes the file ptr to == 0 even on open for append,
		   as per POSIX specs (?) */
		
		fseek(obuf, 0L, SEEK_END);

		fchmod(fileno(obuf), 0600);
	}
	for (mp = &message[0]; mp < &message[msgCount]; mp++)
		if (mp->m_flag & MBOX) {

#ifdef ASIAN_I18N
			if (send(mp, obuf, 0, 0, NOCHANGE) < 0) {
#else
			if (send(mp, obuf, 0) < 0) {
#endif
				perror(mbox);
				fclose(ibuf);
				fclose(obuf);
				unlock_file();
				fclose(fbuf);
				return;
			}

		}

	/*
	 * Copy the user's old mbox contents back
	 * to the end of the stuff we just saved.
	 * If we are appending, this is unnecessary.
	 */

	if (value("append") == NULLSTR) {
		rewind(ibuf);
		c = getc(ibuf);
		while (c != EOF) {
			putc(c, obuf);
			if (ferror(obuf))
				break;
			c = getc(ibuf);
		}
		fclose(ibuf);
		fflush(obuf);
	}
	trunc(obuf);
	if (ferror(obuf)) {
		perror(mbox);
		fclose(obuf);
		unlock_file();
		fclose(fbuf);
		return;
	}


	fclose(obuf);
	if (mcount == 1)
		printf(MSGSTR(MSGSAVED1, "Saved 1 message in %s\n"), mbox); /*MSG*/
	else
		printf(MSGSTR(MSGSSAVED1, "Saved %d messages in %s\n"), mcount, mbox); /*MSG*/

	/*
	 * Now we are ready to copy back preserved files to
	 * the system mailbox, if any were requested.
	 */

	if (p != 0) {
		writeback(rbuf);
		unlock_file();
		fclose(fbuf);
		return;
	}

	/*
	 * Finally, remove his /usr/mail file.
	 * If new mail has arrived, copy it back.
	 */

cream:
	if (rbuf != NULL) {
		abuf = fopen(mailname, "r+");
		if (abuf == NULL)
			goto newmail;
		while ((c = getc(rbuf)) != EOF)
			putc(c, abuf);
		fclose(rbuf);
		trunc(abuf);
		fclose(abuf);
		alter(mailname);
		unlock_file();
		fclose(fbuf);
		return;
	}
	demail();
	unlock_file();
	fclose(fbuf);
	return;

newmail:
	printf(MSGSTR(THOU, "Thou hast new mail.\n")); /*MSG*/
	if (fbuf != NULL)
		unlock_file();
		fclose(fbuf);
}

/*
 * Preserve all the appropriate messages back in the system
 * mailbox, and print a nice message indicated how many were
 * saved.  On any error, just return -1.  Else return 0.
 * Incorporate the any new mail that we found.
 */
writeback(res)
	register FILE *res;
{
	register struct message *mp;
	register int p, c, ck;
	FILE *obuf;

	p = 0;
	if ((obuf = fopen(mailname, "r+")) == NULL) {
		perror(mailname);
		return(-1);
	}
#ifndef APPEND
	if (res != NULL)
		while ((c = getc(res)) != EOF) {
			ck = putc(c, obuf);
			if (ck == EOF) {
			perror(mailname);
			fclose(obuf);
			return(-1);
			}
 		}
#endif
	for (mp = &message[0]; mp < &message[msgCount]; mp++)
		if ((mp->m_flag&MPRESERVE)||(mp->m_flag&MTOUCH)==0) {
			p++;
#ifdef ASIAN_I18N
			if (send(mp, obuf, 0, 0, NOCHANGE) < 0) {
#else
			if (send(mp, obuf, 0) < 0) {
#endif
				perror(mailname);
				fclose(obuf);
				return(-1);
			}
		}
#ifdef APPEND
	if (res != NULL)
		while ((c = getc(res)) != EOF) {
			ck = putc(c, obuf);
			if (ck == EOF) {
			perror(mailname);
			fclose(obuf);
			return(-1);
			}
 		}
#endif
	fflush(obuf);
	trunc(obuf);
	if (ferror(obuf)) {
		perror(mailname);
		fclose(obuf);
		return(-1);
	}
	if (res != NULL)
		fclose(res);
	fclose(obuf);
	alter(mailname);
	if (p == 1)
		printf(MSGSTR(MSGHELD, "Held 1 message in %s\n"), mailname); /*MSG*/
	else
		printf(MSGSTR(MSGSKEPT, "Held %d messages in %s\n"), p, mailname); /*MSG*/
	return(0);
}

/************************************
**  User-tunable locking style code.
**
**  The code block directly below between "#ifdef DOUBLE_LK" is replicated
**  in mailx/quit.c, the MH directories/lock.c, and binmail.c
**  R.W.
************************************/
#if defined DOUBLE_LK
#if ! defined _MTS_H_	/* Then this is not MH */
/* The following are for the locking style, and follow the naming
** conventions from MH  - R.W.
*/
#define LOK_UNIX	0	/* Defaults to LOK_KERNEL */
#define	LOK_BELL	1	/* ".lock" lock file */
#define	LOK_MMDF	2	/* mmdf-style lock file; not supported */
#define	LOK_KERNEL	4	/* Explicitly request lockf */

#define	LOK_VALID(lok)	(!((lok) & ~(LOK_BELL|LOK_KERNEL)))
#define	LOK_INVAL	-1

#define	LOK_DFLT	(LOK_KERNEL|LOK_BELL)	/* Don't make this LOK_UNIX */
int lockstyle = LOK_DFLT;
#endif	/* _MTS_H_ */

typedef struct {
    char *fname;
    char *pattern;
} Lkcfg_tbl;

static Lkcfg_tbl lkcfg_tbl[] = {
    { "/etc/rc.config", "MAILLOCKING=" },
#ifndef	_MTS_H_
    { "/usr/lib/mh/mtstailor", "lockstyle:" },
#endif
    { NULL, NULL }
};

static int
lock_style_init()
{
    /*	Get the style of locking from the configuration file(s).
    **  After the first invocation, it simply returns the cached return
    **  status.
    **  Returns: the found, legal lockvalue
    **          0 if the value doesn't exist (or file not found)
    **          LOK_INVAL (-1) on an illegal value from the file
    **	LOK_UNIX is never returned (since it is 0).  LOK_KERNEL is used instead.
    */

    register Lkcfg_tbl *tp;
    register char *cp;

    static int initd = 0;
    FILE *fp;
    register int  len;
    int val;
    char buf[128];
    char *ep;

    if (initd)
	return(initd - 2);
    else
	initd = 2;

    for (tp = lkcfg_tbl; tp->fname; tp++){
	if (!(fp = fopen(tp->fname, "r")))
	    continue;

	cp = buf + (len = strlen(tp->pattern));
	while (fgets(buf, sizeof(buf), fp)){
	    /*  Search for pattern.  Assume that the full pattern is:
	    **	{pattern}{some terminating char}{optional '"'}
	    */
	    if (strncmp(buf, tp->pattern, len))
		continue;
	    cp[-1] = '\0';	/* wipe out the terminator */
	    if (*cp == '"')
		cp++;

	    errno=0;
	    val = strtol(cp, &ep, 0);
	    if (!errno && cp != ep && LOK_VALID(val)){
		if (val == LOK_UNIX)
		    val = LOK_KERNEL;
		lockstyle = val;
	    }
	    else {
#ifdef	_MTS_H_		/* Then MH */
                advise(NULLCP, "illegal %s value in %s", buf, tp->fname);
#else
		fprintf(stderr, MSGSTR(ILL_LKSTYLE, "%s: illegal %s value in %s\n"), PGMNAME, buf, tp->fname);
#endif

#if defined OSF1 || defined OSF	/* Log in syslog too */
		(void) openlog(PGMNAME, 1, LOG_MAIL);
                syslog(LOG_ERR, "Illegal %s value in %s", buf, tp->fname);
		closelog();
#endif
		val = -1;
	    }
	    fclose(fp);
	    initd += val;
	    return(val);
	}
	fclose(fp);
    }
    return(0);
}
#endif	/* DOUBLE_LK */


/* the lockfile mechanism for compatibility with ULTRIX locks */

#define MAXLOADAPPROX 10  /* in place of getla(); */
int OVERRIDE = 0;		/* Flag net if lock file overridden */
int FIRSTSLEEP = 1;
int LOCKSLEEP = 4;		/* Basic # seconds to sleep between
				 * checks for name.lock file existance
				 * and to ck the peak load ave.
				 */
#define OLOCKSLEEPS 19
int LOCKSLEEPS = OLOCKSLEEPS;   /* Basic # of times to sleep & wait for
				 * name.lock  file to disappear of its'
				 * own accord before we blow it away.
				 */

#define LF_TMOUT	60	/* Timeout value for lockf call */

char	*maillock	= ".lock";		/* Lock suffix for mailname */
char	curlock[PATH_MAX+1];			/* Last used name of lock */
int	locked;					/* To note that we locked it */

lock_file(file)

char *file;
{
	register int f;
	struct stat sbuf;
	struct stat original;
     /* long curtime; - removed AV */
	int statfailed;

	register int n;
	off_t osize;
	off_t nsize;
	struct stat mbox;

	if (locked)
		return(0);

#ifdef  DOUBLE_LK
	if (!(lockstyle & LOK_BELL))
	    return(0);
#endif

	n = strlen(file);
	if (n > sizeof(curlock)-2){
	    fprintf(stderr,MSGSTR(ILL_FNAME,"%s: invalid file name\n"),PGMNAME);
	    exit(1);
	}
	(void) strncpy(curlock, file, sizeof(curlock)-1);
	n = sizeof(curlock)-1 - n;
	(void) strncat(curlock, maillock, n);

	statfailed = 0;

top:
	/* Get the original size of the users' mail box
	 * and save it to check for changes to the mail box whilst
	 * we are sleeping on a lock file (if any).
	 */
	if (stat(file,&mbox) < 0)
		osize = 0;
	else
		osize = mbox.st_size;

	/* Get original mod time of possible lock file to test
	 * for creation of new lock file while we were sleeping.
	 */
	if (stat(curlock, &original) < 0) {
		original.st_ctime = 0;
	}

	/* Make number of sleep cycles.
	 */
	LOCKSLEEPS = OLOCKSLEEPS + MAXLOADAPPROX;

	for (n=0; n < LOCKSLEEPS; n++) {

		if ((f = lock1(curlock)) == -1){
		    /*  Some serious error in creating the lock */
		    exit(1);
		}
		if (f == 0) {
			if (OVERRIDE) {
				/*
	 			 * At this point, we would have waited 
				 * a long time for the lock file to go
				 * away. If it didn't, log a complaint.
				 */
				 (void) openlog(PGMNAME,1, LOG_MAIL);
				 syslog(LOG_ERR,"Overriding mail lock file for  %s",file);
				 closelog();
     
			}
			/* We have locked the file, return to caller.
			 */
			locked = 1;
			OVERRIDE = 0;
			return(0);
		}
		if (stat(curlock, &sbuf) < 0) {
			if (statfailed++ > 5){
				/*  In some twisted race cycle (NFS timeouts?).
				**  Give up.
				*/
				(void) openlog(PGMNAME,1, LOG_MAIL);
				syslog(LOG_ERR,"Cannot stat mail lock file\n");
				closelog();
				fprintf(stderr, MSGSTR(CREAT_LKFILE, "%s: cannot create lock file %s\n"), PGMNAME, curlock);
				exit(1);
			}

			(void) sleep((unsigned) /* AV */ LOCKSLEEP);

			/* Take a new reading on the load.
			(void) getla(); */
			continue;
		}
		statfailed = 0;

		/* A lock file exists. Sleep for awhile and look again.
		 */
		if (FIRSTSLEEP) {
			FIRSTSLEEP = 0;
			(void) openlog(PGMNAME, 1, LOG_MAIL);
			syslog(LOG_ERR,"Waiting on mail lock file %s",curlock);
			closelog(); 
		}
		(void) sleep((unsigned) /* AV */ (LOCKSLEEP /* + peak */));

		/* Take a new reading on the load.

		(void) getla(); */

		/* While we were sleeping, the mail box may have grown,
		 * shrunk, -or- disappeared....
		 * Get a new size to compare to the original.
		 */
		if (stat(file,&mbox) < 0) {
			osize = nsize = 0;
		}
		else
			nsize = mbox.st_size;

		if ((nsize != osize) ||
			(original.st_ctime != sbuf.st_ctime)) {

			/* If the users' mail box changed size, reset
			 * to new size and restart the entire wait
			 * cycle over. ie. We have to see the mail box
			 * not change size for the required amount of
			 * time if there was a lock file present
			 * in the first place before we think about
			 * removing the existing lock file.
			 */
			original.st_ctime = sbuf.st_ctime;
			n = 0;
			osize = nsize;
			LOCKSLEEPS = OLOCKSLEEPS; /*  + peak; */
		}
		continue;
	}
	/* If we get here, the mail lock file (name.lock) has existed
	 * for the required amount of time &  we didn't see the
	 * users' mail box change size. -or- If we saw it change size,
	 * we reset our counters and rewound the clock for another
	 * time and then waited the respectable interval before
	 * resorting to removing the lock file by force.
	 *
	 * After our last sleep, make one final attempt to gracefully
	 * create a lock file.
	 */
	if ((f = lock1(curlock)) == -1){
	    /*  Some serious error in creating the lock */
	    exit(1);
	}
	if (f == 0) {
		/*
		 * We got lucky and were able to create the lock file.
		 */
		locked = 1;
		return(0);
	}	
	/* Make one last ck to see if a new lock file has
	 * been made whilst we were asleep.
	 */
	(void) stat(curlock, &sbuf);
	if (original.st_ctime != sbuf.st_ctime) {
		OVERRIDE = 0;
		goto top;
	}
	/* We have to remove the lock file by force.
	 */
	f = unlink(curlock);

	if (f < 0) {
		/* If we can't remove the lock file, send the mail
		 * back and record our complaint.
		 */
		if (errno != ENOENT) {
			(void) openlog(PGMNAME,1, LOG_MAIL);
			syslog(LOG_ERR,"Cannot override mail lock file %s",curlock);
			closelog();
			exit(1);
		}
	}
	OVERRIDE = 1;
	goto top;	/* Rewind */

}

/*
 * Remove the mail lock, and note that we no longer
 * have it locked.
 */

unlock_file()

{
#if defined DOUBLE_LK
	if (lockstyle & LOK_BELL)
#endif
	(void) unlink(curlock);
	locked = 0;
}

/*
 * Attempt to create the lock file.
 * Returns: 0 on success
 *	    1 if the lock already exists
 *	   -1 if a problem exists in creating the lock.
 * If a problem occurs, it displays an error message (stderr & syslog).
 *
 * N.B. This version takes advantage of the O_EXCL flag to atomically
 * create the lock file, and the good errno's back from it.  Other methods
 * need (e.g. link), need a more prolonged create, test, retry scheme.
 */
lock1(name)
	char name[];
{
	register int fd;

	if ((fd = open(name, O_EXCL|O_CREAT|O_RDONLY, 0)) >= 0){
	    (void) close(fd);
	    return(0);
	}

	if (errno == EEXIST)
	    return(1);

	(void) openlog(PGMNAME,1, LOG_MAIL);
	syslog(LOG_ERR, "Cannot create mail lock file %s\n", name);
	closelog();
	fprintf(stderr, MSGSTR(CREAT_LKFILE,"%s: cannot create mail lock file %s\n"), PGMNAME, name);

	/*  Note: ETIMEDOUT ==> NFS file system busy, otherwise probably
	**  directory permissions.  Either way, we can't do it now.
	*/
	return(-1);
}


/*  Simple alarm service routine to enable lockf to timeout
*/
static jmp_buf alrmbuf;
void
alrmser()
{
    longjmp(alrmbuf, 1);
}


/*  Lockf the file descriptor.
**  Modified from the binmail.c routine, the main difference is that
**  we exit on any error conditions.
**  Also, we use setjmp/longjmp, since mailx is compiled with libbsd,
**  which modifies the signal behavior.
*/
int lock (fd)
int fd;
{
	struct stat status;
	void (*f)();

#if defined DOUBLE_LK
	if (lock_style_init() == LOK_INVAL){
	    exit(1);
	}

	/*  Cheat.  Tell the code that we successfully lockf'd the file */
	if (!(lockstyle & LOK_KERNEL))
	    return(1);
#endif

	f=signal(SIGALRM, alrmser);
	alarm(LF_TMOUT);
	if (setjmp(alrmbuf) || lockf (fd, F_LOCK, 0L) == -1){
	    alarm(0);
	    (void) openlog(PGMNAME, 1, LOG_MAIL);
	    syslog(LOG_ERR,"cannot lockf %s", mailname);
	    fprintf(stderr, MSGSTR(LF_ELOCK, "%s: cannot lockf %s\n"), PGMNAME, mailname);
	    /* Unlike binmail (return -1), we simply */
	    exit(1);
	}
	alarm(0);
	signal(SIGALRM, f);

	/* File locked, now do fstat to check link count. */
	if (fstat (fd, &status) == -1) {
		perror ("fstat failed");
		exit (1);
	}
	return (status.st_nlink == 0) ? 0 : 1; 
}
