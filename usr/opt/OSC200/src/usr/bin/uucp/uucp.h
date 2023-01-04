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
/* @(#)$RCSfile: uucp.h,v $ $Revision: 4.2.7.2 $ (DEC) $Date: 1993/09/07 16:09:01 $ */
/* 
 * COMPONENT_NAME: UUCP uucp.h
 * 
 * FUNCTIONS: ASSERT, BASENAME, CDEBUG, DEBUG, DIRECTORY, EQUALS, 
 *            EQUALSN, LASTCHAR, MSGSTR, NOTEMPTY, PREFIX, READANY, 
 *            READSOME, UTEXT, VERBOSE, VERSION, WRITEANY 
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 * 
 * uucp.h	1.13+  com/cmd/uucp,3.1,8943OSF7 11/13/89 15:44:51
 */

/*	uucp.h	1.18
*/
/*
#ifndef lint
static	char	h_uucp[] = "uucp.h	1.18";
#endif lint
*/

#define TIMBUF_SIZE 128         /* mik */
#include <langinfo.h>           /* mik */

#define	DFS				/* silly -- for stat.h */

#include "parms.h"
#include "msg.h"

extern char *catgets();

#include <locale.h>
#include "uucp_msg.h"
nl_catd catd; 
#define MSGSTR(N,S)	catgets(catd, MS_UUCP, N, S)


#ifdef BSD4_2
#define V7
#undef NONAP
#undef FASTTIMER
#endif

#ifdef FASTTIMER
#undef NONAP
#endif

#include <stdio.h>
#include <ctype.h>
#include <setjmp.h>
#include <sys/param.h>
#include <unistd.h>
/*
#if defined (ATTSV) && ! defined (CDLIMIT)
#include <sys/fmgr.h>
#endif
*/

/*
 * param.h includes types.h and signal.h in 4bsd
 */
#ifdef V7
#include <sgtty.h>
#include <sys/timeb.h>
#else
#include <termio.h>
#include <sys/types.h>
#include <signal.h>
#include <fcntl.h>
#endif

#include <sys/stat.h>
#include <dirent.h>

#ifdef BSD4_2
#include <sys/time.h>
#else
#include <time.h>
#endif

#include <sys/times.h>
#include <errno.h>

/*
#ifdef ATTSV
#include <sys/sysmacros.h>
#endif
*/

#include <sys/types.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <sys/ustat.h>

#include <sys/access.h>

/* what mode should D. files have upon creation? */
#define DFILEMODE 0600

/* what mode should C. files have upon creation? */
#define CFILEMODE 0644

/* define the value of DIRMASK, for umask call */
/* used for creating system subdirectories */
#define DIRMASK 0022

#define MAXSTART	300	/* how long to wait on startup */

/* define the last characters for ACU  (used for 801/212 dialers) */
#define ACULAST "<"

/*  caution - the following names are also in Makefile 
 *    any changes here have to also be made there
 *
 * it's a good idea to make directories .foo, since this ensures
 * that they'll be ignored by processes that search subdirectories in SPOOL
 *
 *  XQTDIR=/usr/spool/uucp/.Xqtdir
 *  CORRUPT=/usr/spool/uucp/.Corrupt
 *  LOGDIR=/usr/spool/uucp/.Log
 *  SEQDIR=/usr/spool/uucp/.Sequence
 *  STATDIR=/usr/spool/uucp/.Status
 *  
 */

/* where to put the STST. files? */
#define STATDIR		"/var/spool/uucp/.Status"

/* where should logfiles be kept? */
#define LOGUUX		"/var/spool/uucp/.Log/uux"
#define LOGUUXQT	"/var/spool/uucp/.Log/uuxqt"
#define LOGUUCP		"/var/spool/uucp/.Log/uucp"
#define LOGCICO		"/var/spool/uucp/.Log/uucico"
#define CORRUPTDIR	"/var/spool/uucp/.Corrupt"

/* some sites use /usr/lib/uucp/.XQTDIR here */
/* use caution since things are linked into there */
#define XQTDIR		"/var/spool/uucp/.Xqtdir"

/* how much of a system name can we print in a [CX]. file? */
/* MAXBASENAME - 1 (pre) - 1 ('.') - 1 (grade) - 4 (sequence number) */
#define SYSNSIZE (MAXBASENAME - 7)

