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
static char	*sccsid = "@(#)$RCSfile: putpw.c,v $ $Revision: 4.2.2.5 $ (DEC) $Date: 1992/11/05 13:30:47 $";
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
#if !defined(lint) && !defined(_NOIDENT)

#endif
/*
 * COMPONENT_NAME: (LIBCADM) Standard C Library System Admin Functions 
 *
 * FUNCTIONS: putpw 
 *
 * ORIGINS: 27 
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 * OBJECT CODE ONLY SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989 
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * putpw.c	1.2  com/lib/c/adm,3.1,8943 9/13/89 10:44:44
 */

/*                                                                    
 * FUNCTION: Update a user description in the file /etc/passwd.
 *		If the file already contains an entry with the same user
 *		name as specified by the parameter, the entry is replaced.
 *		Otherwise, the entry is appended to the end of the file.
 *
 * PARAMETERS: pw - struct passwd *pw;  the description of the users
 *					password entry.
 *
 * RETURN VALUE DESCRIPTIONS: Upon successful conpletion, PUTPW returns
 *				a value of 0. If PUTPW fails a nonzero
 *				value is returned.
 *
 */

#include <stdio.h>
#include <sys/stat.h>
#include <sys/fullstat.h>
#include <fcntl.h>
#include <pwd.h>
#include <unistd.h>

#define PASSWD		"/etc/passwd"
#define OPASSWD		"/etc/opasswd"
#define TMPPASSWD	"/etc/tmppasswd"

/* return codes */
#define FAIL		-1
#define OK		0

/* type of change to password file */
/* #define FALSE	0	defined in <sys/types.h> */
#define MODIFY		1
#define APPEND		2
#define PUTPW		"putpw"

/* 
 * For copy file function 
 *  These defines are vacuous. They maybe needed if the lib/libIN/cp.c 
 *  routine is used.
 */
#define TO_FD	0
#define FROM_FD 0

/* local system call error messages */
#define ERR_FCNT	"can't fcntl, "
#define ERR_OPEN	"can't open, "
#define ERR_SEEK	"can't seek, "
#define ERR_CP		"can't cp files, "

/* local routine error messages */
#define ERR_LOCK	"can't lock %s\n"

#ifdef DEBUGX
#define sys_err(a, b)	sys_err(a, b)
#else
#define sys_err(a, b)
#endif

extern int errno;	/* system error number */

/* file objects for password files - original and temporary */
static int		pwfd  	 = FAIL;
static int		tmppwfd	 = FAIL;
static FILE		*tmppwf  = NULL;

static int		old_audit;	/* this process's auditing status */
static short		change;		/* the type of change performed */


/* Forward references */
static int update();
static int unlock();
static int err_cleanup();
static int do_lock();
static int org2old();
static int tmp2org();
static int putpw_audit();
static int cp();

int
putpw (newp)
struct passwd *newp;	/* the new pwd values */
{

	struct passwd 	*oldp;		/* Holds the current pwd values */
	struct passwd   *getpwent ();

#ifdef  DEBUGX
	printf("putpw: name = %s\n", newp->pw_name);
#endif /* DEBUGX */

	/* check for a NULL pointer */
	if (newp == NULL)
	{
		return (FAIL);
	}

	/* suspend auditing and save the old audit value */

	/* Open and lock the password files */
        if ((pwfd = do_lock (PASSWD)) == FAIL)
	{
		err_cleanup (newp);
		return ((errno == 0) ? FAIL : errno);
	}

	/* open temporary password files */
	tmppwfd = open (TMPPASSWD, O_RDWR | O_CREAT, 0600); 
	if ((tmppwf = fdopen (tmppwfd, "w+")) == NULL)
	{
		sys_err (ERR_OPEN, TMPPASSWD);
		err_cleanup(newp);
		return ((errno == 0) ? FAIL : errno);
	}
	unlink (TMPPASSWD);

