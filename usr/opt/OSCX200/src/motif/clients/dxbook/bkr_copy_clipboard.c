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

#ifndef VMS
 /*
#else
#module BKR_COPY_CLIPBOARD "V03-0000"
#endif
#ifndef VMS
  */
#endif

#ifdef COPY

/*
***************************************************************
**  Copyright (c) Digital Equipment Corporation, 1988, 1991  **
**  All Rights Reserved.  Unpublished rights reserved	     **
**  under the copyright laws of the United States.  	     **
**  	    	    	    	    	    	    	     **
**  The software contained on this media is proprietary	     **
**  to and embodies the confidential technology of  	     **
**  Digital Equipment Corporation.  Possession, use, 	     **
**  duplication or dissemination of the software and	     **
**  media is authorized only pursuant to a valid written     **
**  license from Digital Equipment Corporation.	    	     **
**  	    	    	    	    	    	    	     **
**  RESTRICTED RIGHTS LEGEND   Use, duplication, or 	     **
**  disclosure by the U.S. Government is subject to 	     **
**  restrictions as set forth in Subparagraph (c)(1)(ii)     **
**  of DFARS 252.227-7013, or in FAR 52.227-19, as  	     **
**  applicable.	    	    	    	    	    	     **
***************************************************************
*/


/*
**++
**  FACILITY:
**      Bookreader User Interface ( bkr )
**
**  ABSTRACT:
**	This module contains all of the Clipboard and Quick Copy callback
**	routines for the Bookreader.  In both cases, only character (string)
**	data is transferred.
**
**  AUTHOR:
**      Tom Rose
**
**  CREATION DATE:     30-Jan-1992
**
**  MODIFICATION HISTORY:
**	V01-2	Tom Rose		17-March-1992
**		Check global to see if clipboard is enabled; if not, display
**		message indicating Plus license required.
**
**	V01-1	Tom Rose		16-March-1992
**		Display error messages for Clipboard failures in popup.
**
**--
**/

#include <Xm/MessageB.h>
#include <Xm/Text.h>
#include <Xm/Xm.h>
#include <Xm/CutPaste.h>
#include <DXm/DECspecific.h>
#include <Mrm/MrmPublic.h>
#include <X11/Xatom.h>
#include <stdio.h>
#include <ctype.h>
#include "br_common_defs.h"  /* common BR #defines */
#include "br_meta_data.h"    /* typedefs and #defines for I/O of BR files */
#include "br_typedefs.h"     /* BR high-level typedefs and #defines */
#include "br_globals.h"      /* BR external variables declared here */
#include "br_malloc.h"       /* BKR_MALLOC, etc defined here */
#include "bkr_button.h"
#include "bkr_copy_clipboard.h"

#define SAMPLE_TEXT "Sample Text"
#define BKR_APPL_TITLE "Bookreader"
#define BKR_CLIPBOARD_FORMAT "STRING"
#define BKR_PRIVATE_ID 123



/********************************
 *	Forward Routines	*
 ********************************/
static Time		fetch_timestamp_from_event ();
void			bkr_copy_clipboard PROTOTYPE((Widget, int *,
                                                      XmAnyCallbackStruct *));
XtConvertSelectionProc	bkr_convert_selection_cbk ();
XtLoseSelectionProc	bkr_lose_selection_cbk ();
XtSelectionDoneProc	bkr_selection_done_cbk ();



/*
**++
**  ROUTINE NAME:
**	fetch_timestamp_from_event (event);
**                                
**  FUNCTIONAL DESCRIPTION:
**	This routine extracts the timestamp from the event; if the event
**	does not have a timestamp or if the event is NULL, CurrentTime
**	is returned.
**
**  FORMAL PARAMETERS
**	XEvent		*event; - Pointer to event
**
**  IMPLICIT INPUTS:
**	None
**
**  IMPLICIT OUTPUTS:
**	None
**
**  FUNCTION VALUE:
**	Timestamp Time
**
**  SIDE EFFECTS:                   
**	None
**
**--
*/
static Time fetch_timestamp_from_event (event)
    XEvent	*event;
{
    if (event == NULL)
	return (CurrentTime);

    /* If the event contains a timestamp, return it */
    switch (event->type) {
	case KeyPress:
            return (((XKeyPressedEvent *) event)->time);

	case KeyRelease:
	    return (((XKeyReleasedEvent *) event)->time);

	case ButtonPress:
 	    return (((XButtonPressedEvent *) event)->time);

	case ButtonRelease:
	    return (((XButtonReleasedEvent *) event)->time);

	case MotionNotify:
	    return (((XPointerMovedEvent *) event)->time);

	case EnterNotify:
	    return (((XEnterWindowEvent *) event)->time);

	case LeaveNotify:
	    return (((XLeaveWindowEvent *) event)->time);

	case PropertyNotify:
	    return (((XPropertyEvent *) event)->time);

	case SelectionClear:
	    return (((XSelectionClearEvent *) event)->time);

	case SelectionRequest:
	    return (((XSelectionRequestEvent *) event)->time);

	case SelectionNotify:
	    return (((XSelectionEvent *) event)->time);

	case FocusIn:
	case FocusOut:
	case KeymapNotify:
	case Expose:
	case GraphicsExpose:
	case NoExpose:
	case VisibilityNotify:
	case CreateNotify:
	case DestroyNotify:
	case UnmapNotify:
	case MapNotify:
	case MapRequest:
	case ReparentNotify:
	case ConfigureNotify:
	case ConfigureRequest:
	case GravityNotify:
	case ResizeRequest:
	case CirculateNotify:
	case CirculateRequest:
	case ColormapNotify:
	case ClientMessage:
	case MappingNotify:
	    return (CurrentTime);
    }
}




