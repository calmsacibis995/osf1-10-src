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
#include "smresource.h"
#define SS_NORMAL 1
#include <X11/Xlib.h>
#include <X11/decwcursor.h>
#include <X11/cursorfont.h>
#include "ultrixcursors.h"

char errout[80];

/* macros */
#define accel_match_none(n,d,t) ((n == temp_none_num)&&(d == temp_none_den)&&(t == temp_none_t))
#define accel_match_slow(n,d,t) ((n == temp_slow_num)&&(d == temp_slow_den)&&(t == temp_slow_t))
#define accel_match_med(n,d,t) ((n == temp_med_num)&&(d == temp_med_den)&&(t == temp_med_t))
#define accel_match_fast(n,d,t) ((n == temp_fast_num)&&(d == temp_fast_den)&&(t == temp_fast_t))

static	int temp_none_num = 1;
static	int temp_none_den = 1;
static  int temp_none_t = 2000;
static	int temp_slow_num = 7;
static	int temp_slow_den = 2;
static  int temp_slow_t = 4;
static	int temp_med_num = 7;
static	int temp_med_den = 1;
static  int temp_med_t = 3;
static	int temp_fast_num = 10;
static	int temp_fast_den = 1;
static  int temp_fast_t = 2;

/* moved from smstrings.h -- only used here */
static	XmString	pointer_shapes[num_pointers];

void	point_action();
void	point_cancel();
void	point_apply();
void	listbox_callback();

extern void color_select();

int	create_point_attrs()
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	This routine is called when the user selects Pointer from the
**	Session Manager customize menu.  It creates a modeless dialog
**	box which allows the user to set certain features of the mouse
**	and pointer.
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
XFontStruct *fontstruct;
XmFontList fontlist;
int num_args;
char	pointer_convert[2];
unsigned int i;
XtCallbackRec	ss_callback[2];
static MrmRegisterArg reglist[] = {
    {"PointCancelCallback", (caddr_t) point_cancel},
    {"PointApplyCallback", (caddr_t) point_apply},
    {"PointOkCallback", (caddr_t) point_action},
    {"accelbox_id", (caddr_t) &pointsetup.accel_id},
    {"accelnone_id",(caddr_t) &pointsetup.none_id},
    {"accelslow_id",(caddr_t) &pointsetup.slow_id},
    {"accelmedium_id", (caddr_t) &pointsetup.med_id},
    {"accelfast_id", (caddr_t) &pointsetup.fast_id},
    {"buttonbox_id", (caddr_t) &pointsetup.button_id},
    {"buttonright_id", (caddr_t) &pointsetup.right_id},
    {"buttonleft_id", (caddr_t) &pointsetup.left_id},
    {"doubleclick_id", (caddr_t) &pointsetup.double_id},
    {"shapebox_id", (caddr_t) &pointsetup.cursor_id},
    {"ShapeListboxCallback", (caddr_t) listbox_callback},
};
static int reglist_num = (sizeof reglist / sizeof reglist [0]);
static MrmRegisterArg bw_reglist[] = {
    {"bwforeground_id", (caddr_t) &pointsetup.foreground.widget_list[0]},
    {"bwblack_id", (caddr_t) &pointsetup.foreground.widget_list[1]},
    {"bwwhite_id", (caddr_t) &pointsetup.foreground.widget_list[2]},
};
static int bw_reglist_num = (sizeof bw_reglist / sizeof bw_reglist [0]);
static MrmRegisterArg color_reglist[] = {
    {"clforeground_id", (caddr_t) &pointsetup.foreground.widget_list[0]},
    {"clbackground_id", (caddr_t) &pointsetup.background.widget_list[0]},
    {"ColorSelectCallback", (caddr_t) color_select},
};
static int color_reglist_num = (sizeof color_reglist/sizeof color_reglist [0]);
Arg listargs[10];

MrmRegisterNames (reglist, reglist_num);

/* build the dialog using UIL */
if (system_color_type == black_white_system) {
    MrmRegisterNames (bw_reglist, bw_reglist_num);

    MrmFetchWidget(s_DRMHierarchy, "BWCustomizePointer", smdata.toplevel,
		   &pointsetup.point_attr_id,
		   &drm_dummy_class);
}
else {
    pointsetup.foreground.mix_changed = 0;
    pointsetup.background.mix_changed = 0;

    MrmRegisterNames (color_reglist, color_reglist_num);

    MrmFetchWidget(s_DRMHierarchy, "CLCustomizePointer", smdata.toplevel,
		   &pointsetup.point_attr_id,
		   &drm_dummy_class);
}

for (i=0; i<num_pointers; i++)
{
    /* this is a bit of a cludge.  The cursor font for the session manager
    has each cursor offset by one from the decw$cursor font.  This is
    because listboxes couldn't handle a font offset of 0. */
    pointer_convert[0] = pointer_index[i] + 1;
    pointer_convert[1] = 0;
    pointer_shapes[i] = XmStringCreate(&pointer_convert[0], 
				def_char_set);
}
num_args = 0;
XtSetArg(listargs[num_args], XmNitems, pointer_shapes);  num_args++;
XtSetArg(listargs[num_args], XmNitemCount, num_pointers);  num_args++;

pointsetup.cursor_selected = NULL;
XtUnmanageChild(pointsetup.cursor_id);
XtSetValues(pointsetup.cursor_id, listargs, num_args);
XtManageChild(pointsetup.cursor_id);

return(1);
}

static	void	listbox_callback(widget, tag, data)
Widget	widget;
caddr_t	tag;
XmListCallbackStruct    *data;
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**  This routine is called every time a user selects an item from the
**  listbox.  We need to mark that something was selected so that the
**  appropriate Xlib call can be made when the OK button is hit.  If
**  nothing is ever selected, the we won't make an xlib call 
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

if (pointsetup.cursor_selected != NULL)
    XmStringFree(pointsetup.cursor_selected);

pointsetup.cursor_selected = XmStringCopy(data->item);
}

void	point_action(widget, tag, reason)
Widget	*widget;
caddr_t tag;
unsigned int reason;
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	The OK button was hit from the Pointer Customization dialog box.
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
if (pointsetup.managed == ismanaged)
{
    pointsetup.managed = notmanaged;

    /* Get the settings of the widgets */
    point_dialog_get_values();

    /* Put the new property on the root window, and exeucte XLIB calls */
    point_put_attrs();

    XtUnmanageChild(XtParent(*widget));

    /* Free the colors we possibly allocated for this dialog box */
    free_allocated_color(&pointsetup.foreground);
    free_allocated_color(&pointsetup.background);
    if (pointsetup.cursor_selected != NULL)
	XmStringFree(pointsetup.cursor_selected);

    pointsetup.cursor_selected = NULL;
}
}

void	point_apply(widget, tag, reason)
Widget	*widget;
caddr_t tag;
unsigned int reason;
{
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	The APPLY button was hit from the Pointer Customization dialog box.
**	We need to look at the widgets, determine what changed, make
**	the appropriate xlib calls, put the resources on the root window.
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
/* Get the current settings */
point_dialog_get_values();
/* Update the property and make xlib calls */
point_put_attrs();
}

void	point_cancel(widget, tag, reason)
Widget	*widget;
caddr_t	tag;
unsigned int reason;
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	The CANCEL button was hit from the Pointer Customization dialog box.
**	We need to unmanage the dialog box and reset the listbox
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
if (pointsetup.managed == ismanaged)
{
    pointsetup.managed = notmanaged;
    pointsetup.changed = 0;
    XtUnmanageChild(XtParent(*widget));

    /* Free the colors that we allocated for pointer fore/back ground */
    free_allocated_color(&pointsetup.foreground);
    free_allocated_color(&pointsetup.background);
    if (pointsetup.cursor_selected != NULL)
	XmStringFree(pointsetup.cursor_selected);

    pointsetup.cursor_selected = NULL;
}
}

int	point_dialog_get_values()
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	The OK or APPLY button was hit.  Look at each widget and store
**	its value in the data structures IF it is different from the
**	current value.  Also mark the bit in the change mask if it
**	changes so that we will make the appropriate XLIB Calls.
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
unsigned    int	i,j,k,l,found;

/* get value of pointer foreground/background */
i = get_bwcolor_widget_state(&pointsetup.foreground, &pointsetup.background,
    &pointsetup.changed, mptforeground, mptbackground);

/* get value of button order arrangement */
i = XmToggleButtonGetState(pointsetup.right_id);
if (i == 1)
{
    /* make it right handed */
    if (pointsetup.button != right_handed)
	{
	pointsetup.changed = pointsetup.changed | mptbutton;
	pointsetup.button = right_handed;
	}
}
else
{
    if (pointsetup.button != left_handed)
	{
	pointsetup.changed = pointsetup.changed | mptbutton;
	pointsetup.button = left_handed;
	}
}

/* get the value of the mouse acceleration*/
if (XtIsSensitive(pointsetup.accel_id))
{
    i = XmToggleButtonGetState(pointsetup.none_id);
    j = XmToggleButtonGetState(pointsetup.slow_id);
    k = XmToggleButtonGetState(pointsetup.med_id);
    l = XmToggleButtonGetState(pointsetup.fast_id);

    if (i==1)
	{
	if (pointsetup.accel != none)
	{
	    pointsetup.accel_num = temp_none_num;
	    pointsetup.accel_denom = temp_none_den;
	    pointsetup.accel_threshold = temp_none_t;
	    pointsetup.accel = none;
	    pointsetup.changed = pointsetup.changed | mptaccel;
	}
	}
    else
	{
	if (j == 1)
	{
	    if (pointsetup.accel != slow)
		{
		pointsetup.accel_num = temp_slow_num;
		pointsetup.accel_denom = temp_slow_den;
		pointsetup.accel_threshold = temp_slow_t;
		pointsetup.accel = slow;
	        pointsetup.changed = pointsetup.changed | mptaccel;
		}
	}
	else
	{
	    if (k == 1)
		{
		if (pointsetup.accel != medium)
		{
		    pointsetup.accel_num = temp_med_num;
		    pointsetup.accel_denom = temp_med_den;
		    pointsetup.accel_threshold = temp_med_t;
		    pointsetup.accel = medium;
		    pointsetup.changed = pointsetup.changed | mptaccel;
		}
		 }
	    else
		{
		if (l == 1)
		{
		    if (pointsetup.accel != fast)
			{
			pointsetup.accel_num = temp_fast_num;
			pointsetup.accel_denom = temp_fast_den;
			pointsetup.accel_threshold = temp_fast_t;
			pointsetup.accel = fast;
			pointsetup.changed = pointsetup.changed | mptaccel;
			}
		}
		 }
	}
	}
}


/* get the value of the mouse double click slider */
XmScaleGetValue(pointsetup.double_id, &i);
if (i != pointsetup.double_time)
{
    pointsetup.double_time = i;
    pointsetup.changed = pointsetup.changed | mptdouble;
}


/* get the index for the cursor shape */
/* We do this by comparing the string that was selected to each value
   in the list box.  When we get the index into the list box, we will
   have the index into the cursor font.  It is designed with this
   algorithm in mind */
if (pointsetup.cursor_selected != NULL)
    for (i=0; (i < num_pointers); i++)
	if (XmStringCompare(pointer_shapes[i], pointsetup.cursor_selected))
	{
	    if (pointsetup.cursor_index != -pointer_index[i])
	    {
		pointsetup.cursor_index = -pointer_index[i];
		pointsetup.changed = pointsetup.changed | mptcursor;
	    }
	    break;
	}
}

int	pointattr_get_values()
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	This routine will get the resources from the resource database
**	for each item in the Pointer Customization.  We need to 
**	set the values of the widgets when the dialog box is managed.
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


/* foreground color*/
get_color_resource(&pointsetup.foreground, iptrforeground);

/* background color*/
get_color_resource(&pointsetup.background, iptrbackground);

/* button arrangement */
size = sm_get_any_resource(iptrbuttonorder, svalue, value);
if (strcmp(svalue, "left") == 0)
{
    pointsetup.button = left_handed;
}
else
{
    pointsetup.button = right_handed;
}

/* mouse acceleration */
size = sm_get_any_resource(iptraccelnum, svalue, value);
pointsetup.accel_num = value[0];

size = sm_get_any_resource(iptraccelden, svalue, value);
pointsetup.accel_denom = value[0];

size = sm_get_any_resource(iptraccelthr, svalue, value);
pointsetup.accel_threshold = value[0];

/*** initialize this to none selected, so that when we set the toggle
	buttons, they are set correctly ***/
pointsetup.accel = no_accel;

/* mouse double click */
size = sm_get_any_resource(iptrdoubleclick, svalue, value);
pointsetup.double_time = value[0];

