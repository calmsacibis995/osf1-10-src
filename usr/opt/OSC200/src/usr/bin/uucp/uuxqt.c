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
static char rcsid[] = "@(#)$RCSfile: uuxqt.c,v $ $Revision: 4.3.6.3 $ (DEC) $Date: 1993/10/11 19:36:10 $";
#endif
/* 
 * COMPONENT_NAME: UUCP uuxqt.c
 * 
 * FUNCTIONS: APPCMD, APPCMDNS, Muuxqt, chkFile, chkpart, cleanup, 
 *            cuantos, doFileChk, gotfiles, gt_Xfile, mv_Xfiles, 
 *            onintr, retosndr, rm_Xfiles, unmv_Xfiles, uucpst, 
 *            xcompare, xget, xprocess, xsort, xstash 
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
1.12  uuxqt.c, bos320 10/18/90 13:16:33";
*/

#include "uucp.h"
/* VERSION( uuxqt.c	5.3 -  -  ); */

/*
 * execute commands set up by a uux command,
 * usually from a remote machine - set by uucp.
 */

#ifndef	V7
#define LOGNAME	"LOGNAME=uucp"
#else
#define LOGNAME	"USER=uucp"
#endif

#define C_COMMAND	1
#define C_FILE		2
#define BAD_COMMAND	1
#define BAD_FILE	2
#define USAGE	MSGSTR(MSG_UUXQT_1, "[-xDEBUG] [-sSYSTEM]")
#define APPCMD(p)	{(void) strcat(_Cmd, p); (void) strcat(_Cmd, " ");}
#define APPCMDNS(p)	{(void) strcat(_Cmd, p);}

char	_Xfile[MAXFULLNAME];
char	_Cmd[2 * BUFSIZ];	/* build up command buffer */
int	_CargType;		/* argument type of next C argument */

static void retosndr(), uucpst();
static xprocess();
static int chkFile();
static int doFileChk();

