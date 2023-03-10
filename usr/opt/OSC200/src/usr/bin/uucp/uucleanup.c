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
static char rcsid[] = "@(#)$RCSfile: uucleanup.c,v $ $Revision: 4.3.8.3 $ (DEC) $Date: 1993/10/11 19:31:11 $";
#endif
/* 
 * COMPONENT_NAME: UUCP uucleanup.c
 * 
 * FUNCTIONS: FULLNAME, Muucleanup, _age, analFrom, cleanup, cprocess, 
 *            dDprocess, dMprocess, dNprocess, dType, dXprocess, 
 *            execRnews, init_msg, logent, logit, oprocess, sendMail, 
 *            systat, toWho, tprocess, wprocess, xprocess 
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*
1.14  com/cmd/uucp/uucleanup.c, 9115320a, bos320 11/28/90 17:11:00";
*/
/*	uucp:uucleanup.c	1.6.1.1

 *	uucleanup - This is a program based on the heuristics
 *	for cleaning up and doing something
 *	useful with old files left in the uucp queues.
 *	It also will send warning messags to users where requests are not
 *	going out due to failure to contact the remote system.
 *
 *	This program knows a lot about the construction and
 *	contents of the C., D. and X. files.  In addition, it
 *	thinks it knows what mail and netnews data files look like.
 *
 *	At present, this is what is done:
 *	For WARNING messages:
 *	C. files of age given by -W option are read, looking for
 *		either user files to be sent or received, or
 *		mail to be sent.  (Other remote execution that
 *		does not involve sending user files is not checked
 *		for now.)  In either of the cases, the user is
 *		informed by mail that the request is not being
 *		processed due to lack of communications with the remote
 *		system, and the request will be deleted in the future
 *		if it the condition remains for several more days.
 *
 *	For DELETIONS:
 *	C. files - if they reference only D. files, the C. is
 *		merely deleted, because the D. files are usually
 *		mail or news, and the later D. processing will
 *		take care of them.
 *		- if they reference files from the file system,
 *		a message is constructed that will contain a
 *		lines like
 *		   We can't contact the remote.
 *
 *		   local!file -> remote!otherfile
 *
 *		   can't be executed.
 *	X. files - merely deleted at present - D.s will be taken
 *		care of later. Besides, some of the D.s are
 *		missing or else the X. wouldn't be left around.
 *	D. files - mail type data is sent to a local person if that
 *		is where it originated.  If not, it is returned to the
 *		sender -- assumed to be from the first From line.  If
 *		a sender can't be determing, the file is merely deleted.
 *		- rnews: if locally generated, just delete.  If remote,
 *		the X. got lost, so execute rnews.
 *	other files - just delete them.
 *
 *	Deletions and executions are logged in
 *	(CLEANUPLOG)--/usr/spool/uucp/.Admin/uucleanup
*/

#include	"uucp.h"
/* VERSION( uucleanup.c	5.2 -  -  ); */

#ifdef	V7
#define O_RDONLY	0
#endif

#define USAGE	"Usage: uucleanup [-oDAYS] [-mSTRING] [-Cdays] [-Ddays] \
[-Tdays] [-Wdays] [-Xdays] [-xLEVEL] [-sSYSTEM]\n"

extern int _age();		/* find the age of a file */
extern void oprocess(), xprocess(), cprocess(), tprocess();
extern void dXprocess(), dNprocess(), dMprocess(), dDprocess(), wprocess(); 
extern int toWho(), sendMail(), execRnews();
extern void logit();

/* need these dummys to satisy some .o files */
cleanup(){}
void systat(){}
void logent(){}


/* types of D. files */
#define D_MAIL	1
#define D_NEWS	2
#define D_XFILE	3
#define D_DATA	4
#define FULLNAME(full,dir,file)	(void) sprintf(full, "%s/%s", dir, file);

int _Ddays = 7;			/* D. limit */
int _Cdays = 7;			/* C. limit */
int _Tdays = 2;			/* T. limit */
int _Xdays = 2;			/* X. limit */
int _Odays = 2;			/* O. limit ... actually for "other" files */
int _Wdays = 1;			/* Warning limit for C. files */
char _ShortLocal[6];		/* 5 char or less version of local name */

char *_Undeliverable[6]; 
#ifdef notdef
char *_Undeliverable[] = {
"Subject: Undeliverable Mail\n",
"This mail message is undeliverable.\n",
"(Probably to or from system '%s')\n",
"It was sent to you or by you.\n",
"Sorry for the inconvenience.\n",
"",
};
#endif

#define CANT1	2	/* first line to fill in */
#define CANT2	3	/* second line to fill in */

char *_CantContact[5];
#ifdef notdef
char *_CantContact[] = {
"Subject: Job Killed By uucp\n",
"We can't contact machine '%s'.\n",
" ",	/* uucleanup will fill in variable text here */
" ",	/* fill in jobid of killed job */
"",
};
#endif

#define WARN1	2
#define WARN2	5
#define WARN3	6

char *_Warning[8];
#ifdef notdef
char *_Warning[] = {
"Subject: Warning From uucp\n",
"We have been unable to contact machine '%s' since you queued your job.\n",
" ",	/*  wprocess FILLS IN THIS LINE OF TEXT */
"The job will be deleted in several days if the problem is not corrected.\n",
"If you care to kill the job, execute the following command:\n",
" ",	/*  wprocess FILLS IN THIS LINE WITH:  uustat -kJOBid */
" ",	/* FILL IN THE -m STRING IF SPECIFIED */
"",
};
#endif

main(argc, argv, envp)
char *argv[];
char **envp;
{
	DIR *spooldir, *subdir;
	char f[MAXFULLNAME], subf[MAXFULLNAME];
	char fullname[MAXFULLNAME];
	int pass = 0;	/* pass counter - multi passes through directory */
	char soptName[MAXFULLNAME];	/* name from -s option */
	int i, value, got_one;

	setlocale(LC_ALL,"");
	catd = catopen(MF_UUCP, NL_CAT_LOCALE);

	init_msg();

	soptName[0] = NULLCHAR;
	(void) strcpy(Logfile, CLEANUPLOGFILE);
	uucpname(Myname);
	(void) strncpy(_ShortLocal, Myname, 5);
	_ShortLocal[5] = NULLCHAR;
	(void) strcpy(Progname, "uucleanup");
	while ((i = getopt(argc, argv, "C:D:T:W:X:m:o:s:x:")) != EOF) {
		switch(i){
		case 's':	/* for debugging - choose system */
			(void) strcpy(soptName, optarg);
			
		case 'x':
			Debug = atoi(optarg);
			if (Debug <= 0)
				Debug = 1;
			break;
		case 'm':
			_Warning[WARN3] = optarg;
			break;
		default:
			fprintf(stderr,MSGSTR(MSG_UCL_1,USAGE));
			exit(1);

		case 'C':
		case 'D':
		case 'T':
		case 'W':
		case 'X':
		case 'o':
		    value = atoi(optarg);
		    if (value < 1) {
			fprintf(stderr,MSGSTR(MSG_UCL_12,
			    " Options: CDTWXo require value > 0\n"));
			exit(1);
		    }
		    switch(i) {
		    case 'C':
			_Cdays = value;
			break;
		    case 'D':
			_Ddays = value;
			break;
		    case 'T':
			_Tdays = value;
			break;
		    case 'W':
			_Wdays = value;
			break;
		    case 'X':
			_Xdays = value;
			break;
		    case 'o':
			_Odays = value;
			break;
		    }
		    break;
		}
	}

	if (argc != optind) {
		fprintf(stderr, MSGSTR(MSG_UCL_1, USAGE));
		exit(1);
	}

	DEBUG(5, "Progname (%s): STARTED\n", Progname);
	DEBUG(5, "Myname (%s), ", Myname);
	DEBUG(5, "_ShortLocal (%s)\n", _ShortLocal);
	DEBUG(5, "Days C.(%d), ", _Cdays);
	DEBUG(5, "D.(%d), ", _Ddays);
	DEBUG(5, "T.(%d), ", _Tdays);		/* this is a PS/2-ism */
	DEBUG(5, "W.(%d), ", _Wdays);
	DEBUG(5, "X.(%d), ", _Xdays);
	DEBUG(5, "other (%d)\n", _Odays);

	if (chdir(SPOOL) != 0) {
	    (void) fprintf(stderr, MSGSTR(MSG_UCL_13, 
			"CAN'T CHDIR (%s): errno (%d)\n"), SPOOL, errno);
		exit(1);
	}
	if ( (spooldir = opendir(SPOOL)) == NULL) {
	    (void) fprintf(stderr, MSGSTR(MSG_UCL_14, 
		"CAN'T OPEN (%s): errno (%d)\n"), SPOOL, errno);
		exit(1);
	}

	got_one = 0;
	while (gnamef(spooldir, f) == TRUE) {
	    if (EQUALSN("LCK..", f, 5))
		continue;

	    if (*soptName && !EQUALS(soptName, f))
		continue;

	    if (DIRECTORY(f)) {
		got_one = 1;
		(void) strcpy(Rmtname, f);
		pass = 0;
		/*
		 * pass 1 - process old C. and X. files
		 * pass 2 - process D.s that are going to X.
		 * pass 3 - processs D. mail and netnews and other D.s
		 * pass 4 - process other
		 */
		while ( (++pass < 5) && (subdir = opendir(f)) ) {

		    DEBUG(7, "Directory: %s\n", f);
		    while (gnamef(subdir, subf) == TRUE) {
			DEBUG(9, "file: %s\n", subf);
			FULLNAME(fullname, f, subf);
			switch (pass) {
			case 1:
			    if (EQUALSN(subf, "C.", 2)
			    &&  _age(fullname) >=_Cdays)
					cprocess(fullname);
			    else if (EQUALSN(subf, "C.", 2)
			    &&  _age(fullname) ==_Wdays)
					wprocess(f, subf);
			    else if (EQUALSN(subf, "TM.", 2)
			    &&  _age(fullname) >=_Tdays)
					tprocess(fullname);
			    else if (EQUALSN(subf, "X.", 2)
			    &&  _age(fullname) >=_Xdays)
					xprocess(fullname);
			    break;

			case 2:	/* pass 2 */
			    if (EQUALSN(subf, "D.", 2)
			    &&  _age(fullname) >= _Ddays
			    && dType(fullname) == D_XFILE)
					dXprocess(fullname);
			    break;

			case 3:	/* pass 3 */
			    if (EQUALSN(subf, "D.", 2)
			    &&  _age(fullname) >= _Ddays)
				switch (dType(fullname)) {
				case D_MAIL:
					dMprocess(f, subf);
					break;
				case D_NEWS:
					dNprocess(f, subf);
					break;
				case D_DATA:
					dDprocess(fullname);
					break;
				default:	/* do nothing */
					break;
				}
			    break;
			case 4:
			    if ( _age(fullname) >= _Odays
				 && !EQUALSN(subf, "C.", 2)
				 && !EQUALSN(subf, "D.", 2)
				 && !EQUALSN(subf, "T.", 2)
				 && !EQUALSN(subf, "X.", 2) )
					oprocess(fullname);
			    break;
			}
		    }
		    closedir(subdir);
		}
	    }
	}
	if ( *soptName && !got_one ) {
		fprintf( stderr, MSGSTR(MSG_UCL_27, 
			"uucleanup: Files for machine %s not found.\n"), 
			soptName);
		exit(1);
	}
	exit(0);
}

