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
#include <pwd.h>
#include <signal.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/file.h>

#include <X11/keysym.h>
#define SS_NORMAL 1
#include "smdata.h"
#include "smresource.h"

void	keyboard_action();
void	keyboard_apply();
void	keyboard_cancel();
void	listbox_callback();
void    kbdprobe_callback();
void	keyboard_listbox_setcontents();

int	create_keyboard_attrs()
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	This routine is called when the user selects Keyboard from the
**	Session Manager customize menu.  It creates a modeless dialog
**	box which allows the user to set certain features of the keyboard.
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
        {"KeyCancelCallback", (caddr_t) keyboard_cancel},
        {"KeyApplyCallback", (caddr_t) keyboard_apply},
        {"KeyOkCallback", (caddr_t) keyboard_action},
        {"KeyListboxCallback", (caddr_t) listbox_callback},
	{"KeyboardProbeCallback", (caddr_t) kbdprobe_callback},
        {"bellenablebox_id", (caddr_t) &keysetup.bellonoff_id},
        {"bellenable_id", (caddr_t) &keysetup.bellenable_id},
        {"belldisable_id", (caddr_t) &keysetup.belldisable_id},
        {"bellslider_id", (caddr_t) &keysetup.bell_id},
        {"clickenablebox_id", (caddr_t) &keysetup.clickonoff_id},
        {"clickenable_id", (caddr_t) &keysetup.clickenable_id},
        {"clickdisable_id", (caddr_t) &keysetup.clickdisable_id},
        {"clickslider_id", (caddr_t) &keysetup.click_id},
        {"autorepeatbox_id", (caddr_t) &keysetup.autorepeat_id},
        {"repeatenable_id", (caddr_t) &keysetup.repeatenable_id},
        {"repeatdisable_id", (caddr_t) &keysetup.repeatdisable_id},
	{"kbdprobe_id", (caddr_t) &keysetup.kbdprobe_id},
        {"lockbox_id", (caddr_t) &keysetup.lock_id},
        {"lockcaps_id", (caddr_t) &keysetup.caps_id},
        {"lockshift_id", (caddr_t) &keysetup.shift_id},
        {"operatorbox_id", (caddr_t) &keysetup.operator_id},
        {"operator_f2_id", (caddr_t) &keysetup.operatorf2_id},
        {"operator_f1_id", (caddr_t) &keysetup.operatorf1_id},
        {"operatormodbox_id", (caddr_t) &keysetup.operator_mod_id},
        {"operator_ctrl_id", (caddr_t) &keysetup.operator_ctrl_id},
        {"operator_shift_id", (caddr_t) &keysetup.operator_shift_id},
        {"keylistbox_id", (caddr_t) &keysetup.keyboard_id},
	};

static int reglist_num = (sizeof reglist / sizeof reglist [0]);

MrmRegisterNames (reglist, reglist_num);

/* build the dialog using UIL */
MrmFetchWidget(s_DRMHierarchy, "CustomizeKeyboard", smdata.toplevel,
		    &keysetup.key_attr_id,
		    &drm_dummy_class);
keysetup.keyboard_selected = NULL;
keysetup.managed = ismanaged;

/* Set up the listbox */
keyboard_listbox_setcontents();

return(1);
}

void	keyboard_listbox_setcontents()
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	Get a list of the keyboards installed on the system and put the
**	names in a list box.
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
  int	num_items;
  XtCallbackRec	ss_callback[2];
  char	**keyboard_list_strings;
  unsigned    int	i;
  XmString	*cs_array = NULL;
  Arg	arglist[5];

  /* get the list of keymaps in the keymap directory*/
  keyboard_list_strings = (char **)DXListKeyboardMaps(&num_items);

  /* allocate the space for the comp strings array for list box */
  if (num_items != 0)
    {
      cs_array = (XmString *)XtMalloc(num_items * sizeof(XmString));
      /* convert the list of c strings, to a list of compstrings */
      convert_list(keyboard_list_strings, cs_array, num_items);
    }

  /* set up the listbox*/
  XtSetArg (arglist[0], XmNitemCount, num_items);
  XtSetArg (arglist[1], XmNitems, cs_array);
  XtSetValues (keysetup.keyboard_id, arglist, 2);

  /* Free space allocated for keyboard list */
  if (num_items != 0)
    {
      DXFreeKeyboardMapList(keyboard_list_strings);
    }
  
  for (i=0; i<num_items; i++)
    {
      /* convert_list allocates storage for each string - free it here */
      XmStringFree(cs_array[i]);
    }
  
  /* free the storage for the actual array as well */
  XtFree((char *)cs_array);
}

