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
#ifndef lint	/* BuildSystemHeader added automatically */
static char *BuildSystemHeader= "$Id: error.c,v 1.1.2.3 92/11/24 16:24:56 Russ_Kuhn Exp $";
#endif		/* BuildSystemHeader */
#include "smdata.h"
#include "smconstants.h"
#include <Xm/MessageB.h>

XtEventHandler DoErrorGrab(widget, client_data, event, cont)
Widget widget;
XtPointer client_data;
XEvent *event;
Boolean *cont;
{
    if ((widget == smdata.err_window_id) && (event->type == Expose)) {
        if (XtGrabKeyboard(smdata.err_window_id, False, GrabModeAsync,
			   GrabModeAsync, CurrentTime)
	     == GrabSuccess)
            XtRemoveEventHandler(smdata.err_window_id, ExposureMask, False,
		                 (XtEventHandler)DoErrorGrab, NULL);
    }
}

/*  display the error message in the error window */
int	error_display(message)
XmString message;
{
    Arg arglist[5];
    Position dix=0, diy=0;
    int argcnt;

    smdata.err_ack = 0;
    if (smdata.err_window_id == 0) create_error(message);
    else {
	argcnt = 0;
	XtSetArg(arglist[argcnt], XmNmessageString, message); argcnt++;
	XtSetArg(arglist[argcnt], XmNmappedWhenManaged, False); argcnt++;
        XtSetValues(smdata.err_window_id, arglist, argcnt);
    }

    /* turn off the buttons we don't need */
    if (XtIsManaged(XmMessageBoxGetChild(smdata.err_window_id,
		  		         XmDIALOG_HELP_BUTTON)))
	XtUnmanageChild(XmMessageBoxGetChild(smdata.err_window_id,
		  		             XmDIALOG_HELP_BUTTON));
    if (XtIsManaged(XmMessageBoxGetChild(smdata.err_window_id,
					 XmDIALOG_CANCEL_BUTTON)))
        XtUnmanageChild(XmMessageBoxGetChild(smdata.err_window_id,
		    			     XmDIALOG_CANCEL_BUTTON));

    /* force the size calculations */
    XtManageChild(smdata.err_window_id);
    get_center_coor(XtDisplay(smdata.err_window_id), (Widget)0,
		    smdata.err_window_id, &dix, &diy);
    argcnt = 0;
    XtSetArg (arglist[argcnt], XmNx, dix); argcnt++;
    XtSetArg (arglist[argcnt], XmNy, diy); argcnt++;
    XtSetArg(arglist[argcnt], XmNmappedWhenManaged, True); argcnt++;
    XtSetArg(arglist[argcnt], XmNdefaultButtonType, XmDIALOG_OK_BUTTON);
    argcnt++;
    XtSetValues(smdata.err_window_id, arglist, argcnt);
}


/*  display the error message in the pause error window */
int	pause_error_display(message)
XmString message;
{
    Arg arglist[6];
    Position dix=0, diy=0;
    int argcnt;

    smdata.err_ack = 0;
    if (smdata.err_window_id == 0) create_error(message);
    else {
	argcnt = 0;
	XtSetArg(arglist[argcnt], XmNmessageString, message); argcnt++;
	XtSetArg(arglist[argcnt], XmNmappedWhenManaged, False); argcnt++;
        XtSetValues(smdata.err_window_id, arglist, argcnt);
    }

    /* Turn off parent DialogShell's window manager decorations */
      {
	Arg wargs[2];
	XtSetArg (wargs[0], XtNoverrideRedirect, TRUE);
	XtSetValues(XtParent(smdata.err_window_id), wargs, 1);
      }
    
    /* turn off the buttons we don't need */
    if (XtIsManaged(XmMessageBoxGetChild(smdata.err_window_id,
		  		         XmDIALOG_HELP_BUTTON)))
	XtUnmanageChild(XmMessageBoxGetChild(smdata.err_window_id,
		  		             XmDIALOG_HELP_BUTTON));
    if (XtIsManaged(XmMessageBoxGetChild(smdata.err_window_id,
					 XmDIALOG_CANCEL_BUTTON)))
        XtUnmanageChild(XmMessageBoxGetChild(smdata.err_window_id,
		    			     XmDIALOG_CANCEL_BUTTON));

    XtAddEventHandler(smdata.err_window_id, ExposureMask, False,
		      (XtEventHandler)DoErrorGrab, NULL);

    /* force the size calculations */
    XtManageChild(smdata.err_window_id);
    get_center_coor(XtDisplay(smdata.err_window_id), (Widget)0,
		    smdata.err_window_id, &dix, &diy);
    argcnt = 0;
    XtSetArg (arglist[argcnt], XmNx, dix); argcnt++;
    XtSetArg (arglist[argcnt], XmNy, diy); argcnt++;
    XtSetArg(arglist[argcnt], XmNmappedWhenManaged, True); argcnt++;
    XtSetArg(arglist[argcnt], XmNdefaultButtonType, XmDIALOG_OK_BUTTON);
    argcnt++;
    XtSetValues(smdata.err_window_id, arglist, argcnt);
}

/* called when the ok button is hit from the error window.  Unmap the window
 * Set the global variable which says the error has been acknowledged
 */
void	restart(widget, tag, reason)
Widget	*widget;
caddr_t tag;
unsigned int reason;
{
    smdata.err_ack = 1;
    XtUnmanageChild(smdata.err_window_id);
    if (return_focus != 0) {
        XtUngrabKeyboard(smdata.err_window_id, CurrentTime);
    	XtGrabKeyboard(return_focus, False,
		       GrabModeAsync, GrabModeAsync, CurrentTime);
        return_focus = 0;
    }
}


/* create the error window.  */
int	create_error(message)
XmString message;
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
**/
{
    Arg	arglist[8];
    int 	argcnt = 0;
    XtCallbackRec	cb_err_callback[2];

    cb_err_callback[0].callback = (XtCallbackProc) restart;
    cb_err_callback[0].closure = NULL;
    cb_err_callback[1].callback = NULL;
    cb_err_callback[1].closure = NULL;

    XtSetArg(arglist[argcnt], XmNokCallback, cb_err_callback); argcnt++;
    XtSetArg(arglist[argcnt], XmNmappedWhenManaged, False); argcnt++;
    XtSetArg(arglist[argcnt], XmNmessageString, message); argcnt++;
    XtSetArg(arglist[argcnt], XmNdialogStyle, XmDIALOG_APPLICATION_MODAL);
    argcnt++;

    MrmFetchWidgetOverride(s_DRMHierarchy, "error_mb", smdata.toplevel,
        "error_mb", arglist, argcnt, &smdata.err_window_id, &drm_dummy_class);
}
