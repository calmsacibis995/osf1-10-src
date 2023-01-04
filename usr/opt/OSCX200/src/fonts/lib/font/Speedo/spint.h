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
/* $XConsortium: spint.h,v 1.6 92/09/17 11:57:07 gildea Exp $ */
/*
 * Copyright 1990, 1991 Network Computing Devices;
 * Portions Copyright 1987 by Digital Equipment Corporation and the
 * Massachusetts Institute of Technology
 *
 * Permission to use, copy, modify, distribute, and sell this software and
 * its documentation for any purpose is hereby granted without fee, provided
 * that the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the names of Network Computing Devices, Digital or
 * MIT not be used in advertising or publicity pertaining to distribution of
 * the software without specific, written prior permission.
 *
 * NETWORK COMPUTING DEVICES, DIGITAL AND MIT DISCLAIM ALL WARRANTIES WITH
 * REGARD TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS, IN NO EVENT SHALL NETWORK COMPUTING DEVICES, DIGITAL OR MIT BE
 * LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#ifndef _SPINT_H_
#define _SPINT_H_

#ifndef DEBUG
#define NDEBUG
#endif

#include	<stdio.h>
#include	"fontfilest.h"
#include	"speedo.h"

#define	SaveMetrics	0x1

#define GLWIDTHBYTESPADDED(bits,nbytes) \
        ((nbytes) == 1 ? (((bits)+7)>>3)        /* pad to 1 byte */ \
        :(nbytes) == 2 ? ((((bits)+15)>>3)&~1)  /* pad to 2 bytes */ \
        :(nbytes) == 4 ? ((((bits)+31)>>3)&~3)  /* pad to 4 bytes */ \
        :(nbytes) == 8 ? ((((bits)+63)>>3)&~7)  /* pad to 8 bytes */ \
        : 0)

#define GLYPH_SIZE(ch, nbytes)          \
        GLWIDTHBYTESPADDED((ch)->metrics.rightSideBearing - \
                        (ch)->metrics.leftSideBearing, (nbytes))

#define	MasterFileOpen	0x1

typedef struct _sp_master {
    FontEntryPtr    entry;	/* back pointer */
    FILE       *fp;
    char       *fname;
    ufix8      *f_buffer;
    ufix8      *c_buffer;
    char       *copyright;
    ufix8      *key;
    buff_t      font;
    buff_t      char_data;
    ufix16      mincharsize;
    int         first_char_id;
    int         num_chars;
    int         max_id;
    int         state;		/* open, closed */
    int         refcount;	/* number of instances */
    int        *enc;
    int         enc_size;
}           SpeedoMasterFontRec, *SpeedoMasterFontPtr;

typedef struct _cur_font_stats {
    fsBitmapFormat format;
    /* current glyph info */
    ufix16      char_index;
    ufix16      char_id;

    fix15       bit_width,
                bit_height;
    fix15       cur_y;
    int         bpr;

    /*
     * since Speedo returns extents that are not identical to what it feeds to
     * the bitmap builder, and we want to be able to use the extents for
     * preformance reasons, some of the bitmaps require padding out.  the next
     * two flags keep track of this.
     */
    fix15       last_y;
    int         trunc;

    pointer     bp;
    int         scanpad;
}           CurrentFontValuesRec, *CurrentFontValuesPtr;


typedef struct _sp_font {
    struct _sp_master *master;
    specs_t     specs;

    FontEntryPtr    entry;

    FontScalableRec vals;

    /* char & metric data */
    CharInfoPtr encoding;
    CharInfoPtr pDefault;
    pointer     bitmaps;

#ifdef DEBUG
    unsigned long bitmap_size;
#endif

}           SpeedoFontRec, *SpeedoFontPtr;

extern SpeedoFontPtr sp_fp_cur;

extern int  sp_open_font();
extern int  sp_open_master();
extern void sp_close_font();
extern void sp_close_master_font();
extern void sp_close_master_file();
extern void sp_reset_master();
extern void SpeedoErr();

extern void sp_make_standard_props();
extern void sp_make_header();
extern void sp_compute_bounds();
extern void sp_compute_props();
extern int  sp_build_all_bitmaps();
extern unsigned long sp_compute_data_size();

extern int  sp_bics_map[];
extern int  sp_bics_map_size;

#ifdef EXTRAFONTS
extern int  adobe_map[];
extern int  adobe_map_size;

#endif

#endif				/* _SPINT_H_ */
