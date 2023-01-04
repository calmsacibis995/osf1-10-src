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
 * $XConsortium: defaults.c,v 1.1 91/05/10 14:46:27 keith Exp $
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

#include    <X11/X.h>
#include    <X11/Xproto.h>
#include    <server/include/servermd.h>

#ifndef DEFAULT_BIT_ORDER
#ifdef BITMAP_BIT_ORDER
#define DEFAULT_BIT_ORDER BITMAP_BIT_ORDER
#else
#define DEFAULT_BIT_ORDER MSBFirst
#endif
#endif

#ifndef DEFAULT_BYTE_ORDER
#ifdef IMAGE_BYTE_ORDER
#define DEFAULT_BYTE_ORDER IMAGE_BYTE_ORDER
#else
#define DEFAULT_BYTE_ORDER MSBFirst
#endif
#endif

#ifndef DEFAULT_GLYPH_PAD
#ifdef GLYPHPADBYTES
#define DEFAULT_GLYPH_PAD GLYPHPADBYTES
#else
#define DEFAULT_GLYPH_PAD 4
#endif
#endif

#ifndef DEFAULT_SCAN_UNIT
#define DEFAULT_SCAN_UNIT 1
#endif

FontDefaultFormat (bit, byte, glyph, scan)
    int	    *bit, *byte, *glyph, *scan;
{
    *bit = DEFAULT_BIT_ORDER;
    *byte = DEFAULT_BYTE_ORDER;
    *glyph = DEFAULT_GLYPH_PAD;
    *scan = DEFAULT_SCAN_UNIT;
}
