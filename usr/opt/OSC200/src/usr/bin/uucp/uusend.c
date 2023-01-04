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
#if !defined(lint) && !defined(_NOIDENT)
static char rcsid[] = "@(#)$RCSfile: uusend.c,v $ $Revision: 4.3.8.4 $ (DEC) $Date: 1993/12/17 21:16:59 $";
#endif
/* 
 * COMPONENT_NAME: UUCP uusend.c
 * 
 * FUNCTIONS: MSGSTR, Muusend, getc, getfname, index, makedir 
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
uusend.c	1.6  com/cmd/uucp,3.1,9013 3/16/90 10:48:40";
*/

/*
 * uusend: primitive operation to allow uucp like copy of binary files
 * but handle indirection over systems.
 *
 * usage: uusend [-r] [-m ooo] localfile sysname1!sysname2!...!destfile
 *        uusend [-r] [-m ooo]     -     sysname1!sysname2!...!destfile
 *
 * Author: Mark Horton, May 1980.
 *
 * "-r" switch added.  Has same effect as "-r" in uux. 11/82  CCW
 *
 * Error recovery (a la uucp) added & ifdefs for ruusend (as in rmail).
 * Checks for illegal access to /usr/lbin/uucp.
 *				February 1983  Christopher Woodbury
 * Fixed mode set[ug]id loophole. 4/8/83  CCW
 *
 * Add '-f' to make uusend syntax more similar to UUCP.  "destname"
 * can now be a directory.	June 1983  CCW
 */

#include <stdio.h>
#include <pwd.h>
#include <sys/types.h>
#include <sys/stat.h>

#include	"uusend_msg.h"
#include	"pathnames.h"
#include	<locale.h>
nl_catd catd;
#define	MSGSTR(n,s)	catgets(catd,MS_UUSEND,n,s)


/*
 * define RECOVER to permit requests like 'uusend file sys1!sys2!~uucp'
 * (abbreviation for 'uusend file sys1!sys2!~uucp/file').
 * define DEBUG to keep log of uusend uusage.
 * define RUUSEND if neighboring sites permit 'ruusend',
 * which they certainly should to avoid security holes
 */
#define	RECOVER
/*#define	DEBUG	"/usr/spool/uucp/uusend.log"*/

FILE	*in, *out;
FILE	*dout;

extern FILE	*popen();
extern char	*index(), *strcpy(), *strcat(), *ctime();

#ifdef	RUUSEND
int	rsend;
#endif 
int	mode = -1;	/* mode to chmod new file to */
char	*nextsys;	/* next system in the chain */
char	dnbuf[200];	/* buffer for result of ~user/file */
char	cmdbuf[256];	/* buffer to build uux command in */
char	*rflg = "";	/* default value of rflg  ccw -- 1 Nov '82 */

struct	passwd *user;	/* entry  in /etc/passwd for ~user */
struct	passwd *getpwnam();
struct	stat	stbuf;

char	*excl;		/* location of first ! in destname */
char	*sl;		/* location of first / in destname */
char	*sourcename;	/* argv[1] */
char	*destname;	/* argv[2] */
char	*UULIB = "/usr/lbin/uucp";	  /* UUCP lib directory */

#ifdef	RECOVER
char	*UUPUB = "/usr/spool/uucppublic/";  /* public UUCP directory */
char	*filename;	/* file name from end of destname */
char	*getfname();	/* routine to get filename from destname */
int	fflg;
char	f[100];		/* name of default output file */
#else
char	*f	= "";	/* so we waste a little space */
#endif

