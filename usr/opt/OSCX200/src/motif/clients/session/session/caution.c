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
#include "smdata.h"
#include "smconstants.h"
#include <Xm/MessageB.h>


/*  display the error message in the error window */
int	caution_display(message_num)
unsigned    int	*message_num;
{
    XEvent	event;

    create_caution(message_num);
    smdata.fcaution = 0;

    /* wait until a button has been pressed to return 
     * if we return here right away, then we will continue
     * in end session code
     */

    while (smdata.fcaution == 0) {
	XtNextEvent(&event);
	XtDispatchEvent(&event);
    }
}


/* called when the ok button is hit from the error window.  Unmap the window
	Set the global variable which says the error has been acknowledged
*/
void	caution_ack(widget, tag, reason)
Widget	*widget;
caddr_t tag;
unsigned int *reason;
{
    switch(*reason) {
        case XmCR_OK:	  smdata.fcaution = yes;
			  break;
        case XmCR_HELP:   smdata.fcaution = no;
			  break;
        case XmCR_CANCEL: smdata.fcaution = cancel;
			  break;
    }
}   


/* create the error window.  */
int	create_caution(message_value)
unsigned    int	message_value;
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
**/
{
    Arg	arglist[5];
    int argcnt = 0;
    XtCallbackRec	cb_err_callback[2];
    Widget	*widget = NULL;

    if (message_value == end_session_number)
	widget = &smdata.end_caution_id;
    else
	widget = &smdata.caution_id;

    /* if not already created, do the widget creation */   
    if (((smdata.caution_id == 0) 
	 && (message_value == resource_changed_number))
	|| ((smdata.end_caution_id == 0) 
	     && (message_value == end_session_number)))
    {
        cb_err_callback[0].callback = (XtCallbackProc)caution_ack;
        cb_err_callback[0].closure = (XtPointer)message_value;
        cb_err_callback[1].callback = NULL;
        cb_err_callback[1].closure = NULL;
        XtSetArg(arglist[argcnt], XmNokCallback, cb_err_callback); argcnt++;
        XtSetArg(arglist[argcnt], XmNhelpCallback, cb_err_callback); argcnt++;
        XtSetArg(arglist[argcnt], XmNcancelCallback, cb_err_callback); argcnt++;

        if (message_value == end_session_number) {
	    MrmFetchWidgetOverride(s_DRMHierarchy, "end_quit_cb",
				   smdata.toplevel, "end_quit_cb",
				   arglist, argcnt, widget, &drm_dummy_class);
	    /* turn off the help (save) button - not needed */
	    if (XtIsManaged(XmMessageBoxGetChild(*widget,
						 XmDIALOG_HELP_BUTTON)))
    	        XtUnmanageChild(XmMessageBoxGetChild(*widget,
			    XmDIALOG_HELP_BUTTON));
        }
        else {
	    MrmFetchWidgetOverride(s_DRMHierarchy, "end_save_cb",
				   smdata.toplevel, "end_save_cb",
				   arglist, argcnt, widget, &drm_dummy_class);
	}
    }

    XtManageChild(*widget);
}
