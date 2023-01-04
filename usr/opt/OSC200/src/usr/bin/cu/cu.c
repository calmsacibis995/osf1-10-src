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
static char rcsid[] = "@(#)$RCSfile: cu.c,v $ $Revision: 4.2.7.3 $ (DEC) $Date: 1993/10/11 16:11:03 $";
#endif
/* 
 * COMPONENT_NAME: UUCP cu.c
 * 
 * FUNCTIONS: Mcu, _bye, _dopercen, _flush, _mode, _onintrpt, _quit, 
 *            _rcvdead, _receive, _shell, _w_str, assert, blckcnt, 
 *            cleanup, dofork, logent, r_char, recfork, sysname, 
 *            tdmp, tilda, transmit, w_char 
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/* cu.c	1.12  com/cmd/uucp,3.1,9013 3/16/90 10:42:54"; */
/* static char sccsid[] = "cu.c	5.1 -  - "; */
/* cu.c	1.34 */
/********************************************************************
 *cu [-sspeed] [-lline] [-h] [-t] [-d] [-n] [-o|-e] telno | systemname
 *
 *	legal baud rates: 300, 1200, 2400, 4800, 9600.
 *
 *	-l is for specifying a line unit from the file whose
 *		name is defined in /usr/lib/uucp/Devices.
 *	-h is for half-duplex (local echoing).
 *	-t is for adding CR to LF on output to remote (for terminals).
 *	-d can be used  to get some tracing & diagnostics.
 *	-o or -e is for odd or even parity on transmission to remote.
 *	-n will request the phone number from the user.
 *	Telno is a telephone number with `=' for secondary dial-tone.
 *	If "-l dev" is used, speed is taken from /usr/lib/uucp/Devices.
 *	Only systemnames that are included in /usr/lib/uucp/Systems may
 *	be used.
 *
 *	Escape with `~' at beginning of line.
 *	Silent output diversions are ~>:filename and ~>>:filename.
 *	Terminate output diversion with ~> alone.
 *	~. is quit, and ~![cmd] gives local shell [or command].
 *	Also ~$ for local procedure with stdout to remote.
 *	Both ~%put from [to]  and  ~%take from [to] invoke built-ins.
 *	Also, ~%break or just ~%b will transmit a BREAK to remote.
 *	~%nostop toggles on/off the DC3/DC1 input control from remote,
 *		(certain remote systems cannot cope with DC3 or DC1).
 *	~%debug(~%b) toggles on/off the program tracing and diagnostics output;
 *	cu should no longer be compiled with #ifdef ddt.
 *
 *	Cu no longer uses dial.c to reach the remote.  Instead, cu places
 *	a telephone call to a remote system through the uucp conn() routine
 *	when the user picks the systemname option or through altconn()--
 *	which bypasses /usr/lib/uucp/Systems -- if a telno or direct
 *	line is chosen. The line termio attributes are set in fixline(),
 *	before the remote connection is made.  As a device-lockout semaphore
 *	mechanism, uucp creates an entry in /usr/spool/locks whose name is
 *	LCK..dev where dev is the device name from the Devices file.
 *	When cu terminates, for whatever reason, cleanup() must be
 *	called to "release" the device, and clean up entries from
 *	the locks directory.  Cu runs with uucp ownership, and thus provides
 *	extra insurance that lock files will not be left around.	
# ******************************************************************/

#include "pathnames.h"
#include <sys/wait.h>
#include <string.h>
#include "uucp.h"

#define MID	BUFSIZ/2	/* mnemonic */
#define	RUB	'\177'		/* mnemonic */
#define	XON	'\21'		/* mnemonic */
#define	XOFF	'\23'		/* mnemonic */
#define	TTYIN	0		/* mnemonic */
#define	TTYOUT	1		/* mnemonic */
#define	TTYERR	2		/* mnemonic */
#define	YES	1		/* mnemonic */
#define	NO	0		/* mnemonic */
#define IOERR	4		/* exit code */
#define	MAXPATH1	100
#define	NPL	50

int Sflag=0;
int Cn;				/*fd for remote comm line */
static char *_Cnname;		/*to save associated ttyname */
jmp_buf Sjbuf;			/*needed by uucp routines*/


static struct call {		/*NOTE-also included in altconn.c--> */
				/*any changes must be made in both places*/
	char *speed;		/* transmission baud rate */
	char *line;		/* device name for outgoing line */
	char *telno;		/* ptr to tel-no digit string */
	char *class;		/* call class */
	}Cucall;

static int dofork(), r_char(), w_char();

void recfork(void);
int transmit(void);
int tilda(char *);
void tdmp(int);
void sysname(char *);
void blckcnt(int);

extern int conn(const char *);
extern int altconn( struct call *);
extern sethup(int);

extern int
	errno,			/* supplied by system interface */
	optind;			/* variable in getopt() */


extern char
	*optarg;
	
static struct termio _Tv, _Tv0;	/* for saving, changing TTY atributes */
static struct termio _Lv;	/* attributes for the line to remote */
static struct utsname utsn; 

