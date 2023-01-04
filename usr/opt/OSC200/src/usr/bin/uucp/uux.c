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
static char rcsid[] = "@(#)$RCSfile: uux.c,v $ $Revision: 4.3.8.3 $ (DEC) $Date: 1993/10/11 19:36:06 $";
#endif
/* 
 * COMPONENT_NAME: UUCP uux.c
 * 
 * FUNCTIONS: APPCMD, GENRCV, GENSEND, Muux, cleanup, onintr 
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
1.12  uux.c, bos320 3/27/91 10:59:31";
*/
#include "uucp.h"
/* VERSION( uux.c	5.3 -  -  ); */

#define NOSYSPART 0
#define HASSYSPART 1

#define GENSEND(f, a, b, c) {\
ASSERT(fprintf(f, "S %s %s %s -%s %s 0666 %s %s\n", a, b, User, _Statop?"o":"", c, User, _Sfile) >= 0, Ct_WRITE, "", errno);\
}
#define GENRCV(f, a, b) {\
ASSERT(fprintf(f, "R %s %s %s - %s 0666 %s\n", a, b, User, *_Sfile ? _Sfile : "dummy", User) >= 0, \
Ct_WRITE, "", errno);\
}

#define USAGE	 "Usage: uux [-c | -C] [-n | -z] [- | -p] [-auser] [-bjr]\
 [-ggrade]\n\t[-sfile] [-xdebug_level] command_string\n" 
#define APPCMD(p)	{(void) strcat(cmd, p); (void) strcat(cmd, " ");}

static char	_Sfile[MAXFULLNAME];
static int	_Statop;

char	_Grade = 'N';
/*
 *	uux
 */
