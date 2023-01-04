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
/* header automatically added by the build system */
static char *BuildSystemHeader = "$Header: /usr/sde/osf1/rcs/x11/src/motif/clients/session/session/unixcallbacks.c,v 1.1.7.2 1993/06/25 18:57:42 Paul_Henderson Exp $";
#endif

#include "smdata.h"
#include "smresource.h"
#include <stdio.h>
#include <stdlib.h>
#  include <sys/file.h>

#ifndef _BSD
#  define _BSD
#  include <sys/wait.h>
#  undef _BSD
#else
#  include <sys/wait.h>
#endif

#include <sys/time.h>
#include <sys/ioctl.h>
#include <signal.h>
#include <netdb.h>

#include <X11/Shell.h>
#include <X11/StringDefs.h>
#include <X11/Xproto.h>
#include <X11/Xatom.h>

char global_label[1024];
int timerInterval = 5000;

static Window *windows; /* list of windows which support WM_SAVE_YOURSELF */
static int *states;     /* list of above windows' states */
static char **responses; /* list of command lines from above windows */
static int num_windows, num_responses; /* count of client windows, etc. */
static Atom WM_STATE, WM_SAVE_YOURSELF, WM_COMMAND, WM_PROTOCOLS;
static Boolean error_encountered;
static void FindSaveYourselfClientWindows(), GetChildren(),
	    FindClientWindow();
Boolean HasSaveYourself();
static void set_error_handler(), remove_error_handler();
static int error_handler();
static void WatchForNewWMCommand(), WriteCommands(), ModifyArguments();
static char *QuoteString(), *QuoteQuotedString();

typedef struct _cl_info {
	Boolean query, add_display, dxwm, add_iconic;
	char *filename; /* file to write command lines into */
	int timeout;    /* timeout for response wait */
} Cl_info, *Cl_infoPtr;

static Cl_info cl_info;

#define RSH_COMMAND "rsh" /* command to execute a remote shell */

extern OptionsRec options;

static struct proc {
	char *name;
	int pid;
	struct	proc *next;
	Boolean quiet;
} head = { NULL, 0, NULL, TRUE};

static char utmpName [1024];

#ifdef _NO_PROTO
void SendClientMsg (dsp, window, type, data0, time, pData, dataLen)
Display *dsp;
Window window;
long type;
long data0;
Time time;
long *pData;
int dataLen;

#else /* _NO_PROTO */
void SendClientMsg (Display *dsp, Window window, long type, long data0,\
		    Time time, \
		    long *pData, int dataLen)
#endif /* _NO_PROTO */
{
    XClientMessageEvent clientMsgEvent;
    int i;

    clientMsgEvent.type = ClientMessage;
    clientMsgEvent.window = window;
    clientMsgEvent.message_type = type;
    clientMsgEvent.format = 32;
    clientMsgEvent.data.l[0] = data0;
    clientMsgEvent.data.l[1] = (long)time;
    if (pData)
    {
	/*
	 * Fill in the rest of the ClientMessage event (that holds up to
	 * 5 words of data).
	 */

        if (dataLen > 3)
        {
	    dataLen = 3;
        }
        for (i = 2; i < (2 + dataLen); i++)
        {
	    clientMsgEvent.data.l[i] = pData[i];
        }
    }
    
    
    XSendEvent (dsp, window, False, NoEventMask,
	(XEvent *)&clientMsgEvent);
    XFlush(dsp);


} /* END OF FUNCTION SendClientMsg */

/*************************************<->*************************************
 *
 *  TimeoutProc (client_data, id)
 *
 *
 *************************************<->***********************************/

#ifdef _NO_PROTO
void TimeoutProc (client_data, id)
	caddr_t client_data;
	XtIntervalId *id;

#else /* _NO_PROTO */
void TimeoutProc (caddr_t client_data, XtIntervalId *id)
#endif /* _NO_PROTO */
{
  
    /* Kill Mwm and all other clients*/
    killpg(getpgrp(0), SIGTERM);
    killpg(getpgrp(0), SIGHUP); 

    /* * Free up the timer. */
    XtRemoveTimeOut (*id);
    exit(0);

}


