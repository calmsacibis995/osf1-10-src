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
#define SS_NORMAL 1
#include "smconstants.h"
#include <Xm/Text.h>
/* #include "smpatterns.h" */

void	window_action();
void	window_cancel();
void	window_apply();
void	radio_select();
void	radio_select_wmexe();

extern void Set_Sample();
extern void Set_Sample_Pattern();
extern void Clicked_On_Pattern();
extern void Refresh_Pattern_Window();
extern void color_select();

extern OptionsRec options;

int	create_window_attrs()
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      This routine is called when the user selects Window from the
**      Session Manager customize menu.  It creates a modeless dialog
**      box which allows the user to set certain features of the windows
**	on the screen.
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
        {"WindowCancelCallback", (caddr_t) window_cancel},
        {"WindowApplyCallback", (caddr_t) window_apply},
        {"WindowOkCallback", (caddr_t) window_action},
        {"saveronoff_id", (caddr_t) &windowsetup.saveronoff_id},
        {"saveron_id", (caddr_t) &windowsetup.saverenable_id},
        {"saveroff_id", (caddr_t) &windowsetup.saverdisable_id},
        {"saverscale_id", (caddr_t) &windowsetup.saver_id},
        {"winmgricon_id", (caddr_t) &windowsetup.icon_id},
        {"winmgrsmall_id", (caddr_t) &windowsetup.icon_small_id},
        {"winmgrlarge_id", (caddr_t) &windowsetup.icon_large_id},
        {"winmgrexe_id", (caddr_t) &windowsetup.wmexe_id},
        {"winmgrdefault_id", (caddr_t) &windowsetup.wmexe_default_id},
        {"winmgrother_id", (caddr_t) &windowsetup.wmexe_other_id},
        {"winmgrothertext_id", (caddr_t) &windowsetup.wmother_text_id},
	{"patterncurrent_id", (caddr_t) &windowsetup.patterncurrent_id},
	{"patternfg_id", (caddr_t) &windowsetup.patternfg_id},
	{"patternbg_id", (caddr_t) &windowsetup.patternbg_id},
	{"patterndefault_id", (caddr_t) &windowsetup.patterndefault_id},
	{"palette_id", (caddr_t) &windowsetup.palette_id},
	{"bwscreenfg_id",
	   (caddr_t)&windowsetup.screen_foreground.widget_list[0]},
	{"bwwindowfg_id", (caddr_t) &windowsetup.foreground.widget_list[0]},
	{"clscreenfg_id",
	   (caddr_t) &windowsetup.screen_foreground.widget_list[0]},
	{"clscreenbg_id",
	   (caddr_t) &windowsetup.screen_background.widget_list[0]},
	{"clwindowfg_id", (caddr_t) &windowsetup.foreground.widget_list[0]},
	{"clwindowbg_id", (caddr_t) &windowsetup.background.widget_list[0]},
	{"clhighlight_id",(caddr_t)  &windowsetup.activetitle.widget_list[0]},
	{"clborder_id", (caddr_t) &windowsetup.inactivetitle.widget_list[0]},
	{"BWScreenCallback", (caddr_t) radio_select},
	{"WMDefaultCallback", (caddr_t) radio_select_wmexe},
	{"PatternCurrentCallback", (caddr_t) Set_Sample},
	{"PatternFgCallback", (caddr_t) Set_Sample_Pattern},
	{"PatternBgCallback", (caddr_t) Set_Sample_Pattern},
	{"PatternDefaultCallback", (caddr_t) Set_Sample_Pattern},
	{"bwblack_id", (caddr_t) &windowsetup.foreground.widget_list[1]},
	{"bwwhite_id", (caddr_t) &windowsetup.foreground.widget_list[2]},
	{"bwscreenblack_id",
	   (caddr_t) &windowsetup.screen_foreground.widget_list[1]},
	{"bwscreenwhite_id",
	   (caddr_t) &windowsetup.screen_foreground.widget_list[2]},
/*
	{"Clicked_On_Pattern", (caddr_t) Clicked_On_Pattern},
	{"Refresh_Pattern_Window", (caddr_t) Refresh_Pattern_Window}
*/
	{"ColorSelectCallback", (caddr_t) color_select},
        };

