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
static char	*sccsid = "@(#)$RCSfile: lpr.c,v $ $Revision: 4.2.7.3 $ (DEC) $Date: 1993/10/08 15:14:34 $";
#endif 
/*
 * (c) Copyright 1990, 1991, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0.1
 */
/*
 * Copyright (c) 1983 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice and this paragraph are
 * duplicated in all such forms and that any documentation,
 * advertising materials, and other materials related to such
 * distribution and use acknowledge that the software was developed
 * by the University of California, Berkeley.  The name of the
 * University may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 * 
 * lpr.c	5.4 (Berkeley) 6/30/88
 * lpr.c	4.2 13:32:20 7/20/90 SecureWare 
 */


/*
 *      lpr -- off line print
 *
 * Allows multiple printers and printers on remote machines by
 * using information from a printer data base.
 */

#include <sys/secdefines.h>
#if SEC_BASE
#include <sys/security.h>
#endif

#include <stdio.h>
#include <sys/types.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <pwd.h>
#include <grp.h>
#include <signal.h>
#include <ctype.h>
#include <syslog.h>
#include "lp.local.h"
#include <locale.h>
#include "printer_msg.h"

nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_PRINTER,n,s)

#if SEC_BASE
#define MSGSTR_SEC(n,s) catgets(catd,MS_PRINTER_SEC,n,s)
#endif SEC_BASE

#if SEC_MAC
#include <mandatory.h>
#endif

char    *tfname;		/* tmp copy of cf before linking */
char    *cfname;		/* daemon control files, linked from tf's */
char    *dfname;		/* data files */

int	nact;			/* number of jobs to act on */
int	tfd;			/* control file descriptor */
int     mailflg;		/* send mail */
int	qflag;			/* q job, but don't exec daemon */
char	format = 'f';		/* format char for printing files */
int	rflag;			/* remove files upon completion */	
int	sflag;			/* symbolic link flag */
int	inchar;			/* location to increment char in file names */
int     ncopies = 1;		/* # of copies to make */
int	iflag;			/* indentation wanted */
int	indent;			/* amount to indent */
int	hdr = 1;		/* print header or not (default is yes) */
int     userid;			/* user id */
int     jobnumber,jobFlag=0;	/* Job Number for stdout */
char    *numberup = NULL;       /* # for number_up */
char	*person;		/* user name */
char	*title;			/* pr'ing title */
char	*fonts[4];		/* troff font names */
char	*width;			/* width for printing */
char	host[32];		/* host name */
char	*class = host;		/* class title on header page */
char    *jobname;		/* job name on header page */
char	*name;			/* program name */
char	*printer;		/* printer name */
char	*orient = NULL;		/* page orientation (for page printers) */
char	*out_tray = NULL;		/* output tray selection (for page printers) */
char	*in_tray=NULL;		/* input tray selection (for page printers) */
char	*sides=NULL;			/* number of sides/tumble (for page printers) */

struct	stat statb;

int	MX;			/* maximum number of blocks to copy */
int	MC;			/* maximum number of copies allowed */
int	DU;			/* daemon user-id */
char	*SD;			/* spool directory */
char	*LO;			/* lock file name */
char	*RG;			/* restrict group */
short	SC;			/* suppress multiple copies */

#if SEC_MAC
int	label_override = 0;
#endif
#if SEC_BASE
uid_t	lp_uid;
gid_t	lp_gid;
#endif

char	*getenv();
char	*rindex();
char	*linked();
void	cleanup();

#if SEC_BASE
extern priv_t	*privvec();
#endif

