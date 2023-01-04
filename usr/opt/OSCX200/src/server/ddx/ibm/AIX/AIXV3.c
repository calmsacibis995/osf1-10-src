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
/*
 * $XConsortium: AIXV3.c,v 1.4 91/07/16 12:55:43 jap Exp $
 *
 * Copyright IBM Corporation 1987,1988,1989,1990,1991
 *
 * All Rights Reserved
 *
 * License to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted,
 * provided that the above copyright notice appear in all copies and that
 * both that copyright notice and this permission notice appear in
 * supporting documentation, and that the name of IBM not be
 * used in advertising or publicity pertaining to distribution of the
 * software without specific, written prior permission.
 *
 * IBM DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
 * ALL IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS, AND 
 * NONINFRINGEMENT OF THIRD PARTY RIGHTS, IN NO EVENT SHALL
 * IBM BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
 * ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
 * WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
 * ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
 * SOFTWARE.
 *
*/

#ifndef AIXV3
#include <sys/select.h>
#endif
#include <ctype.h>
#include "ibmTrace.h"
#include "hftUtils.h"

#ifdef AIXV3
#include <sys/time.h>
#include "X_msg.h"
extern void aixErrMsg();
#endif

/*int     AIXDefaultDisplay= 0;*/
int     AIXDefaultDisplay= HFT_SKYWAY_ID;
#ifdef SOFTWARE_CURSOR
int     AIXCurrentX= 640;
int     AIXCurrentY= 512;
#else
int     AIXCurrentX= 0;
int     AIXCurrentY= 0;
#endif
int     AIXMouseChordDelay=     20;

extern  int     hftPending;
static  int     handleInput;

/***====================================================================***/

	/*
	 * BlockHandler is called just before the server goes into select
	 * forever (effectively),  WakeupHandler is called when select
	 * returns (times out or is interrupted).
	 *
	 * Normally, the hft interrupt knocks us out of select, but not
	 * back to Dispatch() -- only to WaitForSomething().  WaitForSomething()
	 * does *not* check device input -- it loops back around to the block
	 * handler and select.  It doesn't return to dispatch until a client
	 * does something.  The WakeupHandler only needs to dispatch input
	 * events when we're waiting for clients in WaitForSomething().
	 *
	 * We need to set a timeout if input is pending, because
	 * hftDispatchEvents wants to be called periodically if it has
	 * a pending timeout, and it won't be generating any interrupts to
	 * blow the select away.
	 *
	 * waittime is used to get a reasonable amount of time for timeouts
	 * in the hft queue.  The value of the timeout below was derived
	 * experimentally -- I played with the length of the timeout
	 * until the mouse felt "right."
	 */

int SelectWaitTime = 10000; /* usec */

void
AIXBlockHandler(scrNdx,blockData,ppTimeout,pReadmask)
	int     scrNdx;
	char    *blockData;
	struct timeval  **ppTimeout;
	unsigned        *pReadmask;
{
static  struct timeval waittime = { 0, 0 };  /* {sec, usec} */

    TRACE(("AIXBlockHandler()\n"));
    waittime.tv_usec = SelectWaitTime;
    if (hftPending) {
	*ppTimeout= &waittime;
	handleInput= 1;
    }
    else {
	hftInterruptAlways();
	handleInput= 0;
    }
    return;
}

/***====================================================================***/

void
AIXWakeupHandler(scrNdx,blockData,pTimeout,pReadmask)
	int     scrNdx;
	char    *blockData;
	struct timeval  *pTimeout;
	unsigned        *pReadmask;
{
    TRACE(("AIXWakeupHandler()\n"));
    if (handleInput) {
	ProcessInputEvents();
    }
    hftInterruptOnFirst();
    return;
}

/***====================================================================***/

#ifndef NO_SECRETS
static  int     aixShowSecretArgs= 0;
#endif

#ifdef AIXV3
void
AIXUseMsg()
{
     aixErrMsg(M_MSG_4);
}
#else
void
AIXUseMsg()
{
    ErrorF("The \"X\" command accepts the following flags:\n");
    ErrorF("   -a  <accel>     specify the mouse acceleration\n");
    ErrorF("   -bp <color>     specify a BlackPixel color\n");
    ErrorF("   -c  <volume>    specify the key click volume\n");
    ErrorF("   -D  <file>      specify the color definition data base file\n");
    ErrorF("   -f  <volume>    specify the beep volume\n");
    ErrorF("   -fc <font>      specify the font for cursor glyphs and masks\n");
    ErrorF("   -fn <font>      specify the default text font\n");
    ErrorF("   -fp <path>      specify the search path for fonts\n");
    ErrorF("   -n :<num>       specify the connection number\n");
    ErrorF("   -p  <interval>  specify the screen saver interval\n");
    ErrorF("   -r              disables auto repeat\n");
    ErrorF("   -s <delay>      specify the screen saver delay\n");
    ErrorF("   -t <threshold>  specify the mouse threshold\n");
    ErrorF("   -to <time>      specify the time between connection checks\n");
    ErrorF("   -T              disable the Ctrl-Alt-Backspace key sequence\n");
    ErrorF("   -v              activate screen saver with background color\n");
    ErrorF("   -wp <color>     specify a WhitePixel color\n");
#ifndef NO_SECRETS
    if (aixShowSecretArgs) {
    ErrorF("For the moment, X also accepts some undocumented, unsupported\n");
    ErrorF("flags.  These flags are *not* guaranteed to exist in future\n");
    ErrorF("releases or to work all that well in this release.\n");
    ErrorF("These unsupported flags are:\n");
    ErrorF("   -bs             enable backing store (default)\n");
    ErrorF("   -mdelay <count> set delay for middle button chord (default 10)\n");
    ErrorF("   -nobs           disable backing store\n");
    ErrorF("   -nohdwr         use generic functions where applicable\n");
    ErrorF("   -pckeys         swap CAPS LOCK and CTRL (for touch typists)\n");
#ifdef IBM_SPECIAL_MALLOC
    ErrorF("   -plumber <file> dump malloc arena to named file\n");
#endif
    ErrorF("   -quiet          do not print information messages (default)\n");
    ErrorF("   -refresh        refresh clients to restore HFT\n");
    ErrorF("   -repaint        repaint to restore HFT (default)\n");
    ErrorF("   -rtkeys         use CAPS LOCK and CTRL as labelled (default)\n");
#ifdef TRACE_X
    ErrorF("   -trace          trace execution of IBM specific functions\n");
#endif /* TRACE_X */
    ErrorF("   -verbose        print information messages\n");
    ErrorF("   -wrap           wrap mouse in both dimensions\n");
    ErrorF("   -wrapx          wrap mouse in X only\n");
    ErrorF("   -wrapy          wrap mouse in Y only\n");
    }
#endif
    ErrorF("See the X User's Guide or X server manual page for more information\n");
}
#endif
/***====================================================================***/

extern  int     ibmRefreshOnActivate;

int
AIXProcessArgument(argc,argv,i)
int      argc;
char    *argv[];
int     i;
{
extern  char    *rgbPath;
extern  char    *display;

    TRACE(("AIXProcessArgument(%d,0x%x,%d)\n",argc,argv,i));

    if      (( strcmp( argv[i], "-c" ) == 0) && ((i+1) < argc))
    {
	   if ((argv[i+1][0] >= '0') && (argv[i+1][0] <= '9'))
	        argv[i] = "c";
	   return(0);
    }
    else if ( strcmp( argv[i], "-D" ) == 0)
    {
	    if(++i < argc)
	        rgbPath = argv[i];
	    else
	        return(0);
	    return(2);
    }
    else if (strcmp(argv[i], "-mdelay") == 0 ) {
	if ((++i<argc)&&(isdigit(argv[i][0]))) {
	    AIXMouseChordDelay= atoi(argv[i]);
	    return(2);
	}
    }
    else if ( strcmp( argv[i], "-n") == 0) {
	char *dpy= argv[++i];

	if (i<argc) {
	    if (dpy[0]==':') {
	        display= &dpy[1];
	    }
	    else {
	        ErrorF("display specification must begin with ':'\n");
	        UseMsg();
	        exit(1);
	    }
	}
	else {
	    ErrorF("must specify display number after -n\n");
	    UseMsg();
	    exit(1);
	}
	return(2);
    }
    else if ( strcmp( argv[i], "-refresh" ) == 0 ) {
	ibmRefreshOnActivate= 1;
	return(1);
    }
    else if ( strcmp( argv[i], "-repaint" ) == 0 ) {
	ibmRefreshOnActivate= 0;
	return(1);
    }
#ifndef NO_SECRETS
    else if (strcmp(argv[i],"-secrethelp")==0) {
	aixShowSecretArgs= 1;
	AIXUseMsg();
	exit(1);
    }
    else if (strcmp(argv[i],"-dwa")==0) {  /* dwa test version */
	/* playground */
	ibmRefreshOnActivate= 1;
	ErrorF("Refresh ON \n");
	return(1);
    }
#endif
    return(0);
}

/***====================================================================***/

int     (*ibmAdditionalDfltScreen)();

void
osGetDefaultScreens()
{
unsigned        Device, hftQueryCurrentDevice();
char            *devflag, **fakeArgv= &devflag;
char            *devstring = "unknown device";

    TRACE(("osGetDefaultScreens()\n"));

    Device = hftQueryCurrentDevice();

    if( ! Device )
	return;

    if( ibmFindProbeAndAdd(Device, -1, -1) )
	return;

    switch (Device & HFT_DEVID_MASK) {

	case HFT_SKYWAY_ID:
	        devflag= "-skyway";
	        devstring = "skyway";
	        break;
	case HFT_SKYMONO_ID:
	        devflag= "-skyMono";
	        devstring = "skymono";
	        break;
	case HFT_SABINE_ID:
	        devflag= "-sabine";
	        devstring = "sabine";
	        break;
	case HFT_GEMINI_ID:
	        devflag= "-gemini";
	        devstring = "gemini";
	        break;
	case HFT_PEDER_ID:
	        devflag= "-peder";
	        devstring = "peder";
	        break;
	default:
	        if (ibmAdditionalDfltScreen) {
#if MERGED_AIXNEW
	            if (mtxAdditionalDfltScreen(Device,&devstring,&devflag))
#else
	            if ((*ibmAdditionalDfltScreen)(Device,&devstring,&devflag))
#endif
	                break;
	        }
	        devstring= "unknown device";
	        devflag= "-gai";
	        break;
	case HFT_ILLEGAL_ID:
	        ErrorF("Cannot determine current device for X screen.\n");
	        ErrorF("Possible problems: remote execution, no active\n");
	        ErrorF("tty, or running in background on old kernel.\n");
	        ErrorF("Retry by specifying a screen option on the command\n");
	        ErrorF("Check out the documentation if all else fails\n\n");
	        Error("Goodbye!\n");    /* should *NEVER* return */
	        _exit();        /* NOTREACHED */
	        break;          /* NOTREACHED */
    }

    ddxProcessArgument(1,fakeArgv,0);

    return;
}
