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
/* $XConsortium: spfont.c,v 1.18 92/11/18 21:31:09 gildea Exp $ */
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
 *
 * Author: Dave Lemke, Network Computing Devices Inc
 */

/*
 * Speedo font loading
 */

#include	"FSproto.h"
#include	"spint.h"
#include	<server/include/servermd.h>

#ifndef DEFAULT_BIT_ORDER

#ifdef BITMAP_BIT_ORDER
#define DEFAULT_BIT_ORDER BITMAP_BIT_ORDER
#else
#define DEFAULT_BIT_ORDER UNKNOWN_BIT_ORDER
#endif

#endif

extern void SpeedoCloseFont();
static int sp_get_glyphs();
static int sp_get_metrics();
static int sp_load_font();

static CharInfoRec junkDefault;

static int
sp_get_glyphs(pFont, count, chars, charEncoding, glyphCount, glyphs)
    FontPtr     pFont;
    unsigned long count;
    register unsigned char *chars;
    FontEncoding charEncoding;
    unsigned long *glyphCount;	/* RETURN */
    CharInfoPtr *glyphs;	/* RETURN */
{
    SpeedoFontPtr spf;
    unsigned int firstCol;
    register unsigned int numCols;
    unsigned int firstRow;
    unsigned int numRows;
    CharInfoPtr *glyphsBase;
    register unsigned int c;
    register CharInfoPtr pci;
    unsigned int r;
    CharInfoPtr encoding;
    CharInfoPtr pDefault;
    int         itemSize;
    int         err = Successful;

    spf = (SpeedoFontPtr) pFont->fontPrivate;
    encoding = spf->encoding;
    pDefault = spf->pDefault;
    firstCol = pFont->info.firstCol;
    numCols = pFont->info.lastCol - firstCol + 1;
    glyphsBase = glyphs;


    /* XXX - this should be much smarter */
    /* make sure the glyphs are there */
    if (charEncoding == Linear8Bit || charEncoding == TwoD8Bit)
	itemSize = 1;
    else
	itemSize = 2;

#ifdef notyet
    if (!fsd->complete)
	err = fs_load_glyphs(NULL, pFont, count, itemSize, chars);
#endif

    if (err != Successful)
	return err;

    switch (charEncoding) {

    case Linear8Bit:
    case TwoD8Bit:
	if (pFont->info.firstRow > 0)
	    break;
	if (pFont->info.allExist && pDefault) {
	    while (count--) {
		c = (*chars++) - firstCol;
		if (c < numCols)
		    *glyphs++ = &encoding[c];
		else
		    *glyphs++ = pDefault;
	    }
	} else {
	    while (count--) {
		c = (*chars++) - firstCol;
		if (c < numCols && (pci = &encoding[c])->bits)
		    *glyphs++ = pci;
		else if (pDefault)
		    *glyphs++ = pDefault;
	    }
	}
	break;
    case Linear16Bit:
	if (pFont->info.allExist && pDefault) {
	    while (count--) {
		c = *chars++ << 8;
		c = (c | *chars++) - firstCol;
		if (c < numCols)
		    *glyphs++ = &encoding[c];
		else
		    *glyphs++ = pDefault;
	    }
	} else {
	    while (count--) {
		c = *chars++ << 8;
		c = (c | *chars++) - firstCol;
		if (c < numCols && (pci = &encoding[c])->bits)
		    *glyphs++ = pci;
		else if (pDefault)
		    *glyphs++ = pDefault;
	    }
	}
	break;

    case TwoD16Bit:
	firstRow = pFont->info.firstRow;
	numRows = pFont->info.lastRow - firstRow + 1;
	while (count--) {
	    r = (*chars++) - firstRow;
	    c = (*chars++) - firstCol;
	    if (r < numRows && c < numCols &&
		    (pci = &encoding[r * numCols + c])->bits)
		*glyphs++ = pci;
	    else if (pDefault)
		*glyphs++ = pDefault;
	}
	break;
    }
    *glyphCount = glyphs - glyphsBase;
    return Successful;
}

static CharInfoRec nonExistantChar;

