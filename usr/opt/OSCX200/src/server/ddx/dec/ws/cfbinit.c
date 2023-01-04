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
/* $XConsortium: cfbinit.c,v 1.3 92/04/06 18:18:23 keith Exp $ */
/***********************************************************
Copyright 1991 by Digital Equipment Corporation, Maynard, Massachusetts,
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
/*
        Todo.
        -----
  ****  Fix plane mask handling.  The screen devprivate should have
        a hardware specific planemask setting proc.  For now, since
        pmax is the only machine with a plane mask, so keep it global
        for now.
*/

#include <stdio.h>
#include <sys/types.h>
#include <sys/file.h>
#include <sys/time.h>
#include <sys/tty.h>
#include <errno.h>
#include <sys/devio.h>


#include "misc.h"
#include "X.h"
#include "scrnintstr.h"
#include "pixmap.h"
#include "input.h"
#include "cursorstr.h"
#include "regionstr.h"
#include "resource.h"
#include "dixstruct.h"

#include <sys/workstation.h>
#include <sys/inputdevice.h>
#include "ws.h"

#include "mfb.h"

extern int lastEventTime;
extern int defaultColorVisualClass;

Bool wsRealizeCursor();
Bool wsUnrealizeCursor();
Bool wsDisplayCursor();
void wsRecolorCursor();
void wsCursorControl();
Bool wsSetCursorPosition();
void wsCursorLimits();
void wsConstrainCursor();
void wsPointerNonInterestBox();
void wsChangeKeyboardControl();
void wsChangePointerControl();
void wsClick();

extern	ws_event_queue	*queue;
#undef VSYNCFIXED
#ifdef VSYNCFIXED
#define CURRENT_TIME	queue->time
#else
#define CURRENT_TIME	GetTimeInMillis()
#endif

extern int wsScreenInit();
extern void miRecolorCursor();
extern void NoopDDA();
extern void NewCurrentScreen();
extern int wsFd;
extern ws_screen_descriptor screenDesc[];
static ws_depth_descriptor depthDesc[MAXSCREENS];
static mapped[MAXSCREENS];

/*
 * We don't do anything of note if there's no plane mask register 
 */

unsigned int
cfbpmaxSetPlaneMask(planemask, pScreen)
    unsigned int planemask;
    ScreenPtr pScreen;
{
    /* shadow a writeonly reg */
    static unsigned int currentmask[MAXSCREENS] = {
	~0, ~0, ~0,
#if MAXSCREENS > 3
	~0,
#endif
#if MAXSCREENS > 4
	~0,
#endif
#if MAXSCREENS > 5
	~0,
#endif
#if MAXSCREENS > 6
	~0,
#endif
#if MAXSCREENS > 7
	We don't handle MAXSCREENS > 7 in this initialization. Add more...
#endif
	};
    unsigned int prev,*pm;
    int screen_no = WS_SCREEN(pScreen);

    prev = currentmask[screen_no];
    currentmask[screen_no] =  planemask;

    if(pm = (u_int *)depthDesc[screen_no].plane_mask)
    	*pm = planemask;

    return(prev);
}


void
wsQueryBestSize16(class, pwidth, pheight)
    int class;
    short *pwidth;
    short *pheight;
{
    unsigned width, test;

    if (*pwidth > 0)
    {
      switch(class)
      {
        case CursorShape:
	  *pwidth = 16;
	  *pheight = 16;
	  break;
	default: 
	  mfbQueryBestSize(class, pwidth, pheight);
	  break;
       }
    }
}


void
wsQueryBestSize64(class, pwidth, pheight)
    int class;
    short *pwidth;
    short *pheight;
{
    unsigned width, test;

    if (*pwidth > 0)
    {
      switch(class)
      {
        case CursorShape:
	  *pwidth = 64;
	  *pheight = 64;
	  break;
	default: 
	  mfbQueryBestSize(class, pwidth, pheight);
	  break;
       }
    }
}

Bool
wsScreenClose(index, pScreen)
    int index;
    ScreenPtr pScreen;
{
    wsScreenPrivate *wsp = (wsScreenPrivate *)
		pScreen->devPrivates[wsScreenPrivateIndex].ptr;

    pScreen->CloseScreen = wsp->CloseScreen;
    Xfree(wsp);
    return (*pScreen->CloseScreen) (index, pScreen);
}


static void
colorNameToColor( pname, pred, pgreen, pblue)
    char *      pname;
    unsigned short *     pred;
    unsigned short *     pgreen;
    unsigned short *     pblue;
{
    if ( *pname == '#')
    {
        pname++;                /* skip over # */
        sscanf( pname, "%2x", pred);
        *pred <<= 8;

        pname += 2;
        sscanf( pname, "%2x", pgreen);
        *pgreen <<= 8;

        pname += 2;
        sscanf( pname, "%2x", pblue);
        *pblue <<= 8;
    }
    else /* named color */
    {
        OsLookupColor( 0 /*"screen", not used*/, pname, strlen( pname),
                pred, pgreen, pblue);
    }
}