/*ARGSUSED*/
main(argc, argv)
	int argc;
	char *argv[];
{
	struct passwd *pw;
	struct group *gptr;
	extern char *itoa();
	register char *arg, *cp;
	char buf[BUFSIZ];
	int i, f, index;
	struct stat stb;
	char *sides_op[][2]={ /* ULTRIX needs the params as per 2d column for inter-operability */
		{"1",			"one"},
		{"one",			"one"},
		{"one_sided",		"one"},
		{"one_sided_duplex",	"one_sided_duplex"},
		{"one_sided_simplex",	"one"},
		{"one_sided_tumble",	"one_sided_tumble"},
		{"tumble",		"tumble"},
		{"2",			"two"},
		{"two",			"two"},
		{"two_sided",		"two"},
		{"two_sided_simplex",	"two_sided_simplex"},
		{"two_sided_duplex",	"two"},
		{"two_sided_tumble",	"tumble"},
		{NULL,			NULL}
	};

        (void) setlocale( LC_ALL, "" );
        catd = catopen(MF_PRINTER,NL_CAT_LOCALE);

#if SEC_BASE
	set_auth_parameters(argc, argv);
	initprivs();
	lp_uid = pw_nametoid("lp");
	if (lp_uid == (uid_t) -1) {
		fprintf(stderr, MSGSTR_SEC(USERUNDEF, "%s: user \"lp\" is not defined\n"), "lpr");
		exit(1);
	}
	lp_gid = gr_nametoid("lp");
	if (lp_gid == (gid_t) -1) {
		fprintf(stderr, MSGSTR_SEC(GROUPUNDEF, "%s: group \"lp\" is not defined\n"), "lpr");
		exit(1);
	}
#if SEC_MAC
	if (mand_init()) {
		fprintf(stderr, MSGSTR_SEC(INITLBL, "%s: cannot initialize for sensitivity labels\n"), "lpr");
		exit(1);
	}
#endif
#if SEC_ILB
	if (forceprivs(privvec(SEC_ALLOWILBACCESS, SEC_ILNOFLOAT, -1),
			(priv_t *) 0)) {
		fprintf(stderr, MSGSTR_SEC(PRIVS, "%s: insufficient privileges\n"), "lpr");
		exit(1);
	}
	setilabel(mand_syslo);
#endif
#endif /* SEC_BASE */

	if (signal(SIGHUP, SIG_IGN) != SIG_IGN)
		signal(SIGHUP, cleanup);
	if (signal(SIGINT, SIG_IGN) != SIG_IGN)
		signal(SIGINT, cleanup);
	if (signal(SIGQUIT, SIG_IGN) != SIG_IGN)
		signal(SIGQUIT, cleanup);
	if (signal(SIGTERM, SIG_IGN) != SIG_IGN)
		signal(SIGTERM, cleanup);

	name = argv[0];
	gethostname(host, sizeof (host));
	openlog("lpd", 0, LOG_LPR);

	while (argc > 1 && argv[1][0] == '-') 
	{
		argc--;
		arg = *++argv;
		switch (arg[1]) 
		{

		case 'P':		/* specifiy printer name */
			if (arg[2])
				printer = &arg[2];
			else if (argc > 1) {
				argc--;
				printer = *++argv;
			}
			break;

		case 'C':		/* classification spec */
			hdr++;
			if (arg[2])
				class = &arg[2];
			else if (argc > 1) {
				argc--;
				class = *++argv;
			}
			break;

		case 'J':		/* job name */
			hdr++;
			if (arg[2])
				jobname = &arg[2];
			else if (argc > 1) {
				argc--;
				jobname = *++argv;
			}
			break;

		case 'T':		/* pr's title line */
			if (arg[2])
				title = &arg[2];
			else if (argc > 1) {
				argc--;
				title = *++argv;
			}
			break;

		case 'l':		/* literal output */
#if SEC_MAC
			if (!authorized_user("filter")) {
			    fprintf(stderr, MSGSTR_SEC(FILTERAUTH,
				"%s: -l option requires filter authorization\n"),
				"lpr");
			    exit(1);
			}
#endif
		case 'p':		/* print using ``pr'' */
		case 't':		/* print troff output (cat files) */
		case 'n':		/* print ditroff output */
		case 'd':		/* print tex output (dvi files) */
		case 'g':		/* print graph(1G) output */
		case 'c':		/* print cifplot output */
		case 'v':		/* print vplot output */
			format = arg[1];
			break;

		case 'f':		/* print fortran output */
			format = 'r';
			break;

		case 'x':		/* untranslated literal output */
#if SEC_MAC
			if (!authorized_user("filter")) {
			    fprintf(stderr, MSGSTR_SEC(FILTERAUTH,
				"%s: -x option requires filter authorization\n"),
				"lpr");
			    exit(1);
			}
#endif
			format = 'x';
			break;

	        case 'j':	        /* print the jobnumber */
			jobFlag=1;
			break;
		case '4':		/* troff fonts */
		case '3':
		case '2':
		case '1':
			if (argc > 1) {
				argc--;
				fonts[arg[1] - '1'] = *++argv;
			}
			break;

		case 'w':		/* versatec page width */
			width = arg+2;
			break;

		case 'r':		/* remove file when done */
			rflag++;
			break;

		case 'm':		/* send mail when done */
			mailflg++;
			break;

		case 'h':		/* toggle want of header page */
#if SEC_MAC
			/* Header page always required on systems with MAC */
#else
			hdr = !hdr;
#endif
			break;

		case 's':		/* try to link files */
			sflag++;
			break;

		case 'q':		/* just q job */
			qflag++;
			break;

		case 'i':		/* indent output */
			iflag++;
			indent = arg[2] ? atoi(&arg[2]) : 8;
			break;

		case '#':		/* n copies */
			if (isdigit(arg[2])) {
				i = atoi(&arg[2]);
				if (i > 0)
					ncopies = i;
			}
                        break;
                case '\0':		/* use stdin at this position */
                        break;		/* handled further down */
#if SEC_MAC
		case 'V':
			if (!authorized_user("label")) 
			{
			    fprintf(stderr, MSGSTR_SEC(LBLAUTH,
				"%s: -V option requires label authorization\n"),
				"lpr");
			    exit(1);
			}
			label_override = 1;
			break;
#endif
		case 'O':		/* orientation of output */
			if (arg[2])
				orient = &arg[2];
			else if (argc > 1) {
				argc--;
				orient = *++argv;
			}
			break;

		case 'o':		/* output tray selection */
			if (arg[2])
				out_tray = &arg[2];
			else if (argc > 1) {
				argc--;
				out_tray = *++argv;
			}
			break;

		case 'I':     		/* input tray selection */
			if (arg[2])
				in_tray = &arg[2];
			else if (argc > 1) {
				argc--;
				in_tray = *++argv;
			}
			break;

		case 'N':              /* number up  001-gray */
                        if (arg[2])
                                numberup = &arg[2];
                        else if (argc > 1) {
                                argc--;
                                numberup = *++argv;
                        }

			if (numberup)
				/*
				 * check the argument for validity
				 */
				if (isdigit(numberup[0])) {
					i = atoi(numberup);
					if (i >= 0 && i <= 100)
						break;
				}

			/*
			 * an incorrect or missing argument was
			 * specified for N
			 */
			fprintf(stderr, MSGSTR(LPR_35,
				"Invalid argument for N - must be number in range 0-100\n"));
			exit(1);

		case 'K':		/* number of sides and tumble */
			if (arg[2])
				sides = &arg[2];
			else if (argc > 1) {
				argc--;
				sides = *++argv;
			}

			if (sides) {
				/*
				 * convert the argument for inter-operability
                                 * with ULTRIX by searching the "sides_op"
				 * array for a match, substitute the parameter
				 * from the array or NULL if not found
				 */
				index = -1;
				while((sides_op[++index][0]) &&
				      (strcmp(sides,sides_op[index][0])));
				sides = sides_op[index][1];
			}

			if (!sides) {
				/*
				 * an incorrect or missing argument was
				 * specified for K
				 */
				fprintf(stderr, MSGSTR(LPR_34,
					"Invalid argument for K\n"));
				exit(1);
			}
			break;

		default:
			fprintf(stderr, MSGSTR(LPR_33, 
			      "Unknown option -%c\n"), arg[1]);
			exit(1);
		}
	    }
	if (printer == NULL && 
          ((printer = getenv("PRINTER")) == NULL || strlen(printer) == 0))
		printer = DEFLP;
	chkprinter(printer);
	if (SC && ncopies > 1)
		fatal(MSGSTR(LPR_1, "multiple copies are not allowed"));
	if (MC > 0 && ncopies > MC)
		fatal(MSGSTR(LPR_2, "only %d copies are allowed"), MC);
	/*
	 * Get the identity of the person doing the lpr using the same
	 * algorithm as lprm. 
	 */
#if SEC_BASE
	userid = getluid();
#else
	userid = getuid();
#endif
	if ((pw = getpwuid(userid)) == NULL)
		fatal(MSGSTR(LPR_3, "Who are you?"));
	person = pw->pw_name;

	/*
	 * Check for restricted group access.
	 */
	if (RG != NULL) {
		if ((gptr = getgrnam(RG)) == NULL)
			fatal(MSGSTR(LPR_5, "Restricted group specified incorrectly"));
		if (gptr->gr_gid != getgid()) {
			while (*gptr->gr_mem != NULL) {
				if ((strcmp(person, *gptr->gr_mem)) == 0)
					break;
				gptr->gr_mem++;
			}
			if (*gptr->gr_mem == NULL)
				fatal(MSGSTR(LPR_6, "Not a member of the restricted group"));
		}
	}
	/*
	 * Check to make sure queuing is enabled if userid is not root.
	 */
	(void) sprintf(buf, "%s/%s", SD, LO);
#if SEC_BASE
	if (!haslpauth() && stat(buf, &stb) == 0 && (stb.st_mode & 010))
#else
	if (userid && stat(buf, &stb) == 0 && (stb.st_mode & 010))
#endif
		fatal(MSGSTR(LPR_7, "Printer queue is disabled"));
	/*
	 * Initialize the control file.
	 */
	mktemps();
	tfd = nfile(tfname);
#if SEC_BASE
	/* We already set the attributes we want in nfile. */
#else
	(void) fchown(tfd, DU, -1);	/* owned by daemon for protection */
#endif
	card('H', host);
	card('P', person);
	if (hdr) {
		if (jobname == NULL) {
			if (argc == 1)
				jobname = "stdin";
			else
				jobname = (arg = rindex(argv[1], '/')) ? arg+1 : argv[1];
		}
		card('J', jobname);
		card('C', class);
		card('L', person);
	}
	if (iflag)
		card('I', itoa(indent));
	if (mailflg)
		card('M', person);
#if SEC_MAC
	if (label_override)
		card('V', "");
#endif
	if (format == 't' || format == 'n' || format == 'd')
		for (i = 0; i < 4; i++)
			if (fonts[i] != NULL)
				card('1'+i, fonts[i]);
	if (width != NULL)
		card('W', width);

	if (in_tray != NULL)
		card('<', in_tray);

	if (out_tray != NULL)
		card('>', out_tray);

	if (orient != NULL)
		card('O', orient);

	if (numberup)			/* 001-gray */
		card('G', numberup);

	if (sides != NULL)
	        card('K', sides);


	/*
	 * Read the files and spool them.
	 */
	if (argc == 1)
		copy(0, " ");
	else while (--argc) {
                arg = *++argv;
                if (arg[0] == '-' && arg[1] == '\0')
                {			/* use stdin at this position */
                   copy(0, " ");
                   continue;
                }
		if ((f = test(arg)) < 0)
			continue;	/* file unreasonable */

		if (sflag && (cp = linked(arg)) != NULL) {
			(void) sprintf(buf, "%d %d", statb.st_dev, statb.st_ino);
			card('S', buf);
			if (format == 'p')
				card('T', title ? title : arg);
			for (i = 0; i < ncopies; i++)
				card(format, &dfname[inchar-2]);
			card('U', &dfname[inchar-2]);
			if (f)
				card('U', cp);
			card('N', arg);
			dfname[inchar]++;
			nact++;
			continue;
		}
		if (sflag)
			printf(MSGSTR(LPR_8, "%s: %s: not linked, copying instead\n"), name, arg);
		if ((i = open(arg, O_RDONLY)) < 0) {
			fprintf(stderr, MSGSTR(LPR_9, "%s: cannot open %s\n"), name, arg);
			continue;
		}
		copy(i, arg);
		(void) close(i);
		if (f && unlink(arg) < 0)
			fprintf(stderr, MSGSTR(LPR_10, "%s: %s: not removed\n"), name, arg);
	}

	if (nact) {
		(void) close(tfd);
		tfname[inchar]--;
		/*
		 * Touch the control file to fix position in the queue.
		 */
		if ((tfd = open(tfname, O_RDWR)) >= 0) {
			char c;

			if (read(tfd, &c, 1) == 1 && lseek(tfd, 0L, 0) == 0 &&
			    write(tfd, &c, 1) != 1) {
				fprintf(stderr, MSGSTR(LPR_11, "%s: cannot touch %s\n"), name, tfname);
				tfname[inchar]++;
				cleanup();
			}
			(void) close(tfd);
		}
#if SEC_MAC || SEC_NCAV
		forceprivs(privvec(
#if SEC_MAC
					SEC_ALLOWMACACCESS,
#endif
#if SEC_NCAV
					SEC_ALLOWNCAVACCESS,
#endif
					-1), (priv_t *) 0);
#endif
		if (link(tfname, cfname) < 0) {
			fprintf(stderr, MSGSTR(LPR_12, "%s: cannot rename %s\n"), name, cfname);
			tfname[inchar]++;
			cleanup();
		}
		unlink(tfname);
		if (jobFlag)
		    printf(MSGSTR(LPR_13, "Job Number is: %d\n"), jobnumber);
		if (qflag)		/* just q things up */
			exit(0);
#if SEC_BASE
		forcepriv(SEC_REMOTE);
#endif
		if (!startdaemon(printer))
			fprintf(stderr, MSGSTR(LPR_14, "jobs queued, but cannot start daemon.\n"));
		exit(0);
	}
	cleanup();
	/* NOTREACHED */
}