main(argc, argv)
int	argc;
char	**argv;
{
	register int c;
	int count;
	extern char **environ;
	setlocale(LC_ALL,"");
	catd = catopen(MF_UUSEND, NL_CAT_LOCALE);

#ifdef DEBUG
	{
	time_t t;
	umask(022);
	dout = fopen(DEBUG, "a");
	if (dout == NULL) {
		fprintf(stdout, MSGSTR(NO_APPEND, "Cannot append to %s\n"),
			DEBUG);
		/* printf("Cannot append to %s\n", DEBUG); */
		exit(1);
	}
	freopen(DEBUG, "a", stdout);
	fprintf(dout, "\nuusend run: ");
	for (c=0; c<argc; c++)
		fprintf(dout, "%s ", argv[c]);
	time(&t);
	fprintf(dout, "%s", ctime(&t));
	}
#endif

#ifdef	RUUSEND
	if(argv[0][0] == 'r')
		rsend++;
#endif
	while (argc > 1 && argv[1][0] == '-' && argv[1][1]) {
		switch(argv[1][1]) {
		case 'm':
			if(argc > 2)		/*mik*/
			{
			sscanf(argv[2], "%o", &mode);
			mode &= 0777;  /* fix set[ug]id loophole */
			argc--; argv++;
			}
			break;
		case 'r':		/* -r flag for uux */
			rflg = "-r ";
			break;
#ifdef	RECOVER
		case 'f':
			fflg++;
			strcpy(f, argv[1]);
			break;
#endif
		default:
			fprintf(stderr, MSGSTR(BADFLAG, "Bad flag: %s\n"),
				argv[1]);
			/* fprintf(stderr, "Bad flag: %s\n", argv[1]); */
			break;
		}
		argc--; argv++;
	}

	if (argc != 3) {
		fprintf(stderr, MSGSTR(USAGE, "Usage: uusend [-m ooo] [-r] -/file sys!sys!..!rfile\n"));
		exit(1);
	}

	sourcename = argv[1];
	destname = argv[2];

	if (sourcename[0] == '-')
		in = stdin;
	else {
#ifdef	RUUSEND
		if (rsend) {
			fprintf(stderr, MSGSTR(BADINPUT, "illegal input\n"));
			/* fprintf(stderr, "illegal input\n"); */
			exit(2);
		}
#endif
		in = fopen(sourcename, "r");
		if (in == NULL) {
			perror(argv[1]);
			exit(2);
		}
		if (!fflg || f[2] == '\0') {
			strcpy(f, "-f");
			strcat(f, getfname(sourcename));
			fflg++;
		}
	}

	excl = index(destname, '!');
	if (excl) {
		/*
		 * destname is on a remote system.
		 */
		nextsys = destname;
		*excl++ = 0;
		destname = excl;
		if (mode < 0) {
			fstat(fileno(in), &stbuf);
			mode = stbuf.st_mode & 0777;
		}
#ifdef	RUUSEND
		sprintf(cmdbuf,"uux -gn -z %s- \"%s!ruusend %s -m %o - (%s)\"",
#else
		sprintf(cmdbuf, "uux -gn -z %s- \"%s!uusend %s -m %o - (%s)\"",
#endif
			rflg, nextsys, f, mode, destname);
#ifdef DEBUG
		fprintf(dout, "remote: nextsys='%s', destname='%s', cmd='%s'\n", nextsys, destname, cmdbuf);
#endif
		out = popen(cmdbuf, "w");
	} else {
		/*
		 * destname is local.
		 */
		if (destname[0] == '~') {
#ifdef DEBUG
			fprintf(dout, "before ~: '%s'\n", destname);
fflush(dout);
#endif
			sl = index(destname, '/');
#ifdef	RECOVER
			if (sl == NULL && !fflg) {
				fprintf(stderr, MSGSTR(BADUSER,
					"Illegal !user\n"));
				/* fprintf(stderr, "Illegal ~user\n"); */
				exit(3);
			}
			for (sl = destname; *sl != '\0'; sl++)
				;	/* boy, is this a hack! */
#else
			if (sl == NULL) {
				fprintf(stderr, MSGSTR(BADUSER,
					"Illegal !user\n"));
				/* fprintf(stderr, "Illegal ~user\n"); */
				exit(3);
			}
			*sl++ = 0;
#endif
			user = getpwnam(destname+1);
			if (user == NULL) {
				fprintf(stderr, MSGSTR(NOUSER,
					"No such user as %s\n"), destname);
				/* fprintf(stderr, "No such user as %s\n",
					destname); */
#ifdef	RECOVER
				if ((filename =getfname(sl)) == NULL &&
				     !fflg)
					exit(4);
				strcpy(dnbuf, UUPUB);
				if (fflg)
					strcat(dnbuf, &f[2]);
				else
					strcat(dnbuf, filename);
			}
			else {
				strcpy(dnbuf, user->pw_dir);
				strcat(dnbuf, "/");
				strcat(dnbuf, sl);
			}
#else
				exit(4);
			}
			strcpy(dnbuf, user->pw_dir);
			strcat(dnbuf, "/");
			strcat(dnbuf, sl);
#endif
			destname = dnbuf;
		}
#ifdef	RECOVER
		else
			destname = strcpy(dnbuf, destname);
#endif
		if(strncmp(UULIB, destname, strlen(UULIB)) == 0) {
			fprintf(stderr, MSGSTR(BADFILE, "illegal file: %s\n"),
				destname);
			/* fprintf(stderr, "illegal file: %s", destname); */
			exit(4);
		}
#ifdef	RECOVER
		if (stat(destname, &stbuf) == 0 &&
		    (stbuf.st_mode & S_IFMT) == S_IFDIR &&
		     fflg) {
			strcat(destname, "/");
			strcat(destname, &f[2]);
		}
#endif
		out = fopen(destname, "w");
#ifdef DEBUG
		fprintf(dout, "local, file='%s'\n", destname);
#endif
		if (out == NULL) {
			perror(destname);
#ifdef	RECOVER
			if (strncmp(destname,UUPUB,strlen(UUPUB)) == 0)
				exit(5);	/* forget it! */
			filename = getfname(destname);
			if (destname == dnbuf) /* cmdbuf is scratch */
				filename = strcpy(cmdbuf, filename);
			destname = strcpy(dnbuf, UUPUB);
			if (user != NULL) {
				strcat(destname, user->pw_name);
				if (stat(destname, &stbuf) == -1) {
					mkdir(destname, 0777);
				}
				strcat(destname, "/");
			}
			if (fflg)
				strcat(destname, &f[2]);
			else
				strcat(destname, filename);
			if ((out = fopen(destname, "w")) == NULL)
				exit(5); /* all for naught! */
#else
			exit(5);
#endif
		}
		if (mode > 0)
			chmod(destname, mode);	/* don't bother to check it */
	}

	/*
	 * Now, in any case, copy from in to out.
	 */

	count = 0;
	while ((c=getc(in)) != EOF) {
		putc(c, out);
		count++;
	}
#ifdef DEBUG
	fprintf(dout, "count %ld bytes\n", count);
	fclose(dout);
#endif

	fclose(in);
	fclose(out);	/* really should pclose in that case */
	exit(0);
}

