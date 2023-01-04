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
static char *rcsid = "@(#)$RCSfile: dialog.c,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/11/05 22:04:25 $";
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
#include <Xm/MessageB.h>
#include <Xm/SelectioB.h>
#include <Xm/Label.h>
#include <Xm/PushB.h>
#include <Xm/FileSB.h>
#include <Xm/Form.h>

#include <X11/cursorfont.h>

#include "xmtravel.h"

/* Handles responses to a message dialog */

/* ARGSUSED */
void response (widget, client_data, call_data)
Widget          widget;
XtPointer 	client_data; 
XtPointer 	call_data;
{
  int *answer = (int *) client_data;
  XmAnyCallbackStruct *reason = (XmAnyCallbackStruct *) call_data;
  switch (reason->reason) {
  case XmCR_OK:
    *answer = RET_OK;
    break;
  case XmCR_CANCEL:
    *answer = RET_CANCEL;
    break;
  case XmCR_HELP:
    *answer = RET_HELP;
    break;
  default:
    return;
  }
}


/* Handles responses to a saving changes dialog */

/* ARGSUSED */
void save_response (widget, client_data, call_data)
Widget          widget;
XtPointer 	client_data; 
XtPointer 	call_data;
{
  int *answer = (int *) client_data;
  XmAnyCallbackStruct *reason = (XmAnyCallbackStruct *) call_data;
  switch (reason->reason) {
  case XmCR_OK:
    *answer = RET_SAVE;
    break;
  case XmCR_APPLY:
    *answer = RET_DISCARD;
    break;
  case XmCR_CANCEL:
    *answer = RET_CANCEL;
    break;
  case XmCR_HELP:
    *answer = RET_HELP;
    break;
  default:
    return;
  }
}


/* Timeout callback */

/* ARGSUSED */
void working_timer_activate (client_data, id)
XtPointer	client_data;
XtIntervalId	*id;
{
  int *answer = (int *) client_data;

  *answer = RET_DONE;
  return;
}


/* Posts a Question dialog */

int Question (widget, name, buttons, defbut)
Widget          widget;		/* needed for context */
char           *name;           /* name of widget */
int 		buttons;	/* RET_OK | RET_CANCEL | RET_HELP */
unsigned char	defbut;		/* for XmdefaultButtonType */
{
  int             	answer = RET_NONE;
  Widget          	dialog;	
  Arg             	args[10];
  int             	n = 0;
  XtAppContext    	context;

  n = 0;
  XtSetArg (args[n], XmNdialogStyle, XmDIALOG_FULL_APPLICATION_MODAL); n++;
  XtSetArg (args[n], XmNdefaultButtonType, defbut); n++;
  dialog = XmCreateQuestionDialog (widget, name, args, n);
  
  XtAddCallback (dialog, XmNokCallback, response, &answer);
  XtAddCallback (dialog, XmNcancelCallback, response, &answer);
  XtAddCallback (dialog, XmNhelpCallback, response, &answer);
  
  if ( !(RET_OK & buttons) )
    XtUnmanageChild (XmMessageBoxGetChild (dialog, XmDIALOG_OK_BUTTON));
  if ( !(RET_CANCEL & buttons) )
    XtUnmanageChild (XmMessageBoxGetChild (dialog,XmDIALOG_CANCEL_BUTTON));
  if ( !(RET_HELP & buttons) )
    XtUnmanageChild (XmMessageBoxGetChild (dialog,XmDIALOG_HELP_BUTTON));
  
  XtManageChild (dialog);
  
  context = XtWidgetToApplicationContext (widget);
  while ( answer == RET_NONE || XtAppPending (context) ) {
    XtAppProcessEvent (context, XtIMAll);
  }

  XtUnmanageChild (dialog);
  return answer;
}




/* Posts a Information dialog */

int Information (widget, name, buttons, defbut)
Widget          widget;		/* needed for context */
char           *name;           /* name of widget */
int 		buttons;	/* RET_OK | RET_CANCEL | RET_HELP */
unsigned char	defbut;		/* for XmdefaultButtonType */
{
  int             	answer = RET_NONE;
  Widget          	dialog;	
  Arg             	args[10];
  int             	n = 0;
  XtAppContext    	context;

  n = 0;
  XtSetArg (args[n], XmNdialogStyle, XmDIALOG_FULL_APPLICATION_MODAL); n++;
  XtSetArg (args[n], XmNdefaultButtonType, defbut); n++;
  dialog = XmCreateInformationDialog (widget, name, args, n);
  
  XtAddCallback (dialog, XmNokCallback, response, &answer);
  XtAddCallback (dialog, XmNcancelCallback, response, &answer);
  XtAddCallback (dialog, XmNhelpCallback, response, &answer);
  
  if ( !(RET_OK & buttons) )
    XtUnmanageChild (XmMessageBoxGetChild (dialog, XmDIALOG_OK_BUTTON));
  if ( !(RET_CANCEL & buttons) )
    XtUnmanageChild (XmMessageBoxGetChild (dialog,XmDIALOG_CANCEL_BUTTON));
  if ( !(RET_HELP & buttons) )
    XtUnmanageChild (XmMessageBoxGetChild (dialog,XmDIALOG_HELP_BUTTON));
  
  XtManageChild (dialog);
  
  context = XtWidgetToApplicationContext (widget);
  while ( answer == RET_NONE || XtAppPending (context) ) {
    XtAppProcessEvent (context, XtIMAll);
  }

  XtUnmanageChild (dialog);
  return answer;
}


/* Posts an error dialog */

int Error (widget, name, buttons, defbut)
Widget          widget;		/* needed for context */
char           *name;           /* name of widget */
int 		buttons;	/* RET_OK | RET_CANCEL | RET_HELP */
unsigned char	defbut;		/* for XmdefaultButtonType */
{
  int             	answer = RET_NONE;
  Widget          	dialog;	
  Arg             	args[5];
  int             	n = 0;
  XtAppContext    	context;

  n = 0;
  XtSetArg (args[n], XmNdialogStyle, XmDIALOG_FULL_APPLICATION_MODAL); n++;
  XtSetArg (args[n], XmNdefaultButtonType, defbut); n++;
  dialog = XmCreateErrorDialog (widget, name, args, n);
  
  XtAddCallback (dialog, XmNokCallback, response, &answer);
  XtAddCallback (dialog, XmNcancelCallback, response, &answer);
  XtAddCallback (dialog, XmNhelpCallback, response, &answer);
  
  if ( !(RET_OK & buttons) )
    XtUnmanageChild (XmMessageBoxGetChild (dialog, XmDIALOG_OK_BUTTON));
  if ( !(RET_CANCEL & buttons) )
    XtUnmanageChild (XmMessageBoxGetChild (dialog,XmDIALOG_CANCEL_BUTTON));
  if ( !(RET_HELP & buttons) )
    XtUnmanageChild (XmMessageBoxGetChild (dialog,XmDIALOG_HELP_BUTTON));
  
  XtManageChild (dialog);
  
  context = XtWidgetToApplicationContext (widget);
  while ( answer == RET_NONE || XtAppPending (context) ) {
    XtAppProcessEvent (context, XtIMAll);
  }

  XtUnmanageChild (dialog);
  return answer;
}


/* Posts a selection dialog */

/*ARGSUSED*/
int Selection(widget, name, buttons, defbut)
Widget          widget;         /* needed for context */
char           *name;           /* name of widget */
int             buttons;        /* RET_OK | RET_CANCEL | RET_HELP | RET_APPLY */
unsigned char   defbut;         /* for XmdefaultButtonType */
{
  int                   answer = RET_NONE;
  Widget                dialog;
  Arg                   args[10];
  int                   n = 0;
  XtAppContext          context;

  n = 0;
  XtSetArg (args[n], XmNdialogStyle, XmDIALOG_FULL_APPLICATION_MODAL); n++;
  XtSetArg (args[n], XmNdefaultButtonType, defbut); n++;
  dialog = XmCreateSelectionDialog(widget, name, args, n);

  XtAddCallback (dialog, XmNokCallback, response, &answer);
  XtAddCallback (dialog, XmNcancelCallback, response, &answer);
  XtAddCallback (dialog, XmNhelpCallback, response, &answer);

  if ( !(RET_OK & buttons) )
    XtUnmanageChild (XmSelectionBoxGetChild (dialog, XmDIALOG_OK_BUTTON));
  if ( !(RET_CANCEL & buttons) )
    XtUnmanageChild (XmSelectionBoxGetChild (dialog,XmDIALOG_CANCEL_BUTTON));
  if ( !(RET_APPLY & buttons) )
    XtUnmanageChild (XmSelectionBoxGetChild (dialog,XmDIALOG_APPLY_BUTTON));
  if ( !(RET_HELP & buttons) )
    XtUnmanageChild (XmSelectionBoxGetChild (dialog,XmDIALOG_HELP_BUTTON));

  XtManageChild (dialog);

  context = XtWidgetToApplicationContext (widget);
  while ( answer == RET_NONE || XtAppPending (context) ) {
    XtAppProcessEvent (context, XtIMAll);
  }

  XtUnmanageChild (dialog);
  return answer;
}


