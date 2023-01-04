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
static char rcsid[] = "@(#)$RCSfile: expreserve.c,v $ $Revision: 4.2.7.3 $ (DEC) $Date: 1993/10/11 16:46:44 $";
#endif
/*
 * HISTORY
 */
/*
 * OSF/1 1.2
 */
/*
 * COMPONENT_NAME: (CMDEDIT) expreserve.c
 *
 * FUNCTION: main, copyout, mkdigits, mknext, notify
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * 1.18  com/cmd/edit/vi/expreserve.c, cmdedit, bos320, 9132320a 8/2/91 13:13:37
 * 
 */
/* Copyright (c) 1981 Regents of the University of California */
/* @(#)expreserve.c	5.1 23:20:07 8/15/90 SecureWare */

#include <sys/secdefines.h>

#include <stdio.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
/* #include <sys/id.h> */
#include <dirent.h>
#include <pwd.h>
#include <locale.h>

#include "ex_msg.h"
nl_catd	ex_catd;
#define MSGSTR(id,ds)      catgets(ex_catd, MS_EX, id, ds)
				/* mjm: "/tmp" --> TMP */
#define TMP     "/var/tmp"

#define	HBLKS	2
#define LBSIZE  (2048)
#define BUFFERSIZ (2048)

/*char xstr[1];			 make loader happy */

/*
 * Expreserve - preserve a file in /var/preserve
 * Bill Joy UCB November 13, 1977
 *
 * This routine is very naive - it doesn't remove anything from
 * /var/preserve... this may mean that we  * stuff there...
 * the danger in doing anything with /var/preserve
 * is that the clock may be screwed up and we may get confused.
 *
 * We are called in two ways - first from the editor with no argumentss
 * and the standard input open on the temp file. Second with an argument
 * to preserve the entire contents of /tmp (root only).
 *
 * BUG: should do something about preserving Rx... (register contents)
 *      temporaries.
 */

#define	LBLKS	900
#define	FNSIZE	1024

struct 	header {
	time_t	Time;			/* Time temp file last updated */
	uid_t	Uid;			/* This users identity */
	int	Flines;				/* Number of lines in file */
	char	Savedfile[FNSIZE];	/* The current file name */
	short	Blocks[LBLKS];		/* Blocks where line pointers stashed */
} H;
int copyout(char *);
void mkdigits(char *);
void mknext(char *);
void notify(uid_t, char *, int);
#include <fcntl.h>
#include <string.h>

#if SEC_MAC
FILE	*expreserve_popen(char *);
#endif

#define eq(a, b) strcmp(a, b) == 0


main(int argc, char *argv[])
{
	register DIR *tf;
	struct dirent *dir_entr;
	struct stat stbuf;
        char *tmpdir;

	(void)setlocale(LC_ALL,"");
	ex_catd = catopen(MF_EX,NL_CAT_LOCALE);

#if SEC_MAC
	set_auth_parameters(argc, argv);
	initprivs();
#endif

	/*
	 * If only one argument, then preserve the standard input.
	 */
	if (argc == 1) {
		if (copyout((char *) 0)) {
			catclose(ex_catd);
			exit(1);
		}
		catclose(ex_catd);
		exit(0);
	}

#if SEC_MAC
	if (!expreserve_check_auth()) {
		(void)fprintf(stderr, "expreserve: need sysadmin authorization\n");
		catclose(ex_catd);
		exit(1);
	}
	expreserve_full_copy(TMP, "Ex");
#else
	/*
	 * If not super user, then can only preserve standard input.
	 */
	if (getuid()) {
		(void)fprintf(stderr, MSGSTR(M_258, "NOT super user\n"));
		catclose(ex_catd);
		exit(1);
	}

        /*
         * ... else preserve all the stuff in /tmp (or the specified dir),
         * removing it as we go.
         */
        tmpdir = argv[1];
        if (eq(argv[1],"-a") || chdir(tmpdir) < 0) {
                tmpdir = TMP;
                if (chdir(tmpdir) < 0) {
                        perror(tmpdir);
			catclose(ex_catd);
                        exit(1);
                }
        }

	tf = opendir(".");
	if (tf == NULL) {
		perror(TMP);
		catclose(ex_catd);
		exit(1);
	}
	while ((dir_entr = readdir(tf)) != NULL) {
		if (dir_entr->d_ino == 0)
			continue;
		/*
		 * Ex temporaries must begin with Ex;
		 * we check that the 10th character of the name is null
		 * so we won't have to worry about non-null terminated names
		 * later on.
		 */
		if (dir_entr->d_name[0] != 'E' || dir_entr->d_name[1] != 'x')
			continue;
		if (stat(dir_entr->d_name, &stbuf))
			continue;
		if ((stbuf.st_mode & S_IFMT) != S_IFREG)
			continue;
		(void)copyout(dir_entr->d_name);
	}
	(void)closedir(tf);
#endif
	catclose(ex_catd);
	exit(0);
	return(0);	/* return put here to make lint happy */
}

