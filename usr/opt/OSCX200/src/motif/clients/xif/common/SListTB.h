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
 * @(#)$RCSfile: SListTB.h,v $ $Revision: 1.1.2.3 $ (DEC) $Date: 1993/09/03 18:16:29 $
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
******************************************************************************/

/***********************************************************************
 *
 * Select List With Toggle Buttons Widget
 *
 ***********************************************************************/
#ifndef _XmSelectListTB_h
#define _XmSelectListTB_h

#include <Xm/Xm.h>
#include <Xm/List.h>

#define XmNselectListTBItems			"selectListTBItems"
#define XmNavailableListTBItems			"availableListTBItems"
#define XmNselectListTBScrolledList		"selectListTBScrolledList"
#define XmNavailableListTBScrolledWindow	"availableListTBScrolledWindow"
#define XmNavailableListTBRowColumn		"availableListTBRowColumn"
#define XmNselectListTBLabel			"selectListTBLabel"
#define XmNavailableListTBLabel			"availableListTBLabel"
#define XmNselectListTBLabelString		"selectListTBLabelString"
#define XmNselectListTBOnLabelString		"selectListTBOnLabelString"
#define XmNselectListTBOffLabelString		"selectListTBOffLabelString"
#define XmNavailableListTBLabelString		"availableListTBLabelString"
#define XmNalphabetizeListTBs          		"alphabetizeListTBs"
#define XmNlistTBChanged               		"listTBChanged"

#define LEFT_TOGGLE_BUTTON	1
#define RIGHT_TOGGLE_BUTTON	2

typedef struct _SListTBType_ {
   char *name;
   int   on_flag;
   int   off_flag;
} SListTBType;

#ifdef _NO_PROTO
extern Widget XmCreateSelectListTB();
extern void   XmAvailableListTBAddItem();
extern void   XmSelectListTBAddItem();
extern void   XmAvailableListTBAddItems();
extern void   XmSelectListTBAddItems();
extern void   XmSelectListTBSetItemSelected();
extern void   XmSelectListTBSetItemAvailable();
extern void   XmSelectListTBSetAllItemsSelected();
extern void   XmSelectListTBSetAllItemsAvailable();
extern void   XmSelectListTBGetItems();
extern void   XmSelectListTBFree();
extern void   XmSelListTBAddTabGroup();
extern void   XmSelListTBInitChangeFlag();
extern Boolean XmAvailableListTBItemExists();
extern Boolean XmSelectListTBItemExists();
extern Boolean XmSelListTBMadeChanges();
extern void   XmSelListTBSetAllTBs();
extern Boolean XmSelListTBAllTBsInState();

#else /* _NO_PROTO */
extern Widget XmCreateSelectListTB(Widget  parent, 
                                   char   *name, 
                                   Arg    *arglist, 
                                   int     argCount);
extern Widget XmAvailableListTBAddItem(Widget   w, 
                                       XmString item, 
                                       int      position,
                                       int      on_state, 
                                       int      off_state);
extern Widget XmAvailableListTBAddItems(Widget       w, 
                                        SListTBType *data_lst;
                                        int          count;
                                        int          position);
extern Widget XmSelectListTBAddItem(Widget   w, 
                                    XmString item, 
                                    int      on_state, 
                                    int      off_state);
extern Widget XmSelectListTBAddItems(Widget   w, 
                                     SListTBType *data_lst,
                                     int          count);
extern Widget XmSelectListTBSetItemSelected(Widget   w, 
                                            XmString item);
extern Widget XmSelectListTBSetItemsSelected(Widget   w, 
                                             SListTBType *data_lst,
                                             int          count);
extern Widget XmSelectListTBSetItemAvailable(Widget   w, 
                                             XmString item, 
                                             int      position);
extern Widget XmSelectListTBSetItemsAvailable(Widget       w, 
                                              SListTBType *data_lst;
                                              int          count,
                                              int          position);
extern void   XmSelectListTBSetAllItemsSelected(Widget w);
extern void   XmSelectListTBSetAllItemsAvailable(Widget w);
extern void   XmSelectListTBClean(Widget w);
extern void   XmSelectListTBGetItems(Widget w,SListTBType **data_lst,
                                     int *count);
extern void   XmSelectListTBFree(SListTBType *data_lst; int count);
extern void   XmSelListTBAddTabGroup(Widget w);
extern void   XmSelListTBInitChangeFlag(Widget w);
extern Boolean XmAvailableListTBItemExists(Widget w,char *str);
extern Boolean XmSelectListTBItemExists(Widget w,char *str);
extern Boolean XmSelListTBMadeChanges(Widget w);
extern void   XmSelListTBSetAllTBs(Widget w,int tb1_state,int tb2_state);
extern Boolean XmSelListTBAllTBsInState(Widget w,int tb_index,Boolean state);
#endif /* _NO_PROTO */

externalref WidgetClass xmSelectListWidgetClass;
typedef struct _XmSelectListTBClassRec *XmSelectListTBWidgetClass;
typedef struct _XmSelectListTBRec      *XmSelectListTBWidget;


/*fast subclass define */
#ifndef XmIsSelectListTB
#define XmIsSelectListTB(w)     XtIsSubclass(w, xmSelectListTBWidgetClass)
#endif /* XmIsSelectListTB */

#endif /* _XmSelectListTB_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */
