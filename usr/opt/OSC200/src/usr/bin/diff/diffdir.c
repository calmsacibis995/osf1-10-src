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
static char rcsid[] = "@(#)$RCSfile: diffdir.c,v $ $Revision: 4.2.9.3 $ (DEC) $Date: 1993/12/09 20:32:26 $";
#endif
/*
 * HISTORY
 */
/*
 * OSF/1 1.2
 */
/*
 * COMPONENT_NAME: (CMDFILES) commands that manipulate files
 *
 * FUNCTIONS:
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * diffdir.c	1.10  com/cmd/files/diff/diffdir.c, bos320 4/28/91 20:35:18";
 */

/*
 * diff - directory comparison
 */
#include <stdio.h>
#include <stdlib.h>

#include <ctype.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <signal.h>
#include <dirent.h>
#include <unistd.h>
#include "diff.h"

extern nl_catd catd;

/*
 * Output format options
 */
extern int	opt;

/*
 * Options on hierarchical diffs.
 */
extern int	lflag;			/* long output format with header */
extern int	rflag;			/* recursively trace directories */
extern int	sflag;			/* announce files which are same */
extern char	*begin;			/* do file only if name >= this */

/*
 * State for exit status.
 */
extern int	status;
extern int oldstatus;
int	anychange;

/*
 * Variables for diffdir.
 */
extern char	**diffargv;	/* option list to pass to recursive diffs */

/*
 * Input file names.
 * With diffdir, file1 and file2 are allocated BUFSIZ space,
 * and padded with a '/', and then efile0 and efile1 point after
 * the '/'.
 */
extern char	*file1, *file2, *efile1, *efile2;
extern struct	stat stb1, stb2;
extern int	done();

#define	ONLY	1		/* Only in this directory */
#define	SAME	2		/* Both places and same */
#define	DIFFER	4		/* Both places and different */
#define	DIRECT	8		/* Directory */
#define INIT	16
#define MAGIC	"/etc/magic"

extern char diffh[], diff[], pr[];
/*
extern	pid_t	fork(), wait();
*/
extern int name_max;	/* Passes in NAME_MAX */
extern int optind;

struct	dirent **setupdir();
int	header;
char	title[2*BUFSIZ], *etitle;

/* 
 * NAME: diffdir
 * FUNCTION: directs diffs between directories
 */
diffdir(argv)
char **argv;
{
	struct dirent **dir1, **dir2;
	int nentries1,nentries2;
	register int i,j;
	int cmp;

	if (opt == DI_IFDEF) {
		fprintf(stderr,MSGSTR(ENOI,
			 "diff: can't specify -D with directories\n"));
		status=2;
		done();
	}
	if (opt == DI_EDIT && (sflag || lflag))
		fprintf(stderr,MSGSTR(ENOSL,
		    "diff: warning: shouldn't give -s or -l with -e\n"));
	title[0] = 0;
	strcpy(title, "diff ");
	for (i = 1; diffargv[i+2]; i++) {
		strcat(title, diffargv[i]);
		strcat(title, " ");
	}
	for (etitle = title; *etitle; etitle++)
		;
	setfile(&file1, &efile1, file1);
	setfile(&file2, &efile2, file2);
	argv[optind] = file1;
	argv[optind+1] = file2;
	dir1 = setupdir(file1,&nentries1);
	dir2 = setupdir(file2,&nentries2);
	i = j = 0;
	while (i < nentries1 || j < nentries2) {
		if (i < nentries1 && dir1[i]->d_name[0] && !useless(dir1[i])) {
			i++;
			continue;
		}
		if (j < nentries2 && dir2[j]->d_name[0] && !useless(dir2[j])) {
			j++;
			continue;
		}
		if (i >= nentries1 || dir1[i]->d_name[0] == 0)
			cmp = 1;
		else if (j >= nentries2 || dir2[j]->d_name[0] == 0)
			cmp = -1;
		else
			cmp = strncmp(dir1[i]->d_name,dir2[j]->d_name,name_max+1);
		if (cmp < 0) {
			if (lflag)
				dir1[i]->d_ino |= ONLY;
			else if (opt == 0 || opt == 2) {
				only(dir1[i], 1);
				printf(": %.*s\n", name_max+1, dir1[i]->d_name);
			}
			i++;
		} else if (cmp == 0) {
			compare(dir1[i]);
			i++;
			j++;
		} else {
			if (lflag)
				dir2[j]->d_ino |= ONLY;
			else if (opt == 0 || opt == 2) {
				only(dir2[j], 2);
				printf(": %.*s\n", name_max+1, dir2[j]->d_name);
			}
			j++;
		}
	} /* end of while loop */
	if (lflag) {
		scanpr(dir1,ONLY,MSGSTR(EONLY,"Only in %.*s"), file1, efile1,
						nentries1);
		scanpr(dir2,ONLY,MSGSTR(EONLY,"Only in %.*s"), file2, efile2,
						nentries2);
		scanpr(dir1,SAME,MSGSTR(EIDENT,"Common identical files"), 0, 0,
						nentries1);
		scanpr(dir1,DIFFER,MSGSTR(EBINARY,"Binary files which differ")
						, 0, 0, nentries1);
		scanpr(dir1,DIRECT,MSGSTR(ESUBDIR,"Common subdirectories")
						, 0, 0, nentries1);
	}
	if (rflag) {
		if (header && lflag)
			printf("\f");
		for (i = 0; i < nentries1; i++)  {
			if ((dir1[i]->d_ino & DIRECT) == 0)
				continue;
			strncpy(efile1, dir1[i]->d_name, name_max+1);
			strncpy(efile2, dir1[i]->d_name, name_max+1);
			calldiff(0);
		}
	}
}

/*
 * NAME: setfile
 * FUNCTION: finds the end of the path name and adds a / and epp points at
 *   the character following the added /.
 */
setfile(fpp, epp, file)
	char **fpp, **epp;
	char *file;
{
	register char *cp;

	*fpp = malloc((size_t)BUFSIZ);
	if (*fpp == 0) {
		fprintf(stderr,MSGSTR(EMEM, "diff: ran out of memory\n"));
		status=2;
		exit(1);
	}
	strcpy(*fpp, file);
	for (cp = *fpp; *cp; cp++)
		continue;
	*cp++ = '/';
	*cp = '\0';
	*epp = cp;
}

/*
 * NAME: scanpr
 * FUNCTION: scan the list of files in a directory and prints out its
 *   name if it passes the test.
 */
scanpr(dp, test, title, file, efile,nentries)
register struct dirent **dp;
int test;
char *title, *file, *efile;
int nentries;
{
	int titled = 0,i;

	for (i=0;i<nentries;i++) {
		if (dp[i]->d_ino & test) {
			if (titled == 0) {
				if (header == 0) {
					if (anychange)
						printf("\f");
					header = 1;
				} else
					printf("\n");
				printf(title, efile - file - 1, file);
				printf(":\n");
				titled = 1;
			}
			ptname(dp[i]);
		}
	}
}

/*
 * NAME: only
 * FUNCTION: prints outs the message associated with a file that is in only
 *   one of the directories.
 */ 
only(dp, which)
	struct dirent *dp;
	int which;
{
	char *file = which == 1 ? file1 : file2;
	char *efile = which == 1 ? efile1 : efile2;

	printf(MSGSTR(EONLY,"Only in %.*s"),
		 efile - file - 1, file, name_max+1, dp->d_name);
	oldstatus = status = 1;
}

/*
 * NAME: ptname
 * FUNCTION: prints out the name of a file
 */
ptname(dp)
	struct dirent *dp;
{

	printf("\t%.*s\n", name_max+1, dp->d_name);
}


/*
 * NAME: setupdir
 * FUNCTION: gets all the valid entires for the directory cp
 */
struct dirent **
setupdir(cp,nentries)
char *cp;
int *nentries;
{
	struct dirent **queue;
	int useless(),alphasort(),i;

	if ((*nentries = scandir(cp,&queue,useless,alphasort)) < 0) {
		perror(cp);
		status=2;
		exit(1);
	}
	if (lflag || rflag)
		for (i = 0; i < *nentries; i++)
			queue[i]->d_ino = INIT;
	return(queue);
}


/*
 * NAME: compare
 * FUNCTION: compares two files to determine if they are different or the 
 *    same.
 */
