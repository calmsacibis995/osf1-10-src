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
static char *rcsid = "@(#)$RCSfile: TextList.c,v $ $Revision: 1.1.2.6 $ (DEC) $Date: 1993/12/20 21:30:33 $";
#endif

/******************************************************************************
*******************************************************************************
*
*  (c) Copyright 1989, 1990, OPEN SOFTWARE FOUNDATION, INC.
*  (c) Copyright 1989, DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASS.
*  (c) Copyright 1987, 1988, 1989, 1990, HEWLETT-PACKARD COMPANY
*  ALL RIGHTS RESERVED
*  
*  	THIS SOFTWARE IS FURNISHED UNDER A LICENSE AND MAY BE USED
*  AND COPIED ONLY IN ACCORDANCE WITH THE TERMS OF SUCH LICENSE AND
*  WITH THE INCLUSION OF THE ABOVE COPYRIGHT NOTICE.  THIS SOFTWARE OR
*  ANY OTHER COPIES THEREOF MAY NOT BE PROVIDED OR OTHERWISE MADE
*  AVAILABLE TO ANY OTHER PERSON.  NO TITLE TO AND OWNERSHIP OF THE
*  SOFTWARE IS HEREBY TRANSFERRED.
*  
*  	THE INFORMATION IN THIS SOFTWARE IS SUBJECT TO CHANGE WITHOUT
*  NOTICE AND SHOULD NOT BE CONSTRUED AS A COMMITMENT BY OPEN SOFTWARE
*  FOUNDATION, INC. OR ITS THIRD PARTY SUPPLIERS  
*  
*  	OPEN SOFTWARE FOUNDATION, INC. AND ITS THIRD PARTY SUPPLIERS,
*  ASSUME NO RESPONSIBILITY FOR THE USE OR INABILITY TO USE ANY OF ITS
*  SOFTWARE .   OSF SOFTWARE IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
*  KIND, AND OSF EXPRESSLY DISCLAIMS ALL IMPLIED WARRANTIES, INCLUDING
*  BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
*  FITNESS FOR A PARTICULAR PURPOSE.
*  
*  Notice:  Notwithstanding any other lease or license that may pertain to,
*  or accompany the delivery of, this computer software, the rights of the
*  Government regarding its use, reproduction and disclosure are as set
*  forth in Section 52.227-19 of the FARS Computer Software-Restricted
*  Rights clause.
*  
*  (c) Copyright 1989, 1990, Open Software Foundation, Inc.  Unpublished - all
*  rights reserved under the Copyright laws of the United States.
*  
*  RESTRICTED RIGHTS NOTICE:  Use, duplication, or disclosure by the
*  Government is subject to the restrictions as set forth in subparagraph
*  (c)(1)(ii) of the Rights in Technical Data and Computer Software clause
*  at DFARS 52.227-7013.
*  
*  Open Software Foundation, Inc.
*  11 Cambridge Center
*  Cambridge, MA   02142
*  (617)621-8700
*  
*  RESTRICTED RIGHTS LEGEND:  This computer software is submitted with
*  "restricted rights."  Use, duplication or disclosure is subject to the
*  restrictions as set forth in NASA FAR SUP 18-52.227-79 (April 1985)
*  "Commercial Computer Software- Restricted Rights (April 1985)."  Open
*  Software Foundation, Inc., 11 Cambridge Center, Cambridge, MA  02142.  If
*  the contract contains the Clause at 18-52.227-74 "Rights in Data General"
*  then the "Alternate III" clause applies.
*  
*  (c) Copyright 1989, 1990, Open Software Foundation, Inc.
*  ALL RIGHTS RESERVED 
*  
*  
* Open Software Foundation is a trademark of The Open Software Foundation, Inc.
* OSF is a trademark of Open Software Foundation, Inc.
* OSF/Motif is a trademark of Open Software Foundation, Inc.
* Motif is a trademark of Open Software Foundation, Inc.
* DEC is a registered trademark of Digital Equipment Corporation
* DIGITAL is a registered trademark of Digital Equipment Corporation
* X Window System is a trademark of the Massachusetts Institute of Technology
*
*******************************************************************************
*
* TextList
*
* The textlist widget is a subclass of the text(field) widget.  It differs 
* from a text widget in that it creates a small "nipple" button which,
* when depressed creates a pulldown scrolled list of choices.
* 
* In fact, textlist was create from ticker (Jeff Glass) which was inspired 
* by the "NYSE stockmarket ticker program" posted to the DW_EXAMPLES notesfile 
* by Roger (UBEAUT::) Luk, who also acknowledged Patrick (SDSVAX::) Sweeney and * Mark 'DECwindows' Priebatsch. Many ideas and some source code was taken from 
* this program.
*
* This widget is derived from Motif source code (the Text,ToggleButton and 
* Label widgets).  It is Copyright 1990 by Digital Equipment Corporation.
*
* Robert Primak 8/05/93
*
******************************************************************************/
/*
 * Include files & Static Routine Definitions
 */