/*
 * Create the file n and copy from file descriptor f.
 */
copy(f, n)
	int f;
	char n[];
{
	register int fd, i, nr, nc;
	char buf[BUFSIZ];
#if SEC_MAC
	extern char	*strdup();
	char	*save_dfname = strdup(dfname);
#endif

	if (format == 'p')
		card('T', title ? title : n);
	for (i = 0; i < ncopies; i++)
		card(format, &dfname[inchar-2]);
	card('U', &dfname[inchar-2]);
	card('N', n);
	fd = nfile(dfname);
	nr = nc = 0;
	while ((i = read(f, buf, BUFSIZ)) > 0) {
		if (write(fd, buf, i) != i) {
			fprintf(stderr, MSGSTR(LPR_15, "%s: %s: temp file write error\n"), name, n);
			break;
		}
		nc += i;
		if (nc >= BUFSIZ) {
			nc -= BUFSIZ;
			nr++;
			if (MX > 0 && nr > MX) {
				fprintf(stderr, MSGSTR(LPR_16, "%s: %s: copy file is too large\n"), name, n);
				break;
			}
		}
	}
        if (i < 0) {
             perror(MSGSTR(E_READ, "lpr: read error"));
             exit(2);
         }
	(void) close(fd);
#if SEC_MAC
	/* Set MAC label of queued copy to match original file */
	lpr_save_label(f, save_dfname);
	free(save_dfname);
#endif
	if (nc==0 && nr==0) 
		fprintf(stderr, MSGSTR(LPR_17, "%s: %s: empty input file\n"), name, f ? n : "stdin");
	else
		nact++;
}