static char
	_Cxc,			/* place into which we do character io*/
	_Tintr,			/* current input INTR */
	_Tquit,			/* current input QUIT */
	_Terase,		/* current input ERASE */
	_Tkill,			/* current input KILL */
	_Teol,			/* current secondary input EOL */
	_Myeof;			/* current input EOF */

int
	Terminal=0,		/* flag; remote is a terminal */
	Oddflag = 0,		/*flag- odd parity option*/
	Evenflag = 0,		/*flag- even parity option*/
	Echoe,			/* save users ECHOE bit */
	Echok,			/* save users ECHOK bit */
	Child,			/* pid for receive process */
	Intrupt=NO,		/* interrupt indicator */
	Duplex=YES,		/* Unix= full duplex=YES; half = NO */ 
	Sstop=YES,		/* NO means remote can't XON/XOFF */
	Rtn_code=0,		/* default return code */
	Takeflag=NO;		/* indicates a ~%take is in progress */

static void
	_onintrpt(),		/* interrupt routines */
	_rcvdead(),
	_quit(),
	_bye();

extern void	cleanup();
extern void     tdmp();

static void
	_flush(),
	_shell(),
	_dopercen(),
	_receive(),
	_mode(int),
	_w_str();

char *Myline = NULL;  /* flag to force the requested line to be used  */
		      /* by rddev() in uucp conn.c		    */

/* Message translation for the following messages is done in line */
char *P_USAGE= "USAGE:%s [-s speed] [-l line] [-h] [-n] [-t] [-d] [-o|-e] telno | systemname\n";
char *P_CON_FAILED = "Connect failed: %s\r\n";
char *P_Ct_OPEN = "Cannot open: %s\r\n";
char *P_LINE_GONE = "Remote line gone\r\n";
char *P_Ct_EXSH = "Can't execute shell\r\n";
char *P_Ct_DIVERT = "Can't divert %s\r\n";
char *P_STARTWITH = "Use `~~' to start line with `~'\r\n";
char *P_CNTAFTER = "after %ld bytes\r\n";
char *P_CNTLINES = "%d lines/";
char *P_CNTCHAR = "%ld characters\r\n";
char *P_FILEINTR = "File transmission interrupted\r\n";
char *P_Ct_FK = "Can't fork -- try later\r\n";
char *P_Ct_SPECIAL = "r\nCan't transmit special character `%#o'\r\n";
char *P_TOOLONG = "\nLine too long\r\n";
char *P_IOERR = "r\nIO error\r\n";
char *P_USECMD = "Use `~$'cmd \r\n"; 
char *P_USEPLUSCMD ="Use `~+'cmd \r\n";
char *P_NOTERMSTAT = "Can't get terminal status\r\n";
char *P_3BCONSOLE = "Sorry, you can't cu from a 3B console\r\n";
char *P_PARITY  = "Parity option error\r\n";
char *P_TELLENGTH = "Telno cannot exceed 58 digits!\r\n";

/***************************************************************
 *	main: get command line args, establish connection, and fork.
 *	Child invokes "receive" to read from remote & write to TTY.
 *	Main line invokes "transmit" to read TTY & write to remote.
 ***************************************************************/
