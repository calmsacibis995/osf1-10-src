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
#include <X11/keysym.h>
#include <X11/cursorfont.h>
#include <X11/decwcursor.h>
#include "smdata.h"
#include "smresource.h"

int	execute_keyboard(changemask)
unsigned    int	changemask;
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	Look at the users settings for keyboard customization and make
**	the correct calls to xlib to set the keyboard as the user
**	desires.   The mask parameter will indicate which parts of the
**	keyboard have changed since the last call to this routine.
**
**  FORMAL PARAMETERS:
**
**	changemask - A bit mask.  One bit per customize settting.  If set
**		     to 1, then change the setting, otherwise skip the call
**		     to change the setting.
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
XKeyboardControl	keyboard;
unsigned	long	keymask = 0;
unsigned    int	i,mode;

/* Check if we need to change the bell volume.  They could have changed
   the bell volume by either hitting enable/disable or by moving the
   slider */
if (((changemask & mbell) != 0) || ((changemask & mbellvolume) != 0))
    { 
    /* Check if bell enabled */
    if (keysetup.bell)
	{
	 keyboard.bell_percent = keysetup.bell_volume;
        }
    else
	/* Otherwise, set the volume to zero */
	{
	keyboard.bell_percent = 0;
	}
    /* The Xlib changekeyboard call takes a mask.  Set the bell bit */
    keymask = keymask | KBBellPercent;
    }

/* Check if we need to change the keyclick volume.  They could have changed
   the volume by either hitting enable/disable or by moving the
   slider */
if (((changemask & mclick) != 0) || ((changemask & mclickvolume) != 0))
    { 
    /* Check the enable switch */
    if (keysetup.click)
	{
	keyboard.key_click_percent = keysetup.click_volume;
	}
    else
	{
	keyboard.key_click_percent = 0;
	}
    /* Set the Xlib bit mask for changekeyboard call */
    keymask = keymask | KBKeyClickPercent;
    }

/* get auto repeatmode */
if ((changemask & mautorepeat) != 0)
    {
    /* they turned it off */
    if (keysetup.autorepeat == 0)
	{
	keyboard.auto_repeat_mode = AutoRepeatModeOff;
	}
    else
	/* they turned it on */
	{
	keyboard.auto_repeat_mode = AutoRepeatModeOn;
	}
    keymask = keymask | KBAutoRepeatMode;
    }

/* Make the Xlib call to set bell, keyclick and autorepeat */
if (keymask != 0)
	XChangeKeyboardControl(display_id, keymask, &keyboard);

/* set keyboard to correct dialect */
if ((changemask & mkeyboard) != 0)
    {
      if (!strcmp(keysetup.keyboard, "System Default")) {
	static Atom DEC_PROMPTER_KEYBOARD = None;
	Atom actual_type;
	int actual_format;
	unsigned long leftover;
	unsigned long nitems;
	char *prompter_keyboard;
	
	if (DEC_PROMPTER_KEYBOARD == None) {
	  DEC_PROMPTER_KEYBOARD = XInternAtom(display_id,
					      "DEC_PROMPTER_KEYBOARD",
					      FALSE);
	}
	if (XGetWindowProperty(display_id,
			 RootWindowOfScreen(XtScreen(smdata.toplevel)),
			 DEC_PROMPTER_KEYBOARD, 0L, 256L,
			 False, DEC_PROMPTER_KEYBOARD,
			 &actual_type, &actual_format, &nitems,
			 &leftover,
			 (unsigned char **) &prompter_keyboard) == Success) {
	  if (prompter_keyboard && strlen(prompter_keyboard)) {
	    strcpy(keysetup.keyboard, prompter_keyboard);
	    if (keysetup.managed == ismanaged) {
	      keyattr_set_values();
	    }
	  }
	  XFree(prompter_keyboard);
	}
      }
    DXLoadKeyboardMap (display_id, keysetup.keyboard);
    /* this could reset the lock state so make sure we test that */
    changemask = changemask | mlockstate;
    }

