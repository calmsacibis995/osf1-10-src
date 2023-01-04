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
/***********************************************************
Copyright 1987 by Digital Equipment Corporation, Maynard, Massachusetts,
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

#include "xfb.h"

#include "pixmap.h"
#include "region.h"
#include "colormap.h"

/* edg: output of C prototyper needs to be rearranged */
extern int xfbListInstalledColormaps();
extern void xfbInstallColormap();
extern void xfbUninstallColormap();
extern void xfbResolveColor();
extern Bool xfbInitializeColormap();
extern void xfbGetImage();
extern void xfbGetSpans();
extern Bool xfbCreateGC();
extern PixmapPtr xfbCreatePixmap();
extern Bool xfbDestroyPixmap();
extern void xfbPaintWindow();
extern void xfbWindowExposures();
extern Bool xfbScreenInit();
extern int xfbGetSelectDepth();
extern Bool xfbCreateWindow();
extern Bool xfbDestroyWindow();
extern Bool xfbMapWindow();
extern Bool xfbPositionWindow();
extern Bool xfbUnmapWindow();
extern void xfbFindDepthAndHwCmap();
extern void xfbCopyWindowSetCmaps();
extern void xfbCopyWindow();
extern Bool xfbChangeWindowAttributes();
extern void xfbSaveAreas();
extern void xfbRestoreAreas();



/* included from mfb.h; we can't include mfb.h directly because of other 
 * conflicts */
extern void mfbPushPixels();
extern void mfbSetSpans();
extern void mfbGetSpans();
extern void mfbUnnaturalTileFS();
extern void mfbUnnaturalStippleFS();
extern Bool mfbRealizeFont();
extern Bool mfbUnrealizeFont();
extern void mfbQueryBestSize();
extern RegionPtr mfbPixmapToRegion();
extern void mfbCopyRotatePixmap();