main(argc, argv, envp)
char	*argv[], *envp[];
{
	DIR	 *fp1;
	int	ret, maxnumb;
#ifdef PDA
	char	dirname[NAME_MAX], lockname[NAME_MAX];
#else
	char	dirname[MAXFULLNAME], lockname[MAXFULLNAME];
#endif
	extern	onintr();
	FILE	*fp;
	char	lineBuf[BUFSIZ];

	(void) signal(SIGILL, (void(*)(int)) onintr);
	(void) signal(SIGTRAP, (void(*)(int)) onintr);
	(void) signal(SIGIOT, (void(*)(int)) onintr);
	(void) signal(SIGFPE, (void(*)(int)) onintr);
	(void) signal(SIGBUS, (void(*)(int)) onintr);
	(void) signal(SIGSEGV, (void(*)(int)) onintr);
	(void) signal(SIGSYS, (void(*)(int)) onintr);
	(void) signal(SIGPIPE, (void(*)(int)) onintr);
	(void) signal(SIGTERM, SIG_IGN);

	setlocale(LC_ALL,"");
	catd = catopen(MF_UUCP, NL_CAT_LOCALE);

	/* choose LOGFILE */
	(void) strcpy(Logfile, LOGUUXQT);

	/*
	 * get local system name
	 */
	Env = envp;
	Nstat.t_qtime = time((time_t *)0);
	(void) strcpy(Progname, "uuxqt");
	Pchar = 'Q';
	uucpname(Myname);
	Ofn = 1;
	Ifn = 0;
	dirname[0] = NULLCHAR;
	while ((ret = getopt(argc, argv, "s:x:")) != EOF) {
		switch(ret){

		/*
		 * debugging level
		 */
		case 'x':
			Debug = atoi(optarg);
			if (Debug <= 0)
				Debug = 1;
			break;

		case 's':
			/* fake out uuxqt and use the argument as if
			 * it were the spool directory for the purpose
			 * of determining what subdirectories to search
			 *  EX: mkdir /tmp/foo; touch /tmp/foo/{baz,gorp}
			 *      uuxqt -s/tmp/foo
			 * this will cause uuxqt to only run on the sub
			 * baz and gorp in the Spool directory.  Trust me.
			 */
			(void) strncpy(dirname, optarg, MAXFULLNAME);
			break;

		default:
			(void) fprintf(stderr, MSGSTR(MSG_UUXQT_2,
				"\tusage: %s %s\n"), Progname, USAGE);
			exit(1);
		}
	}
	if (argc != optind) {
		(void) fprintf(stderr, MSGSTR(MSG_UUXQT_3,
			"\tusage: %s %s\n"), Progname, USAGE);
		exit(1);
	}

	DEBUG(4, "\n\n** START **\n", "");
	maxnumb = 1;		/* If there's no ctl file, or we can't read it,
				   we'll allow 1 incarnation. */
	fp = fopen(LMTUUXQT, "r");
	if (fp == NULL) {
		DEBUG(1, "No limitfile - %s\n", LMTUUXQT);
	} else {
		while (fgets(lineBuf, BUFSIZ, fp))
			if (*lineBuf != '#')
				if (sscanf(lineBuf, "%d", &maxnumb) == 1)
					break;
		(void) fclose(fp);
		DEBUG(4, "Uuxqt limit %d -- ", maxnumb);
		ret = cuantos(X_LOCKPRE, X_LOCKDIR);
		DEBUG(4, "found %d -- ", ret);
		if (maxnumb >= 0 && ret >= maxnumb) {
			DEBUG(4, "exiting\n", maxnumb);
			exit(0);
		}
		DEBUG(4, "continuing\n", maxnumb);
	}

	/*
	 * determine user who started uuxqt (in principle)
	 */
	strcpy(User, "uucp");	/* in case all else fails (can't happen) */
	Uid = getuid();
	Euid = geteuid();	/* this should be UUCPUID */
	guinfo(Euid, User);
	setuucp(User);
	DEBUG(4, "User - %s\n", User);
	guinfo(Uid, Loginuser);

	DEBUG(4, "process %s\n", "");

	fp1 = opendir(Spool);
	ASSERT(fp1 != NULL, Ct_OPEN, Spool, errno);

	if (dirname[0] != NULLCHAR) {
	    (void) sprintf(lockname, "%s.%s", X_LOCK, dirname);
	    if (ulockf(lockname, (time_t)  X_LOCKTIME) == 0) {
		xprocess(dirname);
	    	rmlock(CNULL);
		clrlock(CNULL);
	    }
	}
	else {
	    while (gdirf(fp1, dirname, Spool) == TRUE) {
	        (void) sprintf(lockname, "%s.%s", X_LOCK, dirname);
	        if (ulockf(lockname, (time_t)  X_LOCKTIME) != 0)
	    	    continue;
		xprocess(dirname);
	        rmlock(CNULL);
		clrlock(CNULL);
	    }
	 }

	closedir(fp1);
	cleanup(0);
}

cleanup(code)
int	code;
{
	rmlock(CNULL);
	clrlock(CNULL);
	catclose(catd);

	exit(code);
}

/*
 * catch signal then cleanup and exit
 */
onintr(inter)
register int	inter;
{
	char	str[30];
	(void) signal(inter, SIG_IGN);
	(void) sprintf(str, "QSIGNAL %d", inter);
	logent(str, "QCAUGHT");
	cleanup(-inter);
}

#define XCACHESIZE (4096 / (MAXBASENAME + 1))
static char	xcache[XCACHESIZE][MAXBASENAME + 1];	/* cache for X. files */
static int	xcachesize = 0;			/* how many left? */

/*
 * stash an X. file so we can process them sorted first by grade, then by
 * sequence number
 */
static int
xstash(file)
char	*file;
{
	DEBUG(4, "stashing %s\n", file);
	strcpy(xcache[xcachesize++], file);
}

/*
 * xcompare
 *	comparison routine for for qsort()
 */
static int
xcompare(f1, f2)
register char	*f1, *f2;
{
	/* assumes file name is X.siteG1234 */
	/* use -strcmp() so that xstash is sorted largest first */
	/* pull files out of the stash from largest index to smallest */

	return(-strcmp(f1 + strlen(f1) - 5, f2 + strlen(f2) - 5));
}
	
/*
 * xsort
 *	sort the cached X. files,
 *	largest (last) to smallest (next to be processed)
 */
static int
xsort()
{
	DEBUG(4, "xsort:  first was %s\n", xcache[0]);
	qsort(xcache, xcachesize, MAXBASENAME + 1, xcompare);
	DEBUG(4, "xsort:  first is %s\n", xcache[0]);
}

/*
 * xget
 *	return smallest X. file in cache
 *	(hint:  it's the last one in the array)
 */
