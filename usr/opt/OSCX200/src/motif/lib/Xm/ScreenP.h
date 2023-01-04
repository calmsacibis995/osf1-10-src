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
 * @(#)$RCSfile: ScreenP.h,v $ $Revision: 1.1.4.2 $ (DEC) $Date: 1993/05/06 15:47:49 $
 */
/* 
 * (c) Copyright 1989, 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
*/ 
/* 
 * Motif Release 1.2.1
*/ 
/*   $RCSfile: ScreenP.h,v $ $Revision: 1.1.4.2 $ $Date: 1993/05/06 15:47:49 $ */
/*
*  (c) Copyright 1989, 1990  DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASS. */
/*
*  (c) Copyright 1987, 1988, 1989, 1990, 1991, 1992 HEWLETT-PACKARD COMPANY */
/*
*  (c) Copyright 1988 MASSACHUSETTS INSTITUTE OF TECHNOLOGY  */
/*
*  (c) Copyright 1988 MICROSOFT CORPORATION */
#ifndef _XmScreenP_h
#define _XmScreenP_h
#if defined(VMS) || defined (__VMS)
#include <X11/apienvset.h>
#endif

#include <Xm/DesktopP.h>
#include <Xm/Screen.h>
#include <Xm/DragIcon.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _XmScreenClassPart{
    XtPointer		extension;
}XmScreenClassPart, *XmScreenClassPartPtr;

typedef struct _XmScreenClassRec{
    CoreClassPart		core_class;
    XmDesktopClassPart 		desktop_class;
    XmScreenClassPart		screen_class;
}XmScreenClassRec;

typedef struct _XmDragCursorRec{
    struct _XmDragCursorRec	*next;
    Cursor			cursor;
    XmDragIconObject		stateIcon;
    XmDragIconObject		opIcon;
    XmDragIconObject		sourceIcon;
}XmDragCursorRec, *XmDragCursorCache;

typedef struct _XmScratchPixmapRec *XmScratchPixmap;

typedef struct _XmScratchPixmapRec{
    XmScratchPixmap     next;
    Pixmap              pixmap;
    Cardinal		depth;
    Dimension           width;
    Dimension           height;
    Boolean             inUse;
}XmScratchPixmapRec;

typedef struct {
    Boolean		mwmPresent;
    unsigned short	numReparented;
    int			darkThreshold;
    int			foregroundThreshold;
    int			lightThreshold;
    XmDragIconObject	defaultNoneCursorIcon;
    XmDragIconObject	defaultValidCursorIcon;
    XmDragIconObject	defaultInvalidCursorIcon;
    XmDragIconObject	defaultMoveCursorIcon;
    XmDragIconObject	defaultCopyCursorIcon;
    XmDragIconObject	defaultLinkCursorIcon;
    XmDragIconObject	defaultSourceCursorIcon;

    Cursor		nullCursor;
    XmDragCursorRec	*cursorCache;
    Cardinal		maxCursorWidth;
    Cardinal		maxCursorHeight;

    Cursor		menuCursor;
    unsigned char	unpostBehavior;
    XFontStruct *	font_struct;
    int			h_unit;
    int			v_unit;
    XmScratchPixmap	scratchPixmaps;
    unsigned char     moveOpaque;

    /* to save internally-created XmDragIcons */

    XmDragIconObject	xmStateCursorIcon;
    XmDragIconObject	xmMoveCursorIcon;
    XmDragIconObject	xmCopyCursorIcon;
    XmDragIconObject	xmLinkCursorIcon;
    XmDragIconObject	xmSourceCursorIcon;

    GC			imageGC;		/* ImageCache.c */
    int			imageGCDepth;
    Pixel		imageForeground;
    Pixel		imageBackground;

    XtPointer		screenInfo;		/* extension */
} XmScreenPart, *XmScreenPartPtr;

typedef struct _XmScreenInfo {
	/* so much for information hiding */
	XtPointer	menu_state;		/* MenuUtil.c */
	Boolean		destroyCallbackAdded;	/* ImageCache.c */
} XmScreenInfo;

externalref XmScreenClassRec 	xmScreenClassRec;

typedef struct _XmScreenRec{
    CorePart			core;
    XmDesktopPart		desktop;
    XmScreenPart		screen;
}XmScreenRec;

#ifdef DEC_MOTIF_BUG_FIX
externalref XrmQuark _XmInvalidCursorIconQuark ;
externalref XrmQuark _XmValidCursorIconQuark ;
externalref XrmQuark _XmNoneCursorIconQuark ;
externalref XrmQuark _XmDefaultDragIconQuark ;
externalref XrmQuark _XmMoveCursorIconQuark ;
externalref XrmQuark _XmCopyCursorIconQuark ;
externalref XrmQuark _XmLinkCursorIconQuark ;
#else
extern XrmQuark _XmInvalidCursorIconQuark ;
extern XrmQuark _XmValidCursorIconQuark ;
extern XrmQuark _XmNoneCursorIconQuark ;
extern XrmQuark _XmDefaultDragIconQuark ;
extern XrmQuark _XmMoveCursorIconQuark ;
extern XrmQuark _XmCopyCursorIconQuark ;
extern XrmQuark _XmLinkCursorIconQuark ;
#endif

/********    Private Function Declarations    ********/
#ifdef _NO_PROTO

extern XmDragIconObject _XmScreenGetOperationIcon() ;
extern XmDragIconObject _XmScreenGetStateIcon() ;
extern XmDragIconObject _XmScreenGetSourceIcon() ;
extern Pixmap _XmAllocScratchPixmap() ;
extern void _XmFreeScratchPixmap() ;
extern XmDragCursorCache * _XmGetDragCursorCachePtr() ;
extern void _XmGetMaxCursorSize() ;
extern Cursor _XmGetNullCursor() ;
extern Cursor _XmGetMenuCursorByScreen() ;
extern Boolean _XmGetMoveOpaqueByScreen() ;
extern unsigned char _XmGetUnpostBehavior() ;
extern int _XmGetFontUnit() ;
extern void _XmScreenRemoveFromCursorCache() ;

#else

extern XmDragIconObject _XmScreenGetOperationIcon( 
                        Widget w,
#if NeedWidePrototypes
                        unsigned int operation) ;
#else
                        unsigned char operation) ;
#endif /* NeedWidePrototypes */
extern XmDragIconObject _XmScreenGetStateIcon( 
                        Widget w,
#if NeedWidePrototypes
                        unsigned int state) ;
#else
                        unsigned char state) ;
#endif /* NeedWidePrototypes */
extern XmDragIconObject _XmScreenGetSourceIcon( 
                        Widget w) ;
extern Pixmap _XmAllocScratchPixmap( 
                        XmScreen xmScreen,
#if NeedWidePrototypes
                        unsigned int depth,
                        int width,
                        int height) ;
#else
                        Cardinal depth,
                        Dimension width,
                        Dimension height) ;
#endif /* NeedWidePrototypes */
extern void _XmFreeScratchPixmap( 
                        XmScreen xmScreen,
                        Pixmap pixmap) ;
extern XmDragCursorCache * _XmGetDragCursorCachePtr( 
                        XmScreen xmScreen) ;
extern void _XmGetMaxCursorSize( 
                        Widget w,
                        Dimension *width,
                        Dimension *height) ;
extern Cursor _XmGetNullCursor( 
                        Widget w) ;
extern Cursor _XmGetMenuCursorByScreen( 
                        Screen *screen) ;
extern Boolean _XmGetMoveOpaqueByScreen( 
                        Screen *screen) ;
extern unsigned char _XmGetUnpostBehavior( 
                        Widget wid) ;
extern int _XmGetFontUnit( 
                        Screen *screen,
                        int dimension) ;
extern void _XmScreenRemoveFromCursorCache(
			XmDragIconObject icon) ;

#endif /* _NO_PROTO */
/********    End Private Function Declarations    ********/


#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#if defined(VMS) || defined (__VMS)
#include <X11/apienvrst.h>
#endif
#endif /* _XmScreenP_h */
/* DON'T ADD STUFF AFTER THIS #endif */
