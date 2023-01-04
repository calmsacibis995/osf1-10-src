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
#include "prdw.h"

extern char     *user_resource;
extern char     *system_resource; 

int	setup_menu_cb(widgetID, tag, reason)
Widget	*widgetID;
caddr_t	tag;
XmRowColumnCallbackStruct	*reason;
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	Handles the Customize menu.  Determines which customization
**	function is being performed.  Then it checks if the dialog
**	is already managers, and if so pops it to the top of the
**	screen.  If it is not manages, it resets its widgets to be
**	the current value and then manages the dialog box.
**
**  FORMAL PARAMETERS:
**
**	widetID - The menu widget id
**	tag - not used
**	reason - A structure which includes info on what menu item
**	         was selected
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
unsigned	int	status;
int	file_id;
Time	time = CurrentTime;

/* Application Definition Customization */
if (reason->widget == smdata.appdefs_button)
	{
    	/* If the box isn't created yet, then create it, set the widgets
           to the correct values and manage it */
 	if (appdefsetup.menu_attr_id == 0)
		{
		/* create the dialog box */
		create_appdef_attrs();
	    	/* set the right button values to on and off for the dialog */
		InitAppDefListboxContents();
		appdefsetup.managed = ismanaged;
		XtManageChild(appdefsetup.menu_attr_id);
		}
	else
	    {
	    /* If the box is currently managed, just pop it to the front */
	    if (XtIsManaged(appdefsetup.menu_attr_id))
		{
		/* it is visible, pop it to the top of the screen */
		XRaiseWindow(display_id, 
		    XtWindow(XtParent(appdefsetup.menu_attr_id)));
		/*
		(* XtCoreProc (appdefsetup.menu_attr_id, accept_focus))
			    (appdefsetup.menu_attr_id, &time);
		*/
		}
	    else
		{
	    	/* set the right button values to on and off for the dialog */
		/* we have to do this call here so that the color cells will
		    be  allocated for the color buttons.  */
		InitAppDefListboxContents();
		appdefsetup.managed = ismanaged;
	    	XtManageChild(appdefsetup.menu_attr_id);
		}
	    }
	return;
	}

/* Application Menu Customization */
if (reason->widget == smdata.appmenu_button)
	{
    	/* If the box isn't created yet, then create it, set the widgets
           to the correct values and manage it */
 	if (appmenusetup.menu_attr_id == 0)
		{
		/* create the dialog box */
		create_appmenu_attrs();
	    	/* set the right button values to on and off for the dialog */
		InitListboxContents();
		appmenusetup.managed = ismanaged;
		XtManageChild(appmenusetup.menu_attr_id);
		}
	else
	    {
	    /* If the box is currently managed, just pop it to the front */
	    if (XtIsManaged(appmenusetup.menu_attr_id))
		{
		/* it is visible, pop it to the top of the screen */
		XRaiseWindow(display_id, 
		    XtWindow(XtParent(appmenusetup.menu_attr_id)));
		/*
		(* XtCoreProc (appmenusetup.menu_attr_id, accept_focus))
			    (appmenusetup.menu_attr_id, &time);
		*/
		}
	    else
		{
	    	/* set the right button values to on and off for the dialog */
		/* we have to do this call here so that the color cells will
		    be  allocated for the color buttons.  */
		InitListboxContents();
		appmenusetup.managed = ismanaged;
	    	XtManageChild(appmenusetup.menu_attr_id);
		}
	    }
	return;
	}

/* Automatic Startup Customization */
if (reason->widget == smdata.autostart_button)
	{
    	/* If the box isn't created yet, then create it, set the widgets
           to the correct values and manage it */
 	if (autostartsetup.menu_attr_id == 0)
		{
		/* create the dialog box */
		create_autostart_attrs();
	    	/* set the right button values to on and off for the dialog */
		InitAutoStartListboxContents();
		autostartsetup.managed = ismanaged;
		XtManageChild(autostartsetup.menu_attr_id);
		}
	else
	    {
	    /* If the box is currently managed, just pop it to the front */
	    if (XtIsManaged(autostartsetup.menu_attr_id))
		{
		/* it is visible, pop it to the top of the screen */
		XRaiseWindow(display_id, 
		    XtWindow(XtParent(autostartsetup.menu_attr_id)));
		/*
		(* XtCoreProc (autostartsetup.menu_attr_id, accept_focus))
			    (autostartsetup.menu_attr_id, &time);
		*/
		}
	    else
		{
	    	/* set the right button values to on and off for the dialog */
		/* we have to do this call here so that the color cells will
		    be  allocated for the color buttons.  */
		InitAutoStartListboxContents();
		autostartsetup.managed = ismanaged;
	    	XtManageChild(autostartsetup.menu_attr_id);
		}
	    }
	return;
	}

