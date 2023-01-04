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
/**---------------------------------------------------------------------
***	
***	file:		xmfonts.c
***
***     Author:   Daniel Dardailler
***
***-------------------------------------------------------------------*/

#include <stdio.h>
#include <Xm/Xm.h>
#include <Xm/MainW.h>
#include <Xm/PushB.h>
#include <Xm/RowColumn.h>
#include <Xm/CascadeB.h>
#include <Xm/MessageB.h>
#include <Xm/PushBG.h>


/***** some forwards ******/
Widget CreateApplication ();	/*  create main window		*/
Widget CreateFontSample ();	/*  create font display window	*/
Widget CreateHelp ();		/*  create help window		*/

void SelectFontCB ();		/*  callback for font buttons	*/
void HelpCB ();			/*  callback for help button	*/
void QuitCB ();			/*  callback for quit button	*/


/***** application wide resource ******/
typedef struct {
  char *fontpattern;
  int   maxfont ;    /* number max of font to be displayed */
  int   maxlen ;     /* max length of font name to be displayed */
  Boolean   usegadget ;  
} ApplicationData, *ApplicationDataPtr;

ApplicationData AppData;

#define XtNfontPattern "fontPattern"
#define XtCFontPattern "FontPattern"
#define XtNmaxFont     "maxFont"
#define XtCMaxFont     "MaxFont"
#define XtNmaxLen      "maxLen"
#define XtCMaxLen      "MaxLen"
#define XtNuseGadget   "useGadget"
#define XtCUseGadget   "UseGadget"

static XtResource resources[] = {
  { XtNfontPattern, XtCFontPattern, XmRString, sizeof(String),
    XtOffset(ApplicationDataPtr, fontpattern), XmRString, "*" },
  { XtNmaxFont, XtCMaxFont, XmRInt, sizeof(int),
    XtOffset(ApplicationDataPtr, maxfont), XmRImmediate, (caddr_t) 1000 },
  { XtNmaxLen, XtCMaxLen, XmRInt, sizeof(int),
    XtOffset(ApplicationDataPtr, maxlen), XmRImmediate, (caddr_t) 10 },
  { XtNuseGadget, XtCUseGadget, XmRBoolean, sizeof (Boolean),
    XtOffset(ApplicationDataPtr, usegadget), XmRImmediate, (caddr_t) TRUE }
};


/*-------------------------------------------------------------
**	Main logic
*/
void main (argc,argv) 
	unsigned int	argc;
	char 		**argv;
{
	Display		*display;	/*  Display		*/
	Widget		app_shell;	/*  ApplicationShell	*/
	Widget		main_window;	/*  MainWindow		*/
	XtAppContext	app_context;

	/*	Initialize toolkit and open the display. 
	 */
	app_shell = XtAppInitialize(&app_context, "XMdemos", NULL, 0,
				    &(int)argc, argv, NULL, NULL, 0);

	/*	Parse the application resource data
	 */
	XtGetApplicationResources(app_shell,
			      (XtPointer)&AppData,
			      resources,
			      XtNumber(resources),
			      NULL,
			      0);

	main_window = CreateApplication (app_shell);
	XtRealizeWidget (app_shell);

	/*	Get and dispatch events.
	*/
	XtAppMainLoop (app_context);
}



