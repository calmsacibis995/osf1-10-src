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
#ifdef DOPRINT
#include "smdata.h"
#include "smconstants.h"
#include <X11/decwcursor.h>
#include "prdw.h"

void execprintwindow();
void execprintrect();
void	print_menu_cb();
void confirm_print_action();
void confirm_print_cancel();
void confirm_screen_action();
void confirm_screen_cancel();
void create_options_list();

static	Widget	the_item;
static	int screennum;
static	int both_prompts;


void	print_menu_cb(widgetID, tag, reason)
Widget	*widgetID;
caddr_t	tag;
XmRowColumnCallbackStruct	*reason;
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
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
**/
{
/* put up a prompt for the name of a file to write the bits to.  Prefill
    it with the value of the print setup filename */
/* This routine also checks for prompting for screen number on multi-head
    machines */
confirm_file(reason->widget);
}

int do_capture_screen(awidget)
Widget	awidget;
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
**/
{
unsigned int	status;
int		file_id;
Window		window;
XRectangle	rect;
int		message;

if (awidget == smdata.print_es)
	{
	/* do what we have to do to print the entire screen */
	create_options_list(1);
	/* We will be printing the root window */
	window = XRootWindow(display_id, screennum);
	XUngrabPointer( display_id, CurrentTime);
	XFlush (display_id);
	status = dxPrscPrintScreenWindow (display_id, screennum, window, 
			smdata.optionslist, smdata.toplevel);
	root_wait_cursor(0);
	if (status != Normal)  /* Normal from Print Screen include file */
	    {
	    print_screen_error (status, &message);
	    put_error (0, message);
	    }
	return;
	}

if (awidget == smdata.print_pos)
	{
	/* do what we have to do to print portion of the screen */
        create_options_list(1);
	XUngrabPointer( display_id, CurrentTime);
	XFlush (display_id);
	/* Get the area to print */
	status = getrectwin (display_id, screennum, &rect, &window);

	/* smgetrectwin doesn't return any errors right now...*/
	if (window != NULLWINDOW) 
	    {
	    status = dxPrscPrintScreenWindow (display_id, screennum, window, 
			smdata.optionslist, smdata.toplevel);
	    }
	else
	    {
	    status = dxPrscPrintScreenRect (display_id, screennum, smdata.optionslist, 
	    		rect.x, rect.y, ++(rect.width), ++(rect.height), 
	    		smdata.toplevel);
	    }
	root_wait_cursor(0);
	if (status != Normal)  /* Normal from Print Screen include file */
	    {
	    print_screen_error (status, &message);
	    put_error (0, message);
	    }
	return;
	}

create_options_list(0);

if (awidget == smdata.capture_es)
	{
	/* do what we have to do to write entire screen to file */
	window = XRootWindow(display_id, screennum);
	XUngrabPointer( display_id, CurrentTime);
	XFlush (display_id);
	status = dxPrscPrintScreenWindow (display_id, screennum, window, 
			smdata.optionslist, smdata.toplevel);
	root_wait_cursor(0);
	if (status != Normal)  /* Normal from Print Screen include file */
	    {
	    print_screen_error (status, &message);
	    put_error (0, message);
	    }
	return;
	}

if (awidget == smdata.capture_pos)
	{
	/* do what we have to do to write portion of screen to file */
	XUngrabPointer( display_id, CurrentTime);
	XFlush (display_id);
	status = getrectwin (display_id, screennum, &rect, &window);
	/* smgetrectwin doesn't return any errors right now...*/
	if (window != NULLWINDOW) 
	    {
	    status = dxPrscPrintScreenWindow (display_id, screennum, window, 
			smdata.optionslist, smdata.toplevel);
	    }
	else
	    {
	    status = dxPrscPrintScreenRect (display_id, screennum, smdata.optionslist, 
	    		rect.x, rect.y, ++(rect.width), ++(rect.height), 
	    		smdata.toplevel);
	    }
	root_wait_cursor(0);
	if (status != Normal)  /* Normal from Print Screen include file */
	    {
	    print_screen_error (status, &message);
	    put_error (0, message);
	    }
	return;
	}
root_wait_cursor(0);
XUngrabPointer( display_id, CurrentTime);
}

root_wait_cursor(on)
int on;
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	Change the cursor on the root window to a watch, or
**	back to the default cursor. 
**
**  FORMAL PARAMETERS:
**
**	on - If = 1, display the watch
**	     If = 0, remove the watch
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
int numscreens, i;
Window	*wmrootlist = 0;

