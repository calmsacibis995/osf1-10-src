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
* $XConsortium: CommandP.h,v 1.30 90/12/01 13:00:10 rws Exp $
*/


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

/* 
 * CommandP.h - Private definitions for Command widget
 * 
 */

#ifndef _XawCommandP_h
#define _XawCommandP_h

#include <X11/Xaw/Command.h>
#include <X11/Xaw/LabelP.h>

/***********************************************************************
 *
 * Command Widget Private Data
 *
 ***********************************************************************/

typedef enum {
  HighlightNone,		/* Do not highlight. */
  HighlightWhenUnset,		/* Highlight only when unset, this is
				   to preserve current command widget 
				   functionality. */
  HighlightAlways		/* Always highlight, lets the toggle widget
				   and other subclasses do the right thing. */
} XtCommandHighlight;

/************************************
 *
 *  Class structure
 *
 ***********************************/


   /* New fields for the Command widget class record */
typedef struct _CommandClass 
  {
    int makes_compiler_happy;  /* not used */
  } CommandClassPart;

   /* Full class record declaration */
typedef struct _CommandClassRec {
    CoreClassPart	core_class;
    SimpleClassPart	simple_class;
    LabelClassPart	label_class;
    CommandClassPart    command_class;
} CommandClassRec;

extern CommandClassRec commandClassRec;

/***************************************
 *
 *  Instance (widget) structure 
 *
 **************************************/

    /* New fields for the Command widget record */
typedef struct {
    /* resources */
    Dimension   highlight_thickness;
    XtCallbackList callbacks;

    /* private state */
    Pixmap      	gray_pixmap;
    GC          	normal_GC;
    GC          	inverse_GC;
    Boolean     	set;
    XtCommandHighlight	highlighted;
    /* more resources */
    int			shape_style;    
    Dimension		corner_round;
} CommandPart;


/*    XtEventsPtr eventTable;*/


   /* Full widget declaration */
typedef struct _CommandRec {
    CorePart         core;
    SimplePart	     simple;
    LabelPart	     label;
    CommandPart      command;
} CommandRec;

#endif /* _XawCommandP_h */