doexecstr(str, number, quiet)
char *str;
int number;
Boolean quiet;
{
    struct proc *newproc;
    char *shell = "/bin/sh";
    char *temp_shell;
    int pid;
    int sig_mask;
    char *name = NULL;
    char *ptr;
    char buf[2048];
    char buf1[1024];
    char msgbuf[2048];
#define EXECSTR "exec "

    if(temp_shell = getenv("SHELL"))
	if (strlen(temp_shell))
	    shell = temp_shell;
    sprintf(buf,"%s %s", EXECSTR, str);
    if ((ptr = index(str, ' ')) != NULL) {
    	strncpy(buf1, str, ptr - str);
    	buf1[ptr-str] = 0;
    	name = rindex(buf1,'/');
    	if(name) name++;
    	else name = buf1;
    	if (number) {
    	    sprintf(ptr = XtMalloc(strlen(name)+5),"%s%d", name,number);
    	    name = ptr;
    	    strcat(buf, " -n ");
    	    strcat(buf,name);
    	}
    }
    else {
    	name = rindex(str,'/');
    	if(name) name++;
    	else name = str;
    	if (number) {
    	    sprintf(ptr = XtMalloc(strlen(name)+5),"%s%d", name,number);
    	    name = ptr;
    	    strcat(buf, " -n ");
    	    strcat(buf,name);
    	}
    }

    if (global_label && strlen(global_label)) {
        name = XtMalloc(strlen(global_label)+1);
        strcpy(name, global_label);
    }
    sig_mask = sigblock(sigmask(SIGCHLD));
    if ((pid = vfork()) == 0) {
    	int i;
    	for(i = 1; i < 32 ; i ++) signal(i, SIG_DFL);
    	sigsetmask(0);
    	close(0);
    	close(ConnectionNumber(XtDisplay(smdata.toplevel)));
    	open("/dev/null", 0, 0);
    	execlp(shell, shell, "-c", buf, 0);
    	exit(-1);
    }
    else if (pid > 0) {
    	newproc = (struct proc *)XtMalloc(sizeof(*newproc));
    	newproc->pid = pid;
    	newproc->name = strcpy((char *)XtMalloc(strlen(name)+1), name);
    	newproc->next = head.next;
    	head.next = newproc;
    	sigsetmask(sig_mask);
	if(!(newproc->quiet = quiet)) {
	    if (global_label && strlen(global_label)) {
		sprintf(msgbuf, "Starting Application Process %d \"%s\"\n",
		       	newproc->pid, newproc->name);
	    } else {
		sprintf(msgbuf, "Starting Process %d \"%s\"\n",
	       	newproc->pid, newproc->name);
	    }
    	}
    }
    else {
    	sigsetmask(sig_mask);
    	if (!quiet) sprintf(msgbuf, "Can't start %s (can't fork)\n", name);
    }
    fprintf(stderr, "%s", msgbuf); fflush(stderr);
}

XtEventHandler unixunidle(w, client_data, event)
Widget w;
XtPointer client_data;
XEvent *event;
{
    if (utmpName && strlen(utmpName)) utimes(utmpName, NULL);
    else XtRemoveEventHandler(w, EnterWindowMask, True,
			      (XtEventHandler)unixunidle, 0);
}

unixrundown(widgetID)
  Widget *widgetID;
  
{
    FILE *fp;
    char buf[1024];
    XtAppContext appContext;
    XEvent event;
    XtIntervalId timerId, id;
    Display      *DISPLAY;

    int i;

#ifdef DEBUG
    char *name;
    printf( "unixrundown: \n" );
#endif /*DEBUG*/

    signal(SIGCHLD, SIG_IGN);
    signal(SIGTERM, SIG_IGN);
    signal(SIGHUP, SIG_IGN);

    DISPLAY = XtDisplay (*widgetID);
    
    sprintf(buf,"/usr/bin/X11/getcons %s -", utmpName);

    if((fp = popen(buf, "r")) == NULL) perror("getcons");
    else pclose(fp);

   /* in reality this will do little good - xterm can't be
    * killed by us and many others will be of the wrong
    * pgrp or ignore these signals.  we tried.
    */

    /*  Get the atoms etc. and change property of root window to
	send a save_yourself message to all the clients.  The actual
	killing of process group is done in the timeout procedure
    */
    /*  The SAVE_YOURSELF does not seem to properly migrate to clients.
	So explicitly send out a SAVE_YOURSELF to each client looking 
	for it (ph-unx) */
    
    WM_SAVE_YOURSELF = XInternAtom(DISPLAY, "WM_SAVE_YOURSELF", False);
    WM_COMMAND       = XInternAtom(DISPLAY, "WM_COMMAND", False);
    WM_STATE         = XInternAtom(DISPLAY, "WM_STATE", False);
    WM_PROTOCOLS     = XInternAtom(DISPLAY, "WM_PROTOCOLS", False);
    
    appContext = XtWidgetToApplicationContext(*widgetID);

    /* find clients windows that would like to receive the SAVE_YOURSELF 
       message <<< */
    FindSaveYourselfClientWindows( DISPLAY, XtScreen(*widgetID),
				   &windows, &num_windows, &states );

#ifdef DEBUG
	printf( "unixrundown: numwindows = %d\n", num_windows );
#endif

    if( num_windows )
      {
      responses = (char **) XtMalloc( num_windows*sizeof( char * ) );
      num_responses = 0;
      for( i=0; i<num_windows; i++ )
	 {
	 responses[ i ] = NULL;
	 XSelectInput( DISPLAY, windows[ i ], PropertyChangeMask );
#ifdef DEBUG
	   XFetchName( DISPLAY, windows[ i ], &name );
	   printf("Sending WM_SAVE_YOURSELF to window %d, %s, (0x%x). \n",
		   i, name, windows[i] );
	   XFree( name );
#endif /*DEBUG*/
	 event.type = ClientMessage;
	 event.xclient.display = DISPLAY;
	 event.xclient.window = windows[ i ];
	 event.xclient.message_type = WM_PROTOCOLS;
	 event.xclient.format = 32;
	 event.xclient.data.l[0] = (long) WM_SAVE_YOURSELF;
/*<<<
	 event.xclient.data.l[1] = (long) CurrentTime;
*/
	 XSendEvent( DISPLAY, windows[i], False, NULL, &event );
	 }
      XSync( DISPLAY, False );

      id = XtAppAddTimeOut( appContext, timerInterval, 
			    (XtTimerCallbackProc)TimeoutProc, NULL );
      while( True )
	{
	XtAppNextEvent( appContext, &event );
	if ( event.type == PropertyNotify )
	   {
	   WatchForNewWMCommand( DISPLAY, &event, &id );
	   }
	else
	   {
	   XtDispatchEvent( &event );
	   }
	}
     }
     else /* num_windows */
	{
	exit(0);
	}
}
void
reaper()
{
    union wait status;
    int pid;
    struct proc *proc;
    struct proc *theone;

    pid = wait3(&status, WNOHANG, 0);
    if(pid == -1 || pid == 0) return;

    for(proc = &head; proc->next != NULL ; proc= proc->next){
    	if(proc->next->pid == pid) {
      	    theone = proc->next;
	    if(!theone->quiet || status.w_retcode != 0) {
	        fprintf(stderr, "Process %d exited \"%s\"\n",
				pid, theone->name);
		fflush(stderr);
	    }
	    XtFree(theone->name);
	    proc->next = proc->next->next;
	    break;
	}
    }
}

static looping = TRUE;

static
foo(display,event) 
Display      *display;
XErrorEvent  *event;
{
    char buf[256];

    if (event->request_code ==  X_ChangeWindowAttributes) {
	looping = FALSE;
	return;
    } 
  
    XGetErrorText(display, event->error_code, buf, 256);
    fprintf(stderr, "X protocol error detected by server: %s\n", buf);
    fprintf(stderr, "  Failed request major op code %d\n",
	    (int) event->request_code);
    fflush(stderr);
}

void
waitforwm(wid)
Widget wid;
{
    int status;

    XSetErrorHandler(foo);
    for (;head.next && looping ; ) {
        sleep(1);
        XGrabServer(XtDisplay(wid));
        XSelectInput(XtDisplay(wid), DefaultRootWindow(XtDisplay(wid)),
                     SubstructureRedirectMask);
        XSync(XtDisplay(wid), False);
        if(looping)
            XSelectInput(XtDisplay(wid), DefaultRootWindow(XtDisplay(wid)), 0);
        XUngrabServer(XtDisplay(wid));
        XSync(XtDisplay(wid), False);
    }
}


#define XtNstartupfile  "startupfilename"
#define XtCStartupfile  "Startupfilename"
#define XtNwmName	"windowManagerName"
#define XtCWmName	"WindowManagerName"
#define XtNtermName	"terminalEmulatorName"
#define XtCTermName	"TerminalEmulatorName"
#define XtNueName	"ueName"
#define XtCUeName	"UeName"
#define XtNnoCaution	"noCaution"
#define XtCNoCaution	"NoCaution"
#define XtNdontPutDatabase	"dontPutDatabase"
#define XtCDontPutDatabase	"DontPutDatabase"
#define XtNvues         "create_vue"
#define XtCvues          "Create_vue"

/* Removed from above
   #define XtNterms        "create_terminal"
   #define XtCterms        "Create_terminal"
*/

typedef struct {
    unsigned int changed;
    Widget sm_attr_id;
    Widget state_id;
    unsigned int startup_state;
    Widget mapped_id;
    Widget icon_id;
    Widget header_id;
    char *header_text;
    Widget pause_id;
    char *pause_text;
    unsigned int x;
    unsigned int y;
    unsigned int end_confirm;
    unsigned int end_confirm_id;
    char managed;
    unsigned int terms;
    unsigned int vues;
} smsetupType;

static XtResource applicationResources[] = {
    {XtNstartupfile, XtCStartupfile, XtRString, sizeof(char *) ,
	XtOffsetOf(OptionsRec, startupfile),  XtRString,
	".X11Startup"},
    {XtNwmName, XtCWmName, XtRString, sizeof(char *),
        XtOffsetOf(OptionsRec, wmstring), XtRString,
	"/usr/bin/X11/mwm"},
    {XtNtermName, XtCTermName, XtRString, sizeof(char *),
	XtOffsetOf(OptionsRec, termstring), XtRString,
	"/usr/bin/X11/xterm -ls"},
    {XtNueName, XtCUeName, XtRString, sizeof(char *),
	XtOffsetOf(OptionsRec, uestring), XtRString,
	"/usr/bin/X11/dxue"},
    {XtNnoCaution, XtCNoCaution, XtRBoolean, sizeof(Boolean),
	 XtOffsetOf(OptionsRec, nocautions), XtRString, "FALSE"},
    {XtNdontPutDatabase, XtCDontPutDatabase, XtRBoolean, sizeof(Boolean),
	 XtOffsetOf(OptionsRec, dontPutDatabase), XtRString,
	 "FALSE"},
    {XtNvues, XtCvues, XtRInt, sizeof(int),
	 XtOffsetOf(smsetupType, vues), XtRString, "0"},
    {"dontStartMultipleWM", "DontStartMultipleWM",
	 XtRBoolean, sizeof(Boolean),
	 XtOffsetOf(OptionsRec, dontStartMultipleWM), XtRString,
	 "FALSE"},
} ;

/* Removed from above
       {XtNterms, XtCterms, XtRInt, sizeof(int),
	 XtOffsetOf(smsetupType, terms), XtRString, "1"},
*/

void
doinit(wid)
Widget wid;
{
    static Atom DEC_PROMPTER_LANGUAGE = None;
    Atom actual_type;
    int actual_format;
    unsigned long leftover;
    unsigned long nitems;
    char *prompter_language;
      
    XtGetApplicationResources(wid, &options, applicationResources,
			      XtNumber(applicationResources),NULL, 0);

    if (DEC_PROMPTER_LANGUAGE == None) {
	DEC_PROMPTER_LANGUAGE = XInternAtom(display_id,
					    "DEC_PROMPTER_LANGUAGE",
					    FALSE);
    }
    if (XGetWindowProperty(display_id,
			 RootWindowOfScreen(XtScreen(smdata.toplevel)),
			 DEC_PROMPTER_LANGUAGE, 0L, 256L,
			 False, DEC_PROMPTER_LANGUAGE,
			 &actual_type, &actual_format, &nitems,
			 &leftover,
			 (unsigned char **) &prompter_language) == Success)
    {
	if (prompter_language && strlen(prompter_language))
	    def_table[ilanguage].def_value = prompter_language;
	XFree(prompter_language);
    }
}

startwm()
  {
    int i, numscreens;
    char *old_display, *ptr, new_display[256], wm_command[256];
    
    numscreens = XScreenCount(display_id);
    if (numscreens == 1) 
      {
	doexecstr(windowsetup.wmother, 0, TRUE);
      }
    else if (strcmp(windowsetup.wmother, "/usr/bin/X11/mwm") == 0)
    /* if the other window manger is the default one */
      {
	strcpy(wm_command, windowsetup.wmother);
	strcat(wm_command, " -multiscreen");
	doexecstr(wm_command, 0, TRUE);
      }
    
    else 
      {
	/* start one for screen 0 */
	doexecstr(windowsetup.wmother, 0, TRUE);
	if (!options.dontStartMultipleWM) 
	  {
	
	    /* For each screen, start a wm */
	    old_display = (char *)getenv("DISPLAY");
	    for (i=0; i < numscreens; i++) {
	      if (i != XDefaultScreen(display_id)) {
		strcpy(new_display, old_display);
		ptr = rindex(new_display, ':');
		if (ptr && (ptr = index(ptr, '.'))) *ptr = '\0';
		strcat(new_display, ".");
		sprintf(new_display+strlen(new_display), "%d", i);
		setenv("DISPLAY=", new_display);
		doexecstr(windowsetup.wmother, 0, TRUE);
	      }
	    }
        setenv("DISPLAY=", old_display);
	  }
      }
  }
  

void
start_terminal()
{
    static int term_number = 0;

    doexecstr(options.termstring,term_number++,FALSE);
}

void
start_vue()
{
    static int ue_number = 0;

    doexecstr(options.uestring, ue_number++,FALSE);
}

int
start_unix_command (command,screennum,label_text)
char *command, *label_text;
int  screennum;
{
    char *old_display, *ptr, new_display[256];

    if (label_text) strcpy(global_label, label_text);
    else strcpy(global_label, "");

    old_display = (char *)getenv("DISPLAY");
    strcpy(new_display, old_display);
    ptr = rindex(new_display, ':');
    if (ptr && (ptr = index(ptr, '.'))) *ptr = '\0';
    strcat(new_display, ".");
    sprintf(new_display+strlen(new_display), "%d", screennum);
    setenv("DISPLAY=", new_display);

    doexecstr(command,0,FALSE);
    strcpy(global_label, "");

    setenv("DISPLAY=", old_display);
    return (1);
}

void
DoStartUp()
{
    int fd;
    int pid;
    int sig_mask;
    struct proc *newproc;
    char *sh = "/bin/sh";

    if (getenv("SHELL")) sh = (char *)getenv("SHELL");

    if ((fd = open(options.startupfile, O_RDONLY)) == -1) return;

    sig_mask = sigblock(sigmask(SIGCHLD));
    if (!(pid = vfork())) {
	int i;
	for(i = 1; i < 32 ; i ++) signal(i, SIG_DFL);
	sigsetmask(0);
        dup2(fd, 0);
        close(ConnectionNumber(XtDisplay(smdata.toplevel)));
        execlp(sh, sh, "-s", 0);
        exit(-1);
    }
    newproc = (struct proc *)XtMalloc(sizeof(*newproc));
    newproc->quiet = TRUE;
    newproc->pid = pid;
    newproc->name = strcpy((char *)XtMalloc(strlen(options.startupfile)+1),
		           options.startupfile);
    newproc->next = head.next;
    head.next = newproc;
    sigsetmask(sig_mask);
}

void get_pty_console(displayName)
char *displayName;
{
    FILE *fp;
    char buf[1024], *ptr;

    sprintf(buf,"/usr/bin/X11/getcons %s", displayName);
    if((fp = popen(buf, "r")) == NULL) {
        perror("getcons");
        return;
    }

    *buf = '\0';
    if (fscanf(fp,"%s",buf) == EOF) {
        fflush(fp);
        if (fscanf(fp,"%s",buf) == EOF) {
            pclose(fp);
            return;
        }
    }
    pclose(fp);
    sprintf(utmpName, "%s", buf); /* save utmp name for later */
 
#ifndef RETURN_PTY
    if (strlen(buf) > 0) {
        if (ptr = rindex(buf, '/')) ptr++;
        if (ptr && *ptr == 't') *ptr = 'p';
    }
#endif
}
/* return the window id's and states of all client windows for the given
   screen which support WM_SAVE_YOURSELF */
