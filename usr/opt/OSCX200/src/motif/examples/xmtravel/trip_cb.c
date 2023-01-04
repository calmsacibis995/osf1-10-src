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
static char *rcsid = "@(#)$RCSfile: trip_cb.c,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/11/05 22:06:31 $";
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


/* Widget creation callback */

/*ARGSUSED*/
void t_create_widgets (widget, tag, call_data)
Widget          widget;
int             *tag;
XtPointer       call_data;
{
  int widget_num = *tag;

  l_widget_array [widget_num] = widget;

  switch ( widget_num ) {
  case find_b:
  case menu_t_find:
    XtSetSensitive (widget, False);
    break;
  default:
    break;
  }
}


/* Button callbacks */

/* Find flights button callback - gets flight no matter what */ 

/*ARGSUSED*/
void find_flights_activate (widget, client_data, call_data)
Widget		widget;
XtPointer	client_data;
XtPointer	call_data;
{

  int answer ;
  answer = RET_NONE;
  answer = Selection(widget,"select_flight",
                      RET_OK | RET_CANCEL | RET_HELP, XmDIALOG_CANCEL_BUTTON);

  if ( answer == RET_CANCEL )
      return;

  if ( WORKING_TIME ) {
    answer = RET_NONE; /* Calling Airline Computer */
    answer = Working (widget, "working_find_flights_call",
                      RET_CANCEL | RET_HELP, XmDIALOG_CANCEL_BUTTON);

    if ( answer == RET_CANCEL )
      return;

    answer = RET_NONE; /* Confirming Reservation */
    answer = Working (widget, "working_find_flights_confirm",
                      RET_CANCEL | RET_HELP, XmDIALOG_CANCEL_BUTTON);

    if ( answer == RET_CANCEL ) {
       answer = RET_NONE; /* Reservation Canceled */
       answer = Information (widget,"info_find_flights_cancel",
                      RET_OK , XmDIALOG_CANCEL_BUTTON);
      return;
    }
    else { 
       answer = RET_NONE;
       answer = Information(widget,/*Reservation Made...Please pck yr bags!*/
          "info_find_flights_made", RET_OK , XmDIALOG_CANCEL_BUTTON);
      return;
    }
  }


}   


/* Cancel button callback - removes window */

/*ARGSUSED*/
void cancel_activate (widget, client_data, call_data)
Widget		widget;
XtPointer	client_data;
XtPointer	call_data;
{
  XtUnmapWidget (globalData.t_toplevel);
}   


/* Data operations callbacks */

/* Callbacks to change global variables when data is changed */

/*ARGSUSED*/
void number_changed (widget, client_data, call_data)
Widget		widget;
XtPointer	client_data;
XtPointer	call_data;
{
  char 	*number_string;
  number_string = XmTextGetString (widget);
  if ( !strcmp (number_string, "") ) {
    globalData.number_entered = False;
    if ( XtIsSensitive (l_widget_array [find_b]) ) {
      XtSetSensitive (l_widget_array [find_b], False);
      XtSetSensitive (l_widget_array [menu_t_find], False);
    }
  }
  else {
    globalData.number_entered = True;
    if ( globalData.number_entered && globalData.date_entered &&
	globalData.air_select && globalData.ori_select && 
	globalData.dest_select ) {
      if ( !XtIsSensitive (l_widget_array [find_b]) ) {
	XtSetSensitive (l_widget_array [find_b], True);
	XtSetSensitive (l_widget_array [menu_t_find], True);
      }
    }
  }
  XtFree (number_string);
}   


/*ARGSUSED*/
void date_changed (widget, client_data, call_data)
Widget		widget;
XtPointer	client_data;
XtPointer	call_data;
{
  char 	*date_string;
  date_string = XmTextGetString (widget);
  if ( !strcmp (date_string, "") ) {
    globalData.date_entered = False;
    if ( XtIsSensitive (l_widget_array [find_b]) ) {
      XtSetSensitive (l_widget_array [find_b], False);
      XtSetSensitive (l_widget_array [menu_t_find], False);
    }
  }
  else {
    globalData.date_entered = True;
    if ( globalData.number_entered && globalData.date_entered &&
	globalData.air_select && globalData.ori_select && 
	globalData.dest_select ) {
      if ( !XtIsSensitive (l_widget_array [find_b]) ) {
	XtSetSensitive (l_widget_array [find_b], True);
	XtSetSensitive (l_widget_array [menu_t_find], True);
      }
    }
  }
  XtFree (date_string);
}   


