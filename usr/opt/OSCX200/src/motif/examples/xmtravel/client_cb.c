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
#ifndef lint
static char *rcsid = "@(#)$RCSfile: client_cb.c,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/11/05 22:03:56 $";
#endif
/*
 * (c) Copyright 1989, 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 */
/*
 * Motif Release 1.2
 */

#include <Xm/Xm.h>
#include <Xm/DialogS.h>
#include <Xm/SelectioB.h>
#include <Xm/MessageB.h>
#include <Xm/Text.h>

#include "xmtravel.h"

/* local data */


/* client selection box declared locally */

Widget	client_sb;


/* Widget creation callback */

/*ARGSUSED*/
void c_create_widgets (widget, tag, call_data)
Widget          widget;
int             *tag;
XtPointer       call_data;
{
  int widget_num = *tag;

  l_widget_array [widget_num] = widget;

  switch ( widget_num ) {
  case coach:
  case non_smoking:
  case none_seat:
  case none_meal:
    XmToggleButtonSetState (widget, True, False);
    break;
  case save_b:
  case menu_c_save:
    XtSetSensitive (widget, False);
    break;
  default:
    break;
  }
}


/* Button callbacks */

/* Select client callback */

/*ARGSUSED*/
void client_select_activate (widget, client_data, call_data)
Widget		widget;
XtPointer	client_data;
XtPointer	call_data;
{
  Widget	client_list;
  Arg		args[10];
  int		n,  answer;

  /* Put up a warning dialog if changes need to be saved */

  if ( globalData.changed && globalData.name_entered ) {
    answer = RET_NONE;
    answer = SaveWarning (widget, "save_client_select" , 
			  RET_OK | RET_CANCEL | RET_HELP, XmDIALOG_OK_BUTTON);

    if ( answer == RET_CANCEL ) {
      return;
    }

    if ( answer == RET_SAVE ) {
      XtSetSensitive (l_widget_array [save_b], False);
      XtSetSensitive (l_widget_array [menu_c_save], False);
      globalData.changed = False;
    }
  }  

  /* Set up a selection dialog to show client names and allow the user */
  /* to select one.                                                    */
  
  n = 0;
  XtSetArg (args[n], XmNdialogStyle, XmDIALOG_MODELESS); n++;
  XtSetArg (args[n], XmNmustMatch, True); n++;
  XtSetArg (args[n], XmNautoUnmanage, False); n++;
  
  client_sb = XmCreateSelectionDialog(widget,"selection_client_select",args,n);
  
  XtUnmanageChild (XmSelectionBoxGetChild (client_sb,XmDIALOG_APPLY_BUTTON));
  XtAddCallback (client_sb, XmNokCallback, ok_response, NULL); 
  XtAddCallback (client_sb, XmNcancelCallback, cancel_response, NULL);
  XtAddCallback (client_sb, XmNhelpCallback, help_response, NULL);
  XtAddCallback (client_sb, XmNnoMatchCallback, nomatch_response, NULL);
  
  client_list = XmSelectionBoxGetChild (client_sb, XmDIALOG_LIST);
  for ( n = 0; n < XtNumber (names); n++ ) {
    XmListAddItem (client_list, XmStringCreateLtoR 
		   (names[n], XmSTRING_DEFAULT_CHARSET), 0);
  }
  XtManageChild (client_sb);
}


/* Save client callback */

/*ARGSUSED*/
void client_save_activate (widget, client_data, call_data)
Widget		widget;
XtPointer	client_data;
XtPointer	call_data;
{
  /* Disable save button and reset global variable */

  globalData.changed = False;
  XtSetSensitive (l_widget_array [save_b], False);
  XtSetSensitive (l_widget_array [menu_c_save], False);
}   


/* Select client callback */

/*ARGSUSED*/
void client_bill_activate (widget, client_data, call_data)
Widget		widget;
XtPointer	client_data;
XtPointer	call_data;
{
  Widget	client_list;
  Arg		args[10];
  int		n ; 


  /* Set up a selection dialog to show client names and allow the user */
  /* to select one.                                                    */
  
  n = 0;
  XtSetArg (args[n], XmNdialogStyle, XmDIALOG_MODELESS); n++;
  XtSetArg (args[n], XmNmustMatch, True); n++;
  XtSetArg (args[n], XmNautoUnmanage, False); n++;
  
  client_sb = XmCreateSelectionDialog (widget,"selection_client_bill", args, n);
  
  XtUnmanageChild (XmSelectionBoxGetChild (client_sb,XmDIALOG_APPLY_BUTTON));
  XtAddCallback (client_sb, XmNokCallback, bill_client, NULL) ;
  XtAddCallback (client_sb, XmNcancelCallback, cancel_response, NULL);
  XtAddCallback (client_sb, XmNhelpCallback, help_response, NULL);
  XtAddCallback (client_sb, XmNnoMatchCallback, nomatch_bill_delete, NULL);
  
  client_list = XmSelectionBoxGetChild (client_sb, XmDIALOG_LIST);
  for ( n = 0; n < XtNumber (names); n++ ) {
    XmListAddItem (client_list, XmStringCreateLtoR 
		   (names[n], XmSTRING_DEFAULT_CHARSET), 0);
  }
  XtManageChild (client_sb);
}