static void
FindSaveYourselfClientWindows( DISPLAY, scrn, wlist, wnum, slist )
	Display *DISPLAY;
	Screen *scrn;
	Window **wlist;
	int *wnum;
	int **slist;
{
	Window root, *children, client;
	int numchildren, i, state;

	*wnum = 0;
	*wlist = NULL;

	/* find the root window */
	root = RootWindowOfScreen( scrn );
	
	/* get its children */
	GetChildren( DISPLAY, root, &children, &numchildren );
#ifdef DEBUG
	printf( "FindSaveYourselfClientWindows: numchildren = %d\n",
		 numchildren );
#endif

	/* find each child who would like to get WM_SAVE_YOURSELF */
	for( i=0; i<numchildren; i++ )
	   {
	   FindClientWindow( DISPLAY, children[i], &client, &state );
	   if( client && state != WithdrawnState )
	     {
#ifdef DEBUG
	     printf( "client window found!\n" );
#endif
	     if( HasSaveYourself( DISPLAY, client ) )
	       {
#ifdef DEBUG
	       printf( "client window has save yourself!\n" );
#endif
	       (*wnum)++;
	       *wlist = (Window *) XtRealloc( (char *)*wlist, 
				   	      *wnum*sizeof(Window) );
	       *slist = (int *) XtRealloc( (char *)*slist, 
					      *wnum*sizeof(int) );
	       (*wlist)[*wnum-1] = client;
	       (*slist)[*wnum-1] = state;
	       }
	     }
	   }
}
/* GetChildren (not recommended :-) )
	returns the child of a given window
*/
static void
GetChildren( DISPLAY, window, children, numchildren )
	Display *DISPLAY;
	Window window, **children;
	unsigned int *numchildren;
{
	Window returnroot, returnparent;
	Status status;

	error_encountered = False;
	set_error_handler();
	/* interogate the widget tree */
	status = XQueryTree( DISPLAY, window, &returnroot, &returnparent,
			     children, numchildren );

	if( error_encountered || !status )
	  *numchildren = 0;
	remove_error_handler();
}
static void
FindClientWindow( DISPLAY, window, winp, statep )
	Display *DISPLAY;
	Window window, *winp;
	int *statep;
{
	Window *children;
	int rvalue, numchildren, i;
	Atom actualtype;
	unsigned long nitems, bytesafter;
	int actualformat; 
	unsigned char *propreturn;

	error_encountered = False;
	set_error_handler();

	rvalue = XGetWindowProperty( DISPLAY, window, WM_STATE, 0, 1,
				     False, AnyPropertyType, &actualtype,
				     &actualformat, &nitems, &bytesafter,
				     &propreturn );

	remove_error_handler();
	
	if( !error_encountered && rvalue == Success && actualtype != None )
	  {
	  bcopy( propreturn, (char *)statep, sizeof(int) );
	  XFree( propreturn );
	  *winp = window;
	  return;
	  }
	
	GetChildren( DISPLAY, window, &children, &numchildren );
	*winp = NULL;
	for( i=0; i<numchildren && !*winp; i++ )
	   FindClientWindow( DISPLAY, children[i], winp, statep );

	if( numchildren )
	  XFree( (char *)children );
}
/*
 * HasSaveYourself
 * determine if a window supports WM_SAVE_YOURSELF by looking for it in
 * its WM_PROTOCOLS list
 */
Boolean
HasSaveYourself( DISPLAY, window )
	Display *DISPLAY;
	Window window;
{
	int rvalue, i, nitems;
	Atom *propreturn;

	rvalue = XGetWMProtocols( DISPLAY, window, &propreturn, &nitems );
	if (!rvalue)
	   return False;
	else
	   {
	   for( i=0; i<nitems; i++ )
	      {
	      if( propreturn[i] == WM_SAVE_YOURSELF )
		{
	        /* got it! */
		XFree( (char *)propreturn );
		return True;
		}
	      }
	    XFree( (char *)propreturn );
	    return False;
	    }
}
static void set_error_handler()
{
	XSetErrorHandler( error_handler );
}
static void remove_error_handler()
{
	XSetErrorHandler( NULL );
}
static int error_handler()
{
	error_encountered = True;
}
/* WatchForNewWMCommand
 * watches for WM_COMMAND property changes. If it is the last
 * one, turn off the timeout and go directly to WriteCommands
 */
