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
 * (c) Copyright 1989, 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
*/ 
/* 
 * Motif Release 1.2.2
*/ 
/*   $RCSfile: DragUnderI.h,v $ $Revision: 1.1.4.3 $ $Date: 1993/07/15 16:22:18 $ */
/*
*  (c) Copyright 1990, 1991, 1992 HEWLETT-PACKARD COMPANY */

#ifndef _XmDragUnderI_h
#define _XmDragUnderI_h
#if defined(VMS) || defined (__VMS)
#include <X11/apienvset.h>
#endif

#include <Xm/XmP.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Structure describing a pixmap */

typedef struct _PixmapData {
    Pixmap		pixmap;
    int			x, y;
    unsigned int	width, height;
} PixmapData;

typedef struct _XmAnimationSaveData {
    Display		*display;
    XmScreen		xmScreen;
    Window		window;
    Position		windowX;
    Position		windowY;
    unsigned int	windowDepth;
    XmRegion		clipRegion;
    XmRegion		dropSiteRegion;
    Dimension		shadowThickness;
    Dimension		highlightThickness;
    Pixel		background;
    Pixel		foreground;
    Pixel		highlightColor;
    Pixmap		highlightPixmap;
    Pixel		topShadowColor;
    Pixmap		topShadowPixmap;
    Pixel		bottomShadowColor;
    Pixmap		bottomShadowPixmap;

    Dimension		borderWidth;
    Pixmap		animationMask;
    Pixmap		animationPixmap;
    unsigned int	animationPixmapDepth;
    unsigned char	animationStyle;
    Widget		dragOver;

    GC			highlightGC;
    GC			topShadowGC;
    GC			bottomShadowGC;
    GC			drawGC;
    PixmapData		*savedPixmaps;
    Cardinal		numSavedPixmaps;
} XmAnimationSaveDataRec, *XmAnimationSaveData;

#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#if defined(VMS) || defined (__VMS)
#include <X11/apienvrst.h>
#endif
#endif /* _XmDragUnderI_h */
