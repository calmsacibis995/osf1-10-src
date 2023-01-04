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
static char *rcsid = "@(#)$RCSfile: xmlist.c,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/11/05 22:02:12 $";
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
static char rcsid[] = "$RCSfile: xmlist.c,v $ $Revision: 1.1.2.2 $ $Date: 1993/11/05 22:02:12 $"
#endif
#endif
/*
*  (c) Copyright 1989 HEWLETT-PACKARD COMPANY. */

/**---------------------------------------------------------------------
***	
***	file:		xmlist.c
***
***	project:	Motif Widgets example programs
***
***	description:	This program creates a List widget with a 
***			number of items that you can choose from.
***			Each of these items demonstrates a particular
***	                capability of the List widget.
***	
***	defaults:	xmlist.c depends on these defaults:
!
*allowShellResize:		true
*borderWidth:			0
*highlightThickness:		2
*traversalOn:			true
*keyboardFocusPolicy:		explicit
!
xmlist*XmList.listMarginHeight:		5
xmlist*XmList.listMarginWidth:		5
xmlist*XmList.selectionPolicy:  	browse_select
xmlist*XmList.ScrollBarDisplayPolicy:	as_needed
xmlist*XmScrolledWindow.ScrolledWindowMarginHeight:	10
xmlist*XmScrolledWindow.ScrolledWindowMarginWidth:	10
! uncomment the next two lines for a horizontal scroll bar
!xmlist*XmList.listSizePolicy:		constant
!xmlist*XmList.width:			150
!
***---------------------------------------------------------------------**/

#include <stdio.h>
#include <X11/Xlib.h>
#include <X11/X.h>
#include <X11/Intrinsic.h>
#include <X11/Core.h>
#include <X11/Shell.h>
#include <Xm/Xm.h>
#include <Xm/List.h>

Widget  toplevel;
Widget  ListWidgetID;

Arg Args[20];

XmString FirstItem, MiddleItem, LastItem;

Dimension Spacing = 0;
/****************
 *
 * These are the strings that will be displayed by the list. They will
 * be converted into compound strings at run time by the main program.
 *
 ****************/
static char	*CharListItems[25] = {
 "New Item List",
 "New Selected Items",
 "Maximum Visible",
 "Five Visible",
 "New Policy",
 "New Spacing",
 "New Font",
 "Add Item at Top",
 "Add item at End",
 "Add Item in Middle",
 "Delete 'Middle Item'",
 "Delete Item at End",
 "Select Top Item",
 "Select Bottom Item",
 "Deselect Top Item",
 "Deselect Bottom",
 "Deselect All",
 "Make 5th Item Top",
 "Make 1st Top",
 "Make 5th Item Bottom",
 "Make Last Bottom",
 "AutoSelect ON",
 "AutoSelect OFF",
 "Quit"
 };

#define NUM_LIST_ITEMS 24

XmString ListItems[NUM_LIST_ITEMS];

/****************
 *
 * This is the second set of items that will be displayed by the list.
 *
 ****************/
static char *CharNewListItems[7] = {
 "Old Item List",
 "New List item 2",
 "New List item 3",
 "New List item 4",
 "New List item 5",
 "New List item 6",
 "New List item 7"};
 
#define NUM_NEW_LIST_ITEMS 7
XmString NewListItems[NUM_NEW_LIST_ITEMS];

/****************
 *
 * These are the items that will be initially selected by the list.
 *
 ****************/
static char *CharSelectedItems[2] = {
 "New Item List",
 "New Policy"};

#define NUM_SELECTED_ITEMS 2
XmString SelectedItems[NUM_SELECTED_ITEMS];

XmStringCharSet cs = "ISOLatin1";

XFontStruct *fnt, *default_font;
XmFontList font1, font2, font3, curfont;

/************************************************************************
 *									*
 * DumpListCBStruct is called by all of the callback routines.  It	*
 * looks at the reason field to determine which fields should contain	*
 * valid data, and then prints the values of those fields.		*
 *									*
 ************************************************************************/
