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
#ifndef lint	/* BuildSystemHeader added automatically */
static char *BuildSystemHeader= "$Header: /alphabits/u3/x11/ode/rcs/x11/src/motif/lib/Xm/Vendor.c,v 1.1.2.5 93/01/25 17:15:41 Peter_Derr Exp $";	/* BuildSystemHeader */
#endif		/* BuildSystemHeader */
#ifdef REV_INFO
#ifndef lint
static char SCCSID[] = "OSF/Motif: @(#)Vendor.c	3.26.1.7 91/06/20";
#endif /* lint */
#endif /* REV_INFO */
/******************************************************************************
*******************************************************************************
*
*  (c) Copyright 1989, 1990, 1991 OPEN SOFTWARE FOUNDATION, INC.
*  (c) Copyright 1987, 1988, 1989, 1990, HEWLETT-PACKARD COMPANY
*  (c) Copyright 1987, 1988, DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASS.
*  (c) Copyright 1988 MASSACHUSETTS INSTITUTE OF TECHNOLOGY
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
*  (c) Copyright 1989, 1990, 1991 Open Software Foundation, Inc.  Unpublished - all
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
*  (c) Copyright 1989, 1990, 1991 Open Software Foundation, Inc.
*  ALL RIGHTS RESERVED 
*  
*  
* Open Software Foundation is a trademark of The Open Software Foundation, Inc.
* OSF is a trademark of Open Software Foundation, Inc.
* OSF/Motif is a trademark of Open Software Foundation, Inc.
* Motif is a trademark of Open Software Foundation, Inc.
* X Window System is a trademark of the Massachusetts Institute of Technology
*
*******************************************************************************
******************************************************************************/
/* Make sure all wm properties can make it out of the resource manager */

#ifdef hpux             /* For HP systems only */
#ifndef _NO_PROTO
#define _INCLUDE_HPUX_SOURCE
#endif
#endif

#ifdef VMS
#include <stdio>
#include <string>
#else
#include <pwd.h>
#include <stdio.h>
#include <ctype.h>
#include <sys/param.h>
#endif

/*#include <X11/Xlibos.h>*/
#ifdef DEC_MOTIF_BUG_FIX
#include <Xm/XmP.h>
#else
#include "XmP.h"
#endif
#include <X11/ShellP.h>
#include <Xm/VendorEP.h>
#include "BaseClassI.h"
#include <Xm/AtomMgr.h>
#include <Xm/Protocols.h>

#ifdef DEC_MOTIF_EXTENSION
#include <X11/XUICompat.h>
#endif

#include <X11/Xmu/Editres.h>

#ifdef VAXC
#pragma noinline (_XmFilterResources,\
		  IsRealizeWrapper,\
		  AddToGrabList,\
		  GetNextShell,\
		  VendorExtSetValues)
#endif

#ifdef VMS
externalref XmVendorShellExtClassRec 	xmVendorShellExtClassRec;
externalref WidgetClass xmVendorShellExtObjectClass;
externalref WidgetClass xmShellExtObjectClass;
#ifndef XmIsShellExt
#define XmIsShellExt(w)	XtIsSubclass(w, xmShellExtObjectClass)
#endif /* XmIsShellExt */
externalref WidgetClass xmDesktopObjectClass;
#ifndef XmIsDesktopObject
#define XmIsDesktopObject(w)	XtIsSubclass(w, xmDesktopObjectClass)
#endif /* XmIsDesktopObject */
externalref WidgetClass xmDisplayObjectClass;
#ifndef XmIsDisplayObject
#define XmIsDisplayObject(w)	XtIsSubclass(w, xmDisplayObjectClass)
#endif /* XmIsDisplayObject */
externalref WidgetClass xmWorldObjectClass;
#ifndef XmIsWorldObject
#define XmIsWorldObject(w)	XtIsSubclass(w, xmWorldObjectClass)
#endif /* XmIsWorldObject */
#endif

#define DONT_CARE -1

#ifndef _XA_WM_DELETE_WINDOW
#define	_XA_WM_DELETE_WINDOW 	"WM_DELETE_WINDOW"
#endif /* _XA_WM_DELETE_WINDOW */
    
/* forward declarations for internal functions */

static void 	SetMwmHints();
static Status 	GetMwmHints();
static void 	SetMwmMenu();
static void     RemoveGrabCallback();

static void GetCleanArgList();

/***************************************************************************
 *
 * Vendor shell class record
 *
 ***************************************************************************/

#define Offset(field) (XtOffsetOf(WMShellRec, field))

static int default_unspecified_shell_int = XtUnspecifiedShellInt;
/*
 * Warning, casting XtUnspecifiedShellInt (which is -1) to an (XtPointer)
 * can result is loss of bits on some machines (i.e. crays)
 */

static XtResource resources[] =
{
    {
	XmNx, XmCPosition, XmRShellHorizPos, sizeof(Position),
	XtOffset (Widget, core.x), XmRImmediate, (caddr_t) 0,
    },
    
    {
	XmNy, XmCPosition, XmRShellVertPos, sizeof(Position),
	XtOffset (Widget, core.y), XmRImmediate, (caddr_t) 0,
    },
    
    {
	XmNwidth, XmCDimension, XmRShellHorizDim, sizeof(Dimension),
	XtOffset (Widget, core.width), XmRImmediate, (caddr_t) 0,
    },
    
    {
	XmNheight, XmCDimension, XmRShellVertDim, sizeof(Dimension),
	XtOffset (Widget, core.height), XmRImmediate, (caddr_t) 0,
    },
    
    {
	XmNborderWidth, XmCBorderWidth, XmRShellHorizDim, sizeof(Dimension),
	XtOffset (Widget, core.border_width), XmRImmediate, (caddr_t) 0,
    },
    
    /* size_hints minus things stored in core */
    { XmNbaseWidth, XmCBaseWidth, XmRHorizontalInt, sizeof(int),
	Offset(wm.base_width),
	XmRHorizontalInt, (XtPointer) &default_unspecified_shell_int},
    { XmNbaseHeight, XmCBaseHeight, XmRVerticalInt, sizeof(int),
	Offset(wm.base_height),
	XmRVerticalInt, (XtPointer) &default_unspecified_shell_int},
    { XmNminWidth, XmCMinWidth, XmRHorizontalInt, sizeof(int),
	Offset(wm.size_hints.min_width),
	XmRHorizontalInt, (XtPointer) &default_unspecified_shell_int},
    { XmNminHeight, XmCMinHeight, XmRVerticalInt, sizeof(int),
	Offset(wm.size_hints.min_height),
	XmRVerticalInt, (XtPointer) &default_unspecified_shell_int},
    { XmNmaxWidth, XmCMaxWidth, XmRHorizontalInt, sizeof(int),
	Offset(wm.size_hints.max_width),
	XmRHorizontalInt, (XtPointer) &default_unspecified_shell_int},
    { XmNmaxHeight, XmCMaxHeight, XmRVerticalInt, sizeof(int),
	Offset(wm.size_hints.max_height),
	XmRVerticalInt, (XtPointer) &default_unspecified_shell_int},
    { XmNwidthInc, XmCWidthInc, XmRHorizontalInt, sizeof(int),
	Offset(wm.size_hints.width_inc),
	XmRHorizontalInt, (XtPointer) &default_unspecified_shell_int},
    { XmNheightInc, XmCHeightInc, XmRVerticalInt, sizeof(int),
	Offset(wm.size_hints.height_inc),
	XmRVerticalInt, (XtPointer) &default_unspecified_shell_int},
    { XmNminAspectX, XmCMinAspectX, XmRHorizontalInt, sizeof(int),
	Offset(wm.size_hints.min_aspect.x),
	XmRHorizontalInt, (XtPointer) &default_unspecified_shell_int},
    { XmNminAspectY, XmCMinAspectY, XmRVerticalInt, sizeof(int),
	Offset(wm.size_hints.min_aspect.y),
	XmRVerticalInt, (XtPointer) &default_unspecified_shell_int},
    { XmNmaxAspectX, XmCMaxAspectX, XmRHorizontalInt, sizeof(int),
	Offset(wm.size_hints.max_aspect.x),
	XmRHorizontalInt, (XtPointer) &default_unspecified_shell_int},
    { XmNmaxAspectY, XmCMaxAspectY, XmRVerticalInt, sizeof(int),
	Offset(wm.size_hints.max_aspect.y),
	XmRVerticalInt, (XtPointer) &default_unspecified_shell_int},
    /* wm_hints */
    { XmNiconPixmap, XmCIconPixmap, XmRPixmap, sizeof(Pixmap),
	Offset(wm.wm_hints.icon_pixmap), XmRPixmap, NULL},
    { XmNiconX, XmCIconX, XmRHorizontalInt, sizeof(int),
	Offset(wm.wm_hints.icon_x),
	XmRHorizontalInt, (XtPointer) &default_unspecified_shell_int},
    { XmNiconY, XmCIconY, XmRVerticalInt, sizeof(int),
	Offset(wm.wm_hints.icon_y),
	XmRVerticalInt, (XtPointer) &default_unspecified_shell_int},

    { /* override dec default */
	XmNinput, XmCInput, XmRBool, 
	sizeof(Bool), Offset(wm.wm_hints.input), 
	XmRImmediate, (caddr_t) TRUE,
    },
    { /* override incorrect default in Shell.c */
	XmNwindowGroup, XmCWindowGroup, XmRWindow, sizeof(Window),
	Offset(wm.wm_hints.window_group),
	XmRImmediate, (XtPointer)XtUnspecifiedWindowGroup,
    },
};	
#undef Offset

/* Foward reference for class routines */

static void		ClassInitialize();
static void		ClassPartInitialize();
static void 		Initialize();
static void		InsertChild();
static Boolean 		SetValues();
static void		Resize();
static void 		Realize();
static void		GetValuesHook();
static void		Destroy();
static XtGeometryResult	GeometryManager();
static void 		ChangeManaged();
static void		InitializePrehook();
static Boolean		SetValuesPrehook();
static void		InitializePosthook();
static Boolean		SetValuesPosthook();
static void		RealizeWrapper();

static CompositeClassExtensionRec compositeClassExtRec = {
    NULL,
    NULLQUARK,
    XtCompositeExtensionVersion,
    sizeof(CompositeClassExtensionRec),
    TRUE,
};

extern XtGeometryResult 	_XmRootGeometryManager();

static ShellClassExtensionRec shellClassExtRec = {
    NULL,
    NULLQUARK,
    XtShellExtensionVersion,
    sizeof(ShellClassExtensionRec),
    _XmRootGeometryManager
};

/* Initialize and SetValues Pre and Post hooks */

static void InitializePrehook();
static void InitializePosthook();
static Boolean SetValuesPrehook();
static Boolean SetValuesPosthook();
static void GetValuesPrehook();
static void GetValuesPosthook();

static Widget 		SecondaryObjectCreate();
static Cardinal		GetSecResData();

static XmBaseClassExtRec baseClassExtRec = {
    NULL,
    NULLQUARK,
    XmBaseClassExtVersion,
    sizeof(XmBaseClassExtRec),
    InitializePrehook,				/* init prehook		*/
    SetValuesPrehook,				/* setval prehook	*/
    InitializePosthook,				/* init posthook	*/
    SetValuesPosthook,				/* setval posthook	*/
    (WidgetClass)&xmVendorShellExtClassRec,	/* secondary class	*/
    (XtInitProc)SecondaryObjectCreate,		/* secondary create	*/
    (XmGetSecResDataFunc)GetSecResData,		/* getSecRes data       */
    {0},					/* class flags		*/
    GetValuesPrehook,				/* get_values_prehook	*/
    GetValuesPosthook,				/* get_values_posthook  */
};