/* Posts a warning dialog */

int Warning (widget, name, buttons, defbut)
Widget          widget;		/* needed for context */
char           *name;           /* name of widget */
int 		buttons;	/* RET_OK | RET_CANCEL | RET_HELP */
unsigned char	defbut;		/* for XmdefaultButtonType */
{
  int             	answer = RET_NONE;
  Widget          	dialog;	
  Arg             	args[5];
  int             	n = 0;
  XtAppContext    	context;

  n = 0;
  XtSetArg (args[n], XmNdialogStyle, XmDIALOG_FULL_APPLICATION_MODAL); n++;
  XtSetArg (args[n], XmNdefaultButtonType, defbut); n++;
  dialog = XmCreateWarningDialog (widget, name, args, n);
  
  XtAddCallback (dialog, XmNokCallback, response, &answer);
  XtAddCallback (dialog, XmNcancelCallback, response, &answer);
  XtAddCallback (dialog, XmNhelpCallback, response, &answer);
  
  if ( !(RET_OK & buttons) )
    XtUnmanageChild (XmMessageBoxGetChild (dialog, XmDIALOG_OK_BUTTON));
  if ( !(RET_CANCEL & buttons) )
    XtUnmanageChild (XmMessageBoxGetChild (dialog,XmDIALOG_CANCEL_BUTTON));
  if ( !(RET_HELP & buttons) )
    XtUnmanageChild (XmMessageBoxGetChild (dialog,XmDIALOG_HELP_BUTTON));
  
  XtManageChild (dialog);
  
  context = XtWidgetToApplicationContext (widget);
  while ( answer == RET_NONE || XtAppPending (context) ) {
    XtAppProcessEvent (context, XtIMAll);
  }

  XtUnmanageChild (dialog);
  return answer;
}


/* Posts a warning dialog */

/* ARGSUSED */
int SaveWarning (widget, name, buttons, defbut)
Widget          widget;		/* needed for context */
char           *name;           /* name of widget */
int 		buttons;	/* RET_OK | RET_CANCEL | RET_HELP */
unsigned char	defbut;		/* for XmdefaultButtonType */
{
  int             	answer = RET_NONE;
  Widget          	dialog, discard_button ;
  Arg             	args[10];
  int             	n = 0;
  XtAppContext    	context;

  n = 0;
  XtSetArg (args[n], XmNallowShellResize, True); n++;
  XtSetArg (args[n], XmNdialogStyle, XmDIALOG_FULL_APPLICATION_MODAL); n++;
  XtSetArg (args[n], XmNdefaultButtonType, defbut); n++;
  dialog = XmCreateWarningDialog (widget, name, args, n);
  
  XtAddCallback (dialog, XmNokCallback, save_response, &answer);
  XtAddCallback (dialog, XmNcancelCallback, save_response, &answer);
  XtAddCallback (dialog, XmNhelpCallback, save_response, &answer);

  n = 0;
  discard_button = XmCreatePushButton (dialog, "discard_button", args, n);
  XtManageChild (discard_button);

  XtManageChild (dialog);
  
  context = XtWidgetToApplicationContext (widget);
  while ( answer == RET_NONE || XtAppPending (context) ) {
    XtAppProcessEvent (context, XtIMAll);
  }

  XtUnmanageChild (dialog);
  return answer;
}


/* Posts a working dialog and sets a timer */