/* xprocess - X. file processing -- just remove the X. for now */

void
xprocess(fullname)
char *fullname;
{
	char text[BUFSIZ];

	DEBUG(5, "xprocess(%s), ", fullname);
	DEBUG(5, "unlink(%s)\n", fullname);
	(void) sprintf(text, "xprocess: unlink(%s)", fullname);
	errno = 0;
	(void) unlink(fullname);
	logit(text, errno);
}

/*
** tprocess - remove the old T.* files
**		This was added as an enhancement;  it already existed
**		as a PS/2ism. 
**
**		Actually, the same functionality could be attained simply
**		by removing temporary files as "other" files, but...
**
**					jkimble
*/

void
tprocess(fullname)
char *fullname;
{
	char text[BUFSIZ];

	DEBUG(5, "tprocess(%s), ", fullname);
	DEBUG(5, "unlink(%s)\n", fullname);
	(void) sprintf(text, "tprocess: unlink(%s)", fullname);
	errno = 0;
	(void) unlink(fullname);
	logit(text, errno);
}
/*
 * cprocess - Process old C. files
 *
 */

#define CMFMT  "\n\t%s!%s -> %s!%s   (Date %2.2d/%2.2d)\n\nCan't be executed."
#define XFMT  "\n\t%s!%s  (Date %2.2d/%2.2d)\n"
#define WFMT  "\n\t%s!%s -> %s!%s   (Date %2.2d/%2.2d)\n"
void
cprocess(fullname)
char *fullname;
{
	struct stat s;
	register struct tm *tp;
	char buf[BUFSIZ], user[9];
	char file1[BUFSIZ], file2[BUFSIZ], file3[BUFSIZ], type[2], opt[256];
	char text[BUFSIZ], text1[BUFSIZ], text2[BUFSIZ];
	FILE *fp;
	int ret;

	DEBUG(5, "cprocess(%s)\n", fullname);


	fp = fopen(fullname, "r");
	if (fp == NULL) {
		DEBUG(5, "Can't open file (%s), ", fullname);
		DEBUG(5, "errno=%d -- skip it!\n", errno);
		return;
	}
	if (fstat(fileno(fp), &s) != 0) {
	    /* can't happen; _age() did stat of this file and file is opened */
	    (void) fclose(fp);
	    return;
	}
	tp = localtime(&s.st_mtime);

	if (s.st_size == 0) { /* dummy C. for polling */
		DEBUG(5, "dummy C. -- unlink(%s)\n", fullname);
		(void) sprintf(text, MSGSTR(MSG_UCL_15, 
			"dDprocess: dummy C. unlinked(%s)"), fullname);
		errno = 0;
		(void) unlink(fullname);
		logit(text, errno);
		(void) fclose(fp);
		return;
	}

	/* Read the C. file and process it */
	while (fgets(buf, BUFSIZ, fp) != NULL) {
	    buf[strlen(buf)-1] = NULLCHAR; /* remove \n */
	    if (sscanf(buf,"%s%s%s%s%s%s", type, file1, file2,
	      user, opt, file3) <5) {
		(void) sprintf(text, MSGSTR(MSG_UCL_16,
		    "cprocess: Bad C. %s, unlink(%s)"), buf, fullname);
		break;
	    }

	    *text = NULLCHAR;
	    ret = 0;
	    /* fill in line 3 of text */
	    (void) sprintf(text2, MSGSTR(MSG_UCL_17,
		    "Job (%s) killed!\n"), BASENAME(fullname, '/')+2);
	    _CantContact[CANT2] = text2;
	    if (*type == 'S') {
	        if (EQUALSN(file1, "D.", 2))
	        /* generated file (mail/news) I think */
	        /* D. processing will return it later */
		    continue;

		/* some data was requested -- tell user */
		
		(void) sprintf(text1, CMFMT, Myname, file1, Rmtname, file2,
		    tp->tm_mon + 1, tp->tm_mday);
		_CantContact[CANT1] = text1;
		ret = sendMail((char *) NULL, user, "", _CantContact);
	    }
	    else if (*type == 'R') {
		(void) sprintf(text1, CMFMT, Rmtname, file1, Myname, file2,
		    tp->tm_mon + 1, tp->tm_mday);
		_CantContact[CANT1] = text1;
		ret = sendMail((char *) NULL, user, "", _CantContact);
	    }
	}

	if (!*text) {
	    (void) sprintf(text, MSGSTR(MSG_UCL_18,
		"cprocess: C. %s, mail returned (%d), unlink(%s)"),
		    buf, ret, fullname);
	}
	DEBUG(3, "text (%s)\n", text);

	errno = 0;
	(void) unlink(fullname);
	logit(text, errno);

	(void) fclose(fp);
	return;
}

