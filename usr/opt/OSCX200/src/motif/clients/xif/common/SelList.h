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
/*
 * @(#)$RCSfile: SelList.h,v $ $Revision: 1.1.2.5 $ (DEC) $Date: 1993/09/23 20:43:00 $
 */

/******************************************************************************
*******************************************************************************
*
*  (c) Copyright 1989, 1990 OPEN SOFTWARE FOUNDATION, INC.
*  (c) Copyright 1989, 1990 DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASS.
*  (c) Copyright 1987, 1988, 1989, 1990 HEWLETT-PACKARD COMPANY
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
*  (c) Copyright 1989, 1990 Open Software Foundation, Inc.  Unpublished - all
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
*  (c) Copyright 1989, 1990 Open Software Foundation, Inc.
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
******************************************************************************/
/***********************************************************************
 *
 * Select List Widget
 *
 ***********************************************************************/
#ifndef _XmSelectList_h
#define _XmSelectList_h

#include <Xm/Xm.h>
#include <Xm/List.h>

#define XmNselectListItems		"selectListItems"
#define XmNavailableListItems		"availableListItems"
#define XmNselectListScrolledList	"selectListScrolledList"
#define XmNavailableListScrolledList	"availableListScrolledList"
#define XmNselectListLabel		"selectListLabel"
#define XmNavailableListLabel		"availableListLabel"
#define XmNselectListLabelString	"selectListLabelString"
#define XmNavailableListLabelString	"availableListLabelString"
#define XmNalphabetizeLists             "alphabetizeLists"
#define XmNlistChanged                  "listChanged"

#ifdef _NO_PROTO
extern Widget XmCreateSelectList();
extern void   XmAvailableListAddItem();
extern void   XmAvailableListDeleteItem();
extern void   XmAvailableListDeleteAllItems();
extern void   XmAvailableListAddItems();
extern void   XmSelectListAddItem();
extern void   XmSelectListDeleteItem();
extern void   XmSelectListDeleteAllItems();
extern void   XmSelectListAddItems();
extern void   XmSelectListGetItems();
extern void   XmSetAllItemsAvailable();
extern void   XmSetAllItemsAvailable();
extern void   XmSelListAddTabGroup();
extern void   XmSelListInitChangeFlag();
extern void   XmSelListAddSelectedCallback();
extern void   XmSelListAddAvailableCallback();
extern Boolean XmAvailableListItemExists();
extern Boolean XmSelectListItemExists();
extern Boolean XmSelListMadeChanges();
#else /* _NO_PROTO */
extern Widget XmCreateSelectList(Widget parent, char *name, Arg *arglist, 
                                 int argCount);
extern void   XmSelectListGetItems(Widget w, char **name_lst, int *name_count);
extern void   XmSelectListAddItem(Widget w, XmString item, int position);
extern void   XmSelectListAddItems(Widget w, XmString *items, 
                                   int item_count, int position);
extern void   XmSelectListDeleteItem(Widget w, XmString item);
extern void   XmSelectListDeleteAllItems(Widget w);
extern void   XmAvailableListAddItem(Widget w, XmString item, int position);
extern void   XmAvailableListAddItems(Widget w, XmString *items, 
                                      int item_count, int position);
extern void   XmAvailableListDeleteItem(Widget w, XmString item);
extern void   XmAvailableListDeleteAllItems(Widget w);
extern void   XmSetAllItemsSelected(Widget w);
extern void   XmSetAllItemsAvailable(Widget w);
extern void   XmSelListAddTabGroup(Widget w);
extern void   XmSelListInitChangeFlag(Widget w);
extern Boolean XmAvailableListItemExists(Widget w,char *str);
extern Boolean XmSelectListItemExists(Widget w,char *str);
extern Boolean XmSelListMadeChanges(Widget w);
extern void   XmSelListAddSelectedCallback(Widget w,void(*CB)(),XtPointer data);
extern void   XmSelListAddAvailableCallback(Widget w,void(*CB)(),
                                            XtPointer data);
#endif /* _NO_PROTO */

externalref WidgetClass xmSelectListWidgetClass;
typedef struct _XmSelectListClassRec *XmSelectListWidgetClass;
typedef struct _XmSelectListRec      *XmSelectListWidget;


/*fast subclass define */
#ifndef XmIsSelectList
#define XmIsSelectList(w)     XtIsSubclass(w, xmSelectListWidgetClass)
#endif /* XmIsSelectList */

#endif /* _XmSelectList_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */
