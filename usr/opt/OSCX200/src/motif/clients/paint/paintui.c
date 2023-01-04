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
/*
**************************************************************************
**                   DIGITAL EQUIPMENT CORPORATION                      **
**                         CONFIDENTIAL                                 **
**    NOT FOR MODIFICATION OR REDISTRIBUTION IN ANY MANNER WHATSOEVER   **
**************************************************************************
*/
#ifndef lint	/* BuildSystemHeader added automatically */
static char *BuildSystemHeader= "$Id: paintui.c,v 1.1.4.2 1993/06/25 22:46:47 Ronald_Hegli Exp $";
#endif		/* BuildSystemHeader */
/*
****************************************************************************
**                                                                          *
**  Copyright (c) 1987                                                      *
**  By DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASS.                        *
**  All Rights Reserved
**                                                                          *
**  This software is furnished under a license and may be used and  copied  *
**  only  in  accordance  with  the  terms  of  such  license and with the  *
**  inclusion of the above copyright notice.  This software or  any  other  *
**  copies  thereof may not be provided or otherwise made available to any  *
**  other person.  No title to and ownership of  the  software  is  hereby  *
**  transferred.                                                            *
**                                                                          *
**  The information in this software is subject to change  without  notice  *
**  and  should  not  be  construed  as  a commitment by DIGITAL EQUIPMENT  *
**  CORPORATION.                                                            *
**                                                                          *
**  DIGITAL assumes no responsibility for the use or  reliability  of  its  *
**  software on equipment which is not supplied by DIGITAL.                 *
**                                                                          *
*****************************************************************************
**++
**  FACILITY:
**
**   DECpaint - VMS DECwindows paint program
**
**  AUTHOR
**
**   Daniel Latham, October 1987
**
**  ABSTRACT:
**
**   This module contains routines that use the toolkit to build the 
**   human interface.  
**
**  ENVIRONMENT:
**
**   User mode, executable image.
**
**  MODIFICATION HISTORY:
**
**  refresh zoom magnifier so it will be in front of grid jj-10/12/88 
**
**      dl      10/5/88
**      Set global to note if picture contents have been modified.  If
**      exiting and a save has been made, user will not be prompted to
**      save the contents.
**
**      jj      10/12/88
**      refresh zoom magnifier so it will be in front of grid.
**--
*/           

#define XWINDOW		    /* treg */
#include "paintrefs.h" 

#if !defined(HYPERHELP)
#define HYPERHELP 1
#endif

#if defined(VMS) && !defined(__DECC)
#pragma nostandard
#endif
#include <Xm/DrawingAP.h>
#include <Xm/MessageB.h>
#if HYPERHELP
#include <DXm/DXmHelpB.h>
#endif
#if defined(VMS) && !defined(__DECC)
#pragma standard
#endif

/* #include "xwindow.h"	    *//* treg -> */
/* here's what was in xwindow.h */

/* forward declaration */
static void Realize();	/* JJ-MOTIF */ 
void help_error();

typedef struct
{
    caddr_t		extension;	/* Pointer to extension record */
}
    PTDrawingAreaClassPart;


typedef struct _PTDrawingAreaClassRec
{
    CoreClassPart	    core_class;
    CompositeClassPart	    composite_class;
    ConstraintClassPart     constraint_class;
    XmManagerClassPart      manager_class;
    XmDrawingAreaClassPart  drawing_area_class;
    PTDrawingAreaClassPart  ptdrawing_area_class;
} 
    PTDrawingAreaClassRec, *PTDrawingAreaWidgetClass;


#ifndef XWINDOW
external PTDrawingAreaClassRec	    ptDrawingAreaClassRec;
external PTDrawingAreaWidgetClass   ptDrawingAreaWidgetClass;
#endif

/* <- treg */


/* variables for interface */
Widget file_menu, edit_menu, options_menu, icon_menu, custom_menu;
/* Widget write_dialog, read_dialog; */


/*									    
 *    Necessary UIL/DRM stuff -- name of the UID file
 */
static char *db_filename_vec [1];         /* DRM hierarchy file list */

static int quitting = 0;
static int opening = 0;
#define EXIT_QUESTION 0
#define OPEN_QUESTION 1
static int default_question_question;

Widget help_can_button;
/* Widget quit_dialog, open_caution_box, ai_error_caution_box; */
static int ai_error_reason;

/* Scroll bars */
static int horiz_uinc, horiz_pinc;
static int vert_uinc, vert_pinc;

static char *action_str;
static char *t_actions[MAX_ACTION];
static int toggle_undo; /* used to toggle undo/redo */
static int prv_toggle_undo;

struct msg_list {
    char *msg;
    struct msg_list *next;
};

static struct msg_list *main_msg_list = NULL;
static int messaging_begun = FALSE;

/* treg -> */


#if defined(VMS) && !defined(__DECC)
#pragma nostandard
#endif
externaldef (ptDrawingAreaClassRec)
#if defined(VMS) && !defined(__DECC)
#pragma standard
#endif
XmDrawingAreaClassRec ptDrawingAreaClassRec = {
  {
    /* superclass         */    (WidgetClass) &xmDrawingAreaClassRec,
    /* class_name         */    "Different_visual_window",
    /* widget_size        */    sizeof(XmDrawingAreaRec),
    /* class_initialize   */    NULL,
    /* class inited part  */    NULL,
    /* class_inited       */    FALSE,
    /* initialize         */    NULL,
    /* initialize hook    */    NULL,
    /* realize            */    Realize,
    /* actions            */    NULL,
    /* num_actions        */    0,
    /* resources          */    NULL,
    /* num_resources      */    0,
    /* xrm_class          */    NULLQUARK,
    /* compress_motion    */    FALSE,
    /* compress_exposure  */    FALSE,
    /* compress enter/exit*/    FALSE,
    /* visible_interest   */    FALSE,
    /* destroy            */    NULL,
    /* resize             */    XtInheritResize,
    /* expose             */    XtInheritExpose,
    /* set_values         */    NULL,
    /* set values hook    */    NULL,
    /* set values almost  */    XtInheritSetValuesAlmost,
    /* get values hook    */    NULL,
    /* accept_focus       */    NULL,
    /* version            */    XtVersionDontCheck,
    /* callback list      */    NULL,
    /* default trans      */    NULL,
    /* query qeometry     */    NULL,
    /* disp accelerators  */    NULL,
    /* extension          */    NULL,
  },
  {                                     /* composite class record */
    /* geometry mgr proc  */    XtInheritGeometryManager,
    /* set changed proc   */    XtInheritChangeManaged,
    /* add a child        */    XtInheritInsertChild,
    /* remove a child     */    XtInheritDeleteChild,
    /* extension          */    NULL,
  },
  {                                     /* constraint class record */
    /* resources	  */    NULL,
    /* num_resources	  */    0,
    /* constraint_size    */    0,
    /* initialize	  */    NULL,
    /* destroy		  */    NULL,
    /* set_values	  */    NULL,
    /* extension          */    NULL,
  },
  {                                     /* manager class record */
    /* translations	  */    NULL,
    /* syn_resources	  */    NULL,
    /* num_syn_resources	    */    0,
    /* syn_constraint_resources	    */    NULL,
    /* num_syn_constraint_resources */    0,
    /* parent_process	  */    NULL,
    /* extension          */    NULL,
  },
  {                                     /* drawing area class record */
    /* mumble		  */    0,
  }

};

/* this is the pointer to the widget's class */

#if defined(VMS) && !defined(__DECC)
#pragma nostandard
#endif
externaldef (ptDrawingAreaWidgetClass)
#if defined(VMS) && !defined(__DECC)
#pragma standard
#endif
XmDrawingAreaWidgetClass ptDrawingAreaWidgetClass = &ptDrawingAreaClassRec;

/* <- treg */



extern	Pressed_Button(), Released_Button_1(), Moved_Mouse(), Pressed_Key(),
	Released_Key(), Refresh_Window(), Timer_Alarm(), Leave_Picture(),
	Enter_Picture(), Configure_Window(), Change_Font_Family(),
	Change_Font_Size(), Change_Font_Style(), Change_Units(),
	Dismiss_Brush_Dialog(), Dismiss_Line_Dialog(), Dismiss_Pattern_Dialog(),
	Delete_Edit_Pattern_Dialog(), Set_Sample(), Set_Sample_Pattern(),
	Set_Standard_Size(), Set_Picture_Size(), Scale_Size(),
	Clicked_On_Icon(), Set_Resolution(), Create_Icon_Button (),
	Change_Aspect_Ratio(), Print_File_Callback(); Refresh_Key(),
	Dismiss_Color_Dialog(), Clicked_On_Icon(),
	Color_Mix_Apply(), Color_Mix_OK(), Color_Mix_Cancel(),
	Color_Mix_Apply_Reply(), Change_Pr_H_Alignment(),
	Change_Pr_V_Alignment(), Change_Pr_Output_Device(),
	Change_Pr_Send_To (), Change_Pr_Output_Format(), Change_Pr_Printer(),
	Print_2_File_Callback(), Clicked_In_Pos_Window(),
	Position_Moved_Mouse(), Released_In_Pos_Window(), Refresh_Pos_Window(),
	Change_Resize_Crop_Or_Scale (), Change_Grid_Size_Val (),
	Grid_Size_Button (), Change_Resolution (), Released_Button_2(),
	Released_Button_3(), Pressed_Button2();
#ifdef I18N_MULTIBYTE
extern	Change_Font_Local();
#endif /* I18N_MULTIBYTE */


/*
 * Routine to get UID file name
 */