static int reglist_num = (sizeof reglist / sizeof reglist [0]);

MrmRegisterNames (reglist, reglist_num);

/* if this is a color system, create the widget for colors, etc. */
if (system_color_type != black_white_system)
    {
    MrmFetchWidget(s_DRMHierarchy, "CustomizeColorWindow", smdata.toplevel,
                    &windowsetup.window_attr_id,
                    &drm_dummy_class);
#ifdef XUI
    windowsetup.inactivetitle.mix_changed = 0;
#endif
    windowsetup.activetitle.mix_changed = 0;
    }
else
    {
    MrmFetchWidget(s_DRMHierarchy, "CustomizeBWWindow", smdata.toplevel,
                    &windowsetup.window_attr_id,
                    &drm_dummy_class);
    }

Create_Pattern_Dialog();

return(1);
}

void	window_action(widget, tag, reason)
Widget	*widget;
caddr_t tag;
unsigned int reason;
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      The OK button was hit from the Window Customization dialog box.
**      We need to look at the widgets, determine what changed, make
**      the appropriate xlib calls, put the resources on the root window,
**      and unmanage the dialog box.
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
if (windowsetup.managed == ismanaged)
    {
    windowsetup.managed = notmanaged;

    /* Get the settings of the widgets */
    window_dialog_get_values();

    /* Put the new property on the root window, and exeucte XLIB calls */
    window_put_attrs();

    XtUnmanageChild(XtParent(*widget));

    /* Free the colors we possibly allocated for this dialog box */
    free_allocated_color(&windowsetup.screen_foreground);
    free_allocated_color(&windowsetup.screen_background);
    free_allocated_color(&windowsetup.foreground);
    free_allocated_color(&windowsetup.background);
    free_allocated_color(&windowsetup.activetitle);
#ifdef XUI
    free_allocated_color(&windowsetup.inactivetitle);
#endif
    }
}

void	window_apply(widget, tag, reason)
Widget	*widget;
caddr_t tag;
unsigned int reason;
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      The APPLY button was hit from the Window Customization dialog box.
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
/* Get the settings of the widgets */
window_dialog_get_values();

/* Put the new property on the root window, and exeucte XLIB calls */
window_put_attrs();
}

void	window_cancel(widget, tag, reason)
Widget	*widget;
caddr_t	tag;
unsigned int reason;
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      The CANCEL button was hit from the Window Customization dialog box.
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
if (windowsetup.managed == ismanaged)
    {
    windowsetup.managed = notmanaged;
    windowsetup.changed = 0;
    XtUnmanageChild(XtParent(*widget));

    /* Free the colors that we allocated for pointer fore/back ground */
    free_allocated_color(&windowsetup.screen_foreground);
    free_allocated_color(&windowsetup.screen_background);
    free_allocated_color(&windowsetup.foreground);
    free_allocated_color(&windowsetup.background);
    free_allocated_color(&windowsetup.activetitle);
#ifdef XUI
    free_allocated_color(&windowsetup.inactivetitle);
#endif
    }
}

int	window_dialog_get_values()
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
unsigned    int	i,j,k,found,changed;

/* get value of the screen saver on off switch */
i = XmToggleButtonGetState(windowsetup.saverenable_id);
if (i != windowsetup.saver)
    {
    windowsetup.saver = i;
    windowsetup.changed = windowsetup.changed | mdissaver;
    }

/* get value of screen saver seconds slider control */
XmScaleGetValue(windowsetup.saver_id, &i);
if (i != windowsetup.saver_seconds)
    {
    windowsetup.saver_seconds = i;
    windowsetup.changed = windowsetup.changed | mdissaverseconds;
    }

/* get value of window color */
i = get_bwcolor_widget_state(&windowsetup.screen_foreground, 
	&windowsetup.screen_background, 
	&windowsetup.changed, mdisforeground, mdisbackground);


/* get the index for the window pattern */
if (windowsetup.pattern_selected != -1)
    {
    if (windowsetup.pattern_index != windowsetup.pattern_selected)
		{
		windowsetup.pattern_index = windowsetup.pattern_selected;
		windowsetup.changed = windowsetup.changed | mdispattern;
		}
    }

