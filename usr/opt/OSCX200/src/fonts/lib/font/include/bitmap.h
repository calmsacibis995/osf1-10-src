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
 * $XConsortium: bitmap.h,v 1.3 91/09/11 11:12:52 keith Exp $
 *
 * Copyright 1990 Massachusetts Institute of Technology
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
 * Author:  Keith Packard, MIT X Consortium
 */

#ifndef _BITMAP_H_
#define _BITMAP_H_

#include <fontfileio.h>
#include <stdio.h>  /* just for NULL */

/*
 * Internal format used to store bitmap fonts
 */

typedef struct _BitmapExtra {
    Atom       *glyphNames;
    int        *sWidths;
    CARD32      bitmapsSizes[GLYPHPADOPTIONS];
    FontInfoRec info;
}           BitmapExtraRec, *BitmapExtraPtr;

typedef struct _BitmapFont {
    unsigned    version_num;
    int         num_chars;
    int         num_tables;
    CharInfoPtr metrics;	/* font metrics, including glyph pointers */
    xCharInfo  *ink_metrics;	/* ink metrics */
    char       *bitmaps;	/* base of bitmaps, useful only to free */
    CharInfoPtr *encoding;	/* array of char info pointers */
    CharInfoPtr pDefault;	/* default character */
    BitmapExtraPtr bitmapExtra;	/* stuff not used by X server */
}           BitmapFontRec, *BitmapFontPtr;

extern int  bitmapReadFont(), bitmapReadFontInfo();
extern int  bitmapGetGlyphs(), bitmapGetMetrics();
extern int  bitmapGetBitmaps(), bitmapGetExtents();
extern void bitmapUnloadFont();

extern void bitmapComputeFontBounds();
extern void bitmapComputeFontInkBounds();

#endif				/* _BITMAP_H_ */