/*
 * Try and link the file to dfname. Return a pointer to the full
 * path name if successful.
 */
char *
linked(file)
	register char *file;
{
#if SEC_BASE
	/*
	 * Always spool a copy of the file on a secure system
	 */
	return NULL;
#else /* !SEC_BASE */
	register char *cp;
	static char buf[BUFSIZ];

	if (*file != '/') {
		if (getwd(buf) == NULL)
			return(NULL);
		while (file[0] == '.') {
			switch (file[1]) {
			case '/':
				file += 2;
				continue;
			case '.':
				if (file[2] == '/') {
					if ((cp = rindex(buf, '/')) != NULL)
						*cp = '\0';
					file += 3;
					continue;
				}
			}
			break;
		}
		strcat(buf, "/");
		strcat(buf, file);
		file = buf;
	}
	return(symlink(file, dfname) ? NULL : file);
#endif /* !SEC_BASE */
}

/*
 * Put a line into the control file.
 */
card(c, p2)
	register char c, *p2;
{
	char buf[BUFSIZ];
	register char *p1 = buf;
	register int len = 2;

	*p1++ = c;
	while ((c = *p2++) != '\0') {
		*p1++ = c;
		len++;
	}
	*p1++ = '\n';
	write(tfd, buf, len);
}

/*
 * Create a new file in the spool directory.
 */
