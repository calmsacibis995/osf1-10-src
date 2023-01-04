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
static char rcsid[] = "$RCSfile: WmError.c,v $ $Revision: 1.1.4.3 $ $Date: 1993/07/16 21:30:00 $"
#endif
#endif
/*
 * (c) Copyright 1987, 1988, 1989, 1990 HEWLETT-PACKARD COMPANY */

/*
 * Included Files:
 */

#include "WmGlobal.h"
#include <stdio.h>


/*
 * Function Declarations:
 */

#ifdef _NO_PROTO
void WmInitErrorHandler ();
int WmXErrorHandler ();
int WmXIOErrorHandler ();
void WmXtErrorHandler ();
void WmXtWarningHandler ();
void Warning ();
#else /* _NO_PROTO */
void WmInitErrorHandler (Display *display);
int WmXErrorHandler (Display *display, XErrorEvent *errorEvent);
int WmXIOErrorHandler (Display *display);
void WmXtErrorHandler (char *message);
void WmXtWarningHandler (char *message);
void Warning (char *message);
#endif /* _NO_PROTO */





/*************************************<->*************************************
 *
 *  WmInitErrorHandler (display)
 *
 *
 *  Description:
 *  -----------
 *  This function initializes the window manager error handler.
 *
 *
 *  Inputs:
 *  ------
 *  display = display we're talking about
 *  -------
 *
 *************************************<->***********************************/

#ifdef _NO_PROTO
void
WmInitErrorHandler (display)
    Display *display;

#else /* _NO_PROTO */
void
WmInitErrorHandler (Display *display)
#endif /* _NO_PROTO */
{

    XSetErrorHandler (WmXErrorHandler);
    XSetIOErrorHandler (WmXIOErrorHandler);

    XtSetWarningHandler (WmXtWarningHandler);
    XtSetErrorHandler (WmXtErrorHandler);

} /* END OF FUNCTION WmInitErrorHandler */


/*************************************<->*************************************
 *
 *  WmXErrorHandler (display, errorEvent)
 *
 *
 *  Description:
 *  -----------
 *  This function is the X error handler that is registered with X to
 *  handle X errors resulting from window management activities.
 *
 *
 *  Inputs:
 *  ------
 *  display = display on which X error occurred
 *
 *  errorEvent = pointer to a block of information describing the error
 *
 * 
 *  Outputs:
 *  -------
 *  wmGD.errorFlag = set to True
 *
 *  Return = 0
 *
 *************************************<->***********************************/

#ifdef _NO_PROTO
int
WmXErrorHandler (display, errorEvent)
    Display *display;
    XErrorEvent *errorEvent;

#else /* _NO_PROTO */
int
WmXErrorHandler (Display *display, XErrorEvent *errorEvent)
#endif /* _NO_PROTO */
{
    ClientData *pCD;


    /*
     * Check for a BadWindow error for a managed window.  If this error
     * is detected indicate in the client data that the window no longer
     * exists.
     */

    if ((errorEvent->error_code == BadWindow) &&
	!XFindContext (DISPLAY, errorEvent->resourceid, wmGD.windowContextType,
	     (caddr_t *)&pCD))
    {
	if (errorEvent->resourceid == pCD->client)
	{
	    pCD->clientFlags |= CLIENT_DESTROYED;
	}
    }

    wmGD.errorFlag = True;

    return (0);

} /* END OF FUNCTION WmXErrorHandler */



/*************************************<->*************************************
 *
 *  WmXIOErrorHandler (display)
 *
 *
 *  Description:
 *  -----------
 *  This function is the X IO error handler that is registered with X to
 *  handle X IO errors.  This function exits the window manager.
 *
 *
 *  Inputs:
 *  ------
 *  display = X display on which the X IO error occurred
 * 
 *************************************<->***********************************/

#ifdef _NO_PROTO
int
WmXIOErrorHandler (display)
    Display *display;

#else /* _NO_PROTO */
int
WmXIOErrorHandler (Display *display)
#endif /* _NO_PROTO */
{

    exit (WM_ERROR_EXIT_VALUE);

} /* END OF FUNCTIONS WmXIOErrorHandler */



/*************************************<->*************************************
 *
 *  WmXtErrorHandler (message)
 *
 *
 *  Description:
 *  -----------
 *  This function is registered as the X Toolkit error handler.
 *
 *
 *  Inputs:
 *  ------
 *  message = pointer to an error message
 * 
 *************************************<->***********************************/

#ifdef _NO_PROTO
void
WmXtErrorHandler (message)
    char * message;

#else /* _NO_PROTO */
void
WmXtErrorHandler (char *message)
#endif /* _NO_PROTO */
{
    exit (WM_ERROR_EXIT_VALUE);

} /* END OF FUNCTION WmXtErrorHandler */



/*************************************<->*************************************
 *
 *  WmXtWarningHandler (message)
 *
 *
 *  Description:
 *  -----------
 *  This function is registered as an X Toolkit warning handler.
 *
 *
 *  Inputs:
 *  ------
 *  message = pointer to a warning message
 * 
 *************************************<->***********************************/

#ifdef _NO_PROTO
void
WmXtWarningHandler (message)
    char * message;

#else /* _NO_PROTO */
void
WmXtWarningHandler (char *message)
#endif /* _NO_PROTO */
{


} /* END OF FUNCTIONS WmXtWarningHandler */


/*************************************<->*************************************
 *
 *  Warning (message)
 *
 *
 *  Description:
 *  -----------
 *  This function lists a message to stderr.
 *
 *
 *  Inputs:
 *  ------
 *  message = pointer to a message string
 * 
 *************************************<->***********************************/

#ifdef _NO_PROTO
void
Warning (message)
    char * message;

#else /* _NO_PROTO */
void
Warning (char *message)
#endif /* _NO_PROTO */
{
    fprintf (stderr, "%s: %s\n", wmGD.mwmName, message);
    fflush (stderr);

} /* END OF FUNCTION Warning */

#ifdef DEC_MOTIF_EXTENSION

/*************************************<->*************************************
 *
 *  WarningStr (message,string)
 *
 *
 *  Description:
 *  -----------
 *  This function lists a message and a string to stderr.
 *
 *
 *  Inputs:
 *  ------
 *  message = pointer to a message string
 *  string = pointer to an additional string
 * 
 *************************************<->***********************************/

#ifdef _NO_PROTO
void
WarningStr (message,string)
    char * message;
    char * string;
 
#else /* _NO_PROTO */
void
WarningStr (char *message, char *string)
#endif /* _NO_PROTO */
{
    fprintf (stderr, "%s: %s %s\n", wmGD.mwmName, message, string);
    fflush (stderr);

} /* END OF FUNCTION WarningStr */

#endif