/*-------------------------------------------------------------
**	CreateApplication	- create main window
*/
Widget CreateApplication (parent) 
Widget		parent;		/*  parent widget	*/
{
	Widget		main_window;	/*  MainWindow		*/
	Widget		menu_bar;	/*  MenuBar		*/
	Widget		menu_pane;	/*  MenuPane		*/
	Widget		cascade;	/*  CascadeButton	*/
	Widget		row_column;	/*  RowColumn		*/
	Widget		button;		/*  PushButton      	*/

	Arg		args[5];	/*  arg list		*/
	int	n;		        /*  arg count		*/

	               /* for the font buttons list management  */
	char *	name, ** fontnamelist ;
	int count, i  ;
	XmString	label_string;


	/*	Create MainWindow.
	*/
	n = 0;
	XtSetArg (args[n], XmNscrollingPolicy, XmAUTOMATIC);  n++;
	main_window = XmCreateMainWindow (parent, "main_window", args, n);
	XtManageChild (main_window);

	/*	Create MenuBar in MainWindow.
	*/
	n = 0;
	menu_bar = XmCreateMenuBar (main_window, "menu_bar", args, n); 
	XtManageChild (menu_bar);

	/*	Create "Exit" PulldownMenu.
	*/
	n = 0;
	menu_pane = XmCreatePulldownMenu (menu_bar, "menu_pane", args, n);

	n = 0;
	button = XmCreatePushButton (menu_pane, "Quit", args, n);
	XtManageChild (button);
	XtAddCallback (button, XmNactivateCallback, QuitCB, NULL);

	n = 0;
	XtSetArg (args[n], XmNsubMenuId, menu_pane);  n++;
	cascade = XmCreateCascadeButton (menu_bar, "Exit", args, n);
	XtManageChild (cascade);


	/*	Create "Help" button.
	*/
	n = 0;
	cascade = XmCreateCascadeButton (menu_bar, "Help", args, n);
	XtManageChild (cascade);
	XtAddCallback (cascade, XmNactivateCallback, HelpCB, NULL);

	n = 0;
	XtSetArg (args[n], XmNmenuHelpWidget, cascade);  n++;
	XtSetValues (menu_bar, args, n);


	/*	Create RowColumn in MainWindow to manage buttons.
	*/
	n = 0;
	row_column = XmCreateRowColumn (main_window, "row_column", args, n);
	XtManageChild (row_column);


	/*	Set MainWindow areas 
	*/
	XmMainWindowSetAreas (main_window, menu_bar, NULL, NULL, NULL,
			      row_column);

	/*	Create a PushButton widget|gadget for each font.
		Get the font list from the server and keep
		only the font name whose length is less than 
		the given maxLen resource.
	*/
	
	fontnamelist = XListFonts(XtDisplay(main_window), 
				  AppData.fontpattern, 
				  AppData.maxfont, &count);
	if (!fontnamelist) {
	    fprintf(stderr,"Zero font on your server. GoodBye.\n");
	    exit(0);
	}

	/*  read one entry each time through the loop  */
	for (i = 0;  i < count;  i++ ) 
	{
		name = fontnamelist[i] ;

		if (strlen(name) > AppData.maxlen) continue ;

		/*  create PushButton in RowCol  */
		n = 0;
		label_string = XmStringCreateLtoR(name, 
						  XmSTRING_DEFAULT_CHARSET);
		XtSetArg (args[n], XmNlabelString, label_string);  n++;

		if (AppData.usegadget) 
		    button = XmCreatePushButtonGadget (row_column, name, 
						       args, n);
		else 
		    button = XmCreatePushButton(row_column, name, 
						args, n);
		XtManageChild (button);
		XmStringFree (label_string);
	}

	XtAddCallback (row_column, XmNentryCallback, SelectFontCB, NULL);
		
	XFreeFontNames(fontnamelist);

	return (main_window);
}



/*-------------------------------------------------------------
**	QuitCB			- callback for quit button
*/
void QuitCB (w, client_data, call_data) 
Widget		w;		/*  widget id		*/
caddr_t		client_data;	/*  data from applicaiton   */
caddr_t		call_data;	/*  data from widget class  */
{
    exit (0);
}



/*-------------------------------------------------------------
**	HelpCB			- callback for help button
*/
void HelpCB (w, client_data, call_data) 
Widget		w;		/*  widget id		*/
caddr_t		client_data;	/*  data from application   */
caddr_t		call_data;	/*  data from widget class  */
{
	static Widget message_box = NULL ;

	if (!message_box) message_box = CreateHelp (w);

	XtManageChild (message_box);
}



/*-------------------------------------------------------------
**	SelectFontCB		- callback for font buttons
*/
void SelectFontCB (w, client_data, call_data) 
Widget		w;		/*  widget id		*/
caddr_t		client_data;	/*  data from application   */
caddr_t		call_data;	/*  data from widget class  */
{
	static Widget font_box = NULL;
	Widget button ;

	XmRowColumnCallbackStruct * rcCB = 
	    (XmRowColumnCallbackStruct *) call_data ;	
	    
	button = rcCB->widget  ;

	if (!font_box) font_box = CreateFontSample (button);

	XtManageChild (font_box);
}



