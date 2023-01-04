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
static char rcs_id[] = "@(#)$RCSfile: command.c,v $ $Revision: 1.1.5.3 $ (DEC) $Date: 1993/12/21 14:34:30 $";
#endif

/*
 *                     Copyright (c) 1987, 1991 by
 *              Digital Equipment Corporation, Maynard, MA
 *                      All rights reserved.
 *
 *   This software is furnished under a license and may be used and
 *   copied  only  in accordance with the terms of such license and
 *   with the  inclusion  of  the  above  copyright  notice.   This
 *   software  or  any  other copies thereof may not be provided or
 *   otherwise made available to any other person.  No title to and
 *   ownership of the software is hereby transferred.
 *
 *   The information in this software is subject to change  without
 *   notice  and should not be construed as a commitment by Digital
 *   Equipment Corporation.
 *
 * DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
 * ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
 * DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
 * ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
 * WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
 * ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
 * SOFTWARE.
 */

/*
 */

/* command.c -- interface to exec mh commands. */

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/signal.h>
#include <sys/wait.h>
#include <sys/time.h>
#ifndef PRIO_MIN
#include <sys/resource.h>
#endif
#include <sys/file.h>
#include <errno.h>
#include "decxmail.h"

#ifndef FD_SET
#define NFDBITS         (8*sizeof(fd_set))
#define FD_SETSIZE      NFDBITS
#define FD_SET(n, p)    ((p)->fds_bits[(n)/NFDBITS] |= (1 << ((n) % NFDBITS)))
#define FD_CLR(n, p)    ((p)->fds_bits[(n)/NFDBITS] &= ~(1 << ((n) % NFDBITS)))
#define FD_ISSET(n, p)  ((p)->fds_bits[(n)/NFDBITS] & (1 << ((n) % NFDBITS)))
#define FD_ZERO(p)      bzero((char *)(p), sizeof(*(p)))
#endif FD_SET

extern int catch_sigchld();

/* Return the full path name of the given mh command. */

static char *FullPathOfCommand(str)
  char *str;
{
    static char result[100];
    (void) sprintf(result, "%s/%s", defMhPath, str);
    return result;
}

static int numpids = 0;
static int maxpids = 0;
static int *pids = NULL;

static void UpdateRunningProcesses()
{
    register int i, child, found;

    found = 0;
    if (numpids > 0) {
	do {
	    child = wait3((union wait *) NULL, WNOHANG,
			  (struct rusage *) NULL);
	    if (child > 0) {
		found++;
		for (i=0 ; i<numpids ; i++) {
		    if (child == pids[i]) {
                        numpids--;
                        pids[i] = 0;    /* In case numpids = 0. */
                        pids[i] = pids[numpids];
			break;
		    }
		}
	    }
	    if (child == -1 && found == 0) numpids = 0;
	} while (numpids > 0 && child > 0);
    }
}


static void AddRunningProcesses(pid)
int pid;
{
    numpids++;
    if (numpids > maxpids) {
	pids = (int *) XtRealloc((char *) pids, sizeof(int) * numpids);
	maxpids = numpids;
    }
    pids[numpids - 1] = pid;
    UpdateRunningProcesses();
}


static Boolean IsRunningProcess(pid)
int pid;
{
    register int i;
    for (i=0 ; i<numpids ; i++) {
	if (pids[i] == pid) return TRUE;
    }
    return FALSE;
}

static void WaitForBackground()
{
    fd_set readfds, fds;
    int connum;
    struct timeval timeout;
    timeout.tv_usec = 0;
    timeout.tv_sec = 0;


    FD_ZERO(&fds);
    if (theDisplay) {
	connum = ConnectionNumber(theDisplay);
	FD_SET(connum, &fds);
    } else {
	connum = 0;
    }
    if (numpids == 0) return;
    BeginLongOperation();
/*    while (numpids > 0) { */
	UpdateRunningProcesses();
	readfds = fds;
	if (select(connum+1, (int *) &readfds, (int *) NULL, (int *) NULL,
		       &timeout ) < 0) {
	    if (errno != EINTR)
		Punt("Select failed!");
	}
	if (theDisplay && FD_ISSET(connum, &readfds)) {
	    (void) SaveQueuedEvents();
	}
/*    } */
    RestoreQueuedEvents();
    EndLongOperation();
}

