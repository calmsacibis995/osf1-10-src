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
#include "smresource.h"
#include "smconstants.h"
#define SS_NORMAL 1

void	dialog_action();
void	cancel_action();
void	apply_action();

int	create_sm_attrs()
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	This routine is called when the user selects Session from the
**	Session Manager customize menu.  It creates a modeless dialog
**	box which allows the user to set certain features of the session
**	manager.
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
static MrmRegisterArg reglist[] = {
        {"SmCancelCallback", (caddr_t) cancel_action},
        {"SmApplyCallback", (caddr_t) apply_action},
        {"SmOkCallback", (caddr_t) dialog_action},
        {"statebox_id", (caddr_t) &smsetup.state_id},
        {"statemapped_id", (caddr_t) &smsetup.mapped_id},
        {"stateicon_id", (caddr_t) &smsetup.icon_id},
        {"pausetext_id", (caddr_t) &smsetup.pause_id},
        {"endconfirm_id", (caddr_t) &smsetup.end_confirm_id},
        };

static int reglist_num = (sizeof reglist / sizeof reglist [0]);

MrmRegisterNames (reglist, reglist_num);

/* build the dialog using UIL */
MrmFetchWidget(s_DRMHierarchy, "CustomizeSession", smdata.toplevel,
                    &smsetup.sm_attr_id,
                    &drm_dummy_class);
return(1);
}

void	dialog_action(widget, tag, reason)
Widget	*widget;
caddr_t tag;
unsigned int reason;
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	The OK button was hit from the Session Customization dialog box.
**	We need to look at the widgets, determine what changed, make
**	the appropriate xlib calls, put the resources on the root window,
**	and unmanage the dialog box.
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
/* this call back can be called multiple times if the user hits the
    button quickly several times with either mouse, or CR.  Check
    to see if we have already unmanaged the widget before we do
    anything */
if (smsetup.managed == ismanaged)
    {
    smsetup.managed = notmanaged;
    XtUnmanageChild(XtParent(*widget));
    /* Get the current settings of widgets */
    sm_dialog_get_values();
    /* Update the resource database */
    sm_put_attrs();
    }
}

void	apply_action(widget, tag, reason)
Widget	*widget;
caddr_t tag;
unsigned int reason;
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      The APPLY button was hit from the Session Customization dialog box.
**      We need to look at the widgets, determine what changed, make
**      the appropriate xlib calls, put the resources on the root window.
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
/* Get the current settings of widgets */
sm_dialog_get_values();
/* Update the resource database */
sm_put_attrs();
}

void	cancel_action(widget, tag, reason)
Widget	*widget;
caddr_t	tag;
unsigned int reason;
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      The CANCEL button was hit from the Session Customization dialog box.
**      We need to unmanage the dialog box and reset the listbox
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
/* The user can hit the buttons several times before the dialog box is
   actually unmanaged.   Mark the first time through the callback so that
   we only free stuff once */
if (smsetup.managed == ismanaged)
    {
    smsetup.managed = notmanaged;
    XtUnmanageChild(XtParent(*widget));
    smsetup.changed = 0;
    }
}

int	sm_dialog_get_values()
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      The OK or APPLY button was hit.  Look at each widget and store
**      its value in the data structures IF it is different from the
**      current value.  Also mark the bit in the change mask if it
**      changes so that we will make the appropriate XLIB Calls.
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
unsigned int	i,j;

/* icon or window startup state */
i = XmToggleButtonGetState(smsetup.mapped_id);
if (i == 1)
    {
    if (smsetup.startup_state != mapped)
	{
	smsetup.startup_state = mapped;
	smsetup.changed = smsetup.changed | mstate;
        }
    }
else
    {
    if (smsetup.startup_state != iconified)
	{
	smsetup.startup_state = iconified;
	smsetup.changed = smsetup.changed | mstate;
        }
    }

/* confirmation on end session */
j = XmToggleButtonGetState(smsetup.end_confirm_id);
if (j != smsetup.end_confirm)
	    {
	    smsetup.end_confirm = j;
            smsetup.changed = smsetup.changed | mconfirm;
	    }
}

int	smattr_get_values()
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      This routine will get the resources from the resource database
**      for each item in the Session Customization.  We need to
**      set the values of the widgets when the dialog box is managed.
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
char	svalue[256];
int	value[4];
int	size;

/* startup state */
size = sm_get_any_resource(smstate, svalue, value);
if (strcmp(svalue, "iconified") == 0)
	{
	smsetup.startup_state = iconified;
	}