if ((changemask & mlockstate) != 0)
    {
    if (keysetup.lock == caps_value)
	{
	i = XK_Caps_Lock;
	}
    else
	{
	i = XK_Shift_Lock;
	}
    /* Get the current lock state and compare it to what we want */
    mode = DXGetKeyboardLockMode(display_id);
    if (mode != i)
	DXSetKeyboardLockMode(display_id, i);
    }
}

int	execute_pointer(changemask)
unsigned    int	changemask;
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**
**	Look at the users settings for pointer customization and make
**	the correct calls to xlib to set the pointer as the user
**	desires.   The mask parameter will indicate which parts of the
**	pointer have changed since the last call to this routine.
**
**  FORMAL PARAMETERS:
**
**	changemask - A bit mask.  One bit per customize settting.  If set
**		     to 1, then change the setting, otherwise skip the call
**		     to change the setting.
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
XColor	foreground_color;
XColor	background_color;
unsigned    int	status, i, numerator, denominator, threshold;
unsigned    char	map[4], temp;
unsigned    int	nmaps = 4;
unsigned    int	already_colored = 0;
Window	windo;
int index;
int numscreens;

/* Get the number of screens on the system */
numscreens = XScreenCount(display_id);
if (current_cursor == 0)
    {
    current_cursor = (Cursor *) XtMalloc(numscreens * sizeof(Cursor));
    for (i=0; i<numscreens; i++)
	current_cursor[i] = 0;
    }

/** Cursor shape **/
if ((changemask & mptcursor) != 0)
    {
    Window	*wmrootlist = 0;

    /* For ultrix, will combine the two lists */
    /* MIT will be positive and decwindows willl be negative. */

    /* If the index is <0, then we are going to user the XLIB MIT cursors.
       They can only select these by editing their xdefaults file.   Normally
       the index will be positive and we will use the DEC cursor font */
    index = pointsetup.cursor_index;
    /* make sure it is in legal range */
    if (pointsetup.cursor_index < 0) 
	    if	(abs(pointsetup.cursor_index) > decw$c_num_glyphs)
			/* need to change to abs if negative A.R. */
		    index = decw$c_leftselect_cursor;
    if (pointsetup.cursor_index >= 0) 
	    if (pointsetup.cursor_index > XC_num_glyphs)
		    index = decw$c_leftselect_cursor;
    /* Get rid of old cursor */
    /* For each screen, get rid of cursor*/
    wmrootlist = (Window *) XtMalloc(sizeof(Window)*numscreens);
    for (i=0; i < numscreens; i++)
	{
	Window	root,wmroot;

	root = XRootWindow(display_id, i);
	wmroot = WmRootWindow(display_id,root);
	wmrootlist[i] = WmRootWindow(display_id,wmroot);
	if (current_cursor[i] != 0)    
		{
		XUndefineCursor(display_id, XRootWindow(display_id, i));
		XUndefineCursor(display_id, wmrootlist[i]);
		XFreeCursor(display_id, current_cursor[i]);
		current_cursor[i] = 0;
		}
 	}
    /* Handle normal case first.  DEC cursor font */
    if (index <= 0)
	{
	index = abs(pointsetup.cursor_index);
	/* Load the font */
	if (cursor_font == 0)
	    cursor_font = XLoadFont(display_id, "decw$cursor"); 
	/* Get the user settable foreground and background color */
	for (i=0; i < numscreens; i++)
	    {
	    status = get_pointer_color(&foreground_color, 
					&background_color, i);
	    if ((cursor_font != NULL) && (status != 0))
		    {
		    current_cursor[i] = XCreateGlyphCursor(display_id, cursor_font,
			cursor_font, 
			index, index + 1, 
			&foreground_color, &background_color);
		    already_colored = 1;
		    }	    
	    }
	}
    else
	{
	for (i=0; i < numscreens; i++)
	    {
	    current_cursor[i] = XCreateFontCursor(display_id, index); 
	    status = get_pointer_color(&foreground_color, 
					&background_color, i);
	    /* recolor cursor */
	    if (status != 0)
		{
		XRecolorCursor(display_id, current_cursor[i], 
			    &foreground_color, &background_color);
		}
	    }
	already_colored = 1;
	}
    /* Set the cursor over the root window and window manager pseudo 
       root window */
    for (i=0; i < numscreens; i++)
	{
	XDefineCursor(display_id, XRootWindow(display_id, i), 
				    current_cursor[i]);
	}
    XtFree((char *)wmrootlist);
    }

/** Cursor color **/
if (((changemask & mptforeground) != 0) ||((changemask & mptbackground) != 0))
    { 
    for (i=0; i < numscreens; i++)
	{
        /* We may have already set the color if they changed the shape */
	if ((current_cursor[i] != 0) && (already_colored == 0))
	    {
	    /* now recolor it */
	    status = get_pointer_color(&foreground_color, 
					&background_color, i);
	    if (status != 0)
		{
		XRecolorCursor(display_id, current_cursor[i], &foreground_color, 
			&background_color);
		}
	    }
	 }
    }

/** left-handed/right handed **/
if ((changemask & mptbutton) != 0)
    {
    for (i = 0; i<nmaps; i++)
	map[i] = 0;

    /* We can get X errors if we set more or less buttons than actually
       exist on the mouse.  Get the number of buttons first */
    status = XGetPointerMapping(display_id, map, nmaps);
    if (pointsetup.button == right_handed)
	{
	/* Set them 1,2,3 */
	for (i=0; i<status; i++)
	    map[i] = i + 1;
        }
    else
	{
	/* Set them 3,2,1 */
/*  Doesn't work with 4 button puck. where north and south stay constant.
	for (i=0; i<status; i++)
	    map[i] = status - i;
	}
*/
	  temp = map[0];
	  map[0] = map[2];
	  map[2] = temp;
	}
    i = XSetPointerMapping(display_id, map, status);
    }

/** pointer acceleration **/

if ((changemask & mptaccel) != 0)
    {
    XChangePointerControl(display_id, 1, 1, pointsetup.accel_num, 
	    pointsetup.accel_denom,
	    pointsetup.accel_threshold);
    }
}

int	get_pointer_color(foreground_color, background_color, screennum)
XColor	*foreground_color;
XColor	*background_color;
int screennum;
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	Get the current customization color for pointer foreground
**	and background.
**
**  FORMAL PARAMETERS:
**
**	foreground_color - A pointer to a place to store the foreground
**			   color.
**	background_color - A pointer to a place to store the background
**			   color.
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  COMPLETION CODES:
**
**  SIDE EFFECTS:
**--
**/
{
unsigned    int	status,type;
struct	color_data  foreground;
struct	color_data  background;


type = determine_system_color(display_id, screennum);
if (type == system_color_type)
	{
	bcopy(&pointsetup.foreground, &foreground, sizeof(struct color_data));
	bcopy(&pointsetup.background, &background, sizeof(struct color_data));
	}
else
	{
	get_color_screen_resource(&foreground, iptrforeground,
			    screennum, type);
	get_color_screen_resource(&background, iptrbackground,
			    screennum,type);
	}

if (foreground.by_name == 1)
    {
    /* Color is stored by a name string so call parsecolor to get
       the rgb value */
    status = XParseColor(display_id, XDefaultColormap(display_id, screennum),
		    foreground.name, foreground_color);
    if (status == 0)
	{
	return(0);
	}
    }
else
    /* Color is stored as RGB, just copy it into return storage */
    {
    bcopy(&foreground.color, foreground_color, sizeof(XColor));
    }

if (background.by_name == 1)
    {
    /* Color is stored by a name string so call parsecolor to get
       the rgb value */
    status = XParseColor(display_id, XDefaultColormap(display_id, screennum),
		    background.name, background_color);
    if (status == 0)
	{
	return(0);
	}
    }
else
    /* Color is stored as RGB, just copy it into return storage */
    {
    bcopy(&background.color, background_color, sizeof(XColor));
    }
return(1);
}