/* Customize session manager */
if (reason->widget == smdata.sm_button)
	{
	/* If the box isn't created yet, then create it, set the widgets
           to the correct values and manage it */
	if (smsetup.sm_attr_id == 0)
		{
		/* create the dialog box */
		create_sm_attrs();
	    	/* set the right button values to on and off for the dialog */
		smattr_set_values();
		smsetup.managed = ismanaged;
		XtManageChild(smsetup.sm_attr_id);
		}
	else
	    {
	    /* If the box is currently managed, just pop it to the front */
	    if (XtIsManaged(smsetup.sm_attr_id))
		{
		/* it is visible, pop it to the top of the screen */
		XRaiseWindow(display_id, 
		    XtWindow(XtParent(smsetup.sm_attr_id)));
		/*
		(* XtCoreProc (smsetup.sm_attr_id, accept_focus))
			    (smsetup.sm_attr_id, &time);
		*/
		}
	    else
		{
	    	/* set the right button values to on and off for the dialog 
		   and manage it */
		smattr_set_values();
		smsetup.managed = ismanaged;
	    	XtManageChild(smsetup.sm_attr_id);
		}
	    }
	return;
	}

#ifdef DOPRINT
/* Printscreen Customization */
if (reason->widget == smdata.printer_button)
	{
	/* If the box isn't created yet, then create it, set the widgets
           to the correct values and manage it */
	if (prtsetup.prt_attr_id == 0)
		{
		/* create the dialog box */
		create_print_attrs();
		prtattr_set_values();
		prtsetup.managed = ismanaged;
		XtManageChild(prtsetup.prt_attr_id);
		}
	else
	    {
	    /* If the box is currently managed, just pop it to the front */
	    if (XtIsManaged(prtsetup.prt_attr_id))
		{
		/* it is visible, pop it to the top of the screen */
		XRaiseWindow(display_id, 
		    XtWindow(XtParent(prtsetup.prt_attr_id)));
		/*
		(* XtCoreProc (prtsetup.prt_attr_id, accept_focus))
			    (prtsetup.prt_attr_id, &time);
		*/
		}
	    else
		{
	    	/* set the right button values to on and off for the dialog 
		   and manage it */
		prtattr_set_values();
		prtsetup.managed = ismanaged;
	    	XtManageChild(prtsetup.prt_attr_id);
		}
	    }
	return;
	}
#endif /* DOPRINT */

/* Keyboard Customization */
if (reason->widget == smdata.key_button)
	{
	/* If the box isn't created yet, then create it, set the widgets
           to the correct values and manage it */
	if (keysetup.key_attr_id == 0)
		{
		/* create the dialog box */
		create_keyboard_attrs();
		keyattr_set_values();
		keysetup.managed = ismanaged;
		XtManageChild(keysetup.key_attr_id);
		}
	else
	    {
	    /* If the box is currently managed, just pop it to the front */
	    if (XtIsManaged(keysetup.key_attr_id))
		{
		/* it is visible, pop it to the top of the screen */
		XRaiseWindow(display_id, 
		    XtWindow(XtParent(keysetup.key_attr_id)));
		/*
		(* XtCoreProc (keysetup.key_attr_id, accept_focus))
			    (keysetup.key_attr_id, &time);
		*/
		}
	    else
	    	/* set the right button values to on and off for the dialog 
		   and manage it */
		{
		/* create the list box */
		keyboard_listbox_setcontents();
		keyattr_set_values();
		keysetup.managed = ismanaged;
	    	XtManageChild(keysetup.key_attr_id);
		}
	    }
	return;
	}