	/* copy original and change to temp file */
	change = FALSE;
	setpwent ();
	while ((oldp = getpwent ()) != NULL)
	{
		if (strcmp(oldp->pw_name,newp->pw_name) == 0)
		{
			if (update (newp) == FAIL)
			{
				err_cleanup(newp);
				return ((errno == 0) ? FAIL : errno);
			}
			change = MODIFY;
		}
		else 
		{
			if (update (oldp) == FAIL)
			{
				err_cleanup(newp);
				return ((errno == 0) ? FAIL : errno);
			}
		}

	}
	if (change == FALSE)
	{
		if (update (newp) == FAIL)
		{
			err_cleanup(newp);
			return ((errno == 0) ? FAIL : errno);
		}
		change = APPEND;
	}

	/* flush the temp files */
	(void) fflush (tmppwf);

	/* 
	 * Copy original files to OLD files
	 */
	if (org2old (pwfd, OPASSWD) != OK)
	{
		err_cleanup(newp);
		return ((errno == 0) ? FAIL : errno);
	}
	/*
	 * Copy TMP files to the original
	 */
	if (tmp2org (tmppwfd, pwfd) != OK)
	{
		err_cleanup(newp);
		return ((errno == 0) ? FAIL : errno);
	}

	/*
	 * Unlock files
	 */
	(void) unlock (pwfd);
	/*
	 * Close the password files
	 */
	endpwent ();
	(void) close (pwfd);
	(void) fclose (tmppwf);
	pwfd = tmppwfd = FAIL;
	tmppwf = NULL;
	/* 
	 * Cut audit record
	 */
	(void) putpw_audit (newp, 0);
	return (OK);
}
static int
update (p)
register struct passwd *p;
{

	if (putpwent (p, tmppwf) != OK)
	{	
		return (FAIL);
	}
	return (OK);
}

#define UNLOCK	"in unlock"

static int 
unlock(fd)
int    fd;
{
        static struct flock filelock =
        {
                F_UNLCK, 0, 0, 0, 0
        };                    /* unlock the file  */

        /* unlock the file */
        if ((fcntl(fd, F_SETLK, &filelock)) < 0)
	{
		sys_err (ERR_FCNT, UNLOCK);
		return (FAIL);
	}

        return (OK);
}
static int
err_cleanup (newp)
struct passwd   *newp;
{
	if (pwfd != FAIL)
		close (pwfd);
	if (tmppwf != NULL)
	{
		fclose (tmppwf);
		unlink (TMPPASSWD);
	}

	/* cut an audit record to show a failure */
	(void) putpw_audit (newp, (errno == 0) ? FAIL : errno);

	pwfd = tmppwfd = FAIL;
	tmppwf = NULL;

	return;
}
/* 
 * This function opens and locks the file.  If successful, 0  and
 * the file descriptor are returned; if not, FAIL is returned.            
 */
static int 
do_lock (filename)
char 	*filename;             /* name of file         */
{
        static struct flock filelock =
        {
                F_WRLCK, 0, 0, 0, 0
        };                              /* r/w lock all of file         */
	int 		fd = FAIL;      /* file descriptor      */

        /* open the file read/write                                     */
        if ((fd = open(filename,O_RDWR)) < 0)
        {
		sys_err (ERR_OPEN, filename);
                return (FAIL);
        }

        /* lock all of the file with a read/write lock                  */
        if (fcntl(fd,F_SETLK,&filelock) < 0)
        {
		sys_err (ERR_LOCK, filename);
                return(FAIL);
        }
        return(fd);
}
static int
org2old (fd, oldpath)
int	fd;		/* original file descriptor */
char	*oldpath;	/* pathname to old passwd file */
{
	int  oldfd;	/* file descriptor for old file */

	/* create the old passwd file if it is not present */
	if ((oldfd = open (oldpath, O_WRONLY)) < 0)
		if ((oldfd = open (oldpath, O_CREAT | O_RDWR, 0660)) < 0)
		{
			sys_err (ERR_OPEN, oldpath);
			return (FAIL);
		}

	/* copy original to old */
	if (cp (TO_FD | FROM_FD, fd, oldfd) != OK)
	{
		sys_err (ERR_CP, oldpath);
		return (FAIL);
	}
	close (oldfd);
	return (OK);
}

static int
tmp2org (tmpfd, fd)
int	tmpfd;		/* pathname to tmp passwd file */
int	fd;		/* original file descriptor */
{
	/* go back to the start in the original file */
        if (lseek(fd, (off_t) 0, SEEK_SET) < (off_t) 0)
        {
		sys_err (ERR_SEEK, TMPPASSWD);
                return(FAIL);
        }
	/* copy temp to original */
	if (cp (TO_FD | FROM_FD, tmpfd, fd) != OK)
	{
		sys_err (ERR_CP, TMPPASSWD);
		return (FAIL);
	}
	return (OK);
}
static int
putpw_audit (p, result)
register struct passwd *p;
register int		result;
{
#	define	ADD		"add"	/* the type of change */
#	define CHG		"chg"	/* keep these the same len */

	return (OK);
}

#ifdef  DEBUGX
static void
sys_err (err, str)
char *err;
char *str;
{
	extern char *sys_errlist[]; 	/* system error messages */
	extern int sys_nerr;		/* max system error number */

	perror (strcat (err, str));
	printf("errno: %d\n", errno);

	if (errno <= sys_nerr)
		(void) fprintf(stderr,"%s\n", sys_errlist[errno]);
	perror ("putpw");
	return;
}
#endif /* DEBUGX */

/*
 * Copy a file from the original
 */
#define FILECOPY	"cp:"
#define FAIL		-1
#define OK		0


#define ERR_ALLO	"can't allocate memory, "
#define ERR_READ	"can't read, "
#define ERR_WRIT	"can't write, "
#define ERR_STAT	"can't stat, "
#define ERR_SEEK	"can't seek, "
#define ERR_TRUN	"can't truncat, "

extern  int errno;

static int 
cp (flag, ofd,cfd)
int     flag;		/* unused */
int	ofd;		/* original file */
int	cfd;		/* the copy */
{
        unsigned int 	cpysize;        /* size of cpy file             */
        unsigned int 	orgsize;        /* size of org file             */
        char    	*mbuf;          /* memory location for file     */
        struct stat 	sbuf;           /* stat buffer                  */

	/* go back to the start in the original file */
        if (lseek(ofd, (off_t) 0, SEEK_SET) < (off_t) 0)
        {
		sys_err (ERR_SEEK, FILECOPY);
                return(FAIL);
        }
	/* get the size of the original */
        if (fstat(ofd, &sbuf) < 0)
        {
		sys_err (ERR_STAT, FILECOPY);
                return(FAIL);
        }
        else
                orgsize = sbuf.st_size;
	/* get size of the copy */
        if (fstat(cfd, &sbuf) < 0)
        {
		sys_err (ERR_STAT, FILECOPY);
                return(FAIL);
        }
        else
                cpysize = sbuf.st_size;
	/* Allocate memory for read */
        if ((mbuf = (char *) malloc(orgsize)) == NULL)
        {
		sys_err (ERR_ALLO, FILECOPY);
                return(FAIL);
        }
        if (read(ofd,mbuf,orgsize) != orgsize)
        {
		sys_err (ERR_READ, FILECOPY);
                return(FAIL);
        }
        if (write(cfd,mbuf,orgsize) != orgsize)
        {
		sys_err (ERR_WRIT, FILECOPY);
                return(FAIL);
        }
        if (orgsize < cpysize)
                if (ftruncate(cfd, orgsize) < 0)
                {
			sys_err (ERR_TRUN, FILECOPY);
                        return(FAIL);
                }
        free (mbuf);
        return (OK);
}
/* TCSEC Division C Class C2 */