static int
sp_get_metrics(pFont, count, chars, charEncoding, glyphCount, glyphs)
    FontPtr     pFont;
    unsigned long count;
    register unsigned char *chars;
    FontEncoding charEncoding;
    unsigned long *glyphCount;	/* RETURN */
    xCharInfo **glyphs;		/* RETURN */
{
    int         ret;
    SpeedoFontPtr spf;
    CharInfoPtr	oldDefault;

    spf = (SpeedoFontPtr) pFont->fontPrivate;
    oldDefault = spf->pDefault;
    spf->pDefault = &nonExistantChar;
    ret = sp_get_glyphs(pFont, count, chars, charEncoding,
			glyphCount, (CharInfoPtr *) glyphs);

    spf->pDefault = oldDefault;
    return ret;
}

void
sp_fixup_vals(vals)
    FontScalablePtr vals;
{
    fsResolution *res;
    int         x_res = 75;
    int         y_res = 75;
    int         pointsize = 120;
    int         num_res;

    if (!vals->x || !vals->y || (!vals->point && !vals->pixel)) {
	res = (fsResolution *) GetClientResolutions(&num_res);
	if (num_res) {
	    if (res->x_resolution)
		x_res = res->x_resolution;
	    if (res->y_resolution)
		y_res = res->y_resolution;
	    if (res->point_size)
		pointsize = res->point_size;
	}
	if (!vals->x)
	    vals->x = x_res;
	if (!vals->y)
	    vals->y = y_res;
	if (!vals->point)
	    vals->point = pointsize;
    }
}


int
sp_open_font(fontname, filename, entry, format, fmask, flags, spfont)
    char       *fontname,
               *filename;
    FontEntryPtr entry;
    fsBitmapFormat format;
    fsBitmapFormatMask fmask;
    Mask        flags;
    SpeedoFontPtr *spfont;
{
    SpeedoFontPtr spf;
    SpeedoMasterFontPtr spmf;
    int         ret;
    char        tmpname[MAXFONTNAMELEN];
    specs_t     specs;
    FontScalableRec vals;
    double      pointsize;

    /* find a master (create it if necessary) */
    spmf = (SpeedoMasterFontPtr) entry->u.scalable.extra->private;
    if (!spmf)
    {
	ret = sp_open_master(filename, &spmf);
	if (ret != Successful)
	    return ret;
	entry->u.scalable.extra->private = (pointer) spmf;
	spmf->entry = entry;
    }

    spf = (SpeedoFontPtr) xalloc(sizeof(SpeedoFontRec));
    if (!spf)
	return AllocError;
    bzero((char *) spf, sizeof(SpeedoFontRec));

    spf->master = spmf;
    spf->entry = entry;
    spmf->refcount++;
    sp_reset_master(spmf);
    /* now we've done enough that if we bail out we must call sp_close_font */

    /* tear apart name to get sizes */
    strcpy(tmpname, fontname);
    if (!FontParseXLFDName(tmpname, &vals, FONT_XLFD_REPLACE_NONE))
    {
	sp_close_font(spf);
	return BadFontName;
    }

    sp_fixup_vals(&vals);
    if (vals.point > 0)
	pointsize = vals.point;
    else if (vals.pixel > 0) {
	/* make sure we don't caculate it to 0 */
	pointsize = (vals.pixel * 722.70)/vals.y;
    }
    spf->vals.point = pointsize;
    spf->vals.x = vals.x;
    spf->vals.y = vals.y;
    spf->vals.pixel = ((pointsize * vals.y) / 722.7) + 0.5;	/* round it */

    /* set up specs */

    specs.pfont = &spmf->font;
    /* XXX beware of overflow */
    /* Note that point size is in decipoints */
    specs.xxmult = (int) (pointsize * vals.x / 720 * (1 << 16));
    specs.xymult = 0L << 16;
    specs.xoffset = 0L << 16;
    specs.yxmult = 0L << 16;
    specs.yymult = (int) (pointsize * vals.y / 720 * (1 << 16));
    specs.yoffset = 0L << 16;
    specs.flags = MODE_SCREEN;
    specs.out_info = NULL;

    /* When Speedo tries to generate a very small font bitmap, it
       often crashes or goes into an infinite loop.
       Don't know why this is so, but until we can fix it properly,
       return BadFontName for anything smaller than 4 pixels.
       */
#define TINY_FACTOR (4 << 16)
    /* XXX may have to do more tweaking for ROTATED_TEXT */
    if (specs.xxmult < TINY_FACTOR  ||  specs.yymult < TINY_FACTOR)
    {
	sp_close_font(spf);
	return BadFontName;
    }

    /* clobber global state to avoid wrecking future obliqued fonts */
    bzero ((char *) &sp_globals, sizeof(sp_globals));

    if (!sp_set_specs(&specs))
    {
	sp_close_font(spf);
	return BadFontName;
    }

    spf->specs = specs;
    spf->master = spmf;

    *spfont = spf;
    return Successful;
}