static void get_uid_filename(default_file)
    char **default_file;
{
#ifdef VMS

#ifdef NO_I18N
    *default_file = XtMalloc (strlen ("SYS$LIBRARY:.UID") +
			      strlen ("DECW$PAINT") + 1);
    sprintf (*default_file, "SYS$LIBRARY:%s.UID", "DECW$PAINT");
#else
    *default_file = XtMalloc (strlen ("DECW$PAINT") + 1);
    sprintf (*default_file, "%s", "DECW$PAINT");
#endif

#else

    extern char *getenv();
/* get_uid_filename looks up the environment variable UIDDIR, and attempts
   to find a uid file with the correct name in it. */

    char *def_name, *uiddir, *uid_name;
    int uiddir_len;
    FILE *fopen(), *uid_fd;

#ifdef NO_I18N
    *default_file = XtMalloc (strlen ("/usr/lib/X11/uid/.uid") +
			      strlen ("DXpaint") + 1);
    sprintf (*default_file, "/usr/lib/X11/uid/%s.uid", "DXpaint");
#else
    *default_file = XtMalloc (strlen ("DXpaint") + 1);
    sprintf (*default_file, "%s", "DXpaint");
#endif

    if ((uiddir = getenv("UIDDIR")) == NULL) return;
    uiddir_len = strlen (uiddir);
    if ((def_name = rindex(*default_file, '/')) == NULL) {
	def_name = *default_file;
    }
    else def_name++;
    uiddir_len += strlen (def_name) + 2; /* 1 for '/', 1 for '\0' */
    uid_name = (char *) XtMalloc(uiddir_len * sizeof (char));
    strcpy (uid_name, uiddir);
    strcat (uid_name, "/");
    strcat (uid_name, def_name);
    if ((uid_fd = fopen(uid_name, "r")) == NULL)
    {
	XtFree(uid_name);
    }
    else
    {
	fclose (uid_fd);
	XtFree (*default_file);
	*default_file = uid_name;
    }

#endif
}
 

void Clear_No_Undo_Actions ()
{
    quitting = opening = FALSE;
}

void Client_Message (w, event, params, num_params)
    Widget w;
    XEvent *event;
    char **params;
    int num_params;
{
	Atom take_focus_atom1, take_focus_atom2;

/*
 * When the user clicks on the title bar of a window, the window manager sends
 * a client message to the owner.  This message is a hint that the client
 * should set the input focus to the clicked window.
 */
        take_focus_atom1 = XInternAtom(disp, "DECWmTakeFocus", TRUE);
        take_focus_atom2 = XInternAtom(disp, "DEC_WM_TAKE_FOCUS", TRUE);

        if (event->xclient.message_type == take_focus_atom1 ||
            event->xclient.message_type == take_focus_atom2)
	    XSetInputFocus (disp, pwindow, RevertToParent, 
			    event->xclient.data.l[0]);
}


static char shell_translation_table[] =
         "<Message>: Client_Message()";

#define NUM_ATABLE_ENTRIES 17
/* jj-port */
static XtActionsRec action_table [] =
{
	{"Pressed_Button", (XtActionProc) Pressed_Button},
	{"Pressed_Button2", (XtActionProc) Pressed_Button2},
	{"Released_Button_1", (XtActionProc) Released_Button_1},
	{"Released_Button_2", (XtActionProc) Released_Button_2},
	{"Released_Button_3", (XtActionProc) Released_Button_3},
	{"Moved_Mouse",	(XtActionProc) Moved_Mouse},              
	{"Pressed_Key",	(XtActionProc) Pressed_Key},
	{"Released_Key", (XtActionProc) Released_Key},
	{"Refresh_Window", (XtActionProc) Refresh_Window},
	{"Timer_Alarm", (XtActionProc) Timer_Alarm},
	{"Configure_Window", (XtActionProc) Configure_Window},
	{"Client_Message", (XtActionProc) Client_Message},
	{"Refresh_Key", (XtActionProc) Refresh_Key},
	{"Clicked_In_Pos_Window", (XtActionProc) Clicked_In_Pos_Window},
	{"Position_Moved_Mouse", (XtActionProc) Position_Moved_Mouse},
	{"Released_In_Pos_Window", (XtActionProc) Released_In_Pos_Window},
	{"Refresh_Pos_Window", (XtActionProc) Refresh_Pos_Window}
};


/*   need to store widget's id in order to reference that widget independantly 
 *   later on.
 */
void Create_Callback (w, widget_id)
    Widget w;
    int	   *widget_id;
{
    widget_ids [*widget_id] = w;
}


/* treg -> */

static void Realize (w, valuemask, attributes)
    Widget               w;
    Mask                 *valuemask;
    XSetWindowAttributes *attributes;
{
    XmDrawingAreaWidget ww;

    ww = (XmDrawingAreaWidget) w;

    attributes->colormap = paint_colormap;
    *valuemask = *valuemask | CWColormap;

    XtCreateWindow (w, InputOutput, visual_info->visual, *valuemask,
		    attributes);
}

Widget WindowCreate (parent, name, arglist, argCount)
    Widget parent;
    char   *name;
    Arg    *arglist;
    int    argCount;
{
    return (XtCreateWidget (name, (WidgetClass)ptDrawingAreaWidgetClass, parent, 
			    arglist, argCount));
}


/* <- treg */


/*
 *
 * ROUTINE:  Pop_Widget
 *
 * ABSTRACT:
 *
 * pop the widget to the front of the screen
 *
 */
void Pop_Widget (w)
    Widget w;
{
/*
    XtWidgetGeometry request, reply;
    XtGeometryResult result;

    request.request_mode = CWStackMode;
    request.stack_mode = Above;

    result = XtMakeGeometryRequest(w, &request, &reply);
*/
    XRaiseWindow (disp, XtWindow (XtParent (w)));
}

/* static Widget msgbox=0; */

/*
 *
 * ROUTINE:  Unmap_Message_Box
 *
 * ABSTRACT: 
 *
 * Unmanage the message box
 *   
 */

static void Unmap_Message_Box(w, stuff, reason)
    Widget w;
    caddr_t stuff;
    int reason;
{
    struct msg_list *tmp;

    if (main_msg_list == NULL) {
	XtUnmanageChild (msgbox);
	if (exiting_paint) {
	    Exit_Paint (0);
	}
    }
    else {
/* pop message of front of queue */
	Fetch_Set_Attribute (msgbox, XmNmessageString, main_msg_list->msg);
	tmp = main_msg_list;
	main_msg_list = main_msg_list->next;
	XtFree ((char *)tmp);
    }
}

/*
 *
 * ROUTINE:  Append_Message
 *
 * ABSTRACT:
 *
 * Append message to the queue of messages to be displayed.
 *
 */
void Append_Message (label)
    char *label;
{
    struct msg_list *tmp;

/* put message at end of queue */
    tmp = main_msg_list;
    if (tmp == NULL) {
        main_msg_list = (struct msg_list *)XtMalloc (sizeof (struct msg_list));
        tmp = main_msg_list;
    }
    else {
        while (tmp->next != NULL)
            tmp = tmp->next;
        tmp->next = (struct msg_list *)XtMalloc (sizeof (struct msg_list));
        tmp = tmp->next;
    }
    tmp->msg = label;
    tmp->next = NULL;
}

/*
 *
 * ROUTINE:  Display_Message
 *
 * ABSTRACT: 
 *
 * Displays the given message with the given push button label
 *   
 */
void Display_Message( label )
    char *label;
{
    Widget w;
/*
 * If toplevel is not realized (messaging has not yet begun), just stick the
 * message on the queue.
 */

    if (!messaging_begun) {
        Append_Message (label);
        return;
    }

/* if the message dialog has not been created, create it. */
    if (!msgbox) {
	if (Fetch_Widget ("main_message_box", main_widget,
			  &msgbox) != MrmSUCCESS)
	    DRM_Error ("can't fetch message box");
/* remove the cancel button */
	w = XmMessageBoxGetChild (msgbox, XmDIALOG_CANCEL_BUTTON);
	XtUnmanageChild (w);
/* remove the help button */
	w = XmMessageBoxGetChild (msgbox, XmDIALOG_HELP_BUTTON);
	XtUnmanageChild (w);
    }
/*
 * If the message dialog is currently being displayed with a message, stick
 * this message on the queue. Otherwise, put up the message dialog with this
 * message.
 */
    if (XtIsManaged (msgbox)) {
	Append_Message (label);
    }
    else {
	Fetch_Set_Attribute (msgbox, XmNmessageString, label);
	XtManageChild (msgbox);	
    }
}

/*
 * If there are messages on the queue, display the first message off the queue.
 * If the queue is empty, just return.
 */
void Begin_Messaging ()
{
    struct msg_list *tmp;
    char *label;

    messaging_begun = TRUE;
    if (main_msg_list == NULL)
        return;
    XSync (disp, 0);
    tmp = main_msg_list;
    label = tmp->msg;
    main_msg_list = main_msg_list->next;
    XtFree ((char *)tmp);
    Display_Message (label);
}

