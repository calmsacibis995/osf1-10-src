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
static char rcsid[] = "@(#)$RCSfile: cico.c,v $ $Revision: 4.3.7.3 $ (DEC) $Date: 1993/10/11 19:31:05 $";
#endif
/* 
 * COMPONENT_NAME: UUCP cico.c
 * 
 * FUNCTIONS: Mcico, TMname, cleanTM, cleanup, closedem, intrEXIT, 
 *            onintr, pskip, timeout 
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
1.12  cico.c, bos320 10/18/90 11:20:44";
*/

/*
**	cico.c
**
** uucp file transfer program:
** to place a call to a remote machine, login, and
** copy files between the two machines.
*/

#include "uucp.h"
/* VERSION( cico.c	5.3 -  -  ); */

jmp_buf Sjbuf;
extern int Errorrate;
char	uuxqtarg[MAXBASENAME] = {'\0'};
int	maxmsg = MAXMSGTIME;
int	maxstart = MAXSTART;

#define USAGE	"Usage: uucico [-rROLE_NUMBER] [-xDEBUG_LEVEL] [-sSYSTEM_NAME]\n"

extern void closedem();
static char *pskip();
int	_open_max;

main(argc, argv, envp)
char *argv[];
char **envp;
{
	extern onintr(), timeout();
	extern intrEXIT();
	int ret, seq, exitcode;
	char file[NAMESIZE];
	char msg[BUFSIZ], *p, *q;
	char xflag[6];	/* -xN N is single digit */
	char *ttyn;
	char	cb[128];
	time_t	ts, tconv;
#ifndef	V7
	long 	minulimit, dummy;
#endif
	if( (_open_max = sysconf(_SC_OPEN_MAX)) == -1) {
		perror("uucico: sysconf error");
		exit(1);
	}

	Uid = getuid();
	Euid = geteuid();	/* this should be UUCPUID */
	if (Uid == 0)
	    setuid(UUCPUID);	/* fails in ATTSV, but so what? */
	Env = envp;
	Role = SLAVE;
	strcpy(Logfile, LOGCICO);
	*Rmtname = NULLCHAR;

	setlocale(LC_ALL,"");
	catd = catopen(MF_UUCP, NL_CAT_LOCALE);


	closedem();
	time(&Nstat.t_qtime);
	tconv = Nstat.t_start = Nstat.t_qtime;
	strcpy(Progname, "uucico");
	Pchar = 'C';
	(void) signal(SIGILL, (void(*)(int)) intrEXIT);
	(void) signal(SIGTRAP, (void(*)(int)) intrEXIT);
	(void) signal(SIGIOT, (void(*)(int)) intrEXIT);
	(void) signal(SIGFPE, (void(*)(int)) intrEXIT);
	(void) signal(SIGBUS, (void(*)(int)) intrEXIT);
	(void) signal(SIGSEGV, (void(*)(int)) intrEXIT);
	(void) signal(SIGSYS, (void(*)(int)) intrEXIT);
	if (signal(SIGPIPE, SIG_IGN) != SIG_IGN)	/* This for sockets */
		(void) signal(SIGPIPE, (void(*)(int)) intrEXIT);
	(void) signal(SIGINT, (void(*)(int)) onintr);
	(void) signal(SIGHUP, (void(*)(int)) onintr);
	(void) signal(SIGQUIT, (void(*)(int)) onintr);
	(void) signal(SIGTERM, (void(*)(int)) onintr);
#ifdef ATTSV
	(void) signal(SIGUSR1, SIG_IGN);
	(void) signal(SIGUSR2, SIG_IGN);
#endif
        ret = guinfo(Euid, User);
	ASSERT(ret == 0, MSGSTR(MSG_CICOA1,"BAD UID "), "", ret);

	strncpy(Uucp, User, NAMESIZE);
	setuucp(User);

	ret = guinfo(Uid, Loginuser);
	ASSERT(ret == 0, MSGSTR(MSG_CICOA2,"BAD LOGIN_UID "), "", ret);

	*xflag = NULLCHAR;
	Ifn = Ofn = -1;
	while ((ret = getopt(argc, argv, "d:r:s:t:x:")) != EOF) {
		switch (ret) {
		case 'd':
			Spool = optarg;
			break;
		case 'r':
			Role = atoi(optarg);
			if (Role != SLAVE && Role != MASTER) {
			    fprintf(stderr, MSGSTR(MSG_CICO1,
						   "\tusage: %s %s\n"),
				    Progname, USAGE);
			    exit(1);
			}
			break;
		case 's':
			if (versys(optarg)) {
			    DEBUG(4, "%s not in Systems file\n", optarg);
			    fprintf(stderr, MSGSTR(MSG_CICO22,
				"cico: %s not in Systems file\n"),
				optarg);
			    /*
			     * It is possible to do a cleanup(101)
			     * here, but since we are still early in
			     * processing its possible to do an exit().
			     */ 
			    exit(101);
			}
			strncpy(Rmtname, optarg, MAXBASENAME);
			Rmtname[MAXBASENAME] = '\0';
			/* set args for possible xuuxqt call */
			strcpy(uuxqtarg, Rmtname);
			break;
		case 't':
			if (strlen(optarg) != strspn(optarg, "1234567890")) {
			  fprintf(stderr, MSGSTR(MSG_CICO21, "Invalid timeout value\n"));
			  exit(1);
			}
			maxmsg = atoi(optarg);
			maxstart = (atoi(optarg) * 10);
			if (maxmsg < 1) {
			  fprintf(stderr, MSGSTR(MSG_CICO21, "Invalid timeout value\n"));
			  exit(1);
			}
			break;
		case 'x':
			Debug = atoi(optarg);
			if (Debug <= 0)
				Debug = 1;
			if (Debug > 9)
				Debug = 9;
			(void) sprintf(xflag, "-x%d", Debug);
			break;
		default:
			(void) fprintf(stderr, MSGSTR(MSG_CICO1, USAGE));
			exit(1);
		}
	}

	/*
	** maxmsg and maxstart were added to allow users to vary the
	** startup time for uucico.
	*/

	DEBUG(7, "maxstart = %d\n", maxstart);
	DEBUG(7, "maxmsg = %d\n", maxmsg);

	if (Role == MASTER) {
	    if (*Rmtname == NULLCHAR) {
		DEBUG(5, "No -s specified\n" , "");
		cleanup(101);
	    }
	    /* get Myname - it depends on who I'm calling--Rmtname */
	    (void) mchFind(Rmtname);
	    myName(Myname);
	    if (EQUALSN(Rmtname, Myname, SYSNSIZE)) {
		DEBUG(5, "This system specified: -sMyname: %s, ", Myname);
		cleanup(101);
	    }
	}

	ASSERT(chdir(Spool) == 0, Ct_CHDIR, Spool, errno);
	strcpy(Wrkdir, Spool);

	if (Role == SLAVE) {

		/*
		 * initial handshake
		 */
		ret = savline();
		Ifn = 0;
		Ofn = 1;
		fixline(Ifn, 0, D_ACU);
		freopen(RMTDEBUG, "a", stderr);
		/* get MyName - use logFind to check PERMISSIONS file */
		(void) logFind(Loginuser, "");
		myName(Myname);

		DEBUG(4,"cico.c: Myname - %s\n",Myname);
		DEBUG(4,"cico.c: Loginuser - %s\n",Loginuser);
		Nstat.t_scall = times(&Nstat.t_tga);
		(void) sprintf(msg, "here=%s", Myname);
		omsg('S', msg, Ofn);
		if (setjmp(Sjbuf)) {

			/*
			 * timed out
			 */
			ret = restline();
			clrlock(CNULL); /* rm tty locks */
			rmlock(CNULL);
			exit(0);
		}
		(void) signal(SIGALRM, (void(*)(int)) timeout);
		(void) alarm(2 * maxmsg);	/* give slow machines a second chance */
		for (;;) {
			ret = imsg(msg, Ifn);
			if (ret != 0) {
				(void) alarm(0);
				ret = restline();
				clrlock(CNULL);	/* rm tty locks */
				rmlock(CNULL);
				exit(0);
			}
			if (msg[0] == 'S')
				break;
		}
		Nstat.t_ecall = times(&Nstat.t_tga);
		(void) alarm(0);
		q = &msg[1];
		p = pskip(q);
		strncpy(Rmtname, q, MAXBASENAME);
		Rmtname[MAXBASENAME] = '\0';

		seq = 0;
		while (*p == '-') {
			q = pskip(p);
			switch(*(++p)) {
			case 'x':
				Debug = atoi(++p);
				if (Debug <= 0)
					Debug = 1;
				break;
			case 'Q':
				seq = atoi(++p);
				break;
			default:
				break;
			}
			p = q;
		}
		DEBUG(4, "sys-%s\n", Rmtname);

#ifdef NOSTRANGERS
/* here's the place to look the remote system up in the Systems file.
 * If the command NOSTRANGERS is executable and 
 * If they're not in my file then hang up */
		if ( (access(NOSTRANGERS, 1) == 0) && versys(Rmtname)) {
		    char unkcmd[64];

		    omsg('R', "You are unknown to me", Ofn);
		    (void) sprintf(unkcmd, "%s %s", NOSTRANGERS, Rmtname);
		    system(unkcmd);
		    cleanup(101);
		}
#endif

		if (ttylock(Rmtname)) {
			omsg('R', "LCK", Ofn);
			cleanup(101);
		}
		
		/* validate login using PERMISSIONS file */
		if (logFind(Loginuser, Rmtname) == FAIL) {
			Uerror = SS_BAD_LOG_MCH;
			logent(UERRORTEXT, MSGSTR(MSG_CICOL1,"FAILED"));
			systat(Rmtname, SS_BAD_LOG_MCH, UERRORTEXT,
			    Retrytime);
			omsg('R', "LOGIN", Ofn);
			cleanup(101);
		}

		ret = callBack();
		DEBUG(4,"return from callcheck: %s",ret ? "TRUE" : "FALSE");
		if (ret==TRUE) {
			(void) signal(SIGINT, SIG_IGN);
			(void) signal(SIGHUP, SIG_IGN);
			omsg('R', "CB", Ofn);
			logent(MSGSTR(MSG_CICOL2,"CALLBACK"), 
			       MSGSTR(MSG_CICOL3,"REQUIRED"));

			/*
			 * set up for call back
			 */
			systat(Rmtname, SS_CALLBACK, MSGSTR(MSG_CICO2,
				"CALL BACK"), Retrytime);
			gename(CMDPRE, Rmtname, 'C', file);
			(void) close(creat(file, CFILEMODE));
			xuucico(Rmtname);
			cleanup(101);
		}

		if (callok(Rmtname) == SS_SEQBAD) {
			Uerror = SS_SEQBAD;
			logent(UERRORTEXT, MSGSTR(MSG_CICOL4,"PREVIOUS"));
			omsg('R', "BADSEQ", Ofn);
			cleanup(101);
		}

		if ((ret = gnxseq(Rmtname)) == seq) {
			omsg('R', "OK", Ofn);
			cmtseq();
		} else {
			Uerror = SS_SEQBAD;
			systat(Rmtname, SS_SEQBAD, UERRORTEXT, Retrytime);
			logent(UERRORTEXT, MSGSTR(MSG_CICOL5,
				"HANDSHAKE FAILED"));
			ulkseq();
			omsg('R', "BADSEQ", Ofn);
			cleanup(101);
		}
		ttyn = ttyname(Ifn);
		if (ttyn != NULL) {
			strcpy(Dc, BASENAME(ttyn, '/'));
			chmod(ttyn, 0666);	/* can fail, but who cares? */
		} else
			strcpy(Dc, "notty");
		/* set args for possible xuuxqt call */
		strcpy(uuxqtarg, Rmtname);
	}

	strcpy(User, Uucp);
/*
 *  Ensure reasonable ulimit (MINULIMIT)
 */
/***OSF
#ifndef	V7
	minulimit = ulimit(1,dummy);
	ASSERT(minulimit >= MINULIMIT, MSGSTR(MSG_CICOA3,"ULIMIT TOO SMALL"),
	    Loginuser, minulimit);
#endif
****/
	if (Role == MASTER && callok(Rmtname) != 0) {
		logent(MSGSTR(MSG_CICOL21,"SYSTEM STATUS"), 
		       MSGSTR(MSG_CICOL6,"CAN NOT CALL"));
		cleanup(101);
	}

	chremdir(Rmtname);

	(void) strcpy(Wrkdir, RemSpool);
	if (Role == MASTER) {

		/*
		 * master part
		 */
		(void) signal(SIGINT, SIG_IGN);
		(void) signal(SIGHUP, SIG_IGN);
		(void) signal(SIGQUIT, SIG_IGN);
		if (Ifn != -1 && Role == MASTER) {
			(void) write(Ofn, EOTMSG, strlen(EOTMSG));
			(void) close(Ofn);
			(void) close(Ifn);
			Ifn = Ofn = -1;
			clrlock(CNULL);
			rmlock(CNULL);
			sleep(3);
		}
		(void) sprintf(msg, MSGSTR(MSG_CICO9,"call to %s "), Rmtname);
		if (ttylock(Rmtname) != 0) {
			logent(msg, MSGSTR(MSG_CICOL22,"LOCKED"));
			CDEBUG(1, "Currently Talking With %s\n",
			    Rmtname);
 			cleanup(100);
		}
		Nstat.t_scall = times(&Nstat.t_tga);
		Ofn = Ifn = conn(Rmtname);
		Nstat.t_ecall = times(&Nstat.t_tga);
		if (Ofn < 0) {
			ttyunlock(Rmtname);
			logent(UERRORTEXT, MSGSTR(MSG_CICOL7,"CONN FAILED"));
			systat(Rmtname, Uerror, UERRORTEXT, Retrytime);
			cleanup(101);
		} else {
			logent(msg, MSGSTR(MSG_CICOL8,"SUCCEEDED"));
			ttyn = ttyname(Ifn);
			if (ttyn != NULL)
				chmod(ttyn, DEVICEMODE);
		}
	
		if (setjmp(Sjbuf)) {
			ttyunlock(Rmtname);
			Uerror = SS_LOGIN_FAILED;
			logent(Rmtname, UERRORTEXT);
			systat(Rmtname, SS_LOGIN_FAILED,
			    UERRORTEXT, Retrytime);
			DEBUG(4, "%s - failed\n", UERRORTEXT);
			cleanup(101);
		}
		(void) signal(SIGALRM, (void(*)(int)) timeout);
		/* give slow guys lots of time to thrash */
		(void) alarm(3 * maxmsg);
		for (;;) {
			ret = imsg(msg, Ifn);
			if (ret != 0) {
				continue; /* try again */
			}
			if (msg[0] == 'S')
				break;
		}
		(void) alarm(0);
		if(EQUALSN("here=", &msg[1], 5)){
			/*
			 * this is a problem.  We'd like to compare with an
			 * untruncated Rmtname but we fear incompatability.
			 * So we'll look at most 6 chars (at most).
			 */
			if(!EQUALSN(&msg[6], Rmtname, (strlen(Rmtname)< 7 ?
						strlen(Rmtname) : 6))){
				ttyunlock(Rmtname);
				Uerror = SS_WRONG_MCH;
				logent(&msg[6], UERRORTEXT);
				systat(Rmtname, SS_WRONG_MCH, UERRORTEXT,
				     Retrytime);
				DEBUG(4, "%s - failed\n", UERRORTEXT);
				cleanup(101);
			}
		}
		CDEBUG(1,MSGSTR(MSG_CICOCD1,"Login Successful: System=%s\n"),
			&msg[6]);
		seq = gnxseq(Rmtname);
		(void) sprintf(msg, "%s -Q%d %s", Myname, seq, xflag);
		omsg('S', msg, Ofn);
		(void) alarm(2 * maxmsg);	/* give slow guys some thrash time */
		for (;;) {
			ret = imsg(msg, Ifn);
			DEBUG(4, "msg-%s\n", msg);
			if (ret != 0) {
				(void) alarm(0);
				ttyunlock(Rmtname);
				ulkseq();
				cleanup(101);
			}
			if (msg[0] == 'R')
				break;
		}
		(void) alarm(0);

		/*  check for rejects from remote */
		Uerror = 0;
		if (EQUALS(&msg[1], "LCK")) 
			Uerror = SS_RLOCKED;
		else if (EQUALS(&msg[1], "LOGIN"))
			Uerror = SS_RLOGIN;
		else if (EQUALS(&msg[1], "CB"))
			Uerror = SS_CALLBACK;
		else if (EQUALS(&msg[1], "You are unknown to me"))
			Uerror = SS_RUNKNOWN;
		else if (EQUALS(&msg[1], "BADSEQ"))
			Uerror = SS_SEQBAD;
		else if (!EQUALS(&msg[1], "OK"))
			Uerror = SS_UNKNOWN_RESPONSE;
		if (Uerror)  {
			ttyunlock(Rmtname);
			systat(Rmtname, Uerror, UERRORTEXT, Retrytime);
			logent(UERRORTEXT, MSGSTR(MSG_CICOL5,
				"HANDSHAKE FAILED"));
			CDEBUG(1, MSGSTR(MSG_CICOCD2,"HANDSHAKE FAILED: %s\n"),
				 UERRORTEXT);
			ulkseq();
			cleanup(101);
		}
		cmtseq();
	}
	DEBUG(4, " Rmtname %s, ", Rmtname);
	DEBUG(4, "Role %s,  ", Role ? "MASTER" : "SLAVE");
	DEBUG(4, "Ifn - %d, ", Ifn);
	DEBUG(4, "Loginuser - %s\n", Loginuser);

	/* alarm/setjmp added here due to experience with uucico
	 * hanging for hours in imsg().
	 */
	if (setjmp(Sjbuf)) {
		ttyunlock(Rmtname);
		logent(MSGSTR(MSG_CICOL10,"startup"), MSGSTR(MSG_CICOL11,
				"TIMEOUT"));
		DEBUG(4, "%s - timeout\n", "startup");
		cleanup(101);
	}
	(void) alarm(maxstart);
	ret = startup(Role);
	(void) alarm(0);

	if (ret != SUCCESS) {
		ttyunlock(Rmtname);
		logent(MSGSTR(MSG_CICOL10,"startup"), MSGSTR(MSG_CICOL1,
			"FAILED"));
		Uerror = SS_STARTUP;
		CDEBUG(1, "%s\n", UERRORTEXT);
		systat(Rmtname, Uerror, UERRORTEXT, Retrytime);
		exitcode = 101;
	} else {
		logent(MSGSTR(MSG_CICOL10,"startup"), MSGSTR(MSG_CICO4,"OK"));
		systat(Rmtname, SS_INPROGRESS, UTEXT(SS_INPROGRESS),Retrytime);
		Nstat.t_sftp = times(&Nstat.t_tga);

		exitcode = cntrl(Role);
		Nstat.t_eftp = times(&Nstat.t_tga);
		DEBUG(4, "cntrl - %d\n", exitcode);
		(void) signal(SIGINT, SIG_IGN);
		(void) signal(SIGHUP, SIG_IGN);
		(void) signal(SIGALRM, (void(*)(int)) timeout);

		if (exitcode == 0) {
			(void) time(&ts);
			(void) sprintf(cb, MSGSTR(MSG_CICO7,
				"conversation complete %s %ld"),Dc, ts - tconv);
			logent(cb, MSGSTR(MSG_CICO4,"OK"));
			systat(Rmtname, SS_OK, UTEXT(SS_OK), Retrytime);

		} else {
			logent(MSGSTR(MSG_CICOL17,"conversation complete"), 
				MSGSTR(MSG_CICOL1,"FAILED"));
			systat(Rmtname, SS_CONVERSATION,
			    UTEXT(SS_CONVERSATION), Retrytime);
		}
		(void) alarm(2 * maxmsg);	/* give slow guys some thrash time */
		omsg('O', "OOOOO", Ofn);
		CDEBUG(4, MSGSTR(MSG_CICOCD3,"send OO %d,"), ret);
		if (!setjmp(Sjbuf)) {
			for (;;) {
				omsg('O', "OOOOO", Ofn);
				ret = imsg(msg, Ifn);
				if (ret != 0)
					break;
				if (msg[0] == 'O')
					break;
			}
		}
		(void) alarm(0);
	}
	cleanup(exitcode);
}

