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
static char rcsid[] = "@(#)$RCSfile: rm.c,v $ $Revision: 4.2.9.3 $ (DEC) $Date: 1993/10/11 17:42:55 $";
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
 * FUNCTIONS: rm, rmdir
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * rm.c       1.23  com/cmd/files bos320 5/31/91 10:34:03
 */

/*
**	rm [-fir] file ...  (also recognizes -R==-r, and -e for echo)
**	rmdir file ...
*/

#include	<stdio.h>
#include	<sys/types.h>
#include	<sys/limits.h>	/* for PATH_MAX */
#include	<sys/stat.h>
#include	<dirent.h>
#include	<sys/access.h>
#include	<sys/mode.h>
#include	<sys/errno.h>
#include 	<locale.h>
#include 	<nl_types.h>
#include 	<string.h>
#include 	"rm_msg.h"

nl_catd catd;
#define MSGSTR(Num,Str) catgets(catd,MS_RM,Num,Str)

extern int optind;
extern char *optarg;

void append(char *);

int	errcode;

int fflg, iflg, rflg, eflg = 0;         /* options for rm command */

/*
 * NAME: rm [-fiRre] file1 ... fileN
 *       rmdir [-p] dir1 ... dirN
 *
 * FUNCTION: Removes file and directory entries.
 *
 * NOTES:   -f  do not prompt for confirmation
 *	    -i  prompt for confirmation
 *          -R  remove file hierarchies
 *          -r  same as -R
 *          -e  displays a message after deleting each file 
 *
 *          -p  remove all directories in a pathname
 */

main(int argc, char **argv)
{
    char *name;
    int rv = 2;

    (void) setlocale (LC_ALL,"");
    catd = catopen(MF_RM,NL_CAT_LOCALE);

    /* find our cmdname */
    name = strrchr(argv[0],'/');
    if (name) name++; else name = argv[0];

    if (!strcmp(name,"rmdir")) rv = rmdir_main(argc,argv);
    else rv = rm_main(argc, argv);	/* assume rm */
    exit(rv);
}

/*
 * NAME: rm_main
 *
 * FUNCTION: main rm routine -- checks options and calls rm
 */
int
rm_main(int argc, char *argv[])
{
    register char *arg;
    int c;

	while ((c=getopt(argc, argv, "fiRrep")) != EOF) 
	  switch(c) {
		case 'f':
			fflg++;
			iflg = 0;
			break;
		case 'i':
			iflg++;
			fflg = 0;
			break;
		case 'R':
		case 'r':
			rflg++;
			break;
		case 'e':
			eflg++;
			break;
		default:
		    usage();
		}

    argc -= optind;
    argv += optind;

    if (argc < 1 && !fflg) {
	usage();
    }

    while (argc-- > 0) {
	arg = strrchr(*argv, '/');
	if (arg) arg++; else arg = *argv;
	if(dotname(arg)) {
		fprintf(stderr, 
			MSGSTR(CANTREMOVE,"rm: cannot remove '.' or '..'\n"));
		errcode++;
		argv++;
		continue;
	}
	rm(*argv, 0);
	argv++;
    } 

    return(errcode?2:0);
}

/*
 * NAME: rm
 *
 * FUNCTION: (recursive) removes files and directories 
 */

char	*path;		/* pointer to malloc'ed buffer for path */
char	*pathp;		/* current pointer to end of path */
int	pathsz;		/* size of path */
int	memexpand=0;	/* indicated we have realloc'd path buffer */

#define	isdot(a)	(a[0] == '.' && (!a[1] || a[1] == '.' && !a[2]))
/*
 * Return TRUE if sucessful. Recursive with -r (rflg)
 */
