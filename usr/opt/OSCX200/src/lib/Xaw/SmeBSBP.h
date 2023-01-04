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
 * $XConsortium: SmeBSBP.h,v 1.6 89/12/11 15:20:15 kit Exp $
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
 * SmeP.h - Private definitions for Sme object
 * 
 */

#ifndef _XawSmeBSBP_h
#define _XawSmeBSBP_h

/***********************************************************************
 *
 * Sme Object Private Data
 *
 ***********************************************************************/

#include <X11/Xaw/SmeP.h>
#include <X11/Xaw/SmeBSB.h>

/************************************************************
 *
 * New fields for the Sme Object class record.
 *
 ************************************************************/

typedef struct _SmeBSBClassPart {
  XtPointer extension;
} SmeBSBClassPart;

/* Full class record declaration */
typedef struct _SmeBSBClassRec {
    RectObjClassPart       rect_class;
    SmeClassPart     sme_class;
    SmeBSBClassPart  sme_bsb_class;
} SmeBSBClassRec;

extern SmeBSBClassRec smeBSBClassRec;

/* New fields for the Sme Object record */
typedef struct {
    /* resources */
    String label;		/* The entry label. */
    int vert_space;		/* extra vert space to leave, as a percentage
				   of the font height of the label. */
    Pixmap left_bitmap, right_bitmap; /* bitmaps to show. */
    Dimension left_margin, right_margin; /* left and right margins. */
    Pixel foreground;		/* foreground color. */
    XFontStruct * font;		/* The font to show label in. */
    XtJustify justify;		/* Justification for the label. */

/* private resources. */

    Boolean set_values_area_cleared; /* Remember if we need to unhighlight. */
    GC norm_gc;			/* noral color gc. */
    GC rev_gc;			/* reverse color gc. */
    GC norm_gray_gc;		/* Normal color (grayed out) gc. */
    GC invert_gc;		/* gc for flipping colors. */

    Dimension left_bitmap_width; /* size of each bitmap. */
    Dimension left_bitmap_height;
    Dimension right_bitmap_width;
    Dimension right_bitmap_height;

} SmeBSBPart;

/****************************************************************
 *
 * Full instance record declaration
 *
 ****************************************************************/

typedef struct _SmeBSBRec {
  ObjectPart         object;
  RectObjPart        rectangle;
  SmePart	     sme;
  SmeBSBPart   sme_bsb;
} SmeBSBRec;

/************************************************************
 *
 * Private declarations.
 *
 ************************************************************/

#endif /* _XawSmeBSBP_h */