/*
**++
**  ROUTINE NAME:
**	bkr_copy_clipboard 
**
**  FUNCTIONAL DESCRIPTION:
**	This routine gets called when the Copy button from the EDIT menu
**	is chosen.  We respond by stuffing the buffer containing the
**	current selection into the Clipboard.
**
**  FORMAL PARAMETERS
**	Widget			widget;	- Widget calling back
**	int			*tag;	- User tag data 
**	XmAnyCallbackStruct	*cbs;	- Callback structure
**
**  IMPLICIT INPUTS:
**	bkr_copy_buffer, bkr_copy_buffer_len
**
**  IMPLICIT OUTPUTS:
**	None
**
**  FUNCTION VALUE:
**	None
**
**  SIDE EFFECTS:
**	None
**
**--
*/
void bkr_copy_clipboard PARAM_NAMES((w,tag,cbs))
    Widget w PARAM_SEP
    int *tag PARAM_SEP
    XmAnyCallbackStruct *cbs PARAM_END
{
    BKR_WINDOW  *bkr_window;
    Window	window;
    Display	*display;
    XmString	compound_string;
    int		status;
    long	item_id_return;
    Time	timestamp;
    int		data_id;
    char	*error_string;

    /* Check global set by LMF check */
    if (!bkrplus_g_allow_copy) {
	bkr_display_need_license_box ();
	return;
    }

    bkr_window = (BKR_WINDOW *) bkr_window_find (w);
    
    display = XtDisplay (bkr_window->appl_shell_id);
    window = XtWindow (bkr_window->appl_shell_id);

    /*--- report msg and return if book not licensed for copy ---*/
    if (bri_book_no_print(bkr_window->shell->book->book_id)) {
        if(error_string = (char *) bkr_fetch_literal("PE_BOOK_NO_COPY",ASCIZ)) {
          sprintf( errmsg, error_string );
          bkr_error_modal( errmsg, NULL);
          XtFree( error_string );
        }
       return;
    }

    /* Return if there is no window or if there is nothing to copy */
    if (window == 0 || bkr_copy_ctx.bkr_copy_buffer_len == 0)
	return;

    /* Create a compound string to pass to the Clipboard.  This describes the
       type (owner) of the data we are about to send */
    compound_string = XmStringCreateSimple (BKR_APPL_TITLE);

    /* Fetch the timestamp from the callback structure */
    timestamp = fetch_timestamp_from_event (cbs->event);

    /* Initialize clipboard */
    status = XmClipboardStartCopy (display, window,
				   compound_string,
				   timestamp,
				   0,	/* widget to handle "pass by name" callbacks */
				   NULL,/* "pass by name" callback procedure */
				   &item_id_return);

    XmStringFree (compound_string);

    /* If unsuccessful, display error message and return */
    if (status != ClipboardSuccess) {
	if (status == ClipboardLocked)
	    error_string = (char *) bkr_fetch_literal ("CLIPBOARD_LOCK_ERROR", ASCIZ);
	else
	    error_string = (char *) bkr_fetch_literal ("CLIPBOARD_UNKNOWN_ERROR", ASCIZ);

	if (error_string != NULL) {
	    sprintf (errmsg, error_string);
	    bkr_error_modal (errmsg, NULL);
	    XtFree (error_string);
	}
	return;
    }

    /* Now copy the data that is selected into the Clipboard */
    status = XmClipboardCopy (display, window,
			      item_id_return,
			      BKR_CLIPBOARD_FORMAT,
			      bkr_copy_ctx.bkr_copy_buffer,
			      (unsigned long) bkr_copy_ctx.bkr_copy_buffer_len,
			      BKR_PRIVATE_ID,
			      &data_id);

    /* If successful, enter data into clipboard storage for other applications
       to grab */ 
    if (status == ClipboardSuccess) {
	status = XmClipboardEndCopy (display, window, item_id_return);
	bkr_un_highlight(bkr_copy_ctx.window);
        }

    /* If either the copy or the end copy failed, cancel copy and display
       error message */ 
    if (status != ClipboardSuccess) {

	XmClipboardCancelCopy (display, window, item_id_return);

	if (status == ClipboardLocked)
	    error_string = (char *) bkr_fetch_literal ("CLIPBOARD_LOCK_ERROR", ASCIZ);
	else
	    error_string = (char *) bkr_fetch_literal ("CLIPBOARD_UNKNOWN_ERROR", ASCIZ);

	if (error_string != NULL) {
	    sprintf (errmsg, error_string);
	    bkr_error_modal (errmsg, NULL);
	    XtFree (error_string);
	}
    }
}




