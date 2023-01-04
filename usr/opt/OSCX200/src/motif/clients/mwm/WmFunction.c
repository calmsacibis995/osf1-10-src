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
 * (c) Copyright 1989, 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
*/ 
/* 
 * Motif Release 1.2.2
*/ 
#ifdef REV_INFO
#ifndef lint
static char rcsid[] = "$RCSfile: WmFunction.c,v $ $Revision: 1.1.4.5 $ $Date: 1993/12/03 21:03:47 $"
#endif
#endif
/*
 * (c) Copyright 1987, 1988, 1989, 1990 HEWLETT-PACKARD COMPANY */

/*
 * Included Files:
 */

#ifdef VMS
#include <descrip.h>
#include <ssdef.h>
#include <lib$routines.h>
#endif
#include <signal.h>
#include <stdio.h>
#include <X11/Xos.h>
#include "WmGlobal.h"
#include "WmICCC.h"
#ifdef DEC_MOTIF_EXTENSION
#ifdef HYPERHELP
#include "mwm_help.h"
#endif
#include <X11/ShellP.h>
#include <X11/Shell.h>
#include <Mrm/MrmPublic.h>
#endif


/*
 * include extern functions
 */
#include "WmFunction.h"
#include "WmCEvent.h"
#include "WmCDInfo.h"
#include "WmColormap.h"
#include "WmError.h"
#include "WmEvent.h"
#include "WmFeedback.h"
#include "WmIPlace.h"
#include "WmIconBox.h"
#include "WmKeyFocus.h"
#include "WmMenu.h"
#include "WmProperty.h"
#include "WmProtocol.h"
#include "WmResParse.h"
#include "WmWinConf.h"
#include "WmWinInfo.h"
#include "WmWinList.h"
#include "WmWinState.h"
#ifdef DEC_MOTIF_EXTENSION
#include "mwm_cust.h"
#include "mwm_util.h"
#endif

#include <Xm/XmosP.h>		/* for _XmOSPutenv */

extern char * getenv ();

#ifdef _NO_PROTO
static unsigned int GetEventInverseMask();
#else	/* _NO_PROTO */
static unsigned int GetEventInverseMask(XEvent *event);
#endif	/* _NO_PROTO */

/*
 * Global Variables:
 */

/*
 * The 'dirty' variables are used to keep track of the transient window
 * that has been lowered via "f.lower freeFamily".
 */
static ClientListEntry dirtyStackEntry = { NULL, NULL, NORMAL_STATE, NULL };
static ClientData *dirtyLeader = NULL;



/******************************<->*************************************
 *
 *  F_Beep (args, pCD, event)
 *
 *
 *  Description:
 *  -----------
 *  This is the window manager function handler for beeping.
 *
 *
 *  Inputs:
 *  ------
 *  args = function arguments (specified in .mwmrc file)
 *
 *  pCD = pointer to the client data for the client window to which the
 *        function is to be applied
 *
 *  event = X event that invoked the function (key, button, or menu/NULL)
 *
 *
 *  Outputs:
 *  -------
 *  RETURN = if True then further button binding/function processing can
 *           be done for the event that caused this function to be called.
 *
 ******************************<->***********************************/

#ifdef _NO_PROTO
Boolean F_Beep (args, pCD, event)
    String args;
    ClientData *pCD;
    XEvent *event;

#else /* _NO_PROTO */
Boolean F_Beep (String args, ClientData *pCD, XEvent *event)
#endif /* _NO_PROTO */
{

    /* !!! what is a good value for percent (the second arg) !!! */
    XBell (DISPLAY, 0);

    return (True);

} /* END OF FUNCTION F_Beep */



/*
 * Handle Special case where the dirty window is the top most
 * transient window.  When this is the case, raising the window
 * that was on top (the window just below the dirty window) will
 * fail because Mwm stack database is out of sync.  So the solution
 * is to restack the dirty transient relative to the second to the
 * top transient.  This function is used to support freeFamily stacking.
 */
ClientData * FindSecondToTopTransient (pcd)
ClientData *pcd;
{
    ClientData *pcdNext;
    static ClientData *second;

    pcdNext = pcd->transientChildren;
    while (pcdNext)
    {
	if (pcdNext->transientChildren)
	{
	    if (!pcdNext->transientChildren->transientChildren)
	    {
		second = pcdNext;
	    }
	    FindSecondToTopTransient (pcdNext);
	}
	pcdNext = pcdNext->transientSiblings;
	if (pcdNext && !pcdNext->transientSiblings)
	{
	    second = pcdNext;
	}
    }

    return (second);

} /* END OF FUNCTION */



/*************************************<->*************************************
 *
 *  RestoreTransients (pcd, stack_mode)
 *
 *
 *  Description:
 *  -----------
 *  A function to restore dirty transient windows to their original position
 *  before changing the stack again.  This function is used to support
 *  freeFamily stacking.
 *
 *************************************<->***********************************/
#ifdef _NO_PROTO
void RestoreTransients (pcd, stack_mode)
     ClientData *pcd;
     int stack_mode;
#else /* _NO_PROTO */
void RestoreTransients (ClientData *pcd, int stack_mode)
#endif /* _NO_PROTO */
{
    ClientData *pcdLeader, *pcd_temp;
    XWindowChanges changes;
    Window win;

    if (dirtyStackEntry.pCD)
    {
	pcdLeader = FindTransientTreeLeader (dirtyStackEntry.pCD);

	/*
	 * Handle Special case where the dirty window is the top most
	 * transient window.  When this is the case, raising the window
	 * that was on top (the window just below the dirty window) will
	 * fail because Mwm stack database is out of sync.  So the solution
	 * is to restack the dirty transient relative to the second to the
	 * top transient.
	 */

	if (dirtyStackEntry.pCD == FindTransientOnTop(pcdLeader))
	{
	    pcd_temp = FindSecondToTopTransient (pcdLeader);
	    win = pcd_temp->clientFrameWin;
	}
	else
	{
	    win = pcd->clientFrameWin;
	}
	changes.sibling = win;
	changes.stack_mode = stack_mode;
	XConfigureWindow (DISPLAY, dirtyStackEntry.pCD->clientFrameWin,
			  (CWSibling | CWStackMode), &changes);

	RestackTransients (dirtyStackEntry.pCD, False);
    }
}

#ifdef _NO_PROTO
Boolean ForceLowerWindow (pcd)
	ClientData *pcd;
#else /* _NO_PROTO */
Boolean ForceLowerWindow (ClientData *pcd)
#endif /* _NO_PROTO */
{
#if 0
    Window stackWindow;
    WmScreenData *pSD = (ACTIVE_WS)->pSD;
#endif
    XWindowChanges changes;
    Boolean restack = False;

#if 0
    if (pSD->lastClient->type == MINIMIZED_STATE)
    {
	stackWindow = ICON_FRAME_WIN(pSD->lastClient->pCD);
    }
    else
    {
	stackWindow = pSD->lastClient->pCD->clientFrameWin;
    }
#endif

    changes.stack_mode = Below;
    XConfigureWindow (DISPLAY, pcd->clientFrameWin, CWStackMode,
		      &changes);

    return (restack);
}



/*************************************<->*************************************
 *
 *  F_Lower (args, pCD, event)
 *
 *
 *  Description:
 *  -----------
 *  This is the window manager function handler for bottoming a client window
 *  or icon.
 *
 *
 *  Inputs:
 *  ------
 *  args = function arguments (specified in .mwmrc file)
 *
 *  pCD = pointer to the client data for the client window to which the
 *        function is to be applied
 *
 *  event = X event that invoked the function (key, button, or menu/NULL)
 *
 *
 *  Outputs:
 *  -------
 *  RETURN = if True then further button binding/function processing can
 *           be done for the event that caused this function to be called.
 *
 *************************************<->***********************************/

#ifdef _NO_PROTO
Boolean F_Lower (args, pCD, event)
    String args;
    ClientData *pCD;
    XEvent *event;

#else /* _NO_PROTO */
Boolean F_Lower (String args, ClientData *pCD, XEvent *event)
#endif /* _NO_PROTO */
{
    ClientListEntry *pEntry;
    ClientListEntry *pNextEntry;
    ClientListEntry *pStackEntry;
    String string = args;
    int flags = STACK_NORMAL;

    if (string)
    {
	/* process '-client' argument */
	if (string[0] == '-')
	{
	    string = &string[1];
	    string = (String) GetString ((unsigned char **) &string);

	    pStackEntry = NULL;
	    pNextEntry = ACTIVE_PSD->lastClient;
	    while (pNextEntry &&
		   (pEntry = FindClientNameMatch (pNextEntry, False,
							string,	F_GROUP_ALL)))
	    {
		pNextEntry = pEntry->prevSibling;
		Do_Lower (pEntry->pCD, pStackEntry, STACK_NORMAL);
		pStackEntry = pEntry;
	    }
	}
	/* process family stacking stuff */
	else if (*string)
	{
	    unsigned int  slen, len, index;

	    slen = strlen(args) - 2;		/* subtract '\n' and NULL */
	    for (index = 0; index < slen; string = &args[index+1])
	    {
		if ((string = (String) GetString ((unsigned char **) &string)) == NULL)
		   break;
		len = strlen(string);
		if (!strcmp(string,"within"))
		{
		    flags |= STACK_WITHIN_FAMILY;
		}
		else if (!strcmp(string,"freeFamily"))
		{
		    flags |= STACK_FREE_FAMILY;
		}
		index += len;
	    }
	    Do_Lower (pCD, (ClientListEntry *) NULL, flags);
	}
    }
    else if (pCD)
    {
	Do_Lower (pCD, (ClientListEntry *) NULL, STACK_NORMAL);
    }

    wmGD.passButtonsCheck = False;
    return (True);

} /* END OF FUNCTION F_Lower */



/*************************************<->*************************************
 *
 *  Do_Lower (pCD, pStackEntry)
 *
 *
 *  Description:
 *  -----------
 *  This is the window manager function handler for lowering the client window
 *  so that it does not obscure any other window above the stack entry
 *  window.
 *
 *
 *  Inputs:
 *  ------
 *  pCD = pointer to the client data of the window (or icon) to be lowered.
 * 
 *  pStackEntry = pointer to client list entry for window that is to be 
 *	below the lowered window (if NULL, window is lowered to the bottom
 *	of the	stack).
 *
 *************************************<->***********************************/

#ifdef _NO_PROTO
void Do_Lower (pCD, pStackEntry, flags)
    ClientData *pCD;
    ClientListEntry *pStackEntry;
    int flags;
#else /* _NO_PROTO */
void Do_Lower (ClientData *pCD, ClientListEntry *pStackEntry, int flags)
#endif /* _NO_PROTO */
{
    Boolean restackTransients;
    ClientData *pcdLeader;

#ifdef DEC_MOTIF_BUG_FIX
    WmWorkspaceData *pWS = pCD->pSD->pActiveWS;
#else
    WmWorkspaceData *pWS = ACTIVE_WS;
#endif


    pcdLeader = (pCD->transientLeader) ? FindTransientTreeLeader (pCD) : pCD;

    if ((pcdLeader->clientState == MINIMIZED_STATE) && !P_ICON_BOX(pcdLeader))
    {
	/*
	 * Only restack the icon if it is not currently lowered.
	 */

	if (pStackEntry)
	{
	    if (pStackEntry->prevSibling != &pcdLeader->iconEntry)
	    {
		StackWindow (pWS, &pcdLeader->iconEntry, True /*above*/,
		    pStackEntry);
		MoveEntryInList (pWS, &pcdLeader->iconEntry, True /*above*/,
		    pStackEntry);
	    }
	}
	else
	{
	    if (ACTIVE_PSD->lastClient != &pcdLeader->iconEntry)
	    {
		StackWindow (pWS, &pcdLeader->iconEntry, 
			     False /*on bottom*/, (ClientListEntry *) NULL);
		MoveEntryInList (pWS, &pcdLeader->iconEntry, 
			     False /*on bottom*/, (ClientListEntry *) NULL);
	    }
	}
    }
    else /* NORMAL_STATE, MAXIMIZED_STATE, adoption */
    {
	/*
	 * If this is a transient window then put it below its
	 * sibling transient windows.
	 */

	restackTransients = False;
	if (pCD->transientLeader)
	{

	    /*
	     * If freeFamily stacking, then put dirty transient window
	     * (if any) back in place before force lowering current window
	     * to the bottom of the global window stack.  Then return.
	     */

	    if (flags & STACK_FREE_FAMILY)
	    {
		if (dirtyStackEntry.pCD)
		{
		    RestoreTransients (pCD, Below);
		}

		dirtyStackEntry.pCD = pCD;

		if (dirtyStackEntry.pCD->transientLeader)
		  dirtyLeader = FindTransientTreeLeader (pCD);
		else
		  dirtyLeader = dirtyStackEntry.pCD;

		restackTransients = ForceLowerWindow (pCD);
		return;
	    }

	    /*
	     * Reach here only if NOT doing a f.lower freeFamily (see
	     * return; statement above).  Put current transient below
	     * its sibling transient windows.
	     */
	    restackTransients = PutTransientBelowSiblings (pCD);
	}

	/*
	 * If doing a regular f.lower and you have a dirty window, then
	 * clean up dirty transient window.
	 */

	if (dirtyStackEntry.pCD)
	{
	    /* 
	     * If lowering a window in the same family as the dirty
	     * transient window, then just restack before lowering.
	     * Else, restore the dirty transient in place before
	     * lowering the current window.  Clear dirtyStack.
	     */
	    if (dirtyLeader == pcdLeader)
	    {
		if (dirtyStackEntry.pCD == FindTransientOnTop(pCD))
		{
		    RestackTransients (dirtyStackEntry.pCD, True);
		}
		else
		{
		    RestackTransients (dirtyStackEntry.pCD, False);
		}	
	    }
	    else
	    {
		RestoreTransients (pCD, Above);
	    }
	    dirtyStackEntry.pCD = NULL;
	}

	/*
	 * Only restack the window or transient window tree if it is
	 * not currently lowered and the window is not a system
	 * modal window.
	 */

	if (pStackEntry)
	{
	    if ((pStackEntry->prevSibling != &pcdLeader->clientEntry) &&
		!(wmGD.systemModalActive &&
		  (pcdLeader == wmGD.systemModalClient)))
	    {
	        StackWindow (pWS, &pcdLeader->clientEntry, True /*above*/,
		    pStackEntry);
		MoveEntryInList (pWS, &pcdLeader->clientEntry, True /*above*/,
		    pStackEntry);
	    }
	    else if (restackTransients)
	    {
		RestackTransients (pCD, False);
	    }
	}
	else
	{
	    if ((pWS->pSD->lastClient != &pcdLeader->clientEntry) &&
		!(wmGD.systemModalActive &&
		  (pcdLeader == wmGD.systemModalClient)) &&
		!(flags & STACK_WITHIN_FAMILY))
	    {
	        StackWindow (pWS, &pcdLeader->clientEntry, False /*on bottom*/,
		    (ClientListEntry *) NULL);
		MoveEntryInList (pWS, &pcdLeader->clientEntry,
		    False /*on bottom*/, (ClientListEntry *) NULL);
	    }
	    else if (restackTransients)
	    {
		RestackTransients (pCD, False);
	    }
	}
    }

} /* END OF FUNCTION Do_Lower */