#include <strings.h>
#include <stdio.h>

#include <X11/X.h>
#include <X11/cursorfont.h>
#include <Xm/TextFP.h>
#include <Xm/XmP.h>
#include "TextListP.h"
#include <Mrm/MrmPublic.h>
#include <Xm/PushB.h>
#include <Xm/List.h>

#include "textlist.xbm"

static void	Initialize();
static void	Destroy();
static void     MakeCopy();

void XmTextListAddItem();
void XmTextListAddItems();
void XmTextListAddTabGroup();
void XmTextListSetSensitive();

void ManualTextEntryActivateCB();

#define RT_X_COORD_OF_WIDGET(w)  ((w)->core.x + (w)->core.width +\
                       ((XmPrimitiveWidget)w)->primitive.highlight_thickness + \
                       ((XmPrimitiveWidget)w)->primitive.shadow_thickness)

#define TOP_Y_COORD_OF_WIDGET(w) ((w)->core.y)

/*************************************<->*************************************
 *
 *
 *   Description:  resource list for class: TextList
 *   -----------
 *
 *   To get the full set of default settings, examine the resource lists
 *   of superclasses of this class.
 *
 *************************************<->***********************************/

static XtResource resources[] = 
{
   {
     XmNtextListForm,
     XmCWidget,
     XmRWidget,
     sizeof(Widget),
     XtOffset(XmTextListWidget,textlist.text_list_fr),
     XmRImmediate,
     (caddr_t)10
   },

   {
     XmNtextListCancelPushButton,
     XmCWidget,
     XmRWidget,
     sizeof(Widget),
     XtOffset(XmTextListWidget,textlist.text_list_cancel_pb),
     XmRImmediate,
     (caddr_t)300
   },

   {
     XmNtextListScrolledList,
     XmCWidget,
     XmRWidget,
     sizeof(Widget),
     XtOffset(XmTextListWidget,textlist.text_list_sl),
     XmRImmediate,
     (caddr_t)600
   },

   {
     XmNtextListPushButton,
     XmCWidget,
     XmRWidget,
     sizeof(Widget),
     XtOffset(XmTextListWidget,textlist.text_list_pb),
     XmRImmediate,
     (caddr_t)900
   },

   {
     XmNtextListItems,
     XmCXmString,
     XmRXmStringTable,
     sizeof(XmStringTable),
     XtOffset(XmTextListWidget,textlist.text_list_items),
     XmRImmediate,
     (caddr_t)1200
   },

   {
     XmNtextListNoNewEntry,
     XmCBoolean,
     XmRBoolean,
     sizeof(Boolean),
     XtOffset(XmTextListWidget,textlist.no_new_entry),
     XmRImmediate,
     (caddr_t)1500
   },

   {
     XmNtextListAlphabetizeLists,
     XmCBoolean,
     XmRBoolean,
     sizeof(Boolean),
     XtOffset(XmTextListWidget,textlist.text_list_alphabetize_lists),
     XmRImmediate,
     (caddr_t)1600
   },
};

/* Definition for resources that need special processing in get values */

static XmSyntheticResource get_resources[] =
{
   {
     XmNmarginWidth,
     sizeof(Dimension),
     XtOffset(XmTextFieldWidget, text.margin_width),
     _XmFromHorizontalPixels,
     _XmToHorizontalPixels
   },

   {
     XmNmarginHeight,
     sizeof(Dimension),
     XtOffset(XmTextFieldWidget, text.margin_height),
     _XmFromVerticalPixels,
     _XmToVerticalPixels
   },
   {
     XmNvalue,
     sizeof(char *),
     XtOffset(XmTextFieldWidget, text.value),
     MakeCopy,
     NULL,
   },
};

/*************************************<->*************************************
 *
 *
 *   Description:  global class record for instances of class: Ticker
 *   -----------
 *
 *   Defines default field settings for this class record.
 *
 *************************************<->***********************************/

