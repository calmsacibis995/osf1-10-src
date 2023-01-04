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
 * $XConsortium: SmeLineP.h,v 1.3 89/12/11 15:20:20 kit Exp $
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
 *
 * Author:  Chris D. Peterson, MIT X Consortium
 */

/* 
 * SmeLineP.h - Private definitions for SmeLine widget
 * 
 */

#ifndef _XawSmeLineP_h
#define _XawSmeLineP_h

/***********************************************************************
 *
 * SmeLine Widget Private Data
 *
 ***********************************************************************/

#include <X11/Xaw/SmeP.h>
#include <X11/Xaw/SmeLine.h>

/************************************************************
 *
 * New fields for the SmeLine widget class record.
 *
 ************************************************************/

typedef struct _SmeLineClassPart {
  XtPointer extension;
} SmeLineClassPart;

/* Full class record declaration */
typedef struct _SmeLineClassRec {
    RectObjClassPart    rect_class;
    SmeClassPart	sme_class;
    SmeLineClassPart	sme_line_class;
} SmeLineClassRec;

extern SmeLineClassRec smeLineClassRec;

/* New fields for the SmeLine widget record */
typedef struct {
    /* resources */
    Pixel foreground;		/* Foreground color. */
    Pixmap stipple;		/* Line Stipple. */
    Dimension line_width;	/* Width of the line. */

    /* private data.  */

    GC gc;			/* Graphics context for drawing line. */
} SmeLinePart;

/****************************************************************
 *
 * Full instance record declaration
 *
 ****************************************************************/

typedef struct _SmeLineRec {
  ObjectPart     object;
  RectObjPart    rectangle;
  SmePart	 sme;
  SmeLinePart	 sme_line;
} SmeLineRec;

/************************************************************
 *
 * Private declarations.
 *
 ************************************************************/

#endif /* _XawSmeLineP_h */
