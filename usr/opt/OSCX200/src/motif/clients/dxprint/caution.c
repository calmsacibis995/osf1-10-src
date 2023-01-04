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
**
**++
** FACILITY:  Session
**
** ABSTRACT:
**
**	This module handles the caution boxes displayed at the end of a
**	session.
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

/*
** Include files
*/
#include "iprdw.h"
#include "smdata.h"
#include "smconstants.h"
#include "prdw_entry.h"

int decw$caution_ack
#if _PRDW_PROTO_
(
    Widget	widget,
    unsigned int tag,
    XmAnyCallbackStruct *any
)
#else
(widget, tag, any)
    Widget	widget;
    unsigned int tag;
    XmAnyCallbackStruct *any;
#endif
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	Callback from the end session caution boxes.  Depending on which
**	box we had up and the callback reason, we may need to save
**	the current resources, end the session, or simply return.
**
**  FORMAL PARAMETERS:
**
**	widget - the caution box
**	tag - which caution box was up - resource changed or end session
**	      confirmation
**	any - the button selected by the user
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
    /* They selected the YES button */
    if (any->reason == (int)XmCR_OK)
    {
	/* If the question was saving resources, do it */
	if (tag == resource_changed_number)
	{
	    sm_save_database(XtDisplay(widget));
	}
	/* This will end the session */
	finish();
    }	
    else
    {
	/* If the answer was no */
	if (any->reason == (int)XmCR_CANCEL)
	{
	    /* If the question was saving resources - still end the session */
	    if (tag == resource_changed_number) finish();
	}
	else
	/* In all other cases simply continue the session */
	{
	    XtUnmanageChild(widget);
	    return;
	}
    }
    /* In all other cases simply continue the session */
    return;
}
	
/* create the error window.  */

int decw$create_caution
#if _PRDW_PROTO_
(
    unsigned int	message_value
)
#else
(message_value)
    unsigned int	message_value;
#endif
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	This routine handles two kinds of confirmation boxes.  One with
**	two buttons which asks if the user really wants to end the
**	session, and one with three buttons which allows the user to
**	1. end the session and save customization settings, 2. end the
**	session and not save customization settings, 3. cancel the end
**	session.   Since confirmation boxes cannot have the number of
**	buttons modified after creation, we need to create a seperate
**	widget for each type.  If we have already created the box, just
**	manage it.
**
**  FORMAL PARAMETERS:
**
**	message_value - If equal to the constant resource_changed then
**		        put up the 3 button type, else put up the two
**		        button type
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
Arg	arglist[5];
XtCallbackRec	cb_err_callback[2];
Widget	*widget;

if (message_value == end_session_number)
	widget = &smdata.end_caution_id;
else
	widget = &smdata.caution_id;

/* if not already created, do the widget creation */   
if (((smdata.caution_id == 0) && (message_value == resource_changed_number))|| 
    ((smdata.end_caution_id == 0) && (message_value == end_session_number)))
    {
    /* In any case here is the callback.  Pass in message_value as
       parameter */
    cb_err_callback[0].callback = (XtCallbackProc)decw$caution_ack;
    cb_err_callback[0].closure = (XtPointer)message_value;
    cb_err_callback[1].callback = NULL;
    cb_err_callback[1].closure = NULL;
    XtSetArg (arglist[0], XmNokCallback, cb_err_callback);
    XtSetArg (arglist[1], XmNcancelCallback, cb_err_callback);
    XtSetArg (arglist[2], XmNhelpCallback, cb_err_callback);
    if (message_value == end_session_number)
	{
	/* create a modal message box  */
	MrmFetchWidgetOverride(s_DRMHierarchy, "end_quit_cb", smdata.toplevel,
		"end_quit_cb", arglist, 3, widget,
		&drm_dummy_class);
	}
    else
	{
	MrmFetchWidgetOverride(s_DRMHierarchy, "end_save_cb", smdata.toplevel,
		"end_save_cb", arglist, 3, widget, 
		&drm_dummy_class);
	}
    }
/* map the window */
XtManageChild(*widget);
}
