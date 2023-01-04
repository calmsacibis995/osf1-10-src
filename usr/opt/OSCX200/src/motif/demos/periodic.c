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
/************************************************************************
# Copyright 1991 OPEN SOFTWARE FOUNDATION, INC.
#
# Permission is hereby granted to use, copy, modify and freely distribute
# the software in this file and its documentation for any purpose without
# fee, provided that the above copyright notice appears in all copies and
# that both the copyright notice and this permission notice appear in
# supporting documentation.  Further, provided that the name of Open
# Software Foundation, Inc. ("OSF") not be used in advertising or
# publicity pertaining to distribution of the software without prior
# written permission from OSF.  OSF makes no representations about the
# suitability of this software for any purpose.  It is provided "as is"
# without express or implied warranty.
#
# Open Software Foundation is a trademark of The Open Software Foundation, Inc.
# OSF is a trademark of Open Software Foundation, Inc.
# OSF/Motif is a trademark of Open Software Foundation, Inc.
# Motif is a trademark of Open Software Foundation, Inc.
************************************************************************/

/*****************************************************************
!*                                                              **
!*   Author: Michael K. Yee                                     **
!*   Date: August 18, 1990                                      **
!*                                                              **
!*****************************************************************/


#include <stdio.h>

#include <Xm/Xm.h>                        /* Motif Toolkit */
#include <Xm/Label.h>
#include <Xm/LabelG.h>
#include <Xm/PushB.h>
#include <Xm/PushBG.h>
#include <Xm/ToggleB.h>
#include <Xm/ToggleBG.h>
#include <Xm/RowColumn.h>
#include <Xm/SeparatoG.h>
#include <Xm/Separator.h>
#include <Mrm/MrmPublic.h>                   /* Mrm Toolkit */

#define MAX_ARGS  100

#define K_scale_widget           1
#define K_drawnbutton_widget     2
#define K_togglebutton_widget    3
#define K_vtext1_widget          4
#define K_vtext2_widget          5
#define K_popup_menu_widget      6

static Widget Scale_1,Drawnbutton_1,Togglebutton_1,VText_1,VText_2,PopMenu_1,PMenu_1;
static Widget Label1, Separator1, PushBtn1, ToggleBtn1;

static MrmHierarchy	s_MrmHierarchy;		/* MRM database hierarch id */
static char		*vec[]={"periodic.uid"};
						/* MRM database file list   */
static MrmCode		class ;

static void quit_button_activate();
static void foo_activate();
static void create_callback();
static void zippy_null();
static void postmenu ();

static MrmCount		regnum = 4 ;
static MrmRegisterArg	regvec[] = {
	{"quit_button_activate",(caddr_t)quit_button_activate},
	{"foo_activate",(caddr_t)foo_activate},
	{"create_callback",(caddr_t)create_callback},
	{"zippy_null",(caddr_t)zippy_null}
	};

static Widget  CreateLabel(label, parent)
    char   *label;
    Widget  parent;
{
    Widget        widget;
    register int  n;
    Arg           args[MAX_ARGS];
    XmString      tcs;


    n = 0;
    tcs = XmStringLtoRCreate(label, XmSTRING_DEFAULT_CHARSET);
    XtSetArg(args[n], XmNlabelString, tcs);  n++;
    widget = XmCreateLabel(parent, "label", args, n);
    XtManageChild(widget);
    XmStringFree(tcs);

    return(widget);
}


static Widget  CreatePushButton(label, parent)
    char   *label;
    Widget  parent;
{
    Widget        widget;
    register int  n;
    Arg           args[MAX_ARGS];
    XmString      tcs;


    n = 0;
    tcs = XmStringLtoRCreate(label, XmSTRING_DEFAULT_CHARSET);
    XtSetArg(args[n], XmNlabelString, tcs);  n++;
    widget = XmCreatePushButton(parent, "pushButton", args, n);
    XtManageChild(widget);
    XmStringFree(tcs);

    return(widget);
}

static Widget  CreateToggle(label, parent)
    char   *label;
    Widget  parent;
{
    Widget        widget;
    register int  n;
    Arg           args[MAX_ARGS];
    XmString      tcs;


    n = 0;
    tcs = XmStringLtoRCreate(label, XmSTRING_DEFAULT_CHARSET);
    XtSetArg(args[n], XmNlabelString, tcs);  n++;
    widget = XmCreateToggleButton(parent, "toggle", args, n);
    XtManageChild(widget);
    XmStringFree(tcs);

    return(widget);
}

XtAppContext	app_context;
Display*	display;

/*
 *  Main program
 */