void
WatchForNewWMCommand( DISPLAY, event, id )
	Display *DISPLAY;
	XEvent *event;
	XtIntervalId *id;
{	
	int rvalue, i, j, argc, sum;
	char **v, **argv, *h1, *h2, work[1000];
	struct hostent *he;
	Atom actualtype;
	int actualformat;
	unsigned long nitems, bytesafter;
	unsigned char *propreturn;
	char hostbuf[1000];
	Boolean do_quotes=False, doing_rsh=False;

	if( event->type == PropertyNotify )
	  {
	  for( i=0;
	       i<num_windows && event->xproperty.window != windows[i];
	       i++ )
	     {;}
	  if( event->xproperty.window == windows[i] &&
	      event->xproperty.atom   == WM_COMMAND &&
	      event->xproperty.state  == PropertyNewValue )
	    {
	    rvalue = XGetCommand( event->xproperty.display, 
			          event->xproperty.window,
				  &v, &argc );
	    if( rvalue && !argc )
	      {
	      responses[i] = XtMalloc( 1 );
	      strcpy( responses[i], "" );
	      num_responses++;
	      }
	    else if (rvalue)
	      {
	      argv = (char **) XtMalloc( argc*sizeof(char *) );
	      for( j=0; j<argc; j++ )
		 {
	 	 if( !j )
		   {
		   argv[0] = XtMalloc( strlen( v[0] ) + 1 );
		   strcpy( argv[0], v[0] );
		   }
		 else
		   argv[j] = QuoteString( v[j] );
		 }
	      XFreeStringList( v );
	      if( responses[i] )
	        XtFree( responses[i] );
	      else
		num_responses++;
	
	      strcpy( work, "" );
	      /* compare the host the application is running on
	         with the host we're running on. If different, make
		 this a remote command-line */
	      gethostname( hostbuf, sizeof( hostbuf ) );
	      rvalue = XGetWindowProperty( event->xproperty.display,
					   event->xproperty.window,
					   XA_WM_CLIENT_MACHINE,
					   0, 1000,
					   False, AnyPropertyType,
					   &actualtype, &actualformat,
					   &nitems, &bytesafter,
					   &propreturn );

	      if( rvalue == Success && actualtype == XA_STRING && nitems )
		{
		he = gethostbyname( propreturn );
		h1 = XtMalloc( strlen( he->h_name ) + 1 );
		strcpy( h1, he->h_name );
		he = gethostbyname( hostbuf );
		h2 = XtMalloc( strlen( he->h_name ) + 1 );
		strcpy( h2, he->h_name );
		if( strcmp( h1, h2 ) )
		  {
		  strcat( work, RSH_COMMAND );
		  strcat( work, " " );
		  strcat( work, propreturn );
		  strcat( work, " " );
		  do_quotes = True;
		  doing_rsh = True;
		  }
		XtFree( h1 );
		XtFree( h2 );
	        }
	
		ModifyArguments( DISPLAY, &argc, &argv, !doing_rsh,
				 (states[i] == IconicState) );
		for( j=0; j<argc; j++ )
		   {
		   if( do_quotes )
		     {
		     if( *(argv[j]) != '\0' )
		       {
		       if( doing_rsh )
			 {
			 char *tmp;

			 tmp = XtMalloc( strlen( argv[j] ) + 3 );
			 strcpy( tmp, "'" );
			 strcat( tmp, argv[j] );
			 strcpy( tmp, "'" );
			 XtFree( argv[j] );
			 argv[j] = QuoteQuotedString( tmp );
			 XtFree( tmp );
			 strcat( work, argv[j] );
			 }
		       else
			 {
			 strcat( work, "'" );
			 strcat( work, argv[j] );
			 strcat( work, "'" );
			 }
		       }
		     else
			strcat( work, argv[j] );
		     }

		     do_quotes = True;
		     XtFree( argv[j] );
		     if( j != argc-1 )
		       strcat( work, " " );
		   } 

		   responses[i] = XtMalloc( strlen(work) +1 );
		   strcpy( responses[i], work );
#ifdef DEBUG
		   printf( "WatchForNewWMCommand: got a response \n" );
#endif
         }
      }

   if( num_responses == num_windows )
     {
     XtRemoveTimeOut( *id );
     WriteCommands( DISPLAY );
     exit(0);
     }
  }
}
/*
 *  Either we timed out, or all expected responses have been received.
 *  Write out the resposes to the user-selected file.
 */
void
WriteCommands( DISPLAY )
	Display *DISPLAY;
{
  int i;
  FILE *fd;

#ifdef DEBUG
    printf("Writing %d of %d commands to file.\n",
           num_responses, num_windows);
#endif /*DEBUG*/

  if (cl_info.query) {
    char *name;

    printf("\n");
    for (i=0; i<num_windows; i++) {
      XFetchName(DISPLAY, windows[i], &name);
      if (responses[i]) {
        printf("%s (0x%x): [%s].\n", name, windows[i], responses[i]);
      } else {
        printf("%s (0x%x): No response to WM_SAVE_YOURSELF.\n",
               name, windows[i]);
      }
    }
  } else if (cl_info.filename && strlen(cl_info.filename)) {
    fd = fopen(cl_info.filename, "w");
    if (!fd) {
      fprintf(stderr, "Couldn't open %s to write commands.\n", cl_info.filename);
      exit(1);
    } else {
      for (i=0; i<num_windows; i++) {
        if (responses[i] && strlen(responses[i])) {
          fprintf(fd, "%s &\n", responses[i]);
        }
      }
      fclose(fd);
    }
  }
}

/*
 *  Handle any Single quotes in the string.
 */

char *
QuoteString(s)
char *s;
{
  int i, len, sum;
  char *result;

  len = strlen(s);
  sum = 0;
  for (i=0; i<len; i++) {
    if (s[i] == '\'') {
      sum++;
    }
  }
  result = XtMalloc(len+3*sum+1);
  sum = 0;
  for (i=0; i<len; i++) {
    if (s[i] == '\'') {
/* It's a single quote -- put "'\''" in place of it to undo the quoting,
   insert a single quote, and start quoting again. */
      result[sum++] = '\'';
      result[sum++] = '\\';
      result[sum++] = '\'';
      result[sum++] = '\'';
    } else {
      result[sum++] = s[i];
    }
  }
  result[sum] = '\0';
  return result;
}

/*
 *  Quote quoted strings which are going to go through two shells when rsh'd.
 */

char *
QuoteQuotedString(s)
char *s;
{
  int i, len, tmp;
  Boolean in_quote;
  char *result, work[1000];

  strcpy(work, "");
  len = strlen(s);
  if (len) {
    in_quote = False;
    for (i=0; i<len; i++) {
      if (s[i] == '\'' && !in_quote) {
        strcat(work, "\\''");
        in_quote = True;
      } else if (s[i] == '\'' && in_quote) {
        strcat(work, "'\\'");
        in_quote = False;
      } else if (s[i] == '\\' && !in_quote) {
/* We assume that the only special character that outside of single quotes
   is '\'.  For these, change them to "\\\". */
        strcat(work, "\\\\\\");
        tmp = strlen(work);
        i++;
        work[tmp++] = s[i];
        work[tmp] = '\0';
      } else {
        tmp = strlen(work);
        work[tmp++] = s[i];
        work[tmp] = '\0';
      }
    }
  }
  result = XtMalloc(strlen(work)+1);
  strcpy(result, work);
  return result;
}

/*
 *  Modify certain arguments in the returned command.  For instance,
 *  set -display to the current display.  Also, if iconic, add a
 *  -iconic.
 */

void
ModifyArguments(DISPLAY, argc, argv, locally, iconic)
Display *DISPLAY;
int *argc;
char ***argv;
Boolean locally, iconic;
{
  int i;
  static char *display_string = NULL, display_name[1000];
  char *tmp;
  Boolean added_display = False, added_iconic = False;

  if (cl_info.add_display) {
    display_string = XDisplayString(DISPLAY);
    if (*display_string == ':' && !locally) {
      gethostname(display_name, sizeof(display_name));
      strcat(display_name, display_string);
    } else {
/* It's okay to set the display name to ":0.0" if its local.  In fact, this
   is better because "localhost" is always in the security list. */
      strcpy(display_name, display_string);
    }
    for (i=0; i<*argc; i++) {
      if (!strcmp((*argv)[i], "-display") && i<*argc-1) {
        tmp = (*argv)[++i];
        (*argv)[i] = XtMalloc(strlen(display_name)+1);
        strcpy((*argv)[i], display_name);
        XtFree(tmp);
        added_display = True;
      }
    }

    if (!added_display) {
      *argc += 2;
      *argv = (char **) XtRealloc(*argv, *argc*sizeof(char *));
      for (i=(*argc)-3; i>0; i--) {
        (*argv)[i+2] = (*argv)[i];
      }
      (*argv)[1] = XtMalloc(sizeof("-display")+1);
      strcpy((*argv)[1], "-display");
      (*argv)[2] = XtMalloc(sizeof(display_name)+1);
      strcpy((*argv)[2], display_name);
    }
  }

  if (cl_info.add_iconic) {
    for (i=0; i<*argc && !added_iconic; i++) {
      if (!strcmp((*argv)[i], "-iconic")) {
        if (iconic) {
          added_iconic = True;
        } else {
          tmp = (*argv)[i];
          (*argv)[i] = XtMalloc(1);
          strcpy((*argv)[i], "");
          XtFree(tmp);
        }
      } else if (!strcmp((*argv)[i], "-xrm") &&
                 (!strcmp((*argv)[i+1], "\"*iconic: true\"") ||
                  !strcmp((*argv)[i+1], "\"*iconic: True\"") ||
                  !strcmp((*argv)[i+1], "\"*iconic: TRUE\"") ||
                  !strcmp((*argv)[i+1], "\"*Iconic: true\"") ||
                  !strcmp((*argv)[i+1], "\"*Iconic: True\"") ||
                  !strcmp((*argv)[i+1], "\"*Iconic: TRUE\""))) {
        if (iconic) {
          added_iconic = True;
        } else {
          tmp = (*argv)[i];
          (*argv)[i] = XtMalloc(1);
          strcpy((*argv)[i], "");
          XtFree(tmp);
          tmp = (*argv)[++i];
          (*argv)[i] = XtMalloc(1);
          strcpy((*argv)[i], "");
          XtFree(tmp);
        }
      }
    }

    if (!added_iconic && iconic) {
      *argc += 1;
      *argv = (char **) XtRealloc(*argv, *argc*sizeof(char *));
      for (i=(*argc)-2; i>0; i--) {
        (*argv)[i+1] = (*argv)[i];
      }
      (*argv)[1] = XtMalloc(sizeof("-iconic")+1);
      strcpy((*argv)[1], "-iconic");
    }
  }

}

