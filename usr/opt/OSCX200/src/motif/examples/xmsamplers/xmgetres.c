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
static char *rcsid = "@(#)$RCSfile: xmgetres.c,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/11/05 22:02:03 $";
#endif
/*
 * (c) Copyright 1989, 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 */
/*
 * Motif Release 1.2
 */

/* This XmAll.h header only officially exists in 1.2, one would
   have to create it for a 1.1 compilation */
#include <Xm/XmAll.h>


#ifdef _NO_PROTO
# include <varargs.h>
# define Va_start(a,b) va_start(a)
#else
# include <stdarg.h>
# define Va_start(a,b) va_start(a,b)
#endif

#if (XmREVISION >=2) && (!defined(NO_HIDDEN_BABIES))
#include <Xm/ExtObjectP.h>
#include <Xm/DesktopP.h>
#include <Xm/WorldP.h>
#include <Xm/SashP.h>
#include <Xm/TearOffBP.h>
/* 
#include <Xm/ProtocolsP.h>
#include <Xm/VendorSEP.h>
#include <Xm/ShellEP.h>
#include <Xm/DialogSEP.h>
*/
#endif

static void QuitCB ();			
static void HelpCB ();			

static Widget CreateHelp ();		

typedef struct _ClassArrayRec {
    String name ;
    WidgetClass class ;
} ClassArrayRec ;


/* Cannot initialize everything here, got a: 
    gcc error : initializer for static variable is not constant */
static ClassArrayRec class_array[] = {
  { "Object",  (WidgetClass) NULL },
  { "RectObj",  (WidgetClass) NULL },
  { "Widget",  (WidgetClass) NULL },
  { "Core",  (WidgetClass) NULL },
  { "Composite",  (WidgetClass) NULL },
  { "Constraint",  (WidgetClass) NULL },  

  { "Primitive",  (WidgetClass) NULL },
  { "ScrollBar",  (WidgetClass) NULL },
  { "ArrowB",  (WidgetClass) NULL },
  { "List",  (WidgetClass) NULL },
  { "Separator",  (WidgetClass) NULL },
  { "TextF",  (WidgetClass) NULL },
  { "Label",  (WidgetClass) NULL },
  { "DrawnB",  (WidgetClass) NULL },
  { "PushB",  (WidgetClass) NULL },
  { "ToggleB",  (WidgetClass) NULL },
  { "CascadeB",  (WidgetClass) NULL },  

  { "Text",  (WidgetClass) NULL },       

  { "Gadget",  (WidgetClass) NULL },
  { "ArrowBG",  (WidgetClass) NULL },
  { "SeparatorG",  (WidgetClass) NULL },
  { "LabelG",  (WidgetClass) NULL },
  { "PushBG",  (WidgetClass) NULL },
  { "ToggleBG", (WidgetClass) NULL },    
  { "CascadeBG",  (WidgetClass) NULL }, 

  { "Manager", (WidgetClass) NULL },
  { "BulletinB", (WidgetClass) NULL },
  { "DrawingA", (WidgetClass) NULL },
  { "Frame", (WidgetClass) NULL  },
  { "MainW", (WidgetClass) NULL },
  { "Form", (WidgetClass) NULL },
  { "RowCol", (WidgetClass) NULL },
  { "ScrolledW", (WidgetClass) NULL },
  { "PanedW", (WidgetClass) NULL },
  { "SelectionB", (WidgetClass) NULL },
  { "FileSB", (WidgetClass) NULL },
  { "MessageB", (WidgetClass) NULL },
  { "Scale",  (WidgetClass) NULL },
  { "Command",  (WidgetClass) NULL },    

  { "Shell",  (WidgetClass) NULL },
  { "OverrideShell",  (WidgetClass) NULL },
  { "WMShell",  (WidgetClass) NULL },   

  { "MenuShell",  (WidgetClass) NULL },  

  { "VendorS",  (WidgetClass) NULL },
  { "ToplevelShell",  (WidgetClass) NULL },
  { "ApplicationShell",  (WidgetClass) NULL },
  { "TransientShell",  (WidgetClass) NULL },  

  { "DialogShell",  (WidgetClass) NULL },    


/** No support in this program for hidden classes in 1.1 **/

#if (XmREVISION >=2) && (!defined(NO_HIDDEN_BABIES))
  { "ExtObj",  (WidgetClass) NULL },
  { "Desktop",  (WidgetClass) NULL },
  { "World",  (WidgetClass) NULL },
  { "Sash",  (WidgetClass) NULL },
  { "TearOffB",  (WidgetClass) NULL },

/*  those are not real Xt object anymore, remove them */
/*  { "Protocol",  (WidgetClass) NULL },    */
/*  { "ShellE",  (WidgetClass) NULL },      */
/*  { "VendorSE",  (WidgetClass) NULL },    */
/*  { "DialogSE",  (WidgetClass) NULL },    */
#endif

#if XmREVISION >=2
  { "Display",  (WidgetClass) NULL },
  { "Screen",  (WidgetClass) NULL },
  { "DragContext",  (WidgetClass) NULL },
  { "DragIcon",  (WidgetClass) NULL },
  { "DropSiteMgr",  (WidgetClass) NULL },
  { "DropTransfer",  (WidgetClass) NULL }, 

  { "DragOverS",  (WidgetClass) NULL },    
#endif
  
};