/*************************************<->*************************************
 *
 *  F_CircleDown (args, pCD, event)
 *
 *
 *  Description:
 *  -----------
 *  This is the window manager function handler for moving the client window
 *  on top of stack to the bottom.
 *
 *
 *  Inputs:
 *  ------
 *  args = function arguments (specified in .mwmrc file)
 *
 *  pCD = pointer to the client data for the client window to which the
 *        function is to be applied
 *
 *  event = X event that invoked the function (key, button, or menu/NULL)
 *
 *
 *  Outputs:
 *  -------
 *  RETURN = if True then further button binding/function processing can
 *           be done for the event that caused this function to be called.
 *
 *************************************<->***********************************/

#ifdef _NO_PROTO
Boolean F_Circle_Down (args, pCD, event)
    String args;
    ClientData *pCD;
    XEvent *event;

#else /* _NO_PROTO */
Boolean F_Circle_Down (String args, ClientData *pCD, XEvent *event)
#endif /* _NO_PROTO */
{
    unsigned long types;
    unsigned long windowType;
    ClientListEntry *pNextEntry;
    ClientData *pcdNext;


    /*
     * Go down through the client list looking for a window of an
     * appropriate type that is obscuring lower windows.
     */

    types = (unsigned long)args;
    pNextEntry = ACTIVE_PSD->clientList;
    while (pNextEntry)
    {
	/*
	 * Only check out the window if it is onscreen.
	 */

	pcdNext = pNextEntry->pCD;
	if (((pNextEntry->type == NORMAL_STATE) &&
	     (pcdNext->clientState != MINIMIZED_STATE)) ||
	    ((pNextEntry->type == MINIMIZED_STATE) &&
	     (pcdNext->clientState == MINIMIZED_STATE)))
	{
	    if (pcdNext->clientState == MINIMIZED_STATE)
	    {
		windowType = F_GROUP_ICON;
	    }
	    else
	    {
		windowType = F_GROUP_WINDOW;
		if (pcdNext->transientLeader || pcdNext->transientChildren)
		{
		    windowType |= F_GROUP_TRANSIENT;
		}
	    }
	    if (types & windowType)
	    {
		if (CheckIfClientObscuringAny (pcdNext))
		{
		    /*
		     * This window (or window tree) is obscuring another window
		     * on the screen.  Lower the window.
		     */

		    F_Lower (NULL, pcdNext, (XEvent *) NULL);
		    break;
		}
	    }
	}
	pNextEntry = pNextEntry->nextSibling;
    }

    return (True);

} /* END OF FUNCTION F_Circle_Down */



/*************************************<->*************************************
 *
 *  F_Circle_Up (args, pCD, event)
 *
 *
 *  Description:
 *  -----------
 *  This is the window manager function handler for moving the client window
 *  on the bottom of the stack to the top.
 *
 *
 *  Inputs:
 *  ------
 *  args = function arguments (specified in .mwmrc file)
 *
 *  pCD = pointer to the client data for the client window to which the
 *        function is to be applied
 *
 *  event = X event that invoked the function (key, button, or menu/NULL)
 *
 *
 *  Outputs:
 *  -------
 *  RETURN = if True then further button binding/function processing can
 *           be done for the event that caused this function to be called.
 *
 *************************************<->***********************************/

#ifdef _NO_PROTO
Boolean F_Circle_Up (args, pCD, event)
    String args;
    ClientData *pCD;
    XEvent *event;

#else /* _NO_PROTO */
Boolean F_Circle_Up (String args, ClientData *pCD, XEvent *event)
#endif /* _NO_PROTO */
{
    unsigned long types;
    unsigned long windowType;
    ClientListEntry *pNextEntry;
    ClientData *pcdNext;


    /*
     * Go up through the client list looking for a window of an
     * appropriate type that is obscured by higher windows.
     */

    types = (unsigned long)args;
    pNextEntry = ACTIVE_PSD->lastClient;
    while (pNextEntry)
    {
	/*
	 * Only check out the window if it is onscreen.
	 */

	pcdNext = pNextEntry->pCD;
	if (((pNextEntry->type == NORMAL_STATE) &&
	     (pcdNext->clientState != MINIMIZED_STATE)) ||
	    ((pNextEntry->type == MINIMIZED_STATE) &&
	     (pcdNext->clientState == MINIMIZED_STATE)))
	{
	    if (pcdNext->clientState == MINIMIZED_STATE)
	    {
		windowType = F_GROUP_ICON;
	    }
	    else
	    {
		windowType = F_GROUP_WINDOW;
		if (pcdNext->transientLeader || pcdNext->transientChildren)
		{
		    windowType |= F_GROUP_TRANSIENT;
		}
	    }
	    if (types & windowType)
	    {
		if (CheckIfClientObscuredByAny (pcdNext))
		{
		    /*
		     * This window (or window tree) is obscured by another
		     * window on the screen.  Raise the window.
		     */

		    F_Raise (NULL, pcdNext, (XEvent *) NULL);
		    break;
		}
	    }
	}
	pNextEntry = pNextEntry->prevSibling;
    }

    return (True);


} /* END OF FUNCTION F_Circle_Up */



/*************************************<->*************************************
 *
 *  F_Focus_Color (args, pCD, event)
 *
 *
 *  Description:
 *  -----------
 *  This is the window manager function handler for setting the colormap
 *  focus to a client window or reinstalling the default colormap.
 *
 *************************************<->***********************************/

#ifdef _NO_PROTO
Boolean F_Focus_Color (args, pCD, event)
    String args;
    ClientData *pCD;
    XEvent *event;

#else /* _NO_PROTO */
Boolean F_Focus_Color (String args, ClientData *pCD, XEvent *event)
#endif /* _NO_PROTO */
{

    if (wmGD.colormapFocusPolicy == CMAP_FOCUS_EXPLICIT)
    {
        if (pCD)
        {
	    /*
	     * The window selected for the colormap focus is a top-level client
	     * window.  If there are subwindow colormaps then determine if the
	     * selection was in one of the subwindows.
	     */

	    if (pCD->clientState == MINIMIZED_STATE)
	    {
		/* !!! colormap for client supplied icon window !!! */
		pCD = NULL;
	    }
        }

        SetColormapFocus (ACTIVE_PSD, pCD);
    }

    return (True);

} /* END OF FUNCTION F_Focus_Color */



/*************************************<->*************************************
 *
 *  F_Exec (args, pCD, event)
 *
 *
 *  Description:
 *  -----------
 *  This is the window manager function handler for executing a command
 *  (with /bin/sh).
 *
 *************************************<->***********************************/

#ifndef VMS
#ifdef _NO_PROTO
Boolean F_Exec (args, pCD, event)
    String args;
    ClientData *pCD;
    XEvent *event;

#else /* _NO_PROTO */
Boolean F_Exec (String args, ClientData *pCD, XEvent *event)
#endif /* _NO_PROTO */
{
    int   status;
    int   pid;
    int   w;
    void (*intStat) ();
    void (*quitStat) ();
    char *shell;
    char *shellname;


    /* make sure the f.exec command runs on the right display. */
    if (wmGD.pActiveSD->displayString)
      {
	_XmOSPutenv(wmGD.pActiveSD->displayString);
      }
    

    /*
     * Fork a process to exec a shell to run the specified command:
     */

#ifdef PORT_NOVFORK
    if ((pid = fork ()) == 0)
#else
    if ((pid = vfork ()) == 0)
#endif
    {

    /*
    * We needed to add yet another level of conditional compilation 
    * to replace the setpgrp code with setsid.  This is to fix several
    * problems reported (CLD CFS.4899, SPR 14733 from OSF_QAR, and
    * others) regarding restarting mwm and then hanging when calling f.exec.
    *
    *  		-jhs 08-Nov-93
    */

#ifdef  DEC_MOTIF_BUG_FIX
        setsid();
#else
#ifndef NO_SETPGRP
	int tpid;

	tpid = getpid();
#ifdef SYSV
	setpgrp();
#else
	setpgrp(tpid, tpid);
#endif  /* SYSV		     */
#endif  /* NO_SETPGRP	     */
#endif  /* DEC_MOTIF_BUG_FIX */

	/*
	 * Exec the command using $MWMSHELL if set or 
	 * $SHELL if set and $MWMSHELL not set or sh.
	 */

        if (((shell = getenv ("MWMSHELL")) != NULL) ||
	    ((shell = getenv ("SHELL")) != NULL))

	{
	    shellname = rindex(shell, '/');
	    if (shellname == NULL)
	    {
		shellname = shell;
	    }
	    else
	    {
		shellname++;
	    }
	    execl (shell, shellname, "-c", args, 0);
	}

	/*
	 * There is no SHELL environment variable or the first execl failed.
	 * Try /bin/sh .
	 */
	execl ("/bin/sh", "sh", "-c", args, 0);


	/*
	 * Error - command could not be exec'ed.
	 */

	_exit (127);
    }

    else if (pid == -1)
      return(True);

    /*
     * Have the window manager wait for the shell to complete.
     */

    intStat = (void (*)())signal (SIGINT, SIG_IGN);
    quitStat = (void (*)())signal (SIGQUIT, SIG_IGN);

    while ((w = wait (&status)) != pid && (w != -1));

    if (w == -1)
    {
	status = -1;
    }

    signal (SIGINT, intStat);
    signal (SIGQUIT, quitStat);

    return (True);


} /* END OF FUNCTION F_Exec */
#else                         

/********************************/

        /* VMS */

#include <descrip.h>

#define CLI$M_NOWAIT 1

Boolean F_Exec( args, pCD, event )

String args;
ClientData *pCD;
XEvent *event;

{
int status;
int flags = CLI$M_NOWAIT;
char error_text[ 128 ];   
struct dsc$descriptor command;

/********************************/

    command.dsc$b_class = DSC$K_CLASS_S;
    command.dsc$b_dtype = DSC$K_DTYPE_T;

    /* make sure the f.exec command runs on the right display. */
    if (( wmGD.numScreens > 0 ) && (wmGD.pActiveSD->displayString))
      {
        command.dsc$w_length = strlen( wmGD.pActiveSD->displayString );
        command.dsc$a_pointer = wmGD.pActiveSD->displayString;
        /* Spawn the command */
        status = lib$spawn( &command, 0, 0, &flags );

        /* Is there an error ? */
        if ( !(status & 1) )
          /* Yes, report the error as a warning.
             The text should be internationalized and put in a message file. */
          {
    	    sprintf( error_text, "Warning: the Exec command could not set the correct screen: status %d", status );
            Warning( error_text );
          }    
      }

    /* Run the command */
    command.dsc$w_length = strlen( args );
    command.dsc$a_pointer = args;
    if (( args == NULL ) || ( strlen( args ) == 0 ))
      {
	sprintf( error_text, "The Exec command has no arguments" );
        Warning( error_text );
        return( FALSE );
      }
    /* Remove the "&" from the end, which was added for sending a
       process to the background in Unix. Possibly fix this later
       so that & is not added to the string for VMS. */
    if ( args[ strlen( args ) - 1 ] == '&' )
        args[ strlen( args ) - 1 ] = '\0';
    command.dsc$w_length = strlen( args );
    command.dsc$a_pointer = args;

    /* Spawn the command */
    status = lib$spawn( &command, 0, 0, &flags );

    /* Is there an error ? */
    if ( !(status & 1) )
      /* Yes, report the error as a warning.
         The text should be internationalized and put in a message file. */
      {
	sprintf( error_text, "The Exec command failed with status %d", status );
        Warning( error_text );
      }

    return( True );

} /* END OF FUNCTION F_Exec */

/*******************************************************************/
#endif /* VMS */

/*************************************<->*************************************
 *
 *  F_Quit_Mwm (args, pCD, event)
 *
 *
 *  Description:
 *  -----------
 *  This is the window manager function handler for terminating the window
 *  manager.
 *
 *************************************<->***********************************/

#ifdef _NO_PROTO
Boolean F_Quit_Mwm (args, pCD, event)
    String args;
    ClientData *pCD;
    XEvent *event;

#else /* _NO_PROTO */
Boolean F_Quit_Mwm (String args, ClientData *pCD, XEvent *event)
#endif /* _NO_PROTO */
{
    if (wmGD.showFeedback & WM_SHOW_FB_QUIT)
    {
	ConfirmAction (ACTIVE_PSD, QUIT_MWM_ACTION);
    }
    else
    {
	Do_Quit_Mwm(False);
    }
    
    return (False);

} /* END OF FUNCTION F_Quit_Mwm */



/*************************************<->*************************************
 *
 *  Do_Quit_Mwm (diedOnRestart)
 *
 *
 *  Description:
 *  -----------
 *  Callback to do the f.quit_mwm function.
 *
 *************************************<->***********************************/

#ifdef _NO_PROTO
void Do_Quit_Mwm (diedOnRestart)
    Boolean diedOnRestart;
#else /* _NO_PROTO */
void Do_Quit_Mwm (Boolean diedOnRestart)
#endif /* _NO_PROTO */
{
    int scr;
    ClientListEntry *pNextEntry;
#ifdef DEC_MOTIF_EXTENSION
#ifndef VMS
    Atom xa_WM_COMMAND = None;
#endif
#endif

    /*
     * Close the X connection to get all the X resources cleaned up.
     * !!! maybe windows should be reparented / rebordered  before closing? !!!
     * !!! clean up the _MOTIF_WM_INFO property on the root window !!!
     */

#ifdef DEC_MOTIF_EXTENSION
#ifdef HYPERHELP
    /* Exit any hyperhelp windows */
    mwm_help_exit();
#endif
#endif
    if (DISPLAY)
    {
        XSetInputFocus(DISPLAY, PointerRoot, RevertToPointerRoot, CurrentTime);
	for (scr = 0; scr < wmGD.numScreens; scr++)
	{
	    if (wmGD.Screens[scr].managed)
	    {
		pNextEntry = wmGD.Screens[scr].lastClient;
		while (pNextEntry)
		{
		    if (pNextEntry->type == NORMAL_STATE)
		    {
			if (!(pNextEntry->pCD->clientFlags & 
			      CLIENT_WM_CLIENTS))
			{
			    ReBorderClient (pNextEntry->pCD, diedOnRestart);
			}
		    }
		    pNextEntry = pNextEntry->prevSibling;
		}

		XDeleteProperty(DISPLAY, wmGD.Screens[scr].rootWindow,
				wmGD.xa_MWM_INFO);
	    }
	}
#ifdef DEC_MOTIF_EXTENSION
#ifndef VMS
        /* For save yourself */
        xa_WM_COMMAND = XInternAtom( DISPLAY, "WM_COMMAND", FALSE );
        XChangeProperty (DISPLAY, DefaultRootWindow(DISPLAY), xa_WM_COMMAND,
                         xa_WM_COMMAND, 32, PropModeReplace, 
                         (unsigned char *)NULL, 0 );
#endif /* not VMS */
#endif /* DEC_MOTIF_BUG_FIX */
        XSync (DISPLAY, False);
        XCloseDisplay (DISPLAY);
    }
    
    if(diedOnRestart)
    {
	exit (WM_ERROR_EXIT_VALUE);
    }
    else
    {
	exit (0);
    }

} /* END OF FUNCTION Do_Quit_Mwm */


/*************************************<->*************************************
 *
 *  ReBorderClient (pCD, reMapClient)
 *
 *
 *  Description:
 *  -----------
 *  Restores X border for client window and reparents the
 *  window back to the root.
 *
 *
 *  Inputs:
 *  -------
 *  pCD = pointer to the client data for the window to be re-bordered.
 *
 *************************************<->***********************************/

#ifdef _NO_PROTO
void ReBorderClient (pCD, reMapClient)
    ClientData *pCD;
    Boolean  reMapClient;
#else /* _NO_PROTO */
void ReBorderClient (ClientData *pCD, Boolean reMapClient)
#endif /* _NO_PROTO */
{
    int x, y;
    int xoff, yoff;
    XWindowChanges windowChanges;

    while (pCD)
    {
        if (pCD->iconWindow && (pCD->clientFlags & ICON_REPARENTED) &&
	    (!(reMapClient)))
        {
	    XUnmapWindow (DISPLAY, pCD->iconWindow);
	    XReparentWindow (DISPLAY, pCD->iconWindow, 
		ROOT_FOR_CLIENT(pCD), pCD->iconX, pCD->iconY);
        }

	if (!(reMapClient))
	{
	    if (pCD->maxConfig)
	    {
		x = pCD->maxX;
		y = pCD->maxY;
	    }
	    else
	    {
		if(wmGD.positionIsFrame)
		{
		    CalculateGravityOffset (pCD, &xoff, &yoff);
		    x = pCD->clientX - xoff;
		    y = pCD->clientY - yoff;
		}
		else
		{
		    x = pCD->clientX;
		    y = pCD->clientY;
		}
	    }
	    XUnmapWindow(DISPLAY, pCD->clientFrameWin);
	    XReparentWindow (DISPLAY, pCD->client, 
			     ROOT_FOR_CLIENT(pCD), x, y);
	}
	else
	{
	    XMapWindow(wmGD.display, pCD->client);
	}

	if (pCD->transientChildren)
	{
	    ReBorderClient (pCD->transientChildren, reMapClient);
	}

	if (!(reMapClient))
	{
	    /*
	     * restore X border
	     */
	    windowChanges.x = x;
	    windowChanges.y = y;
	    windowChanges.border_width = pCD->xBorderWidth;
	    XConfigureWindow (DISPLAY, pCD->client, 
			      CWBorderWidth | CWX | CWY, &windowChanges);
	}

	if (pCD->transientLeader)
	{
	    pCD = pCD->transientSiblings;
	}
	else
	{
	    pCD = NULL;
	}
    }

} /* END OF FUNCTION ReBorderClient */



/*************************************<->*************************************
 *
 *  F_Focus_Key (args, pCD, event)
 *
 *
 *  Description:
 *  -----------
 *  This is the window manager function handler for setting the keyboard
 *  focus to a particular client window.
 *
 *
 *  Inputs:
 *  ------
 *  args = (immediate value) focus flags
 *
 *  pCD = pointer to the client data
 *
 *  event = X event that invoked the function (key, button, or menu/NULL)
 *
 *************************************<->***********************************/

#ifdef _NO_PROTO
Boolean F_Focus_Key (args, pCD, event)
    String args;
    ClientData *pCD;
    XEvent *event;

#else /* _NO_PROTO */
Boolean F_Focus_Key (String args, ClientData *pCD, XEvent *event)
#endif /* _NO_PROTO */
{
    long focusFlags = (long)args;


    if (pCD && (wmGD.keyboardFocusPolicy == KEYBOARD_FOCUS_EXPLICIT))
    {
	Do_Focus_Key (pCD, GetFunctionTimestamp ((XButtonEvent *)event),
	    (focusFlags | ALWAYS_SET_FOCUS));
    }

    return (True);

} /* END OF FUNCTION F_Focus_Key */


/*************************************<->*************************************
 *
 *  FindSomeReasonableClient
 *
 *  Description:
 *  -----------
 *  Find a client, any client to set the focus to, return client or NULL.
 *  This code is ripped off from AutoResetKeyFocus(). 
 *  
 *************************************<->***********************************/

#ifdef _NO_PROTO
static Window FindSomeReasonableClient()
#else /* _NO_PROTO */
static Window FindSomeReasonableClient(void)
#endif /* _NO_PROTO */
{
   ClientData *pcdNoFocus=NULL;

    ClientListEntry *pNextEntry;
    ClientData *pCD;
    ClientData *pcdLastFocus = (ClientData *) NULL;
    ClientData *pcdFocus;
    Window focusWindow = (Window) NULL;

    /*
     * Scan through the list of clients to find a window to get the focus.
     */

    pNextEntry = ACTIVE_PSD->clientList;

    while (pNextEntry)
    {
	pCD = pNextEntry->pCD;
	if (!wmGD.systemModalActive ||
	    (wmGD.systemModalClient == pCD))
	{
	    if ((pNextEntry->type != MINIMIZED_STATE) &&
	        (pCD->clientState != MINIMIZED_STATE) &&
	        (pCD != pcdNoFocus))
	    {
	        if (pCD->transientChildren)
	        {
		    pcdFocus = FindLastTransientTreeFocus (pCD, pcdNoFocus);
	        }
	        else
	        {
		    pcdFocus = pCD;
	        }
	        if (pcdFocus &&
		    ((pcdLastFocus == NULL) ||
		     (pcdFocus->focusPriority > pcdLastFocus->focusPriority)))
	        {
		    pcdLastFocus = pcdFocus;
	        }
	    }
	}
	pNextEntry = pNextEntry->nextSibling;
    }

    /*
     * Set the focus window if one is found
     */

    if (pcdLastFocus)
      focusWindow = pcdLastFocus->client;

    /*
     * If a client window could not be found, then just put focus
     * on any icon.
     */

    if (focusWindow == (Window) NULL)
    {
	pNextEntry = ACTIVE_PSD->clientList;

	while (pNextEntry)
	{
	    pCD = pNextEntry->pCD;

	    if ((pNextEntry->type == MINIMIZED_STATE) ||
	        (pCD->clientState == MINIMIZED_STATE))
	    {
		focusWindow = ICON_FRAME_WIN(pCD);
		break;
	    }
	    pNextEntry = pNextEntry->nextSibling;
	}
    }

    return (focusWindow);

} /* END OF FUNCTION FindSomeReasonableClient */




/*************************************<->*************************************
 *
 *  Do_Focus_Key (pCD, focusTime, flags)
 *
 *
 *  Description:
 *  -----------
 *  This function is used to set the focus to a window.  The focus indication
 *  is not changed until the FocusIn event is received.
 *
 *
 *  Inputs:
 *  ------
 *  pCD = pointer to the client data
 *
 *  focusTime = focus change time
 *
 *  flags = wm focus change flags
 *
 *************************************<->***********************************/

#ifdef _NO_PROTO
void Do_Focus_Key (pCD, focusTime, flags)
    ClientData *pCD;
    Time focusTime;
    long flags;

#else /* _NO_PROTO */
void Do_Focus_Key (ClientData *pCD, Time focusTime, long flags)
#endif /* _NO_PROTO */
{
    ClientData *pcdFocus;
    Window focusWindow;


#ifdef DEC_MOTIF_BUG_FIX
    /* Clear the replay flag */
    wmGD.replayEnterEvent = False;
#endif
    pcdFocus = pCD;
    if (pCD)
    {
	if (pCD->clientState == MINIMIZED_STATE)
	{
	    focusWindow = ICON_FRAME_WIN(pCD);
	}
	else if (wmGD.keyboardFocusPolicy == KEYBOARD_FOCUS_EXPLICIT)
	{
	    /*
	     * Set the keyboard focus to the indicated client window.
	     * If the window has an application modal subordinate then
	     * set the input focus to that window if the focus isn't
	     * already owned by a subordinate.
	     */

	    if (IS_APP_MODALIZED(pCD))
	    {
		ClientData *pcdFocusLeader,*currFocusLeader;

		/*
		 * Handle case where a modal window exists when Mwm starts up.
		 * wmGD.keyboardFocus is NULL, give focus to the modal dialog.
		 */

	        if (wmGD.keyboardFocus)
		{
		    currFocusLeader = wmGD.keyboardFocus->transientLeader;
		}
		else
		{
		    currFocusLeader = (ClientData *) NULL;
		}

		/*
		 * Find focus leader for pCD
		 */

		pcdFocusLeader = pCD;
		while (pcdFocusLeader->transientLeader &&
		       (pcdFocusLeader != currFocusLeader))
		{
		    pcdFocusLeader = pcdFocusLeader->transientLeader;
		}

		if (pcdFocusLeader == currFocusLeader)
		{
		    pcdFocus = wmGD.keyboardFocus;
		    flags = 0;
		}
		else
		{
		    pcdFocus = FindTransientFocus (pcdFocusLeader);
		}
	    }

#ifdef DEC_MOTIF_BUG_FIX
    /* If the user clicks on another application window when
       there is a full application modal dialog box up, then
       do not change focus.  This avoids an ACCVIO. */
            if ( pcdFocus == NULL )
                return;
#endif /* DEC_MOTIF_BUG_FIX */
	    focusWindow = pcdFocus->client;
	}
	else
	{
	    /*
	     * If the focus policy is "pointer" don't set the focus to a
	     * window is it has an application modal subordinate.
	     */

	    if (IS_APP_MODALIZED(pCD))
	    {
		pcdFocus = NULL;
		focusWindow = ACTIVE_PSD->wmWorkspaceWin;
#ifdef DEC_MOTIF_BUG_FIX
                /* Replay this later when the modal window is removed. */
                wmGD.replayEnterEvent = True;
#endif
	    }
	    else
	    {
		focusWindow = pcdFocus->client;
	    }
	}
    }
    else
    {
	/*
	 * Set up the default (non client specific) keyboard input focus.
	 */

	if (wmGD.keyboardFocusPolicy == KEYBOARD_FOCUS_POINTER)
	{
	    focusWindow = PointerRoot;
	}
	else
	{
	    /*
	     * The WORKSPACE_IF_NULL flag is used to prevent client
	     * windows from flashing when deiconifying a client.
	     */

	    if (WORKSPACE_IF_NULL & flags)
	    {
	    	focusWindow = ACTIVE_PSD->wmWorkspaceWin;
	    }
	    else
	    {
		/* find some reasonable client so that focus is not lost */

		focusWindow = FindSomeReasonableClient();
		if (focusWindow == (Window)NULL)
		{
		    focusWindow = ACTIVE_PSD->wmWorkspaceWin;
		}
	    }
	}
    }


    if ((pcdFocus != wmGD.keyboardFocus) || (flags & ALWAYS_SET_FOCUS))
    {
        if (pcdFocus)
	{
	    /*
	     * Set the focus and/or send a take focus client message.  This
	     * is not done if a client area button press was done to set
	     * set the focus and the window is a globally active input
	     * style window (See ICCCM).
	     */

#ifdef DEC_MOTIF_EXTENSION
	    if (!((flags & CLIENT_AREA_FOCUS) &&
                  ((pcdFocus->protocolFlags & PROTOCOL_WM_TAKE_FOCUS) ||
                   (wmGD.useDECMode &&
                    pcdFocus->protocolFlags & PROTOCOL_DEC_WM_OLD_ICCCM)) &&
                  (!pcdFocus->inputFocusModel) &&
                  (pcdFocus == pCD) &&
                  (wmGD.keyboardFocusPolicy == KEYBOARD_FOCUS_EXPLICIT)))

#else
	    if (!((flags & CLIENT_AREA_FOCUS) &&
		  (pcdFocus->protocolFlags & PROTOCOL_WM_TAKE_FOCUS) &&
		  !pcdFocus->inputFocusModel &&
		  (pcdFocus == pCD) &&
		  (wmGD.keyboardFocusPolicy == KEYBOARD_FOCUS_EXPLICIT)))

#endif
	    {
	        if (pcdFocus->protocolFlags & PROTOCOL_WM_TAKE_FOCUS)
	        {
#ifdef DEC_MOTIF_BUG_FIX
	            SendClientMsg (focusWindow, 
			(long) wmGD.xa_WM_PROTOCOLS,
		        (long) wmGD.xa_WM_TAKE_FOCUS, 
			focusTime, NULL, 0);
#else
	            SendClientMsg (pcdFocus->client, 
			(long) wmGD.xa_WM_PROTOCOLS,
		        (long) wmGD.xa_WM_TAKE_FOCUS, 
			focusTime, NULL, 0);
#endif
	        }

#ifdef DEC_MOTIF_EXTENSION
        /* Send DEC_WM_TAKE_FOCUS messages for XUI clients which act
         * same as new-ICCCM WM_TAKE_FOCUS message.
         */
                if (!(pcdFocus->inputFocusModel &&
                      wmGD.useDECMode)){
                     /* changed this to ACTIVE_PS->actionNbr on 7/19/90 */
                     /* this might be wrong, but nobody seems to know   */
		     /* for sure.                                       */
 
                     /* extern int actionNbr; */
                     static Atom DEC_WM_TAKE_FOCUS = None;
                     XClientMessageEvent clientMsgEvent;

                     if (ACTIVE_PSD->actionNbr != RESTART_ACTION) {
                        if (DEC_WM_TAKE_FOCUS == None) {
                          DEC_WM_TAKE_FOCUS = XInternAtom(DISPLAY,
                                                "DEC_WM_TAKE_FOCUS",
                                                 FALSE);
                        }

                        clientMsgEvent.type = ClientMessage;
                        clientMsgEvent.window = pcdFocus->client;
                        clientMsgEvent.message_type = DEC_WM_TAKE_FOCUS;
                        clientMsgEvent.format = 32;
			
			/************************************************/
			/*						*/
			/* Send CurrentTime instead of focus time to	*/
			/* avoid problems with the time stamp.  If	*/
			/* the focus time is sent, the client may not	*/
			/* always get the input focus.  This is problem */
			/* sometimes shows up when double clicking on	*/
			/* the icon of an XUI-based application.	*/
			/*						*/
			/************************************************/
			clientMsgEvent.data.l[0] = CurrentTime;
			/*clientMsgEvent.data.l[0] = focusTime;*/
                        XSendEvent (DISPLAY, focusWindow, False, NULL,
                                (XEvent *)&clientMsgEvent);
                     }
                }
#endif
#ifdef OLD_TAKE_FOCUS
		if (wmGD.enforceKeyFocus || pcdFocus->inputFocusModel ||
		    (pcdFocus->clientState == MINIMIZED_STATE) ||
		    !(pcdFocus->protocolFlags & PROTOCOL_WM_TAKE_FOCUS))
	        {
	            XSetInputFocus (DISPLAY, focusWindow, RevertToParent,
		        CurrentTime);
	        }
#else
		/*
		 * Don't set the input focus if the window has input_field set
		 * to False or has expressed an interest in WM_TAKE_FOCUS
		 * (ie. 'No Input', 'Globally Active', or 'Locally Active'),
		 * and the user click in the client area.  If the user clicks
		 * on the titlebar or traverses to this window via f.next_key,
		 * set the focus so that the user can access the window menu
		 * and accelerators.
		 */
		if (wmGD.enforceKeyFocus ||
		    (flags & ALWAYS_SET_FOCUS) ||
		    !(flags & CLIENT_AREA_FOCUS) ||
		    pcdFocus->inputFocusModel ||
		    (pcdFocus->clientState == MINIMIZED_STATE) ||
		    ((!(pcdFocus->protocolFlags & PROTOCOL_WM_TAKE_FOCUS))
		     && pcdFocus->inputFocusModel))
	        {
	            XSetInputFocus (DISPLAY, focusWindow, RevertToParent,
		        CurrentTime);
	        }
		else
		{
		    /*
		     * We've decided that the window shouldn't get the focus,
		     * so don't change the focus.
		     */
		    pcdFocus = wmGD.nextKeyboardFocus;
		}
#endif
	    }
	}
	else
	{
	    /* !!! used timestamp based on SCREEN_SWITCH_FOCUS !!! */
	    XSetInputFocus (DISPLAY, focusWindow, RevertToPointerRoot,
		((flags & SCREEN_SWITCH_FOCUS) ? focusTime : CurrentTime));
	}

	wmGD.nextKeyboardFocus = pcdFocus;
    }


} /* END OF FUNCTION Do_Focus_Key */




/******************************<->*************************************
 *
 *  F_Next_Key (args, pCD, event)
 *
 *
 *  Description:
 *  -----------
 *  This is the window manager function handler for setting the keyboard
 *  input focus to the next window in the set of managed windows.
 *
 *
 *  Inputs:
 *  ------
 *  args = (immediate value) window type flags
 *
 *  pCD = pointer to the client data
 *
 *  event = X event that invoked the function (key, button, or menu/NULL)
 *
 *************************************<->***********************************/

#ifdef _NO_PROTO
Boolean F_Next_Key (args, pCD, event)
    String args;
    ClientData *pCD;
    XEvent *event;

#else /* _NO_PROTO */
Boolean F_Next_Key (String args, ClientData *pCD, XEvent *event)
#endif /* _NO_PROTO */
{
    if (wmGD.keyboardFocusPolicy == KEYBOARD_FOCUS_EXPLICIT)
    {
	FocusNextWindow ((unsigned long)args,
			 GetFunctionTimestamp ((XButtonEvent *)event));
    }

    return (True);

} /* END OF FUNCTION F_Next_Key */



/*************************************<->*************************************
 *
 *  F_Prev_Cmap (args, pCD, event)
 *
 *
 *  Description:
 *  -----------
 *  This is the window manager function handler installing the previous
 *  colormap in the list of client window colormaps.
 *
 *************************************<->***********************************/

#ifdef _NO_PROTO
Boolean F_Prev_Cmap (args, pCD, event)
    String args;
    ClientData *pCD;
    XEvent *event;

#else /* _NO_PROTO */
Boolean F_Prev_Cmap (String args, ClientData *pCD, XEvent *event)
#endif /* _NO_PROTO */
{
    if (pCD == NULL)
    {
	pCD = ACTIVE_PSD->colormapFocus;
    }

    if (pCD && (pCD->clientCmapCount > 0) &&
        ((pCD->clientState == NORMAL_STATE) ||
	 (pCD->clientState == MAXIMIZED_STATE)))
    {
	if (--(pCD->clientCmapIndex) < 0)
	{
	    pCD->clientCmapIndex = pCD->clientCmapCount - 1;
	}
	pCD->clientColormap = pCD->clientCmapList[pCD->clientCmapIndex];
	if (ACTIVE_PSD->colormapFocus == pCD)
	{
#ifndef OLD_COLORMAP /* colormap */
	    /*
	     * We just re-ordered the colormaps list,
	     * so we need to re-run the whole thing.
	     */
	    pCD->clientCmapFlagsInitialized = 0;
	    ProcessColormapList (ACTIVE_PSD, pCD);
#else /* OSF original */
	    WmInstallColormap (ACTIVE_PSD, pCD->clientColormap);
#endif
	}
    }

    return (True);

} /* END OF FUNCTION F_Prev_Cmap */



/*************************************<->*************************************
 *
 *  F_Prev_Key (args, pCD, event)
 *
 *
 *  Description:
 *  -----------
 *  This is the window manager function handler for setting the keyboard
 *  input focus to the previous window in the set of managed windows.
 *
 *
 *  Inputs:
 *  ------
 *  args = (immediate value) window type flags
 *
 *  pCD = pointer to the client data
 *
 *  event = X event that invoked the function (key, button, or menu/NULL)
 *
 *************************************<->***********************************/

#ifdef _NO_PROTO
Boolean F_Prev_Key (args, pCD, event)
    String args;
    ClientData *pCD;
    XEvent *event;

#else /* _NO_PROTO */
Boolean F_Prev_Key (String args, ClientData *pCD, XEvent *event)
#endif /* _NO_PROTO */
{
    if (wmGD.keyboardFocusPolicy == KEYBOARD_FOCUS_EXPLICIT)
    {
	FocusPrevWindow ((unsigned long)args,
			    GetFunctionTimestamp ((XButtonEvent *)event));


    }

    return (True);

} /* END OF FUNCTION F_Prev_Key */



/*************************************<->*************************************
 *
 *  F_Pass_Key (args, pCD, event)
 *
 *
 *  Description:
 *  -----------
 *  This is a function stub for the f.pass_key window manager function.
 *
 *
 *  Inputs:
 *  ------
 *  args = (immediate value) window type flags
 *
 *  pCD = pointer to the client data
 *
 *  event = X event that invoked the function (key, button, or menu/NULL)
 *
 *************************************<->***********************************/

Boolean F_Pass_Key (args, pCD, event)
    String args;
    ClientData *pCD;
    XEvent *event;

{
    if (wmGD.passKeysActive)
    {
	/*
	 * Get out of pass keys mode.
	 */

	wmGD.passKeysActive = False;
	wmGD.passKeysKeySpec = NULL;
    }
    else
    {
	/*
	 * Get into pass keys mode.
	 */

	wmGD.passKeysActive = True;
    }

    return (False);

} /* END OF FUNCTION F_Pass_Key */



/*************************************<->*************************************
 *
 *  F_Maximize (args, pCD, event)
 *
 *
 *  Description:
 *  -----------
 *  This is the window manager function handler for maximizing a client
 *  window.
 *
 *************************************<->***********************************/

#ifdef _NO_PROTO
Boolean F_Maximize (args, pCD, event)
    String args;
    ClientData *pCD;
    XEvent *event;

#else /* _NO_PROTO */
Boolean F_Maximize (String args, ClientData *pCD, XEvent *event)
#endif /* _NO_PROTO */
{
    if (pCD && (pCD->clientFunctions & MWM_FUNC_MAXIMIZE))
    {
	SetClientStateWithEventMask (pCD, MAXIMIZED_STATE,
	    GetFunctionTimestamp ((XButtonEvent *)event),
		GetEventInverseMask(event));
    }

    return (False);

} /* END OF FUNCTION F_Maximize */



/*************************************<->*************************************
 *
 *  F_Menu (args, pCD, event)
 *
 *
 *  Description:
 *  -----------
 *  This is the window manager function handler for posting a menu.
 *  This function can only be invoked by a key or button event.
 *   wmGD.menuUnpostKeySpec is assumed set appropriately; it will be set to
 *     NULL when the menu is unposted.
 *
 *************************************<->***********************************/

#ifdef _NO_PROTO
Boolean F_Menu (args, pCD, event)
    String args;
    ClientData *pCD;
    XEvent *event;

#else /* _NO_PROTO */
Boolean F_Menu (String args, ClientData *pCD, XEvent *event)
#endif /* _NO_PROTO */
{
    MenuSpec    *menuSpec;
    Context      menuContext;
    unsigned int button;
    int          x;
    int		 y;
    long         flags = POST_AT_XY;
    WmScreenData *pSD;


    if (event && 
	((event->type == ButtonPress) || (event->type == ButtonRelease)))
    {
        button = event->xbutton.button;
	x = event->xbutton.x_root;
	y = event->xbutton.y_root;
        if (event->type == ButtonRelease)
	{
	    flags |= POST_TRAVERSAL_ON;
	}
    }
    else if (event && 
	((event->type == KeyPress) || (event->type == KeyRelease)))
    {
        button = NoButton;
	x = event->xkey.x_root;
	y = event->xkey.y_root;
    }
    else
    {
	/*
	 * A button or key event must be used to post a menu using this 
	 * function.
	 */

	return (False);
    }

    if (pCD)
    {
	if (pCD->clientState == NORMAL_STATE)
	{
	    menuContext = F_CONTEXT_NORMAL;
	}
	else if (pCD->clientState == MAXIMIZED_STATE)
	{
	    menuContext = F_CONTEXT_MAXIMIZE;
	}
	else 
	{
	    menuContext = F_CONTEXT_ICON;
	}
	if (P_ICON_BOX(pCD) &&
            event->xany.window == ICON_FRAME_WIN(pCD))
	{
	    if (pCD->clientState == MINIMIZED_STATE)
	    {
		menuContext = F_SUBCONTEXT_IB_IICON;
	    }
	    else
	    {
		menuContext = F_SUBCONTEXT_IB_WICON;
	    }
	}
    }
    else
    {
	menuContext = F_CONTEXT_ROOT;
    }


    /* We do not add this MenuSpec to wmGD.acceleratorMenuSpecs.
     * This should have been done in MakeWmFunctionResources().
     */

    pSD = (pCD) ? PSD_FOR_CLIENT(pCD) : ACTIVE_PSD;
    if (menuSpec = MakeMenu (pSD, args, menuContext, 
			     menuContext, (MenuItem *) NULL, FALSE))
    {
        PostMenu (menuSpec, pCD, x, y, button, menuContext, flags, event);
    }

    return (False);

} /* END OF FUNCTION F_Menu */


/*************************************<->*************************************
 *
 *  F_Minimize (args, pCD, event)
 *
 *
 *  Description:
 *  -----------
 *  This is the window manager function handler for minimizing a client
 *  window.
 *
 *************************************<->***********************************/

#ifdef _NO_PROTO
Boolean F_Minimize (args, pCD, event)
    String args;
    ClientData *pCD;
    XEvent *event;

#else /* _NO_PROTO */
Boolean F_Minimize (String args, ClientData *pCD, XEvent *event)
#endif /* _NO_PROTO */
{
    ClientData *pcdLeader;


    if (pCD)
    {
	/*
	 * If the window is a transient then minimize the entire transient
	 * tree including the transient leader.
	 */
	
	pcdLeader = (pCD->transientLeader) ?
					FindTransientTreeLeader (pCD) : pCD;
	if (pcdLeader->clientFunctions & MWM_FUNC_MINIMIZE)
	{
	    SetClientStateWithEventMask (pCD, MINIMIZED_STATE,
		GetFunctionTimestamp ((XButtonEvent *)event),
		GetEventInverseMask(event));
	}
    }

    return (False);

} /* END OF FUNCTION F_Minimize */



/*************************************<->*************************************
 *
 *  F_Move (args, pCD, event)
 *
 *
 *  Description:
 *  -----------
 *  This is the window manager function handler for moving a client window
 *  or icon.
 *
 *************************************<->***********************************/

#ifdef _NO_PROTO
Boolean F_Move (args, pCD, event)
    String args;
    ClientData *pCD;
    XEvent *event;

#else /* _NO_PROTO */
Boolean F_Move (String args, ClientData *pCD, XEvent *event)
#endif /* _NO_PROTO */
{
    if (pCD && (pCD->clientFunctions & MWM_FUNC_MOVE))
    {
	StartClientMove (pCD, event);
	HandleClientFrameMove (pCD, event);
    }

    return (False);

} /* END OF FUNCTION F_Move */



/*************************************<->*************************************
 *
 *  F_Next_Cmap (args, pCD, event)
 *
 *
 *  Description:
 *  -----------
 *  This is the window manager function handler installing the next
 *  colormap in the list of client window colormaps.
 *
 *************************************<->***********************************/

#ifdef _NO_PROTO
Boolean F_Next_Cmap (args, pCD, event)
    String args;
    ClientData *pCD;
    XEvent *event;

#else /* _NO_PROTO */
Boolean F_Next_Cmap (String args, ClientData *pCD, XEvent *event)
#endif /* _NO_PROTO */
{
    if (pCD == NULL)
    {
	pCD = ACTIVE_PSD->colormapFocus;
    }

    if (pCD && (pCD->clientCmapCount > 0) &&
        ((pCD->clientState == NORMAL_STATE) ||
	 (pCD->clientState == MAXIMIZED_STATE)))
    {
	if (++(pCD->clientCmapIndex) >= pCD->clientCmapCount)
	{
	    pCD->clientCmapIndex = 0;
	}
	pCD->clientColormap = pCD->clientCmapList[pCD->clientCmapIndex];
	if (ACTIVE_PSD->colormapFocus == pCD)
	{
#ifndef OLD_COLORMAP /* colormap */
	    /*
	     * We just re-ordered the colormaps list,
	     * so we need to re-run the whole thing.
	     */
	    pCD->clientCmapFlagsInitialized = 0;
	    ProcessColormapList (ACTIVE_PSD, pCD);
#else /* OSF original */
	    WmInstallColormap (ACTIVE_PSD, pCD->clientColormap);
#endif
	}
    }

    return (True);

} /* END OF FUNCTION F_Next_Cmap */



/*************************************<->*************************************
 *
 *  F_Nop (args, pCD, event)
 *
 *
 *  Description:
 *  -----------
 *  This is the window manager function handler for doing nothing.
 *
 *************************************<->***********************************/

#ifdef _NO_PROTO
Boolean F_Nop (args, pCD, event)
    String args;
    ClientData *pCD;
    XEvent *event;

#else /* _NO_PROTO */
Boolean F_Nop (String args, ClientData *pCD, XEvent *event)
#endif /* _NO_PROTO */
{

    return (True);

} /* END OF FUNCTION F_Nop */



/*************************************<->*************************************
 *
 *  F_Normalize (args, pCD, event)
 *
 *
 *  Description:
 *  -----------
 *  This is the window manager function handler for putting a client window
 *  in the normal state.
 *
 *************************************<->***********************************/

#ifdef _NO_PROTO
Boolean F_Normalize (args, pCD, event)
    String args;
    ClientData *pCD;
    XEvent *event;

#else /* _NO_PROTO */
Boolean F_Normalize (String args, ClientData *pCD, XEvent *event)
#endif /* _NO_PROTO */
{

    if (pCD)
    {
	SetClientStateWithEventMask (pCD, NORMAL_STATE,
	    GetFunctionTimestamp ((XButtonEvent *)event),
		GetEventInverseMask(event));
    }

    return (False);

} /* END OF FUNCTION F_Normalize */



/*************************************<->*************************************
 *
 *  F_Normalize_And_Raise (args, pCD, event)
 *
 *
 *  Description:
 *  -----------
 *  This is the window manager function handler for putting a client window
 *  in the normal state and raising it from and icon.
 *
 *************************************<->***********************************/

#ifdef _NO_PROTO
Boolean F_Normalize_And_Raise (args, pCD, event)
    String args;
    ClientData *pCD;
    XEvent *event;

#else /* _NO_PROTO */
Boolean F_Normalize_And_Raise (String args, ClientData *pCD, XEvent *event)
#endif /* _NO_PROTO */
{
    
    if (pCD)
    {
        if (pCD->clientState == MINIMIZED_STATE)
        {
            /* normalize window  */
            SetClientStateWithEventMask (pCD, NORMAL_STATE,
                          (Time)
                          (event
                           ? GetFunctionTimestamp ((XButtonEvent *)event)
                           : GetTimestamp ()),
			GetEventInverseMask(event));
        }
        else
        {
	    /* Make sure we are in NORMAL_STATE */
	    SetClientStateWithEventMask (pCD, NORMAL_STATE,
			    GetFunctionTimestamp ((XButtonEvent *)event),
				GetEventInverseMask(event));

            /* Raise the window and set the keyboard focus to the window */
            F_Raise (NULL, pCD, (XEvent *)NULL);
	    if (wmGD.raiseKeyFocus)
	    {
		F_Focus_Key (NULL, pCD,
			     (event 
			      ? ((XEvent *)event)
			      : ((XEvent *)NULL)));
	    }
        }
	wmGD.clickData.clickPending = False;
	wmGD.clickData.doubleClickPending = False;
    }

    return (False);

} /* END OF FUNCTION F_Normalize_And_Raise */



/*************************************<->*************************************
 *
 *  F_Restore (args, pCD, event)
 *
 *
 *  Description:
 *  -----------
 *  This is the window manager function handler for putting a client window
 *  in the normal state.
 *
 *************************************<->***********************************/

#ifdef _NO_PROTO
Boolean F_Restore (args, pCD, event)
    String args;
    ClientData *pCD;
    XEvent *event;

#else /* _NO_PROTO */
Boolean F_Restore (String args, ClientData *pCD, XEvent *event)
#endif /* _NO_PROTO */
{
    int newState;

    if (pCD)
    {
	/*
	 * If current state is MAXIMIZED state then just go to NORMAL state,
	 * otherwise (you are in MINIMIZED state) return to previous state.
	 */

	if (pCD->clientState == MAXIMIZED_STATE)
	{
	    SetClientStateWithEventMask (pCD, NORMAL_STATE,
			    GetFunctionTimestamp ((XButtonEvent *)event),
				GetEventInverseMask(event));
	}
	else
	{
	    if (pCD->maxConfig)
	    {
		newState = MAXIMIZED_STATE;
	    }
	    else
	    {
		newState = NORMAL_STATE;
	    }

	    SetClientStateWithEventMask (pCD, newState,
			    GetFunctionTimestamp ((XButtonEvent *)event),
				GetEventInverseMask(event));
	}
    }

    return (False);

} /* END OF FUNCTION F_Restore */



/*************************************<->*************************************
 *
 *  F_Restore_And_Raise (args, pCD, event)
 *
 *
 *  Description:
 *  -----------
 *  This is the window manager function handler for putting a client window
 *  in the normal state and raising it from and icon.
 *
 *************************************<->***********************************/

#ifdef _NO_PROTO
Boolean F_Restore_And_Raise (args, pCD, event)
    String args;
    ClientData *pCD;
    XEvent *event;

#else /* _NO_PROTO */
Boolean F_Restore_And_Raise (String args, ClientData *pCD, XEvent *event)
#endif /* _NO_PROTO */
{
    int newState;
    
    if (pCD)
    {
        if (pCD->clientState == MINIMIZED_STATE)
        {
            /* Restore window  */
	    if (pCD->maxConfig)
	    {
		newState = MAXIMIZED_STATE;
	    }
	    else
	    {
		newState = NORMAL_STATE;
	    }

            SetClientStateWithEventMask (pCD, newState,
                          (Time)
                          (event
                           ? GetFunctionTimestamp ((XButtonEvent *)event)
                           : GetTimestamp ()),
			GetEventInverseMask(event));
        }
        else
        {
	    /* Make sure we restore the window first */
	    F_Restore (NULL, pCD, event);

            /* Raise the window and set the keyboard focus to the window */
            F_Raise (NULL, pCD, (XEvent *)NULL);
	    if (wmGD.raiseKeyFocus)
	    {
		F_Focus_Key (NULL, pCD,
			     (event 
			      ? ((XEvent *)event)
			      : ((XEvent *)NULL)));
	    }
        }
	wmGD.clickData.clickPending = False;
	wmGD.clickData.doubleClickPending = False;
    }

    return (False);

} /* END OF FUNCTION F_Restore_And_Raise */