/*
 * wprocess - send warning messages for C. == Wdays
 */

void
wprocess(dir, file)
char *dir, *file;
{
	struct stat s;
	register struct tm *tp;
	char fullname[BUFSIZ], xfile[BUFSIZ], xF_file[BUFSIZ];
	char buf[BUFSIZ], user[BUFSIZ];
	char file1[BUFSIZ], file2[BUFSIZ], file3[BUFSIZ], type[2], opt[256];
	char text[BUFSIZ], text1[BUFSIZ], text2[BUFSIZ];
	char *realuser, uline_m[NAMESIZE], uline_u[BUFSIZ], retaddr[BUFSIZ];
	FILE *fp, *xfp;
	int ret;

	FULLNAME(fullname, dir, file);
	DEBUG(5, "wprocess(%s)\n", fullname);

	fp = fopen(fullname, "r");
	if (fp == NULL) {
		DEBUG(4, "Can't open file (%s), ", fullname);
		DEBUG(4, "errno=%d -- skip it!\n", errno);
		return;
	}
	if (fstat(fileno(fp), &s) != 0) {
	    /* can't happen; _age() did stat of this file and file is opened */
	    (void) fclose(fp);
	    return;
	}
	tp = localtime(&s.st_mtime);

	if (s.st_size == 0) { /* dummy C. for polling */
		DEBUG(5, "dummy C. -- skip(%s)\n", fullname);
		(void) fclose(fp);
		return;
	}

	/* read C. and process it */
	while (fgets(buf, BUFSIZ, fp) != NULL) {
	    buf[strlen(buf)-1] = NULLCHAR; /* remove \n */
	    if (sscanf(buf,"%s%s%s%s%s%s", type, file1, file2,
	      user, opt, file3) <5) {
		DEBUG(5, "short line (%s): ", buf);
		DEBUG(5, "bad D. -- skip(%s)\n", fullname);
		(void) fclose(fp);
		return;
	    }

	    /* set up the 6th text line of the mail message */
	    (void) sprintf(text2,
		    "\n\tuustat -k%s\n", BASENAME(fullname, '/')+2);

	    _Warning[WARN2] = text2;
	    if (*type == 'S') {	/* Send type C. file processing */
		if (EQUALSN(file2, "X.", 2)) {
		    /* this is a uux job - tell user about it */
		    FULLNAME(xfile, dir, file1);
		    if ( (xfp = fopen(xfile, "r")) == NULL) {
			/* Can't read X. file ??? - just skip */
			DEBUG(3, "Can't read %s\n", xfile);
			break;
		    }
		    *retaddr = *uline_u = *uline_m = *text = NULLCHAR;
		    while (fgets(buf, BUFSIZ, xfp) != NULL) {
	    		buf[strlen(buf)-1] = NULLCHAR; /* remove \n */
			switch(*buf) {
			case 'F':	/* save the file name */
			    FULLNAME(xF_file, dir, &buf[2]);
			    break;
			case 'R':	/* save return address */
			    sscanf(buf+2, "%s", retaddr);
			    DEBUG(7, "retaddr (%s)\n", retaddr);
			    break;
			case 'U':	/* save machine, user */
			    sscanf(buf+2, "%s%s", uline_u, uline_m);
			    break;
			}
			if (buf[0] != 'C')
			    continue;
			realuser = uline_u;
			if (*retaddr != NULLCHAR)
			    realuser = retaddr;
			if (*realuser == NULLCHAR)
				strcpy(realuser, user);
			if (!EQUALS(uline_m, Myname))
			    sprintf(user, "%s!%s", uline_m, realuser);
			else
			    strcpy(user, realuser);
			
			(void) sprintf(text1, XFMT, Rmtname,
			    EQUALSN(&buf[2], "rmail", 5) ? &buf[3] :  &buf[2],
			    tp->tm_mon + 1, tp->tm_mday);
			_Warning[WARN1] = text1;
			if (EQUALSN(&buf[2], "rmail", 5) ) {
			    /* this is mail; append user mail (xF_file) */
		    ret = sendMail((char *) NULL, user, xF_file, _Warning);
		    }
			else {
			  ret = sendMail((char *) NULL, user, "", _Warning);
			}
			break;
		    }
		    (void) fclose(xfp);
		    break;
		}

	        if (EQUALSN(file1, "D.", 2))
	        /* generated file (mail/news) I think */
	        /* D. processing will return it later */
		    continue;

		/* some data was requested -- tell user */
		
		/* set up the 2nd text line of the mail message */
		(void) sprintf(text1, WFMT, Myname, file1, Rmtname, file2,
		    tp->tm_mon + 1, tp->tm_mday);
		_Warning[WARN1] = text1;
		ret = sendMail((char *) NULL, user, "", _Warning);
	    }

	    else if (*type == 'R') {	/* Receive C. file processing */
		if (EQUALSN(file1, "D.", 2) && EQUALSN(file2, "D.", 2))
		    continue;
		(void) sprintf(text1, WFMT, Rmtname, file1, Myname, file2,
		    tp->tm_mon + 1, tp->tm_mday);
		_Warning[WARN1] = text1;
		ret = sendMail((char *) NULL, user, "", _Warning);
	    }
	}	/* end while - read C. lines loop */

	(void) sprintf(text, MSGSTR(MSG_UCL_19,
		"wprocess: %s: %s, warning message sent to %s, returned (%d)"),
		    fullname, buf, user, ret);
	DEBUG(3, "text (%s)\n", text);

	logit(text, errno);

	(void) fclose(fp);
	return;
}
/*
 * oprocess - some unknown file just remove the file
 */