int Working (widget, name, buttons, defbut)
Widget          widget;		/* needed for context */
char           *name;           /* name of widget */
int 		buttons;	/* RET_OK | RET_CANCEL | RET_HELP */
unsigned char	defbut;		/* for XmdefaultButtonType */
{
  int             	answer = RET_NONE;
  Widget          	dialog;	
  Arg             	args[5];
  int             	n = 0;
  XtAppContext    	context;
  Cursor		cursor;
  Pixmap		hour_pixmap, hourm_pixmap;
  Pixel			bg, fg;
  XColor		backg, foreg;

#define hour_x_hot 7
#define hour_y_hot 7
#define hour_width 16
#define hour_height 16
  static char hour_bits[] = {
    0x00, 0x00, 0xfe, 0x7f, 0x14, 0x28, 0x14, 0x28, 0x14, 0x28, 0x24, 0x24,
    0x44, 0x22, 0x84, 0x21, 0x84, 0x21, 0x44, 0x22, 0x24, 0x24, 0x14, 0x28,
    0x94, 0x29, 0xd4, 0x2b, 0xfe, 0x7f, 0x00, 0x00};
#define hourm_width 16
#define hourm_height 16
  static char hourm_bits[] = {
    0xff, 0xff, 0xff, 0xff, 0xfe, 0x7f, 0xfe, 0x7f, 0xfe, 0x7f, 0xfe, 0x7f,
    0xfe, 0x7f, 0xfe, 0x7f, 0xfe, 0x7f, 0xfe, 0x7f, 0xfe, 0x7f, 0xfe, 0x7f,
    0xfe, 0x7f, 0xfe, 0x7f, 0xff, 0xff, 0xff, 0xff};

  n = 0;
  XtSetArg (args[n], XmNforeground, &fg); n++;
  XtSetArg (args[n], XmNbackground, &bg); n++;
  XtGetValues (widget, args, n);

  hour_pixmap =  (Pixmap) XCreatePixmapFromBitmapData
    (XtDisplay(widget),
     DefaultRootWindow(XtDisplay(widget)),
     hour_bits,
     hour_width,
     hour_height,
     0,
     1,
     1);
  
  hourm_pixmap = (Pixmap)XCreatePixmapFromBitmapData
    (XtDisplay(widget),
     DefaultRootWindow(XtDisplay(widget)),
     hourm_bits,
     hourm_width,
     hourm_height,
     1,
     0,
     1);
  
  backg.pixel = WhitePixelOfScreen (XtScreen(widget));
  foreg.pixel = BlackPixelOfScreen (XtScreen(widget));
  
  XQueryColor (XtDisplay(widget), DefaultColormapOfScreen (XtScreen(widget)),
	       &backg);
  XQueryColor (XtDisplay(widget), DefaultColormapOfScreen (XtScreen(widget)),
	       &foreg);

  cursor = XCreatePixmapCursor (XtDisplay(widget), hour_pixmap, hourm_pixmap,
				&backg, &foreg,	hour_x_hot, hour_y_hot);

  n = 0;
  XtSetArg (args[n], XmNdialogStyle, XmDIALOG_FULL_APPLICATION_MODAL); n++;
  XtSetArg (args[n], XmNdefaultButtonType, defbut); n++;
  dialog = XmCreateWorkingDialog (widget, name, args, n);
  
  XtAddCallback (dialog, XmNokCallback, response, &answer);
  XtAddCallback (dialog, XmNcancelCallback, response, &answer);
  XtAddCallback (dialog, XmNhelpCallback, response, &answer);

  if ( !(RET_OK & buttons) )
    XtUnmanageChild (XmMessageBoxGetChild (dialog, XmDIALOG_OK_BUTTON));
  if ( !(RET_CANCEL & buttons) )
    XtUnmanageChild (XmMessageBoxGetChild (dialog,XmDIALOG_CANCEL_BUTTON));
  if ( !(RET_HELP & buttons) )
    XtUnmanageChild (XmMessageBoxGetChild (dialog,XmDIALOG_HELP_BUTTON));

  context = XtWidgetToApplicationContext (widget);

  XtAppAddTimeOut (context, WORKING_TIME, working_timer_activate, &answer);
  
  XtManageChild (dialog);
  XDefineCursor (XtDisplay (dialog), XtWindow (dialog), cursor);
  
  while ( answer == RET_NONE || answer == RET_OK || XtAppPending (context) ) {
    XtAppProcessEvent (context, XtIMAll);
  }

  XtUnmanageChild (dialog);
  XFreeCursor (XtDisplay (widget), cursor);
  return answer;
}