main(argc, argv)
char *argv[];
{
	FILE *fprx = NULL, *fpc = NULL, *fpd = NULL, *fp = NULL;
	extern onintr();
	int cfileUsed = 0;	/*  >0 if commands put in C. file flag  */
	int rflag = 0;		/*  C. files for receiving flag  */
	int cflag = 0;		/* if > 0 make local copy of files to be sent */
	int nflag = 0;		/* if != 0, do not request error notification */
	int zflag = 0;		/* if != 0, request success notification */
	int pipein = 0;
	int startjob = 1;
	short jflag = 0;	/* -j flag  output Jobid */
	int bringback = 0;	/* return stdin to invoker on error */
	int ret, i;
	char *getprm();
	char redir = '\0';
	char command = TRUE;
	char cfile[NAMESIZE];	/* send commands for files from here */
	char dfile[NAMESIZE];	/* used for all data files from here */
	char rxfile[NAMESIZE];	/* file for X_ commands */
	char tfile[NAMESIZE];	/* temporary file name */
	char t2file[NAMESIZE];	/* temporary file name */
	char buf[BUFSIZ];
	char inargs[BUFSIZ];
	char cmd[BUFSIZ];
	char *ap;
	char infile[BUFSIZ];
	char prm[BUFSIZ];
	char syspart[NAMEBUF], rest[BUFSIZ];
	char xsys[NAMEBUF];
	char	*fopt = NULL;
	char	*retaddr = NULL;
	struct stat stbuf;

	setlocale(LC_ALL,"");
	catd = catopen(MF_UUCP, NL_CAT_LOCALE);

	/*
	 * Test that we have at least one argument.
	 * If not stop here and exit.
	 */
	if ( argc < 2 ) {
		fprintf(stderr, MSGSTR(MSG_UUX_1, USAGE));
		exit(1);
	}

 /* we want this to run as uucp, even if the kernel doesn't */
	Uid = getuid();
	Euid = geteuid();	/* this should be UUCPUID */
	if (Uid == 0)
	    setuid(UUCPUID);

	/* choose LOGFILE */
	(void) strcpy(Logfile, LOGUUX);

	/*
	 * determine local system name
	 */
	(void) strcpy(Progname, "uux");
	Pchar = 'X';
	(void) signal(SIGILL, (void(*)(int)) onintr);
	(void) signal(SIGTRAP, (void(*)(int)) onintr);
	(void) signal(SIGIOT, (void(*)(int)) onintr);
	(void) signal(SIGFPE, (void(*)(int)) onintr);
	(void) signal(SIGBUS, (void(*)(int)) onintr);
	(void) signal(SIGSEGV, (void(*)(int)) onintr);
	(void) signal(SIGSYS, (void(*)(int)) onintr);
	(void) signal(SIGTERM, SIG_IGN);
	uucpname(Myname);
	Ofn = 1;
	Ifn = 0;
	*_Sfile = '\0';
	/*
	 * since getopt() can't handle the pipe input option '-'
	 * I'll change it to "-p"
	 */
	for (i=0; i<argc; i++)
	    if (EQUALS(argv[i], "-"))
		argv[i] = "-p";

	while ((i = getopt(argc, argv, "a:bcCjg:nprs:x:z")) != EOF) {
		switch(i){

		/*
		 * use this name in the U line
		 */
		case 'a':
			retaddr = optarg;
			break;

		/*
		 * if return code non-zero, return command's input
		 */
		case 'b':
			bringback = 1;
			break;

		/* do not make local copies of files to be sent (default) */
		case 'c':
			cflag = 0;
			break;

		/* make local copies of files to be sent */
		case 'C':
			cflag = 1;
			break;
		/*
		 * set priority of request
		 */
		case 'g':
			if ( !isalnum(*optarg) ) {
				fprintf(stderr, MSGSTR(MSG_UUX_19,
					"uux: The grade character %c is invalid use: a-z, A-Z, 0-9.\n"), *optarg);
				exit(2);
			}
			else
			_Grade = *optarg;
			break;


		case 'j':	/* job id */
			jflag = 1;
			break;


		/*
		 * do not send failure notification to user
		 */
		case 'n':
			nflag++;
			break;

		/*
		 * send success notification to user
		 */
		case 'z':
			zflag++;
			break;

		/*
		 * -p or - option specifies input from pipe
		 */
		case 'p':
			pipein = 1;
			break;

		/*
		 * do not start transfer
		 */
		case 'r':
			startjob = 0;
			break;

		case 's':
			fopt = optarg;
			_Statop++;
			break;

		/*
		 * debugging level
		 */
		case 'x':
			Debug = atoi(optarg);
			if (Debug <= 0)
				Debug = 1;
			break;

		default:
			fprintf(stderr, MSGSTR(MSG_UUX_1, USAGE));
			exit(2);
		}
	}

	DEBUG(4, "\n\n** %s **\n", "START");

	/*
	 * copy arguments into a buffer for later
	 * processing
	 */
	inargs[0] = '\0';
	for (; optind < argc; optind++) {
		DEBUG(4, "arg - %s:", argv[optind]);
		(void) strcat(inargs, " ");
		(void) strcat(inargs, argv[optind]);
	}
	
        /* 
	 * warn user if there are non-ascii characters
         * in the command line
         */
	DEBUG(4, "arg - %s\n", inargs);
#ifdef _KJI 		
	namep = inargs;
	while (*namep)
	  if (! (isascii(*namep++))) {
	    fprintf (stdout, MSGSTR(MSG_UUX_17,
	    "\nWARNING: A non-ascii character was detected in the uux command string:\n%s\n"), inargs);
	    fprintf (stdout, MSGSTR(MSG_UUX_18,
	    "\nNon-ascii characters are not allowed in system, user or file names\n\n"));
	    break;
        }
#endif

	/*
	 * get working directory and change
	 * to spool directory
	 */
	DEBUG(4, "arg - %s\n", inargs);
	gwd(Wrkdir);
	if(fopt){
		if(*fopt != '/')
			(void) sprintf(_Sfile, "%s/%s", Wrkdir, fopt);
		else
			(void) sprintf(_Sfile, "%s", fopt);

	}
	if (chdir(WORKSPACE) != 0) {
	    (void) fprintf(stderr, MSGSTR(MSG_UUX_3,
		"No spool directory - %s - get help\n"), WORKSPACE);
	    cleanup(12);
	}
	/*
	 * find remote system name
	 * remote name is first to know that 
	 * is not > or <
	 */
	ap = inargs;
	xsys[0] = '\0';
	while ((ap = getprm(ap, prm)) != NULL) {
		if (prm[0] == '>' || prm[0] == '<') {
			ap = getprm(ap, prm);
			continue;
		}

		/*
		 * split name into system name
		 * and command name
		 */
		split(prm, xsys, rest);
		break;
	}
	if (xsys[0] == '\0')
		(void) strcpy(xsys, Myname);
	strncpy(Rmtname, xsys, MAXBASENAME);
	Rmtname[MAXBASENAME] = '\0';
	DEBUG(4, "xsys %s\n", xsys);

	/*
	 * check to see if system name is valid
	 */
	if (versys(xsys) != 0) {
		/*
		 * bad system name
		 */
		fprintf(stderr, MSGSTR(MSG_UUX_4, "bad system name: %s\n"), 
			xsys);
		if (fprx != NULL)
			(void) fclose(fprx);
		if (fpc != NULL)
			(void) fclose(fpc);
		cleanup(11);
	}

	/*
	 * determine id of user starting remote 
	 * execution
	 */
	guinfo(Uid, User);
	(void) strcpy(Loginuser,User);

	DEBUG(6, "User %s\n", User);
	if (retaddr == NULL)
		retaddr = User;

	/*
	 * initialize command buffer
	 */
	*cmd = '\0';

	/*
	 * generate JCL files to work from
	 */

	/*
	 * fpc is the C. file for the local site.
	 * collect commands into cfile.
	 * commit if not empty (at end).
	 * 
	 * the appropriate C. file.
	 */
	gename(CMDPRE, xsys, _Grade, cfile);
	DEBUG(9, "cfile = %s\n", cfile);
	ASSERT(access(cfile, 0) != 0, Fl_EXISTS, cfile, errno);
	fpc = fdopen(ret = creat(cfile, CFILEMODE), "w");
	ASSERT(ret >= 0 && fpc != NULL, Ct_OPEN, cfile, errno);
	setbuf(fpc, CNULL);

	/*  set Jobid -- C.jobid */
	(void) strncpy(Jobid, BASENAME(cfile, '.'), NAMESIZE);
	Jobid[NAMESIZE-1] = '\0';

	/*
	 * rxfile is the X. file for the job, fprx is its stream ptr.
	 * if the command is to be executed locally, rxfile becomes
	 * a local X. file, otherwise we send it as a D. file to the
	 * remote site.
	 */
	
	gename(DATAPRE, xsys, 'X', rxfile);
	DEBUG(9, "rxfile = %s\n", rxfile);
	ASSERT(access(rxfile, 0) != 0, Fl_EXISTS, rxfile, errno);
	fprx = fdopen(ret = creat(rxfile, DFILEMODE), "w");
	ASSERT(ret >= 0 && fprx != NULL, Ct_WRITE, rxfile, errno);
	setbuf(fprx, CNULL);
	clearerr(fprx);

	(void) fprintf(fprx,"%c %s %s\n", X_USER, User, Myname);
	if (zflag) {
		(void) fprintf(fprx, MSGSTR(MSG_UUX_5,
			"%c return status on success\n"), X_COMMENT);
		(void) fprintf(fprx,"%c\n", X_SENDZERO);
	}

	if (nflag) {
		(void) fprintf(fprx, MSGSTR(MSG_UUX_6, 
			"%c don't return status on failure\n"), X_COMMENT);
		(void) fprintf(fprx,"%c\n", X_SENDNOTHING);
	} else {
		(void) fprintf(fprx, MSGSTR(MSG_UUX_7, 
			"%c return status on failure\n"), X_COMMENT);
		fprintf(fprx,"%c\n", X_NONZERO);
	}

	if (bringback) {
		(void) fprintf(fprx, MSGSTR(MSG_UUX_8, 
			"%c return input on abnormal exit\n"), X_COMMENT);
		(void) fprintf(fprx,"%c\n", X_BRINGBACK);
	}
	if (_Statop) {
		/* Check and make sure file exists */
		if (access(_Sfile,W_OK) == (int) NULL) {
			fprintf(stderr, MSGSTR(MSG_UUX_2,
				"File %s is not writable.\n"), _Sfile);
			exit(2);
		}
		else
			(void) fprintf(fprx,"%c %s\n", X_MAILF, _Sfile);
	}

	if (retaddr != NULL) {
		(void) fprintf(fprx, MSGSTR(MSG_UUX_9,
		  "%c return address for status or input return\n"), X_COMMENT);
		(void) fprintf(fprx,"%c %s\n", X_RETADDR, retaddr);
	}

	/*
	 * create a JCL file to spool pipe input into
	 */
	if (pipein) {
		/*
		 * fpd is the D. file into which we now read
		 * input from stdin
		 */
		
		gename(DATAPRE, Myname, 'B', dfile);
		
		ASSERT(access(dfile, 0) != 0, Fl_EXISTS, dfile, errno);
		fpd = fdopen(ret = creat(dfile, DFILEMODE), "w");
		ASSERT(ret >= 0 && fpd != NULL, Ct_OPEN, dfile, errno);

		/*
		 * read pipe to EOF
		 */
		while (!feof(stdin)) {
			ret = fread(buf, 1, BUFSIZ, stdin);
			ASSERT(fwrite(buf, 1, ret, fpd) == ret, Ct_WRITE,
				dfile, errno);
		}
		ASSERT(ferror(fpd) == 0, Ct_WRITE, dfile, errno);
		(void) fclose(fpd);

		/*
		 * if command is to be executed on remote
		 * create extra JCL
		 */
		if (!EQUALSN(Myname, xsys, SYSNSIZE)) {
			GENSEND(fpc, dfile, dfile, dfile);
		}

		/*
		 * create file for X_ commands
		 */
		(void) fprintf(fprx, "%c %s\n", X_RQDFILE, dfile);
		(void) fprintf(fprx, "%c %s\n", X_STDIN, dfile);

		wfcommit(dfile, dfile, xsys);

	}
	/*
	 * parse command
	 */
	ap = inargs;
	while ((ap = getprm(ap, prm)) != NULL) {
		DEBUG(4, "prm - %s\n", prm);

		/*
		 * redirection of I/O
		 */
		if (prm[0] == '>' || prm[0] == '<') {
			redir = prm[0];
			continue;
		}

		if (prm[0] == '{') {
			char *rquote, *strchr();
			if ((rquote = strchr(prm, '}')) != NULL) {
				redir = '>';
				*(ap-1) = ' ';
				ap = ap - strlen(prm) + 1;
				continue;
			}
		}

		/*
		 * some terminator
		 */
		if ( prm[0] == '|' || prm[0] == '^'
		  || prm[0] == '&' || prm[0] == ';') {
			if (*cmd != '\0')	/* not 1st thing on line */
				APPCMD(prm);
			command = TRUE;
			continue;
		}

		/*
		 * process command or file or option
		 * break out system and file name and
		 * use default if necessary
		 */
		ret = split(prm, syspart, rest);
		DEBUG(4, "syspart -> %s, ", syspart);
		DEBUG(4, "rest -> %s, ", rest);
		DEBUG(4, "ret -> %d\n", ret);

		if (command  && redir == '\0') {
			/*
			 * command
			 */
			APPCMD(rest);
			(void) strcpy(infile, rest);
			command = FALSE;
			continue;
		}

		if (syspart[0] == '\0') {
			(void) strcpy(syspart, Myname);
			DEBUG(6, "syspart -> %s\n", syspart);
		} else if (versys(syspart) != 0) {
			/*
			 * bad system name
			 */
			fprintf(stderr, MSGSTR(MSG_UUX_11,
			  "bad system name: %s\n"), syspart);
			if (fprx != NULL)
				(void) fclose(fprx);
			if (fpc != NULL)
				(void) fclose(fpc);
			cleanup(11);
		}

		/*
		 * process file or option
		 */

		/*
		 * process file argument
		 * expand filename and create JCL card for
		 * redirected output
		 * e.g., X file sys
		 */
		if (redir == '>') {
			if (rest[0] != '~')
				if (ckexpf(rest))
					cleanup(6);
			ASSERT(fprintf(fprx, "%c %s %s\n", X_STDOUT, rest,
			 syspart) >= 0, Ct_WRITE, rxfile, errno);
			redir = '\0';
			continue;
		}

		/*
		 * if no system specified, then being
		 * processed locally
		 */
		if (ret == NOSYSPART && redir == '\0') {

			/*
			 * option
			 */
			APPCMD(rest);
			continue;
		}


		/* local xeqn + local file  (!x !f) */
		if ((EQUALSN(xsys, Myname, SYSNSIZE))
		 && (EQUALSN(xsys, syspart, SYSNSIZE))) {
			/*
			 * create JCL card
			 */
			if (ckexpf(rest))
				cleanup(7);
			/*
			 * JCL card for local input
			 * e.g., I file
			 */
			if (redir == '<') {
				(void) fprintf(fprx, "%c %s\n", X_STDIN, rest);
			} else
				APPCMD(rest);
			ASSERT(fprx != NULL, Ct_WRITE, rxfile, errno);
			redir = '\0';
			continue;
		}

		/* remote xeqn + local file (sys!x !f) */
		if (EQUALSN(syspart, Myname, SYSNSIZE)) {
			/*
			 * check access to local file
			 * if cflag is set, copy to spool directory
			 * otherwise, just mention it in the X. file
			 */
			if (ckexpf(rest))
				cleanup(6);
			DEBUG(4, "rest %s\n", rest);

			/* see if I can read this file as read uid, gid */
			if (!access(rest,R_OK) != 0) {
				fprintf(stderr,MSGSTR(MSG_UUX_13,
				   "permission denied %s\n"), rest);
				cleanup(1);
			}

			/* D. file for sending local file */
			gename(DATAPRE, xsys, 'A', dfile);

			if (cflag) {
				/* make local copy */
				if (uidxcp(rest, dfile) != 0) {
				    fprintf(stderr,MSGSTR(MSG_UUX_15,
					"can't copy %s\n"), rest);
				    cleanup(5);
				}
				(void) chmod(dfile, DFILEMODE);
				/* generate 'send' entry in command file */
				GENSEND(fpc, rest, dfile, dfile);
				wfcommit(dfile, dfile, xsys);	/* commit input */
			} else		/* don't make local copy */
				GENSEND(fpc, rest, dfile, "D.0");

			/*
			 * JCL cards for redirected input in X. file,
			 * e.g.
			 * I D.xxx
			 * F D.xxx
			 */
			if (redir == '<') {
				/*
				 * don't bother making a X_RQDFILE line that
				 * renames stdin on the remote side, since the
				 * remote command can't know its name anyway
				 */
				(void) fprintf(fprx, "%c %s\n", X_STDIN, dfile);
				(void) fprintf(fprx, "%c %s\n", X_RQDFILE, dfile);
			} else {
				APPCMD(BASENAME(rest, '/'));;
				/*
				 * generate X. JCL card that specifies
				 * F file 
				 */
				(void) fprintf(fprx, "%c %s %s\n", X_RQDFILE,
				 dfile, BASENAME(rest, '/'));
			}
			redir = '\0';

			continue;
		}

		/* local xeqn + remote file (!x sys!f ) */
		if (EQUALS(Myname, xsys)) {
			/*
			 * expand receive file name
			 */
			if (ckexpf(rest))
				cleanup(6);
			/*
			 * tfile is command file for receive from remote.
			 * we defer commiting until later so 
			 * that only one C. file is created per site.
			 *
			 * dfile is name of data file to receive into;
			 * we don't use it, just name it.
			 *
			 * the name of the remote is appended to the
			 * X_RQDFILE line to help uuxqt in finding the
			 * D. file when it arrives.
			 */
			if (gtcfile(tfile, syspart) != SUCCESS) {
				gename(CMDPRE, syspart, 'R', tfile);
				
				ASSERT(access(tfile, 0) != 0,
				    Fl_EXISTS, tfile, errno);
				svcfile(tfile, syspart);
				(void) close(creat(tfile, CFILEMODE));
			}
			fp = fopen(tfile, "a");
			ASSERT(fp != NULL, Ct_OPEN, tfile, errno);
			setbuf(fp, CNULL);
			gename(DATAPRE, syspart, 'R', dfile);

			/* prepare JCL card to receive file */
			GENRCV(fp, rest, dfile);
			ASSERT(ferror(fp) == 0, Ct_WRITE, dfile, errno);
			(void) fclose(fp);
			rflag++;
			if (rest[0] != '~')
				if (ckexpf(rest))
					cleanup(7);

			/*
			 * generate receive entries
			 */
			if (redir == '<') {
				(void) fprintf(fprx,
					"%c %s/%s/%s\n", X_RQDFILE, Spool,
					syspart, dfile);
				(void) fprintf(fprx, "%c %s\n", X_STDIN, dfile);
			} else {
				(void) fprintf(fprx, "%c %s/%s/%s %s\n",
				X_RQDFILE, Spool, syspart, dfile,
				BASENAME(rest, '/'));
				APPCMD(BASENAME(rest, '/'));
			}

			redir = '\0';
			continue;
		}

		/* remote xeqn/file, different remotes (xsys!cmd syspart!rest) */
		if (!EQUALS(syspart, xsys)) {
			/*
			 * strategy:
			 * request rest from syspart.
			 *
			 * set up a local X. file that will send rest to xsys,
			 * once it arrives from syspart.
			 *
			 * arrange so that the xsys D. file (fated to become
			 * an X. file on xsys), rest is required and named.
			 *
			 * pictorially:
			 *
			 * ===== syspart/C.syspartR.... =====	(tfile)
			 * R rest D.syspart...			(dfile)
			 *
			 *
			 * ===== local/X.local... =====		(t2file)
			 * F Spool/syspart/D.syspart... rest	(dfile)
			 * C uucp -C rest D.syspart...		(dfile)
			 *
			 * ===== xsys/D.xsysG....		(fprx)
			 * F D.syspart... rest			(dfile)
			 * or, in the case of redir == '<'
			 * F D.syspart...			(dfile)
			 * I D.syspart...			(dfile)
			 *
			 * while we do push rest around a bunch,
			 * we use the protection scheme to good effect.
			 *
			 * we must rely on uucp's treatment of requests
			 * form XQTDIR to get the data file to the right
			 * place ultimately.
			 */

			/* build (or append to) C.syspartR... */
			if (gtcfile(tfile, syspart) != SUCCESS) {
				gename(CMDPRE, syspart, 'R', tfile);
				
				ASSERT(access(tfile, 0) != 0,
				    Fl_EXISTS, tfile, errno);
				svcfile(tfile, syspart);
				(void) close(creat(tfile, CFILEMODE));
			}
			fp = fopen(tfile, "a");
			ASSERT(fp != NULL, Ct_OPEN, tfile, errno);
			setbuf(fp, CNULL);
			gename(DATAPRE, syspart, 'R', dfile);
			GENRCV(fp, rest, dfile);
			ASSERT(ferror(fp) == 0, Ct_WRITE, dfile, errno);
			(void) fclose(fp);

			/* build local/X.localG... */
			gename(XQTPRE, Myname, _Grade, t2file);
			ASSERT(access(t2file, 0)!=0, Fl_EXISTS, t2file, errno);
			(void) close(creat(t2file, CFILEMODE));
			fp = fopen(t2file, "w");
			ASSERT(fp != NULL, Ct_OPEN, t2file, errno);
			setbuf(fp, CNULL);
			(void) fprintf(fp, "%c %s/%s/%s %s\n", X_RQDFILE,
				Spool, syspart, dfile, BASENAME(rest, '/'));
			(void) fprintf(fp, "%c uucp -C %s %s!%s\n",
				X_CMD, BASENAME(rest, '/'), xsys, dfile);
			ASSERT(ferror(fp) == 0, Ct_WRITE, t2file, errno);
			(void) fclose(fp);

			/* put t2file where uuxqt can get at it */
			wfcommit(t2file, t2file, Myname);

			/* generate xsys/X.sysG... cards */
			if (redir == '<') {
				(void) fprintf(fprx, "%c %s\n",
					X_RQDFILE, dfile);
				(void) fprintf(fprx, "%c %s\n", X_STDIN, dfile);
			} else {
				(void) fprintf(fprx, "%c %s %s\n", X_RQDFILE,
				 dfile, BASENAME(rest, '/'));
				APPCMD(BASENAME(rest, '/'));
			}
			redir = '\0';
			continue;
		}

		/* remote xeqn + remote file, same remote (sys!x sys!f) */
		if (rest[0] != '~')	/* expand '~' on remote */
			if (ckexpf(rest))
				cleanup(7);
		if (redir == '<') {
			(void) fprintf(fprx, "%c %s\n", X_STDIN, rest);
		}
		else
			APPCMD(rest);
		redir = '\0';
		continue;

	}

	/*
	 * check access to local file
	 * if cflag is set, copy to spool directory
	 * otherwise, just mention it in the X. file
	 */
	if (cflag) {
		if (uidxcp(infile, dfile) != 0) {
			fprintf(stderr,MSGSTR(MSG_UUX_15,
				"uux: Cannot copy%s.\n"), rest);
			cleanup(5);
		}
		(void) chmod(dfile, DFILEMODE);
		/* generate 'send' entry in command file */
		GENSEND(fpc, infile, dfile, dfile);
		wfcommit(dfile, dfile, xsys);   /* commit input */
	}
	else            /* don't make local copy */
		GENSEND(fpc, infile, dfile, "D.0");

	/*
	 * place command to be executed in JCL file
	 */
	(void) fprintf(fprx, "%c %s\n", X_CMD, cmd);
	ASSERT(ferror(fprx) == 0, Ct_WRITE, rxfile, errno);
	(void) fclose(fprx);		/* rxfile is ready for commit */
	logent(cmd, "QUEUED");

	/* the X. name must have the X.Jobid so status reporting is by jobid */
	(void) sprintf(tfile, "X.%s", Jobid);
	if (EQUALS(xsys, Myname)) {
		/* local xeqn -- use X_ file here */
		wfcommit(rxfile, tfile, xsys);
		
		/*
		 * see if -r option requested JCL to be queued only
		 */
		if (startjob)
			xuuxqt(Myname);
	} else {
		/* remote xeqn -- send rxfile to remote */
		/* put it in a place where cico can get at it */
		wfcommit(rxfile, rxfile, xsys);

		GENSEND(fpc, rxfile, tfile, rxfile);
	}

	cfileUsed = (ftell(fpc) != 0L);	/* was cfile used? */
	ASSERT(ferror(fpc) == 0, Ct_WRITE, cfile, errno);
	(void) fclose(fpc);

	/*
	 * has any command been placed in command JCL file
	 */
	if (cfileUsed) {
		wfcommit(cfile, cfile, xsys);
		/*
		 * see if -r option requested JCL to be queued only
		 */
		if (startjob)
			xuucico(xsys);
	} else
		unlink(cfile);

	/* commit any leftover C. files for remote receive */
	commitall();
	if (jflag)	/* print Jobid */
	    printf("%s\n", Jobid);

	cleanup(0);
}


/*
 * cleanup and unlink if error
 *	code	-> exit code
 * return:
 *	none
 */
cleanup(code)
register int code;
{
	rmlock(CNULL);
	if (code) {
		wfabort();
		fprintf(stderr, MSGSTR(MSG_UUX_14,
			"uux failed ( %d )\n"), code);
	}
	DEBUG(1, "exit code %d\n", code);
	catclose(catd);

	if (code < 0)
		exit(-code);
	else
		exit(code);
}

/*
 * catch signal then cleanup and exit
 */
onintr(inter)
register int inter;
{
	char str[30];
	(void) signal(inter, SIG_IGN);
	(void) sprintf(str, "XSIGNAL %d", inter);
	logent(str, "XCAUGHT");
	cleanup(-inter);
}