void
oprocess(fullname)
char *fullname;
{

	char text[BUFSIZ];

	DEBUG(5, "oprocess(%s), ", fullname);
	DEBUG(5, "unlink(%s)\n", fullname);
	(void) sprintf(text, "oprocess: unlink(%s)", fullname);
	errno = 0;
	(void) unlink(fullname);
	logit(text, errno);
}

/*
 * dDprocess - random D. file (not mail or rnews)
 *--just delete it for now
 */

void
dDprocess(fullname)
char *fullname;
{
	char text[BUFSIZ];

	DEBUG(5, "dDprocess(%s), ", fullname);
	DEBUG(5, "unlink(%s)\n", fullname);
	(void) sprintf(text, "dDprocess: unlink(%s)", fullname);
	errno = 0;
	(void) unlink(fullname);
	logit(text, errno);
}

/*
 * dXprocess - process D. files that are destined for X. on remote
 * --for now just delete it
 */

void
dXprocess(fullname)
char *fullname;
{
	char text[BUFSIZ];

	DEBUG(5, "dXprocess(%s), ", fullname);
	DEBUG(5, "  unlink(%s)\n", fullname);
	(void) sprintf(text, "dXprocess: unlink(%s)", fullname);
	errno = 0;
	(void) unlink(fullname);
	logit(text, errno);
}

/*
 * dMprocess - process ophan D. mail files
 * There are two types: ones generated locally and
 * others that are from remotes.  They can be identified
 * by the system name following the D.
 * Local ones have the local name.
 */

void
dMprocess(dir, file)
char *dir, *file;
{
	int ret;
	char fullname[MAXFULLNAME];
	char *toUser, *toSystem;
	char text[BUFSIZ];

	(void) sprintf(fullname, "%s/%s", dir, file);
	DEBUG(5, "dMprocess(%s)\n", fullname);


	if (PREFIX(_ShortLocal, &file[2])) {
	    DEBUG(5, "  Local file %s: ", file);
	}
	else {
	    DEBUG(5, "  Remote file %s: ", file);
	}
	if (toWho(fullname, &toUser, &toSystem)) {
	    DEBUG(5, "toUser %s, ", toUser);
	    DEBUG(5, "toSystem %s  ", toSystem);
	    ret = sendMail(toSystem, toUser, fullname, _Undeliverable);
	    DEBUG(5, "Mail sent, unlink(%s)\n", fullname);
	    (void) sprintf(text, MSGSTR(MSG_UCL_20,
		"dMprocess: mail %s to %s!%s, returned (%d),  unlink(%s)"),
		fullname, toSystem, toUser, ret, fullname);
	    errno = 0;
	    (void) unlink(fullname);
	    logit(text, errno);
	}
}

/*
 * dNprocess - process ophan D. netnews files
 * There are two types: ones generated locally and
 * others that are from remotes.  They can be identified
 * by the system name following the D.
 * Local ones have the local name.
 */

void
dNprocess(dir, file)
char *dir, *file;
{
	char fullname[MAXFULLNAME];
	char text[BUFSIZ];
	int ret;

	(void) sprintf(fullname, "%s/%s", dir, file);
	DEBUG(5, "dNprocess(%s)\n", fullname);


	if (PREFIX(_ShortLocal, &file[2])) {
	    /* just delete it, the C. is gone */
	    DEBUG(5, "  Local file %s, ", file);
	    DEBUG(5, "unlink(%s)\n", fullname);
	    (void) unlink(fullname);
	    (void) sprintf(text, MSGSTR(MSG_UCL_21, 
		"dNprocess: Local news item unlink(%s)"), fullname);
	    errno = 0;
	    (void) unlink(fullname);
	    logit(text, errno);
	}
	else {
	    /* execute rnews with this file - the X. is missing */
	    DEBUG(5, "  Remote file %s, ", file);
	    DEBUG(5, "exec rnews(%s), ", fullname);
	    ret = execRnews(fullname);
	    DEBUG(5, "unlink(%s)\n", fullname);
	    (void) sprintf(text, MSGSTR(MSG_UCL_22,
		"dNprocess: Remote - exec rnews %s: returned (%d), unlink(%s)"),
		fullname, ret, fullname);
	    errno = 0;
	    (void) unlink(fullname);
	    logit(text, errno);
	}

}



static long _sec_per_day = 86400L;

/*
 * _age - find the age of "file" in days
 * return:
 *	age of file
 *	0 - if stat fails
 */

int
_age(fullname)
char *fullname;
{
	static time_t ptime = 0;
	time_t time();
	struct stat stbuf;

	if (!ptime)
		(void) time(&ptime);
	if (stat(fullname, &stbuf) != -1) {
		return ((int)((ptime - stbuf.st_mtime)/_sec_per_day));
	}
	else
		return(0);
}

/*
 * dType - return the type of D. file
 * return:
 *	FAIL - can't read D. file
 *	D_MAIL - mail message D. file
 *	D_NEWS - netnews D. file
 *	D_DATA - other kind of D. file
 *	D_XFILE - destined for X. on destination machine
 */

/* NLINES - number of lines of D. file to read to determine type */
#define NLINES	10

int
dType(fullname)
char *fullname;
{
	char buf[BUFSIZ];
	FILE *fp;
	int i, type;

	fp = fopen(fullname, "r");
	if (fp == NULL) {
		DEBUG(4, "Can't open file (%s), ", fullname);
		DEBUG(4, "errno=%d -- skip it!\n", errno);
		return(FAIL);
	}
	type = D_DATA;

	/* read first NLINES lines to determine file type */
	for (i=0; i<NLINES; i++) {
	    DEBUG(9, "buf: %s\n", buf);
	    if (fgets(buf, BUFSIZ, fp) == NULL)
		break;	/* no more lines */
	    if (EQUALSN(buf, "From ", 5)) {
		type = D_MAIL;
		break;
	    }
	    if (EQUALSN(buf, "U ", 2)) {
		type = D_XFILE;
		break;
	    }
	    if (EQUALSN(buf, "Newsgroups: ", 12)) {
		type = D_NEWS;
		break;
	    }
	}

	(void) fclose(fp);
	return(type);
}

/*
 * sendMail - send mail file and message to user (local or remote)
 * return:
 *	the return from the pclose - mail exit status
 */

sendMail(system, user, file, mtext)
char *system, *user, *file;
char *mtext[];
{
	register FILE *fp, *fi;
	char cmd[BUFSIZ];

	DEBUG(5, "Mail %s to ", file);
	DEBUG(5, "%s\n", user);

	if (system != NULL && *system != '\0') {
		(void) sprintf(cmd, "%s mail %s!%s", PATH, system, user);
	}
	else {
		(void) sprintf(cmd, "%s mail %s", PATH, user);
	}

	DEBUG(7, "sendMail: %s\n", cmd);
	if ((fp = popen(cmd, "w")) == NULL)
		return(-errno);
	while (*mtext[0] )
	    (void) fprintf(fp, *mtext++, Rmtname);

	(void) fprintf(fp, MSGSTR(MSG_UCL_23, "\n\tSincerely,\n\t%s!uucp\n"),
		 Myname);
	(void) fprintf(fp,
		"\n#############################################\n");

	if (*file) {
	    if ((fi= fopen(file, "r")) == NULL) /* never happen;I read once */
		 return(pclose(fp));
	    (void) fprintf(fp,MSGSTR(MSG_UCL_24,
		"##### Data File: ############################\n"));
	    xfappend(fi, fp);
	    (void) fclose(fi);
	}
	return(pclose(fp));
}

/*
 * execRnews - execute rnews command with stdin file
 * return:
 *	the return from the pclose - rnews exit status
 */

execRnews(file)
char *file;
{
	register FILE *fp, *fi;
	char cmd[BUFSIZ];

	DEBUG(5, "Rnews %s\n", file);

	(void) sprintf(cmd, "%s rnews ", PATH);
	if ((fp = popen(cmd, "w")) == NULL)
		return(-errno);

	if ( (fi = fopen(file, "r")) == NULL) /* never happen - I read once */
		return(pclose(fp));
	xfappend(fi, fp);
	(void) fclose(fi);

	return(pclose(fp));
}

/*
 * toWho - figure out who to send this dead mail to
 *	It is a guess;
 *	If there is a local address, send it there.
 *	If not, send it back where it came from.
 * return:
 *	0 - could not find system and user information
 *	1 - found it
 */