int
rm(char *arg, int level)
{
	int ok;				/* true if recursive rm succeeded */
	struct stat buf;		/* for finding out what a file is */
	struct dirent *dp;		/* for reading a directory */
	DIR *dirp;			/* for reading a directory */
	char prevname[PATH_MAX + 1];	/* previous name for -r */
	char *cp, *rindex();

	if (lstat(arg, &buf)) {
		if (!fflg) {
			fprintf(stderr, "rm: ");
			perror(arg);
			errcode++;
		}
		return (0);		/* error */
	}
	if ((buf.st_mode&S_IFMT) == S_IFDIR) {
		if (!rflg) {
			if (!fflg) {
				fprintf(stderr, MSGSTR(ISDIRECT, 
					"rm: %s directory\n"), arg);
				errcode++;
			}
			return (0);
		}
		if (iflg && level != 0) {
			fprintf(stderr, MSGSTR(DIRECTORY, 
				"rm: remove files in directory %s? "), arg);
			if (!yes())
				return (0);	/* didn't remove everything */
		}
		if (access(arg, R_OK|W_OK|X_OK) != 0) {
			if (rmdir(arg) == 0)
				return (1);  /* salvaged: removed empty dir */
			if (!fflg) {
				fprintf(stderr, MSGSTR(NOCHANGE, 
					"rm: %s not changed\n"), arg);
				errcode++;
			}
			return (0);		/* error */
		}
		if ((dirp = opendir(arg)) == NULL) {
			if (!fflg) {
				fprintf(stderr, MSGSTR(CANTREAD, 
					"rm: cannot read %s\n"), arg);
				errcode++;
			}
			return (0);
		}
		if (level == 0)
			append(arg);
		prevname[0] = '\0';
		while ((dp = readdir(dirp)) != NULL) {
			if (isdot(dp->d_name)) {
				strcpy(prevname, dp->d_name);
				continue;
			}
			append(dp->d_name);
			closedir(dirp);
			ok = rm(path, level + 1);
			for (cp = pathp; *--cp != '/' && cp > path; )
				;
			pathp = cp;
			*cp++ = '\0';
			if (memexpand && level > 0) 
				/* reset arg since path may have moved */
				arg = path; 
			if ((dirp = opendir(arg)) == NULL) {
				if (!fflg) {
					fprintf(stderr, MSGSTR(CANTREAD, 
						"rm: cannot read %s\n"), arg);
					errcode++;
				}
				break;
			}
			/* pick up where we left off */
			if (prevname[0] != '\0') {
				while ((dp = readdir(dirp)) != NULL &&
				    strcmp(prevname, dp->d_name) != 0)
					;
			}
			/* skip the one we just failed to delete */
			if (!ok) {
				dp = readdir(dirp);
				if (dp != NULL && strcmp(cp, dp->d_name)) {
					fprintf(stderr, MSGSTR(SYNCERR, 
			"rm: internal synchronization error: %s, %s, %s\n"),
						arg, cp, dp->d_name);
				}
				strcpy(prevname, dp->d_name);
			}
		}
		closedir(dirp);
		if (level == 0) {
			pathp = path;
			*pathp = '\0';
		}
		if (iflg) {
			fprintf(stderr, MSGSTR(REMPROMPT, 
				"rm: remove %s? "), arg);
			if (!yes())
				return (0);
		}
		if (rmdir(arg) < 0) {
			if (!fflg || iflg) {
				fprintf(stderr, MSGSTR(NOTREMOVED, 
					"rm: %s not removed. "), arg);
				perror("");
				errcode++;
			}
			return (0);
		}
		if (eflg)
			fprintf(stderr, MSGSTR(REMOVEDDIR, 
				"rm: removing directory %s\n"), arg);
		return (1);
	}

	if (!fflg) {
		if ((buf.st_mode&S_IFMT) != S_IFLNK && access(arg, W_OK) < 0) {
			fprintf(stderr, MSGSTR(OVERRIDE, 
				"rm: override protection %o for %s? "),
				buf.st_mode&0777, arg);
			if (!yes())
				return (0);
			goto rm;
		}
	}
	if (iflg) {
		fprintf(stderr, MSGSTR(REMPROMPT, "rm: remove %s? "), arg);
		if (!yes())
			return (0);
	}
rm:	if (unlink(arg) < 0) {
		if (!fflg || iflg) {
			fprintf(stderr, "rm: %s: ", arg);
			perror("");
			errcode++;
		}
		return (0);
	} else if (eflg)
		fprintf(stderr, MSGSTR(FILEREMOVED, "rm: removing %s\n"), arg);

	return (1);
}

/*
 * NAME: dotname
 * 
 * FUNCTION: Test whether s is "." or ".."
 * 
 * RETURNS:  1 - yes, s is "." or ".."    
 * 	     0 - no
 */
int
dotname(char *s)
{
	return(s[0] == '.' && (!s[1] || s[1] == '.' && !s[2]));
}

/*
 * NAME: rmdir_main
 *
 * FUNCTION: main routine for rmdir command
 */
int
rmdir_main(int argc, char **argv)
{
    char rc = 0;
    int Errors = 0;
    int c;
    int pflg = 0;

    while ((c = getopt(argc, argv, "p")) != EOF)
    switch(c) {
	case 'p':
		pflg++;
		break;
	default:
		rmdir_usage();
		/* break; 	line warning: statement not reached */
	}

    argc -= optind;
    argv += optind;
		
    if(argc < 1) {
	rmdir_usage();
    }

    while(argc--) {
	char *cp = *argv++;
	char *p;
	int i;
	i = strlen(cp) - 1;
	if (cp[i] == '/')
		cp[i] = '\0';    /* remove '/' at end of pathname if any */
	do {
		if ((rc = rmdir(cp)) != 0) {
		    fprintf(stderr,"rmdir: ");
		    perror(cp);
		    ++Errors;
		    break;
		}
		if (p=strrchr(cp,'/'))  
			*p = '\0';		/* remove last dirname*/
		else
			break;
	    } while(pflg && *cp);
    }
    return(Errors? 2: 0);
}

/*
 * NAME: yes
 *
 * FUNCTION: gets interactive response from user
 */

int
yes(void)
{
	char response[100];

	gets(response);
	return(rpmatch(response) == 1);
}

/*
 * Append 'name' to 'path'.
 */
void
append(char *name)
{
	register int n;
	int   offset;
	char *malloc();

	n = strlen(name);
	if (path == NULL) {
		pathsz = PATH_MAX + 2;
		if ((path = malloc((u_int)pathsz)) == NULL) {
			fprintf(stderr,MSGSTR(NOMEM,"rm: ran out of memory\n"));
			exit(1);
		}
		pathp = path;
	} else if (pathp + n + 2 > path + pathsz) {
		offset = pathp - path;		/* save pathp */
		pathsz += PATH_MAX;		/* increment the buffer size */
		if ((path = (char *)realloc(path, pathsz)) == NULL) {
			fprintf(stderr,MSGSTR(NOMEM,"rm: ran out of memory\n"));
			exit(1);
		}
		memexpand++;
		pathp = path + offset;
	}
	if (pathp != path && pathp[-1] != '/')
		*pathp++ = '/';
	strcpy(pathp, name);
	pathp += n;
}

usage(void)
{
	fprintf(stderr, MSGSTR(USAGE,"usage: rm [-efirR] file ...\n"));
	exit(2);
}

rmdir_usage(void)
{
	fprintf(stderr, MSGSTR(USAGE_RMDIR,"usage: rmdir [-p] dirname ...\n"));
	exit(2);
}
