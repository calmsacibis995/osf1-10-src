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
/* $XConsortium: Drawing.h,v 1.10 91/07/22 23:45:48 converse Exp $
 *
 * Copyright 1988 by the Massachusetts Institute of Technology
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted, provided 
 * that the above copyright notice appear in all copies and that both that 
 * copyright notice and this permission notice appear in supporting 
 * documentation, and that the name of M.I.T. not be used in advertising
 * or publicity pertaining to distribution of the software without specific, 
 * written prior permission. M.I.T. makes no representations about the 
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 * The X Window System is a Trademark of MIT.
 *
 * The interfaces described by this header file are for miscellaneous utilities
 * and are not part of the Xlib standard.
 */

#ifndef _XMU_DRAWING_H_
#define _XMU_DRAWING_H_

#include <X11/Xfuncproto.h>

#if NeedFunctionPrototypes
#include <stdio.h>
#if ! defined(_XtIntrinsic_h) && ! defined(PIXEL_ALREADY_TYPEDEFED)
typedef unsigned long Pixel;
#endif
#endif

_XFUNCPROTOBEGIN

extern void XmuDrawRoundedRectangle(
#if NeedFunctionPrototypes
    Display*	/* dpy */,
    Drawable 	/* draw */,
    GC 		/* gc */,
    int		/* x */,
    int		/* y */,
    int		/* w */,
    int		/* h */,
    int		/* ew */,
    int		/* eh */
#endif
);

extern void XmuFillRoundedRectangle(
#if NeedFunctionPrototypes
    Display*	/* dpy */,
    Drawable 	/* draw */,
    GC 		/* gc */,
    int		/* x */,
    int		/* y */,
    int		/* w */,
    int		/* h */,
    int		/* ew */,
    int		/* eh */
#endif
);

extern void XmuDrawLogo(
#if NeedFunctionPrototypes
    Display*	/* dpy */,
    Drawable 	/* drawable */,
    GC		/* gcFore */,
    GC		/* gcBack */,
    int		/* x */,
    int		/* y */,
    unsigned int /* width */,
    unsigned int /* height */
#endif
);

extern Pixmap XmuCreatePixmapFromBitmap(
#if NeedFunctionPrototypes
    Display*		/* dpy */,
    Drawable 		/* d */,
    Pixmap 		/* bitmap */,
    unsigned int	/* width */,
    unsigned int	/* height */,
    unsigned int	/* depth */,
    unsigned long	/* fore */,
    unsigned long	/* back */
#endif
);

extern Pixmap XmuCreateStippledPixmap(
#if NeedFunctionPrototypes
    Screen*		/* screen */,
    Pixel		/* fore */,
    Pixel		/* back */,
    unsigned int	/* depth */
#endif
);

extern void XmuReleaseStippledPixmap(
#if NeedFunctionPrototypes
    Screen*		/* screen */,
    Pixmap 		/* pixmap */
#endif
);

extern Pixmap XmuLocateBitmapFile(
#if NeedFunctionPrototypes
    Screen*		/* screen */,
    _Xconst char*	/* name */,
    char*		/* srcname_return */,
    int 		/* srcnamelen */,
    int*		/* width_return */,
    int*		/* height_return, */,
    int*		/* xhot_return */,
    int*		/* yhot_return */
#endif
);

extern Pixmap XmuLocatePixmapFile(
#if NeedFunctionPrototypes
    Screen*		/* screen */,
    _Xconst char*	/* name */,
    unsigned long	/* fore */,
    unsigned long	/* back */,
    unsigned int	/* depth */,
    char*		/* srcname_return */,
    int 		/* srcnamelen */,
    int*		/* width_return */,
    int*		/* height_return, */,
    int*		/* xhot_return */,
    int*		/* yhot_return */
#endif
);

extern int XmuReadBitmapData(
#if NeedFunctionPrototypes
    FILE*		/* fstream */,
    unsigned int*	/* width_return */,
    unsigned int*	/* height_return */,
    unsigned char**	/* datap_return */,
    int*		/* xhot_return */,
    int*		/* yhot_return */
#endif
);

extern int XmuReadBitmapDataFromFile(
#if NeedFunctionPrototypes
    _Xconst char*	/* filename */,
    unsigned int*	/* width_return */,
    unsigned int*	/* height_return */,
    unsigned char**	/* datap_return */,
    int*		/* xhot_return */,
    int*		/* yhot_return */
#endif
);

_XFUNCPROTOEND

#endif /* _XMU_DRAWING_H_ */