/* stupid c compiler */
static void InitClassArray () {
  int n;

  n = 0;
  class_array[n].class = objectClass; n++;
  class_array[n].class = rectObjClass; n++;
  class_array[n].class = widgetClass; n++;  /* unNamed */
  class_array[n].class = coreWidgetClass; n++;
  class_array[n].class = compositeWidgetClass; n++;
  class_array[n].class = constraintWidgetClass; n++;

  class_array[n].class = xmPrimitiveWidgetClass; n++;
  class_array[n].class = xmScrollBarWidgetClass; n++;
  class_array[n].class = xmArrowButtonWidgetClass; n++;
  class_array[n].class = xmListWidgetClass; n++;
  class_array[n].class = xmSeparatorWidgetClass; n++;
  class_array[n].class = xmTextFieldWidgetClass; n++;
  class_array[n].class = xmLabelWidgetClass; n++;
  class_array[n].class = xmDrawnButtonWidgetClass; n++;
  class_array[n].class = xmPushButtonWidgetClass; n++;
  class_array[n].class = xmToggleButtonWidgetClass; n++;
  class_array[n].class = xmCascadeButtonWidgetClass; n++;
  class_array[n].class = xmTextWidgetClass; n++;

  class_array[n].class = xmGadgetClass; n++;
  class_array[n].class = xmArrowButtonGadgetClass; n++;
  class_array[n].class = xmSeparatorGadgetClass; n++;
  class_array[n].class = xmLabelGadgetClass; n++;
  class_array[n].class = xmPushButtonGadgetClass; n++;
  class_array[n].class = xmToggleButtonGadgetClass; n++;
  class_array[n].class = xmCascadeButtonGadgetClass; n++;

  class_array[n].class = xmManagerWidgetClass; n++;
  class_array[n].class = xmBulletinBoardWidgetClass; n++;
  class_array[n].class = xmDrawingAreaWidgetClass; n++;
  class_array[n].class = xmFrameWidgetClass; n++;
  class_array[n].class = xmMainWindowWidgetClass; n++;
  class_array[n].class = xmFormWidgetClass; n++;
  class_array[n].class = xmRowColumnWidgetClass; n++;
  class_array[n].class = xmScrolledWindowWidgetClass; n++;
  class_array[n].class = xmPanedWindowWidgetClass; n++;
  class_array[n].class = xmSelectionBoxWidgetClass; n++;
  class_array[n].class = xmFileSelectionBoxWidgetClass; n++;
  class_array[n].class = xmMessageBoxWidgetClass; n++;
  class_array[n].class = xmScaleWidgetClass; n++;
  class_array[n].class = xmCommandWidgetClass; n++;

  class_array[n].class = shellWidgetClass; n++;
  class_array[n].class = overrideShellWidgetClass; n++;
  class_array[n].class = wmShellWidgetClass; n++;

  class_array[n].class = xmMenuShellWidgetClass; n++;

  class_array[n].class = vendorShellWidgetClass; n++;
  class_array[n].class = topLevelShellWidgetClass; n++;
  class_array[n].class = applicationShellWidgetClass; n++;
  class_array[n].class = transientShellWidgetClass; n++;
  class_array[n].class = xmDialogShellWidgetClass; n++;

#if (XmREVISION >=2) && (!defined(NO_HIDDEN_BABIES))
                                                   /* Superclass */

  class_array[n].class = xmExtObjectClass; n++;    /* object */
  class_array[n].class = xmDesktopClass; n++;      /* ext */
  class_array[n].class = xmWorldClass; n++;        /* desktop */
  class_array[n].class = xmSashWidgetClass; n++;          /* primitive */
  class_array[n].class = xmTearOffButtonWidgetClass; n++; /* pushbutton */

/*  not real Xt object, remove them from the test */
/*  class_array[n].class = xmProtocolObjectClass; n++;       ext */
/*  class_array[n].class = xmShellExtObjectClass; n++;       desktop */
/*  class_array[n].class = xmVendorShellExtObjectClass ; n++;  shelle */
/*  class_array[n].class = xmDialogShellExtObjectClass; n++;   vendorse */
#endif

#if XmREVISION >=2
  class_array[n].class = xmDisplayClass; n++;              /* appshell */
  class_array[n].class = xmScreenClass; n++;               /* core */
  class_array[n].class = xmDragContextClass; n++;          /* core */
  class_array[n].class = xmDragIconObjectClass; n++;       /* object */
  class_array[n].class = xmDropSiteManagerObjectClass; n++;/* object */
  class_array[n].class = xmDropTransferObjectClass; n++;   /* object */
  class_array[n].class = xmDragOverShellWidgetClass; n++;  /* vendors */
#endif

}

