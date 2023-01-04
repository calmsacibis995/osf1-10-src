/*
*****************************************************************************
**                                                                          *
**  COPYRIGHT (c) 1988, 1989, 1991, 1992 BY                                       *
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
**	This module handles the callback from the Help menu
**
** ENVIRONMENT:
**
**      VAX/VMS operating system.
**
** AUTHOR:  Jake VanNoy, 	July 1988
**
** Modified by:
**
**	 3-Apr-1992	Edward P Luwish
**		Convert to HyperHelp
**
**	10-Apr-1991	Edward P Luwish
**		Port to Motif UI, add context-sensitive help
**
**	21-MAY-1989	Karen Brouillette
**	    Set the topic to overview if necessary.   This was to
**	    avoid the window coming up blank if overview was selected
**	02-MAY-1989	Karen Brouillette
**	    Ultrix changes
**      28-APR-1989     Karen Brouillette
**          New format for modification history.
**	17-FEB-1989	Karen Brouillette
**	    Convert to UIL
**	01-31-1989	Karen Brouillette
**	    Update copyright.  Add module headers
**
*/

/*
** Include files
*/
#include "iprdw.h"
#include "smdata.h"
#include "smconstants.h"
#include "prdw_entry.h"

static Opaque	help_context=0;
static char	libname[256];

static void create_help PROTOTYPE ((
    char	*topic
));

static void help_error PROTOTYPE ((
    char	*message,
    int		status
));

void help_menu_cb
#if _PRDW_PROTO_
(
    Widget			*widgetID,
    int				*tag,
    XmRowColumnCallbackStruct	*any
)
#else
(widgetID, tag, any)
    Widget			*widgetID;
    int				*tag;
    XmRowColumnCallbackStruct	*any;
#endif
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**  This is the callback routine when the user selects an entry on the HELP
**  pulldown menu.  It creates and manages the help widget and selects the
**  first topic to be displayed to the user.
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
*/
{
    char	*topic;
    int		help_code;

    /*
    ** If the help widget is already being displayed, this routine simply
    ** pops the new topic into the existing widget.
    */
    wait_cursor(1);

    topic = NULL;
    help_code = *tag;

    switch (help_code)
    {
    case k_help_on_window_value:
	topic = CSToLatin1(get_drm_message(k_help_window_msg));
	create_help(topic);
	break;

    case k_help_on_version_value:
	topic = CSToLatin1(get_drm_message(k_help_version_msg));
	create_help(topic);
	break;

    case k_help_on_help_value:
	topic = CSToLatin1(get_drm_message(k_help_on_help_msg));
	create_help(topic);
	break;

    case k_help_on_context_value:
	DXmHelpOnContext(smdata.toplevel, FALSE);
	break;
    }

    if (topic != NULL) XtFree ((char *)topic);

    wait_cursor (0);
    return;
}

void help_unmap ()
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	This routine is called when the user selects "Exit" in the help widget. 
**	just delete the widget.  If the user requests help again, the widget 
**	will be recreated from scratch.    
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
**	We should probably be re-using the help widget.
**--
*/
{
    help_context = (Opaque)smdata.help_widget;
    if (help_context != (Opaque)0)
	DXmHelpSystemClose(help_context, help_error, "Help system close error");
    smdata.help_widget = NULL;
}

void sens_help_proc
#if _PRDW_PROTO_
(
    Widget		*w,
    char		*tag,
    XmAnyCallbackStruct	*any
)
#else
(w, tag, any)
    Widget		*w;
    char		*tag;
    XmAnyCallbackStruct	*any;
#endif
{
    wait_cursor(1);
    create_help(tag);
    wait_cursor(0);
}

static void create_help
#if _PRDW_PROTO_
(
    char	*topic
)
#else
(topic)
    char	*topic;
#endif
{
    char	*libname;

#ifdef vms
    libname = CSToLatin1(get_drm_message (k_help_vms_msg));
#else
    libname = CSToLatin1(get_drm_message (k_help_ultrix_msg));
#endif /* vms */

    if (help_context == (Opaque)0)
    {
	DXmHelpSystemOpen
	(
	    &help_context,
	    smdata.toplevel,
	    libname,
	    help_error,
	    "Help system open error"
	);
    }

    DXmHelpSystemDisplay
    (
	help_context,
	libname,
	"topic",
	topic,
	help_error,
	"Help system display error"
    );
    smdata.help_widget = (Widget)help_context;

}

static void help_error
#if _PRDW_PROTO_
(
    char	*message,
    int		status
)
#else
(message, status)
    char	*message;
    int		status;
#endif
{
    printf("%s, %x\n", message, status);
    return;
}
