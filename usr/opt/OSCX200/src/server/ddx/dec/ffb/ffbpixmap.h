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
 * @(#)$RCSfile: ffbpixmap.h,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/11/19 21:12:12 $
 */
/*
 */

/*****************************************************************************

Declares a wrapper structure for pixmaps.  The problem we're solving here is
that we need to associate buffer specific info with each drawable.  Windows
have the devPrivates field into which we can stuff this stuff; pixmaps don't.
So we wrap this structure around the cfb pixmap record.  cfb code won't know
it's there; ffb non-copy code will know it is; and ffb copy code (which might
get fed a pixmap not allocated by us) doesn't need it since that code is broken
out into scr_scr, mem_scr, and scr_mem (the plain cfb pixmap would always
correspond to a main memory src or dst).

This is really just a hack.  Joel is reporting the pixmap record definition to
mit as a bug.  In the meantime, use only the provided macros to get at the
'hidden' field.

******************************************************************************/

#ifndef _N_FFBPIXMAP_H
#define _N_FFBPIXMAP_H

/*

Make width (in pixels) mod 8 = 4

Note that ALL drawables in ffb+ VRAM memory MUST use this padding.  In order to
avoid reloading the pixel shift register every scanline, the screen->screen
copy code depends upon the fact that all scanlines in all drawables are padded
identically.  This code will fail miserably if this assumption if violated.

*/

#define FFB_SCANLINE_PAD(width)     (((width + 3) & ~7) + 4)

/*
#undef FFB_SCANLINE_PAD
#define  FFB_SCANLINE_PAD(width) ((width + 7) & ~ 7)
*/

extern PixmapPtr    ffbCreatePixmap();
extern Bool	    ffbDestroyPixmap();
extern PixmapPtr    ffbScreenToMainPixmap();
extern pointer      ffbScreenMalloc();
extern void	    ffbScreenFree();
extern void	    ffbPadPixmap();
extern PixmapPtr    ffbExpandTile();
#endif /* _N_FFBPIXMAP_H */

/*
 * HISTORY
 */