static WidgetClass GetClassPointer(class)
String class ;
{
    Cardinal i ;
    static Boolean first_time = True ;

    if (first_time) InitClassArray () ;
    first_time = False ;

    for (i = 0 ; i < XtNumber(class_array); i++) {
	if (strcmp(class, class_array[i].name) == 0)
	    return class_array[i].class ;
    }
    return NULL ;
}


static String buffer = NULL ;
static short rows = 0, columns = 0 ;
    
static void 
#ifndef _NO_PROTO
AddToBuffer (String fmt, ...)
#else
AddToBuffer (fmt, va_alist)
String fmt ;
va_dcl
#endif
{
    va_list args;
    static Cardinal curpos = 0 ;
    char tmp[256] ;
    Cardinal i, tmplen ;

    Va_start(args, fmt);

    (void) vsprintf(tmp, fmt, args);
    tmplen = strlen(tmp) ;
    columns = (columns > tmplen)? columns : tmplen ;
    buffer = XtRealloc(buffer, curpos + tmplen);
    for (i=0 ; tmp[i]; i++) {
	buffer[curpos++] = tmp[i] ;
	if (tmp[i] == '\n') rows ++ ;
    }
    buffer[curpos] = '\0' ;
    

    va_end(args);
}


static void GetPrintRes (name) 
String name ;
{
    Cardinal i, j, k, num_resources, num_sec;
    XtResourceList resource_list ;
    XmSecondaryResourceData * res_sec_list ;
    WidgetClass class ;
    char buff_line[256] ;

    if ((class = GetClassPointer(name)) == NULL) {
	class = widgetClass ;
	name = "Widget" ;
    }

    XtInitializeWidgetClass (class) ;

    AddToBuffer("Fetching resources for widget %s:\n", name);
    AddToBuffer("=======================================\n");

    /* fecth Xt regular */
    XtGetResourceList(class, &resource_list, &num_resources);

    AddToBuffer("\nRegular Xt resources: %d\n",num_resources);
    AddToBuffer(  "------------------------\n");
    for (j=0; j < 256; j++) buff_line[j] = ' ' ;
    for (i = 0 ; i < num_resources; i++) {
	if ((i%2) == 0) {
	    strncpy (buff_line, resource_list[i].resource_name,
		     strlen(resource_list[i].resource_name)) ;
	} else {
	    strcpy (buff_line + 37, resource_list[i].resource_name) ;
	    AddToBuffer(" %s\n", buff_line);
	    for (j=0; j < 256; j++) buff_line[j] = ' ' ;
	}
    }
    if ((i%2) != 0) {
	buff_line[strlen(resource_list[i-1].resource_name)] = '\0' ;
	AddToBuffer(" %s\n", buff_line);
	for (j=0; j < 256; j++) buff_line[j] = ' ' ;
    }

    XtFree((char*)resource_list) ;

    /* fecth Xt constraint  */
    XtGetConstraintResourceList(class, &resource_list, &num_resources);

    if (num_resources) {
	AddToBuffer("\nConstraint Xt resources: %d\n",num_resources);
	AddToBuffer(  "------------------------\n");
	for (j=0; j < 256; j++) buff_line[j] = ' ' ;
	for (i = 0 ; i < num_resources; i++) {
	    if ((i%2) == 0) {
		strncpy (buff_line, resource_list[i].resource_name,
			 strlen(resource_list[i].resource_name)) ;
	    } else {
		strcpy (buff_line + 37, resource_list[i].resource_name) ;
		AddToBuffer(" %s\n", buff_line);
		for (j=0; j < 256; j++) buff_line[j] = ' ' ;
	    }
	}
	if ((i%2) != 0) {
	    buff_line[strlen(resource_list[i-1].resource_name)] = '\0' ;
	    AddToBuffer(" %s\n", buff_line);
	    for (j=0; j < 256; j++) buff_line[j] = ' ' ;
	}

	XtFree((char*)resource_list) ;
    }

    
    /* fetch Motif second */
    num_sec = XmGetSecondaryResourceData (class, &res_sec_list);

    if (num_sec) {
	AddToBuffer("\n\nMotif secondary blocks: %d\n", num_sec);    
	AddToBuffer(    "--------------------------\n");
    
	for (i = 0; i < num_sec; i++) {
	    AddToBuffer("\nSecondary[%d] : %d resources\n\n", 
		   i, res_sec_list[i]->num_resources);
	    for (j = 0 ; j < res_sec_list[i]->num_resources; j++) {
		if ((j%2) == 0) {
		    strncpy (buff_line, 
			     res_sec_list[i]->resources[j].resource_name,
			     strlen(
				res_sec_list[i]->resources[j].resource_name)) ;
		} else {
		    strcpy (buff_line + 37, 
			    res_sec_list[i]->resources[j].resource_name) ;
		    AddToBuffer(" %s\n", buff_line);
		    for (k=0; k < 256; k++) buff_line[k] = ' ' ;
		}
	    }
	    if ((j%2) != 0) {
		buff_line[strlen(
		  res_sec_list[i]->resources[j-1].resource_name)] = '\0' ;
		AddToBuffer(" %s\n", buff_line);
		for (k=0; k < 256; k++) buff_line[k] = ' ' ;
	    }

	    XtFree((char *)res_sec_list[i]->resources);
	    XtFree((char *)res_sec_list[i]);
	}

	XtFree((char*) res_sec_list);
    }
}