/**International button**/
if (reason->widget == smdata.international_button)
	{
	if (intersetup.inter_attr_id == 0)
		{
		/* create the dialog box */
		create_inter_attrs();
		/* set the right button values to on and off for the dialog */
		interattr_set_values();
		intersetup.managed = ismanaged;
		XtManageChild(intersetup.inter_attr_id);
		}
	else
	    {
	    if (XtIsManaged(intersetup.inter_attr_id))
		{
		/* it is visible, pop it to the top of the screen */
		XRaiseWindow(display_id, 
		    XtWindow(XtParent(intersetup.inter_attr_id)));
		/*
		(* XtCoreProc (intersetup.inter_attr_id, accept_focus))
			    (intersetup.inter_attr_id, &time);
		*/
		}
	    else
		{
		/* set the right button values to on and off for the dialog */
		interattr_set_values();
		intersetup.managed = ismanaged;
	    	XtManageChild(intersetup.inter_attr_id);
		}
	    }
	return;
	}
/* Pointer Customization */
if (reason->widget == smdata.pointer_button)
	{
	/* If the box isn't created yet, then create it, set the widgets
           to the correct values and manage it */
 	if (pointsetup.point_attr_id == 0)
		{
		/* create the dialog box */
		create_point_attrs();
	    	/* set the right button values to on and off for the dialog */
		pointattr_set_values();
		pointsetup.managed = ismanaged;
		XtManageChild(pointsetup.point_attr_id);
		}
	else
	    {
	    /* If the box is currently managed, just pop it to the front */
	    if (XtIsManaged(pointsetup.point_attr_id))
		{
		/* it is visible, pop it to the top of the screen */
		XRaiseWindow(display_id, 
		    XtWindow(XtParent(pointsetup.point_attr_id)));
		/*
		(* XtCoreProc (pointsetup.point_attr_id, accept_focus))
			    (pointsetup.point_attr_id, &time);
		*/
		}
	    else
		{
	    	/* set the right button values to on and off for the dialog */
		/* we have to do this call here so that the color cells will
		    be  allocated for the color buttons.  */
		pointattr_set_values();
		pointsetup.managed = ismanaged;
	    	XtManageChild(pointsetup.point_attr_id);
		}
	    }
	return;
	}

/* Window Customization */
if (reason->widget == smdata.window_button)
	{
	/* If the box isn't created yet, then create it, set the widgets
           to the correct values and manage it */
	if (windowsetup.window_attr_id == 0)
		{
		/* create the dialog box */
		create_window_attrs();
		/* set the right button values to on and off for the dialog */
		windowattr_set_values();
		windowsetup.managed = ismanaged;
		XtManageChild(windowsetup.window_attr_id);
		}
	else
	    {
	    /* If the box is currently managed, just pop it to the front */
	    if (XtIsManaged(windowsetup.window_attr_id))
		{
		/* it is visible, pop it to the top of the screen */
		XRaiseWindow(display_id, 
		    XtWindow(XtParent(windowsetup.window_attr_id)));
		/*
		(* XtCoreProc (windowsetup.window_attr_id, accept_focus))
			    (windowsetup.window_attr_id, &time);
		*/
		}
	    else
		{
	    	/* set the right button values to on and off for the dialog 
		   and manage it */
		/* we have to do this call here so that the color cells will
		    be  allocated for the color buttons.  */
		windowattr_set_values();
		windowsetup.managed = ismanaged;
	    	XtManageChild(windowsetup.window_attr_id);
		}
	    }
	return;
	}

/* security pick */
if (reason->widget == smdata.security_button)
    {
	/* If the box isn't created yet, then create it, set the widgets
           to the correct values and manage it */
    if (securitysetup.sec_attr_id == 0)
	{
	create_security_attrs();
	securitysetup.managed = ismanaged;
	XtManageChild(securitysetup.sec_attr_id);
	}
    else
	{
	  SetListboxContents();
	    /* If the box is currently managed, just pop it to the front */
	if (XtIsManaged(securitysetup.sec_attr_id))
		{
		/* it is visible, pop it to the top of the screen */
		XRaiseWindow(display_id, 
		    XtWindow(XtParent(securitysetup.sec_attr_id)));
		/*
		(* XtCoreProc (securitysetup.sec_attr_id, accept_focus))
			    (securitysetup.sec_attr_id, &time);
		*/
		}
	else
		{
	    	/* set the right button values to on and off for the dialog 
		   and manage it */
		securitysetup.managed = ismanaged;
	    	XtManageChild(securitysetup.sec_attr_id);
		}
	}
    return;
    }

