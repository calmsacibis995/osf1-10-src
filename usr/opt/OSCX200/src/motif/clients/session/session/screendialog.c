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
static char *BuildSystemHeader= "$Header: /usr/sde/osf1/rcs/x11/src/motif/clients/session/session/screendialog.c,v 1.1.4.2 1993/06/25 18:40:10 Paul_Henderson Exp $";	/* BuildSystemHeader */
#endif		/* BuildSystemHeader */
#include "smdata.h"
#include "smresource.h"
#include <Xm/ToggleB.h>


void	screen_action();
void	screen_cancel();
void	confirm_screen_action();
void	confirm_screen_cancel();
void	screen_apply();
void    set_screen_data_screennum();


int	create_screen_attrs()
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	This routine is called when the user selects Screen Number from the
**	Session Manager customize menu.  It creates a modeless dialog
**	box which allows the user to set certain defaults which will
**	be used when they select the PrintScreen or Create Application
**	menu items
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
Arg	arglist[10];
static Widget label1,appl_scale_id, prt_scale_id;
int screennum,i;

static MrmRegisterArg reglist[] = {
        {"ScreenCancelCallback", (caddr_t) screen_cancel},
        {"ScreenApplyCallback", (caddr_t) screen_apply},
        {"ScreenOkCallback", (caddr_t) screen_action},
        {"scrapplprompt_id", (caddr_t) &screensetup.appl_prompt_id},
#ifdef DOPRINT
        {"scrprtprompt_id", (caddr_t) &screensetup.prt_prompt_id},
#endif
        {"scrapplscale_id", (caddr_t) &appl_scale_id},
#ifdef DOPRINT
        {"scrprtscale_id", (caddr_t) &prt_scale_id},
#endif
};

static int reglist_num = (sizeof reglist / sizeof reglist [0]);

    MrmRegisterNames (reglist, reglist_num);

    /* build the dialog using UIL */
    MrmFetchWidget(s_DRMHierarchy, "CustomizeScreen", smdata.toplevel,
                    &screensetup.scr_attr_id,
                    &drm_dummy_class);

    /* create the screen number toggle buttons to put inside the radio box */
    screennum = ScreenCount(display_id);
    for (i=0; i<screennum; i++) {
        char    number[20];

        if (i == 0) {
	    screensetup.appl_scale_id =
	      (Widget *)XtMalloc(screennum * sizeof(Widget));
#ifdef DOPRINT
	    screensetup.prt_scale_id =
	      (Widget *)XtMalloc(screennum * sizeof(Widget));
#endif
	}
        int_to_str(i, number, 20);
        XtSetArg(arglist[0], XmNlabelString,
	         XmStringCreate(number, def_char_set));
        screensetup.appl_scale_id[i]
		= XtCreateWidget(number, xmToggleButtonWidgetClass,
				 appl_scale_id, arglist, 1);
#ifdef DOPRINT
        screensetup.prt_scale_id[i]
		= XtCreateWidget(number, xmToggleButtonWidgetClass,
				prt_scale_id, arglist, 1);
        XtManageChild(screensetup.prt_scale_id[i]);
#endif

        XtManageChild(screensetup.appl_scale_id[i]);
    }
    return(1);
}

void	screen_action(widget, tag, reason)
Widget	*widget;
caddr_t tag;
unsigned int reason;
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	The OK button was hit from the Screen Customization dialog box.
**	We need to look at the widgets, determine what changed, 
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
    if (screensetup.managed == ismanaged) {
        screensetup.managed = notmanaged;
        XtUnmanageChild(XtParent(*widget));

        screen_dialog_get_values();
        screen_put_attrs();
    }
}

void	screen_apply(widget, tag, reason)
Widget	*widget;
caddr_t tag;
unsigned int reason;
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      The APPLY button was hit from the PrintScreen Customization dialog box.
**      We need to look at the widgets, determine what changed.
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
    screen_dialog_get_values();
    screen_put_attrs();
}

void	screen_cancel(widget, tag, reason)
Widget	*widget;
caddr_t	tag;
unsigned int reason;
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      The CANCEL button was hit from the PrintScreen Customization dialog box.
**      We need to unmanage the dialog box.
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
    if (screensetup.managed == ismanaged) {
        screensetup.managed = notmanaged;
        XtUnmanageChild(XtParent(*widget));
        screensetup.changed = 0;
    }
}


