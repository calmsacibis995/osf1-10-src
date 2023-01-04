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
**
** FACILITY:  PrintScreen
**
** ABSTRACT:
**
**	This module handles the callback from the printscreen menu
**
** ENVIRONMENT:
**
**	VAX/VMS operating system.
**
** AUTHOR:  Karen Brouillette October 1989
**
** Modified by:
**
**	10-Aug-1992	Edward P Luwish
**		Removed file-prompt code.
**		Restored screen-prompt code.
**		Changed message-passing code so it no longer assumes that
**		sizeof(int) == sizeof(int *).
**
**	29-Jan-1992	Edward P Luwish
**		Numerous changes to eliminate option item list in favor of
**		global command structure.  Most of the control flow has moved
**		into main().
**
**	04-Apr-1991	Edward P Luwish
**		Port to Motif UI
**
*/

/*
** Include files
*/
#include "iprdw.h"
#include "smdata.h"
#include "smconstants.h"
#include "smshare.h"

#ifdef VMS
#include <decw$cursor.h>
#else
#include <X11/decwcursor.h>
#endif

#include "smresource.h"

#if defined(VMS) && !defined(__DECC)
#pragma nostandard
#endif
#include <Xm/ToggleB.h>
#if defined(VMS) && !defined(__DECC)
#pragma standard
#endif

#include "prdw_entry.h"

static	int screennum;
static	int both_prompts;

static void create_options_list PROTOTYPE((void));

static void print_screen_error PROTOTYPE((long status, int *message));

static void confirm_file PROTOTYPE((void));

static int create_screen_confirm PROTOTYPE((void));

static void confirm_screen_action PROTOTYPE((
    Widget		widget,
    int			*tag,
    XmAnyCallbackStruct	*any
));

static void confirm_screen_cancel PROTOTYPE((
    Widget		widget,
    caddr_t		tag,
    XmAnyCallbackStruct	*any
));

void print_menu_cb
#if _PRDW_PROTO_
(
    Widget			*widgetID,
    caddr_t			tag,
    XmRowColumnCallbackStruct	*any
)
#else
(widgetID, tag, any)
    Widget			*widgetID;
    caddr_t			tag;
    XmRowColumnCallbackStruct	*any;
#endif
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	Callback when the printscreen menu is selected.   There are four
**	options from the printscreen menu.   If we are printing to
**	a printer, send ourselves a client message right away.  Otherwise,
**	prompt for a file name and then send a client message.  Sending
**	a client message allows all of the exposes to be processes before
**	the printscreen starts.  This is important so that the area underneath
**	the pulldown menu is redrawn before the print is executed.
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
    if (*tag == 1)
	prtsetup.print_widget = dxPrscPrintWidget;
    else
	prtsetup.print_widget = dxPrscQuickPrint;

    /*
    ** First get the latest button settings from the dialog box
    */
    prt_dialog_get_values();

    create_options_list ();
    /*
    ** Are we multi-screen?
    */
    if (ScreenCount(display_id) > 1)
    {
	/*
	** Create the screen selection box with the default value set.
	*/
	if (smdata.screen_confirm_box_id == NULL)
	    create_screen_confirm();

	/*
	** If we are requested to, actually put it on the screen.
	** Else, do the print.
	*/
	if (prtsetup.print_widget == dxPrscPrintWidget)
	{
	    XtManageChild(smdata.screen_confirm_box_id);
	}
	else
	{
	    if (prtsetup.send_to_printer != dxPrscFile)
		getqueue (NULL, smdata.toplevel);
	    timed_capture_screen ();
	}
	return;
    }

    screennum = 0;
    screensetup.prt_screennum = 0;

    if (prtsetup.send_to_printer != dxPrscFile)
    {
	/*
	** We need to either display or initialize the print widget.
	*/
	getqueue (NULL, smdata.toplevel);
	/*
	** If we haven't displayed the print widget go do the capture.
	** If we have displayed the print widget, the capture will be
	** done in the callback.
	*/
	if (prtsetup.print_widget == dxPrscQuickPrint)
	    timed_capture_screen ();
    }
    else
    {
	/*
	** We are going to a file only and thus have no interaction with
	** the printWidget.
	*/
	timed_capture_screen ();
    }

    return;
}

#if defined (XtIsRealized)
#undef XtIsRealized
#define XtIsRealized(object) (XtWindowOfObject(object) != 0)
#endif

int timed_capture_screen ()
{
    /*
    ** Determine whether and how to do the timer.  If we are not in -X mode,
    ** then we use sleep.  Else we use XtAppAddTimeout.
    */
#if 0
    if (prtsetup.delay > 0)
    {
#endif
	extern XtAppContext applicationContext;

	if (smdata.toplevel != NULL)
	    if (XtIsRealized((Widget)smdata.toplevel))
		XtAppAddTimeOut
		(
		    applicationContext,
		    (unsigned long) (prtsetup.delay * 1000),
		    (XtTimerCallbackProc) do_capture_screen,
		    (XtPointer) NULL
		);
	    else
		sleep(prtsetup.delay);
	else
	    sleep(prtsetup.delay);
#if 0
    }
    else
    {
	do_capture_screen ();
    }
#endif
}