/*-------------------------------------------------------------
**	CreateHelp		- create help window
*/
Widget CreateHelp (parent) 
	Widget		parent;		/*  parent widget	*/
{
	Widget		button;
	Widget		message_box;	/*  Message Dialog 	*/
	Arg		args[20];	/*  arg list		*/
	register int	n;		/*  arg count		*/

	static char	message[BUFSIZ];	/*  help text	*/
	XmString	title_string = NULL;
	XmString	message_string = NULL;
	XmString	button_string = NULL;



	/*	Generate message to display.
	*/
	sprintf (message, "\
These are buttons for the fonts in your X11 server whose  \n\
name lengths are below the given maxLen application resource.\n\
The button label is the name of the font.  When you select \n\
a button, a small window will display a sample of the font.  \n\n\
Press the 'close' button to close a font window.  \n\
Select 'quit' from the 'exit' menu to exit this application.\0");

	message_string = XmStringCreateLtoR (message, 
					     XmSTRING_DEFAULT_CHARSET);
	button_string = XmStringCreateLtoR ("Close", 
					    XmSTRING_DEFAULT_CHARSET);
	title_string = XmStringCreateLtoR ("xmfonts help", 
					   XmSTRING_DEFAULT_CHARSET);


	/*	Create MessageBox dialog.
	*/
	n = 0;
	XtSetArg (args[n], XmNdialogTitle, title_string);  n++;
	XtSetArg (args[n], XmNokLabelString, button_string);  n++;
	XtSetArg (args[n], XmNmessageString, message_string);  n++;
	message_box = XmCreateMessageDialog (parent, "helpbox", args, n);

	button = XmMessageBoxGetChild (message_box, XmDIALOG_CANCEL_BUTTON);
	XtUnmanageChild (button);
	button = XmMessageBoxGetChild (message_box, XmDIALOG_HELP_BUTTON);
	XtUnmanageChild (button);


	/*	Free strings and return MessageBox.
	*/
	if (title_string) XmStringFree (title_string);
	if (message_string) XmStringFree (message_string);
	if (button_string) XmStringFree (button_string);

	return (message_box);
}



/*-------------------------------------------------------------
**	CreateFontSample	- create font display window
*/
Widget CreateFontSample (parent) 
Widget		parent;		/*  parent widget	*/
{
	Widget		message_box;		/*  MessageBox Dialog	*/
	Widget		button;
	Arg		args[20];		/*  arg list		*/
	register int	n;			/*  arg count		*/
	
	char		*name = NULL;		/*  font name		*/
	XFontStruct	*font = NULL;		/*  font pointer	*/
	XmFontList	fontlist = NULL;	/*  fontlist pointer	*/
	static char	message[BUFSIZ];	/*  text sample		*/
	XmString	name_string = NULL;
	XmString	message_string = NULL;
	XmString	button_string = NULL;

	
	/*	Get font name.
	*/
	n = 0;
	XtSetArg (args[n], XmNlabelString, &name_string); n++;
	XtGetValues (parent, args, n);
	XmStringGetLtoR (name_string, XmSTRING_DEFAULT_CHARSET, &name);


	/*	Load font and generate message to display. */
	if (name)
	    font = XLoadQueryFont (XtDisplay (XtParent (parent)), name);
	if  (!font)
	    sprintf (message, "Unable to load font: %s\0", name);
	else {
	    fontlist = XmFontListCreate (font, XmSTRING_DEFAULT_CHARSET);
	    sprintf (message, "\
This is font %s.\n\
The quick brown fox jumps over the lazy dog.\0", name);
	}

	message_string = XmStringCreateLtoR (message, 
					     XmSTRING_DEFAULT_CHARSET);
	button_string = XmStringCreateLtoR ("Close", 
					    XmSTRING_DEFAULT_CHARSET);


	/*	Create MessageBox dialog.
	*/
	n = 0;
	XtSetArg (args[n], XmNdialogTitle, name_string);  n++;
	XtSetArg (args[n], XmNokLabelString, button_string);  n++;
	XtSetArg (args[n], XmNmessageString, message_string);  n++;
	message_box = XmCreateMessageDialog (XtParent (XtParent(parent)), 
					     "fontbox",
					     args, n);

	button = XmMessageBoxGetChild (message_box, XmDIALOG_MESSAGE_LABEL);
	if (fontlist) 
	{
		n = 0;
		XtSetArg (args[n], XmNfontList, fontlist);  n++;
		XtSetValues(button, args, n);
	}
	
	button = XmMessageBoxGetChild (message_box, XmDIALOG_CANCEL_BUTTON);
	XtUnmanageChild (button);
	button = XmMessageBoxGetChild (message_box, XmDIALOG_HELP_BUTTON);
	XtUnmanageChild (button);

	/*	Free strings and return MessageBox.
	*/
	if (fontlist) XmFontListFree (fontlist);
	if (name_string) XmStringFree (name_string);
	if (message_string) XmStringFree (message_string);
	if (button_string) XmStringFree (button_string);

	return (message_box);
}

/******** Application default resource:
xmfonts.fontPattern:             *
xmfonts.maxFont:                 1000
xmfonts.maxLen:                  10
xmfonts*main_window.height:	 400
xmfonts*main_window.width:	 300
xmfonts*menu_bar.background:	 LightGrey
xmfonts*row_column.numColumns:	 5
xmfonts*row_column.packing:      XmPACK_COLUMN
*useGadget:			 True
**********************************************/