externaldef(xmtextlistclassrec) 
	XmTextListClassRec xmTextListClassRec = {
   {
    /* superclass	  */	(WidgetClass)&xmTextFieldClassRec,
    /* class_name	  */	"XmTextList",
    /* widget_size	  */	sizeof(XmTextListRec),
    /* class_initialize   */    NULL,
    /* class_part_init    */    NULL,				
    /* class_inited       */	FALSE,
    /* initialize	  */	Initialize,
    /* initialize_hook    */    NULL,
    /* realize		  */	XtInheritRealize,
    /* actions		  */	NULL,
    /* num_actions	  */	0,
    /* resources	  */	resources,
    /* num_resources	  */	XtNumber(resources),
    /* xrm_class	  */	NULLQUARK,
    /* compress_motion	  */	TRUE,
    /* compress_exposure  */	XtExposeCompressMaximal,
    /* compress_enterlv   */    TRUE,
    /* visible_interest	  */	FALSE,
    /* destroy		  */	Destroy,
    /* resize		  */	XtInheritResize,
    /* expose		  */	XtInheritExpose,
    /* set_values	  */	NULL,
    /* set_values_hook    */    NULL,
    /* set_values_almost  */    XtInheritSetValuesAlmost,
    /* get_values_hook    */	NULL,
    /* accept_focus       */    XtInheritAcceptFocus,
    /* version            */	XtVersion,
    /* callback_private   */    NULL,
    /* tm_table           */    NULL,
    /* query_geometry     */	XtInheritQueryGeometry, 
    /* display_accelerator */   NULL,
    /* extension          */    NULL,
   },

   {                            /* primitive_class fields       */
      (XtWidgetProc)_XtInherit,/* Primitive border_highlight   */
      (XtWidgetProc)_XtInherit, /* Primitive border_unhighlight */
      NULL,                     /* translations                 */
      NULL,                     /* arm_and_activate             */
      get_resources,            /* get resources                */
      XtNumber(get_resources),  /* num get_resources            */
      NULL,                     /* extension                    */
   },

   {                            /* text class fields */
      NULL,                     /* extension         */
   }
};

externaldef(xmtextlistwidgetclass)
   WidgetClass xmTextListWidgetClass = (WidgetClass)&xmTextListClassRec;

static XmStringTable items;
static int item_count;
static Dimension original_width;
static int PARTIAL_LIST = 0;

/******************************************************************************
 * MakeCopy()
 ******************************************************************************/
static void MakeCopy(w, offset, value)
Widget w;
int offset;
XtArgVal * value;
{
    (*value) = (XtArgVal) XmTextFieldGetString (w);
}
/******************************************************************************
 * OriginalParent()
 ******************************************************************************/
static Widget OriginalParent(child)
Widget child;

{
Widget w,parent;

for (w=child; w!=NULL; w=XtParent(w))
    {
    parent=w;
    }
return(parent);
}


XtEventHandler TextListHandleFocusOutEvent(w, tl, event, cont)
Widget           w;
XmTextListWidget tl;
XEvent          *event;
Boolean         *cont;

{
    switch(event->type)
    {
      case FocusOut:
             /* Trigger the User Cancel PB Activate Callback */
             XtCallCallbacks((Widget)tl->textlist.text_list_cancel_pb,
                             XmNactivateCallback,NULL);
        break;
      default:
        break;
    }
}
/******************************************************************************
 *  DisplayScrolledList()
 ******************************************************************************/
static void DisplayScrolledList(tl)
XmTextListWidget tl;

