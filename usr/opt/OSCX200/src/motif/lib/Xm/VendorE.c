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
static char *BuildSystemHeader= "$Header: /alphabits/u3/x11/ode/rcs/x11/src/motif/lib/Xm/VendorE.c,v 1.1.4.2 93/03/02 17:22:17 Dave_Hill Exp $";	/* BuildSystemHeader */
#endif		/* BuildSystemHeader */
#ifdef REV_INFO
#ifndef lint
static char SCCSID[] = "OSF/Motif: @(#)VendorE.c	3.15.1.4 91/06/18";
#endif /* lint */
#endif /* REV_INFO */
/******************************************************************************
*******************************************************************************
*
*  (c) Copyright 1989, 1990, 1991 OPEN SOFTWARE FOUNDATION, INC.
*  (c) Copyright 1989, 1990  DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASS.
*  (c) Copyright 1987, 1988, 1989, 1990, HEWLETT-PACKARD COMPANY
*  (c) Copyright 1988 MASSACHUSETTS INSTITUTE OF TECHNOLOGY 
*  (c) Copyright 1988 MICROSOFT CORPORATION
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
* DEC is a registered trademark of Digital Equipment Corporation
* DIGITAL is a registered trademark of Digital Equipment Corporation
* X Window System is a trademark of the Massachusetts Institute of Technology
*
*******************************************************************************
******************************************************************************/
/* Make sure all wm properties can make it out of the resource manager */

#include <Xm/XmP.h>
#include <X11/ShellP.h>
#include <Xm/VendorEP.h>
#include "BaseClassI.h"

#ifdef DEC_MOTIF_EXTENSION
#include <X11/XUICompat.h>
#endif

#define DONT_CARE -1
#define BIGSIZE ((Dimension)32767)

#ifdef VMS
externalref XmDesktopClassRec 	xmDesktopClassRec;
externalref WidgetClass xmScreenObjectClass;
#ifndef XmIsScreenObject
#define XmIsScreenObject(w)	XtIsSubclass(w, xmScreenObjectClass)
#endif /* XmIsScreenObject */

#define LowerCase _XtLowerCase
#endif

/* forward declarations for internal functions */

/* Foward reference for class routines */

static void		ShellClassInitialize();
static void		ShellClassPartInitialize();
static void 		ShellInitialize();
static Boolean 		ShellSetValues();
static void		ShellDestroy();
static XtEventHandler	StructureNotifyHandler();

/* forward reference for utility routines */

#define Offset(field) XtOffset(XmShellExtObject, shell.field)

static XtResource shellResources[] =
{    
    {
	XmNuseAsyncGeometry, XmCUseAsyncGeometry, XmRBoolean, 
	sizeof(Boolean), Offset(useAsyncGeometry),
	XmRImmediate, FALSE,
    },
};
#undef Offset

#ifdef DEC_MOTIF_BUG_FIX
externaldef(xmshellextclassrec)
#endif
XmShellExtClassRec xmShellExtClassRec = {
    {	
	(WidgetClass) &xmDesktopClassRec,/* superclass		*/   
	"Shell",			/* class_name 		*/   
	sizeof(XmShellExtRec),	 	/* size 		*/   
	NULL,		 		/* Class Initializer 	*/   
	ShellClassPartInitialize, 	/* class_part_init 	*/ 
	FALSE, 				/* Class init'ed ? 	*/   
	NULL,				/* initialize         	*/   
	NULL, 				/* initialize_notify    */ 
	NULL,	 			/* realize            	*/   
	NULL,	 			/* actions            	*/   
	0,				/* num_actions        	*/   
	shellResources,			/* resources          	*/   
	XtNumber(shellResources),	/* resource_count     	*/   
	NULLQUARK, 			/* xrm_class          	*/   
	FALSE, 				/* compress_motion    	*/   
	FALSE, 				/* compress_exposure  	*/   
	FALSE, 				/* compress_enterleave	*/   
	FALSE, 				/* visible_interest   	*/   
	NULL,				/* destroy            	*/   
	NULL,		 		/* resize             	*/   
	NULL, 				/* expose             	*/   
	NULL,		 		/* set_values         	*/   
	NULL, 				/* set_values_hook      */ 
	NULL,			 	/* set_values_almost    */ 
	NULL,				/* get_values_hook      */ 
	NULL, 				/* accept_focus       	*/   
	XtVersion, 			/* intrinsics version 	*/   
	NULL, 				/* callback offsets   	*/   
	NULL,				/* tm_table           	*/   
	NULL, 				/* query_geometry       */ 
	NULL, 				/* display_accelerator  */ 
	NULL, 				/* extension            */ 
    },	
    {					/* ext			*/
	NULL,				/* synthetic resources	*/
	0,				/* num syn resources	*/
	NULL,				/* extension		*/
    },
    {					/* desktop		*/
	NULL,				/* child_class		*/
	XtInheritInsertChild,		/* insert_child		*/
	XtInheritDeleteChild,		/* delete_child		*/
	NULL,				/* extension		*/
    },
    {					/* shell ext		*/
	(XtEventHandler)StructureNotifyHandler,	/* structureNotify*/
	NULL,				/* extension		*/
    },
};


externaldef(xmShellExtobjectclass) WidgetClass 
  xmShellExtObjectClass = (WidgetClass) (&xmShellExtClassRec);



/***************************************************************************
 *
 * Vendor shell class record
 *
 ***************************************************************************/

#define Offset(field) XtOffset(XmVendorShellExtObject, vendor.field)

static XtResource extResources[] =
{
    {
	XmNextensionType,
	XmCExtensionType, XmRExtensionType, sizeof (unsigned char),
	XtOffset (XmExtObject, ext.extensionType),
	XmRImmediate, (XtPointer)XmSHELL_EXTENSION,
    },
    {
	XmNdefaultFontList,
	XmCDefaultFontList, XmRFontList, sizeof (XmFontList),
	Offset (default_font_list),
#ifdef DEC_MOTIF_EXTENSION

/*	XmRString, DXmDefaultFont,	*/
	XmRString, (caddr_t) NULL,		/* I18N */
#else
	XmRString, (caddr_t) "fixed",
#endif
    },
    {
	XmNshellUnitType, XmCShellUnitType, XmRShellUnitType, 
	sizeof (unsigned char), Offset (unit_type),
	XmRImmediate, (caddr_t) XmPIXELS,
    },	
    {
	XmNdeleteResponse, XmCDeleteResponse, 
	XmRDeleteResponse, sizeof(unsigned char),
	Offset(delete_response), 
	XmRImmediate, (caddr_t) XmDESTROY,
    },
    {
	XmNkeyboardFocusPolicy, XmCKeyboardFocusPolicy, XmRKeyboardFocusPolicy, 
	sizeof(unsigned char),
	Offset(focus_policy), 
	XmRImmediate, (caddr_t)XmEXPLICIT,
    },
    { 
	XmNmwmDecorations, XmCMwmDecorations, XmRInt, 
	sizeof(int), Offset(mwm_hints.decorations), 
	XmRImmediate, (caddr_t) DONT_CARE,
    },
    { 
	XmNmwmFunctions, XmCMwmFunctions, XmRInt, 
	sizeof(int), Offset(mwm_hints.functions), 
	XmRImmediate, (caddr_t) DONT_CARE,
    },
    { 
	XmNmwmInputMode, XmCMwmInputMode, XmRInt, 
	sizeof(int), Offset(mwm_hints.input_mode), 
	XmRImmediate, (caddr_t) DONT_CARE,
    },
    { 
	XmNmwmMenu, XmCMwmMenu, XmRString, 
	sizeof(int), Offset(mwm_menu), 
	XmRString, NULL,
    },
    { 
	XmNfocusMovedCallback, XmCCallback, XmRCallback, 
	sizeof(XtCallbackList), Offset(focus_moved_callback), 
	XmRImmediate, NULL,
    },
    { 
	XmNrealizeCallback, XmCCallback, XmRCallback, 
	sizeof(XtCallbackList), Offset(realize_callback), 
	XmRImmediate, NULL,
    },
};
#undef Offset

/*  Definition for resources that need special processing in get values  */

#define Offset(x) XtOffset(VendorShellWidget, x)

static void 		ExtParentDimFromHorizontalPixels();
static XmImportOperator ExtParentDimToHorizontalPixels();
static void		ExtParentDimFromVerticalPixels();
static XmImportOperator ExtParentDimToVerticalPixels();

static void 		ExtParentIntFromHorizontalPixels();
static XmImportOperator ExtParentIntToHorizontalPixels();
static void		ExtParentIntFromVerticalPixels();
static XmImportOperator ExtParentIntToVerticalPixels();

static XmSyntheticResource synResources[] =
{
    { 
	XmNx, sizeof (Position),
	Offset (core.x), 
	ExtParentDimFromHorizontalPixels,
	ExtParentDimToHorizontalPixels,
    },
    {
	XmNy, sizeof (Position),
	Offset (core.y), 
	ExtParentDimFromVerticalPixels,
	ExtParentDimToVerticalPixels,
    },
    {
	XmNwidth, sizeof (Dimension),
	Offset (core.width), 
	ExtParentDimFromHorizontalPixels,
	ExtParentDimToHorizontalPixels,
    },
    { 
	XmNheight, sizeof (Dimension),
	Offset (core.height), 
	ExtParentDimFromVerticalPixels,
	ExtParentDimToVerticalPixels,
    },
    {
	XmNborderWidth, sizeof (Dimension),
	Offset (core.border_width), 
	ExtParentDimFromHorizontalPixels,
	ExtParentDimToHorizontalPixels,
    },

/* size_hints minus things stored in core */

    { 
	XmNminWidth, sizeof(int),
	Offset(wm.size_hints.min_width), 
	ExtParentIntFromHorizontalPixels,
	ExtParentIntToHorizontalPixels,
    },	
    { 
	XmNminHeight, sizeof(int),
	Offset(wm.size_hints.min_height), 
	ExtParentIntFromVerticalPixels,
	ExtParentIntToVerticalPixels,
    },
    { 
	XmNmaxWidth, sizeof(int),
	Offset(wm.size_hints.max_width), 
	ExtParentIntFromHorizontalPixels,
	ExtParentIntToHorizontalPixels,
    },	
    { 	
	XmNmaxHeight,sizeof(int),
	Offset(wm.size_hints.max_height),
	ExtParentIntFromVerticalPixels,
	ExtParentIntToVerticalPixels,
    },

/* wm_hints */

    { 
	XmNiconX, sizeof(int),
	Offset(wm.wm_hints.icon_x), 
	ExtParentIntFromHorizontalPixels,
	ExtParentIntToHorizontalPixels,
    },
    { 
	XmNiconY, sizeof(int),
	Offset(wm.wm_hints.icon_y),  
	ExtParentIntFromVerticalPixels,
	ExtParentIntToVerticalPixels,
    }
};


static void		VendorClassInitialize();
static void		VendorClassPartInitialize();
static void 		VendorInitialize();
static Boolean 		VendorSetValues();
static void		VendorDestroy();

static XtCallbackProc	OffsetHandler();
static XtCallbackProc	DeleteWindowHandler();
static void		InitializePrehook();

static XmBaseClassExtRec       myBaseClassExtRec = {
    NULL,                                     /* Next extension       */
    NULLQUARK,                                /* record type XmQmotif */
    XmBaseClassExtVersion,                    /* version              */
    sizeof(XmBaseClassExtRec),                /* size                 */
    InitializePrehook,		              /* initialize prehook   */
    XmInheritSetValuesPrehook,	              /* set_values prehook   */
    NULL,			              /* initialize posthook  */
    NULL,			              /* set_values posthook  */
    NULL,				      /* secondary class      */
    NULL,			              /* creation proc        */
    NULL,                                     /* getSecRes data       */
#ifdef DEC_MOTIF_BUG_FIX
    {0},                                      /* fast subclass        */
#else
    {NULL},                                   /* fast subclass        */
#endif
    XmInheritGetValuesPrehook,	              /* get_values prehook   */
    NULL,			              /* get_values posthook  */
#ifndef NO_CLASS_PART_INIT_HOOK
    XmInheritClassPartInitPrehook,	      /* class_part_prehook   */
    XmInheritClassPartInitPosthook,	      /* class_part_posthook  */
    NULL,	 			      /* compiled_ext_resources*/   
    NULL,	 			      /* ext_resources       	*/   
    0,					      /* resource_count     	*/   
    TRUE,				      /* use_sub_resources	*/
#endif /* NO_CLASS_PART_INIT_HOOK */
};