/* Get the number of screens on the system */
numscreens = XScreenCount(display_id);
wmrootlist = XtMalloc(sizeof(Window)*numscreens);
/* For each screen, get the window manager root*/
for (i=0; i < numscreens; i++)
    {
    Window	root,wmroot;

    root = XRootWindow(display_id, i);
    wmroot = WmRootWindow(display_id,root);
    wmrootlist[i] = WmRootWindow(display_id,wmroot);
    do_cursor(on, i, wmrootlist[i]);
    }
XtFree(wmrootlist);
}

do_cursor(on, scrnnbr, win)
int on;
int scrnnbr; /* added screen number to allow correct restore; PAC */
Window win;
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	Change the cursor on a window to a watch, or
**	back to the default cursor. 
**
**  FORMAL PARAMETERS:
**
**	on - If = 1, display the watch
**	     If = 0, remove the watch
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
Font cfont;
static Cursor wait_cursor = 0;
static XColor fore = {0, 65535, 65535, 65535};	/* white */
static XColor back = {0, 0, 0, 0}; /* black */
int wait_c;

if (on == 1) {
    if (wait_cursor == 0)
	{
	cfont = XLoadFont (display_id, "decw$cursor");
	if (!cfont) return;

	wait_c = decw$c_wait_cursor;
	wait_cursor = XCreateGlyphCursor (display_id, 
		cfont, cfont, wait_c, wait_c + 1, &fore, &back);
	}
    XDefineCursor (display_id, win, wait_cursor);
    XFlush (display_id);
    }
else
    {
    if (current_cursor!=0)
    	XDefineCursor (display_id, win, current_cursor[scrnnbr]);
    else
	XUndefineCursor (display_id, win);
    }
}

void create_options_list( toqueue )
unsigned int	toqueue;
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
long	status;		/* return status code 			*/
int i = 0;

smdata.optionslist[i++]	= dxPrscAspect;
smdata.optionslist[i++]	= prtsetup.ratio;

smdata.optionslist[i++]	= dxPrscPrintColor;
smdata.optionslist[i++]	= prtsetup.color;

smdata.optionslist[i++]	= dxPrscReverseImage;
smdata.optionslist[i++]	= prtsetup.saver;

smdata.optionslist[i++]	= dxPrscStorageFormat;
smdata.optionslist[i++] 	= prtsetup.format;

smdata.optionslist[i++]	= dxPrscFormControl;
smdata.optionslist[i++] 	= prtsetup.rotate_prompt;

if (!toqueue)
    {
    smdata.optionslist[i++]	= dxPrscPrintDest;
    smdata.optionslist[i++]	= smdata.print_destination;
    }

smdata.optionslist[i++]	= dxPrscPrintQueue;
if (!toqueue)
    smdata.optionslist[i++]	= dxPrscAccumulate;
else
    smdata.optionslist[i++]	= dxPrscImmediate;
    
smdata.optionslist[i++]	= dxPrscEndOfList;
}
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

print_screen_error ( status, message )
long 	status;
int *message;
{
switch( status )
	{
	case DECW_PRSC_INVDEV_ID:
		*message = k_print_device_msg;
		break;
	case DECW_PRSC_INVWIN_ID:
		*message = k_print_window_msg;
		break;
	case DECW_PRSC_INVASPECT:
		*message = k_print_aspect_msg;
		break;
	case DECW_PRSC_INVPR_COLOR:
		*message = k_print_color_msg;
		break;
	case DECW_PRSC_INVPR_DEST:
		*message = k_print_dest_msg;
		break;
	case DECW_PRSC_INVPR_Q:
		*message = k_print_queue_msg;
		break;
	case DECW_PRSC_INVREVIMG:
		*message = k_print_reverse_msg;
		break;
	case DECW_PRSC_INVSTORAGE:
		*message =  k_print_storage_msg;
		break;
	case DECW_PRSC_INVFORM:
		*message = k_print_form_msg;
		break;
	case dxPrscInvItem:
		*message = k_print_code_msg;
		break;
	case DECW_PRSC_FATERRLIB:
		*message = k_print_fatal_msg;
		break;
	case DECW_PRSC_NOIMAGE:
		*message = k_print_image_msg;
		break;
	case DECW_PRSC_BADFILEIO:
		*message = k_print_file_msg;
		break;
	case DECW_PRSC_INTCHKFAIL:
		*message = k_print_check_msg;
		break;
	case DECW_PRSC_NOMEMORY:
		*message = k_print_alloc_msg;
		break;
	case DECW_PRSC_X_ERROR:
		*message = k_print_intx_msg;
		break;
	case DECW_PRSC_FUN_ERROR:
		*message = k_print_func_msg;
		break;
	default:
		*message = k_print_unknown_msg;
		break;
	}
}

int	confirm_file(awidget)
Widget	awidget;
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	Put up a confirmation box asking for the file name or the
**	screen number.
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
XtCallbackRec cb_start_callback[2];
unsigned int done = 0;
int file_prompt = 0;
int screen_prompt = 0;
Arg arglist[2];