{
int n;
Arg args[10];
Dimension width,height,shadow_thickness;
Position x,y,pbroot_x,pbroot_y,droot_x,droot_y;
XtAppContext app_context;

/* 
 * Find the width and height of the Push Button, this can be done at
 * Push Button creation instead of repeatedly in the Callback as the
 * with and height should not change. 
 */
n=0;
XtSetArg(args[n],XmNwidth,&width);n++;
XtSetArg(args[n],XmNheight,&height);n++;
XtSetArg(args[n],XmNshadowThickness,&shadow_thickness);n++;
XtGetValues(tl->textlist.text_list_pb,args,n);

/* Manage the Form Dialog Scrolled List to set its coordinates */
XtManageChild(tl->textlist.text_list_fr);
 
/* Find the root coordinates of the spawning PB and the Form Dialog */
XtTranslateCoords(tl->textlist.text_list_pb,0,0,&pbroot_x,&pbroot_y);
XtTranslateCoords(tl->textlist.text_list_fr,0,0,&droot_x,&droot_y);
 
/* Find the current X and Y coordinates of the Form Dialog */
n=0;
XtSetArg(args[n],XmNx,&x);n++;
XtSetArg(args[n],XmNy,&y);n++;
XtGetValues(tl->textlist.text_list_fr,args,n);

/* printf("adjustment x %d y %d\n",(droot_x-pbroot_x),(droot_y-pbroot_y)); */

/* Adjust the Form Dialog to position next to the spawning PB */
n=0;
XtSetArg(args[n],XmNx,x-(droot_x-pbroot_x)+width+shadow_thickness);n++;
XtSetArg(args[n],XmNy,y-(droot_y-pbroot_y)+height+shadow_thickness);n++;
XtSetValues(tl->textlist.text_list_fr,args,n);

/* Process Events until the window maps */

app_context = XtWidgetToApplicationContext(tl->textlist.text_list_fr);

XSync(XtDisplay(tl->textlist.text_list_fr),False);

while (1)
   {
   XEvent event;

   XtAppNextEvent(app_context,&event);
   XtDispatchEvent(&event);
   if ((event.type == Expose) && (event.xexpose.window == 
       XtWindow(tl->textlist.text_list_fr)))
      break;
   }

XSync(XtDisplay(tl->textlist.text_list_fr),False);

XSetInputFocus(XtDisplay(tl),XtWindow(tl->textlist.text_list_fr),
               RevertToParent, CurrentTime);
XmProcessTraversal(tl->textlist.text_list_fr,XmTRAVERSE_CURRENT);
}
/******************************************************************************
 *  RepopulatePartialList()
 ******************************************************************************/
static void RepopulatePartialList(tl)
XmTextListWidget tl;

{
int i;
Arg args[10];

if (PARTIAL_LIST)
   {
   /* Delete the partial item list */
   XmTextListDeleteAllItems(tl);
   /* Reset the list to the globally allocated superset of items */
   XmTextListAddItems(tl,items,item_count,0);

   /* Reset the list to its original width */
   XtSetArg(args[0],XmNwidth,original_width);
   XtSetValues(tl->textlist.text_list_sl,args,1);

   /* Free the item list */
   for (i=0; i<item_count; i++)
       XmStringFree(items[i]);
   XtFree((char *)items);

   PARTIAL_LIST = 0;
   }
}
/******************************************************************************
 *  ReduceListToMatchingItems()
 ******************************************************************************/
static Boolean ReduceListToMatchingItems(tl)
XmTextListWidget tl;

{
int i,n;
Arg args[10];
char *str,*str2,*str3;
XmStringTable items_tmp;

str = XmTextFieldGetString(tl);

/* Check for metacharacter from back of string (eg "Smi*") */
if ((str2 = (rindex(str,'*'))) != NULL)
   {
   /* remove the metacharacter from the string */
   str[str2 - str] = NULL;

   /* Make a copy of the item string table */
   n = 0;
   XtSetArg(args[n],XmNitems,&items_tmp);n++;
   XtSetArg(args[n],XmNitemCount,&item_count);n++;

   /* Store the width in case the remaining items shrink the scrolled list */
   XtSetArg(args[n],XmNwidth,&original_width);n++;

   XtGetValues(tl->textlist.text_list_sl,args,n);

   /* Malloc a String Table for item_count items */
   items = (XmStringTable)XtMalloc(sizeof(XmStringTable)*item_count);
   if (items == NULL)
      exit(0);

   /* Copy the Items */
   for (i=0; i<item_count; i++)
       items[i] = XmStringCopy(items_tmp[i]);

   /* delete the existing items */
   XmTextListDeleteAllItems(tl);

   /* Add back only the matching items */
   for (i=0; i<item_count; i++)
       {
       if (XmStringGetLtoR(items[i],XmSTRING_DEFAULT_CHARSET,&str2))
          {
          str3 = strstr(str2,str);
          if (str3 == str2)
             {
             XmTextListAddItem(tl,items[i],0);
             }
          }
       }

   PARTIAL_LIST = 1;

   return(True);
   }

return(False);
}
/******************************************************************************
 *  ListItemSelectedCB()
 ******************************************************************************/