Unmap_Help (w, tag, reason)
    Widget		    w;
    caddr_t		    tag;
    XmAnyCallbackStruct    *reason;
{
#if !HYPERHELP
    XtUnmanageChild (w);
#endif
}

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      Create_Help_Dialog
**      Creates the help widget
**
**  FORMAL PARAMETERS:
**
**      topic - topic string
**
**--
**/
void Create_Help_Dialog( topic )
    char            *topic;
{
    long bc, status;
/*     static Widget help_widget = 0; */
    static XmString HelpLibrary = NULL;
    XmString tmp_xmstr;

    Set_Cursor_Watch (pwindow);

#if HYPERHELP
    DXmHelpSystemDisplay
    (
	hyperhelp_context,
	PAINT_HELP,
	"topic",
	topic,
	help_error,
	NoPaintHelp
    );
#else
    if (HelpLibrary == NULL)
	HelpLibrary = DXmCvtOStoCS (PAINT_HELP, &bc, &status);
	   
/* Bring up the help widget with the appropriate topic string */
/* Reuse if any available */
    if (!help_widget) {
/*
	if (Fetch_Widget ("help_dialog_box", toplevel,
			  &help_widget) != MrmSUCCESS)
*/
	if (Fetch_Widget ("help_dialog_box", main_widget,
			  &help_widget) != MrmSUCCESS)
	DRM_Error ("can't fetch help box");
	Set_Attribute (help_widget, DXmNlibrarySpec, HelpLibrary);
    }

    tmp_xmstr = DXmCvtOStoCS (topic, &bc, &status);
    Set_Attribute (help_widget, DXmNfirstTopic, tmp_xmstr);
    XmStringFree (tmp_xmstr);

    XtManageChild (help_widget);
    Pop_Widget (help_widget);
#endif
    Set_Cursor (pwindow, current_action);
}

/* Help callback */
static void
help (w, topic, reason)				/* user pushed the button */
    Widget   w;
    char *topic;
    XmAnyCallbackStruct *reason;
{
    Create_Help_Dialog (topic);
}

static void Help_On_Context (w, tag, reason)
    Widget w;
    caddr_t tag;
    XmAnyCallbackStruct *reason;
{
    DXmHelpOnContext (toplevel, FALSE);
}


void To_Top ( w, scroll_bar_id, info )
    int                        w;
    int                        *scroll_bar_id;
    XmScrollBarCallbackStruct *info;
{
    int pixel = info->pixel;

    if (Finished_Action())
	switch (*scroll_bar_id) {
	    case VERTICAL_SCROLL_BAR_ID :
		Move_Picture (picture_x + pic_xorigin,
			      MIN ((pimage_ht - pwindow_ht),
			           ((int)picture_y + (int)pic_yorigin + 
				    (int)pixel)));
		Set_Attribute (widget_ids[PAINT_V_SCROLL_BAR], XmNvalue,
			       picture_y + pic_yorigin);
		break;
	    case HORIZONTAL_SCROLL_BAR_ID :
		Move_Picture (MIN ((pimage_wd - pwindow_wd),
				   ((int)picture_x + (int)pic_xorigin +	
				    (int)pixel)),
			      picture_y + pic_yorigin);
		Set_Attribute (widget_ids[PAINT_H_SCROLL_BAR], XmNvalue,
			       picture_x + pic_xorigin);
		break;
	}
}


void To_Bottom ( w, scroll_bar_id, info )
    int                        w;
    int                        *scroll_bar_id;
    XmScrollBarCallbackStruct *info;
{
    int pixel = info->pixel;

    if (Finished_Action())
	switch (*scroll_bar_id) {
	    case VERTICAL_SCROLL_BAR_ID :
		Move_Picture (picture_x + pic_xorigin, 
			      MAX(0, ((int)picture_y + (int)pic_yorigin + 
				      (int)pixel - (int)pwindow_ht)));
		Set_Attribute (widget_ids[PAINT_V_SCROLL_BAR], XmNvalue,
			       picture_y + pic_yorigin);
		break;
	    case HORIZONTAL_SCROLL_BAR_ID :
		Move_Picture (MAX (0, ((int)picture_x + (int)pic_xorigin + 
				       (int)pixel - (int)pwindow_wd)),
			     picture_y + pic_yorigin);
		Set_Attribute (widget_ids[PAINT_H_SCROLL_BAR], XmNvalue,
			       picture_x + pic_xorigin);
		break;
	}
}



/* Horizontal scroll bar callbacks */
void Hbar_Change( w, wclose, info )
    int                        w;
    int                        wclose;
    XmScrollBarCallbackStruct *info;
{
  if (Finished_Action())
    if (info->event)	
	Move_Picture(info->value, picture_y + pic_yorigin);
}


void Hbar_Increment( w, wclose, info )
    int                        w;
    int                        wclose;
    XmScrollBarCallbackStruct *info;
{
    if (Finished_Action())
	Move_Picture(info->value, picture_y + pic_yorigin);
}


/*
void hbar_drag( w, wclose, info )
    int                        w;
    int                        wclose;
    XmScrollBarCallbackStruct *info;
{
    if (Finished_Action())
	Scroll_Picture( info->value, pic_yorigin );
}
*/

/* Vertical scroll bar callbacks */
void Vbar_Change( w, wclose, info )
    int                        w;
    int                        wclose;
    XmScrollBarCallbackStruct *info;
{
/*
	printf( "reason= %d, value= %d\n", info->reason, info->value );
*/
  if (Finished_Action())
    if (info->event)	
	Move_Picture(picture_x + pic_xorigin, info->value);
}

void Vbar_Increment( w, wclose, info )
    int                        w;
    int                        wclose;
    XmScrollBarCallbackStruct *info;
{
    if (Finished_Action())
	Move_Picture(picture_x + pic_xorigin, info->value);
}

/*
void vbar_drag ( w, wclose, info )
    int                        w;
    int                        wclose;
    XmScrollBarCallbackStruct *info;
{
    if (Finished_Action())
	Scroll_Picture( pic_xorigin, info->value );
}         
*/

static void Edit_Command (w, edit_id, info)
    Widget   w;
    int      *edit_id;
    XmAnyCallbackStruct *info;
{
/* dl - 10/5/88 set global noting the picture contents may have changed */
  if (Finished_Action()) {
    switch (*edit_id) {
	case EDIT_UNDO_ID :
	    if (!XtIsSensitive (w))
		return;
	    if (undo_available) {
		Undo (info->event->xbutton.time);
		if (exiting_paint)
                    return;
	    }
	    break;
	case EDIT_CUT_ID :
	    if (!XtIsSensitive (w))
		return;
	    Cut(info->event->xbutton.time);
	    break;
	case EDIT_COPY_ID :
	    if (!XtIsSensitive (w))
		return;
	    Copy(info->event->xbutton.time);
	    break;
	case EDIT_QUICK_COPY_ID :
	    Quick_Copy ();
	    break;
	case EDIT_PASTE_ID :
	    Paste(info->event->xbutton.time);
	    break;
	case EDIT_CLEAR_ID :
	    Clear_Piece(); 
	    break;
	case EDIT_INVERT_ID :
	    Invert();
	    break;
	case EDIT_SCALE_ID :
	    moved_only = FALSE;
	    Create_Scale_Dialog (*edit_id);
	    break;
	case EDIT_CROP_ID :
	    if (paint_view == NORMAL_VIEW)
		Crop();
	    else
		SP_Crop ();
	    if (exiting_paint)
		return;
	    break;
	case EDIT_CLEAR_WW_ID :
	    Clear_Work_Window ();
	    break;
	case EDIT_SELECT_ALL_ID :
	    Select_All();
	    break;
	case EDIT_SCALE_PICTURE_ID :
	    Create_Scale_Dialog (*edit_id);
            break;
    }
    picture_changed = TRUE;
    pixmap_changed = TRUE;
  }
}


           
/*
 *
 * ROUTINE:  Set_Grid
 *
 * ABSTRACT: 
 *
 * Toggle between grid on/off
 *
 */           
