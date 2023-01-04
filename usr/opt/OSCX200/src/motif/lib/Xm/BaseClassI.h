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
/* BuildSystemHeader added automatically */
/* $Header: /alphabits/u3/x11/ode/rcs/x11/src/motif/lib/dec/Xm/BaseClassI.h,v 1.1.2.2 92/04/01 14:48:46 Russ_Kuhn Exp $ */
#ifdef REV_INFO
#ifndef lint
static char SCCSID[] = "OSF/Motif: @(#)BaseClassI.h	3.5 90/06/26";
#endif /* lint */
#endif /* REV_INFO */
/******************************************************************************
*******************************************************************************
*
*  (c) Copyright 1989, 1990 OPEN SOFTWARE FOUNDATION, INC.
*  (c) Copyright 1987, 1988, 1989 HEWLETT-PACKARD COMPANY
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
#ifndef _BaseClassI_h
#define _BaseClassI_h

typedef Widget (*XmSecObjCreateFunc)();

#define XmInheritClass		   ((WidgetClass) &_XmInheritClass)

#define XmInheritCreate		   ((XtInitProc) _XtInherit)
#define XmInheritInitializePrehook ((XtInitProc) _XtInherit)
#define XmInheritSetValuesPrehook  ((XtSetValuesFunc) _XtInherit)
#define XmInheritGetValuesPrehook  ((XtArgsProc) _XtInherit)
#define XmInheritInitializePosthook ((XtInitProc) _XtInherit)
#define XmInheritSetValuesPosthook  ((XtSetValuesFunc) _XtInherit)
#define XmInheritGetValuesPosthook  ((XtArgsProc) _XtInherit)
#define XmInheritGetSecResData	   ((XmGetSecResDataFunc) _XtInherit)
#ifndef NO_CLASS_PART_INIT_HOOK
#define XmInheritClassPartInitPrehook ((XtWidgetClassProc) _XtInherit)
#define XmInheritClassPartInitPosthook ((XtWidgetClassProc) _XtInherit)
#endif /* NO_CLASS_PART_INIT_HOOK */
#define BCEPTR(wc)  ((XmBaseClassExt *)(&(((WidgetClass)(wc))->core_class.extension)))

#ifdef notdef
#define _XmGetBaseClassExtPtr(wc, owner) \
  ((*BCEPTR(wc) && (((*BCEPTR(wc))->record_type) == owner))\
   ? BCEPTR(wc) \
   :((XmBaseClassExt *) _XmGetClassExtensionPtr(((XmGenericClassExt *)BCEPTR(wc)), owner)))
#else
#define _XmGetBaseClassExtPtr(wc, owner) \
  ((XmBaseClassExt *)_XmGetClassExtensionPtr(((XmGenericClassExt *)BCEPTR(wc)), owner))
#endif /* notdef */

typedef struct _XmObjectClassExtRec{
    XtPointer 		next_extension;	
    XrmQuark 		record_type;	
    long 		version;	
    Cardinal 		record_size;	
}XmObjectClassExtRec, *XmObjectClassExt;

typedef struct _XmGenericClassExtRec{
    XtPointer 		next_extension;	
    XrmQuark 		record_type;	
    long 		version;	
    Cardinal 		record_size;	
}XmGenericClassExtRec, *XmGenericClassExt;

#define XmBaseClassExtVersion 1L
/* 
 * defines for 256 bit (at least) bit field
 */
#define _XmGetFlagsBit(field, bit) \
  ((field[ ((bit >> 3) & 31) ]) & (1 << (bit & 7)))

#define _XmSetFlagsBit(field, bit, value) \
  (field[ ((bit >> 3) & 31) ] |= (1 << (bit & 7)))

typedef int (* XmGetSecResDataFunc) ();

typedef struct _XmBaseClassExtRec{
    XtPointer 		next_extension;	
    XrmQuark 		record_type;	
    long 		version;	
    Cardinal 		record_size;	
    XtInitProc		initializePrehook;
    XtSetValuesFunc 	setValuesPrehook;
    XtInitProc		initializePosthook;
    XtSetValuesFunc 	setValuesPosthook;
    WidgetClass		secondaryObjectClass;
    XtInitProc		secondaryObjectCreate;
    XmGetSecResDataFunc	getSecResData;
    unsigned char	flags[32];
    XtArgsProc		getValuesPrehook;
    XtArgsProc		getValuesPosthook;
#ifndef NO_CLASS_PART_INIT_HOOK
    XtWidgetClassProc	classPartInitPrehook;
    XtWidgetClassProc	classPartInitPosthook;
    XtResourceList	ext_resources;
    XtResourceList	compiled_ext_resources;
    Cardinal		num_ext_resources;
    Boolean		use_sub_resources;
#endif /* NO_CLASS_PART_INIT_HOOK */
}XmBaseClassExtRec, *XmBaseClassExt;

typedef struct _XmWidgetExtDataRec{
    Widget		widget;
    Widget		reqWidget;
    Widget		oldWidget;
}XmWidgetExtDataRec, *XmWidgetExtData;

typedef struct _XmWrapperDataRec{
    XtInitProc		initializeLeaf;
    XtSetValuesFunc	setValuesLeaf;
    XtArgsProc		getValuesLeaf;
    Cardinal		initializeNestingLevel;
    Cardinal		setValuesNestingLevel;
    Cardinal		getValuesNestingLevel;
    XtRealizeProc	realize;
#ifndef NO_CLASS_PART_INIT_HOOK
    XtWidgetClassProc	classPartInitLeaf;
#endif /* NO_CLASS_PART_INIT_HOOK */
}XmWrapperDataRec, *XmWrapperData;

#ifndef BASECLASS
externalref XrmQuark		XmQmotif;
externalref int 	_XmInheritClass;
#endif

#ifdef _NO_PROTO
extern void _XmPushWidgetExtData();
extern void _XmPopWidgetExtData();
extern Boolean _XmIsSlowSubclass ();
extern XmGenericClassExt *_XmGetClassExtensionPtr ();
extern XmWrapperData _XmGetWrapperData ();
extern void _XmFreeWrapperData ();
extern XmWidgetExtData _XmGetWidgetExtData ();
extern void _XmFreeWidgetExtData ();
extern void _XmBaseClassPartInitialize ();
extern void _XmInitializeExtensions ();
#else /* _NO_PROTO */
extern void _XmPushWidgetExtData(Widget w, XmWidgetExtData data, unsigned char extType);
extern void _XmPopWidgetExtData(Widget w, XmWidgetExtData *dataRtn, unsigned char extType);
extern Boolean _XmIsSlowSubclass (WidgetClass wc, unsigned int bit);
extern XmGenericClassExt *_XmGetClassExtensionPtr (XmGenericClassExt *listHeadPtr, XrmQuark owner);
extern XmWrapperData _XmGetWrapperData (WidgetClass class);
extern void _XmFreeWrapperData (WidgetClass class);
extern XmWidgetExtData _XmGetWidgetExtData (Widget widget, unsigned char extType);
extern void _XmFreeWidgetExtData (Widget widget);
extern void _XmBaseClassPartInitialize (WidgetClass wc);
extern void _XmInitializeExtensions (void);
#endif /* _NO_PROTO */

#endif /* _BaseClassI_h */