/* cursor shape index into the cursor font */
size = sm_get_any_resource(iptrshape, svalue, value);
pointsetup.cursor_index = value[0];
}

int	pointattr_set_values()
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	When the dialog box is managed, we need to set all of the widgets
**	to the correct values based on the current settings of the
**	resources.
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
unsigned	int	status,found,i,j;
Arg	arglist[10];
    int	accel_num;
    int	accel_denom;
    int	accel_threshold;
XmString	temparray[2];
char	svalue[256];
unsigned    int	size;


pointsetup.changed = 0;

/* pointer foreground color */
set_bwcolor_widget_state(&pointsetup.foreground, &pointsetup.background);

/* button order */
if (pointsetup.button == right_handed)
	{
	XmToggleButtonSetState(pointsetup.right_id, 1, 0);
	XmToggleButtonSetState(pointsetup.left_id,0, 0);
	}
else
	{
	XmToggleButtonSetState(pointsetup.right_id, 0, 0);
	XmToggleButtonSetState(pointsetup.left_id,1, 0);
	}

/* mouse acceleration */
accel_num = pointsetup.accel_num;
accel_denom = pointsetup.accel_denom;
accel_threshold = pointsetup.accel_threshold;

/*******temporarily allow user to modify slow, med, fast, none values
    for user interface testing purposes *********/
size = sm_get_resource("DXsession.slow_num",svalue);
if (size != 0)
{
    status = str_to_int(svalue, &size);
    if (status == SS_NORMAL)
	 temp_slow_num = size;
}
size = sm_get_resource("DXsession.slow_denom",svalue);
if (size != 0)
{
    status = str_to_int(svalue, &size);
    if (status == SS_NORMAL)
	 temp_slow_den = size;
}

size = sm_get_resource("DXsession.slow_thresh",svalue);
if (size != 0)
{
    status = str_to_int(svalue, &size);
    if (status == SS_NORMAL)
	 temp_slow_t = size;
}

size = sm_get_resource("DXsession.none_num",svalue);
if (size != 0)
{
    status = str_to_int(svalue, &size);
    if (status == SS_NORMAL)
	 temp_none_num = size;
}
size = sm_get_resource("DXsession.none_denom",svalue);
if (size != 0)
{
    status = str_to_int(svalue, &size);
    if (status == SS_NORMAL)
	 temp_none_den = size;
}

size = sm_get_resource("DXsession.none_thresh",svalue);
if (size != 0)
{
    status = str_to_int(svalue, &size);
    if (status == SS_NORMAL)
	 temp_none_t = size;
}
size = sm_get_resource("DXsession.med_num",svalue);
if (size != 0)
{
    status = str_to_int(svalue, &size);
    if (status == SS_NORMAL)
	 temp_med_num = size;
}
size = sm_get_resource("DXsession.med_denom",svalue);
if (size != 0)
{
    status = str_to_int(svalue, &size);
    if (status == SS_NORMAL)
	 temp_med_den = size;
}

size = sm_get_resource("DXsession.med_thresh",svalue);
if (size != 0)
{
    status = str_to_int(svalue, &size);
    if (status == SS_NORMAL)
	 temp_med_t = size;
}
size = sm_get_resource("DXsession.fast_num",svalue);
if (size != 0)
{
    status = str_to_int(svalue, &size);
    if (status == SS_NORMAL)
	 temp_fast_num = size;
}
size = sm_get_resource("DXsession.fast_denom",svalue);
if (size != 0)
{
    status = str_to_int(svalue, &size);
    if (status == SS_NORMAL)
	 temp_fast_den = size;
}

size = sm_get_resource("DXsession.fast_thresh",svalue);
if (size != 0)
{
    status = str_to_int(svalue, &size);
    if (status == SS_NORMAL)
	 temp_fast_t = size;
}