void Set_Grid()
{
    if (grid_on) {
	grid_on = FALSE;
	Fetch_Set_Attribute (widget_ids[OPTIONS_GRID_BUTTON], XmNlabelString,
			     "T_GRID_ON");
	Delete_Grid();
    }
    else {
	grid_on = TRUE;
	Fetch_Set_Attribute (widget_ids[OPTIONS_GRID_BUTTON], XmNlabelString,
			     "T_GRID_OFF");
	Display_Grid( pic_xorigin, pic_yorigin, pwindow_wd, pwindow_ht);
/* refresh zoom magnifier so it will be in front of grid jj-10/12/88 */
	if (zoomed)
	    Refresh_Magnifier (zoom_xorigin, zoom_yorigin, zoom_width, 
			       zoom_height);
    }
}


           
static void Option_Command( w, options_id )
    Widget  w;
    int	    *options_id;
{
void Set_Zoom(); /* jj-port */
static void Set_Writing_Mode();

  if (Finished_Action()) {
    switch (*options_id) {
    	case OPTIONS_PICTURE_SIZE_ID :
	    Create_Picture_Shape_Dialog ();
	    break;
    	case OPTIONS_OPAQUE_ID :
	    if (opaque_fill)
		Set_Attribute (widget_ids[OPAQUE_TOGGLE], XmNset, TRUE);
	    else
		Set_Writing_Mode();
	    break;
    	case OPTIONS_TRANSPARENT_ID :
	    if (!opaque_fill)
		Set_Attribute (widget_ids[TRANSPARENT_TOGGLE], XmNset, TRUE);
	    else
		Set_Writing_Mode();
	    break;
	case OPTIONS_GRID_ID :
	    Set_Grid ();
	    break;
	case OPTIONS_BRUSHES_ID :
	    Create_Brush_Dialog();
	    break;
	case OPTIONS_PATTERNS_ID :
	    Create_Pattern_Dialog();
	    break;
	case OPTIONS_LINES_ID :
	    Create_Line_Dialog();
	    break;
	case OPTIONS_COLOR_ID :
	    Create_Color_Dialog();
	    break;
	case OPTIONS_COLOR_MIX_ID :
	    Create_Color_Mix_Dialog();
	    break;
	case OPTIONS_GRID_SIZE_ID :
	    Create_Grid_Size_Dialog();
	    break;
	case OPTIONS_ZOOM_ID :
	    Set_Zoom(w);
	    break;
	case OPTIONS_EDIT_PATTERN_ID :
	    Create_Edit_Pattern_Dialog();
	    break;
    	case OPTIONS_PAINT_VIEW_ID :
	    if (paint_view == NORMAL_VIEW)
		Set_Attribute (widget_ids[OPTIONS_PAINT_VIEW_BUTTON], 
			       XmNset, TRUE);
	    else {
		Exit_Select_Portion ();
		if (exiting_paint)
                    return;
	    }
	    break;
    	case OPTIONS_FULL_VIEW_ID :
	    if (paint_view == FULL_VIEW)
		Set_Attribute (widget_ids[OPTIONS_FULL_VIEW_BUTTON],
			       XmNset, TRUE);
	    else {
		Select_Portion ();
		if (exiting_paint)
                    return;
	    }
	    break;
    }
  }
  else {
    switch (*options_id) {
	case OPTIONS_OPAQUE_ID :
	    if (opaque_fill)
		Set_Attribute (widget_ids[OPAQUE_TOGGLE], XmNset, TRUE);
	    else
		Set_Attribute (widget_ids[OPAQUE_TOGGLE], XmNset, FALSE);
	    break;
	case OPTIONS_TRANSPARENT_ID :
	    if (opaque_fill)
		Set_Attribute(widget_ids[TRANSPARENT_TOGGLE], XmNset, FALSE);
	    else
		Set_Attribute(widget_ids[TRANSPARENT_TOGGLE], XmNset, TRUE);
	    break;
	case OPTIONS_PAINT_VIEW_ID :
	    if (paint_view == NORMAL_VIEW)
		Set_Attribute (widget_ids[OPTIONS_PAINT_VIEW_BUTTON], 
			       XmNset, TRUE);
	    else 
		Set_Attribute (widget_ids[OPTIONS_PAINT_VIEW_BUTTON],
                               XmNset, FALSE);
	    break;
	case OPTIONS_FULL_VIEW_ID :
	    if (paint_view == FULL_VIEW)
		Set_Attribute (widget_ids[OPTIONS_FULL_VIEW_BUTTON],
			       XmNset, TRUE);
	    else
		Set_Attribute (widget_ids[OPTIONS_FULL_VIEW_BUTTON],
			       XmNset, FALSE);
	    break;
    }
  }
}


void Set_File_Format (new_format, button)
    int new_format;
    int button;
{
    int prv_file_color;

    if (file_format == new_format) {
	return;
    }

    file_format = new_format;
    if ((!button) && (write_dialog != 0)) {
	switch (file_format) {
	    case XBITMAP_FORMAT :
		XmToggleButtonSetState (widget_ids[X11_FILE_FORMAT_TOGGLE],
					 TRUE, TRUE);
		XmToggleButtonSetState (widget_ids[DDIF_FILE_FORMAT_TOGGLE],
					 FALSE, TRUE);
		break;
	    case DDIF_FORMAT :
		XmToggleButtonSetState (widget_ids[DDIF_FILE_FORMAT_TOGGLE],
					 TRUE, TRUE);
		XmToggleButtonSetState (widget_ids[X11_FILE_FORMAT_TOGGLE],
					 FALSE, TRUE);
		break;
	}
    }

    if (file_format == XBITMAP_FORMAT) {
	if (file_color != SAVE_BW) {
	    prv_file_color = file_color;
	    file_color = SAVE_BW;	    
	    if (write_dialog != 0) {
		if (prv_file_color == SAVE_COLOR) {
		    XmToggleButtonSetState (widget_ids[FILE_COLOR_COLOR_TOGGLE],
					     FALSE, TRUE);
		}
		else {
		    XmToggleButtonSetState (widget_ids[FILE_COLOR_GRAY_TOGGLE],
					     FALSE, TRUE);
		}
		XmToggleButtonSetState (widget_ids[FILE_COLOR_BW_TOGGLE],
					 TRUE, TRUE);
	    }	    
	}
    }
}

void Set_File_Color (new_color, button)
    int new_color;
    int button;
{
    int prv_file_color, prv_file_format;

    if (file_color == new_color) {
	return;
    }

    prv_file_color = file_color;
    file_color = new_color;

    if ((!button) && (write_dialog != 0)) {
	switch (prv_file_color) {
	    case SAVE_COLOR :
		XmToggleButtonSetState (widget_ids[FILE_COLOR_COLOR_TOGGLE],
					 FALSE, TRUE);
		break;
	    case SAVE_GRAY :
		XmToggleButtonSetState (widget_ids[FILE_COLOR_GRAY_TOGGLE],
					 FALSE, TRUE);
		break;
	    case SAVE_BW :
		XmToggleButtonSetState (widget_ids[FILE_COLOR_BW_TOGGLE],
					 FALSE, TRUE);
		break;
	}
	switch (file_color) {
	    case SAVE_COLOR :
		XmToggleButtonSetState (widget_ids[FILE_COLOR_COLOR_TOGGLE],
					 TRUE, TRUE);
		break;
	    case SAVE_GRAY :
		XmToggleButtonSetState (widget_ids[FILE_COLOR_GRAY_TOGGLE],
					 TRUE, TRUE);
		break;
	    case SAVE_BW :
		XmToggleButtonSetState (widget_ids[FILE_COLOR_BW_TOGGLE],
					 TRUE, TRUE);
		break;
	}
    }

    if ((file_color == SAVE_COLOR) || (file_color == SAVE_GRAY)) {
	if (file_format == XBITMAP_FORMAT) {
	    file_format = DDIF_FORMAT;
	    if (write_dialog != 0) {
		XmToggleButtonSetState (widget_ids[DDIF_FILE_FORMAT_TOGGLE],
					 TRUE, TRUE);
		XmToggleButtonSetState (widget_ids[X11_FILE_FORMAT_TOGGLE],
					 FALSE, TRUE);
	    }
	}
    }
}



void Change_File_Format( w, f_format, r )
    Widget w;					/* pulldown menu */
    int *f_format;
    XmToggleButtonCallbackStruct *r;		/* just pick up the boolean */
{
    if (r->event) {
	switch (*f_format) {
	    case X11_FILE_FORMAT_ID :
		if (file_format == DDIF_FORMAT) {
		    XmToggleButtonSetState (widget_ids[DDIF_FILE_FORMAT_TOGGLE],
					     FALSE, TRUE);
		    Set_File_Format (XBITMAP_FORMAT, TRUE);
		}
		else {
		    XmToggleButtonSetState (widget_ids[X11_FILE_FORMAT_TOGGLE],
					     TRUE, TRUE);
		}
		break;
	    case DDIF_FILE_FORMAT_ID :
		if (file_format == XBITMAP_FORMAT) {
		    XmToggleButtonSetState (widget_ids[X11_FILE_FORMAT_TOGGLE],
					     FALSE, TRUE);
		    Set_File_Format (DDIF_FORMAT, TRUE);
		}
		else {
		    XmToggleButtonSetState (widget_ids[DDIF_FILE_FORMAT_TOGGLE],
					     TRUE, TRUE);
		}
		break;
	}
    }
}


void Change_File_Color (w, f_color, r)
    Widget w;					/* pulldown menu */
    int *f_color;
    XmToggleButtonCallbackStruct *r;		/* just pick up the boolean */
{
    if (r->event) {
	if (file_color == *f_color) {
	    switch (file_color) {
		case SAVE_COLOR :
		    XmToggleButtonSetState (widget_ids[FILE_COLOR_COLOR_TOGGLE],
					     TRUE, TRUE);
		    break;
		case SAVE_GRAY :
		    XmToggleButtonSetState (widget_ids[FILE_COLOR_GRAY_TOGGLE],
					     TRUE, TRUE);
		    break;
		case SAVE_BW :
		    XmToggleButtonSetState (widget_ids[FILE_COLOR_BW_TOGGLE],
					     TRUE, TRUE);
		    break;
	    }
	}
	else {
	    switch (file_color) {
		case SAVE_COLOR :
		    XmToggleButtonSetState (widget_ids[FILE_COLOR_COLOR_TOGGLE],
					     FALSE, TRUE);
		    break;
		case SAVE_GRAY :
		    XmToggleButtonSetState (widget_ids[FILE_COLOR_GRAY_TOGGLE],
					     FALSE, TRUE);
		    break;
		case SAVE_BW :
		    XmToggleButtonSetState (widget_ids[FILE_COLOR_BW_TOGGLE],
					     FALSE, TRUE);
		    break;
	    }
	    Set_File_Color (*f_color);
	}
    }
}


