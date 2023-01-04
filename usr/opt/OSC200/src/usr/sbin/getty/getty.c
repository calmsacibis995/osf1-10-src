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
static char	*sccsid = "@(#)$RCSfile: getty.c,v $ $Revision: 4.3.11.3 $ (DEC) $Date: 1993/10/08 16:21:03 $";
#endif 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0
 */
/*
 * getty.c
 *
 *	Revision History:
 *
 * 10-Oct-91	Mike Larson
 *	8-bit fixes: mask input characters based on character size.
 *	Fix setting of character size in fields().
 *
 * 21-Aug-91	Mike Larson
 *	Set baud rate in setupline().
 *
 * 12-Mar-91	Fred Canter
 *	MIPS C 2.20+
 *
 */



/*	getty - sets up speed, various terminal flags, line discipline,	*/
/*	and waits for new prospective user to enter name, before	*/
/*	calling "login".						*/
/*									*/
/*	Usage:	getty [-h] [-t time] line speed_label terminal ldisc	*/
/*									*/
/*	-h says don't hangup by dropping carrier during the		*/
/*		initialization phase.  Normally carrier is dropped to	*/
/*		make the dataswitch release the line.			*/
/*	-t says timeout after the number of seconds in "time" have	*/
/*		elapsed even if nothing is typed.  This is useful	*/
/*		for making sure dialup lines release if someone calls	*/
/*		in and then doesn't actually login in.			*/
/*	"line" is the device in "/dev".					*/
/*	"speed_label" is a pointer into the "/etc/getty_defs"		*/
/*			where the definition for the speeds and		*/
/*			other associated flags are to be found.		*/
/*	"terminal" is the name of the terminal type.			*/
/*	"ldisc"    is the name of the line discipline.			*/
/*									*/
/*	Usage:  getty -c gettydefs_like_file				*/
/*									*/
/*	The "-c" flag is used to have "getty" check a gettydefs file.	*/
/*	"getty" parses the entire file and prints out its findings so	*/
/*	that the user can make sure that the file contains the proper	*/
/*	information.							*/
/*									*/


#include	<stdio.h>
#include	<fcntl.h>
#include	<unistd.h>
#include	<sys/types.h>
#define TTYDEFCHARS
#include	<sys/termios.h>
#include	<sys/ioctl.h>
#include	<signal.h>
#include	<sys/stat.h>
#include	<utmp.h>
#include	<sys/utsname.h>
#include	<ctype.h>
#ifdef NLS
#include <locale.h>
#endif

#ifdef MSG
#include "getty_msg.h"
nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_GETTY,n,s)
#else
#define MSGSTR(n,s) s
#endif

#if defined(KJI) || defined(NLS)
#include <NLchar.h>
#include <NLctype.h>
#endif


#define		TRUE		1
#define		FALSE		0

#define		FAILURE		-1
#define		SUCCESS		0
#define		ID		1
#define		IFLAGS		2
#define		FFLAGS		3
#define		MESSAGE		4
#define		NEXTID		5

#define		ACTIVE		1
#define		FINISHED	0

#define		ABORT		control('C')	/* ^C */
#define		QUIT		control('\\')	/* ^\ */
#define		ERASE		0177		/* Delete */
#define		BACKSPACE	'\b'
#define		KILL		control('U')	/* ^U */

#define		control(x)	(x&037)

#define		GOODNAME	1
#define		NONAME		0
#define		BADSPEED	-1

struct Gdef {
	char	*g_id;	/* identification for modes & speeds */
	struct termios	g_iflags;	/* initial terminal flags */
	struct termios	g_fflags;	/* final terminal flags */
	char	*g_message;	/* login message */
	char	*g_nextid;	/* next id if this speed is wrong */
};

#define	MAXIDLENGTH	15	/* Maximum length the "g_id" and "g_nextid"
				 * strings can take.  Longer ones will be
				 * truncated.
				 */
#define MAXMESSAGE	79	/* Maximum length the "g_message" string
				 * can be.  Longer ones are truncated.
				 */

/*	Maximum length of line in /etc/gettydefs file and the maximum	*/
/*	length of the user response to the "login" message.		*/

#define	MAXLINE		255
#define	MAXARGS		64	/* Maximum number of arguments that can be
				 * passed to "login"
				 */

struct Symbols {
	char		*s_symbol;	/* Name of symbol */
	unsigned	s_value	;	/* Value of symbol */
};

/*	The following four symbols define the "SANE" state.		*/

#define	ISANE	(BRKINT|IGNPAR|ICRNL|IXON)
#define	OSANE	(OPOST|ONLCR)
#define	CSANE	(CS8|CREAD)
#define	LSANE	(ISIG|ICANON|IEXTEN|ECHO|ECHOK)

/*	Modes set with the tcsetattr() .			*/

struct Symbols imodes[] = {
	"IGNBRK",	IGNBRK,
	"BRKINT",	BRKINT,
	"IGNPAR",	IGNPAR,
	"PARMRK",	PARMRK,
	"INPCK",	INPCK,
	"ISTRIP",	ISTRIP,
	"INLCR",	INLCR,
	"IGNCR",	IGNCR,
	"ICRNL",	ICRNL,
	"IUCLC",	IUCLC,
	"IXON",		IXON,
	"IXANY",	IXANY,
	"IXOFF",	IXOFF,
	"IMAXBEL",	IMAXBEL,
	NULL,	0
};

struct Symbols omodes[] = {
	"OPOST",	OPOST,
	"OLCUC",	OLCUC,
	"ONLCR",	ONLCR,
	"OCRNL",	OCRNL,
	"ONOCR",	ONOCR,
	"ONLRET",	ONLRET,
	"OFILL",	OFILL,
	"OFDEL",	OFDEL,
	"NLDLY",	NLDLY,
	"NL0",		NL0,
	"NL1",		NL1,
	"CRDLY",	CRDLY,
	"CR0",		CR0,
	"CR1",		CR1,
	"CR2",		CR2,
	"CR3",		CR3,
	"TABDLY",	TABDLY,
	"TAB0",		TAB0,
	"TAB1",		TAB1,
	"TAB2",		TAB2,
	"TAB3",		TAB3,
	"BSDLY",	BSDLY,
	"BS0",		BS0,
	"BS1",		BS1,
	"VTDLY",	VTDLY,
	"VT0",		VT0,
	"VT1",		VT1,
	"FFDLY",	FFDLY,
	"FF0",		FF0,
	"FF1",		FF1,
	NULL,	0
};

struct Symbols speeds[] = {
	"B0",		B0,
	"B50",		B50,
	"B75", 		B75,
	"B110",		B110,
	"B134",		B134,
	"B150",		B150,
	"B200",		B200,
	"B300",		B300,
	"B600",		B600,
	"B1200",	B1200,
	"B1800",	B1800,
	"B2400",	B2400,
	"B4800",	B4800,
	"B9600",	B9600,
	"B19200",	B19200,
	"B38400",	B38400,
	"EXTA",		EXTA,
	"EXTB",		EXTB,
};

struct Symbols cmodes[] = {
	"CS5",		CS5,
	"CS6",		CS6,
	"CS7",		CS7,
	"CS8",		CS8,
	"CSTOPB",	CSTOPB,
	"CREAD",	CREAD,
	"PARENB",	PARENB,
	"PARODD",	PARODD,
	"HUPCL",	HUPCL,
	"CLOCAL",	CLOCAL,
	NULL,	0
};

struct Symbols lmodes[] = {
	"ISIG",		ISIG,
	"ICANON",	ICANON,
	"IEXTEN",	IEXTEN,
	"XCASE",	XCASE,
	"ECHO",		ECHO,
	"ECHOE",	ECHOE,
	"ECHOK",	ECHOK,
	"ECHOKE",	ECHOKE,
	"ECHOPRT",	ECHOPRT,
	"ECHOCTL",	ECHOCTL,
	"ECHONL",	ECHONL,
	"ALTWERASE",	ALTWERASE,
	"NOFLSH",	NOFLSH,
	NULL,	0
};

struct Symbols linedisc[] = {
	"POSIX",	0,
	NULL,		0
};

char *term;
char *env[2] = {NULL, NULL};
char editedhost[64];

/*	If the /etc/gettydefs file can't be opened, the following	*/
/*	default is used.						*/

struct Gdef DEFAULT = {
	"default",
	{ICRNL,0,CREAD+HUPCL,0,
	     {0,0,0,CERASE,0,CKILL,0,CINTR, CQUIT}, B300, B300},
	{ICRNL,OPOST+ONLCR+NLDLY+TAB3,CS8+CREAD+HUPCL,
	     ISIG+ICANON+IEXTEN+ECHO+ECHOE+ECHOK,
	     {0,0,0,CERASE,0,CKILL,0,CINTR, CQUIT}, B300, B300},
	"LOGIN: ",
	"default"
};

char	*CTTY		=	"/dev/console";
char	*GETTY_DEFS	=	"/etc/gettydefs";

int 	check;

char	*checkgdfile;		/* Name of gettydefs file during
				 * check mode.
				 */

int check_errors = 0; 		/* error count for check mode. paw: qar 6383 */

main(argc,argv)
int argc;
char **argv;
{
	char *line;
	char *cp;
	register struct Gdef *speedef;
	char oldspeed[MAXIDLENGTH+1],newspeed[MAXIDLENGTH+1];
	extern struct Gdef *find_def();
	int lined;
	extern char *GETTY_DEFS;
	int hangup,timeout;
	extern struct Symbols *search();
	extern void timedout();
	register struct Symbols *answer;
	char user[MAXLINE],*largs[MAXARGS],*ptr,buffer[MAXLINE];
	register int i;
	register int illegal = 0;
	FILE *fp;
	struct utsname utsname;
	struct termios termios;
	extern char *malloc();
#ifdef NLS
        (void) setlocale( LC_ALL, "" );
#endif

#ifdef MSG
        catd = catopen(MF_GETTY,NL_CAT_LOCALE);
#endif

	signal(SIGINT,SIG_IGN);
	signal(SIGQUIT,SIG_DFL);

	gethostname(editedhost, sizeof(editedhost));
	if (editedhost[0] == '\0')
		strcpy(editedhost, MSGSTR(MSG1, "Amnesiac"));
	hangup = TRUE;
	timeout = 0;
	while((--argc > 0) && **++argv == '-') {
		for(ptr = *argv + 1; *ptr;ptr++) switch(*ptr) {
		case 'h':
			hangup = FALSE;
			break;
		case 't':

			if(isdigit(*++ptr)) {
				sscanf(ptr,"%d",&timeout);

/* Advance "ptr" so that it is pointing to the last digit of the */
/* timeout argument. */
				while(isdigit(*++ptr));
				ptr--;
				break;
			} else if(--argc) 
				if(isdigit(*(ptr = *++argv))) {
					sscanf(ptr,"%d",&timeout);
					break;
				}
		/*
                 * if the time argument is invalid
                 * write error msg, sleep for 20 sec
                 * so not get give init a hard time
                 * then exit - RTM-01
                 */
			error(MSGSTR(TIMEOUT, "getty: timeout argument invalid. \"%s\"\n"), *argv);
			sleep(20);
			exit(1);

/* Check a "gettydefs" file mode. */
		case 'c':
			signal(SIGINT,SIG_DFL);
			if(--argc == 0) {
				goto c_error;
			}
			check = TRUE;
			checkgdfile = *++argv;

/* Attempt to open the check gettydefs file. */
			if((fp = fopen(checkgdfile,"r")) == NULL) {
				fprintf(stderr,MSGSTR(CANTOPEN, "Cannot open %s\n"),checkgdfile);
				exit(1);
			}
			fclose(fp);

/* Call "find_def" to check the check file.  With the "check" flag */
/* set, it will parse the entire file, printing out the results. */
			find_def(NULL);
			exit(check_errors > 0 ? 2 : 0); /* paw: qar 6383 */
		default:
			fprintf(stderr,
				MSGSTR(USAGE, "Usage: getty [-h] [-t time] line speed-label terminal-type [line-discipline]\n"));
c_error:		fprintf(stderr,
				MSGSTR(CHECK_USG, "Check Mode Usage: getty -c gettydefs-like-file\n"));
			exit(1);
		}
	}

/* There must be at least one argument.  If there isn't, complain */
/* and then die after 20 seconds.  The 20 second sleep is to keep */
/* "init" from working too hard. */
	if (argc < 1) {
		error(MSGSTR(NO_TERMINAL, "getty: no terminal line specified.\n"));
		sleep(20);
		exit(1);
	} else line = *argv;

/* If a "speed_label" was provided, search for it in the */
/* "getty_defs" file.  If none was provided, take the first entry */
/* of the "getty_defs" file as the initial settings. */
	if (--argc > 0 ) {
		if((speedef = find_def(*++argv)) == NULL) {
			error(MSGSTR(UNABLE, "getty: unable to find %s in \"%s\".\n"),
			    *argv,GETTY_DEFS);

/* Use the default value instead. */
			speedef = find_def(NULL);
		}
	} else
		speedef = find_def(NULL);
	/*
	 * Use the terminal type, if specified, to set the TERM
	 * environment variable.
	 */
	if (--argc > 0)
		cp = *++argv;
	else
		cp = MSGSTR(UNKNOWN, "unknown");

	term = malloc(strlen(cp) + 6);
	strcpy(term, "TERM=");
	strcat(term, cp);

	/* If a line discipline was supplied, try to find it in list */
	lined = 0;
	if (--argc > 0)
		if ((answer = search(*++argv, linedisc)) == NULL)
			error(MSGSTR(LD_UNKNOWN, "getty: %s is an unknown line discipline.\n"), *argv);
		else
			lined = answer->s_value;

	/* Perform "utmp" accounting. */
	account(line);

	/*
	 * Attempt to open standard input, output,
	 * and error on specified line.
	 */
	chdir("/dev");
	openline(line, speedef, lined, hangup);

/* Loop until user is successful in requesting login. */
	for(;;) {
		putstr("\r\n");
		/*
		 * If getty is supposed to die if no one logs in after a
		 * predetermined amount of time, set the timer.
		 */
		if(timeout) {
			signal(SIGALRM,timedout);
			alarm(timeout);
		}

#ifdef SYS_NAME
/* Generate a message with the system identification in it. */
		if (uname(&utsname) != FAILURE) {
			sprintf(buffer,"%.9s\r\n", utsname.nodename);

#ifdef	UPPERCASE_ONLY
/* Make all the alphabetics upper case. */
			for (ptr= buffer; *ptr;ptr++) *ptr = tolower(*ptr);
#endif
			fputs(buffer,stderr);
		}
#endif

		/* Print the login message. */
		writemsg(speedef->g_message);

		/* Get the user's typed response and respond appropriately. */
		switch(getname(user,&termios)) {
		case GOODNAME:
			if (timeout) alarm(0);

			termios.c_iflag |= speedef->g_fflags.c_iflag;
			termios.c_oflag |= speedef->g_fflags.c_oflag;
			termios.c_cflag |= speedef->g_fflags.c_cflag;
			termios.c_lflag |= speedef->g_fflags.c_lflag;
			if (ioctl(0, TIOCSETD, &lined) < 0)
				error(MSGSTR(CANT_SET_LD, "Cannot set discipline %d\n"), lined);
			tcsetattr(0, TCSADRAIN, &termios);

			/*
			 * Parse the input line from the user,
			 * breaking it at white spaces.
			 */
			largs[0] = "login";
			parse(user,&largs[1],MAXARGS-1);

			/*
			 * Check for potential security violation.
			 * Don't allow a '-' as the first character of a
			 * login argument.  Someone might try to type 
			 * "-f <name>" as a name and gain <name>'s privilege 
			 * without the passwd.
			 */
			for (i=1, illegal=0; ((i<MAXARGS) && 
					      (largs[i] != NULL)); i++) {
				if (largs[i][0] == '-')
					illegal++;
			}
			if (illegal) {
				writemsg("Illegal login name.\r\n");
				break;
			}

			env[0] = term;

#ifndef DEBUG
			execve("/usr/bin/login", largs, env);
			exit(1);
#else
			exit(0);
#endif /* DEBUG */


/* If the speed supplied was bad, try the next speed in the list. */
		case BADSPEED:

/* Save the name of the old speed definition incase new one is */
/* bad.  Copy the new speed out of the static so that "find_def" */
/* won't overwrite it in the process of looking for new entry. */
			strcpy(oldspeed,speedef->g_id);
			strcpy(newspeed,speedef->g_nextid);
			if ((speedef = find_def(newspeed)) == NULL) {
				error(MSGSTR(BAD_ENTRY, "getty: pointer to next speed in entry %s is bad.\n"),
				oldspeed);

/* In case of error, go back to the original entry. */
				if((speedef = find_def(oldspeed)) == NULL) {

/* If the old entry has disappeared, then quit and let next "getty" try. */
					error(MSGSTR(UNABLE_AGAIN, "getty: unable to find %s again.\n"),
						oldspeed);
					exit(1);
				}
			}

			/* Setup the terminal for the new information. */
			setupline(speedef, lined);
			break;

/* If no name was supplied, not nothing, but try again. */
		case NONAME:
			break;
		}
	}
}

account(line)
char *line;
{
	register int ownpid;
	register struct utmp *u;
	extern struct utmp *getutent(), *pututline();
	register FILE *fp;

	/* Look in "utmp" file for our own entry and change it to LOGIN. */
	ownpid = getpid();

	while ((u = getutent()) != NULL) {

		/* Is this our own entry? */
		if (u->ut_type == INIT_PROCESS && u->ut_pid == ownpid) {
			strncpy(u->ut_line,line,sizeof(u->ut_line));
			strncpy(u->ut_user,"LOGIN",sizeof(u->ut_user));
			u->ut_type = LOGIN_PROCESS;

			/* Write out the updated entry. */
			pututline(u);
			break;
		}
	}
	/*
	 * If we were successful in finding an entry for ourself in the
	 * utmp file, then attempt to append to the end of the wtmp file.
	 */
	if (u != NULL && (fp = fopen(WTMP_FILE,"r+")) != NULL) {
		fseek(fp,0L,2);	/* Seek to end of file */
		fwrite(u,sizeof(*u),1,fp);
		fclose(fp);
	}

	/* Close the utmp file. */
	endutent();
}

/*	"search" scans through a table of Symbols trying to find a	*/
/*	match for the supplied string.  If it does, it returns the	*/
/*	pointer to the Symbols structure, otherwise it returns NULL.	*/

struct Symbols *search(target,symbols)
register char *target;
register struct Symbols *symbols;
{
	/*
	 * Each symbol array terminates with a null pointer for an
	 * "s_symbol".  Scan until a match is found, or the null pointer
	 * is reached.
	 */
	for (;symbols->s_symbol != NULL; symbols++)
	{
		if (strcmp(target,symbols->s_symbol) == 0) return(symbols);
	}
	return(NULL);
}

error(format,arg1,arg2,arg3,arg4)
char *format;
caddr_t arg1,arg2,arg3,arg4;
{
	register FILE *fp;

	if ((fp = fopen(CTTY,"w")) != NULL) {
		fprintf(fp,format,arg1,arg2,arg3,arg4);
		fclose(fp);
	}
}

openline(line,speedef, lined, hangup)
register char *line;
register struct Gdef *speedef;
int lined;
int hangup;
{
	struct stat statb;
	extern int errno;

	close(0);
	close(1);
	close(2);

#if SEC_BASE
	if (security_is_on())
        	getty_condition_line(line);
	else
#endif /* SEC_BASE */
	{
	/*
	 * Change the ownership of the terminal line to root and set
	 * the protections to only allow root to read the line.
	 */
	stat(line,&statb);
	chown(line,0,statb.st_gid);
	chmod(line,0622);
	revoke(line);
	}

	/*
	 * Attempt to open the line.  It should become "stdin".  If not,
	 * then close.
	 */
	if (open(line,O_RDWR) < 0) {
		error(MSGSTR(CANT_OPN1, "getty: cannot open \"%s\". errno: %d\n"),line,errno);
		sleep(20);
		exit(1);
	}
	(void) dup(0);
	(void) dup(1);

	/*
	 * Create a controlling session if init did not spawn getty
	 */
	if ( !( getsid() == getpid() ))
		setsid();  
		
	if (ioctl(0, TIOCSCTTY, (char *) 0) < 0)
		error(MSGSTR(CANT_ASSIGN, "getty: cannot assign controlling tty: errno = %d\n"),
								errno);
	/*
	 * Unless getty is being invoked by ct, make sure that DTR has been
	 * dropped and reasserted
	 */
	if (hangup) hang_up_line();

	setupline(speedef, lined);

}

#ifdef HANGUP

hang_up_line()
{
	struct termio termios;

	tcgetattr(0, &termios);
	termios.c_cflag &= ~CBAUD;
	termios.c_cflag |= B0;
	tcsetattr(0, TCSAFLUSH, &termios);
	sleep(1);
}
#else
hang_up_line()
{
}
#endif

void
timedout()
{
	exit(1);
}

setupline(speedef, lined)
register struct Gdef *speedef;
int lined;
{
	struct termios termios;
	unsigned short timer;

	if (ioctl(0, TIOCSETD, &lined) < 0)
		error(MSGSTR(CANT_SET_LD, "Cannot set discipline %d\n"), lined);

	/* Get the current state of the modes and such for the terminal. */
	tcgetattr(0, &termios);
	
	termios.c_iflag = speedef->g_iflags.c_iflag;
	termios.c_oflag = speedef->g_iflags.c_oflag;
	termios.c_cflag = speedef->g_iflags.c_cflag;
	termios.c_lflag = speedef->g_iflags.c_lflag;
	/*
	 * Make sure that raw reads are 1 character at a time with no
	 * timeout.
	 */
	termios.c_cc[VMIN] = 1;
	termios.c_cc[VTIME] = 0;
	/*
	 * Set initial input and output speeds.
	 */
	termios.c_ispeed = speedef->g_iflags.c_ispeed;
	termios.c_ospeed = speedef->g_iflags.c_ospeed;

	tcsetattr(0, TCSAFLUSH, &termios);

	/* Pause briefly while terminal settles. */
	for(timer=0; ++timer != 0;);
}

/*	"getname" picks up the user's name from the standard input.	*/
/*	It makes certain						*/
/*	determinations about the modes that should be set up for the	*/
/*	terminal depending upon what it sees.  If it sees all UPPER	*/
/*	case characters, it sets the IUCLC & OLCUC flags.  If it sees	*/
/*	a line terminated with a <linefeed>, it sets ICRNL.  If it sees	*/
/*	the user using the "standard" OSS erase, kill, abort, or line	*/
/*	termination characters ( '_','$','&','/','!' respectively)	*/
/*	it resets the erase, kill, and end of line characters.		*/

int getname(user,termios)
    char *user;
    struct termios *termios;
{
	register char *ptr;
	register int c;
	char rawc;
	int upper,lower;
	unsigned char charmask;

	/* Get the previous modes, erase, and kill characters and speeds. */
	tcgetattr(0, termios);
	if ((termios->c_cflag & CSIZE) == CS8)
		charmask = 0xff;
	else
		charmask = 0x7f;

	/* Set the flags to 0 and set the standard erase and kill characters */
	termios->c_iflag &= ICRNL;
	termios->c_oflag = 0;
	termios->c_cflag = 0;
	termios->c_lflag &= ECHO;
	memcpy(termios->c_cc, ttydefchars, NCCS);
	ptr = user;
	upper = 0;
	lower = 0;
	do {
		/* If it isn't possible to read line, exit. */
		if (read(0, &rawc, 1) <= 0)
			exit(0);

		if ((c = (rawc & charmask)) == '\0')
			return(BADSPEED);

		if (c == CEOF)
			exit(1);

		if (c == ERASE || c == BACKSPACE) {
			/*
			 * If there is anything to erase, erase a character.
			 */
			if (ptr > user) {
				--ptr;
				if (termios->c_ospeed >= B1200)
					putstr("\b \b");
				else
					putchr(rawc);
			}
			continue;
		}
		/*
		 * If the character is a kill line or abort character,
		 * reset the line.
		 */
		else if (c == KILL || c == ABORT || c == control('U')) {
			ptr = user;
			putstr("\r\n");
			continue;
		}
		else if (islower(c)) {
			lower++;
			*ptr++ = c;
		}
		else if (isupper(c)) {
			upper++;
			*ptr++ = c;
		}
		/* Just store all other characters. */
		else
			*ptr++ = c;
		/* Echo the character if ECHO is off. */
		if( (termios->c_lflag & ECHO) == 0 )
			putchr(rawc);
	}
	/*
	 * Continue the above loop until a line terminator is found or
	 * until user name array is full.
	 */
	while (c != '\n' && c != '\r'
	    && ptr < (user + MAXLINE));

	/* Remove the last character from name. */
	*--ptr = '\0';
	if (ptr == user)
		return(NONAME);

	if (c == '\r')
		putchr('\n');
	/*
	 * If the line terminated with a <lf>, put ICRNL and ONLCR into
	 * into the modes.
	 */
	if (c == '\r') {
		termios->c_iflag |= ICRNL;
		termios->c_oflag |= ONLCR;
	/* When line ends with a <lf>, then add the <cr>. */
	} else
		putchr('\r');
	/*
	 * Set the upper-lower case conversion switchs if only upper
	 * case characters were seen in the login and no lower case.
	 * Also convert all the upper case characters to lower case.
	 */
	if (upper > 0 && lower == 0) {
		termios->c_iflag |= IUCLC;
		termios->c_oflag |= OLCUC;
		termios->c_lflag |= XCASE;
		for (ptr=user; *ptr; ptr++)
			if (isupper(*ptr))
				*ptr = tolower(*ptr);
	}
	return(GOODNAME);
}

/*	"find_def" scans "/etc/gettydefs" for a string with the		*/
/*	requested "id".  If the "id" is NULL, then the first entry is	*/
/*	taken, hence the first entry must be the default entry.		*/
/*	If a match for the "id" is found, then the line is parsed and	*/
/*	the Gdef structure filled.  Errors in parsing generate error	*/
/*	messages on the system console.					*/