main(argc, argv)
int argc ; char **argv;
{
    Widget toplevel, main_window, menu_bar, menu_pane, cascade, 
           button, viewer ;
    XtAppContext app_context;
    String name ;
    Cardinal i ;
    Arg args[10] ;
    Cardinal n;


    /* Usage: getres class | getres All*/
    toplevel = XtAppInitialize(&app_context, "Test", NULL, 0,
			       &argc, argv, NULL, NULL, 0);

    /* get a default in no name provided */
    if (argc == 1) name = "Widget";
    else name = argv[1] ;
    
    /* Fill a string buffer with lines of text (this demo was originally
       designed with printf...) */
    if (strcmp(name, "All") == NULL) {
	for (i = 0; i < XtNumber(class_array); i++) {
	    GetPrintRes (class_array[i].name);
	    AddToBuffer(
            "\n*******************************************************\n\n");
	}
    } else {
	GetPrintRes(name) ;
    }

    n = 0;
    main_window = XmCreateMainWindow (toplevel, "main_window", args, n);
    XtManageChild (main_window);

    /*	Create MenuBar in MainWindow.
     */
    n = 0;
    menu_bar = XmCreateMenuBar (main_window, "menu_bar", args, n); 
    XtManageChild (menu_bar);


    /*	Create "File" PulldownMenu with a Quit button
     */
    n = 0;
    menu_pane = XmCreatePulldownMenu (menu_bar, "menu_pane", args, n);

    n = 0;
    button = XmCreatePushButton (menu_pane, "Quit", args, n);
    XtManageChild (button);
    XtAddCallback (button, XmNactivateCallback, QuitCB, NULL);

    n = 0;
    XtSetArg (args[n], XmNsubMenuId, menu_pane);  n++;
    cascade = XmCreateCascadeButton (menu_bar, "File", args, n);
    XtManageChild (cascade);


    /*	Create "Help" PulldownMenu with Help button.
     */
    n = 0;
    menu_pane = XmCreatePulldownMenu (menu_bar, "menu_pane", args, n);

    n = 0;
    button = XmCreatePushButton (menu_pane, "Help", args, n);
    XtManageChild (button);
    XtAddCallback (button, XmNactivateCallback, HelpCB, NULL);

    n = 0;
    XtSetArg (args[n], XmNsubMenuId, menu_pane);  n++;
    cascade = XmCreateCascadeButton (menu_bar, "Help", args, n);
    XtManageChild (cascade);

    n = 0;
    XtSetArg (args[n], XmNmenuHelpWidget, cascade);  n++;
    XtSetValues (menu_bar, args, n);


    /* Create the viewer widget: here a text */
    n = 0;
    XtSetArg (args[n], XmNvalue, buffer);  n++;
    XtSetArg (args[n], XmNrows, rows+1);  n++;
    XtSetArg (args[n], XmNcolumns, columns-1);  n++;
    XtSetArg (args[n], XmNeditMode, XmMULTI_LINE_EDIT);  n++;
    if (strcmp(name, "All") != NULL) {
	viewer = XmCreateText(main_window, "viewer", args, n);
    } else  {
	viewer = XmCreateScrolledText(main_window, "viewer", args, n);
    } 
    XtManageChild(viewer);

    XtFree(buffer) ; /* Text has its own copy */

    XtRealizeWidget(toplevel);
    XtAppMainLoop(app_context);
}