else
	{
	smsetup.startup_state = mapped;
	}

/* pause screen text */
size = sm_get_any_resource(smpausetext, svalue, value);
if ((smsetup.pause_text) != NULL)
    XtFree(smsetup.pause_text);
smsetup.pause_text = XtMalloc(strlen(svalue) + 1);
strcpy(smsetup.pause_text, svalue);

/* sm x and y */
size = sm_get_any_resource(smx, svalue, value);
smsetup.x = value[0];
size = sm_get_any_resource(smy, svalue, value);
smsetup.y = value[0];

/* end session confirmation box */
size = sm_get_any_resource(smconfirm, svalue, value);
if (size == 0)
    smsetup.end_confirm = 1;
else
    if (value[0] == 1)
	smsetup.end_confirm = 1;
    else
	smsetup.end_confirm = 0;
}

int	smattr_set_values()
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      When the dialog box is managed, we need to set all of the widgets
**      to the correct values based on the current settings of the
**      resources.
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
char	temp[256];
unsigned	int	status;


smsetup.changed = 0;

/* startup state */
if (smsetup.startup_state == mapped)
	{
	XmToggleButtonSetState(smsetup.mapped_id, 1, 0);
	XmToggleButtonSetState(smsetup.icon_id,0, 0);
	}
else
	{
	XmToggleButtonSetState(smsetup.mapped_id, 0, 0);
	XmToggleButtonSetState(smsetup.icon_id, 1, 0);
	}

/* pause text */
XmTextSetString(smsetup.pause_id, smsetup.pause_text);


if (smsetup.end_confirm == 0)
	{
	XmToggleButtonSetState(smsetup.end_confirm_id, 0, 0);
	}
else
	{
	XmToggleButtonSetState(smsetup.end_confirm_id, 1, 0);
	}
}

int	sm_put_attrs()
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      When Session Customization is changed, we need to write the
**      new values of the resources to the resource database.  This
**      routine will write each resource to the database.
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
char	*value;
unsigned	int	ivalue,status;
char	astring[10];	
Arg	arglist[4];

/* startup state */
if ((smsetup.changed & mstate) != 0)
	{
	if (smsetup.startup_state == mapped)
		{
		sm_put_resource(smstate, "mapped");
		}
	else
		{
		sm_put_resource(smstate, "iconified");
		}
	}

/* Pause text */
value = (char *)XmTextGetString(smsetup.pause_id);
if (smsetup.pause_text != 0)
    {
    if ((strcmp(value, smsetup.pause_text) != 0))
	{
	XtFree(smsetup.pause_text);
	smsetup.changed = smsetup.changed | mpausetext;
	}
    }
else
    {
    if (value != 0)
	smsetup.changed = smsetup.changed | mpausetext;
    }

if ((smsetup.changed & mpausetext) != 0)
    {
    sm_put_resource(smpausetext, value);
    smsetup.pause_text = XtMalloc(strlen(value) + 1);
    strcpy(smsetup.pause_text, value);
    if (smdata.pause_label != 0)
	{
	XtSetArg(arglist[0], XmNlabelString,
		 XmStringCreate(smsetup.pause_text, def_char_set));
	XtSetValues(smdata.pause_label, arglist, 1);
	}
    }
XtFree(value);

/* confirmation box on end session */
if ((smsetup.changed & mconfirm) != 0)
	{
	sm_put_int_resource(smconfirm, smsetup.end_confirm);
	}

if (smsetup.changed != 0)
	{
	sm_change_property(XtDisplay(smdata.toplevel));
	smdata.resource_changed = 1;
	smsetup.changed = 0;
	}
}

set_control_size()
{
Arg	arglist[10];
Position    x,y;
Dimension   width,height,border_width;

/* Get the current x and y of the dialog box */
XtSetArg (arglist[0], XmNx, &x);
XtSetArg (arglist[1], XmNy, &y);
XtGetValues (smdata.toplevel, arglist, 2);

if ((x != smsetup.x) || (y != smsetup.y))
    {
    XtSetArg (arglist[0], XmNx, smsetup.x);
    XtSetArg (arglist[1], XmNy, smsetup.y);
    XtSetValues (smdata.toplevel, arglist, 2);
    }

/* set the Pause label correctly */
if (smdata.pause_label != 0)
    {
    XtSetArg(arglist[0], XmNlabelString,
	     XmStringCreate(smsetup.pause_text, def_char_set));
    XtSetValues(smdata.pause_label, arglist, 1);
    }
}