#ifdef DEC_MOTIF_BUG_FIX
externaldef(xmvendorshellextclassrec)
#endif
XmVendorShellExtClassRec xmVendorShellExtClassRec = {
    {	
	(WidgetClass) &xmShellExtClassRec,/* superclass		*/   
	"VendorShell",			/* class_name 		*/   
	sizeof(XmVendorShellExtRec), 	/* size 		*/   
	VendorClassInitialize, 		/* Class Initializer 	*/   
	VendorClassPartInitialize,	/* class_part_init 	*/ 
	FALSE, 				/* Class init'ed ? 	*/   
	NULL,				/* initialize         	*/   
	NULL, 				/* initialize_notify    */ 
	NULL,	 			/* realize            	*/   
	NULL,	 			/* actions            	*/   
	0,				/* num_actions        	*/   
	extResources, 			/* resources          	*/   
	XtNumber(extResources),		/* resource_count     	*/   
	NULLQUARK, 			/* xrm_class          	*/   
	FALSE, 				/* compress_motion    	*/   
	FALSE, 				/* compress_exposure  	*/   
	FALSE, 				/* compress_enterleave	*/   
	FALSE, 				/* visible_interest   	*/   
	VendorDestroy,			/* destroy            	*/   
	NULL,		 		/* resize             	*/   
	NULL, 				/* expose             	*/   
	NULL,		 		/* set_values         	*/   
	NULL, 				/* set_values_hook      */ 
	NULL,			 	/* set_values_almost    */ 
	NULL,				/* get_values_hook      */ 
	NULL, 				/* accept_focus       	*/   
	XtVersion, 			/* intrinsics version 	*/   
	NULL, 				/* callback offsets   	*/   
	NULL,				/* tm_table           	*/   
	NULL, 				/* query_geometry       */ 
	NULL, 				/* display_accelerator  */ 
	(XtPointer)&myBaseClassExtRec,	/* extension            */ 
    },	
    {					/* ext			*/
	synResources,			/* synthetic resources	*/
	XtNumber(synResources),		/* num syn resources	*/
	NULL,				/* extension		*/
    },
    {					/* desktop		*/
	NULL,				/* child_class		*/
	XtInheritInsertChild,		/* insert_child		*/
	XtInheritDeleteChild,		/* delete_child		*/
	NULL,				/* extension		*/
    },
    {					/* shell ext		*/
	XmInheritEventHandler,		/* structureNotify	*/
	NULL,				/* extension		*/
    },
    {					/* vendor ext		*/
	(XtCallbackProc)DeleteWindowHandler,/* delete window handler*/
	(XtCallbackProc)OffsetHandler,	/* offset handler	*/
	NULL,				/* extension		*/
    },
};

externaldef(xmVendorShellExtobjectclass) WidgetClass 
  xmVendorShellExtObjectClass = (WidgetClass) (&xmVendorShellExtClassRec);


static XmImportOperator ExtParentDimToHorizontalPixels(ve, offset, value)
    XmVendorShellExtObject	ve;
    int				offset;
    XtArgVal    	  	*value;
{
    _XmToHorizontalPixels ((Widget)ve, offset, value);
    *((Dimension *) ((caddr_t)(ve->ext.logicalParent) + offset)) = (Dimension)*value;
    return XmSYNTHETIC_NONE;
}

static void ExtParentDimFromHorizontalPixels(ve, offset, value)
    XmVendorShellExtObject	ve;
    int				offset;
    XtArgVal		      	*value;
{
     *value = *((Dimension *)(((caddr_t)ve->ext.logicalParent) + offset));
     _XmFromHorizontalPixels ((Widget)ve, offset, value);
}

static XmImportOperator ExtParentDimToVerticalPixels(ve, offset, value)
    XmVendorShellExtObject	ve;
    int				offset;
    XtArgVal    	  	*value;
{
    _XmToVerticalPixels ((Widget)ve, offset, value);
    *((Dimension *) ((caddr_t)(ve->ext.logicalParent) + offset)) = (Dimension)*value;
    return XmSYNTHETIC_NONE;
}

static void ExtParentDimFromVerticalPixels(ve, offset, value)
    XmVendorShellExtObject	ve;
    int				offset;
    XtArgVal		      	*value;
{
     *value = *((Dimension *)(((caddr_t)ve->ext.logicalParent) + offset));
     _XmFromVerticalPixels ((Widget)ve, offset, value);
}


static XmImportOperator ExtParentIntToHorizontalPixels(ve, offset, value)
    XmVendorShellExtObject	ve;
    int				offset;
    XtArgVal    	  	*value;
{
    _XmToHorizontalPixels ((Widget)ve, offset, value);
    *((int *) ((caddr_t)(ve->ext.logicalParent) + offset)) = (int)*value;
    return XmSYNTHETIC_NONE;
}

static void ExtParentIntFromHorizontalPixels(ve, offset, value)
    XmVendorShellExtObject	ve;
    int				offset;
    XtArgVal		      	*value;
{
     *value = *((Dimension *)(((caddr_t)ve->ext.logicalParent) + offset)) ; 
     _XmFromHorizontalPixels ((Widget)ve, offset, value);
}

static XmImportOperator ExtParentIntToVerticalPixels(ve, offset, value)
    XmVendorShellExtObject	ve;
    int				offset;
    XtArgVal    	  	*value;
{
    _XmToVerticalPixels ((Widget)ve, offset, value);
    *((int *) ((caddr_t)(ve->ext.logicalParent) + offset)) = (int)*value;
    return XmSYNTHETIC_NONE;
}

static void ExtParentIntFromVerticalPixels(ve, offset, value)
    XmVendorShellExtObject	ve;
    int				offset;
    XtArgVal		      	*value;
{
     *value = *((Dimension *)(((caddr_t)ve->ext.logicalParent) + offset));
     _XmFromVerticalPixels ((Widget)ve, offset, value);
}


#define	done(value, type) \
	{							\
	    if (toVal->addr != NULL) {				\
		if (toVal->size < sizeof(type)) {		\
		    toVal->size = sizeof(type);			\
		    return False;				\
		}						\
		*(type*)(toVal->addr) = (value);		\
	    }							\
	    else {						\
		static type static_val;				\
		static_val = (value);				\
		toVal->addr = (XtPointer)&static_val;		\
	    }							\
	    toVal->size = sizeof(type);				\
	    return True;					\
	}


/************************************************************************
 *
 *  NoopConverter
 *
 ************************************************************************/
    /*ARGSUSED*/
static Boolean NoopConverter(dpy, args, num_args, fromVal, toVal)
    Display	*dpy;
    XrmValuePtr args;
    Cardinal    *num_args;
    XrmValuePtr	fromVal;
    XrmValuePtr	toVal;
{
    if (*num_args != 0)
	XtWarningMsg("wrongParameters","noopConverter",
		     "XmToolkitError",
		     "String to noop conversion needs no extra arguments",
		     (String *)NULL, (Cardinal *)NULL);
    return False;
}

/************************************************************************
 *
 *  CvtStringToDeleteResponse
 *
 ************************************************************************/
    /*ARGSUSED*/
static Boolean CvtStringToDeleteResponse(dpy, args, num_args, fromVal, toVal)
    Display	*dpy;
    XrmValuePtr args;
    Cardinal    *num_args;
    XrmValuePtr	fromVal;
    XrmValuePtr	toVal;
{
    unsigned char delete_response;
    static XrmQuark	q, XmQEdestroy, XmQEunmap, XmQEdoNothing;
    char	lowerName[1000];
    static Boolean	first_time = TRUE;

    if (*num_args != 0)
	XtWarningMsg("wrongParameters","cvtStringToDeleteResponse",
		     "XmToolkitError",
		     "String to DeleteResponse conversion needs no extra arguments",
		     (String *)NULL, (Cardinal *)NULL);

    if (first_time)
      {
	  XmQEdestroy	= XrmStringToQuark("destroy");
	  XmQEunmap   	= XrmStringToQuark("unmap");
	  XmQEdoNothing = XrmStringToQuark("do_nothing");
	  first_time = FALSE;
      }
    LowerCase((char *) fromVal->addr, lowerName);
    q = XrmStringToQuark(lowerName);

    if (q == XmQEdestroy)
      {
	  delete_response = XmDESTROY;
	  done(delete_response, unsigned char);
      }
    if (q == XmQEunmap)
      {
	  delete_response = XmUNMAP;
	  done(delete_response, unsigned char);
      }
    if (q == XmQEdoNothing)
      {
	  delete_response = XmDO_NOTHING;
	  done(delete_response, unsigned char);
      }

    XtStringConversionWarning((char *) fromVal->addr, "DeleteResponse");
    return False;
}


/************************************************************************
 *
 *  CvtStringToNavigationType
 *
 ************************************************************************/
    /*ARGSUSED*/
static Boolean CvtStringToNavigationType(dpy, args, num_args, fromVal, toVal)
    Display	*dpy;
    XrmValuePtr args;
    Cardinal    *num_args;
    XrmValuePtr	fromVal;
    XrmValuePtr	toVal;
{
    unsigned char 	navigation_type;
    static XrmQuark		q, XmQENone, XmQENormal, XmQESticky, XmQEExclusive;
    char			*name, lowerName[1000];
    static Boolean		first_time = TRUE;

    if (*num_args != 0)
	XtWarningMsg("wrongParameters","cvtStringToNavigationType",
		     "XmToolkitError",
		     "String to NavigationType conversion needs no extra arguments",
		     (String *)NULL, (Cardinal *)NULL);

    if (first_time)
      {
	  XmQENone	= XrmStringToQuark("none");
	  XmQENormal   	= XrmStringToQuark("tab_group");
	  XmQESticky	= XrmStringToQuark("sticky_tab_group");
	  XmQEExclusive	= XrmStringToQuark("exclusive_tab_group");
	  first_time = FALSE;
      }
    LowerCase((char *) fromVal->addr, lowerName);
    
    if ((lowerName[0] == 'x') &&
	(lowerName[1] == 'm'))
      name = lowerName + 2;
    else
      name = lowerName;

    q = XrmStringToQuark(name);

    if (q == XmQENone)
      {
	  navigation_type = XmNONE;
	  done(navigation_type, unsigned char);
      }
    if (q == XmQENormal)
      {
	  navigation_type = XmTAB_GROUP;
	  done(navigation_type, unsigned char);
      }
    if (q == XmQESticky)
      {
	  navigation_type = XmSTICKY_TAB_GROUP;
	  done(navigation_type, unsigned char);
      }
    if (q == XmQEExclusive)
      {
	  navigation_type = XmEXCLUSIVE_TAB_GROUP;
	  done(navigation_type, unsigned char);
      }

    XtStringConversionWarning((char *) fromVal->addr, "NavigationType");
    return False;
}








/* ARGSUSED */
static Boolean CvtStringToHorizDim (dpy, args, num_args, from_val, toVal)
    Display  * dpy;
    XrmValue * args;
    Cardinal * num_args;
    XrmValue * from_val;
    XrmValue * toVal;
{
    unsigned char unitType = (unsigned char) *(args[0].addr);
    Screen * screen = * ((Screen **) args[1].addr);
    Dimension pixels;
    int intermediate;
    
    intermediate = (int) atoi(from_val->addr);
    
    pixels = (Dimension)
      _XmConvertUnits(screen,XmHORIZONTAL,(int) unitType,
		      intermediate,XmPIXELS);
    done(pixels, Dimension);
}

/* ARGSUSED */
static Boolean CvtStringToHorizPos (dpy, args, num_args, from_val, toVal)
    Display  * dpy;
    XrmValue * args;
    Cardinal * num_args;
    XrmValue * from_val;
    XrmValue * toVal;
{
    unsigned char unitType = (unsigned char) *(args[0].addr);
    Screen * screen = * ((Screen **) args[1].addr);
    Position pixels;
    int intermediate;
    
    intermediate = (int) atoi(from_val->addr);
    
    pixels = (Position)
      _XmConvertUnits(screen,XmHORIZONTAL,(int) unitType,
		      intermediate,XmPIXELS);
    done(pixels, Position);

}
/* ARGSUSED */
static Boolean CvtStringToVertDim (dpy, args, num_args, from_val, toVal)
    Display  * dpy;
    XrmValue * args;
    Cardinal * num_args;
    XrmValue * from_val;
    XrmValue * toVal;
{
    unsigned char unitType = (unsigned char) *(args[0].addr);
    Screen * screen = * ((Screen **) args[1].addr);
    Dimension pixels;
    int intermediate;
    
    intermediate = (int) atoi(from_val->addr);
    
    pixels = (Dimension)
      _XmConvertUnits(screen,XmVERTICAL,(int) unitType,
		      intermediate,XmPIXELS);
    done(pixels, Dimension);
}