/* routine to delete list box */
void	keyboard_listbox_delete()
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	We will rebuild the keyboard listbox each time we manage the
**	dialog box.  This will allow the sophisticated user to install
**	keymaps on the fly.   This routine deletes the listbox.  It is
**	called every time we unmanager the Keyboard Customization dialog
**	box.
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
  Arg arglist[5];
  unsigned int	num_items = 0;

  /* if we don't delselect all items, the listbox
     code accvios when we reset the list.  */
  XmListDeselectAllItems(keysetup.keyboard_id);

  XtSetArg (arglist[0], XmNitemCount, 0);
  XtSetValues (keysetup.keyboard_id, arglist, 1);

  if (keysetup.keyboard_selected != NULL)
    XmStringFree(keysetup.keyboard_selected);

  keysetup.keyboard_selected = NULL;
}

void	listbox_callback(widget, tag, data)
Widget	widget;
caddr_t	tag;
XmListCallbackStruct    *data;
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	Called when something is selected from the keyboard listbox.
**	We convert the value to a null terminated string and store
**	it in our data structures.
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
/* this routine is called every time a user selects an item from the
    listbox.  We need to mark that something was selected so that the
    appropriate Xlib call can be made when the OK button is hit.  If
    nothing is ever selected, the we won't make an xlib call */
  keysetup.changed = keysetup.changed | mkeyboard;
  if (keysetup.keyboard_selected != NULL)
    XmStringFree(keysetup.keyboard_selected);

  keysetup.keyboard_selected = XmStringCopy(data->item);
}

void	kbdprobe_callback(widget, tag, data)
     Widget	widget;
     caddr_t	tag;
     XmListCallbackStruct    *data;
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	Called when the user presses the Edit Keycaps button.  It
**	fetches the current keyboard type, parses out the keyboard
**	model number, and passes it to the dxkeycaps program.
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
**	This routine assumes that the keyboard model number has the
**	format "lknnn", where lk may be upper or lower case.
**	
**--
**/

{
  register int i;
  char *c;
  char *start;
  char buf[64];

  XmStringGetLtoR( keysetup.keyboard_selected, def_char_set, &c);
  for (i = 0; i < strlen(c); i++) {
      if (*c == 'l' || *c == 'L') {
	  start = c++;
	  if (*c == 'k' || *c == 'K') {
	      c++;
	      if (isdigit((int)*c) && isdigit((int)*(c+1)) && 
		  isdigit((int)*(c+2))) {
		  sprintf( buf, "/usr/bin/X11/dxkeycaps -keyboard %.5s", start);
		  doexecstr(buf, 0, TRUE);
		  return;
	      }
	  }
      }
      c++;
  }
  /* Don't recognize keyboard type */
  doexecstr("/usr/bin/X11/dxkeycaps -keyboard LK401", 0, TRUE);
}

void	keyboard_action(widget, tag, reason)
     Widget	*widget;
     caddr_t tag;
     unsigned int reason;
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	The OK button was hit from the Keyboard Customization dialog box.
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
  if (keysetup.managed == ismanaged)
    {
      keysetup.managed = notmanaged;
      /* Unmap the dialog box */
      XtUnmanageChild(XtParent(*widget));
      /* Get the settings on each widget */
      key_dialog_get_values();
      /* Save the resources and update the property */
      key_put_attrs();
      /* Clean up the listbox code */
      keyboard_listbox_delete();
    }
}

void	keyboard_apply(widget, tag, reason)
     Widget	*widget;
     caddr_t tag;
     unsigned int reason;
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	The APPLY button was hit from the Keyboard Customization dialog box.
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
{
  /* Get the current settings */
  key_dialog_get_values();
  /* Update the property and make xlib calls */
  key_put_attrs();
}

void	keyboard_cancel(widget, tag, reason)
     Widget	*widget;
     caddr_t	tag;
     unsigned int reason;
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	The CANCEL button was hit from the Keyboard Customization dialog box.
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
  if (keysetup.managed == ismanaged)
    {
      keysetup.managed = notmanaged;
      XtUnmanageChild(XtParent(*widget));
      keysetup.changed = 0;
      keyboard_listbox_delete();
    }
}

int	key_dialog_get_values()
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
unsigned	int	i,j,k,newvalue;

/* get value of bell enable disable */
i = XmToggleButtonGetState(keysetup.bellenable_id);
if (i != keysetup.bell)
    {
    keysetup.bell = i;
    keysetup.changed = keysetup.changed | mbell;
    }

/* get value of bell volume slider control */
XmScaleGetValue(keysetup.bell_id, &i);
if (i != keysetup.bell_volume)
    {
    keysetup.bell_volume = i;
    keysetup.changed = keysetup.changed | mbellvolume;
    }

/* get value of keyclick enable disable */
i = XmToggleButtonGetState(keysetup.clickenable_id);
if (i != keysetup.click)
    {
    keysetup.click = i;
    keysetup.changed = keysetup.changed | mclick;
    }

/* get value of keyclick volume slider control */
XmScaleGetValue(keysetup.click_id, &i);
if (i != keysetup.click_volume)
    {
    keysetup.click_volume = i;
    keysetup.changed = keysetup.changed | mclickvolume;
    }

/* get value of autorepeat enable disable */
i = XmToggleButtonGetState(keysetup.repeatenable_id);
if (i != keysetup.autorepeat)
    {
    keysetup.autorepeat = i;
    keysetup.changed = keysetup.changed | mautorepeat;
    }

/* get value of caps lock shift lock */
i = XmToggleButtonGetState(keysetup.shift_id);
if (i != keysetup.lock)
    {
    keysetup.lock = i;
    keysetup.changed = keysetup.changed | mlockstate;
    }

}

int	keyattr_get_values()
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	This routine will get the resources from the resource database
**	for each item in the Keyboard Customization.  We need to 
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

/* bell volume enable/disable */
size = sm_get_any_resource(ibell, svalue, value);
if (strcmp(svalue, "disable") == 0)
    {
    keysetup.bell = disable_value;
    }
else
    {
    keysetup.bell = enable_value;
    }

/* bell volume percent */
size = sm_get_any_resource(ibell_volume, svalue, value);
keysetup.bell_volume = value[0];

/* keyclick volume enable/disable */
size = sm_get_any_resource(ikeyclick, svalue, value);
if (strcmp(svalue, "disable") == 0)
    {
    keysetup.click = disable_value;
    }
else
    {
    keysetup.click = enable_value;
    }

/* keyclick volume percent */
size = sm_get_any_resource(ikey_volume, svalue, value);
keysetup.click_volume = value[0];

/* autorepeat enable/disable */
size = sm_get_any_resource(iautorepeat, svalue, value);
if (strcmp(svalue, "disable") == 0)
    {
    keysetup.autorepeat = disable_value;
    }
else
    {
    keysetup.autorepeat = enable_value;
    }

/* lock state caps_lock or shift_lock */
size = sm_get_any_resource(ilock, svalue, value);
if (strcmp(svalue, "caps") == 0)
    {
    keysetup.lock = caps_value;
    }
else
    {
    keysetup.lock = shift_value;
    }

/* get the resource for keyboard dialect */
size = sm_get_any_resource(ikey_dialect, svalue, value);
keysetup.keyboard[0] = 0;
strcpy(keysetup.keyboard, svalue);
}

int	keyattr_set_values()
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
unsigned	int	status;
XmString	temp1;

/* Mark that nothing has been changed yet */
keysetup.changed = 0;

/* bell volume enable */
if (keysetup.bell == enable_value)
	{
	XmToggleButtonSetState(keysetup.bellenable_id, 1, 0);
	XmToggleButtonSetState(keysetup.belldisable_id,0, 0);
	}
else
	{
	XmToggleButtonSetState(keysetup.bellenable_id, 0, 0);
	XmToggleButtonSetState(keysetup.belldisable_id,1, 0);
	}

/* bell volume slider */
XmScaleSetValue(keysetup.bell_id, keysetup.bell_volume);