/*-------------------------------------------------------------
**	QuitCB			- callback for quit button
*/
static void QuitCB (w, client_data, call_data) 
Widget		w;		/*  widget id		*/
caddr_t		client_data;	/*  data from applicaiton   */
caddr_t		call_data;	/*  data from widget class  */
{
    exit (0);
}

/*-------------------------------------------------------------
**	HelpCB			- callback for help button
*/
static void HelpCB (w, client_data, call_data) 
Widget		w;		/*  widget id		*/
caddr_t		client_data;	/*  data from application   */
caddr_t		call_data;	/*  data from widget class  */
{
	static Widget message_box = NULL ;

	if (!message_box) message_box = CreateHelp (w);

	XtManageChild (message_box);
}



/*-------------------------------------------------------------
**	CreateHelp		- create help window
*/
static Widget CreateHelp (parent) 
	Widget		parent;		/*  parent widget	*/
{
	Widget		button;
	Widget		message_box;	/*  Message Dialog 	*/
	Arg		args[20];	/*  arg list		*/
	register int	n;		/*  arg count		*/

	static char	message[1000];	/*  help text	*/
	XmString	title_string = NULL;
	XmString	message_string = NULL;
	XmString	button_string = NULL;

	/*	Generate message to display.
	*/
	sprintf (message, "\
You can specify which widget resources to display on the command\n\
line by using the syntax \"xmgetres <name>\" with one of those names:\n\n\
  Object, RectObj, Widget, Core, Composite, Constraint,\n\n\
  Primitive, ScrollBar, ArrowB, List, Separator, TextF,\n\
  Label, DrawnB, PushB, ToggleB, CascadeB, Text,\n\
  Gadget, ArrowBG, SeparatorG, LabelG, PushBG, ToggleBG, CascadeBG,\n\n\
  Manager, BulletinB, DrawingA, Frame, MainW, Form, RowCol,\n\n\
  ScrolledW, PanedW, SelectionB, FileSB, MessageB, Scale, Command,\n\n\
  Display, Screen, DragContext, DragIcon, DropSiteMgr,\n\
  DropTransfer, DragOverS,\n\n\
  Shell, OverrideShell, WMShell, MenuShell, VendorS,\n\
  ToplevelShell, ApplicationShell, TransientShell, DialogShell.\n\n\
Or you can use \"xmgetres All\".\0");

	message_string = XmStringCreateLtoR (message, 
					     XmSTRING_DEFAULT_CHARSET);
	button_string = XmStringCreateLtoR ("Close", 
					    XmSTRING_DEFAULT_CHARSET);
	title_string = XmStringCreateLtoR ("General Help", 
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

