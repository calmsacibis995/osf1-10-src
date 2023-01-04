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
static char *rcsid = "@(#)$RCSfile: kmknod.c,v $ $Revision: 1.1.3.5 $ (DEC) $Date: 1992/06/29 10:37:05 $";
#endif

#include	<sys/conf.h>
#include	<sys/sysinfo.h>
#include	<sys/stat.h>
#include	<dirent.h>
#include	<pwd.h>
#include	<grp.h>
#ifdef	DEBUG
#include	<stdio.h>
#endif	/* DEBUG */

/*
 * routine name:	del_by_name()
 * description:		if the named file exists, then delete it
 * input:		char *	filename
 * output:		none
 * assumptions:
 * notes:
 */
del_by_name (n)
char	*n;
{
struct stat 	statbuf;

	if (stat(n, &statbuf) == 0) {
#ifdef	DEBUG
		printf (" (n)%s", n);
		fflush (stdout);
#endif	/* DEBUG */
		if (unlink (n)) {
			perror ("kmknod: del_by_name()");
			exit (0);
		}
	}
}

/*
 * routine name:	del_by_major()
 * description:		search the given directory, and any
 *			subdirectories, for a given major number.  If a
 *			file with that major number is found, then
 *			delete it.
 * input:		int		major number
 *			char	*	directory name
 *			char	*	"b", or "c", for block or character
 *					  device
 * output:
 * assumptions:		directory name is a full path name.
 * notes:
 */
del_by_major (m, dirname, bc)
int	m;
char	*dirname;
char	*bc;
{
DIR		*dirp;
struct dirent	*dp;
char		fname[ANAMELEN];
struct stat 	statbuf;
int		do_unlink;

	do_unlink = 0;
	dirp = opendir(dirname);
	while (dp = readdir(dirp)) {
	    if (strcmp (dp->d_name, ".") && strcmp (dp->d_name, "..")) {
		sprintf (fname, "%s/%s", dirname, dp->d_name);
		stat(fname, &statbuf);
		if (S_ISDIR(statbuf.st_mode)) {
	    	    del_by_major (m, fname, bc);
		} else {
	    	    if (m == major(statbuf.st_rdev)) {
	    	    	if (S_ISBLK(statbuf.st_mode) && (*bc == 'b')) {
#ifdef	DEBUG
	    	    	    printf (" (m)%s", fname);
	    	    	    fflush (stdout);
#endif	/* DEBUG */
	    	    	    if (unlink (fname)) {
	    	    	    	perror ("kmknod: del_by_major()");
	    	    	    	exit (0);
	    	    	    }
	    	    	} else if (S_ISCHR(statbuf.st_mode) && (*bc == 'c')) {
#ifdef	DEBUG
	    	    	    printf (" (m)%s", fname);
	    	    	    fflush (stdout);
#endif	/* DEBUG */
	    	    	    if (unlink (fname)) {
	    	    	    	perror ("kmknod: del_by_major()");
	    	    	    	exit (0);
	    	    	    }
	    	    	}
	    	    }
		}
	    }
	}
	closedir(dirp);
}

/*
 * routine name:	check_dir()
 * description:		determine if the named directory exists.  If it
 *			does exist, then return 0.  If it does not
 *			exist, then recursively call check_dir() to
 *			determine if the parent exists.  Once the
 *			parents existence has been confirmed, or the
 *			parent has been created, then create the named
 *			directory.
 * input:		char	*	directory name
 * output:		success:	0
 *			failure:	-1
 * assumptions:		directory name is a full path name.
 * notes:
 */
check_dir (dir)
char	*dir;
{
extern char *strrchr();
struct stat 	statbuf;
int		ret;
char		dir2[ANAMELEN];
char		*d;

	if (stat(dir, &statbuf) == 0) {
		if (S_ISDIR(statbuf.st_mode)) {
			ret = 0;
		} else {
			ret = -1;
		}
	} else {
		strcpy (dir2, dir);
		d = strrchr (dir2, '/');
		*d = '\0';
		ret = check_dir (dir2);
		if (ret == 0) {
			ret = mkdir (dir, 0755);
		}
	}
	return (ret);
}

/*
 * utility name:	kmknod
 * description:		search the kernel assign_table[].  For each
 *			entry in this table delete the named special
 *			device file, and delete any other special
 *			device file which has a matching major number.
 * input:		none
 * output:		none
 * assumptions:
 * notes:		mode needs to be added to assign_entry.
 * errors:		- a call to unlink() fails
 *			- a call to getsysinfo() fails
 *			- a call to mknod() fails
 */
main()
{
int		asize;
Assign_entry	abuf;
int		minptr;
char		*devptr;
int		ret;
int		first;
int		not_done;
char		name[ANAMELEN];
int		md;
struct passwd	*pw;
struct group	*gr;


#ifdef	DEBUG
	printf ("kmknod: deleting...");
	fflush (stdout);
#endif	/* DEBUG */
	asize = sizeof (Assign_entry);
	minptr = 0;
	devptr = 0;
	not_done = 1;
	first = 1;

	/* first loop thru assign_table looking for files to delete
	 */
	while (not_done) {
		ret = getsysinfo (GSI_STATIC_DEF, &abuf, asize, 
		                  &minptr, &devptr);
		if (ret > 0) {
			sprintf (name, "%s/%s", abuf.dirname, abuf.dev_name);
			del_by_name (name);
			if (first) {
				del_by_major (abuf.ae_major, "/dev", abuf.bc);
				first = 0;
			}
			if (minptr == 0) {
				first = 1;
			}
			if ((minptr == 0) && (devptr == 0)) {
				not_done = 0;
			}
		} else {
			not_done = 0;
		}
	}
#ifdef	DEBUG
	printf ("\n");
	fflush (stdout);
#endif	/* DEBUG */

	/* check for error from getsysinfo()
	 */
	if (ret < 0) {
		perror ("kmknod: getsysinfo()");
		exit (0);
	}

	/* second loop to make the new files.
	 */
	not_done = 1;
#ifdef	DEBUG
	printf ("kmknod: creating...");
	fflush (stdout);
#endif	/* DEBUG */
	while (not_done) {
		ret = getsysinfo (GSI_STATIC_DEF, &abuf, asize, 
		                  &minptr, &devptr);
		if (ret > 0) {
			if (strcmp (abuf.bc, "b") == 0) {
				md = S_IFBLK;
			} else {
				md = S_IFCHR;
			}
			md |= abuf.mode;
			ret = check_dir (abuf.dirname);
			if (ret) {
				not_done = 0;
			} else {
				sprintf (name, "%s/%s", abuf.dirname, 
				    abuf.dev_name);
#ifdef	DEBUG
				printf (" %s", name);
				fflush (stdout);
#endif	/* DEBUG */
				ret = mknod (name, md, makedev(abuf.ae_major, 
				    abuf.ae_minor));
				if (ret) {
					not_done = 0;
				} else {
					ret = chmod (name, abuf.mode);
					if (strlen(abuf.username)) {
						pw = getpwnam(abuf.username);
						if (pw) {
							abuf.user = pw->pw_uid;
						}
					}
					if (strlen(abuf.groupname)) {
						gr = getgrnam(abuf.groupname);
						if (gr) {
							abuf.group = gr->gr_gid;
						}
					}
					ret = chown (name, abuf.user, 
					    abuf.group);
				}
			}
			if ((minptr == 0) && (devptr == 0)) {
				not_done = 0;
			}
		} else {
			not_done = 0;
		}
	}
#ifdef	DEBUG
	printf ("\n");
	fflush (stdout);
#endif	/* DEBUG */

	/* check for error from getsysinfo() or mknod()
	 */
	if (ret < 0) {
		perror ("kmknod: getsysinfo() or mknod()");
		exit (0);
	}

}