void
main( int argc, char *argv[])
{
	char s[MAXPH];
	char *string = NULL;
	int i;
	int errflag=0;
	int nflag=0;
	int systemname=0;
	Verbose = 1;		/*for uucp callers,  dialers feedback*/

	Cucall.speed = "Any";       /*default speed*/
	Cucall.line = NULL;
	Cucall.telno = NULL;
	Cucall.class = NULL;

	setlocale(LC_ALL,"");
	catd = catopen(MF_UUCP, NL_CAT_LOCALE);


		
/*Flags for -h, -t, -e, and -o options set here; corresponding line attributes*/
/*are set in fixline() in culine.c before remote connection is made           */

	while((i = getopt(argc, argv, "dhteons:l:c:")) != EOF)
		switch(i) {
			case 'd':
				Debug = 9; /*turns on uucp debugging-level 9*/
				break;
			case 'h':
				Duplex  = NO;
				Sstop = NO;
				break;
			case 't':
				Terminal = YES;
				break;
			case 'e':
				Evenflag = 1;
				break;
			case 'o':
				Oddflag = 1;
				break;
			case 's':
				Sflag++;
				Cucall.speed = optarg;
			  	break;
			case 'l':
				Cucall.line = optarg;
				break;
#ifdef forfutureuse
/* -c specifies the class-selecting an entry type from the Devices file */
			case 'c':
				Cucall.class = optarg;
				break;
#endif
			case 'n':
				nflag++;
				printf(MSGSTR(MSG_CU22,
					"Please enter the number: "));
				gets(s);
				break;
			case '?':
				++errflag;
		}

	if((optind < argc && optind > 0) || (nflag && optind > 0)) {  
		if(nflag) 
			string=s;
		else
			string = argv[optind];
		if((strlen(string) == strspn(string, "0123456789=-*#")) ||
		   (Cucall.class != NULL))
			Cucall.telno = string;
		else 			/*if it's not a legitimate telno,  */
			systemname++;	/*then it's a systemname           */
	} else
		if(Cucall.line == NULL)   /*if none of above, must be direct */
			++errflag;
	
	if(errflag) {
		VERBOSE(MSGSTR(MSG_CU1, P_USAGE), argv[0]);
		exit(1);
	}

	if( Cucall.telno && strlen(Cucall.telno) >= (MAXPH - 1)) {
		VERBOSE(MSGSTR(MSG_CU21, P_TELLENGTH),"");
		exit(0);
	}

	(void)ioctl(TTYIN, TCGETA, &_Tv0); /* save initial tty state */
	_Tintr = _Tv0.c_cc[VINTR]? _Tv0.c_cc[VINTR]: '\377';
	_Tquit = _Tv0.c_cc[VQUIT]? _Tv0.c_cc[VQUIT]: '\377';
	_Terase = _Tv0.c_cc[VERASE]? _Tv0.c_cc[VERASE]: '\377';
	_Tkill = _Tv0.c_cc[VKILL]? _Tv0.c_cc[VKILL]: '\377';
	_Teol = _Tv0.c_cc[VEOL]? _Tv0.c_cc[VEOL]: '\377';
	_Myeof = _Tv0.c_cc[VEOF]? _Tv0.c_cc[VEOF]: '\04';
	Echoe = _Tv0.c_lflag & ECHOE;
	Echok = _Tv0.c_lflag & ECHOK;

	(void)signal(SIGHUP, cleanup);
	(void)signal(SIGQUIT, cleanup);
	(void)signal(SIGINT, cleanup);

/* place call to system; if "cu systemname", use conn() from uucp
   directly.  Otherwise, use altconn() which dummies in the
   Systems file line.
*/

	if(systemname)
		Cn = conn(string);
	else
		Cn = altconn(&Cucall);

	_Cnname = ttyname(Cn);
	if(_Cnname != NULL) {
		chown(_Cnname, UUCPUID, UUCPGID);
		chmod(_Cnname, DEVICEMODE);
	}

	Euid = geteuid();
	if(setuid(getuid()) && setgid(getgid()) < 0) {
		VERBOSE(MSGSTR(MSG_CUV1,"Unable to setuid/gid\n"),"");
		cleanup();
		}

	if(Cn < 0) {

		VERBOSE(MSGSTR(MSG_CU2, P_CON_FAILED), UERRORTEXT);
		cleanup(-Cn);
	}


	if(Debug) tdmp(Cn); 

	/* At this point succeeded in getting an open communication line */
	/* Conn() takes care of closing the Systems file                       */

	(void)signal(SIGINT,_onintrpt);
	_mode(1);			/* put terminal in `raw' mode */
	VERBOSE(MSGSTR(MSG_CUV2,"Connected\007\r\n"),"");	/*bell!*/

	recfork();		/* checks for child == 0 */

	if(Child > 0) {
		(void)signal(SIGUSR1, _bye);
		(void)signal(SIGHUP, cleanup);
		(void)signal(SIGQUIT, _onintrpt);
		Rtn_code = transmit();
		_quit(Rtn_code);
	} else {
		cleanup(Cn);
	}
}

/*
 *	Kill the present child, if it exists, then fork a new one.
 */
void
recfork(void)
{
	if (Child)
		kill(Child, SIGKILL);
	Child = dofork();
	if(Child == 0) {
		(void)signal(SIGHUP, _rcvdead);
		(void)signal(SIGQUIT, SIG_IGN);
		(void)signal(SIGINT, SIG_IGN);

		_receive();	/* This should run until killed */
		/*NOTREACHED*/
	}
}

/***************************************************************
 *	transmit: copy stdin to remote fd, except:
 *	~.	terminate
 *	~!	local login-style shell
 *	~!cmd	execute cmd locally
 *	~$proc	execute proc locally, send output to line
 *	~%cmd	execute builtin cmd (put, take, or break)
 ****************************************************************/
#ifdef forfutureuse
 /*****************************************************************
  *	~+proc	execute locally, with stdout to and stdin from line.
  ******************************************************************/
#endif

int
transmit(void)
{
	char b[BUFSIZ];
	char prompt[sizeof (struct utsname)];
	register char *p;
	register int id = 0;  /*flag for systemname prompt on tilda escape*/

	CDEBUG(4,MSGSTR(MSG_CUCD1,"transmit started\n\r"),"");
	sysname(prompt);

	/* In main loop, always waiting to read characters from  */
	/* keyboard; writes characters to remote, or to TTYOUT   */
	/* on a tilda escape                                     */

	while(TRUE) {
		p = b;
		while(r_char(TTYIN) == YES) {
		    	register int escape = 0;

			if(p == b)  	/* Escape on leading  ~    */
				escape = (_Cxc == '~');
			else if(p == b+1)   	/* But not on leading ~~   */
				escape &= (_Cxc != '~');
			if(escape) {
			     if(_Cxc == '\n' || _Cxc == '\r' || _Cxc == _Teol) {
					*p = '\0';
					if(tilda(b+1) == YES)
						return(0);
					id = 0;
					break;
				}
				if(_Cxc == _Tintr || _Cxc == _Tkill || _Cxc
					 == _Tquit ||
					    (Intrupt && _Cxc == '\0')) {
					if(!(_Cxc == _Tkill) || Echok)
						VERBOSE("\r\n","");
					break;
				}
				if((p == b+1) && (_Cxc != _Terase) && (!id)) {
					id = 1;
					VERBOSE("[%s]", prompt);
				}
				if(_Cxc == _Terase) { 
					p = (--p < b)? b:p;
					if(p > b)
						if(Echoe)
							VERBOSE("\b \b", "");
						else 
						 	(void)w_char(TTYOUT);
				} else {
					(void)w_char(TTYOUT);
					if(p-b < BUFSIZ) 
						*p++ = _Cxc;
					else {
						VERBOSE(MSGSTR(MSG_CU14,
							P_TOOLONG),"");
						break;
					}
				}
	/*not a tilda escape command*/
			} else {
				if(Intrupt && _Cxc == '\0') {
					CDEBUG(4,MSGSTR(MSG_CUCD2,
					  "got break in transmit\n\r"),"");
					Intrupt = NO;
					(void)ioctl(Cn, TCSBRK, 0);
					_flush();
					break;
				}
				if(w_char(Cn) == NO) {
					VERBOSE(MSGSTR(MSG_CU4,P_LINE_GONE),"");
					return(IOERR);
				}
				if(Duplex == NO)
					if(w_char(TTYERR) == NO)
						return(IOERR);
				if ((_Cxc == _Tintr) || (_Cxc == _Tquit) ||
					 ( (p==b) && (_Cxc == _Myeof) ) ) {
					CDEBUG(4,MSGSTR(MSG_CUCD3,
						"got a tintr\n\r"),"");
					_flush();
					break;
				}
				if(_Cxc == '\n' || _Cxc == '\r' ||
					_Cxc == _Teol || _Cxc == _Tkill) {
					Takeflag = NO;
					break;
				}
				p = (char*)0;
			}
		}
	}
}

/***************************************************************
 *	routine to halt input from remote and flush buffers
 ***************************************************************/
static void
_flush()
{
	(void)ioctl(TTYOUT, TCXONC, 0);	/* stop tty output */
	(void)ioctl(Cn, TCFLSH, 0);		/* flush remote input */
	(void)ioctl(TTYOUT, TCFLSH, 1);	/* flush tty output */
	(void)ioctl(TTYOUT, TCXONC, 1);	/* restart tty output */
	if(Takeflag == NO) {
		return;		/* didn't interupt file transmission */
	}
	VERBOSE(MSGSTR(MSG_CU11,P_FILEINTR),"");
	(void)sleep(3);
	_w_str("echo '\n~>\n';mesg y;stty echo\n");
	Takeflag = NO;
}

/**************************************************************
 *	command interpreter for escape lines
 **************************************************************/
int
tilda(cmd)
register char	*cmd;
{

	VERBOSE("\r\n","");
	CDEBUG(4,MSGSTR(MSG_CUCD4, "call tilda(%s)\r\n"), cmd);

	switch(cmd[0]) {
		case '.':
			if(Cucall.telno == NULL)
				if(cmd[1] != '.') {
					_w_str("\04\04\04\04\04");
					if (Child)
						kill(Child, SIGKILL);
					(void) ioctl (Cn, TCGETA, &_Lv);
        				/* speed to zero for hangup */ 
					_Lv.c_cflag |= (HUPCL | B0);
        				(void) ioctl (Cn, TCSETA, &_Lv);
        				(void) sleep (2);
				}
			return(YES);
		case '!':
			_shell(cmd);	/* local shell */
			VERBOSE("\r%c\r\n", *cmd);
			VERBOSE(MSGSTR(MSG_CUV3,"(continue)"),"");
			break;
		case '$':
			if(cmd[1] == '\0') {
				VERBOSE(MSGSTR(MSG_CU16, P_USECMD),"");
				VERBOSE(MSGSTR(MSG_CUV3, "(continue)"),"");
			}
			else {
				_shell(cmd);	/*Local shell  */
				VERBOSE("\r%c\r\n", *cmd);
			}
			break;	

#ifdef forfutureuse
		case '+':
			if(cmd[1] == '\0') {
				VERBOSE(MSGSTR(MSG_CU17,P_USEPLUSCMD), "");
				VERBOSE(MSGSTR(MSG_CUV3, "(continue)"),"");
			}
			else {
				if (*cmd == '+')
					      /* must suspend receive to give*/
					      /*remote out to stdin of cmd */
					kill(Child, SIGKILL);
					 _shell(cmd);	/* Local shell */
				if (*cmd == '+')
					recfork();
				VERBOSE("\r%c\r\n", *cmd);
			}
			break;
#endif
		case '%':
			_dopercen(++cmd);
			break;
			
		case 't':
			tdmp(TTYIN);
			VERBOSE(MSGSTR(MSG_CUV3, "(continue)"),"");
			break;
		case 'l':
			tdmp(Cn);
			VERBOSE(MSGSTR(MSG_CUV3, "(continue)"),"");
			break;
		
		default:
			VERBOSE(MSGSTR(MSG_CU7, P_STARTWITH),"");
			VERBOSE(MSGSTR(MSG_CUV3, "(continue)"),"");
			break;
	}
	return(NO);
}

/***************************************************************
 *	The routine "shell" takes an argument starting with
 *	either "!" or "$", and terminated with '\0'.
 *	If $arg, arg is the name of a local shell file which
 *	is executed and its output is passed to the remote.
 *	If !arg, we escape to a local shell to execute arg
 *	with output to TTY, and if arg is null, escape to
 *	a local shell and blind the remote line.  In either
 *	case, '^D' will kill the escape status.
 **************************************************************/

#ifdef forfutureuse
/***************************************************************
 *	Another argument to the routine "shell" may be +.  If +arg,
 *	arg is the name of a local shell file which is executed with
 *	stdin from and stdout to the remote.
 **************************************************************/
#endif

static void
_shell(str)
char	*str;
{
	int	fk;
	void	(*xx)(), (*yy)();
       

	CDEBUG(4,MSGSTR(MSG_CUCD5, "call _shell(%s)\r\n"), str);
	fk = dofork();
	if(fk < 0)
		return;
	_mode(0);	/* restore normal tty attributes */
	xx = signal(SIGINT, SIG_IGN);
	yy = signal(SIGQUIT, SIG_IGN);
	if(fk == 0) {
		char *shell, *getenv();
		shell = getenv("SHELL");

		if(shell == NULL)
			shell = _PATH_SH;
					   /*user's shell is set if*/
					   /*different from default*/
		(void)close(TTYOUT);

		/***********************************************
		 * Hook-up our "standard output"
		 * to either the tty for '!' or the line
		 * for '$'  as appropriate
		 ***********************************************/
#ifdef forfutureuse

		/************************************************
		 * Or to the line for '+'.
		 **********************************************/
#endif

		(void)fcntl((*str == '!')? TTYERR:Cn,F_DUPFD,TTYOUT);

#ifdef forfutureuse
		/*************************************************
		 * Hook-up "standard input" to the line for '+'.
		 * **********************************************/
		if (*str == '+')
			{
			(void)close(TTYIN);
			(void)fcntl(Cn,F_DUPFD,TTYIN);
			}
#endif

		/***********************************************
		 * Hook-up our "standard input"
		 * to the tty for '!' and '$'.
		 ***********************************************/

		(void)close(Cn);   	/*parent still has Cn*/
		(void)signal(SIGINT, SIG_DFL);
		(void)signal(SIGHUP, SIG_DFL);
		(void)signal(SIGQUIT, SIG_DFL);
		(void)signal(SIGUSR1, SIG_DFL);
		if(*++str == '\0')
			(void)execl(shell,shell,(char*)0,(char*)0,0);
		else
			(void)execl(shell,"bsh","-c",str,0);
		VERBOSE(MSGSTR(MSG_CU5, P_Ct_EXSH), "");
		exit(0);
	}
	while(wait((int*)0) != fk);
	(void)signal(SIGINT, xx);
	(void)signal(SIGQUIT, yy);
	_mode(1);
}


/***************************************************************
 *	This function implements the 'put', 'take', 'break', and
 *	'nostop' commands which are internal to cu.
 ***************************************************************/