int main(argc, argv)
unsigned int argc;
char **argv;
{
     /*
     *  Declare the variables to contain the two widget ids
     */
    Widget toplevel, periodic_table_main = NULL;
    Arg arglist[1] ;
    int         status;

    /*
     *  Initialize the MRM
     */

    MrmInitialize ();

    /*
     *  Initialize the toolkit.  This call returns the id of the "toplevel"
     *  widget.  The applications "main" widget must be the only child
     *  of this widget.
     */

    XtToolkitInitialize();
    app_context = XtCreateApplicationContext();
    display = XtOpenDisplay(app_context, NULL,
	"periodic",    		/* application name */
	"XMdemos",              /* application class */
	NULL, 0,                /* options */
	&argc, argv);           /* command line parameters */
    if (!display) { printf("Unable to open display\n"); exit(0); }


    XtSetArg (arglist[0], XtNallowShellResize, FALSE) ;
    toplevel = XtAppCreateShell("periodic", NULL,
		    applicationShellWidgetClass,
		    display, arglist, 1);

    /*
     *  Define the Mrm.hierarchy (only 1 file)
     */

    if (MrmOpenHierarchy (1,			    /* number of files	    */
			vec, 			    /* files     	    */
			NULL,			    /* os_ext_list (null)   */
			&s_MrmHierarchy)	    /* ptr to returned id   */
			!= MrmSUCCESS) {
	printf ("can't open hierarchy\n");
     }

    /*
     * 	Register our callback routines so that the resource manager can 
     * 	resolve them at widget-creation time.
     */

    if (MrmRegisterNames (regvec, regnum)
			!= MrmSUCCESS)
			    printf("can't register names\n");

    /*
     *  Call MRM to fetch and create the pushbutton and its container
     */

    if (MrmFetchWidget (s_MrmHierarchy,
			"periodic_table_main",
			toplevel,
			&periodic_table_main,
			&class)
			!= MrmSUCCESS)
			    printf("can't fetch interface\n");

    /*
     *  Make the toplevel widget "manage" the main window (or whatever the
     *  the uil defines as the topmost widget).  This will
     *  cause it to be "realized" when the toplevel widget is "realized"
     */

    XtManageChild(periodic_table_main);
    
    /*
     *  Realize the toplevel widget.  This will cause the entire "managed"
     *  widget hierarchy to be displayed
     */

    XtRealizeWidget(toplevel);

    /*
    **  PopupMenu1
    */
    PMenu_1 = XmCreatePopupMenu(PopMenu_1, "popMenu", NULL, 0);
    XtAddEventHandler(PopMenu_1, ButtonPressMask, False, postmenu, PMenu_1);

    /*
    **  Menu1
    */
    Label1       = CreateLabel("POPUP MENU", PMenu_1);

    XtSetArg(arglist[0], XmNseparatorType, XmDOUBLE_LINE);
    Separator1   = XmCreateSeparator(PMenu_1, "separator", arglist, 1);

    PushBtn1     = CreatePushButton("pushbutton", PMenu_1);

    ToggleBtn1   = CreateToggle("togglebutton", PMenu_1);


    /*
     *  Loop and process events
     */

    XtAppMainLoop(app_context);

    /* UNREACHABLE */
    return (0);
}

static void zippy_null( widget, tag, callback_data )
	Widget	widget;
	char    *tag;
	XmAnyCallbackStruct *callback_data;
{
}

#define fatal 1
static void  postmenu (w, popup, event)
    Widget         w;
    Widget         popup;
    XButtonEvent  *event;
{
    int         status;
    if (event->button != Button3)
        return;

    XmMenuPosition(PMenu_1, event);
    XtManageChild (PMenu_1);
}

/*
 * All widgets that are created call back to this procedure.
 */

static void create_callback(w, tag, reason)
    Widget w;
    int *tag;
    unsigned long *reason;
{

    /*  For internationalization ease, we capture a few strings from the
     *  widgets themselves.  We could go out and fetch them as needed but
     *  since we use these all the time, this method if more efficient.
     */
    switch (*tag) {
        case K_scale_widget: 
	    Scale_1 = w;
            break;
        case K_drawnbutton_widget: 
	    Drawnbutton_1 = w;
            break;
        case K_togglebutton_widget: 
	    Togglebutton_1 = w;
            break;
        case K_vtext1_widget: 
	    VText_1 = w;
            break;
        case K_vtext2_widget: 
	    VText_2 = w;
            break;
        case K_popup_menu_widget: 
	    PopMenu_1 = w;
            break;
        default: 
            break;
    }
}

static void foo_activate( widget, tag, callback_data )
	Widget	widget;
	char    *tag;
	XmAnyCallbackStruct *callback_data;
{
    Arg arglist[2];
    int scale_value;
    Boolean toggle_state;

    XtSetArg( arglist[0], XmNvalue, &scale_value);
    XtGetValues( Scale_1, arglist, 1 );
    XtSetArg( arglist[0], XmNset, &toggle_state);
    XtGetValues( Togglebutton_1, arglist, 1 );

    if ((scale_value == 13) && (toggle_state == True)) {
	XtSetArg( arglist[0], XmNvalue,
"OSF Motif Credits");
	XtSetValues( VText_1, arglist, 1 );
	XtSetArg( arglist[0], XmNvalue,
"DEV: Vania, Ellis, Mike,\n\
..Alka, Al, Scott,\n\
..Daniel, and Paul\n\
REL: Ron and Martha\n\
Q/A: Libby, Tom, Ken,\n\
..Carl, and Dany\n\
DOC: Ken and Bob\n\
Moral Support:\n\
Mary Chung's Restaurant\n");
	XtSetValues( VText_2, arglist, 1 );
    }

}

static void quit_button_activate( widget, tag, callback_data )
	Widget	widget;
	char    *tag;
	XmAnyCallbackStruct *callback_data;
{
    exit(1);
}