void Create_Write_Dialog()
{
Arg args[5];
int argcnt;

/*
 * Fetch the write file selection widget and file format toggle buttons if
 * necessary
 */ 

    Set_Cursor_Watch (pwindow);
    if (!write_dialog) {
	if (Fetch_Widget ("write_file_box", main_widget,
		          &write_dialog) != MrmSUCCESS) {
	    DRM_Error ("can't fetch write_dialog box");
	}

/* Set the toggle button values */
	if (file_format == DDIF_FORMAT) {
	    Set_Attribute (widget_ids[DDIF_FILE_FORMAT_TOGGLE],
			   XmNset, TRUE);
	}
	else {
	    Set_Attribute (widget_ids[X11_FILE_FORMAT_TOGGLE],
			   XmNset, TRUE);
	}
	switch (file_color) {
	    case SAVE_COLOR :
		Set_Attribute (widget_ids[FILE_COLOR_COLOR_TOGGLE],
			       XmNset, TRUE);
		break;
	    case SAVE_GRAY :
		Set_Attribute (widget_ids[FILE_COLOR_GRAY_TOGGLE],
			       XmNset, TRUE);
		break;
	    case SAVE_BW :
		Set_Attribute (widget_ids[FILE_COLOR_BW_TOGGLE],
			       XmNset, TRUE);
		break;
	}

	switch (visual_info->class) {
	    case StaticGray :
		XtSetSensitive (widget_ids[FILE_COLOR_GRAY_TOGGLE],
			       INSENSITIVE);
	    case GrayScale :
		XtSetSensitive (widget_ids[FILE_COLOR_COLOR_TOGGLE],
			       INSENSITIVE);
		break;
	}
    }

    XtManageChild (write_dialog);
    Pop_Widget (write_dialog);
    Set_Cursor (pwindow, current_action);
}

void Create_Read_Dialog()
{

/* Fetch the read file selection widget if necessary */
    Set_Cursor_Watch (pwindow);
	if( !read_dialog )
	    if	(Fetch_Widget ("read_file_box", main_widget,
		               &read_dialog) != MrmSUCCESS)
	        DRM_Error ("can't fetch read dialog box");

	XtManageChild( read_dialog );
	Pop_Widget (read_dialog);
    Set_Cursor (pwindow, current_action);
}


void Create_Include_Dialog()
{

/* Fetch the read file selection widget if necessary */
    Set_Cursor_Watch (pwindow);
    if (!include_dialog)
	if (Fetch_Widget ("include_file_box", main_widget,
		           &include_dialog) != MrmSUCCESS)
	    DRM_Error ("can't fetch read dialog box");

    XtManageChild (include_dialog);
    Pop_Widget (include_dialog);
    Set_Cursor (pwindow, current_action);
}


void Read_File_Callback (w, reason, info)
    Widget   w;
    int      *reason;
    XmFileSelectionBoxCallbackStruct *info;
{
    int st = TRUE;
    long bc, status;
    char *tmpstr;

    if (Finished_Action())
    {
	if (*reason == FILE_SELECTION_ACTIVATE_ID)
	{
	    if (info->length != 0) {
		tmpstr = (char *)DXmCvtCStoOS (info->value, &bc, &status);
/*		tmpstr = (char *)DXmCvtCStoFC (info->value); */
		if (tmpstr != NULL)
		{
		    strcpy (temp_file, tmpstr); 
		    XtFree (tmpstr);
		    tmpstr = NULL;
		    if (strlen (temp_file) != 0)
		    {
			Read_File();
			if (exiting_paint)
			    return;
		    }
		    else
			st = FALSE;
		}
		else
		{
		    st = FALSE;
		}
	    }
	    else
	    {
		st = FALSE;
	    }
	    if (!st) {
		Display_Message ("T_NO_FILE_NAME");
		return;
	    }
	}
	else {
	    XtUnmanageChild (w);
	}
    }
}

void Include_File_Callback (w, reason, info)
    Widget   w;
    int      *reason;
    XmFileSelectionBoxCallbackStruct *info;
{
    int st = TRUE;
    char *tmpstr;
    long bc, status;

    if (Finished_Action()) {
	if (*reason == FILE_SELECTION_ACTIVATE_ID) {
	    if (info->length != 0) {
		tmpstr = (char *)DXmCvtCStoOS (info->value, &bc, &status);
/*		tmpstr = (char *)DXmCvtCStoFC (info->value); */
		if (tmpstr != NULL)
		{
		    strcpy (temp_file, tmpstr); 
		    XtFree (tmpstr);
		    tmpstr = NULL;
		    if (strlen (temp_file) != 0)
		    {
			Include_File();
			if (exiting_paint)
			    return;
		    }
		    else
			st = FALSE;
		}
		else
		{
		    st = FALSE;
		}
	    }
	    else
	    {
		st = FALSE;
	    }
	    if (!st)
	    {
		Display_Message ("T_NO_FILE_NAME");
		return;
	    }
	}
	else
	{
	    XtUnmanageChild (w);
	}
    }
}

/*
 *  If write file was called from quit or open.
 */
Continue_After_Write_File ()
{
    if (opening) {
	Create_Read_Dialog ();
	opening = FALSE;
    }

    if (quitting)
	Exit_Paint(0);
}

void Write_File_Callback( w, reason, info )
    Widget   w;
    int	     *reason;
    XmFileSelectionBoxCallbackStruct *info;
{
    int st = TRUE;
    long bc, status;
    char *tmpstr;

    if (Finished_Action()) {
	switch (*reason) {
	    case FILE_SELECTION_ACTIVATE_ID :
		if (info->length != 0) {
		    tmpstr = (char *)DXmCvtCStoOS (info->value, &bc, &status);
/*		    tmpstr = (char *)DXmCvtCStoFC (info->value); */
		    if (tmpstr != NULL)
		    {
			strcpy (temp_file, tmpstr); 
			XtFree (tmpstr);
			tmpstr = NULL;
			if ((strlen (temp_file) != 0) || 
			    (strlen (cur_file) != 0)) {
			    if (strlen (temp_file) != 0)
				strcpy (cur_file, temp_file);
			    Write_File();
/* if Write file was successful then it unmanaged the write dialog */
			    if (!XtIsManaged (w)) {
				Continue_After_Write_File ();
			    }
			}
			else
			    st = FALSE;
		    }
		    else {
			st = FALSE;
		    }
		}
		else {
		    st = FALSE;
		}
		if (!st) {
		    Display_Message ("T_NO_FILE_NAME");
		    return;
		}
		break;
	    case FILE_SELECTION_CANCEL_ID :
		opening = FALSE;
		quitting = FALSE;
		XtUnmanageChild (w);
	}
    }
}



static void Question_Reply (w, reply, reason)
    Widget   w;
    int	     *reply;
    int	     reason;
{
    switch (*reply) {
	case DQ_YES_ID :
	    switch (default_question_question) {
		case EXIT_QUESTION :
		    if (strlen(cur_file) == 0) {
			quitting = TRUE;
			Create_Write_Dialog();
		    }
		    else {
			Write_File();
    			if ( exiting_paint )
			    Exit_Paint(0);
		    }
		    break;
		case OPEN_QUESTION :
		    opening = TRUE;
		    Create_Write_Dialog();
		    break;
		default :
		    break;
	    }
	    XtUnmanageChild (default_question_dialog);
	    break;
	case DQ_NO_ID :
	    switch (default_question_question) {
		case EXIT_QUESTION :
		    Exit_Paint(0);
		    break;
		case OPEN_QUESTION :
		    Create_Read_Dialog ();
		    break;
		default :
		    break;
	    }
	    XtUnmanageChild (default_question_dialog);
	    break;
	case DQ_CANCEL_ID :
	    switch (default_question_question) {
		case EXIT_QUESTION :
		case OPEN_QUESTION :
		    break;
		default :
		    break;
	    }
	    XtUnmanageChild (default_question_dialog);
	    break;
	case DQ_HELP_ID :
	    switch (default_question_question) {
		case EXIT_QUESTION :
#ifdef I18N_BUG_FIX
		    Create_Help_Dialog("file_exit");
#endif /* I18N_BUG_FIX */
		    break;
		case OPEN_QUESTION :
#ifdef I18N_BUG_FIX
		    Create_Help_Dialog("file_open");
#endif /* I18N_BUG_FIX */
		    break;
		default :
		    break;
	    }
	    break;
    }
}

void AI_Error_Exit_Reply (w, reply, reason)
    Widget   w;
    int	     *reply;
    int      reason;
{
#ifdef EPIC_CALLABLE
    switch (*reply) {
	case AI_ERROR_EXIT_YES_ID :
	    Exit_Paint (0);
	    break;
	case  AI_ERROR_EXIT_NO_ID :
	    switch (ai_error_reason) {
		case AI_ERR_NO_RESPOND :
		    break;
		case AI_ERR_BROKEN_LINK :
		    run_as_child = FALSE;
		    break;
	    }
	    break;
	case  AI_ERROR_EXIT_HELP_ID :
	    return;
    }
    XtUnmanageChild (ai_error_caution_box);
#endif
}
    

void Create_Default_Question_Dialog (label)
    char* label;
{
    int wd1, wd2, sp, ht1, ht2, yoff;

    Set_Cursor_Watch (pwindow);
    if (!default_question_dialog) {
	if (Fetch_Widget ("default_question_dialog", main_widget,
			  &default_question_dialog) != MrmSUCCESS)
	    DRM_Error ("can't fetch default question dialog box");

	XtManageChild (default_question_dialog);
/* Center the label with respect to the pix map */
	ht1 = XtHeight (widget_ids[DQ_PIXMAP]);
	ht2 = XtHeight (widget_ids[DQ_LABEL]);
	yoff = ((ht1 - ht2) / 2);
	Set_Attribute (widget_ids[DQ_LABEL], XmNtopOffset, yoff);
    }

/* set the label */
    Fetch_Set_Attribute (widget_ids[DQ_LABEL], XmNlabelString, label);
/* center the push buttons */
    wd1 = XtWidth (widget_ids[DQ_BUTTONS_ROW_COLUMN]);
    wd2 = XtWidth (widget_ids[DQ_YES_BUTTON]);
    sp = (wd1 - (4 * wd2)) / 3;
    Set_Attribute (widget_ids[DQ_BUTTONS_ROW_COLUMN], XmNspacing, sp);

    XtManageChild (default_question_dialog);
    Set_Cursor (pwindow, current_action);
}