int	screen_dialog_get_values()
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      The OK or APPLY button was hit.  Look at each widget and store
**      its value in the data structures IF it is different from the
**      current value.  Also mark the bit in the change mask if it
**      changes.
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
unsigned    int	i,j;

    /* get value of application screen prompt*/
    i = XmToggleButtonGetState(screensetup.appl_prompt_id);

    if (i != screensetup.appl_prompt) {
        screensetup.changed = screensetup.changed | mscreen_applprompt;
        screensetup.appl_prompt = i; 
    }

#ifdef DOPRINT
    /* get value of printscreen screen prompt*/
    i = XmToggleButtonGetState(screensetup.prt_prompt_id);

    if (i != screensetup.prt_prompt) {
        screensetup.changed = screensetup.changed | mscreen_prtprompt;
        screensetup.prt_prompt = i; 
    }
#endif

    for (i=0; i<ScreenCount(display_id); i++) {
        /* default application startup screen*/
        j = XmToggleButtonGetState(screensetup.appl_scale_id[i]);
        if (j == 1) {
	    if (i != screensetup.appl_screennum) {
	        screensetup.changed
			= screensetup.changed | mscreen_applscreennum;
	        screensetup.appl_screennum = i; 
	    }
	}

#ifdef DOPRINT
        /* default print screen*/
        j = XmToggleButtonGetState(screensetup.prt_scale_id[i]);
        if (j == 1) {
	    if (i != screensetup.prt_screennum) {
	        screensetup.changed
			= screensetup.changed | mscreen_prtscreennum;
	        screensetup.prt_screennum = i; 
	    }
	}
#endif
    }
}

int	screenattr_get_values()
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      This routine will get the resources from the resource database
**      for each item in the Screen Customization.  We need to
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

    /* application default screen number*/
    size = sm_get_any_resource(smscreennum, svalue, value);
    if ((value[0] == -1) || (size == 0))
        screensetup.appl_screennum = XDefaultScreen(display_id);
    else screensetup.appl_screennum = value[0];

#ifdef DOPRINT
    /* print default screen number*/
    size = sm_get_any_resource(pscreennum, svalue, value);
    if ((value[0] == -1) || (size == 0))
        screensetup.prt_screennum = XDefaultScreen(display_id);
    else screensetup.prt_screennum = value[0];
#endif

    /* application prompt screen number*/
    size = sm_get_any_resource(smscreenprompt, svalue, value);
    if (size == 0) screensetup.appl_prompt = 0;
    else screensetup.appl_prompt = value[0];

#ifdef DOPRINT
    /* print prompt screen number*/
    size = sm_get_any_resource(pscreenprompt, svalue, value);
    if (size == 0) screensetup.prt_prompt = 0;
    else screensetup.prt_prompt = value[0];
#endif
}

int	screenattr_set_values()
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
unsigned	int	status,numscreens,i;

    screensetup.changed = 0;

    /* appl prompt*/
    if (screensetup.appl_prompt == 1)
	XmToggleButtonSetState(screensetup.appl_prompt_id, 1, 0);
    else XmToggleButtonSetState(screensetup.appl_prompt_id, 0, 1);

#ifdef DOPRINT
    /* prt prompt*/
    if (screensetup.prt_prompt == 1)
	XmToggleButtonSetState(screensetup.prt_prompt_id, 1, 0);
    else XmToggleButtonSetState(screensetup.prt_prompt_id, 0, 1);
#endif

    for (i=0; i<ScreenCount(display_id); i++) {
        if (i == screensetup.appl_screennum)
	    XmToggleButtonSetState(screensetup.appl_scale_id[i], 1, 0);
        else
	    XmToggleButtonSetState(screensetup.appl_scale_id[i], 0, 1);

#ifdef DOPRINT
        if (i == screensetup.prt_screennum)
	    XmToggleButtonSetState(screensetup.prt_scale_id[i], 1, 0);
        else
	    XmToggleButtonSetState(screensetup.prt_scale_id[i], 0, 1);
#endif
    }
}


int	screen_put_attrs()
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      When Customization is changed, we need to write the
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

    /* appl prompt */
    if ((screensetup.changed & mscreen_applprompt) != 0)
	sm_put_int_resource(smscreenprompt, screensetup.appl_prompt);

#ifdef DOPRINT
    /* prt prompt */
    if ((screensetup.changed & mscreen_prtprompt) != 0)
	sm_put_int_resource(pscreenprompt, screensetup.prt_prompt);