void DumpListCBStruct(cb)
    XmListCallbackStruct *cb;
{
    int	i;
    char *s;
    
    printf("List Callback Structure: \n");
    printf("    Reason:   ");
	switch(cb->reason) {
	case XmCR_SINGLE_SELECT:
		printf("XmCR_SINGLE_SELECT");break;
	case XmCR_MULTIPLE_SELECT:
		printf("XmCR_MULTIPLE_SELECT");break;
	case XmCR_EXTENDED_SELECT:
		printf("XmCR_EXTENDED_SELECT");break;
	case XmCR_BROWSE_SELECT:
		printf("XmCR_BROWSE_SELECT");break;
	case XmCR_DEFAULT_ACTION:
		printf("XmCR_DEFAULT_ACTION");break;
	case XmCR_HELP:
		printf("XmCR_HELP");break;
	default:
	        printf("Oops - unknown callback reason!");break;
	}
    printf("\n");    
    printf("    Event:    %x\n",cb->event);    
	switch(cb->reason) {
	case XmCR_SINGLE_SELECT:
	case XmCR_BROWSE_SELECT:
	case XmCR_DEFAULT_ACTION:
	    XmStringGetLtoR(cb->item,cs,&s);
	    printf("    Item:     %s\n",s);
	    printf("    Length:   %d\n",cb->item_length);        
	    printf("    Position: %d\n",cb->item_position);    
	    break;

    	case XmCR_EXTENDED_SELECT:
		switch(cb->selection_type) {
		     case XmINITIAL:
		          printf("    Type:     XmINITIAL\n");
			  break;
		     case XmMODIFICATION:
		          printf("    Type:     XmMODIFICATION\n");
			  break;
		     case XmADDITION:
		          printf("    Type:     XmADDITION\n");
			  break;
			  }
	case XmCR_MULTIPLE_SELECT:
	    printf("    SelectedItemCount: %d\n",cb->selected_item_count);    
	    for (i = 0; i < cb->selected_item_count; i++)
	    {
	        XmStringGetLtoR(cb->selected_items[i],cs,&s);
		printf("       SelectedItems[%d](%x): %s\n",i,&(cb->selected_items[i]),s);
		 				     
	    }
	    break;
	}

}

/************************************************************************
 *									*
 * DoubleClickProc is the XmDEFAULT_ACTION callback.  It functions as a	*
 * big case statement, comparing the item that was double-clicked to	*
 * the items in the list.  When it finds a match, it takes the		*
 * appropriate action.							*
 *									*
 ************************************************************************/