/*ARGSUSED*/
void client_delete_activate (widget, client_data, call_data)
Widget		widget;
XtPointer	client_data;
XtPointer	call_data;
{
  Widget	client_list;
  Arg		args[10];
  int		n;


  /* Set up a selection dialog to show client names and allow the user */
  /* to select one.                                                    */
  
  n = 0;
  XtSetArg (args[n], XmNdialogStyle, XmDIALOG_MODELESS); n++;
  XtSetArg (args[n], XmNmustMatch, True); n++;
  XtSetArg (args[n], XmNautoUnmanage, False); n++;
  
  client_sb = XmCreateSelectionDialog(widget,"selection_client_delete",args,n);
  
  XtUnmanageChild (XmSelectionBoxGetChild (client_sb,XmDIALOG_APPLY_BUTTON));
  XtAddCallback (client_sb, XmNokCallback, delete_client, NULL) ;
  XtAddCallback (client_sb, XmNcancelCallback, cancel_response, NULL);
  XtAddCallback (client_sb, XmNhelpCallback, help_response, NULL);
  XtAddCallback (client_sb, XmNnoMatchCallback, nomatch_bill_delete, NULL);
  
  client_list = XmSelectionBoxGetChild (client_sb, XmDIALOG_LIST);
  for ( n = 0; n < XtNumber (names); n++ ) {
    XmListAddItem (client_list, XmStringCreateLtoR 
		   (names[n], XmSTRING_DEFAULT_CHARSET), 0);
  }
  XtManageChild (client_sb);
}



/* Schedule trip callback */

/*ARGSUSED*/
void schedule_trip_activate (widget, client_data, call_data)
Widget		widget;
XtPointer	client_data;
XtPointer	call_data;
{
  Widget	toplevel, t_main_window = NULL;
  Arg		args[10];
  int		n ;
  MrmType	class;


  /* Fetch widgets for schedule trip window */

  if ( globalData.t_toplevel == NULL ) {
    n = 0;
    XtSetArg (args[n], XmNallowShellResize, True); n++;
    XtSetArg (args[n], XmNshellUnitType, Xm100TH_FONT_UNITS); n++;
    toplevel = XtAppCreateShell (NULL, "appshell_xmtravel", 
                                 topLevelShellWidgetClass,
				 XtDisplay (globalData.c_toplevel), args, n);
    
    if ( MrmFetchWidget (globalData.hierarchy, "trip_main_window", toplevel,
			 &t_main_window, &class) != MrmSUCCESS ) {
      printf ("Can't fetch %s\n", "trip_main_window");
      exit (1);
    }
  }    

  /* Set global variables for schedule trip window */

  globalData.number_entered = False;
  globalData.date_entered = False;

  globalData.ori_select = False;
  globalData.dest_select = False;
  
  /* Make the schedule trip window appear */

  if ( globalData.t_toplevel == NULL ) {
    globalData.t_toplevel = toplevel;
    XtManageChild (t_main_window);
    XtRealizeWidget (toplevel);
  }
  else {
    XtMapWidget (globalData.t_toplevel);
  }
}


/* Data components callbacks */

/* Callbacks to make toggle buttons act like radio buttons for seating */
/* preferences                                                         */

/*ARGSUSED*/
void first_class_changed (widget, client_data, call_data)
Widget		widget;
XtPointer	client_data;
XtPointer	call_data;
{
}   


/*ARGSUSED*/
void business_class_changed (widget, client_data, call_data)
Widget		widget;
XtPointer	client_data;
XtPointer	call_data;
{
}   


/*ARGSUSED*/
void coach_changed (widget, client_data, call_data)
Widget		widget;
XtPointer	client_data;
XtPointer	call_data;
{
}   


/*ARGSUSED*/
void non_smoking_changed (widget, client_data, call_data)
Widget		widget;
XtPointer	client_data;
XtPointer	call_data;
{
}   


/*ARGSUSED*/
void smoking_changed (widget, client_data, call_data)
Widget		widget;
XtPointer	client_data;
XtPointer	call_data;
{
}   


/*ARGSUSED*/
void aisle_changed (widget, client_data, call_data)
Widget		widget;
XtPointer	client_data;
XtPointer	call_data;
{
}   


/*ARGSUSED*/
void window_changed (widget, client_data, call_data)
Widget		widget;
XtPointer	client_data;
XtPointer	call_data;
{
}   


/*ARGSUSED*/
void none_seat_changed (widget, client_data, call_data)
Widget		widget;
XtPointer	client_data;
XtPointer	call_data;
{
}   


/* Callback to change global variables when client data changed */

/*ARGSUSED*/
void data_changed (widget, client_data, call_data)
Widget		widget;
XtPointer	client_data;
XtPointer	call_data;
{
  if ( !globalData.changed ) {
    globalData.changed = True;
    
    if ( globalData.name_entered ) {
      if ( !XtIsSensitive (l_widget_array [save_b]) ) {
	XtSetSensitive (l_widget_array [save_b], True);
	XtSetSensitive (l_widget_array [menu_c_save], True);
      }
    }
  }   
}