nfile(n)
	char *n;
{
	register f;
	int oldumask = umask(0);		/* should block signals */
#if SEC_BASE
	privvec_t saveprivs;

	if (forceprivs(privvec(SEC_CHOWN,
#if SEC_MAC
				SEC_ALLOWMACACCESS,
#endif
#if SEC_ILB
				SEC_ILNOFLOAT,
#endif
#if SEC_NCAV
				SEC_ALLOWNCAVACCESS,
#endif
				-1), saveprivs)) {
		fprintf(stderr, MSGSTR_SEC(PRIVS, "%s: insufficient privileges\n"), "lpr");
		cleanup();
	}
#endif

	f = creat(n, FILMOD);
	(void) umask(oldumask);
	if (f < 0) {
		fprintf(stderr, MSGSTR(LPR_18, "%s: cannot create %s\n"), name, n);
		cleanup();
	}
#if SEC_BASE
	if (fchown(f, lp_uid, lp_gid) < 0)
#else
	if (fchown(f, userid, -1) < 0)
#endif
	{
		fprintf(stderr, MSGSTR(LPR_19, "%s: cannot chown %s\n"), name, n);
		cleanup();
	}
#if SEC_BASE
	seteffprivs(saveprivs, (priv_t *) 0);
#endif
	if (++n[inchar] > 'z') {
		if (++n[inchar-2] == 't') {
			fprintf(stderr, MSGSTR(LPR_20, "too many files - break up the job\n"));
			cleanup();
		}
		n[inchar] = 'A';
	} else if (n[inchar] == '[')
		n[inchar] = 'a';
	return(f);
}