static int
xget(file)
char	*file;
{
	if (xcachesize > 0) {
		strcpy(file, xcache[--xcachesize]);
		DEBUG(4, "xget: returning %s\n", file);
		return(1);
	} else {
		/* avoid horror of xcachesize < 0 (impossible, you say?)! */
		xcachesize = 0;
		return(0);
	}
}


/*
 * get a file to execute
 *	file	-> a read to return filename in
 * returns:
 *	0	-> no file
 *	1	-> file to execute
 */
gt_Xfile(file, dir)
register char	*file, *dir;
{
	DIR *pdir;

	if (xcachesize == 0) {
		/* open spool directory */
		pdir = opendir(dir);
		/* this was an ASSERT, but it's not so bad as all that */
		if (pdir == NULL)
			return(0);

		/* scan spool directory looking for X. files to stash */
		while (gnamef(pdir, file) == TRUE) {
			DEBUG(4, "gt_Xfile got %s\n", file);
			/* look for x prefix */
			if (file[0] != XQTPRE)
				continue;

			/* check to see if required files have arrived */
			if (gotfiles(file))
				xstash(file);
			if (xcachesize >= XCACHESIZE)
				break;
		}
		closedir(pdir);
		xsort();
	}

	return(xget(file));
}

/*
 * check for needed files
 *	file	-> name of file to check
 * return: 
 *	0	-> not ready
 *	1	-> all files ready
 */
gotfiles(file)
register char	*file;
{
	register FILE *fp;
	struct stat stbuf;
	char	buf[BUFSIZ], rqfile[MAXFULLNAME];

	fp = fopen(file, "r");
	if (fp == NULL)
		return(FALSE);

	while (fgets(buf, BUFSIZ, fp) != NULL) {
		DEBUG(4, "%s\n", buf);

		/*
		 * look at required files
		 */
		if (buf[0] != X_RQDFILE)
			continue;
		(void) sscanf(&buf[1], "%s", rqfile);

		/*
		 * expand file name 
		 */
		expfile(rqfile);

		/*
		 * see if file exists
		 */
		if (stat(rqfile, &stbuf) == -1) {
			fclose(fp);
			return(FALSE);
		}
	}

	fclose(fp);
	return(TRUE);
}

/*
 * remove execute files to x-directory
 *
 * _Xfile is a global
 * return:
 *	none
 */
rm_Xfiles()
{
	register FILE *fp;
	char	buf[BUFSIZ], file[NAMESIZE], tfile[NAMESIZE];
	char	tfull[MAXFULLNAME];

	if ((fp = fopen(_Xfile, "r")) == NULL) {
		DEBUG(4, "rm_Xfiles: can't read %s\n", _Xfile);
		return;
	}

	/*
	 * (void) unlink each file belonging to job
	 */
	while (fgets(buf, BUFSIZ, fp) != NULL) {
		if (buf[0] != X_RQDFILE)
			continue;
		if (sscanf(&buf[1], "%s%s", file, tfile) < 2)
			continue;
		(void) sprintf(tfull, "%s/%s", XQTDIR, tfile);
		(void) unlink(tfull);
	}
	fclose(fp);
	return;
}

/*
 * move execute files to x-directory
 *	_Xfile is a global
 * return: 
 *	none
 */
mv_Xfiles()
{
	register FILE *fp;
	char	buf[BUFSIZ], ffile[MAXNAMESIZE], tfile[MAXNAMESIZE];
	char	tfull[MAXNAMESIZE];

	if ((fp = fopen(_Xfile, "r")) == NULL) {
		DEBUG(4, "mv_Xfiles: can't read %s\n", _Xfile);
		return;
	}

	while (fgets(buf, BUFSIZ, fp) != NULL) {
		if (buf[0] != X_RQDFILE)
			continue;
		if (sscanf(&buf[1], "%s%s", ffile, tfile) < 2)
			continue;

		/*
		 * expand file names and move to
		 * execute directory
		 * Make files readable by anyone
		 */
		expfile(ffile);
		(void) sprintf(tfull, "%s/%s", XQTDIR, tfile);
		
		ASSERT(xmv(ffile, tfull) == 0, "XMV ERROR", tfull, errno);
		chmod(tfull, 0666);
	}
	fclose(fp);
	return;
}

/*
 * undo what mv_Xfiles did
 *	_Xfile is a global
 * return: 
 *	none
 */