/*
 * Return the ptr in sp at which the character c appears;
 * NULL if not found.  Included so I don't have to fight the
 * index/strchr battle.
 */

char *
index(sp, c)
register char *sp, c;
{
	do {
		if (*sp == c)
			return(sp);
	} while (*sp++);
	return(NULL);
}
void
cleanup(code){;}
/*
int     code;
{
        rmlock(CNULL);
        clrlock(CNULL);
        catclose(catd);

        exit(code);
}
*/

#ifdef	RECOVER
char *
getfname(p)
register char *p;
{
	register char *s;
	s = p;
	while (*p != '\0')
		p++;
	if (p == s)
		return (NULL);
	for (;p != s; p--)
		if (*p == '/') {
			p++;
			break;
		}
	return (p);
}

#ifndef BSD4_2
makedir(dirname, mode)
char *dirname;
int mode;
{
	register int pid;
	int retcode, status;
	switch ((pid = fork())) {
	    case -1:		/* error */
		return (-1);
	    case 0:		/* child */
		umask(0);
		execl(_PATH_MKDIR, "mkdir", dirname, (char *)0);
		exit(1);
		/* NOTREACHED */
	    default:		/* parent */
		while ((retcode=wait(&status)) != pid && retcode != -1)
			;
		if (retcode == -1)
			return  -1;
		else {
			chmod(dirname, mode);
			return status;
		}
	}
	/* NOTREACHED */
}
#endif /* !BSD4_2 */
#endif /* RECOVER */