/* ARGSUSED */
static Boolean CvtStringToVertPos (dpy, args, num_args, from_val, toVal)
    Display  * dpy;
    XrmValue * args;
    Cardinal * num_args;
    XrmValue * from_val;
    XrmValue * toVal;
{
    unsigned char unitType = (unsigned char) *(args[0].addr);
    Screen * screen = * ((Screen **) args[1].addr);
    Position pixels;
    int intermediate;
    
    intermediate = (int) atoi(from_val->addr);
    
    pixels = (Position)
      _XmConvertUnits(screen,XmVERTICAL,(int) unitType,
		      intermediate,XmPIXELS);
    done(pixels, Position);
}
/* ARGSUSED */
static Boolean CvtStringToHorizontalInt (dpy, args, num_args, from_val, toVal)
    Display  * dpy;
    XrmValue * args;
    Cardinal * num_args;
    XrmValue * from_val;
    XrmValue * toVal;
{
    unsigned char unitType = (unsigned char) *(args[0].addr);
    Screen * screen = * ((Screen **) args[1].addr);
    int pixels;
    int intermediate;
    
    intermediate = (int) atoi(from_val->addr);
    
    pixels = (int)
      _XmConvertUnits(screen,XmHORIZONTAL,(int) unitType,
		      intermediate,XmPIXELS);
    done(pixels, int);
}

/************************************************************************
 *
 * 	CvtStringToShellUnitType
 *
 ************************************************************************/
static Boolean CvtStringToShellUnitType (dpy, args, num_args, fromVal, toVal)
    Display     *dpy;
    XrmValuePtr args;
    Cardinal    *num_args;
    XrmValue *fromVal;
    XrmValue *toVal;
{
    static XrmQuark	q, XmQEpixels, XmQEmillimeters, XmQEinches,
    	       		XmQEpoints, XmQEfontUnits;
    static Boolean	first_time = TRUE;
    char 		lower_name[100];
    unsigned char 	unitType;
    
    if (first_time) {
	XmQEpixels = XrmStringToQuark("pixels");
	XmQEmillimeters = XrmStringToQuark("100th_millimeters");
	XmQEinches = XrmStringToQuark("1000th_inches");
	XmQEpoints = XrmStringToQuark("100th_points");
	XmQEfontUnits = XrmStringToQuark("100th_font_units");
	first_time = FALSE;
    }
    
    if (*num_args != 0)
      XtWarningMsg("wrongParameters","cvtStringTounitType",
		   "XmToolkitError",
		   "String to unitType conversion needs no extra arguments",
		   (String *)NULL, (Cardinal *)NULL);
    
    LowerCase((char *) fromVal->addr, lower_name);
    q = XrmStringToQuark(lower_name);
    
    if (q == XmQEpixels)
      {
	  unitType = XmPIXELS;
	  done(unitType, unsigned char);
      }
    if (q == XmQEmillimeters)
      {
	  unitType = Xm100TH_MILLIMETERS;
	  done(unitType, unsigned char);
      }
    if (q == XmQEinches)
      {
	  unitType = Xm1000TH_INCHES;
	  done(unitType, unsigned char);
      }
    if (q == XmQEpoints)
      {
	  unitType = Xm100TH_POINTS;
	  done(unitType, unsigned char);
      }
    
    if (q == XmQEfontUnits)
      {
	  unitType = Xm100TH_FONT_UNITS;
	  done(unitType, unsigned char);
      }
    
    XtStringConversionWarning((char *) fromVal->addr, "unitType");
    return False;
}

/************************************************************************
 *
 *  CvtStringToFocusPolicy
 *
 *  		Convert a string to a focus policy
 *
 ************************************************************************/
static Boolean CvtStringToFocusPolicy (dpy, args, numArgs, fromVal, toVal)
    Display  * dpy;
    XrmValue * args;
    Cardinal * numArgs;
    XrmValue * fromVal;
    XrmValue * toVal;
    
{
    static XrmQuark	q, XmQEexplicit, XmQEpointer;
    static Boolean	first_time = TRUE;
    char 		lower_name[100];
    unsigned char	focusPolicy;
    
    if (first_time) {
	XmQEexplicit = XrmStringToQuark("explicit");
	XmQEpointer = XrmStringToQuark("pointer");
	first_time = FALSE;
    }
    
    if (*numArgs != 0)
      XtWarningMsg("wrongParameters","cvtStringToFocusPolicy",
		   "XmToolkitError",
		   "String to focusPolicy conversion needs no extra arguments",
		   (String *)NULL, (Cardinal *)NULL);
    
    LowerCase((char *) fromVal->addr, lower_name);
    q = XrmStringToQuark(lower_name);
    
    if (q == XmQEexplicit)
      {
	  focusPolicy = XmEXPLICIT;
	  done(focusPolicy, unsigned char);
      }
    if (q == XmQEpointer)
      {
	  focusPolicy = XmPOINTER;
	  done(focusPolicy, unsigned char);
      }
    
    XtStringConversionWarning((char *) fromVal->addr, "focusPolicy");
    return False;
}
#undef done

static void FetchUnitType(widget, size, value)
    Widget widget;
    Cardinal *size;
    XrmValue* value;
{
    XmExtObject	extObject;
    static unsigned char pixelType = XmPIXELS;

    if (widget == NULL) {
	XtErrorMsg("missingWidget", "fetchUnitType", "XtToolkitError",
                   "FetchUnitType called without a widget to reference",
                   (String*)NULL, (Cardinal*)NULL);
    }

    if (XmIsVendorShell(widget))
      {
	  XmWidgetExtData extData;
	  XmVendorShellExtObject ve;

	  if ((extData = _XmGetWidgetExtData(widget, XmSHELL_EXTENSION)) &&
	      (ve = (XmVendorShellExtObject)extData->widget))
	    value->addr = (caddr_t)&(ve->vendor.unit_type);
	  else
	    value->addr = (caddr_t)&(pixelType);
      }
    else 
      XtWarning("FetchUnitType: bad widget class");

    value->size = sizeof(unsigned char);
}

static XtConvertArgRec resIndConvertArgs[] = {
    { XtProcedureArg, 
      (XtPointer)FetchUnitType, 
      0
    },
    { XtWidgetBaseOffset,
        (XtPointer) XtOffsetOf(WidgetRec, core.screen),
        sizeof (Screen*)
    }
};

