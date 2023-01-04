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
#include <stdio.h>
#include "smdata.h"
#include <X11/Vendor.h>

/*
 * prototype
 */
XmString get_drm_message ();
char *CSToLatin1();


int	display_drm_message(status, text_index)
unsigned	int	status;
int text_index;
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	Displays a status and text in the session manager control panel.
**
**  FORMAL PARAMETERS:
**
**	status - A VMS status value
**	text_index - A index into the UIL array of text strings
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  COMPLETION CODES:
**
**  SIDE EFFECTS:
**
**	Not dealing with compound strings correctly here.   We are
**	not preserving all the attributes of a compound string.  Since
**	we need to send it in a message, we just turn it into a
**	null terminated string and pass it to our other routine.  We
**	are therefore not supporing any language other than iso-latin1
**	languages.
**
**--
**/
{
char	*str;
XmString message_cs;

if (text_index >= 0) {
    message_cs = get_drm_message (text_index);
    if (message_cs == NULL) {
	fprintf (stderr, "Session Error: DRM message #%d doesn't exist\n",
		text_index);
	return;
    }
    /* str = CSToLatin1 (message_cs); */
    XmStringGetLtoR(message_cs, def_char_set, &str);
}
else str = NULL;

display_message(status, str);
if (str != NULL) XtFree(str);
}

int	put_error(status, text_index)
unsigned	int	status;
int	text_index;
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	Put an error in the error dialog box.
**
**  FORMAL PARAMETERS:
**
**	status - A VMS status value
**	other_text - An index into the UIL message array. Or -1 if no
**		     text is to be displayed
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
char    *message_text;
XmString message_cs;

    /* nothing sent to this routine, just return */
    if ((status == 0) && (text_index < 0)) return;

    /* get the UIL text to go with this error */
    if (text_index >= 0) {
        message_cs = get_drm_message (text_index);
        if (message_cs == NULL) {
            fprintf(stderr, "Session Error: DRM message #%d doesn't exist\n",
		   text_index);
            return;
        }
    }

    /* it is possible that we haven't been able to open the display yet.  in
       that case, print the message to the log file.  Otherwise, put it in
       a dialog box. */
    /* A.R.  when would this happen????? */
/* ifdef ONE_DOT_TWO */
/* else 
    if (smdata.toplevel != 0) { */

    if ( !sm_inited || (smdata.toplevel != 0)) {
	error_display(message_cs);
    }
    else {
	if (message_cs != NULL) {	
	    message_text = CSToLatin1(message_cs);
	    fprintf(stderr, "Session Error: %s\n", message_text);
	    XtFree(message_text);
	}
    }
}


void client_message(widget, event, params, num_params)
Widget	widget;
XEvent	*event;
char	*params;
unsigned    int	*num_params;
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	Called when a client message is received.  There are two
**      reasons the session manager will receive a client message.
**	    1. Print screen - this was needed so that the screen would
**	       repaint before the print screen operation began.
**	    2. Displaying messages in control panel.  Can't make toolkit
**	       calls from AST level so needed a way to display a message
**	       from NON-AST level.
**
**  FORMAL PARAMETERS:
**
**	widget - The widget that received the client message - toplevel
**	event - A pointer to the client message event structure
**	params - An array of parameters
**	num_params - A pointer to an integer specifiying the number of params
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**	Print screen utility may be invoked
**	Text may be output to text widget control panel
**
**  COMPLETION CODES:
**
**  SIDE EFFECTS:
**
**--
**/
{
XClientMessageEvent *event_recv;
char	*message;
unsigned    int	bell;

event_recv = (XClientMessageEvent *)event;

/* Not a client message */
if ((event_recv->type & 127) != ClientMessage)
    return;

#ifdef DOPRINT
/* handle print screen case */
if (event_recv->message_type == XInternAtom(display_id, "DECW_SM_PRINT", FALSE))
	{
	do_capture_screen(event_recv->data.l[0]);
	return;
	}

#endif
/* If it is not the sm_message type, then return */
if (event_recv->message_type
	!= XInternAtom(display_id, "DECW_SM_MESSAGE", FALSE)) return;

/* Data pointer will be in the first part of the buffer */
message = (char *)event_recv->data.l[0];
/* Whether or not to ring the bell will be in the second element of the buffer*/
bell = event_recv->data.l[1];

/* A.R.  need a resource for Rich and Charles to turn it off */
/* Ring the bell if it was suppose to be rung */
if (bell) XBell(display_id, 0);

/* Free the memory which was allocated for the message */
free(message);

/* May need to change the appearance of the icon if we are currently
   iconified and a new message comes in */
check_icon();
}

int check_icon()
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	If the session manager is an icon, and the icon is not already
**	reverse video, then set it reverse video to alert the user that
**	there is new data.
**
**  FORMAL PARAMETERS:
**
**	icon_state - global variable set by a property notify callback
**		     When the application is iconified or de-iconified,
**		     this property notify callback is activated, it
**		     checks the state of the application, and sets the
**		     global flag to either am_icon (1) or not_icon (0)
**	icon_type -  state of the current icon.  Set by this routine, and
**		     cleared by the property notify callback when the 
**		     session manager is no longer an icon.  It can be
**		     icon_standard (0) or icon_reverse (1).
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
Arg     arglist[3];

if (icon_state == am_icon)
    {
    if (icon_type != icon_reverse)
	{
	XtSetArg(arglist[0], XtNiconPixmap, smdata.reverse_icon);
	XtSetValues(smdata.toplevel, arglist, 1);
	icon_type = icon_reverse;
	}
    }
}

int	put_pause_error(status, text_index)
unsigned	int	status;
int	text_index;
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	Put an error in the error dialog box for the pause window.
**
**  FORMAL PARAMETERS:
**
**	status - A VMS status value
**	other_text - An index into the UIL message array. Or -1 if no
**		     text is to be displayed
**
**--
**/
{
XmString message_cs;
char *message_text;

    /* nothing sent to this routine, just return */
    if ((status == 0) && (text_index < 0)) return;

    /* get the UIL text to go with this error */
    if (text_index >= 0) {
        message_cs = get_drm_message (text_index);
        if (message_cs == NULL) {
            fprintf(stderr, "Session Error: DRM message #%d doesn't exist\n",
		   text_index);
            return;
        }
    }

    /* it is possible that we haven't been able to open the display yet.  in
       that case, print the message to the log file.  Otherwise, put it in
       a dialog box. */
    if (smdata.toplevel != 0) {
	pause_error_display(message_cs);
    }
    else {
	if (message_cs != NULL) {	
	    message_text = CSToLatin1(message_cs);
	    fprintf(stderr, "Session Error: %s\n", message_text);
	    XtFree(message_text);
	}
    }
}