/*
 * Cleanup after interrupts and errors.
 */
void cleanup()
{
	register i;

	signal(SIGHUP, SIG_IGN);
	signal(SIGINT, SIG_IGN);
	signal(SIGQUIT, SIG_IGN);
	signal(SIGTERM, SIG_IGN);
#if SEC_BASE
	forceprivs(privvec(SEC_ALLOWDACACCESS,
#if SEC_MAC
				SEC_ALLOWMACACCESS,
#endif
#if SEC_NCAV
				SEC_ALLOWNCAVACCESS,
#endif
				-1), (priv_t *) 0);
#endif
	i = inchar;
	if (tfname)
		do
			unlink(tfname);
		while (tfname[i]-- != 'A');
	if (cfname)
		do
			unlink(cfname);
		while (cfname[i]-- != 'A');
	if (dfname)
		do {
			do
				unlink(dfname);
			while (dfname[i]-- != 'A');
			dfname[i] = 'z';
		} while (dfname[i-2]-- != 'd');
	exit(1);
}

#ifdef  MACHO
#ifdef  multimax
#define COFF    1
#endif  /* multimax */
#ifdef  mips
#define COFF    1
#endif  /* mips */
#ifdef  i386
#ifdef  PS2
#define COFF    1
#endif  /* PS2 */
#endif  /* i386 */
#endif  /* MACHO */

/*
 * Test to see if this is a printable file.
 * Return -1 if it is not, 0 if its printable, and 1 if
 * we should remove it after printing.
 */
