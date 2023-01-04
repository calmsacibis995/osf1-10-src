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
static char *rcsid = "@(#)$RCSfile: SelList.c,v $ $Revision: 1.1.2.6 $ (DEC) $Date: 1993/12/20 21:30:31 $";
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
******************************************************************************/
/*
 * Include files & Static Routine Definitions
 */

#include <stdio.h>

#include <Xm/ListP.h>
#include <Xm/XmP.h>
#include "SelListP.h"
#include <Mrm/MrmPublic.h>
#include <Xm/List.h>
#include <Xm/Form.h>
#include <Xm/Label.h>

#define ALPHABETIC_ORDER -1

static void	Initialize();
static void	Destroy();
static void     CvtToExternalPos();
static XmImportOperator CvtToInternalPos();

void XmSelectListAddItem();
void XmSelectListDeleteItem();
void XmSelectListAddItems();
void XmSelectListGetItems();
void XmAvailableListAddItem();
void XmAvailableListDeleteItem();
void XmAvailableListAddItems();

static int  AlphabeticPosition();
static void AddItemToSelList();

/*************************************<->*************************************
 *
 *
 *   Description:  resource list for class: SelectList
 *   -----------
 *
 *   To get the full set of default settings, examine the resource lists
 *   of superclasses of this class.
 *
 *************************************<->***********************************/

static XtResource resources[] = 
{
   {
     XmNselectListScrolledList,
     XmCWidget,
     XmRWidget,
     sizeof(Widget),
     XtOffset(XmSelectListWidget,selectlist.selected_sl),
     XmRImmediate,
     (caddr_t)10
   },

   {
     XmNavailableListScrolledList,
     XmCWidget,
     XmRWidget,
     sizeof(Widget),
     XtOffset(XmSelectListWidget,selectlist.available_sl),
     XmRImmediate,
     (caddr_t)200
   },

   {
     XmNselectListLabel,
     XmCWidget,
     XmRWidget,
     sizeof(Widget),
     XtOffset(XmSelectListWidget,selectlist.selected_lbl),
     XmRImmediate,
     (caddr_t)350
   },

   {
     XmNavailableListLabel,
     XmCWidget,
     XmRWidget,
     sizeof(Widget),
     XtOffset(XmSelectListWidget,selectlist.available_lbl),
     XmRImmediate,
     (caddr_t)500
   },

   {
     XmNselectListItems,
     XmCXmString,
     XmRXmStringTable,
     sizeof(XmStringTable),
     XtOffset(XmSelectListWidget,selectlist.select_list_items),
     XmRImmediate,
     (caddr_t)650
   },

   {
     XmNalphabetizeLists,
     XmCBoolean,
     XmRBoolean,
     sizeof(Boolean),
     XtOffset(XmSelectListWidget,selectlist.alphabetize_lists),
     XmRImmediate,
     (caddr_t)750
   },

   {
     XmNselectListLabelString,
     XmCString,
     XmRString,
     sizeof(String),
     XtOffset(XmSelectListWidget,selectlist.selected_lbl_string),
     XmRImmediate,
     (caddr_t)850
   },

   {
     XmNavailableListLabelString,
     XmCString,
     XmRString,
     sizeof(String),
     XtOffset(XmSelectListWidget,selectlist.available_lbl_string),
     XmRImmediate,
     (caddr_t)950
   },

   {
     XmNlistChanged,
     XmCBoolean,
     XmRBoolean,
     sizeof(Boolean),
     XtOffset(XmSelectListWidget,selectlist.list_changed),
     XmRImmediate,
     (caddr_t)1050
   },
};

/****************
 *
 * Resolution independent resources
 *
 ****************/

static XmSyntheticResource get_resources[] =
{
   { XmNlistSpacing,
     sizeof (Dimension),
     XtOffset (XmListWidget, list.ItemSpacing),
     _XmFromVerticalPixels,
     _XmToVerticalPixels },

   { XmNlistMarginWidth,
     sizeof (Dimension),
     XtOffset (XmListWidget, list.margin_width),
     _XmFromHorizontalPixels,
     _XmToHorizontalPixels },

   { XmNlistMarginHeight,
     sizeof (Dimension),
     XtOffset (XmListWidget, list.margin_height),
     _XmFromVerticalPixels,
     _XmToVerticalPixels },

   { XmNtopItemPosition,
     sizeof (int),
     XtOffset (XmListWidget, list.top_position),
     CvtToExternalPos,
     CvtToInternalPos },
};

