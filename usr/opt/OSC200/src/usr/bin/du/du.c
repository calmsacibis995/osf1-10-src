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
static char rcsid[] = "@(#)$RCSfile: du.c,v $ $Revision: 4.2.10.4 $ (DEC) $Date: 1993/10/11 16:46:19 $";
#endif
/*
 * HISTORY
 */
/*
 * OSF/1 1.2
 */
/*
 * COMPONENT_NAME: (CMDFS) commands that deal with the file system
 *
 * FUNCTIONS: du
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * 1.23  com/cmd/files/du.c, cmdfiles, bos320, bosarea.9125 6/19/91
 */
/*
**	du -- summarize disk usage
**              du [-akrslx] [name ...]
*/

#include	<stdio.h>
#include 	<unistd.h>
#include 	<stdlib.h>
#include	<string.h>
#include	<sys/limits.h>
#include	<sys/types.h>
#include	<sys/stat.h>
#include	<sys/param.h>
#include	<dirent.h>
#include 	<limits.h>
#include 	<errno.h>

/* NOTE: ubsize should really be UBSIZE unless the -k option is used, except 
 *	 UBSIZE is now defined to be 1024 instead of 512.  XPG4 says 512
 *	 is the default.  POSIX says 512 is default for similar reporting in
 *	 'find' command.  The -k option makes the block size be 1024 bytes.
 */

#define ROUND_UP         (report_bsize-1)
#define UBLOCKS(x)  ((x.st_blocks * S_BLKSIZE + ROUND_UP ) / report_bsize)

#include <locale.h>

#include <nl_types.h>
#include "du_msg.h"
nl_catd catd;
#define MSGSTR(Num,Str) catgets(catd,MS_DU,Num,Str)

int	report_bsize = 512;		/* default is 512-byte, per XPG4 */
					/* Used to be UBSIZE, but UBSIZE */
					/* is defined as 1024		 */

char    *path = NULL;
int 	path_max = 0;
int 	curpath_max = 0;
int 	curname_max = 0;
int 	name_max;

#define EQ(x,y)	(strcmp(x,y)==0)
#define ISDIR() ((Statb.st_mode&S_IFMT)==S_IFDIR)
#define ML	500

struct 	{
	dev_t	dev;
	ino_t	ino;
} ml[ML];
int	linkc = 0;

struct	stat	Statb;
dev_t	current_device;		/* for -x flag */

char    aflag = 0;
char    rflag = 1;	/* POSIX.2a D11 - print message by default */
char    sflag = 0;
char    lflag = 0;
char    xflag = 0;

char    nodotdot[] = "du: fatal error - can't cd to .. (%s)\n";

struct elem {
	struct elem *next;
	char *data;
};

struct list {
	struct elem *first;
	struct elem *last;
};

/* Local ANSI function prototypes */
void list_empty(struct list *);
void list_append(struct list *, char *, ushort_t);
char *list_first(struct list *);
long descend(char *);
int  error(char *);

int
main(int argc, char **argv)
{
	int c, len, status = 0;
	long blocks = 0;
	char *userdir = NULL;
	extern int optind;

	(void) setlocale (LC_ALL,"");
	catd = catopen(MF_DU,NL_CAT_LOCALE);

	while (( c = getopt(argc,argv,"akrslx")) != EOF) {
			switch(c) {
			case 'a':
				aflag++;	/* report for each file */
				sflag = 0;
				break;

			case 'k':		/* 1K blocksize */
				report_bsize = 1024;
				break;

			case 'r':		/* print error messages */
				rflag++;	/* on by default */
				break;

			case 's':		/* summarize */
				sflag++;
				aflag = 0;
				break;

			case 'l':		/* count linked files */
				lflag++;
				break;

			case 'x':
				xflag++;	/* don't cross mount points */
				break;

			default:
				fprintf(stderr,MSGSTR(USAGE,"usage: du [-a|-s] [-krlx] [name ...]\n"));
				exit(2);
				break;
			}
	}
	argc -= optind;
	argv += optind;

	if(!*argv) {
		argv[0] = ".";
		argc = 1;
	}

	for(c=1;c<=argc;c++,argv++) {

		if ((path_max = pathconf(*argv, _PC_PATH_MAX)) == -1)
			path_max = PATH_MAX;

		if ((name_max= pathconf(*argv, _PC_NAME_MAX)) == -1)
			name_max = NAME_MAX;

		if (path_max > curpath_max)  {
 			if (path)
				free(path);

			/* allocate extra byte for null termination */
			if ((path = malloc(path_max+1)) == NULL) {
				status = error(MSGSTR(NOMEM,"Out of memory"));
				exit(status);
			}

			curpath_max = path_max;
		}

		if (name_max > curname_max)  {
 			if (userdir)
				free(userdir);

			/* allocate extra byte for null termination */
			if ((userdir = malloc(name_max+1)) == NULL) {
				status = error(MSGSTR(NOMEM,"Out of memory"));
				exit(status);
			}

			curname_max = name_max;
		}

		if ((char *)getcwd(userdir, name_max+1) == NULL) {
			fprintf(stderr,MSGSTR(CANTFIND,
					 "du: can't find current directory\n"));
			exit(2);
		}

		if ((len = strlen(*argv)) >= (path_max+1)) {
			fprintf(stderr,MSGSTR(NAMETOOLONG,
				"du: %s: name too long\n"), *argv);
			status = 2;
			continue;
		}

		strcpy(path, *argv);

		if(lstat(path,&Statb)<0) {
			status = error(path);
			continue;
		}

		current_device = Statb.st_dev;

		if (!ISDIR()) {
			printf("%ld\t%s\n", UBLOCKS(Statb), path);
			continue;
		}

		if (chdir(path) < 0) {
			status = error(path);
			continue;
		}
		blocks = descend(path + len);
		if(sflag)
			printf("%ld\t%s\n", blocks, path);

		if (chdir(userdir)) {
			status = error(userdir);
			exit(status);
		}
	}

	exit(status);
}