test(file)
	char *file;
{
#if	COFF
	struct filehdr execb;
#else
	struct exec execb;
#endif
	register int fd, file_uid;
	register char *cp, *dir;

	if (access(file, 4) < 0) {
		fprintf(stderr, MSGSTR(LPR_21, "%s: cannot access %s\n"), name, file);
		return(-1);
	}
	if (stat(file, &statb) < 0) {
		fprintf(stderr, MSGSTR(LPR_22, "%s: cannot stat %s\n"), name, file);
		return(-1);
	}
	if ((statb.st_mode & S_IFMT) == S_IFDIR) {
		fprintf(stderr, MSGSTR(LPR_23, "%s: %s is a directory\n"), name, file);
		return(-1);
	}
	if (statb.st_size == 0) {
		fprintf(stderr, MSGSTR(LPR_24, "%s: %s is an empty file\n"), name, file);
		return(-1);
 	}
	if ((fd = open(file, O_RDONLY)) < 0) {
		fprintf(stderr, MSGSTR(LPR_9, "%s: cannot open %s\n"), name, file);
		return(-1);
	}
	if (read(fd, &execb, sizeof(execb)) == sizeof(execb))
#ifdef	multimax
#define	BADMAG(X) (X.f_magic != NS32GMAGIC && X.f_magic != NS32SMAGIC)
#endif	multimax
#if defined(mips) || defined(__alpha)
#define	BADMAG(X) (!(ISCOFF(X.f_magic)))
#endif	mips
#ifndef	BADMAG
#define	BADMAG	N_BADMAG
#endif	BADMAG
	{
		if (!BADMAG(execb)) {
			fprintf(stderr, MSGSTR(LPR_25, "%s: %s is an executable program"), name, file);
			goto error1;
		}
		if (strncmp((char *) &execb, ARMAG, SARMAG) == 0) {
			fprintf(stderr, MSGSTR(LPR_26, "%s: %s is an archive file"), name, file);
			goto error1;
		}
	}
	(void) close(fd);

	if (rflag) {
		/*
		 * find the parent directory for the file
		 */
		if ((cp = rindex(file, '/')) == NULL) {
			dir = strdup(".");		/* default directory */
		}
		else {
			*cp = '\0';
			if (strlen(file) == 0)
				dir = strdup("/");	/* root directory */
			else
				dir = strdup(file);	/* specific directory */
			*cp = '/';
		}

		if (access(dir, 2) == 0) {
			/*
			 * we have write access to parent directory
                         */
			file_uid = statb.st_uid;

			if (stat(dir, &statb) < 0) {
				fprintf(stderr, MSGSTR(LPR_22, "%s: cannot stat %s\n"), name, dir);
				free(dir);
				return(0);
			}

			if (statb.st_mode & S_ISVTX) {
				/*
				 * the "sticky" bit is set on the directory
				 * in order to delete the file we must either:
				 *   - be superuser
				 *   - own the file
				 *   - own the directory
				 */
				if ((userid == 0) ||
				    (userid == file_uid) ||
				    (userid == statb.st_uid)) {
					free(dir);
					return(1);
				}
			}
			else {
				free(dir);
				return(1);
			}
		}

		fprintf(stderr, MSGSTR(LPR_27, "%s: %s: is not removable by you\n"), name, file);
	}
	return(0);

error1:
	fprintf(stderr, MSGSTR(LPR_28, " and is unprintable\n"));
	(void) close(fd);
	return(-1);
}

/*
 * itoa - integer to string conversion
 */
char *
itoa(i)
	register int i;
{
	static char b[10] = "########";
	register char *p;

	p = &b[8];
	do
		*p-- = i%10 + '0';
	while (i /= 10);
	return(++p);
}

/*
 * Perform lookup for printer name or abbreviation --
 */
chkprinter(s)
	char *s;
{
	int status;
	char buf[BUFSIZ];
	static char pbuf[BUFSIZ/2];
	char *bp = pbuf;
	extern char *pgetstr();

	if ((status = pgetent(buf, s)) < 0)
		fatal(MSGSTR(LPR_29, "cannot open printer description file"));
	else if (status == 0)
		fatal(MSGSTR(LPR_30, "%s: unknown printer"), s);
	if ((SD = pgetstr("sd", &bp)) == NULL)
		SD = DEFSPOOL;
	if ((LO = pgetstr("lo", &bp)) == NULL)
		LO = DEFLOCK;
	RG = pgetstr("rg", &bp);
	if ((MX = pgetnum("mx")) < 0)
		MX = DEFMX;
	if ((MC = pgetnum("mc")) < 0)
		MC = DEFMAXCOPIES;
	if ((DU = pgetnum("du")) < 0)
		DU = DEFUID;
	SC = pgetflag("sc");
}

/*
 * Make the temp files.
 */
