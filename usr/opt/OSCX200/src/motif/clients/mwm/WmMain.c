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
 * (c) Copyright 1989, 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
*/ 
/* 
 * Motif Release 1.2.1
*/ 
#ifdef REV_INFO
#ifndef lint
static char rcsid[] = "$RCSfile: WmMain.c,v $ $Revision: 1.1.4.4 $ $Date: 1993/10/18 17:20:22 $"
#endif
#endif
/*
 * (c) Copyright 1987, 1988, 1989, 1990 HEWLETT-PACKARD COMPANY */

/*
 * Included Files:
 */

#include "WmGlobal.h"

/*
 * include extern functions
 */

#include "WmCEvent.h"
#include "WmEvent.h"
#include "WmInitWs.h"


/*
 * Function Declarations:
 */

#define ManagedRoot(w) (!XFindContext (DISPLAY, (w), wmGD.screenContextType, \
(caddr_t *)&pSD) ? (SetActiveScreen (pSD), True) : False)

WmScreenData *pSD;

/*
 * Global Variables:
 */

WmGlobalData wmGD;
#ifdef VMS
char *WMProgramName;
#endif


/*************************************<->*************************************
 *
 *  main (argc, argv, environ)
 *
 *
 *  Description:
 *  -----------
 *  This is the main window manager function.  It calls window manager
 *  initializtion functions and has the main event processing loop.
 *
 *
 *  Inputs:
 *  ------
 *  argc = number of command line arguments (+1)
 *
 *  argv = window manager command line arguments
 *
 *  environ = window manager environment
 *
 *************************************<->***********************************/

#ifdef _NO_PROTO
#ifndef VMS 
int
#endif
main (argc, argv, environ)
    int argc;
    char *argv[];
    char *environ[];

#else /* _NO_PROTO */
#ifndef VMS
int
#endif
main (int argc, char *argv [], char *environ [])
#endif /* _NO_PROTO */
{
    XEvent	event;
    Boolean	dispatchEvent;
#ifdef VMS
    static char *FakeName = "mwm";
#endif /* VMS */
#ifndef NO_MULTIBYTE
    XtSetLanguageProc (NULL, (XtLanguageProc)NULL, NULL);
#endif
#ifdef VMS
    WMProgramName = argv[0];
    argv[0] = FakeName;
    vms_initialize();
#endif /* VMS */

    /*
     * Initialize the workspace:
     */

    InitWmGlobal (argc, argv, environ);
    
    /*
     * MAIN EVENT HANDLING LOOP:
     */

    for (;;)
    {
        XtAppNextEvent (wmGD.mwmAppContext, &event);


        /*
	 * Check for, and process non-widget events.  The events may be
	 * reported to the root window, to some client frame window,
	 * to an icon window, or to a "special" window management window.
	 * The lock modifier is "filtered" out for window manager processing.
	 */

	wmGD.attributesWindow = 0L;

	if ((event.type == ButtonPress) || (event.type == ButtonRelease) ||
	     (event.type == KeyPress) || (event.type == KeyRelease))
	{
	    wmGD.currentEventState = event.xbutton.state;
	    if (wmGD.ignoreLockMod)
	    {
	        event.xbutton.state &= ~(LockMask);
	    }
#ifdef DEC_MOTIF_EXTENSION
            /* For internationalization, if the mod keys are on and if
               the resource is set, ignore mod keys. */
            /* Is the resource set ? */
            if ( wmGD.ignoreModKeys || wmGD.ignoreAllModKeys )
              {
                if ( wmGD.ignoreAllModKeys || ModeSwitchOfDisplay( DISPLAY ) & Mod2Mask )
	            event.xbutton.state &= ~(Mod2Mask);
                if ( wmGD.ignoreAllModKeys || ModeSwitchOfDisplay( DISPLAY ) & Mod3Mask )
	            event.xbutton.state &= ~(Mod3Mask);
                if ( wmGD.ignoreAllModKeys || ModeSwitchOfDisplay( DISPLAY ) & Mod4Mask )
    	            event.xbutton.state &= ~(Mod4Mask);
              }
            else if (wmGD.ignoreNumLockMod) {
	        event.xbutton.state &= ~(NumLockMask);
	    }
#endif /* DEC_MOTIF_EXTENSION */
	}

	dispatchEvent = True;
	if (wmGD.menuActive)
	{
	    /*
	     * Do special menu event preprocessing.
	     */

	    if (wmGD.checkHotspot || wmGD.menuUnpostKeySpec ||
		wmGD.menuActive->accelKeySpecs)
	    {
	        dispatchEvent = WmDispatchMenuEvent ((XButtonEvent *) &event);
	    }
	}

	if (dispatchEvent)
	{
	    if (ManagedRoot(event.xany.window))
	    {
	        dispatchEvent = WmDispatchWsEvent (&event);
	    }
	    else
	    {
	        dispatchEvent = WmDispatchClientEvent (&event);
	    }

	    if (dispatchEvent)
	    {
                /*
                 * Dispatch widget related event:
                 */

                XtDispatchEvent (&event);
	    }
	}
    }

} /* END OF FUNCTION main */