/*************************************<->*************************************
 *
 *  F_Pack_Icons (args, pCD, event)
 *
 *
 *  Description:
 *  -----------
 *  This is the window manager function handler for packing icons in the
 *  icon box or on the desktop.
 *
 *************************************<->***********************************/

#ifdef _NO_PROTO
Boolean F_Pack_Icons (args, pCD, event)
    String args;
    ClientData *pCD;
    XEvent *event;

#else /* _NO_PROTO */
Boolean F_Pack_Icons (String args, ClientData *pCD, XEvent *event)
#endif /* _NO_PROTO */
{
    
    IconBoxData *pIBD;

    if ( ACTIVE_PSD->useIconBox )
    {
	pIBD = ACTIVE_WS->pIconBox;
	if (pCD)
	{
	    while (pCD != pIBD->pCD_iconBox)
	    {
		if (pIBD->pNextIconBox)
		{
		    pIBD = pIBD->pNextIconBox;
		}
		else
		{
		    pIBD = NULL;
		    break;
		}
	    }
	}
	if (pIBD)
	{
	    PackIconBox (pIBD, False, False, 0, 0);
	}
	else
	{
	   PackRootIcons ();
	}
    }
    else
    {
	PackRootIcons ();
    }

    return (True);


} /* END OF FUNCTION F_Pack_Icons */



/*************************************<->*************************************
 *
 *  F_Post_SMenu (args, pCD, event)
 *
 *
 *  Description:
 *  -----------
 *  This is the window manager function handler for posting the system menu
 *  for the specified client.
 *  This function can only be invoked by a key or button event.
 *  wmGD.menuUnpostKeySpec is assumed set appropriately; it will be set to
 *    NULL when the menu is unposted.
 *
 *************************************<->***********************************/

#ifdef _NO_PROTO
Boolean F_Post_SMenu (args, pCD, event)
    String args;
    ClientData *pCD;
    XEvent *event;

#else /* _NO_PROTO */
Boolean F_Post_SMenu (String args, ClientData *pCD, XEvent *event)
#endif /* _NO_PROTO */
{
    Context menuContext;


    /*
     * An event must be used to post the system menu using this function.
     */

    if (event && pCD && pCD->systemMenuSpec)
    {
        /*
	 * Determine whether the keyboard is posting the menu and post
	 * the menu at an appropriate place.
         */

	if (pCD->clientState == NORMAL_STATE)
	{
	    menuContext = F_CONTEXT_NORMAL;    
	}
	else if (pCD->clientState == MAXIMIZED_STATE)
	{
	    menuContext = F_CONTEXT_MAXIMIZE;
	}
	else 
	{
	    menuContext = F_CONTEXT_ICON;
	}
	if (P_ICON_BOX(pCD) &&
            event->xany.window == ICON_FRAME_WIN(pCD))
	{
	    if (pCD->clientState == MINIMIZED_STATE)
	    {
		menuContext = F_SUBCONTEXT_IB_IICON;
	    }
	    else
	    {
		menuContext = F_SUBCONTEXT_IB_WICON;
	    }
	}

	if ((event->type == KeyPress) || (event->type == KeyRelease))
	{
	    /*
	     * Set up for "sticky" menu processing if specified.
	     */

	    if (pCD->clientState == MINIMIZED_STATE ||
		menuContext == (F_SUBCONTEXT_IB_IICON | F_SUBCONTEXT_IB_WICON))
	    {
		if (wmGD.iconClick)
		{
		    wmGD.checkHotspot = True;
		}
	    }
	    else if (wmGD.systemButtonClick && (pCD->decor & MWM_DECOR_MENU))
	    {
		wmGD.checkHotspot = True;
	    }

	    PostMenu (pCD->systemMenuSpec, pCD, 0, 0, NoButton, menuContext,
		      0, event);
	}
	else if (event->type == ButtonPress)
	{
	    PostMenu (pCD->systemMenuSpec, pCD, 
		event->xbutton.x_root, event->xbutton.y_root,
	  	event->xbutton.button, menuContext, POST_AT_XY, event);
	}
	else if (event->type == ButtonRelease)
	{
	    PostMenu (pCD->systemMenuSpec, pCD, 
		event->xbutton.x_root, event->xbutton.y_root,
	  	event->xbutton.button, menuContext,
		POST_AT_XY | POST_TRAVERSAL_ON, event);
	}
    }

    return (False);

} /* END OF FUNCTION F_PostSMenu */



/*************************************<->*************************************
 *
 *  F_Kill (args, pCD, event)
 *
 *
 *  Description:
 *  -----------
 *  This is the window manager function handler for terminating a client.
 *  Essentially the client connection is shut down.
 *
 *************************************<->***********************************/

#ifdef _NO_PROTO
Boolean F_Kill (args, pCD, event)
    String args;
    ClientData *pCD;
    XEvent *event;

#else /* _NO_PROTO */
Boolean F_Kill (String args, ClientData *pCD, XEvent *event)
#endif /* _NO_PROTO */
{
    if (pCD && (pCD->clientFunctions & MWM_FUNC_CLOSE))
    {
	Boolean do_delete_window =
		pCD->protocolFlags & PROTOCOL_WM_DELETE_WINDOW;
	Boolean do_save_yourself =
		pCD->protocolFlags & PROTOCOL_WM_SAVE_YOURSELF;

	if (!do_delete_window && !do_save_yourself)
	  {
	    XKillClient (DISPLAY, pCD->client);
	  }

	else if (do_delete_window)
	  {
	    /*
	     * The client wants to be notified, not killed.
	     */

	    SendClientMsg (pCD->client, (long) wmGD.xa_WM_PROTOCOLS,
		   (long) wmGD.xa_WM_DELETE_WINDOW, CurrentTime, NULL, 0);

	    if (do_save_yourself)
	      {
		SendClientMsg (pCD->client, (long) wmGD.xa_WM_PROTOCOLS,
		       (long) wmGD.xa_WM_SAVE_YOURSELF, CurrentTime, NULL, 0);
	      }
	  }
	else /* do_save_yourself */
	  {
	    /*
	     * Send a WM_SAVE_YOURSELF message and wait for a change to
	     * the WM_COMMAND property.
	     * !!! button and key input should be kept from the window !!!
	     */

	    if (AddWmTimer (TIMER_QUIT, (unsigned long) wmGD.quitTimeout, pCD))
	      {
	        SendClientMsg (pCD->client, (long) wmGD.xa_WM_PROTOCOLS,
		       (long) wmGD.xa_WM_SAVE_YOURSELF, CurrentTime, NULL, 0);

		pCD->clientFlags |= CLIENT_TERMINATING;
	      }
	    else
	      {
		XKillClient (DISPLAY, pCD->client);
	      }
	  }
      }

    return (False);

} /* END OF FUNCTION F_Kill */



/*************************************<->*************************************
 *
 *  F_Refresh (args, pCD, event)
 *
 *
 *  Description:
 *  -----------
 *  This is the window manager function handler for causing all windows
 *  in the workspace to be redrawn.
 *
 *************************************<->***********************************/

#ifdef _NO_PROTO
Boolean F_Refresh (args, pCD, event)
    String args;
    ClientData *pCD;
    XEvent *event;

#else /* _NO_PROTO */
Boolean F_Refresh (String args, ClientData *pCD, XEvent *event)
#endif /* _NO_PROTO */
{
    Window win;

			 /* default background_pixmap is None */
    win = XCreateWindow (DISPLAY,
			 ACTIVE_ROOT, 0, 0,
	                 (unsigned int) DisplayWidth (DISPLAY, 
			     ACTIVE_SCREEN),
	                 (unsigned int) DisplayHeight (DISPLAY, 
			     ACTIVE_SCREEN),
	                 0, 
                         0,
	                 InputOutput,
                         CopyFromParent,
	                 0, 
			 (XSetWindowAttributes *)NULL);   

    XMapWindow (DISPLAY, win);
    XDestroyWindow (DISPLAY, win);
    XFlush (DISPLAY);

    return (True);

} /* END OF FUNCTION F_Refresh */



/*************************************<->*************************************
 *
 *  F_Resize (args, pCD, event)
 *
 *
 *  Description:
 *  -----------
 *  This is the window manager function handler for resizing a client window.
 *
 *************************************<->***********************************/

#ifdef _NO_PROTO
Boolean F_Resize (args, pCD, event)
    String args;
    ClientData *pCD;
    XEvent *event;

#else /* _NO_PROTO */
Boolean F_Resize (String args, ClientData *pCD, XEvent *event)
#endif /* _NO_PROTO */
{
    if (pCD && (pCD->clientFunctions & MWM_FUNC_RESIZE) &&
	((pCD->clientState == NORMAL_STATE) ||
					(pCD->clientState == MAXIMIZED_STATE)))
    {
	StartClientResize (pCD, event);
	HandleClientFrameResize (pCD, event);
    }

    return (False);

} /* END OF FUNCTION F_Resize */



/*************************************<->*************************************
 *
 *  F_Restart (args, pCD, event)
 *
 *
 *  Description:
 *  -----------
 *  This is the window manager function handler for restarting the window
 *  manager.
 *
 *************************************<->***********************************/

#ifdef DEC_MOTIF_EXTENSION
#ifdef _NO_PROTO
Boolean F_Restart( args, pCD, wid )

String args;
ClientData *pCD;
Widget wid;

#else /* _NO_PROTO */

Boolean F_Restart( String args, ClientData *pCD, Widget wid )

#endif /* _NO_PROTO */

#else
#ifdef _NO_PROTO
Boolean F_Restart (args, pCD, event)
    String args;
    ClientData *pCD;
    XEvent *event;

#else /* _NO_PROTO */
Boolean F_Restart (String args, ClientData *pCD, XEvent *event)
#endif /* _NO_PROTO */
#endif /* DEC_MOTIF_EXTENSION */
{
    if (wmGD.showFeedback & WM_SHOW_FB_RESTART)
    {
#ifdef DEC_MOTIF_EXTENSION
/* Check if the customization info has been saved. */
      if ( !mwm_cust_apply_up( WID_SCREEN, WID_SCREEN_NUM, TRUE ))
#endif
	ConfirmAction (ACTIVE_PSD, RESTART_ACTION);
    }
    else
    {
	RestartWm (MWM_INFO_STARTUP_CUSTOM);
    }
    return (False);

} /* END OF FUNCTION F_Restart */



/*************************************<->*************************************
 *
 *  Do_Restart (dummy)
 *
 *
 *  Description:
 *  -----------
 *  Callback function for restarting the window manager.
 *
 *************************************<->***********************************/

#ifdef _NO_PROTO
void Do_Restart (dummy)
Boolean dummy;
#else /* _NO_PROTO */
void Do_Restart (Boolean dummy)
#endif /* _NO_PROTO */
{
    RestartWm (MWM_INFO_STARTUP_CUSTOM);

} /* END OF FUNCTION Do_Restart */



/*************************************<->*************************************
 *
 *  RestartWm (startupFlags)
 *
 *
 *  Description:
 *  -----------
 *  Actually restarts the window manager.
 *
 *
 *  Inputs:
 *  ------
 *  startupFlags = flags to be put into the Wm_INFO property for restart.
 *
 *************************************<->***********************************/

#ifdef _NO_PROTO
void RestartWm (startupFlags)
    long startupFlags;

#else /* _NO_PROTO */
void RestartWm (long startupFlags)
#endif /* _NO_PROTO */
{
    ClientListEntry *pNextEntry;
    int scr;
#ifdef VMS
int unit = 0;
#endif

    for (scr=0; scr<wmGD.numScreens; scr++)
    {
	if(wmGD.Screens[scr].managed)
	{
	    
	    /*
	     * Set up the _MOTIF_WM_INFO property on the root window 
	     * to indicate a restart.
	     */
	    
	    SetMwmInfo (wmGD.Screens[scr].rootWindow, startupFlags, 0);
	    
	    
	    /*
	     * Unmap client windows and reparent them to the root window.
	     */
	    
	    pNextEntry = wmGD.Screens[scr].lastClient;
	    while (pNextEntry)
	    {
		if (pNextEntry->type == NORMAL_STATE)
		{
		    if (pNextEntry->pCD->clientFlags & CLIENT_WM_CLIENTS)
		    {
			if (pNextEntry->pCD->clientState != MINIMIZED_STATE)
			{
			    XUnmapWindow (DISPLAY, 
					  pNextEntry->pCD->clientFrameWin);
			}
		    }
		    else
		    {
			DeFrameClient (pNextEntry->pCD);
		    }
		}
		pNextEntry = pNextEntry->prevSibling;
	    }
	}
	
    }

#ifdef DEC_MOTIF_EXTENSION
#ifdef HYPERHELP
    /* Exit any hyperhelp windows */
    mwm_help_exit();
#endif
#endif
    /*
     * This fixes restart problem when going from explicit focus to
    /*
     * This fixes restart problem when going from explicit focus to
     * pointer focus.  Window under pointer was not getting focus indication
     * until pointer was moved to new window, or out of and into the
     * window.
     */

    XSetInputFocus (DISPLAY, PointerRoot, RevertToPointerRoot, CurrentTime);
    XSync (DISPLAY, False);



    /*
     * Restart the window manager with the initial arguments plus
     * the restart settings.
     */

#ifdef VMS
    exit ( vms_restart( DISPLAY, &unit ));
#else
    execvp (*(wmGD.argv), wmGD.argv);
    Warning ("Cannot restart the window manager");
    Do_Quit_Mwm (True);
#endif


} /* END OF FUNCTION RestartWm */



