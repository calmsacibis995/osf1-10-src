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
static char *rcsid = "@(#)$RCSfile: SListTB.c,v $ $Revision: 1.1.2.4 $ (DEC) $Date: 1993/12/20 21:30:28 $";
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
#include "SListTBP.h"
#include <Mrm/MrmPublic.h>
#include <Xm/List.h>
#include <Xm/Form.h>
#include <Xm/Label.h>
#include <Xm/RowColumn.h>
#include <Xm/DrawnB.h>
#include <Xm/ToggleB.h>

#define ALPHABETIC_ORDER -1

static void	Initialize();
static void	Destroy();
static void     CvtToExternalPos();
static XmImportOperator CvtToInternalPos();

void XmSelectListTBAddItem();
void XmSelectListTBDeleteItem();
void XmSelectListTBAddItems();
void XmSelectListTBGetItems();
void XmAvailableListTBDeleteItem();

static int  AlphabeticPosition();
static void AddItemToSelListTB();
static void AddItemsToSelListTB();
static void UnManageSelectedListTBWidget();
static void UnManageSelectedListTBWidgets();
static void ManageSelectedListTBWidget();
static void ManageSelectedListTBWidgets();

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
     XmNselectListTBScrolledList,
     XmCWidget,
     XmRWidget,
     sizeof(Widget),
     XtOffset(XmSelectListTBWidget,selectlisttb.selected_sw),
     XmRImmediate,
     (caddr_t)10
   },

   {
     XmNavailableListTBScrolledWindow,
     XmCWidget,
     XmRWidget,
     sizeof(Widget),
     XtOffset(XmSelectListTBWidget,selectlisttb.available_sl),
     XmRImmediate,
     (caddr_t)200
   },

   {
     XmNavailableListTBRowColumn,
     XmCWidget,
     XmRWidget,
     sizeof(Widget),
     XtOffset(XmSelectListTBWidget,selectlisttb.selected_rc),
     XmRImmediate,
     (caddr_t)200
   },

   {
     XmNselectListTBLabel,
     XmCWidget,
     XmRWidget,
     sizeof(Widget),
     XtOffset(XmSelectListTBWidget,selectlisttb.selected_lbl),
     XmRImmediate,
     (caddr_t)350
   },

   {
     XmNselectListTBLabel,
     XmCWidget,
     XmRWidget,
     sizeof(Widget),
     XtOffset(XmSelectListTBWidget,selectlisttb.selectedon_lbl),
     XmRImmediate,
     (caddr_t)400
   },

   {
     XmNselectListTBLabel,
     XmCWidget,
     XmRWidget,
     sizeof(Widget),
     XtOffset(XmSelectListTBWidget,selectlisttb.selectedoff_lbl),
     XmRImmediate,
     (caddr_t)450
   },

   {
     XmNavailableListTBLabel,
     XmCWidget,
     XmRWidget,
     sizeof(Widget),
     XtOffset(XmSelectListTBWidget,selectlisttb.available_lbl),
     XmRImmediate,
     (caddr_t)500
   },

   {
     XmNselectListTBItems,
     XmCXmString,
     XmRXmStringTable,
     sizeof(XmStringTable),
     XtOffset(XmSelectListTBWidget,selectlisttb.select_list_items),
     XmRImmediate,
     (caddr_t)650
   },

   {
     XmNalphabetizeListTBs,
     XmCBoolean,
     XmRBoolean,
     sizeof(Boolean),
     XtOffset(XmSelectListTBWidget,selectlisttb.alphabetize_lists),
     XmRImmediate,
     (caddr_t)750
   },

   {
     XmNselectListTBLabelString,
     XmCString,
     XmRString,
     sizeof(String),
     XtOffset(XmSelectListTBWidget,selectlisttb.selected_lbl_string),
     XmRImmediate,
     (caddr_t)850
   },

   {
     XmNselectListTBOnLabelString,
     XmCString,
     XmRString,
     sizeof(String),
     XtOffset(XmSelectListTBWidget,selectlisttb.selectedon_lbl_string),
     XmRImmediate,
     (caddr_t)900
   },

   {
     XmNselectListTBOffLabelString,
     XmCString,
     XmRString,
     sizeof(String),
     XtOffset(XmSelectListTBWidget,selectlisttb.selectedoff_lbl_string),
     XmRImmediate,
     (caddr_t)950
   },

   {
     XmNavailableListTBLabelString,
     XmCString,
     XmRString,
     sizeof(String),
     XtOffset(XmSelectListTBWidget,selectlisttb.available_lbl_string),
     XmRImmediate,
     (caddr_t)1000
   },

   {
     XmNlistTBChanged,
     XmCBoolean,
     XmRBoolean,
     sizeof(Boolean),
     XtOffset(XmSelectListTBWidget,selectlisttb.list_changed),
     XmRImmediate,
     (caddr_t)1100
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

externaldef(xmselectlisttbclassrec) 
	XmSelectListTBClassRec xmSelectListTBClassRec = {
   {
    /* superclass	  */	(WidgetClass)&xmListClassRec,
    /* class_name	  */	"XmSelectList",
    /* widget_size	  */	sizeof(XmSelectListTBRec),
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

externaldef(xmselectlisttbwidgetclass)
   WidgetClass xmSelectListTBWidgetClass = (WidgetClass)&xmSelectListTBClassRec;

#define NOT_FOUND -1

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
 * ToggleButtonTwiddleCB()
 ******************************************************************************/
static void ToggleButtonTwiddleCB(w,sltb,call_data)
Widget  w;
XmSelectListTBWidget sltb;
XmListCallbackStruct *call_data;

{
/* Set flag to indicate a change has taken place */
sltb->selectlisttb.list_changed = 1;
}
/******************************************************************************
 * AvailableItemCB()
 ******************************************************************************/
static void AvailableItemCB(w,sltb,call_data)
Widget  w;
XmSelectListTBWidget sltb;
XmListCallbackStruct *call_data;
{

ManageSelectedListTBWidget(sltb,call_data->item);

XmListDeleteItem(sltb->selectlisttb.available_sl,call_data->item);

/* Set flag to indicate a change has taken place */
sltb->selectlisttb.list_changed = 1;
}
/******************************************************************************
 * SelectedItemCB()
 ******************************************************************************/
static void SelectedItemCB(w,sltb,call_data)
Widget  w;
XmSelectListTBWidget sltb;
XmListCallbackStruct *call_data;
{
int       i,n;
Arg       args[10];
XmString  xmstring;
char     *str1;

n=0;
XtSetArg(args[n],XmNlabelString,&xmstring);n++;
XtGetValues(w,args,n);

UnManageSelectedListTBWidget(sltb,xmstring);

/* Add the item back into the Available Scrolled List */
AddItemToSelListTB(sltb->selectlisttb.available_sl,xmstring,
                   sltb->selectlisttb.alphabetize_lists);

/* Set flag to indicate a change has taken place */
sltb->selectlisttb.list_changed = 1;
}
/******************************************************************************
 * Initialize()                         
 ******************************************************************************/
static void Initialize(request,new)
XmSelectListTBWidget request, new;

{
int       n;
Arg       args[15];
Widget    hsb,vsb;
Dimension width,vsbwidth;

/* Initialize the Available List Data Struct */
new->selectlisttb.AvailableItems = NULL;
new->selectlisttb.AvailableItemCount = 0;

/* assign the available scrolled list field (redundant info) */
new->selectlisttb.available_sl = (Widget)new;

/* Create the Available List of Items Label */
n=0;
XtSetArg(args[n],XmNleftAttachment,XmATTACH_OPPOSITE_WIDGET);n++;
XtSetArg(args[n],XmNleftWidget,(Widget)new->selectlisttb.available_sl);n++;
XtSetArg(args[n],XmNrightAttachment,XmATTACH_OPPOSITE_WIDGET);n++;
XtSetArg(args[n],XmNrightWidget,(Widget)new->selectlisttb.available_sl);n++;
XtSetArg(args[n],XmNbottomAttachment,XmATTACH_WIDGET);n++;
XtSetArg(args[n],XmNbottomWidget,(Widget)new->selectlisttb.available_sl);n++;
if (new->selectlisttb.available_lbl_string == NULL)
   new->selectlisttb.available_lbl = XmCreateLabel(XtParent(XtParent(new)),
                                  "Available",args,n);
else 
   new->selectlisttb.available_lbl=XmCreateLabel(XtParent(XtParent(new)),
                                 new->selectlisttb.available_lbl_string,args,n);
XtManageChild(new->selectlisttb.available_lbl);

/* Initialize the selected item counter */
new->selectlisttb.selected_item_count = 0;

/* Create the Scrolled Window of Selected Items */
n=0;
XtSetArg(args[n],XmNleftAttachment,XmATTACH_WIDGET);n++;
XtSetArg(args[n],XmNleftWidget,(Widget)new->selectlisttb.available_sl);n++;
XtSetArg(args[n],XmNleftOffset,5);n++;
XtSetArg(args[n],XmNrightAttachment,XmATTACH_FORM);n++;
XtSetArg(args[n],XmNtopAttachment,XmATTACH_OPPOSITE_WIDGET);n++;
XtSetArg(args[n],XmNtopWidget,(Widget)new->selectlisttb.available_sl);n++;
XtSetArg(args[n],XmNbottomAttachment,XmATTACH_OPPOSITE_WIDGET);n++;
XtSetArg(args[n],XmNbottomWidget,(Widget)new->selectlisttb.available_sl);n++;
XtSetArg(args[n],XmNscrollingPolicy,XmAUTOMATIC);n++;
XtSetArg(args[n],XmNscrollBarDisplayPolicy,XmSTATIC);n++;
new->selectlisttb.selected_sw = XmCreateScrolledWindow(XtParent(XtParent(new)),
                                                      "SelectedSW",args,n);


/* Create the Row Column under the scrolled window */
n=0;
XtSetArg(args[n],XmNpacking,XmPACK_COLUMN);n++;
XtSetArg(args[n],XmNorientation,XmVERTICAL);n++;
XtSetArg(args[n],XmNnumColumns,1);n++;
XtSetArg(args[n],XmNspacing,0);n++;
XtSetArg(args[n],XmNmarginHeight,0);n++;
XtSetArg(args[n],XmNmarginWidth,0);n++;
new->selectlisttb.selected_rc=XmCreateRowColumn(new->selectlisttb.selected_sw,
                                                "SelectedRC",args,n);

XtManageChild(new->selectlisttb.selected_sw);
XtManageChild(new->selectlisttb.selected_rc);

/* Retrieve the horizontal and vertical scroll bar ids */
n=0;
XtSetArg(args[n],XmNhorizontalScrollBar,&hsb);n++;
XtSetArg(args[n],XmNverticalScrollBar,&vsb);n++;
XtGetValues(new->selectlisttb.selected_sw,args,n);

/* Unmanage the horizontal scrollbar */
XtUnmanageChild(hsb);

/* Set highlight on enter to False for the vertical scrollbar */
n=0;
XtSetArg(args[n],XmNhighlightThickness,0);n++;
XtSetValues(vsb,args,n);

/* Reset the Scroll Bar width to be 4 less pixels as we turned off highlight */
n=0;
XtSetArg(args[n],XmNwidth,&vsbwidth);n++;
XtGetValues(vsb,args,n);

n=0;
XtSetArg(args[n],XmNwidth,vsbwidth-4);n++;
XtSetValues(vsb,args,n);

/* Add Callback that causes the Available items to move to "Selected" list */
XtAddCallback(new->selectlisttb.available_sl,XmNsingleSelectionCallback,
              (XtCallbackProc) AvailableItemCB,(caddr_t)new);

/* Get the Scrolled List width in order to size buttons */
n=0;
XtSetArg(args[n],XmNwidth,&width);n++;
XtGetValues(new->selectlisttb.available_lbl,args,n);

/* Create the Selected List of Items Label */
n=0;
XtSetArg(args[n],XmNleftAttachment,XmATTACH_OPPOSITE_WIDGET);n++;
XtSetArg(args[n],XmNleftWidget,(Widget)new->selectlisttb.selected_sw);n++;
XtSetArg(args[n],XmNwidth,width);n++;
/****
XtSetArg(args[n],XmNleftOffset,3);n++;
 ****/
XtSetArg(args[n],XmNbottomAttachment,XmATTACH_WIDGET);n++;
XtSetArg(args[n],XmNbottomWidget,(Widget)new->selectlisttb.selected_sw);n++;
if (new->selectlisttb.selected_lbl_string == NULL)
   new->selectlisttb.selected_lbl = XmCreateLabel(XtParent(XtParent(new)),
                                  "Selected",args,n);
else 
   new->selectlisttb.selected_lbl = XmCreateLabel(XtParent(XtParent(new)),
                                  new->selectlisttb.selected_lbl_string,args,n);
XtManageChild(new->selectlisttb.selected_lbl);

/* Create the Off Toggle Button Label */
n=0;
XtSetArg(args[n],XmNrightAttachment,XmATTACH_OPPOSITE_WIDGET);n++;
XtSetArg(args[n],XmNrightWidget,(Widget)new->selectlisttb.selected_sw);n++;
XtSetArg(args[n],XmNrightOffset,vsbwidth);n++;
XtSetArg(args[n],XmNbottomAttachment,XmATTACH_WIDGET);n++;
XtSetArg(args[n],XmNbottomWidget,(Widget)new->selectlisttb.selected_sw);n++;
if (new->selectlisttb.selectedoff_lbl_string == NULL)
   new->selectlisttb.selectedoff_lbl = XmCreateLabel(XtParent(XtParent(new)),
                                  " F ",args,n);
else 
   new->selectlisttb.selectedoff_lbl = XmCreateLabel(XtParent(XtParent(new)),
                              new->selectlisttb.selectedoff_lbl_string,args,n);
XtManageChild(new->selectlisttb.selectedoff_lbl);

/* Create the On Toggle Button Label */
n=0;
XtSetArg(args[n],XmNrightAttachment,XmATTACH_WIDGET);n++;
XtSetArg(args[n],XmNrightWidget,(Widget)new->selectlisttb.selectedoff_lbl);n++;
XtSetArg(args[n],XmNbottomAttachment,XmATTACH_WIDGET);n++;
XtSetArg(args[n],XmNbottomWidget,(Widget)new->selectlisttb.selected_sw);n++;
if (new->selectlisttb.selectedon_lbl_string == NULL)
   new->selectlisttb.selectedon_lbl = XmCreateLabel(XtParent(XtParent(new)),
                                                    "S  ",args,n);
else 
   new->selectlisttb.selectedon_lbl = XmCreateLabel(XtParent(XtParent(new)),
                                new->selectlisttb.selectedon_lbl_string,args,n);

XtManageChild(new->selectlisttb.selectedon_lbl);
}
/******************************************************************************
 * Destroy()
 *	Free text list's private alocatted structures.
 ******************************************************************************/
static void Destroy(tw)
	XmSelectListTBWidget tw;
{
}

/*
===============================================================================
=
= P R I V A T E   F U N C T I O N S
=
===============================================================================
*/

/******************************************************************************
 * AlphabetizeDataList()
 ******************************************************************************/
static void AlphabetizeDataList(data_lst,count)
SListTBType **data_lst;
int           count;

{
int   i,j;
SListTBType dlp;

for (j=0; j<count-1; j++)
    {
    for (i=0; i<count-1; i++)
        {
        /* Compare each item to the string to be inserted */ 
        if (strcmp((*data_lst)[i].name,(*data_lst)[i+1].name) > 0) 
           { 
           dlp = (*data_lst)[i];
           (*data_lst)[i] = (*data_lst)[i+1];
           (*data_lst)[i+1] = dlp; 
           } 
        }
    }
}
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
 * AvailableItemsIndex()           
 ******************************************************************************/
static int AvailableItemsIndex(w,item)
XmSelectListTBWidget w;
XmString item;

{
int i;

for (i=0; i<(w->selectlisttb.AvailableItemCount); i++)
    if (XmStringCompare(w->selectlisttb.AvailableItems[i].item,item))
       return(i);

return(NOT_FOUND);
}
/******************************************************************************
 * UnManageSelectedListTBWidget()           
 ******************************************************************************/
static void UnManageSelectedListTBWidget(w,item)
XmSelectListTBWidget w;
XmString             item;

{
int   i;
char *str;

/* Get the index of AvailableItems list */
if ((i = AvailableItemsIndex(w,item)) == NOT_FOUND)
   {
   /* Post an error box */
   if (XmStringGetLtoR(item,XmSTRING_DEFAULT_CHARSET,&str))
      {
      printf("Unable to locate item *%s* in AvailableItems\n",str);
      XtFree(str);
      }
   return;
   }

w->selectlisttb.AvailableItems[i].is_managed = False;

XtUnmanageChild(w->selectlisttb.AvailableItems[i].fr);
}
/******************************************************************************
 * UnManageSelectedListTBWidgets()           
 ******************************************************************************/
static void UnManageSelectedListTBWidgets(w,data_lst,count)
XmSelectListTBWidget w;
SListTBType *data_lst;
int          count;

{
int   i,j;
char *str;
Widget   *w_lst;
XmString  xmstr;
     
if (count == 0)
   return;

w_lst = (Widget *)XtMalloc(sizeof(Widget) * count);
if (w_lst == NULL)
   {
   /* Memory Error */
   exit(0);
   }

for (j=0; j<count; j++)
    {
    xmstr = XmStringCreateLtoR(data_lst[j].name, XmSTRING_DEFAULT_CHARSET);

    /* Get the index of AvailableItems list */
    if ((i = AvailableItemsIndex(w,xmstr)) == NOT_FOUND)
       {
       /* Post an error box */
       if (XmStringGetLtoR(xmstr,XmSTRING_DEFAULT_CHARSET,&str))
          {
          printf("Unable to locate item *%s* in AvailableItems\n",str);
          XtFree(str);
          }
       return;
       }

    w->selectlisttb.AvailableItems[i].is_managed = False;

    w_lst[j] = w->selectlisttb.AvailableItems[i].fr;

    XmStringFree(xmstr);
    }

XtManageChildren(w_lst,count);

XtFree((char *)w_lst);
}
/******************************************************************************
 * ManageSelectedListTBWidget()           
 ******************************************************************************/
static void ManageSelectedListTBWidget(w,item)
XmSelectListTBWidget w;
XmString             item;

{
int i;
char *str;

/* Get the index of AvailableItems list */
if ((i = AvailableItemsIndex(w,item)) == NOT_FOUND)
   {
   /* Post an error box */
   if (XmStringGetLtoR(item,XmSTRING_DEFAULT_CHARSET,&str))
      {
      printf("Unable to locate item *%s* in AvailableItems\n",str);
      XtFree(str);
      }
   return;
   }

w->selectlisttb.AvailableItems[i].is_managed = True;

XtManageChild(w->selectlisttb.AvailableItems[i].fr);
}
/******************************************************************************
 * ManageSelectedListTBWidgets()           
 ******************************************************************************/
static void ManageSelectedListTBWidgets(w,data_lst,count)
XmSelectListTBWidget w;
SListTBType *data_lst;
int          count;

{
int       i,j;
char     *str;
Widget   *w_lst;
XmString  xmstr;
     
if (count == 0)
   return;

w_lst = (Widget *)XtMalloc(sizeof(Widget) * count);
if (w_lst == NULL)
   {
   /* Memory Error */
   exit(0);
   }

for (j=0; j<count; j++)
    {
    xmstr = XmStringCreateLtoR(data_lst[j].name, XmSTRING_DEFAULT_CHARSET);

    /* Get the index of AvailableItems list */
    if ((i = AvailableItemsIndex(w,xmstr)) == NOT_FOUND)
       {
       /* Post an error box */
       if (XmStringGetLtoR(xmstr,XmSTRING_DEFAULT_CHARSET,&str))
          {
          printf("Unable to locate item *%s* in AvailableItems\n",str);
          XtFree(str);
          }
       return;
       }

    w->selectlisttb.AvailableItems[i].is_managed = True;

    w_lst[j] = w->selectlisttb.AvailableItems[i].fr;

    XmStringFree(xmstr);
    }

XtManageChildren(w_lst,count);

XtFree((char *)w_lst);
}
/******************************************************************************
 * CreateSelectedListTBWidgets()           
 *****r************************************************************************/
static void CreateSelectedListTBWidgets(sltb,item,i,on_state,off_state)
XmSelectListTBWidget sltb;
XmString             item;
int                  i;
int                  on_state;
int                  off_state;

{
int n;
Arg args[10];
char *str1;
Dimension width;

/* Get the Scrolled List width in order to size buttons */
n=0;
XtSetArg(args[n],XmNwidth,&width);n++;
XtGetValues(sltb->selectlisttb.available_sl,args,n);

n=0;
XtSetArg(args[n],XmNhighlightThickness,0);n++;
XtSetArg(args[n],XmNmarginHeight,0);n++;
sltb->selectlisttb.AvailableItems[i].fr =
   XmCreateForm(sltb->selectlisttb.selected_rc,"SelectedFR",args,n);

n=0;
XtSetArg(args[n],XmNlabelString,item);n++;
XtSetArg(args[n],XmNalignment,XmALIGNMENT_BEGINNING);n++;
XtSetArg(args[n],XmNshadowThickness,0);n++;
XtSetArg(args[n],XmNwidth,width);n++;
XtSetArg(args[n],XmNleftAttachment,XmATTACH_FORM);n++;
XtSetArg(args[n],XmNmarginHeight,0);n++;
XtSetArg(args[n],XmNpushButtonEnabled,True);n++;
sltb->selectlisttb.AvailableItems[i].db = 
   XmCreateDrawnButton(sltb->selectlisttb.AvailableItems[i].fr,"SelectedDB",
                       args,n);
XtAddCallback(sltb->selectlisttb.AvailableItems[i].db,XmNactivateCallback,
              (XtCallbackProc) SelectedItemCB,(caddr_t)sltb);
XtManageChild(sltb->selectlisttb.AvailableItems[i].db);

n=0;
XtSetArg(args[n],XmNleftAttachment,XmATTACH_WIDGET);n++;
XtSetArg(args[n],XmNleftWidget,
         (Widget) sltb->selectlisttb.AvailableItems[i].db);n++;
XtSetArg(args[n],XmNtopAttachment,XmATTACH_OPPOSITE_WIDGET);n++;
XtSetArg(args[n],XmNtopWidget,
         (Widget) sltb->selectlisttb.AvailableItems[i].db);n++;
XtSetArg(args[n],XmNspacing,0);n++;
XtSetArg(args[n],XmNmarginHeight,0);n++;
XtSetArg(args[n],XmNmarginWidth,0);n++;
XtSetArg(args[n],XmNhighlightThickness,1);n++;
XtSetArg(args[n],XmNset,on_state);n++;
sltb->selectlisttb.AvailableItems[i].ontb = 
   XmCreateToggleButton(sltb->selectlisttb.AvailableItems[i].fr,"",args,n);

/* Add Callback that sets the changes_made flag when the Toggle is set */
XtAddCallback(sltb->selectlisttb.AvailableItems[i].ontb,XmNvalueChangedCallback,
              (XtCallbackProc) ToggleButtonTwiddleCB,(caddr_t)sltb);

XtManageChild(sltb->selectlisttb.AvailableItems[i].ontb);

n=0;
XtSetArg(args[n],XmNleftAttachment,XmATTACH_WIDGET);n++;
XtSetArg(args[n],XmNleftWidget,
         (Widget) sltb->selectlisttb.AvailableItems[i].ontb);n++;
XtSetArg(args[n],XmNtopAttachment,XmATTACH_OPPOSITE_WIDGET);n++;
XtSetArg(args[n],XmNtopWidget,
         (Widget) sltb->selectlisttb.AvailableItems[i].db);n++;
XtSetArg(args[n],XmNspacing,0);n++;
XtSetArg(args[n],XmNhighlightThickness,1);n++;
XtSetArg(args[n],XmNmarginWidth,0);n++;
XtSetArg(args[n],XmNmarginHeight,0);n++;
XtSetArg(args[n],XmNset,off_state);n++;
sltb->selectlisttb.AvailableItems[i].offtb=
   XmCreateToggleButton(sltb->selectlisttb.AvailableItems[i].fr,"",args,n);

/* Add Callback that sets the changes_made flag when the Toggle is set */
XtAddCallback(sltb->selectlisttb.AvailableItems[i].offtb,
              XmNvalueChangedCallback,
              (XtCallbackProc) ToggleButtonTwiddleCB,(caddr_t)sltb);

XtManageChild(sltb->selectlisttb.AvailableItems[i].offtb);
}
/******************************************************************************
 * AddItemToSelListTB()           
 ******************************************************************************/
static void AddItemToSelListTB(w,item,alphabetize_flag)
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
/******************************************************************************
 * AddItemsToSelListTB()           
 ******************************************************************************/
static void AddItemsToSelListTB(w,items,count,alphabetize_flag)
Widget    w;
XmString *items;
int       count;
Boolean   alphabetize_flag;

{
if (count == 0)
   return;

/* check alphabetize field (boolean) for insertion order */
if (alphabetize_flag)
   {
   AlphabetizeStringTable(&items,count);
   XmListAddItems(w,items,count,0);
   }
else
   {
   /* Add in given order at end of list */
   XmListAddItems(w,items,count,0);
   }

/* Make the last item added visible at the bottom */
XmListSetBottomItem(w,items[count-1]);
}

/*
===============================================================================
=
= P U B L I C   F U N C T I O N S
=
===============================================================================
*/

/******************************************************************************
 * MrmRegisterSelectListTBClass()
 ******************************************************************************/
Cardinal MrmRegisterSelectListTBClass()
{
return	MrmRegisterClass(MrmwcUnknown,
			 "XmSelectListTB",
			 "XmCreateSelectListTB",
			 XmCreateSelectListTB,
			 xmSelectListTBWidgetClass);
}
/******************************************************************************
 * XmCreateSelectListTB()
 ******************************************************************************/
Widget XmCreateSelectListTB(parent, name, args, argCount)
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

lw = XtCreateWidget( name, xmSelectListTBWidgetClass, sw, args, argCount);
XtAddCallback (lw, XmNdestroyCallback, _XmDestroyParentCallback, NULL);

return (lw);
}
/******************************************************************************
 * XmAvailableListTBAddItem()
 ******************************************************************************/
void XmAvailableListTBAddItem(w,item,position,on_state,off_state)
XmSelectListTBWidget w;
XmString             item;
int                  position;
int                  on_state;
int                  off_state;

{
AddItemToSelListTB(w->selectlisttb.available_sl,item,
                   w->selectlisttb.alphabetize_lists);

(w->selectlisttb.AvailableItemCount)++;
w->selectlisttb.AvailableItems = 
   (AvailableItemType *)XtRealloc((char *)w->selectlisttb.AvailableItems,
                  w->selectlisttb.AvailableItemCount*sizeof(AvailableItemType));

/* Assign the item name as a method of indexing and initialize */
w->selectlisttb.AvailableItems[w->selectlisttb.AvailableItemCount-1].item = 
   XmStringCopy(item);

CreateSelectedListTBWidgets(w,item,w->selectlisttb.AvailableItemCount-1,
                            on_state,off_state);

w->selectlisttb.AvailableItems[w->selectlisttb.AvailableItemCount-1].is_managed=
   False;
w->selectlisttb.AvailableItems[w->selectlisttb.AvailableItemCount-1].on_state =
   on_state;
w->selectlisttb.AvailableItems[w->selectlisttb.AvailableItemCount-1].off_state=
   off_state;
}
/******************************************************************************
 * XmAvailableListTBAddItems()
 ******************************************************************************/
void XmAvailableListTBAddItems(w,data_lst,count,position)
XmSelectListTBWidget  w;
SListTBType          *data_lst;
int                   count;
int                   position;

{
int      i;
XmString xmstr;
XmString *xmstringtable;

xmstringtable = (XmString * ) XtMalloc(sizeof(XmString) * count);
if (xmstringtable == NULL)
   {
   /* Memory Error */
   exit(0);
   }

AlphabetizeDataList(&data_lst,count);

for (i=0; i<count; i++)
    {
    /* No need to free as it will be assigned to structure */
    xmstr = XmStringCreateLtoR(data_lst[i].name, XmSTRING_DEFAULT_CHARSET);

    xmstringtable[i]=xmstr;

    (w->selectlisttb.AvailableItemCount)++;
    w->selectlisttb.AvailableItems = 
       (AvailableItemType *)XtRealloc((char *)w->selectlisttb.AvailableItems,
                  w->selectlisttb.AvailableItemCount*sizeof(AvailableItemType));

    /* Assign the item name as a method of indexing and initialize */
    w->selectlisttb.AvailableItems[w->selectlisttb.AvailableItemCount-1].item = 
       xmstr;

    CreateSelectedListTBWidgets(w,xmstr,w->selectlisttb.AvailableItemCount-1,
                                data_lst[i].on_flag,data_lst[i].off_flag);

    w->selectlisttb.AvailableItems[w->selectlisttb.AvailableItemCount-1].is_managed = False;
    w->selectlisttb.AvailableItems[w->selectlisttb.AvailableItemCount-1].on_state = data_lst[i].on_flag;
    w->selectlisttb.AvailableItems[w->selectlisttb.AvailableItemCount-1].off_state = data_lst[i].off_flag;
    }

AddItemsToSelListTB(w->selectlisttb.available_sl,xmstringtable,count,
                   w->selectlisttb.alphabetize_lists);

XtFree((char *)xmstringtable);
}
/******************************************************************************
 * XmSelectListTBAddItem()
 ******************************************************************************/
void XmSelectListTBAddItem(w,item,position,on_state,off_state)
XmSelectListTBWidget w;
XmString             item;
int                  on_state;
int                  off_state;

{
w->selectlisttb.AvailableItemCount++;
w->selectlisttb.AvailableItems = 
   (AvailableItemType *)XtRealloc((char *)w->selectlisttb.AvailableItems,
                w->selectlisttb.AvailableItemCount * sizeof(AvailableItemType));

/* Assign the item name as a method of indexing and initialize */
w->selectlisttb.AvailableItems[w->selectlisttb.AvailableItemCount-1].item  = 
   XmStringCopy(item);

CreateSelectedListTBWidgets(w,item,w->selectlisttb.AvailableItemCount-1,
                            on_state,off_state);

w->selectlisttb.AvailableItems[w->selectlisttb.AvailableItemCount-1].is_managed=
   True;
w->selectlisttb.AvailableItems[w->selectlisttb.AvailableItemCount-1].on_state =
   on_state;
w->selectlisttb.AvailableItems[w->selectlisttb.AvailableItemCount-1].off_state=
   off_state;

XtManageChild(w->selectlisttb.AvailableItems[w->selectlisttb.AvailableItemCount-1].fr);
}
/******************************************************************************
 * XmSelectListTBAddItems()
 ******************************************************************************/
void XmSelectListTBAddItems(w,data_lst,count)
XmSelectListTBWidget  w;
SListTBType          *data_lst;
int                   count;

{
int       i;
Widget   *w_lst;
XmString  xmstr;
     
if (count == 0)
   return;

w_lst = (Widget *)XtMalloc(sizeof(Widget) * count);
if (w_lst == NULL)
   {
   /* Memory Error */
   exit(0);
   }

AlphabetizeDataList(&data_lst,count);

for (i=0; i<count; i++)
    {
    /* No need to free as it will be assigned to structure */
    xmstr = XmStringCreateLtoR(data_lst[i].name, XmSTRING_DEFAULT_CHARSET);

    w->selectlisttb.AvailableItemCount++;
    w->selectlisttb.AvailableItems = 
    (AvailableItemType *)XtRealloc((char *)w->selectlisttb.AvailableItems,
                w->selectlisttb.AvailableItemCount * sizeof(AvailableItemType));

    /* Assign the item name as a method of indexing and initialize */
    w->selectlisttb.AvailableItems[w->selectlisttb.AvailableItemCount-1].item = 
       xmstr;

    CreateSelectedListTBWidgets(w,xmstr,w->selectlisttb.AvailableItemCount-1,
                                data_lst[i].on_flag,data_lst[i].off_flag);

    w->selectlisttb.AvailableItems[w->selectlisttb.AvailableItemCount-1].is_managed = True;
    w->selectlisttb.AvailableItems[w->selectlisttb.AvailableItemCount-1].on_state = data_lst[i].on_flag;
    w->selectlisttb.AvailableItems[w->selectlisttb.AvailableItemCount-1].off_state = data_lst[i].off_flag;

    w_lst[i] = 
        w->selectlisttb.AvailableItems[w->selectlisttb.AvailableItemCount-1].fr;

/*** Why is this left over?
    XtManageChild(w->selectlisttb.AvailableItems[w->selectlisttb.AvailableItemCount-1].fr);
 ***/

    }

XtManageChildren(w_lst,count);

XtFree((char *)w_lst);
}
/******************************************************************************
 * XmSelectListTBSetItemSelected()
 ******************************************************************************/
void XmSelectListTBSetItemSelected(w,item,on_state,off_state)
XmSelectListTBWidget w;
XmString             item;
int                  on_state;
int                  off_state;

{
int   i;
char *str;

/* Take item out of "Available" list */
XmListDeleteItem(w->selectlisttb.available_sl,item);

/* Set the Toggle Button States appropriately */
if ((i = AvailableItemsIndex(w,item)) == NOT_FOUND)
   {
   /* Post an error box */
   if (XmStringGetLtoR(item,XmSTRING_DEFAULT_CHARSET,&str))
      {
      printf("Unable to locate item *%s* in AvailableItems\n",str);
      XtFree(str);
      }
   return;
   }

XmToggleButtonSetState(w->selectlisttb.AvailableItems[i].ontb,on_state,0);
XmToggleButtonSetState(w->selectlisttb.AvailableItems[i].offtb,off_state,0);

ManageSelectedListTBWidget(w,item);

/* Set flag to indicate a change has taken place */
w->selectlisttb.list_changed = 1;
}
/******************************************************************************
 * XmSelectListTBSetItemsSelected()
 ******************************************************************************/
void XmSelectListTBSetItemsSelected(w,data_lst,count)
XmSelectListTBWidget  w;
SListTBType          *data_lst;
int                   count;

{
int       i,j;
char     *str;
XmString *xmstringtable;
     
if (count == 0)
   return;

xmstringtable = (XmString * ) XtMalloc(sizeof(XmString) * count); 
if (xmstringtable == NULL)
   {
   /* Memory Error */
   exit(0);
   }

for (j=0; j<count; j++)
    {
    xmstringtable[j] = XmStringCreateLtoR(data_lst[j].name, 
                                          XmSTRING_DEFAULT_CHARSET);

    /* Set the Toggle Button States appropriately */
    if ((i = AvailableItemsIndex(w,xmstringtable[j])) == NOT_FOUND)
       {
       /* Post an error box */
       if (XmStringGetLtoR(xmstringtable[j],XmSTRING_DEFAULT_CHARSET,&str))
          {
          printf("Unable to locate item *%s* in AvailableItems\n",str);
          XtFree(str);
          }
       return;
       }

    XmToggleButtonSetState(w->selectlisttb.AvailableItems[i].ontb,
                           data_lst[j].on_flag,0);
    XmToggleButtonSetState(w->selectlisttb.AvailableItems[i].offtb,
                           data_lst[j].off_flag,0);
    }

/* Take items out of "Available" list */
XmListDeleteItems(w->selectlisttb.available_sl,xmstringtable,count);

ManageSelectedListTBWidgets(w,data_lst,count);

/* free string table */
for (j=0; j<count; j++)
    XmStringFree(xmstringtable[j]);

XtFree((char *)xmstringtable);

/* Set flag to indicate a change has taken place */
w->selectlisttb.list_changed = 1;
}
/******************************************************************************
 * XmSelectListTBSetItemAvailable()
 ******************************************************************************/
void XmSelectListTBSetItemAvailable(w,item,position)
XmSelectListTBWidget w;
XmString             item;
int                  position;

{
/* Take item out of "Selected" list */
UnManageSelectedListTBWidget(w,item);

AddItemToSelListTB(w->selectlisttb.available_sl,item,
                   w->selectlisttb.alphabetize_lists);

/* Set flag to indicate a change has taken place */
w->selectlisttb.list_changed = 1;
}
/******************************************************************************
 * XmSelectListTBSetItemsAvailable()
 ******************************************************************************/
void XmSelectListTBSetItemsAvailable(w,data_lst,count)
XmSelectListTBWidget  w;
SListTBType          *data_lst;
int                   count;

{
int      i;
XmString xmstr;

if (count == 0)
   return;

/* Take item out of "Selected" list */
UnManageSelectedListTBWidgets(w,data_lst,count);

for (i=0; i<count; i++)
    {
    xmstr = XmStringCreateLtoR(data_lst[i].name, XmSTRING_DEFAULT_CHARSET);

    AddItemToSelListTB(w->selectlisttb.available_sl,xmstr,
                       w->selectlisttb.alphabetize_lists);

    XmStringFree(xmstr);
    }

/* Set flag to indicate a change has taken place */
w->selectlisttb.list_changed = 1;
}
/******************************************************************************
 * XmSelectListTBSetAllItemsSelected()
 ******************************************************************************/
void XmSelectListTBSetAllItemsSelected(w)
XmSelectListTBWidget w;

{
int            i,j;
Widget        *w_lst;

/* Take all items out of "Available" list */
XmListDeleteAllItems(w->selectlisttb.available_sl);

w_lst = (Widget *)XtMalloc(sizeof(Widget) * w->selectlisttb.AvailableItemCount);
if (w_lst == NULL)
   {
   /* Memory Error */
   exit(0);
   }

/* Manage All Available Items */
j=0;
for (i=0; i<(w->selectlisttb.AvailableItemCount); i++)
    {
    if (! w->selectlisttb.AvailableItems[i].is_managed)
       {
       w_lst[j] = w->selectlisttb.AvailableItems[i].fr;
       w->selectlisttb.AvailableItems[i].is_managed = True;
       j++;
       }
    }

XtManageChildren(w_lst,j);

XtFree((char *)w_lst);

/* Set flag to indicate a change has taken place */
w->selectlisttb.list_changed = 1;
}
/******************************************************************************
 * XmSelectListTBSetAllItemsAvailable()
 ******************************************************************************/
void XmSelectListTBSetAllItemsAvailable(w)
XmSelectListTBWidget w;

{
int            i,j;
Widget        *w_lst;
XmString      *xmstringtable;

w_lst = (Widget *)XtMalloc(sizeof(Widget) * w->selectlisttb.AvailableItemCount);
if (w_lst == NULL)
   {
   /* Memory Error */
   exit(0);
   }

xmstringtable = (XmString * ) XtMalloc(sizeof(XmString) * 
                                       w->selectlisttb.AvailableItemCount); 
if (xmstringtable == NULL)
   {
   /* Memory Error */
   exit(0);
   }

/* Unmanage All Available Items */
j=0;
for (i=0; i<(w->selectlisttb.AvailableItemCount); i++)
    {
    if (w->selectlisttb.AvailableItems[i].is_managed)
       {
       w_lst[j] = w->selectlisttb.AvailableItems[i].fr;
       w->selectlisttb.AvailableItems[i].is_managed = False;

       xmstringtable[j] = w->selectlisttb.AvailableItems[i].item;

       j++;
       }
    }

XtUnmanageChildren(w_lst,j);

AddItemsToSelListTB(w->selectlisttb.available_sl,xmstringtable,j,
                    w->selectlisttb.alphabetize_lists);

XtFree((char *)w_lst);

XtFree((char *)xmstringtable);

/* Set flag to indicate a change has taken place */
w->selectlisttb.list_changed = 1;
}
/******************************************************************************
 * XmSelectListTBClean()
 ******************************************************************************/
void XmSelectListTBClean(w)
XmSelectListTBWidget w;

{
int i;

/* Set items available (unmanage selected) to optimize Destroy widget process */
XmSelectListTBSetAllItemsAvailable(w);

/* This is overkill as they should all be deleted from above */
XmListDeleteAllItems(w->selectlisttb.available_sl);

/* for each selected item... */
for (i=0; i<(w->selectlisttb.AvailableItemCount); i++)
    {
    XmStringFree(w->selectlisttb.AvailableItems[i].item);

    /* Destroy the Widget structure and reset the widget fields to NULL */
    XtDestroyWidget(w->selectlisttb.AvailableItems[i].fr);
    }
w->selectlisttb.AvailableItemCount = 0;

if (w->selectlisttb.AvailableItems)
   {
   XtFree((char *)w->selectlisttb.AvailableItems);
   w->selectlisttb.AvailableItems = NULL;
   }
}
/******************************************************************************
 * XmSelectListTBFree()
 ******************************************************************************/
void XmSelectListTBFree(data_lst,count)
SListTBType *data_lst;
int          count;

{
int i;

for (i=0; i<count; i++)
    if (data_lst[i].name)
       XtFree(data_lst[i].name);

if (data_lst)
   XtFree((char *)data_lst);
}
/******************************************************************************
 * XmSelectListTBGetItems()
 ******************************************************************************/
void XmSelectListTBGetItems(w,data_lst,count)
XmSelectListTBWidget    w;
SListTBType           **data_lst;
int                    *count;

{
int            i,j;
char          *str;
XmStringTable  items;

(*count) = 0;

for (i=0; i<(w->selectlisttb.AvailableItemCount); i++)
    {
    if (w->selectlisttb.AvailableItems[i].is_managed)
       (*count)++;
    }

if (*count == 0)
   {
   *data_lst = NULL;
   return;
   }

/* Malloc the Data Struct Array, Application responsible for freeing */
if ((*data_lst=(SListTBType *)XtMalloc(sizeof(SListTBType)*(*count)))==NULL)
   {
   /* Memory Error ???*/
   return;
   }

/* Extract the character string to be inserted */
j=0;
for (i=0; i<(w->selectlisttb.AvailableItemCount); i++)
    {
    if (w->selectlisttb.AvailableItems[i].is_managed)
       {
       if (XmStringGetLtoR(w->selectlisttb.AvailableItems[i].item,
                           XmSTRING_DEFAULT_CHARSET,&str))
          {
          (*data_lst)[j].name=(char *)str;
          (*data_lst)[j].on_flag =
             XmToggleButtonGetState(w->selectlisttb.AvailableItems[i].ontb);
          (*data_lst)[j].off_flag =
             XmToggleButtonGetState(w->selectlisttb.AvailableItems[i].offtb);
          j++;
          }
       else
          {
          /* Post an error box as an item is supposed to exist */
          printf("Unable to XmStringGetLtoR of *%s*\n",
                 w->selectlisttb.AvailableItems[i].item);
          }
       }
    }
}
/******************************************************************************
 * XmSelectListTBItemExists()
 ******************************************************************************/
Boolean XmSelectListTBItemExits(w,str)
XmSelectListTBWidget  w;
char               *str;

{
int      i;
XmString xmstr;

xmstr = XmStringCreateLtoR(str, XmSTRING_DEFAULT_CHARSET);

/* Must Exit if there are memory problems */
if (! xmstr)
   exit(0);

/* for each selected item... */
for (i=0; i<(w->selectlisttb.AvailableItemCount); i++)
    {
    if ((w->selectlisttb.AvailableItems[i].is_managed != NULL) &&
        XmStringCompare(w->selectlisttb.AvailableItems[i].item,xmstr))
       {
       XmStringFree(xmstr);
       return(True);
       }
    }
XmStringFree(xmstr);
return(False);
}
/******************************************************************************
 * XmAvailableListTBItemExists()
 ******************************************************************************/
Boolean XmAvailableListTBItemExists(w,str)
XmSelectListTBWidget  w;
char                 *str;

{
XmString xmstr;

xmstr = XmStringCreateLtoR(str, XmSTRING_DEFAULT_CHARSET);

/* Must Exit if there are memory problems */
if (! xmstr)
   exit(0);

if (XmListItemExists(w->selectlisttb.available_sl,xmstr))
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
 * XmSelListTBAddTabGroup()
 ******************************************************************************/
void XmSelListTBAddTabGroup(w)
XmSelectListTBWidget w;

{
XmAddTabGroup(w->selectlisttb.available_sl);
XmAddTabGroup(w->selectlisttb.selected_sw);
}
/******************************************************************************
 * XmSelListTBInitChangeFlag()
 ******************************************************************************/
void XmSelListInitTBChangeFlag(w)
XmSelectListTBWidget w;

{
w->selectlisttb.list_changed = 0;
}
/******************************************************************************
 * XmSelListTBMadeChanges()
 ******************************************************************************/
Boolean XmSelListTBMadeChanges(w)
XmSelectListTBWidget w;

{
if (w->selectlisttb.list_changed == 1)
   return(True);
return(False);
}
/******************************************************************************
 * XmSelListTBSetAllTBs()
 ******************************************************************************/
void XmSelListTBSetAllTBs(w, tb1_state,tb2_state)
XmSelectListTBWidget w;
int                  tb1_state;
int                  tb2_state;

{
int i;

/* Pass 0 or 1 to set all toggle buttons to a particular state, -1 to ignore */

if ((tb1_state == 0) || (tb1_state == 1))
   for (i=0; i<(w->selectlisttb.AvailableItemCount); i++)
       XmToggleButtonSetState(w->selectlisttb.AvailableItems[i].ontb,tb1_state,
                              0);

if ((tb2_state == 0) || (tb2_state == 1))
   for (i=0; i<(w->selectlisttb.AvailableItemCount); i++)
       XmToggleButtonSetState(w->selectlisttb.AvailableItems[i].offtb,tb2_state,
                              0);
}
/******************************************************************************
 * XmSelListTBAllTBsInState()
 ******************************************************************************/
Boolean XmSelListTBAllTBsInState(w, tb_index,state)
XmSelectListTBWidget w;
int                  tb_index;
Boolean              state;

{
int i;

if (tb_index == LEFT_TOGGLE_BUTTON)
   {
   for (i=0; i<(w->selectlisttb.AvailableItemCount); i++)
       if (XmToggleButtonGetState(w->selectlisttb.AvailableItems[i].ontb) != 
           state)
           return(False);

   return(True);
   }

else if (tb_index == RIGHT_TOGGLE_BUTTON)
   {
   for (i=0; i<(w->selectlisttb.AvailableItemCount); i++)
       if (XmToggleButtonGetState(w->selectlisttb.AvailableItems[i].offtb) != 
           state)
           return(False);

   return(True);
   }

else
   return(False);
}