i = get_bwcolor_widget_state(&windowsetup.foreground,
	&windowsetup.background, 
	&windowsetup.changed, mwinforeground,mwinbackground);


/* get value of border and highlight color */
if (system_color_type != black_white_system)
	{
	changed = get_color_widget_state(&windowsetup.activetitle);
	if (changed != 0)
	    windowsetup.changed = windowsetup.changed | mwinactive;
#ifdef XUI
	changed = get_color_widget_state(&windowsetup.inactivetitle);
	if (changed != 0)
	    windowsetup.changed = windowsetup.changed | mwininactive;
#endif
	}
else
	{
	if ((windowsetup.changed & mwinforeground) != 0)
	    {
	    bcopy(&windowsetup.foreground,&windowsetup.activetitle,sizeof(struct color_data));
	    windowsetup.changed = windowsetup.changed | mwininactive;
#ifdef XUI
	    bcopy(&windowsetup.foreground,&windowsetup.inactivetitle,sizeof(struct color_data));
	    windowsetup.changed = windowsetup.changed | mwinactive;
#endif
	    }
	}

#ifdef XUI
if ((windowsetup.changed & mwinbackground) != 0)
    {
    bcopy(&windowsetup.background,&windowsetup.winmgrformbackground,
		sizeof(struct color_data));
    windowsetup.changed = windowsetup.changed | mwinformbackground;
    }

if ((windowsetup.changed & mwinforeground) != 0)
    {
    bcopy(&windowsetup.foreground,&windowsetup.winmgrformforeground,
		sizeof(struct color_data));
    windowsetup.changed = windowsetup.changed | mwinformforeground;
    }
i = XmToggleButtonGetState(windowsetup.icon_small_id);
if (i != windowsetup.icon_style)
    {
    windowsetup.icon_style = i;
    windowsetup.changed = windowsetup.changed | mwiniconstyle;
    }
#endif
i = XmToggleButtonGetState(windowsetup.wmexe_default_id);
if (i != windowsetup.wmexe)
    {
    windowsetup.wmexe = i;
    windowsetup.changed = windowsetup.changed | mwinexe;
    }
}

int	windowattr_get_values()
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      This routine will get the resources from the resource database
**      for each item in the Window Customization.  We need to
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
int	accelnum, acceldenom, accelthresh;

/* screen saver enable disable */
sm_get_any_resource(idissaver, svalue, value);
if (strcmp(svalue, "disable") == 0)
    windowsetup.saver = disable_value;
else
    windowsetup.saver = enable_value;

/* saver seconds*/
sm_get_any_resource(idissaverseconds, svalue, value);
windowsetup.saver_seconds = value[0];

/* display foreground/background color */
get_color_resource(&windowsetup.screen_foreground, idisforeground);
get_color_resource(&windowsetup.screen_background, idisbackground);

/* get the pattern resource */
sm_get_any_resource(idispattern, svalue, value);
windowsetup.pattern_index = value[0];

/* active title bar */
get_color_resource(&windowsetup.activetitle, iwinactive);
#ifdef XUI
get_color_resource(&windowsetup.inactivetitle, iwininactive);
#endif

/* foreground color*/
get_color_resource(&windowsetup.foreground, iwinforeground);

/* background color*/
get_color_resource(&windowsetup.background, iwinbackground);

/* window manager border color */
#ifdef XUI
get_color_resource(&windowsetup.winmgrbordercolor, iwmbordercolor);
#endif

/* window manager border foreground color */
#ifdef XUI
get_color_resource(&windowsetup.winmgrformforeground, iwmformforeground);
#endif

/* window icon style */
#ifdef XUI
sm_get_any_resource(iwmiconstyle, svalue, value);
windowsetup.icon_style = value[0];
#endif

/* window manager exe */
  if (strcmp(options.session_wm, "System Default") == 0)
    strcpy(svalue, options.session_default_wm);
  else
    strcpy(svalue, options.session_wm);
if (windowsetup.wmother != NULL)
    XtFree(windowsetup.wmother);