void  DoubleClickProc(w,closure,call_data)
    Widget w;
    XtPointer  closure, call_data;
{
    int j;
    XmListCallbackStruct *cb =  (XmListCallbackStruct *)call_data;
    int	i = 0;
    unsigned char k;

    DumpListCBStruct(call_data);
    
    if (XmStringCompare(cb->item,ListItems[0]))		/* Set a new item list */
    {
       XtSetArg(Args[i], XmNitems, (XtArgVal) NewListItems); i++;
       XtSetArg(Args[i], XmNitemCount, (XtArgVal) NUM_NEW_LIST_ITEMS); i++;	
    }
    
    if (XmStringCompare(cb->item,NewListItems[0]))	/* Set the original Item List */
    {
       XtSetArg(Args[i], XmNitems, (XtArgVal) ListItems); i++;
       XtSetArg(Args[i], XmNitemCount, (XtArgVal) NUM_LIST_ITEMS); i++;	
    }

    if (XmStringCompare(cb->item,ListItems[1]))		/* Set a new Selected Item List */
    {
	XtSetArg(Args[i], XmNselectedItems, (XtArgVal) SelectedItems); i++;
	XtSetArg(Args[i], XmNselectedItemCount, (XtArgVal) NUM_SELECTED_ITEMS); i++;
    }

    if (XmStringCompare(cb->item,ListItems[2]))		/* Make all items visible by */
    {                                                   /* getting the current item  */
	XtSetArg(Args[0], XmNitemCount, &j);            /* count and making that the */
	XtGetValues(w, Args, 1);                        /* visible item count. */
	XtSetArg(Args[i], XmNvisibleItemCount, (XtArgVal) j); i++;
    }

    if (XmStringCompare(cb->item,ListItems[3]))		/* Make five items visible */
    {
	XtSetArg(Args[i], XmNvisibleItemCount, (XtArgVal) 5); i++;
    }

    if (XmStringCompare(cb->item,ListItems[4]))		/* Set a new selection policy */
    {
	XtSetArg(Args[0], XmNselectionPolicy, &k);
	XtGetValues(w, Args, 1);
	if (k == XmSINGLE_SELECT)
	   k = XmMULTIPLE_SELECT;
	   else
		  if (k == XmMULTIPLE_SELECT)
		     k = XmBROWSE_SELECT;
		  else
			 if (k == XmBROWSE_SELECT)
			    k = XmEXTENDED_SELECT;
			 else
				if (k == XmEXTENDED_SELECT)
				   k = XmSINGLE_SELECT;
	XtSetArg(Args[i], XmNselectionPolicy, (XtArgVal)k ); i++;
    }
    
    if (XmStringCompare(cb->item,ListItems[5]))         /* Increase the spacing between */
    {                                                   /* items                        */
	Spacing += 2;
	XtSetArg(Args[i], XmNlistSpacing, (XtArgVal)Spacing ); i++;
    }
    
    if (XmStringCompare(cb->item,ListItems[6]))         /* Change the font the items */
    {                                                   /* are displayed in */
	if (curfont == font1) 
            curfont = font2;
	else 
            if (curfont == font2) 
                curfont = font3;
	    else 
                curfont = font1;
	XtSetArg(Args[i], XmNfontList, (XtArgVal) curfont ); i++;
    }

    if (XmStringCompare(cb->item,ListItems[21]))        /* Set automatic selection ON */
    {
	XtSetArg(Args[i], XmNautomaticSelection, (XtArgVal) TRUE); i++;
    }

    if (XmStringCompare(cb->item,ListItems[22]))        /* Set automatic selection OFF */
    {
	XtSetArg(Args[i], XmNautomaticSelection, (XtArgVal) FALSE); i++;
    }
/****************
 *
 * If we have set any arguments, do the SetValues and return.
 *
 ****************/
    if (i > 0)    
    {                              
        XtSetValues(w,Args,i);
        return;
    }
    
    if (XmStringCompare(cb->item,ListItems[7]))		/* Add an item at the first */
    {                                                   /* position in the list     */
	XmListAddItem(w,FirstItem,1);
    }

    if (XmStringCompare(cb->item,ListItems[8]))		/* Add an item at the last  */
    {                                                   /* position in the list     */
	XmListAddItem(w,LastItem,0);
    }

    if (XmStringCompare(cb->item,ListItems[9]))		/* Add an item at the fifth */
    {                                                   /* position in the list     */
	XmListAddItem(w,MiddleItem,5);
    }

    if (XmStringCompare(cb->item,ListItems[10]))	/* Delete the 'Middle Item' */
    {                                                   /* list element             */
	XmListDeleteItem(w, MiddleItem);
    }

    if (XmStringCompare(cb->item,ListItems[11]))	/* Delete the last item     */
    {
	XmListDeletePos(w,0);
    }

    if (XmStringCompare(cb->item,ListItems[12]))	/* Select the first item */
    {
        XmListSelectPos(w,1,TRUE);
    }

    if (XmStringCompare(cb->item,ListItems[13]))	/* Select the last item */
    {
        XmListSelectPos(w,0,TRUE);
    }

    if (XmStringCompare(cb->item,ListItems[14]))	/* Deselect the first item */
    {
        XmListDeselectPos(w,1);
    }

    if (XmStringCompare(cb->item,ListItems[15]))	/* Deselect the last item */
    {
        XmListDeselectPos(w,0);
    }

    if (XmStringCompare(cb->item,ListItems[16]))	/* Deselect all selected items */
    {
        XmListDeselectAllItems(w);
    }
    
    if (XmStringCompare(cb->item,ListItems[17]))	/* Make the fifth item the top */
    {
        XmListSetPos(w,5);
    }

    if (XmStringCompare(cb->item,ListItems[18]))	/* Make the first item the top */
    {
        XmListSetPos(w,1);
    }

    if (XmStringCompare(cb->item,ListItems[19]))	/* Make the fifth item bottom */
    {
        XmListSetBottomPos(w,5);
    }

    if (XmStringCompare(cb->item,ListItems[20]))	/* Make the last item the bottom */
    {
        XmListSetBottomPos(w,0);
    }

    if (XmStringCompare(cb->item,ListItems[23]))	/* End the program. */
    {
        exit(0);
    }

}

/************************************************************************
 *									*
 * These are the selection callbacks for each type of selection.  They	*
 * simply print a message indicating what type of selection occured,	*
 * then call DumpListCBStruct to print the values in the callback	*
 * structure.								*
 *									*
 ************************************************************************/
void  SelectProc(w,closure,call_data)
    Widget w;
    XtPointer  closure, call_data;
{

    printf("Single Selection.\n");
    DumpListCBStruct(call_data);

}

void  MultiProc(w,closure,call_data)
    Widget w;
    XtPointer  closure, call_data;
{

    printf("Multiple Selection.\n");
    DumpListCBStruct(call_data);

}

void  ExtendProc(w,closure,call_data)
    Widget w;
    XtPointer  closure, call_data;
{

    printf("Extended Selection.\n");
    DumpListCBStruct(call_data);

}

void  BrowseProc(w,closure,call_data)
    Widget w;
    XtPointer  closure, call_data;
{

    printf("Browse Selection.\n");
    DumpListCBStruct(call_data);

}

void  Help(w,closure,call_data)
    Widget w;
    XtPointer  closure, call_data;
{
    DumpListCBStruct(call_data);
}

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
    Widget	w1,w2;
    XmString    dir,xs;
           
    XtToolkitInitialize();
    app_context = XtCreateApplicationContext();
    display = XtOpenDisplay(app_context, NULL,
	argv[0], "XMdemos", NULL, 0, &argc, argv);
    if (!display) { printf("Unable to open display\n"); exit(0); }

    toplevel = XtAppCreateShell(argv[0], NULL,
		applicationShellWidgetClass,
		display, NULL, 0);

    i = 0; items = 0; selected = 0;
    