void
list_empty(struct list *listp)
{
	listp->first = listp->last = NULL;
}

void
list_append(struct list *listp, char *data, ushort_t datalen)
{
	struct elem *ep;
	int status;

	if ((ep = (struct elem *) calloc(1, sizeof(struct elem))) == NULL){
		status = error(MSGSTR(NOMEM, "Out of memory"));
		exit(status);
	}
	if ((ep->data = malloc(datalen+1)) == NULL){ 
		status = error(MSGSTR(NOMEM, "Out of memory"));
		exit(status);
	}
		
	memcpy(ep->data, data, datalen);

	/* null terminate data */
	ep->data[datalen] = 0;

	if (listp->first == NULL)
		listp->first = ep;
	else
		listp->last->next = ep;
	listp->last = ep;
}

char *
list_first(struct list *listp)
{
	struct elem *ep;
	char *data;

	if ((ep = listp->first) == NULL)
		return(NULL);
	listp->first = ep->next;
	data = ep->data;
	free((char *)ep);
	return(data);
}

/* On entry, endofname points to the NUL at the end of path and
 * Statb contains a stat of the file whose full name is in path.
 * If path is a directory, it is the working directory.
 */

long
descend(char *endofname)
{
	register struct	dirent	*dp;
	register char	*c1, *c2;
	long blocks = 0;
	DIR	*dir;			/* open directory */
	int	i;
	struct list list;
	char *elem;

	if (xflag && Statb.st_dev != current_device)
		return(0);	/* don't count this file */

	blocks = UBLOCKS(Statb);

	if (!ISDIR()) {
		if(Statb.st_nlink > 1) {
			if (lflag) {
				blocks += Statb.st_nlink - 1;
				blocks /= Statb.st_nlink;
			} else if (linkc<ML) {
				for(i = 0; i <= linkc; ++i) {
					if(ml[i].ino==Statb.st_ino &&
					   ml[i].dev==Statb.st_dev)
						return 0;
				}
				ml[linkc].dev = Statb.st_dev;
				ml[linkc].ino = Statb.st_ino;
				++linkc;
			}
		}
		if(aflag)
			printf("%ld\t%s\n", blocks, path);
		return(blocks);
	}

	if ((dir=opendir(".")) == NULL) {
		error(path);
		return(0);
	}
	list_empty(&list);
	while ((dp=readdir(dir)) != NULL) {
		if(dp->d_fileno==0
			|| EQ(dp->d_name, ".") || EQ(dp->d_name, "..")
			|| dp->d_name[0] == 0)      /* ?? */
				continue;

		/* verify that the path name will not exceed the max length */
		if ((endofname + dp->d_namlen+1) >= &path[path_max+1]) {
			fprintf(stderr,MSGSTR(TOODEEP,"du: %s: TOO DEEP!\n"), 
				path);
			return(blocks);
		}

		list_append(&list, dp->d_name, dp->d_namlen);
	}
	closedir(dir);
	while ((elem = list_first(&list)) != NULL) {
		/* each directory entry */
		c1 = endofname;
		if (*(c1-1) != '/')	/* append '/' only if needed */
			*c1++ = '/';
		c2 = c1;
		c1 = strcpy(c1, elem);
		c1 += strlen(elem);
		free(elem);
		if(lstat(c2, &Statb) < 0) {
			error(path);
			return(0);
		}
		if (ISDIR()) {
			if (chdir(c2)) {
				error(path);
				continue;
			}
			blocks += descend(c1);
			*endofname = '\0';
			if (chdir("..")) {
				fprintf(stderr, MSGSTR(NODOTDOT,nodotdot), path);
				exit(2);
			}
		}
		else blocks += descend(c1);
	}   /* End of loop over entries in a directory */
	*endofname = '\0';
	if(!sflag)
		printf("%ld\t%s\n", blocks, path);
	return(blocks);
}

int
error(char *s)
{
	int status = 2;
	int e;

	if (rflag) {
		e = errno;
		fprintf(stderr, "du: ");
		errno = e;
		perror(s);
	}
	return(status);
}