windowsetup.wmother = NULL;
windowsetup.wmexe = 1;
if (strcmp(svalue, "System Default") != 0)
    {
    windowsetup.wmother =  XtMalloc(strlen(svalue) + 1);
    strcpy(windowsetup.wmother, svalue);
    windowsetup.wmexe = 0;
    }
}


int	windowattr_set_values()
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
unsigned	int	status,found,found1,i,j;
Arg	arglist[10];
XmString	temparray[2];


windowsetup.changed = 0;

/* screen saver */
if (windowsetup.saver == enable_value)
	{
	XmToggleButtonSetState(windowsetup.saverenable_id, 1, 0);
	XmToggleButtonSetState(windowsetup.saverdisable_id,0, 0);
	}
else
	{
	XmToggleButtonSetState(windowsetup.saverenable_id, 0, 0);
	XmToggleButtonSetState(windowsetup.saverdisable_id,1, 0);
	}

/* saver volume slider */
XmScaleSetValue(windowsetup.saver_id, windowsetup.saver_seconds);


/* window color */
set_bwcolor_widget_state(&windowsetup.screen_foreground, 
	    &windowsetup.screen_background);

/* set the correct colors in the pattern box - based on screen fg/bg */
windowsetup.pattern_selected = -1;
set_pattern_colors(&windowsetup.screen_foreground,
	    &windowsetup.screen_background);

/* set the correct pattern */
set_correct_pattern();

/* window active title color */
if (system_color_type != black_white_system)
    {
    status = set_color_widget_state(&windowsetup.activetitle);
    if (!status)
	    {
	    color_allocation_failure();
	    XtSetSensitive(windowsetup.activetitle.widget_list[0], FALSE);
	    }
#ifdef XUI
    status = set_color_widget_state(&windowsetup.inactivetitle);
    if (!status)
    	{
	color_allocation_failure();
	XtSetSensitive(windowsetup.inactivetitle.widget_list[0], FALSE);
	}
#endif
    }

/* window foreground */
set_bwcolor_widget_state(&windowsetup.foreground, &windowsetup.background);

/* icon style */
#ifdef XUI
if (windowsetup.icon_style == small_icon)
	{
	XmToggleButtonSetState(windowsetup.icon_small_id, 1, 0);
	XmToggleButtonSetState(windowsetup.icon_large_id,0, 0);
	}
else
	{
	XmToggleButtonSetState(windowsetup.icon_small_id, 0, 0);
	XmToggleButtonSetState(windowsetup.icon_large_id,1, 0);
	}
#endif
/* Window manager exe*/
if (windowsetup.wmexe == 1)
	{
	XmToggleButtonSetState(windowsetup.wmexe_default_id, 1, 0);
	XmToggleButtonSetState(windowsetup.wmexe_other_id,0, 0);
	XmTextSetString(windowsetup.wmother_text_id,
			options.session_default_wm);
	}
else
	{
	XmToggleButtonSetState(windowsetup.wmexe_default_id, 0, 0);
	XmToggleButtonSetState(windowsetup.wmexe_other_id, 1, 0);
	XmTextSetString(windowsetup.wmother_text_id, windowsetup.wmother);
	}
}

int	window_put_attrs()
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      When Window Customization is changed, we need to write the
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

/* saver enable/disable */
if ((windowsetup.changed & mdissaver) != 0)
	{
	if (windowsetup.saver == enable_value)
		{
		sm_put_resource(idissaver, "enable");
		}
	else
		{
		sm_put_resource(idissaver, "disable");
		}
	}

/* saver slider */
if ((windowsetup.changed & mdissaverseconds) != 0)
	{
	status = int_to_str(windowsetup.saver_seconds, astring, sizeof(astring));
	if (status == SS_NORMAL)
		{
		sm_put_resource(idissaverseconds, astring);
		}
	}

/* display colors*/
if ((windowsetup.changed & mdisforeground) != 0)
	{
	set_color_resource(&windowsetup.screen_foreground, idisforeground);
	}

if ((windowsetup.changed & mdisbackground) != 0)
	{
	set_color_resource(&windowsetup.screen_background, idisbackground);
	}