if ((awidget == smdata.capture_es) || (awidget == smdata.capture_pos))
    {
    /* The user can turn off the file prompt option.  If they did turn
       it off, just send the client message and return */
    if (prtsetup.file_prompt)
	{
	file_prompt = 1;
	}
    else
	{
	strcpy(smdata.print_destination, prtsetup.capture_file);
	}
    }

if (ScreenCount(display_id) > 1)
    {
    /* The user can turn off the screen prompt option.  If they did turn
       it off, just send the client message and return */
    if (screensetup.prt_prompt)
	{
	screen_prompt = 1;
	}
    }

the_item = awidget;
screennum =GETSCREEN(screensetup.prt_screennum, display_id);

if ((!file_prompt) && (!screen_prompt))
    {
    send_print_message(awidget);
    return;
    }

if ((file_prompt) && (screen_prompt))
    both_prompts = 1;
else
    both_prompts = 0;

if (file_prompt)
    {
    if (smdata.print_confirm_box_id == 0)
	create_print_confirm();

    /* We need to know the widget in the OK callback.  You can't change
       the tag of a callback.  We would have to remove and add the 
       callback with a new tag.  Just use a global variable */

    /* Pre-fill it with the value from the print screen customize box */
    XmTextSetString(smdata.print_stext_id, prtsetup.capture_file);
    XtSetArg(arglist[0], XmNcolumns, 25);
    XtSetValues(smdata.print_stext_id, arglist, 1);
    XtManageChild(smdata.print_confirm_box_id);
    }

else if (screen_prompt)
    {
    if (smdata.screen_confirm_box_id == 0)
	create_screen_confirm();
    /* Pre-fill it with the value from the print screen customize box */
    set_correct_button(screensetup.prt_screennum);
    set_screen_data(&screennum, prt_operation, &the_item);
    XtManageChild(smdata.screen_confirm_box_id);
    }
}

int create_print_confirm()
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	Create a modal confirmation box for the print screen filename
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
        {"ConfirmPrtCancelCallback", (caddr_t) confirm_print_cancel},
        {"ConfirmPrtOkCallback", (caddr_t) confirm_print_action},
        {"confirmprttext_id", &smdata.print_stext_id},
        };

static int reglist_num = (sizeof reglist / sizeof reglist [0]);

MrmRegisterNames (reglist, reglist_num);

/* build the dialog using UIL */
MrmFetchWidget(s_DRMHierarchy, "ConfirmPrint", smdata.toplevel,
                    &smdata.print_confirm_box_id,
                    &drm_dummy_class);
return(1);
}

void	confirm_print_action(widget, tag, reason)
Widget	*widget;
caddr_t tag;
unsigned int reason;
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	Get the file name and send the client message which will 
**	start the print screen.
** 
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
strcpy(smdata.print_destination, XmTextGetString(smdata.print_stext_id));

if (both_prompts == 1)
    {
    if (smdata.screen_confirm_box_id == 0)
	create_screen_confirm();
    /* Pre-fill it with the value from the print screen customize box */
    XtUnmanageChild(XtParent(*widget));
    set_screen_data(&screennum, prt_operation, &the_item);
    set_correct_button(screensetup.prt_screennum);
    XtManageChild(smdata.screen_confirm_box_id);
    }
else
    {
    XtUnmanageChild(XtParent(*widget));
    XFlush(display_id);
    send_print_message(the_item);
    }
}

void	confirm_print_cancel(widget, tag, reason)
Widget	*widget;
caddr_t	tag;
unsigned int reason;
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	The user canceled the print screen operation.  Just return.
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
XtUnmanageChild(XtParent(*widget));
}

send_print_message(awidget)
Widget	awidget;
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	Change the cursor on the root window to a watch.  Then
**	send a client message to ourselves for a print screen
**	operation.   Do this so that the screen will handle all
**	exposes before the print screen starts.
**	
**  FORMAL PARAMETERS:
**
**	awidget = The option which was selected.  
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
unsigned    int length;
char    *new_message;
XClientMessageEvent event;

root_wait_cursor(1);
XGrabPointer( display_id, XtWindow(smdata.toplevel), True, None,
		 GrabModeSync, GrabModeSync, None, 
		 None, CurrentTime);
XFlush(display_id);
event.type = ClientMessage;
event.display = display_id;
event.window = control_window;
event.format = 32;
event.message_type = XInternAtom(display_id, "DECW_SM_PRINT", FALSE);
event.data.l[0] = awidget;
XSendEvent(display_id, control_window, FALSE, ClientMessage, &event);
}
#endif /* DOPRINT */