static int childdoneflag;		/* Gets nonzero when the child process
				   finishes. */
static int backdone = FALSE;
ChildDone()
{
    DEBUG(("ChildDone incrementing counters\n"));
    childdoneflag++;
    backdone++;
}

/* Execute the given command, and wait until it has finished.  While the
   command is executing, watch the X socket and cause Xlib to read in any
   incoming data.  This will prevent the socket from overflowing during
   long commands. */

DoCommand(argv, inputfile, outputfile)
  char **argv;			/* The command to execute, and its args. */
  char *inputfile;		/* Input file for command. */
  char *outputfile;		/* Output file for command. */
{
    int pid, connum;
    fd_set readfds, fds;
    Boolean thischilddone;
    FD_ZERO(&fds);
    if (theDisplay) {
	connum = ConnectionNumber(theDisplay);
	FD_SET(connum, &fds);
    } else {
	connum = 0;
    }

    WaitForBackground();
    childdoneflag = 0;
    (void) signal(SIGCHLD, ChildDone);
#ifdef VFORK
    while ((pid = vfork()) == -1)
#else
    while ((pid = fork()) == -1)
#endif VFORK
	sleep_for_child();
    if (pid == 0)				/* We're the child process. */
	DoExecToFile(argv,inputfile,outputfile);
    else {					/* We're the parent process. */
	AddRunningProcesses(pid);
	BeginLongOperation();
	thischilddone = FALSE;
	while (!thischilddone) {
	    readfds = fds;
	    if (childdoneflag == 0 &&
		select(connum+1, (int *) &readfds, (int *) NULL, (int *) NULL,
		       (struct timeval *) NULL) < 0) {
		if (errno != EINTR)
		    Punt("Select failed!");
	    }
	    if (childdoneflag) {
		UpdateRunningProcesses();
		if (IsRunningProcess(pid))
		    childdoneflag--;
		else
		    thischilddone = TRUE;
	    }
	    if (theDisplay && FD_ISSET(connum, &readfds)) {
		(void) SaveQueuedEvents();
	    }
	}
	DEBUG((" done\n"));
	RestoreQueuedEvents();
	EndLongOperation();
    }
}

DoExecToFile(argv,inputfile,outputfile)
char	**argv, *inputfile, *outputfile;
{
FILE *fid;

#ifdef DO_DEBUG
    if (debug)
	pr_argv(argv);
#endif DO_DEBUG

    if (inputfile) {
	fid = FOpenAndCheck(inputfile, "r");
	(void) dup2(fileno(fid), fileno(stdin));
    }
    if (outputfile) {
	fid = FOpenAndCheck(outputfile, "w");
	(void) dup2(fileno(fid), fileno(stdout));
    }
    if (outputfile)
	(void) dup2(fileno(fid), fileno(stderr));
    else {
	fid = FOpenAndCheck("/dev/null", "w");
	(void) dup2(fileno(fid), fileno(stderr));
    }

    (void) execv(FullPathOfCommand(argv[0]), argv);
    (void) execvp(argv[0], argv);
    Punt("Execvp failed!");
    _exit(1);	/* Do we want the child still executing...? Sooo, kill it. */
}

/* Execute the given command, waiting until it's finished.  Put the output
   in a newly mallocced string, and return a pointer to that string. */

char *DoCommandToString(argv)
char ** argv;
{
    char *result;
    char *file;
    int fid, length;
    file = DoCommandToFile(argv);
    length = GetFileLength(file);
    result = XtMalloc((unsigned) length + 1);
    fid = myopen(file, O_RDONLY, 0666);
    if (length != read(fid, result, length))
	Punt("Couldn't read result from DoCommandToString");
    result[length] = 0;
    DEBUG(("('%s')\n", result));
    (void) myclose(fid);
    DeleteFileAndCheck(file);
    return result;
}
    

