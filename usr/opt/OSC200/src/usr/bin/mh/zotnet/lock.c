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
/* lock.c - universal locking routines */
#ifndef	lint
static char ident[] = "@(#)$RCSfile: lock.c,v $ $Revision: 4.2.7.2 $ (DEC) $Date: 1993/12/03 15:23:32 $ devrcs Exp Locker: devbld $";
#endif	/* lint */

#ifdef	MMDFONLY
#define	LOCKONLY
#endif	MMDFONLY

#include <stdio.h>
#ifndef	LOCKONLY
#include "../h/strings.h"
#include "mts.h"
#else	LOCKONLY
#include "strings.h"
#ifdef	MMDFONLY
#include "mmdfonly.h"
#include "mts.h"
#else	not MMDFONLY
#include "lockonly.h"
#endif	not MMDFONLY
#endif	LOCKONLY
#include <sys/types.h>
#include <sys/stat.h>
#ifdef	LOCKF
#include <sys/errno.h>
#include <unistd.h>
#endif	LOCKF
/* for the ULTRIX compatible file lock code */
#include <userpw.h>
#include <errno.h>
#include <syslog.h>
#include <sysexits.h>
int error;

#define	NOTOK	(-1)
#define	OK	0

#define	NULLCP	((char *) 0)

#ifdef	SYS5
#define	index	strchr
#define	rindex	strrchr
#endif	SYS5


extern int  errno;

#ifdef	LOCKONLY
#ifndef	MMDFONLY
char   *lockldir = "/usr/spool/locks";
#endif	not MMDFONLY
#endif	LOCKONLY

static int	b_lkopen(), lockit(), f_lkopen();
static		lockname(), timerON(), timerOFF();

/* long	time (); */

/*  */

int	lkopen (file, access)
register char   *file;
register int     access;
{
    mts_init ("mts");
#if (defined OSF1 || defined OSF) && defined DOUBLE_LK
    /*  Read Global File for lock style */
    if (lock_style_init() == LOK_INVAL)
	return(NOTOK);
#endif
    switch (lockstyle) {
	case LOK_UNIX:
#if defined DOUBLE_LK
	case LOK_KERNEL:
	case LOK_KERNEL|LOK_BELL:
#endif
#if     defined (FLOCK) || defined(LOCKF)
	    return f_lkopen (file, access);
#endif

	default:
	    return b_lkopen (file, access);
	}
}

/*  */

static int  b_lkopen (file, access)
register char   *file;
register int     access;
{
    register int    i,
                    j;
    long    curtime;
    char    curlock[BUFSIZ],
            tmplock[BUFSIZ];
    struct stat st;
#ifdef BSD42
#include <sys/file.h>
#endif

    if (stat (file, &st) == NOTOK)
	return NOTOK;
    lockname (curlock, tmplock, file, (int) st.st_dev, (int) st.st_ino);

    for (i = 0;;)
	switch (lockit (tmplock, curlock)) {
	    case OK: 
		if ((i = open (file, access)) == NOTOK) {
		    j = errno;
		    (void) unlink (curlock);
		    errno = j;
		}
		timerON (curlock, i);
		return i;

	    case NOTOK: 
		if (stat (curlock, &st) == NOTOK) {
		    if (i++ > 5)
			return NOTOK;
		    sleep (5);
		    break;
		}

		i = 0;
#if defined OSF1 || defined OSF
		/*  curtime should be time_t, not "long".
		**  Here, we have a transportable quick-fix
		*/
		curtime = (long)time(NULL);
#else
		(void) time (&curtime);
#endif
		if (curtime < st.st_ctime + 60L)
		    sleep (5);
		else
		    (void) unlink (curlock);
		break;
	}
}