unmv_Xfiles()
{
	FILE *fp;
	char	buf[BUFSIZ], ffile[MAXNAMESIZE], tfile[MAXNAMESIZE];
	char	tfull[MAXNAMESIZE], ffull[MAXNAMESIZE], xfull[MAXNAMESIZE];

	(void) sprintf(xfull, "%s/%s", RemSpool, _Xfile);
	if ((fp = fopen(xfull, "r")) == NULL) {
		DEBUG(4, "unmv_Xfiles: can't read %s\n", xfull);
		return;
	}

	while (fgets(buf, BUFSIZ, fp) != NULL) {
		if (buf[0] != X_RQDFILE)
			continue;
		if (sscanf(&buf[1], "%s%s", ffile, tfile) < 2)
			continue;

		/*
		 * expand file names and move back to
		 * spool directory
		 * Make files readable by uucp
		 */
		(void) sprintf(ffull, "%s/%s", RemSpool, ffile);
		/* i know we're in .Xqtdir, but what the hell ... */
		(void) sprintf(tfull, "%s/%s", XQTDIR, tfile);
		
		ASSERT(xmv(tfull, ffull) == 0, "XMV ERROR", ffull, errno);
		(void) chmod(ffull, 0600);
	}
	fclose(fp);
	return;
}

/* chkpart - checks the string (ptr points to it) for illegal command or
 *  file permission restriction - called recursively
 *  to check lines that have `string` or (string) form.
 *  _Cmd is the buffer where the command is built up.
 *  _CargType is the type of the next C line argument
 *
 * Return:
 *	BAD_FILE if a non permitted file is found
 *	BAD_COMMAND if non permitted command is found
 *	0 - ok
 */

static
int
chkpart(ptr)
char *ptr;
{
	char	prm[BUFSIZ], xcmd[BUFSIZ];
	char	savechar[2]; /* one character string with NULL */
	int	ret;

	/* _CargType is the arg type for this iteration (cmd or file) */
	while ((ptr = getprm(ptr, prm)) != NULL) {
	    DEBUG(4, "prm='%s'\n", prm);
	    switch(*prm) {

	    /* End of command delimiter */
	    case ';':
	    case '^':
	    case '&':
	    case '|':
	        APPCMDNS(prm);
		_CargType = C_COMMAND;
		continue;

	    /* Other delimiter */
	    case '>':
	    case '<':
	        APPCMDNS(prm);
		continue;

	    case '`':	/* don't allow any ` commands */
	    case '\\':
		 return(BAD_COMMAND);

	    /* Some allowable quoted string */
	    case '(':
	    case '"':
	    case '\'':
	        /* must recurse */
		savechar[0] = *prm;
		savechar[1] = NULLCHAR;
		APPCMD(savechar);	/* put first char into command */
		savechar[0] = prm[strlen(prm)-1];
		prm[strlen(prm)-1] = NULLCHAR; /* delete last character */

		/* recurse */
		if (ret = chkpart(prm+1)) { /* failed */
		    return(ret);
		}
		APPCMD(savechar);	/* put last char into command */
		continue;

	    default:	/* check for command or file */
		break;
	    }

	    if (_CargType == C_COMMAND) {
		if ( (cmdOK(prm, xcmd)) == FALSE)
		    return(BAD_COMMAND);
		APPCMD(xcmd);
		_CargType = C_FILE;
		continue;
	    }

	    if (chkFile(prm))
		return(BAD_FILE);
	    else
		APPCMD(prm);
   	}
	return(0);	/* all ok */
}

/* chkFile - try to find a path name in the prm.
 * 	if found, check it for access permission.
 *
 * check file access permissions
 * if ! in name assume that access on local machine is required
 *
 * Return:
 *	BAD_FILE - not permitted
 *	0 - ok
 */

static
int
chkFile(prm)
char *prm;
{
	char	*p, buf[BUFSIZ];

	(void) strcpy(buf, prm);
	switch(*prm) {
	case '~':
	case '/':
	    if (doFileChk(buf)) 
		return(BAD_FILE);
	    else
	        return(0);
	    /*NOTREACHED*/

	case '!':
	    return(chkFile(buf+1));
	    /*NOTREACHED*/

	default:
	    break;
	}

	if ( (p =(char*)strchr(buf, '!')) == NULL) {  /* no "!", look for "/" */
	    if ( (p = (char *)strchr(buf, '/')) == NULL) {  /* ok */
		return(0);
	    }
	    if (doFileChk(p)) 
		return(BAD_FILE);
	    else
	        return(0);
	}

	/* there is at least one '!' - see if it refers to my system */
	if (PREFIX(Myname, buf))  /*  my system so far, check further */
	    return(chkFile(p+1));  /* recurse with thing after '!' */
	else		/* not my system - not my worry */
	    return(0);
}

