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
/* vfake.c - Implementation module for the VFake widget

*****************************************************************************
*									    *
*  COPYRIGHT (c) 1988, 1989, 1990  BY               			    *
*  DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASSACHUSETTS.		    *
*  ALL RIGHTS RESERVED.						    	    *
* 									    *
*  THIS SOFTWARE IS FURNISHED UNDER A LICENSE AND MAY BE USED AND COPIED    *
*  ONLY IN  ACCORDANCE WITH  THE  TERMS  OF  SUCH  LICENSE  AND WITH THE    *
*  INCLUSION OF THE ABOVE COPYRIGHT NOTICE. THIS SOFTWARE OR  ANY  OTHER    *
*  COPIES THEREOF MAY NOT BE PROVIDED OR OTHERWISE MADE AVAILABLE TO ANY    *
*  OTHER PERSON.  NO TITLE TO AND OWNERSHIP OF  THE  SOFTWARE IS  HEREBY    *
*  TRANSFERRED.							            *
* 									    *
*  THE INFORMATION IN THIS SOFTWARE IS  SUBJECT TO CHANGE WITHOUT NOTICE    *
*  AND  SHOULD  NOT  BE  CONSTRUED AS  A COMMITMENT BY DIGITAL EQUIPMENT    *
*  CORPORATION.							            *
* 									    *
*  DIGITAL ASSUMES NO RESPONSIBILITY FOR THE USE  OR  RELIABILITY OF ITS    *
*  SOFTWARE ON EQUIPMENT WHICH IS NOT SUPPLIED BY DIGITAL.		    *
*									    *
*****************************************************************************

*****************************************************************************
*									    *
*		       DIGITAL INTERNAL USE ONLY			    *
*									    *
*      THIS SOFTWARE IS NOT FORMALLY SUPPORTED AND IS PROVIDED AS IS.       *
*  Do not distribute outside Digital without first contacting the author.   *
*									    *
*		Steve Klein, DECWIN::KLEIN, Digital ZKO			    *
*									    *
*****************************************************************************

This module implements the VFake widget.  It contains all the action
routines, special routines and widget data structures necessary
to create and manipulate a VFake widget.  Compile this module and link the
resulting object into any executable image that uses the VFake widget.

MODIFICATION HISTORY:

21-Feb-1990 (sjk) Initial public release.
*/

#include "vtoolkit_m.h"
#include "vfake.h"

typedef struct {			/* widget class contribution */
    int			notused;
} VFakeClassPart;

typedef struct {			/* widget class record */
    CoreClassPart	core_class;
    CompositeClassPart	composite_class;
    VFakeClassPart	vfake_class;
} VFakeClassRec, *VFakeClass;

typedef struct {			/* Widget record */
    CorePart		core;		/* Core widget fields */
    CompositePart	composite;	/* composite widget fields */

    /* Resource fields */
    Window		window;
} VFakeWidgetRec, *VFakeWidget;

static void DoChangeManaged(w)
    Widget w;
{
}

static XtGeometryResult DoGeometryManager(w, requestP, replyP)
    Widget w;
    XtWidgetGeometry *requestP, *replyP;
{
    if (requestP->request_mode & CWX) w->core.x = requestP->x;
    if (requestP->request_mode & CWY) w->core.y = requestP->y;
    if (requestP->request_mode & CWWidth) w->core.width = requestP->width;
    if (requestP->request_mode & CWHeight) w->core.height = requestP->height;
    if (requestP->request_mode & CWBorderWidth)
	w->core.border_width = requestP->border_width;

    return (XtGeometryYes);		/* just say yes */
}

static void DoRealize(w, maskP, attributesP)
    VFakeWidget w;
    Mask *maskP;
    XSetWindowAttributes *attributesP;
{
    w->core.window = w->window;
}

#define Offset(x) XtOffset(VFakeWidget, x)

static XtResource resources[] = {
 {VFakeNwindow, XtCValue, XtRWindow, sizeof(Window),
  Offset(window), XtRImmediate, 0},
};

externaldef(vfakewidgetclassrec) VFakeClassRec vfakewidgetclassrec = {
    {/* core_class fields	*/
	/* superclass		*/	(WidgetClass)&compositeClassRec,
	/* class_name	  	*/	"VFake",
	/* widget_size	  	*/	sizeof(VFakeWidgetRec),
	/* class_initialize   	*/    	0,
	/* class_part_initialize */	0,
	/* class_inited       	*/	0,
	/* initialize	  	*/	0,
	/* initialize_hook	*/	0,
	/* realize		*/	(void *) DoRealize,
	/* actions		*/    	0,
	/* num_actions	  	*/	0,
	/* resources	  	*/	resources,
	/* num_resources	*/	XtNumber(resources),
	/* xrm_class	  	*/	0,
	/* compress_motion	*/	0,
	/* compress_exposure  	*/	0,
	/* compress_enterleave	*/	0,
	/* visible_interest	*/	0,
	/* destroy		*/	0,
	/* resize		*/	0,
	/* expose		*/	0,
	/* set_values	  	*/	0,
	/* set_values_hook	*/	0,
	/* set_values_almost  	*/	(void *) _XtInherit,
	/* get_values_hook    	*/	0,
	/* accept_focus	  	*/	0,
	/* version		*/	XtVersionDontCheck,
	/* callback_private   	*/	0,
	/* tm_table		*/	0,
	/* query_geometry	*/	0,
	/* display_accelerator	*/	0,
	/* extension		*/	0
    },
    {/* composite class fields	*/
	/* geometry_manager	*/	DoGeometryManager,
	/* change_managed	*/	DoChangeManaged,
	/* insert_child		*/	(void *) _XtInherit,
	/* delete_child		*/	(void *) _XtInherit,
	/* extension		*/	0
    }
};

externaldef(vfakewidgetclass) WidgetClass vfakewidgetclass =
    (WidgetClass)&vfakewidgetclassrec;

Widget VFakeCreate(pW, nameP, argsP, argCnt)
    Widget pW;		/* parent (ignored) */
    char *nameP;	/* widget name */
    Arg *argsP;		/* pointer to argument vector */
    int argCnt;		/* number of arguments in vector */
{
    Widget w = XtCreatePopupShell (nameP, vfakewidgetclass, pW, argsP, argCnt);

    return (w);
}

void VFakeInitializeForDRM()
{
    MrmRegisterClass (MrmwcUnknown, "VFake", "VFakeCreate", VFakeCreate,
	vfakewidgetclass);
}