/*
 * clean and exit with "code" status
 */
cleanup(code)
register int code;
{
	int ret;
	char *ttyn;

	(void) signal(SIGINT, SIG_IGN);
	(void) signal(SIGHUP, SIG_IGN);
	clrlock(CNULL);
	rmlock(CNULL);
	closedem();
	if (Role == SLAVE) {
		ret = restline();
		DEBUG(4, "ret restline - %d\n", ret);
		sethup(0);
	}
	if (Ofn != -1) {
		if (Role == MASTER)
			(void) write(Ofn, EOTMSG, strlen(EOTMSG));
		ttyn = ttyname(Ifn);
		if (ttyn != NULL)
			chmod(ttyn, 0666);	/* can fail, but who cares? */
		(void) close(Ifn);
		(void) close(Ofn);
	}
	DEBUG(4, "exit code %d\n", code);
	if(code)
		CDEBUG(1, MSGSTR(MSG_CICOCD4,
			"Conversation Complete: Status FAILED\n\n"), ""); 
	else
		CDEBUG(1, MSGSTR(MSG_CICOCD5,
			"Conversation Complete: Status SUCCEEDED\n\n"), ""); 

	cleanTM();
	if (code == 0)
		/* Have uuxqt do all remote job requests.
		   If only the Rmtname jobs are done then
		   no local X. files will get executed even
		   though remote data files are now present. */
		xuuxqt(NULL);
	catclose(catd);
	exit(code);
}

short TM_cnt = 0;
char TM_name[MAXNAMESIZE];

cleanTM()
{
	register int i;
	char tm_name[MAXNAMESIZE];

	DEBUG(7,"TM_cnt: %d\n",TM_cnt);
	for(i=0; i < TM_cnt; i++) {
		(void) sprintf(tm_name, "%s.%3.3d", TM_name, i);
		DEBUG(7, "tm_name: %s\n", tm_name);
		unlink(tm_name);
	}
}

TMname(file, pnum)
char *file;
{

	(void) sprintf(file, "%s/TM.%.5d.%.3d", RemSpool, pnum, TM_cnt);
	if (TM_cnt == 0)
	    (void) sprintf(TM_name, "%s/TM.%.5d", RemSpool, pnum);
	DEBUG(7, "TMname(%s)\n", file);
	TM_cnt++;
}

/*
 * intrrupt - remove locks and exit
 */
onintr(inter)
register int inter;
{
	char str[30];
	/* I'm putting a test for zero here because I saw it happen
	 * and don't know how or why, but it seemed to then loop
	 * here for ever?
	 */
	if (inter == 0)
	    exit(99);
	(void) signal(inter, SIG_IGN);
	(void) sprintf(str, "SIGNAL %d", inter);
	logent(str, MSGSTR(MSG_CICOL19,"CAUGHT"));
	cleanup(inter);
}

/*ARGSUSED*/
intrEXIT(inter)
{
	char	cb[10];
	extern int errno;

	(void) sprintf(cb, "%d", errno);
	logent(MSGSTR(MSG_CICOL20,"INTREXIT"), cb);
	(void) signal(SIGIOT, SIG_DFL);
	(void) signal(SIGILL, SIG_DFL);
	clrlock(CNULL);
	rmlock(CNULL);
	closedem();
	(void) setuid(Uid);
	abort();
}

/*
 * catch SIGALRM routine
 */
timeout()
{
	longjmp(Sjbuf, 1);
}

static char *
pskip(p)
register char *p;
{
	while( *p && *p != ' ' )
		++p;
	if( *p ) *p++ = 0;
	return(p);
}

void
closedem()
{
	register i;

	for(i=3;i<_open_max;i++)
		(void) close(i);
}
