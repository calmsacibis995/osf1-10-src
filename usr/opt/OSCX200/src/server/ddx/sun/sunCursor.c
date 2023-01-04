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
/* $XConsortium: sunCursor.c,v 5.9 91/11/14 13:57:03 keith Exp $ */
/*-
 * sunCursor.c --
 *	Functions for maintaining the Sun software cursor...
 *
 */

#define NEED_EVENTS
#include    "sun.h"
#include    <windowstr.h>
#include    <regionstr.h>
#include    <dix.h>
#include    <dixstruct.h>
#include    <opaque.h>

#include    <servermd.h>
#include    "mipointer.h"
#include    "cursorstr.h"

#ifdef FBIOGCURMAX  /* has hardware cursor kernel support */

#define GetCursorPrivate(s) (&(GetScreenPrivate(s)->hardwareCursor))
#define SetupCursor(s)	    sunCursorPtr pCurPriv = GetCursorPrivate(s)

static Bool
sunRealizeCursor (pScreen, pCursor)
    ScreenPtr	pScreen;
    CursorPtr	pCursor;
{
    return TRUE;
}

static Bool
sunUnrealizeCursor (pScreen, pCursor)
    ScreenPtr	pScreen;
    CursorPtr	pCursor;
{
    return TRUE;
}

sunInitFakePixmap (pScreen, p, w, h, bits)
    ScreenPtr	    pScreen;
    PixmapPtr	    p;
    int		    w, h;
    unsigned char   *bits;
{
    p->drawable.type = DRAWABLE_PIXMAP;
    p->drawable.class = 0;
    p->drawable.pScreen = pScreen;
    p->drawable.depth = 1;
    p->drawable.bitsPerPixel = 1;
    p->drawable.id = 0;
    p->drawable.serialNumber = NEXT_SERIAL_NUMBER;
    p->drawable.x = 0;
    p->drawable.y = 0;
    p->drawable.width = w;
    p->drawable.height = h;
    p->devKind = PixmapBytePad(w, 1);
    p->refcnt = 1;
    p->devPrivate.ptr = (pointer) bits;
}

static void
sunCursorRepad (pScreen, bits, src_bits, dst_bits, ptSrc, w, h)
    ScreenPtr	    pScreen;
    CursorBitsPtr   bits;
    unsigned char   *src_bits, *dst_bits;
    DDXPointPtr	    ptSrc;
    int		    w, h;
{
    SetupCursor(pScreen);
    PixmapRec	src, dst;
    BoxRec	box;
    RegionRec	rgnDst;

    sunInitFakePixmap (pScreen, &src, bits->width, bits->height, src_bits);
    sunInitFakePixmap (pScreen, &dst, w, h, dst_bits);
    box.x1 = 0;
    box.y1 = 0;
    box.x2 = w;
    box.y2 = h;
    (*pScreen->RegionInit)(&rgnDst, &box, 1);
    mfbDoBitblt(&src, &dst, GXcopy, &rgnDst, ptSrc);
    (*pScreen->RegionUninit)(&rgnDst);
}

static void
sunLoadCursor (pScreen, pCursor, x, y)
    ScreenPtr	pScreen;
    CursorPtr	pCursor;
    int		x, y;
{
    SetupCursor(pScreen);
    struct fbcursor fbcursor;
    int	w, h;
    unsigned char   r[2], g[2], b[2];
    DDXPointRec	ptSrc;
    unsigned char   source_temp[1024], mask_temp[1024];

    fbcursor.set = FB_CUR_SETALL;
    fbcursor.enable = 1;
    fbcursor.pos.x = x;
    fbcursor.pos.y = y;
    fbcursor.hot.x = pCursor->bits->xhot;
    fbcursor.hot.y = pCursor->bits->yhot;
    r[0] = pCursor->backRed >> 8;
    g[0] = pCursor->backGreen >> 8;
    b[0] = pCursor->backBlue >> 8;
    r[1] = pCursor->foreRed >> 8;
    g[1] = pCursor->foreGreen >> 8;
    b[1] = pCursor->foreBlue >> 8;
    fbcursor.cmap.index = 0;
    fbcursor.cmap.count = 2;
    fbcursor.cmap.red = r;
    fbcursor.cmap.green = g;
    fbcursor.cmap.blue = b;
    fbcursor.image = (char *) pCursor->bits->source;
    fbcursor.mask = (char *) pCursor->bits->mask;
    w = pCursor->bits->width;
    h = pCursor->bits->height;
    if (w > pCurPriv->width || h > pCurPriv->height) {
	ptSrc.x = 0;
	ptSrc.y = 0;
	if (w > pCurPriv->width)
	    w = pCurPriv->width;
	if (h > pCurPriv->height)
	    h = pCurPriv->height;
	sunCursorRepad (pScreen, pCursor->bits, pCursor->bits->source,
			source_temp, &ptSrc, w, h);
	sunCursorRepad (pScreen, pCursor->bits, pCursor->bits->mask,
			mask_temp, &ptSrc, w, h);
	fbcursor.image = (char *) source_temp;
	fbcursor.mask = (char *) mask_temp;
    }
    fbcursor.size.x = w;
    fbcursor.size.y = h;
    (void) ioctl (sunFbs[pScreen->myNum].fd, FBIOSCURSOR, &fbcursor);
}