if ((windowsetup.changed & mdispattern) != 0)
	{
	sm_put_int_resource(idispattern, windowsetup.pattern_index);
	}

/* active title */
if ((windowsetup.changed & mwinactive) != 0)
	{
	set_color_resource(&windowsetup.activetitle, iwinactive);
	}
#ifdef XUI
if ((windowsetup.changed & mwininactive) != 0)
	{
	set_color_resource(&windowsetup.inactivetitle, iwininactive);
	}
#endif

/* foreground color */
if ((windowsetup.changed & mwinforeground) != 0)
	{
	set_color_resource(&windowsetup.foreground, iwinforeground);
	}

/* background color */
if ((windowsetup.changed & mwinbackground) != 0)
	{
	set_color_resource(&windowsetup.background, iwinbackground);
	}

/* these are based on foreground and background colors, but they are
   seperate resources */
#ifdef XUI
if ((windowsetup.changed & mwinformforeground) != 0)
	{
	set_color_resource(&windowsetup.winmgrformforeground, 
			iwmformforeground);
	}
if ((windowsetup.changed & mwinbordercolor) != 0)
	{
	set_color_resource(&windowsetup.winmgrbordercolor, iwmbordercolor);
	}
/* icon style */
if ((windowsetup.changed & mwiniconstyle) != 0)
	{
	sm_put_int_resource(iwmiconstyle, windowsetup.icon_style);
	}
#endif
/* window manager exe */
value = XmTextGetString(windowsetup.wmother_text_id);
if (windowsetup.wmother != 0)
    {
    if ((strcmp(value, windowsetup.wmother) != 0))
	{
	windowsetup.changed = windowsetup.changed | mwinexe;
	}
    }
else
    {
    if (*value != 0)
	windowsetup.changed = windowsetup.changed | mwinexe;
    }

if ((windowsetup.changed & mwinexe) != 0)
	{
	if (windowsetup.wmexe == 1)
	    {
	    sm_put_resource(iwinmgrexe, options.session_default_wm);
	    if (windowsetup.wmother != NULL)
		{
		XtFree(windowsetup.wmother);
		windowsetup.wmother = NULL;
		}
	    }
	else
	    {
	    sm_put_resource(iwinmgrexe, value);
	    if (windowsetup.wmother != NULL)
		{
		XtFree(windowsetup.wmother);
		}
	    windowsetup.wmother = XtMalloc(strlen(value) + 1);
	    strcpy(windowsetup.wmother, value);
	   }
	}
XtFree(value);


/* update the property and make the x calls */
if (windowsetup.changed != 0)
	{
	sm_change_property(XtDisplay(smdata.toplevel));
	execute_display(windowsetup.changed);
	smdata.resource_changed = 1;
	if ((windowsetup.changed & mwincolors) != 0)
	    {
	    if ((windowsetup.changed & mwinexe) != 0)
		put_error(0, k_cust_both_msg);
            else
		put_error(0, k_cust_window_msg);
	    }
        else
	    {
	    if ((windowsetup.changed & mwinexe) != 0)
		put_error(0, k_cust_iconbox_msg);
	    }
	windowsetup.changed = 0;
	}
}

void	radio_select(widget, tag, reason)
Widget	*widget;
caddr_t tag;
unsigned int reason;
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**
**	the user switched foreground and background color.  We need to
**	change the solid foreground and background buttons in the
**	pattern box to reflect this 
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
unsigned int i;

i = XmToggleButtonGetState(widget);

if (i == 1)
    {
    /* make it black */
    set_widget_color(1, 1, BlackPixel(display_id,screen) , 
		WhitePixel(display_id,screen));
    }
else
    {
    /* make it white */
    set_widget_color(1, 1, WhitePixel(display_id,screen) , 
		BlackPixel(display_id,screen));
    }
}


void	radio_select_wmexe(widget, tag, reason)
Widget	*widget;
caddr_t tag;
caddr_t reason;
{
  XmToggleButtonCallbackStruct *d;

  d = (XmToggleButtonCallbackStruct *) reason;

  if (d->set == True)
    XmTextSetString(windowsetup.wmother_text_id, options.session_default_wm);
}
  