if (reason->widget == smdata.screen_button)
    {
	/* If the box isn't created yet, then create it, set the widgets
           to the correct values and manage it */
    if (screensetup.scr_attr_id == 0)
	{
	create_screen_attrs();
	screensetup.managed = ismanaged;
	XtManageChild(screensetup.scr_attr_id);
	/* set the right button values to on and off for the dialog */
	screenattr_set_values();
	}
    else
	{ 
	    /* If the box is currently managed, just pop it to the front */
	if (XtIsManaged(screensetup.scr_attr_id))
		{
		/* it is visible, pop it to the top of the screen */
		XRaiseWindow(display_id, 
		    XtWindow(XtParent(screensetup.scr_attr_id)));
		/*
		(* XtCoreProc (screensetup.scr_attr_id, accept_focus))
			    (screensetup.scr_attr_id, &time);
		*/
		}
	else
		{
	    	/* set the right button values to on and off for the dialog 
		   and manage it */
		screensetup.managed = ismanaged;
	    	XtManageChild(screensetup.scr_attr_id);
		}
	}
    return;
    }

/* Use last saved settings */
if (reason->widget == smdata.use_last_button)
	{
	display_drm_message(0, k_resource_change_msg);
	
	/* Read in the files and reset the resource database */
        status = sm_switch_database(user_resource);
	if (status == 0) {
		/* Couldn't read in user's files, try system files */
		/* PAC status = sm_use_managers(XtDisplay(smdata.toplevel)); */
        	status = sm_switch_database(system_resource);
		if (!status) return;
	}

	/* Mark that resources have changed since the last save */
	smdata.resource_changed = 0;

	/* make the X calls to set up the keyboard according to attributes */
	execute_keyboard((unsigned int)mkeyboard_mask);

	/* make the X calls to set up the pointer according to attributes */
	execute_pointer((unsigned int)mpt_mask);

	/* make the X calls to set up the display according to attributes */
	execute_display((unsigned int)mdisplay_mask);

	/* reset the control panel to be the correct size */
	set_control_size();

#ifdef DOPRINT
	if (prtsetup.format != DECWC_PRSC_DDIF) {
	  XtSetSensitive(smdata.print_es, TRUE);    
	  XtSetSensitive(smdata.print_pos, TRUE);    
	} else {
	  XtSetSensitive(smdata.print_es, FALSE);    
	  XtSetSensitive(smdata.print_pos, FALSE);    
	}
#endif

	return;
	}

/* Use system defaults */
if (reason->widget == smdata.use_system_button)
	{
	/* Read in the system session manager resource files */
	/* PAC status = sm_use_managers(XtDisplay(smdata.toplevel)); */
        status = sm_switch_database(system_resource);

	if (!status) return;

	/* using resources which are not saved now.  So set the flag */
	smdata.resource_changed = 1;

	/* make the X calls to set up the keyboard according to attributes */
	execute_keyboard((unsigned int)mkeyboard_mask);

	/* make the X calls to set up the pointer according to attributes */
	execute_pointer((unsigned int)mpt_mask);

	/* make the X calls to set up the display according to attributes */
	execute_display((unsigned int)mdisplay_mask);

	/* reset the control panel to be the correct size */
	set_control_size();

#ifdef DOPRINT
	if (prtsetup.format != DECWC_PRSC_DDIF) {
	  XtSetSensitive(smdata.print_es, TRUE);    
	  XtSetSensitive(smdata.print_pos, TRUE);    
	} else {
	  XtSetSensitive(smdata.print_es, FALSE);    
	  XtSetSensitive(smdata.print_pos, FALSE);    
	}
#endif

	return;
	}

/* Save current settings */
if (reason->widget == smdata.save_current_button)
	{
	/* check if the control panel has moved */
	move_event();
	/* If nothing has changed, don't bother writing it out */
	if (!smdata.resource_changed)
		{
		display_drm_message(0, k_resource_nosave_msg);
		}
	else
		{
		/* Save the database in various files */
		status = sm_save_database();
		if (status == 1)
		    display_drm_message(0,k_resource_save_msg );
		}
	}
}
