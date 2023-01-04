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
/* $XConsortium: ObjectP.h,v 1.10 89/10/04 12:22:44 swick Exp $ */
/* $oHeader: ObjectP.h,v 1.2 88/08/18 15:55:35 asente Exp $ */
/***********************************************************
Copyright 1987, 1988 by Digital Equipment Corporation, Maynard, Massachusetts,
and the Massachusetts Institute of Technology, Cambridge, Massachusetts.

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its 
documentation for any purpose and without fee is hereby granted, 
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in 
supporting documentation, and that the names of Digital or MIT not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.  

DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.

******************************************************************/

#ifndef _Xt_ObjectP_h_
#define _Xt_ObjectP_h_
#if defined(VMS) || defined (__VMS)
#include <X11/apienvset.h>
#endif

#include <X11/Object.h>

/**********************************************************
 * Object Instance Data Structures
 *
 **********************************************************/
/* these fields match CorePart and can not be changed */

typedef struct _ObjectPart {
    Widget          self;               /* pointer to widget itself          */
    WidgetClass     widget_class;       /* pointer to Widget's ClassRec      */
    Widget          parent;             /* parent widget                     */
    XrmName         xrm_name;           /* widget resource name quarkified   */
    Boolean         being_destroyed;    /* marked for destroy                */
    XtCallbackList  destroy_callbacks;  /* who to call when widget destroyed */
    XtPointer       constraints;        /* constraint record                 */
} ObjectPart;

typedef struct _ObjectRec {
    ObjectPart  object;
} ObjectRec;

/********************************************************
 * Object Class Data Structures
 *
 ********************************************************/
/* these fields match CoreClassPart and can not be changed */
/* ideally these structures would only contain the fields required;
   but because the CoreClassPart cannot be changed at this late date
   extraneous fields are necessary to make the field offsets match */

typedef struct _ObjectClassPart {

    WidgetClass     superclass;         /* pointer to superclass ClassRec   */
    String          class_name;         /* widget resource class name       */
    Cardinal        widget_size;        /* size in bytes of widget record   */
    XtProc          class_initialize;   /* class initialization proc        */
    XtWidgetClassProc class_part_initialize; /* dynamic initialization      */
    XtEnum          class_inited;       /* has class been initialized?      */
    XtInitProc      initialize;         /* initialize subclass fields       */
    XtArgsProc      initialize_hook;    /* notify that initialize called    */
    XtProc          obj1;		/* NULL                             */
#ifdef DEC_BUG_FIX
    /* MIT fix-trackers patch */
    XtPointer       obj2;               /* NULL                             */
#else
    XtProc          obj2;               /* NULL                             */
#endif
    Cardinal        obj3;               /* NULL                             */
    XtResourceList  resources;          /* resources for subclass fields    */
    Cardinal        num_resources;      /* number of entries in resources   */
    XrmClass        xrm_class;          /* resource class quarkified        */
    Boolean         obj4;               /* NULL                             */
#ifdef DEC_BUG_FIX
    /* MIT fix-trackers patch */
    XtEnum          obj5;               /* NULL                             */
#else
    Boolean         obj5;               /* NULL                             */
#endif
    Boolean         obj6;               /* NULL				    */
    Boolean         obj7;               /* NULL                             */
    XtWidgetProc    destroy;            /* free data for subclass pointers  */
    XtProc          obj8;               /* NULL                             */
    XtProc          obj9;               /* NULL			            */
    XtSetValuesFunc set_values;         /* set subclass resource values     */
    XtArgsFunc      set_values_hook;    /* notify that set_values called    */
    XtProc          obj10;              /* NULL                             */
    XtArgsProc      get_values_hook;    /* notify that get_values called    */
    XtProc          obj11;              /* NULL                             */
    XtVersionType   version;            /* version of intrinsics used       */
    XtPointer       callback_private;   /* list of callback offsets       */
    String          obj12;              /* NULL                             */
    XtProc          obj13;              /* NULL                             */
    XtProc          obj14;              /* NULL                             */
    XtPointer       extension;          /* pointer to extension record      */
}ObjectClassPart;

typedef struct _ObjectClassRec {
    ObjectClassPart object_class;
} ObjectClassRec;


externalref ObjectClassRec objectClassRec;

#if defined(VMS) || defined (__VMS)
#include <X11/apienvrst.h>
#endif
#endif /*_Xt_ObjectP_h_*/