/* Callback to change global variables when client name changed */

/*ARGSUSED*/
void name_changed (widget, client_data, call_data)
Widget		widget;
XtPointer	client_data;
XtPointer	call_data;
{
  char 	*name_string;

  name_string = XmTextGetString (widget);
  if ( !strcmp (name_string, "") ) {
    globalData.name_entered = False;
    XtSetSensitive (l_widget_array [save_b], False);
    XtSetSensitive (l_widget_array [menu_c_save], False);
  }
  else {
    if ( !globalData.name_entered || !globalData.changed ) {
      globalData.name_entered = True;
      globalData.changed = True;
      XtSetSensitive (l_widget_array [save_b], True);
      XtSetSensitive (l_widget_array [menu_c_save], True);
    }
  }
  XtFree (name_string);
}   


/* Client selection box callbacks */

/* Display data if an existing client is selected */

/*ARGSUSED*/
void ok_response (widget, client_data, call_data)
Widget widget;
XtPointer client_data;
XtPointer call_data;
{
  XmSelectionBoxCallbackStruct *reason = 
    (XmSelectionBoxCallbackStruct *) call_data;
  int		answer;
  char  	*client;
  
  XtUnmanageChild (widget);

  XmStringGetLtoR (reason->value, XmSTRING_DEFAULT_CHARSET, &client);
  
  /* Show a working dialog to let the user know that the database is */
  /* being accessed.                                                 */

  if ( WORKING_TIME ) {

    answer = RET_NONE;
    answer = Working (widget, "working_ok_response", 
		      RET_CANCEL | RET_HELP, XmDIALOG_CANCEL_BUTTON);

    if ( answer == RET_CANCEL )
      return;
  }

  globalData.changed = True;
  globalData.name_entered = True;

}   


/* Remove the selection box if canceled */

/*ARGSUSED*/
void cancel_response (widget, client_data, call_data)
Widget          widget;
XtPointer 	client_data;
XtPointer 	call_data;
{
  XtUnmanageChild (widget);
}


/* Remove the selection box if help selected - this really should */
/* provide some help                                              */

/*ARGSUSED*/
void help_response (widget, client_data, call_data)
Widget          widget;
XtPointer 	client_data;
XtPointer 	call_data;
{
  XtUnmanageChild (widget);
}


/* If a non existent client is entered, try to match names.  If names   */
/* match, display data.  If no match, ask to add new client to database */

/*ARGSUSED*/
void nomatch_response (widget, client_data, call_data)
Widget          widget;
XtPointer 	client_data;
XtPointer 	call_data;
{
  int           answer;
  
  /* Show a working dialog to let the user know that the database is */
  /* being searched.                                                 */

  if ( WORKING_TIME ) {
    answer = RET_NONE;
    answer = Working (widget, "working_nomatch_response",
                      RET_CANCEL | RET_HELP, XmDIALOG_CANCEL_BUTTON);

    if ( answer == RET_CANCEL )
      return;
  }

}


/* Actions*/

/* Post the popup menu for the name field */

/* ARGSUSED */
void name_text_popup (widget, event, params, num_params)
Widget widget;
XEvent *event;
String *params;
Cardinal *num_params;
{
  static Widget name_popup = NULL;
  MrmType	class;

  if ( globalData.name_entered ) {
    if ( name_popup == NULL ) {
      if ( MrmFetchWidget (globalData.hierarchy, "name_popup", widget, 
			   &name_popup, &class) != MrmSUCCESS ) {
	printf ("Can't fetch %s\n", "name_popup");
	exit (1);
      }
    }
    
    XmMenuPosition (name_popup, (XButtonPressedEvent *) event);
    XtManageChild (name_popup);
  }
}


/* Frequent flier number text fields traversal actions */

/* ARGSUSED */
void move_left (widget, event, params, num_params)
Widget widget;
XEvent *event;
String *params;
Cardinal *num_params;
{
  XmProcessTraversal (widget, XmTRAVERSE_LEFT);
}


/* ARGSUSED */
void move_right (widget, event, params, num_params)
Widget widget;
XEvent *event;
String *params;
Cardinal *num_params;
{
  XmProcessTraversal (widget, XmTRAVERSE_RIGHT);
}

/* ARGSUSED */
void move_down (widget, event, params, num_params)
Widget widget;
XEvent *event;
String *params;
Cardinal *num_params;
{
  XmProcessTraversal (widget, XmTRAVERSE_CURRENT);
}


/* ARGSUSED */
void move_up (widget, event, params, num_params)
Widget widget;
XEvent *event;
String *params;
Cardinal *num_params;
{
  XmProcessTraversal (widget, XmTRAVERSE_UP);
}


/* ARGSUSED */
void nomatch_bill_delete (widget, client_data, call_data)
Widget          widget;
XtPointer       client_data;
XtPointer       call_data;
{

    Warning(widget, "warning_nomatch_bill", RET_OK | RET_HELP,
               XmDIALOG_CANCEL_BUTTON) ;

}

 
