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
 * @(#)$RCSfile: TextList.h,v $ $Revision: 1.1.2.5 $ (DEC) $Date: 1993/09/23 20:43:07 $
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
 * Text List Widget
 *
 ***********************************************************************/
#ifndef _XmTextList_h
#define _XmTextList_h

#include <Xm/Xm.h>
#include <Xm/Text.h>

#define XmNtextListItems		"textListItems"
#define XmNtextListPushButton		"textListPushButton"
#define XmNtextListScrolledList		"textListScrolledList"
#define XmNtextListCancelPushButton	"textListCancelPushButton"
#define XmNtextListForm			"textListForm"
#define XmNtextListNoNewEntry           "textListNoNewEntry"
#define XmNtextListAlphabetizeLists     "textListAlphabetizeLists"

#ifdef _NO_PROTO
extern Widget  XmCreateTextList();
extern void    XmTextListAddItem();
extern void    XmTextListDeleteItem();
extern void    XmTextListAddItems();
extern void    XmTextListDeleteAllItems();
extern Boolean XmTextListItemExists();
extern void    XmTextListAddTabGroup();
extern void    XmTextListSetSensitive();
extern Boolean XmTextListIsSearchString();
extern void    XmTextListSetLoadRoutine();
#else /* _NO_PROTO */
extern Widget  XmCreateTextList(Widget parent, 
                                char *name, 
                                Arg *arglist, 
                                int argCount);
extern void    XmTextListAddItem(Widget w, 
                                 XmString item, 
                                 int position);
extern void    XmTextListDeleteItem(Widget w, 
                                    XmString item);
extern void    XmTextListAddItems(Widget w, 
                                  XmString *items, 
                                  int item_count, 
                                  int position);
extern void    XmTextListDeleteAllItems(Widget w); 
extern Boolean XmTextListItemExists(Widget w, char *itemstr);
extern void    XmTextListAddTabGroup(Widget w);
extern void    XmTextListSetSensitive(Widget w,int value);
extern Boolean XmTextListIsSearchString(Widget w);
extern void    XmTextListSetLoadRoutine(Widget w,
                                        void (*LoadRoutine)());
#endif /* _NO_PROTO */

externalref WidgetClass xmTextListWidgetClass;
typedef struct _XmTextListClassRec *XmTextListWidgetClass;
typedef struct _XmTextListRec      *XmTextListWidget;


/*fast subclass define */
#ifndef XmIsTextList
#define XmIsTextList(w)     XtIsSubclass(w, xmTextListWidgetClass)
#endif /* XmIsTextList */

#define FALSE 0
#define TRUE  1

#endif /* _XmTextList_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */
