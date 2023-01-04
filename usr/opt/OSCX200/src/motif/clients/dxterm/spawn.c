/*
 *  Title:	spawn.c
 *
 *  +------------------------------------------------------------------------+
 *  | Copyright © 1988, 1990                                                 |
 *  | By Digital Equipment Corporation, Maynard, Mass.                       |
 *  | All Rights Reserved.                                                   |
 *  |                                                                        |
 *  | This software is furnished under a license and may be used and  copied |
 *  | only  in  accordance  with  the  terms  of  such  license and with the |
 *  | inclusion of the above copyright notice.  This software or  any  other |
 *  | copies  thereof may not be provided or otherwise made available to any |
 *  | other person.  No title to and ownership of  the  software  is  hereby |
 *  | transfered.                                                            |
 *  |                                                                        |
 *  | The information in this software is subject to change  without  notice |
 *  | and  should  not  be  construed  as  a commitment by Digital Equipment |
 *  | Corporation.                                                           |
 *  |                                                                        |
 *  | DIGITAL assumes no responsibility for the use or  reliability  of  its |
 *  | software on equipment which is not supplied by DIGITAL.                |
 *  +------------------------------------------------------------------------+
 *  
 *  Module Abstract:
 *
 *	This module contains the code to create and destroy ptys and processes
 *	on Ultrix.
 *
 *	Code copied from xterm's main.c and misc.c.
 *
 *  Procedures contained in this module:
 *
 *	spawn()
 *	un_spawn()
 *
 *  Author:	Tom Porcher	3-Jul-1988
 *
 *  Modification history:
 *
 * Alfred von Campe     20-Nov-1993	    BL-E
 *      - Notify our exit handler when we receive a signal.
 *
 * Alfred von Campe     15-Oct-1993     BL-E
 *      - Remove I18N ifdef.
 *
 * Alfred von Campe     25-Mar-1993     V1.2/BL2
 *      - Make UTMP logging work by using SYSV style utmp (from xterm).
 *
 * Alfred von Campe     08-Oct-1992     Ag/BL10
 *      - Set pass8 field in the lmode structure.
 *
 * Eric Osman           11-June-1992     Sun
 *      - On Sun, openpty isn't in libraries, so use private version.
 *
 * Alfred von Campe     02-Apr-1992     Ag/BL6.2.1
 *      - Use new openpty() system call.
 *      - Add I18N support (from To-lung).
 *      - Change #ifdef DECterm to #ifdef DECTERM
 *
 * Alfred von Campe     05-Nov-1991     Hercules/1 BL5
 *      - Call goodbye on SIGINT and SIGQUIT as well as SIGTERM.
 *	- Restore the default protection of the tty even if the group struct
 *	  exists.  Also, restore the gid of the tty to 0 to be consistent.
 *
 * Alfred von Campe    06-Oct-1991     Hercules/1 T0.7
 *      - Added new callback for SIGTERM to exit gracefully.
 *      - Don't call vhangup(), as it's not available in OSF/1.
 *      - Put all UTMP stuff in #ifdef sections.
 *      - Don't close all standard file descriptors (from ULTRIX).
 *      - Add setsid() and ioctl(0, TIOCSCTTY, 0) calls to set up tty.
 *
 * Alfred von Campe     04-Feb-1991     T3.0
 *      - Change malloc to XtMalloc and calloc to XtCalloc.
 *
 * Bob Messenger	03-May-1990	V2.1 (special release for 3MAX)
 *	- Fix a problem where DECterm can take over 4 minutes to start up
 *	  because of a deadlock with the shell.
 *
 * Bob Messenger	01-Mar-1990	V2.1
 *	- Set Exit as the IOErrorHandler in UWS V4.0, in order to clean up
 *	  the utmp entry.
 *
 * Bob Messenger	01-Oct-1989	V2.0
 *	- Reset SIGINT, SIGQUIT and SIGTERM signals to SIG_DFL in child
 *	  process.
 *
 * Bob Messenger	24-Jul-1989	X2.0-16
 *	- On Ultrix V4.0 and later, open tty with group "tty", mode 0x620.
 *
 * Bob Messenger	18-Jul-1989	X2.0-16
 *	- Add "vt300" to the list of possible TERM values.
 *
 * Bob Messenger	21-Feb-1989	X1.1-1 (UWSV2.1)
 *	- Do equivalent of "stty dec" at startup
 *
 * Eric Osman		26-Sep-1988	BL10.2
 *	- add "extern int errno" and only include time.h if _TIMEH
 *	  not defined (Durga Rao's changes)
 *
 */

/*
********************************************************************************

  Description of changes from Ultrix version of spawn():

	- removed TScreen structure,
	  replaced with:
		screen->display->fd	=>	display->fd (param)
		screen->uid		=>	my_uid
		screen->gid		=>	my_gid
		screen->pid		=>	*pidp (param)
		screen->respond		=>	*pty_fdp (param)
		screen->max_row		=>	rows-1 (param)
		screen->max_col		=>	cols-1 (param)

	- added as parameters:
		get_ty		ptr to string
		am_slave	fd of passedPty
		passedPty	last two characters of PTY name
		command_to_exec	pointer to argv
		login_shell	Boolean
		Console		Boolean

		display		X display
		rows		number of rows
		cols		number of columns
		width
		height

		pty_fdp		ptr to PTY fd (return)
		pidp		ptr to pid (retrurn)
		tslotp		ptr to tslot (return)

	- copied code to open loginpty from main.c.

	- copied code to set FIONBIO from main.c.

	- conditionally compiled for DECterm to set rows/cols/width/height
	  from parameters.

	- conditionally compiled for DECterm to not move fd's around.

	- statics from main.c:
		d_sg
		d_tc
		d_ltc
		d_disipline
		d_lmode
		etc_utmp
		env
		ttydev
		ptydev
		xterm_name	(changed to DECterm)
		vtterm		(changed:  xterm => DECterm, added vt200)

	- changed resize() to take rows/cols rather than screen.

	- changed XDisplayName() to XDisplayString()

	- changed Exit(n) to un_spawn( ... ); it no longer exits.

	- changed consolepr() to use xterm_name rather than "xterm".

********************************************************************************
*/

#include <pwd.h>
#include <sgtty.h>
#include <grp.h>
#include <sys/wait.h>
#ifndef _TIMEH
#include <sys/time.h>
#endif
#include <sys/resource.h>
#include <stdio.h>
#include <sys/file.h>
#include <errno.h>
	extern int errno;
#include <signal.h>
#include <strings.h>
#include <setjmp.h>
#include <termio.h>

#ifdef apollo
#include <sys/types.h>
#define ttyslot() 1
#define vhangup() ;
#endif

#include <utmp.h>

#include <sys/param.h>	/* for NOFILE */
#include <X11/Xlib.h>

#include "error.h"

#ifndef FALSE
#define FALSE 0
#endif
#ifndef TRUE
#define TRUE 1
#endif

#define DECTERM


static struct  sgttyb d_sg = {
        0, 0, 0177, CKILL, EVENP|ODDP|ECHO|XTABS|CRMOD
};
static struct  tchars d_tc = {
        CINTR, CQUIT, CSTART,
        CSTOP, CEOF, CBRK,
};
static struct  ltchars d_ltc = {
        CSUSP, CDSUSP, CRPRNT,
        CFLUSH, CWERASE, CLNEXT
};
static int d_disipline = NTTYDISC;
static long int d_lmode = LCRTBS|LCRTERA|LCRTKIL|LCTLECH;
static char etc_utmp[] = "/etc/utmp";
static jmp_buf env;
static char ttydev[] = "/dev/ttyxx";
static char ptydev[] = "/dev/ptyxx";

#ifdef WTMP
static char etc_wtmp[] = "/usr/adm/wtmp";
#endif

static char *xterm_name = "DECterm";

static struct group *group_struct = NULL;

/* from misc.c */
SysError (i)
int i;
{
	fprintf (stderr, "%s: Error %d, errno %d:\n", xterm_name, i, errno);
	perror ("");
	Cleanup(i);
}

Error (i)
int i;
{
	fprintf (stderr, "%s: Error %d\n", xterm_name, i);
	Cleanup(i);
}

/*
 * cleanup by sending SIGHUP to client processes
 */
Cleanup (code)
int code;
{
#ifdef notdef
	extern Terminal term;
	register TScreen *screen;

	screen = &term.screen;
	if (screen->pid > 1)
		killpg(getpgrp(screen->pid), SIGHUP);
#endif
	Exit (code);
}

/*
 * sets the value of var to be arg in the Unix 4.2 BSD environment env.
 * Var should end with '=' (bindings are of the form "var=value").
 * This procedure assumes the memory for the first level of environ
 * was allocated using XtCalloc, with enough extra room at the end so not
 * to have to do a realloc().
 */
Setenv (var, value)
register char *var, *value;
{
	extern char **environ;
	register int index = 0;
	register int len = strlen(var);

	while (environ [index] != NULL) {
	    if (strncmp (environ [index], var, len) == 0) {
		/* found it */
		environ[index] = (char *)XtMalloc ((unsigned)len + strlen (value) + 1);
		strcpy (environ [index], var);
		strcat (environ [index], value);
		return;
	    }
	    index ++;
	}

#ifdef DEBUG
	if (debug) fputs ("expanding env\n", stderr);
#endif DEBUG

	environ [index] = (char *) XtMalloc ((unsigned)len + strlen (value) + 1);
	(void) strcpy (environ [index], var);
	strcat (environ [index], value);
	environ [++index] = NULL;
}

/*
 * returns a pointer to the first occurrence of s2 in s1,
 * or NULL if there are none.
 */
char *strindex (s1, s2)
register char	*s1, *s2;
{
	register char	*s3;
	char		*index();

	while ((s3=index(s1, *s2)) != NULL) {
		if (strncmp(s3, s2, strlen(s2)) == 0)
			return (s3);
		s1 = ++s3;
	}
	return (NULL);
}

XStrCmp(s1, s2)
char *s1, *s2;
{
  if (s1 && s2) return(strcmp(s1, s2));
  if (s1 && *s1) return(1);
  if (s2 && *s2) return(-1);
  return(0);
}

/* VARARGS1 */
consolepr(fmt,x0,x1,x2,x3,x4,x5,x6,x7,x8,x9)
char *fmt;
{
	extern int errno;
	extern char *sys_errlist[];
	int oerrno;
	int f;
 	char buf[ BUFSIZ ];

	oerrno = errno;
 	strcpy(buf, xterm_name);
 	strcat(buf, ": ");
 	sprintf(buf+strlen(buf), fmt, x0,x1,x2,x3,x4,x5,x6,x7,x8,x9);
 	strcat(buf, ": ");
 	strcat(buf, sys_errlist[oerrno]);
 	strcat(buf, "\n");	
	f = open("/dev/console",O_WRONLY);
	write(f, buf, strlen(buf));
	close(f);
	if ((f = open("/dev/tty", 2)) >= 0) {
		ioctl(f, TIOCNOTTY, (char *)NULL);
		close(f);
	}
}

#ifndef __osf__

/*
 * Temporary replacement for openpty for environments not having this routine
 * in their libraries.
 */

openpty (pty, tty)
/*
   opens a pty, storing fildes in pty and tty.
 */
int *pty, *tty;
{
	int devindex, letter = 0;

	while (letter < 11) {
	    ttydev [8] = ptydev [8] = "pqrstuvwxyz" [letter++];
	    devindex = 0;

	    while (devindex < 16) {
		ttydev [9] = ptydev [9] = "0123456789abcdef" [devindex++];
		if ((*pty = open (ptydev, O_RDWR)) < 0)
			continue;
		if ((*tty = open (ttydev, O_RDWR)) < 0) {
			close(*pty);
			continue;
		}
		return;
	    }
	}
	
	fprintf (stderr, "%s: Not enough available pty's\n", xterm_name);
	exit (ERROR_PTYS);
}

#endif

get_pty (pty, tty)
/*
   opens a pty, storing fildes in pty and tty.
 */
int *pty, *tty;
{
    struct termios term;
    struct winsize win;

    if(openpty(pty, tty, ptydev, NULL, NULL) == -1)
    {
        fprintf(stderr, "%s: Can't open pty!\n", xterm_name);
        exit(ERROR_PTYS);
    }    
    
    strncpy(&ttydev[8], &ptydev[8], 2);		/* Note: ttydev is used later */
}

static char *vtterm[] = {
	"DECterm",
        "vt382",
        "vt382d",
        "vt382c",
        "vt382k",
	"vt300",
	"vt200",
	"vt102",
	"vt100",
	"ansi",
	"dumb",
	0
};

hungtty()
{
	longjmp(env, 1);
}

io_error_handler( display )
	Display *display;
{
	Exit(0);	/* clean up utmp entry */
	exit(0);	/* Exit doesn't exit! If we don't explicitly exit,
			   DECterm will go into a tight loop when we quit
			   out of the session manager */
}

extern int run_flag, pipe_fd[];

goodbye()
{
    int tty;

    run_flag=0;
    
    /* Clearing the run_flag will cause our main loop to terminate, but
     * since the toolkit is most likely blocking on something, it will
     * not check run_flag.  Ideally, we would just do our cleanup here,
     * but we can't do any X-calls from signal level.  So we write in
     * to a pipe for which we did an XtAppAddInput earlier, and even
     * though the toolkit will still be blocked, it will call our input
     * handler, where we can do our clean up (see pty_ultrix.c).
     */

    write(pipe_fd[1], " ", 1);
}

spawn( get_ty, am_slave, passedPty, command_to_exec, login_shell, Console,
		display, rows, cols, width, height, window,
		pty_fdp, pidp, tslotp )
	char	*get_ty;
	int	am_slave;
	char	*passedPty;
	char	**command_to_exec;
	char	login_shell;
	char	Console;
	Display	*display;
	int	rows, cols, width, height;
	int	*pty_fdp;
	int	*pidp;
	int	*tslotp;
/* 
 *  Inits pty and tty and forks a login process.
 *  Does not close fd Xsocket.
 *  If getty,  execs getty rather than csh and uses std fd's rather
 *  than opening a pty/tty pair.
 *  If slave, the pty named in passedPty is already open for use
 */
{
	int Xsocket = ConnectionNumber(display);
	int index1, tty = -1, tty2;
	int discipline;
	unsigned lmode;
	struct tchars tc;
	struct ltchars ltc;
	struct sgttyb sg;

	char termcap [1024];
	char newtc [1024];
	char *ptr, *shname;
	int i, no_dev_tty = FALSE;
	char **envnew;		/* new environment */
	char buf[32];
	char *TermName = NULL;
	int ldisc = 0;

	int my_uid;
	int my_gid;
	int loginpty;

#ifdef sun
#ifdef TIOCSSIZE
	struct ttysize ts;
#endif TIOCSSIZE
#else sun
#ifdef TIOCSWINSZ
	struct winsize ws;
#endif TIOCSWINSZ
#endif sun
	struct passwd *pw = NULL;
#ifdef UTMP
	struct utmp utmp;
	char *ptyname;
	char *ptynameptr;
#endif UTMP
#ifdef WTMP
	int fd;
#endif
	extern int Exit();
	char *getenv();
	char *index (), *rindex (), *strindex ();

	my_uid = getuid();
	my_gid = getgid();

	group_struct = getgrnam( "tty" );

	/* so that TIOCSWINSZ || TIOCSIZE doesn't block */
	signal(SIGTTOU,SIG_IGN);
#ifndef DECTERM
	if(!(screen->TekEmu ? TekInit() : VTInit()))
		exit(ERROR_INIT);

	if(screen->TekEmu) {
		envnew = tekterm;
		ptr = newtc;
	} else
#endif DECTERM
		{
		envnew = vtterm;
		ptr = termcap;
	}
	while(*envnew) {
		if(tgetent(ptr, *envnew) == 1) {
			TermName = *envnew;
#ifndef DECTERM
			if(!screen->TekEmu)
#endif DECTERM
			    resize(rows, cols, TermName, termcap, newtc);
			break;
		}
		envnew++;
	}

	if (get_ty) {
		char tt[32];

		strcpy(tt,"/dev/");
		strcat(tt, get_ty);
		tt[5] = 'p';
		loginpty = open( tt, O_RDWR, 0 );
		dup2( loginpty, 4 );
		close( loginpty );
		loginpty = 4;
		tt[5] = 't';
		if ( group_struct != NULL )
		    {  /* tty group exists: Ultrix 4.0 or later */
		    chown(tt, 0, group_struct->gr_gid);
		    chmod(tt, 0620);
		    }
		else
		    {  /* tty group doesn't exist: pre-Ultrix 4.0 */
		    chown(tt, 0, 0);
		    chmod(tt, 0622);
		    }
		if (open(tt, O_RDWR, 0) < 0) {
			consolepr("open(%s) failed\n", tt);
		}
		signal(SIGHUP, SIG_IGN);

#if 0 /* Removed for Hercules/1, but may still be needed in the future */
		vhangup();
#endif 0
		setpgrp(0,0);
		signal(SIGHUP, SIG_DFL);
		(void) close(0);
		open(tt, O_RDWR, 0);
		dup2(0, 1);
		dup2(0, 2);

		*pty_fdp = loginpty;
#ifdef UTMP
		if((*tslotp = ttyslot()) <= 0){
			SysError(ERROR_TSLOT);
		}
#endif UTMP
	} else if (am_slave) {
		*pty_fdp = am_slave;
		ptydev[8] = ttydev[8] = passedPty[0];
		ptydev[9] = ttydev[9] = passedPty[1];
#ifdef UTMP
		if((*tslotp = ttyslot()) <= 0){
			SysError(ERROR_TSLOT2);
		}
#endif UTMP
		setgid (my_gid);
		setuid (my_uid);
	} else {
 		/*
 		 * Sometimes /dev/tty hangs on open (as in the case of a pty
 		 * that has gone away).  Simply make up some reasonable
 		 * defaults.
 		 */
 		signal(SIGALRM, hungtty);
 		alarm(1);		
 		if (! setjmp(env)) {
 			tty = open ("/dev/tty", O_RDWR, 0);
 			alarm(0);
 		} else {
 			tty = -1;
 			errno = ENXIO;
 		}
 		signal(SIGALRM, SIG_DFL);
 
 		if (tty < 0) {
			if (errno != ENXIO) SysError(ERROR_OPDEVTTY);
			else {
				no_dev_tty = TRUE;
				sg = d_sg;
				tc = d_tc;
				discipline = d_disipline;
				ltc = d_ltc;
				lmode = d_lmode;
			}
		} else {
			/* get a copy of the current terminal's state */

			if(ioctl(tty, TIOCGETP, (char *)&sg) == -1)
				SysError (ERROR_TIOCGETP);
			if(ioctl(tty, TIOCGETC, (char *)&tc) == -1)
				SysError (ERROR_TIOCGETC);
			if(ioctl(tty, TIOCGETD, (char *)&discipline) == -1)
				SysError (ERROR_TIOCGETD);
			if(ioctl(tty, TIOCGLTC, (char *)&ltc) == -1)
				SysError (ERROR_TIOCGLTC);
			if(ioctl(tty, TIOCLGET, (char *)&lmode) == -1)
				SysError (ERROR_TIOCLGET);
			close (tty);

#if 0
			/* close all std file descriptors */
			for (index1 = 0; index1 < 3; index1++)
				close (index1);
			if ((tty = open ("/dev/tty", O_RDWR, 0)) < 0)
				SysError (ERROR_OPDEVTTY2);

			if (ioctl (tty, TIOCNOTTY, (char *) NULL) == -1)
				SysError (ERROR_NOTTY);
			close (tty);
#endif
		}
		get_pty (pty_fdp, &tty);

#ifndef DECTERM
		if (*pty_fdp != Xsocket + 1) {
			dup2 (*pty_fdp, Xsocket + 1);
			close (*pty_fdp);
			*pty_fdp = Xsocket + 1;
		}
#endif DECTERM

		if ( group_struct != NULL )
		    {  /* tty group exists: Ultrix 4.0 or later */
		    /* change ownership of tty to real group and user id */
		    chown (ttydev, my_uid, group_struct->gr_gid);

		    /* change protection of tty */
		    chmod (ttydev, 0620);
		    }
		else
		    {  /* tty group doesn't exist: pre-Ultrix 4.0 */
		    /* change ownership of tty to real group and user id */
		    chown (ttydev, my_uid, my_gid);

		    /* change protection of tty */
		    chmod (ttydev, 0622);
		    }

#ifndef DECTERM
		if (tty != Xsocket + 2)	{
			dup2 (tty, Xsocket + 2);
			close (tty);
			tty = Xsocket + 2;
		}
#endif DECTERM

		/* set the new terminal's state to be the old one's 
		   with minor modifications for efficiency */

		sg.sg_flags &= ~(ALLDELAY | XTABS | CBREAK | RAW);
		sg.sg_flags |= ECHO | CRMOD;
		/* stty dec */
		sg.sg_erase = 0177;	/* delete */
		sg.sg_kill = CKILL;	/* ^U */
		/* make sure speed is set on pty so that editors work right*/
		sg.sg_ispeed = B9600;
		sg.sg_ospeed = B9600;
		/* reset t_brkc to default value */
		tc.t_brkc = -1;
		/* stty dec */
		tc.t_intrc = CINTR;	/* ^C */
		/* backspace-space-backspace to erase, and ^Q to restart */
		lmode |= LCRTERA | LDECCTQ | LPASS8;

		if (ioctl (tty, TIOCSETP, (char *)&sg) == -1)
			SysError (ERROR_TIOCSETP);
		if (ioctl (tty, TIOCSETC, (char *)&tc) == -1)
			SysError (ERROR_TIOCSETC);
		if (ioctl (tty, TIOCSETD, (char *)&discipline) == -1)
			SysError (ERROR_TIOCSETD);
		if (ioctl (tty, TIOCSLTC, (char *)&ltc) == -1)
			SysError (ERROR_TIOCSLTC);
		if (ioctl (tty, TIOCLSET, (char *)&lmode) == -1)
			SysError (ERROR_TIOCLSET);
#ifdef TIOCCONS
		if (Console) {
			int on = 1;
			if (ioctl (tty, TIOCCONS, (char *)&on) == -1)
				SysError(ERROR_TIOCCONS);
		}
#endif TIOCCONS

		close (open ("/dev/null", O_RDWR, 0));

		for (index1 = 0; index1 < 3; index1++)
			dup2 (tty, index1);
#ifdef UTMP
                pw = getpwuid(my_uid);
                if (pw && pw->pw_name)
                    Setenv ("LOGNAME=", pw->pw_name); /* for POSIX */
#ifdef __osf__
#define PTYCHARLEN 5
               /* Set up our utmp entry now.  We need to do it here
                ** for the following reasons:
                **   - It needs to have our correct process id (for
                **     login).
                **   - If our parent was to set it after the fork(),
                **     it might make it out before we need it.
                **   - We need to do it before we go and change our
                **     user and group id's.
                */

                (void) setutent ();
                /* set up entry to search for */
                ptyname = ttydev;
		if (PTYCHARLEN >= strlen(ptyname))
		    ptynameptr = ptyname;
		else
		    ptynameptr = ptyname + strlen(ptyname) - PTYCHARLEN;
                (void) strncpy(utmp.ut_id, ptynameptr, sizeof (utmp.ut_id));
                utmp.ut_type = DEAD_PROCESS;

                /* position to entry in utmp file */
                (void) getutid(&utmp);

                /* set up the new entry */
                utmp.ut_type = USER_PROCESS;
                utmp.ut_exit.e_exit = 2;
                (void) strncpy(utmp.ut_user,
                               (pw && pw->pw_name) ? pw->pw_name : "????",
                               sizeof(utmp.ut_user));

                (void)strncpy(utmp.ut_id, ptynameptr, sizeof(utmp.ut_id));
                (void) strncpy (utmp.ut_line,
                        ptyname + strlen("/dev/"), sizeof (utmp.ut_line));

                (void) strncpy(buf, DisplayString(display), sizeof(buf));
                {
                    char *disfin = rindex(buf, ':');
                    if (disfin)
                        *disfin = '\0';
                }
                (void) strncpy(utmp.ut_host, buf, sizeof(utmp.ut_host));

                (void) strncpy(utmp.ut_name, pw->pw_name, sizeof(utmp.ut_name));

                utmp.ut_pid = getpid();
                utmp.ut_time = time ((long *) 0);

                /* write out the entry */
                if (login_shell)
                    (void) pututline(&utmp);
#ifdef WTMP
                if ( login_shell &&
                     (i = open(etc_wtmp, O_WRONLY|O_APPEND)) >= 0) {
                    write(i, (char *)&utmp, sizeof(struct utmp));
                    close(i);
                }
#endif
                /* close the file */
                (void) endutent();

#else   /* __osf__ */

		if((*tslotp = ttyslot()) <= 0)
			SysError(ERROR_TSLOT3);
		if((pw = getpwuid(my_uid)) &&
		 (i = open(etc_utmp, O_WRONLY)) >= 0) {
			bzero((char *)&utmp, sizeof(struct utmp));
			(void) strncpy(utmp.ut_line, &ttydev[5],
		                        	sizeof(utmp.ut_line));
			(void) strncpy(utmp.ut_name, pw->pw_name,
			                        sizeof(utmp.ut_name));
			(void) strncpy(utmp.ut_host, XDisplayString(display),
			                        sizeof(utmp.ut_host));
			time(&utmp.ut_time);
			lseek(i, (long)(*tslotp * sizeof(struct utmp)), 0);
			write(i, (char *)&utmp, sizeof(struct utmp));
			close(i);
		} else
			*tslotp = -*tslotp;
#endif /* __osf__ */
#endif UTMP
	}

#ifdef sun
#ifdef TIOCSSIZE
	/* tell tty how big window is */
#ifdef DECTERM
	ts.ts_lines = rows;
	ts.ts_cols = cols;
#else
	if(screen->TekEmu) {
		ts.ts_lines = 38;
		ts.ts_cols = 81;
	} else {
		ts.ts_lines = screen->max_row + 1;
		ts.ts_cols = screen->max_col + 1;
	}
#endif DECTERM
	ioctl  (*pty_fdp, TIOCSSIZE, &ts);
#endif TIOCSSIZE
#else sun
#ifdef TIOCSWINSZ
	/* tell tty how big window is */
#ifdef DECTERM
	ws.ws_row = rows;
	ws.ws_col = cols;
	ws.ws_xpixel = width;
	ws.ws_ypixel = height;
#else
	if(screen->TekEmu) {
		ws.ws_row = 38;
		ws.ws_col = 81;
		ws.ws_xpixel = TFullWidth(screen);
		ws.ws_ypixel = TFullHeight(screen);
	} else {
		ws.ws_row = screen->max_row + 1;
		ws.ws_col = screen->max_col + 1;
		ws.ws_xpixel = FullWidth(screen);
		ws.ws_ypixel = FullHeight(screen);
	}
#endif DECTERM
	ioctl (*pty_fdp, TIOCSWINSZ, (char *)&ws);
#endif TIOCSWINSZ
#endif sun

	/*
	 * This code should be executed before the fork,
	 * so that the parent doesn't have to
	 * open a channel to the tty after the fork (which might
	 * cause a deadlock).
	 */

	if (!no_dev_tty) {
		if ((tty2 = open ("/dev/tty", O_RDWR, 0)) < 0)
			SysError(ERROR_OPDEVTTY3);
		for (index1 = 0; index1 < 3; index1++)
			dup2 (tty2, index1);
		if (tty2 > 2) close (tty2);
	}

	if (!am_slave) {
	    if ((*pidp = fork ()) == -1)
		SysError (ERROR_FORK);
		
	    if (*pidp == 0) {
		extern char **environ;
		int pgrp = getpid();

		close (Xsocket);
		close (*pty_fdp);
		if(fileno(stderr) >= 3)
			close (fileno(stderr));

		/*
		 * Clone the fd to /dev/ttyxx before closing it.
		 */

		for (index1 = 0; index1 < 3; index1++)
			dup2 (tty, index1);
		if (tty >= 0) close (tty);

		signal (SIGCHLD, SIG_DFL);
		signal (SIGHUP, SIG_IGN);
		/* restore various signals to their defaults */
		signal (SIGINT, SIG_DFL);
		signal (SIGQUIT, SIG_DFL);
		signal (SIGTERM, SIG_DFL);

		/* copy the environment before Setenving */
		for (i = 0 ; environ [i] != NULL ; i++) ;
		/*
		 * The `4' is the number of Setenv() calls which may add
		 * a new entry to the environment.  The `1' is for the
		 * NULL terminating entry.
		 */
		envnew = (char **) XtCalloc ((unsigned) i + (4 + 1), sizeof(char *));
		bcopy((char *)environ, (char *)envnew, i * sizeof(char *));
		environ = envnew;
		Setenv ("TERM=", TermName);
		if(!TermName)
			*newtc = 0;
		Setenv ("TERMCAP=", newtc);
#ifndef DECTERM
		sprintf(buf, "%d", screen->TekEmu ? (int)TWindow(screen) :
		 (int)VWindow(screen));
		Setenv ("WINDOWID=", buf);
#endif DECTERM
		/* put the display into the environment of the shell*/
		Setenv ("DISPLAY=", XDisplayString(display));

		signal(SIGTERM, SIG_DFL);
		setsid();
#ifdef TIOCSCTTY
		ioctl(0, TIOCSCTTY, 0);
#endif
		ioctl(0, TIOCSPGRP, (char *)&pgrp);
		setpgrp (0, 0);
		close(open(ttyname(0), O_WRONLY, 0));
		setpgrp (0, pgrp);

		setgid (my_gid);
		setuid (my_uid);

		if (command_to_exec) {
			execvp(*command_to_exec, command_to_exec);
			/* print error message on screen */
			fprintf(stderr, "%s: Can't execvp %s\n", xterm_name,
			 *command_to_exec);
		}
		signal(SIGHUP, SIG_IGN);
		if (get_ty) {
			ioctl (0, TIOCNOTTY, (char *) NULL);
			execl ("/etc/getty", "+", "Xwindow", get_ty, 0);
		}
		signal(SIGHUP, SIG_DFL);

#ifdef UTMP
		if(((ptr = getenv("SHELL")) == NULL || *ptr == 0) &&
		 ((pw == NULL && (pw = getpwuid(my_uid)) == NULL) ||
		 *(ptr = pw->pw_shell) == 0))
#else UTMP
		if(((ptr = getenv("SHELL")) == NULL || *ptr == 0) &&
		 ((pw = getpwuid(my_uid)) == NULL ||
		 *(ptr = pw->pw_shell) == 0))
#endif UTMP
			ptr = "/bin/sh";
		if(shname = rindex(ptr, '/'))
			shname++;
		else
			shname = ptr;
		ldisc = XStrCmp("csh", shname + strlen(shname) - 3) == 0 ?
		 NTTYDISC : 0;
		ioctl(0, TIOCSETD, (char *)&ldisc);
		execl (ptr, login_shell ? "-" : shname, 0);
		fprintf (stderr, "%s: Could not exec %s!\n", xterm_name, ptr);
		sleep(5);
		exit(ERROR_EXEC);
	    }
	}

	if(tty >= 0) close (tty);
	signal(SIGHUP,SIG_IGN);

	signal(SIGINT, goodbye); 
	signal(SIGQUIT, goodbye);
	signal(SIGTERM, goodbye);

	{
	int mode = 1;
	if (ioctl (*pty_fdp, FIONBIO, (char *)&mode) == -1) SysError (ERROR_FIONBIO);
	}

     if (am_slave) {      /* write window ID to pty */
          write( *pty_fdp, (char *) &window, sizeof(Window) );
          write( *pty_fdp, "\n", 1);
    }

	XSetIOErrorHandler( io_error_handler );
}

extern char login_shell_flag;

un_spawn( pty, tslot, get_ty, am_slave )
	int pty;  /* file descriptor of pty */
	int tslot;
	char *get_ty;
	int am_slave;
{
#ifdef UTMP
#ifdef __osf__
        struct utmp utmp;
        struct utmp *utptr;
        char *ptyname;
        char *ptynameptr;
#ifdef WTMP
        int fd;                 /* for /etc/wtmp */
        int i;
#endif
        /* cleanup the utmp entry we forged earlier */
        if (login_shell_flag) {
            ptyname = ttydev;
            utmp.ut_type = USER_PROCESS;
	    if (PTYCHARLEN >= strlen(ptyname))
		ptynameptr = ptyname;
	    else
		ptynameptr = ptyname + strlen(ptyname) - PTYCHARLEN;
            (void) strncpy(utmp.ut_id, ptynameptr, sizeof(utmp.ut_id));
            (void) setutent();
            utptr = getutid(&utmp);
            /* write it out only if it exists */
            if (utptr) {
                    utptr->ut_type = DEAD_PROCESS;
                    bzero(utptr->ut_user, sizeof(utptr->ut_user));
                    bzero(utptr->ut_host, sizeof(utptr->ut_host));
                    utptr->ut_time = time((long *) 0);
                    (void) pututline(utptr);
#ifdef WTMP
                    /* set wtmp entry if wtmp file exists */
                    if ((fd = open(etc_wtmp, O_WRONLY | O_APPEND)) >= 0) {
                      i = write(fd, utptr, sizeof(utmp));
                      i = close(fd);
                    }
#endif

            }
            (void) endutent();
        }
#else /* not __osf__ */

	register int i;
	struct utmp utmp;

	if(!am_slave && tslot > 0 && (i = open(etc_utmp, O_WRONLY)) >= 0) {
		bzero((char *)&utmp, sizeof(struct utmp));
		lseek(i, (long)(tslot * sizeof(struct utmp)), 0);
		write(i, (char *)&utmp, sizeof(struct utmp));
		close(i);
	}
#endif /* __osf__ */
#endif UTMP
        close(pty); /* close explicitly to avoid race with slave side */
#ifndef DECTERM
	if(screen->logging)
		CloseLog(screen);
#endif DECTERM

	if(!get_ty && !am_slave) {
		if ( group_struct != NULL )
		    {  /* tty group exists: Ultrix 4.0 or later */
		    /* Why were we restoring the ownership to gr_gid? */
		    /* chown (ttydev, 0, group_struct->gr_gid); */

		    /* restore ownership of tty */
		    chown (ttydev, 0, 0);

		    /* restore modes of tty */
		    chmod (ttydev, 0666);
		    }
		else
		    {  /* tty group doesn't exist: pre-Ultrix 4.0 */
		    /* restore ownership of tty */
		    chown (ttydev, 0, 0);

		    /* restore modes of tty */
		    chmod (ttydev, 0666);
		    }
	}
}

resize(rows, cols, TermName, oldtc, newtc)
int rows, cols;
char *TermName;
register char *oldtc, *newtc;
{
	register char *ptr1, *ptr2;
	register int i;
	register int li_first = 0;
	register char *temp;
	char *index(), *strindex();

	if ((ptr1 = strindex (oldtc, "co#")) == NULL){
		fprintf(stderr, "%s: Can't find co# in termcap string %s\n",
			xterm_name, TermName);
		exit (ERROR_NOCO);
	}
	if ((ptr2 = strindex (oldtc, "li#")) == NULL){
		fprintf(stderr, "%s: Can't find li# in termcap string %s\n",
			xterm_name, TermName);
		exit (ERROR_NOLI);
	}
	if(ptr1 > ptr2) {
		li_first++;
		temp = ptr1;
		ptr1 = ptr2;
		ptr2 = temp;
	}
	ptr1 += 3;
	ptr2 += 3;
	strncpy (newtc, oldtc, i = ptr1 - oldtc);
	newtc += i;
	sprintf (newtc, "%d", li_first ? rows : cols );
	newtc += strlen(newtc);
	ptr1 = index (ptr1, ':');
	strncpy (newtc, ptr1, i = ptr2 - ptr1);
	newtc += i;
	sprintf (newtc, "%d", li_first ? cols : rows );
	ptr2 = index (ptr2, ':');
	strcat (newtc, ptr2);
}