struct Gdef *find_def(id)
char *id;
{
	register struct Gdef *gptr;
	register char *ptr,c;
	FILE *fp;
	int i,input,state,size,rawc,field;
	char oldc,*optr,quoted(),*gdfile;
	char line[MAXLINE+1];
	int linesize = 0;
	static struct Gdef def;
	static char d_id[MAXIDLENGTH+1],d_nextid[MAXIDLENGTH+1];
	static char d_message[MAXMESSAGE+1];
	extern char *getword(),*fields(),*speed();
	static char *states[] = {
		"","id","initial flags","final flags","message","next id"
	};

/* Decide whether to read the real /etc/gettydefs or the supplied */
/* check file. */
	if (check) gdfile = checkgdfile;
	else gdfile = GETTY_DEFS;

/* Open the "/etc/gettydefs" file.  Be persistent. */
	for (i=0; i < 3;i++) {
		if ((fp = fopen(gdfile,"r")) != NULL) break;
		else sleep(3);	/* Wait a little and then try again. */
	}

/* If unable to open, complain and then use the built in default. */
	if (fp == NULL) {
		error(MSGSTR(CANT_OPN2, "getty: can't open \"%s\").\n"),gdfile);
		return(&DEFAULT);
	}

/* Start searching for the line with the proper "id". */
	input = ACTIVE;
	do {
		for(ptr=line,linesize=0,oldc='\0'; ptr < &line[sizeof(line)] &&
		    (rawc = getc(fp)) != EOF; ptr++,linesize++,oldc = c) {
			c = *ptr = rawc;

/* Search for two \n's in a row. */
			if (c == '\n' && oldc == '\n') break;
		}

/* If we didn't end with a '\n' or EOF, then the line is too long. */
/* Skip over the remainder of the stuff in the line so that we */
/* start correctly on next line. */
		if (rawc != EOF && c != '\n') {
			for (oldc='\0'; (rawc = getc(fp)) != EOF;oldc=c) {
				c = rawc;
				if (c == '\n' && oldc != '\n') break;
			}
			if (check) {
				printf(stdout,MSGSTR(TOO_LONG, "Entry too long.\n"));
				check_errors++; /* paw: qar 6383 - increment error count */
			}
		}

/* If we ended at the end of the file, then if there is no */
/* input, break out immediately otherwise set the "input" */
/* flag to FINISHED so that the "do" loop will terminate. */
		if (rawc == EOF) {
			if (ptr == line) break;
			else input = FINISHED;
		}
		
/* If the last character stored was an EOF or '\n', replace it */
/* with a '\0'. */
		if (line[linesize] == EOF || line[linesize] == '\n') 
			line[linesize] = '\0';

/* If the next-to-last character stored was an '\n', replace it */
/* with a '\0'. */
		if (line[linesize-1] == '\n') 
			line[linesize-1] = '\0';

/* If the buffer is full, then make sure there is a null after the */
/* last character stored. */
		else *++ptr == '\0';
		if (check) fprintf(stdout,MSGSTR(NEXT_ENTRY, "\n**** Next Entry ****\n%s\n"),line);

/* If line starts with #, treat as comment */
		if(line[0] == '#') continue;

/* Initialize "def" and "gptr". */
		gptr = &def;
		gptr->g_id = (char*)NULL;
		gptr->g_iflags.c_iflag = 0;
		gptr->g_iflags.c_oflag = 0;
		gptr->g_iflags.c_cflag = 0;
		gptr->g_iflags.c_lflag = 0;
		gptr->g_fflags.c_iflag = 0;
		gptr->g_fflags.c_oflag = 0;
		gptr->g_fflags.c_cflag = 0;
		gptr->g_fflags.c_lflag = 0;
		gptr->g_message = (char*)NULL;
		gptr->g_nextid = (char*)NULL;

/* Now that we have the complete line, scan if for the various */
/* fields.  Advance to new field at each unquoted '#'. */
		for (state=ID,ptr= line; state != FAILURE && state != SUCCESS;) {
			switch(state) {
			case ID:

/* Find word in ID field and move it to "d_id" array. */
				strncpy(d_id,getword(ptr,&size),MAXIDLENGTH);
				gptr->g_id = d_id;

/* Move to the next field.  If there is anything but white space */
/* following the id up until the '#', then set state to FAILURE. */
				ptr += size;
				while (isspace(*ptr)) ptr++;
				if (*ptr != '#') {
					field = state;
					state = FAILURE;
				} else {
					ptr++;	/* Skip the '#' */
					state = IFLAGS;
				}
				break;

/* Extract the "g_iflags" */
			case IFLAGS:
				if ((ptr = fields(ptr,&gptr->g_iflags)) == NULL) {
					field = state;
					state = FAILURE;
				} else {
					gptr->g_iflags.c_iflag &= ICRNL;
					if((gptr->g_iflags.c_cflag & CSIZE) == 0)
						gptr->g_iflags.c_cflag |= CS8;
					gptr->g_iflags.c_cflag |= CREAD|HUPCL;
					gptr->g_iflags.c_lflag &= ~(ISIG|ICANON
						|XCASE|ECHOE|ECHOK);
					ptr++;
					state = FFLAGS;
				}
				break;

/* Extract the "g_fflags". */
			case FFLAGS:
				if ((ptr = fields(ptr,&gptr->g_fflags)) == NULL) {
					field = state;
					state = FAILURE;
				} else {

/* Force the CREAD mode in regardless of what the user specified. */
					gptr->g_fflags.c_cflag |= CREAD;
					ptr++;
					state = MESSAGE;
				}
				break;

/* Take the entire next field as the "login" message. */
/* Follow usual quoting procedures for control characters. */
			case MESSAGE:
				for (optr= d_message; (c = *ptr) != '\0'
				    && c != '#';ptr++) {

/* If the next character is a backslash, then get the quoted */
/* character as one item. */
					if (c == '\\') {
						if (ptr[1] == '\n') {
							++ptr;
							continue;
						}
						c = quoted(ptr,&size);
/* -1 accounts for ++ that takes place later. */
						ptr += size - 1;
					}

/* If there is room, store the next character in d_message. */
					if (optr < &d_message[MAXMESSAGE])
						*optr++ = c;
				}

/* If we ended on a '#', then all is okay.  Move state to NEXTID. */
/* If we didn't, then set state to FAILURE. */
				if (c == '#') {
					gptr->g_message = d_message;
					state = NEXTID;

/* Make sure message is null terminated. */
					*optr++ = '\0';
					ptr++;
				} else {
					field = state;
					state = FAILURE;
				}
				break;

/* Finally get the "g_nextid" field.  If this is successful, then */
/* the line parsed okay. */
			case NEXTID:

/* Find the first word in the field and save it as the next id. */
				strncpy(d_nextid,getword(ptr,&size),MAXIDLENGTH);
				gptr->g_nextid = d_nextid;

/* There should be nothing else on the line.  Starting after the */
/* word found, scan to end of line.  If anything beside white */
/* space, set state to FAILURE. */
				ptr += size;
				while (isspace(*ptr)) ptr++;
				if (*ptr != '\0') {
					field = state;
					state = FAILURE;
				} else state = SUCCESS;
				break;
			}
		}

/* If a line was successfully picked up and parsed, compare the */
/* "g_id" field with the "id" we are looking for. */
		if (state == SUCCESS) {

/* If there is an "id", compare them. */
			if (id != NULL) {
				if (strcmp(id,gptr->g_id) == 0) {
					fclose(fp);
					return(gptr);
				}

/* If there is no "id", then return this first successfully */
/* parsed line outright. */
			} else if (check == FALSE) {
				fclose(fp);
				return(gptr);

/* In check mode print out the results of the parsing. */
			} else {
				fprintf(stdout,MSGSTR(G_ID, "id: %s\n"),gptr->g_id);
				fprintf(stdout,MSGSTR(INITIAL_FLG, "initial flags:\niflag- %o oflag- %o cflag- %o lflag- %o\n"),
					gptr->g_iflags.c_iflag,
					gptr->g_iflags.c_oflag,
					gptr->g_iflags.c_cflag,
					gptr->g_iflags.c_lflag);
				fprintf(stdout,MSGSTR(FINAL_FLG, "final flags:\niflag- %o oflag- %o cflag- %o lflag- %o\n"),
					gptr->g_fflags.c_iflag,
					gptr->g_fflags.c_oflag,
					gptr->g_fflags.c_cflag,
					gptr->g_fflags.c_lflag);
				fprintf(stdout,MSGSTR(MSG2, "message: %s\n"),gptr->g_message);
				fprintf(stdout,MSGSTR(NEXT_ID, "next id: %s\n"),gptr->g_nextid);
			}

/* If parsing failed in check mode, complain, otherwise ignore */
/* the bad line. */
		} else if (check) {
			if (ptr != NULL)
				*++ptr = '\0';
			check_errors++; /* paw: qar 6383 - increment error count */
			fprintf(stdout,MSGSTR(PARSING_FAIL, "Parsing failure in the \"%s\" field\n\
%s<--error detected here\n"),
				states[field],line);
		}
	} while (input == ACTIVE);

/* If no match was found, then return NULL. */
	fclose(fp);
	return(NULL);
}