void Create_Quit_Dialog()
{
    Create_Default_Question_Dialog ("T_EXIT_MSG");
    default_question_question = EXIT_QUESTION;
}


void Create_Open_Caution_Box()
{
    Create_Default_Question_Dialog ("T_OPEN_MSG");
    default_question_question = OPEN_QUESTION;
}


void Create_AI_Error_Caution_Box (reason)
    int	reason;
{
#ifdef EPIC_CALLABLE    
    Widget w;

    Set_Cursor_Watch (pwindow);
/* If necessary fetch the sp crop cation box. */
	if ( !ai_error_caution_box ) {
	    if	(Fetch_Widget ("ai_error_caution_box", main_widget,
		               &ai_error_caution_box) != MrmSUCCESS)
	        DRM_Error ("can't fetch ai error caution box");
/* remove the help button */
/*
	    w = XmMessageBoxGetChild (ai_error_caution_box,
				      XmDIALOG_HELP_BUTTON);
	    XtUnmanageChild (w);
*/
	}

	switch (reason) {
	    case AI_ERR_NO_RESPOND :
		Fetch_Set_Attribute (ai_error_caution_box, XmNmessageString,
				     "T_AI_ERROR_NO_RESPOND_MSG");
		break; 
	    case AI_ERR_BROKEN_LINK :
		Fetch_Set_Attribute (ai_error_caution_box, XmNmessageString,
				     "T_AI_ERROR_TERMINATE_MSG");
		break; 
	}
	ai_error_reason = reason;
	XtManageChild (ai_error_caution_box);
    Set_Cursor (pwindow, current_action);
#endif
}


static void File_Command( w, file_id, reason)
    Widget   w;
    int	     *file_id;
    int	     reason;
{
  if (Finished_Action()) {
    switch (*file_id) {
	case FILE_QUIT_ID :
	    if (picture_changed)
		Create_Quit_Dialog ();
	    else
	        Exit_Paint(0);
	    break;
/*
 * save a file
 */
	case FILE_SAVE_ID :
	    if( strlen(cur_file) == 0 )
		Create_Write_Dialog();
	    else
		if (picture_changed)
		    Write_File();
	    break;
	case FILE_SAVE_AS_ID :
	    Create_Write_Dialog();
	    break;
/*
 * read a file
 */
	case FILE_OPEN_ID :
	    if (picture_changed)
		Create_Open_Caution_Box ();
	    else
		Create_Read_Dialog();
	    break;
/*
 * include a file
*/
	case FILE_INCLUDE_ID :
	    Create_Include_Dialog();
	    break;
#ifdef PRINT
/*
 * print a file
 */
	case FILE_PRINT_AS_ID :
	    if( !print_dialog )
		Create_Print_Dialog();
	    else {
		XtManageChild( print_dialog );
	        Pop_Widget (print_dialog);
	    }
	    break;
	case FILE_PRINT_ID :
	    if( !print_dialog )
		Create_Print_Dialog();
	    else
		Print_File();
	    break;
#endif
    }
  }
}


/*
 *
 * ROUTINE:  Zoom_Off
 *
 * ABSTRACT: 
 *
 * turn zoom off
 *
 */           
void Zoom_Off()
{
	Fetch_Set_Attribute (widget_ids[OPTIONS_ZOOM_BUTTON], XmNlabelString,
			     "T_ZOOM_ON" );
	End_Zoom();
}
/*
 *
 * ROUTINE:  Set_Zoom
 *
 * ABSTRACT: 
 *
 * Toggle between grid on/off
 *
 */           
static void Set_Zoom(w)
    Widget w;
{
	if( zoomed ){
		Fetch_Set_Attribute ( w, XmNlabelString, "T_ZOOM_ON" );
		End_Zoom();
		}
	else{
		Fetch_Set_Attribute ( w, XmNlabelString, "T_ZOOM_OFF" );
		Begin_Zoom( pic_xorigin + MIN (pwindow_wd, pimage_wd) / 2,
			    pic_yorigin + MIN (pwindow_ht, pimage_ht) / 2 ); 
		}
}

/*
 *
 * ROUTINE:  Set_Edit_Buttons
 *
 * ABSTRACT: 
 *
 * Change the sensitivity of the buttons in the edit menu.
 * These buttons will be sensitive when an area is selected and
 * unsensitive when nothing is selected.  Not all buttons in
 * the edit menu will be changed, only those that make sense when
 * something is selected.
 *                    
 */
void Set_Edit_Buttons( state )
int state;
{
	XtSetSensitive (widget_ids [EDIT_CROP_BUTTON], 	state);
	XtSetSensitive (widget_ids [EDIT_CLEAR_BUTTON], state);
	XtSetSensitive (widget_ids [EDIT_CUT_BUTTON], state);
	XtSetSensitive (widget_ids [EDIT_COPY_BUTTON], state);
	XtSetSensitive (widget_ids [EDIT_QUICK_COPY_BUTTON], state);
	XtSetSensitive (widget_ids [EDIT_SCALE_BUTTON], state);
	XtSetSensitive (widget_ids [EDIT_INVERT_BUTTON], state);
}
/*
 *
 * ROUTINE:  Set_Undo_Button
 *
 * ABSTRACT:                   
 *
 * Append the previous action to the undo label
 *
 */
void Set_Undo_Button( action )
int action;
{
char tmp[UNDO_ACTION_LENGTH];
static int first = TRUE;
Arg args[2];

    if (undo_available) {
	if( first ){
		t_actions[LINE] = "DO_LINE";
		t_actions[RECTANGLE] = "DO_RECTANGLE";
		t_actions[SQUARE] = "DO_SQUARE";
		t_actions[ELLIPSE] = "DO_ELLIPSE";
		t_actions[CIRCLE] = "DO_CIRCLE";
		t_actions[POLYGON] = "DO_POLYLINE";
		t_actions[STROKE] = "DO_STROKE";
		t_actions[ERASE] = "DO_ERASE";
		t_actions[SELECT_RECT] = "DO_SELECT";
		t_actions[SELECT_AREA] = "DO_SELECT";
		t_actions[PENCIL] = "DO_PENCIL";
		t_actions[ARC] = "DO_ARC";
		t_actions[BRUSH] = "DO_BRUSH";
		t_actions[FLOOD] = "DO_FLOOD";
		t_actions[TEXT] = "DO_TEXT";
		t_actions[SPRAYCAN] = "DO_SPRAY";
		t_actions[COPY] = "DO_COPY";
		t_actions[PASTE] = "DO_PASTE";
		t_actions[INCLUDE] = "DO_INCLUDE";
/*
		t_actions[MOVE] = "DO_MOVE";
		t_actions[CLEAR] = "DO_CLEAR";
		t_actions[CUT] = "DO_CUT";
		t_actions[INVERT] = "DO_INVERT";
		t_actions[SCALE] = "DO_SCALE_TITLE";
		t_actions[CROP] = "DO_CROP";
*/
		t_actions[MOVE] = "DO_TRANSFORMATIONS";
		t_actions[CLEAR] = "DO_TRANSFORMATIONS";
		t_actions[CUT] = "DO_TRANSFORMATIONS";
		t_actions[INVERT] = "DO_TRANSFORMATIONS";
		t_actions[SCALE] = "DO_TRANSFORMATIONS";
		t_actions[CROP] = "DO_TRANSFORMATIONS";

		t_actions[CHANGE_PICTURE_SIZE] = "DO_CHANGE_PICTURE_SIZE";
		t_actions[SCALE_PICTURE] = "DO_SCALE_PICTURE";
		t_actions[FULLVIEW_CROP] = "DO_FULLVIEW_CROP";
		t_actions[QUICK_COPY] = "DO_QUICK_COPY";
		t_actions[CLEAR_WW] = "DO_CLEAR_WW";

		first = FALSE;
		}

	strcpy (tmp, "T_UN");
	if( action != NO_ACTION ) {
	    undo_action = action;
	    action_str = t_actions[action];
	    strcat (tmp, action_str);
	}
	else
	    strcat (tmp, "DO");
	toggle_undo = TRUE; /* used in set_redo_button */

/* Set undo sensitivity */

	if( action == NO_ACTION )
	    XtSetSensitive (widget_ids [EDIT_UNDO_BUTTON], INSENSITIVE);
	else
	    XtSetSensitive (widget_ids [EDIT_UNDO_BUTTON], SENSITIVE);
	Fetch_Set_Attribute (widget_ids [EDIT_UNDO_BUTTON], XmNlabelString, tmp );
    }
}            

/*
 *
 * ROUTINE:  Set_Redo_Button
 *
 * ABSTRACT: 
 *
 * Append the previous action to the redo label
 *
 */
void Set_Redo_Button()
{
char tmp[UNDO_ACTION_LENGTH];

	if( toggle_undo ){
		strcpy (tmp, "T_RE");
		toggle_undo = FALSE;
		}
	else{
		strcpy (tmp, "T_UN");
		toggle_undo = TRUE;
		}

	strcat (tmp, action_str);
	Fetch_Set_Attribute ( widget_ids [EDIT_UNDO_BUTTON], XmNlabelString, tmp );
}

/*
 *
 * ROUTINE:  Undo_Button_State
 *
 * ABSTRACT: 
 *
 * Return TRUE if last undo action was an UNDO( redo shows in the menu)
 * Return FALSE if last undo action was a REDO( undo shows in the menu)
 *
 */
int Undo_Button_State()
{
	if( !toggle_undo )
		return( TRUE);
	else
		return(FALSE);
}