#ifdef ETCLOCKS
#define DEVICE_LOCKPRE		"/etc/locks/"
#define LOCKPRE			"/etc/locks/LCK."
#else
#ifdef USRSPOOLLOCKS
#define DEVICE_LOCKPRE	"/var/spool/locks/"
#define LOCKPRE		"/var/spool/locks/LCK."
#else
#define DEVICE_LOCKPRE	"/var/spool/uucp/"
#define LOCKPRE		"/var/spool/uucp/LCK."
#endif /* USRSPOOLLOCKS */
#endif /* ETCLOCKS */

#define SQFILE		"/etc/uucp/SQFILE"
#define SQTMP		"/etc/uucp/SQTMP"
#define SLCKTIME	5400	/* system/device timeout (LCK.. files) */
#define SYSFILE		"/etc/uucp/Systems"
#define DEVFILE		"/etc/uucp/Devices"
#define DIALERFILE	"/etc/uucp/Dialers"
#define DIALFILE	"/etc/uucp/Dialcodes"
#define PFILE		"/etc/uucp/Permissions"


#define SPOOL		"/var/spool/uucp"
#define SEQDIR		"/var/spool/uucp/.Sequence"

#define X_LOCKTIME	3600
#ifdef ETCLOCKS
#define SEQLOCK		"/etc/locks/LCK.SQ."
#define SQLOCK		"/etc/locks/LCK.SQ"
#define X_LOCK		"/etc/locks/LCK.X"
#define S_LOCK		"/etc/locks/LCK.S"
#define X_LOCKDIR	"/etc/locks"	/* must be dir part of above */
#else
#ifdef USRSPOOLLOCKS
#define SEQLOCK		"/var/spool/locks/LCK.SQ."
#define SQLOCK		"/var/spool/locks/LCK.SQ"
#define X_LOCK		"/var/spool/locks/LCK.X"
#define S_LOCK		"/var/spool/locks/LCK.S"
#define X_LOCKDIR	"/var/spool/locks"	/* must be dir part of above */
#else
#define SEQLOCK		"/var/spool/uucp/LCK.SQ."
#define SQLOCK		"/var/spool/uucp/LCK.SQ"
#define X_LOCK		"/var/spool/uucp/LCK.X"
#define S_LOCK		"/var/spool/uucp/LCK.S"
#define X_LOCKDIR	"/var/spool/uucp"	/* must be dir part of above */
#endif /* USRSPOOLLOCKS */
#endif /* ETCLOCKS */
#define X_LOCKPRE	"LCK.X"		/* must be last part of above */

#define PUBDIR		"/var/spool/uucppublic"
#define ADMIN		"/var/spool/uucp/.Admin"
#define ERRLOG		"/var/spool/uucp/.Admin/errors"
#define SYSLOG		"/var/spool/uucp/.Admin/xferstats"
#define RMTDEBUG	"/var/spool/uucp/.Admin/audit"
#define CLEANUPLOGFILE	"/var/spool/uucp/.Admin/uucleanup"

#define	WORKSPACE	"/var/spool/uucp/.Workspace"

#define SQTIME		60
#define TRYCALLS	2	/* number of tries to dial call */
#define MINULIMIT	(1L<<11)	/* minimum reasonable ulimit */

/*
 * CDEBUG is for communication line debugging 
 * DEBUG is for program debugging 
 * #define SMALL to compile without the DEBUG code
 *
 * Note: If MSG is defined, CDEBUG and DEBUG expect f and s to be strings 
 * 	 that have been retreived by or prepared for the Message Facility.
 *	 
 */
#define CDEBUG(l, f, s) if (Debug >= l) fprintf(stderr, f, s)

#ifndef SMALL
#define DEBUG(l, f, s) if (Debug >= l) fprintf(stderr, f, s)
#else
#define DEBUG(l, f, s)
#endif

/*
 * VERBOSE is used by cu and ct to inform the user
 * about the progress of connection attempts.
 * In uucp, this will be NULL.
 *
 * Note: If MSG is defined, VERBOSE expects f and s to be strings 
 * 	 that have been retreived by or prepared for the Message Facility.
 */

#ifdef STANDALONE
#define VERBOSE(f, s) if (Verbose > 0) fprintf(stderr, f, s); else
#else
#define VERBOSE(f, s)
#endif

#define PREFIX(pre, str)	(strncmp((pre), (str), strlen(pre)) == SAME)
#define BASENAME(str, c) ((Bnptr = (char *)strrchr((str), c)) ? (Bnptr + 1) : (str))
#define EQUALS(a,b)	((a) && (b) && (strcmp((a),(b))==SAME))
#define EQUALSN(a,b,n)	((a) && (b) && (strncmp((a),(b),(n))==SAME))
#define LASTCHAR(s)	(s+strlen(s)-1)

#define SAME 0
#define ANYREAD 04
#define ANYWRITE 02
#define FAIL -1
#define SUCCESS 0
#define NULLCHAR	'\0'
#define CNULL (char *) 0
#define STBNULL (struct sgttyb *) 0
#define MASTER 1
#define SLAVE 0

/* Changing MAXNASENAME and NAMEASIZE is required for initial PDA 
   support. That is updating the sources to use directory access
   routines instead of doing so directly.
   In this first pass of adding PDA support, NAME_MAX will be
   kept to 14.  However, before NAME_MAX is set higher than the 
   current 14, the BNU sources should be reviewed for space efficiency.
   Other defines might also need to be changed.
*/
#ifdef PDA 
#define MAXBASENAME 	NAME_MAX 
#define NAMESIZE 	MAXBASENAME /* includes ending null char */
#else
#define MAXBASENAME 14 /* should be DIRSIZ but 4.2bsd prohibits that */
#define NAMESIZE MAXBASENAME+1
#endif

#define MAXFULLNAME BUFSIZ
#define MAXNAMESIZE	64	/* /usr/spool/uucp/<14 chars>/<14 chars>+slop */
#define MAXMSGTIME 33
#define MAXEXPECTTIME 45
#define MAXCHARTIME 15
#define	SIZEOFPID	10		/* maximum number of digits in a pid */
#define EOTMSG "\004\n\004\n"
#define CALLBACK 1

/* manifest for chkpth flag */
#define CK_READ		0
#define CK_WRITE	1

/*
 * commands
 */
#define SHELL		"/usr/bin/sh"
#define MAIL		"mail"
#define UUCICO		"/usr/lbin/uucp/uucico"
#define UUXQT		"/usr/lbin/uucp/uuxqt"
#define UUCP		"uucp"


/* system status stuff */
#define SS_OK			0
#define SS_NO_DEVICE		1
#define SS_TIME_WRONG		2
#define SS_INPROGRESS		3
#define SS_CONVERSATION		4
#define SS_SEQBAD		5
#define SS_LOGIN_FAILED		6
#define SS_DIAL_FAILED		7
#define SS_BAD_LOG_MCH		8
#define SS_LOCKED_DEVICE	9
#define SS_ASSERT_ERROR		10
#define SS_BADSYSTEM		11
#define SS_CANT_ACCESS_DEVICE	12
#define SS_DEVICE_FAILED	13	/* No longer used */
#define SS_WRONG_MCH		14
#define SS_CALLBACK		15
#define SS_RLOCKED		16
#define SS_RUNKNOWN		17
#define SS_RLOGIN		18
#define SS_UNKNOWN_RESPONSE	19
#define SS_STARTUP		20
#define SS_CHAT_FAILED		21

#define MAXPH	60	/* maximum phone string size */
#define	MAXC	BUFSIZ

#define	TRUE	1
#define	FALSE	0
#define NAMEBUF	32

/* structure of an Systems file line */
#define F_MAX	50	/* max number of fields in Systems file line */
#define F_NAME 0
#define F_TIME 1
#define F_TYPE 2
#define F_CLASS 3	/* an optional prefix and the speed */
#define F_PHONE 4
#define F_LOGIN 5

/* structure of an Devices file line */
#define D_TYPE 0
#define D_LINE 1
#define D_CALLDEV 2
#define D_CLASS 3
#define D_CALLER 4
#define D_ARG 5
#define D_MAX	50	/* max number of fields in Devices file line */

#define D_ACU 1
#define D_DIRECT 2
#define D_PROT 4

/* past here, local changes are not recommended */
#define CMDPRE		'C'
#define DATAPRE		'D'
#define XQTPRE		'X'

/*
 * stuff for command execution
 */