static void
sunSetCursor (pScreen, pCursor, x, y)
    ScreenPtr	pScreen;
    CursorPtr	pCursor;
    int		x, y;
{
    SetupCursor(pScreen);

    if (pCursor)
    	sunLoadCursor (pScreen, pCursor, x, y);
    else
	sunDisableCursor (pScreen);
}

static void
sunMoveCursor (pScreen, x, y)
    ScreenPtr	pScreen;
    int		x, y;
{
    SetupCursor(pScreen);
    struct fbcursor fbcursor;
    struct fbcurpos pos;

    pos.x = x;
    pos.y = y;
    ioctl (sunFbs[pScreen->myNum].fd, FBIOSCURPOS, &pos);
}

miPointerSpriteFuncRec sunPointerSpriteFuncs = {
    sunRealizeCursor,
    sunUnrealizeCursor,
    sunSetCursor,
    sunMoveCursor,
};

static void
sunQueryBestSize (class, pwidth, pheight, pScreen)
    int	class;
    short   *pwidth, *pheight;
    ScreenPtr	pScreen;
{
    SetupCursor (pScreen);

    switch (class)
    {
    case CursorShape:
	if (*pwidth > pCurPriv->width)
	    *pwidth = pCurPriv->width;
	if (*pheight > pCurPriv->height)
	    *pheight = pCurPriv->height;
	if (*pwidth > pScreen->width)
	    *pwidth = pScreen->width;
	if (*pheight > pScreen->height)
	    *pheight = pScreen->height;
	break;
    default:
	mfbQueryBestSize (class, pwidth, pheight, pScreen);
	break;
    }
}

extern miPointerScreenFuncRec	sunPointerScreenFuncs;

#endif

Bool
sunCursorInitialize (pScreen)
    ScreenPtr	pScreen;
{
#ifdef FBIOGCURMAX
    SetupCursor (pScreen);
    int	    fd;
    struct fbcursor fbcursor;
    struct fbcurpos maxsize;
    char    *source, *mask;
    int	    h, w;
    int	    size;

    pCurPriv->has_cursor = FALSE;
    if (ioctl (sunFbs[pScreen->myNum].fd, FBIOGCURMAX, &maxsize) == -1)
	return FALSE;
    pCurPriv->width = maxsize.x;
    pCurPriv->height= maxsize.y;
    pScreen->QueryBestSize = sunQueryBestSize;
    miPointerInitialize (pScreen,
			 &sunPointerSpriteFuncs,
			 &sunPointerScreenFuncs,
			 FALSE);
    pCurPriv->has_cursor = TRUE;
    return TRUE;
#else
    return FALSE;
#endif
}

sunDisableCursor (pScreen)
    ScreenPtr	pScreen;
{
#ifdef FBIOGCURMAX
    SetupCursor (pScreen);
    struct fbcursor fbcursor;

    if (pCurPriv->has_cursor)
    {
    	fbcursor.set = FB_CUR_SETCUR;
    	fbcursor.enable = 0;
    	(void) ioctl (sunFbs[pScreen->myNum].fd, FBIOSCURSOR, &fbcursor);
    }
#endif
}

/*
 * The following struct is from win_cursor.h.  This file can't be included 
 * directly, because it drags in all of the SunView attribute stuff along 
 * with it.
 */

#ifdef SUN_WINDOWS

struct cursor {
    short       cur_xhot, cur_yhot;	/* offset of mouse position from shape */
    int         cur_function;		/* relationship of shape to screen */
    struct pixrect *cur_shape;		/* memory image to use */
    int         flags;			/* various options */
    short       horiz_hair_thickness;	/* horizontal crosshair height */
    int         horiz_hair_op;		/* drawing op       */
    int         horiz_hair_color;	/* color            */
    short       horiz_hair_length;	/* width           */
    short       horiz_hair_gap;		/* gap             */
    short       vert_hair_thickness;	/* vertical crosshair width  */
    int         vert_hair_op;		/* drawing op       */
    int         vert_hair_color;	/* color            */
    short       vert_hair_length;	/* height           */
    short       vert_hair_gap;		/* gap              */
};
#endif SUN_WINDOWS

void
sunInitCursor ()
{
    if ( sunUseSunWindows() ) {
#ifdef SUN_WINDOWS
	static struct cursor cs;
	static struct pixrect pr;
	/* 
	 * Give the pixwin an empty cursor so that the kernel's cursor drawing 
	 * doesn't conflict with our cursor drawing.
	 */
	cs.cur_xhot = cs.cur_yhot = cs.cur_function = 0;
	cs.flags = 0;
	cs.cur_shape = &pr;
	pr.pr_size.x = pr.pr_size.y = 0;
	win_setcursor( windowFd, &cs );
#endif SUN_WINDOWS
    }
}


#ifdef SUN_WINDOWS
/*
 * We need to find out when dix warps the mouse so we can
 * keep SunWindows in sync.
 */

extern int sunIgnoreEvent;

Bool
sunSetCursorPosition(pScreen, x, y, generateEvent)
	ScreenPtr pScreen;
	int x, y;
	Bool generateEvent;
{
    SetupScreen(pScreen);

    pScreen->SetCursorPosition = pPrivate->SetCursorPosition;
    (*pScreen->SetCursorPosition) (pScreen, x, y, generateEvent);
    if (sunUseSunWindows())
	if (!sunIgnoreEvent)
	    win_setmouseposition(windowFd, x, y);
    pScreen->SetCursorPosition = sunSetCursorPosition;
    return TRUE;
}
#endif