mktemps()
{
	register int c, len, fd, n;
	register char *cp;
	char buf[BUFSIZ];
	char *mktemp();
#if SEC_BASE
	privvec_t saveprivs;
#endif

	(void) sprintf(buf, "%s/.seq", SD);
#if SEC_BASE
	if (forceprivs(privvec(SEC_CHOWN, SEC_ALLOWDACACCESS,
#if SEC_MAC
				SEC_ALLOWMACACCESS,
#endif
#if SEC_ILB
				SEC_ILNOFLOAT,
#endif
#if SEC_NCAV
				SEC_ALLOWNCAVACCESS,
#endif
				-1), saveprivs)) {
		fprintf(stderr, MSGSTR_SEC(PRIVS, "%s: insufficient privileges\n"), "lpr");
		exit(1);
	}
	/*
	 * If the sequence number file doesn't exist, create it
	 * and set its attributes.
	 */
	if ((fd = open(buf, O_RDWR|O_CREAT|O_EXCL, 0660)) >= 0) {
		if (fchown(fd, lp_uid, lp_gid) < 0) {
			fprintf(stderr, MSGSTR_SEC(CHOWNGRP, "%s: cannot set owner/group of %s\n"), "lpr", buf);
			unlink(buf);
			exit(1);
		}
		close(fd);
#if SEC_MAC
		if (chslabel(buf, mand_syslo) < 0) {
			fprintf(stderr, MSGSTR(SETLBL, "%s: cannot set sensitivity of %s\n"), "lpr", buf);
			unlink(buf);
			exit(1);
		}
#endif
	}
#endif /* SEC_BASE */
	if ((fd = open(buf, O_RDWR|O_CREAT, 0661)) < 0) {
		fprintf(stderr, MSGSTR(LPR_18, "%s: cannot create %s\n"), name, buf);
		exit(1);
	}
	if (flock(fd, LOCK_EX)) {
		fprintf(stderr, MSGSTR(LPR_31, "%s: cannot lock %s\n"), name, buf);
		exit(1);
	}
	n = 0;
	if ((len = read(fd, buf, sizeof(buf))) > 0) {
		for (cp = buf; len--; ) {
			if (*cp < '0' || *cp > '9')
				break;
			n = n * 10 + (*cp++ - '0');
		}
	}
	jobnumber = n;
	len = strlen(SD) + strlen(host) + 9;
	tfname = mktemp("tf", n, len);
	cfname = mktemp("cf", n, len);
	dfname = mktemp("df", n, len);
	inchar = strlen(SD) + 3;
	n = (n + 1) % 1000;
	(void) lseek(fd, 0L, 0);
	sprintf(buf, "%03d\n", n);
	(void) write(fd, buf, strlen(buf));
	(void) close(fd);	/* unlocks as well */
#if SEC_BASE
	seteffprivs(saveprivs, (priv_t *) 0);
#endif
}

/*
 * Make a temp file name.
 */
char *
mktemp(id, num, len)
	char	*id;
	int	num, len;
{
	register char *s;
	extern char *malloc();

	if ((s = malloc(len)) == NULL)
		fatal(MSGSTR(LPR_32, "out of memory"));
	(void) sprintf(s, "%s/%sA%03d%s", SD, id, num, host);
	return(s);
}

#if SEC_BASE
haslpauth()
{
	static int	has_auth = -1;

	if (has_auth == -1)
		has_auth = authorized_user("lp");
	return has_auth;
}
#endif

#if SEC_MAC
lpr_save_label(fd, file)
	int	fd;
	char	*file;
{
	int		error = 0;
	privvec_t	saveprivs;
	mand_ir_t	*sl;
#if SEC_ILB
	ilb_ir_t	*il;
#endif

	forceprivs(privvec(SEC_OWNER, SEC_ALLOWDACACCESS, SEC_ALLOWMACACCESS,
#if SEC_ILB
				SEC_ALLOWILBACCESS,
#endif
#if SEC_NCAV
				SEC_ALLOWNCAVACCESS,
#endif
				-1), saveprivs);

	if ((sl = mand_alloc_ir()) == (mand_ir_t *) 0 ||
	    fstatslabel(fd, sl) || chslabel(file, sl))
		++error;
	if (sl)
		free(sl);
#if SEC_ILB
	if ((il = ilb_alloc_ir()) == (ilb_ir_t *) 0 ||
	    fstatilabel(fd, il) || chilabel(file, il))
		++error;
	if (il)
		free(il);
#endif

	if (error)
		unlink(file);

	seteffprivs(saveprivs, (priv_t *) 0);
}
#endif /* SEC_MAC */