int do_capture_screen
#if _PRDW_PROTO_
(
)
#else
()
#endif
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	This routine is called when we receive the client message.
**	It actually calls into the printscreen code to capture the
**	bits of the screen.
**
**  FORMAL PARAMETERS:
**
**	The selection which was picked off of the print screen menu.
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
    unsigned int	status = Normal;
    int			file_id;
    int			options[100];
    Window		window;
    XRectangle		rect;
    int			message;
    int			screennum;

    /*
    ** Take over the screen.  This is the earliest point at which we
    ** can do this.  Don't single step past here unless you are on different
    ** server (or a terminal) than where printscreen is actually running.
    */
    XGrabServer (display_id);
    XFlush (display_id);

    /*
    ** Make sure that that the printscreen window is up to date.
    */
    if (smdata.toplevel != NULL) if (XtIsRealized((Widget)smdata.toplevel))
	XmUpdateDisplay (smdata.toplevel);

    screennum = screensetup.prt_screennum;

    /*
    ** Get setups.
    */
    prt_dialog_get_values ();

    create_options_list ();

    window = XRootWindow(display_id, screennum);

    wait_cursor(1);

    switch (prtsetup.capture_method)
    {
    case dxPrscEntire:
	status = dxPrscPrintScreenWindow
	    (display_id, screennum, window, &command, smdata.toplevel);

	break;
    case dxPrscPartial:
	status = getrectwin (display_id, screennum, &rect, &window);

	if (window != NULLWINDOW) 
	{
	    status = dxPrscPrintScreenWindow
		(display_id, screennum, window, &command, smdata.toplevel);
	}
	else
	{
	    status = dxPrscPrintScreenRect
	    (
		display_id,
		screennum,
		&command,
		rect.x,
		rect.y,
		++(rect.width),
		++(rect.height), 
		smdata.toplevel
	    );
	}
	break;
    }

    wait_cursor(0);

    if (status != Normal)  /* Normal from iprdw.h */
    {
	print_screen_error (status, &message);
	put_error (0, message);
    }

    return;
}

static void create_options_list
#if _PRDW_PROTO_
(
)
#else
()
#endif
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	Create the data list to send to print screen.
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
    command.options.aspect		= prtsetup.ratio;
    command.options.print_color		= prtsetup.color;
    command.options.reverse_image	= prtsetup.saver;
    command.options.storage_format	= prtsetup.format;
    command.options.form_feed		= prtsetup.rotate_prompt;
    command.options.print_widget	= prtsetup.print_widget;
    command.options.send_to_printer	= prtsetup.send_to_printer;
    command.options.fit			= prtsetup.fit;
    command.options.orient		= prtsetup.orientation;
    command.options.page_size		= prtsetup.page_size;
    command.options.sixel_device	= prtsetup.sixel_printer;    
    command.options.delay		= prtsetup.delay;
    strcpy (command.print_dest, prtsetup.capture_file);

    return;
}

static void print_screen_error
#if _PRDW_PROTO_
(
    long 	status,
    int		*message
)
#else
(status, message)
    long 	status;
    int		*message;
#endif
/*
**++
**  taken from [PRINTSCR]ERROR.C
**
**  ABSTRACT:
**
**	Print error message and exit or print warning string
**
**--
**/
/*
 *	error( status )
 *		status  - (RO) longword error status
 *	warning( string )
 *		string	- (RO) warning string
 *
 *	environment:
 *	requires:
 * 		none
 *	returns:
 *		exit status codes
 *	notes:
 *		this is DECWindows and its undefined
 *		(dialog boxes might be nice)
 */	

#define BUFSIZE 512
{
    switch (status)
    {
    case dxPrscInvDevId:
	*message = k_print_device_msg;
	break;
    case dxPrscInvWinId:
	*message = k_print_window_msg;
	break;
    case dxPrscInvAspect:
	*message = k_print_aspect_msg;
	break;
    case dxPrscInvPrColor:
	*message = k_print_color_msg;
	break;
    case dxPrscInvPrDest:
	*message = k_print_dest_msg;
	break;
    case dxPrscInvPrQ:
	*message = k_print_queue_msg;
	break;
    case dxPrscInvRevImg:
	*message = k_print_reverse_msg;
	break;
    case dxPrscInvStorage:
	*message =  k_print_storage_msg;
	break;
    case dxPrscInvForm:
	*message = k_print_form_msg;
	break;
    case dxPrscInvItem:
	*message = k_print_code_msg;
	break;
    case dxPrscFatErrLib:
	*message = k_print_fatal_msg;
	break;
    case dxPrscNoImage:
	*message = k_print_image_msg;
	break;
    case dxPrscBadFileIO:
	*message = k_print_file_msg;
	break;
    case dxPrscIntChkFail:
	*message = k_print_check_msg;
	break;
    case dxPrscNoMemory:
	*message = k_print_alloc_msg;
	break;
    case dxPrscXError:
	*message = k_print_intx_msg;
	break;
    case dxPrscFunError:
	*message = k_print_func_msg;
	break;
    default:
	*message = k_print_unknown_msg;
	break;
    }
}