/*************************************<->*************************************
 *
 *  DeFrameClient (pCD)
 *
 *
 *  Description:
 *  -----------
 *  Unmaps a client window (and client icon window) and reparents the
 *  window back to the root.
 *
 *
 *  Inputs:
 *  -------
 *  pCD = pointer to the client data for the window to be de-framed.
 *
 *************************************<->***********************************/

#ifdef _NO_PROTO
void DeFrameClient (pCD)
    ClientData *pCD;

#else /* _NO_PROTO */
void DeFrameClient (ClientData *pCD)
#endif /* _NO_PROTO */
{
    int x, y;
    int xoff, yoff;
    XWindowChanges windowChanges;

    while (pCD)
    {
        if (pCD->clientState != MINIMIZED_STATE)
        {
	    XUnmapWindow (DISPLAY, pCD->clientFrameWin);
        }

        if (pCD->iconWindow && (pCD->clientFlags & ICON_REPARENTED))
        {
	    XUnmapWindow (DISPLAY, pCD->iconWindow);
	    XRemoveFromSaveSet (DISPLAY, pCD->iconWindow);
	    XReparentWindow (DISPLAY, pCD->iconWindow, 
		ROOT_FOR_CLIENT(pCD), pCD->iconX, pCD->iconY);
        }

        if (pCD->maxConfig)
        {
	    x = pCD->maxX;
	    y = pCD->maxY;
        }
        else
        {
	    if(wmGD.positionIsFrame)
	    {
		CalculateGravityOffset (pCD, &xoff, &yoff);
		x = pCD->clientX - xoff;
		y = pCD->clientY - yoff;
	    }
	    else
	    {
		x = pCD->clientX;
		y = pCD->clientY;
	    }
        }

#ifndef UNMAP_ON_RESTART
	if (pCD->clientState == MINIMIZED_STATE)
	{
	    XUnmapWindow (DISPLAY, pCD->client);
	}
#else
	XUnmapWindow (DISPLAY, pCD->client);
#endif
	XRemoveFromSaveSet (DISPLAY, pCD->client);
        XReparentWindow (DISPLAY, pCD->client, 
	    ROOT_FOR_CLIENT(pCD), x, y);

	if (pCD->transientChildren)
	{
	    DeFrameClient (pCD->transientChildren);
	}

	/*
	 * restore X border
	 */
	windowChanges.x = x;
	windowChanges.y = y;
	windowChanges.border_width = pCD->xBorderWidth;
	XConfigureWindow (DISPLAY, pCD->client, CWBorderWidth | CWX | CWY,
			  &windowChanges);

	if (pCD->transientLeader)
	{
	    pCD = pCD->transientSiblings;
	}
	else
	{
	    pCD = NULL;
	}
    }

} /* END OF FUNCTION DeFrameClient */



/******************************<->*************************************
 *
 *  F_Send_Msg (args, pCD, event)
 *
 *
 *  Description:
 *  -----------
 *  This is the window manager function handler for sending a client
 *  message event to a client window.
 *
 *
 *  Inputs:
 *  ------
 *  args = (immediate value) message id
 *
 *  pCD = pointer to the client data
 *
 *  event = X event that invoked the function (key, button, or menu/NULL)
 *
 *
 ******************************<->***********************************/

#ifdef _NO_PROTO
Boolean F_Send_Msg (args, pCD, event)
    String args;
    ClientData *pCD;
    XEvent *event;

#else /* _NO_PROTO */
Boolean F_Send_Msg (String args, ClientData *pCD, XEvent *event)
#endif /* _NO_PROTO */
{
    register int i;


    if (pCD && pCD->mwmMessagesCount)
    {
	/*
	 * A message id must be made "active" by being included in the
	 * _MWM_MESSAGES property before the associated message can be sent.
	 */

	for (i = 0; i < pCD->mwmMessagesCount; i++)
	{
	    if (pCD->mwmMessages[i] == (long)args)
	    {
		SendClientMsg (pCD->client, (long) wmGD.xa_MWM_MESSAGES, 
		    (long)args, CurrentTime, NULL, 0);
		return (True);
	    }
	}
    }

    return (True);

} /* END OF FUNCTION F_Send_Msg */



/*************************************<->*************************************
 *
 *  F_Separator (args, pCD, event)
 *
 *
 *  Description:
 *  -----------
 *  This is a placeholder function; it should never be called.
 *
 *************************************<->***********************************/

#ifdef _NO_PROTO
Boolean F_Separator (args, pCD, event)
    String args;
    ClientData *pCD;
    XEvent *event;

#else /* _NO_PROTO */
Boolean F_Separator (String args, ClientData *pCD, XEvent *event)
#endif /* _NO_PROTO */
{

    return (True);

} /* END OF FUNCTION F_Separator */


#ifdef _NO_PROTO
Boolean ForceRaiseWindow (pcd)
	ClientData *pcd;
#else /* _NO_PROTO */
Boolean ForceRaiseWindow (ClientData *pcd)
#endif /* _NO_PROTO */
{
#if 0
    Window stackWindow;
    WmScreenData *pSD = (ACTIVE_WS)->pSD;
#endif
    XWindowChanges changes;
    Boolean restack = False;

#if 0
    if (pSD->clientList->type == MINIMIZED_STATE)
    {
	stackWindow = ICON_FRAME_WIN(pSD->clientList->pCD);
    }
    else
    {
	stackWindow = pSD->clientList->pCD->clientFrameWin;
    }
#endif

    /*
     * Windows did not raise on regular f.raise because the raise was
     * not relative to another window (methinks).
     */
    changes.stack_mode = Above;
    XConfigureWindow (DISPLAY, pcd->clientFrameWin, CWStackMode,
		      &changes);

    return (restack);
}



/*************************************<->*************************************
 *
 *  F_Raise (args, pCD, event)
 *
 *
 *  Description:
 *  -----------
 *  This is the window manager function handler for topping the client window
 *  so that it is unobscured.
 *
 *************************************<->***********************************/

#ifdef _NO_PROTO
Boolean F_Raise (args, pCD, event)
    String args;
    ClientData *pCD;
    XEvent *event;

#else /* _NO_PROTO */
Boolean F_Raise (String args, ClientData *pCD, XEvent *event)
#endif /* _NO_PROTO */
{
    ClientListEntry *pEntry;
    ClientListEntry *pNextEntry;
    ClientListEntry *pStackEntry;
    String string = args;
    int flags = STACK_NORMAL;

    if (string)
    {
	/* process '-client' argument */
	if (string[0] == '-')
	{
	    string = &string[1];
	    string = (String) GetString ((unsigned char **) &string);

	    pStackEntry = NULL;
	    pNextEntry = ACTIVE_PSD->clientList;
	    while (pNextEntry &&
		   (pEntry = FindClientNameMatch (pNextEntry, True, args,
						  F_GROUP_ALL)))
	    {
		pNextEntry = pEntry->nextSibling;
		Do_Raise (pEntry->pCD, pStackEntry, STACK_NORMAL);
		pStackEntry = pEntry;
	    }
	}
	/* process family stacking stuff */
	else if (*string)
	{
	    unsigned int  slen, len, index;

	    slen = strlen(args) - 2;		/* subtract '\n' and NULL */
	    for (index = 0; index < slen; string = &args[index+1])
	    {
		if ((string = (String) GetString ((unsigned char **) &string)) == NULL)
		   break;
		len = strlen(string);
		if (!strcmp(string,"within"))
		{
		    flags |= STACK_WITHIN_FAMILY;
		}
		else if (!strcmp(string,"freeFamily"))
		{
		    flags |= STACK_FREE_FAMILY;
		}
		index += len;
	    }
	    Do_Raise (pCD, (ClientListEntry *) NULL, flags);
	}

    }
    else if (pCD)
    {
	Do_Raise (pCD, (ClientListEntry *) NULL, STACK_NORMAL);
    }

    return (True);

} /* END OF FUNCTION F_Raise */



/*************************************<->*************************************
 *
 *  Do_Raise (pCD, pStackEntry)
 *
 *
 *  Description:
 *  -----------
 *  This is the window manager function handler for topping the client window
 *  so that it is unobscured.
 *
 *
 *  Inputs:
 *  ------
 *  pCD = pointer to the client data of the window (or icon) to be raised.
 * 
 *  pStackEntry = pointer to client list entry for window that is to be 
 *	above the raised window (if NULL window is raised to the top of the
 *	stack).
 *
 *************************************<->***********************************/

#ifdef _NO_PROTO
void Do_Raise (pCD, pStackEntry, flags)
    ClientData *pCD;
    ClientListEntry *pStackEntry;
    int flags;

#else /* _NO_PROTO */
void Do_Raise (ClientData *pCD, ClientListEntry *pStackEntry, int flags)
#endif /* _NO_PROTO */
{
    Boolean restackTransients;
    ClientData *pcdLeader;

#ifdef DEC_MOTIF_BUG_FIX
    WmWorkspaceData *pWS = pCD->pSD->pActiveWS;
#else
    WmWorkspaceData *pWS = ACTIVE_WS;
#endif

    pcdLeader = (pCD->transientLeader) ? FindTransientTreeLeader (pCD) : pCD;

    if (wmGD.systemModalActive && (pcdLeader != wmGD.systemModalClient))
    {
	/*
	 * Don't raise the window above the system modal window.
	 */
    }
    else if ((pcdLeader->clientState == MINIMIZED_STATE) &&
	     !P_ICON_BOX(pcdLeader))
    {
	/*
	 * Only restack the icon if it is not currently raised.
	 */

	if (pStackEntry)
	{
	    if (pStackEntry->nextSibling != &pcdLeader->iconEntry)
	    {
	        StackWindow (pWS, &pcdLeader->iconEntry, False /*below*/,
			     pStackEntry);
	        MoveEntryInList (pWS, &pcdLeader->iconEntry, False /*below*/,
				 pStackEntry);
	    }
	}
	else
	{
	    if (ACTIVE_PSD->clientList != &pcdLeader->iconEntry)
	    {
	        StackWindow (pWS, &pcdLeader->iconEntry, 
		    True /*on top*/, (ClientListEntry *) NULL);
	        MoveEntryInList (pWS, &pcdLeader->iconEntry, 
		    True /*on top*/, (ClientListEntry *) NULL);
	    }
	}
    }
    else /* NORMAL_STATE, MAXIMIZED_STATE, adoption */
    {
	/*
	 * If this is a transient window then put it on top of its
	 * sibling transient windows.
	 */

	restackTransients = False;
	if (pCD->transientLeader)
	{

	    /*
	     * If freeFamily stacking, then put dirty transient window
	     * (if any) back in place before force raise current window
	     * to the top of the global window stack.  Then return.
	     */

	    if (flags & STACK_FREE_FAMILY)
	    {
		if (dirtyStackEntry.pCD)
		{
		    RestoreTransients (pCD, Above);
		}

		dirtyStackEntry.pCD = pCD;

		if (dirtyStackEntry.pCD->transientLeader)
		  dirtyLeader = FindTransientTreeLeader (pCD);
		else
		  dirtyLeader = dirtyStackEntry.pCD;

		restackTransients = ForceRaiseWindow (pCD);
		return;
	    }

	    /*
	     * Reach here only if NOT doing a f.raise freeFamily (see
	     * return; statement above).  Put current transient on top of
	     * its sibling transient windows.
	     */
	    restackTransients = PutTransientOnTop (pCD);
	}

	/*
	 * If doing a regular f.raise and you have a dirty window, then
	 * clean up dirty transient window.
	 */

	if (dirtyStackEntry.pCD)
	{
	    /* 
	     * If raising a window in the same family as the dirty
	     * transient window, then just restack before raising.
	     * Else, restore the dirty transient in place before
	     * raising the current window.  Clear dirtyStack.
	     */
	    if (dirtyLeader == pcdLeader)
	    {
		if (dirtyStackEntry.pCD == FindTransientOnTop(pCD))
		{
		    RestackTransients (dirtyStackEntry.pCD, True);
		}
		else
		{
		    RestackTransients (dirtyStackEntry.pCD, False);
		}	
	    }
	    else
	    {
		RestoreTransients (pCD, Above);
	    }
	    dirtyStackEntry.pCD = NULL;
	}

	/*
	 * Only restack the window or transient window tree if it is
	 * not currently on top.
	 */

	if (pStackEntry)
	{
	    if (pStackEntry->nextSibling != &pcdLeader->clientEntry)
	    {
		StackWindow (pWS, &pcdLeader->clientEntry, False /*below*/,
		    pStackEntry);
		MoveEntryInList (pWS, &pcdLeader->clientEntry, False /*below*/,
		    pStackEntry);
	    }
	    else if (restackTransients)
	    {
	        RestackTransients (pCD, False);
	    }
	}
	else
	{
	    if (ACTIVE_PSD->clientList != &pcdLeader->clientEntry)
	    {
		StackWindow (pWS, &pcdLeader->clientEntry, 
		    True /*on top*/, (ClientListEntry *) NULL);
	        MoveEntryInList (pWS, &pcdLeader->clientEntry, 
		    True /*on top*/, (ClientListEntry *) NULL);
		dirtyStackEntry.pCD = NULL;
	    }
	    else if (restackTransients)
	    {
	        RestackTransients (pCD, True);
	    }
	}
    }

} /* END OF FUNCTION Do_Raise */