/************************************************************************
 *
 *  RegisterVendorConverters
 *
 ************************************************************************/

static void RegisterVendorConverters()
{
    static Boolean firstTime = TRUE;

    if (!firstTime)
      return;
    else
      firstTime = FALSE;
    
    XtSetTypeConverter(XmRString, 
		       XmRPixmap, 
		       (XtTypeConverter)NoopConverter, 
		       NULL, 
		       0,
		       XtCacheAll,
		       (XtDestructor)NULL);
    
    XtSetTypeConverter(XmRString, 
		       XmRShellHorizDim, 
		       (XtTypeConverter) CvtStringToHorizDim, 
		       resIndConvertArgs, 
		       XtNumber(resIndConvertArgs),
		       XtCacheNone, (XtDestructor)NULL);
    XtSetTypeConverter(XmRString, 
		       XmRShellVertDim,
		       (XtTypeConverter) CvtStringToVertDim,
		       resIndConvertArgs, 
		       XtNumber(resIndConvertArgs),
		       XtCacheNone, (XtDestructor)NULL);
    
    XtSetTypeConverter(XmRString, 
		       XmRShellHorizPos, 
		       (XtTypeConverter) CvtStringToHorizPos, 
		       resIndConvertArgs, 
		       XtNumber(resIndConvertArgs),
		       XtCacheNone, (XtDestructor)NULL);
    XtSetTypeConverter(XmRString, 
		       XmRShellVertPos,
		       (XtTypeConverter) CvtStringToVertPos,
		       resIndConvertArgs, 
		       XtNumber(resIndConvertArgs),
		       XtCacheNone, (XtDestructor)NULL);

    XtSetTypeConverter(XmRString, 
		       XmRHorizontalInt, 
		       (XtTypeConverter) CvtStringToHorizontalInt, 
		       resIndConvertArgs, 
		       XtNumber(resIndConvertArgs),
		       XtCacheNone, (XtDestructor)NULL);
    XtSetTypeConverter(XmRString, 
		       XmRVerticalInt, 
		       (XtTypeConverter) CvtStringToHorizontalInt, 
		       resIndConvertArgs, 
		       XtNumber(resIndConvertArgs),
		       XtCacheNone, (XtDestructor)NULL);
    
    XtSetTypeConverter(XmRString, XmRDeleteResponse, 
		       (XtTypeConverter) CvtStringToDeleteResponse,
		       NULL, 0, XtCacheNone, (XtDestructor)NULL);
    XtSetTypeConverter (XmRString, XmRKeyboardFocusPolicy,
			(XtTypeConverter) CvtStringToFocusPolicy, NULL, 0,
			XtCacheNone, (XtDestructor)NULL);
    XtSetTypeConverter (XmRString, XmRShellUnitType,
			(XtTypeConverter) CvtStringToShellUnitType, NULL, 0,
			XtCacheNone, (XtDestructor)NULL);
    XtSetTypeConverter (XmRString, XmRNavigationType,
			(XtTypeConverter) CvtStringToNavigationType, NULL, 0,
			XtCacheNone, (XtDestructor)NULL);
}

/************************************************************************
 *
 *  ClassPartInitialize
 *    Set up the inheritance mechanism for the routines exported by
 *    vendorShells class part.
 *
 ************************************************************************/

static void ShellClassPartInitialize (w)
    WidgetClass w;
    
{
    XmShellExtObjectClass wc = (XmShellExtObjectClass) w;
    XmShellExtObjectClass sc =
      (XmShellExtObjectClass) wc->object_class.superclass;
    
    if (wc == (XmShellExtObjectClass)xmShellExtObjectClass)
      return;

    if (wc->shell_class.structureNotifyHandler == XmInheritEventHandler)
      wc->shell_class.structureNotifyHandler = 
	sc->shell_class.structureNotifyHandler;
}


static void VendorClassInitialize()
{
    RegisterVendorConverters();
    myBaseClassExtRec.record_type = XmQmotif;
}

/************************************************************************
 *
 *  ClassPartInitialize
 *    Set up the inheritance mechanism for the routines exported by
 *    vendorShells class part.
 *
 ************************************************************************/

static void VendorClassPartInitialize (w)
    WidgetClass w;
    
{
    XmVendorShellExtObjectClass wc = (XmVendorShellExtObjectClass) w;
    XmVendorShellExtObjectClass sc =
      (XmVendorShellExtObjectClass) wc->object_class.superclass;
    
    if (wc == (XmVendorShellExtObjectClass)xmVendorShellExtObjectClass)
      return;

    if (wc->vendor_class.delete_window_handler == XmInheritProtocolHandler)
      wc->vendor_class.delete_window_handler = 
	sc->vendor_class.delete_window_handler;

    if (wc->vendor_class.offset_handler == XmInheritProtocolHandler)
      wc->vendor_class.offset_handler = 
	sc->vendor_class.offset_handler;
}

/************************************************************************
 *
 *  StructureNotifyHandler
 *
 ************************************************************************/