void ListItemSelectedCB(w,tl,call_data)
Widget  w;
XmTextListWidget tl;
XmListCallbackStruct *call_data;
{
int     ret;
char   *str;
XEvent  event;

XtUnmanageChild(XtParent(XtParent(w)));

if (XmStringGetLtoR(call_data->item,XmSTRING_DEFAULT_CHARSET,&str))
   {
   XmTextSetString(tl,str);

   XtRemoveCallback((Widget)tl,XmNactivateCallback,
                 (XtCallbackProc) ManualTextEntryActivateCB,(caddr_t)tl);

   /* Trigger the User Defined Text Field Activate Callback */
   XtCallCallbacks((Widget)tl,XmNactivateCallback,NULL);

   XtAddCallback((Widget)tl,XmNactivateCallback,
                 (XtCallbackProc) ManualTextEntryActivateCB,(caddr_t)tl);
   }
else 
   XmTextSetString(tl,"?????????");

/* Set the input focus to the text field */
XSetInputFocus(XtDisplay(tl),XtWindow(tl), RevertToParent, CurrentTime);
XmProcessTraversal(tl,XmTRAVERSE_CURRENT);

RepopulatePartialList(tl);
}
/******************************************************************************
 *  CancelPushButtonActivateCB()
 ******************************************************************************/
void CancelPushButtonActivateCB(w,tl,call_data)
Widget  w;
XmTextListWidget tl;
caddr_t call_data;
{
XtUnmanageChild(tl->textlist.text_list_fr);

/* Set the input focus to the text field */
/* Set the input focus to the text field */
XSetInputFocus(XtDisplay(tl),XtWindow(tl), RevertToParent, CurrentTime);
XmProcessTraversal(tl,XmTRAVERSE_CURRENT);

RepopulatePartialList(tl);
}
/******************************************************************************
 *  PushButtonActivateCB()
 ******************************************************************************/
void PushButtonActivateCB(w,tl,call_data)
Widget  w;
XmTextListWidget tl;
caddr_t call_data;

{
static Cursor   working_cursor = NULL;
static Window   window;
static Display *display;

RepopulatePartialList(tl);

/* Check for a load routine */
if (tl->textlist.loadRoutine != NULL)
   {
   display = XtDisplay(w);
   window = XtWindow(XtParent(tl));
   working_cursor = XCreateFontCursor(display, XC_watch);
   if (working_cursor)
      {           
      XDefineCursor(display, window, working_cursor);
      XFlush(display);
      }

   tl->textlist.loadRoutine();

   if (working_cursor)
      XUndefineCursor(display, window);
   }

if (ReduceListToMatchingItems(tl))
   {
   DisplayScrolledList(tl);
   }

else DisplayScrolledList(tl);
}
/******************************************************************************
 * ManualTextEntryActivateCB()
 ******************************************************************************/
void ManualTextEntryActivateCB(w,tl,call_data)
XmTextListWidget w;
XmTextListWidget tl;
caddr_t call_data;
{
char *str;

static Cursor   working_cursor = NULL;
static Window   window;
static Display *display;

/* Check for a load routine */
if (tl->textlist.loadRoutine != NULL)
   {
   display = XtDisplay(w);
   window = XtWindow(XtParent(tl));
   working_cursor = XCreateFontCursor(display, XC_watch);
   if (working_cursor)
      {           
      XDefineCursor(display, window, working_cursor);
      XFlush(display);
      }

   tl->textlist.loadRoutine();

   if (working_cursor)
      XUndefineCursor(display, window);
   }

str = XmTextFieldGetString(tl);

if (ReduceListToMatchingItems(tl))
   {
   DisplayScrolledList(tl);
   }

else if ((tl->textlist.no_new_entry) && (! XmTextListItemExists(tl,str)))
   {
   DisplayScrolledList(tl);
   }

XtFree(str);
}
/******************************************************************************
 * AlphabeticPosition()

******************************************************************************/
static int AlphabeticPosition(w,item)
Widget   w;
XmString item;

{
int i,n;
Arg args[10];
int item_count;
char *str1,*str2;
XmStringTable items;

/* Extract a copy of the string table */
n = 0;
XtSetArg(args[n],XmNitems,&items);n++;
XtSetArg(args[n],XmNitemCount,&item_count);n++;
XtGetValues(w,args,n);
 
/* Extract the character string to be inserted */
XmStringGetLtoR(item,XmSTRING_DEFAULT_CHARSET,&str1);

for (i=0; i<item_count; i++)
    {
    /* Extract the character string to be inserted */
    XmStringGetLtoR(items[i],XmSTRING_DEFAULT_CHARSET,&str2);

    /* Compare each item to the string to be inserted */
    if (strcmp(str1,str2) < 0)
       {
       return(i+1);
       }

    XtFree(str2);
    }
 
XtFree(str1);

/* Add to last position */
return(0);
}
/******************************************************************************
 * CreateTextEntryFRDContents()                     
 ******************************************************************************/