/* doFileChk - check file path permission
 * NOTE: file is assumed to be a buffer that expfile an 
 *  write into.
 * Return
 *	BAD_FILE - not allowed
 *	0 - ok
 */

static
int
doFileChk(file)
char *file;
{
	    expfile(file);
	    DEBUG(7, "fullname: %s\n", file);
	    if (chkpth(file, CK_READ) == FAIL
	      || chkpth(file, CK_WRITE) == FAIL )
		return(BAD_FILE);
	    else
		return(0);
}


/*
 * return stuff to user
 *	user	-> user to notify
 *	rmt	-> system name where user resides
 *	file	-> file to return (generally contains input)
 *	cmd	-> command that was to be executed
 *	buf	-> user friendly face saving uplifting edifying missive
 *	errfile	-> stderr output from cmd xeqn
 * return:
 *	none
 */
static void
retosndr(user, rmt, file, cmd, buf, errfile)
char	*user, *rmt, *file, *cmd, *buf, *errfile;
{
	char	ruser[BUFSIZ], msg[BUFSIZ];

	(void) sprintf(msg,
	    "remote execution\t[uucp job %s (%s)]\n\t%s\n%s\n",
	    &_Xfile[2], timeStamp(), cmd, buf);

	DEBUG(5, "retosndr %s, ", msg);

	if (EQUALS(rmt, Myname)) 
		(void) strcpy(ruser, user);
	else
		(void) sprintf(ruser, "%s!%s", rmt, user);

        mailst(ruser, msg, file, errfile);
}


/*
 * uucpst - send the status message back using a uucp command
 * NOTE - this would be better if the file could be appended.
 * - suggestion for the future - if rmail would take a file name
 * instead of just person, then that facility would be correct,
 * and this routine would not be needed.
 */

static
void
uucpst(rmt, tofile, errfile, cmd, buf)
char	*rmt, *tofile, *errfile, *cmd, *buf;
{
	char	arg[MAXFULLNAME], tmp[MAXBASENAME], msg[BUFSIZ];
	int ret;
	FILE *fp, *fi;

	(void) sprintf(msg,
	    "uucp job %s (%s) remote execution\n\t%s\n%s\n",
	    &_Xfile[2], timeStamp(), cmd, buf);

	(void) sprintf(tmp, "%s.%d", rmt, getpid());
	if ((fp = fopen(tmp, "w")) == NULL)
		return;
	(void) fprintf(fp, "%s\n", msg);

	/* copy back stderr */
	if (*errfile != '\0' && NOTEMPTY(errfile)
	   && (fi = fopen(errfile, "r")) != NULL) {
		fputs("\n\t===== stderr was =====\n", fp);
		if (xfappend(fi, fp) != SUCCESS)
			fputs("\n\t===== well, i tried =====\n", fp);
		(void) fclose(fi);
		fputc('\n', fp);
	}


	(void) fclose(fp);
	(void) sprintf(arg, "%s!%s", rmt, tofile);

	/* start uucp */

	if (vfork() == 0) {
		(void) close(0);
		(void) close(1);
		(void) close(2);
		(void) open("/dev/null", 2);
		(void) open("/dev/null", 2);
		(void) open("/dev/null", 2);
		(void) signal(SIGINT, SIG_IGN);
		(void) signal(SIGHUP, SIG_IGN);
		(void) signal(SIGQUIT, SIG_IGN);
		closelog();
	
		(void) execle("/usr/bin/uucp", "UUCP",
		    "-C", tmp, arg, 0, Env);
		(void) _exit(100);
	}

	(void) wait(&ret);
	(void) unlink(tmp);
	return;
}