/* click volume enable */
if (keysetup.click == enable_value)
	{
	XmToggleButtonSetState(keysetup.clickenable_id, 1, 0);
	XmToggleButtonSetState(keysetup.clickdisable_id,0, 0);
	}
else
	{
	XmToggleButtonSetState(keysetup.clickenable_id, 0, 0);
	XmToggleButtonSetState(keysetup.clickdisable_id,1, 0);
	}

/* click volume slider */
XmScaleSetValue(keysetup.click_id, keysetup.click_volume);

/* auto repeat enable/disable */
if (keysetup.autorepeat == enable_value)
	{
	XmToggleButtonSetState(keysetup.repeatenable_id, 1, 0);
	XmToggleButtonSetState(keysetup.repeatdisable_id,0, 0);
	}
else
	{
	XmToggleButtonSetState(keysetup.repeatenable_id, 0, 0);
	XmToggleButtonSetState(keysetup.repeatdisable_id,1, 0);
	}

/* lock state shift/caps*/
if (keysetup.lock == caps_value)
	{
	XmToggleButtonSetState(keysetup.caps_id, 1, 0);
	XmToggleButtonSetState(keysetup.shift_id,0, 0);
	}
else
	{
	XmToggleButtonSetState(keysetup.caps_id, 0, 0);
	XmToggleButtonSetState(keysetup.shift_id,1, 0);
	}

/*  Select an element in the list box, if it matches */
if (keysetup.keyboard[0] != 0)
    {
    temp1 = XmStringCreate(keysetup.keyboard , 
				def_char_set);
    XmListSelectItem(keysetup.keyboard_id, temp1, 0);
    /* Save initial setting */
    keysetup.keyboard_selected = XmStringCopy( temp1 );
    XmStringFree(temp1);
    }
}

int	key_put_attrs()
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	When Keyboard Customization is changed, we need to write the
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
char	*temp;

/* bell enable/disable */
if ((keysetup.changed & mbell) != 0)
	{
	if (keysetup.bell == enable_value)
		{
		sm_put_resource(ibell, "enable");
		}
	else
		{
		sm_put_resource(ibell, "disable");
		}
	}

/* bell slider */
if ((keysetup.changed & mbellvolume) != 0)
	{
	status = int_to_str(keysetup.bell_volume, astring, sizeof(astring));
	if (status == SS_NORMAL)
		{
		sm_put_resource(ibell_volume, astring);
		}
	}

/* click enable/disable */
if ((keysetup.changed & mclick) != 0)
	{
	if (keysetup.click == enable_value)
		{
		sm_put_resource(ikeyclick, "enable");
		}
	else
		{
		sm_put_resource(ikeyclick, "disable");
		}
	}

/* click slider */
if ((keysetup.changed & mclickvolume) != 0)
	{
	status = int_to_str(keysetup.click_volume, astring, sizeof(astring));
	if (status == SS_NORMAL)
		{
		sm_put_resource(ikey_volume, astring);
		}
	}

/* auto repeat */
if ((keysetup.changed & mautorepeat) != 0)
	{
	if (keysetup.autorepeat == enable_value)
		{
		sm_put_resource(iautorepeat, "enable");
		}
	else
		{
		sm_put_resource(iautorepeat, "disable");
		}
	}

/* lock state */
if ((keysetup.changed & mlockstate) != 0)
	{
	if (keysetup.lock == caps_value)
		{
		sm_put_resource(ilock, "caps");
		}
	else
		{
		sm_put_resource(ilock, "shift");
		}
	}

if ((keysetup.changed & mkeyboard) != 0)
    {
    if (keysetup.keyboard_selected != NULL)
	{
	/* temp = CSToLatin1 (keysetup.keyboard_selected); */
        XmStringGetLtoR(keysetup.keyboard_selected,
		def_char_set, &temp);
	strcpy(keysetup.keyboard, temp);
	sm_put_resource(ikey_dialect, temp);
	XtFree(temp);    
	}
   else
       keysetup.changed = keysetup.changed & (~(unsigned int)mkeyboard);
    }
    
if (keysetup.changed != 0)
	{
	sm_change_property(XtDisplay(smdata.toplevel));
	smdata.resource_changed = 1;
	execute_keyboard(keysetup.changed);
	keysetup.changed = 0;
	}
}