#endif

    /* appl screennum*/
    if ((screensetup.changed & mscreen_applscreennum) != 0)
	sm_put_int_resource(smscreennum, screensetup.appl_screennum);

#ifdef DOPRINT
    /* print screennum*/
    if ((screensetup.changed & mscreen_prtscreennum) != 0)
	sm_put_int_resource(pscreennum, screensetup.prt_screennum);
#endif

    if (screensetup.changed != 0) {
	sm_change_property(XtDisplay(smdata.toplevel));
	smdata.resource_changed = 1;
	screensetup.changed = 0;
    }
}

int create_screen_confirm()
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	Create a modal confirmation box for the screen number
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
Arg	arglist[10];
static Widget	rb_id;
int	i, screennum;

static MrmRegisterArg reglist[] = {
        {"ConfirmScrOkCallback", (caddr_t) confirm_screen_action},
        {"ConfirmScrCancelCallback", (caddr_t) confirm_screen_cancel},
	{"confirmprtbox_id", (caddr_t) &rb_id},
};

static int reglist_num = (sizeof reglist / sizeof reglist [0]);

    MrmRegisterNames (reglist, reglist_num);

/* build the dialog using UIL */
    MrmFetchWidget(s_DRMHierarchy, "ConfirmScreen", smdata.toplevel,
                    &smdata.screen_confirm_box_id,
                    &drm_dummy_class);

/* create the screen number toggle buttons to put inside the radio box */
    screennum = ScreenCount(display_id);
    for (i=0; i<screennum; i++) {
        char    number[20];

        if (i == 0)
	    smdata.screen_confirm_list =
	      (Widget *) XtMalloc(screennum * sizeof(Widget));

        int_to_str(i, number, 20);
        XtSetArg(arglist[0], XmNlabelString,
	         XmStringCreate(number, def_char_set));
        smdata.screen_confirm_list[i]
	    = XtCreateWidget(number, xmToggleButtonWidgetClass,
			     rb_id, arglist, 1);
        XtManageChild(smdata.screen_confirm_list[i]);
    }

    return(1);
}

void	confirm_screen_action(widget, tag, reason)
Widget	*widget;
int	*tag;
unsigned int reason;
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	Get the screen number - The user hit OK from the confirm box
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
    int i,j;

    for (i=0; i<ScreenCount(display_id); i++) {
           /* default application startup screen*/
        j = XmToggleButtonGetState(smdata.screen_confirm_list[i]);
        if (j == 1) {
	  /* for some odd reason, the new compiler screws up on
	     this.  Hence we resort to a function call to set the
	     screennum in the struct...

	    *smdata.screen_data.screennum = i;
	    
	  */

	    set_screen_data_screennum(&i);
	    break;
	}
    }
    
    XtUnmanageChild(XtParent(*widget));
    XFlush(display_id);
#ifdef DOPRINT
    if (smdata.screen_data.operation == prt_operation)
        send_print_message(*smdata.screen_data.the_widget);
    else
        do_create(*smdata.screen_data.the_widget,*smdata.screen_data.screennum);
#else
    do_create(*smdata.screen_data.the_widget,*smdata.screen_data.screennum);
#endif
}

void	confirm_screen_cancel(widget, tag, reason)
Widget	*widget;
caddr_t	tag;
unsigned int reason;
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	The user canceled the screen operation.  Just return.
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
    XtUnmanageChild(XtParent(*widget));
}

set_correct_button(screennum)
int screennum;
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	Sets the correct toggle button on for the screen prompt box.
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
    int i;

    for (i=0; i<ScreenCount(display_id); i++) {
        if (i == screennum)
	    XmToggleButtonSetState(smdata.screen_confirm_list[i], 1, 1);
        else
	    XmToggleButtonSetState(smdata.screen_confirm_list[i], 0, 1);
    }
}


set_screen_data(screennum, operation, the_widget)
int *screennum;
int operation;
Widget	*the_widget;
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	Sets the global data to be used by the print screen and
**	create application routines.  They need to know the operation
**	selected and the screen number.
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
    smdata.screen_data.screennum = screennum;
    smdata.screen_data.operation = operation;
    smdata.screen_data.the_widget = the_widget;
}



void set_screen_data_screennum(screennum)
int *screennum;
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	Sets the global data to be used by the print screen and
**	create application routines.  They need to know the operation
**	selected and the screen number.
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
    smdata.screen_data.screennum = screennum;

}