static int  lockit (tmp, file)
register char   *tmp,
	        *file;
{
    register int    fd;

#if defined OSF1 || defined OSF
    /*  Bypass the tmp file.  This avoids a subtle race condition in
    **  NFS file systems.
    */
    if ((fd = open(file, O_EXCL|O_CREAT|O_RDONLY, 0400)) >= 0)
	(void) close(fd);
#else
    if ((fd = creat (tmp, 0400)) == NOTOK)
	return NOTOK;
#ifdef hpux
    write(fd, "MH lock\n",8);
#endif hpux
    (void) close (fd);

    fd = link (tmp, file);
    (void) unlink (tmp);
#endif
    return (fd != NOTOK ? OK : NOTOK);
}

/*  */

static  lockname (curlock, tmplock, file, dev, ino)
register char   *curlock,
	        *tmplock,
	        *file;
register int     dev,
		 ino;
{
    register char  *bp,
                   *cp;

    bp = curlock;
    if ((cp = rindex (file, '/')) == NULL || *++cp == NULL)
	cp = file;
    if (lockldir == NULL || *lockldir == NULL) {
	if (cp != file) {
	    (void) sprintf (bp, "%.*s", cp - file, file);
	    bp += strlen (bp);
	}
    }
    else {
	(void) sprintf (bp, "%s/", lockldir);
	bp += strlen (bp);
    }

#if defined	DOUBLE_LK
    switch (lockstyle & (LOK_BELL|LOK_MMDF))
#else
    switch (lockstyle)
#endif
    {
	case LOK_BELL: 
	default: 
	    (void) sprintf (bp, "%s.lock", cp);
	    break;

	case LOK_MMDF: 
	    (void) sprintf (bp, "LCK%05d.%05d", dev, ino);
	    break;
    }

#if defined OSF1 || defined OSF
    /*  Avoid the use of tmplock, since there are some rare race
    **  conditions possible with NFS mounting.
    */
#else
    if (tmplock) {
	if ((cp = rindex (curlock, '/')) == NULL || *++cp == NULL)
	    (void) strcpy (tmplock, ",LCK.XXXXXX");
	else
	    (void) sprintf (tmplock, "%.*s,LCK.XXXXXX",
		cp - curlock, curlock);
	(void) unlink (mktemp (tmplock));
    }
#endif
}

/*  */

#ifdef	BSD42

#include <sys/file.h>
#ifdef	SUN40
#include <sys/fcntl.h>
#endif	SUN40

static int  f_lkopen (file, access)
register char   *file;
register int     access;
{
    register int    fd,
                    i,
		    j;

    for (i = 0; i < 5; i++) {
#ifdef	LOCKF
	j = access;
	access &= ! O_APPEND;	/* make sure we open at the beginning */
	if ((access & 03) == O_RDONLY) {
	/* We MUST have write permission of lockf won't work */
	/* (Stupid eh?) */
	    access &= ! O_RDONLY;
	    access |= O_RDWR;
	}
#endif	/* LOCKF */
	if ((fd = open (file, access | O_NDELAY)) == NOTOK)
	    return NOTOK;
#ifndef	LOCKF
	if (flock (fd, LOCK_EX | LOCK_NB) != NOTOK)
	    return fd;
#else	/* LOCKF */
	if (lockf (fd, F_TLOCK, 0L) != NOTOK) {
	    /* see if we should be at the end */
	    if (j & O_APPEND) lseek (fd, 0L, L_XTND);
# if defined DOUBLE_LK && (defined OSF1 || defined OSF)
	    if (lockstyle & LOK_BELL){
		/* ULTRIX compat. locking BELL style locking.
		** TBD: better integrate w. the standard MH bell locking code
		*/
		if (lock_file(file) == NOTOK){
		    (void) close(fd);
		    fd = NOTOK;
		}
	    }
# endif
	    return fd;
	}

	/* Fix errno - lockf screws it */
	if (errno == EACCES) errno = EWOULDBLOCK;
#endif	/* LOCKF */
	j = errno;
	(void) close (fd);

	sleep (5);
    }

    (void) close (fd);
    errno = j;
    return NOTOK;
}
#endif	BSD42

