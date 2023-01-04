/*
*****************************************************************************
**									    *
**  COPYRIGHT (c) 1978, 1980, 1982, 1984, 1988, 1989, 1991, 1992 BY	    *
**  DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASSACHUSETTS.		    *
**  ALL RIGHTS RESERVED.						    *
** 									    *
**  THIS SOFTWARE IS FURNISHED UNDER A LICENSE AND MAY BE USED AND COPIED   *
**  ONLY IN  ACCORDANCE WITH  THE  TERMS  OF  SUCH  LICENSE  AND WITH THE   *
**  INCLUSION OF THE ABOVE COPYRIGHT NOTICE. THIS SOFTWARE OR  ANY  OTHER   *
**  COPIES THEREOF MAY NOT BE PROVIDED OR OTHERWISE MADE AVAILABLE TO ANY   *
**  OTHER PERSON.  NO TITLE TO AND OWNERSHIP OF  THE  SOFTWARE IS  HEREBY   *
**  TRANSFERRED.							    *
** 									    *
**  THE INFORMATION IN THIS SOFTWARE IS  SUBJECT TO CHANGE WITHOUT NOTICE   *
**  AND  SHOULD  NOT  BE  CONSTRUED AS  A COMMITMENT BY DIGITAL EQUIPMENT   *
**  CORPORATION.							    *
** 									    *
**  DIGITAL ASSUMES NO RESPONSIBILITY FOR THE USE  OR  RELIABILITY OF ITS   *
**  SOFTWARE ON EQUIPMENT WHICH IS NOT SUPPLIED BY DIGITAL.		    *
** 									    *
**									    *
*****************************************************************************
**++
** FACILITY:  Session
**
** ABSTRACT:
**
**	This module handles the interactive errors for a decwindows login.
**
** ENVIRONMENT:
**
**	VAX/VMS operating system.
**
** AUTHOR:  Karen Brouillette September 1987
**
** Modified by:
**
**	04-Apr-1991	Edward P Luwish
**		Port to Motif UI
*/

#ifndef lint	/* BuildSystemHeader added automatically */
static char *BuildSystemHeader= "$Header: /b5/aguws3.0/aguws3.0_rcs/src/dec/clients/print/error.c,v 1.2 91/12/30 12:48:20 devbld Exp $";	/* BuildSystemHeader */
#endif		/* BuildSystemHeader */

/*
** Include files
*/
#include "iprdw.h"
#include "smdata.h"
#include "smconstants.h"
#include "prdw_entry.h"


/* create the error window.  */

int	decw$create_error()
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	Create an error message dialog box.  This box is modal.
**
**  FORMAL PARAMETERS:
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  COMPLETION CODES:
**
**  SIDE EFFECTS:
**
**--
*/
{
Arg	arglist[2];
XtCallbackRec	cb_err_callback[2];

/* call decw$restart when the OK button is pressed to acknowledge
   the error */
cb_err_callback[0].callback = (XtCallbackProc)decw$restart;
cb_err_callback[0].closure = NULL;
cb_err_callback[1].callback = NULL;
cb_err_callback[1].closure = NULL;
XtSetArg (arglist[0], XmNokCallback, cb_err_callback);

/* create a modal message box  */
MrmFetchWidgetOverride(s_DRMHierarchy, "error_mb", smdata.toplevel,
        "error_mb", arglist, 1, &smdata.err_window_id, &drm_dummy_class);
}

int decw$error_display
#if _PRDW_PROTO_
(
    XmString	message
)
#else
(message)
    XmString	message;
#endif
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	Display an error message in a modal error message box
**
**  FORMAL PARAMETERS:
**
**	message - A pointer to a compound string to be displayed
**		  in the message box.
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  COMPLETION CODES:
**
**  SIDE EFFECTS:
**
**--
**/
{
Arg arglist[2];
XEvent	xev;
char    *dummy;

/* Create the box if it is not already created */
if (smdata.err_window_id == 0)
	decw$create_error();

/* set the value of the message box */
XtSetArg(arglist[0], XmNmessageString, message);
XtSetValues(smdata.err_window_id, arglist, 1);

/* map the window */
XtManageChild(smdata.err_window_id);
}

int decw$restart
#if _PRDW_PROTO_
(
    Widget		*widget,
    caddr_t		tag,
    XmAnyCallbackStruct	*any
)
#else
(widget, tag, any)
    Widget		*widget;
    caddr_t		tag;
    XmAnyCallbackStruct *any;
#endif
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	Called when the user hits the acknowledged button from the
**	modal error message box.  Unmap the window and perhaps set
**	focus back to the dialog box which had it before
**
**  FORMAL PARAMETERS:
**
**  IMPLICIT INPUTS:
**
**	return_focus - widget id of a window which had the input focus
**		       before the error box was mapped.
**
**  IMPLICIT OUTPUTS:
**
**  COMPLETION CODES:
**
**  SIDE EFFECTS:
**
**	Should be using some other time than currenttime here.  If the
**	user sets focus somewhere else, then acknowledges the error then
**	we will be setting the focus back to a place which is not correct.
**
**--
**/
{
Time	time = CurrentTime;

/* now unmap the error window */
XtUnmanageChild(smdata.err_window_id);

/* return input focus to callers dialog */
if (return_focus != 0)
    {
    /* There are problems if you return focus to a window which is not 
       managed */
    if (XtIsManaged(return_focus))
	{
	/* it is visible, pop it to the top of the screen */
	XRaiseWindow(display_id, 
	             XtWindow(XtParent(return_focus)));
	(* XtCoreProc (return_focus, accept_focus))
		      (return_focus, &time);
	}
    return_focus = 0;
    }

return;
}