/*************************************<->*************************************
 *
 *
 *   Description:  global class record for instances of class: SelectList
 *   -----------
 *
 *   Defines default field settings for this class record.
 *
 *************************************<->***********************************/

externaldef(xmselectlistclassrec) 
	XmSelectListClassRec xmSelectListClassRec = {
   {
    /* superclass	  */	(WidgetClass)&xmListClassRec,
    /* class_name	  */	"XmSelectList",
    /* widget_size	  */	sizeof(XmSelectListRec),
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
    /* compress_motion	  */	FALSE,
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
    /* accept_focus       */    NULL,
    /* version            */	XtVersion,
    /* callback_private   */    NULL,
    /* tm_table           */    NULL,
    /* query_geometry     */	XtInheritQueryGeometry, 
    /* display_accelerator */   NULL,
    /* extension          */    NULL,
   },

   {                            /* primitive_class fields       */
      (XtWidgetProc)_XtInherit, /* Primitive border_highlight   */
      (XtWidgetProc)_XtInherit, /* Primitive border_unhighlight */
      NULL,                     /* translations                 */
      NULL,                     /* arm_and_activate             */
      get_resources,            /* get resources                */
      XtNumber(get_resources),  /* num get_resources            */
      NULL,                     /* extension                    */
   },

};

externaldef(xmselectlistwidgetclass)
   WidgetClass xmSelectListWidgetClass = (WidgetClass)&xmSelectListClassRec;

/************************************************************************
 *
 * Conversion routines for XmNtopItemPostion.  Necessary because the
 *
 * outside world is 1-based, and the inside world is 0-based.  Sigh.
 *
 ************************************************************************/
/* ARGSUSED */
static void CvtToExternalPos(lw, offset, value)
register XmListWidget lw;
int      offset;
XtArgVal * value;
{
    (*value) = (XtArgVal) lw->list.top_position + 1;
}

/* ARGSUSED */
static XmImportOperator CvtToInternalPos(lw, offset, value)
register XmListWidget lw;
int      offset;
XtArgVal * value;
{
    (*value)--;
    return (XmSYNTHETIC_LOAD);
}
/******************************************************************************
 * AvailableItemCB()
 ******************************************************************************/
void AvailableItemCB(w,sl,call_data)
Widget  w;
XmSelectListWidget sl;
XmListCallbackStruct *call_data;
{

AddItemToSelList(sl->selectlist.selected_sl,call_data->item,
                 sl->selectlist.alphabetize_lists);
XmListDeleteItem(sl->selectlist.available_sl,call_data->item);

/* Set flag to indicate a change has taken place */
sl->selectlist.list_changed = 1;
}
/******************************************************************************
 * SelectedItemCB()
 ******************************************************************************/
void SelectedItemCB(w,sl,call_data)
Widget  w;
XmSelectListWidget sl;
XmListCallbackStruct *call_data;
{

AddItemToSelList(sl->selectlist.available_sl,call_data->item,
                 sl->selectlist.alphabetize_lists);
XmListDeleteItem(sl->selectlist.selected_sl,call_data->item);

/* Set flag to indicate a change has taken place */
sl->selectlist.list_changed = 1;
}
/******************************************************************************
 * Initialize()                         
 ******************************************************************************/
static void Initialize(request,new)
XmSelectListWidget request, new;

{
int n;
Arg args[10];

/* assign the available scrolled list field (redundant info) */
new->selectlist.selected_sl = (Widget)new;

/*
 * Add the Callback that causes the selected items to move to the
 * "Available" scrolled list of items.
 */
XtAddCallback(new->selectlist.selected_sl,XmNsingleSelectionCallback,
              (XtCallbackProc) SelectedItemCB,(caddr_t)new);

/* Create theSelected List of Items Label */
n=0;
XtSetArg(args[n],XmNleftAttachment,XmATTACH_OPPOSITE_WIDGET);n++;
XtSetArg(args[n],XmNleftWidget,(Widget)XtParent(new));n++;
XtSetArg(args[n],XmNrightAttachment,XmATTACH_OPPOSITE_WIDGET);n++;
XtSetArg(args[n],XmNrightWidget,(Widget)XtParent(new));n++;
XtSetArg(args[n],XmNbottomAttachment,XmATTACH_WIDGET);n++;
XtSetArg(args[n],XmNbottomWidget,(Widget)XtParent(new));n++;
XtSetArg(args[n],XmNtopAttachment,XmATTACH_FORM);n++;
if (new->selectlist.selected_lbl_string == NULL)
   new->selectlist.selected_lbl = XmCreateLabel(XtParent(XtParent(new)),
                                  "Selected",args,n);
else 
   new->selectlist.selected_lbl = XmCreateLabel(XtParent(XtParent(new)),
                                  new->selectlist.selected_lbl_string,args,n);
XtManageChild(new->selectlist.selected_lbl);

/* Create the Scrolled List of Available Items */
n=0;
XtSetArg(args[n],XmNleftAttachment,XmATTACH_FORM);n++;
XtSetArg(args[n],XmNrightAttachment,XmATTACH_WIDGET);n++;
XtSetArg(args[n],XmNrightWidget,(Widget)XtParent(new));n++;
XtSetArg(args[n],XmNbottomAttachment,XmATTACH_OPPOSITE_WIDGET);n++;
XtSetArg(args[n],XmNbottomWidget,(Widget)XtParent(new));n++;
XtSetArg(args[n],XmNvisibleItemCount,new->list.visibleItemCount);n++;
XtSetArg(args[n],XmNscrollBarDisplayPolicy,XmSTATIC);n++;
XtSetArg(args[n],XmNselectionPolicy,XmSINGLE_SELECT);n++;
new->selectlist.available_sl = XmCreateScrolledList(XtParent(XtParent(new)),
                                                   "AvailableSL",args,n);

/*
 * Add the Callback that causes the selected items to move to the
 * "Selected" scrolled list of items.
 */
XtAddCallback(new->selectlist.available_sl,XmNsingleSelectionCallback,
              (XtCallbackProc) AvailableItemCB,(caddr_t)new);
XtManageChild(new->selectlist.available_sl);

/* Create the Available List of Items Label */
n=0;
XtSetArg(args[n],XmNleftAttachment,XmATTACH_OPPOSITE_WIDGET);n++;
XtSetArg(args[n],XmNleftWidget,(Widget)new->selectlist.available_sl);n++;
XtSetArg(args[n],XmNrightAttachment,XmATTACH_OPPOSITE_WIDGET);n++;
XtSetArg(args[n],XmNrightWidget,(Widget)new->selectlist.available_sl);n++;
XtSetArg(args[n],XmNbottomAttachment,XmATTACH_WIDGET);n++;
XtSetArg(args[n],XmNbottomWidget,(Widget)new->selectlist.available_sl);n++;
XtSetArg(args[n],XmNtopAttachment,XmATTACH_FORM);n++;
if (new->selectlist.available_lbl_string == NULL)
   new->selectlist.available_lbl = XmCreateLabel(XtParent(XtParent(new)),
                                  "Available",args,n);
else 
   new->selectlist.available_lbl = XmCreateLabel(XtParent(XtParent(new)),
                                  new->selectlist.available_lbl_string,args,n);
XtManageChild(new->selectlist.available_lbl);
}
/******************************************************************************
 * Destroy()
 *	Free text list's private alocatted structures.
 ******************************************************************************/
static void Destroy(tw)
	XmSelectListWidget tw;
{
}
/******************************************************************************
 * XmCreateSelectList()
 ******************************************************************************/
Widget XmCreateSelectList(parent, name, args, argCount)
Widget	parent;
char   *name;
Arg    *args;
int     argCount;

{
Widget sw, lw;
int i = 0;
char *s;
ArgList Args;

if (name == NULL)
   s = name;
else
   {
   s = XtMalloc(strlen(name) + 3);     /* Name + NULL + "SW" */
   strcpy(s, name);
   strcat(s, "SW");
   };

Args = (ArgList) XtCalloc(argCount+4, sizeof(Arg));
for (i = 0; i < argCount; i++)
    {
    Args[i].name = args[i].name;
    Args[i].value = args[i].value;
    }

XtSetArg (Args[i], XmNscrollingPolicy, (XtArgVal )XmAPPLICATION_DEFINED); i++;
XtSetArg (Args[i], XmNvisualPolicy, (XtArgVal )XmVARIABLE); i++;
XtSetArg (Args[i], XmNscrollBarDisplayPolicy, (XtArgVal )XmSTATIC); i++;
XtSetArg (Args[i], XmNshadowThickness, (XtArgVal ) 0); i++;
sw = XtCreateManagedWidget(s, xmScrolledWindowWidgetClass, parent,
                           (ArgList)Args, i);
if (s != NULL)
   XtFree(s);
XtFree((char *)Args);

lw = XtCreateWidget( name, xmSelectListWidgetClass, sw, args, argCount);
XtAddCallback (lw, XmNdestroyCallback, _XmDestroyParentCallback, NULL);

return (lw);
}

/*
===============================================================================
=
= P R I V A T E   F U N C T I O N S
=
===============================================================================
*/

/******************************************************************************
 * AlphabetizeStringTable()
 ******************************************************************************/
static void AlphabetizeStringTable(items,item_count)
XmString **items;
int        item_count;