static
xprocess(dirname)
char *dirname;
{
    int		return_stdin;	/* return stdin for failed commands */
    int		cmdok, mask, ret, badfiles;
    int		send_zero;	/* return successful completion status */
    int		send_nonzero;	/* return unsuccessful completion status */
    int		send_nothing;	/* request for no exit status */
    int		store_status;	/* store status of command in local file */
    char	lbuf[BUFSIZ];
    char	*p;
    char	dfile[MAXFULLNAME], cfile[MAXFULLNAME], incmd[BUFSIZ];
    char	errDfile[BUFSIZ];
    char	fin[MAXFULLNAME], sysout[NAMESIZE], fout[MAXFULLNAME];
    char	file[MAXNAMESIZE], tempname[NAMESIZE];
    char	_Sfile[MAXFULLNAME];	/* name of local file for status */
    FILE	*xfp, *dfp, *fp, *errdfp;
    struct	stat sb;
    char	buf[BUFSIZ], user[BUFSIZ], retaddr[BUFSIZ], msgbuf[BUFSIZ];

    (void) strcpy(Rmtname, dirname);
    chremdir(Rmtname);
    (void) mchFind(Rmtname);
    while (gt_Xfile(_Xfile, RemSpool) > 0) {
	DEBUG(4, "_Xfile - %s\n", _Xfile);

	if ( (xfp = fopen(_Xfile, "r")) == NULL) {
	   toCorrupt(_Xfile);
	   continue;
	}
	ASSERT(xfp != NULL, Ct_OPEN, _Xfile, errno);

	if (stat(_Xfile, &sb) != -1)
	    Nstat.t_qtime = sb.st_mtime;
	/*
	 * initialize to defaults
	 */
	(void) strcpy(user, User);
	(void) strcpy(fin, "/dev/null");
	(void) strcpy(fout, "/dev/null");
	(void) sprintf(sysout, "%.*s", MAXBASENAME, Myname);
	badfiles = 0;
	*incmd = *retaddr = NULLCHAR;
	(void) initSeq();
	send_zero = send_nonzero = send_nothing = store_status = return_stdin = 0;

	while (fgets(buf, BUFSIZ, xfp) != NULL) {
	    /*
	     * interpret JCL card
	     */
	    switch (buf[0]) {
		case X_USER:	 /* user name */
			    /* ignore Rmtname -- i don't believe it */
			    (void) sscanf(&buf[1], "%s", user);
			    break;

		case X_STDIN:	/* standard input */
			    (void) sscanf(&buf[1], "%s", fin);
			    expfile(fin);
			    if (chkpth(fin, CK_READ)) {
				DEBUG(4, "badfile - in: %s\n", fin);
				badfiles = 1;
			    }
			    break;

		case X_STDOUT:	/* standard output */
			    (void) sscanf(&buf[1], "%s%s", fout, sysout);
			    if ((p = strpbrk(sysout, "!/")) != NULL)
				*p = NULLCHAR;	/* these can hurt! */
			    if (*sysout != NULLCHAR && !EQUALS(sysout, Myname))
				  break;

			    expfile(fout);
			    if (chkpth(fout, CK_WRITE)) {
				badfiles = 1;
				DEBUG(4, "badfile - out: %s\n", fout);
			    }
			    break;


		case X_CMD:	/* command to execute */
			    (void) strcpy(incmd, &buf[2]);
			    if (*(incmd + strlen(incmd) - 1) == '\n')
				*(incmd + strlen(incmd) - 1) = NULLCHAR;
			    break;

		case X_MAILF:	/* put status in _Sfile */
			    store_status = 1;
			    (void) sscanf(&buf[1], "%s", _Sfile);
			    break;

		case X_SENDNOTHING:	/* no failure notification */
			    send_nothing++;
			    break;
	
		case X_SENDZERO:	/* success notification */
			    send_zero++;
			    break;
	
		case X_NONZERO:	/* failure notification */
			    send_nonzero++;
			    break;
	
		case X_BRINGBACK:  /* return stdin on command failure */
			    return_stdin = 1;
			    break;


		case X_RETADDR:	/* return address */
			    (void) sscanf(&buf[1], "%s", retaddr);
			    break;

		default:
			    break;
	    }
	}

	fclose(xfp);
	DEBUG(4, "fin - %s, ", fin);
	DEBUG(4, "fout - %s, ", fout);
	DEBUG(4, "sysout - %s, ", sysout);
	DEBUG(4, "user - %s\n", user);
	DEBUG(4, "incmd - %s\n", incmd);

	if (retaddr[0] != NULLCHAR)
	    (void) strcpy(user, retaddr);	/* pick on this guy */

	/* get rid of stuff that can hurt */
	if ( (p = strpbrk(user, ";&|<>^`\\('\"{}\n")) != NULL) {
	    *p = NULLCHAR;
	}

	if (incmd[0] == NULLCHAR) {
	    /* this is a bad X. file - just get rid of it */
	    toCorrupt(_Xfile);
	    continue;
	}

	/*
	 * send_nothing must be explicitly requested to avert failure status
	 * send_zero must be explicitly requested for success notification
	 */
	if (!send_nothing)
		send_nonzero++;
		
	/*
	 * command execution
	 * generate a temporary file (if necessary)
	 * to hold output to be shipped back
	 */
	if (EQUALS(fout, "/dev/null"))
	    (void) strcpy(dfile, "/dev/null");
	else {
	    gename(DATAPRE, sysout, 'O', tempname);
	    (void) sprintf(dfile, "%s/%s", WORKSPACE, tempname);
	}

	/* initialize command line */
	/* set up two environment variables, remote machine name */
	/* and remote user name if available from R line */
/*
	(void) sprintf(_Cmd,
	   "%s %s UU_MACHINE=%s UU_USER=%s export UU_MACHINE UU_USER PATH; ",
	    PATH, LOGNAME, Rmtname, user);
*/
	(void) sprintf(_Cmd,
	   "%s UU_MACHINE=%s UU_USER=%s export UU_MACHINE UU_USER PATH; ",
	    PATH, Rmtname, user);

	/*
	 * check to see if command can be executed
	 */
	DEBUG(9, "incmd = %s\n", incmd);
	_CargType = C_COMMAND;	/* the first thing is a command */
	cmdok = chkpart(incmd);
	DEBUG(9, "_Cmd = %s\n", _Cmd);

	if (badfiles || (cmdok == BAD_COMMAND) || cmdok == BAD_FILE) {
	    if (cmdok == BAD_COMMAND) {
		(void) sprintf(lbuf, MSGSTR(MSG_UUXQT_4, "%s XQT DENIED"), 
			user);
		(void) sprintf(msgbuf, MSGSTR(MSG_UUXQT_5, 
			"execution permission denied to %s"), user);
	    } else {
		(void) sprintf(lbuf, MSGSTR(MSG_UUXQT_6,
		    "%s XQT-  STDIN/STDOUT/FILE ACCESS DENIED"), user);
		(void) sprintf(msgbuf, MSGSTR(MSG_UUXQT_7,
			"file access denied to %s"), user);
	    }
	    logent(incmd, lbuf);
	    DEBUG(4, "bad command %s\n", incmd);

	    if (send_nonzero)
		retosndr(user, Rmtname, return_stdin ? fin : "", incmd, msgbuf, "");
	    if (store_status) 
		    uucpst(Rmtname, _Sfile, "", incmd, msgbuf);
	    goto rmfiles;
	}

	(void) sprintf(lbuf, "%s XQT", user);
	logent(_Cmd, lbuf);
	DEBUG(4, "cmd %s\n", _Cmd);

      /* move files to execute directory and change to that directory */

	mv_Xfiles();
	
	ASSERT(chdir(XQTDIR) == 0, Ct_CHDIR, XQTDIR, errno);
      
      /* invoke shell to execute command */

	mask = umask(0);
	DEBUG(7, "full cmd: %s\n", _Cmd);

	/* temp file to capture error output */
	gename(DATAPRE, sysout, 'E', tempname);
	(void) sprintf(errDfile, "%s/%s", WORKSPACE, tempname);
	ret = shio(_Cmd, fin, dfile, errDfile);
	umask(mask);
	if (ret == -1) {	/* -1 means the fork() failed */
		unmv_Xfiles();	/* put things back */
		errent(Ct_FORK, buf, errno, rcsid, __FILE__, __LINE__);
		cleanup(1);	/* g'nite! */
	}

	if (ret == 0) {				/* exit == signal == 0 */
	    (void) strcpy(msgbuf, "exited normally");	
	    if (send_zero)
		retosndr(user, Rmtname, "", incmd, msgbuf, "");
	    if (store_status)
		uucpst(Rmtname, _Sfile, "", incmd, msgbuf);
	} else {				/* exit != 0 */
	    int exitcode = (ret >> 8) & 0377;

	    if (exitcode)	/* exit != 0 */
	    	(void) sprintf(msgbuf, MSGSTR(MSG_UUXQT_8,
			"exited with status %d"), exitcode);
	    else		/* signal != 0 */
	    	(void) sprintf(msgbuf, MSGSTR(MSG_UUXQT_9,
			"terminated by signal %d"), ret & 0177);
	    DEBUG(5, "%s\n", msgbuf);

	    if (send_nonzero)
		retosndr(user, Rmtname, return_stdin ? fin : "", incmd, msgbuf, errDfile);
	    if (store_status)
		uucpst(Rmtname, _Sfile, errDfile, incmd, msgbuf);

	    (void) sprintf(lbuf, "%s - %s", incmd, msgbuf);
	    logent(lbuf, MSGSTR(MSG_UUXQT_10, "COMMAND FAIL"));
	}

      /* change back to spool directory */

	chremdir(Rmtname);

      /* remove file */

	rm_Xfiles();

	if (ret != 0) {
	    /*
	     * exit status not zero,
	     * so append bad news message and stderr to returned data
	     */
	    if (access(dfile, 0) != 0)
		close(creat(dfile, DFILEMODE));
	    dfp = fopen(dfile, "a");
	    ASSERT(dfp != NULL, Ct_OPEN, dfile, errno);
	    if (NOTEMPTY(errDfile) && (errdfp = fopen(errDfile, "r")) != NULL) {
		(void) fprintf(dfp, "%s\n\t===== error output =====\n", msgbuf);
		(void) xfappend(errdfp, dfp);	/* who cares if it fails? */
		(void) fclose(errdfp);
	    }
	    (void) fclose(dfp);
	}

	if (!EQUALS(fout, "/dev/null")) {
	    /*
	     * if output is on this machine copy output
	     * there, otherwise spawn job to send to send
	     * output elsewhere.
	     */

	    if (EQUALS(sysout, Myname)) {
		DEBUG(9, "sysout = |%s|\n", sysout);
		DEBUG(9, "xmv(%s, ", dfile);
		DEBUG(9, "%s)\n", fout);
		if (xmv(dfile, fout) == FAIL) {
		    (void) sprintf(lbuf, MSGSTR(MSG_UUXQT_11,
			    "UUXQT: CANNOT CREATE %s"), fout);
		    (void) sprintf(msgbuf, MSGSTR(MSG_UUXQT_11,
				"uuxqt: Cannot create %s"), fout);
		    logent(incmd, lbuf);

		    if (send_nonzero)
			retosndr(user, Rmtname,
			    return_stdin ? fin : "", incmd, msgbuf, "");
		    if (store_status) 
			    uucpst(Rmtname, _Sfile, "", incmd, msgbuf);
		}
	    }
	    else {
		gename(CMDPRE, sysout, 'O', tempname);
	        (void) sprintf(cfile, "%s/%s", WORKSPACE, tempname);
		fp = fdopen(ret = creat(cfile, CFILEMODE), "w");
		ASSERT(ret >= 0 && fp != NULL, Ct_OPEN, cfile, errno);
		(void) fprintf(fp, "S %s %s %s -d %s 0666\n",
		  BASENAME(dfile, '/'), fout, user, BASENAME(dfile, '/'));
		fclose(fp);
		wfcommit(dfile, BASENAME(dfile, '/'), sysout);
		wfcommit(cfile, BASENAME(cfile, '/'), sysout);
	    }
	}
rmfiles:

      /* delete job files in spool directory */
	xfp = fopen(_Xfile, "r");
	ASSERT(xfp != NULL, Ct_OPEN, _Xfile, errno);
	while (fgets(buf, BUFSIZ, xfp) != NULL) {
	    if (buf[0] != X_RQDFILE)
		continue;
	    (void) sscanf(&buf[1], "%s", file);
	    (void) unlink(file);
	}
	(void) unlink(_Xfile);
	fclose(xfp);
	unlink(errDfile);
    }
}


/*
 * return the number of files in directory <dir> who's names
 * begin with <prefix>
 * This is used to count the number of uuxqts currently running.
 *
 */

cuantos(prefix, dir)
char *prefix, *dir;
{
	int i = 0;
	DIR	*pdir;
#ifdef PDA
	char fullname[NAME_MAX], file[NAME_MAX];
#else
	char fullname[MAXNAMESIZE], file[MAXNAMESIZE];
#endif

	pdir = opendir(dir);
	ASSERT(pdir != NULL, Ct_OPEN, dir, errno);

	while (gnamef(pdir, file) == TRUE)
		if (PREFIX(prefix, file)) {
		    (void) sprintf(fullname, "%s/%s", dir, file);
		    if (checkLock(fullname))
			i++;
		}
	closedir(pdir);
	return(i);
}
