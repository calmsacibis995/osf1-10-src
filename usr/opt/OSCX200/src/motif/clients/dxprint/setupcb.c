/*
*****************************************************************************
**									    *
**  COPYRIGHT (c) 1988, 1989, 1991, 1992 BY				    *
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
**++
** FACILITY:  PrintScreen
**
** ABSTRACT:
**
**	This module handles the callback from the Customize menu
**
** ENVIRONMENT:
**
**	VAX/VMS operating system.
**
** AUTHOR:  Karen Brouillette October 1989
**
** Modified by:
**
**	07-Apr-1991	Edward P Luwish
**		Port to Motif UI
**
**    25 Oct 1990	KMR		Kathy Robinson
**		If we can't open any defaults file, pop up an error
**
*/

/*
** Include files
*/
#include "iprdw.h"
#include "smdata.h"
#include "smconstants.h"
#include "smresource.h"
#include "prdw_entry.h"

void setup_menu_cb
#if _PRDW_PROTO_
(
    Widget			widgetID,
    int				tag,
    XmRowColumnCallbackStruct	*reason
)
#else
(widgetID, tag, reason)
    Widget			widgetID;
    int				tag;
    XmRowColumnCallbackStruct	*reason;
#endif
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
*/
{
    unsigned int	status;
    int			file_id;
    XmString		error_message;
    Widget		widget;

    prt_dialog_get_values();
    prt_put_attrs();

    /* OK callback from PostScript Options */
    if (widgetID == prtsetup.page_size_OK_id)
    {
	XtUnmanageChild(smdata.pagesize_panel);
    }

    /* OK callback from Sixel Options */
    else if (widgetID == prtsetup.sixel_OK_id)
    {
	XtUnmanageChild(smdata.sixel_panel);
    }

    /* PostScript Options (mostly page size) */
    else if (reason->widget == smdata.postopbutton)
    {
	XtManageChild(smdata.pagesize_panel);
	XtRealizeWidget(smdata.pagesize_panel);
    }

    /* Sixel Options (mostly device names) */
    else if (reason->widget == smdata.sixopbutton)
    {
	XtManageChild(smdata.sixel_panel);
	XtRealizeWidget(smdata.sixel_panel);
    }

    /* Use last saved settings */
    else if (reason->widget == smdata.use_last_button)
    {
    	wait_cursor(1);
	/* Read in the files and reset the resource database */
	status = sm_switch_database(XtDisplay(smdata.toplevel));
	if (!status)
	{
	    /* we need an error message here */
	    error_message = (XmString)get_drm_message (3);
	    decw$error_display(error_message);
	    wait_cursor(0);
	    return;
	}

	/* Mark that resources have changed since the last save */
	smdata.resource_changed = 0;
        /* reset the control panel to be the correct size */
        set_control_size();

    	wait_cursor(0);
	return;
    }

    /* Use system defaults */
    else if (reason->widget == smdata.use_system_button)
    {
    	wait_cursor(1);
	/* Read in the system session manager resource files */
	status = sm_use_managers(XtDisplay(smdata.toplevel));
	if (!status)
	{
	    wait_cursor(0);
	    return;
	}

	/* using resources which are not saved now.  So set the flag */
	smdata.resource_changed = 1;
        /* reset the control panel to be the correct size */
        set_control_size();
    	wait_cursor(0);
	return;
    }

    /* Save current settings */
    else if (reason->widget == smdata.save_current_button)
    {
    	wait_cursor(1);
	/* check if the control panel has moved */
	move_event();

	/* Save the database in various files */
	status = sm_save_database(XtDisplay(smdata.toplevel));

    	wait_cursor(0);
	return;
    }
    return;
}

void page_size_cancel
#if _PRDW_PROTO_
(
    Widget			widgetID,
    int				tag,
    XmPushButtonCallbackStruct	*reason
)
#else
(widgetID, tag, reason)
    Widget			widgetID;
    int				tag;
    XmPushButtonCallbackStruct	*reason;
#endif
{
	reset_page_size();
	XtUnmanageChild(smdata.pagesize_panel);
}

void sixel_device_cancel
#if _PRDW_PROTO_
(
    Widget			widgetID,
    int				tag,
    XmPushButtonCallbackStruct	*reason
)
#else
(widgetID, tag, reason)
    Widget			widgetID;
    int				tag;
    XmPushButtonCallbackStruct	*reason;
#endif
{
	reset_sixel_printer();
	XtUnmanageChild(smdata.sixel_panel);
}

void updatesetup ()
{
    /* update the structure for printer attributes */
    prtattr_get_values();
    if (prtsetup.postscript_id != 0)
    {
	prtattr_set_values();
    }


    /* update the structure for session manager attributes */
    smattr_get_values();

    return;
}

