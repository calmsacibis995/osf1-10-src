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
/* $XConsortium: fservestr.h,v 1.9 92/11/18 21:31:07 gildea Exp $ */
/*
 * Copyright 1990 Network Computing Devices
 *
 * Permission to use, copy, modify, distribute, and sell this software and
 * its documentation for any purpose is hereby granted without fee, provided
 * that the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of Network Computing Devices not be
 * used in advertising or publicity pertaining to distribution of the
 * software without specific, written prior permission.  Network Computing
 * Devices makes no representations about the suitability of this software
 * for any purpose.  It is provided "as is" without express or implied
 * warranty.
 *
 * NETWORK COMPUTING DEVICES DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS
 * SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS,
 * IN NO EVENT SHALL NETWORK COMPUTING DEVICES BE LIABLE FOR ANY SPECIAL,
 * INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
 * LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE
 * OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE
 * OR PERFORMANCE OF THIS SOFTWARE.
 *
 * Author:  	Dave Lemke, Network Computing Devices, Inc
 */

#ifndef _FSERVESTR_H_
#define _FSERVESTR_H_

#include	"fserve.h"
#include	"fsio.h"

/*
 * font server data structures
 */
/*
 * font server private storage
 */

/*
 * XXX -- currently, all the glyph & metric data is sucked over from the
 * FS, so this is pretty simplistic.  needs work to deal with caching
 * properly.
 */

typedef struct _fs_font {
    CharInfoPtr pDefault;
    CharInfoPtr encoding;
    CharInfoPtr inkMetrics;
    pointer     bitmaps;
}           FSFontRec, *FSFontPtr;

/* FS special data for the font */
typedef struct _fs_font_data {
    long        fontid;
    int		generation;	/* FS generation when opened */
    FontPathElementPtr fpe;
    Bool        complete;	/* all glyphs sucked over? */
}           FSFontDataRec;

/* OpenFont specific data for blocked request */
typedef struct _fs_blocked_font {
    FontPtr     pfont;
    long        fontid;
    int         state;		/* how many of the replies have landed */
    int         errcode;
    int         flags;
    fsBitmapFormat format;
}           FSBlockedFontRec;

/* LoadGlyphs data for blocked request */
typedef struct _fs_blocked_glyphs {
    FontPtr     pfont;
    fsRange     expected_range;
    Bool        done;
}           FSBlockedGlyphRec;

/* LoadExtents data for blocked request */
typedef struct _fs_blocked_extents {
    FontPtr     pfont;
    fsRange    *expected_ranges;
    int         nranges;
    Bool        done;
    unsigned long nextents;
    fsXCharInfo *extents;
}           FSBlockedExtentRec;

/* LoadBitmaps data for blocked request */
typedef struct _fs_blocked_bitmaps {
    FontPtr     pfont;
    fsRange    *expected_ranges;
    int         nranges;
    Bool        done;
    unsigned long size;
    unsigned long nglyphs;
    fsOffset32   *offsets;
    pointer     gdata;
}           FSBlockedBitmapRec;

/* state for blocked ListFonts */
typedef struct _fs_blocked_list {
    FontNamesPtr names;
    int         patlen;
    int         errcode;
    Bool        done;
}           FSBlockedListRec;

/* state for blocked ListFontsWithInfo */
typedef struct _fs_blocked_list_info {
    int         status;
    char       *name;
    int         namelen;
    FontInfoPtr pfi;
    int         remaining;
    int         errcode;
}           FSBlockedListInfoRec;

/* state for blocked request */
typedef struct _fs_block_data {
    int			    type;	/* Open Font, LoadGlyphs, ListFonts,
					 * ListWithInfo */
    pointer		    client;	    /* who wants it */
    int			    sequence_number;/* expected */
    fsGenericReply	    header;
    pointer		    data;	    /* type specific data */
    struct _fs_block_data   *depending;	    /* clients depending on this one */
    struct _fs_block_data   *next;
}           FSBlockDataRec;

/* state for reconnected to dead font server */
typedef struct _fs_reconnect {
    int	    i;
} FSReconnectRec, *FSReconnectPtr;


#if __STDC__ && !defined(UNIXCPP)
#define fsCat(x,y) x##_##y
#else
#define fsCat(x,y) x/**/_/**/y
#endif


/* copy XCharInfo parts of a protocol reply into a xCharInfo */

#define fsUnpack_XCharInfo(packet, structure) \
    (structure)->leftSideBearing = fsCat(packet,left); \
    (structure)->rightSideBearing = fsCat(packet,right); \
    (structure)->characterWidth = fsCat(packet,width); \
    (structure)->ascent = fsCat(packet,ascent); \
    (structure)->descent = fsCat(packet,descent); \
    (structure)->attributes = fsCat(packet,attributes)


/* copy XFontInfoHeader parts of a protocol reply into a FontInfoRec */

#define fsUnpack_XFontInfoHeader(packet, structure) \
    (structure)->allExist = ((packet)->font_header_flags & FontInfoAllCharsExist) != 0; \
    (structure)->drawDirection = \
        ((packet)->font_header_draw_direction == LeftToRightDrawDirection) ? \
	LeftToRight : RightToLeft; \
    (structure)->inkInside = ((packet)->font_header_flags & FontInfoInkInside) != 0; \
 \
    (structure)->firstRow = (packet)->font_header_char_range_min_char_high; \
    (structure)->firstCol = (packet)->font_header_char_range_min_char_low; \
    (structure)->lastRow = (packet)->font_header_char_range_max_char_high; \
    (structure)->lastCol = (packet)->font_header_char_range_max_char_low; \
    (structure)->defaultCh = (packet)->font_header_default_char_low \
                           + ((packet)->font_header_default_char_high << 8); \
 \
    (structure)->fontDescent = (packet)->font_header_font_descent; \
    (structure)->fontAscent = (packet)->font_header_font_ascent; \
 \
    fsUnpack_XCharInfo((packet)->font_header_min_bounds, &(structure)->minbounds); \
    fsUnpack_XCharInfo((packet)->font_header_min_bounds, &(structure)->ink_minbounds); \
    fsUnpack_XCharInfo((packet)->font_header_max_bounds, &(structure)->maxbounds); \
    fsUnpack_XCharInfo((packet)->font_header_max_bounds, &(structure)->ink_maxbounds)


#endif				/* _FSERVESTR_H_ */