char	pattern[] =	"/var/preserve/Exaa`XXXXX";

/*
 * Copy file name into /var/preserve/...
 * If name is (char *) 0, then do the standard input.
 * We make some checks on the input to make sure it is
 * really an editor temporary, generate a name for the
 * file (this is the slowest thing since we must stat
 * to find a unique name), and finally copy the file.
 */
int copyout(char *name)
{
	int i;
	static int reenter;
	char buf[BUFFERSIZ];

	/*
	 * The first time we put in the digits of our
	 * process number at the end of the pattern.
	 */
	if (reenter == 0) {
		mkdigits(pattern);
		reenter++;
	}

	/*
	 * If a file name was given, make it the standard
	 * input if possible.
	 */
	if (name != 0) {
		(void)close(0);
		/*
		 * Need read/write access for arcane reasons
		 * (see below).
		 */
		if (open(name, 2) < 0)
			return (-1);
	}

	/*
	 * Get the header block.
	 */
	(void)lseek(0, 0l, 0);
	if (read(0, (char *) &H, sizeof H) != sizeof H) {
format:
		if (name == 0)
                        (void)fprintf(stderr, MSGSTR(M_259, "Buffer format error\n"));
		return (-1);
	}

	/*
	 * Consistency checsks so we don't copy out garbage.
	 */
	if (H.Flines < 0) {
#ifdef DEBUG
		(void) fprintf(stderr, "Negative number of lines\n");
#endif
		goto format;
	}
	if (H.Blocks[0] != HBLKS || H.Blocks[1] != HBLKS+1) {
#ifdef DEBUG
		(void) fprintf(stderr, "Blocks %d %d\n", H.Blocks[0], H.Blocks[1]);
#endif
		goto format;
	}
	if (name == 0 && H.Uid != getuid()) {
#ifdef DEBUG
		(void) fprintf(stderr, "Wrong user-id\n");
#endif
		goto format;
	}
	if (lseek(0, 0l, 0)) {
#ifdef DEBUG
		(void) fprintf(stderr, "Negative number of lines\n");
#endif
		goto format;
	}

	/*
	 * If no name was assigned to the file, then give it the name
	 * LOST, by putting this in the header.
	 */
	if (H.Savedfile[0] == 0) {
		(void) strcpy(H.Savedfile, "LOST");
		(void) write(0, (char *) &H, sizeof H);
		H.Savedfile[0] = 0;
		(void) lseek(0, 0l, 0);
	}

	/*
	 * File is good.  Get a name and create a file for the copy.
	 */
	mknext(pattern);
	(void) close(1);
#if SEC_MAC
	if (expreserve_create_file(name, pattern, H.Uid) < 0)
#else
	if (creat(pattern, 0600) < 0)
#endif
	{
		if (name == 0)
			perror(pattern);
		return (1);
	}

#if !SEC_MAC
	/*
	 * Make the target be owned by the owner of the file.
	 */
	(void) chown(pattern, H.Uid, 0);
#endif

	/*
	 * Copy the file.
	 */
	for (;;) {
		i = read(0, buf, BUFFERSIZ);
		if (i < 0) {
			if (name)
				perror(MSGSTR(M_260, "Buffer read error"));
#if SEC_MAC
			expreserve_unlink(pattern);
#else
			(void) unlink(pattern);
#endif
			return (-1);
		}
		if (i == 0) {
			if (name)
				unlink(name);
			notify(H.Uid, H.Savedfile, (int) name);
			return (0);
		}
		if (write(1, buf, i) != i) {
			if (name == 0)
				perror(pattern);
#if SEC_MAC
			expreserve_unlink(pattern);
#else
			(void) unlink(pattern);
#endif
			return (-1);
		}
	}
}