void Save_Toggle_State ()
{
    prv_toggle_undo = toggle_undo;
}

void Restore_Toggle_State ()
{
    toggle_undo = prv_toggle_undo;
}

/*
 *
 * ROUTINE:  Set_Writing_Mode
 *
 * ABSTRACT: 
 *
 * Toggle between opaque and transparent writing modes
 *
 */
static void Set_Writing_Mode()
{
    if (opaque_fill) {
	opaque_fill = FALSE;
	Set_Attribute (widget_ids[OPAQUE_TOGGLE], XmNset, FALSE);
	Set_Attribute (widget_ids[TRANSPARENT_TOGGLE], XmNset, TRUE);
	cur_fill_style = FillStippled;
    }
    else {
	opaque_fill = TRUE;
	Set_Attribute (widget_ids[OPAQUE_TOGGLE], XmNset, TRUE);
	Set_Attribute (widget_ids[TRANSPARENT_TOGGLE], XmNset, FALSE);
	cur_fill_style = FillOpaqueStippled;
    }
    XSetFillStyle( disp, Get_GC(GC_SD_FILL), cur_fill_style );
    XSetFillStyle( disp, Get_GC(GC_SD_SQUARE_BRUSH), cur_fill_style );
    XSetFillStyle( disp, Get_GC(GC_SD_ROUND_BRUSH), cur_fill_style );
    XSetFillStyle( disp, Get_GC(GC_PD_FILL), cur_fill_style );
    XSetFillStyle( disp, Get_GC(GC_PD_SQUARE_BRUSH), cur_fill_style );
    XSetFillStyle( disp, Get_GC(GC_PD_ROUND_BRUSH), cur_fill_style );
/* jj-01/04/89   Spraycan obeys writing mode */
    XSetFillStyle( disp, Get_GC(GC_PD_SPRAY), cur_fill_style );

    if (num_hipts > 0) {
	if (select_on) {
	    Stop_Highlight_Blink ();
	}

	if (select_rectangle && opaque_fill) {
	    if (clip_mask)
	    {
                XFreePixmap (disp, clip_mask);
                clip_mask = 0;
            }
            XSetClipMask (disp, Get_GC (GC_PD_MASK), None);
        }
	else {
            if (select_rectangle) {
                clip_mask = XCreatePixmap (disp, DefaultRootWindow(disp),
                                           select_width, select_height, 1);

/* if could not allocate clip_mask, reset the writing mode to Opaque and
   print a message */
                if (clip_mask == 0) {
                    Display_Message ("T_NO_MEM_FOR_TRANSPARENT");
                    Set_Writing_Mode ();
                    return;
                }
            }
            Create_Clip_Mask ();
        }

         if (select_on) {
            Copy_Select_To_Image (PMX_TO_IX (select_x), PMY_TO_IY (select_y),
                                  TRUE);
            Start_Highlight_Blink ();
        }
    }
}                             



/*
 *
 * ROUTINE:  Set_Titlebar
 *
 * ABSTRACT: 
 *
 * Concatenate: application name + delimeter + filename
 *		set the icon name to be the filename - if there is no file name
 *		then set the icon name to be the application name.
 */
void Set_Titlebar()
{
    char title[80], delimeter[10];

    if (Get_UIL_Value (title, "T_DECPAINT") != SUCCESS) 
        DRM_Error ("Could not fetch value from UID file");
    if (strlen (cur_name)) {
	if (Get_UIL_Value (delimeter, "T_DELIMETER") != SUCCESS)
	    DRM_Error ("Could not fetch value from UID file");
	strcat (title, delimeter);
	strcat (title, cur_name);
#ifdef R5_XLIB
	XmbSetWMProperties(disp, XtWindow(toplevel), title, cur_name,
		NULL, 0, NULL, NULL, NULL);
#else
	Set_Attribute (toplevel, XtNiconName, cur_name);
#endif /* R5_XLIB */
    }
    else {
#ifdef R5_XLIB
	XmbSetWMProperties(disp, XtWindow(toplevel), title, title,
		NULL, 0, NULL, NULL, NULL);
#else
	Set_Attribute (toplevel, XtNiconName, title);
#endif /* R5_XLIB */
    }

#ifndef R5_XLIB
    Set_Attribute( toplevel, XtNtitle, title );
#endif /* R5_XLIB */
    strcpy (last_file_name, cur_file);    

/* since the title has changed, update the print filename */
    Update_Print_Filename ();
}



/*									   
 *      List of names which the UID file must see
 */
 
MrmRegisterArg 
reglist [] =	/* jj-port */
{
/* variables */
    { "pimage_wd", 0 },						    /* 01 */
    { "pimage_ht", 0 },						    /* 02 */
    { "position_ht", 0 },					    /* 03 */
/* procedures */
    { "Read_File_Callback", (caddr_t) Read_File_Callback },	    /* 01 */
    { "Write_File_Callback", (caddr_t) Write_File_Callback },	    /* 02 */
    { "Change_File_Format", (caddr_t) Change_File_Format },	    /* 03 */
    { "File_Command", (caddr_t) File_Command },			    /* 04 */
    { "Edit_Command", (caddr_t) Edit_Command },			    /* 05 */
    { "Option_Command", (caddr_t) Option_Command },		    /* 06 */
    { "Change_Font_Family", (caddr_t) Change_Font_Family },	    /* 07 */
    { "Change_Font_Size", (caddr_t) Change_Font_Size },		    /* 08 */
    { "help", (caddr_t) help },					    /* 09 */
    { "Hbar_Change", (caddr_t) Hbar_Change },			    /* 10 */
    { "Hbar_Increment", (caddr_t) Hbar_Increment },		    /* 11 */
    { "Vbar_Change", (caddr_t) Vbar_Change },			    /* 12 */
    { "Vbar_Increment", (caddr_t) Vbar_Increment },		    /* 13 */
    { "Dismiss_Brush_Dialog", (caddr_t) Dismiss_Brush_Dialog },	    /* 14 */
    { "Dismiss_Line_Dialog", (caddr_t) Dismiss_Line_Dialog },	    /* 15 */
    { "Dismiss_Pattern_Dialog", (caddr_t) Dismiss_Pattern_Dialog }, /* 16 */
    { "Delete_Edit_Pattern_Dialog", (caddr_t) Delete_Edit_Pattern_Dialog },
								    /* 17 */
    { "Set_Sample", (caddr_t) Set_Sample },			    /* 18 */
    { "Set_Sample_Pattern", (caddr_t) Set_Sample_Pattern },	    /* 19 */
    { "Set_Standard_Size", (caddr_t) Set_Standard_Size },	    /* 20 */
    { "Set_Picture_Size", (caddr_t) Set_Picture_Size },		    /* 21 */
    { "Change_Units", (caddr_t) Change_Units },			    /* 22 */
    { "Scale_Size", (caddr_t) Scale_Size },			    /* 23 */
    { "Question_Reply", (caddr_t) Question_Reply },		    /* 24 */
    { "Change_Font_Style", (caddr_t) Change_Font_Style },	    /* 25 */
    { "Unmap_Message_Box", (caddr_t) Unmap_Message_Box },	    /* 26 */
    { "Create_Callback", (caddr_t) Create_Callback },		    /* 27 */
    { "Set_Resolution", (caddr_t) Set_Resolution},		    /* 28 */
    { "Change_Aspect_Ratio", (caddr_t) Change_Aspect_Ratio },	    /* 29 */
    { "Print_File_Callback", (caddr_t) Print_File_Callback },	    /* 30 */
    { "Unmap_Help", (caddr_t) Unmap_Help },			    /* 31 */
    { "To_Bottom", (caddr_t) To_Bottom },			    /* 32 */
    { "To_Top", (caddr_t) To_Top },				    /* 33 */
    { "AI_Error_Exit_Reply", (caddr_t) AI_Error_Exit_Reply },       /* 34 */
    { "Change_Resolution", (caddr_t) Change_Resolution },	    /* 35 */
    { "Dismiss_Color_Dialog", (caddr_t) Dismiss_Color_Dialog },	    /* 36 */
    { "Create_Icon_Button", (caddr_t) Create_Icon_Button },	    /* 37 */
    { "Clicked_On_Icon", (caddr_t) Clicked_On_Icon },		    /* 38 */
    { "Color_Mix_Apply", (caddr_t) Color_Mix_Apply },		    /* 39 */
    { "Color_Mix_OK", (caddr_t) Color_Mix_OK },			    /* 40 */
    { "Color_Mix_Cancel", (caddr_t) Color_Mix_Cancel },		    /* 41 */
    { "Color_Mix_Apply_Reply", (caddr_t) Color_Mix_Apply_Reply },   /* 42 */
    { "Change_File_Color", (caddr_t) Change_File_Color },	    /* 43 */
    { "Change_Pr_H_Alignment", (caddr_t) Change_Pr_H_Alignment },   /* 44 */
    { "Change_Pr_V_Alignment", (caddr_t) Change_Pr_V_Alignment },   /* 45 */
    { "Change_Pr_Output_Device", (caddr_t) Change_Pr_Output_Device },  
								    /* 46 */
    { "Change_Pr_Send_To", (caddr_t) Change_Pr_Send_To },	    /* 47 */
    { "Change_Pr_Output_Format", (caddr_t) Change_Pr_Output_Format },
								    /* 48 */
    { "Change_Pr_Printer", (caddr_t) Change_Pr_Printer },	    /* 49 */
    { "Print_2_File_Callback", (caddr_t) Print_2_File_Callback },   /* 50 */
    { "Change_Resize_Crop_Or_Scale", (caddr_t) Change_Resize_Crop_Or_Scale },
								    /* 51 */
    { "Change_Grid_Size_Val", (caddr_t) Change_Grid_Size_Val },     /* 52 */
    { "Grid_Size_Button", (caddr_t) Grid_Size_Button },		    /* 53 */
    { "Include_File_Callback", (caddr_t) Include_File_Callback },   /* 54 */
    { "Help_On_Context", (caddr_t) Help_On_Context }		    /* 55 */
#ifdef I18N_MULTIBYTE
    ,{ "Change_Font_Local", (caddr_t) Change_Font_Local }	    /* 56 */
#endif /* I18N_MULTIBYTE */
};