static void CreateTextEntryFRDContents(new)
XmTextListWidget new;

{
int n;
Arg args[10];

/* Create the form dialog which will house the scrolled list and cancel pb */
n=0;
XtSetArg(args[n],XmNx,0);n++;
XtSetArg(args[n],XmNy,0);n++;
XtSetArg(args[n],XmNshadowThickness,2);n++;
XtSetArg(args[n],XmNmwmDecorations,0L);n++;
/* XtSetArg(args[n],XmNdialogStyle,XmDIALOG_PRIMARY_APPLICATION_MODAL);n++;  */
XtSetArg(args[n],XmNdialogStyle,XmDIALOG_FULL_APPLICATION_MODAL);n++; 
new->textlist.text_list_fr = 
   XmCreateFormDialog(OriginalParent(new),"TextEntryListFRD",args,n);

/* Add an event handler to pop down Form Dialog when focus is lost */
/***********
 * Not working at this time 
 * XtAddEventHandler(new->textlist.text_list_fr,
 *                   FocusOut,
 *                   False,
 *                   (XtEventHandler)TextListHandleFocusOutEvent,
 *                   (XtPointer) new);
 ************/

/* Create the Scrolled List */
n=0;
XtSetArg(args[n],XmNleftAttachment,XmATTACH_FORM);n++;
XtSetArg(args[n],XmNrightAttachment,XmATTACH_FORM);n++;
XtSetArg(args[n],XmNtopAttachment,XmATTACH_FORM);n++;
XtSetArg(args[n],XmNleftOffset,2);n++;
XtSetArg(args[n],XmNrightOffset,2);n++;
XtSetArg(args[n],XmNtopOffset,2);n++;
XtSetArg(args[n],XmNvisibleItemCount,5);n++;
XtSetArg(args[n],XmNselectionPolicy,XmSINGLE_SELECT);n++;
new->textlist.text_list_sl=
   XmCreateScrolledList(new->textlist.text_list_fr,"TextEntrySL",args,n);
XtManageChild(new->textlist.text_list_sl);
XmAddTabGroup(new->textlist.text_list_sl);

/* Add the Scrolled List Selection CB */
XtAddCallback(new->textlist.text_list_sl,XmNsingleSelectionCallback,
              (XtCallbackProc) ListItemSelectedCB,(caddr_t)new);

n=0;
XtSetArg(args[n],XmNleftAttachment,XmATTACH_FORM);n++;
XtSetArg(args[n],XmNleftOffset,2);n++;
XtSetArg(args[n],XmNrightAttachment,XmATTACH_FORM);n++;
XtSetArg(args[n],XmNrightOffset,2);n++;
XtSetArg(args[n],XmNtopAttachment,XmATTACH_WIDGET);n++;
XtSetArg(args[n],XmNtopWidget,(Widget)new->textlist.text_list_sl); n++;
XtSetArg(args[n],XmNbottomAttachment,XmATTACH_FORM);n++;
XtSetArg(args[n],XmNbottomOffset,2);n++;
XtSetArg(args[n],XmNlabelString,XmStringCreate("Cancel",
         XmSTRING_DEFAULT_CHARSET));n++;
new->textlist.text_list_cancel_pb=
   XmCreatePushButton(new->textlist.text_list_fr, "CandListCancelPB", args,n);
XtManageChild(new->textlist.text_list_cancel_pb);
XmAddTabGroup(new->textlist.text_list_cancel_pb);

/*
 * Add the Cancel Push Button Callback that causes the Scrolled List to 
 * disappear. Pass the TextList widget in order to unmanage the Scrolled List 
 * widget.
 */
XtAddCallback(new->textlist.text_list_cancel_pb,XmNactivateCallback,
              (XtCallbackProc)CancelPushButtonActivateCB,(caddr_t)new);
}
/******************************************************************************
 * InitializeAsChildOfNonForm()                         
 ******************************************************************************/
static void InitializeAsChildOfNonForm(request,new)
XmTextListWidget request, new;

