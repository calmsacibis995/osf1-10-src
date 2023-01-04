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
static char *rcsid = "@(#)$RCSfile: xmprotocol.c,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/11/05 22:02:35 $";
#endif
/* 
 * (c) Copyright 1989, 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
*/ 
/* 
 * Motif Release 1.2
*/ 
#ifdef REV_INFO
#ifndef lint
static char rcsid[] = "$RCSfile: xmprotocol.c,v $ $Revision: 1.1.2.2 $ $Date: 1993/11/05 22:02:35 $"
#endif
#endif
/*
*  (c) Copyright 1989 HEWLETT-PACKARD COMPANY. */

/**---------------------------------------------------------------------
***	
***	file:		xmprotocol.c
***
***	project:	Motif Widgets example programs
***
***	description:	This program demonstrates the Motif protocols
***			for communication between the toolkit and the
***			window manager.
***	
***-------------------------------------------------------------------*/


#include <stdio.h>
#include <sys/signal.h>
  
#include <X11/IntrinsicP.h>
#include <X11/StringDefs.h>
  
#include <X11/Shell.h>
#include <Xm/Xm.h>
#include <Xm/XmP.h>
  
#include <Xm/RowColumn.h>
#include <Xm/ScrollBar.h>
#include <Xm/DrawingA.h>
#include <Xm/ScrolledW.h>
#include <Xm/PushB.h>
  
#include <Xm/Protocols.h>
#include <Xm/AtomMgr.h>
#include <Xm/MwmUtil.h>
  
Atom	wm_delete_window, mwm_messages;
  
  
static Widget	ShellAncestor(w)
    Widget	w;
{
    while ((w != NULL) && !XtIsShell(w))
      w = w->core.parent;
    
    return(w);
}


#define SET_ITEMS 0
#define REMOVE_CALLBACKS 1
#define ACTIVATE_ITEMS 2
#define DEACTIVATE_ITEMS 3
#define REMOVE_PROTOCOLS 4

typedef struct _MenuChangeRec{
    String	label;
    Cardinal	start_num, end_num;
    int		operation;
}MenuChangeRec;


static void EntrySelected(w, closure, call_data)
    Widget	w;
    XtPointer	closure, call_data;
{
    printf("entry %d selected\n", closure);
}


void MakeMenu(start_num, end_num, str_rtn)
    Cardinal	start_num, end_num;
    String	str_rtn;
{
    Cardinal	i, j;
    String	str = str_rtn;
    
    if (!start_num && !end_num)
      {
	  *str = '\0';
      }
    else 
      {
	  for (i = start_num; i<= end_num ; i++)
	    {
		sprintf(str, " entry_%d f.send_msg %d \n",i,i);
		str += strlen(str);
	    }
	  *(str-1) = '\0';
      }
}

void ChangeMenu(w, client_data, call_data)
     Widget      w;
     XtPointer   client_data;
     XtPointer   call_data;
    
{
    MenuChangeRec *menu_data = (MenuChangeRec *) client_data;
    Widget		shell;
    Arg			arg;
    static char		menu_string[256];
    Cardinal		i;
    Atom		atoms[16];
    Boolean		doUnmap = False;
    
    shell = ShellAncestor(w);
    
    switch (menu_data->operation)
      {
	case SET_ITEMS:
	  

	  /*
	   * Generate the string that is passed to mwm for appending
	   * (after interpretation) to the system menu. 
	   */
	  MakeMenu(menu_data->start_num,
		   menu_data->end_num,
		   menu_string);
	  
	  XtSetArg(arg, XmNmwmMenu, menu_string);
	  XtSetValues(shell, &arg, 1);
	  
	  for (i = menu_data->start_num; i<= menu_data->end_num ; i++)
	    /* 
	     * XmAddProtocolCallback will check to see if the protocol
	     * already exists, and add it (by calling XmAddProtocols)
	     * if it doesn't. It doesn't check for duplicate callbacks
	     * or change the state of the activation. 
	     */
	    XmAddProtocolCallback(shell, mwm_messages,
				  (Atom)i,
				  EntrySelected, (XtPointer)i);
          doUnmap = True;
	  break;
	  
	case REMOVE_CALLBACKS:
	  for (i = menu_data->start_num; i<= menu_data->end_num ; i++)
	    /*
	     * RemoveProtocolCallback will remove the first callback
	     * with this signature from the list. It does not check
	     * for duplicates or change the activation (gray-out). 
	     */
	    XmRemoveProtocolCallback(shell, mwm_messages,
				     (Atom)i,
				     EntrySelected, (XtPointer)i);
	  break;

	case REMOVE_PROTOCOLS:
	  for (i = menu_data->start_num; i<= menu_data->end_num ; i++)
	    atoms[i - menu_data->start_num] = i;
	  XmRemoveProtocols(shell, mwm_messages,
			    atoms, 
			    menu_data->end_num - menu_data->start_num +1);
          doUnmap = True;
	  break;
	  
	case ACTIVATE_ITEMS:
	  for (i = menu_data->start_num; i<= menu_data->end_num ; i++)
	    /* 
	     * Tell Mwm these protocols (menu items) are now active
	     */
	    XmActivateProtocol(shell, mwm_messages, (Atom)i);
	  break;
	case DEACTIVATE_ITEMS:
	  for (i = menu_data->start_num; i<= menu_data->end_num ; i++)
	    /* 
	     * Tell Mwm these protocols (menu items) are now inactive
	     */
	    XmDeactivateProtocol(shell, mwm_messages, (Atom)i);
	  break;
      }
    
    if (XtIsRealized(shell) && doUnmap) {
	XtUnmapWidget(shell);
	XtMapWidget(shell);
    }
}

static void Quit(i)
     int i;
{
    exit(0);
}

static void QuitCB(w, client_data, call_data)
     Widget w;
     XtPointer client_data, call_data;
{
    exit(0);
}


MenuChangeRec	menuops[] = {
    { "add entry 0-4 to the System Menu", 0, 4, SET_ITEMS},
    { "add entry 1-3 to the System Menu", 1, 3, SET_ITEMS},
    { "add entry 2 to the System Menu", 2, 2, SET_ITEMS},
    { "deactivate entry 0-2 from the System Menu", 0, 2, DEACTIVATE_ITEMS},
    { "activate entry 0-2 from the System Menu" , 0, 2, ACTIVATE_ITEMS},
    { "deactivate entry 2-4 from the System Menu", 2, 4, DEACTIVATE_ITEMS},
    { "activate entry 2-4 from the System Menu" , 2, 4, ACTIVATE_ITEMS},
};


XtAppContext	app_context;
Display*	display;