/*************end temp****************/
if (accel_match_slow(accel_num, accel_denom, accel_threshold))
{
    XmToggleButtonSetState(pointsetup.none_id, 0, 0);
    XmToggleButtonSetState(pointsetup.slow_id,1, 0);
    XmToggleButtonSetState(pointsetup.med_id,0, 0);
    XmToggleButtonSetState(pointsetup.fast_id,0, 0);
    XtSetSensitive(pointsetup.accel_id, 1);
}    
else
{
    if (accel_match_med(accel_num, accel_denom, accel_threshold))
    {
	XmToggleButtonSetState(pointsetup.none_id, 0, 0);
	XmToggleButtonSetState(pointsetup.slow_id,0, 0);
	XmToggleButtonSetState(pointsetup.med_id,1, 0);
	XmToggleButtonSetState(pointsetup.fast_id,0, 0);
	XtSetSensitive(pointsetup.accel_id, 1);
	}
    else
	{
	if (accel_match_fast(accel_num, accel_denom, accel_threshold))
	{
	    XmToggleButtonSetState(pointsetup.none_id, 0, 0);
	    XmToggleButtonSetState(pointsetup.slow_id,0, 0);
	    XmToggleButtonSetState(pointsetup.med_id,0, 0);
	    XmToggleButtonSetState(pointsetup.fast_id,1, 0);
	    XtSetSensitive(pointsetup.accel_id, 1);
	}    
	else
	{
	    if (accel_match_none(accel_num, accel_denom, accel_threshold))
	   {
		XmToggleButtonSetState(pointsetup.none_id, 1, 0);
		XmToggleButtonSetState(pointsetup.slow_id,0, 0);
		XmToggleButtonSetState(pointsetup.med_id,0, 0);
		XmToggleButtonSetState(pointsetup.fast_id,0, 0);
	        XtSetSensitive(pointsetup.accel_id, 1);
		} 
	    else
		{
		XmToggleButtonSetState(pointsetup.none_id, 0, 0);
		XmToggleButtonSetState(pointsetup.slow_id,0, 0);
		XmToggleButtonSetState(pointsetup.med_id,0, 0);
		XmToggleButtonSetState(pointsetup.fast_id,0, 0);
		}
	}
	}
 }
    
/* double click time*/
XmScaleSetValue(pointsetup.double_id, pointsetup.double_time);


/* list box for pointer shape. Check if the index value is one that
    we know of */
found = 0;
j= num_pointers;
for (i = 0; ((i< j) && (!found)); i++)
    if (pointsetup.cursor_index == -pointer_index[i])
    {
	XtSetArg(arglist[0], XmNselectedItemCount, 1);
	temparray[0] = pointer_shapes[i];
	temparray[1] = 0;
	XtSetArg(arglist[1], XmNselectedItems, temparray);
	XtSetValues(pointsetup.cursor_id, arglist, 2);
	found = 1;
	break;
    }
if (!found)
{
    XtSetArg(arglist[0], XmNselectedItemCount, 0);
    XtSetArg(arglist[1], XmNselectedItems, NULL);
    XtSetValues(pointsetup.cursor_id, arglist, 2);
}

if (pointsetup.cursor_selected != NULL)
    XmStringFree(pointsetup.cursor_selected);

pointsetup.cursor_selected = NULL;
}


int	point_put_attrs()
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	When Pointer Customization is changed, we need to write the
**	new values of the resources to the resource database.  This
**	routine will write each resource to the database.
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

/* foreground color */
if ((pointsetup.changed & mptforeground) != 0)
	set_color_resource(&pointsetup.foreground, iptrforeground);

/* background color */
if ((pointsetup.changed & mptbackground) != 0)
	set_color_resource(&pointsetup.background, iptrbackground);

/* button arrangement */
if ((pointsetup.changed & mptbutton) != 0)
    if (pointsetup.button == right_handed)
	sm_put_resource(iptrbuttonorder, "right");
    else
	sm_put_resource(iptrbuttonorder, "left");

/* mouse acceleration */
if ((pointsetup.changed & mptaccel) != 0)
{
    sm_put_int_resource(iptraccelnum, pointsetup.accel_num);
    sm_put_int_resource(iptraccelden, pointsetup.accel_denom);
    sm_put_int_resource(iptraccelthr, pointsetup.accel_threshold);
}

/* double click time*/
if ((pointsetup.changed & mptdouble) != 0)
    sm_put_int_resource(iptrdoubleclick, pointsetup.double_time);

/* pointer pattern */
if ((pointsetup.changed & mptcursor) != 0)
    sm_put_int_resource(iptrshape, pointsetup.cursor_index);

if (pointsetup.changed != 0)
    sm_change_property(XtDisplay(smdata.toplevel));
    smdata.resource_changed = 1;
    execute_pointer(pointsetup.changed);
    pointsetup.changed = 0;
}