int
toWho(file, user, system)
char *file;	/* the D. mail message file */
char **system;	/* pointer to the system name */
char **user;	/* pointer to the user name */
{
	char buf[BUFSIZ];
	FILE *fp;
	int i;
	static char fuser[BUFSIZ], fsystem[MAXBASENAME+1];  /* from first From */
	static char luser[BUFSIZ], lsystem[MAXBASENAME+1];  /* from other From */

	*fuser = NULLCHAR;
	DEBUG(5, "toWho(%s)\n", file);
	fp = fopen(file, "r");
	for (i=0; i<NLINES; i++) {
	    if (fgets(buf, BUFSIZ, fp) == NULL)
		break;	/* no more lines */
	    DEBUG(9, "buf: %s\n", buf);
	    if (!analFrom(buf, luser, lsystem))
		continue;
	    if ( !*fuser) {
		(void) strcpy(fuser, luser);
		(void) strcpy(fsystem, lsystem);
	    }
	    if (EQUALS(Myname, lsystem)) {
		*user = luser;
		*system = lsystem;
		(void) fclose(fp);
		return(1);
	    }
	}

	/* could not find local user - use first line */
	(void) fclose(fp);
	if (!*fuser)	/* didn't find all information */
	    return(0);
	*user = fuser;
	*system = fsystem;
	return(1);
}

/* analFrom - analyze From line
 * return:
 *	0 - didn't find both from and remote from info
 *	1 - found info.
 */

int
analFrom(line, user, system)
char *line, *user, *system;
{
	char *s;
	int i;

	if (!PREFIX("From ", line) && !PREFIX(">From ", line))
	    return(0);
	
	s = (char *) strchr(line, ' ') + 1;
	for (i = 0;  *s && *s != ' ' && *s != '\n'; i++)
	    user[i] = *s++;
	user[i] = NULLCHAR;

	/* look for "remote from" */
	while (*s && ((s = (char *) strchr(s, ' ')) != NULL)) {
	    s++;
	    if (PREFIX("remote from ", s)) {	/* found it */
		s = s + strlen("remote from ");
		for (i = 0; (i<MAXBASENAME) && *s && *s != ' ' && *s != '\n'; i++)
		    system[i] = *s++;
		system[i] = NULLCHAR;
		return(1);
	    }
	}
	return(0);
}



static FILE	*_Lf = NULL;

/*
 * Make log entry
 *	text	-> ptr to text string
 *	status	errno number
 * Returns:
 *	none
 */

void
logit(text, status)
register char	*text;
int status;
{

	if (Nstat.t_pid == 0)
		Nstat.t_pid = getpid();

	if (_Lf == NULL) {
		_Lf = fopen(Logfile, "a");
		(void) chmod(Logfile, 0666);
		if (_Lf == NULL)
			return;
		setbuf(_Lf, CNULL);
	}
	(void) fseek(_Lf, 0L, 2);
	(void) fprintf(_Lf, "%s ", Rmtname);
	(void) fprintf(_Lf, "(%s,%d,%d) ", timeStamp(), Nstat.t_pid, Seqn);
	(void) fprintf(_Lf, "%s (%d)\n", text, status);
	return;
}


init_msg()
{
_Undeliverable[0] = MSGSTR(MSG_UUCLEAN1, "Subject: Undeliverable Mail\n");
_Undeliverable[1]=MSGSTR(MSG_UUCLEAN2,"This mail message is undeliverable.\n");
_Undeliverable[2] =MSGSTR(MSG_UUCLEAN3, "(Probably to or from system '%s')\n");
_Undeliverable[3] = MSGSTR(MSG_UUCLEAN4, "It was sent to you or by you.\n");
_Undeliverable[4] = MSGSTR(MSG_UUCLEAN5, "Sorry for the inconvenience.\n");

_CantContact[0] = MSGSTR(MSG_UUCLEAN6, "Subject: Job Killed By uucp\n");
_CantContact[1] = MSGSTR(MSG_UUCLEAN7, "We can't contact machine '%s'.\n");
_CantContact[2] = " ";	/* uucleanup will fill in variable text here */
_CantContact[3] = " ";	/* fill in jobid of killed job */
_CantContact[4] = "";	

_Warning[0] = MSGSTR(MSG_UUCLEAN8, "Subject: Warning From uucp\n");
_Warning[1] = MSGSTR(MSG_UUCLEAN9, "We have been unable to contact machine '%s' since you queued your job.\n");
_Warning[2] = " ";	/*  wprocess FILLS IN THIS LINE OF TEXT */
_Warning[3] = MSGSTR(MSG_UUCLEAN10, "The job will be deleted in several days if the problem is not corrected.\n");
_Warning[4] = MSGSTR(MSG_UUCLEAN11, "If you care to kill the job); execute the following command:\n");
_Warning[5] = " ";	/*  wprocess FILLS IN THIS LINE WITH:  uustat -kJOBid */
_Warning[6] = " ";	/* FILL IN THE -m STRING IF SPECIFIED */
_Warning[7] = "";
}