/*  */

/* ARGSUSED */

int     lkclose (fd, file)
register int     fd;
register char   *file;
{
    char    curlock[BUFSIZ];
    struct stat st;

    if (fd == NOTOK)
	return OK;
    switch (lockstyle) {
	case LOK_UNIX: 
#if defined DOUBLE_LK
	case LOK_KERNEL:
	case LOK_KERNEL|LOK_BELL:

	    if (lockstyle & LOK_BELL)
		unlock_file();
#endif
#ifdef	BSD42
#ifndef	LOCKF
	    flock (fd, LOCK_UN);
#else	LOCKF
	    lseek (fd, 0L, L_SET); /* make sure we unlock the whole thing */
	    lockf (fd, F_ULOCK, 0L);
#endif	LOCKF
	    break;
#endif	BSD42

	default: 
	    if (fstat (fd, &st) != NOTOK) {
		lockname (curlock, NULLCP, file, (int) st.st_dev, (int) st.st_ino);
		(void) unlink (curlock);
		timerOFF (fd);
	    }
    }

    return (close (fd));
}


/*  */

FILE	*lkfopen (file, mode)
register char   *file,
 	        *mode;
{
    register int    fd;
    register FILE  *fp;

    if ((fd = lkopen (file, strcmp (mode, "r") ? 2 : 0)) == NOTOK)
	return NULL;

    if ((fp = fdopen (fd, mode)) == NULL) {
	(void) close (fd);
	return NULL;
    }

    return fp;
}


/* ARGSUSED */

int	lkfclose (fp, file)
register FILE	*fp;
register char	*file;
{
    char    curlock[BUFSIZ];
    struct stat st;

    if (fp == NULL)
	return OK;

    switch (lockstyle) {
	case LOK_UNIX: 
#if defined DOUBLE_LK
	case LOK_KERNEL:
	case LOK_KERNEL|LOK_BELL:

	    if (lockstyle & LOK_BELL)
		unlock_file();
#endif
#ifdef	BSD42
#ifndef	LOCKF
	    flock (fileno(fp), LOCK_UN);
#else	LOCKF
	    fseek (fp, 0L, 0); /* make sure we unlock the whole thing */
	    lockf (fileno(fp), F_ULOCK, 0L);
#endif	LOCKF
	    break;
#endif	BSD42

	default: 
	    if (fstat (fileno (fp), &st) != NOTOK) {
		lockname (curlock, NULLCP, file, (int) st.st_dev, (int) st.st_ino);
		(void) unlink (curlock);
	    }
    }

    return (fclose (fp));
}

/*  */

#include <signal.h>

#define	NSECS	((unsigned) 20)


struct lock {
    int		 l_fd;
    char	*l_lock;
    struct lock *l_next;
};
#define	NULLP	((struct lock *) 0)

static struct lock *l_top = NULLP;


/* ARGSUSED */

static void alrmser (sig)
int	sig;
{
    register int    j;
    register char  *cp;
    register struct lock   *lp;

#ifndef	BSD42
    (void) signal (SIGALRM, alrmser);
#endif	BSD42

    for (lp = l_top; lp; lp = lp -> l_next)
	if (*(cp = lp -> l_lock) && (j = creat (cp, 0400)) != NOTOK)
	    (void) close (j);

    (void) alarm (NSECS);
}

/*  */

static timerON (lock, fd)
char   *lock;
int	fd;
{
    register struct lock   *lp;

    if ((lp = (struct lock *) malloc ((unsigned) (sizeof *lp))) == NULLP)
	return;			/* XXX */

    lp -> l_fd = fd;
    if ((lp -> l_lock = malloc ((unsigned) (strlen (lock) + 1))) == NULLCP) {
	free ((char *) lp);
	return;			/* XXX */
    }
    (void) strcpy (lp -> l_lock, lock);
    lp -> l_next = NULLP;

    if (l_top)
	lp -> l_next = l_top -> l_next;
    else {
	(void) signal (SIGALRM, alrmser);/* perhaps SIGT{STP,TIN,TOU} */
	(void) alarm (NSECS);
    }
    l_top = lp;
}