char *getword(ptr,size)
register char *ptr;
int *size;
{
	register char *optr,c;
	char quoted();
	static char word[MAXIDLENGTH+1];
	int qsize;

	/* Skip over all white spaces including quoted spaces and tabs. */
	for (*size=0; isspace(*ptr) || *ptr == '\\';) {
		if (*ptr == '\\') {
			c = quoted(ptr,&qsize);
			(*size) += qsize;
			ptr += qsize+1;
			/*
			 * If this quoted character is not a space or a
			 * tab or a newline then break.
			 */
			if (isspace(c) == 0) break;
		} else {
			(*size)++;
			ptr++;
		}
	}

	/*
	 * Put all characters from here to next white space or '#' or '\0'
	 * into the word, up to the size of the word.
	 */
	for (optr= word,*optr='\0'; isspace(*ptr) == 0 &&
	    *ptr != '\0' && *ptr != '#'; ptr++,(*size)++) {

		/* If the character is quoted, analyze it. */
		if (*ptr == '\\') {
			c = quoted(ptr,&qsize);
			(*size) += qsize;
			ptr += qsize;
		} else c = *ptr;

		/* If there is room, add this character to the word. */
		if (optr < &word[MAXIDLENGTH+1] ) *optr++ = c;
	}

	*optr++ = '\0';
	return(word);
}

/*	"quoted" takes a quoted character, starting at the quote	*/
/*	character, and returns a single character plus the size of	*/
/*	the quote string.  "quoted" recognizes the following as		*/
/*	special, \n,\r,\v,\t,\b,\f as well as the \nnn notation.	*/

char quoted(ptr,qsize)
char *ptr;
int *qsize;
{
	register char c,*rptr;
	register int i;

	rptr = ptr;
	switch(*++rptr) {
	case 'n':
		c = '\n';
		break;
	case 'r':
		c = '\r';
		break;
	case 'v':
		c = '\013';
		break;
	case 'b':
		c = '\b';
		break;
	case 't':
		c = '\t';
		break;
	case 'f':
		c = '\f';
		break;
	default:
		/*
		 * If this is a numeric string, take up to three characters of
		 * it as the value of the quoted character.
		 */
		if (*rptr >= '0' && *rptr <= '7') {
			for (i=0,c=0; i < 3;i++) {
				c = c*8 + (*rptr - '0');
				if (*++rptr < '0' || *rptr > '7') break;
			}
			rptr--;
		/*
		 * If the character following the '\\' is a NULL, back up the
		 * ptr so that the NULL won't be missed.  The sequence
		 * backslash null is essentially illegal.
		 */
		} else if (*rptr == '\0') {
			c = '\0';
			rptr--;

		/* In all other cases the quoting does nothing. */
		} else c = *rptr;
		break;
	}

	/* Compute the size of the quoted character. */
	(*qsize) = rptr - ptr + 1;
	return(c);
}

/*	"fields" picks up the words in the next field and converts all	*/
/*	recognized words into the proper mask and puts it in the target	*/
/*	field.								*/

char *fields(ptr,termios)
register char *ptr;
struct termios *termios;
{
	extern struct Symbols *search();
	register struct Symbols *symbol;
	char *word,*getword();
	int size;

	termios->c_iflag = 0;
	termios->c_oflag = 0;
	termios->c_cflag = 0;
	termios->c_lflag = 0;
	while (*ptr != '#' && *ptr != '\0') {

		/* Pick up the next word in the sequence. */
		word = getword(ptr,&size);

		/* If there is a word, scan the two mode tables for it. */
		if (*word != '\0') {
			/*
			 * If the word is the special word "SANE",
			 * put in all the flags that are needed
			 * for SANE tty behavior.
			 */
			if (strcmp(word,"SANE") == 0) {
				termios->c_iflag |= ISANE;
				termios->c_oflag |= OSANE;
				termios->c_cflag |= CSANE;
				termios->c_lflag |= LSANE;
			} else if ((symbol = search(word,imodes)) != NULL)
				termios->c_iflag |= symbol->s_value;
			else if ((symbol = search(word,omodes)) != NULL)
				termios->c_oflag |= symbol->s_value;
			else if ((symbol = search(word,cmodes)) != NULL) {
				switch(symbol->s_value) {
				case CS5: case CS6: case CS7: case CS8:
					termios->c_cflag &= ~CSIZE;
					break;
				}
				termios->c_cflag |= symbol->s_value;
			} else if ((symbol = search(word,lmodes)) != NULL)
				termios->c_lflag |= symbol->s_value;
			else if ((symbol = search(word,speeds)) != NULL)
				termios->c_ispeed = termios->c_ospeed
							= symbol->s_value;
			else if (check) fprintf(stdout,MSGSTR(UNDEFINED, "Undefined: %s\n"),word);
		}

		/* Advance pointer to after the word. */
		ptr += size;
	}
	/*
	 * If we didn't end on a '#', return NULL, otherwise return the
	 * updated pointer.
	 */
	return(*ptr != '#' ? NULL : ptr);
}

/*	"parse" breaks up the user's response into seperate arguments	*/
/*	and fills the supplied array with those arguments.  Quoting	*/
/*	with the backspace is allowed.					*/

parse(string,args,cnt)
char *string,**args;
int cnt;
{
	register char *ptrin,*ptrout;
	register int i;
	extern char quoted();
	int qsize;

	for (i=0; i < cnt; i++) args[i] = (char *)NULL;
	for (ptrin = ptrout = string,i=0; *ptrin != '\0' && i < cnt; i++) {

		/* Skip excess white spaces between arguments. */

		while(*ptrin == ' ' || *ptrin == '\t') {
			ptrin++;
			ptrout++;
		}

		/*
		 * Save the address of the argument if there is
		 * something there.
		 */
		if (*ptrin == '\0')
			break;
		else
			args[i] = ptrout;
		/*
		 * Span the argument itself.  The '\' character causes quoting
		 * of the next character to take place (except for '\0').
		 */
		while (*ptrin != '\0') {
			/* Is this the quote character? */
			if (*ptrin == '\\') {
				*ptrout++ = quoted(ptrin,&qsize);
				ptrin += qsize;
			}
			/* Is this the end of the argument?  If so quit loop. */

			else if (*ptrin == ' ' || *ptrin == '\t') {
				ptrin++;
				break;
			}

			/*
			 * If this is a normal letter of the argument,
			 * save it, advancing the pointers at the same time.
			 */
			
			else
				*ptrout++ = *ptrin++;
		}
		/* Null terminate the string. */

		*ptrout++ = '\0';
	}
}

writemsg(cp)
register char *cp;
{
	char *ttyn, *slash;
#ifdef notdef
	char datebuffer[60];
#endif
	extern char *ttyname(), *rindex();

	while (*cp) {
		if (*cp != '%') {
			putchr(*cp++);
			continue;
		}
		switch (*++cp) {

		case 't':
			ttyn = ttyname(0);
			slash = rindex(ttyn, '/');
			if (slash == (char *) 0)
				putstr(ttyn);
			else
				putstr(&slash[1]);
			break;

		case 'h':
			putstr(editedhost);
			break;
#ifdef notdef
		case 'd':
			get_date(datebuffer);
			putstr(datebuffer);
			break;
#endif
		case '%':
			putchr('%');
			break;
		}
		cp++;
	}
}

putstr(s)
	register char *s;
{

	while (*s)
		putchr(*s++);
}

putchr(cc)
{
	char c;

	c = cc;
	write(1, &c, 1);
}