static int
sp_load_font(fontname, filename, entry, format, fmask, pfont, flags)
    char       *fontname,
               *filename;
    FontEntryPtr    entry;
    fsBitmapFormat format;
    fsBitmapFormatMask fmask;
    FontPtr     pfont;
    Mask        flags;
{
    SpeedoFontPtr spf;
    SpeedoMasterFontPtr spmf;
    int         esize;
    int         ret;

    ret = sp_open_font(fontname, filename, entry, format, fmask, flags, &spf);

    if (ret != Successful)
	return ret;

    spmf = spf->master;
    sp_reset_master(spmf);
    esize = sizeof(CharInfoRec) * (spmf->max_id - spmf->first_char_id + 1);

    spf->encoding = (CharInfoPtr) xalloc(esize);
    bzero((char *) spf->encoding, esize);
    if (!spf->encoding) {
	sp_close_font(spf);
	return AllocError;
    }
    sp_fp_cur = spf;

    sp_make_header(spf, &pfont->info);

    sp_compute_bounds(spf, &pfont->info, SaveMetrics);

    sp_compute_props(spf, fontname, &pfont->info);

    pfont->fontPrivate = (pointer) spf;

/* XXX */
    flags |= FontLoadBitmaps;

    if (flags & FontLoadBitmaps) {
	sp_fp_cur = spf;
	ret = sp_build_all_bitmaps(pfont, format, fmask);
    }
    if (ret != Successful)
	return ret;

    /* compute remaining accelerators */
    FontComputeInfoAccelerators (&pfont->info);

    pfont->format = format;

    pfont->get_metrics = sp_get_metrics;
    pfont->get_glyphs = sp_get_glyphs;
    pfont->unload_font = SpeedoCloseFont;
    pfont->refcnt = 0;
    pfont->maxPrivate = -1;
    pfont->devPrivates = (pointer *) 0;

    /* have to hold on to master for min/max id */
    sp_close_master_file(spmf);

    return ret;
}

int
SpeedoFontLoad(ppfont, fontname, filename, entry, format, fmask, flags)
    FontPtr    *ppfont;
    char       *fontname;
    char       *filename;
    FontEntryPtr    entry;
    fsBitmapFormat format;
    fsBitmapFormatMask fmask;
    Mask        flags;
{
    FontPtr     pfont;
    int         ret;

    pfont = (FontPtr) xalloc(sizeof(FontRec));
    if (!pfont) {
	return AllocError;
    }
    ret = sp_load_font(fontname, filename, entry, format, fmask, pfont, flags);

    if (ret == Successful)
	*ppfont = pfont;
    else
	xfree (pfont);
    
    return ret;
}

void
sp_close_font(spf)
    SpeedoFontPtr spf;
{
    SpeedoMasterFontPtr spmf;

    spmf = spf->master;
    --spmf->refcount;
    if (spmf->refcount == 0)
	sp_close_master_font (spmf);
    xfree(spf->encoding);
    xfree(spf->bitmaps);
    xfree(spf);
}

void
SpeedoCloseFont(pfont)
    FontPtr     pfont;
{
    SpeedoFontPtr spf;

    spf = (SpeedoFontPtr) pfont->fontPrivate;
    sp_close_font(spf);
    xfree(pfont->info.isStringProp);
    xfree(pfont->info.props);
    xfree(pfont);
}