#ifdef NOTDEF	/* This implementation doesn't work right on null return. */
char *DoCommandToString(argv)
  char **argv;
{
    static char result[1030];
    register int fildes[2], pid, l;

    DEBUG(("Executing %s ...", argv[0]));
    (void) pipe(fildes);
#ifdef VFORK
    while ((pid = vfork()) == -1)
#else
    while ((pid = fork()) == -1)
#endif VFORK
	sleep_for_child();
    if (pid) {
	while (wait((union wait *) 0) == -1) ;
	l = read(fildes[0], result, 1024);
	if (l <= 0) Punt("Couldn't read result from DoCommandToString");
	(void) myclose(fildes[0]);
	result[l] = 0;
	while (result[--l] == 0) ;
	while (result[l] == '\n') result[l--] = 0;
	DEBUG((" done: '%s'\n", result));
	return result;
    } else {
	(void) dup2(fildes[1], fileno(stdout));
	(void) execv(FullPathOfCommand(argv[0]), argv);
	(void) execvp(argv[0], argv);
	Punt("Execvp failed!");
	return NULL;		/* EXIT ? */
    }
}
#endif NOTDEF

/* Execute the command to a temporary file, and return the name of the file. */

char *DoCommandToFile(argv)
char **argv;
{
    static char name[256];
    strcpy(name, MakeNewTempFileName());
    DoCommand(argv, (char *) NULL, name);
    return name;
}


typedef struct _BackgroundRec {
    int pid;
    XtWorkProc proc;
    Opaque param;
    struct _BackgroundRec *next;
} BackgroundRec, *Background;

Background backlist = NULL;


static void CheckForBackgroundDone()
{
    Background back, last, next;
    if (backdone) {
	backdone = FALSE;
	(void) signal(SIGCHLD, ChildDone);
	UpdateRunningProcesses();
	last = NULL;
	back = backlist;
	while (back) {
	    next = back->next;
	    if (!IsRunningProcess(back->pid)) {
		DEBUG(("done\n"));
		if (back->proc)
		    XtAddWorkProc(back->proc, back->param);
		if (last) {
		    last->next = next;
		} else {
		    backlist = next;
		}
		XtFree((char *) back);
	    } else {
		last = back;
	    }
	    back = next;
	}
    }
    if (backlist != NULL) {
	(void) XtAddTimeOut((unsigned long) 2000, (XtTimerCallbackProc) CheckForBackgroundDone,
			    NULL);
    }
}



/*
 * Execute the given command.  This procedure will return immediately.  When
 * the command finishes, the given procedure will be called, with the given
 * parameter as its only argument.  The procedure will be called as a work
 * procedure, so all toolkit calls will be legal.
 */

void DoCommandInBackground(argv, inputfile, outputfile, proc, param)
char **argv;
char *inputfile, *outputfile;
XtWorkProc proc;
Opaque param;
{
    Background back;
    int pid;

    DEBUG(("Backgrounding %s ...", argv[0]));
#ifdef VFORK
    while ((pid = vfork()) == -1)
#else
    while ((pid = fork()) == -1)
#endif VFORK
	sleep_for_child();
    if (pid == 0)			/* We're the child process. */
	DoExecToFile(argv,inputfile,outputfile);
    else {				/* We're the parent process. */
	AddRunningProcesses(pid);
	back = XtNew(BackgroundRec);
	back->pid = pid;
	back->proc = proc;
	back->param = param;
	back->next = backlist;
	(void) signal(SIGCHLD, ChildDone);
	if (backlist == NULL) {
	    backlist = back;
	    CheckForBackgroundDone();
	} else backlist = back;
      }
}

/* If a fork didn't work, wait for a process to die that is already forked.
 * It does away with some of the 'backgrounding' features, but what else?
 * If there are no more backgrounded processes and we STILL can't fork,
 * give it all up and go home for a nice cup of tea...
 */
sleep_for_child()
{
register int	i, child;

    if (numpids < 1)
	Punt("Couldn't fork!");		/* Not a lot I can really do... */
    else {
	child = wait((union wait *)0);	/* So what with the status! */
	if (child > 0) {		/* A child has died... */
	    for (i=0; i<numpids; i++) {
		if (child == pids[i]) {
                    numpids--;
                    pids[i] = 0;    /* In case numpids = 0. */
                    pids[i] = pids[numpids];
		    break;
		}
	    }
	}
    }
}

pr_argv(argv)
char	**argv;
{
    while (*argv != (char *)NULL) {
	fprintf(stderr,"%s ",*argv);
	++argv;
    }
    fputc('\n', stderr);
}

check_bg_processes(w)
Widget w;
{

  if (numpids) {
    Scrn scrn = ScrnFromWidget(w);
    Warning(scrn->parent,"Active edits","Exit all editors before quitting!\n");
    return TRUE;
  }
  else
    return FALSE;
    
}