#define X_RQDFILE	'F'
#define X_STDIN		'I'
#define X_STDOUT	'O'
#define X_CMD		'C'
#define X_USER		'U'
#define X_BRINGBACK	'B'
#define X_MAILF		'M'
#define X_RETADDR	'R'
#define X_COMMENT	'#'
#define X_NONZERO	'Z'
#define X_SENDNOTHING	'N'
#define X_SENDZERO	'n'


/* This structure describes call routines */
struct caller {
	char	*CA_type;
	int	(*CA_caller)();
};

/* This structure describes dialing routines */
struct dialer {
	char	*DI_type;
	int	(*DI_dialer)();
};

struct nstat {
	int	t_pid;		/* process id				*/
	long	t_start;	/* process id				*/
	time_t	t_beg;		/* start  time				*/
	time_t	t_scall;	/* start call to system			*/
	time_t	t_ecall;	/* end call to system			*/
	time_t	t_tacu;		/* acu time				*/
	time_t	t_tlog;		/* login time				*/
	time_t	t_sftp;		/* start file transfer protocol		*/
	time_t	t_sxf;		/* start xfer 				*/
	time_t	t_exf;		/* end xfer 				*/
	time_t	t_eftp;		/* end file transfer protocol		*/
	time_t	t_qtime;	/* time file queued			*/
	int	t_ndial;	/* # of dials				*/
	int	t_nlogs;	/* # of login trys			*/
	struct tms t_tbb;	/* start execution times		*/
	struct tms t_txfs;	/* xfer start times			*/
	struct tms t_txfe;	/* xfer end times 			*/
	struct tms t_tga;	/* garbage execution times		*/
};


/* external declarations */

extern char *Spool;	/*CMR001*/
extern char *LineType;
extern int Ifn, Ofn;
extern int Debug, Verbose;
extern int Bspeed;
extern int Uid, Euid;		/* user-id and effective-uid */
extern char Wrkdir[];
extern int Retrytime;
extern char **Env;
extern char Uucp[];
extern char Pchar;
extern struct nstat Nstat;
extern char Dc[];			/* line name			*/
extern char Fwdname[];		/* foward name			*/
extern int Seqn;			/* sequence #			*/
extern int Role;
extern char Logfile[];
extern int linebaudrate;	/* adjust sleep time on read in pk driver */
extern char Rmtname[];
extern char User[];
extern char Loginuser[];
extern char *Thisdir;
extern char *Pubdir;
extern char Myname[];
extern char Progname[];
extern char RemSpool[];
extern char *Bnptr;		/* used when BASENAME macro is expanded */
extern char *sys_errlist[];

extern char Jobid[];		/* Jobid of current C. file */
extern int Uerror;		/* global error code */
extern char *UerrorText[];	/* text for error code */

/*	Some global I need for section 2 and section 3 routines */
extern errno;
extern char *optarg;	/* for getopt() */
extern int optind;	/* for getopt() */


#define UERRORTEXT		UerrorText[Uerror]
#define UTEXT(x)		UerrorText[x]

/* things get kind of gross beyond this point -- please stay out */
#ifdef ATTSV
#define index strchr
#define rindex strrchr 
#define vfork fork
#define ATTSVKILL
#define UNAME
#else
#define strchr index
#define strrchr rindex
#endif

#ifdef lint
#define VERSION(x)	;
#define ASSERT(e, s1, s2, i1)	;

#else

#define VERSION(x)	static	char	rcsid[] = "x";
#define ASSERT(e, s1, s2, i1) if (!(e)) {\
	assert(s1, s2, i1, rcsid, __FILE__, __LINE__);\
	cleanup(FAIL);};
#endif

extern struct stat __s_;
#define READANY(f)	((stat((f),&__s_)==0) && ((__s_.st_mode&(0004))!=0) )
#define READSOME(f)	((stat((f),&__s_)==0) && ((__s_.st_mode&(0444))!=0) )

#define WRITEANY(f)	((stat((f),&__s_)==0) && ((__s_.st_mode&(0002))!=0) )
#define DIRECTORY(f)	((stat((f),&__s_)==0) && ((__s_.st_mode&(S_IFMT))==S_IFDIR) )
#define NOTEMPTY(f)	((stat((f),&__s_)==0) && (__s_.st_size!=0) )

/*
**	This was commented out because of problems created when 
**	readdir was attempting to handle files that really weren't
**	files ...
*/