static int create_screen_confirm ()
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
    Arg			arglist[10];
    static Widget	rb_id;
    int			i, screencount, currscreen;

    static MrmRegisterArg reglist[] =
    {
	{"ConfirmScrOkCallback", (caddr_t) confirm_screen_action},
	{"ConfirmScrCancelCallback", (caddr_t) confirm_screen_cancel},
	{"confirmprtbox_id", (caddr_t) &rb_id},
    };

    static int reglist_num = (sizeof reglist / sizeof reglist [0]);

    MrmRegisterNames (reglist, reglist_num);

    /*
    ** build the dialog using UIL
    */
    MrmFetchWidget
    (
	s_DRMHierarchy,
	"ConfirmScreen",
	smdata.toplevel,
	&smdata.screen_confirm_box_id,
	&drm_dummy_class
    );

    screencount = ScreenCount(display_id);
    currscreen = XScreenNumberOfScreen(
	XtScreen(smdata.screen_confirm_box_id));

    /*
    ** create the screen number toggle buttons to put inside the radio box
    */
    for (i=0; i<screencount; i++)
    {
	char    number[20];

	if (i == 0)
	{
	    smdata.screen_confirm_list = (Widget *)XtMalloc
		(screencount * sizeof(Widget));
	}

	int_to_str (i, number, 20);
	XtSetArg
	(
	    arglist[0],
	    XmNlabelString,
	    XmStringCreate(number, XmSTRING_DEFAULT_CHARSET)
	);

	smdata.screen_confirm_list[i] = XmCreateToggleButton
	    (rb_id, number, arglist, 1);
    }

    XmToggleButtonSetState
	(smdata.screen_confirm_list[currscreen], True, True);
    screennum = currscreen;
    screensetup.prt_screennum = screennum;

    XtManageChildren (smdata.screen_confirm_list, screencount);

    return (1);
}

static void confirm_screen_action
#if _PRDW_PROTO_
(
    Widget		widget,
    int			*tag,
    XmAnyCallbackStruct	*any
)
#else
(widget, tag, any)
    Widget		widget;
    int			*tag;
    XmAnyCallbackStruct	any;
#endif
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
*/
{
    int		i,j,type;

    /*
    ** default application startup screen
    */
    screennum = XScreenNumberOfScreen(XtScreen(widget));

    for (i = 0; i < ScreenCount(display_id); i++)
    {
        j = XmToggleButtonGetState(smdata.screen_confirm_list[i]);
        if (j == 1)
	{
	    screennum = i;
	    break;
	}
    }

    type = determine_system_color (display_id, screennum);
    switch (type)
    {
    case black_white_system:

	/* De-sensitize color & grayscale options */

	XtSetSensitive(prtsetup.color_id, FALSE);
	XtSetSensitive(prtsetup.grey_id, FALSE);
	XtSetSensitive(prtsetup.bw_id, TRUE);

	/* Set output color button to b/w if not already */

	if (XmToggleButtonGetState(prtsetup.bw_id) == 0)
	{
	    XmToggleButtonSetState(prtsetup.color_id, 0, 1);
	    XmToggleButtonSetState(prtsetup.grey_id, 0, 1);
	    XmToggleButtonSetState(prtsetup.bw_id, 1, 1);
	}
	break;

    case color_system:

	/* Sensitize color, grayscale and b/w options */

	XtSetSensitive(prtsetup.color_id, TRUE);
	XtSetSensitive(prtsetup.grey_id, TRUE);
	XtSetSensitive(prtsetup.bw_id, TRUE);

	break;

    case gray_system:

	/* De-sensitize color option, sensitize grayscale and b/w */

	XtSetSensitive(prtsetup.color_id, FALSE);
	XtSetSensitive(prtsetup.grey_id, TRUE);
	XtSetSensitive(prtsetup.bw_id, TRUE);

	/* Set output color button to grayscale if COLOR */

	if(XmToggleButtonGetState(prtsetup.color_id) == 1)
	{
	    XmToggleButtonSetState(prtsetup.color_id, 0, 1);
	    XmToggleButtonSetState(prtsetup.grey_id, 1, 0);
	    XmToggleButtonSetState(prtsetup.bw_id, 1, 0);
	}
    }

    XtUnmanageChild(XtParent(widget));

    XFlush(display_id);

    screensetup.prt_screennum = screennum;

    /*
    ** Get the queue to send it to.
    */
    if (prtsetup.send_to_printer != dxPrscFile)
	getqueue (NULL, smdata.toplevel);
    else
	timed_capture_screen ();
}

static void confirm_screen_cancel
#if _PRDW_PROTO_
(
    Widget		widget,
    caddr_t		tag,
    XmAnyCallbackStruct *any
)
#else
(widget, tag, any)
    Widget		widget;
    caddr_t		tag;
    XmAnyCallbackStruct *any;
#endif
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
*/
{
    XtUnmanageChild(XtParent(widget));
}