compare(dp)
	register struct dirent *dp;
{
	register int i, j;
	int f1, f2, fmt1, fmt2;
	struct stat stb1, stb2;
	int flag = 0;
	char *type1, *type2;
	char buf1[BUFSIZ], buf2[BUFSIZ];


	strncpy(efile1, dp->d_name, name_max+1);
	strncpy(efile2, dp->d_name, name_max+1);
	if (lflag)
		dp->d_ino = SAME;
	f1 = open(file1, 0);
	if (f1 < 0) {
		perror(file1);
		status=2;
		return;
	}
	f2 = open(file2, 0);
	if (f2 < 0) {
		perror(file2);
		status=2;
		close(f1);
		return;
	}
	fstat(f1, &stb1);
 	fstat(f2, &stb2);
	fmt1 = stb1.st_mode & S_IFMT;
	fmt2 = stb2.st_mode & S_IFMT;
	if (fmt1 != S_IFREG || fmt2 != S_IFREG) {
		if (fmt1 == fmt2) {
			if (fmt1 != S_IFDIR && stb1.st_rdev == stb2.st_rdev)
				goto same;
			if (fmt1 == S_IFDIR) {
				if (slinkcmp(file1, file2, dp)) goto closem;
				if (lflag || opt == DI_EDIT || rflag) {
					dp->d_ino = DIRECT;
					goto closem;
				}
				printf(MSGSTR(SUBDIR,
					"Common subdirectories: %s and %s\n"),
				    file1, file2);
				goto closem;
			}
		}
		/* file names are same, but the types are different */
		/* This message is printed when no '-e' or '-f' specified */
		if (opt != DI_EDIT && opt != DI_REVERSE) {
			switch(fmt1) {
			case S_IFDIR:
				type1 = MSGSTR(MDIR,"directory");
				break;
			case S_IFREG:
				type1 = MSGSTR(MREG,"regular file");
				break;
			case S_IFCHR:
				type1 = MSGSTR(MCHAR,"character special");
				break;
			case S_IFLNK:
				type1 = MSGSTR(MLINK,"symbolic link"); 
				break;
			case S_IFIFO:
				type1 = MSGSTR(MFIFO,"fifo");
				break;
			case S_IFBLK:
				type1 = MSGSTR(MBLOCK,"block special");
				break;
			default:
				type1 = MSGSTR(UNKNOWN,"unknown");
				break;
			}

			switch(fmt2) {
			case S_IFDIR:
				type2 = MSGSTR(MDIR,"directory");
				break;
			case S_IFREG:
				type2 = MSGSTR(MREG,"regular file");
				break;
			case S_IFCHR:
				type2 = MSGSTR(MCHAR,"character special");
				break;
			case S_IFLNK:
				type2 = MSGSTR(MLINK,"symbolic link"); 
				break;
			case S_IFIFO:
				type2 = MSGSTR(MFIFO,"fifo");
				break;
			case S_IFBLK:
				type2 = MSGSTR(MBLOCK,"block special");
				break;
			default:
				type2 = MSGSTR(UNKNOWN,"unknown");
				break;
			}

			printf(MSGSTR(TYPDIF,
				"File %s is a %s while file %s is a %s\n"),
		    	file1, type1, file2, type2);
		}
		goto closem;
	}
	if (stb1.st_size != stb2.st_size)
		goto notsame;
	for (;;) {
		i = read(f1, buf1, BUFSIZ);
		j = read(f2, buf2, BUFSIZ);
		if (i < 0 || j < 0 || i != j)
			goto notsame;
		if (i == 0 && j == 0)
			goto same;
		for (j = 0; j < i; j++)
			if (buf1[j] != buf2[j])
				goto notsame;
	}
same:
	if (sflag == 0)
		goto closem;
	if (lflag)
		dp->d_ino = SAME;
	else
		printf(MSGSTR(FILES,
			"Files %s and %s are identical\n"), file1, file2);
	goto closem;
notsame:
	oldstatus = status=1;
	if (!ascii(f1) || !ascii(f2)) {
		if (lflag)
			dp->d_ino = DIFFER;
		else if (opt == DI_NORMAL || opt == DI_CONTEXT)
			printf(MSGSTR(DBINARY,"Binary files %s and %s differ\n")
			,file1, file2);
		goto closem;
	}
	if (lflag)
		dp->d_ino = INIT;
	close(f1); 
	close(f2);
	anychange = 1;
	if (lflag)
		calldiff(title);
	else {
		if (opt == DI_EDIT) {
			printf("ed - %.*s << '-*-END-*-'\n",
			    name_max+1, dp->d_name);
			calldiff(0);
		} else {
			printf("%s%s %s\n", title, file1, file2);
			calldiff(0);
		}
		if (opt == DI_EDIT)
			printf("w\nq\n-*-END-*-\n");
	}
	return;
closem:
	close(f1); 
	close(f2);
}

char	*prargs[] = { "pr", "-h", 0, "-f", 0, 0 };

/*
 * NAME: calldiff
 * FUNCTION: fork a second process and call diff again 
 */
calldiff(wantpr)
char *wantpr;
{
	int status1, status2, pv[2];
	pid_t pid;

	prargs[2] = wantpr;
	fflush(stdout);
	if (wantpr) {
		sprintf(etitle, "%s %s", file1, file2);
		pipe(pv);
		pid = fork();
		if (pid == -1) {
			perror("diff");
			status=2;
			done();
		}
		if (pid == 0) {
			close(0);
			dup(pv[0]);
			close(pv[0]);
			close(pv[1]);
			execv(pr, prargs);
			perror(pr);
			status=2;
			done();
		}
	}
		/* Reset optind for forked process getopt() */
	optind = 1;
	pid = fork();
	if (pid == -1) {
		perror("diff");
		status=2;
		done();
	}
	if (pid == 0) {
		if (wantpr) {
			close(1);
			dup(pv[1]);
			close(pv[0]);
			close(pv[1]);
		}
		execv(diff, diffargv);
		perror(diff);
		status=2;
		done();
	}
	if (wantpr) {
		close(pv[0]);
		close(pv[1]);
	}
	while (wait(&status1) != pid)
		continue;
	while (wait(&status2) != -1)
		continue;
	status = status1 >> 8;
    if (oldstatus < status)
		oldstatus = status;
	else
		status = oldstatus;
/*
	if ((status >> 8) >= 2)
		done();
*/
}

/*
 * NAME: ascii
 * FUNCITON: checks to see if file is ascii
 */
ascii(f)
int f;
{
	char buf[BUFSIZ], cmpbuf[BUFSIZ], s1[BUFSIZ], msg[BUFSIZ];
	char type[40], svalue[40], *tmp;
	FILE *fd;
	long int prevoff, offset;
	long unsigned nvalue;

	if ((fd = fopen(MAGIC,"r")) == NULL) {
		fprintf(stderr,MSGSTR(DMAGIC,"Can not open %s file\n"),MAGIC);
		return(1);
	}
	prevoff = -1;
	while ((fgets(buf,BUFSIZ,fd)) != NULL) {
		if (!iswdigit((wint_t)buf[0]))
			continue;
		sscanf(buf,"%s %s %s %s",s1,type,svalue,msg);
		offset = strtoul(s1,&tmp,0);
		if (strcmp("short",type) != 0 &&
		    strcmp("long",type) != 0 &&
		    strcmp("byte",type) != 0 )
			continue;
		nvalue = strtoul(svalue, NULL, 0);
		if (offset != prevoff) {
			lseek(f, offset, 0);
			bzero(cmpbuf, BUFSIZ);
			read(f, cmpbuf, BUFSIZ);
			prevoff = offset;
		}
		if (strcmp("short",type) == 0) {
			if (nvalue != (long)(*(unsigned short *) cmpbuf))
				continue;
		} else if (strcmp("long",type) == 0) {
			if (nvalue != (*(long *) cmpbuf))
				continue;
		} else if (nvalue != (long)(*(unsigned char *) cmpbuf))
			continue;
		(void) fclose(fd);
		return(0);
	}
	(void) fclose(fd);
	/* At this point it looks like the file might be "ASCII",
	 * but there are "string"-type entries in /etc/magic which
	 * identify binary files too, but the string-type entries are
	 * skipped above.  The file(1) command also knows about binary
	 * files that are not in /etc/magic, but file(1) returns strings
	 * which may be internationalized, so we can't depend on those
	 * message strings.
	 *
	 * Therefore, for now, lets fall back to the old reliable method
	 * of checking if files are a.out files, or contain non-text
	 * characters.
	 */
	return(ascii_bsd(f));
}

