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
#define SS_NORMAL 1
#include "smdata.h"
#include "smconstants.h"

/*
 * prototyping
 */
XmString get_drm_message ();
static	Widget	the_widget;

int	create_menu_cb(widgetID, tag, reason)
Widget	*widgetID;
caddr_t	tag;
XmRowColumnCallbackStruct	*reason;
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	Take the appropriate action based on the menu item selected.
**
**  FORMAL PARAMETERS:
**
**	widgetID - the widget ID of the create menu
**	tag - data passed to the callback
**	reason - a menu callback.   The s_widget of the reason structure
**		 will point to the widget of the item selected.
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
unsigned    int	status;
int screennum;

    if (ScreenCount(display_id) > 1) {
        /* The user can turn off the screen prompt option.  If they did turn
           it off, just send the client message and return */
        if (screensetup.appl_prompt) {
	    if (smdata.screen_confirm_box_id == 0)
	        create_screen_confirm();

	    /* Pre-fill it with the value from the print screen customize box */
	    set_correct_button(screensetup.appl_screennum);
	    screennum = screensetup.appl_screennum;
	    the_widget = reason->widget;
	    set_screen_data(&screennum, create_operation, &the_widget);
	    XtManageChild(smdata.screen_confirm_box_id);
	    return;
	}
    }

    do_create(reason->widget,GETSCREEN(screensetup.appl_screennum,display_id));
}

do_create(s_widget,screennum)
Widget	s_widget;
int screennum;
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	Take the appropriate action based on the menu item selected.
**
**  FORMAL PARAMETERS:
**
**	widgetID - the widget ID of the create menu item selected
**	screennum - The screennumber to start the application on
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
Arg     arglist[2];
XmString label = NULL;
char	*label_text = NULL;

    /* Get the label of the menu which was selected */
    XtSetArg (arglist[0], XmNlabelString, &label);
    XtGetValues (s_widget, arglist, 1);
    if (label == NULL) {
        put_error(0, k_label_missing_msg);
        return;
    }
    XmStringGetLtoR(label, def_char_set, &label_text);

    if (label_text == NULL) {
        put_error(0, k_label_missing_msg);
        return;
    }
    create_app(label_text, screennum);
    XtFree(label_text);
}

int create_app(label_text, screennum)
char	*label_text;
int screennum;
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	Create an application. Get the DCL command
**	for the text which was passed in.  Then call
**	the routine which knows how to start loginout
**	and pass it a DCL command.
**
**  FORMAL PARAMETERS:
**
**	label_text - pointer to text of menu item selected
**	screennum - The screennumber to start the application on.
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
unsigned    int	status;
char	*command = NULL;
int dummy;

    /* Get the command of the menu which was selected */
    get_menu_command(label_text, &command, &dummy);
    if (command == NULL) {
        put_error(0, k_verb_missing_msg);
        return;
    }

    status = start_unix_command (command,screennum,label_text);
    XtFree(command);
}

display_app_msg(label_text,message_num)
char	*label_text;
unsigned    int	message_num;
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	Display that a particular application has been started.
**	The message will be displayed in the control panel.
**
**  FORMAL PARAMETERS:
**
**	label_text - pointer to text of menu item selected
**	message_num - pointer to UIL string index of message text
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
    char    out_string[256];
    unsigned    short   ret_length = 0;
    char *title_text;
    XmString title_cs;

    title_cs = get_drm_message (message_num);
    /* title_text = CSToLatin1 (title_cs); */
    XmStringGetLtoR(title_cs, def_char_set, &title_text);
    out_string[ret_length] = 0;
    display_message(0,out_string);
    XtFree(title_text);
}
