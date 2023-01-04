/*
*****************************************************************************
**                                                                          *
**  COPYRIGHT (c) 1988, 1989, 1991, 1992 BY                                 *
**  DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASSACHUSETTS.                  *
**  ALL RIGHTS RESERVED.                                                    *
**                                                                          *
**  THIS SOFTWARE IS FURNISHED UNDER A LICENSE AND MAY BE USED AND COPIED   *
**  ONLY IN  ACCORDANCE WITH  THE  TERMS  OF  SUCH  LICENSE  AND WITH THE   *
**  INCLUSION OF THE ABOVE COPYRIGHT NOTICE. THIS SOFTWARE OR  ANY  OTHER   *
**  COPIES THEREOF MAY NOT BE PROVIDED OR OTHERWISE MADE AVAILABLE TO ANY   *
**  OTHER PERSON.  NO TITLE TO AND OWNERSHIP OF  THE  SOFTWARE IS  HEREBY   *
**  TRANSFERRED.                                                            *
**                                                                          *
**  THE INFORMATION IN THIS SOFTWARE IS  SUBJECT TO CHANGE WITHOUT NOTICE   *
**  AND  SHOULD  NOT  BE  CONSTRUED AS  A COMMITMENT BY DIGITAL EQUIPMENT   *
**  CORPORATION.                                                            *
**                                                                          *
**  DIGITAL ASSUMES NO RESPONSIBILITY FOR THE USE  OR  RELIABILITY OF ITS   *
**  SOFTWARE ON EQUIPMENT WHICH IS NOT SUPPLIED BY DIGITAL.                 *
**                                                                          *
**                                                                          *
*****************************************************************************
**
** FACILITY:  Session
**
** ABSTRACT:
**
**	This module handles the callback from the Session menu
**	This includes Pause and quit.
**
** ENVIRONMENT:
**
**      VAX/VMS operating system.
**
** AUTHOR:  Karen Brouillette March 1988
**
** Modified by:
**
**	05-Apr-1991	Edward P Luwish
**		Port to Motif UI
**
*/

/*
** Include files
*/
#include "iprdw.h"
#include "smdata.h"
#include "smconstants.h"
#include "prdw_entry.h"

void end_session
#if _PRDW_PROTO_
(
    Widget		*widgetID,
    caddr_t		tag,
    XmAnyCallbackStruct	*cbs
)
#else
(widgetID, tag, cbs)
    Widget		*widgetID;
    caddr_t		tag;
    XmAnyCallbackStruct	*cbs;
#endif
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	Depending on customization settings, prompt the user with a
**	confirmation box, or simply end the session.
**
**  FORMAL PARAMETERS:
**
**	widgetID - the end session menu button id
**	tag - nothing for now
**	cbs - callback structure
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
    unsigned int	result;

    /*
    ** if any of the setup features have been changed, but not saved, then
    ** alert the user.
    */

    prt_dialog_get_values();
    prt_put_attrs();
    help_unmap();	/* need dummy variable for compat. with widget */

    if (smdata.resource_changed)
    {
	decw$create_caution(resource_changed_number);
	return;
    }
    else
    {
	/* If the user has the end session confirmation option selected
	   put up the confirmation box */
	if (smsetup.end_confirm != 0)
	{
	    decw$create_caution(end_session_number);
	    return;
	}
    }
    /* This will end the session */
    finish();
    return;
}

void session_menu_cb
#if _PRDW_PROTO_
(
    Widget			*widgetID,
    caddr_t			tag,
    XmRowColumnCallbackStruct	*cbs
)
#else
(widgetID, tag, cbs)
    Widget			*widgetID;
    caddr_t			tag;
    XmRowColumnCallbackStruct	*cbs;
#endif
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	If the function selected was pause session, then call the
**	pause routine, otherwise call the end session code
**
**  FORMAL PARAMETERS:
**
**	widgetID - The menu widget id
**	tag - A pointer not used
**	cbs - A structure which includes the menu item selected.
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

    if (cbs->widget == smdata.quit_button)
    {
	end_session (widgetID, tag, (XmAnyCallbackStruct *)cbs);
	return;
    }

    return;
}