#include <a.out.h>
#ifdef	multimax
#define	BADMAG(X) (X.f_magic != NS32GMAGIC && X.f_magic != NS32SMAGIC)
#endif	/* multimax */
#if     defined(mips) || defined(alpha)
#define	BADMAG(X) (!(ISCOFF(X.f_magic)))
#endif	/* mips or alpha */
#if	defined(i386) || defined (PS2)
#define BADMAG(X) ((X.f_magic != I386MAGIC) && (X.f_magic != COFF386MAGIC))
#endif  /* i386 || PS2 */

ascii_bsd(f)
	int f;
{
	char buf[BUFSIZ];
	register int cnt;
	register int c;
	register char *cp;

	lseek(f, (long)0, 0);
	cnt = read(f, buf, BUFSIZ);
#if	COFF || MACHO
	if (cnt >= sizeof (struct filehdr)) {
		struct filehdr hdr;
		hdr = *(struct filehdr *)buf;
		if (!BADMAG(hdr))
#else	/* COFF || MACHO */
	if (cnt >= sizeof (struct exec)) {
		struct exec hdr;
		hdr = *(struct exec *)buf;
		if (!N_BADMAG(hdr))
#endif	/* COFF || MACHO */
			return (0);
	}
	cp = buf;
	while (cnt > 0)
	{
		c = mblen(cp, cnt);
		if (!(c & 0377))
			return(0);			/* GA001 */
		if ((c < 0) && (cnt >= MB_LEN_MAX))
			return(0) ;	/* Invalid multi-byte character */
		else if (c < 0)
			break ;		/* Not binary */
		else if (c == 0)
			cnt--, cp++ ;
		else
		{
			cnt -= c ;
			cp  += c ;
		}
	}
	return (1);
}

/*
 * NAME: useless
 * FUNCTION: checks to see if a file should be included in current list
 *  of files for the current directory.
 */
useless(cp)
struct dirent *cp;
{

	if (cp->d_name[0] == '.') {
		if (cp->d_name[1] == '\0')
			return (0);	/* directory "." */
		if (cp->d_name[1] == '.' && cp->d_name[2] == '\0')
			return (0);	/* directory ".." */
	}
	if (begin && strcmp(begin, cp->d_name) > 0)
		return (0);
	return (1);
}

/* 
 * NAME: slinkcmp
 * FUNCTION: trace symbolic links, determine if link to other directory.
 */
slinkcmp(file1, file2, dp)
char *file1, *file2;
struct dirent *dp;
{
	extern char *slinkcmpsbuf1, *slinkcmpsbuf2; /* Set outside recursion in diff.c */
	char *sbuf1, *sbuf2;
	int sfmt1, sfmt2;
	struct stat symb1, symb2;

	lstat(file1, &symb1); lstat(file2, &symb2);
	sfmt1 = symb1.st_mode & S_IFMT;
	sfmt2 = symb2.st_mode & S_IFMT;
	if (sfmt1 == sfmt2 && sfmt1 == S_IFLNK) {
		sbuf1 = slinkcmpsbuf1;
		sbuf2 = slinkcmpsbuf2;
		if (sfmt1 == S_IFLNK) readlink(file1, sbuf1, name_max+1);
		if (sfmt2 == S_IFLNK) readlink(file2, sbuf2, name_max+1);
		if( (strcmp(sbuf1,".") == 0 || strcmp(sbuf1,"..") == 0) &&
		    (strcmp(sbuf2,".") == 0 || strcmp(sbuf2,"..") == 0)) {
			printf(MSGSTR(SUBARE,"Common subdirectories are "));
			printf(MSGSTR(SYMLINK,"symbolic link files:\n\t"));
			if (strcmp(sbuf1,sbuf2) == 0) {
				if (lflag)
					dp->d_ino = SAME;
				else {
					printf(MSGSTR(IDENT,
					   "%s and %s are identical\n"),
					   file1, file2);
				}
			} else {
				if (lflag)
					dp->d_ino = DIFFER;
				else {
				   printf("%s --> %s ",
				     file1, sbuf1);
				   printf(MSGSTR(DDIFFER2,
				     " and %s --> %s differ\n"),file2, sbuf2);
				}
			}
			return(1);
		 }
	}
	return(0);
}