/*
**++
**  ROUTINE NAME:
**	bkr_convert_selection_cbk (widget, p_selection, p_target,
**				   p_type_return, p_value_return, 
**				   p_length_return, p_format_return)
**
**  FUNCTIONAL DESCRIPTION:
**	This routine gets called by the X Toolkit when another application
**	requests the value of our selection as a given type.  "p_selection"
**	is the atom descibing the type of selection (e.g. primary secondary),
**	"p_value_return" is set to the pointer of the converted value, with
**	"p_length_return" elements of data, each of size indicated by
**	"p_format_return".  "p_target" is a pointer to the type that the
**	conversion should use if possible; "p_type_return" is a pointer to the
**	acutal type that is returned.  "p_format_return" should be a pointer to
**	either 8, 16, or 32, and specifies the word size of the selection.
**
**  FORMAL PARAMETERS
**	Widget		widget;		- Widget owning the selection
**	Atom		*selection	- Selection atom
**	Atom		*target		- Type we should return if possible
**	Atom		*type_return	- Type we are returning
**	XtPointer	*value_return	- Converted selection value (text)
**	unsigned long	*length_return	- Number of characters in value_return
**	int		*format_return	- Size of data chunks (8)
**
**  IMPLICIT INPUTS:
**	bkr_copy_buffer, bkr_copy_buffer_len
**
**  IMPLICIT OUTPUTS:
**	None
**
**  FUNCTION VALUE:
**	TRUE if we recognize the format requested and can send it off
**	FALSE is we don't support the requested format
**
**  SIDE EFFECTS:
**	None
**
**--
*/
XtConvertSelectionProc
    bkr_convert_selection_cbk ( widget, p_selection, p_target, p_type_return, 
				p_value_return, p_length_return,
				p_format_return) 
    Widget		widget;
    Atom		*p_selection, *p_target, *p_type_return;
    XtPointer		*p_value_return;
    unsigned long	*p_length_return;
    int			*p_format_return;
{
    /* If the target is not text, or if there is nothing selcted,
       then we can't accomodate the request */
    if (*p_target != XA_STRING || bkr_copy_ctx.bkr_copy_buffer_len == 0)
	return (FALSE);

    /* Let the Toolkit know we're writing text */
    *p_type_return = (Atom) XA_STRING;

    /* Point the return value at the copy buffer */
    *p_value_return = (XtPointer) bkr_copy_ctx.bkr_copy_buffer;

    /* Set the return length to the buffer length */
    *p_length_return = (unsigned int) bkr_copy_ctx.bkr_copy_buffer_len;

    /* The return format is 8 */
    *p_format_return = 8;

    return ((XtConvertSelectionProc) TRUE);
}



/*
**++
**  ROUTINE NAME:
**	bkr_lose_selection_cbk (widget, selection)
**
**  FUNCTIONAL DESCRIPTION:
**	This routine gets called by the X Toolkit whenever another application
**	has asserted ownership of the selection atom which we currently own.
**	It is our responsibility to "kill" (unhighlight) our selection.
**
**  FORMAL PARAMETERS
**	Widget		widget;		- Widget owning the selection
**	Atom		*selection	- Selection atom
**
**  IMPLICIT INPUTS:
**	None
**
**  IMPLICIT OUTPUTS:
**	None
**
**  FUNCTION VALUE:
**	None
**
**  SIDE EFFECTS:
**	None
**
**--
*/
XtLoseSelectionProc bkr_lose_selection_cbk (widget, p_selection)
    Widget		widget;
    Atom		*p_selection;
{
    /* Call routine to delete selection */
   BKR_WINDOW              *window = NULL;

   window = (BKR_WINDOW *) bkr_window_find( widget );
     if ( window == NULL )
        return;

    if(window != bkr_copy_ctx.window);
       bkr_un_highlight(bkr_copy_ctx.window);
   
}




/*
**++
**  ROUTINE NAME:
**	bkr_lose_selection_cbk (widget, selection, target)
**
**  FUNCTIONAL DESCRIPTION:
**	This routine gets called by the X Toolkit to inform us that the
**	receiver has successfully retrieved the selection value.
**
**  FORMAL PARAMETERS
**	Widget		widget;		- Widget owning the selection
**	Atom		*selection	- Selection atom
**	Atom		*target		- Target atom
**
**  IMPLICIT INPUTS:
**	None
**
**  IMPLICIT OUTPUTS:
**	None
**
**  FUNCTION VALUE:
**	None
**
**  SIDE EFFECTS:
**	None
**
**--
*/
XtSelectionDoneProc bkr_selection_done_cbk (widget, p_selection, p_target)
    Widget		widget;
    Atom		*p_selection, *p_target;
{

}
#endif  /* COPY */