static timerOFF (fd)
int	fd;
{
    register struct lock   *pp,
                           *lp;

    (void) alarm (0);

    if (l_top) {
	for (pp = lp = l_top; lp; pp = lp, lp = lp -> l_next)
	    if (lp -> l_fd == fd)
		break;
	if (lp) {
	    if (lp == l_top)
		l_top = lp -> l_next;
	    else
		pp -> l_next = lp -> l_next;

	    free (lp -> l_lock);
	    free ((char *) lp);
	}
    }

    if (l_top)
	(void) alarm (NSECS);
}



/**** ULTRIX style file locking to maintain compatibility ****/
#define	PGMNAME		"MH"

char	*maillock	= ".lock";		/* Lock suffix for mailname */
char	curlock[PATH_MAX];			/* Last used name of lock */
int	locked;					/* To note that we locked it */


#define OLOCKSLEEPS 19 
#define MAXLOADAPPROX 10  /* in place of getla(); */

int OVERRIDE = 0;	/* Flag net if lock file overridden */
int FIRSTSLEEP = 1;
int LOCKSLEEP = 4;	/* Basic # seconds to sleep between checks for
			 * name.lock file existance and to ck the
			 * peak load ave.
			 */



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

	
	/* Basic # of times to sleep & wait for 
	 * name.lock  file to disappear of its' 
	 * own accord before we blow it away.
	 */ 
	int LOCKSLEEPS = OLOCKSLEEPS;	

	if (locked)
		return(0);

        n = strlen(file);
        if (n > sizeof(curlock)-2){
#ifdef	_MTS_H_
            advise(NULLCP, "invalid file name\n");
#else
            fprintf(stderr, "%s: invalid file name\n", PGMNAME);
#endif
	    return(NOTOK);
        }
        (void) strncpy(curlock, file, sizeof(curlock)-1);
        n = sizeof(curlock)-1 - n;
        (void) strncat(curlock, maillock, n);

	statfailed = 0;

top:
/*
 */
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

		if ((f = lock1(curlock)) == NOTOK){
		    /*  Some serious error in creating the lock */
		    return(NOTOK);
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
				(void) openlog(PGMNAME, 1, LOG_MAIL);
				syslog(LOG_ERR,"Cannot stat mail lock file\n");
				closelog();
#ifdef	_MTS_H_		/* Then MH */
				advise(NULLCP, "cannot create mail lock file %s\n", curlock);
#else
				fprintf(stderr, "%s: cannot create mail lock file %s\n", PGMNAME, curlock);
#endif
				return(NOTOK);
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
			(void) openlog(PGMNAME,1, LOG_MAIL);
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
	if ((f = lock1(curlock)) == NOTOK)
	    return(NOTOK);
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
			syslog(LOG_ERR,"Cannot override mail lock file  %s",curlock);
			closelog();
			return(NOTOK);
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

	(void) openlog(PGMNAME, 1, LOG_MAIL);
	syslog(LOG_ERR, "Cannot create mail lock file %s\n", name);
	closelog();
	fprintf(stderr, "%s: cannot create mail lock file %s\n", PGMNAME, name);

	/*  Assume: ETIMEDOUT ==> NFS file system busy, otherwise probably
	**  directory permissions.  Either way, we can't do it now.
	*/
	return(-1);
}


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
		fprintf(stderr, "%s: illegal %s value in %s\n", PGMNAME, buf, tp->fname);
#endif

#if defined OSF1 || defined OSF	 /*  Log in syslog too */
		(void) openlog(PGMNAME,1, LOG_MAIL);
                syslog(LOG_ERR, "illegal %s value in %s", buf, tp->fname);
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