/****************
 *
 * Create the arrays of compound strings from the character arrays.
 *
 ****************/
    for(i = 0; i < NUM_LIST_ITEMS; i++)
	ListItems[i] = (XmString )XmStringCreateLtoR(CharListItems[i],cs);
    for(i = 0; i < NUM_SELECTED_ITEMS; i++)
	SelectedItems[i] = (XmString )XmStringCreateLtoR(CharSelectedItems[i],cs);
    for(i = 0; i < NUM_NEW_LIST_ITEMS; i++)
	NewListItems[i] = (XmString )XmStringCreateLtoR(CharNewListItems[i],cs);

/****************
 *
 * Load the X font structures, and create the necessary fontlists.
 *
 ****************/
    default_font = XLoadQueryFont(XtDisplay(toplevel), "fixed");

    fnt = XLoadQueryFont(XtDisplay(toplevel), "9x15bold");
    if (fnt != NULL) 
    	font1 = XmFontListCreate(fnt, cs);
    else
	font1 = XmFontListCreate(default_font, cs);

    fnt = XLoadQueryFont(XtDisplay(toplevel), "6x12");
    if (fnt != NULL) 
    	font2 = XmFontListCreate(fnt, XmSTRING_DEFAULT_CHARSET);
    else 
	font2 = XmFontListCreate(default_font, XmSTRING_DEFAULT_CHARSET);

    fnt = XLoadQueryFont(XtDisplay(toplevel), "10x20");
    if (fnt != NULL) 
    	font3 = XmFontListCreate(fnt, "FooSet");
    else 
	font3 = XmFontListCreate(default_font, "FooSet");

    curfont = font1;

/****************
 *
 * Create the individual items to add to the list.
 *
 ****************/
    FirstItem = XmStringCreateLtoR("First Item",cs);
    MiddleItem = XmStringCreateLtoR("*Middle Item*", cs);
    LastItem = XmStringCreateLtoR("Tail End Charles",cs);

/****************
 *
 * Create the scrolled list. We will give it the selcted items now, and
 * add in the item list later.
 *
 ****************/
    i = 0;
    XtSetArg(Args[i], XmNfontList, (XtArgVal) curfont); i++;
    XtSetArg(Args[i], XmNselectedItems, (XtArgVal) SelectedItems); i++;
    XtSetArg(Args[i], XmNselectedItemCount, (XtArgVal) NUM_SELECTED_ITEMS); i++;
    XtSetArg(Args[i], XmNvisibleItemCount, (XtArgVal) NUM_LIST_ITEMS); i++;    

    ListWidgetID = XmCreateScrolledList(toplevel, "ListWidget", Args, i);

    XtManageChild(ListWidgetID);

/****************
 *
 * Add the callbacks.
 *
 ****************/
    XtAddCallback(ListWidgetID, XmNhelpCallback, Help, NULL);
    XtAddCallback(ListWidgetID, XmNsingleSelectionCallback, SelectProc, NULL);
    XtAddCallback(ListWidgetID, XmNdefaultActionCallback, DoubleClickProc,NULL);
    XtAddCallback(ListWidgetID, XmNmultipleSelectionCallback, MultiProc, NULL);
    XtAddCallback(ListWidgetID, XmNextendedSelectionCallback, ExtendProc, NULL);
    XtAddCallback(ListWidgetID, XmNbrowseSelectionCallback, BrowseProc, NULL);

/****************
 *
 * Get the widget IDs of the scrollbars and add them to the tab group.
 * (See the section on "Keyboard Traversal" for more details on tab
 * groups).
 *
 ****************/
    i = 0;
    XtSetArg(Args[i], XmNhorizontalScrollBar, (XtArgVal) &w1); i++;
    XtSetArg(Args[i], XmNverticalScrollBar, (XtArgVal) &w2); i++;    
    XtGetValues(ListWidgetID,Args,i);    

    XmAddTabGroup(ListWidgetID);
    XmAddTabGroup(w2);    
    if (w1) XmAddTabGroup(w1);    
        
/****************
 *
 * Now set the items into the list.
 *
 ****************/
    i = 0;
    XtSetArg(Args[i], XmNitems, (XtArgVal) ListItems); i++;    
    XtSetArg(Args[i], XmNitemCount, (XtArgVal) NUM_LIST_ITEMS); i++;
    XtSetValues(ListWidgetID,Args,i);

/****************
 *
 * Realize the hierarchy, and call the event processing loop.
 *
 ****************/
    XtRealizeWidget(toplevel);
    XtAppMainLoop(app_context);
}