/*************************************<->*************************************
 *
 *  F_Raise_Lower (args, pCD, event)
 *
 *
 *  Description:
 *  -----------
 *  This window manager function tops an obscured window or icon and bottoms 
 *  a window or icon that is on top of the window stack.
 *
 *************************************<->***********************************/

#ifdef _NO_PROTO
Boolean F_Raise_Lower (args, pCD, event)
    String args;
    ClientData *pCD;
    XEvent *event;

#else /* _NO_PROTO */
Boolean F_Raise_Lower (String args, ClientData *pCD, XEvent *event)
#endif /* _NO_PROTO */
{
    ClientData *pcdLeader;

    if (pCD)
    {
	pcdLeader = (pCD->transientLeader) ?
					FindTransientTreeLeader (pCD) : pCD;

	/*
	 * Treat a raise/lower on a window in a transient tree as if it is
	 * a raise/lower for the whole tree.
	 */

	if (CheckIfClientObscuredByAny (pcdLeader))
	{
	    /*
	     * The window is obscured by another window, raise the window.
	     */

	    F_Raise (NULL, pcdLeader, (XEvent *)NULL);
	}
	else if (CheckIfClientObscuringAny (pcdLeader) &&
	        !(wmGD.systemModalActive &&
	         (pcdLeader == wmGD.systemModalClient)))
	{
	    /*
             * The window is obscuring another window and is
             * not system modal, lower the window.
	     */

	    F_Lower (NULL, pcdLeader, (XEvent *)NULL);
	}
    }

    return (True);

} /* END OF FUNCTION F_Raise_Lower */



/*************************************<->*************************************
 *
 *  F_Refresh_Win (args, pCD, event)
 *
 *
 *  Description:
 *  -----------
 *  This is the window manager function handler for causing a client window
 *  to redisplay itself.
 *
 *************************************<->***********************************/

#ifdef _NO_PROTO
Boolean F_Refresh_Win (args, pCD, event)
    String args;
    ClientData *pCD;
    XEvent *event;

#else /* _NO_PROTO */
Boolean F_Refresh_Win (String args, ClientData *pCD, XEvent *event)
#endif /* _NO_PROTO */
{
    Window win;
    unsigned int w, h;

    if (pCD && ((pCD->clientState == NORMAL_STATE) ||
		(pCD->clientState == MAXIMIZED_STATE)))
    {
        if (pCD->clientState == NORMAL_STATE)
	{
	    w = (unsigned int) pCD->clientWidth;
	    h = (unsigned int) pCD->clientHeight;
	}
	else
	{
	    w = (unsigned int) pCD->maxWidth;
	    h = (unsigned int) pCD->maxHeight;
	}

			 /* default background_pixmap is None */
        win = XCreateWindow (DISPLAY,
		         pCD->clientBaseWin,
		         pCD->matteWidth,
		         pCD->matteWidth,
		         w, h,
	                 0, 
	                 0,
	                 InputOutput,
                         CopyFromParent,
	                 0, 
			 (XSetWindowAttributes *)NULL);  

        XMapWindow (DISPLAY, win);
        XDestroyWindow (DISPLAY, win);
        XFlush (DISPLAY);
    }

    return (True);

} /* END OF FUNCTION F_Refresh_Win */



/*************************************<->*************************************
 *
 *  F_Set_Behavior (args, pCD, event)
 *
 *
 *  Description:
 *  -----------
 *  This function is used to switch the window manager configuration between
 *  the built-in configuration (for CXI behavior) and the user's custom
 *  configuration.
 *
 *************************************<->***********************************/

#ifdef _NO_PROTO
Boolean F_Set_Behavior (args, pCD, event)
    String args;
    ClientData *pCD;
    XEvent *event;

#else /* _NO_PROTO */
Boolean F_Set_Behavior (String args, ClientData *pCD, XEvent *event)
#endif /* _NO_PROTO */
{
    /*
     * Go system modal in starting to do the set behavior.
     */

    /* !!! grab the server and the pointer !!! */


    /*
     * Confirm that a set_behavior should be done.
     * Execute restart if so.
     */

    if (wmGD.showFeedback & WM_SHOW_FB_BEHAVIOR)
    {
	ConfirmAction (ACTIVE_PSD, (wmGD.useStandardBehavior) ?
		       CUSTOM_BEHAVIOR_ACTION : DEFAULT_BEHAVIOR_ACTION);
    }
    else
    {
	RestartWm ((long) ((wmGD.useStandardBehavior) ?
			MWM_INFO_STARTUP_CUSTOM : MWM_INFO_STARTUP_STANDARD));
    }
    return (False);

} /* END OF FUNCTION F_Set_Behavior */



/*************************************<->*************************************
 *
 *  Do_Set_Behavior (dummy)
 *
 *
 *  Description:
 *  -----------
 *  Callback to do the f.set_behavior function.
 *
 *************************************<->***********************************/

#ifdef _NO_PROTO
void Do_Set_Behavior (dummy)
Boolean dummy;
#else /* _NO_PROTO */
void Do_Set_Behavior (Boolean dummy)
#endif /* _NO_PROTO */
{
    RestartWm ((long) ((wmGD.useStandardBehavior) ?
			MWM_INFO_STARTUP_CUSTOM : MWM_INFO_STARTUP_STANDARD));

} /* END OF FUNCTION Do_Set_Behavior */



/*************************************<->*************************************
 *
 *  F_Title (args, pCD, event)
 *
 *
 *  Description:
 *  -----------
 *  This is a placeholder function; it should never be called.
 *
 *************************************<->***********************************/

#ifdef _NO_PROTO
Boolean F_Title (args, pCD, event)
    String args;
    ClientData *pCD;
    XEvent *event;

#else /* _NO_PROTO */
Boolean F_Title (String args, ClientData *pCD, XEvent *event)
#endif /* _NO_PROTO */
{

    return (True);

} /* END OF FUNCTION F_Title */



/******************************<->*************************************
 *
 *  F_Screen (args, pCD, event)
 *
 *
 *  Description:
 *  -----------
 *  This is the window manager function handler for warping to screens
 *
 *
 *  Inputs:
 *  ------
 *  args = (immediate value) window type flags
 *
 *  pCD = pointer to the client data
 *
 *  event = X event that invoked the function (key, button, or menu/NULL)
 *
 *  NOTE: May want to consider tracking changes in screen because in
 *	  managing a new window (ie. in ManageWindow()).
 *
 *  Outputs:
 *  -------
 *  RETURN = if True then further button binding/function processing can
 *           be done for the event that caused this function to be called.
 *
 *************************************<->***********************************/

#ifdef _NO_PROTO
Boolean F_Screen (args, pCD, event)
    String args;
    ClientData *pCD;
    XEvent *event;

#else /* _NO_PROTO */
Boolean F_Screen (String args, ClientData *pCD, XEvent *event)
#endif /* _NO_PROTO */
{
    Window dumwin;
    int x, y, dumint;
    unsigned int dummask;
    WmScreenData *newscr = NULL;
    int scr, inc;
    static int PreviousScreen = -1;
    char pch[80];
    

    if (PreviousScreen == -1)
    {
	PreviousScreen = DefaultScreen(DISPLAY);
    }

    if (strcmp (args, "next") == 0)
    {
	scr = ACTIVE_PSD->screen + 1;
	inc = 1;
    }
    else if (strcmp (args, "prev") == 0)
    {
	scr = ACTIVE_PSD->screen - 1;
	inc = -1;
    }
    else if (strcmp (args, "back") == 0)
    {
	scr = PreviousScreen;
	inc = 0;
    }
    else
    {
	scr = atoi (args);
	inc = 0;
    }

    while (!newscr) {
					/* wrap around */
	if (scr < 0) 
	  scr = wmGD.numScreens - 1;
	else if (scr >= wmGD.numScreens)
	  scr = 0;

	newscr = &(wmGD.Screens[scr]);
	if (!wmGD.Screens[scr].managed) { /* make sure screen is managed */
	    if (inc) {			/* walk around the list */
		scr += inc;
		continue;
	    }
	    sprintf(pch, 
		    "Unable to warp to unmanaged screen %d\n", scr);
	    Warning (&pch[0]);
	    XBell (DISPLAY, 0);
	    return (False);
	}
    }

    if (ACTIVE_PSD->screen == scr) return (False);  /* already on that screen */

    PreviousScreen = ACTIVE_PSD->screen;
    XQueryPointer (DISPLAY, ACTIVE_ROOT, &dumwin, &dumwin, &x, &y,
		   &dumint, &dumint, &dummask);

    XWarpPointer (DISPLAY, None, newscr->rootWindow, 0, 0, 0, 0, x, y);

    if (newscr && (wmGD.keyboardFocusPolicy == KEYBOARD_FOCUS_EXPLICIT))
    {
	/*
	 * Set the ACTIVE_PSD to the new screen so that Do_Focus_Key can 
	 * uses the new screen instead of the old screen.  Then call
	 * Do_Focus_Key with a NULL pCD to find a reasonable client to
	 * set focus to.
	 */
	SetActiveScreen (newscr);
        Do_Focus_Key (NULL, GetFunctionTimestamp ((XButtonEvent *)event),
		      ALWAYS_SET_FOCUS);
    }

    return (False);
}



/*************************************<->*************************************
 *
 *  GetFunctionTimestamp (pEvent)
 *
 *
 *  Description:
 *  -----------
 *  This function is used to extract a timestamp from a key or button event.
 *  If the event passed in is not a key or button event then a timestamp
 *  is generated.
 *
 *
 *  Inputs:
 *  ------
 *  event = pointer to an X event
 *
 *
 *  Outputs:
 *  -------
 *  RETURN = a timestamp
 *
 *************************************<->***********************************/

#ifdef _NO_PROTO
Time GetFunctionTimestamp (pEvent)
    XButtonEvent *pEvent;

#else /* _NO_PROTO */
Time GetFunctionTimestamp (XButtonEvent *pEvent)
#endif /* _NO_PROTO */
{
    Time time;

    if (pEvent &&
	(((pEvent->type == ButtonPress) || (pEvent->type == ButtonRelease)) ||
	 ((pEvent->type == KeyPress) || (pEvent->type == KeyRelease))))
    {
	time = pEvent->time;
    }
    else
    {
	time = GetTimestamp ();
    }

    return (time);

} /* END OF FUNCTION GetFunctionTimestamp */

/*
** name the event mask we need for a grab in order to find the matching 
** event for an event; right now handle only button-presses
*/
#ifdef _NO_PROTO
static unsigned int GetEventInverseMask(event)
	XEvent *event;
#else	/* _NO_PROTO */
static unsigned int GetEventInverseMask(XEvent *event)
#endif	/* _NO_PROTO */
{
	if ((XEvent*)NULL == event)
		return 0;
	if (ButtonPress == event->type)
		return ButtonReleaseMask;	/* detail ? */
	/*
	expansion further here
	*/
	else 
		return 0;
}

#ifdef DEC_MOTIF_EXTENSION

/* Customize module */

/*******************************************************************/

    /* Customize the window manager */

#ifdef _NO_PROTO

Boolean F_DEC_Customize( args, pCD, wid )

String args;
ClientData *pCD;
Widget wid;

#else /* _NO_PROTO */

Boolean F_DEC_Customize( String args, ClientData *pCD, Widget wid )

#endif /* _NO_PROTO */

{
/********************************/

    mwm_cust( args, pCD, wid );
    return( True );

}    

/*******************************************************************/

    /* Help on the window manager */

#ifdef _NO_PROTO

Boolean F_DEC_Help( args, pCD, wid )

String args;
ClientData *pCD;
Widget wid;

#else /* _NO_PROTO */

Boolean F_DEC_Help( String args, ClientData *pCD, Widget wid )

#endif /* _NO_PROTO */

{

/********************************/

    mwm_help( args, pCD, wid );
    return( True );
}    

#endif