/* Posts a TextField in a message box with buttons labeled OK, Cancel and Help*/

int PromptDialog (widget, name, buttons, defbut)
Widget          widget;		/* needed for context */
char           *name;           /* name of widget */
int 		buttons;	/* RET_OK | RET_CANCEL | RET_HELP */
unsigned char	defbut;		/* for XmdefaultButtonType */
{
  int             	answer = RET_NONE;
  Widget          	dialog;	
  Arg             	args[10];
  int             	n = 0;
  XtAppContext    	context;

  n = 0;
  XtSetArg (args[n], XmNdialogStyle, XmDIALOG_FULL_APPLICATION_MODAL); n++;
  XtSetArg (args[n], XmNdefaultButtonType, defbut); n++;
  dialog = XmCreatePromptDialog (widget, name, args, n);
  
  XtAddCallback (dialog, XmNokCallback, response, &answer);
  XtAddCallback (dialog, XmNcancelCallback, response, &answer);
  XtAddCallback (dialog, XmNhelpCallback, response, &answer);
  
  if ( !(RET_OK & buttons) )
    XtUnmanageChild (XmMessageBoxGetChild (dialog, XmDIALOG_OK_BUTTON));
  if ( !(RET_CANCEL & buttons) )
    XtUnmanageChild (XmMessageBoxGetChild (dialog,XmDIALOG_CANCEL_BUTTON));
  if ( !(RET_HELP & buttons) )
    XtUnmanageChild (XmMessageBoxGetChild (dialog,XmDIALOG_HELP_BUTTON));
  
  XtManageChild (dialog);
  
  context = XtWidgetToApplicationContext (widget);
  while ( answer == RET_NONE || XtAppPending (context) ) {
    XtAppProcessEvent (context, XtIMAll);
  }

  XtUnmanageChild (dialog);

  return answer;
}





/* Posts a File Selection Dialog */

int FileSelectionDialog (widget, name, buttons, defbut)
Widget          widget;		/* needed for context */
char           *name;           /* name of widget */
int 		buttons;	/* RET_OK | RET_CANCEL | RET_HELP */
unsigned char	defbut;		/* for XmdefaultButtonType */
{
  int             	answer = RET_NONE;
  Widget          	dialog;	
  Arg             	args[10];
  int             	n = 0;
  XtAppContext    	context;

  n = 0;
  XtSetArg (args[n], XmNdialogStyle, XmDIALOG_MODELESS); n++;
  XtSetArg (args[n], XmNdefaultButtonType, defbut); n++;
  dialog = XmCreateFileSelectionDialog (widget, name, args, n);
  
  XtAddCallback (dialog, XmNokCallback, response, &answer);
  XtAddCallback (dialog, XmNcancelCallback, response, &answer);
  XtAddCallback (dialog, XmNhelpCallback, response, &answer);
  
  if ( !(RET_OK & buttons) )
    XtUnmanageChild (XmFileSelectionBoxGetChild (dialog, XmDIALOG_OK_BUTTON));
  if ( !(RET_CANCEL & buttons) )
    XtUnmanageChild (XmFileSelectionBoxGetChild(dialog,XmDIALOG_CANCEL_BUTTON));
  if ( !(RET_APPLY & buttons) )
    XtUnmanageChild (XmFileSelectionBoxGetChild (dialog,XmDIALOG_APPLY_BUTTON));
  if ( !(RET_HELP & buttons) )
    XtUnmanageChild (XmFileSelectionBoxGetChild (dialog,XmDIALOG_HELP_BUTTON));
  
  XtManageChild (dialog);
  
  context = XtWidgetToApplicationContext (widget);
  while ( answer == RET_NONE || XtAppPending (context) ) {
    XtAppProcessEvent (context, XtIMAll);
  }

  XtUnmanageChild (dialog);

  return answer;
}



