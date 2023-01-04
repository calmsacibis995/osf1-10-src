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
static char rcsid[] = "@(#)$RCSfile: main.c,v $ $Revision: 4.2.9.3 $ (DEC) $Date: 1993/10/11 15:35:27 $";
#endif
/*
 * HISTORY
 */
/*
 * OSF/1 Release 1.0.1
 */
/* 
 * COMPONENT_NAME: CMDMAILX main.c
 * 
 * FUNCTIONS: MSGSTR, Mmain, hdrstop 
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
 *	main.c       5.3 (Berkeley) 9/15/85
 */

#include "rcv.h"
#include <sys/stat.h>
#include <locale.h>

#include "Mail_msg.h" 
nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_MAILX,n,s) 

/*
 * Mail -- a mail program
 *
 * Startup -- interface with user.
 */

jmp_buf	hdrjmp;



int     Fflag = 0;                      /* -F option (followup) - SVID-2 */
int     Hflag = 0;                      /* print headers and exit - SVID-2 */
int	exitflg = 0;			/* -e for mail test - SVID-2 */
char	*prompt = NULLSTR;		/* prompt string - SVID-2 */


/*
 * Find out who the user is, copy his mail file (if exists) into
 * gettmpdir()/Rxxxxx and set up the message pointers.  Then, print out the
 * message headers and read user commands.
 *
 * Command line syntax:
 *	Mail [ -i ] [ -r address ] [ -h number ] [ -f [ name ] ]
 * or:
 *	Mail [ -i ] [ -r address ] [ -h number ] people ...
 */