#if defined(DEC_MOTIF_VMS_LP) || defined(VMS_THIN_LAYER) || defined (__DECC) || defined (OSF1_SHARED)
dxm_externaldef(vendorshellclassrec, VendorShellClassRec, _Xm_vendorShellClassRec) = {
#else
externaldef(vendorshellclassrec) 
VendorShellClassRec vendorShellClassRec = {
#endif
    {	
	(WidgetClass) &wmShellClassRec, /* superclass 		*/   
	"VendorShell", 			/* class_name 		*/   
	sizeof(VendorShellRec), 	/* size 		*/   
	ClassInitialize, 		/* Class Initializer 	*/   
	ClassPartInitialize, 		/* class_part_init 	*/ 
	FALSE, 				/* Class init'ed ? 	*/   
	Initialize, 			/* initialize         	*/   
	NULL, 				/* initialize_hook	*/ 
	Realize, 			/* realize            	*/   
	NULL,	 			/* actions            	*/   
	0,				/* num_actions        	*/   
	resources, 			/* resources          	*/   
	XtNumber(resources), 		/* resource_count     	*/   
	NULLQUARK, 			/* xrm_class          	*/   
	FALSE, 				/* compress_motion    	*/   
	TRUE, 				/* compress_exposure  	*/   
	FALSE, 				/* compress_enterleave	*/   
	FALSE, 				/* visible_interest   	*/   
	NULL,				/* destroy            	*/   
	Resize,		 		/* resize             	*/   
	NULL, 				/* expose             	*/   
	SetValues,	 		/* set_values         	*/   
	NULL, 				/* set_values_hook      */ 
	XtInheritSetValuesAlmost, 	/* set_values_almost    */ 
	GetValuesHook,			/* get_values_hook      */ 
	NULL, 				/* accept_focus       	*/   
	XtVersion, 			/* intrinsics version 	*/   
	NULL, 				/* callback offsets   	*/   
	NULL,				/* tm_table           	*/   
	NULL, 				/* query_geometry       */ 
	NULL, 				/* display_accelerator  */ 
	(XtPointer)&baseClassExtRec,	/* extension		*/ 
    },	
    { 					/* composite_class	*/
	GeometryManager,		/* geometry_manager 	*/   
	ChangeManaged,		 	/* change_managed 	*/   
	InsertChild, 			/* insert_child 	*/   
	XtInheritDeleteChild, 		/* delete_child 	*/   
	(XtPointer)&compositeClassExtRec,/* extension            */ 
    },                           
    {                            	/* shell class		*/
	(XtPointer)&shellClassExtRec,	/* extension 		*/ 
    }, 
    {                            	/* wmshell class	*/
	NULL, 				/* extension            */ 
    }, 	
    {   				/* vendorshell class	*/
	NULL, 				/* extension            */ 
    }                            
};	   

#if defined(DEC_MOTIF_VMS_LP) || defined(VMS_THIN_LAYER) || defined (__DECC) || defined (OSF1_SHARED)
dxm_externaldef(vendorshellwidgetclass, WidgetClass, _Xm_vendorShellWidgetClass) = (WidgetClass) (&_Xm_vendorShellClassRec);
#else
externaldef(vendorshellwidgetclass) WidgetClass 
  vendorShellWidgetClass = (WidgetClass) (&vendorShellClassRec);
#endif



/************************************************************************
 *
 *  BaseProc
 *
 ************************************************************************/
static XtPointer BaseProc ( widget, client_data)
    Widget  widget;
    XtPointer client_data;
{   
    XmWidgetExtData	extData;
    Widget		secObj;

    if ((extData = _XmGetWidgetExtData(widget, XmSHELL_EXTENSION)) &&
	(secObj =extData->widget))
      return (XtPointer)secObj;
    else
      return NULL;
}

/************************************************************************
 *
 *  _XmFilterResources
 *
 ************************************************************************/
Cardinal _XmFilterResources(resources, numResources,filterClass,
filteredResourcesRtn)
    XtResource			*resources;
    Cardinal			numResources;
    WidgetClass			filterClass;
    XtResource			**filteredResourcesRtn;
{
    XtResource		*filteredResources;
    Cardinal		copyIndexes[256];
    Cardinal		filterOffset;
    Cardinal		i, j;

    filterOffset = filterClass->core_class.widget_size;

    for (i = 0, j = 0; i < numResources; i++)
      {
	  if (resources[i].resource_offset >= filterOffset)
	    {
		copyIndexes[j++] = i;
	    }
      }

    filteredResources = (XtResource *) XtMalloc(j * sizeof(XtResource));

    for (i = 0; i < j; i++)
      {
	  filteredResources[i] = resources[copyIndexes[i]];
      }
    *filteredResourcesRtn = filteredResources;
    return j;
}

/************************************************************************
 *
 *  GetSecResData
 *
 ************************************************************************/

static Cardinal	GetSecResData(class, secResDataRtn)
    WidgetClass			class;
    XmSecondaryResourceData	**secResDataRtn;
{
    XmBaseClassExt	*bcePtr;
    WidgetClass		secObjClass;
    XtResource		*origResources;
    Cardinal		origNumResources;

    if ((bcePtr = _XmGetBaseClassExtPtr(class, XmQmotif)) &&
	(bcePtr) && (*bcePtr) &&
	(secObjClass = ((*bcePtr)->secondaryObjectClass)))
      {
	  XmSecondaryResourceData	secResData;

	  if (!(secObjClass->core_class.class_inited))
	    XtInitializeWidgetClass(secObjClass);
	  
	  secResData = XtNew(XmSecondaryResourceDataRec);

	  XtGetResourceList(secObjClass,
			    &(origResources),
			    &(origNumResources));

	  secResData->num_resources = 
	    _XmFilterResources(origResources,
			       origNumResources,
			       xmShellExtObjectClass,
			       &(secResData->resources));

	  XtFree((char *)origResources);

	  secResData->name = NULL;
	  secResData->res_class = NULL;
	  secResData->client_data = NULL;
	  secResData->base_proc = BaseProc;

	  *secResDataRtn = &secResData;
	  return 1;
      }
    else
      return 0;
}

/************************************************************************
 *
 *  RealizeWrappers
 *
 ************************************************************************/

static void RealizeWrapper();

static void RealizeWrapper0(w, vmask, attr)
    Widget 		w;
    Mask 		*vmask;
    XSetWindowAttributes *attr;
{
    RealizeWrapper(w, vmask, attr, 0);
}

static void RealizeWrapper1(w, vmask, attr)
    Widget 		w;
    Mask 		*vmask;
    XSetWindowAttributes *attr;
{
    RealizeWrapper(w, vmask, attr, 1);
}

static void RealizeWrapper2(w, vmask, attr)
    Widget 		w;
    Mask 		*vmask;
    XSetWindowAttributes *attr;
{
    RealizeWrapper(w, vmask, attr, 2);
}

static void RealizeWrapper3(w, vmask, attr)
    Widget 		w;
    Mask 		*vmask;
    XSetWindowAttributes *attr;
{
    RealizeWrapper(w, vmask, attr, 3);
}

static void RealizeWrapper4(w, vmask, attr)
    Widget 		w;
    Mask 		*vmask;
    XSetWindowAttributes *attr;
{
    RealizeWrapper(w, vmask, attr, 4);
}

static void RealizeWrapper5(w, vmask, attr)
    Widget 		w;
    Mask 		*vmask;
    XSetWindowAttributes *attr;
{
    RealizeWrapper(w, vmask, attr, 5);
}

static void RealizeWrapper6(w, vmask, attr)
    Widget 		w;
    Mask 		*vmask;
    XSetWindowAttributes *attr;
{
    RealizeWrapper(w, vmask, attr, 6);
}


static void RealizeWrapper7(w, vmask, attr)
    Widget 		w;
    Mask 		*vmask;
    XSetWindowAttributes *attr;
{
    RealizeWrapper(w, vmask, attr, 7);
}

static XtRealizeProc realizeWrappers[] = {
    (XtRealizeProc) RealizeWrapper0,
    (XtRealizeProc) RealizeWrapper1,
    (XtRealizeProc) RealizeWrapper2,
    (XtRealizeProc) RealizeWrapper3,
    (XtRealizeProc) RealizeWrapper4,
    (XtRealizeProc) RealizeWrapper5,
    (XtRealizeProc) RealizeWrapper6,
    (XtRealizeProc) RealizeWrapper7,
};

static Boolean IsRealizeWrapper(realizeProc)
    XtRealizeProc	realizeProc;
{
    Cardinal	i;

    for (i = 0; i < XtNumber(realizeWrappers); i++)
      if (realizeProc == realizeWrappers[i])
	return True;
    return False;
}

static Cardinal GetRealizeDepth(wc)
    WidgetClass	wc;
{
    Cardinal i;

    for (i = 0; 
	 wc && wc != vendorShellWidgetClass;
	 i++, wc = wc->core_class.superclass) {};

    if (wc)
      return i;
#ifdef DEBUG
    else
      XtError("bad class for shell realize");
#endif /* DEBUG */
    return 0 ;
}

/************************************************************************
 *
 *  ClassInitialize
 *    Initialize the vendorShell class structure.  This is called only
 *    the first time a vendorShell widget is created.  It registers the
 *    resource type converters unique to this class.
 *
 ************************************************************************/

static void ClassInitialize()
{
    XmRegisterConverters();
    _XmInitializeExtensions();

    XtInitializeWidgetClass(xmVendorShellExtObjectClass);
    baseClassExtRec.record_type = XmQmotif;
}


/************************************************************************
 *
 *  ClassPartInitialize
 *    Set up the inheritance mechanism for the routines exported by
 *    vendorShells class part.
 *
 ************************************************************************/

static void ClassPartInitialize (wc)
    WidgetClass wc;
    
{
    CompositeWidgetClass	compWc = (CompositeWidgetClass)wc;
    CompositeWidgetClass	superWc =
(CompositeWidgetClass)wc->core_class.superclass;
    CompositeClassExtensionRec 	**compExtPtr;
    XmWrapperData		wcData;
    
    compExtPtr = (CompositeClassExtensionRec **)
&(compWc->composite_class.extension);

    compExtPtr = (CompositeClassExtensionRec **)
      _XmGetClassExtensionPtr((XmGenericClassExt *)compExtPtr, NULLQUARK);

    if (*compExtPtr == NULL) {
	CompositeClassExtensionRec 	**superExtPtr;

	superExtPtr = (CompositeClassExtensionRec **)
&(superWc->composite_class.extension);
	superExtPtr = (CompositeClassExtensionRec **)
	  _XmGetClassExtensionPtr((XmGenericClassExt *)superExtPtr, NULLQUARK);

	*compExtPtr = XtNew(CompositeClassExtensionRec);
	bcopy((char*)*superExtPtr, (char*)*compExtPtr,
sizeof(CompositeClassExtensionRec));
    }

    _XmBaseClassPartInitialize(wc);
/*
  _XmFastSubclassInit(wc)
  */
    wcData = _XmGetWrapperData(wc);

    /*
     * check if this widget was using XtInherit and got the wrapper
     * from the superclass
     */
    if (IsRealizeWrapper(wc->core_class.realize))
      {
	  XmWrapperData	scData = _XmGetWrapperData((WidgetClass)superWc);
	  
	  wcData->realize = scData->realize;
      }
    /*
     * It has declared it's own realize routine so save it
     */
    else
      {
	  wcData->realize = wc->core_class.realize;
      }
    wc->core_class.realize = (XtRealizeProc)realizeWrappers[GetRealizeDepth(wc)];
}



/************************************************************************
 *
 *  RealizeWrapper
 *
 ************************************************************************/
static void RealizeWrapper(w, vmask, attr, depth)
    Widget 		w;
    Mask 		*vmask;
    XSetWindowAttributes *attr;
    Cardinal		depth;
{
    if (XmIsVendorShell(w))
      {
	  XmWidgetExtData	extData;
	  
	  extData = _XmGetWidgetExtData(w, XmSHELL_EXTENSION);
	  
	  if (extData->widget)
	    {
		WidgetClass	wc = XtClass(w);
		XmWrapperData	wrapperData;
		Cardinal	leafDepth = GetRealizeDepth(wc);
		Cardinal	depthDiff = leafDepth - depth;
		for (;
		     depthDiff;
		     depthDiff--, wc = wc->core_class.superclass)
		  {};

		wrapperData = _XmGetWrapperData(wc);
		(*(wrapperData->realize))(w, vmask, attr);
		
		XtCallCallbacks(extData->widget,
				XmNrealizeCallback,
				NULL);
	    }
      }
#ifdef DEBUG
    else
      XtWarning("we only support realize callbacks on shells");
#endif /* DEBUG */
}



/************************************************************************
 *
 *  SetMwmStuff
 *     ov will be null when called from Initialize
 *
 ************************************************************************/
#ifdef _NO_PROTO
void	SetMwmStuff(ove, nve)
    XmVendorShellExtObject	ove, nve;
#else /* _NO_PROTO */
void SetMwmStuff (XmVendorShellExtObject ove, XmVendorShellExtObject nve)
#endif /* _NO_PROTO */
{
    Boolean		changed = FALSE;
    Widget		extParent = nve->ext.logicalParent;

    if (!ove || (ove->vendor.mwm_menu != nve->vendor.mwm_menu))
      {
	  /* make mwm_menu local */
	  if (ove && ove->vendor.mwm_menu) 
	    XtFree(ove->vendor.mwm_menu);
	  if (nve->vendor.mwm_menu)
	    nve->vendor.mwm_menu = XtNewString(nve->vendor.mwm_menu);
	  if (XtIsRealized(extParent))
	    SetMwmMenu(nve);
      }
    
    if (!ove || (ove->vendor.mwm_hints.functions !=
nve->vendor.mwm_hints.functions))
      {
	  if (nve->vendor.mwm_hints.functions == DONT_CARE)
	    nve->vendor.mwm_hints.flags &= ~MWM_HINTS_FUNCTIONS;
	  else
	    nve->vendor.mwm_hints.flags |= MWM_HINTS_FUNCTIONS;
	  changed |= TRUE;
      }
    
    if (!ove || (ove->vendor.mwm_hints.decorations !=
nve->vendor.mwm_hints.decorations))
      {
	  if (nve->vendor.mwm_hints.decorations == DONT_CARE)
	    nve->vendor.mwm_hints.flags &= ~MWM_HINTS_DECORATIONS;
	  else
	    nve->vendor.mwm_hints.flags |= MWM_HINTS_DECORATIONS;
	  changed |= TRUE;
      }
    
    if (!ove || (ove->vendor.mwm_hints.input_mode !=
nve->vendor.mwm_hints.input_mode))
      {
	  if (nve->vendor.mwm_hints.input_mode == DONT_CARE)
	    nve->vendor.mwm_hints.flags &= ~MWM_HINTS_INPUT_MODE;
	  else
	    nve->vendor.mwm_hints.flags |= MWM_HINTS_INPUT_MODE;
	  changed |= TRUE;
      }
    
    if (changed && XtIsRealized(extParent))
      SetMwmHints(nve);
}

/*
 * The AddGrab and RemoveGrab routines manage a virtual Xt modal
 * cascade that allows us to remove entries in the list without
 * flushing out the grabs of all following entries. 
 */
static void AddGrab(ve, exclusive, springLoaded, origKid)
    XmVendorShellExtObject	ve;
    Boolean			exclusive;
    Boolean			springLoaded;
    XmVendorShellExtObject	origKid;
{
    Cardinal		     	position;
    Cardinal	        	i;
    XmModalData			modals;
    XmDisplayObject		dispObj;
    ShellWidget			shell = (ShellWidget)ve->ext.logicalParent;

    dispObj = (XmDisplayObject)_XmGetDisplayObject((Widget)shell, NULL, 0);

    modals = dispObj->display.modals;
    
    position = dispObj->display.numModals;
    
    if (dispObj->display.numModals == dispObj->display.maxModals) {
	/* Allocate more space */
	dispObj->display.maxModals +=  (dispObj->display.maxModals / 2) + 2;
	dispObj->display.modals = modals = (XmModalData) 
	  XtRealloc((XtPointer) modals, (unsigned) 
		    ((dispObj->display.maxModals) * sizeof(XmModalDataRec)));
    }
    modals[position].ve = ve;
    modals[position].grabber = origKid;
    modals[position].exclusive = exclusive;
    modals[position].springLoaded = springLoaded;
    dispObj->display.numModals++;
    XtAddGrab((Widget) shell,
	      exclusive,
	      springLoaded);
    /*
     * if the shell gets destroyed, we don't have to worry about removing
     * the Xt grab, but we do have to remove the ve from the list of modals.
     * Should the client_data be ve or origKid?
     */
    XtAddCallback((Widget) shell, XmNdestroyCallback,
		  RemoveGrabCallback,(XtPointer)ve);
}

/*
 * we add a new argument here, being_destroyed.  If true, it means that
 * we are being called from a callback triggered by the destruction of a
 * shell.  If it is true, we should remove shells from the list of modals,
 * but we should not call XtRemoveGrab on them, because the intrinsics
 * already handle that.
 */
static void RemoveGrab(ve, being_destroyed)
    XmVendorShellExtObject	ve;
    Boolean being_destroyed;
{
    XmDisplayObject		dispObj;
    ShellWidget			shell = (ShellWidget)ve->ext.logicalParent;
    Cardinal		     	incr, i, numRemoves, numModals;
    XmModalData			modals;
    
    /*
     * remove the callback we added in AddGrab()
     * is the ve here the same one that we had in AddGrab?
     */
    if (!being_destroyed)
	XtRemoveCallback((Widget) shell, XmNdestroyCallback,
			 RemoveGrabCallback,(XtPointer)ve);
    
    dispObj = (XmDisplayObject) _XmGetDisplayObject((Widget)shell, NULL, 0);

    modals = dispObj->display.modals;
    numModals = dispObj->display.numModals;

    for (i = 0, numRemoves = 0;
	 i < numModals; 
	 i++) 
      {
	  if (modals[i].ve == ve) 
	    numRemoves++;
      }
    if (numRemoves == 0) return;

    if (!being_destroyed)
    for (i = 0; i < numRemoves; i++)
      XtRemoveGrab((Widget)shell);

    /* Add back all the grabs that were flushed by the removes */

    for (i = 0, incr = 0; 
	 (i + numRemoves) < numModals;
	 i++) 
      {
	  XmVendorShellExtObject incrVe;
	  /*
	   * we remove both the shell that's being pulled off the
	   * cascade and any raw mode shells that we've added. These
	   * should only be the app shells ?!
	   */
	  while ((i + incr) < numModals)
	    {
		Widget	shell;
		incrVe = modals[i+incr].ve;
		shell = incrVe->ext.logicalParent;
		if (incrVe != ve)
		  {
		      if (modals[i+incr].grabber == ve)
			numRemoves++;
		      else
			break;
		  }
		incr++;
	    }
	  if (incr && ((i+incr) < numModals))
	    {
		modals[i] = modals[i+incr];
		/*
		 * call XtAddGrab here, even if we didn't call XtRemoveGrab
		 * above.  Because the destruction of the widget will trigger
		 * appropriate XtRemoveGrabs.  (But it is implementation
		 * dependant when those XtRemoveGrabs get called...)
		 */
		XtAddGrab(modals[i].ve->ext.logicalParent,
			  modals[i].exclusive,
			  modals[i].springLoaded);
	    }
      }
    dispObj->display.numModals = 
      dispObj->display.numModals - numRemoves;
}

/* ARGSUSED */
static void RemoveGrabCallback(w, client_data, call_data)
Widget w;
XtPointer client_data;
XtPointer call_data;
{
    XmVendorShellExtObject	ve;

    ve = (XmVendorShellExtObject)client_data;
    RemoveGrab(ve, True);
}

static void AddToGrabList(parent, excludedKid, origKid)
    XmVendorShellExtObject	parent, excludedKid, origKid;
{
    XmDesktopObject	*currKid;
    Cardinal			i;

    if (!(parent->desktop.num_children))
      return;
    for (i = 0, 
	 currKid = (XmDesktopObject *)parent->desktop.children;
	 i < parent->desktop.num_children;
	 i++, currKid++)
      {
	  XmVendorShellExtObject currVe;
	  ShellWidget shell;

	  currVe = (XmVendorShellExtObject)*currKid;
	  shell = (ShellWidget)currVe->ext.logicalParent;

	  if ((currVe != excludedKid) &&
	      (shell->shell.popped_up))
	    {
		AddGrab(currVe, False, False, origKid);
		AddToGrabList(currVe, NULL, origKid);
	    }
      }
}
static void AddCousinsToGrabList(parent, excludedKid, origKid)
    XmDesktopObject	parent, excludedKid, origKid;
{
    if (XmIsWorldObject((Widget) parent))
      {
	  return;
      }
    else if (XmIsDisplayObject((Widget) parent))
      {
	  XmDesktopObject	*currKid;
	  Cardinal			i;
	  
	  for (i = 0, 
	       currKid = (XmDesktopObject *)parent->desktop.children;
	       i < parent->desktop.num_children;
	       i++, currKid++)
	    {
		XmDesktopObject currVe = (XmDesktopObject)*currKid;
		
		if (currVe != excludedKid)
		  AddToGrabList(currVe, NULL, origKid);
	    }
	  AddCousinsToGrabList(parent->desktop.parent, parent,
			       origKid);
      }
    else
      {
	  AddToGrabList(parent, excludedKid, origKid);
	  AddCousinsToGrabList(parent->desktop.parent, parent,
			       origKid);
      }
}


/************************************************************************
 *
 *     PopupCallback
 *
 ************************************************************************/
static void PopupCallback(shellParent, closure, callData)
    Widget	shellParent;
    XtPointer	closure;
    XtPointer	callData;
{
    XmVendorShellExtObject	ve = (XmVendorShellExtObject)closure;
    XtGrabKind			grab_kind;
    Boolean			grabCousins = False;
    XmScreenObject		screenObj;

    screenObj = _XmGetScreenObject(shellParent, NULL, NULL);
    ve->vendor.xAtMap = shellParent->core.x;
    ve->vendor.yAtMap = shellParent->core.y;

    /*
     * work around broken Xt spec ordering for realize and popup callback
     */
    if (!XtIsRealized(shellParent))
      XtRealizeWidget(shellParent);
    
    /* 
     * get the request num + 1 Since it's a map raised. This will
     * only work when the hierarchy is already realized, i.e. after
     * first time
     */
    
    ve->vendor.lastMapRequest = NextRequest(XtDisplay(shellParent)) + 1;
    

#ifdef notdef
    /*
     * Set the map style to popup if it's not already been set to
     * manage. This allows the right handling of wm unmaps
     */
    if (ve->vendor.mapStyle == _XmRAW_MAP)
      ve->vendor.mapStyle = _XmPOPUP_MAP;
#endif /* notdef */

    switch (ve->vendor.mwm_hints.input_mode)
      {
	case DONT_CARE:
	case MWM_INPUT_MODELESS:
	  grab_kind = XtGrabNone;
	  break;
	case MWM_INPUT_PRIMARY_APPLICATION_MODAL:
	  /*
	   * if we're not running mwm then this becomes full app modal
	   */
	  if (screenObj->screen.mwmPresent)
	    grabCousins = True;
	  grab_kind = XtGrabExclusive;
	  break;
	case MWM_INPUT_SYSTEM_MODAL:
	case MWM_INPUT_FULL_APPLICATION_MODAL:
	  grab_kind = XtGrabExclusive;
	  break;
	default:
	  break;
      }
    if (grab_kind != XtGrabNone)
      AddGrab(ve,
	      (grab_kind == XtGrabExclusive), 
	      False, 
	      ve);

    ve->vendor.grab_kind = grab_kind;
    
    if (grabCousins)
      AddCousinsToGrabList(ve->desktop.parent, ve, ve);
}
/************************************************************************
 *
 *     PopdownCallback
 *
 ************************************************************************/
static void PopdownCallback(shellParent, closure, callData)
    Widget	shellParent;
    XtPointer	closure;
    XtPointer	callData;
{
    XmVendorShellExtObject	ve = (XmVendorShellExtObject)closure;
    
    if (ve->vendor.grab_kind != XtGrabNone)
      RemoveGrab(ve, False);
}
static Widget GetNextShell(vw)
    Widget	vw;
{
    Widget 	parent;
    
    for (parent = XtParent(vw);
	 parent && !XmIsVendorShell(parent);
	 parent = XtParent(parent)) ;

    return parent;
}
static XmDesktopObject GetShellDesktopParent(vw, args, num_args)
    VendorShellWidget	vw;
    ArgList		args;
    Cardinal		*num_args;
{
    Widget			transientParent = NULL;
    XmDesktopObject		desktopParent = NULL;
    XmScreenObject		screenObj;


    if (vw->wm.transient)
      {
	  if (XtIsSubclass((Widget) vw, transientShellWidgetClass))
	    {
		TransientShellWidget tw = (TransientShellWidget)vw;

		  if (!(transientParent = tw->transient.transient_for))
		    {
			tw->transient.transient_for =
			  transientParent = 
			    GetNextShell(vw);
		    }
	    }
	  else
	    {
		transientParent = GetNextShell(vw);
	    }
      }
 
    if (transientParent)
      {
	  XmWidgetExtData	extData;
	  
	  if (XmIsVendorShell(transientParent))
	    {
		extData = _XmGetWidgetExtData(transientParent, XmSHELL_EXTENSION);
		desktopParent = (XmDesktopObject)extData->widget;
	    }
      }
    else
      {
	  desktopParent =  (XmDesktopObject)_XmGetScreenObject((Widget)vw, args, num_args);
      }
     return desktopParent;
}

/************************************************************************
 *
 *  SecondaryObjectCreate
 *
 ************************************************************************/
/* ARGSUSED */
static Widget SecondaryObjectCreate(req, new, args, num_args)
    Widget	req, new;
    ArgList	args;
    Cardinal	*num_args;
{
    XmBaseClassExt		*sePtr, *pePtr;
    Arg				myArgs[4];
    Cardinal			numMyArgs = 0;
    ArgList			mergedArgs;
    VendorShellWidget		shell = (VendorShellWidget)new;
    XmExtObjectClass		vec;
    XmDesktopObject		desktopParent;
    XmDesktopObjectClass	doc;
    Widget			secObj = NULL;
    Widget			parent;
    ArgList                     pCleanArgs;
    Cardinal                    nCleanArgs;
    static Arg                  badArgs[] = {{XmNdestroyCallback,0}};
    Cardinal                    nBadArgs = sizeof(badArgs) / sizeof(Arg);

    GetCleanArgList(args, *num_args,
                    badArgs, nBadArgs,
                    &pCleanArgs, &nCleanArgs);
    
    desktopParent = GetShellDesktopParent((VendorShellWidget) new,
                                                              pCleanArgs,
                                                              &nCleanArgs);

    if (desktopParent)
      {
	  doc = (XmDesktopObjectClass)XtClass(desktopParent);
	  
	  XtSetArg(myArgs[numMyArgs] ,XmNlogicalParent, new);
	  numMyArgs++;
	  XtSetArg(myArgs[numMyArgs] ,XmNdesktopParent, desktopParent);
	  numMyArgs++;
	  
          if (nCleanArgs)
            mergedArgs = XtMergeArgLists(myArgs, numMyArgs,
                                         pCleanArgs, nCleanArgs);
          else
            mergedArgs = myArgs;

	  /*
	   * if the secondary object is using sub_resources then
	   * create it as a child of the shell. Otherwise try to
	   * create it as a sibling in order to fake out resource path
	   */
	  pePtr = _XmGetBaseClassExtPtr(XtClass(new), XmQmotif);
	  vec = (XmExtObjectClass)(*pePtr)->secondaryObjectClass;

	  /*
	   * make sure that the class has already been initialized so
	   * we can dereference it. 
	   */
	  XtInitializeWidgetClass((WidgetClass) vec);
	  sePtr = _XmGetBaseClassExtPtr(vec, XmQmotif);

#ifndef NO_CLASS_PART_INIT_HOOK
	  if ((*sePtr)->use_sub_resources)
	    parent = new;
	  else
#endif /* NO_CLASS_PART_INIT_HOOK */
	    parent = XtParent(new) ? XtParent(new) : new;
	  
	  secObj = XtCreateWidget(XtName(new),
				  (WidgetClass) vec,
				  parent,
				  mergedArgs, 
				  nCleanArgs + numMyArgs);
	  
	  if (mergedArgs != myArgs)
	    XtFree((char *)mergedArgs);

          if (nCleanArgs)
              XtFree((char *) pCleanArgs);
      }
#ifdef DEBUG
    else
      XtWarning("no desktop parent");
#endif /* DEBUG */    

    return secObj;

}

/************************************************************************
 *
 *  InitializePrehook
 *
 ************************************************************************/
/* ARGSUSED */
static void InitializePrehook(req, new, args, num_args)
    Widget	req, new;
    ArgList	args;
    Cardinal	*num_args;
{
    XmBaseClassExt		*cePtr;
    XmSecObjCreateFunc		secondaryCreate;
    VendorShellWidget		vw = (VendorShellWidget)new;
    XmVendorShellExtObject	ve;

    cePtr = _XmGetBaseClassExtPtr(XtClass(new), XmQmotif);

    if (secondaryCreate = (XmSecObjCreateFunc)(*cePtr)->secondaryObjectCreate)
      ve = (XmVendorShellExtObject)
	(*secondaryCreate)(req, new, args, num_args);
}


/************************************************************************
 *
 *     VendorExtInitialize
 *
 ************************************************************************/
static void VendorExtInitialize(req, new, args, num_args)
    Widget	req, new;
    ArgList	args;
    Cardinal	*num_args;
{
    XmVendorShellExtObject	ve, ove;
    XmVendorShellExtObjectClass	vec = (XmVendorShellExtObjectClass) XtClass(new);
    Atom			delete_atom;
    XtCallbackProc		delete_window_handler;
    Widget			extParent;
    Atom			offset_atom, mwm_messages;
    XmShellExtObjectClass	sec = (XmShellExtObjectClass) XtClass(new);
    XtEventHandler		handler;
    extern void			_XmTrackShellFocus();

    ove = (XmVendorShellExtObject) req;
    ve  = (XmVendorShellExtObject) new;

    ve->shell.lastConfigureRequest = 0;
	
    extParent = ve->ext.logicalParent;

    /* add the handler for tracking whether the hierarchy has focus */

    XtInsertEventHandler(extParent, 
			 (EventMask)FocusChangeMask | EnterWindowMask |
LeaveWindowMask,
			 FALSE,
			 _XmTrackShellFocus, 
			 (XtPointer)new,
			 XtListHead);

    handler = sec->shell_class.structureNotifyHandler;
    if (handler)
      {
	  XtInsertEventHandler(extParent, 
			       (EventMask) StructureNotifyMask,
			       TRUE, 
			       handler, 
			       (XtPointer)new,
			       XtListHead);
      }
#ifdef DEBUG
    else
      XtError("No structure notify handler for shell");
#endif /* DEBUG */

    ve->vendor.lastOffsetSerial =
      ve->vendor.lastMapRequest = 0;

    ve->vendor.xAtMap =
      ve->vendor.yAtMap =
	ve->vendor.xOffset =
	  ve->vendor.yOffset = 0;

    XtAddCallback((Widget) ve, XmNrealizeCallback,
		  _XmVendorExtRealize,
		  NULL);

    ve->vendor.externalReposition = False;
#ifdef notdef
    ve->vendor.mapStyle = _XmRAW_MAP;
#endif /* notdef */
    extParent = ve->ext.logicalParent;

    _XmVirtKeysInitialize(extParent);

    ve->vendor.focus_data = (XmFocusData) _XmCreateFocusData();

    switch (ve->vendor.delete_response){
      case XmUNMAP:
      case XmDESTROY:
      case XmDO_NOTHING:
	break;
      default:
	XtWarning("invalid value for delete response");
	ve->vendor.delete_response = XmDESTROY;
    }

    XtAddCallback(extParent, XmNpopupCallback, PopupCallback,(XtPointer)new); 
    XtAddCallback(extParent, XmNpopdownCallback, PopdownCallback,(XtPointer)new); 

    offset_atom = XmInternAtom(XtDisplay(extParent), 
			       _XA_MOTIF_WM_OFFSET, 
			       FALSE);

    mwm_messages = XmInternAtom(XtDisplay(extParent), 
				_XA_MOTIF_WM_MESSAGES, 
				FALSE),

    delete_atom = XmInternAtom(XtDisplay(extParent), 
			       _XA_WM_DELETE_WINDOW,
			       FALSE);

    XmAddWMProtocols(extParent, &mwm_messages, 1);

    XmAddProtocols(extParent,
		   mwm_messages,
		   &offset_atom, 1);

    XmAddProtocolCallback(extParent,
			  mwm_messages, 
			  offset_atom,
			  vec->vendor_class.offset_handler,
			  (caddr_t)ve);
    
    /*
     * add deleteWindow stuff
     */
    XmAddWMProtocols(extParent, &delete_atom, 1);

    /* add a post hook for delete response */

    delete_window_handler = vec->vendor_class.delete_window_handler;

    XmSetWMProtocolHooks(extParent, 
			 delete_atom, NULL, NULL, 
			 delete_window_handler, (caddr_t)ve);


    /* initialize the old_managed field for focus change tracking */

    ve->vendor.old_managed = NULL;

    /* initialize the mwm_hints flags */
    ve->vendor.mwm_hints.flags = 0;

    SetMwmStuff(NULL, (XmVendorShellExtObject)new);

    if ((ve->vendor.focus_policy != XmEXPLICIT) &&
	(ve->vendor.focus_policy != XmPOINTER))
      {
	  ve->vendor.focus_policy = XmEXPLICIT;
      }

/* I18N START */

#ifdef DEC_MOTIF_EXTENSION
    if ( ve->vendor.default_font_list == (XmFontList) NULL ){
	ve->vendor.default_font_list =
		(XmFontList) XmFontListCreateDefault (ve, NULL);
#ifdef DEBUG
	printf("I18n default font is set in vendor shell widget\n");
    } else {
	printf("Use default font in resource for vendor shell\n");
#endif
    }
#endif /* DEC_MOTIF_EXTENSION */

/* I18N END */
}

/************************************************************************
 *
 *  Initialize
 *
 ************************************************************************/
/* ARGSUSED */
static void Initialize(req, new, args, num_args)
    Widget	req, new;
    ArgList	args;
    Cardinal	*num_args;
{
    VendorShellWidget		vw = (VendorShellWidget)new;
    Widget			ve;
    XmWidgetExtData		extData;

    if ((extData = _XmGetWidgetExtData((Widget)vw, XmSHELL_EXTENSION)) &&
	(ve = extData->widget))
      {
	  VendorExtInitialize(extData->reqWidget,
			      extData->widget,
			      args,
			      num_args);
      }
    XtAddEventHandler(new, (EventMask) 0, TRUE, _XEditResCheckMessages,
				(XtPointer) NULL);
}

/************************************************************************
 *
 *  InitializePosthook
 *
 ************************************************************************/
/* ARGSUSED */
static void InitializePosthook(req, new, args, num_args)
    Widget	req, new;
    ArgList	args;
    Cardinal	*num_args;
{
    XmWidgetExtData	ext;

    ext = _XmGetWidgetExtData(new, XmSHELL_EXTENSION);
    XtFree((char *) ext->reqWidget);
    ext->reqWidget = NULL;
    /* extData gets freed at destroy */
}



static void InsertChild(widget)
    Widget widget;
{
    if (! XtIsWidget(widget))
      {
	  if (XtIsRectObj(widget)) {
	      XtAppWarningMsg(XtWidgetToApplicationContext(widget),
			      "invalidClass", "shellInsertChild", "XtToolkitError",
			      "Shell does not accept RectObj children; ignored",
			      (String*)NULL, (Cardinal*)NULL);
	  }
	  else
	    return;
      }
    else {
	(*((CompositeWidgetClass)vendorShellClassRec.core_class.
	   superclass)->composite_class.insert_child) (widget);
    }
}


/************************************************************************
 *
 *  SetValuesPrehook
 *
 ************************************************************************/
/* ARGSUSED */
static Boolean SetValuesPrehook(old,ref,new, args, num_args)
    Widget	old, ref, new;
    ArgList	args;
    Cardinal	*num_args;
{
    XmWidgetExtData	oldExtData, newExtData;


    oldExtData = _XmGetWidgetExtData(new, XmSHELL_EXTENSION);
    newExtData = (XmWidgetExtData) XtCalloc(sizeof(XmWidgetExtDataRec), 1);
    
    _XmPushWidgetExtData(new, newExtData, XmSHELL_EXTENSION);

    XtSetValues(oldExtData->widget, args, *num_args);
    return FALSE;
}

/************************************************************************
 *
 *  SetValues
 *
 ************************************************************************/
/* ARGSUSED */
static Boolean VendorExtSetValues(old,ref,new, args, num_args)
    Widget	old, ref, new;
    ArgList	args;
    Cardinal	*num_args;
{
    XmVendorShellExtPartPtr	ove, nve;
    XmVendorShellExtObject 	ov = (XmVendorShellExtObject) old;
    XmVendorShellExtObject 	nv = (XmVendorShellExtObject) new;

    ove = (XmVendorShellExtPartPtr) &(ov->vendor);
    nve = (XmVendorShellExtPartPtr) &(nv->vendor);

    switch (nve->delete_response){
      case XmUNMAP:
      case XmDESTROY:
      case XmDO_NOTHING:
	break;
      default:
	XtWarning("invalid value for delete response");
	nve->delete_response = XmDESTROY;
    }

    if ((nve->focus_policy != XmEXPLICIT) &&
	(nve->focus_policy != XmPOINTER))
      {
	  nve->focus_policy = 
	    ove->focus_policy;
      }
    
    if (nve->focus_policy != 
	ove->focus_policy)
      {
	  _XmFocusModelChanged((ShellWidget)nv->ext.logicalParent, 
			       NULL, 
			       (caddr_t)nve->focus_policy);
      }
    
    SetMwmStuff(ov, nv);

    return FALSE;
}

/************************************************************************
 *
 *  SetValues
 *
 ************************************************************************/
static Boolean SetValues(current, req, new, args, num_args)
    Widget	current, req, new;
    ArgList	args;
    Cardinal	*num_args;
{

    VendorShellWidget		vw = (VendorShellWidget)new;
    VendorShellWidget		ovw = (VendorShellWidget)current;
    Widget			ve;
    XmWidgetExtData		extData;

    if ((extData = _XmGetWidgetExtData((Widget)vw, XmSHELL_EXTENSION)) &&
	(ve = extData->widget))
      {
	  VendorExtSetValues(extData->oldWidget,
			     extData->reqWidget,
			     extData->widget,
			     args,
			     num_args);
      }
#ifdef notyet
    if (ovw->transient.transient_for != vw->transient.transient_for)
      {
	  /* update the transient_for tree */
      }
#endif /* notyet */
}


/************************************************************************
 *
 *  SetValuesPosthook
 *
 ************************************************************************/
static Boolean SetValuesPosthook(current, req, new, args, num_args)
    Widget	current, req, new;
    ArgList	args;
    Cardinal	*num_args;
{
    XmWidgetExtData	ext;

    _XmPopWidgetExtData(new, &ext, XmSHELL_EXTENSION);

    XtFree((char *)ext->reqWidget);
    XtFree((char *)ext->oldWidget);
    XtFree((char *)ext);
}


/************************************************************************
 *
 *  GetValuesPrehook
 *
 ************************************************************************/
static void GetValuesPrehook(w, args, num_args)
    Widget w;
    ArgList args;
    Cardinal * num_args;
{
    XmWidgetExtData	oldExtData, newExtData;

    oldExtData = _XmGetWidgetExtData(w, XmSHELL_EXTENSION);
    newExtData = (XmWidgetExtData) XtCalloc(sizeof(XmWidgetExtDataRec), 1);
    newExtData->widget = oldExtData->widget;

    _XmPushWidgetExtData(w, newExtData, XmSHELL_EXTENSION);
}

static void GetValuesHook(w, args, num_args)
    Widget w;
    ArgList args;
    Cardinal * num_args;
{
    XmWidgetExtData	ext;

    ext = _XmGetWidgetExtData(w, XmSHELL_EXTENSION);

    XtGetValues(ext->widget, args, *num_args);
}


static void GetValuesPosthook(w, args, num_args)
    Widget w;
    ArgList args;
    Cardinal * num_args;
{
    XmWidgetExtData	ext;

    _XmPopWidgetExtData(w, &ext, XmSHELL_EXTENSION);
    XtFree((char *)ext);
}

static void SetTransientFor();

/*
 * This handles the case where the secondary shells is waiting for the
 * primary to get mapped and is destroyed in the interim.
 */
static void PendingTransientDestroyed(vw, cl_data, ca_data)
    Widget	vw;
    XtPointer	cl_data, ca_data;
{
    Widget	primary	= (Widget) cl_data;

    if (!primary->core.being_destroyed)
      XtRemoveEventHandler(primary, StructureNotifyMask, FALSE, 
			   SetTransientFor, vw);
}

/*
 * Handle having the application shell realized after the secondary shells
 */
static void SetTransientFor(w, client_data, event)
    Widget	w;
    XtPointer	client_data;
    XEvent      *event;
{
    if (event->type == MapNotify)
      {
	  Arg		args[2];
	  Cardinal	i = 0;

	  XtSetArg(args[i], XtNwindowGroup, XtWindow(w));i++;
	  if (XtIsTransientShell((Widget)client_data))
	    {
		TransientShellWidget	source_w ;

		source_w = (TransientShellWidget)client_data;
		/* because Shell.c is broken force the code */
		source_w->transient.transient_for = NULL;

		XtSetArg(args[i], XtNtransientFor, w); i++;
	    }
	  XtSetValues((Widget)client_data, args, i);
	  
	  XtRemoveEventHandler(w, StructureNotifyMask, FALSE, 
			       (XtEventHandler) SetTransientFor, client_data);
	  XtRemoveCallback((Widget)client_data, 
			   XmNdestroyCallback, 
			   PendingTransientDestroyed,
			   (XtPointer)w);
      }
}


/************************************************************************
 *
 *  ChangeManaged
 *
 ************************************************************************/
static void Resize(w)
    Widget w;
{
    register ShellWidget sw = (ShellWidget)w;    
    Widget childwid;
    int i;
    for(i = 0; i < sw->composite.num_children; i++) {
#ifdef DEC_MOTIF_BUG_FIX
	/*
	 *  Since ConfigureNotify's arrive asynchrounsly, it's
	 *  possible for one to arrive when the child is unmanaged.
	 *  However, the child's size still must be adjusted.
	 */
        if(XtIsRectObj(sw->composite.children[i])) {
#else
        if(XtIsManaged(sw->composite.children[i])) {
#endif
	    childwid = sw->composite.children[i];
	    XtResizeWidget(childwid, sw->core.width, sw->core.height,
                           childwid->core.border_width);
        }
    }
}

/************************************************************************
 *
 *  ChangeManaged
 *
 ************************************************************************/
static void ChangeManaged(vw)
    VendorShellWidget vw;
{
    WMShellWidgetClass	super = (WMShellWidgetClass)wmShellWidgetClass;
    Widget		firstManaged = NULL;
    Cardinal		i;

    for (i= 0; i < vw->composite.num_children; i++)
      if (XtIsManaged(vw->composite.children[i]))
	firstManaged = vw->composite.children[i];

    (*(super->composite_class.change_managed)) ((Widget) vw);

    /*
     * make sure that there is a reasonable initial focus path. This
     * is especially important for making sure the event handler is
     * there.
     */
    XtSetKeyboardFocus((Widget)vw, (Widget)firstManaged);
    
#ifdef notdef
    /*
     * if focus policy is pointer then make sure there are no
     * confusing focus events forwarded
     */
    if (_XmGetFocusPolicy((Widget)vw) == XmPOINTER)
      {
	  XtSetKeyboardFocus((Widget)vw, None);
      }
    else 
#endif /* notdef */
if (firstManaged)
      {
	  Arg			arg;
	  unsigned char		navigationType;

	  XtSetArg(arg, XmNnavigationType, &navigationType);
	  XtGetValues(firstManaged, &arg, 1);

	  /*
	   * hack to make sure we have something to forward to 
	   */
	  switch(navigationType)
	    {
	      case XmNONE:
	      case XmTAB_GROUP:
		XtSetArg(arg, XmNnavigationType, XmSTICKY_TAB_GROUP);
		XtSetValues(firstManaged, &arg, 1);
		break;

	    }
      }
}
 
static void UpdateCoreGeometry(vw, vendorExt)
    VendorShellWidget		vw;
    XmVendorShellExtObject	vendorExt;
{
   /* ||| check if geometry was user specified and convert if it was */
    if (vw->shell.geometry)
      {

	  if (vendorExt->vendor.unit_type != XmPIXELS)
	    {
		if (vw->wm.size_hints.flags & USPosition)
		  {
		      vw->core.x = (Position)
			_XmToHorizontalPixels((Widget)vendorExt,
					      NULL,
					      (XtArgVal*)&vw->core.x);
		      vw->core.y = (Position)
			_XmToVerticalPixels((Widget)vendorExt, 
					    NULL,
					    (XtArgVal*)&vw->core.y);
		  }
		if (vw->wm.size_hints.flags & USSize)
		  {
		      vw->core.width = (Dimension)
			_XmToHorizontalPixels((Widget)vendorExt, 
					      NULL,
					      (XtArgVal*)&vw->core.width);

		      vw->core.height = (Dimension)
			_XmToVerticalPixels((Widget)vendorExt, 
					    NULL,
					    (XtArgVal*)&vw->core.height);
		  }
	    }
      }
}
/************************************************************************
 *
 *  Realize
 *
 ************************************************************************/
static void Realize(vw, vmask, attr)
    VendorShellWidget vw;
    Mask *vmask;
    XSetWindowAttributes *attr;
{
    WMShellWidgetClass	super = (WMShellWidgetClass)wmShellWidgetClass;
    XmVendorShellExtObject vendorExt;
    XmWidgetExtData	extData;

    extData = _XmGetWidgetExtData((Widget)vw, XmSHELL_EXTENSION);
    vendorExt = (XmVendorShellExtObject) extData->widget;

#ifdef notdef
    /* ||| Call Shell's (via wmShell) changeManaged routine so that we
     * get the geometry string parsed and other misc stuff done. This
     * should not be in shell's changeManaged routine. We be confused
     * if child is not yet instantiated.  ||| 
     */
    (*(super->composite_class.change_managed)) (vw);
#endif
    
    UpdateCoreGeometry(vw, vendorExt);
    
    /*
     * Set nearest shell as transientFor so Mwm will be able to build tree.
     */
#ifdef DEC_MOTIF_EXTENSION
    /*
     * Don't do this for an XUI hidden shell.  It changes XUI dialog
     * box behavior (e.g. dialog boxes iconify with their parent which
     * did not before...)
     */
    if (XmIsShellExt(vendorExt->desktop.parent) &&
	(strcmp("DwtHiddenShell", vw->core.widget_class->core_class.class_name) != 0))
#else
    if (XmIsShellExt(vendorExt->desktop.parent))
#endif
      {  
	  Widget	ancestor;

	  ancestor = 
	    ((XmExtObject)(vendorExt->desktop.parent))->ext.logicalParent;
	  
	  /* try to have WMShell do the work */
	  if (XtIsRealized(ancestor))
	    vw->wm.wm_hints.window_group = XtWindow(ancestor);
	  else
	    {
		XtAddEventHandler(ancestor, StructureNotifyMask, FALSE, 
				  (XtEventHandler)SetTransientFor, (caddr_t)vw);
		XtAddCallback((Widget) vw, 
			      XmNdestroyCallback, 
			      PendingTransientDestroyed,
			      ancestor);
	    }
      }
    /* Make my superclass do all the dirty work */
    
    (*super->core_class.realize) ((Widget) vw, vmask, attr);
}

/************************************************************************
 *
 *  GeometryManager
 *
 ************************************************************************/
static XtGeometryResult GeometryManager( wid, request, reply )
    Widget wid;
    XtWidgetGeometry *request;
    XtWidgetGeometry *reply;
{
    ShellWidget 	shell = (ShellWidget)(wid->core.parent);
    XtWidgetGeometry 	my_request;

    if(!(shell->shell.allow_shell_resize) && XtIsRealized(wid) &&
       (request->request_mode & (CWWidth | CWHeight | CWBorderWidth)))
      return(XtGeometryNo);

    my_request.request_mode = 0;
    /* %%% worry about XtCWQueryOnly */
    if (request->request_mode & XtCWQueryOnly)
      my_request.request_mode |= XtCWQueryOnly;

    if (request->request_mode & CWWidth) {
	my_request.width = request->width;
	my_request.request_mode |= CWWidth;
    }
    if (request->request_mode & CWHeight) {
	my_request.height = request->height;
	my_request.request_mode |= CWHeight;
    }
    if (request->request_mode & CWBorderWidth) {
	my_request.border_width = request->border_width;
	my_request.request_mode |= CWBorderWidth;
    }
    if (XtMakeGeometryRequest((Widget)shell, &my_request, NULL)
	== XtGeometryYes)
      {
	  if (!(request->request_mode & XtCWQueryOnly))
	    { 
	      if (request->request_mode & CWWidth)
		wid->core.width = shell->core.width;
	      if (request->request_mode & CWHeight)
		wid->core.height = shell->core.height;
	    }
	  return XtGeometryYes;
      } 
    else 
      return XtGeometryNo;
}


/************************************************************************
 *
 *  _XmRootGeometryManager
 *
 ************************************************************************/
/*ARGSUSED*/
XtGeometryResult _XmRootGeometryManager(w, request, reply)
    Widget w;
    XtWidgetGeometry *request, *reply;
{
    XmWidgetExtData	extData = _XmGetWidgetExtData(w, XmSHELL_EXTENSION);
    XmShellExtObject	se = (XmShellExtObject)extData->widget;
    XtGeometryHandler	wmGeoHandler;
    ShellWidgetClass	swc = (ShellWidgetClass)wmShellWidgetClass;
    ShellClassExtensionRec **scExtPtr;
    XtGeometryResult	returnVal = XtGeometryNo;
    WMShellWidget	wmShell = (WMShellWidget)w;

    if (se)
      {
	  se->shell.lastConfigureRequest = NextRequest(XtDisplay(w));
      }
#ifdef DEBUG
    else
      XtError("no extension object");
#endif /* DEBUG */
    
    scExtPtr = (ShellClassExtensionRec **)
      _XmGetClassExtensionPtr((XmGenericClassExt *)&(swc->shell_class.extension),
			      NULLQUARK);
    if (request->request_mode & XtCWQueryOnly)
      {
	  if (!(wmShell->shell.allow_shell_resize) &&
	      (request->request_mode & 
	       (CWWidth | CWHeight | CWBorderWidth)))
	    return XtGeometryNo;
	  /*
	   * we should switch on useAsyncGeometry but we won't |||
	   */
	  else 
	    return XtGeometryYes;
      }

    if (se->shell.useAsyncGeometry)
      {
	  /* always make sure timeout is zero to force desired behaviour */
	  wmShell->wm.wait_for_wm = FALSE;
	  wmShell->wm.wm_timeout = 0;
      }
    if (wmGeoHandler = (*scExtPtr)->root_geometry_manager)
      {
	  returnVal =  (*wmGeoHandler)(w, request, reply);
	  if (se->shell.useAsyncGeometry)
	    returnVal = XtGeometryDone;
      }
    return returnVal;
}

/************************************************************************
 *
 *  SetMwmHints
 *
 ************************************************************************/
static void SetMwmHints(ve)
    XmVendorShellExtObject	ve;
{
    PropMwmHints	prop;
    Atom		mwm_hints_atom;
    Widget		shell = ve->ext.logicalParent;

#ifdef DEC_MOTIF_EXTENSION
    /*
     * Don't do this for an XUI hidden shell.  It changes XUI dialog
     * box behavior.
     */
    if (strcmp("DwtHiddenShell", 
		shell->core.widget_class->core_class.class_name) == 0)
	return;
#endif

    mwm_hints_atom = XmInternAtom(XtDisplay(shell),
				  _XA_MWM_HINTS, 
				  FALSE);

#define SET(field) prop.field = ve->vendor.mwm_hints.field
    SET(flags);
    SET(functions);
    SET(decorations);
    prop.inputMode = ve->vendor.mwm_hints.input_mode;
#undef SET

    XChangeProperty (XtDisplay(shell), 
		     XtWindow(shell),
		     mwm_hints_atom,mwm_hints_atom, 
		     32, PropModeReplace, 
		     (unsigned char *) &prop, PROP_MWM_HINTS_ELEMENTS);
}	




/************************************************************************
 *
 *  SetMwmMenu
 *
 ************************************************************************/
static void SetMwmMenu(ve)
    XmVendorShellExtObject	ve;
{
    Widget		shell = ve->ext.logicalParent;
    Atom		mwm_menu_atom;

#ifdef DEC_MOTIF_EXTENSION
    /*
     * Don't do this for an XUI hidden shell.  It changes XUI dialog
     * box behavior.
     */
    if (strcmp("DwtHiddenShell", 
		shell->core.widget_class->core_class.class_name) == 0)
	return;
#endif

    mwm_menu_atom = XmInternAtom(XtDisplay(shell),
				  _XA_MWM_MENU, 
				  FALSE);

    XChangeProperty (XtDisplay(shell), 
		     XtWindow(shell),
		     mwm_menu_atom, mwm_menu_atom, 
		     8, PropModeReplace, 
		     (unsigned char *) ve->vendor.mwm_menu, 
		     strlen(ve->vendor.mwm_menu) + 1);
}



/************************************************************************
 *
 *  GetCleanArgList
 *
 *  Given an ArgList and a list of resource names that should not
 *  propogate, return an ArgList with those resources removed.  Also
 *  pass back the new number of args.
 *
 *  The space allocated for the new list will be large enough to hold
 *  the old list.  This may make the new list larger than necessary
 *  but that shouldn't be a problem.
 *
 *  If the new ArgList has zero length, NULL is returned as the
 *  new ArgList and 0 as the new number of args.
 *
 ************************************************************************/
/*ARGSUSED*/
static void
#ifdef _NO_PROTO
GetCleanArgList(pOldArgs, nOldArgs, pBadNames, nBadNames, ppNewArgs, pnNewArgs)
    ArgList   pOldArgs;
    Cardinal  nOldArgs;
    ArgList   pBadNames;
    Cardinal  nBadNames;
    ArgList  *ppNewArgs;
    Cardinal *pnNewArgs;
#else
GetCleanArgList( ArgList   pOldArgs,
                 Cardinal  nOldArgs,
                 ArgList   pBadNames,
                 Cardinal  nBadNames,
                 ArgList  *ppNewArgs,
                 Cardinal *pnNewArgs)
#endif /* _NO_PROTO */
{
ArgList pNew,pNewBase,pbn;
int nn,noa,found;
Cardinal nna;

    if (!(noa = nOldArgs)) {
        *pnNewArgs = 0;
        *ppNewArgs = (ArgList) NULL;
        return;
    }

    nna = 0;
    pNewBase = pNew = (ArgList) XtMalloc(sizeof(Arg) * noa);
    while (--noa >= 0) {
        found = 0;
        for (nn = nBadNames, pbn = pBadNames; --nn >= 0;) {
            if (strcmp(pbn->name, pOldArgs->name) == 0) {
                found = 1;
                break;
                }
        }
        if (!found) {
            *pNew++ = *pOldArgs;
            nna++;
        }
        pOldArgs++;
    }

    if (*pnNewArgs = nna)
        *ppNewArgs = pNewBase;
    else {
        XtFree((char *) pNewBase);
        *ppNewArgs = (ArgList) NULL;
    }
}

#ifdef _NO_PROTO
void _XmVendorExtRealize(w)
    Widget	w;
#else /* _NO_PROTO */
void _XmVendorExtRealize (Widget w)
#endif /* _NO_PROTO */
{
    XmVendorShellExtObject	ve = (XmVendorShellExtObject)w;
    VendorShellWidget		vw;

    vw = (VendorShellWidget)ve->ext.logicalParent;
    if (ve->vendor.mwm_hints.flags)
      SetMwmHints(ve);
    
    if (ve->vendor.mwm_menu)
      SetMwmMenu(ve);

    _XmInstallProtocols(ve->ext.logicalParent);

    /*
     * set popped_up field if this is a app shell so that the virtual
     * grab list will treat it correctly. This should be done in Xt
     * |||
     */
    if ((vw->core.parent == NULL) && vw->core.mapped_when_managed)
      vw->shell.popped_up = TRUE;
}

/************************************************************************
 *
 *  XmIsMotifWMRunning
 *
 ************************************************************************/
#ifdef _NO_PROTO
Boolean	XmIsMotifWMRunning(shell)
    Widget	shell;
#else /* _NO_PROTO */
Boolean XmIsMotifWMRunning (Widget shell)
#endif /* _NO_PROTO */
{
    Atom	motif_wm_info_atom;
    long	offset, length;
    Atom	actual_type;
    int		actual_format;
    unsigned long num_items, bytes_after;
    PropMotifWmInfo	*prop = 0;
    Window	root = RootWindowOfScreen(XtScreen(shell));

    motif_wm_info_atom = XmInternAtom(XtDisplay(shell),
				      _XA_MOTIF_WM_INFO,
				      FALSE);

    XGetWindowProperty (XtDisplay(shell), 
			root,
			motif_wm_info_atom,
			0, (long)PROP_MOTIF_WM_INFO_ELEMENTS,
			FALSE, motif_wm_info_atom,
			&actual_type, &actual_format,
			&num_items, &bytes_after,
			(unsigned char **) &prop);

    if ((actual_type != motif_wm_info_atom) ||
	(actual_format != 32) ||
	(num_items < PROP_MOTIF_WM_INFO_ELEMENTS))
      {
	  if (prop != 0) XFree((char *)prop);
	  return (FALSE);
      }
    else
      {
	  Window	wm_window = (Window) prop->wmWindow;
	  Window	top, parent, *children;
	  unsigned int	num_children;
	  Boolean	returnVal;
	  Cardinal	i;

	  if (XQueryTree(XtDisplay(shell),
			 root, &top, &parent,
			 &children, &num_children))
	    {
		for (i = 0; 
		     i < num_children && children[i] != wm_window;
		     i++);
		returnVal =  (i == num_children) ? FALSE : TRUE;
	    }
	  else
	    returnVal = FALSE;

	  if (prop) XFree((char *)prop);
	  if (children) XFree((char *)children);
	  return (returnVal);
      }
}	


#if defined(DEC_MOTIF_VMS_LP) || defined(__DECC) || defined (OSF1_SHARED)
/************************************************************************
 *
 *  _XmOverwriteXtVendorShell
 *	copies Xm vendor shell class record over the Xt vendor shell
 *	called during shareable image initialization by LIB$INITIALIZE
 *	(specified in XmTransfer_NoThinLayer.mar)
 ************************************************************************/
int _XmOverwriteXtVendorShell
	(
	void *	init_coroutine(),
	void *	cli_coroutine(),
	int *	image_info
	)
{
	bcopy (&_Xm_vendorShellClassRec, &vendorShellClassRec,
		sizeof(_Xm_vendorShellClassRec));
	return(1);
}
#endif /* DEC_MOTIF_VMS_LP */