{
int i,j;
char *str1,*str2;
XmString xmstr;

for (j=0; j<item_count-1; j++)
    {
    for (i=0; i<item_count-1; i++)
        {
        XmStringGetLtoR((*items)[i],XmSTRING_DEFAULT_CHARSET,&str1);

        XmStringGetLtoR((*items)[i+1],XmSTRING_DEFAULT_CHARSET,&str2);

        /* Compare each item to the string to be inserted */
        if (strcmp(str1,str2) > 0)
           {
           xmstr = (*items)[i];
           (*items)[i] = (*items)[i+1];
           (*items)[i+1] = xmstr;
           }
        XtFree(str1);

        XtFree(str2);
        }
    }
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
 * AddItemToSelList()           
 ******************************************************************************/
static void AddItemToSelList(w,item,alphabetize_flag)
Widget   w;
XmString item;
Boolean  alphabetize_flag;

{
/* check alphabetize field (boolean) for insertion order */
if (alphabetize_flag)
   {
   XmListAddItem(w,item,AlphabeticPosition(w,item));
   }
else
   {
   /* Add at end of list */
   XmListAddItem(w,item,0);
   }

/* Make the last item added visible at the bottom */
XmListSetBottomItem(w,item);
}

/*
===============================================================================
=
= P U B L I C   F U N C T I O N S
=
===============================================================================
*/

/******************************************************************************
 * MrmRegisterTickerClass()
 *   allows ticker widgets to be used from UIL.
 ******************************************************************************/
Cardinal MrmRegisterSelectListClass()
{
return	MrmRegisterClass(MrmwcUnknown,
			 "XmSelectList",
			 "XmCreateSelectList",
			 XmCreateSelectList,
			 xmSelectListWidgetClass);
}
/******************************************************************************
 * XmSelectListAddItem()
 ******************************************************************************/
void XmSelectListAddItem(w,item,position)
XmSelectListWidget w;
XmString item;
int position;

{
AddItemToSelList(w->selectlist.selected_sl,item,
                 w->selectlist.alphabetize_lists);

/* Set flag to indicate a change has taken place */
w->selectlist.list_changed = 1;
}
/******************************************************************************
 * XmSelectListDeleteItem()
 ******************************************************************************/
void XmSelectListDeleteItem(w,item)
XmSelectListWidget w;
XmString item;

{
XmListDeleteItem(w->selectlist.selected_sl,item);

/* Set flag to indicate a change has taken place */
w->selectlist.list_changed = 1;
}
/******************************************************************************
 * XmSelectListDeleteAllItems()
 ******************************************************************************/
void XmSelectListDeleteAllItems(w)
XmSelectListWidget w;

{
XmListDeleteAllItems(w->selectlist.selected_sl);

/* Set flag to indicate a change has taken place */
w->selectlist.list_changed = 1;
}
/******************************************************************************
 * XSelectListAddItems()
 ******************************************************************************/
void XmSelectListAddItems(w,items,item_count,position)
XmSelectListWidget w;
XmString *items;
int item_count;
int position;

{
if (item_count == 0)
   return;

/* check alphabetize field (boolean) for insertion order */
if (w->selectlist.alphabetize_lists)
   {
   AlphabetizeStringTable(&items,item_count);
   XmListAddItems(w->selectlist.selected_sl,items,item_count,0);
   }
else
   {
   /* Add in given order at end of list */
   XmListAddItems(w->selectlist.selected_sl,items,item_count,position);
   }

/* Set flag to indicate a change has taken place */
w->selectlist.list_changed = 1;
}
/******************************************************************************
 * XmSelectListGetItems()
 ******************************************************************************/
void XmSelectListGetItems(w,name_lst,name_count)
XmSelectListWidget    w;
char               ***name_lst;
int                  *name_count;

{
int            i,n;
Arg            args[10];
char          *str;
XmStringTable  items;

/* Extract a copy of the string table */
n = 0;
XtSetArg(args[n],XmNitems,&items);n++;
XtSetArg(args[n],XmNitemCount,name_count);n++;
XtGetValues(w->selectlist.selected_sl,args,n);

/* Malloc the Character String Array, Application responsible for freeing */
*name_lst = (char **)XtMalloc(sizeof(char *) * (*name_count + 1));

/* Extract the character string to be inserted */
for (i=0; i<*name_count; i++)
    {
    XmStringGetLtoR(items[i],XmSTRING_DEFAULT_CHARSET,&str);
    (*name_lst)[i]=(char *)str;
    }
}
/******************************************************************************
 * XmAvailableListAddItem()
 ******************************************************************************/
void XmAvailableListAddItem(w,item,position)
XmSelectListWidget w;
XmString item;
int position;

{
AddItemToSelList(w->selectlist.available_sl,item,
                 w->selectlist.alphabetize_lists);

/* Set flag to indicate a change has taken place */
w->selectlist.list_changed = 1;
}
/******************************************************************************
 * XmAvailableListDeleteItem()
 ******************************************************************************/
void XmAvailableListDeleteItem(w,item)
XmSelectListWidget w;
XmString item;

{
XmListDeleteItem(w->selectlist.available_sl,item);

/* Set flag to indicate a change has taken place */
w->selectlist.list_changed = 1;
}
/******************************************************************************
 * XmAvailableListDeleteAllItems()
 ******************************************************************************/
void XmAvailableListDeleteAllItems(w)
XmSelectListWidget w;

{
XmListDeleteAllItems(w->selectlist.available_sl);

/* Set flag to indicate a change has taken place */
w->selectlist.list_changed = 1;
}
/******************************************************************************
 * XmAvailableListAddItems()
 ******************************************************************************/
void XmAvailableListAddItems(w,items,item_count,position)
XmSelectListWidget w;
XmString *items;
int item_count;
int position;

{
if (item_count == 0)
   return;

/* check alphabetize field (boolean) for insertion order */
if (w->selectlist.alphabetize_lists)
   {
   AlphabetizeStringTable(&items,item_count);
   XmListAddItems(w->selectlist.available_sl,items,item_count,0);
   }
else
   {
   /* Add in given order at end of list */
   XmListAddItems(w->selectlist.available_sl,items,item_count,position);
   }

/* Set flag to indicate a change has taken place */
w->selectlist.list_changed = 1;
}
/******************************************************************************
 * XmSelectListItemExists()
 ******************************************************************************/
Boolean XmSelectListItemExists(w,str)
XmSelectListWidget  w;
char               *str;

{
XmString xmstr;

xmstr = XmStringCreateLtoR(str, XmSTRING_DEFAULT_CHARSET);

/* Must Exit if there are memory problems */
if (! xmstr)
   exit(0);

if (XmListItemExists(w->selectlist.selected_sl,xmstr))
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
 * XmAvailableListItemExists()
 ******************************************************************************/
Boolean XmAvailableListItemExists(w,str)
XmSelectListWidget  w;
char               *str;

{
XmString xmstr;

xmstr = XmStringCreateLtoR(str, XmSTRING_DEFAULT_CHARSET);

/* Must Exit if there are memory problems */
if (! xmstr)
   exit(0);

if (XmListItemExists(w->selectlist.available_sl,xmstr))
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
 * XmSetAllItemsAvailable()
 ******************************************************************************/
void XmSetAllItemsAvailable(w)
XmSelectListWidget w;

{
int            i,n;
int            count;
Arg            args[10];
XmStringTable  items;

/* Extract a copy of the selected string table */
n = 0;
XtSetArg(args[n],XmNitems,&items);n++;
XtSetArg(args[n],XmNitemCount,&count);n++;
XtGetValues(w->selectlist.selected_sl,args,n);

/* Add items to Available List */

/* check alphabetize field (boolean) for insertion order */
if (w->selectlist.alphabetize_lists)
   {
   for (i=0; i<count; i++)
       AddItemToSelList(w->selectlist.available_sl,items[i],
                        w->selectlist.alphabetize_lists);
   }
else
   {
   XmAvailableListAddItems(w,items,count,0);
   }

/* Delete All Selected Items */
XmSelectListDeleteAllItems(w);

/* Set flag to indicate a change has taken place */
w->selectlist.list_changed = 1;
}
/******************************************************************************
 * XmSetAllItemsSelected()
 ******************************************************************************/
void XmSetAllItemsSelected(w)
XmSelectListWidget w;

{
int            i,n;
int            count;
Arg            args[10];
XmStringTable  items;

/* Extract a copy of the available string table */
n = 0;
XtSetArg(args[n],XmNitems,&items);n++;
XtSetArg(args[n],XmNitemCount,&count);n++;
XtGetValues(w->selectlist.available_sl,args,n);

/* Add items to Selected List */

/* check alphabetize field (boolean) for insertion order */
if (w->selectlist.alphabetize_lists)
   {
   for (i=0; i<count; i++)
       AddItemToSelList(w->selectlist.selected_sl,items[i],
                        w->selectlist.alphabetize_lists);
   }
else
   {
   XmSelectListAddItems(w,items,count,0);
   }

/* Delete All Selected Items */
XmAvailableListDeleteAllItems(w);

/* Set flag to indicate a change has taken place */
w->selectlist.list_changed = 1;
}
/******************************************************************************
 * XmSelListAddTabGroup()
 ******************************************************************************/
void XmSelListAddTabGroup(w)
XmSelectListWidget w;

{
XmAddTabGroup(w->selectlist.available_sl);
XmAddTabGroup(w->selectlist.selected_sl);
}
/******************************************************************************
 * XmSelListInitChangeFlag()
 ******************************************************************************/
void XmSelListInitChangeFlag(w)
XmSelectListWidget w;

{
w->selectlist.list_changed = 0;
}
/******************************************************************************
 * XmSelListMadeChanges()
 ******************************************************************************/
Boolean XmSelListMadeChanges(w)
XmSelectListWidget w;

{
if (w->selectlist.list_changed == 1)
   return(True);
return(False);
}
/******************************************************************************
 * XmSelListAddAvailableCallback()
 ******************************************************************************/
void XmSelListAddAvailableCallback(w,CB,data)
XmSelectListWidget w;
void               (*CB)();
XtPointer          data;

{
XtAddCallback(w->selectlist.available_sl,XmNsingleSelectionCallback,CB,data);
}
/******************************************************************************
 * XmSelListAddSelectedCallback()
 ******************************************************************************/
void XmSelListAddSelectedCallback(w,CB,data)
XmSelectListWidget w;
void               (*CB)();
XtPointer          data;

{
XtAddCallback(w->selectlist.selected_sl,XmNsingleSelectionCallback,CB,data);
}
