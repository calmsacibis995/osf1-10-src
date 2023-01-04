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
 * Motif Release 1.2
*/ 
#ifdef REV_INFO
#ifndef lint
static char rcsid[] = "$RCSfile: WmSignal.c,v $ $Revision: 1.1.4.3 $ $Date: 1993/07/16 21:28:24 $"
#endif
#endif
/*
 * (c) Copyright 1987, 1988, 1989, 1990 HEWLETT-PACKARD COMPANY */

/*
 * Included Files:
 */

#include <signal.h>
#include "WmGlobal.h"

/*
 * include extern functions
 */

#include "WmFeedback.h"
#include "WmFunction.h"


/*
 * Function Declarations:
 */

#ifdef _NO_PROTO
void SetupWmSignalHandlers ();
void QuitWmSignalHandler ();
#else /* _NO_PROTO */
void SetupWmSignalHandlers (int);
void QuitWmSignalHandler (int);
#endif /* _NO_PROTO */




/*
 * Global Variables:
 */



/*************************************<->*************************************
 *
 *  SetupWmSignalHandlers ()
 *
 *
 *  Description:
 *  -----------
 *  This function sets up the signal handlers for the window manager.
 *
 *************************************<->***********************************/

#ifdef _NO_PROTO
void SetupWmSignalHandlers (dummy)
    int dummy;
#else /* _NO_PROTO */
void SetupWmSignalHandlers (int dummy)
#endif /* _NO_PROTO */
{
    void (*signalHandler) ();


    signalHandler = (void (*)())signal (SIGINT, SIG_IGN);
    if (signalHandler != (void (*)())SIG_IGN)
    {
	signal (SIGINT, QuitWmSignalHandler);
    }

    signalHandler = (void (*)())signal (SIGHUP, SIG_IGN);
    if (signalHandler != (void (*)())SIG_IGN)
    {
	signal (SIGHUP, QuitWmSignalHandler);
    }

    signal (SIGQUIT, QuitWmSignalHandler);

    signal (SIGTERM, QuitWmSignalHandler);


} /* END OF FUNCTION SetupWmSignalHandlers */



/*************************************<->*************************************
 *
 *  QuitWmSignalHandler ()
 *
 *
 *  Description:
 *  -----------
 *  This function is called on receipt of a signal that is to terminate the
 *  window manager.
 *
 *************************************<->***********************************/

#ifdef _NO_PROTO
void QuitWmSignalHandler (dummy)
    int dummy;
#else /* _NO_PROTO */
void QuitWmSignalHandler (int dummy)
#endif /* _NO_PROTO */
{
    if (wmGD.showFeedback & WM_SHOW_FB_KILL)
    {
	ConfirmAction (ACTIVE_PSD, QUIT_MWM_ACTION);
	XFlush(DISPLAY);
	SetupWmSignalHandlers(0);	 /* dummy paramater */
    }
    else
    {
	Do_Quit_Mwm(False);
    }

} /* END OF FUNCTION QuitWmSignalHandler */