main(argc, argv)
	char **argv;
{
	register char *ef;
	register int i, argp;
	int mustsend, uflag, f;
	void hdrstop(), (*prevint)();
	FILE *ibuf, *ftat;
	struct sgttyb tbuf;
#ifdef ASIAN_I18N
	struct	stat	sbuf;
	char	mrcstr[128];
#endif

	(void) setlocale(LC_ALL,""); /*GAG*/
	catd = catopen(MF_MAIL,NL_CAT_LOCALE);

#ifdef signal
	Siginit();
#endif

	/*
	 * Set up a reasonable environment.  We clobber the last
	 * element of argument list for compatibility with version 6,
	 * figure out whether we are being run interactively, set up
	 * all the temporary files, buffer standard output, and so forth.
	 */

	uflag = 0;
#ifdef	GETHOST
	inithost();
#endif	/*GETHOST*/
	mypid = getpid();
	intty = isatty(0);
	outtty = isatty(1);
	if (outtty) {
		gtty(1, &tbuf);
		baud = tbuf.sg_ospeed;
	}
	else
		baud = B9600;
	image = -1;

	/*
	 * Now, determine how we are being used.
	 * We successively pick off instances of -r, -h, -f, and -i.
	 * If called as "rmail" we note this fact for letter sending.
	 * If there is anything left, it is the base of the list
	 * of users to mail to.  Argp will be set to point to the
	 * first of these users.
	 */

	ef = NULLSTR;
	argp = -1;
	mustsend = 0;
	if (argc > 0 && **argv == 'r')
		rmail++;
	for (i = 1; i < argc; i++) {

		/*
		 * If current argument is not a flag, then the
		 * rest of the arguments must be recipients.
		 */

		if (*argv[i] != '-') {
			argp = i;
			break;
		}
		switch (argv[i][1]) {
		case 'e':			/* added for SVID-2 */
			/*
			 * exit status only
			 */
			exitflg++;
			break;

		case 'r':
			/*
			 * Next argument is address to be sent along
			 * to the mailer.
			 */
			if (i >= argc - 1) {
				fprintf(stderr, MSGSTR(NEEDADR, "Address required after -r\n")); /*MSG*/
				exit(1);
			}
			mustsend++;
			rflag = argv[i+1];
			i++;
			break;

		case 'T':
			/*
			 * Next argument is temp file to write which
			 * articles have been read/deleted for netnews.
			 */
			if (i >= argc - 1) {
				fprintf(stderr, MSGSTR(NEEDNM, "Name required after -T\n")); /*MSG*/
				exit(1);
			}
			Tflag = argv[i+1];
			if ((f = creat(Tflag, 0600)) < 0) {
				perror(Tflag);
				exit(1);
			}
			close(f);
			i++;
			break;

		case 'u':
			/*
			 * Next argument is person to pretend to be.
			 */
			uflag++;
			if (i >= argc - 1) {
				fprintf(stderr, MSGSTR(NONAME, "Missing user name for -u\n")); /*MSG*/
				exit(1);
			}
			strcpy(myname, argv[i+1]);
			i++;
			break;

		case 'i':
			/*
			 * User wants to ignore interrupts.
			 * Set the variable "ignore"
			 */
			assign("ignore", "");
			break;

		case 'd':
			debug++;
			break;

		case 'h':
			/*
			 * Specified sequence number for network.
			 * This is the number of "hops" made so
			 * far (count of times message has been
			 * forwarded) to help avoid infinite mail loops.
			 */
			if (i >= argc - 1) {
				fprintf(stderr, MSGSTR(NEEDNUM, "Number required for -h\n")); /*MSG*/
				exit(1);
			}
			mustsend++;
			hflag = atoi(argv[i+1]);
			if (hflag == 0) {
				fprintf(stderr, MSGSTR(NEEDNOZ, "-h needs non-zero number\n")); /*MSG*/
				exit(1);
			}
			i++;
			break;

		case 'H':			/* for SVID-2 */
			Hflag++;
			break;

		case 's':
			/*
			 * Give a subject field for sending from
			 * non terminal
			 */
			if (i >= argc - 1) {
				fprintf(stderr, MSGSTR(NEEDSUBJ, "Subject req'd for -s\n")); /*MSG*/
				exit(1);
			}
			mustsend++;
			sflag = argv[i+1];
			i++;
			break;

		case 'f':
			/*
			 * User is specifying file to "edit" with Mail,
			 * as opposed to reading system mailbox.
			 * If no argument is given after -f, we read his
			 * mbox file in his home directory.
			 */
			if (i >= argc - 1)
				ef = mbox;
			else
				ef = argv[i + 1];
			i++;
			break;


                case 'F':		/* added for SVID-2 */
                        Fflag++;
                        mustsend++;
                        break;


		case 'n':
			/*
			 * User doesn't want to source /usr/lib/Mail.rc
			 */
			nosrc++;
			break;

		case 'N':
			/*
			 * Avoid initial header printing.
			 */
			noheader++;
			break;

		case 'v':
			/*
			 * Send mailer verbose flag
			 */
			assign("verbose", "");
			break;

		case 'I':
			/*
			 * We're interactive
			 */
			intty = 1;
			break;

		default:
			fprintf(stderr, MSGSTR(UNKFLAG, "Unknown flag: %s\n"), argv[i]); /*MSG*/
			exit(1);
		}
	}

	/*
	 * Check for inconsistent arguments.
	 */

	if (ef != NULLSTR && argp != -1) {
		fprintf(stderr, MSGSTR(NOWORK, "Cannot give -f and people to send to.\n")); /*MSG*/
		exit(1);
	}
	if (exitflg && (mustsend || argp != -1))	/* added for SVID-2 */
		exit(1);	/* nonsense flags involving -e simply exit */
	if (mustsend && argp == -1) {
		fprintf(stderr, MSGSTR(NOSENSE, "The flags you gave make no sense since you're not sending mail.\n")); /*MSG*/
		exit(1);
	}
	tinit();
	input = stdin;
	rcvmode = argp == -1;
	if (!nosrc)
#ifdef ASIAN_I18N
	{
		strcpy(mrcstr,I18NPATH);
		strcat(mrcstr,MASTER);
		if (stat(mrcstr, &sbuf) < 0) load_ud(MASTER);
		else load_ud(mrcstr);
	}
	load_ud(mailrc);
#else
		load(MASTER);
	load(mailrc);
#endif
	findmail();  /* set up maildir */
	if (!debug && value("debug") != NULLSTR) debug++;
	if (debug) {
		printf("uid = %d, user = %s, mailname = %s\n",
		    uid, myname, mailname);
		printf("deadletter = %s, mailrc = %s, mbox = %s\n",
		    deadletter, mailrc, mbox);
	}
#ifdef ASIAN_I18N
	init_conversion();
#endif
	if (argp != -1) {
		mail(&argv[argp]);

#ifdef ASIAN_I18N
		remove_tempfiles();
#endif
		/*
		 * why wait?
		 */

		exit(senderr);
	}

	/*
	 * Ok, we are reading mail.
	 * Decide whether we are editing a mailbox or reading
	 * the system mailbox, and open up the right stuff.
	 */

	if (ef != NULLSTR) {
		char *ename;

		edit++;
		ename = expand(ef);
		if (ename != ef) {
			ef = (char *) calloc(1, strlen(ename) + 1);
			strcpy(ef, ename);
		}
		editfile = ef;
		strcpy(mailname, ef);
	}
	if (setfile(mailname, edit) < 0) {
		if (edit)
			perror(mailname);
		exit(1);
	}

	else	/* if mail, and -e option, exit with 0 status - for SVID-2 */
	{
		if( exitflg)
			exit(0);
	}

	/* change undocumented environment variable "noheader" to
	 * documented environment varaible "header" for SVID-2
	 */
	if (!noheader && value("header") != NULLSTR) {
		if (setjmp(hdrjmp) == 0) {
			if ((prevint = signal(SIGINT, SIG_IGN)) != SIG_IGN)
				signal(SIGINT, hdrstop);
			announce(!0);
			fflush(stdout);
			signal(SIGINT, prevint);
		}
	}
	/* If the user only wants a printout of new mail headers, exit here
	 * only print no mail message if -H was not specified
	 * added for SVID-2
	 */

	if (Hflag || (!edit && msgCount == 0)) {
		if (!Hflag)
			printf(MSGSTR(NMAIL, "No mail\n")); /*MSG*/
		fflush(stdout);
		exit(0);
	}
	commands();
	if (!edit) {
		signal(SIGHUP, SIG_IGN);
		signal(SIGINT, SIG_IGN);
		signal(SIGQUIT, SIG_IGN);
		quit();
	}

#ifdef ASIAN_I18N
		remove_tempfiles();
#endif
	exit(0);
}

/*
 * Interrupt printing of the headers.
 */
void
hdrstop()
{

	fflush(stdout);
	fprintf(stderr, MSGSTR(INTERPT, "\nInterrupt\n")); /*MSG*/
	longjmp(hdrjmp, 1);
}