#ifdef DEC_MOTIF_BUG_FIX
int main (argc,argv)
#else
void main (argc,argv)
#endif
    int  argc;
    char **argv;
{
    int i,items, selected;
    unsigned char policy;
    XEvent	event;
    Widget  	inner_box, toplevel,fw,bb;
    Widget	buttons[16];
    XmScrolledWindowWidget  sw;
    Widget  	hsb,vsb;
    
    
    
    static Boolean 	BailOut;
    Arg 		myArgs[20];
    
    XmString 	text;
    XmStringCharSet cs = "ISOLatin1";
    
    XFontStruct 	*fnt;
    XmFontList 		font1, font2, font3, font4, font5, curfont;
    int    		NewPbutton;
    


    signal(SIGINT, Quit);
    
    XtToolkitInitialize();
    app_context = XtCreateApplicationContext();
    display = XtOpenDisplay(app_context, NULL,
                argv[0], "XmDemos", NULL, 0, &argc, argv);
    if (!display) { printf("Unable to open display\n"); exit(0); }

    toplevel = XtAppCreateShell(argv[0], NULL,
		applicationShellWidgetClass,
		display, NULL, 0);
    
    /*
     * intern the atoms and add mwm_messages to wm_protocols so that
     * mwm will look for mwm_messages property changes. Also add a
     * callback for handling WM_DELETE_WINDOW for when the user
     * selects the close button.
     */
    mwm_messages = XmInternAtom(XtDisplay(toplevel),
				_XA_MWM_MESSAGES,
				FALSE);
    
    wm_delete_window = XmInternAtom(XtDisplay(toplevel),
				"WM_DELETE_WINDOW",
				FALSE);
    
    XmAddWMProtocolCallback(toplevel, wm_delete_window, QuitCB, NULL);

    XmAddWMProtocols(toplevel, &mwm_messages, 1);
			    
    fnt = XLoadQueryFont(XtDisplay(toplevel), "9x15");
    font1 = XmFontListCreate(fnt, cs);
    fnt = XLoadQueryFont(XtDisplay(toplevel), "8x13");
    font2 = XmFontListCreate(fnt, cs);
    fnt = XLoadQueryFont(XtDisplay(toplevel), "6x12");
    font3 = XmFontListCreate(fnt, cs);

    /*
     * Added to test PIRs #655 and #874
     *
     *						04/23/90
     */

    fnt = XLoadQueryFont(XtDisplay(toplevel), NULL);
    font4 = XmFontListCreate(fnt, cs);
    font5 = XmFontListAdd(font3, fnt, cs);
    curfont = font1;
    
    i = 0; items = 0; selected = 0;
    XtSetArg(myArgs[i], XmNscrollingPolicy, XmAUTOMATIC); i++;    
    XtSetArg(myArgs[i], XmNwidth, 300); i++;    
    XtSetArg(myArgs[i], XmNheight, 150); i++;    
    sw = (XmScrolledWindowWidget )
      XtCreateManagedWidget("ScrolledWindow", 
			    xmScrolledWindowWidgetClass, toplevel, 
			    (ArgList)myArgs, i);
    
    i = 0;
    XtSetArg (myArgs[i], XmNscrollingPolicy,(XtArgVal) &policy); i++;
    XtGetValues((Widget)sw,myArgs,1);
    
    if (policy == XmAPPLICATION_DEFINED)
      {
	  i = 0;
	  XtSetArg (myArgs[i], XmNorientation,(XtArgVal) (XmVERTICAL)); i++;
	  vsb = XtCreateManagedWidget("vScrollBar", xmScrollBarWidgetClass,
				      (Widget) sw, myArgs, i); 
	  i = 0;
	  XtSetArg (myArgs[i], XmNorientation,(XtArgVal) (XmHORIZONTAL)); i++;
	  hsb = XtCreateManagedWidget("hScrollBar", xmScrollBarWidgetClass,
				      (Widget) sw, myArgs, i);
      }
    
    
    i = 0;
    XtSetArg(myArgs[i], XmNborderWidth, (XtArgVal) 2);    i++;
    XtSetArg(myArgs[i], XmNshadowThickness, (XtArgVal) 0);    i++;
    inner_box = XtCreateManagedWidget ("RowCol", xmRowColumnWidgetClass, (Widget)sw,
				       (ArgList)myArgs, i);
    

    /*
     * Create the button list from the item table. All buttons call
     * the ChangeMenu routine.
     */
    for (items = 0; items < XtNumber(menuops); items++)
      {
	  i = 0;
	  text = XmStringCreateLtoR(menuops[items].label, cs);
	  XtSetArg(myArgs[i], XmNfontList, (XtArgVal) curfont);i++;
	  XtSetArg(myArgs[i], XmNlabelString, (XtArgVal) text);i++;
	  XtSetArg(myArgs[i], XmNlabelType, (XtArgVal) XmSTRING);i++;
	  buttons[items] = 
	    XtCreateManagedWidget(argv[0], xmPushButtonWidgetClass,
				  inner_box, (ArgList)myArgs, i);
	  XtAddCallback(buttons[items], XmNactivateCallback, 
			ChangeMenu, (XtPointer)&menuops[items]);
      }

    i = 0;
    if (policy == XmAPPLICATION_DEFINED)
      {
	  XtSetArg(myArgs[i], XmNhorizontalScrollBar, (XtArgVal) hsb); i++;
	  XtSetArg(myArgs[i], XmNverticalScrollBar, (XtArgVal) vsb); i++;
      }
    XtSetArg(myArgs[i], XmNworkWindow, (XtArgVal)inner_box); i++;
    
    XtSetValues((Widget)sw, myArgs, i);
    XtRealizeWidget(toplevel);
    
    XtAppMainLoop(app_context);
  }