/*ARGSUSED*/
void airlines_selected (widget, client_data, call_data)
Widget		widget;
XtPointer	client_data;
XtPointer	call_data;
{
  XmListCallbackStruct *cb = (XmListCallbackStruct *) call_data;

  if ( cb->selected_item_count == 0 ) {
    globalData.air_select = False;
    if ( XtIsSensitive (l_widget_array [find_b]) ) {
      XtSetSensitive (l_widget_array [find_b], False);
      XtSetSensitive (l_widget_array [menu_t_find], False);
    }
  }
  else {
    globalData.air_select = True;
    if ( globalData.number_entered && globalData.date_entered &&
	globalData.air_select && globalData.ori_select && 
	globalData.dest_select ) {
      if ( !XtIsSensitive (l_widget_array [find_b]) ) {
	XtSetSensitive (l_widget_array [find_b], True);
	XtSetSensitive (l_widget_array [menu_t_find], True);
      }
    }
  }
}   


/*ARGSUSED*/
void origin_selected (widget, client_data, call_data)
Widget		widget;
XtPointer	client_data;
XtPointer	call_data;
{
  Arg		args[3];
  int		n, sel_count;

  n = 0;
  XtSetArg (args[n], XmNselectedItemCount, &sel_count); n++;
  XtGetValues (widget, args, n);

  if ( sel_count == 0 ) {
    globalData.ori_select = False;
    if ( XtIsSensitive (l_widget_array [find_b]) ) {
      XtSetSensitive (l_widget_array [find_b], False);
      XtSetSensitive (l_widget_array [menu_t_find], False);
    }
  }
  else {
    globalData.ori_select = True;
    if ( globalData.number_entered && globalData.date_entered &&
	globalData.air_select && globalData.ori_select && 
	globalData.dest_select ) {
      if ( !XtIsSensitive (l_widget_array [find_b]) ) {
	XtSetSensitive (l_widget_array [find_b], True);
	XtSetSensitive (l_widget_array [menu_t_find], True);
      }
    }
  }
}   


/*ARGSUSED*/
void destination_selected (widget, client_data, call_data)
Widget		widget;
XtPointer	client_data;
XtPointer	call_data;
{
  Arg		args[3];
  int		n, sel_count;

  n = 0;
  XtSetArg (args[n], XmNselectedItemCount, &sel_count); n++;
  XtGetValues (widget, args, n);

  if ( sel_count == 0 ) {
    globalData.dest_select = False;
    if ( XtIsSensitive (l_widget_array [find_b]) ) {
      XtSetSensitive (l_widget_array [find_b], False);
      XtSetSensitive (l_widget_array [menu_t_find], False);
    }
  }
  else {
    globalData.dest_select = True;
    if ( globalData.number_entered && globalData.date_entered &&
	globalData.air_select && globalData.ori_select && 
	globalData.dest_select ) {
      if ( !XtIsSensitive (l_widget_array [find_b]) ) {
	XtSetSensitive (l_widget_array [find_b], True);
	XtSetSensitive (l_widget_array [menu_t_find], True);
      }
    }
  }
}   





/*ARGSUSED*/
void bill_client(widget, client_data, call_data)
Widget		widget;
XtPointer	client_data;
XtPointer	call_data;
{

  int answer ;

    answer = RET_NONE; /* Do you want to\nbill this client */
    answer = Question (widget, "ques_bill_client",
                      RET_OK | RET_CANCEL | RET_HELP, XmDIALOG_CANCEL_BUTTON);

    if ( answer == RET_CANCEL )
      return;

    answer = RET_NONE; /* Preparing Billing information */
    answer = Working (widget, "working_bill_client_prep",
                      RET_CANCEL | RET_HELP, XmDIALOG_CANCEL_BUTTON);

    if ( answer == RET_CANCEL ) {
       answer = RET_NONE; /* Client NOT Billed, ABORT */
       answer = Information (widget,"info_bill_client_abort",
                      RET_OK , XmDIALOG_CANCEL_BUTTON);
      return;
    }
    else { 
       answer = RET_NONE; /* Client Billed... */
       answer = Information (widget, "info_bill_client_done",
                       RET_OK , XmDIALOG_CANCEL_BUTTON);
      return;
    }


}   



/*ARGSUSED*/
void delete_client(widget, client_data, call_data)
Widget		widget;
XtPointer	client_data;
XtPointer	call_data;
{

  int answer ;

    answer = RET_NONE; /* Do you want to\ndelete this client */
    answer = Question (widget, "ques_delete_client",
                      RET_OK | RET_CANCEL | RET_HELP, XmDIALOG_CANCEL_BUTTON);

    if ( answer == RET_CANCEL ) {
       answer = RET_NONE; /* Client NOT deleted, ABORT */
       answer = Information (widget, "info_delete_client",
                      RET_OK | RET_HELP, XmDIALOG_CANCEL_BUTTON);
      return;
    }
    else { 
       answer = RET_NONE; /* Client delete left as\nan excercise for user */
       answer = Warning (widget, "warning_delete_client",
                      RET_OK | RET_HELP, XmDIALOG_CANCEL_BUTTON);
      return;
    }


}   