#ifdef I18N_MULTIBYTE
/* we need 1 more */
#define NUM_UIL_NAMES 59
#else
/* if 55 procedures + 3 varibles = 58 names (change line below to match) */ 
#define NUM_UIL_NAMES 58
#endif /* I18N_MULTIBYTE */

/*             
 * Open the DRM Hierarchy and Fetch the main widget..
 */
Widget Build_Main (p)		
	Widget p;
{                       
    Arg args[10];
    int argcnt;

    static char filename_placeholder[80];

/* set the size of the application */
    argcnt = 0;
    XtSetArg (args[argcnt], XtNwidth, main_wd);
    ++argcnt;
    XtSetArg (args[argcnt], XtNheight, main_ht);
    ++argcnt;
    XtSetValues (toplevel, args, argcnt);


/* Make action tables known */
	XtAddActions (action_table, NUM_ATABLE_ENTRIES);
	Set_Attribute (p, XmNtranslations, 
		       XtParseTranslationTable (shell_translation_table));

	get_uid_filename (db_filename_vec);


#if (((XmVERSION == 1) && (XmREVISION >= 2)) || XmVERSION >= 2)

        if (MrmOpenHierarchyPerDisplay (
    		XtDisplay(toplevel),
#else

	if (MrmOpenHierarchy (

#endif

    		1, 
    		db_filename_vec, 
    		NULL, 
    		&s_DRMHierarchy) !=

			MrmSUCCESS)

	    DRM_Error ("can't open DRM hierarchy");

/* fill in reglist with run time values */
	reglist[0].value = (caddr_t) pimage_wd; /* jj-port */
	reglist[1].value = (caddr_t) pimage_ht; /* jj-port */
	reglist[2].value = (caddr_t) position_ht; /* jj-port */

	MrmRegisterNames (reglist, NUM_UIL_NAMES);
	if (MrmFetchWidget (s_DRMHierarchy, "main", p, & main_widget,
                            & dummy_class) != MrmSUCCESS)
	    DRM_Error ("can't fetch main window");

/* treg -> *
* define the Picture Window Widget *
	argcnt = 0;
	XtSetArg (args[argcnt], XtNwidth, picture_wd);
	argcnt++;
	XtSetArg (args[argcnt], XtNheight, picture_ht);
	argcnt++;
	XtSetArg (args[argcnt], XtNborderWidth, 0);
	argcnt++;
	XtSetArg (args[argcnt], XtNdepth, pdepth);
	argcnt++;
* leave these for later (when the colomap has been set up)  ->
	XtSetArg (args[argcnt], XtNborderColor, colormap[BLACK].pixel);
	argcnt++;
	XtSetArg (args[argcnt], XtNbackground, colormap[WHITE].pixel);
	argcnt++;
*

* to be fixed later *
*
	XtSetArg(args[n], XmNcolormap, paint_colormap);
        argcnt++;
*

	picture_widget = WindowCreate (widget_ids[PAINT_WINDOW],
				       "picture_window", args, argcnt++);

	Fetch_Set_Attribute (picture_widget, XmNtranslations,
			     "main_translation_table");
* <- treg */

/* Set the pixmaps for the icon box push buttons */
	Set_Icon_Pixmaps ();

/* create tab grops */
	XmAddTabGroup (widget_ids[ICON_WINDOW]);
	XmAddTabGroup (widget_ids[PAINT_H_SCROLL_BAR]);
	XmAddTabGroup (widget_ids[PAINT_V_SCROLL_BAR]);

/* The variable picture_widget is used ubiquitously and this saves me from */
/* changing it in all those places */

	picture_widget = widget_ids[PICTURE_WINDOW];  /* treg */

	XtInstallAllAccelerators (picture_widget, main_widget);
	Create_Icon_Menu ();
	if (strlen (cur_name) == 0) {
	    if (Get_UIL_Value (filename_placeholder, "T_UNTITLED") != SUCCESS)
		DRM_Error ("Could not fetch value from UID file");
	    strcpy (cur_name, filename_placeholder);
	}
#ifndef I18N_BUG_FIX
/* Set title will be delayed until toplevel is realized */
	Set_Titlebar ();
#endif /* I18N_BUG_FIX */
	if (pdepth == 1) {
	    XtSetSensitive (widget_ids [OPTIONS_COLOR_BUTTON], INSENSITIVE);
	    XtSetSensitive (widget_ids [OPTIONS_COLOR_MIX_BUTTON], INSENSITIVE);
	    Set_Icon_Button_Sensitivity (8, INSENSITIVE);
	}

	XtManageChild (main_widget);
/*	XtManageChild (picture_widget);  *//* treg */
}


/*             
 * Display the main window, the menu bar and the pulldowns.
 */
Display_Main()
{
    XWindowAttributes win_attrs;
/*
    XSetWindowAttributes set_win_attrs;
    Window tmp_win;
*/
    Arg args[5];
    int argcnt;
    int test_int;
    int test_flag = FALSE;
    int extra_wd, extra_ht;

/* treg -> */
    if (visual_info->visual != XDefaultVisual (disp, screen)) {
 
	Set_Attribute (picture_widget, XmNdepth, pdepth);

	Set_Attribute (picture_widget, XmNcolormap, paint_colormap);
	Set_Attribute (picture_widget, XmNbackground, colormap[WHITE].pixel);
	Set_Attribute (picture_widget, XmNborder, colormap[BLACK].pixel);

    }
/* <- treg */

/* Display the widgets */
    XtRealizeWidget (toplevel);

#ifdef I18N_BUG_FIX
/* Set title will be delayed until toplevel is realized */
	Set_Titlebar ();
#endif /* I18N_BUG_FIX */

    if (visual_info->visual == XDefaultVisual (disp, screen)) {
	XSetWindowColormap (disp, XtWindow (toplevel), paint_colormap);
    }
/* treg -> */
    else {
	XInstallColormap (disp, paint_colormap);
    }
/* <- treg */

/*
    argcnt = 0;
    XtSetArg (args[argcnt], XmNwidth, picture_wd);
    argcnt++;
    XtSetArg (args[argcnt], XmNheight, picture_ht);
    argcnt++;
    XtSetValues (widget_ids[BORDER_WINDOW], args, argcnt);
    XLowerWindow (disp, XtWindow (widget_ids[BORDER_WINDOW]));
*/

    if (!exiting_paint) {
	extra_wd = (screen_ht < MIN_HT_FOR_BIG_ICONS) ? 80 : 124;
	max_picture_wd = screen_wd - extra_wd;
	extra_ht = 18 + XtHeight (widget_ids[MAIN_MENU_BAR]);
	max_picture_ht = screen_ht - extra_ht;

	picture_wd = MIN (pimage_wd, max_picture_wd);
	picture_ht = MIN (pimage_ht, max_picture_ht);

	pwindow = XtWindow(picture_widget);

	if (test_flag) {
	    XGetWindowAttributes (disp, pwindow, &win_attrs);
	    test_int = XMaxCmapsOfScreen (XDefaultScreenOfDisplay (disp));
	}

/* find pwindow_wd and pwindow_ht after it has been realized jj-10/11/88 */
	Find_Pwindow_Size();

/* set these values simultaneously for each scroll bar */	
/* First the horizontal scroll bar */
	argcnt = 0;
/* reset slider page increments dl- 9/30/88 */
	XtSetArg (args[argcnt], XmNpageIncrement, pwindow_wd);
	argcnt++;
/* make sure scroll bars are correct size */
	XtSetArg (args[argcnt], XmNsliderSize, pwindow_wd);
	argcnt++;
	XtSetValues (widget_ids[PAINT_H_SCROLL_BAR], args, argcnt);

/* Now the vertical scroll bar */
	argcnt = 0;
/* reset slider page increments dl- 9/30/88 */
        XtSetArg (args[argcnt], XmNpageIncrement, pwindow_ht);
	argcnt++;
/* make sure scroll bars are correct size */
	XtSetArg (args[argcnt], XmNsliderSize, pwindow_ht);
	argcnt++;

	XtSetValues (widget_ids[PAINT_V_SCROLL_BAR], args, argcnt);

/* Set min and max resize values. */
	argcnt = 0;
	XtSetArg (args[argcnt], XtNminWidth, main_min_wd);
	argcnt++;
	XtSetArg (args[argcnt], XtNminHeight, main_min_ht);
	argcnt++;
	XtSetArg (args[argcnt], XtNmaxWidth, screen_wd);
	argcnt++;
	XtSetArg (args[argcnt], XtNmaxHeight, screen_ht);
	argcnt++;
	XtSetValues (toplevel, args, argcnt);
    }

    Set_Cursor(pwindow, current_action);

/* Now that the application is being displayed, begin messaging */
    Begin_Messaging ();
}

void help_error (problem_string, status)
char    *problem_string;               
int     status;

{
    printf("%s, %x\n", problem_string, status);
}