extern Bool mfbScreenInit (), mcfbScreenInit();

/* Convenience routine to allocate the workstation screen private
 * info, index the global structures properly so they don't
 * have to be global. index should be the index that AddScreen
 * passes down. Once this routine indexes screenDesc for all
 * screens at init, screenDesc will no longer be kept in sync
 * with the rest of the server. Instead, wsp->screenDesc should
 * be referenced to obtain the screenDesc info. Same is true
 * about the args.
 */
wsScreenPrivate * wsAllocScreenInfo(index, pScreen)
    int index;
    ScreenPtr pScreen;
{
    wsScreenPrivate 		*wsp;
    wsp = (wsScreenPrivate *) Xalloc(sizeof(wsScreenPrivate));
    wsp->pInstalledMap = NOMAPYET;
    pScreen->devPrivates[wsScreenPrivateIndex].ptr = (pointer) wsp;

    wsp->screenDesc = &screenDesc[index];    
    wsp->args = &screenArgs[index];

    wsScreens[screenDesc[index].screen] = pScreen;

    return (wsp);
}

/* Set default color visual class for screen. Do not override
 * server global default color visual class
 */
int
wsDefaultColorVisualClass(pScreen)
    ScreenPtr pScreen;
{
    ws_visual_descriptor vd;
    wsScreenPrivate 	*wsp = WSP_PTR(pScreen);

    if(wsp->args->flags & ARG_CLASS)
      return(wsp->args->class);

    if(defaultColorVisualClass != -1)
      return(defaultColorVisualClass);

    vd.screen = WS_SCREEN(pScreen);
    vd.which_visual = 0;
    if (ioctl(wsFd, GET_VISUAL_INFO, &vd) == -1)
    {
      ErrorF("GET_VISUAL_INFO failed");
      return(-1);
    }
    return(vd.screen_class);
}

typedef Bool (*BoolProc) ();

/* Some ddx's can use most of fbInitProc, but require a small hook
 * that would replace the call to cfbScreenInit. This is that hook
 * so that interfaces don't have to be changed.
 */
BoolProc wsAltScreenInitProc = (BoolProc)NULL;

/* Note: index is the logical screen. screenDesc[index].screen has the 
 * physical screen number.
 */
Bool
fbInitProc(index, pScreen, argc, argv)
    int index;
    ScreenPtr pScreen;
    int argc;
    char **argv;
{
    int				dpix, dpiy, i;
    static int  		mapOnce = FALSE;
    wsScreenPrivate 		*wsp;
    int	    			depthIndex;
    BoolProc			screenInit;
    int				psn; /* physical screen number */
    ws_depth_descriptor	 	*dd;
    ws_visual_descriptor	vd;
    static ws_map_control 	mc;
    VisualPtr	    		pVisual;
    ColormapPtr	    		pCmap;
/* for initializing color map entries */
    unsigned short 		blackred      = 0x0000;
    unsigned short 		blackgreen    = 0x0000; 
    unsigned short 		blackblue     = 0x0000;

    unsigned short 		whitered      = 0xffff;
    unsigned short 		whitegreen    = 0xffff;
    unsigned short 		whiteblue     = 0xffff;

    lastEventTime = CURRENT_TIME;

    wsp = wsAllocScreenInfo(index, pScreen);
    psn = screenDesc[index].screen;

    /* since driver does not support unmap (yet), only map screen once */
      if (! mapped[psn]) {
  	depthDesc[psn].screen = psn;
	depthIndex = -1;
	for (i = 0; i < wsp->screenDesc->allowed_depths; i++) 
	{
	    extern int forceDepth;

	    depthDesc[psn].which_depth = i;	
	    if (ioctl(wsFd, GET_DEPTH_INFO, &depthDesc[psn]) == -1) {
		ErrorF("GET_DEPTH_INFO failed");
		exit (1);
  	    }
	    if (forceDepth)
		depthDesc[psn].depth = forceDepth;
	    switch (depthDesc[psn].bits_per_pixel) {
	    case 1:
	    case 8:
	    case 16:
	    case 32:
		break;
	    default:
		continue;
	    }
	    depthIndex = i;
  	}
	if (depthIndex == -1) return FALSE;
  
  	mc.screen = psn;
	mc.which_depth = depthIndex;
  	mc.map_unmap = MAP_SCREEN;

	if (ioctl(wsFd,  MAP_SCREEN_AT_DEPTH, &mc) == -1)    {
	    ErrorF("MAP_SCREEN_AT_DEPTH failed");
	    free ((char *) wsp);
	    return FALSE;
	}
	/* 
	 * reget the depth desc.  It now contains the user-mapped bitmap
	 * addr. 
	 */

	if (ioctl(wsFd, GET_DEPTH_INFO, &depthDesc[psn]) == -1) 
	{
	    ErrorF("GET_DEPTH_INFO failed");
	    return FALSE;
	}
	if (forceDepth)
	    depthDesc[psn].depth = forceDepth;

	mapped[psn] = TRUE;
    }

    /* ws routines knows how to initialize many functions, so call init. */
    if (wsScreenInit(index, pScreen, argc, argv) == -1) 
	return FALSE;

    dd = &depthDesc[psn];

    /* Don't leave screen 0 on here since psn 0 may not be lsn 0 
     * We'll turn on the cursor later.
     */
    /*
    if (psn > 0) 
    */
    wsCursorControl(psn, CURSOR_OFF);

/* 
 * this is really dumb.  The driver has the screen geometry in mm.
 * The screen wants it stored as mm, but the damn interface passes
 * inches.  mm => inches => mm.  What a waste.  Should we change cfbscrinit.c?
 * -jmg.
 */

    if (screenArgs[psn].flags & ARG_DPIX)
	dpix = screenArgs[psn].dpix;
    else
	dpix =  ((wsp->screenDesc->width * 254) + 
		 (wsp->screenDesc->monitor_type.mm_width * 5) ) /
		 (wsp->screenDesc->monitor_type.mm_width  * 10);

    if (screenArgs[psn].flags & ARG_DPIY)
	dpiy = screenArgs[psn].dpiy;
    else
	dpiy =  ((wsp->screenDesc->height * 254) + 
		 (wsp->screenDesc->monitor_type.mm_height * 5) ) / 
		 (wsp->screenDesc->monitor_type.mm_height * 10);

    /* Might want to make this table driven  - jmg */

    switch (dd->bits_per_pixel)
    {
    case 1:
	screenInit = mfbScreenInit;
	break;
    case 8:
    case 16:
    case 32:
	screenInit = mcfbScreenInit;
	break;
    }
    if (dd->depth == 1) 
    {
	pScreen->blackPixel = 0;
	pScreen->whitePixel = 1;
    	if(screenArgs[psn].flags & ARG_BLACKVALUE)
	    if((i = atoi(screenArgs[psn].blackValue)) == 0 || i == 1)
	    	pScreen->blackPixel = i;
	    else
	    	wsPixelError(psn);
    
    	if(screenArgs[psn].flags & ARG_WHITEVALUE)
	    if((i = atoi(screenArgs[psn].whiteValue)) == 0 || i == 1)
	    	pScreen->whitePixel = i;
	    else
	    	wsPixelError(psn);
    }
    else
    {
    	if(screenArgs[psn].flags & ARG_BLACKVALUE)
	    colorNameToColor(screenArgs[psn].blackValue, &blackred,
			     &blackgreen, &blackblue); 
    
    	if(screenArgs[psn].flags & ARG_WHITEVALUE)
	    colorNameToColor(screenArgs[psn].whiteValue, &whitered, 
			    &whitegreen, &whiteblue);
    }


    if ( wsAltScreenInitProc != (BoolProc)NULL ) {
	Bool res;
        
	res = (*wsAltScreenInitProc) (pScreen, 
	       dd->plane_mask, wsp->screenDesc->width,
	       wsp->screenDesc->height, dpix, dpiy, dd->fb_width, 
	       dd->bits_per_pixel, dd->depth, dd->plane_mask_phys);
	
	/* reset so it doesn't get used for the next caller. */
	wsAltScreenInitProc = (BoolProc)NULL;

	if (!res) {
	    return FALSE;
        }
    }
    else {
#ifndef ultrix
        if (!(*screenInit) (pScreen, dd->pixmap, wsp->screenDesc->width,
            wsp->screenDesc->height, dpix, dpiy, dd->fb_width,
            dd->bits_per_pixel, dd->depth))
#else
        if (!(*screenInit) (pScreen, dd->plane_mask, wsp->screenDesc->width,
            wsp->screenDesc->height, dpix, dpiy, dd->fb_width,
            dd->bits_per_pixel, dd->depth))
#endif
        {
	        return FALSE;
        }
    }
    /* copy of cfbCreateDefColormap, except variable colors */
    for (pVisual = pScreen->visuals;
	 pVisual->vid != pScreen->rootVisual;
	 pVisual++)
	;

    if (CreateColormap(pScreen->defColormap, pScreen, pVisual, &pCmap,
		       (pVisual->class & DynamicClass) ? AllocNone : AllocAll,
		       0)
	!= Success)
	return FALSE;
    if ((AllocColor(pCmap, &whitered, &whitegreen, &whiteblue,
		    &(pScreen->whitePixel), 0) != Success) ||
	(AllocColor(pCmap, &blackred, &blackgreen, &blackblue,
		    &(pScreen->blackPixel), 0) != Success))
    {
	return FALSE;
    }
    (*pScreen->InstallColormap)(pCmap);
  
    /* Wrap screen close routine to avoid memory leak */
    wsp->CloseScreen = pScreen->CloseScreen;
    pScreen->CloseScreen = wsScreenClose;

    if(wsp->screenDesc->cursor_width == 64)
	 pScreen->QueryBestSize = wsQueryBestSize64;
    else
	 pScreen->QueryBestSize = wsQueryBestSize16;
    return TRUE;
}
