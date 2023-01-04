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
 * $XConsortium: ToggleP.h,v 1.8 91/06/20 16:15:51 converse Exp $
 *
 * Copyright 1989 Massachusetts Institute of Technology
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of M.I.T. not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  M.I.T. makes no representations about the
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 * M.I.T. DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING ALL
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL M.I.T.
 * BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN 
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

/* 
 * ToggleP.h - Private definitions for Toggle widget
 * 
 * Author: Chris D. Peterson
 *         MIT X Consortium
 *         kit@expo.lcs.mit.edu
 *  
 * Date:   January 12, 1989
 *
 */

#ifndef _XawToggleP_h
#define _XawToggleP_h

#include <X11/Xaw/Toggle.h>
#include <X11/Xaw/CommandP.h>

/***********************************************************************
 *
 * Toggle Widget Private Data
 *
 ***********************************************************************/

#define streq(a, b) ( strcmp((a), (b)) == 0 )

typedef struct _RadioGroup {
  struct _RadioGroup *prev, *next; /* Pointers to other elements in group. */
  Widget widget;		  /* Widget corrosponding to this element. */
} RadioGroup;

/************************************
 *
 *  Class structure
 *
 ***********************************/

   /* New fields for the Toggle widget class record */
typedef struct _ToggleClass  {
    XtActionProc Set;
    XtActionProc Unset;
    XtPointer extension;
} ToggleClassPart;

   /* Full class record declaration */
typedef struct _ToggleClassRec {
    CoreClassPart	core_class;
    SimpleClassPart	simple_class;
    LabelClassPart	label_class;
    CommandClassPart	command_class;
    ToggleClassPart     toggle_class;
} ToggleClassRec;

extern ToggleClassRec toggleClassRec;

/***************************************
 *
 *  Instance (widget) structure 
 *
 **************************************/

    /* New fields for the Toggle widget record */
typedef struct {
    /* resources */
    Widget      widget;
    XtPointer   radio_data;

    /* private data */
    RadioGroup * radio_group;
} TogglePart;

   /* Full widget declaration */
typedef struct _ToggleRec {
    CorePart         core;
    SimplePart	     simple;
    LabelPart	     label;
    CommandPart	     command;
    TogglePart       toggle;
} ToggleRec;

#endif /* _XawToggleP_h */