static void
_dopercen(cmd)
register char *cmd;
{
	char	*arg[4];
	char	*getpath, *getenv();
	char	mypath[MAXPATH1];
	int	narg;
	extern	char *strtok();

	blckcnt((long)(-1));

	CDEBUG(4,MSGSTR(MSG_CUCD6, "call _dopercen(\"%s\")\r\n"), cmd);

	arg[narg=0] = strtok(cmd, " \t\n");

		/* following loop breaks out the command and args */
	while((arg[++narg] = strtok((char*) NULL, " \t\n")) != NULL) {
		if(narg < 4)
			continue;
		else
			break;
	}

	/* ~%take file option */
	if(EQUALS(arg[0], "take")) {
		if(narg < 2 || narg > 3) {
			VERBOSE(MSGSTR(MSG_CUV4,"usage: ~%%take from [to]\r\n"),
				"");
			VERBOSE(MSGSTR(MSG_CUV3, "(continue)"),"");
			return;
		}
		if(narg == 2)
			arg[2] = arg[1];

		_w_str("stty -echo;mesg n;echo '~>':");
		_w_str(arg[2]);
		_w_str(";cat ");
		_w_str(arg[1]);
		_w_str(";echo '~>';mesg y;stty echo\n");
		Takeflag = YES;
		return;
	}
	/* ~%put file option*/
	if(EQUALS(arg[0], "put")) {
		FILE	*file;
		char	ch, buf[BUFSIZ], spec[NCC+1], *b, *p, *q;
		int	i, j, len, tc=0, lines=0;
		long	chars=0L;

		if(narg < 2 || narg > 3) {
			VERBOSE(MSGSTR(MSG_CUV5, "usage: ~%%put from [to]\r\n"),
				"");
			VERBOSE(MSGSTR(MSG_CUV3, "(continue)"),"");
			return;
		}
		if(narg == 2)
			arg[2] = arg[1];

		if((file = fopen(arg[1], "r")) == NULL) {
			VERBOSE(MSGSTR(MSG_CU3, P_Ct_OPEN), arg[1]);
			VERBOSE(MSGSTR(MSG_CUV3, "(continue)"),"");
			return;
		}
		_w_str("stty -echo; cat - > ");
		_w_str(arg[2]);
		_w_str("; stty echo\n");
		Intrupt = NO;
		for(i=0,j=0; i < NCC; ++i)
			if((ch=_Tv0.c_cc[i]) != '\0')
				spec[j++] = ch;
		spec[j] = '\0';
		_mode(2);	/*accept interrupts from keyboard*/
		(void)sleep(5);	/*hope that w_str info digested*/

/* Read characters line by line into buf to write to remote with character*/
/*and line count for blckcnt                                              */
		while(Intrupt == NO &&
				fgets(b= &buf[MID],MID,file) != NULL) {
/*worse case= each*/
/*char must be escaped*/
			len = strlen(b);
			chars += len;		/* character count */
			p = b;
			while(q = strpbrk(p, spec)) {
				if(*q == _Tintr || *q == _Tquit ||
							*q == _Teol) {
					VERBOSE(MSGSTR(MSG_CU13, P_Ct_SPECIAL),
						 *q);
					(void)strcpy(q, q+1);
					Intrupt = YES;
				}
				else {
				b = strncpy(b-1, b, q-b);
				*(q-1) = '\\';
				}
			p = q+1;
			}
			if((tc += len) >= MID) {
				(void)sleep(1);
				tc = len;
			}
			if(write(Cn, b, (unsigned)strlen(b)) < 0) {
				VERBOSE(MSGSTR(MSG_CU15, P_IOERR),"");
				Intrupt = YES;
				break;
			}
			++lines;		/* line count */
			blckcnt((long)chars);
		}
		_mode(1);
		blckcnt((long)(-2));		/* close */
		(void)fclose(file);
		if(Intrupt == YES) {
			Intrupt = NO;
			VERBOSE(MSGSTR(MSG_CU11,P_FILEINTR),"");
			_w_str("\n");
			VERBOSE(MSGSTR(MSG_CU8, P_CNTAFTER), ++chars);
		} else
			VERBOSE(MSGSTR(MSG_CU9, P_CNTLINES), lines);
			VERBOSE(MSGSTR(MSG_CU10, P_CNTCHAR), chars);
		_w_str("\04");
		(void)sleep(3);
		return;
	}

		/*  ~%b or ~%break  */
	if(EQUALS(arg[0], "b") || EQUALS(arg[0], "break")) {
		(void)ioctl(Cn, TCSBRK, 0);
		return;
	}
		/*  ~%d or ~%debug toggle  */
	if(EQUALS(arg[0], "d") || EQUALS(arg[0], "debug")) {
		if(Debug == 0)
			Debug = 9;
		else
			Debug = 0;
		VERBOSE(MSGSTR(MSG_CUV3, "(continue)"),"");
		return;
	}
		/*  ~%nostop  toggles start/stop input control  */
	if(EQUALS(arg[0], "nostop")) {
		(void)ioctl(Cn, TCGETA, &_Tv);
		if(Sstop == NO)
			_Tv.c_iflag |= IXOFF;
		else
			_Tv.c_iflag &= ~IXOFF;
		(void)ioctl(Cn, TCSETAW, &_Tv);
		Sstop = !Sstop;
		_mode(1);
		VERBOSE(MSGSTR(MSG_CUV3, "(continue)"),"");
		return;
	}
		/* Change local current directory */
	if(EQUALS(arg[0], "cd")) {
		if (narg < 2) {
			getpath = getenv("HOME");
			strcpy(mypath, getpath);
			if(chdir(mypath) < 0) {
				VERBOSE(MSGSTR(MSG_CUV6, 
				  "Cannot change to %s\r\n"), mypath);
				VERBOSE(MSGSTR(MSG_CUV3, "(continue)"),"");
				return;
			}
		}
		else if (chdir(arg[1]) < 0) {
			VERBOSE(MSGSTR(MSG_CUV7,"Cannot change to %s\r\n"), 
				arg[1]);
			VERBOSE(MSGSTR(MSG_CUV3, "(continue)"),"");
			return;
		}
		recfork();	/* fork a new child so it know about change */
		VERBOSE(MSGSTR(MSG_CUV3, "(continue)"),"");
		return;
	}
	VERBOSE(MSGSTR(MSG_CUV12,"~%%%s unknown to cu\r\n"), arg[0]);
	VERBOSE(MSGSTR(MSG_CUV3, "(continue)"),"");
}

/***************************************************************
 *	receive: read from remote line, write to fd=1 (TTYOUT)
 *	catch:
 *	~>[>]:file
 *	.
 *	. stuff for file
 *	.
 *	~>	(ends diversion)
 ***************************************************************/

static void
_receive()
{
	register silent=NO, file;
	register char *p;
	int	tic = 0;
	char	b[BUFSIZ];
	long	count = 0;

	CDEBUG(4,MSGSTR(MSG_CUCD8, "_receive started\r\n"),"");

	b[0] = '\0';
	file = -1;
	p = b;

	while(r_char(Cn) == YES) {
		if(silent == NO)	/* ie., if not redirecting to file*/
			if(w_char(TTYOUT) == NO) 
				_rcvdead(IOERR);	/* this will exit */
		/* remove CR's and fill inserted by remote */
		if(_Cxc == '\0' || _Cxc == RUB || _Cxc == '\r')
			continue;
		*p++ = _Cxc;
		if(_Cxc != '\n' && (p-b) < BUFSIZ)
			continue;
		/***********************************************
		 * The rest of this code is to deal with what
		 * happens at the beginning, middle or end of
		 * a diversion to a file.
		 ************************************************/
		if(b[0] == '~' && b[1] == '>') {
			/****************************************
			 * The line is the beginning or
			 * end of a diversion to a file.
			 ****************************************/
			if((file < 0) && (b[2] == ':' || b[2] == '>')) {
				/**********************************
				 * Beginning of a diversion
				 *********************************/
				int	append;

				*(p-1) = NULL; /* terminate file name */
				append = (b[2] == '>')? 1:0;
				p = b + 3 + append;
				if(append && (file=open(p,O_WRONLY))>0)
					(void)lseek(file, 0L, 2);
				else
					file = creat(p, 0666);
				if(file < 0) {
					VERBOSE(MSGSTR(MSG_CU6, P_Ct_DIVERT),p);
					perror("");
					(void)sleep(5); /* 10 seemed too long*/
				} else {
					silent = YES; 
					count = tic = 0;
				}
			} else {
				/*******************************
				 * End of a diversion (or queer data)
				 *******************************/
				if(b[2] != '\n')
					goto D;		/* queer data */
				if(silent = close(file)) {
					VERBOSE(MSGSTR(MSG_CU6, P_Ct_DIVERT),b);
					silent = NO;
				}
				blckcnt((long)(-2));
				VERBOSE("~>\r\n","");
				VERBOSE(MSGSTR(MSG_CU9, P_CNTLINES), tic);
				VERBOSE(MSGSTR(MSG_CU10,P_CNTCHAR), count);
				file = -1;
			}
		} else {
			/***************************************
			 * This line is not an escape line.
			 * Either no diversion; or else yes, and
			 * we've got to divert the line to the file.
			 ***************************************/
D:
			if(file > 0) {
				(void)write(file, b, (unsigned)(p-b));
				count += p-b;	/* tally char count */
				++tic;		/* tally lines */
				blckcnt((long)count);
			}
		}
		p = b;
	}
	VERBOSE(MSGSTR(MSG_CUV13, "\r\nLost Carrier\r\n"),"");
	_rcvdead(IOERR);
}

/***************************************************************
 *	change the TTY attributes of the users terminal:
 *	0 means restore attributes to pre-cu status.
 *	1 means set `raw' mode for use during cu session.
 *	2 means like 1 but accept interrupts from the keyboard.
 ***************************************************************/
static void
_mode(int arg)
{
	CDEBUG(4,MSGSTR(MSG_CUCD9, "call _mode(%d)\r\n"), arg);
	if(arg == 0) {
		(void)ioctl(TTYIN, TCSETAW, &_Tv0);
	} else {
		(void)ioctl(TTYIN, TCGETA, &_Tv);
		if(arg == 1) {
			_Tv.c_iflag &= ~(INLCR | ICRNL | IGNCR |
						IXOFF | IUCLC);
			_Tv.c_oflag |= OPOST;
			_Tv.c_oflag &= ~(OLCUC | ONLCR | OCRNL | ONOCR | ONLRET);
			_Tv.c_lflag &= ~(ICANON | ISIG | ECHO);
			if(Sstop == NO)
				_Tv.c_iflag &= ~IXON;
			else
				_Tv.c_iflag |= IXON;
			if(Terminal) {
				_Tv.c_oflag |= ONLCR;
				_Tv.c_iflag |= ICRNL;
			}
			_Tv.c_cc[VEOF] = '\01';
			_Tv.c_cc[VEOL] = '\0';
		}
		if(arg == 2) {
			_Tv.c_iflag |= IXON;
			_Tv.c_lflag |= ISIG;
		}
		(void)ioctl(TTYIN, TCSETAW, &_Tv);
	}
}


static int
dofork()
{
	register int x,i;

	for(i = 0; i < 6; ++i) {
		if((x = fork()) >= 0) {
			return(x);
		}
	}

	if(Debug) perror("dofork");

	VERBOSE(MSGSTR(MSG_CU12, P_Ct_FK),"");
	return(x);
}

static int
r_char(int fd)
{
	int rtn;

	while((rtn = read(fd, &_Cxc, 1)) < 0){
		if(errno == EINTR)
	/* onintrpt() called asynchronously before this line */
			if(Intrupt == YES) {
				_Cxc = '\0';	/* got a BREAK */
				return(YES);
			} else
				continue;	/*a signal other than*/ 
					    /*interrupt received during read*/
		else {
			CDEBUG(4,MSGSTR(MSG_CUCD10, 
				"got read error, not EINTR\n\r"),"");
			break;			/* something wrong */
		}
	}
	return(rtn == 1? YES: NO);	
}