{
int n;
Arg args[10];
Position x,y,root_x,root_y;

/* Create the Push Button which spawns the scrolled list of choices */
n=0;
XtSetArg(args[n],XmNx,RT_X_COORD_OF_WIDGET(new));n++;
XtSetArg(args[n],XmNy,TOP_Y_COORD_OF_WIDGET(new));n++;
new->textlist.text_list_pb=(Widget)XmCreatePushButton(XtParent(new),">",args,n);

/*
 * Add the Push Button Callback that causes the Scrolled List to appear
 * Pass the TextList widget in order to manage the Scrolled List widget
 */
XtAddCallback(new->textlist.text_list_pb,XmNactivateCallback,
              (XtCallbackProc) PushButtonActivateCB,(caddr_t)new);

/* Make the Push Button immediately visible */
XtManageChild(new->textlist.text_list_pb);

/* Create the Form Dialog that houses the Scrolled List Cancel Push Button */
CreateTextEntryFRDContents(new);
}
/******************************************************************************
 * InitializeAsChildOfForm()                         
 ******************************************************************************/
static void InitializeAsChildOfForm(request,new)
XmTextListWidget request, new;

{
int n;
Arg args[15];
Position x,y,root_x,root_y;
Display *dpy;
Pixmap   ptextlist;

dpy = XtDisplay(XtParent(XtParent(new)));

/* Create the Push Button which spawns the scrolled list of choices */
n=0;
XtSetArg(args[n],XmNleftAttachment,XmATTACH_WIDGET);n++;
XtSetArg(args[n],XmNleftWidget,(Widget)new);n++;
XtSetArg(args[n],XmNtopAttachment,XmATTACH_OPPOSITE_WIDGET);n++;
XtSetArg(args[n],XmNtopWidget,(Widget)new);n++;
XtSetArg(args[n],XmNtopOffset,2);n++;
XtSetArg(args[n],XmNbottomAttachment,XmATTACH_OPPOSITE_WIDGET);n++;
XtSetArg(args[n],XmNbottomWidget,(Widget)new);n++;
XtSetArg(args[n],XmNbottomOffset,2);n++;
new->textlist.text_list_pb =
   XmCreatePushButton(XtParent(new),">",args,n);

/* Create the PushButton Pixmap, Must Free the Pixmap space later */
/****
ptextlist=XCreatePixmapFromBitmapData(dpy,XtWindow(new->textlist.text_list_pb),
          textlist_bits,textlist_width,textlist_height,
          WhitePixel(dpy,0),BlackPixel(dpy,0),DefaultDepth(dpy,0));
 ****/

/****
n=0;
XtSetArg(args[n],XmNlabelType,XmPIXMAP);n++;
XtSetArg(args[n],XmNlabelPixmap,ptextlist);n++;
XtSetArg(args[n],XmNlabelInsensitivePixmap,ptextlist);n++;
XtSetValues(new->textlist.text_list_pb,args,n);
 *****/

/*
 * Add the Push Button Callback that causes the Scrolled List to appear
 * Pass the TextList widget in order to manage the Scrolled List widget
 */
XtAddCallback(new->textlist.text_list_pb,XmNactivateCallback,
              (XtCallbackProc) PushButtonActivateCB,(caddr_t)new); 

/* Make the Push Button immediately visible */
XtManageChild(new->textlist.text_list_pb);

/* Create the Form Dialog that houses the Scrolled List Cancel Push Button */
CreateTextEntryFRDContents(new);

/* Initialize the load routine callback to NULL */
new->textlist.loadRoutine = NULL;
}
/******************************************************************************
 * Initialize()                                    
 ******************************************************************************/
static void Initialize(request,new)
XmTextListWidget request, new;
{
int n;
Arg args[10];

/* Turn off typed entry to text field if necessary */
/**** 
n=0; 
if (new->textlist.no_new_entry) 
   { 
   XtSetArg(args[n],XmNeditable,0);n++; 
   } 
else 
   { 
   XtSetArg(args[n],XmNeditable,1);n++; 
   }
XtSetValues(new,args,n); 
 ****/ 

if (XmIsForm(XtParent(new)))
   {
   InitializeAsChildOfForm(request,new);
   }
else
   {
   InitializeAsChildOfNonForm(request,new);
   }

XtAddCallback((Widget)new,XmNactivateCallback,
              (XtCallbackProc) ManualTextEntryActivateCB,(caddr_t)new);
}   
/******************************************************************************
 * Destroy()
 *	Free text list's private alocatted structures.
 ******************************************************************************/
static void Destroy(tw)
	XmTextListWidget tw;
{
}
/******************************************************************************
 * XmCreateTextList()
 *   Creates an instance of a textlist and returns the widget id.
 ******************************************************************************/
Widget XmCreateTextList(parent, name, arglist, argCount)
Widget	parent;
char *	name;
Arg *	arglist;
int	argCount;

{
return XtCreateWidget(name,
		      xmTextListWidgetClass,
		      parent,
		      arglist,
		      argCount);
}
/******************************************************************************
 * MrmRegisterTickerClass()
 *   allows ticker widgets to be used from UIL.
 ******************************************************************************/
Cardinal MrmRegisterTextListClass()
{
return	MrmRegisterClass(MrmwcUnknown,
			 "XmTextList",
			 "XmCreateTextList",
			 XmCreateTextList,
			 xmTextListWidgetClass);
}
/******************************************************************************
 * XmTextListItemExists()
 ******************************************************************************/
Boolean XmTextListItemExists(w,str)
XmTextListWidget  w;
char             *str;

{
XmString xmstr;

xmstr = XmStringCreateLtoR(str, XmSTRING_DEFAULT_CHARSET);

/* Must Exit if there are memory problems */
if (! xmstr)
   exit(0);

if (XmListItemExists(w->textlist.text_list_sl,xmstr))
   {
   XmStringFree(xmstr);
   return(True);
   }
else
   {
   XmStringFree(xmstr);
   return(False);
   }
}
/******************************************************************************
 * XmTextListDeleteItem()
 ******************************************************************************/
void XmTextListDeleteItem(w,item)
XmTextListWidget w;
XmString         item;

{
XmListDeleteItem(w->textlist.text_list_sl,item);
}
/******************************************************************************
 * XmTextListAddItem()
 ******************************************************************************/
void XmTextListAddItem(w,item,position)
XmTextListWidget w;
XmString         item;
int              position;

{
/* check alphabetize field (boolean) for insertion order */
if (w->textlist.text_list_alphabetize_lists)
   {
   XmListAddItem(w->textlist.text_list_sl,item,
                 AlphabeticPosition(w->textlist.text_list_sl,item));   
   }
else
   {
   /* Add at end of list */
   XmListAddItem(w->textlist.text_list_sl,item,position);
   }

/* Make the last item added visible at the bottom */
XmListSetBottomItem(w,item);
}
/******************************************************************************
 * XmTextListAddItems()
 ******************************************************************************/
void XmTextListAddItems(w,items,item_count,position)
XmTextListWidget w;
XmString *items;
int item_count;
int position;

{
XmListAddItems(w->textlist.text_list_sl,items,item_count,position);
}
/******************************************************************************
 * XmTextListDeleteAllItems()
 ******************************************************************************/
void XmTextListDeleteAllItems(w)
XmTextListWidget w;

{
XmListDeleteAllItems(w->textlist.text_list_sl);
}
/******************************************************************************
 * XmTextListAddTabGroup()
 ******************************************************************************/
void XmTextListAddTabGroup(w)
XmTextListWidget w;

{
XmAddTabGroup(w);
XmAddTabGroup(w->textlist.text_list_pb);
}
/******************************************************************************
 * XmTextListSetSensitive()
 ******************************************************************************/
void XmTextListSetSensitive(w,value)
XmTextListWidget w;
int              value;

{
if (value == FALSE)
   {
   XtSetSensitive((Widget) w,FALSE);
   XtSetSensitive(w->textlist.text_list_pb,FALSE);
   }
else if (value == TRUE)
   {
   XtSetSensitive((Widget) w,TRUE);
   XtSetSensitive(w->textlist.text_list_pb,TRUE);
   }
}
/******************************************************************************
 * XmTextListIsSearchString()
 ******************************************************************************/
Boolean XmTextListIsSearchString(w)
XmTextListWidget w;

{
char *str;
char *str2;

str = XmTextFieldGetString(w);

/* Check for metacharacter from back of string (eg "Smi*") */
if ((str2 = (rindex(str,'*'))) != NULL)
   {
   XtFree(str);
   return(True);
   }
XtFree(str);
return(False);
}
/******************************************************************************
 * XmTextListSetLoadRoutine()
 ******************************************************************************/
void XmTextListSetLoadRoutine(w,loadRoutine)
XmTextListWidget w;
void (*loadRoutine)();

{
w->textlist.loadRoutine = loadRoutine;
}