/* ARGSUSED */
static XtEventHandler StructureNotifyHandler(wid, closure, event, continue_to_dispatch)
    Widget wid;
    XtPointer closure;
    XEvent *event;
    Boolean *continue_to_dispatch; /* unused */
{
    register ShellWidget 	w = (ShellWidget) wid;
    WMShellWidget 		wmshell = (WMShellWidget) w;
    Boolean  			sizechanged = FALSE;
    unsigned int 		width, height, border_width, tmpdepth;
    Position 			tmpx, tmpy;
    int 			tmp2x, tmp2y;
    Window 			tmproot, tmpchild;
    XmShellExtObject		shellExt = (XmShellExtObject) closure;
    XmVendorShellExtObject	vendorExt = (XmVendorShellExtObject)closure;
    XmVendorShellExtPart	*vePPtr;
    XmScreenObject		 screenObj;
    Boolean			isScreenShell;
    Widget			kid;

    /*
     *  for right now if this is being used by overrides bug out
     */
    if (!XmIsVendorShell(wid))
      return;
    else
      vePPtr = (XmVendorShellExtPart *) &(vendorExt->vendor);

    if (XmIsScreenObject(vendorExt->desktop.parent))
      screenObj = (XmScreenObject)vendorExt->desktop.parent;
    else
      screenObj = _XmGetScreenObject(wid, NULL, NULL);

#ifdef DEC_MOTIF_BUG_FIX
    if (screenObj == (XmScreenObject)
#else
    if (screenObj->desktop.parent ==
#endif
	vendorExt->desktop.parent)
      isScreenShell = TRUE;
    else
      isScreenShell = FALSE;

    switch(event->type) {
      case MapNotify:
	if (event->xmap.window != XtWindow(wid))
	    return;

	if (isScreenShell)
	  {
	      screenObj->screen.shellMapped = TRUE;
	  }
#ifdef notdef
	/*
	 * The wm may be mapping and unmapping us in different
	 * desktops so we need to keep our state enough in sync so
	 * that requests to manage/pop wont be ignored |||
	 */
	if (vePPtr->lastMapRequest < event->xmap.serial)
	  {
#ifdef XUI_COMPATIBILITY
	      int i;
#endif
	      switch (vePPtr->mapStyle)
		{
		  case _XmRAW_MAP:
#ifdef XUI_COMPATIBILITY
		/*
		 *  If the child is an XUI widget,
		 *  don't do this (It broke OPS5...)
	   	 */
	       	    for (i= 0; i < w->composite.num_children; i++)
		      if (XtIsWidget(w->composite.children[i]))
		      {
			if (!IsXUIV2Widget(w->composite.children[i]))
			    w->core.managed = True;
			break;			
		      }
#else
		    w->core.managed = True;
#endif
		    break;
		  case _XmPOPUP_MAP:
		    w->shell.popped_up = True;
		    break;
		  case _XmMANAGE_MAP:
		    vePPtr->old_managed->core.managed = True;
		    break;
		}
	  }
	if (!(screenObj->screen.mwmPresent))
	  {
	      /*
	       * maybe we should try to look at the wm_frame for
	       * external reposition ???
	       */
	  }
	/*
	 * check if they're not using XtPopup and aren't toplevel!!!
	 */
	if (XtParent(wid) &&
	    event->xany.serial > (vendorExt->vendor.lastMapRequest))
	  {
	      XtWarning("you should be using XtPopup");
	  }
#endif /* notdef */
	break;
      case UnmapNotify:
	if (event->xunmap.window != XtWindow(wid))
	    return;

	/*
	 * try to keep the pop up field synced up so it won't disallow
	 * a new pop up request.
	 */
#ifdef notdef
	if (vePPtr->lastMapRequest < event->xunmap.serial)
	  {
	      switch (vePPtr->mapStyle)
		{
		  case _XmRAW_MAP:
		    /* 
		     * work around brain dead client code since you
		     * shouldn't managing/unmanaging a shell in order
		     * to pop it up and down. It can't hurt :-) ||| 
		     */
		    w->core.managed = False;
		    break;
		  case _XmPOPUP_MAP:
		    /* 
		     * this is sleazy but the popdown will cause a
		     * withdraw window which can confuse the wm
		     */
		    w->shell.popped_up = False;
		    break;
		  case _XmMANAGE_MAP:
		    vePPtr->old_managed->core.managed = False;
		    break;
		}
	  }
#endif /* notdef */
	if (isScreenShell)
	  {
	      screenObj->screen.shellMapped = FALSE;
	  }
	/* 
	 * make sure we have good coords
	 */
	XtTranslateCoords((Widget)w, 0, 0, &tmpx, &tmpy);
	/*
	 * if the offsets match up, then offset our values so we'll go in
	 * the same place the next time.
	 */
	if ((vePPtr->xAtMap != w->core.x) ||
	    (vePPtr->yAtMap != w->core.y))
	  {
	      if (screenObj->screen.mwmPresent)
		{
		    if (vePPtr->lastOffsetSerial &&
			(vePPtr->lastOffsetSerial >= 
			 vendorExt->shell.lastConfigureRequest) &&
			((vePPtr->xOffset + vePPtr->xAtMap) == w->core.x) &&
			((vePPtr->yOffset + vePPtr->yAtMap) == w->core.y))
		      {
			  w->core.x += vePPtr->xOffset;
			  w->core.y += vePPtr->yOffset;
			  vePPtr->externalReposition = False;
		      }
		    else
		      {
			  vePPtr->externalReposition = True;
		      }
		}
	      else
		vePPtr->externalReposition = True;
	  }
	break;

      case ConfigureNotify:
	if (event->xconfigure.window != XtWindow(wid))
	    return;

	/*
	 * only process configureNotifies that aren't stale
	 */
	if (event->xany.serial <
	    shellExt->shell.lastConfigureRequest)
	  {
	      /*
	       *  make sure the hard wired event handler in shell is not called
	       */
	      if (shellExt->shell.useAsyncGeometry)
		*continue_to_dispatch = False;
	  }
	else
	  {
#define NEQ(x)	( w->core.x != event->xconfigure.x )
	      if( NEQ(width) || NEQ(height) || NEQ(border_width) ) {
		  sizechanged = TRUE;
	      }
#undef NEQ
	      w->core.width = event->xconfigure.width;
	      w->core.height = event->xconfigure.height;
	      w->core.border_width = event->xconfigure.border_width;
	      if (event->xany.send_event /* ICCCM compliant synthetic ev */
		  /* || w->shell.override_redirect */
		  || w->shell.client_specified & _XtShellNotReparented)
		{
		    w->core.x = event->xconfigure.x;
		    w->core.y = event->xconfigure.y;
		    w->shell.client_specified |= _XtShellPositionValid;
		}
	      else
#ifdef DEC_MOTIF_BUG_FIX_DISABLED
/* this 'bug fix' disabled due to interaction problems with twm
   and questions about its original intent.  -ddhill */
	      {
		    Window root, parent, child, *children;
		    unsigned int nchildren;
		    int real_x, real_y;

		    XQueryTree ( XtDisplay(w), event->xconfigure.window,
				 &root, &parent, &children, &nchildren);

		    if (nchildren) XFree (children);

		    XTranslateCoordinates ( XtDisplay (w), parent,
					    root, event->xconfigure.x,
					    event->xconfigure.y, &real_x,
					    &real_y, &child );
					    
		    w->core.x = real_x;
		    w->core.y = real_y;
		    w->shell.client_specified |= _XtShellPositionValid;
	      }
#else 
		w->shell.client_specified &= ~_XtShellPositionValid;
#endif
	      if (XtIsWMShell(wid) && !wmshell->wm.wait_for_wm) {
		  /* Consider trusting the wm again */
		  register struct _OldXSizeHints *hintp
		    = &wmshell->wm.size_hints;
#define EQ(x) (hintp->x == w->core.x)
		  if (EQ(x) && EQ(y) && EQ(width) && EQ(height)) {
		      wmshell->wm.wait_for_wm = TRUE;
		  }
#undef EQ
	      }
	  }		    
	break;
      case ReparentNotify:
	if (event->xreparent.window == XtWindow(w)) {
	    if (event->xreparent.parent != RootWindowOfScreen(XtScreen(w)))
	      {
		  w->shell.client_specified &= ~_XtShellNotReparented;
		  /*
		   * check to see if it's mwm
		   */
		  if (((Object)vendorExt == (Object)screenObj) ||
		      !(screenObj->screen.shellMapped))
		    screenObj->screen.mwmPresent = XmIsMotifWMRunning((Widget)w);
	      }
	    else
	      {
		  w->shell.client_specified |= _XtShellNotReparented;
	      }
	    w->shell.client_specified &= ~_XtShellPositionValid;
	}
	return;
	
      default:
	return;
    }
    
    if (sizechanged && 
	XtClass(w)->core_class.resize != (XtWidgetProc) NULL)
	(*(XtClass(w)->core_class.resize))((Widget) w);
    
}



/************************************************************************
 *  DeleteWindowHandler
 *
 ************************************************************************/
static XtCallbackProc DeleteWindowHandler(w, closure, call_data)
    VendorShellWidget	w;
    caddr_t		closure, call_data;
{
    XmVendorShellExtObject ve = (XmVendorShellExtObject) closure;

    switch(ve->vendor.delete_response)
      {
	case XmUNMAP:
	  XtPopdown((Widget)w);
	  break;
	  
	case XmDESTROY:
	  if (XtIsApplicationShell((Widget)w))
	    {
		XtDestroyApplicationContext
		  (XtWidgetToApplicationContext((Widget)w));
		exit(0);
	    }
	  else
	    XtDestroyWidget((Widget)w);
	  break;
	  
	case XmDO_NOTHING:
	default:
	  break;
      }
}    


/************************************************************************
 *
 *     OffsetHandler
 *
 ************************************************************************/
static XtCallbackProc OffsetHandler(shell, clientData, callData)
    Widget		shell;
    XtPointer		clientData;
    XmAnyCallbackStruct	*callData;
{
    XClientMessageEvent		*offsetEvent;
    XmVendorShellExtObject	ve = (XmVendorShellExtObject)clientData;

    offsetEvent = (XClientMessageEvent *) callData->event;

    ve->vendor.lastOffsetSerial = offsetEvent->serial;
    ve->vendor.xOffset = (Position) offsetEvent->data.l[1];
    ve->vendor.yOffset = (Position) offsetEvent->data.l[2];
}


/************************************************************************
 *
 *     InitializePrehook
 *
 ************************************************************************/
static void InitializePrehook(req, new, args, num_args)
    Widget	req, new;
    ArgList	args;
    Cardinal	*num_args;
{
#ifndef NO_CLASS_PART_INIT_HOOK
    XmExtObjectClass		ec = (XmExtObjectClass) XtClass(new);
    XmBaseClassExt		*wcePtr;
    XmExtObject			ne = (XmExtObject) new;
    XmExtObject			re = (XmExtObject) req;
    Widget			parent = ne->ext.logicalParent;
    XmExtObjectClass		pec = (XmExtObjectClass) XtClass(parent);
    XmBaseClassExt		*pcePtr;
    XmWidgetExtData		extData;

    wcePtr = _XmGetBaseClassExtPtr(ec, XmQmotif);
    pcePtr = _XmGetBaseClassExtPtr(pec, XmQmotif);

    if ((*wcePtr)->use_sub_resources)
      {
	  /*
	   * get a uncompiled resource list to use with
	   * XtGetSubresources. We can't do this in
	   * ClassPartInitPosthook because Xt doesn't set class_inited at
	   * the right place and thereby mishandles the
	   * XtGetResourceList call
	   */
	  if ((*wcePtr)->ext_resources == NULL)
	    {
		ec->object_class.resources =
		  (*wcePtr)->compiled_ext_resources;
		ec->object_class.num_resources =		
		  (*wcePtr)->num_ext_resources;

		XtGetResourceList((WidgetClass)ec,
				  &((*wcePtr)->ext_resources),
				  &((*wcePtr)->num_ext_resources));

	    }
	  if ((*pcePtr)->ext_resources == NULL)
	    {
		XtGetResourceList((WidgetClass)pec,
				  &((*pcePtr)->ext_resources),
				  &((*pcePtr)->num_ext_resources));
	    }
	  XtGetSubresources(parent,
			    (XtPointer)new,
			    NULL, NULL,
			    (*wcePtr)->ext_resources,
			    (*wcePtr)->num_ext_resources,
			    args, *num_args);

	  extData = (XmWidgetExtData) XtCalloc(sizeof(XmWidgetExtDataRec), 1);
	  _XmPushWidgetExtData(parent, extData, ne->ext.extensionType);
	  
	  extData->widget = new;
	  extData->reqWidget = (Widget)
	    XtMalloc(XtClass(new)->core_class.widget_size);
	  bcopy((char *)req, (char *)extData->reqWidget,
		XtClass(new)->core_class.widget_size);
	  
	  /*  Convert the fields from unit values to pixel values  */

	  XtGetSubresources(parent,
			    (XtPointer)parent,
			    NULL, NULL,
			    (*pcePtr)->ext_resources,
			    (*pcePtr)->num_ext_resources,
			    args, *num_args);

	  _XmExtImportArgs(new, args, num_args);
      }
#endif /* NO_CLASS_PART_INIT_HOOK */
}


/************************************************************************
 *
 *  Destroy
 *
 *    This needs to be in the ext object because the extension gets
 *    blown away before the primary does since it's a child. Otherwise
 *    we'd have it in the primary.
 *
 ************************************************************************/
static void VendorDestroy(ve)
    XmVendorShellExtObject ve;
{
    if (ve->vendor.mwm_menu)
      XtFree(ve->vendor.mwm_menu);
    _XmDestroyFocusData(ve->vendor.focus_data);
}