/*
#ifndef BSD4_2
#define DIR FILE
#define opendir(x) fopen((x), "r")
#define closedir(x) fclose((x))
#endif BSD4_2
*/

/* standard functions used */

extern char	*strcat(), *strcpy(), *strncpy();
/*
extern char	*strchr(), strrchr(), rindex(), index();
*/
extern char	*strpbrk();
extern char	*getlogin(), *ttyname(), *malloc();
extern char	*calloc();
extern clock_t	times(), atol();
extern off_t	lseek();
extern time_t	time();
extern int	strcmp(), strncmp();
extern int strlen();
extern int	pipe(), close(), getopt();
extern pid_t	fork();
extern struct tm	*localtime();
extern FILE	*popen();

/* uucp functions and subroutine */
extern int	anlwrk(), iswrk(), gtwvec();		/* anlwrk.c */
extern void	chremdir(), mkremdir();			/* chremdir.c */
extern		void toCorrupt();			/* cpmv.c  */
extern		int xcp(), xmv();			/* cpmv.c  */

extern int	getargs();				/* getargs.c */
extern void	bsfix();				/* getargs.c */
extern char	*getprm();				/* getprm.c */

extern void	logent(), syslog(), closelog();		/* logent.c */
extern time_t	millitick();				/* logent.c */

extern char	*protoString();				/* permission.c */
extern		logFind(), mchFind();			/* permission.c */
extern		chkperm(), chkpth();			/* permission.c */
extern		cmdOK(), switchRole();			/* permission.c */
extern		callBack(), requestOK();		/* permission.c */
extern void	myName();				/* permission.c */

extern void	systat();				/* systat.c */
extern int	ttylocked(), clrlock();			/* ttylock.c */
extern int	ttylock(), ttyunlock();			/* ttylock.c */
extern int	ttywait();				/* ttylock.c */
extern void	ttytouchlock();				/* ttylock.c */
extern int	ulockf(), checkLock(), delock();	/* ulockf.c */
extern void	rmlock(), ultouch();			/* ulockf.c */
extern char	*timeStamp();				/* utility.c */
extern void	assert(), errent();			/* utility.c */
extern void	uucpname();				/* uucpname.c */
extern int	versys();				/* versys.c */
extern void	xuuxqt(), xuucico();			/* xqt.c */

#ifdef ATTSV
unsigned	sleep();
void	exit(), setbuf();
long	ulimit();
#else
int	sleep(), exit(), setbuf(), ftime();
#endif

#ifndef NOUSTAT
#ifdef V7USTAT
struct  ustat {
	daddr_t	f_tfree;	/* total free */
	ino_t	f_tinode;	/* total inodes free */
};
#else /* !NOUSTAT && !V7USTAT */
#include <ustat.h>
#endif /* V7USTAT */
#endif /* NOUSTAT */

#ifdef UNAME
#include <sys/utsname.h>
#endif

#if defined(BSD4_2) || defined(BSDTCP)
char *gethostname();
#endif

/* messages */

/*
**	These were commented out because they were causing problems
**	with the NLS Macro expansions.
*/

/*
extern char *Ct_OPEN;
extern char *Ct_WRITE;
extern char *Ct_READ;
extern char *Ct_CREATE;
extern char *Ct_ALLOCATE;
extern char *Ct_LOCK;
extern char *Ct_STAT;
extern char *Ct_CHOWN;
extern char *Ct_CHMOD;
extern char *Ct_LINK;
extern char *Ct_CHDIR;
extern char *Ct_UNLINK;
extern char *Wr_ROLE;
extern char *Ct_CORRUPT;
extern char *Ct_FORK;
extern char *Ct_CLOSE;
extern char *Fl_EXISTS;
extern char *Ue_BADSYSTEM;
extern char *Ue_TIME_WRONG;
extern char *Ue_SYSTEM_LOCKED;
extern char *Ue_NO_DEVICE;
extern char *Ue_DIAL_FAILED;
extern char *Ue_LOGIN_FAILED;
extern char *Ue_SEQBAD;
extern char *Ue_BAD_LOG_MCH;
extern char *Ue_WRONG_MCH;
extern char *Ue_LOCKED_DEVICE;
extern char *Ue_ASSERT_ERROR;
extern char *Ue_CANT_ACCESS_DEVICE;
extern char *Ue_DEVICE_FAILED;
*/