static int
w_char(int fd)
{
	int rtn;

	while((rtn = write(fd, &_Cxc, 1)) < 0)
		if(errno == EINTR)
			if(Intrupt == YES) {
				VERBOSE(MSGSTR(MSG_CUV8, 
				   "\ncu: Output blocked\r\n"),"");
				_quit(IOERR);
			} else
				continue;	/* alarm went off */
		else
			break;			/* bad news */
	return(rtn == 1? YES: NO);
}



static void
_w_str(string)
register char *string;
{
	int len;

	len = strlen(string);
	if(write(Cn, string, (unsigned)len) != len)
		VERBOSE(MSGSTR(MSG_CU4, P_LINE_GONE), "");
}

static void
_onintrpt()
{
	(void)signal(SIGINT, _onintrpt);
	(void)signal(SIGQUIT, _onintrpt);
	Intrupt = YES;
}

static void
_rcvdead(arg)	/* this is executed only in the receive process */
int arg;
{
	CDEBUG(4,MSGSTR(MSG_CUCD11, "call _rcvdead(%d)\r\n"), arg);
	(void)kill(getppid(), SIGUSR1);
	exit((arg == SIGHUP)? SIGHUP: arg);
	/*NOTREACHED*/
}

static void
_quit(arg)	/* this is executed only in the parent process */
int arg;
{
	CDEBUG(4,MSGSTR(MSG_CUCD12, "call _quit(%d)\r\n"), arg);
	(void)kill(Child, SIGKILL);
	_bye(arg);
	/*NOTREACHED*/
}

static void
_bye(arg)	/* this is executed only in the parent proccess */
int arg;
{
	int status;

	CDEBUG(4,MSGSTR(MSG_CUCD13, "call _bye(%d)\r\n"), arg);

	(void)signal(SIGINT, SIG_IGN);
	(void)signal(SIGQUIT, SIG_IGN);
	(void)wait(&status);
	VERBOSE(MSGSTR(MSG_CUV9,"\r\nDisconnected\007\r\n"),"");
	cleanup((arg == SIGUSR1)? (status >>= 8): arg);
	/*NOTREACHED*/
}



void
cleanup(int code) 	/*this is executed only in the parent process*/
{

	CDEBUG(4,MSGSTR(MSG_CUCD14, "call cleanup(%d)\r\n"), code);

	(void) setuid(Euid);
	/* Because the ioctl in tilda() doesn't work in AIX v2,
	   we set HUPCL here.  */
	sethup(Cn);
	if(Cn > 0) {
		chown(_Cnname, 0, 0);       /* give the TTY back to root */
		chmod(_Cnname, 0666);
		(void)close(Cn);
	}


	clrlock((char*) NULL);	/*uucp routine in ulockf.c*/	
	_mode(0);		/*which removes lock files*/
	exit(code);		/* code=negative for signal causing disconnect*/
}



void
tdmp(int arg)
{

	struct termio xv;
	int i;

	VERBOSE(MSGSTR(MSG_CUV10, "\rdevice status for fd=%d\n"), arg);
	VERBOSE(MSGSTR(MSG_CUV11, "\rF_GETFL=%o,"), fcntl(arg, F_GETFL,1));
	if(ioctl(arg, TCGETA, &xv) < 0) {
		char	buf[100];
		i = errno;
		(void)sprintf(buf, "\rtdmp for fd=%d", arg);
		errno = i;
		perror(buf);
		return;
	}
	VERBOSE("iflag=`%o',", xv.c_iflag);
	VERBOSE("oflag=`%o',", xv.c_oflag);
	VERBOSE("cflag=`%o',", xv.c_cflag);
	VERBOSE("lflag=`%o',", xv.c_lflag);
	VERBOSE("line=`%o'\r\n", xv.c_line);
	VERBOSE("cc[0]=`%o',",  xv.c_cc[0]);
	for(i=1; i<8; ++i) {
		VERBOSE("[%d]=", i);
		VERBOSE("`%o' , ",xv.c_cc[i]);
	}
	VERBOSE("\r\n","");
}


void
sysname(char *name)
{

	register char *s;

	if(uname(&utsn) < 0)
		s = "Local";
	else
		s = utsn.nodename;

	strncpy(name,s,strlen(s) + 1);
	*(name + strlen(s) + 1) = '\0';
	return;
}

void
blckcnt(int count)
{
	static long lcharcnt = 0;
	register long c1, c2;
	register int i;
	char c;

	if(count == (long) (-1)) {       /* initialization call */
		lcharcnt = 0;
		return;
	}
	c1 = lcharcnt/BUFSIZ;
	if(count != (long)(-2)) {	/* regular call */
		c2 = count/BUFSIZ;
		for(i = c1; i++ < c2;) {
			c = '0' + i%10;
			write(2, &c, 1);
			if(i%NPL == 0)
				write(2, "\n\r", 2);
		}
		lcharcnt = count;
	}
	else {
		c2 = (lcharcnt + BUFSIZ -1)/BUFSIZ;
		if(c1 != c2)
			write(2, "+\n\r", 3);
		else if(c2%NPL != 0)
			write(2, "\n\r", 2);
		lcharcnt = 0;
	}
}

void assert(){}	/* for ASSERT in gnamef.c */
void logent(){}		/* so we can load ulockf() */