/*
 * Blast the last 5 characters of cp to be the process number.
 */
void mkdigits(char *cp)
{
	register int i, j;

	for (i = getpid(), j = 5, cp += strlen(cp); j > 0; i /= 10, j--)
		*--cp = i % 10 | '0';
}

/*
 * Make the name in cp be unique by clobbering up to
 * three alphabetic characters into a sequence of the form 'aab', 'aac', etc.
 * Mktemp gets weird names too quickly to be useful here.
 */
void mknext(char *cp)
{
	char *dcp;
	struct stat stb;

	dcp = cp + strlen(cp) - 1;
	while (isdigit(*dcp))
		dcp--;
whoops:
	if (dcp[0] == 'z') {
		dcp[0] = 'a';
		if (dcp[-1] == 'z') {
			dcp[-1] = 'a';
			if (dcp[-2] == 'z')
				(void)fprintf(stderr, MSGSTR(M_261, "Can't find a name\t"));
			dcp[-2]++;
		} else
			dcp[-1]++;
	} else
		dcp[0]++;
	if (stat(cp, &stb) == 0)
		goto whoops;
}

/*
 * Notify user uid that his file fname has been saved.
 */
void notify(uid_t uid, char *fname, int flag)
{
	struct passwd *pp = getpwuid(uid);
	register FILE *mf;
	char cmd[LBSIZE];

	if (pp == NULL)
		return;
	(void)sprintf(cmd, "mail %s", pp->pw_name);
	(void)setuid(getuid());
#if SEC_MAC
	mf = expreserve_popen(cmd);
#else
	mf = popen(cmd, "w");
#endif
	if (mf == NULL)
		return;
	setbuffer(mf, cmd, sizeof(cmd));
	if (fname[0] == 0) {
		(void)fprintf(mf, flag ?
MSGSTR(M_262, "A copy of an editor buffer of yours was saved when the system went down.\n") :
MSGSTR(M_263, "A copy of an editor buffer of yours was saved when the editor was killed.\n"));
		(void)fprintf(mf,
MSGSTR(M_264, "No name was associated with this buffer so it has been named \"LOST\".\n"));
	} else
		/*
		 * "the editor was killed" is perhaps still not an ideal
		 * error message.  Usually, either it was forcably terminated
		 * or the phone was hung up, but we don't know which.
		 */
		(void) fprintf(mf, flag ?
MSGSTR(M_265, "A copy of an editor buffer of your file \"%s\"\nwas saved when the system went down.\n") :
MSGSTR(M_266, "A copy of an editor buffer of your file \"%s\"\nwas saved when the editor was killed.\n"), fname);
	(void) fprintf(mf,
MSGSTR(M_267, "This buffer can be retrieved using the \"recover\" command of the editor.\n\
An easy way to do this is to give the command \"ex -r %s\".\n\
This works for \"edit\" and \"vi\" also.\n"), fname);
#if SEC_MAC
	expreserve_pclose(mf);
#else
	(void) pclose(mf);
#endif
}
